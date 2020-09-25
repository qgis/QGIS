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

#include <zlib.h>


QgsMbTiles::QgsMbTiles( const QString &filename )
  : mFilename( filename )
{
}

bool QgsMbTiles::open()
{
  if ( mDatabase )
    return true;  // already opened

  sqlite3_database_unique_ptr database;
  int result = mDatabase.open_v2( mFilename, SQLITE_OPEN_READONLY, nullptr );
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

  sqlite3_database_unique_ptr database;
  int result = mDatabase.open_v2( mFilename, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Can't create MBTiles database: %1" ).arg( database.errorMessage() ) );
    return false;
  }

  QString sql = \
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

QString QgsMbTiles::metadataValue( const QString &key )
{
  if ( !mDatabase )
  {
    QgsDebugMsg( QStringLiteral( "MBTiles database not open: " ) + mFilename );
    return QString();
  }

  int result;
  QString sql = QStringLiteral( "select value from metadata where name='%1'" ).arg( key );
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

void QgsMbTiles::setMetadataValue( const QString &key, const QString &value )
{
  if ( !mDatabase )
  {
    QgsDebugMsg( QStringLiteral( "MBTiles database not open: " ) + mFilename );
    return;
  }

  int result;
  QString sql = QStringLiteral( "insert into metadata values (%1, %2)" ).arg( QgsSqliteUtils::quotedValue( key ), QgsSqliteUtils::quotedValue( value ) );
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

QgsRectangle QgsMbTiles::extent()
{
  QString boundsStr = metadataValue( "bounds" );
  if ( boundsStr.isEmpty() )
    return QgsRectangle();
  QStringList boundsArray = boundsStr.split( ',' );
  if ( boundsArray.count() != 4 )
    return QgsRectangle();

  return QgsRectangle( boundsArray[0].toDouble(), boundsArray[1].toDouble(),
                       boundsArray[2].toDouble(), boundsArray[3].toDouble() );
}

QByteArray QgsMbTiles::tileData( int z, int x, int y )
{
  if ( !mDatabase )
  {
    QgsDebugMsg( QStringLiteral( "MBTiles database not open: " ) + mFilename );
    return QByteArray();
  }

  int result;
  QString sql = QStringLiteral( "select tile_data from tiles where zoom_level=%1 and tile_column=%2 and tile_row=%3" ).arg( z ).arg( x ).arg( y );
  sqlite3_statement_unique_ptr preparedStatement = mDatabase.prepare( sql, result );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "MBTile failed to prepare statement: " ) + sql );
    return QByteArray();
  }

  if ( preparedStatement.step() != SQLITE_ROW )
  {
    QgsDebugMsg( QStringLiteral( "MBTile not found: z=%1 x=%2 y=%3" ).arg( z ).arg( x ).arg( y ) );
    return QByteArray();
  }

  return preparedStatement.columnAsBlob( 0 );
}

QImage QgsMbTiles::tileDataAsImage( int z, int x, int y )
{
  QImage tileImage;
  QByteArray tileBlob = tileData( z, x, y );
  if ( !tileImage.loadFromData( tileBlob ) )
  {
    QgsDebugMsg( QStringLiteral( "MBTile data failed to load: z=%1 x=%2 y=%3" ).arg( z ).arg( x ).arg( y ) );
    return QImage();
  }
  return tileImage;
}

void QgsMbTiles::setTileData( int z, int x, int y, const QByteArray &data )
{
  if ( !mDatabase )
  {
    QgsDebugMsg( QStringLiteral( "MBTiles database not open: " ) + mFilename );
    return;
  }

  int result;
  QString sql = QStringLiteral( "insert into tiles values (%1, %2, %3, ?)" ).arg( z ).arg( x ).arg( y );
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

bool QgsMbTiles::decodeGzip( const QByteArray &bytesIn, QByteArray &bytesOut )
{
  unsigned char *bytesInPtr = reinterpret_cast<unsigned char *>( const_cast<char *>( bytesIn.constData() ) );
  uint bytesInLeft = static_cast<uint>( bytesIn.count() );

  const uint CHUNK = 16384;
  unsigned char out[CHUNK];
  const int DEC_MAGIC_NUM_FOR_GZIP = 16;

  // allocate inflate state
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;

  int ret = inflateInit2( &strm, MAX_WBITS + DEC_MAGIC_NUM_FOR_GZIP );
  if ( ret != Z_OK )
    return false;

  while ( ret != Z_STREAM_END ) // done when inflate() says it's done
  {
    // prepare next chunk
    uint bytesToProcess = std::min( CHUNK, bytesInLeft );
    strm.next_in = bytesInPtr;
    strm.avail_in = bytesToProcess;
    bytesInPtr += bytesToProcess;
    bytesInLeft -= bytesToProcess;

    if ( bytesToProcess == 0 )
      break;  // we end with an error - no more data but inflate() wants more data

    // run inflate() on input until output buffer not full
    do
    {
      strm.avail_out = CHUNK;
      strm.next_out = out;
      ret = inflate( &strm, Z_NO_FLUSH );
      Q_ASSERT( ret != Z_STREAM_ERROR ); // state not clobbered
      if ( ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR )
      {
        inflateEnd( &strm );
        return false;
      }
      unsigned have = CHUNK - strm.avail_out;
      bytesOut.append( QByteArray::fromRawData( reinterpret_cast<const char *>( out ), static_cast<int>( have ) ) );
    }
    while ( strm.avail_out == 0 );
  }

  inflateEnd( &strm );
  return ret == Z_STREAM_END;
}


bool QgsMbTiles::encodeGzip( const QByteArray &bytesIn, QByteArray &bytesOut )
{
  unsigned char *bytesInPtr = reinterpret_cast<unsigned char *>( const_cast<char *>( bytesIn.constData() ) );
  uint bytesInLeft = static_cast<uint>( bytesIn.count() );

  const uint CHUNK = 16384;
  unsigned char out[CHUNK];
  const int DEC_MAGIC_NUM_FOR_GZIP = 16;

  // allocate deflate state
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;

  int ret = deflateInit2( &strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + DEC_MAGIC_NUM_FOR_GZIP, 8, Z_DEFAULT_STRATEGY );
  if ( ret != Z_OK )
    return false;

  strm.avail_in = bytesInLeft;
  strm.next_in = bytesInPtr;

  // run deflate() on input until output buffer not full, finish
  // compression if all of source has been read in
  do
  {
    strm.avail_out = CHUNK;
    strm.next_out = out;
    ret = deflate( &strm, Z_FINISH );  // no bad return value
    Q_ASSERT( ret != Z_STREAM_ERROR ); // state not clobbered

    unsigned have = CHUNK - strm.avail_out;
    bytesOut.append( QByteArray::fromRawData( reinterpret_cast<const char *>( out ), static_cast<int>( have ) ) );
  }
  while ( strm.avail_out == 0 );
  Q_ASSERT( ret == Z_STREAM_END );      // stream will be complete

  // clean up and return
  deflateEnd( &strm );
  return true;
}
