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

/**
 * The QgsPostgresRasterSharedData class is thread safe.
 * The cache owns the tiles and manages its memory.
 */
class QgsPostgresRasterSharedData
{
  public:

    /**
     * The Tile struct represents a raster tile with metadata and data (intially NULL).
     */
    struct Tile
    {
      Tile( long int tileId,
            int srid,
            QgsRectangle extent,
            double upperLeftX,
            double upperLeftY,
            long int width,
            long int height,
            double scaleX,
            double scaleY,
            double skewX,
            double skewY,
            int numBands );

      long int tileId;
      int srid;
      QgsRectangle extent;
      double upperLeftX;
      double upperLeftY;
      long int width;
      long int height;
      double scaleX ;
      double scaleY;
      double skewX;
      double skewY;
      int numBands;
      QByteArray data;

      /**
       *  Returns data for specified \a bandNo
       */
      QByteArray bandData( int bandNo );
    };

    ~QgsPostgresRasterSharedData( );


    /**
     * Returns tiles (possibly with NULL data) for a given \a extent and \a overviewFactor.
     */
    QList<Tile *> tiles( unsigned int overviewFactor, const QgsRectangle &extent );

    /**
     * Adds a \a tile to the index corresponding to the \a overviewFactor, the index takes ownership of the tile.
     */
    Tile *addToIndex( unsigned int overviewFactor, Tile *tile );

    /**
     * Creates spatial indexes for full resolution data and (possibly empty) \a overviewFactors
     */
    void initIndexes( const std::list<unsigned int> &overviewFactors );

    /**
     * Checks if the index \a overviewFactor is empty
     */
    bool indexIsEmpty( unsigned int overviewFactor );


  private:

    //! Protect access to tile indexes
    QMutex mMutex;

    // Note: cannot be a smart pointer because spatial index cannot be copied
    //! Tile caches, index is the overview factor (1 is the full resolution data)
    std::map<unsigned int, QgsGenericSpatialIndex<Tile>*> mSpatialIndexes;

    //! Memory manager for owned tiles
    std::list<std::unique_ptr<Tile>> mTiles;

};

#endif // QGSPOSTGRESRASTERSHAREDDATA_H
