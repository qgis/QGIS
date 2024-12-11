/***************************************************************************
  qgs3dmaptoolidentify.cpp
  --------------------------------------
  Date                 : Sep 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmaptoolidentify.h"
#include "moc_qgs3dmaptoolidentify.cpp"

#include <QScreen>

#include "qgsapplication.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgsvector3d.h"

#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgscoordinateutils.h"
#include "qgsmaptoolidentifyaction.h"

#include "qgspointcloudlayer.h"
#include "qgstiledscenelayer.h"
#include "qgscameracontroller.h"


Qgs3DMapToolIdentify::Qgs3DMapToolIdentify( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
}

Qgs3DMapToolIdentify::~Qgs3DMapToolIdentify() = default;


void Qgs3DMapToolIdentify::mousePressEvent( QMouseEvent *event )
{
  mMouseHasMoved = false;
  mMouseClickPos = event->pos();
}

void Qgs3DMapToolIdentify::mouseMoveEvent( QMouseEvent *event )
{
  if ( !mMouseHasMoved && ( event->pos() - mMouseClickPos ).manhattanLength() >= QApplication::startDragDistance() )
  {
    mMouseHasMoved = true;
  }
}

void Qgs3DMapToolIdentify::mouseReleaseEvent( QMouseEvent *event )
{
  if ( event->button() != Qt::MouseButton::LeftButton || mMouseHasMoved )
    return;

  const QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( event->pos(), mCanvas->size(), mCanvas->cameraController()->camera() );
  QHash<QgsMapLayer *, QVector<QgsRayCastingUtils::RayHit>> allHits = Qgs3DUtils::castRay( mCanvas->scene(), ray, QgsRayCastingUtils::RayCastContext( false, mCanvas->size(), mCanvas->cameraController()->camera()->farPlane() ) );

  QList<QgsMapToolIdentify::IdentifyResult> tiledSceneIdentifyResults;
  QList<QgsMapToolIdentify::IdentifyResult> pointCloudIdentifyResults;
  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
  identifyTool2D->clearResults();

  QgsMapCanvas *canvas2D = identifyTool2D->canvas();
  const QgsCoordinateTransform ct( mCanvas->mapSettings()->crs(), canvas2D->mapSettings().destinationCrs(), canvas2D->mapSettings().transformContext() );

  bool showTerrainResults = true;

  for ( auto it = allHits.constKeyValueBegin(); it != allHits.constKeyValueEnd(); ++it )
  {
    //  We can directly show vector layer results
    if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( it->first ) )
    {
      const QgsRayCastingUtils::RayHit hit = it->second.first();
      const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( hit.pos, mCanvas->mapSettings()->origin() );
      QgsVector3D mapCoordsCanvas2D;
      try
      {
        mapCoordsCanvas2D = ct.transform( mapCoords );
      }
      catch ( QgsCsException &e )
      {
        Q_UNUSED( e )
        QgsDebugError( QStringLiteral( "Could not transform identified coordinates to project crs: %1" ).arg( e.what() ) );
      }

      const QgsPoint pt( mapCoordsCanvas2D.x(), mapCoordsCanvas2D.y(), mapCoordsCanvas2D.z() );
      identifyTool2D->showResultsForFeature( vlayer, hit.fid, pt );
      showTerrainResults = false;
    }
    // We need to restructure point cloud layer results to display them later. We may have multiple hits for each layer.
    else if ( QgsPointCloudLayer *pclayer = qobject_cast<QgsPointCloudLayer *>( it->first ) )
    {
      QVector<QVariantMap> pointCloudResults;
      for ( const QgsRayCastingUtils::RayHit &hit : it->second )
      {
        pointCloudResults.append( hit.attributes );
      }
      identifyTool2D->fromPointCloudIdentificationToIdentifyResults( pclayer, pointCloudResults, pointCloudIdentifyResults );
    }
    else if ( QgsTiledSceneLayer *tslayer = qobject_cast<QgsTiledSceneLayer *>( it->first ) )
    {
      Q_UNUSED( tslayer )
      // We are only handling a single hit for each layer
      const QgsRayCastingUtils::RayHit hit = it->second.first();
      const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( hit.pos, mCanvas->mapSettings()->origin() );

      QMap<QString, QString> derivedAttributes;
      QString x;
      QString y;
      QgsCoordinateUtils::formatCoordinatePartsForProject(
        QgsProject::instance(),
        QgsPointXY( mapCoords.x(), mapCoords.y() ),
        mCanvas->mapSettings()->crs(),
        6, x, y
      );

      derivedAttributes.insert( tr( "(clicked coordinate X)" ), x );
      derivedAttributes.insert( tr( "(clicked coordinate Y)" ), y );
      derivedAttributes.insert( tr( "(clicked coordinate Z)" ), QLocale().toString( mapCoords.z(), 'f' ) );

      const QList<QString> keys = hit.attributes.keys();
      for ( const QString &key : keys )
      {
        derivedAttributes[key] = hit.attributes[key].toString();
      }
      QString nodeId = derivedAttributes[QStringLiteral( "node_id" )];
      // only derived attributes are supported for now, so attributes is empty
      QgsMapToolIdentify::IdentifyResult res( it->first, nodeId, {}, derivedAttributes );
      tiledSceneIdentifyResults.append( res );
    }
  }

  // We only handle terrain results if there were no vector layer results so they don't get overwritten
  if ( showTerrainResults && allHits.contains( nullptr ) )
  {
    const QgsRayCastingUtils::RayHit hit = allHits.value( nullptr ).first();
    // estimate search radius
    Qgs3DMapScene *scene = mCanvas->scene();
    const double searchRadiusMM = QgsMapTool::searchRadiusMM();
    const double pixelsPerMM = mCanvas->screen()->logicalDotsPerInchX() / 25.4;
    const double searchRadiusPx = searchRadiusMM * pixelsPerMM;
    const double searchRadiusMapUnits = scene->worldSpaceError( searchRadiusPx, hit.distance );

    const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( hit.pos, mCanvas->mapSettings()->origin() );
    const QgsPointXY mapPoint( mapCoords.x(), mapCoords.y() );

    // transform the point and search radius to CRS of the map canvas (if they are different)
    QgsPointXY mapPointCanvas2D = mapPoint;
    double searchRadiusCanvas2D = searchRadiusMapUnits;
    try
    {
      mapPointCanvas2D = ct.transform( mapPoint );
      const QgsPointXY mapPointSearchRadius( mapPoint.x() + searchRadiusMapUnits, mapPoint.y() );
      const QgsPointXY mapPointSearchRadiusCanvas2D = ct.transform( mapPointSearchRadius );
      searchRadiusCanvas2D = mapPointCanvas2D.distance( mapPointSearchRadiusCanvas2D );
    }
    catch ( QgsCsException &e )
    {
      Q_UNUSED( e )
      QgsDebugError( QStringLiteral( "Could not transform identified coordinates to project crs: %1" ).arg( e.what() ) );
    }

    QgsMapToolIdentify::IdentifyProperties props;
    props.searchRadiusMapUnits = searchRadiusCanvas2D;
    props.skip3DLayers = true;
    identifyTool2D->identifyAndShowResults( QgsGeometry::fromPointXY( mapPointCanvas2D ), props );
  }

  // We need to show other layer type results AFTER terrain results so they don't get overwritten
  identifyTool2D->showIdentifyResults( tiledSceneIdentifyResults );

  // Finally add all point cloud layers' results
  // We add those last as the list can be quite big.
  identifyTool2D->showIdentifyResults( pointCloudIdentifyResults );
}

void Qgs3DMapToolIdentify::activate()
{
  mIsActive = true;
}

void Qgs3DMapToolIdentify::deactivate()
{
  mIsActive = false;
}

QCursor Qgs3DMapToolIdentify::cursor() const
{
  return QgsApplication::getThemeCursor( QgsApplication::Cursor::Identify );
}
