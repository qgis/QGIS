/***************************************************************************
  qgschunknode_p.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgschunknode_p.h"

#include "qgschunkedentity_p.h"  // for ChunkLoader destructor
#include "qgschunklist_p.h"
#include "qgschunkloader_p.h"
#include <Qt3DCore/QEntity>

///@cond PRIVATE

QgsChunkNode::QgsChunkNode( Type type, const QgsChunkNodeId &nodeId, const QgsAABB &bbox, float error, QgsChunkNode *parent )
  : mType( type )
  , mBbox( bbox )
  , mError( error )
  , mNodeId( nodeId )
  , mParent( parent )
  , mState( Skeleton )
  , mLoaderQueueEntry( nullptr )
  , mReplacementQueueEntry( nullptr )
  , mLoader( nullptr )
  , mEntity( nullptr )
  , mUpdaterFactory( nullptr )
  , mUpdater( nullptr )
{
  int childCount = mType == Quadtree ? 4 : 8;
  for ( int i = 0; i < childCount; ++i )
    mChildren[i] = nullptr;
}

QgsChunkNode::~QgsChunkNode()
{
  Q_ASSERT( mState == Skeleton );
  Q_ASSERT( !mLoaderQueueEntry );
  Q_ASSERT( !mReplacementQueueEntry );
  Q_ASSERT( !mLoader ); // should be deleted when removed from loader queue
  Q_ASSERT( !mEntity ); // should be deleted when removed from replacement queue
  Q_ASSERT( !mUpdater );
  Q_ASSERT( !mUpdaterFactory );

  int childCount = mType == Quadtree ? 4 : 8;
  for ( int i = 0; i < childCount; ++i )
    delete mChildren[i];
}

bool QgsChunkNode::allChildChunksResident( QTime currentTime ) const
{
  int childCount = mType == Quadtree ? 4 : 8;
  for ( int i = 0; i < childCount; ++i )
  {
    if ( !mChildren[i] )
      return false;  // not even a skeleton
    if ( mChildren[i]->mHasData && !mChildren[i]->mEntity )
      return false;  // no there yet
    Q_UNUSED( currentTime ) // seems we do not need this extra time (it just brings extra problems)
    //if (children[i]->entityCreatedTime.msecsTo(currentTime) < 100)
    //  return false;  // allow some time for upload of stuff within Qt3D (TODO: better way to check it is ready?)
  }
  return true;
}

void QgsChunkNode::ensureAllChildrenExist()
{
  float childError = mError / 2;
  float xc = mBbox.xCenter(), yc = mBbox.yCenter(), zc = mBbox.zCenter();

  int childCount = mType == Quadtree ? 4 : 8;
  for ( int i = 0; i < childCount; ++i )
  {
    if ( mChildren[i] )
      continue;

    int dx = i & 1, dy = !!( i & 2 ), dz = !!( i & 4 );
    QgsChunkNodeId childId( mNodeId.d + 1, mNodeId.x * 2 + dx, mNodeId.y * 2 + dy, mType == Quadtree ? -1 : mNodeId.z * 2 + dz );
    // the Y and Z coordinates below are intentionally flipped, because
    // in chunk node IDs the X,Y axes define horizontal plane,
    // while in our 3D scene the X,Z axes define the horizontal plane
    float chXMin = dx ? xc : mBbox.xMin;
    float chXMax = dx ? mBbox.xMax : xc;
    float chZMin = dy ? zc : mBbox.zMin;
    float chZMax = dy ? mBbox.zMax : zc;
    float chYMin = mBbox.yMin;
    float chYMax = mBbox.yMax;
    if ( mType == Octree )
    {
      chYMin = dz ? yc : mBbox.yMin;
      chYMax = dz ? mBbox.yMax : yc;
    }
    mChildren[i] = new QgsChunkNode( mType, childId, QgsAABB( chXMin, chYMin, chZMin, chXMax, chYMax, chZMax ), childError, this );
  }
}

int QgsChunkNode::level() const
{
  int lvl = 0;
  QgsChunkNode *p = mParent;
  while ( p )
  {
    ++lvl;
    p = p->mParent;
  }
  return lvl;
}

QList<QgsChunkNode *> QgsChunkNode::descendants()
{
  QList<QgsChunkNode *> lst;
  lst << this;

  int childCount = mType == Quadtree ? 4 : 8;
  for ( int i = 0; i < childCount; ++i )
  {
    if ( mChildren[i] )
      lst << mChildren[i]->descendants();
  }

  return lst;
}

void QgsChunkNode::setQueuedForLoad( QgsChunkListEntry *entry )
{
  Q_ASSERT( mState == Skeleton );
  Q_ASSERT( !mLoaderQueueEntry );
  Q_ASSERT( !mLoader );

  mState = QgsChunkNode::QueuedForLoad;
  mLoaderQueueEntry = entry;
}

void QgsChunkNode::cancelQueuedForLoad()
{
  Q_ASSERT( mState == QueuedForLoad );
  Q_ASSERT( mLoaderQueueEntry );

  delete mLoaderQueueEntry;
  mLoaderQueueEntry = nullptr;

  mState = QgsChunkNode::Skeleton;
}

void QgsChunkNode::setLoading( QgsChunkLoader *chunkLoader )
{
  Q_ASSERT( mState == QueuedForLoad );
  Q_ASSERT( !mLoader );
  Q_ASSERT( mLoaderQueueEntry );

  mState = Loading;
  mLoader = chunkLoader;
  mLoaderQueueEntry = nullptr;
}

void QgsChunkNode::cancelLoading()
{
  Q_ASSERT( mState == QgsChunkNode::Loading );
  Q_ASSERT( mLoader );
  Q_ASSERT( !mLoaderQueueEntry );
  Q_ASSERT( !mEntity );
  Q_ASSERT( !mReplacementQueueEntry );

  mLoader = nullptr;  // not owned by chunk node

  mState = QgsChunkNode::Skeleton;
}

void QgsChunkNode::setLoaded( Qt3DCore::QEntity *newEntity )
{
  Q_ASSERT( mState == QgsChunkNode::Loading );
  Q_ASSERT( mLoader );
  Q_ASSERT( !mLoaderQueueEntry );
  Q_ASSERT( !mReplacementQueueEntry );

  mEntity = newEntity;
  mEntityCreatedTime = QTime::currentTime();

  mLoader = nullptr;  // not owned by chunk node

  mState = QgsChunkNode::Loaded;
  mReplacementQueueEntry = new QgsChunkListEntry( this );
}

void QgsChunkNode::unloadChunk()
{
  Q_ASSERT( mState == QgsChunkNode::Loaded );
  Q_ASSERT( mEntity );
  Q_ASSERT( mReplacementQueueEntry );

  mEntity->deleteLater();
  mEntity = nullptr;
  delete mReplacementQueueEntry;
  mReplacementQueueEntry = nullptr;
  mState = QgsChunkNode::Skeleton;
}

void QgsChunkNode::setQueuedForUpdate( QgsChunkListEntry *entry, QgsChunkQueueJobFactory *updateJobFactory )
{
  Q_ASSERT( mState == QgsChunkNode::Loaded );
  Q_ASSERT( mEntity );
  Q_ASSERT( mReplacementQueueEntry );
  Q_ASSERT( !mLoaderQueueEntry );
  Q_ASSERT( !mUpdater );
  Q_ASSERT( !mUpdaterFactory );

  mState = QueuedForUpdate;
  mLoaderQueueEntry = entry;
  mUpdaterFactory = updateJobFactory;
}

void QgsChunkNode::cancelQueuedForUpdate()
{
  Q_ASSERT( mState == QueuedForUpdate );
  Q_ASSERT( mEntity );
  Q_ASSERT( mLoaderQueueEntry );
  Q_ASSERT( mUpdaterFactory );
  Q_ASSERT( !mUpdater );

  mState = Loaded;
  mUpdaterFactory = nullptr;  // not owned by the node

  delete mLoaderQueueEntry;
  mLoaderQueueEntry = nullptr;
}

void QgsChunkNode::setUpdating()
{
  Q_ASSERT( mState == QgsChunkNode::QueuedForUpdate );
  Q_ASSERT( mEntity );
  Q_ASSERT( mReplacementQueueEntry );
  Q_ASSERT( mLoaderQueueEntry );
  Q_ASSERT( !mUpdater );
  Q_ASSERT( mUpdaterFactory );

  mState = Updating;
  mUpdater = mUpdaterFactory->createJob( this );
  mUpdaterFactory = nullptr;  // not owned by the node
  mLoaderQueueEntry = nullptr;
}

void QgsChunkNode::cancelUpdating()
{
  Q_ASSERT( mState == QgsChunkNode::Updating );
  Q_ASSERT( mUpdater );
  Q_ASSERT( !mLoaderQueueEntry );

  mUpdater = nullptr;  // not owned by chunk node

  mState = Loaded;
}

void QgsChunkNode::setUpdated()
{
  Q_ASSERT( mState == QgsChunkNode::Updating );
  Q_ASSERT( mUpdater );
  Q_ASSERT( !mLoaderQueueEntry );
  Q_ASSERT( mReplacementQueueEntry );

  mUpdater = nullptr;   // not owned by chunk node

  mState = QgsChunkNode::Loaded;
}

void QgsChunkNode::setExactBbox( const QgsAABB &box )
{
  mBbox = box;

  // TODO: propagate better estimate to children?
}

/// @endcond
