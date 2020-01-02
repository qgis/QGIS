/***************************************************************************
  qgspostgresrasterprovider.cpp - QgsPostgresRasterProvider

 ---------------------
 begin                : 20.12.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cstring>
#include "qgspostgresrasterprovider.h"
#include "qgspostgrestransaction.h"
#include "qgsmessagelog.h"
#include "qgsrectangle.h"
#include "qgspolygon.h"

const QString QgsPostgresRasterProvider::PG_RASTER_PROVIDER_KEY = QStringLiteral( "postgresraster" );
const QString QgsPostgresRasterProvider::PG_RASTER_PROVIDER_DESCRIPTION =  QStringLiteral( "Postgres raster provider" );


QgsPostgresRasterProvider::QgsPostgresRasterProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions )
  : QgsRasterDataProvider( uri, providerOptions )
{
  mUri = QgsDataSourceUri( uri );

  // populate members from the uri structure
  mSchemaName = mUri.schema();
  mTableName = mUri.table();

  mRasterColumn = mUri.geometryColumn();
  if ( mRasterColumn.isEmpty() )
  {
    mRasterColumn = QStringLiteral( "rast" );
  }
  mSqlWhereClause = mUri.sql();
  mRequestedSrid = mUri.srid();

  /* do not support queries for now
  if ( mSchemaName.isEmpty() && mTableName.startsWith( '(' ) && mTableName.endsWith( ')' ) )
  {
    mIsQuery = true;
    mQuery = mTableName;
    mTableName.clear();
  }
  else
  */
  {
    mIsQuery = false;

    if ( !mSchemaName.isEmpty() )
    {
      mQuery += quotedIdentifier( mSchemaName ) + '.';
    }

    if ( !mTableName.isEmpty() )
    {
      mQuery += quotedIdentifier( mTableName );
    }
  }

  // TODO: for now always true
  // mUseEstimatedMetadata = mUri.useEstimatedMetadata();

  QgsDebugMsg( QStringLiteral( "Connection info is %1" ).arg( mUri.connectionInfo( false ) ) );
  QgsDebugMsg( QStringLiteral( "Schema is: %1" ).arg( mSchemaName ) );
  QgsDebugMsg( QStringLiteral( "Table name is: %1" ).arg( mTableName ) );
  QgsDebugMsg( QStringLiteral( "Query is: %1" ).arg( mQuery ) );
  QgsDebugMsg( QStringLiteral( "Where clause is: %1" ).arg( mSqlWhereClause ) );

  // no table/query passed, the provider could be used to get tables
  if ( mQuery.isEmpty() )
  {
    return;
  }

  mConnectionRO = QgsPostgresConn::connectDb( mUri.connectionInfo( false ), true );
  if ( !mConnectionRO )
  {
    return;
  }

  if ( !hasSufficientPermsAndCapabilities() ) // check permissions and set capabilities
  {
    disconnectDb();
    return;
  }

  if ( !getDetails() ) // gets srid and data type
  {
    // the table is not a raster table
    QgsMessageLog::logMessage( tr( "Invalid PostgreSQL raster layer" ), tr( "PostGIS" ) );
    disconnectDb();
    return;
  }

  mValid = true;
}

QgsPostgresRasterProvider::QgsPostgresRasterProvider( const QgsPostgresRasterProvider &other, const QgsDataProvider::ProviderOptions &providerOptions )
  : QgsRasterDataProvider( other.dataSourceUri(), providerOptions )
  , mValid( other.mValid )
  , mCrs( other.mCrs )
  , mUri( other.mUri )
  , mIsQuery( other.mIsQuery )
  , mTableName( other.mTableName )
  , mQuery( other.mQuery )
  , mRasterColumn( other.mRasterColumn )
  , mSchemaName( other.mSchemaName )
  , mSqlWhereClause( other.mSqlWhereClause )
  , mExtent( other.mExtent )
  , mUseEstimatedMetadata( other.mUseEstimatedMetadata )
  , mDataTypes( other.mDataTypes )
  , mDataSizes( other.mDataSizes )
  , mNoDataValues( other.mNoDataValues )
  , mBandCount( other.mBandCount )
  , mIsTiled( other.mIsTiled )
  , mIsOutOfDb( other.mIsOutOfDb )
  , mHasSpatialIndex( other.mHasSpatialIndex )
  , mWidth( other.mWidth )
  , mHeight( other.mHeight )
  , mScaleX( other.mScaleX )
  , mScaleY( other.mScaleY )
  , mConnectionRO( other.mConnectionRO )
  , mConnectionRW( other.mConnectionRW )
{
}

QgsPostgresRasterProvider::~QgsPostgresRasterProvider()
{

}

