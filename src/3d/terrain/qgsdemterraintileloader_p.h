/***************************************************************************
  qgsdemterraintileloader_p.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

#define SIP_NO_FILE

#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>
#include <QElapsedTimer>
#include <QMutex>

#include "qgschunknode.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsrectangle.h"
#include "qgsterraintileloader.h"
#include "qgstilingscheme.h"

class QgsRasterDataProvider;
class QgsRasterLayer;
class QgsCoordinateTransformContext;
class QgsTerrainGenerator;

/**
 * \ingroup 3d
 * \brief Chunk loader for DEM terrain tiles.
 */
class QgsDemTerrainTileLoader : public QgsTerrainTileLoader
{
    Q_OBJECT
  public:
    //! Constructs loader for the given chunk node
    QgsDemTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *node, QgsTerrainGenerator *terrainGenerator );

    Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private slots:
    void onHeightMapReady( int jobId, const QByteArray &heightMap );

  private:
    int mHeightMapJobId;
    QByteArray mHeightMap;
    int mResolution;
    float mSkirtHeight;
};


class QgsTerrainDownloader;

/**
 * \ingroup 3d
 * \brief Utility class to asynchronously create heightmaps from DEM raster for given tiles of terrain.
 */
class QgsDemHeightMapGenerator : public QObject
{
    Q_OBJECT
  public:
    /**
     * Constructs height map generator based on a raster layer with elevation model,
     * terrain's tiling scheme and height map resolution (number of height values on each side of tile)
     */
    QgsDemHeightMapGenerator( QgsRasterLayer *dtm, const QgsTilingScheme &tilingScheme, int resolution, const QgsCoordinateTransformContext &transformContext );
    ~QgsDemHeightMapGenerator() override;

    //! asynchronous terrain read for a tile (array of floats)
    int render( const QgsChunkNodeId &nodeId );

    //! Waits for the tile to finish rendering
    void waitForFinished();

    //! Returns resolution(number of height values on each side of tile)
    int resolution() const { return mResolution; }

    //! returns height at given position (in terrain's CRS)
    float heightAt( double x, double y );

  signals:
    //! emitted when a previously requested heightmap is ready
    void heightMapReady( int jobId, const QByteArray &heightMap );

  private slots:
    void onFutureFinished();

  private:
    //! dtm raster layer's extent in layer crs
    const QgsRectangle mDtmExtent;

    //! cloned provider to be used in worker thread
    QgsRasterDataProvider *mClonedProvider = nullptr;

    QgsTilingScheme mTilingScheme;

    int mResolution;

    int mLastJobId;

    std::unique_ptr<QgsTerrainDownloader> mDownloader;

    struct JobData
    {
        int jobId;
        QgsChunkNodeId tileId;
        QgsRectangle extent;
        QFuture<QByteArray> future;
        QElapsedTimer timer;
    };

    QHash<QFutureWatcher<QByteArray> *, JobData> mJobs;

    void lazyLoadDtmCoarseData( int res, const QgsRectangle &rect );
    mutable QMutex mLazyLoadDtmCoarseDataMutex;
    //! used for height queries
    QByteArray mDtmCoarseData;

    QgsCoordinateTransformContext mTransformContext;
};

/// @endcond

#endif // QGSDEMTERRAINTILELOADER_P_H
