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
  , mWidth( other.mWidth )
  , mHeight( other.mHeight )
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
    // TODO: log
    return false;
  }
  // Fetch data from backend
  QString sql;
  const bool isSingleValue {  width == 1 && height == 1 };
  if ( isSingleValue )
  {
    sql = QStringLiteral( "SELECT ST_Value( ST_Union( %1, %2), ST_GeomFromText( %3, %4 ), FALSE ) FROM %5" )
          .arg( quotedIdentifier( mRasterColumn ) )
          .arg( bandNo )
          .arg( quotedValue( viewExtent.center().asWkt() ) )
          .arg( mCrs.postgisSrid() )
          .arg( mQuery );
  }
  else
  {
    // TODO: resample if width and height are different from x/y size
    sql = QStringLiteral( "SELECT ST_AsBinary( ST_Resize( ST_Clip ( ST_Union( %1, %2 ), %2, ST_GeomFromText( %4, %5 ), %6 ), %8, %9 ) ) FROM %3 "
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
          // TODO: log
          return false;
        }
        std::memcpy( data, &byte, sizeof( unsigned short ) );
      }
      else if ( dataType == Qgis::DataType::UInt16 )
      {
        const unsigned int uint { val.toUInt( &ok ) };
        if ( ! ok )
        {
          // TODO: log
          return false;
        }
        std::memcpy( data, &uint, sizeof( unsigned int ) );
      }
      else if ( dataType == Qgis::DataType::UInt32 )
      {
        const unsigned long ulong { val.toULong( &ok ) };
        if ( ! ok )
        {
          // TODO: log
          return false;
        }
        std::memcpy( data, &ulong, sizeof( unsigned long ) );
      }
      else if ( dataType == Qgis::DataType::Int16 )
      {
        const int _int { val.toInt( &ok ) };
        if ( ! ok )
        {
          // TODO: log
          return false;
        }
        std::memcpy( data, &_int, sizeof( int ) );
      }
      else if ( dataType == Qgis::DataType::Int32 )
      {
        const long _long { val.toInt( &ok ) };
        if ( ! ok )
        {
          // TODO: log
          return false;
        }
        std::memcpy( data, &_long, sizeof( long ) );
      }
      else if ( dataType == Qgis::DataType::Float32 )
      {
        const float _float { val.toFloat( &ok ) };
        if ( ! ok )
        {
          // TODO: log
          return false;
        }
        std::memcpy( data, &_float, sizeof( float ) );
      }
      else if ( dataType == Qgis::DataType::Float64 )
      {
        const double _double { val.toDouble( &ok ) };
        if ( ! ok )
        {
          // TODO: log
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
    const QByteArray bits { result.PQgetvalue( 0, 0 ).toAscii() };
    const size_t dataSizeBytes { static_cast<size_t>( width *height *mDataSizes[ static_cast<unsigned long>( bandNo - 1 ) ] ) };
    // header length is 132, plus \x
    if ( bits.length() != 2 + 132 + dataSizeBytes * 2 )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Wrong data size: expected = %1, actual = %2" )
                                 .arg( 2 + 132 + dataSizeBytes * 2 )
                                 .arg( bits.length() ), QStringLiteral( "PostGIS" ), Qgis::Warning );
      return false;
    }
    // Decode
    const QByteArray bin { QByteArray::fromHex( bits.mid( 2 + 132 ) ) };
    std::memcpy( data, bin.constData(), dataSizeBytes );
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
  // Get information from metadata
  if ( mUseEstimatedMetadata )
  {
    QString sql { QStringLiteral( "SELECT r_raster_column, srid,"
                                  "num_bands, pixel_types, nodata_values, ST_AsBinary(extent), blocksize_x, blocksize_y,"
                                  "out_db, scale_x, scale_y, same_alignment,"
                                  "regular_blocking,"
                                  "spatial_index FROM raster_columns WHERE "
                                  "r_table_name = %1 AND r_table_schema = %2 AND r_table_catalog = %3" )
                  .arg( quotedValue( mTableName ) )
                  .arg( quotedValue( mSchemaName ) )
                  .arg( quotedValue( mUri.database() ) ) };

    QgsDebugMsg( QStringLiteral( "Raster information from raster_columns sql: %1" ).arg( sql ) );

    QgsPostgresResult result( connectionRO()->PQexec( sql ) );
    if ( PGRES_TUPLES_OK == result.PQresultStatus() )
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
      mRasterColumn = result.PQgetvalue( 0, 0 );
      bool ok;
      mCrs = QgsCoordinateReferenceSystem::fromEpsgId( result.PQgetvalue( 0, 1 ).toLong( &ok ) );
      if ( ! ok )
      {
        // TODO: log
        return false;
      }
      mBandCount = result.PQgetvalue( 0, 2 ).toInt( &ok );
      if ( ! ok )
      {
        // TODO: log
        return false;
      }
      const QStringList pxTypes { result.PQgetvalue( 0, 3 ).chopped( 1 ).mid( 1 ).split( ',' ) };
      const QStringList noDataValues { result.PQgetvalue( 0, 4 ).chopped( 1 ).mid( 1 ).split( ',' ) };
      if ( mBandCount != pxTypes.count( ) || mBandCount != noDataValues.count() )
      {
        // TODO: log
        return false;
      }

      int i = 0;
      for ( const QString &t : qgis::as_const( pxTypes ) )
      {
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
        else
        {
          // TODO: log
        }
        mDataTypes.push_back( type );
        mDataSizes.push_back( QgsRasterBlock::typeSize( type ) );
        mNoDataValues.push_back( noDataValues.at( i++ ).toDouble( &ok ) );
        if ( ! ok )
        {
          // TODO: log
          return false;
        }
      }
      // Extent
      QgsPolygon p;
      // Strip \x
      const QByteArray hexAscii { result.PQgetvalue( 0, 5 ).toAscii().mid( 2 ) };
      QgsConstWkbPtr ptr { QByteArray::fromHex( hexAscii ) };
      if ( ! p.fromWkb( ptr ) )
      {
        // TODO: log
        return false;
      }
      mExtent = p.boundingBox();
      // Size
      mWidth = result.PQgetvalue( 0, 6 ).toInt( &ok );
      if ( ! ok )
      {
        // TODO: log
        return false;
      }
      mHeight = result.PQgetvalue( 0, 7 ).toInt( &ok );
      if ( ! ok )
      {
        // TODO: log
        return false;
      }
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    // TODO implement guessing strategy
    return false;
  }
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
  //TODO
}


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsPostgresRasterProviderMetadata();
}
#endif
