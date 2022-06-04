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

#include "qgsapplication.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgsterrainentity_p.h"
#include "qgsvector3d.h"

#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolidentifyaction.h"

#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>

#include "qgspointcloudlayer.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudlayer3drenderer.h"

#include "qgs3dmapscenepickhandler.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"

class Qgs3DMapToolIdentifyPickHandler : public Qgs3DMapScenePickHandler
{
  public:
    Qgs3DMapToolIdentifyPickHandler( Qgs3DMapToolIdentify *identifyTool ): mIdentifyTool( identifyTool ) {}
    void handlePickOnVectorLayer( QgsVectorLayer *vlayer, QgsFeatureId id, const QVector3D &worldIntersection, Qt3DRender::QPickEvent *event ) override;
  private:
    Qgs3DMapToolIdentify *mIdentifyTool = nullptr;
};


void Qgs3DMapToolIdentifyPickHandler::handlePickOnVectorLayer( QgsVectorLayer *vlayer, QgsFeatureId id, const QVector3D &worldIntersection, Qt3DRender::QPickEvent *event )
{
  if ( event->button() == Qt3DRender::QPickEvent::LeftButton )
  {
    const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( QgsVector3D( worldIntersection.x(),
                                  worldIntersection.y(),
                                  worldIntersection.z() ), mIdentifyTool->mCanvas->map()->origin() );
    const QgsPoint pt( mapCoords.x(), mapCoords.y(), mapCoords.z() );

    QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
    identifyTool2D->showResultsForFeature( vlayer, id, pt );
  }
}


//////


Qgs3DMapToolIdentify::Qgs3DMapToolIdentify( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
  mPickHandler.reset( new Qgs3DMapToolIdentifyPickHandler( this ) );
  connect( canvas, &Qgs3DMapCanvas::mapSettingsChanged, this, &Qgs3DMapToolIdentify::onMapSettingsChanged );
}

Qgs3DMapToolIdentify::~Qgs3DMapToolIdentify() = default;


void Qgs3DMapToolIdentify::mousePressEvent( QMouseEvent *event )
{
  Q_UNUSED( event )

  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
  identifyTool2D->clearResults();
}

