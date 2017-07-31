#ifndef TERRAIN_H
#define TERRAIN_H

#include "chunkedentity.h"

#include <memory>

namespace Qt3DRender
{
  class QObjectPicker;
}

class Map3D;
class MapTextureGenerator;
class QgsCoordinateTransform;
class TerrainChunkEntity;
class TerrainGenerator;
class TerrainMapUpdateJobFactory;

/**
 * Controller for terrain - decides on what terrain tiles to show based on camera position
 * and creates them using map's terrain tile generator.
 */
class Terrain : public ChunkedEntity
{
    Q_OBJECT
  public:
    explicit Terrain( int maxLevel, const Map3D &map, Qt3DCore::QNode *parent = nullptr );

    ~Terrain();

    const Map3D &map3D() const { return map; }
    MapTextureGenerator *mapTextureGenerator() { return mMapTextureGenerator; }
    const QgsCoordinateTransform &terrainToMapTransform() const { return *mTerrainToMapTransform; }

    Qt3DRender::QObjectPicker *terrainPicker() const { return mTerrainPicker; }

  private slots:
    void onShowBoundingBoxesChanged();
    void invalidateMapImages();

  private:

    const Map3D &map;
    //! picker of terrain to know height of terrain when dragging
    Qt3DRender::QObjectPicker *mTerrainPicker;
    MapTextureGenerator *mMapTextureGenerator;
    QgsCoordinateTransform *mTerrainToMapTransform;

    std::unique_ptr<TerrainMapUpdateJobFactory> mUpdateJobFactory;
};


#include "chunkloader.h"

//! Handles asynchronous updates of terrain's map images when layers change
class TerrainMapUpdateJob : public ChunkQueueJob
{
    Q_OBJECT
  public:
    TerrainMapUpdateJob( MapTextureGenerator *mapTextureGenerator, ChunkNode *node );

    virtual void cancel() override;

  private slots:
    void onTileReady( int jobId, const QImage &image );

  private:
    MapTextureGenerator *mMapTextureGenerator;
    int mJobId;
};

#endif // TERRAIN_H
