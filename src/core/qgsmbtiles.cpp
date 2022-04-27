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

  const sqlite3_database_unique_ptr database;
  const int result = mDatabase.open_v2( mFilename, SQLITE_OPEN_READONLY, nullptr );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Can't open MBTiles database: %1" ).arg( database.errorMessage() ) );
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
    QgsDebugMsg( QStringLiteral( "Can't create MBTiles database: %1" ).arg( database.errorMessage() ) );
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
    QgsDebugMsg( QStringLiteral( "Failed to initialize MBTiles database: " ) + errorMessage );
    return false;
  }

  return true;
}

QString QgsMbTiles::metadataValue( const QString &key ) const
{
  if ( !mDatabase )
  {
    QgsDebugMsg( QStringLiteral( "MBTiles database not open: " ) + mFilename );
    return QString();
  }

  int result;
  const QString sql = QStringLiteral( "select value from metadata where name='%1'" ).arg( key );
  sqlite3_statement_unique_ptr preparedStatement = mDatabase.prepare( sql, result );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "MBTile failed to prepare statement: " ) + sql );
    return QString();
  }

  if ( preparedStatement.step() != SQLITE_ROW )
  {
    QgsDebugMsg( QStringLiteral( "MBTile metadata value not found: " ) + key );
    return QString();
  }

  return preparedStatement.columnAsText( 0 );
}

void QgsMbTiles::setMetadataValue( const QString &key, const QString &value ) const
{
  if ( !mDatabase )
  {
    QgsDebugMsg( QStringLiteral( "MBTiles database not open: " ) + mFilename );
    return;
  }

  int result;
  const QString sql = QStringLiteral( "insert into metadata values (%1, %2)" ).arg( QgsSqliteUtils::quotedValue( key ), QgsSqliteUtils::quotedValue( value ) );
  sqlite3_statement_unique_ptr preparedStatement = mDatabase.prepare( sql, result );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "MBTile failed to prepare statement: " ) + sql );
    return;
  }

  if ( preparedStatement.step() != SQLITE_DONE )
  {
    QgsDebugMsg( QStringLiteral( "MBTile metadata value failed to be set: " ) + key );
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
    QgsDebugMsg( QStringLiteral( "MBTiles database not open: " ) + mFilename );
    return QByteArray();
  }

  int result;
  const QString sql = QStringLiteral( "select tile_data from tiles where zoom_level=%1 and tile_column=%2 and tile_row=%3" ).arg( z ).arg( x ).arg( y );
  sqlite3_statement_unique_ptr preparedStatement = mDatabase.prepare( sql, result );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "MBTile failed to prepare statement: " ) + sql );
    return QByteArray();
  }

  if ( preparedStatement.step() != SQLITE_ROW )
  {
    // this is not entirely unexpected -- user may have just requested a tile outside of the extent of the mbtiles package
    QgsDebugMsgLevel( QStringLiteral( "MBTile not found: z=%1 x=%2 y=%3" ).arg( z ).arg( x ).arg( y ), 2 );
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
    QgsDebugMsg( QStringLiteral( "MBTile data failed to load: z=%1 x=%2 y=%3" ).arg( z ).arg( x ).arg( y ) );
    return QImage();
  }
  return tileImage;
}

void QgsMbTiles::setTileData( int z, int x, int y, const QByteArray &data ) const
{
  if ( !mDatabase )
  {
    QgsDebugMsg( QStringLiteral( "MBTiles database not open: " ) + mFilename );
    return;
  }

  int result;
  const QString sql = QStringLiteral( "insert into tiles values (%1, %2, %3, ?)" ).arg( z ).arg( x ).arg( y );
  sqlite3_statement_unique_ptr preparedStatement = mDatabase.prepare( sql, result );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "MBTile failed to prepare statement: " ) + sql );
    return;
  }

  sqlite3_bind_blob( preparedStatement.get(), 1, data.constData(), data.size(), SQLITE_TRANSIENT );

  if ( preparedStatement.step() != SQLITE_DONE )
  {
    QgsDebugMsg( QStringLiteral( "MBTile tile failed to be set: %1,%2,%3" ).arg( z ).arg( x ).arg( y ) );
    return;
  }
}
