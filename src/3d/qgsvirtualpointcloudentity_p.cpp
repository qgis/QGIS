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
#include "qgsvirtualpointcloudprovider.h"
#include "qgspointcloudlayerchunkloader_p.h"
#include "qgschunkboundsentity_p.h"
#include "qgs3dutils.h"

///@cond PRIVATE


QgsVirtualPointCloudEntity::QgsVirtualPointCloudEntity( QgsPointCloudLayer *layer,
    const Qgs3DMapSettings &map,
    const QgsCoordinateTransform &coordinateTransform,
    QgsPointCloud3DSymbol *symbol,
    float maximumScreenSpaceError,
    bool showBoundingBoxes,
    double zValueScale,
    double zValueOffset,
    int pointBudget )
  : Qgs3DMapSceneEntity( nullptr )
  , mLayer( layer )
  , mMap( map )
  , mCoordinateTransform( coordinateTransform )
  , mZValueScale( zValueScale )
  , mZValueOffset( zValueOffset )
  , mPointBudget( pointBudget )
  , mMaximumScreenSpaceError( maximumScreenSpaceError )
  , mShowBoundingBoxes( showBoundingBoxes )
{
  mSymbol.reset( symbol );
  mBboxesEntity = new QgsChunkBoundsEntity( this );
  const QVector<QgsPointCloudSubIndex> subIndexes = provider()->subIndexes();
  for ( int i = 0; i < subIndexes.size(); ++i )
  {
    const QgsPointCloudSubIndex &si = subIndexes.at( i );
    mBboxes << Qgs3DUtils::mapToWorldExtent( si.extent(), si.zRange().lower(), si.zRange().upper(), mMap.origin() );

    if ( !si.index() )
      continue;

    QgsPointCloudLayerChunkedEntity *entity = new QgsPointCloudLayerChunkedEntity( si.index(),
        mMap,
        mCoordinateTransform,
        static_cast< QgsPointCloud3DSymbol * >( mSymbol->clone() ),
        mMaximumScreenSpaceError,
        mShowBoundingBoxes,
        mZValueScale,
        mZValueOffset,
        mPointBudget );

    mChunkedEntitiesMap.insert( i, entity );
    entity->setParent( this );
    connect( entity, &QgsChunkedEntity::pendingJobsCountChanged, this, &Qgs3DMapSceneEntity::pendingJobsCountChanged );
  }

  updateBboxEntity();
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

QgsAABB QgsVirtualPointCloudEntity::boundingBox( int i ) const
{
  return mBboxes.at( i );
}

void QgsVirtualPointCloudEntity::createChunkedEntityForSubIndex( int i )
{
  const QVector<QgsPointCloudSubIndex> subIndexes = provider()->subIndexes();
  QgsPointCloudLayerChunkedEntity *newChunkedEntity = new QgsPointCloudLayerChunkedEntity( subIndexes.at( i ).index(),
      mMap,
      mCoordinateTransform,
      static_cast< QgsPointCloud3DSymbol * >( mSymbol->clone() ),
      mMaximumScreenSpaceError,
      mShowBoundingBoxes,
      mZValueScale,
      mZValueOffset,
      mPointBudget );

  mChunkedEntitiesMap.insert( i, newChunkedEntity );
  newChunkedEntity->setParent( this );
  connect( newChunkedEntity, &QgsChunkedEntity::pendingJobsCountChanged, this, &Qgs3DMapSceneEntity::pendingJobsCountChanged );
  emit newEntityCreated( newChunkedEntity );
}

void QgsVirtualPointCloudEntity::handleSceneUpdate( const QgsChunkedEntity::SceneState &state )
{
  const QVector<QgsPointCloudSubIndex> subIndexes = provider()->subIndexes();
  for ( int i = 0; i < subIndexes.size(); ++i )
  {
    const QgsAABB &bbox = mBboxes.at( i );
    // magic number 256 is the common span value for a COPC root node
    constexpr int SPAN = 256;
    const float epsilon = std::min( bbox.xExtent(), bbox.yExtent() ) / SPAN;
    const float distance = bbox.distanceFromPoint( state.cameraPos );
    const float sse = Qgs3DUtils::screenSpaceError( epsilon, distance, state.screenSizePx, state.cameraFov );
    constexpr float THRESHOLD = .2;
    const bool displayAsBbox = sse < THRESHOLD;
    if ( !displayAsBbox && !subIndexes.at( i ).index() )
      provider()->loadSubIndex( i );

    setRenderSubIndexAsBbox( i, displayAsBbox );
    if ( !displayAsBbox && mChunkedEntitiesMap.contains( i ) )
      mChunkedEntitiesMap[i]->handleSceneUpdate( state );
  }
  updateBboxEntity();
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
  QList<QgsAABB> bboxes;
  const QVector<QgsPointCloudSubIndex> subIndexes = provider()->subIndexes();
  for ( int i = 0; i < subIndexes.size(); ++i )
  {
    if ( mChunkedEntitiesMap.contains( i ) && mChunkedEntitiesMap[i]->isEnabled() )
      continue;

    bboxes << mBboxes.at( i );
  }

  mBboxesEntity->setBoxes( bboxes );
}

void QgsVirtualPointCloudEntity::setRenderSubIndexAsBbox( const int i, const bool asBbox )
{
  if ( !mChunkedEntitiesMap.contains( i ) )
    return;

  mChunkedEntitiesMap[i]->setEnabled( !asBbox );
}
/// @endcond
