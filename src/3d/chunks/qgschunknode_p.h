/***************************************************************************
  qgschunknode_p.h
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

#ifndef QGSCHUNKNODE_P_H
#define QGSCHUNKNODE_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgsaabb.h"

#include <QTime>

#define SIP_NO_FILE

namespace Qt3DCore
{
  class QEntity;
}

struct QgsChunkListEntry;
class QgsChunkLoader;
class QgsChunkQueueJob;
class QgsChunkQueueJobFactory;


/**
 * Helper class to store integer coordinates of a chunk node.
 *
 * - "d" is the depth of the tree
 * - when used with a quadtree, "x" and "y" are the coordinates within the depth level of the tree ("z" coordinate is always -1)
 * - when used with an octree, "x", "y" and "z" are the coordinates within the depth level of the tree
 */
struct QgsChunkNodeId
{
  //! Constructs node ID
  QgsChunkNodeId( int _d = -1, int _x = -1, int _y = -1, int _z = -1 )
    : d( _d ), x( _x ), y( _y ), z( _z ) {}

  int d, x, y, z;

  //! Returns textual representation of the node ID in form of "Z/X/Y"
  QString text() const
  {
    if ( z == -1 )
      return QStringLiteral( "%1/%2/%3" ).arg( d ).arg( x ).arg( y );   // quadtree
    else
      return QStringLiteral( "%1/%2/%3/%4" ).arg( d ).arg( x ).arg( y ).arg( z );   // octree
  }

  // TODO c++20 - replace with = default
  bool operator==( const QgsChunkNodeId &other ) const
  {
    return d == other.d && x == other.x && y == other.y && z == other.z;
  }

  bool operator!=( const QgsChunkNodeId &other ) const
  {
    return !( *this == other );
  }
};

/**
 * \ingroup 3d
 * \brief Data structure for keeping track of chunks of data for 3D entities that use "out of core" rendering,
 * i.e. not all of the data are available in the memory all the time.
 *
 * This is useful for large datasets where it may be impossible to load all data into memory or the rendering would get very slow.
 * This is currently used for rendering of terrain, but it is not limited to it and may be used for
 * other data as well.
 *
 * The data structure is essentially a quadtree: each node may have four child nodes. Nodes can exist
 * in several states (e.g. skeleton or loaded state) and they keep being loaded and unloaded as necessary
 * by the 3D rendering.
 *
 * \since QGIS 3.0
 */
class QgsChunkNode
{
  public:

    //! constructs a skeleton chunk
    QgsChunkNode( const QgsChunkNodeId &nodeId, const QgsAABB &bbox, float error, QgsChunkNode *parent = nullptr );

    ~QgsChunkNode();

    /**
     * Enumeration that identifies state of the chunk node.
     *
     * Enjoy the ASCII art for the state machine:
     *
     *    |<---------------------------------------------------------------------(unloaded)--------+
     *    |<---------------------------------------(canceled)--------------+                       |
     *    |<--------(canceled)------------+                                |                       |
     *    |                               |                                |                       |
     * Skeleton  --(requested)-->  QueuedForLoad  --(started load)-->  Loading  --(finished)-->  Loaded
     *                                                                                             |  |
     *                                                                                             |  |
     *                        Updating  <--(started update)--  QueuedForUpdate  <--(needs update)--+  |
     *                           |                                    |                               |
     *                           |                                    +---------(canceled)----------->|
     *                           +-------(finished / canceled)--------------------------------------->|
     *
     */
    enum State
    {
      Skeleton,         //!< Does not contain data of the chunk and data are not being loaded
      QueuedForLoad,    //!< Data are not available yet, but node is in the request queue
      Loading,          //!< Data are being loaded right now
      Loaded,           //!< Data are fully available
      QueuedForUpdate,  //!< Loaded, but some data need to be updated - the node is in the queue
      Updating,         //!< Data are being updated right now
    };

    //! Returns 3D bounding box of the chunk
    QgsAABB bbox() const { return mBbox; }
    //! Returns measure geometric/texture error of the chunk (in world coordinates)
    float error() const { return mError; }
    //! Returns chunk tile coordinates of the tiling scheme
    QgsChunkNodeId tileId() const { return mNodeId; }
    //! Returns pointer to the parent node. Parent is NULLPTR in the root node
    QgsChunkNode *parent() const { return mParent; }
    //! Returns number of children of the node (returns -1 if the node has not yet been populated with populateChildren())
    int childCount() const { return mChildCount; }
    //! Returns array of the four children. Children may be NULLPTR if they were not created yet
    QgsChunkNode *const *children() const { return mChildren; }
    //! Returns current state of the node
    State state() const { return mState; }

