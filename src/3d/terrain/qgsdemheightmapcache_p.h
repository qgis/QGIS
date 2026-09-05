/***************************************************************************
  qgsdemheightmapcache_p.h
  --------------------------------------
  Date                 : January 2026
  Copyright            : (C) 2026 By Oslandia
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDEMHEIGHTMAPCACHE_P_H
#define QGSDEMHEIGHTMAPCACHE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgis_3d.h"

#include <QMap>
#include <QMutex>
#include <QObject>

#define SIP_NO_FILE

class QgsChunkNode;
struct QgsChunkNodeId;
class QgsRectangle;
class QgsDemHeightMapGenerator;
class Qgs3DRenderContext;

/**
 * \ingroup qgis_3d
 * \brief Handle DEM elevation data cache
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsDemHeightMapCache : public QObject
{
    Q_OBJECT
  public:
    //! Default constructor
    QgsDemHeightMapCache( QgsDemHeightMapGenerator *generator, int resolution, int emitLevel, QgsChunkNode *rootNode );

    //! Returns height map cache size
    size_t size() const;

    /**
     * Checks if the tile is in the cache
     * \param tileText text version of the tile
     * \return true if the tile is in the cache
     */
    bool containsTile( const QString &tileText );

    /**
     * Computes height and the height quality at \a x, \a y
     * \param x coordinate in map's CRS
     * \param y coordinate in map's CRS
     * \param height will contain height
     * \param quality will contain height quality
     */
    void heightAndQualityAt( double x, double y, float &height, int &quality ) const;

    /**
     * Update terrain root node
     * \param rootNode new root node
     */
    void setTerrainRootNode( QgsChunkNode *rootNode );

    /**
     * Compute min/max height from cached data
     * \param zMin will contain minheight
     * \param zMax will contain max height
     */
    void heightMinMax( float &zMin, float &zMax ) const;

  signals:
    //! emitted when an hi-res DEM tile has been received
    void maxResTileReceived( const QgsChunkNodeId &tileId, const QgsRectangle &extent );

  public slots:
    //! to connect to QgsDemHeightMapGenerator::heightMapReady
    void onHeightMapReceived( int jobId, const QgsChunkNode *node, const QgsRectangle &extent, const QByteArray &heightMap );

  private:
    //! map used as height map data cache
    mutable QMap<QString, QByteArray> mLoaderMap;
    //! protect cache and mRootNode for concurrent updates
    mutable QRecursiveMutex mRootNodeMutex;

    QgsDemHeightMapGenerator *mHeightMapGenerator = nullptr;

    //! how many vertices to place on one side of the tile
    int mResolution = 16;
    //! min node level to start signal emittion
    int mEmitLevel;
    //! root node used to search best height map data according to x/y
    QgsChunkNode *mRootNode = nullptr;

    float mZMin;
    float mZMax;

    //! cleanup cache: when children are in cache, remove parent
    void cleanup( const QgsChunkNode *currentNode ) const;
};


/**
 * \ingroup qgis_3d
 * \brief Interface from terrain generator with cache
 * \since QGIS 4.2
 */
class QgsTerrainGeneratorWithCache
{
  public:
    QgsTerrainGeneratorWithCache() = default;
    virtual ~QgsTerrainGeneratorWithCache() = default;

    /**
     * Returns height quality at point, the higher the better (max is the mMaxLevel). -1 = no data, 0 = coarse data.
     * \param x coordinate in map's CRS
     * \param y coordinate in map's CRS
     * \param context 3d render context
     * \return height quality at (x,y) in map's CRS
     */
    virtual int qualityAt( double x, double y, const Qgs3DRenderContext &context ) const = 0;

    /**
     * \return height cache manager
     */
    virtual QgsDemHeightMapCache *heightMapCache() const = 0;
};

#endif // QGSDEMHEIGHTMAPCACHE_P_H
