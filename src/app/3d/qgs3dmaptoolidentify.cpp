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

#include "qgisapp.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgsapplication.h"
#include "qgscameracontroller.h"
#include "qgscoordinateutils.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolidentifyaction.h"
#include "qgspointcloudlayer.h"
#include "qgsraycastcontext.h"
#include "qgstiledscenelayer.h"
#include "qgsvector3d.h"

#include <QScreen>

#include "moc_qgs3dmaptoolidentify.cpp"

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

  QgsRayCastContext context;
  context.setSingleResult( false );
  context.setMaximumDistance( mCanvas->cameraController()->camera()->farPlane() );
  context.setAngleThreshold( 0.5f );
  const QgsRayCastResult results = mCanvas->castRay( event->pos(), context );

  QList<QgsMapToolIdentify::IdentifyResult> tiledSceneIdentifyResults;
  QList<QgsMapToolIdentify::IdentifyResult> pointCloudIdentifyResults;
  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
  identifyTool2D->clearResults();

  QgsMapCanvas *canvas2D = identifyTool2D->canvas();
  const QgsCoordinateTransform ct( mCanvas->mapSettings()->crs(), canvas2D->mapSettings().destinationCrs(), canvas2D->mapSettings().transformContext() );

  bool showTerrainResults = true;

  const QList<QgsMapLayer *> layers = results.layers();
  for ( QgsMapLayer *layer : layers )
  {
    //  We can directly show vector layer results
    if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer ) )
    {
      const QList<QgsRayCastHit> layerHits = results.layerHits( layer );
      const QgsRayCastHit hit = layerHits.constFirst();
      const QgsVector3D mapCoords = hit.mapCoordinates();
      QgsVector3D mapCoordsCanvas2D;
      try
      {
        mapCoordsCanvas2D = ct.transform( mapCoords );
      }
      catch ( QgsCsException &e )
      {
        Q_UNUSED( e )
        QgsDebugError( u"Could not transform identified coordinates to project crs: %1"_s.arg( e.what() ) );
      }

      const QgsPoint pt( mapCoordsCanvas2D.x(), mapCoordsCanvas2D.y(), mapCoordsCanvas2D.z() );
      identifyTool2D->showResultsForFeature( vlayer, hit.properties().value( u"fid"_s, FID_NULL ).toLongLong(), pt );
      showTerrainResults = false;
    }
    // We need to restructure point cloud layer results to display them later. We may have multiple hits for each layer.
    else if ( QgsPointCloudLayer *pclayer = qobject_cast<QgsPointCloudLayer *>( layer ) )
    {
      QVector<QVariantMap> pointCloudResults;
      const QList<QgsRayCastHit> layerHits = results.layerHits( layer );
      for ( const QgsRayCastHit &hit : layerHits )
      {
        pointCloudResults.append( hit.properties() );
      }
      identifyTool2D->fromPointCloudIdentificationToIdentifyResults( pclayer, pointCloudResults, pointCloudIdentifyResults );
    }
    else if ( QgsTiledSceneLayer *tslayer = qobject_cast<QgsTiledSceneLayer *>( layer ) )
    {
      Q_UNUSED( tslayer )
      // We are only handling a single hit for each layer
      const QList<QgsRayCastHit> layerHits = results.layerHits( layer );
      const QgsRayCastHit hit = layerHits.constFirst();
      const QgsVector3D mapCoords = hit.mapCoordinates();

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

      const QVariantMap hitAttributes = hit.properties();
      const QList<QString> keys = hitAttributes.keys();
      for ( const QString &key : keys )
      {
        derivedAttributes[key] = hitAttributes[key].toString();
      }
      QString nodeId = derivedAttributes[u"node_id"_s];
      // only derived attributes are supported for now, so attributes is empty
      QgsMapToolIdentify::IdentifyResult res( layer, nodeId, {}, derivedAttributes );
      tiledSceneIdentifyResults.append( res );
    }
  }

  // We only handle terrain results if there were no vector layer results so they don't get overwritten
  if ( showTerrainResults && results.hasTerrainHits() )
  {
    const QgsRayCastHit hit = results.terrainHits().constFirst();
    // estimate search radius
    Qgs3DMapScene *scene = mCanvas->scene();
    const double searchRadiusMM = QgsMapTool::searchRadiusMM();
    const double pixelsPerMM = mCanvas->screen()->logicalDotsPerInchX() / 25.4;
    const double searchRadiusPx = searchRadiusMM * pixelsPerMM;
    const double searchRadiusMapUnits = scene->worldSpaceError( searchRadiusPx, hit.distance() );

    const QgsVector3D mapCoords = hit.mapCoordinates();

    try
    {
      const QgsVector3D mapCoordsCanvas2D = ct.transform( mapCoords );
      const QgsVector3D mapCoordsSearchRadiusX = ct.transform( QgsVector3D( mapCoords.x() + searchRadiusMapUnits, mapCoords.y(), mapCoords.z() ) );
      const QgsVector3D mapCoordsSearchRadiusY = ct.transform( QgsVector3D( mapCoords.x(), mapCoords.y() + searchRadiusMapUnits, mapCoords.z() ) );
      const QgsVector3D mapCoordsSearchRadiusZ = ct.transform( QgsVector3D( mapCoords.x(), mapCoords.y(), mapCoords.z() + searchRadiusMapUnits ) );
      const double searchRadiusX = mapCoordsCanvas2D.distance( mapCoordsSearchRadiusX );
      const double searchRadiusY = mapCoordsCanvas2D.distance( mapCoordsSearchRadiusY );
      const double searchRadiusZ = mapCoordsCanvas2D.distance( mapCoordsSearchRadiusZ );
      const double searchRadiusCanvas2D = std::max( searchRadiusX, std::max( searchRadiusY, searchRadiusZ ) );

      QgsMapToolIdentify::IdentifyProperties props;
      props.searchRadiusMapUnits = searchRadiusCanvas2D;
      props.skip3DLayers = true;
      identifyTool2D->identifyAndShowResults( QgsGeometry::fromPointXY( QgsPointXY( mapCoordsCanvas2D.x(), mapCoordsCanvas2D.y() ) ), props );
    }
    catch ( QgsCsException &e )
    {
      Q_UNUSED( e )
      QgsDebugError( u"Could not transform identified coordinates to project crs: %1"_s.arg( e.what() ) );
    }
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
