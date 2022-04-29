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

#include "qgis_core.h"

#include "sqlite3.h"
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
     * Creates a new MBTiles file and initializes it with metadata and tiles tables.
     * It is up to the caller to set appropriate metadata entries and add tiles afterwards.
     * Returns TRUE on success. If the file exists already, returns FALSE.
     */
    bool create();

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

  private:
    QString mFilename;
    sqlite3_database_unique_ptr mDatabase;
};


#endif // QGSMBTILES_H
