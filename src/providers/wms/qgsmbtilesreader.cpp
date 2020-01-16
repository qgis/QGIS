#include "qgsmbtilesreader.h"

#include "qgslogger.h"
#include "qgsrectangle.h"

#include <QImage>


QgsMBTilesReader::QgsMBTilesReader( const QString &filename )
  : mFilename( filename )
{
}

bool QgsMBTilesReader::open()
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

bool QgsMBTilesReader::isOpen() const
{
  return bool( mDatabase );
}

QString QgsMBTilesReader::metadataValue( const QString &key )
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

QgsRectangle QgsMBTilesReader::extent()
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

QByteArray QgsMBTilesReader::tileData( int z, int x, int y )
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

QImage QgsMBTilesReader::tileDataAsImage( int z, int x, int y )
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