bool QgsPostgresRasterProvider::hasSufficientPermsAndCapabilities()
{
  QgsDebugMsg( QStringLiteral( "Checking for permissions on the relation" ) );

  QgsPostgresResult testAccess;
  if ( !mIsQuery )
  {
    // Check that we can read from the table (i.e., we have select permission).
    QString sql = QStringLiteral( "SELECT * FROM %1 LIMIT 1" ).arg( mQuery );
    QgsPostgresResult testAccess( connectionRO()->PQexec( sql ) );
    if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Unable to access the %1 relation.\nThe error message from the database was:\n%2.\nSQL: %3" )
                                 .arg( mQuery,
                                       testAccess.PQresultErrorMessage(),
                                       sql ), tr( "PostGIS" ) );
      return false;
    }

    bool inRecovery = false;

    if ( connectionRO()->pgVersion() >= 90000 )
    {
      testAccess = connectionRO()->PQexec( QStringLiteral( "SELECT pg_is_in_recovery()" ) );
      if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK || testAccess.PQgetvalue( 0, 0 ) == QLatin1String( "t" ) )
      {
        QgsMessageLog::logMessage( tr( "PostgreSQL is still in recovery after a database crash\n(or you are connected to a (read-only) slave).\nWrite accesses will be denied." ), tr( "PostGIS" ) );
        inRecovery = true;
      }
    }
  }
  return true;
}

QgsCoordinateReferenceSystem QgsPostgresRasterProvider::crs() const
{
  return mCrs;
}

QgsRectangle QgsPostgresRasterProvider::extent() const
{
  return mExtent;
}

bool QgsPostgresRasterProvider::isValid() const
{
  return mValid;
}

QString QgsPostgresRasterProvider::name() const
{
  return QgsPostgresRasterProvider::PG_RASTER_PROVIDER_KEY;
}

QString QgsPostgresRasterProvider::description() const
{
  return QgsPostgresRasterProvider::PG_RASTER_PROVIDER_DESCRIPTION;
}