void Qgs3DMapToolIdentify::mouseReleaseEvent( QMouseEvent *event )
{
  if ( event->button() != Qt::MouseButton::LeftButton )
    return;

  // point cloud identification
  QVector<QPair<QgsMapLayer *, QVector<QVariantMap>>> layerPoints;
  Qgs3DMapCanvas *canvas = this->canvas();

  const QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( event->pos(), canvas->windowSize(), canvas->cameraController()->camera() );

  QHash<QgsPointCloudLayer *, QVector<IndexedPointCloudNode>> layerChunks;
  for ( QgsMapLayer *layer : canvas->map()->layers() )
  {
    if ( QgsPointCloudLayer *pc = qobject_cast<QgsPointCloudLayer *>( layer ) )
    {
      QVector<IndexedPointCloudNode> pointCloudNodes;
      for ( const QgsChunkNode *n : canvas->scene()->getLayerActiveChunkNodes( pc ) )
      {
        const QgsChunkNodeId id = n->tileId();
        pointCloudNodes.push_back( IndexedPointCloudNode( id.d, id.x, id.y, id.z ) );
      }
      if ( pointCloudNodes.empty() )
        continue;
      layerChunks[ pc ] = pointCloudNodes;
    }
  }

  for ( auto it = layerChunks.constBegin(); it != layerChunks.constEnd(); ++it )
  {
    QgsPointCloudLayer *layer = it.key();
    // transform ray
    const QgsVector3D originMapCoords = canvas->map()->worldToMapCoordinates( ray.origin() );
    const QgsVector3D pointMapCoords = canvas->map()->worldToMapCoordinates( ray.origin() + ray.origin().length() * ray.direction().normalized() );
    QgsVector3D directionMapCoords = pointMapCoords - originMapCoords;
    directionMapCoords.normalize();

    const QVector3D rayOriginMapCoords( originMapCoords.x(), originMapCoords.y(), originMapCoords.z() );
    const QVector3D rayDirectionMapCoords( directionMapCoords.x(), directionMapCoords.y(), directionMapCoords.z() );

    const QRect rect = canvas->cameraController()->viewport();
    const int screenSizePx = std::max( rect.width(), rect.height() ); // TODO: is this correct? (see _sceneState)
    QgsPointCloudLayer3DRenderer *renderer = dynamic_cast<QgsPointCloudLayer3DRenderer *>( layer->renderer3D() );
    const QgsPointCloud3DSymbol *symbol = renderer->symbol();
    // Symbol can be null in case of no rendering enabled
    if ( !symbol )
      continue;
    const double pointSize = symbol->pointSize();
    const double limitAngle = 2 * pointSize / screenSizePx * canvas->cameraController()->camera()->fieldOfView();

    // adjust ray to elevation properties
    QgsPointCloudLayerElevationProperties *elevationProps = dynamic_cast<QgsPointCloudLayerElevationProperties *>( layer->elevationProperties() );
    const QVector3D adjutedRayOrigin = QVector3D( rayOriginMapCoords.x(), rayOriginMapCoords.y(), ( rayOriginMapCoords.z() -  elevationProps->zOffset() ) / elevationProps->zScale() );
    QVector3D adjutedRayDirection = QVector3D( rayDirectionMapCoords.x(), rayDirectionMapCoords.y(), rayDirectionMapCoords.z() / elevationProps->zScale() );
    adjutedRayDirection.normalize();

    const QgsRay3D layerRay( adjutedRayOrigin, adjutedRayDirection );

    QgsPointCloudDataProvider *provider = layer->dataProvider();
    QgsPointCloudIndex *index = provider->index();
    QVector<QVariantMap> points;
    const QgsPointCloudAttributeCollection attributeCollection = index->attributes();
    QgsPointCloudRequest request;
    request.setAttributes( attributeCollection );
    for ( const IndexedPointCloudNode &n : it.value() )
    {
      if ( !index->hasNode( n ) )
        continue;
      std::unique_ptr<QgsPointCloudBlock> block( index->nodeData( n, request ) );
      if ( !block )
        continue;

      const QgsVector3D blockScale = block->scale();
      const QgsVector3D blockOffset = block->offset();

      const char *ptr = block->data();
      const QgsPointCloudAttributeCollection blockAttributes = block->attributes();
      const std::size_t recordSize = blockAttributes.pointRecordSize();
      int xOffset = 0, yOffset = 0, zOffset = 0;
      const QgsPointCloudAttribute::DataType xType = blockAttributes.find( QStringLiteral( "X" ), xOffset )->type();
      const QgsPointCloudAttribute::DataType yType = blockAttributes.find( QStringLiteral( "Y" ), yOffset )->type();
      const QgsPointCloudAttribute::DataType zType = blockAttributes.find( QStringLiteral( "Z" ), zOffset )->type();
      for ( int i = 0; i < block->pointCount(); ++i )
      {
        double x, y, z;
        QgsPointCloudAttribute::getPointXYZ( ptr, i, recordSize, xOffset, xType, yOffset, yType, zOffset, zType, blockScale, blockOffset, x, y, z );
        const QVector3D point( x, y, z );

        // check whether point is in front of the ray
        if ( !layerRay.isInFront( point ) )
          continue;

        // calculate the angle between the point and the projected point
        if ( layerRay.angleToPoint( point ) > limitAngle )
          continue;

        // Note : applying elevation properties is done in fromPointCloudIdentificationToIdentifyResults
        QVariantMap pointAttr = QgsPointCloudAttribute::getAttributeMap( ptr, i * recordSize, blockAttributes );
        pointAttr[ QStringLiteral( "X" ) ] = x;
        pointAttr[ QStringLiteral( "Y" ) ] = y;
        pointAttr[ QStringLiteral( "Z" ) ] = z;
        pointAttr[ tr( "Distance to camera" ) ] = ( point - layerRay.origin() ).length();
        points.push_back( pointAttr );
      }

    }
    layerPoints.push_back( qMakePair( layer, points ) );
  }

  QList<QgsMapToolIdentify::IdentifyResult> identifyResults;
  for ( int i = 0; i < layerPoints.size(); ++i )
  {
    QgsPointCloudLayer *pcLayer = qobject_cast< QgsPointCloudLayer * >( layerPoints[i].first );
    QgsMapToolIdentify::fromPointCloudIdentificationToIdentifyResults( pcLayer, layerPoints[i].second, identifyResults );
  }

  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
  identifyTool2D->showIdentifyResults( identifyResults );
}

