#ifndef TERRAIN_H
#define TERRAIN_H

#include "chunkedentity.h"

#include <memory>

namespace Qt3DRender
{
  class QObjectPicker;
}

class Qgs3DMapSettings;
class MapTextureGenerator;
class QgsCoordinateTransform;
class QgsMapLayer;
class TerrainChunkEntity;
class TerrainGenerator;
class TerrainMapUpdateJobFactory;

/** \ingroup 3d
 * Controller for terrain - decides on what terrain tiles to show based on camera position
 * and creates them using map's terrain tile generator.
 * \since QGIS 3.0
 */
class Terrain : public ChunkedEntity
{
    Q_OBJECT
  public:
    //! Constructs terrain entity. The argument maxLevel determines how deep the tree of tiles will be
    explicit Terrain( int maxLevel, const Qgs3DMapSettings &map, Qt3DCore::QNode *parent = nullptr );

    ~Terrain();

    //! Returns associated 3D map settings
    const Qgs3DMapSettings &map3D() const { return map; }
    //! Returns pointer to the generator of textures for terrain tiles
    MapTextureGenerator *mapTextureGenerator() { return mMapTextureGenerator; }
    //! Returns transform from terrain's CRS to map CRS
    const QgsCoordinateTransform &terrainToMapTransform() const { return *mTerrainToMapTransform; }

    //! Returns object picker attached to the terrain entity - used by camera controller
    Qt3DRender::QObjectPicker *terrainPicker() const { return mTerrainPicker; }

  private slots:
    void onShowBoundingBoxesChanged();
    void invalidateMapImages();
    void onLayersChanged();

  private:

    void connectToLayersRepaintRequest();

    const Qgs3DMapSettings &map;
    //! picker of terrain to know height of terrain when dragging
    Qt3DRender::QObjectPicker *mTerrainPicker;
    MapTextureGenerator *mMapTextureGenerator;
    QgsCoordinateTransform *mTerrainToMapTransform;

    std::unique_ptr<TerrainMapUpdateJobFactory> mUpdateJobFactory;

    //! layers that are currently being used for map rendering (and thus being watched for renderer updates)
    QList<QgsMapLayer *> mLayers;
};

///@cond PRIVATE

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

/// @endcond


#endif // TERRAIN_H
