#ifndef DEMTERRAINGENERATOR_H
#define DEMTERRAINGENERATOR_H

#include "qgis_3d.h"

#include "terraingenerator.h"
#include "terrainchunkloader.h"

#include <memory>

class DemHeightMapGenerator;

class QgsRasterLayer;

#include "qgsmaplayerref.h"

/**
 * Implementation of terrain generator that uses a raster layer with DEM to build terrain.
 */
class _3D_EXPORT DemTerrainGenerator : public TerrainGenerator
{
  public:
    DemTerrainGenerator();

    void setLayer( QgsRasterLayer *layer );
    QgsRasterLayer *layer() const;

    void setResolution( int resolution ) { mResolution = resolution; updateGenerator(); }
    int resolution() const { return mResolution; }

    DemHeightMapGenerator *heightMapGenerator() { return mHeightMapGenerator.get(); }

    virtual TerrainGenerator *clone() const override;
    Type type() const override;
    QgsRectangle extent() const override;
    float heightAt( double x, double y, const Qgs3DMapSettings &map ) const override;
    virtual void writeXml( QDomElement &elem ) const override;
    virtual void readXml( const QDomElement &elem ) override;
    virtual void resolveReferences( const QgsProject &project ) override;

    virtual ChunkLoader *createChunkLoader( ChunkNode *node ) const override;

  private:
    void updateGenerator();

    std::unique_ptr<DemHeightMapGenerator> mHeightMapGenerator;

    //! source layer for heights
    QgsMapLayerRef mLayer;
    //! how many vertices to place on one side of the tile
    int mResolution;
};



class DemTerrainChunkLoader : public TerrainChunkLoader
{
    Q_OBJECT
  public:
    DemTerrainChunkLoader( Terrain *terrain, ChunkNode *node );
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

/**
 * Utility class to asynchronously create heightmaps from DEM raster for given tiles of terrain.
 */
class DemHeightMapGenerator : public QObject
{
    Q_OBJECT
  public:
    DemHeightMapGenerator( QgsRasterLayer *dtm, const QgsTilingScheme &tilingScheme, int resolution );
    ~DemHeightMapGenerator();

    //! asynchronous terrain read for a tile (array of floats)
    int render( int x, int y, int z );

    //! synchronous terrain read for a tile
    QByteArray renderSynchronously( int x, int y, int z );

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

#endif // DEMTERRAINGENERATOR_H
