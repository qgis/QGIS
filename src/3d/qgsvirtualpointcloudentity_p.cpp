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


QgsVirtualPointCloudEntity::QgsVirtualPointCloudEntity( QgsPointCloudLayer *layer, const QList<QgsPointCloudSubIndex *> subIndexes,
    const Qgs3DMapSettings &map,
    const QgsCoordinateTransform &coordinateTransform,
    QgsPointCloud3DSymbol *symbol,
    float maximumScreenSpaceError,
    bool showBoundingBoxes,
    double zValueScale,
    double zValueOffset,
    int pointBudget )
  : Qt3DCore::QEntity( nullptr )
  , mLayer( layer )
  , mSubIndexes( subIndexes )
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
  for ( int i = 0; i < mSubIndexes.size(); ++i )
  {
    mBboxes << Qgs3DUtils::mapToWorldExtent( mSubIndexes.at( i )->extent(), mSubIndexes.at( i )->zRange().lower(), mSubIndexes.at( i )->zRange().upper(), mMap.origin() );
  }
  updateBboxEntity();
}

QList<QgsChunkedEntity *> QgsVirtualPointCloudEntity::chunkedEntities() const
{
  return mChunkedEntities;
}

QgsVirtualPointCloudProvider *QgsVirtualPointCloudEntity::provider() const
{
  return qobject_cast<QgsVirtualPointCloudProvider *>( mLayer->dataProvider() );
}

QgsAABB QgsVirtualPointCloudEntity::boundingBox( int i ) const
{
  return mBboxes.at( i );
}

void QgsVirtualPointCloudEntity::createChunkedEntitiesForLoadedSubIndexes()
{
  if ( !mChunkedEntities.isEmpty() )
    return;

  for ( int i = 0; i < mSubIndexes.size(); ++i )
  {
    QgsPointCloudSubIndex *si = mSubIndexes.at( i );
    if ( !si->index() )
      continue;

    QgsPointCloudLayerChunkedEntity *entity = new QgsPointCloudLayerChunkedEntity( si->index(),
        mMap,
        mCoordinateTransform,
        static_cast< QgsPointCloud3DSymbol * >( mSymbol->clone() ),
        mMaximumScreenSpaceError,
        mShowBoundingBoxes,
        mZValueScale,
        mZValueOffset,
        mPointBudget );
    mChunkedEntitiesMap.insert( i, entity );
    mChunkedEntities.append( entity );
  }
}

void QgsVirtualPointCloudEntity::createChunkedEntityForSubIndex( int i )
{
  QgsPointCloudLayerChunkedEntity *newChunkedEntity = new QgsPointCloudLayerChunkedEntity( mSubIndexes.at( i )->index(),
      mMap,
      mCoordinateTransform,
      static_cast< QgsPointCloud3DSymbol * >( mSymbol->clone() ),
      mMaximumScreenSpaceError,
      mShowBoundingBoxes,
      mZValueScale,
      mZValueOffset,
      mPointBudget );
  mChunkedEntities.append( newChunkedEntity );
  mChunkedEntitiesMap.insert( i, newChunkedEntity );
  emit newEntityCreated( newChunkedEntity );
}

void QgsVirtualPointCloudEntity::updateBboxEntity()
{
  QList<QgsAABB> bboxes;
  for ( int i = 0; i < mSubIndexes.size(); ++i )
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
