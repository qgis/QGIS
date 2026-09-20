/***************************************************************************
  qgsgpkgtiles.cpp
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

#include "qgsgpkgtiles.h"

#include <sqlite3.h>

#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsrectangle.h"
#include "qgstiles.h"

#include <QFile>
#include <QString>

using namespace Qt::StringLiterals;

QgsGeoPackageTiles::QgsGeoPackageTiles( const QString &filename )
  : mFilename( filename )
{}

QgsGeoPackageTiles::~QgsGeoPackageTiles()
{
  close();
}

bool QgsGeoPackageTiles::finalize()
{
  if ( !mDatabase )
    return false;

  const int result = mDatabase.exec( u"CREATE UNIQUE INDEX IF NOT EXISTS uk_tiles ON tiles (zoom_level, tile_column, tile_row);"_s, mLastError );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"Failed to create GeoPackage tile index: %1"_s.arg( mLastError ) );
    return false;
  }

  return true;
}

bool QgsGeoPackageTiles::create( const QgsTileMatrix &z0matrix, const QgsRectangle &contentsExtent, int minZoom, int maxZoom, int tileWidth, int tileHeight )
{
  if ( mFilename.isEmpty() )
    return false;

  if ( QFile::exists( mFilename ) )
  {
    mLastError = u"File already exists"_s;
    return false;
  }

  int result = mDatabase.open_v2( mFilename, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr );
  if ( result != SQLITE_OK )
  {
    mLastError = mDatabase.errorMessage();
    QgsDebugError( u"Can't create GeoPackage database: %1"_s.arg( mLastError ) );
    return false;
  }

  QString errorMessage;
  // ignore errors from these, they aren't critical
  // optimise writing speed
  mDatabase.exec( u"PRAGMA journal_mode = WAL;"_s, errorMessage );
  mDatabase.exec( u"PRAGMA synchronous = OFF;"_s, errorMessage );
  mDatabase.exec( u"PRAGMA temp_store = MEMORY;"_s, errorMessage );
  mDatabase.exec( u"PRAGMA cache_size = -64000;"_s, errorMessage );

  mDatabase.exec( u"PRAGMA application_id = 1196444487;"_s, mLastError );
  mDatabase.exec( u"PRAGMA user_version = 10200;"_s, mLastError );
  mDatabase.exec( u"BEGIN TRANSACTION;"_s, errorMessage );

  const QString sql = "CREATE TABLE gpkg_spatial_ref_sys ("
                      "  srs_name TEXT NOT NULL, srs_id INTEGER NOT NULL PRIMARY KEY,"
                      "  organization TEXT NOT NULL, organization_coordsys_id INTEGER NOT NULL,"
                      "  definition TEXT NOT NULL, description TEXT"
                      ");"
                      "CREATE TABLE gpkg_contents ("
                      "  table_name TEXT NOT NULL PRIMARY KEY, data_type TEXT NOT NULL,"
                      "  identifier TEXT UNIQUE, description TEXT DEFAULT '',"
                      "  last_change DATETIME NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ', 'now')),"
                      "  min_x DOUBLE, min_y DOUBLE, max_x DOUBLE, max_y DOUBLE, srs_id INTEGER"
                      ");"
                      "CREATE TABLE gpkg_tile_matrix_set ("
                      "  table_name TEXT NOT NULL PRIMARY KEY, srs_id INTEGER NOT NULL,"
                      "  min_x DOUBLE NOT NULL, min_y DOUBLE NOT NULL,"
                      "  max_x DOUBLE NOT NULL, max_y DOUBLE NOT NULL"
                      ");"
                      "CREATE TABLE gpkg_tile_matrix ("
                      "  table_name TEXT NOT NULL, zoom_level INTEGER NOT NULL,"
                      "  matrix_width INTEGER NOT NULL, matrix_height INTEGER NOT NULL,"
                      "  tile_width INTEGER NOT NULL, tile_height INTEGER NOT NULL,"
                      "  pixel_x_size DOUBLE NOT NULL, pixel_y_size DOUBLE NOT NULL,"
                      "  CONSTRAINT pk_ttm PRIMARY KEY (table_name, zoom_level)"
                      ");"
                      "CREATE TABLE tiles ("
                      "  id INTEGER PRIMARY KEY AUTOINCREMENT, zoom_level INTEGER NOT NULL,"
                      "  tile_column INTEGER NOT NULL, tile_row INTEGER NOT NULL,"
                      "  tile_data BLOB NOT NULL"
                      ");";

  result = mDatabase.exec( sql, mLastError );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"Failed to initialize GeoPackage database tables: "_s + mLastError );
    return false;
  }

  // Insert standard GeoPackage SRS entries
  QString srsSql = "INSERT INTO gpkg_spatial_ref_sys VALUES "
                   "('Undefined Cartesian SRS', -1, 'NONE', -1, 'undefined', 'undefined Cartesian coordinate reference system')," // #spellok
                   "('Undefined geographic SRS', 0, 'NONE', 0, 'undefined', 'undefined geographic coordinate reference system'),";
  srsSql += "('WGS 84 geodetic', 4326, 'EPSG', 4326, 'GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AXIS[\"Latitude\",NORTH],AXIS[\"Longitude\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]', 'longitude/latitude coordinates in decimal degrees on the WGS 84 spheroid'),"_L1;
  srsSql += "('WGS 84 / Pseudo-Mercator', 3857, 'EPSG', 3857, 'GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Mercator_1SP\"],PARAMETER[\"central_meridian\",0],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],EXTENSION[\"PROJ4\",\"+proj=merc +a=6378137 +b=6378137 +lat_ts=0 +lon_0=0 +x_0=0 +y_0=0 +k=1 +units=m +nadgrids=@null +wktext +no_defs\"],AUTHORITY[\"EPSG\",\"3857\"]]', '')"_L1;

  result = mDatabase.exec( srsSql, mLastError );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"Failed to populate GeoPackage gpkg_spatial_ref_sys table: "_s + mLastError );
    return false;
  }

  const QgsCoordinateReferenceSystem crs = z0matrix.crs();
  long srsId = crs.postgisSrid();
  if ( srsId <= 0 )
    srsId = crs.srsid();

  if ( crs != QgsCoordinateReferenceSystem( "EPSG:4326" ) && crs != QgsCoordinateReferenceSystem( "EPSG:3857" ) )
  {
    const QStringList authidParts = crs.authid().split( ':' );
    const QString org = authidParts.size() == 2 && !authidParts.at( 0 ).isEmpty() ? authidParts.at( 0 ) : u"NONE"_s;
    const int orgId = authidParts.size() == 2 ? authidParts.at( 1 ).toInt() : 0;
    const QString wkt = crs.toWkt( Qgis::CrsWktVariant::Wkt1Gdal );

    const QString customSrsSql = QString(
                                   "INSERT OR IGNORE INTO gpkg_spatial_ref_sys (srs_name, srs_id, organization, organization_coordsys_id, definition, description) "
                                   "VALUES (%1, %2, %3, %4, %5, %6);"
    )
                                   .arg( QgsSqliteUtils::quotedValue( crs.description().isEmpty() ? crs.authid() : crs.description() ) )
                                   .arg( srsId )
                                   .arg( QgsSqliteUtils::quotedValue( org ) )
                                   .arg( orgId )
                                   .arg( QgsSqliteUtils::quotedValue( wkt ) )
                                   .arg( QgsSqliteUtils::quotedValue( crs.description() ) );

    result = mDatabase.exec( customSrsSql, mLastError );
    if ( result != SQLITE_OK )
    {
      QgsDebugError( u"Failed to insert target SRS into GeoPackage gpkg_spatial_ref_sys table: "_s + mLastError );
      return false;
    }
  }


  const QString contentsSql = QString(
                                "INSERT INTO gpkg_contents (table_name, data_type, identifier, min_x, min_y, max_x, max_y, srs_id) "
                                "VALUES ('tiles', 'tiles', 'tiles', %1, %2, %3, %4, %5);"
  )
                                .arg( contentsExtent.xMinimum(), 0, 'g', 17 )
                                .arg( contentsExtent.yMinimum(), 0, 'g', 17 )
                                .arg( contentsExtent.xMaximum(), 0, 'g', 17 )
                                .arg( contentsExtent.yMaximum(), 0, 'g', 17 )
                                .arg( srsId );
  result = mDatabase.exec( contentsSql, mLastError );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"Failed to populate GeoPackage gpkg_contents table: "_s + mLastError );
    return false;
  }

  const QgsRectangle tileMatrixSetExtent = z0matrix.extent();
  const QString tmsSql = QString( "INSERT INTO gpkg_tile_matrix_set VALUES ('tiles', %1, %2, %3, %4, %5);" )
                           .arg( srsId )
                           .arg( tileMatrixSetExtent.xMinimum(), 0, 'g', 17 )
                           .arg( tileMatrixSetExtent.yMinimum(), 0, 'g', 17 )
                           .arg( tileMatrixSetExtent.xMaximum(), 0, 'g', 17 )
                           .arg( tileMatrixSetExtent.yMaximum(), 0, 'g', 17 );

  result = mDatabase.exec( tmsSql, mLastError );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"Failed to populate GeoPackage gpkg_tile_matrix_set: "_s + mLastError );
    return false;
  }


  const double tmsWidth = tileMatrixSetExtent.width();
  const double tmsHeight = tileMatrixSetExtent.height();

  for ( int z = minZoom; z <= maxZoom; ++z )
  {
    const long long zoomFactor = 1LL << z;
    const long long matrixWidth = z0matrix.matrixWidth() * zoomFactor;
    const long long matrixHeight = z0matrix.matrixHeight() * zoomFactor;

    const double pixelXSize = tmsWidth / ( matrixWidth * tileWidth );
    const double pixelYSize = tmsHeight / ( matrixHeight * tileHeight );

    const QString matrixSql = QString( "INSERT INTO gpkg_tile_matrix VALUES ('tiles', %1, %2, %3, %4, %5, %6, %7);" )
                                .arg( z )
                                .arg( matrixWidth )
                                .arg( matrixHeight )
                                .arg( tileWidth )
                                .arg( tileHeight )
                                .arg( pixelXSize, 0, 'g', 17 )
                                .arg( pixelYSize, 0, 'g', 17 );

    result = mDatabase.exec( matrixSql, mLastError );
    if ( result != SQLITE_OK )
    {
      QgsDebugError( u"Failed to populate GeoPackage gpkg_tile_matrix for zoom %1: %2"_s.arg( z ).arg( mLastError ) );
      return false;
    }
  }

  result = mDatabase.exec( u"COMMIT TRANSACTION;"_s, mLastError );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"Failed to commit transaction: %1"_s.arg( mLastError ) );
  }

  // index creation is deferred to finalize()

  return true;
}

void QgsGeoPackageTiles::setTileData( const QList<TileData> &tiles ) const
{
  if ( tiles.isEmpty() )
  {
    return;
  }

  if ( !mDatabase )
  {
    QgsDebugError( u"GeoPackage database not open: "_s + mFilename );
    return;
  }

  int result = mDatabase.exec( u"BEGIN TRANSACTION;"_s, mLastError );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"Failed to begin transaction: %1"_s.arg( mLastError ) );
    return;
  }

  const QString sql = u"INSERT OR REPLACE INTO tiles (zoom_level, tile_column, tile_row, tile_data) VALUES (?, ?, ?, ?)"_s;
  sqlite3_statement_unique_ptr preparedStatement = mDatabase.prepare( sql, result );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"GeoPackage failed to prepare statement: %1"_s.arg( sql ) );
    mLastError = mDatabase.errorMessage();
    mDatabase.exec( u"ROLLBACK TRANSACTION;"_s, mLastError );
    return;
  }

  for ( const TileData &tile : tiles )
  {
    sqlite3_reset( preparedStatement.get() );
    sqlite3_clear_bindings( preparedStatement.get() );

    sqlite3_bind_int( preparedStatement.get(), 1, tile.z );
    sqlite3_bind_int( preparedStatement.get(), 2, tile.x );
    sqlite3_bind_int( preparedStatement.get(), 3, tile.y );
    sqlite3_bind_blob( preparedStatement.get(), 4, tile.data.constData(), tile.data.size(), SQLITE_TRANSIENT );

    if ( preparedStatement.step() != SQLITE_DONE )
    {
      mLastError = mDatabase.errorMessage();
      QgsDebugError( u"GeoPackage tile failed to be set: %1,%2,%3"_s.arg( tile.z ).arg( tile.x ).arg( tile.y ) );
    }
  }

  result = mDatabase.exec( u"COMMIT TRANSACTION;"_s, mLastError );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"Failed to commit transaction: %1"_s.arg( mLastError ) );
  }
}

bool QgsGeoPackageTiles::close()
{
  mDatabase.reset();
  return true;
}
