/***************************************************************************
  qgsvirtualpointcloudentity_p.cpp
  --------------------------------------
  Date                 : April 2023
  Copyright            : (C) 2023 by Stefanos Natsis
  Email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvirtualpointcloudentity_p.h"
#include "moc_qgsvirtualpointcloudentity_p.cpp"
#include "qgsvirtualpointcloudprovider.h"
#include "qgspointcloudlayerchunkloader_p.h"
#include "qgschunkboundsentity_p.h"
#include "qgs3dutils.h"

///@cond PRIVATE


QgsVirtualPointCloudEntity::QgsVirtualPointCloudEntity(
  Qgs3DMapSettings *map,
  QgsPointCloudLayer *layer,
  const QgsCoordinateTransform &coordinateTransform,
  QgsPointCloud3DSymbol *symbol,
  float maximumScreenSpaceError,
  bool showBoundingBoxes,
  double zValueScale,
  double zValueOffset,
  int pointBudget
)
  : Qgs3DMapSceneEntity( map, nullptr )
  , mLayer( layer )
  , mCoordinateTransform( coordinateTransform )
  , mZValueScale( zValueScale )
  , mZValueOffset( zValueOffset )
  , mPointBudget( pointBudget )
  , mMaximumScreenSpaceError( maximumScreenSpaceError )
  , mShowBoundingBoxes( showBoundingBoxes )
{
  mSymbol.reset( symbol );
  const QgsRectangle mapExtent = Qgs3DUtils::tryReprojectExtent2D( map->extent(), map->crs(), layer->crs(), map->transformContext() );
  const QVector<QgsPointCloudSubIndex> subIndexes = provider()->subIndexes();
  for ( int i = 0; i < subIndexes.size(); ++i )
  {
    const QgsPointCloudSubIndex &si = subIndexes.at( i );
    const QgsRectangle intersection = si.extent().intersect( mapExtent );

    mBboxes << QgsBox3D( intersection, si.zRange().lower(), si.zRange().upper() );

    createChunkedEntityForSubIndex( i );
  }

  if ( provider()->overview() )
  {
    mOverviewEntity = new QgsPointCloudLayerChunkedEntity(
      mapSettings(),
      mLayer,
      provider()->overview(),
      mCoordinateTransform,
      dynamic_cast<QgsPointCloud3DSymbol *>( mSymbol->clone() ),
      mMaximumScreenSpaceError,
      false,
      mZValueScale,
      mZValueOffset,
      mPointBudget
    );
    mOverviewEntity->setParent( this );
    connect( mOverviewEntity, &QgsChunkedEntity::pendingJobsCountChanged, this, &Qgs3DMapSceneEntity::pendingJobsCountChanged );
    connect( mOverviewEntity, &QgsChunkedEntity::newEntityCreated, this, &Qgs3DMapSceneEntity::newEntityCreated );
    emit newEntityCreated( mOverviewEntity );
  }

  // this is a rather arbitrary point, it could be somewhere else, ideally near the actual data
  QgsVector3D boundsEntityOrigin( mapExtent.center().x(), mapExtent.center().y(), 0 );

  mBboxesEntity = new QgsChunkBoundsEntity( boundsEntityOrigin, this );
  updateBboxEntity();
  connect( this, &QgsVirtualPointCloudEntity::subIndexNeedsLoading, provider(), &QgsVirtualPointCloudProvider::loadSubIndex, Qt::QueuedConnection );
  connect( provider(), &QgsVirtualPointCloudProvider::subIndexLoaded, this, &QgsVirtualPointCloudEntity::createChunkedEntityForSubIndex );
}

QList<QgsChunkedEntity *> QgsVirtualPointCloudEntity::chunkedEntities() const
{
  return mChunkedEntitiesMap.values();
}

QgsVirtualPointCloudProvider *QgsVirtualPointCloudEntity::provider() const
{
  return qobject_cast<QgsVirtualPointCloudProvider *>( mLayer->dataProvider() );
}

void QgsVirtualPointCloudEntity::createChunkedEntityForSubIndex( int i )
{
  const QVector<QgsPointCloudSubIndex> subIndexes = provider()->subIndexes();
  const QgsPointCloudSubIndex &si = subIndexes.at( i );

  // Skip if Index is not yet loaded or is outside the map extents, or it's not valid (e.g. file is missing)
  if ( !si.index() || mBboxes.at( i ).isEmpty() || !si.index().isValid() )
    return;

  QgsPointCloudLayerChunkedEntity *newChunkedEntity = new QgsPointCloudLayerChunkedEntity(
    mapSettings(),
    mLayer,
    si.index(),
    mCoordinateTransform,
    static_cast<QgsPointCloud3DSymbol *>( mSymbol->clone() ),
    mMaximumScreenSpaceError,
    mShowBoundingBoxes,
    mZValueScale,
    mZValueOffset,
    mPointBudget
  );

  mChunkedEntitiesMap.insert( i, newChunkedEntity );
  newChunkedEntity->setParent( this );
  connect( newChunkedEntity, &QgsChunkedEntity::pendingJobsCountChanged, this, &Qgs3DMapSceneEntity::pendingJobsCountChanged );
  connect( newChunkedEntity, &QgsChunkedEntity::newEntityCreated, this, &Qgs3DMapSceneEntity::newEntityCreated );
  emit newEntityCreated( newChunkedEntity );
}

void QgsVirtualPointCloudEntity::handleSceneUpdate( const SceneContext &sceneContext )
{
  QgsVector3D cameraPosMapCoords = QgsVector3D( sceneContext.cameraPos ) + mapSettings()->origin();
  const QVector<QgsPointCloudSubIndex> subIndexes = provider()->subIndexes();
  for ( int i = 0; i < subIndexes.size(); ++i )
  {
    const QgsBox3D &box3D = mBboxes.at( i );

    if ( box3D.isEmpty() )
      continue;

    // magic number 256 is the common span value for a COPC root node
    constexpr int SPAN = 256;
    const float epsilon = static_cast<float>( std::min( box3D.width(), box3D.height() ) ) / SPAN;
    const float distance = static_cast<float>( box3D.distanceTo( cameraPosMapCoords ) );
    const float sse = Qgs3DUtils::screenSpaceError( epsilon, distance, sceneContext.screenSizePx, sceneContext.cameraFov );
    constexpr float THRESHOLD = .2;

    // always display as bbox for the initial temporary camera pos (0, 0, 0)
    // then once the camera changes we display as bbox depending on screen space error
    const bool displayAsBbox = sceneContext.cameraPos.isNull() || sse < THRESHOLD;
    if ( !displayAsBbox && !subIndexes.at( i ).index() )
      emit subIndexNeedsLoading( i );

    setRenderSubIndexAsBbox( i, displayAsBbox );
    if ( !displayAsBbox && mChunkedEntitiesMap.contains( i ) )
      mChunkedEntitiesMap[i]->handleSceneUpdate( sceneContext );
  }
  updateBboxEntity();

  const QgsPointCloudLayer3DRenderer *rendererBehavior = dynamic_cast<QgsPointCloudLayer3DRenderer *>( mLayer->renderer3D() );
  if ( provider()->overview() && rendererBehavior && ( rendererBehavior->zoomOutBehavior() == Qgis::PointCloudZoomOutRenderBehavior::RenderOverview || rendererBehavior->zoomOutBehavior() == Qgis::PointCloudZoomOutRenderBehavior::RenderOverviewAndExtents ) )
  {
    mOverviewEntity->handleSceneUpdate( sceneContext );
  }
}

QgsRange<float> QgsVirtualPointCloudEntity::getNearFarPlaneRange( const QMatrix4x4 &viewMatrix ) const
{
  float fnear = 1e9;
  float ffar = 0;

  for ( QgsChunkedEntity *entity : mChunkedEntitiesMap )
  {
    if ( entity->isEnabled() )
    {
      const QgsRange<float> range = entity->getNearFarPlaneRange( viewMatrix );
      ffar = std::max( range.upper(), ffar );
      fnear = std::min( range.lower(), fnear );
    }
  }

  // if there were no chunked entities available, we will iterate the bboxes as a fallback instead
  if ( fnear == 1e9 && ffar == 0 )
  {
    for ( const QgsBox3D &box : mBboxes )
    {
      QgsAABB aabb = QgsAABB::fromBox3D( box, mBboxesEntity->vertexDataOrigin() );
      float bboxfnear;
      float bboxffar;
      Qgs3DUtils::computeBoundingBoxNearFarPlanes( aabb, viewMatrix, bboxfnear, bboxffar );
      fnear = std::min( fnear, bboxfnear );
      ffar = std::max( ffar, bboxffar );
    }
  }

  return QgsRange<float>( fnear, ffar );
}

int QgsVirtualPointCloudEntity::pendingJobsCount() const
{
  int jobs = 0;
  for ( QgsChunkedEntity *entity : mChunkedEntitiesMap )
  {
    if ( entity->isEnabled() )
      jobs += entity->pendingJobsCount();
  }
  return jobs;
}

bool QgsVirtualPointCloudEntity::needsUpdate() const
{
  for ( QgsChunkedEntity *entity : mChunkedEntitiesMap )
  {
    if ( entity->isEnabled() && entity->needsUpdate() )
      return true;
  }
  return false;
}

void QgsVirtualPointCloudEntity::updateBboxEntity()
{
  QList<QgsBox3D> bboxes;
  // we want to render bounding boxes only when zoomOutBehavior is RenderExtents or RenderOverviewAndExtents
  const QgsPointCloudLayer3DRenderer *renderer = dynamic_cast<QgsPointCloudLayer3DRenderer *>( mLayer->renderer3D() );
  if ( renderer && renderer->zoomOutBehavior() != Qgis::PointCloudZoomOutRenderBehavior::RenderOverview )
  {
    const QVector<QgsPointCloudSubIndex> subIndexes = provider()->subIndexes();
    for ( int i = 0; i < subIndexes.size(); ++i )
    {
      if ( mChunkedEntitiesMap.contains( i ) && mChunkedEntitiesMap[i]->isEnabled() )
        continue;

      if ( mBboxes.at( i ).isEmpty() )
        continue;

      bboxes << mBboxes.at( i );
    }
  }

  mBboxesEntity->setBoxes( bboxes );
}

void QgsVirtualPointCloudEntity::setRenderSubIndexAsBbox( int i, bool asBbox )
{
  if ( !mChunkedEntitiesMap.contains( i ) )
    return;

  mChunkedEntitiesMap[i]->setEnabled( !asBbox );
}
/// @endcond
