/***************************************************************************
  qgsgpkgtiles.h
  --------------------------------------
  Date                 : September 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPKGTILES_H
#define QGSGPKGTILES_H

#include "qgis_core.h"
#include "qgsrectangle.h"
#include "qgssqliteutils.h"

#define SIP_NO_FILE

class QgsCoordinateReferenceSystem;


/**
 * \ingroup core
 * \brief Utility class for creating GeoPackage raster tile databases.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsGeoPackageTiles
{
  public:
    //! Constructs the GeoPackage reader (but it does not open the file yet)
    explicit QgsGeoPackageTiles( const QString &filename );

    ~QgsGeoPackageTiles();

    /**
     * Creates a new GeoPackage database and initializes it with metadata and tiles tables.
     *
     * \param crs target CRS for the tile layer and matrix set.
     * \param tileMatrixSetExtent spatial extent of the full tile matrix set (pyramid bounds), specified in coordinates of the target \a crs.
     * \param contentsExtent actual bounding box of data stored in gpkg_contents, specified in coordinates of the target \a crs.
     * \param minZoom minimum zoom level.
     * \param maxZoom maximum zoom level.
     * \param tileWidth pixel width of individual tiles.
     * \param tileHeight pixel height of individual tiles.
     * \param z0MatrixWidth number of tile columns at zoom level 0.
     * \param z0MatrixHeight number of tile rows at zoom level 0.
     *
     * \returns TRUE on success. If the file exists already, returns FALSE.
     *
     * \note finalize() must be called after creating the database.
     */
    bool create(
      const QgsCoordinateReferenceSystem &crs,
      const QgsRectangle &tileMatrixSetExtent,
      const QgsRectangle &contentsExtent,
      int minZoom,
      int maxZoom,
      int tileWidth = 256,
      int tileHeight = 256,
      int z0MatrixWidth = 1,
      int z0MatrixHeight = 1
    );

    /**
     * Finalizes the database after writing all tiles.
     */
    bool finalize();

    /**
     * Struct representing a raw image tile.
     */
    struct TileData
    {
        int z = 0;
        int x = 0;
        int y = 0;
        QByteArray data;
    };

    /**
     * Adds a batch of tile data within a single SQLite transaction.
     */
    void setTileData( const QList<TileData> &tiles ) const;

    /**
     * Explicitly closes the database.
     */
    bool close();

    /**
     * Returns the latest error message obtained from the database.
     */
    QString lastError() const { return mLastError; }

  private:
    QString mFilename;
    sqlite3_database_unique_ptr mDatabase;
    mutable QString mLastError;
};


#endif // QGSGPKGTILES_H
