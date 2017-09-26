#ifndef QGSDEMTERRAINGENERATOR_H
#define QGSDEMTERRAINGENERATOR_H

#include "qgis_3d.h"

#include "qgsterraingenerator.h"

#include <memory>

class QgsDemHeightMapGenerator;

class QgsRasterLayer;

#include "qgsmaplayerref.h"

/** \ingroup 3d
 * Implementation of terrain generator that uses a raster layer with DEM to build terrain.
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsDemTerrainGenerator : public QgsTerrainGenerator
{
  public:
    QgsDemTerrainGenerator();

    //! Sets raster layer with elevation model to be used for terrain generation
    void setLayer( QgsRasterLayer *layer );
    //! Returns raster layer with elevation model to be used for terrain generation
    QgsRasterLayer *layer() const;

    //! Sets resolution of the generator (how many elevation samples on one side of a terrain tile)
    void setResolution( int resolution ) { mResolution = resolution; updateGenerator(); }
    //! Returns resolution of the generator (how many elevation samples on one side of a terrain tile)
    int resolution() const { return mResolution; }

    //! Returns height map generator object - takes care of extraction of elevations from the layer)
    QgsDemHeightMapGenerator *heightMapGenerator() { return mHeightMapGenerator.get(); }

    virtual QgsTerrainGenerator *clone() const override;
    Type type() const override;
    QgsRectangle extent() const override;
    float heightAt( double x, double y, const Qgs3DMapSettings &map ) const override;
    virtual void writeXml( QDomElement &elem ) const override;
    virtual void readXml( const QDomElement &elem ) override;
    virtual void resolveReferences( const QgsProject &project ) override;

    virtual QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override;

  private:
    void updateGenerator();

    std::unique_ptr<QgsDemHeightMapGenerator> mHeightMapGenerator;

    //! source layer for heights
    QgsMapLayerRef mLayer;
    //! how many vertices to place on one side of the tile
    int mResolution;
};


#endif // QGSDEMTERRAINGENERATOR_H
