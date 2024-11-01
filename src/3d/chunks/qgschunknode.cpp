/***************************************************************************
  qgschunknode.cpp
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

#include "qgschunknode.h"

#include "qgschunkedentity.h" // for ChunkLoader destructor
#include "qgschunklist_p.h"
#include "qgschunkloader.h"
#include <Qt3DCore/QEntity>

///@cond PRIVATE

QgsChunkNode::QgsChunkNode( const QgsChunkNodeId &nodeId, const QgsBox3D &box3D, float error, QgsChunkNode *parent )
  : mBox3D( box3D )
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

  qDeleteAll( mChildren );
}

bool QgsChunkNode::allChildChunksResident( QTime currentTime ) const
{
  Q_ASSERT( mChildrenPopulated );
  for ( int i = 0; i < childCount(); ++i )
  {
    if ( mChildren[i]->mHasData && !mChildren[i]->mEntity )
      return false;         // no there yet
    Q_UNUSED( currentTime ) // seems we do not need this extra time (it just brings extra problems)
    //if (children[i]->entityCreatedTime.msecsTo(currentTime) < 100)
    //  return false;  // allow some time for upload of stuff within Qt3D (TODO: better way to check it is ready?)
  }
  return true;
}

void QgsChunkNode::populateChildren( const QVector<QgsChunkNode *> &children )
{
  Q_ASSERT( !mChildrenPopulated );
  mChildrenPopulated = true;
  mChildren = children;
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

  for ( int i = 0; i < childCount(); ++i )
  {
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

  mLoader = nullptr; // not owned by chunk node

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

  mLoader = nullptr; // not owned by chunk node

  mState = QgsChunkNode::Loaded;
  mReplacementQueueEntry = new QgsChunkListEntry( this );
}

void QgsChunkNode::unloadChunk()
{
  Q_ASSERT( mState == QgsChunkNode::Loaded );
  Q_ASSERT( mEntity );
  Q_ASSERT( mReplacementQueueEntry );

  delete mEntity;
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
  mUpdaterFactory = nullptr; // not owned by the node

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
  mUpdaterFactory = nullptr; // not owned by the node
  mLoaderQueueEntry = nullptr;
}

void QgsChunkNode::cancelUpdating()
{
  Q_ASSERT( mState == QgsChunkNode::Updating );
  Q_ASSERT( mUpdater );
  Q_ASSERT( !mLoaderQueueEntry );

  mUpdater = nullptr; // not owned by chunk node

  mState = Loaded;
}

void QgsChunkNode::setUpdated()
{
  Q_ASSERT( mState == QgsChunkNode::Updating );
  Q_ASSERT( mUpdater );
  Q_ASSERT( !mLoaderQueueEntry );
  Q_ASSERT( mReplacementQueueEntry );

  mUpdater = nullptr; // not owned by chunk node

  mState = QgsChunkNode::Loaded;
}

void QgsChunkNode::setExactBox3D( const QgsBox3D &box3D )
{
  mBox3D = box3D;

  // TODO: propagate better estimate to children?
}

void QgsChunkNode::updateParentBoundingBoxesRecursively() const
{
  QgsChunkNode *currentNode = parent();
  while ( currentNode )
  {
    QgsChunkNode *const *currentNodeChildren = currentNode->children();
    double xMin = std::numeric_limits<double>::max();
    double xMax = -std::numeric_limits<double>::max();
    double yMin = std::numeric_limits<double>::max();
    double yMax = -std::numeric_limits<double>::max();
    double zMin = std::numeric_limits<double>::max();
    double zMax = -std::numeric_limits<double>::max();

    for ( int i = 0; i < currentNode->childCount(); ++i )
    {
      const QgsBox3D childBox3D = currentNodeChildren[i]->box3D();

      // Nodes without data have an empty bbox and should be skipped
      if ( childBox3D.isEmpty() )
        continue;

      if ( childBox3D.xMinimum() < xMin )
        xMin = childBox3D.xMinimum();
      if ( childBox3D.yMinimum() < yMin )
        yMin = childBox3D.yMinimum();
      if ( childBox3D.zMinimum() < zMin )
        zMin = childBox3D.zMinimum();
      if ( childBox3D.xMaximum() > xMax )
        xMax = childBox3D.xMaximum();
      if ( childBox3D.yMaximum() > yMax )
        yMax = childBox3D.yMaximum();
      if ( childBox3D.zMaximum() > zMax )
        zMax = childBox3D.zMaximum();
    }

    // QgsBox3D is normalized in its constructor, so that min values are always smaller than max.
    // If all child bboxes were empty, we can end up with min > max, so let's have an empty bbox instead.
    const QgsBox3D currentNodeBox3D = xMin > xMax || yMin > yMax || zMin > zMax ? QgsBox3D() : QgsBox3D( xMin, yMin, zMin, xMax, yMax, zMax );

    currentNode->setExactBox3D( currentNodeBox3D );
    currentNode = currentNode->parent();
  }
}

/// @endcond
