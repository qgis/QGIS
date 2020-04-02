/***************************************************************************
  qgsmbtilesreader.h
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

#ifndef QGSMBTILESREADER_H
#define QGSMBTILESREADER_H

#include "qgis_core.h"

#include "sqlite3.h"
#include "qgssqliteutils.h"

#define SIP_NO_FILE

class QImage;
class QgsRectangle;

/**
 * \ingroup core
 * Utility class for reading MBTiles files (which are SQLite3 databases).
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsMBTilesReader
{
  public:
    //! Contructs MBTiles reader (but it does not open the file yet)
    explicit QgsMBTilesReader( const QString &filename );

    //! Tries to open the file, returns true on success
    bool open();

    //! Returns whether the MBTiles file is currently opened
    bool isOpen() const;

    //! Requests metadata value for the given key
    QString metadataValue( const QString &key );

    //! Returns bounding box from metadata, given in WGS 84 (if available)
    QgsRectangle extent();

    //! Returns raw tile data for given tile
    QByteArray tileData( int z, int x, int y );

    //! Returns tile decoded as a raster image (if stored in a known format like JPG or PNG)
    QImage tileDataAsImage( int z, int x, int y );

  private:
    QString mFilename;
    sqlite3_database_unique_ptr mDatabase;
};


#endif // QGSMBTILESREADER_H
