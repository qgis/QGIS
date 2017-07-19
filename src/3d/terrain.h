#ifndef TERRAIN_H
#define TERRAIN_H

#include "chunkedentity.h"

class Map3D;
class MapTextureGenerator;
class QgsCoordinateTransform;
class TerrainGenerator;

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

  private:

    const Map3D &map;
    MapTextureGenerator *mMapTextureGenerator;
    QgsCoordinateTransform *mTerrainToMapTransform;
};

#endif // TERRAIN_H
