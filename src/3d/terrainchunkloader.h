#ifndef TERRAINCHUNKLOADER_H
#define TERRAINCHUNKLOADER_H

#include "chunkloader.h"

#include <QImage>
#include "qgsrectangle.h"

class Terrain;


class TerrainChunkLoader : public ChunkLoader
{
  public:
    TerrainChunkLoader( Terrain *terrain, ChunkNode *node );

    void loadTexture();
    void createTextureComponent( Qt3DCore::QEntity *entity );

  protected:
    Terrain *mTerrain;

  private:
    QgsRectangle mExtentMapCrs;
    QString mTileDebugText;
    QImage mTextureImage;
};


#endif // TERRAINCHUNKLOADER_H
