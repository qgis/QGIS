#ifndef CHUNKLOADER_H
#define CHUNKLOADER_H

class ChunkNode;

namespace Qt3DCore
{
  class QEntity;
}

#include <QObject>

/** \ingroup 3d
 * Base class for chunk queue jobs. Job implementations start their work when they are created
 * and all work is done asynchronously, i.e. constructor should exit as soon as possible and
 * all work should be done in a worker thread. Once the job is done, finished() signal is emitted
 * and will be processed by the parent chunked entity.
 *
 * There are currently two types of queue jobs:
 *  1. chunk loaders: prepares all data needed for creation of entities (ChunkLoader sub-class)
 *  2. chunk updaters: given a chunk with already existing entity, it updates the entity (e.g. update texture or geometry)
 *
 * \since QGIS 3.0
 */
class ChunkQueueJob : public QObject
{
    Q_OBJECT
  public:
    ChunkQueueJob( ChunkNode *node )
      : node( node )
    {
    }

    virtual ~ChunkQueueJob();

    ChunkNode *chunk() { return node; }

    //! Request that the loading gets canceled.
    //! Returns only after the async job has been stopped.
    //! The signal finished() will not be emitted afterwards.
    virtual void cancel();

  signals:
    void finished();

  protected:
    ChunkNode *node;
};

/** \ingroup 3d
 * Base class for factories of chunk queue jobs. Derived classes need to implement createJob()
 * method that will create a specific job for given chunk node.
 * \since QGIS 3.0
 */
class ChunkQueueJobFactory
{
  public:
    virtual ~ChunkQueueJobFactory() = default;

    //! Creates a specific chunk queue job for the chunk node. Ownership of the returned is passed to the caller.
    virtual ChunkQueueJob *createJob( ChunkNode *chunk ) = 0;
};


//! Base class for jobs that load chunks
class ChunkLoader : public ChunkQueueJob
{
    Q_OBJECT
  public:
    //! Construct chunk loader for a node
    ChunkLoader( ChunkNode *node )
      : ChunkQueueJob( node )
    {
    }

    virtual ~ChunkLoader();

    //! Run in main thread to use loaded data.
    //! Returns entity attached to the given parent entity in disabled state
    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) = 0;

};


//! Factory for chunk loaders for a particular type of entity
class ChunkLoaderFactory
{
  public:
    virtual ~ChunkLoaderFactory();

    //! Creates loader for the given chunk node. Ownership of the returned is passed to the caller.
    virtual ChunkLoader *createChunkLoader( ChunkNode *node ) const = 0;
};


#endif // CHUNKLOADER_H
