#ifndef TERRAINCHUNKLOADER_H
#define TERRAINCHUNKLOADER_H

#include "chunkloader.h"

#include <QImage>
#include "qgsrectangle.h"

class Terrain;
class TerrainChunkEntity;


/** \ingroup 3d
 * Base class for chunk loaders for terrain tiles.
 * Adds functionality for asynchronous rendering of terrain tile map texture and access to the terrain entity.
 * \since QGIS 3.0
 */
class TerrainChunkLoader : public ChunkLoader
{
  public:
    //! Constructs loader for a chunk node
    TerrainChunkLoader( Terrain *terrain, ChunkNode *node );

  protected:
    //! Starts asynchronous rendering of map texture
    void loadTexture();
    //! Creates material component for the entity with the rendered map as a texture
    void createTextureComponent( TerrainChunkEntity *entity );
    //! Gives access to the terain entity
    Terrain *terrain() { return mTerrain; }

  private slots:
    void onImageReady( int jobId, const QImage &image );

  private:
    Terrain *mTerrain;
    QgsRectangle mExtentMapCrs;
    QString mTileDebugText;
    int mTextureJobId;
    QImage mTextureImage;
};


#endif // TERRAINCHUNKLOADER_H
