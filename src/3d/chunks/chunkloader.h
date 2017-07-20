#ifndef CHUNKLOADER_H
#define CHUNKLOADER_H

class ChunkNode;

namespace Qt3DCore
{
  class QEntity;
}


//! Abstract base class
class ChunkLoader
{
  public:
    ChunkLoader( ChunkNode *node )
      : node( node )
    {
    }

    virtual ~ChunkLoader();

    //! Run in worker thread to load data
    virtual void load() = 0;
    //! Run in main thread to use loaded data.
    //! Returns entity attached to the given parent entity in disabled state
    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) = 0;

  protected:
    ChunkNode *node;
};


//! Factory for chunk loaders for a particular type of entity
class ChunkLoaderFactory
{
  public:
    virtual ~ChunkLoaderFactory();

    virtual ChunkLoader *createChunkLoader( ChunkNode *node ) const = 0;
};


#endif // CHUNKLOADER_H
