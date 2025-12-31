/***************************************************************************
  qgsmbtiles.cpp
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

#include "qgsmbtiles.h"

#include "qgslogger.h"
#include "qgsrectangle.h"

#include <QFile>
#include <QImage>

QgsMbTiles::QgsMbTiles( const QString &filename )
  : mFilename( filename )
{
}

bool QgsMbTiles::open()
{
  if ( mDatabase )
    return true;  // already opened

  if ( mFilename.isEmpty() )
    return false;

  const sqlite3_database_unique_ptr database;
  const int result = mDatabase.open_v2( mFilename, SQLITE_OPEN_READONLY, nullptr );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"Can't open MBTiles database: %1"_s.arg( database.errorMessage() ) );
    return false;
  }
  return true;
}

bool QgsMbTiles::isOpen() const
{
  return bool( mDatabase );
}

bool QgsMbTiles::create()
{
  if ( mDatabase )
    return false;

  if ( QFile::exists( mFilename ) )
    return false;

  const sqlite3_database_unique_ptr database;
  int result = mDatabase.open_v2( mFilename, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"Can't create MBTiles database: %1"_s.arg( database.errorMessage() ) );
    return false;
  }

  const QString sql = \
                      "CREATE TABLE metadata (name text, value text);" \
                      "CREATE TABLE tiles (zoom_level integer, tile_column integer, tile_row integer, tile_data blob);" \
                      "CREATE UNIQUE INDEX tile_index on tiles (zoom_level, tile_column, tile_row);";
  QString errorMessage;
  result = mDatabase.exec( sql, errorMessage );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"Failed to initialize MBTiles database: "_s + errorMessage );
    return false;
  }

  return true;
}

QString QgsMbTiles::metadataValue( const QString &key ) const
{
  if ( !mDatabase )
  {
    QgsDebugError( u"MBTiles database not open: "_s + mFilename );
    return QString();
  }

  int result;
  const QString sql = u"select value from metadata where name='%1'"_s.arg( key );
  sqlite3_statement_unique_ptr preparedStatement = mDatabase.prepare( sql, result );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"MBTile failed to prepare statement: "_s + sql );
    return QString();
  }

  if ( preparedStatement.step() != SQLITE_ROW )
  {
    QgsDebugError( u"MBTile metadata value not found: "_s + key );
    return QString();
  }

  return preparedStatement.columnAsText( 0 );
}

void QgsMbTiles::setMetadataValue( const QString &key, const QString &value ) const
{
  if ( !mDatabase )
  {
    QgsDebugError( u"MBTiles database not open: "_s + mFilename );
    return;
  }

  int result;
  const QString sql = u"insert into metadata values (%1, %2)"_s.arg( QgsSqliteUtils::quotedValue( key ), QgsSqliteUtils::quotedValue( value ) );
  sqlite3_statement_unique_ptr preparedStatement = mDatabase.prepare( sql, result );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"MBTile failed to prepare statement: "_s + sql );
    return;
  }

  if ( preparedStatement.step() != SQLITE_DONE )
  {
    QgsDebugError( u"MBTile metadata value failed to be set: "_s + key );
    return;
  }
}

QgsRectangle QgsMbTiles::extent() const
{
  const QString boundsStr = metadataValue( "bounds" );
  if ( boundsStr.isEmpty() )
    return QgsRectangle();
  QStringList boundsArray = boundsStr.split( ',' );
  if ( boundsArray.count() != 4 )
    return QgsRectangle();

  return QgsRectangle( boundsArray[0].toDouble(), boundsArray[1].toDouble(),
                       boundsArray[2].toDouble(), boundsArray[3].toDouble() );
}

QByteArray QgsMbTiles::tileData( int z, int x, int y ) const
{
  if ( !mDatabase )
  {
    QgsDebugError( u"MBTiles database not open: "_s + mFilename );
    return QByteArray();
  }

  int result;
  const QString sql = u"select tile_data from tiles where zoom_level=%1 and tile_column=%2 and tile_row=%3"_s.arg( z ).arg( x ).arg( y );
  sqlite3_statement_unique_ptr preparedStatement = mDatabase.prepare( sql, result );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"MBTile failed to prepare statement: "_s + sql );
    return QByteArray();
  }

  if ( preparedStatement.step() != SQLITE_ROW )
  {
    // this is not entirely unexpected -- user may have just requested a tile outside of the extent of the mbtiles package
    QgsDebugMsgLevel( u"MBTile not found: z=%1 x=%2 y=%3"_s.arg( z ).arg( x ).arg( y ), 2 );
    return QByteArray();
  }

  return preparedStatement.columnAsBlob( 0 );
}

QImage QgsMbTiles::tileDataAsImage( int z, int x, int y ) const
{
  QImage tileImage;
  const QByteArray tileBlob = tileData( z, x, y );
  if ( !tileImage.loadFromData( tileBlob ) )
  {
    QgsDebugError( u"MBTile data failed to load: z=%1 x=%2 y=%3"_s.arg( z ).arg( x ).arg( y ) );
    return QImage();
  }
  return tileImage;
}

void QgsMbTiles::setTileData( int z, int x, int y, const QByteArray &data ) const
{
  if ( !mDatabase )
  {
    QgsDebugError( u"MBTiles database not open: "_s + mFilename );
    return;
  }

  int result;
  const QString sql = u"insert into tiles values (%1, %2, %3, ?)"_s.arg( z ).arg( x ).arg( y );
  sqlite3_statement_unique_ptr preparedStatement = mDatabase.prepare( sql, result );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( u"MBTile failed to prepare statement: "_s + sql );
    return;
  }

  sqlite3_bind_blob( preparedStatement.get(), 1, data.constData(), data.size(), SQLITE_TRANSIENT );

  if ( preparedStatement.step() != SQLITE_DONE )
  {
    QgsDebugError( u"MBTile tile failed to be set: %1,%2,%3"_s.arg( z ).arg( x ).arg( y ) );
    return;
  }
}
