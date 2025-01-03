/***************************************************************************
  qgspostgresrastershareddata.h - QgsPostgresRasterSharedData

 ---------------------
 begin                : 8.1.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOSTGRESRASTERSHAREDDATA_H
#define QGSPOSTGRESRASTERSHAREDDATA_H

#include <QMutex>

#include "qgsrectangle.h"
#include "qgsgenericspatialindex.h"
#include "qgsgeometry.h"

class QgsPostgresConn;


/**
 * The QgsPostgresRasterSharedData class is thread safe and works as a data source
 * by fetching and caching tiles from the backend.
 * This class owns the tiles and manages its own memory.
 */
class QgsPostgresRasterSharedData
{
  public:
    //! Type for tile IDs, must be in sync with DB tile id extraction logic
    using TileIdType = QString;

    //! Tile data and metadata for a single band
    struct TileBand
    {
        TileIdType tileId;
        int srid;
        QgsRectangle extent;
        double upperLeftX;
        double upperLeftY;
        long int width;
        long int height;
        double scaleX;
        double scaleY;
        double skewX;
        double skewY;
        QByteArray data;
    };

    //! A tiles request
    struct TilesRequest
    {
        //! Band number
        int bandNo;
        QgsRectangle extent;
        unsigned int overviewFactor;
        //! PK
        QString pk;
        //! raster column
        QString rasterColumn;
        //! table name
        QString tableToQuery;
        //! SRID
        QString srid;
        //! where clause
        QString whereClause;
        //! RO DB connection
        QgsPostgresConn *conn;
    };

    //! A tiles response
    struct TilesResponse
    {
        //! Extent of the tiles in the response
        QgsRectangle extent;
        //! Tiles data
        QList<TileBand> tiles;
    };

    ~QgsPostgresRasterSharedData();

    /**
     * Returns tiles (possibly with NULL data) for a given \a request
     */
    TilesResponse tiles( const TilesRequest &request );

    /**
     * Invalidate the cache, for example when case the subset string changes
     */
    void invalidateCache();

    //! Generates the cache key from the request
    static QString keyFromRequest( const TilesRequest &request );

  private:
    //! Protect access to tiles
    QMutex mMutex;

    /**
     * The Tile struct represents a raster tile with metadata and data (initially NULL).
     */
    struct Tile
    {
        Tile( TileIdType tileId, int srid, QgsRectangle extent, double upperLeftX, double upperLeftY, long int width, long int height, double scaleX, double scaleY, double skewX, double skewY, int numBands );

        TileIdType tileId;
        int srid;
        QgsRectangle extent;
        double upperLeftX;
        double upperLeftY;
        long int width;
        long int height;
        double scaleX;
        double scaleY;
        double skewX;
        double skewY;
        int numBands;

        /**
         *  Returns data for specified \a bandNo
         */
        const QByteArray bandData( int bandNo ) const;

      private:
        std::vector<QByteArray> data;

        friend class QgsPostgresRasterSharedData;
    };

    bool fetchTilesData( unsigned int overviewFactor, const QList<TileIdType> &tileIds );
    bool fetchTilesIndex( const QgsGeometry &requestPolygon, const TilesRequest &request );
    //! Fast track for first fetch
    TilesResponse fetchTilesIndexAndData( const QgsGeometry &requestPolygon, const TilesRequest &request );
    Tile const *setTileData( const QString &cacheKey, TileIdType tileId, const QByteArray &data );

    /**
    * Tile caches, index is a key generated from the overview factor (1 is the full resolution data)
    * and the where clause
    * \note cannot be a smart pointer because spatial index cannot be copied
    */
    std::map<QString, QgsGenericSpatialIndex<Tile> *> mSpatialIndexes;

    //! Memory manager for owned tiles (and for tileId access)
    std::map<QString, std::map<TileIdType, std::unique_ptr<Tile>>> mTiles;

    //! Keeps track of loaded index bounds
    std::map<QString, QgsGeometry> mLoadedIndexBounds;
};

#endif // QGSPOSTGRESRASTERSHAREDDATA_H
