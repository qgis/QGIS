#ifndef QGSDEMTERRAINGENERATOR_H
#define QGSDEMTERRAINGENERATOR_H

#include "qgis_3d.h"

#include "qgsterraingenerator.h"
#include "qgsterraintileloader_p.h"

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

    virtual ChunkLoader *createChunkLoader( ChunkNode *node ) const override;

  private:
    void updateGenerator();

    std::unique_ptr<QgsDemHeightMapGenerator> mHeightMapGenerator;

    //! source layer for heights
    QgsMapLayerRef mLayer;
    //! how many vertices to place on one side of the tile
    int mResolution;
};



/** \ingroup 3d
 * Chunk loader for DEM terrain tiles.
 * \since QGIS 3.0
 */
class DemTerrainChunkLoader : public QgsTerrainTileLoader
{
    Q_OBJECT
  public:
    //! Constructs loader for the given chunk node
    DemTerrainChunkLoader( QgsTerrainEntity *terrain, ChunkNode *node );
    ~DemTerrainChunkLoader();

    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent );

  private slots:
    void onHeightMapReady( int jobId, const QByteArray &heightMap );

  private:

    int heightMapJobId;
    QByteArray heightMap;
    int resolution;
};



#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>
#include <QElapsedTimer>

#include "qgsrectangle.h"
class QgsRasterDataProvider;

/** \ingroup 3d
 * Utility class to asynchronously create heightmaps from DEM raster for given tiles of terrain.
 * \since QGIS 3.0
 */
class QgsDemHeightMapGenerator : public QObject
{
    Q_OBJECT
  public:
    //! Constructs height map generator based on a raster layer with elevation model,
    //! terrain's tiling scheme and height map resolution (number of height values on each side of tile)
    QgsDemHeightMapGenerator( QgsRasterLayer *dtm, const QgsTilingScheme &tilingScheme, int resolution );
    ~QgsDemHeightMapGenerator();

    //! asynchronous terrain read for a tile (array of floats)
    int render( int x, int y, int z );

    //! synchronous terrain read for a tile
    QByteArray renderSynchronously( int x, int y, int z );

    //! Returns resolution(number of height values on each side of tile)
    int resolution() const { return res; }

    //! returns height at given position (in terrain's CRS)
    float heightAt( double x, double y );

  signals:
    //! emitted when a previously requested heightmap is ready
    void heightMapReady( int jobId, const QByteArray &heightMap );

  private slots:
    void onFutureFinished();

  private:
    //! raster used to build terrain
    QgsRasterLayer *dtm;

    //! cloned provider to be used in worker thread
    QgsRasterDataProvider *clonedProvider;

    QgsTilingScheme tilingScheme;

    int res;

    int lastJobId;

    struct JobData
    {
      int jobId;
      QgsRectangle extent;
      QFuture<QByteArray> future;
      QFutureWatcher<QByteArray> *fw;
      QElapsedTimer timer;
    };

    QHash<QFutureWatcher<QByteArray>*, JobData> jobs;

    //! used for height queries
    QByteArray dtmCoarseData;
};

#endif // QGSDEMTERRAINGENERATOR_H
