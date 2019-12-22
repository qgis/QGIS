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

  /* deduced
  mRasterColumn = mUri.geometryColumn();
  if ( mRasterColumn.isEmpty() )
  {
    mRasterColumn = QStringLiteral( "rast" );
  }
  */

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
  , mExtent( other.mExtent )
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

bool QgsPostgresRasterProvider::readBlock( int bandNo, const QgsRectangle &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback )
{
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
  return mDataTypes[ bandNo ];
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
                                  "num_bands, pixel_types, nodata_values, extent, out_db,"
                                  "scale_x, scale_y, blocksize_x, blocksize_y, same_alignment,"
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
        mNoDataValues.push_back( noDataValues.at( i++ ).toDouble( &ok ) );
        if ( ! ok )
        {
          // TODO: log
          return false;
        }
      }
      // Extent
      QgsPolygon p;
      const QByteArray hexAscii { result.PQgetvalue( 0, 5 ).toAscii() };
      qDebug() << hexAscii;
      qDebug() << QByteArray::fromHex( hexAscii );
      QgsConstWkbPtr ptr { QByteArray::fromHex( hexAscii ) };
      if ( ! p.fromWkb( ptr ) )
      {
        // TODO: log
        return false;
      }
      mExtent = p.boundingBox();
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
