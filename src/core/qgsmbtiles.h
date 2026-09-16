/***************************************************************************
  qgsmbtiles.h
  --------------------------------------
  Date                 : January 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMBTILES_H
#define QGSMBTILES_H

#include <sqlite3.h>

#include "qgis_core.h"
#include "qgssqliteutils.h"

#define SIP_NO_FILE

class QImage;
class QgsRectangle;

/**
 * \ingroup core
 * \brief Utility class for reading and writing MBTiles files (which are SQLite3 databases).
 *
 * See the specification for more details:
 * https://github.com/mapbox/mbtiles-spec/blob/master/1.3/spec.md
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsMbTiles
{
  public:
    //! Constructs MBTiles reader (but it does not open the file yet)
    explicit QgsMbTiles( const QString &filename );

    //! Tries to open the file, returns true on success
    bool open();

    //! Returns whether the MBTiles file is currently opened
    bool isOpen() const;

    /**
     * Finalizes the database after writing all tiles.
     *
     * This must be called if the database was created using a deferred index.
     *
     * \since QGIS 4.4
     */
    bool finalize();

    /**
     * Creates a new MBTiles file and initializes it with metadata and tiles tables.
     * It is up to the caller to set appropriate metadata entries and add tiles afterwards.
     *
     * \param deferIndexCreation can be set to TRUE to defer creation of the tile_index. This results
     * in much faster tile writes, as the index will not be updated after every tile write. If set to
     * TRUE, a call to finalize() must be made before closing the database. (Since QGIS 4.4)
     *
     * \returns TRUE on success. If the file exists already, returns FALSE.
     */
    bool create( bool deferIndexCreation = false );

    //! Requests metadata value for the given key
    QString metadataValue( const QString &key ) const;

    /**
     * Sets metadata value for the given key. Does not overwrite existing entries.
     * \note the database has to be opened in read-write mode (currently only when opened with create()
     */
    void setMetadataValue( const QString &key, const QString &value ) const;

    //! Returns bounding box from metadata, given in WGS 84 (if available)
    QgsRectangle extent() const;

    //! Returns raw tile data for given tile
    QByteArray tileData( int z, int x, int y ) const;

    //! Returns tile decoded as a raster image (if stored in a known format like JPG or PNG)
    QImage tileDataAsImage( int z, int x, int y ) const;

    /**
     * Adds tile data for the given tile coordinates. Does not overwrite existing entries.
     * \note the database has to be opened in read-write mode (currently only when opened with create()
     */
    void setTileData( int z, int x, int y, const QByteArray &data ) const;

    /**
     * Struct representing a raw image tile.
     * \since QGIS 4.4
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
     * \note the database has to be opened in read-write mode (currently only when opened with create())
     * \since QGIS 4.4
     */
    void setTileData( const QList<TileData> &tiles ) const;

  private:
    QString mFilename;
    sqlite3_database_unique_ptr mDatabase;
    bool mDeferredIndexCreation = false;
};


#endif // QGSMBTILES_H
