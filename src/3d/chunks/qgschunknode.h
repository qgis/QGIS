/***************************************************************************
  qgschunknode.h
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

#ifndef QGSCHUNKNODE_H
#define QGSCHUNKNODE_H

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
#include "qgsbox3d.h"

#include "qgis.h"
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
 * IDs can be stored using quad or octree depth, x, y and z, where:
 *
 * - "d" is the depth of the tree
 * - when used with a quadtree, "x" and "y" are the coordinates within the depth level of the tree ("z" coordinate is always -1)
 * - when used with an octree, "x", "y" and "z" are the coordinates within the depth level of the tree
 *
 * Or alternatively, IDs can be associated with a direct integer ID if a unique
 * ID is already available by the loader.
 */
struct QgsChunkNodeId
{
    /**
   * Constructs node ID from depth, x, y and z.
   */
    QgsChunkNodeId( int _d = -1, int _x = -1, int _y = -1, int _z = -1 )
      : d( _d ), x( _x ), y( _y ), z( _z ) {}

    /**
   * Constructs node ID from a unique integer ID.
   *
   * Useful for nodes which are not structured in a quadtree or octree arrangement.
   */
    QgsChunkNodeId( long long id )
      : uniqueId( id )
    {}

    int d = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    long long uniqueId = -1;

    //! Returns textual representation of the node ID in form of "Z/X/Y"
    QString text() const
    {
      if ( uniqueId != -1 )
        return QString::number( uniqueId );
      else if ( z == -1 )
        return QStringLiteral( "%1/%2/%3" ).arg( d ).arg( x ).arg( y ); // quadtree
      else
        return QStringLiteral( "%1/%2/%3/%4" ).arg( d ).arg( x ).arg( y ).arg( z ); // octree
    }

    bool operator==( const QgsChunkNodeId &other ) const
    {
      return ( uniqueId == -1 && other.uniqueId == -1 && d == other.d && x == other.x && y == other.y && z == other.z )
             || ( uniqueId != -1 && uniqueId == other.uniqueId );
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
 * The data structure is a tree: each node may have several child nodes, all child nodes should
 * have their bounding box within the bounding box of their parent. Typically this is a quadtree,
 * an octree, but it may be also more general structure (with variable number of children). Nodes can exist
 * in several states (e.g. skeleton or loaded state) and they keep being loaded and unloaded as necessary
 * by the 3D rendering.
 *
 */
class QgsChunkNode
{
  public:
    //! constructs a skeleton chunk
    QgsChunkNode( const QgsChunkNodeId &nodeId, const QgsBox3D &box3D, float error, QgsChunkNode *parent = nullptr );

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
      Skeleton,        //!< Does not contain data of the chunk and data are not being loaded
      QueuedForLoad,   //!< Data are not available yet, but node is in the request queue
      Loading,         //!< Data are being loaded right now
      Loaded,          //!< Data are fully available
      QueuedForUpdate, //!< Loaded, but some data need to be updated - the node is in the queue
      Updating,        //!< Data are being updated right now
    };

    //! Returns 3D bounding box (in map coordinates) of the chunk
    QgsBox3D box3D() const { return mBox3D; }
    //! Returns measure geometric/texture error of the chunk (in world coordinates)
    float error() const { return mError; }
    //! Returns chunk tile coordinates of the tiling scheme
    QgsChunkNodeId tileId() const { return mNodeId; }
    //! Returns pointer to the parent node. Parent is NULLPTR in the root node
    QgsChunkNode *parent() const { return mParent; }
    //! Returns number of children of the node (returns -1 if the node has not yet been populated with populateChildren())
    int childCount() const { return mChildren.count(); }
    //! Returns array of the four children. Children may be NULLPTR if they were not created yet
    QgsChunkNode *const *children() const { return mChildren.constData(); }
    //! Returns how the chunked entity should behave when it is going to activate node's children
    Qgis::TileRefinementProcess refinementProcess() const { return mRefinementProcess; }
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

    //! Returns whether the child nodes have been populated already
    bool hasChildrenPopulated() const { return mChildrenPopulated; }

    //! Sets child nodes of this node. Takes ownership of all objects. Must be only called once.
    void populateChildren( const QVector<QgsChunkNode *> &children );

    //! Sets how the chunked entity should behave when it is going to activate node's children
    void setRefinementProcess( Qgis::TileRefinementProcess refinementProcess ) { mRefinementProcess = refinementProcess; }

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

    //! called when the true bounding box is known so that we can use tighter bounding box
    void setExactBox3D( const QgsBox3D &box3D );

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
    QgsBox3D mBox3D; //!< Bounding box in map coordinates
    float mError;    //!< Error of the node in world coordinates (negative error means that chunk at this level has no data, but there may be children that do)

    QgsChunkNodeId mNodeId; //!< Chunk coordinates (for use with a tiling scheme)

    QgsChunkNode *mParent;             //!< TODO: should be shared pointer
    QVector<QgsChunkNode *> mChildren; //!< Child nodes of this node. Initially children are not be populated
    bool mChildrenPopulated = false;   //!< Whether the child nodes (if any) have been already created

    State mState; //!< State of the node

    Qgis::TileRefinementProcess mRefinementProcess = Qgis::TileRefinementProcess::Replacement; //!< How to handle display of the node when children get activated

    QgsChunkListEntry *mLoaderQueueEntry;      //!< Not null <=> QueuedForLoad or QueuedForUpdate state
    QgsChunkListEntry *mReplacementQueueEntry; //!< Not null <=> has non-null entity (Loaded or QueuedForUpdate or Updating state)

    QgsChunkLoader *mLoader;    //!< Contains extra data necessary for entity creation (not null <=> Loading state)
    Qt3DCore::QEntity *mEntity; //!< Contains everything to display chunk as 3D object (not null <=> Loaded or QueuedForUpdate or Updating state)

    QgsChunkQueueJobFactory *mUpdaterFactory; //!< Object that creates updater (not null <=> QueuedForUpdate state)
    QgsChunkQueueJob *mUpdater;               //!< Object that does update of the chunk (not null <=> Updating state)

    QTime mEntityCreatedTime;
    bool mHasData = true; //!< Whether there are (will be) any data in this node and so whether it makes sense to load this node
};

/// @endcond

#endif // CHUNKNODE_H
