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

  QHash<QgsPointCloudLayer *, QVector<QVariantMap>> pointCloudResults;
  QHash<QgsTiledSceneLayer *, QVector<QVariantMap>> tiledSceneResults;

  QList<QgsMapToolIdentify::IdentifyResult> identifyResults;
  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
  identifyTool2D->clearResults();

  bool showTerrainResults = true;

  for ( auto it = allHits.constKeyValueBegin(); it != allHits.constKeyValueEnd(); ++it )
  {
    //  We can directly show vector layer results
    if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( it->first ) )
    {
      const QgsRayCastingUtils::RayHit hit = it->second.first();
      const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( hit.pos, mCanvas->mapSettings()->origin() );
      const QgsPoint pt( mapCoords.x(), mapCoords.y(), mapCoords.z() );
      identifyTool2D->showResultsForFeature( vlayer, hit.fid, pt );
      showTerrainResults = false;
    }
    // We need to restructure point cloud layer results to display them later
    else if ( QgsPointCloudLayer *pclayer = qobject_cast<QgsPointCloudLayer *>( it->first ) )
    {
      pointCloudResults[pclayer] = QVector<QVariantMap>();
      for ( const QgsRayCastingUtils::RayHit &hit : it->second )
      {
        pointCloudResults[pclayer].append( hit.attributes );
      }
    }
    else if ( QgsTiledSceneLayer *tslayer = qobject_cast<QgsTiledSceneLayer *>( it->first ) )
    {
      tiledSceneResults[tslayer] = QVector<QVariantMap>();
      for ( const QgsRayCastingUtils::RayHit &hit : it->second )
      {
        tiledSceneResults[tslayer].append( hit.attributes );
      }
    }
  }

  // We only handle terrain results if there were no vector layer results
  if ( showTerrainResults && allHits.contains( nullptr ) )
  {
    const QgsRayCastingUtils::RayHit hit = allHits.value( nullptr ).first();
    // estimate search radius
    Qgs3DMapScene *scene = mCanvas->scene();
    const double searchRadiusMM = QgsMapTool::searchRadiusMM();
    const double pixelsPerMM = mCanvas->screen()->logicalDotsPerInchX() / 25.4;
    const double searchRadiusPx = searchRadiusMM * pixelsPerMM;
    const double searchRadiusMapUnits = scene->worldSpaceError( searchRadiusPx, hit.distance );

    QgsMapCanvas *canvas2D = identifyTool2D->canvas();

    // transform the point and search radius to CRS of the map canvas (if they are different)
    const QgsCoordinateTransform ct( mCanvas->mapSettings()->crs(), canvas2D->mapSettings().destinationCrs(), canvas2D->mapSettings().transformContext() );

    const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( hit.pos, mCanvas->mapSettings()->origin() );
    const QgsPointXY mapPoint( mapCoords.x(), mapCoords.y() );

    QgsPointXY mapPointCanvas2D = mapPoint;
    double searchRadiusCanvas2D = searchRadiusMapUnits;
    try
    {
      mapPointCanvas2D = ct.transform( mapPoint );
      const QgsPointXY mapPointSearchRadius( mapPoint.x() + searchRadiusMapUnits, mapPoint.y() );
      const QgsPointXY mapPointSearchRadiusCanvas2D = ct.transform( mapPointSearchRadius );
      searchRadiusCanvas2D = mapPointCanvas2D.distance( mapPointSearchRadiusCanvas2D );
    }
    catch ( QgsException &e )
    {
      Q_UNUSED( e )
      QgsDebugError( QStringLiteral( "Caught exception %1" ).arg( e.what() ) );
    }

    identifyTool2D->identifyAndShowResults( QgsGeometry::fromPointXY( mapPointCanvas2D ), searchRadiusCanvas2D );
  }

  // Finally add all point cloud layers' results
  for ( auto it = pointCloudResults.constKeyValueBegin(); it != pointCloudResults.constKeyValueEnd(); ++it )
  {
    QgsMapToolIdentify identifyTool( nullptr );
    identifyTool.fromPointCloudIdentificationToIdentifyResults( it->first, it->second, identifyResults );
    identifyTool2D->showIdentifyResults( identifyResults );
  }

  for ( auto it = tiledSceneResults.constKeyValueBegin(); it != tiledSceneResults.constKeyValueEnd(); ++it )
  {
    // for the whole layer
    for ( const QVariantMap &hitAttributes : it->second )
    {
      QMap<QString, QString> derivedAttributes;
      const QList<QString> keys = hitAttributes.keys();
      for ( const QString &key : keys )
      {
        derivedAttributes[key] = hitAttributes[key].toString();
      }
      QString nodeId = hitAttributes["node_id"].toString();
      QgsMapToolIdentify::IdentifyResult res( it->first, nodeId, QMap<QString, QString>(), derivedAttributes );
      identifyResults.append( res );
    }

    identifyTool2D->showIdentifyResults( identifyResults );
  }
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