    //! Returns node's entry in the loader queue. Not NULLPTR only when in QueuedForLoad / QueuedForUpdate state
    QgsChunkListEntry *loaderQueueEntry() const { return mLoaderQueueEntry; }
    //! Returns node's entry in the replacement queue. Not NULLPTR only when in Loaded / QueuedForUpdate / Updating state
    QgsChunkListEntry *replacementQueueEntry() const { return mReplacementQueueEntry; }
    //! Returns loader of the node. Not NULLPTR only when in Loading state
    QgsChunkLoader *loader() const { return mLoader; }
    //! Returns associated entity (3D object). Not NULLPTR only when Loaded / QueuedForUpdate / Updating state
    Qt3DCore::QEntity *entity() const { return mEntity; }
    //! Returns updater job. Not NULLPTR only when in Updating state
    QgsChunkQueueJob *updater() const { return mUpdater; }

    //! Returns TRUE if all child chunks are available and thus this node could be swapped to the child nodes
    bool allChildChunksResident( QTime currentTime ) const;

    //! Sets child nodes of this node. Takes ownership of all objects. Must be only called once.
    void populateChildren( const QVector<QgsChunkNode *> &children );

    //! how deep is the node in the tree (zero means root node, every level adds one)
    int level() const;

    //! Returns list of all descendants (recursive, not just direct children)
    QList<QgsChunkNode *> descendants();

    //
    // changes of states in the state machine (see State enum)
    //

    //! mark a chunk as being queued for loading
    void setQueuedForLoad( QgsChunkListEntry *entry );

    //! unload node that is in "queued for load" state
    void cancelQueuedForLoad();

    //! mark a chunk as being loaded, using the passed loader
    void setLoading( QgsChunkLoader *chunkLoader );

    //! turn a chunk that is being loaded back to skeleton node
    void cancelLoading();

    //! mark a chunk as loaded, using the loaded entity
    void setLoaded( Qt3DCore::QEntity *mEntity );

    //! turn a loaded chunk into skeleton
    void unloadChunk();

    //! turn a loaded node into one that is queued for update - with custom update job factory
    void setQueuedForUpdate( QgsChunkListEntry *entry, QgsChunkQueueJobFactory *updateJobFactory );

    //! cancel update of the node - back to loaded node
    void cancelQueuedForUpdate();

    //! mark node as being updated right now
    void setUpdating();

    //! turn a chunk that is being updated back to loaded node
    void cancelUpdating();

    //! mark node that it finished updating - back to loaded node
    void setUpdated();

    //! called when bounding box
    void setExactBbox( const QgsAABB &box );

    /**
     * Triggers a recursive update of the node's parent's bounding boxes to ensure
     * that the parent bounding box represents the extent of all child bounding boxes.
     *
     * Will recursively walk up the list of all parent's to the root node and
     * update each in turn.
     */
    void updateParentBoundingBoxesRecursively() const;

    //! Sets whether the node has any data to be displayed. Can be used to set to FALSE after load returned no data
    void setHasData( bool hasData ) { mHasData = hasData; }
    //! Returns whether the node has any data to be displayed. If not, it will be kept as a skeleton node and will not get loaded anymore
    bool hasData() const { return mHasData; }

  private:
    QgsAABB mBbox;      //!< Bounding box in world coordinates
    float mError;    //!< Error of the node in world coordinates (negative error means that chunk at this level has no data, but there may be children that do)

    QgsChunkNodeId mNodeId;  //!< Chunk coordinates (for use with a tiling scheme)

    QgsChunkNode *mParent;        //!< TODO: should be shared pointer
    QgsChunkNode *mChildren[8];   //!< TODO: should be weak pointers. May be nullptr if not created yet or removed already
    int mChildCount = -1;         //! Number of children (-1 == not yet populated)

    State mState;  //!< State of the node

    QgsChunkListEntry *mLoaderQueueEntry;       //!< Not null <=> QueuedForLoad or QueuedForUpdate state
    QgsChunkListEntry *mReplacementQueueEntry;  //!< Not null <=> has non-null entity (Loaded or QueuedForUpdate or Updating state)

    QgsChunkLoader *mLoader;         //!< Contains extra data necessary for entity creation (not null <=> Loading state)
    Qt3DCore::QEntity *mEntity;   //!< Contains everything to display chunk as 3D object (not null <=> Loaded or QueuedForUpdate or Updating state)

    QgsChunkQueueJobFactory *mUpdaterFactory;  //!< Object that creates updater (not null <=> QueuedForUpdate state)
    QgsChunkQueueJob *mUpdater;                //!< Object that does update of the chunk (not null <=> Updating state)

    QTime mEntityCreatedTime;
    bool mHasData = true;   //!< Whether there are (will be) any data in this node (or any descentants) and so whether it makes sense to load this node
};

/// @endcond

#endif // CHUNKNODE_H