void Qgs3DMapToolIdentify::activate()
{
  if ( QgsTerrainEntity *terrainEntity = mCanvas->scene()->terrainEntity() )
  {
    bool disableTerrainPicker = false;
    const Qgs3DMapSettings &map = terrainEntity->map3D();
    // if the terrain contains point cloud data disable the terrain picker signals
    for ( QgsMapLayer *layer : map.layers() )
    {
      if ( layer->type() == QgsMapLayerType::PointCloudLayer )
      {
        disableTerrainPicker = true;
        break;
      }
    }
    if ( !disableTerrainPicker )
      connect( terrainEntity->terrainPicker(), &Qt3DRender::QObjectPicker::clicked, this, &Qgs3DMapToolIdentify::onTerrainPicked );
  }

  mCanvas->scene()->registerPickHandler( mPickHandler.get() );
  mIsActive = true;
}

void Qgs3DMapToolIdentify::deactivate()
{
  if ( QgsTerrainEntity *terrainEntity = mCanvas->scene()->terrainEntity() )
  {
    disconnect( terrainEntity->terrainPicker(), &Qt3DRender::QObjectPicker::clicked, this, &Qgs3DMapToolIdentify::onTerrainPicked );
  }

  mCanvas->scene()->unregisterPickHandler( mPickHandler.get() );
  mIsActive = false;
}

QCursor Qgs3DMapToolIdentify::cursor() const
{
  return QgsApplication::getThemeCursor( QgsApplication::Cursor::Identify );
}

void Qgs3DMapToolIdentify::onMapSettingsChanged()
{
  if ( !mIsActive )
    return;
  connect( mCanvas->scene(), &Qgs3DMapScene::terrainEntityChanged, this, &Qgs3DMapToolIdentify::onTerrainEntityChanged );
}

void Qgs3DMapToolIdentify::onTerrainPicked( Qt3DRender::QPickEvent *event )
{
  if ( event->button() != Qt3DRender::QPickEvent::LeftButton )
    return;

  const QVector3D worldIntersection = event->worldIntersection();
  const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( QgsVector3D( worldIntersection.x(),
                                worldIntersection.y(),
                                worldIntersection.z() ), mCanvas->map()->origin() );
  const QgsPointXY mapPoint( mapCoords.x(), mapCoords.y() );

  // estimate search radius
  Qgs3DMapScene *scene = mCanvas->scene();
  const double searchRadiusMM = QgsMapTool::searchRadiusMM();
  const double pixelsPerMM = mCanvas->logicalDpiX() / 25.4;
  const double searchRadiusPx = searchRadiusMM * pixelsPerMM;
  const double searchRadiusMapUnits = scene->worldSpaceError( searchRadiusPx, event->distance() );

  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
  QgsMapCanvas *canvas2D = identifyTool2D->canvas();

  // transform the point and search radius to CRS of the map canvas (if they are different)
  const QgsCoordinateTransform ct( mCanvas->map()->crs(), canvas2D->mapSettings().destinationCrs(), canvas2D->mapSettings().transformContext() );

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
    QgsDebugMsg( QStringLiteral( "Caught exception %1" ).arg( e.what() ) );
  }

  identifyTool2D->identifyAndShowResults( QgsGeometry::fromPointXY( mapPointCanvas2D ), searchRadiusCanvas2D );
}

void Qgs3DMapToolIdentify::onTerrainEntityChanged()
{
  if ( !mIsActive )
    return;
  // no need to disconnect from the previous entity: it has been destroyed
  // start listening to the new terrain entity
  if ( QgsTerrainEntity *terrainEntity = mCanvas->scene()->terrainEntity() )
  {
    connect( terrainEntity->terrainPicker(), &Qt3DRender::QObjectPicker::clicked, this, &Qgs3DMapToolIdentify::onTerrainPicked );
  }
}
