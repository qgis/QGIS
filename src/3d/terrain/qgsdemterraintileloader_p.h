#ifndef QGSDEMTERRAINTILELOADER_P_H
#define QGSDEMTERRAINTILELOADER_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>
#include <QElapsedTimer>

#include "qgsrectangle.h"
#include "qgsterraintileloader_p.h"
#include "qgstilingscheme.h"

class QgsRasterDataProvider;
class QgsRasterLayer;

/** \ingroup 3d
 * Chunk loader for DEM terrain tiles.
 * \since QGIS 3.0
 */
class QgsDemTerrainTileLoader : public QgsTerrainTileLoader
{
    Q_OBJECT
  public:
    //! Constructs loader for the given chunk node
    QgsDemTerrainTileLoader( QgsTerrainEntity *terrain, ChunkNode *node );
    ~QgsDemTerrainTileLoader();

    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent );

  private slots:
    void onHeightMapReady( int jobId, const QByteArray &heightMap );

  private:

    int heightMapJobId;
    QByteArray heightMap;
    int resolution;
};



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

/// @endcond

#endif // QGSDEMTERRAINTILELOADER_P_H
