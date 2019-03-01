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

namespace Qt3DCore
{
  class QEntity;
}

struct QgsChunkListEntry;
class QgsChunkLoader;
class QgsChunkQueueJob;
class QgsChunkQueueJobFactory;


/**
 * \ingroup 3d
 * Data structure for keeping track of chunks of data for 3D entities that use "out of core" rendering,
 * i.e. not all of the data are available in the memory all the time. This is useful for large datasets
 * where it may be impossible to load all data into memory or the rendering would get very slow.
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
    QgsChunkNode( int tileX, int tileY, int tileZ, const QgsAABB &bbox, float error, QgsChunkNode *parent = nullptr );

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
    //! Returns chunk tile X coordinate of the tiling scheme
    int tileX() const { return mTileX; }
    //! Returns chunk tile Y coordinate of the tiling scheme
    int tileY() const { return mTileY; }
    //! Returns chunk tile Z coordinate of the tiling scheme
    int tileZ() const { return mTileZ; }
    //! Returns pointer to the parent node. Parent is NULLPTR in the root node
    QgsChunkNode *parent() const { return mParent; }
    //! Returns array of the four children. Children may be NULLPTR if they were not created yet
    QgsChunkNode *const *children() const { return mChildren; }
    //! Returns current state of the node
    State state() const { return mState; }

    //! Returns node's entry in the loader queue. Not null only when in QueuedForLoad / QueuedForUpdate state
    QgsChunkListEntry *loaderQueueEntry() const { return mLoaderQueueEntry; }
    //! Returns node's entry in the replacement queue. Not null only when in Loaded / QueuedForUpdate / Updating state
    QgsChunkListEntry *replacementQueueEntry() const { return mReplacementQueueEntry; }
    //! Returns loader of the node. Not null only when in Loading state
    QgsChunkLoader *loader() const { return mLoader; }
    //! Returns associated entity (3D object). Not null only when Loaded / QueuedForUpdate / Updating state
    Qt3DCore::QEntity *entity() const { return mEntity; }
    //! Returns updater job. Not null only when in Updating state
    QgsChunkQueueJob *updater() const { return mUpdater; }

    //! Returns TRUE if all child chunks are available and thus this node could be swapped to the child nodes
    bool allChildChunksResident( QTime currentTime ) const;

    //! make sure that all child nodes are at least skeleton nodes
    void ensureAllChildrenExist();

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

    //! Sets whether the node has any data to be displayed. Can be used to set to FALSE after load returned no data
    void setHasData( bool hasData ) { mHasData = hasData; }
    //! Returns whether the node has any data to be displayed. If not, it will be kept as a skeleton node and will not get loaded anymore
    bool hasData() const { return mHasData; }

  private:
    QgsAABB mBbox;      //!< Bounding box in world coordinates
    float mError;    //!< Error of the node in world coordinates

    int mTileX, mTileY, mTileZ;  //!< Chunk coordinates (for use with a tiling scheme)

    QgsChunkNode *mParent;        //!< TODO: should be shared pointer
    QgsChunkNode *mChildren[4];   //!< TODO: should be weak pointers. May be nullptr if not created yet or removed already

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