bool QgsPostgresRasterProvider::readBlock( int bandNo, const QgsRectangle &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback * )
{
  if ( bandNo > mBandCount )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Invalid band number '%1" ).arg( bandNo ), QStringLiteral( "PostGIS" ), Qgis::Warning );
    return false;
  }
  // Find overview
  const int minPixeSize { static_cast<int>( std::min( viewExtent.width() / width, viewExtent.height() / height ) ) };
  qDebug() << viewExtent.width() << viewExtent.height() << width << height;
  // Pixel size
  qDebug() << viewExtent.width() / width << viewExtent.height() / height << minPixeSize;
  //const int resolution { std::min( viewExtent.width(), viewExtent.height() ) };
  // Fetch data from backend
  QString sql;
  const bool isSingleValue {  width == 1 && height == 1 };
  if ( isSingleValue )
  {
    sql = QStringLiteral( "SELECT ST_Value( ST_Band( %1, %2), ST_GeomFromText( %3, %4 ), FALSE ) FROM %5" )
          .arg( quotedIdentifier( mRasterColumn ) )
          .arg( bandNo )
          .arg( quotedValue( viewExtent.center().asWkt() ) )
          .arg( mCrs.postgisSrid() )
          .arg( mQuery );
  }
  else
  {
    // TODO: resample if width and height are different from x/y size
    sql = QStringLiteral( "SELECT ST_AsBinary( ST_Resize( ST_Clip ( ST_Band( %1, %2 ), 1, ST_GeomFromText( %4, %5 ), %6 ), %8, %9 ) ) FROM %3 "
                          "WHERE ST_Intersects( %1,  ST_GeomFromText( %4, %5 ) )" )
          .arg( quotedIdentifier( mRasterColumn ) )
          .arg( bandNo )
          .arg( mQuery )
          .arg( quotedValue( viewExtent.asWktPolygon() ) )
          .arg( mCrs.postgisSrid() )
          .arg( mNoDataValues[ static_cast<unsigned long>( bandNo - 1 ) ] )
          .arg( width )
          .arg( height );
  }
  QgsDebugMsg( QStringLiteral( "Reading raster block: %1" ).arg( sql ) );
  QgsPostgresResult result( connectionRO()->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage( tr( "Unable to access the %1 relation.\nThe error message from the database was:\n%2.\nSQL: %3" )
                               .arg( mQuery,
                                     result.PQresultErrorMessage(),
                                     sql ), tr( "PostGIS" ) );
    return false;
  }

  if ( isSingleValue )
  {
    bool ok;
    const QString val { result.PQgetvalue( 0, 0 ) };
    const Qgis::DataType dataType { mDataTypes[ bandNo - 1 ] };
    {
      if ( dataType == Qgis::DataType::Byte )
      {
        const unsigned short byte { val.toUShort( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( QStringLiteral( "Cannot convert value to byte" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
          return false;
        }
        std::memcpy( data, &byte, sizeof( unsigned short ) );
      }
      else if ( dataType == Qgis::DataType::UInt16 )
      {
        const unsigned int uint { val.toUInt( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( QStringLiteral( "Cannot convert value to unsigned int" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
          return false;
        }
        std::memcpy( data, &uint, sizeof( unsigned int ) );
      }
      else if ( dataType == Qgis::DataType::UInt32 )
      {
        const unsigned long ulong { val.toULong( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( QStringLiteral( "Cannot convert value to unsigned long" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
          return false;
        }
        std::memcpy( data, &ulong, sizeof( unsigned long ) );
      }
      else if ( dataType == Qgis::DataType::Int16 )
      {
        const int _int { val.toInt( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( QStringLiteral( "Cannot convert value to unsigned long" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
          return false;
        }
        std::memcpy( data, &_int, sizeof( int ) );
      }
      else if ( dataType == Qgis::DataType::Int32 )
      {
        const long _long { val.toInt( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( QStringLiteral( "Cannot convert value to long" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
          return false;
        }
        std::memcpy( data, &_long, sizeof( long ) );
      }
      else if ( dataType == Qgis::DataType::Float32 )
      {
        const float _float { val.toFloat( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( QStringLiteral( "Cannot convert value to float" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
          return false;
        }
        std::memcpy( data, &_float, sizeof( float ) );
      }
      else if ( dataType == Qgis::DataType::Float64 )
      {
        const double _double { val.toDouble( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( QStringLiteral( "Cannot convert value to unsigned double" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
          return false;
        }
        std::memcpy( data, &_double, sizeof( double ) );
      }
      else
      {
        QgsMessageLog::logMessage( QStringLiteral( "Unknown data type" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
        return false;
      }
    }
  }
  else
  {
    const QVariantMap wkbProperties { parseWkb( result.PQgetvalue( 0, 0 ).toAscii() ) };
    if ( wkbProperties.isEmpty() )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error parsing raster WKB" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
      return false;
    }
    const QByteArray rasterData { wkbProperties[ QStringLiteral( "data" )].toByteArray() };
    std::memcpy( data, rasterData.constData(), static_cast<size_t>( rasterData.size() ) );
  }
  return true;
}


QgsPostgresRasterProviderMetadata::QgsPostgresRasterProviderMetadata()
  : QgsProviderMetadata( QgsPostgresRasterProvider::PG_RASTER_PROVIDER_KEY, QgsPostgresRasterProvider::PG_RASTER_PROVIDER_DESCRIPTION )
{

}

QVariantMap QgsPostgresRasterProviderMetadata::decodeUri( const QString &uri )
{
  const QgsDataSourceUri dsUri { uri };
  return
  {
    { QStringLiteral( "dbname" ), dsUri.database() },
    { QStringLiteral( "host" ), dsUri.host() },
    { QStringLiteral( "port" ), dsUri.port() },
    { QStringLiteral( "service" ), dsUri.service() },
    { QStringLiteral( "username" ), dsUri.username() },
    { QStringLiteral( "password" ), dsUri.password() },
    { QStringLiteral( "authcfg" ), dsUri.authConfigId() },
    { QStringLiteral( "type" ), dsUri.wkbType() },
    { QStringLiteral( "selectatid" ), dsUri.selectAtIdDisabled() },
    { QStringLiteral( "table" ), dsUri.table() },
    { QStringLiteral( "schema" ), dsUri.schema() },
    { QStringLiteral( "key" ), dsUri.keyColumn() },
    { QStringLiteral( "srid" ), dsUri.srid() },
    { QStringLiteral( "estimatedmetadata" ), dsUri.useEstimatedMetadata() },
    { QStringLiteral( "sslmode" ), dsUri.sslMode() },
    { QStringLiteral( "sql" ), dsUri.sql() },
    { QStringLiteral( "geometrycolumn" ), dsUri.geometryColumn() },
  };
}

QgsPostgresRasterProvider *QgsPostgresRasterProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options )
{
  return new QgsPostgresRasterProvider( uri, options );
}


Qgis::DataType QgsPostgresRasterProvider::dataType( int bandNo ) const
{
  if ( mDataTypes.size() < bandNo )
  {
    // TODO: raise or at least log!
    return Qgis::DataType::UnknownDataType;
  }
  // Band is 1-based
  return mDataTypes[ bandNo - 1 ];
}

int QgsPostgresRasterProvider::bandCount() const
{
  return mBandCount;
}

QgsRasterInterface *QgsPostgresRasterProvider::clone() const
{
  QgsDataProvider::ProviderOptions options;
  options.transformContext = transformContext();
  QgsPostgresRasterProvider *provider = new QgsPostgresRasterProvider( *this, options );
  provider->copyBaseSettings( *this );
  return provider;
}

QString QgsPostgresRasterProvider::htmlMetadata()
{
  return QString( "Metadata TODO" );
}

QString QgsPostgresRasterProvider::lastErrorTitle()
{
  return mErrorTitle;
}

QString QgsPostgresRasterProvider::lastError()
{
  return mError;
}

int QgsPostgresRasterProvider::capabilities() const
{
  const int capability = QgsRasterDataProvider::Identify
                         | QgsRasterDataProvider::IdentifyValue
                         | QgsRasterDataProvider::Size
                         | QgsRasterDataProvider::BuildPyramids
                         | QgsRasterDataProvider::Create
                         | QgsRasterDataProvider::Remove;
  return capability;
}

QVariantMap QgsPostgresRasterProvider::parseWkb( const QByteArray &wkb )
{
  QVariantMap result;
  const int minWkbSize { 61 * 2 + 2 };
  if ( wkb.size() < minWkbSize )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Wrong wkb size: min expected = %1, actual = %2" )
                               .arg( minWkbSize )
                               .arg( wkb.size() ), QStringLiteral( "PostGIS" ), Qgis::Warning );

    return result;
  }
  // Mutable and skip \x
  const QByteArray wkbBytes { QByteArray::fromHex( wkb.mid( 2 ) ) };
  /*
  { name: 'endianness', byteOffset: 0,  byteLength: 1, type: 'Uint8' },
  { name: 'version',    byteOffset: 1,  byteLength: 2, type: 'Uint16' },
  { name: 'nBands',     byteOffset: 3,  byteLength: 2, type: 'Uint16' },
  { name: 'scaleX',     byteOffset: 5,  byteLength: 8, type: 'Float64' },
  { name: 'scaleY',     byteOffset: 13, byteLength: 8, type: 'Float64' },
  { name: 'ipX',        byteOffset: 21, byteLength: 8, type: 'Float64' },
  { name: 'ipY',        byteOffset: 29, byteLength: 8, type: 'Float64' },
  { name: 'skewX',      byteOffset: 37, byteLength: 8, type: 'Float64' },
  { name: 'skewY',      byteOffset: 45, byteLength: 8, type: 'Float64' },
  { name: 'srid',       byteOffset: 53, byteLength: 4, type: 'Int32' },
  { name: 'width',      byteOffset: 57, byteLength: 2, type: 'Uint16' },
  { name: 'height',     byteOffset: 59, byteLength: 2, type: 'Uint16' },
  */
  // Endianness
  result[ QStringLiteral( "endiannes" ) ] = static_cast<unsigned short int>( wkbBytes[0] );
  // NOTE: For now only little endian is supported
  Q_ASSERT( result[ QStringLiteral( "endiannes" ) ] == 1 );
  result[ QStringLiteral( "version" ) ] = *reinterpret_cast<const unsigned short int *>( &wkbBytes.constData()[1] );
  result[ QStringLiteral( "nBands" ) ] = *reinterpret_cast<const unsigned int *>( &wkbBytes.constData()[3] );
  result[ QStringLiteral( "scaleX" ) ] = *reinterpret_cast<const double *>( &wkbBytes.constData()[5] );
  result[ QStringLiteral( "scaleY" ) ] = *reinterpret_cast<const double *>( &wkbBytes.constData()[13] );
  result[ QStringLiteral( "ipX" ) ] = *reinterpret_cast<const double *>( &wkbBytes.constData()[21] );
  result[ QStringLiteral( "ipY" ) ] = *reinterpret_cast<const double *>( &wkbBytes.constData()[29] );
  result[ QStringLiteral( "skewX" ) ] = *reinterpret_cast<const double *>( &wkbBytes.constData()[37] );
  result[ QStringLiteral( "skewY" ) ] = *reinterpret_cast<const double *>( &wkbBytes.constData()[45] );
  result[ QStringLiteral( "srid" ) ] = static_cast<int>( *reinterpret_cast<const long int *>( &wkbBytes.constData()[53] ) );
  result[ QStringLiteral( "width" ) ] = *reinterpret_cast<const unsigned short int *>( &wkbBytes.constData()[57] );
  result[ QStringLiteral( "height" ) ] = *reinterpret_cast<const unsigned short int *>( &wkbBytes.constData()[59] );
  // Band header data starts at offset 61
  result[ QStringLiteral( "pxType" ) ] = *reinterpret_cast<const unsigned short int *>( &wkbBytes.constData()[61] ) & 0x0F;
  /*
  | 'Bool1'  // unsupported
  | 'Uint2'  // unsupported
  | 'Uint4'  // unsupported
  | 'Int8'
  | 'Uint8'
  | 'Int16'
  | 'Uint16'
  | 'Int32'
  | 'Uint32'
  | 'Float32'
  | 'Float64'
  */
  int pxSize; // in bytes
  switch ( result[ QStringLiteral( "pxType" ) ].toInt() )
  {
    case 4:  // int8
      pxSize = 1;
      result[ QStringLiteral( "nodata" ) ] = *reinterpret_cast<const short int *>( &wkbBytes.constData()[ 62 ] );
      break;
    case 5: // uint8
      result[ QStringLiteral( "nodata" ) ] = *reinterpret_cast<const unsigned short int *>( &wkbBytes.constData()[ 62 ] );
      pxSize = 1;
      break;
    case 6: // int16
      result[ QStringLiteral( "nodata" ) ] = *reinterpret_cast<const int *>( &wkbBytes.constData()[ 62 ] );
      pxSize = 2;
      break;
    case 7: // uint16
      result[ QStringLiteral( "nodata" ) ] = *reinterpret_cast<const unsigned int *>( &wkbBytes.constData()[ 62 ] );
      pxSize = 2;
      break;
    case 8: // int32
      result[ QStringLiteral( "nodata" ) ] = static_cast<long long>( *reinterpret_cast<const long int *>( &wkbBytes.constData()[ 62 ] ) );
      pxSize = 4;
      break;
    case 9: // uint32
      result[ QStringLiteral( "nodata" ) ] = static_cast<unsigned long long>( *reinterpret_cast<const unsigned long int *>( &wkbBytes.constData()[ 62 ] ) );
      pxSize = 4;
      break;
    case 10:
      result[ QStringLiteral( "nodata" ) ] = *reinterpret_cast<const float *>( &wkbBytes.constData()[ 62 ] );
      pxSize = 4;
      break;
    case 11:
      result[ QStringLiteral( "nodata" ) ] = *reinterpret_cast<const double *>( &wkbBytes.constData()[ 62 ] );
      pxSize = 8;
      break;
    default:
      QgsMessageLog::logMessage( QStringLiteral( "Unsupported pixel type: %1" )
                                 .arg( result[ QStringLiteral( "pxType" ) ].toInt() ), QStringLiteral( "PostGIS" ), Qgis::Critical );
      return QVariantMap();
  }
  result[ QStringLiteral( "data" )] = wkbBytes.mid( 61 + 1 + pxSize );
  return result;
}

QgsPostgresConn *QgsPostgresRasterProvider::connectionRO() const
{
  return mConnectionRO;
}

QgsPostgresConn *QgsPostgresRasterProvider::connectionRW()
{
  if ( !mConnectionRW )
  {
    mConnectionRW = QgsPostgresConn::connectDb( mUri.connectionInfo( false ), false );
  }
  return mConnectionRW;
}

void QgsPostgresRasterProvider::disconnectDb()
{
  if ( mConnectionRO )
  {
    mConnectionRO->unref();
    mConnectionRO = nullptr;
  }

  if ( mConnectionRW )
  {
    mConnectionRW->unref();
    mConnectionRW = nullptr;
  }
}

bool QgsPostgresRasterProvider::getDetails()
{

  // Utility to get data type from string
  auto pixelTypeFromString = [ ]( const QString & t ) -> Qgis::DataType
  {
    /* Pixel types
    1BB - 1-bit boolean
    2BUI - 2-bit unsigned integer
    4BUI - 4-bit unsigned integer
    8BSI - 8-bit signed integer
    8BUI - 8-bit unsigned integer
    16BSI - 16-bit signed integer
    16BUI - 16-bit unsigned integer
    32BSI - 32-bit signed integer
    32BUI - 32-bit unsigned integer
    32BF - 32-bit float
    64BF - 64-bit float
    */
    Qgis::DataType type { Qgis::DataType::UnknownDataType };
    if ( t == QStringLiteral( "8BUI" ) )
    {
      type = Qgis::DataType::Byte;
    }
    else if ( t == QStringLiteral( "16BUI" ) )
    {
      type = Qgis::DataType::UInt16;
    }
    else if ( t == QStringLiteral( "16BSI" ) )
    {
      type = Qgis::DataType::Int16;
    }
    else if ( t == QStringLiteral( "32BSI" ) )
    {
      type = Qgis::DataType::Int32;
    }
    else if ( t == QStringLiteral( "32BUI" ) )
    {
      type = Qgis::DataType::UInt32;
    }
    else if ( t == QStringLiteral( "32BF" ) )
    {
      type = Qgis::DataType::Float32;
    }
    else if ( t == QStringLiteral( "64BF" ) )
    {
      type = Qgis::DataType::Float64;
    }
    return type;
  };

  // Get information from metadata
  if ( ! mIsQuery && mUseEstimatedMetadata )
  {
    try
    {
      const QString sql { QStringLiteral( "SELECT r_raster_column, srid,"
                                          "num_bands, pixel_types, nodata_values, ST_AsBinary(extent), blocksize_x, blocksize_y,"
                                          "out_db, spatial_index, scale_x, scale_y, same_alignment,"
                                          "regular_blocking "
                                          "FROM raster_columns WHERE "
                                          "r_table_name = %1 AND r_table_schema = %2 AND r_table_catalog = %3" )
                          .arg( quotedValue( mTableName ) )
                          .arg( quotedValue( mSchemaName ) )
                          .arg( quotedValue( mUri.database() ) ) };

      QgsDebugMsg( QStringLiteral( "Raster information sql: %1" ).arg( sql ) );
      QgsPostgresResult result( connectionRO()->PQexec( sql ) );
      if ( ( PGRES_TUPLES_OK == result.PQresultStatus() ) && ( result.PQntuples() > 0 ) )
      {
        mRasterColumn = result.PQgetvalue( 0, 0 );
        mHasSpatialIndex = result.PQgetvalue( 0, 9 ) == 't';
        bool ok;
        mCrs = QgsCoordinateReferenceSystem::fromEpsgId( result.PQgetvalue( 0, 1 ).toLong( &ok ) );
        if ( ! ok )
        {
          throw QgsPostgresRasterProviderException( QStringLiteral( "Cannot create CRS from EPSG: '%1'" ).arg( result.PQgetvalue( 0, 1 ) ) );
        }
        mBandCount = result.PQgetvalue( 0, 2 ).toInt( &ok );
        if ( ! ok )
        {
          throw QgsPostgresRasterProviderException( QStringLiteral( "Cannot get band count from value: '%1'" ).arg( result.PQgetvalue( 0, 2 ) ) );
        }
        const QStringList pxTypes { result.PQgetvalue( 0, 3 ).chopped( 1 ).mid( 1 ).split( ',' ) };
        const QStringList noDataValues { result.PQgetvalue( 0, 4 ).chopped( 1 ).mid( 1 ).split( ',' ) };
        if ( mBandCount != pxTypes.count( ) || mBandCount != noDataValues.count() )
        {
          throw QgsPostgresRasterProviderException( QStringLiteral( "Band count and nodata items count differs" ) );
        }

        int i = 0;
        for ( const QString &t : qgis::as_const( pxTypes ) )
        {
          Qgis::DataType type { pixelTypeFromString( t ) };
          if ( t == Qgis::DataType::UnknownDataType )
          {
            throw QgsPostgresRasterProviderException( QStringLiteral( "Unsupported data type: '%1'" ).arg( t ) );
          }
          mDataTypes.push_back( type );
          mDataSizes.push_back( QgsRasterBlock::typeSize( type ) );
          double nodataValue { noDataValues.at( i ).toDouble( &ok ) };
          if ( ! ok )
          {
            QgsMessageLog::logMessage( QStringLiteral( "Cannot convert nodata value '%1' to double, default to: %2" )
                                       .arg( noDataValues.at( i ) )
                                       .arg( std::numeric_limits<double>::min() ), QStringLiteral( "PostGIS" ), Qgis::Info );
            nodataValue = std::numeric_limits<double>::min();
          }
          mNoDataValues.push_back( nodataValue );
          ++i;
        }
        // Extent
        QgsPolygon p;
        // Strip \x
        const QByteArray hexAscii { result.PQgetvalue( 0, 5 ).toAscii().mid( 2 ) };
        QgsConstWkbPtr ptr { QByteArray::fromHex( hexAscii ) };
        if ( ! p.fromWkb( ptr ) )
        {
          throw QgsPostgresRasterProviderException( QStringLiteral( "Cannot get extent from raster_columns" ) );
        }
        mExtent = p.boundingBox();
        // Size
        mWidth = result.PQgetvalue( 0, 6 ).toInt( &ok );
        if ( ! ok )
        {
          throw QgsPostgresRasterProviderException( QStringLiteral( "Cannot convert width '%1' to int" ).arg( result.PQgetvalue( 0, 6 ) ) );
        }
        mHeight = result.PQgetvalue( 0, 7 ).toInt( &ok );
        if ( ! ok )
        {
          throw QgsPostgresRasterProviderException( QStringLiteral( "Cannot convert height '%1' to int" ).arg( result.PQgetvalue( 0, 7 ) ) );
        }
        mIsOutOfDb = result.PQgetvalue( 0, 8 ) == 't';
        mScaleX = result.PQgetvalue( 0, 10 ).toInt( &ok );
        if ( ! ok )
        {
          throw QgsPostgresRasterProviderException( QStringLiteral( "Cannot convert scale X '%1' to int" ).arg( result.PQgetvalue( 0, 10 ) ) );
        }
        mScaleY = result.PQgetvalue( 0, 11 ).toInt( &ok );
        if ( ! ok )
        {
          throw QgsPostgresRasterProviderException( QStringLiteral( "Cannot convert scale Y '%1' to int" ).arg( result.PQgetvalue( 0, 11 ) ) );
        }
        return true;
      }
      else
      {
        QgsMessageLog::logMessage( QStringLiteral( "An error occurred while fetching raster metadata: %1" )
                                   .arg( result.PQresultErrorMessage() ),
                                   QStringLiteral( "PostGIS" ), Qgis::Warning );
      }
    }
    catch ( QgsPostgresRasterProviderException &ex )
    {
      QgsMessageLog::logMessage( QStringLiteral( "An error occurred while fetching raster metadata, proceeding with raster data analysis: %1" )
                                 .arg( ex.message ),
                                 QStringLiteral( "PostGIS" ), Qgis::Warning );
    }
  }

  // TODO: query layers + mHasSpatialInded in case metadata are not used
  // Go the hard and slow way: fetch information directly from the layer
  if ( mRasterColumn.isEmpty() )
  {
    const QString sql { QStringLiteral( "SELECT column_name FROM information_schema.columns WHERE "
                                        "table_name = %1 AND table_schema = %2 AND udt_name = 'raster'" )
                        .arg( quotedValue( mTableName ) )
                        .arg( quotedValue( mSchemaName ) )};
    QgsPostgresResult result( connectionRO()->PQexec( sql ) );
    if ( PGRES_TUPLES_OK == result.PQresultStatus() && result.PQntuples() > 0 )
    {
      if ( result.PQntuples() > 1 )
      {
        QgsMessageLog::logMessage( QStringLiteral( "Multiple raster column detected, using the first one" ),
                                   QStringLiteral( "PostGIS" ), Qgis::Warning );

      }
      mRasterColumn = result.PQgetvalue( 0, 0 );
    }
    else
    {
      QgsMessageLog::logMessage( QStringLiteral( "An error occurred while fetching raster column" ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }
  }
  const QString sql { QStringLiteral( "SELECT ST_AsBinary( ST_Envelope( foo.bar) ), ( ST_Metadata( foo.bar ) ).* "
                                      "FROM ( SELECT ST_Union ( %1 ) AS bar FROM %2 ) AS foo" )
                      .arg( quotedIdentifier( mRasterColumn ) )
                      .arg( mQuery ) };
  QgsPostgresResult result( connectionRO()->PQexec( sql ) );
  if ( PGRES_TUPLES_OK == result.PQresultStatus() && result.PQntuples() > 0 )
  {
    bool ok;
    // envelope | upperleftx | upperlefty | width | height | scalex | scaley | skewx | skewy | srid | numbands
    // Extent
    QgsPolygon p;
    // Strip \x
    const QByteArray hexAscii { result.PQgetvalue( 0, 0 ).toAscii().mid( 2 ) };
    QgsConstWkbPtr ptr { QByteArray::fromHex( hexAscii ) };
    if ( ! p.fromWkb( ptr ) )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Cannot get extent from raster" ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }
    mExtent = p.boundingBox();
    // Size
    mWidth = result.PQgetvalue( 0, 3 ).toInt( &ok );
    if ( ! ok )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Cannot convert width '%1' to int" ).arg( result.PQgetvalue( 0, 3 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }
    mHeight = result.PQgetvalue( 0, 4 ).toInt( &ok );
    if ( ! ok )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Cannot convert height '%1' to int" ).arg( result.PQgetvalue( 0, 4 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }
    mScaleX = result.PQgetvalue( 0, 5 ).toInt( &ok );
    if ( ! ok )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Cannot convert scale X '%1' to int" ).arg( result.PQgetvalue( 0, 5 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }
    mScaleY = result.PQgetvalue( 0, 6 ).toInt( &ok );
    if ( ! ok )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Cannot convert scale Y '%1' to int" ).arg( result.PQgetvalue( 0, 6 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }
    mCrs = QgsCoordinateReferenceSystem::fromEpsgId( result.PQgetvalue( 0, 9 ).toLong( &ok ) );
    if ( ! ok )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Cannot create CRS from EPSG: '%1'" ).arg( result.PQgetvalue( 0, 9 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }
    mBandCount = result.PQgetvalue( 0, 10 ).toInt( &ok );
    if ( ! ok )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Cannot convert band count '%1' to int" ).arg( result.PQgetvalue( 0, 10 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }
    // Fetch band data types
    for ( int band = 1; band <= mBandCount; ++band )
    {
      // pixeltype | nodatavalue | isoutdb | path
      const QString sql { QStringLiteral( "SELECT ( ST_BandMetadata( foo.bar, %1 ) ).* "
                                          "FROM ( SELECT ST_Union ( ST_Band ( %2, %1 ) ) AS bar FROM %3 ) AS foo" )
                          .arg( band )
                          .arg( quotedIdentifier( mRasterColumn ) )
                          .arg( mQuery ) };
      QgsPostgresResult result( connectionRO()->PQexec( sql ) );
      if ( PGRES_TUPLES_OK == result.PQresultStatus() && result.PQntuples() > 0 )
      {

        Qgis::DataType type { pixelTypeFromString( result.PQgetvalue( 0, 0 ) ) };

        if ( type == Qgis::DataType::UnknownDataType )
        {
          QgsMessageLog::logMessage( QStringLiteral( "Unsupported data type: '%1'" ).arg( result.PQgetvalue( 0, 0 ) ),
                                     QStringLiteral( "PostGIS" ), Qgis::Critical );
          return false;
        }
        mDataTypes.push_back( type );
        mDataSizes.push_back( QgsRasterBlock::typeSize( type ) );
        double nodataValue { result.PQgetvalue( 0, 1 ).toDouble( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( QStringLiteral( "Cannot convert nodata value '%1' to double, default to: %2" )
                                     .arg( result.PQgetvalue( 0, 1 ) )
                                     .arg( std::numeric_limits<double>::min() ), QStringLiteral( "PostGIS" ), Qgis::Info );
          nodataValue = std::numeric_limits<double>::min();
        }
        mNoDataValues.push_back( nodataValue );
        mIsOutOfDb = result.PQgetvalue( 0, 2 ) == 't';
      }
      else
      {
        QgsMessageLog::logMessage( QStringLiteral( "An error occurred while fetching raster band metadata" ),
                                   QStringLiteral( "PostGIS" ), Qgis::Critical );
        return false;
      }
    }
  }
  else
  {
    QgsMessageLog::logMessage( QStringLiteral( "An error occurred while fetching raster metadata" ),
                               QStringLiteral( "PostGIS" ), Qgis::Critical );
    return false;
  }
  return true;
}

void QgsPostgresRasterProvider::findOverviews()
{
  const QString sql { QStringLiteral( "SELECT overview_factor, o_table_schema, o_table_name, o_raster_columnr_raster_column, srid "
                                      "FROM raster_overviews WHERE r_table_schema = %1 AND r_table_name = %2 AND r_table_catalog = %3" )
                      .arg( quotedIdentifier( mSchemaName ) )
                      .arg( quotedIdentifier( mTableName ) )
                      .arg( quotedValue( mUri.database() ) ) };

  QgsDebugMsg( QStringLiteral( "Raster overview information sql: %1" ).arg( sql ) );

  QgsPostgresResult result( connectionRO()->PQexec( sql ) );
  if ( PGRES_TUPLES_OK == result.PQresultStatus() )
  {
    for ( int i = 0; i < result.PQntuples(); ++i )
    {
      bool ok;
      const int overViewFactor { result.PQgetvalue( i, 0 ).toInt( & ok ) };
      if ( ! ok )
      {
        QgsMessageLog::logMessage( QStringLiteral( "Cannot convert overview factor '%1' to int" ).arg( result.PQgetvalue( i, 0 ) ), QStringLiteral( "PostGIS" ), Qgis::Warning );
        return;
      }
      const QString table { result.PQgetvalue( i, 1 ) };
      const QString schema { result.PQgetvalue( i, 2 ) };
      if ( table.isEmpty() || schema.isEmpty() )
      {
        QgsMessageLog::logMessage( QStringLiteral( "Table or schema is empty" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
        return;
      }
      mOverViews[ overViewFactor ] = QStringLiteral( "%1.%2" ).arg( quotedIdentifier( schema ) ).arg( quotedIdentifier( table ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( QStringLiteral( "No overviews found, performaces may be affected" ), QStringLiteral( "PostGIS" ), Qgis::Info );
  }
}

QString QgsPostgresRasterProvider::overviewName( const double scale ) const
{
  if ( mOverViews.isEmpty() )
  {
    return mQuery;
  }
  return mQuery;
}

int QgsPostgresRasterProvider::xSize() const
{
  return mWidth;
}

int QgsPostgresRasterProvider::ySize() const
{
  return mHeight;
}

Qgis::DataType QgsPostgresRasterProvider::sourceDataType( int bandNo ) const
{
  if ( bandNo <= mBandCount && static_cast<unsigned long>( bandNo ) <= mDataTypes.size() )
  {
    return mDataTypes[ static_cast<unsigned long>( bandNo - 1 ) ];
  }
  else
  {
    QgsMessageLog::logMessage( QStringLiteral( "Data type is unknown" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
    return Qgis::DataType::UnknownDataType;
  }
}


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsPostgresRasterProviderMetadata();
}
#endif
