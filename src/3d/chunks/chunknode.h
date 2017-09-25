#ifndef CHUNKNODE_H
#define CHUNKNODE_H


#include "aabb.h"

#include <QTime>

namespace Qt3DCore
{
  class QEntity;
}

class ChunkListEntry;
class ChunkLoader;
class ChunkQueueJob;
class ChunkQueueJobFactory;


/** \ingroup 3d
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
class ChunkNode
{
  public:
    //! constructs a skeleton chunk
    ChunkNode( int x, int y, int z, const AABB &bbox, float error, ChunkNode *parent = nullptr );

    ~ChunkNode();

    //! Returns true if all child chunks are available and thus this node could be swapped to the child nodes
    bool allChildChunksResident( const QTime &currentTime ) const;

    //! make sure that all child nodes are at least skeleton nodes
    void ensureAllChildrenExist();

    //! how deep is the node in the tree (zero means root node, every level adds one)
    int level() const;

    //! Returns list of all descendants (recursive, not just direct children)
    QList<ChunkNode *> descendants();

    //
    // changes of states in the state machine (see State enum)
    //

    //! mark a chunk as being queued for loading
    void setQueuedForLoad( ChunkListEntry *entry );

    //! unload node that is in "queued for load" state
    void cancelQueuedForLoad();

    //! mark a chunk as being loaded, using the passed loader
    void setLoading( ChunkLoader *chunkLoader );

    //! turn a chunk that is being loaded back to skeleton node
    void cancelLoading();

    //! mark a chunk as loaded, using the loaded entity
    void setLoaded( Qt3DCore::QEntity *entity );

    //! turn a loaded chunk into skeleton
    void unloadChunk();

    //! turn a loaded node into one that is queued for update - with custom update job factory
    void setQueuedForUpdate( ChunkListEntry *entry, ChunkQueueJobFactory *updateJobFactory );

    //! cancel update of the node - back to loaded node
    void cancelQueuedForUpdate();

    //! mark node as being updated right now
    void setUpdating();

    //! turn a chunk that is being updated back to loaded node
    void cancelUpdating();

    //! mark node that it finished updating - back to loaded node
    void setUpdated();

    //! called when bounding box
    void setExactBbox( const AABB &box );

    AABB bbox;      //!< Bounding box in world coordinates
    float error;    //!< Error of the node in world coordinates

    int x, y, z;  //!< Chunk coordinates (for use with a tiling scheme)

    ChunkNode *parent;        //!< TODO: should be shared pointer
    ChunkNode *children[4];   //!< TODO: should be weak pointers. May be null if not created yet or removed already

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

    State state;  //!< State of the node

    ChunkListEntry *loaderQueueEntry;       //!< Not null <=> QueuedForLoad or QueuedForUpdate state
    ChunkListEntry *replacementQueueEntry;  //!< Not null <=> has non-null entity (Loaded or QueuedForUpdate or Updating state)

    ChunkLoader *loader;         //!< Contains extra data necessary for entity creation (not null <=> Loading state)
    Qt3DCore::QEntity *entity;   //!< Contains everything to display chunk as 3D object (not null <=> Loaded or QueuedForUpdate or Updating state)

    ChunkQueueJobFactory *updaterFactory;  //!< Object that creates updater (not null <=> QueuedForUpdate state)
    ChunkQueueJob *updater;                //!< Object that does update of the chunk (not null <=> Updating state)

    QTime entityCreatedTime;
};

#endif // CHUNKNODE_H
