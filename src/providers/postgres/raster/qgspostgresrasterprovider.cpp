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
#include "qgspostgresprovider.h"
#include "qgsgdalutils.h"
#include "qgsstringutils.h"

#include <QRegularExpression>

const QString QgsPostgresRasterProvider::PG_RASTER_PROVIDER_KEY = QStringLiteral( "postgresraster" );
const QString QgsPostgresRasterProvider::PG_RASTER_PROVIDER_DESCRIPTION =  QStringLiteral( "Postgres raster provider" );


QgsPostgresRasterProvider::QgsPostgresRasterProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags )
  : QgsRasterDataProvider( uri, providerOptions, flags )
  , mShared( new QgsPostgresRasterSharedData )
{

  mUri = uri;

  // populate members from the uri structure
  mSchemaName = mUri.schema();
  if ( mSchemaName.isEmpty() )
  {
    mSchemaName = QStringLiteral( "public" );
  }
  mTableName = mUri.table();

  mRasterColumn = mUri.geometryColumn();
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

  QgsDebugMsgLevel( QStringLiteral( "Connection info is %1" ).arg( mUri.connectionInfo( false ) ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "Schema is: %1" ).arg( mSchemaName ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "Table name is: %1" ).arg( mTableName ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "Query is: %1" ).arg( mQuery ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "Where clause is: %1" ).arg( mSqlWhereClause ), 4 );

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

  if ( !init() ) // gets srid and data type
  {
    // the table is not a raster table
    QgsMessageLog::logMessage( tr( "Invalid PostgreSQL raster layer" ), tr( "PostGIS" ) );
    disconnectDb();
    return;
  }

  // Check if requested srid and detected srid match
  if ( ! mDetectedSrid.isEmpty() && ! mRequestedSrid.isEmpty() && mRequestedSrid != mDetectedSrid )
  {
    QgsMessageLog::logMessage( tr( "Requested SRID (%1) and detected SRID (%2) differ" )
                               .arg( mRequestedSrid )
                               .arg( mDetectedSrid ),
                               QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
  }

  mLayerMetadata.setType( QStringLiteral( "dataset" ) );
  mLayerMetadata.setCrs( crs() );

  mValid = true;
}

QgsPostgresRasterProvider::QgsPostgresRasterProvider( const QgsPostgresRasterProvider &other, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags )
  : QgsRasterDataProvider( other.dataSourceUri(), providerOptions, flags )
  , mValid( other.mValid )
  , mCrs( other.mCrs )
  , mUri( other.mUri )
  , mIsQuery( other.mIsQuery )
  , mTableName( other.mTableName )
  , mQuery( other.mQuery )
  , mRasterColumn( other.mRasterColumn )
  , mSchemaName( other.mSchemaName )
  , mSqlWhereClause( other.mSqlWhereClause )
  , mUseEstimatedMetadata( other.mUseEstimatedMetadata )
  , mDataTypes( other.mDataTypes )
  , mDataSizes( other.mDataSizes )
  , mOverViews( other.mOverViews )
  , mBandCount( other.mBandCount )
  , mIsTiled( other.mIsTiled )
  , mIsOutOfDb( other.mIsOutOfDb )
  , mHasSpatialIndex( other.mHasSpatialIndex )
  , mWidth( other.mWidth )
  , mHeight( other.mHeight )
  , mTileWidth( other.mTileWidth )
  , mTileHeight( other.mTileHeight )
  , mScaleX( other.mScaleX )
  , mScaleY( other.mScaleY )
  , mTemporalFieldIndex( other.mTemporalFieldIndex )
  , mTemporalDefaultTime( other.mTemporalDefaultTime )
  , mAttributeFields( other.mAttributeFields )
  , mIdentityFields( other.mIdentityFields )
  , mDefaultValues( other.mDefaultValues )
  , mDataComment( other.mDataComment )
  , mDetectedSrid( other.mDetectedSrid )
  , mRequestedSrid( other.mRequestedSrid )
  , mConnectionRO( other.mConnectionRO )
  , mConnectionRW( other.mConnectionRW )
  , mPrimaryKeyType( other.mPrimaryKeyType )
  , mPrimaryKeyAttrs( other.mPrimaryKeyAttrs )
  , mShared( other.mShared )
{
}


bool QgsPostgresRasterProvider::hasSufficientPermsAndCapabilities()
{
  QgsDebugMsgLevel( QStringLiteral( "Checking for permissions on the relation" ), 4 );

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

    if ( connectionRO()->pgVersion() >= 90000 )
    {
      testAccess = connectionRO()->PQexec( QStringLiteral( "SELECT pg_is_in_recovery()" ) );
      if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK || testAccess.PQgetvalue( 0, 0 ) == QLatin1String( "t" ) )
      {
        QgsMessageLog::logMessage( tr( "PostgreSQL is still in recovery after a database crash\n(or you are connected to a (read-only) standby server).\nWrite accesses will be denied." ), tr( "PostGIS" ) );
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
    QgsMessageLog::logMessage( tr( "Invalid band number '%1" ).arg( bandNo ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
    return false;
  }

  const QgsRectangle rasterExtent = viewExtent.intersect( mExtent );
  if ( rasterExtent.isEmpty() )
  {
    QgsMessageLog::logMessage( tr( "Requested extent is not valid" ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
    return false;
  }

  const bool isSingleValue {  width == 1 && height == 1 };
  QString tableToQuery { mQuery };

  QString whereAnd { subsetStringWithTemporalRange() };
  if ( ! whereAnd.isEmpty() )
  {
    whereAnd = whereAnd.append( QStringLiteral( " AND " ) );
  }

  // Identify
  if ( isSingleValue )
  {
    QString sql;
    sql = QStringLiteral( "SELECT ST_Value( ST_Band( %1, %2), ST_GeomFromText( %3, %4 ), FALSE ) "
                          "FROM %5 "
                          "WHERE %6 %1 && ST_GeomFromText( %3, %4 )" )
          .arg( quotedIdentifier( mRasterColumn ) )
          .arg( bandNo )
          .arg( quotedValue( viewExtent.center().asWkt() ) )
          .arg( mCrs.postgisSrid() )
          .arg( mQuery )
          .arg( whereAnd );

    QgsPostgresResult result( connectionRO()->PQexec( sql ) );
    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Unable to access the %1 relation.\nThe error message from the database was:\n%2.\nSQL: %3" )
                                 .arg( mQuery,
                                       result.PQresultErrorMessage(),
                                       sql ), tr( "PostGIS" ) );
      return false;
    }

    bool ok;
    const QString val { result.PQgetvalue( 0, 0 ) };
    const Qgis::DataType dataType { mDataTypes[ static_cast<unsigned int>( bandNo - 1 ) ] };
    switch ( dataType )
    {
      case Qgis::DataType::Byte:
      {
        const unsigned short byte { val.toUShort( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to byte" ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
          return false;
        }
        std::memcpy( data, &byte, sizeof( unsigned short ) );
        break;
      }
      case Qgis::DataType::UInt16:
      {
        const unsigned int uint { val.toUInt( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to unsigned int" ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
          return false;
        }
        std::memcpy( data, &uint, sizeof( unsigned int ) );
        break;
      }
      case Qgis::DataType::UInt32:
      {
        const unsigned long ulong { val.toULong( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to unsigned long" ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
          return false;
        }
        std::memcpy( data, &ulong, sizeof( unsigned long ) );
        break;
      }
      case Qgis::DataType::Int16:
      {
        const int intVal { val.toInt( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to int" ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
          return false;
        }
        std::memcpy( data, &intVal, sizeof( int ) );
        break;
      }
      case Qgis::DataType::Int32:
      {
        const long longVal { val.toLong( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to long" ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
          return false;
        }
        std::memcpy( data, &longVal, sizeof( long ) );
        break;
      }
      case Qgis::DataType::Float32:
      {
        const float floatVal { val.toFloat( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to float" ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
          return false;
        }
        std::memcpy( data, &floatVal, sizeof( float ) );
        break;
      }
      case Qgis::DataType::Float64:
      {
        const double doubleVal { val.toDouble( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to double" ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
          return false;
        }
        std::memcpy( data, &doubleVal, sizeof( double ) );
        break;
      }
      default:
      {
        QgsMessageLog::logMessage( tr( "Unknown identified data type" ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
        return false;
      }
    }
  }
  else // Fetch block
  {

    const double xRes = viewExtent.width() / width;
    const double yRes = viewExtent.height() / height;

    // Find overview
    const int minPixelSize { static_cast<int>( std::min( xRes, yRes ) ) };
    // TODO: round?
    const unsigned int desiredOverviewFactor { static_cast<unsigned int>( minPixelSize / std::max( std::abs( mScaleX ), std::abs( mScaleY ) ) ) };

    unsigned int overviewFactor { 1 };  // no overview

    // Cannot use overviews if there is a where condition
    if ( whereAnd.isEmpty() )
    {
      const auto ovKeys { mOverViews.keys( ) };
      QList<unsigned int>::const_reverse_iterator rit { ovKeys.rbegin() };
      for ( ; rit != ovKeys.rend(); ++rit )
      {
        if ( *rit <= desiredOverviewFactor )
        {
          tableToQuery = mOverViews[ *rit ];
          overviewFactor = *rit;
          QgsDebugMsgLevel( QStringLiteral( "Using overview for block read: %1" ).arg( tableToQuery ), 3 );
          break;
        }
      }
    }

    //qDebug() << "Overview desired: " << desiredOverviewFactor << "found:" << overviewFactor << tableToQuery;
    //qDebug() << "View extent" << viewExtent.toString( 1 ) << width << height << minPixelSize;

    // Get the the tiles we need to build the block
    const QgsPostgresRasterSharedData::TilesRequest tilesRequest
    {
      bandNo,
      rasterExtent,
      overviewFactor,
      pkSql(),  // already quoted
      quotedIdentifier( mRasterColumn ),
      tableToQuery,
      QString::number( mCrs.postgisSrid() ),
      whereAnd,
      connectionRO()
    };

    const QgsPostgresRasterSharedData::TilesResponse tileResponse
    {
      mShared->tiles( tilesRequest )
    };

    if ( tileResponse.tiles.isEmpty() )
    {
      QgsMessageLog::logMessage( tr( "No tiles available in table %1 for the requested extent: %2" )
                                 .arg( tableToQuery, rasterExtent.toString( ) ), tr( "PostGIS" ), Qgis::MessageLevel::Critical );
      return false;
    }


    // Finally merge the tiles
    // We must have at least one tile at this point (we checked for that before)

    const QgsRectangle &tilesExtent { tileResponse.extent };

    // Prepare tmp output raster
    const int tmpWidth = static_cast<int>( std::round( tilesExtent.width() / tileResponse.tiles.first().scaleX ) );
    const int tmpHeight = static_cast<int>( std::round( tilesExtent.height() / std::fabs( tileResponse.tiles.first().scaleY ) ) );

    GDALDataType gdalDataType { static_cast<GDALDataType>( sourceDataType( bandNo ) ) };

    //qDebug() << "Creating output raster: " << tilesExtent.toString() << tmpWidth << tmpHeight;

    gdal::dataset_unique_ptr tmpDS { QgsGdalUtils::createSingleBandMemoryDataset(
                                       gdalDataType, tilesExtent, tmpWidth, tmpHeight, mCrs ) };
    if ( ! tmpDS )
    {
      {
        QgsMessageLog::logMessage( tr( "Unable to create temporary raster for tiles from %1" )
                                   .arg( tableToQuery ), tr( "PostGIS" ), Qgis::MessageLevel::Critical );
        return false;
      }
    }

    // Write tiles to the temporary raster
    CPLErrorReset();
    for ( auto &tile : std::as_const( tileResponse.tiles ) )
    {
      // Offset in px from the base raster
      const int xOff { static_cast<int>( std::round( ( tile.upperLeftX - tilesExtent.xMinimum() ) / tile.scaleX ) ) };
      const int yOff { static_cast<int>( std::round( ( tilesExtent.yMaximum() - tile.extent.yMaximum() ) / std::fabs( tile.scaleY ) ) )};

      //qDebug() << "Merging tile output raster: " << tile.tileId << xOff << yOff << tile.width << tile.height ;

      CPLErr err =  GDALRasterIO( GDALGetRasterBand( tmpDS.get(), 1 ),
                                  GF_Write,
                                  xOff,
                                  yOff,
                                  static_cast<int>( tile.width ),
                                  static_cast<int>( tile.height ),
                                  ( void * )( tile.data.constData() ),  // old-style because of const
                                  static_cast<int>( tile.width ),
                                  static_cast<int>( tile.height ),
                                  gdalDataType,
                                  0,
                                  0 );
      if ( err != CE_None )
      {
        const QString lastError = QString::fromUtf8( CPLGetLastErrorMsg() ) ;
        QgsMessageLog::logMessage( tr( "Unable to write tile to temporary raster from %1: %2" )
                                   .arg( tableToQuery, lastError ), tr( "PostGIS" ), Qgis::MessageLevel::Critical );
        return false;
      }
    }

#if 0
    // Debug output raster content
    double pdfMin;
    double pdfMax;
    double pdfMean;
    double pdfStdDev;
    GDALGetRasterStatistics( GDALGetRasterBand( tmpDS.get(), 1 ), 0, 1, &pdfMin, &pdfMax, &pdfMean, &pdfStdDev );
    qDebug() << pdfMin << pdfMax << pdfMean << pdfStdDev;
#endif

    // Write data to the output block
    gdal::dataset_unique_ptr dstDS { QgsGdalUtils::createSingleBandMemoryDataset(
                                       gdalDataType, viewExtent, width, height, mCrs ) };
    if ( ! dstDS )
    {
      const QString lastError = QString::fromUtf8( CPLGetLastErrorMsg() ) ;
      QgsMessageLog::logMessage( tr( "Unable to create destination raster for tiles from %1: %2" )
                                 .arg( tableToQuery, lastError ), tr( "PostGIS" ), Qgis::MessageLevel::Critical );
      return false;
    }

    // Resample the raster to the final bounds and resolution
    if ( ! QgsGdalUtils::resampleSingleBandRaster( tmpDS.get(), dstDS.get(), GDALResampleAlg::GRA_NearestNeighbour, nullptr ) )
    {
      const QString lastError = QString::fromUtf8( CPLGetLastErrorMsg() ) ;
      QgsMessageLog::logMessage( tr( "Unable to resample and transform destination raster for tiles from %1: %2" )
                                 .arg( tableToQuery, lastError ), tr( "PostGIS" ), Qgis::MessageLevel::Critical );
      return false;
    }

    // Copy to result buffer
    CPLErrorReset();
    CPLErr err = GDALRasterIO( GDALGetRasterBand( dstDS.get(), 1 ),
                               GF_Read,
                               0,
                               0,
                               width,
                               height,
                               data,
                               width,
                               height,
                               gdalDataType,
                               0,
                               0 );
    if ( err != CE_None )
    {
      const QString lastError = QString::fromUtf8( CPLGetLastErrorMsg() ) ;
      QgsMessageLog::logMessage( tr( "Unable to write raster to block from %1: %2" )
                                 .arg( mQuery, lastError ), tr( "PostGIS" ), Qgis::MessageLevel::Critical );
      return false;
    }

#if 0
    GDALGetRasterStatistics( GDALGetRasterBand( dstDS.get(), 1 ), 0, 1, &pdfMin, &pdfMax, &pdfMean, &pdfStdDev );
    qDebug() << pdfMin << pdfMax << pdfMean << pdfStdDev;

    // Spit it out float 32 data
    for ( int i = 0; i < width * height; ++i )
    {
      qDebug() << reinterpret_cast<const float *>( data )[ i * 4 ];
    }
#endif

  }
  return true;
}


QgsPostgresRasterProviderMetadata::QgsPostgresRasterProviderMetadata()
  : QgsProviderMetadata( QgsPostgresRasterProvider::PG_RASTER_PROVIDER_KEY, QgsPostgresRasterProvider::PG_RASTER_PROVIDER_DESCRIPTION )
{

}

QVariantMap QgsPostgresRasterProviderMetadata::decodeUri( const QString &uri ) const
{
  const QgsDataSourceUri dsUri { uri };
  QVariantMap decoded;

  if ( ! dsUri.database().isEmpty() )
  {
    decoded[ QStringLiteral( "dbname" ) ] = dsUri.database();
  }
  if ( ! dsUri.host().isEmpty() )
  {
    decoded[ QStringLiteral( "host" ) ] = dsUri.host();
  }
  if ( ! dsUri.port().isEmpty() )
  {
    decoded[ QStringLiteral( "port" ) ] = dsUri.port();
  }
  if ( ! dsUri.service().isEmpty() )
  {
    decoded[ QStringLiteral( "service" ) ] = dsUri.service();
  }
  if ( ! dsUri.username().isEmpty() )
  {
    decoded[ QStringLiteral( "username" ) ] = dsUri.username();
  }
  if ( ! dsUri.password().isEmpty() )
  {
    decoded[ QStringLiteral( "password" ) ] = dsUri.password();
  }
  if ( ! dsUri.authConfigId().isEmpty() )
  {
    decoded[ QStringLiteral( "authcfg" ) ] = dsUri.authConfigId();
  }
  if ( ! dsUri.schema().isEmpty() )
  {
    decoded[ QStringLiteral( "schema" ) ] = dsUri.schema();
  }
  if ( ! dsUri.table().isEmpty() )
  {
    decoded[ QStringLiteral( "table" ) ] = dsUri.table();
  }
  if ( ! dsUri.keyColumn().isEmpty() )
  {
    decoded[ QStringLiteral( "key" ) ] = dsUri.keyColumn();
  }
  if ( ! dsUri.srid().isEmpty() )
  {
    decoded[ QStringLiteral( "srid" ) ] = dsUri.srid();
  }
  if ( uri.contains( QStringLiteral( "estimatedmetadata=" ), Qt::CaseSensitivity::CaseInsensitive ) )
  {
    decoded[ QStringLiteral( "estimatedmetadata" ) ] = dsUri.useEstimatedMetadata();
  }
  if ( uri.contains( QStringLiteral( "sslmode=" ), Qt::CaseSensitivity::CaseInsensitive ) )
  {
    decoded[ QStringLiteral( "sslmode" ) ] = dsUri.sslMode();
  }
  // Do not add sql if it's empty
  if ( ! dsUri.sql().isEmpty() )
  {
    decoded[ QStringLiteral( "sql" ) ] = dsUri.sql();
  }
  if ( ! dsUri.geometryColumn().isEmpty() )
  {
    decoded[ QStringLiteral( "geometrycolumn" ) ] = dsUri.geometryColumn();
  }

  // Params
  const static QStringList params {{
      QStringLiteral( "temporalFieldIndex" ),
      QStringLiteral( "temporalDefaultTime" ),
      QStringLiteral( "enableTime" )
    }};

  for ( const QString &pname : std::as_const( params ) )
  {
    if ( dsUri.hasParam( pname ) )
    {
      decoded[ pname ] = dsUri.param( pname );
    }
  }

  return decoded;
}


QString QgsPostgresRasterProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  if ( parts.contains( QStringLiteral( "dbname" ) ) )
    dsUri.setDatabase( parts.value( QStringLiteral( "dbname" ) ).toString() );
  if ( parts.contains( QStringLiteral( "port" ) ) )
    dsUri.setParam( QStringLiteral( "port" ), parts.value( QStringLiteral( "port" ) ).toString() );
  if ( parts.contains( QStringLiteral( "host" ) ) )
    dsUri.setParam( QStringLiteral( "host" ), parts.value( QStringLiteral( "host" ) ).toString() );
  if ( parts.contains( QStringLiteral( "service" ) ) )
    dsUri.setParam( QStringLiteral( "service" ), parts.value( QStringLiteral( "service" ) ).toString() );
  if ( parts.contains( QStringLiteral( "username" ) ) )
    dsUri.setUsername( parts.value( QStringLiteral( "username" ) ).toString() );
  if ( parts.contains( QStringLiteral( "password" ) ) )
    dsUri.setPassword( parts.value( QStringLiteral( "password" ) ).toString() );
  if ( parts.contains( QStringLiteral( "authcfg" ) ) )
    dsUri.setAuthConfigId( parts.value( QStringLiteral( "authcfg" ) ).toString() );
  if ( parts.contains( QStringLiteral( "selectatid" ) ) )
    dsUri.setParam( QStringLiteral( "selectatid" ), parts.value( QStringLiteral( "selectatid" ) ).toString() );
  if ( parts.contains( QStringLiteral( "table" ) ) )
    dsUri.setTable( parts.value( QStringLiteral( "table" ) ).toString() );
  if ( parts.contains( QStringLiteral( "schema" ) ) )
    dsUri.setSchema( parts.value( QStringLiteral( "schema" ) ).toString() );
  if ( parts.contains( QStringLiteral( "key" ) ) )
    dsUri.setParam( QStringLiteral( "key" ), parts.value( QStringLiteral( "key" ) ).toString() );
  if ( parts.contains( QStringLiteral( "srid" ) ) )
    dsUri.setSrid( parts.value( QStringLiteral( "srid" ) ).toString() );
  if ( parts.contains( QStringLiteral( "estimatedmetadata" ) ) )
    dsUri.setParam( QStringLiteral( "estimatedmetadata" ), parts.value( QStringLiteral( "estimatedmetadata" ) ).toString() );
  if ( parts.contains( QStringLiteral( "sslmode" ) ) )
    dsUri.setParam( QStringLiteral( "sslmode" ), QgsDataSourceUri::encodeSslMode( static_cast<QgsDataSourceUri::SslMode>( parts.value( QStringLiteral( "sslmode" ) ).toInt( ) ) ) );
  if ( parts.contains( QStringLiteral( "sql" ) ) )
    dsUri.setSql( parts.value( QStringLiteral( "sql" ) ).toString() );
  if ( parts.contains( QStringLiteral( "geometrycolumn" ) ) )
    dsUri.setGeometryColumn( parts.value( QStringLiteral( "geometrycolumn" ) ).toString() );
  if ( parts.contains( QStringLiteral( "temporalFieldIndex" ) ) )
    dsUri.setParam( QStringLiteral( "temporalFieldIndex" ), parts.value( QStringLiteral( "temporalFieldIndex" ) ).toString() );
  if ( parts.contains( QStringLiteral( "temporalDefaultTime" ) ) )
    dsUri.setParam( QStringLiteral( "temporalDefaultTime" ), parts.value( QStringLiteral( "temporalDefaultTime" ) ).toString() );
  if ( parts.contains( QStringLiteral( "enableTime" ) ) )
    dsUri.setParam( QStringLiteral( "enableTime" ), parts.value( QStringLiteral( "enableTime" ) ).toString() );
  return dsUri.uri( false );
}

QgsPostgresRasterProvider *QgsPostgresRasterProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsPostgresRasterProvider( uri, options, flags );
}


Qgis::DataType QgsPostgresRasterProvider::dataType( int bandNo ) const
{
  if ( mDataTypes.size() < static_cast<unsigned long>( bandNo ) )
  {
    QgsMessageLog::logMessage( tr( "Data type size for band %1 could not be found: num bands is: %2 and the type size map for bands contains: %n item(s)", nullptr, mDataSizes.size() )
                               .arg( bandNo )
                               .arg( mBandCount ),
                               QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
    return Qgis::DataType::UnknownDataType;
  }
  // Band is 1-based
  return mDataTypes[ static_cast<unsigned long>( bandNo ) - 1 ];
}

int QgsPostgresRasterProvider::bandCount() const
{
  return mBandCount;
}

QgsPostgresRasterProvider *QgsPostgresRasterProvider::clone() const
{
  QgsDataProvider::ProviderOptions options;
  options.transformContext = transformContext();
  QgsPostgresRasterProvider *provider = new QgsPostgresRasterProvider( *this, options );
  provider->copyBaseSettings( *this );
  return provider;
}


static inline QString dumpVariantMap( const QVariantMap &variantMap, const QString &title = QString() )
{
  QString result;
  if ( !title.isEmpty() )
  {
    result += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td></td></tr>" ).arg( title );
  }
  for ( auto it = variantMap.constBegin(); it != variantMap.constEnd(); ++it )
  {
    const QVariantMap childMap = it.value().toMap();
    const QVariantList childList = it.value().toList();
    if ( !childList.isEmpty() )
    {
      result += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td><ul>" ).arg( it.key() );
      for ( const QVariant &v : childList )
      {
        const QVariantMap grandChildMap = v.toMap();
        if ( !grandChildMap.isEmpty() )
        {
          result += QStringLiteral( "<li><table>%1</table></li>" ).arg( dumpVariantMap( grandChildMap ) );
        }
        else
        {
          result += QStringLiteral( "<li>%1</li>" ).arg( QgsStringUtils::insertLinks( v.toString() ) );
        }
      }
      result += QLatin1String( "</ul></td></tr>" );
    }
    else if ( !childMap.isEmpty() )
    {
      result += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td><table>%2</table></td></tr>" ).arg( it.key(), dumpVariantMap( childMap ) );
    }
    else
    {
      result += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>" ).arg( it.key(), QgsStringUtils::insertLinks( it.value().toString() ) );
    }
  }
  return result;
}

QString QgsPostgresRasterProvider::htmlMetadata()
{
  // This must return the content of a HTML table starting by tr and ending by tr
  QVariantMap overviews;
  for ( auto it = mOverViews.constBegin(); it != mOverViews.constEnd(); ++it )
  {
    overviews.insert( QString::number( it.key() ), it.value() );
  }

  const QVariantMap additionalInformation
  {
    { tr( "Is Tiled" ), mIsTiled },
    { tr( "Where Clause SQL" ), subsetString() },
    { tr( "Pixel Size" ), QStringLiteral( "%1, %2" ).arg( mScaleX ).arg( mScaleY ) },
    { tr( "Overviews" ),  overviews },
    { tr( "Primary Keys SQL" ),  pkSql() },
    { tr( "Temporal Column" ),  mTemporalFieldIndex >= 0 && mAttributeFields.exists( mTemporalFieldIndex ) ?  mAttributeFields.field( mTemporalFieldIndex ).name() : QString() },
  };
  return  dumpVariantMap( additionalInformation, tr( "Additional information" ) );
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
                         // TODO:| QgsRasterDataProvider::BuildPyramids
                         | QgsRasterDataProvider::Create
                         | QgsRasterDataProvider::Remove
                         | QgsRasterDataProvider::Prefetch;
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

QString QgsPostgresRasterProvider::subsetString() const
{
  return mSqlWhereClause;
}

QString QgsPostgresRasterProvider::defaultTimeSubsetString( const QDateTime &defaultTime ) const
{
  if ( defaultTime.isValid( ) &&
       mTemporalFieldIndex >= 0 &&
       mAttributeFields.exists( mTemporalFieldIndex ) )
  {
    const QgsField temporalField { mAttributeFields.field( mTemporalFieldIndex ) };
    const QString typeCast { temporalField.type() != QVariant::DateTime ? QStringLiteral( "::timestamp" ) : QString() };
    const QString temporalFieldName { temporalField.name() };
    return  { QStringLiteral( "%1%2 = %3" )
              .arg( quotedIdentifier( temporalFieldName ),
                    typeCast,
                    quotedValue( defaultTime.toString( Qt::DateFormat::ISODate ) ) ) };
  }
  else
  {
    return QString();
  }
}

bool QgsPostgresRasterProvider::setSubsetString( const QString &subset, bool updateFeatureCount )
{
  Q_UNUSED( updateFeatureCount )

  const QString oldSql { mSqlWhereClause };

  mSqlWhereClause = subset;
  // Recalculate extent and other metadata calling init()
  if ( !init() )
  {
    // Restore
    mSqlWhereClause = oldSql;
    init();
    return false;
  }

  mStatistics.clear();
  mShared->invalidateCache();

  // Update datasource uri too
  mUri.setSql( subset );
  setDataSourceUri( mUri.uri( false ) );

  return true;
}

QString QgsPostgresRasterProvider::subsetStringWithTemporalRange() const
{
  // Temporal
  if ( mTemporalFieldIndex >= 0 && mAttributeFields.exists( mTemporalFieldIndex ) )
  {
    const QgsField temporalField { mAttributeFields.field( mTemporalFieldIndex ) };
    const QString typeCast { temporalField.type() != QVariant::DateTime ? QStringLiteral( "::timestamp" ) : QString() };
    const QString temporalFieldName { temporalField.name() };

    if ( temporalCapabilities()->hasTemporalCapabilities() )
    {
      QString temporalClause;
      const QgsTemporalRange<QDateTime> requestedRange { temporalCapabilities()->requestedTemporalRange() };
      if ( ! requestedRange.isEmpty() && ! requestedRange.isInfinite() )
      {
        if ( requestedRange.isInstant() )
        {
          temporalClause = QStringLiteral( "%1%2 = %3" )
                           .arg( quotedIdentifier( temporalFieldName ),
                                 typeCast,
                                 quotedValue( requestedRange.begin().toString( Qt::DateFormat::ISODate ) ) );
        }
        else
        {
          if ( requestedRange.begin().isValid() )
          {
            temporalClause = QStringLiteral( "%1%2 %3 %4" )
                             .arg( quotedIdentifier( temporalFieldName ),
                                   typeCast,
                                   requestedRange.includeBeginning() ? ">=" : ">",
                                   quotedValue( requestedRange.begin().toString( Qt::DateFormat::ISODate ) ) );
          }
          if ( requestedRange.end().isValid() )
          {
            if ( ! temporalClause.isEmpty() )
            {
              temporalClause.append( QStringLiteral( " AND " ) );
            }
            temporalClause.append( QStringLiteral( "%1%2 %3 %4" )
                                   .arg( quotedIdentifier( temporalFieldName ),
                                         typeCast,
                                         requestedRange.includeEnd() ? "<=" : "<",
                                         quotedValue( requestedRange.end().toString( Qt::DateFormat::ISODate ) ) ) );
          }
        }
        return mSqlWhereClause.isEmpty() ? temporalClause : QStringLiteral( "%1 AND (%2)" ).arg( mSqlWhereClause, temporalClause );
      }
      const QString defaultTimeSubset { defaultTimeSubsetString( mTemporalDefaultTime ) };
      if ( ! defaultTimeSubset.isEmpty() )
      {
        return mSqlWhereClause.isEmpty() ? defaultTimeSubset : QStringLiteral( "%1 AND (%2)" ).arg( mSqlWhereClause, defaultTimeSubset );
      }
    }
  }
  return mSqlWhereClause;
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

bool QgsPostgresRasterProvider::init()
{

  // WARNING: multiple failure and return points!

  if ( !determinePrimaryKey() )
  {
    QgsMessageLog::logMessage( tr( "PostgreSQL raster layer has no primary key." ), tr( "PostGIS" ) );
    return false;
  }

  // We first try to collect raster information using raster_columns information
  // unless:
  // - it is a query layer (unsupported at the moment)
  // - use estimated metadata is false
  // - there is a WHERE condition (except for temporal default value )
  // If previous conditions are not met or the first method fail try to fetch information
  // directly from the raster data. This can be very slow.
  // Note that a temporal filter set as temporal default value does not count as a WHERE condition

  // utility to get data type from string, used in both branches
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
    if ( t == QLatin1String( "8BUI" ) )
    {
      type = Qgis::DataType::Byte;
    }
    else if ( t == QLatin1String( "16BUI" ) )
    {
      type = Qgis::DataType::UInt16;
    }
    else if ( t == QLatin1String( "16BSI" ) )
    {
      type = Qgis::DataType::Int16;
    }
    else if ( t == QLatin1String( "32BSI" ) )
    {
      type = Qgis::DataType::Int32;
    }
    else if ( t == QLatin1String( "32BUI" ) )
    {
      type = Qgis::DataType::UInt32;
    }
    else if ( t == QLatin1String( "32BF" ) )
    {
      type = Qgis::DataType::Float32;
    }
    else if ( t == QLatin1String( "64BF" ) )
    {
      type = Qgis::DataType::Float64;
    }
    return type;
  };

  // ///////////////////////////////////////////////////////////////////
  // First method: get information from metadata
  if ( ! mIsQuery && mUseEstimatedMetadata && subsetString().isEmpty() )
  {
    try
    {
      const QString sql = QStringLiteral( "SELECT r_raster_column, srid,"
                                          "num_bands, pixel_types, nodata_values, ST_AsBinary(extent), blocksize_x, blocksize_y,"
                                          "out_db, spatial_index, scale_x, scale_y, same_alignment,"
                                          "regular_blocking "
                                          "FROM raster_columns WHERE "
                                          "r_table_name = %1 AND r_table_schema = %2" )
                          .arg( quotedValue( mTableName ), quotedValue( mSchemaName ) );

      QgsPostgresResult result( connectionRO()->PQexec( sql ) );

      if ( ( PGRES_TUPLES_OK == result.PQresultStatus() ) && ( result.PQntuples() > 0 ) )
      {
        mRasterColumn = result.PQgetvalue( 0, 0 );
        mHasSpatialIndex = result.PQgetvalue( 0, 9 ) == 't';
        bool ok;

        mCrs = QgsCoordinateReferenceSystem();
        // FIXME: from Nyall's comment:
        // keep in mind that postgis crs handling is rather broken in QGIS and needs to be rethought for 4.0
        // (we should be retrieving the definition from the server corresponding to the reported postgis srs,
        // and not using any qgis internal databases for this ... which may or may not have any match to the
        // server's definitions.)
        Q_NOWARN_DEPRECATED_PUSH
        mCrs.createFromSrid( result.PQgetvalue( 0, 1 ).toLong( &ok ) );
        Q_NOWARN_DEPRECATED_PUSH

        if ( ! ok )
        {
          throw QgsPostgresRasterProviderException( tr( "Cannot create CRS from EPSG: '%1'" ).arg( result.PQgetvalue( 0, 1 ) ) );
        }

        mDetectedSrid = result.PQgetvalue( 0, 1 );
        mBandCount = result.PQgetvalue( 0, 2 ).toInt( &ok );

        if ( ! ok )
        {
          throw QgsPostgresRasterProviderException( tr( "Cannot get band count from value: '%1'" ).arg( result.PQgetvalue( 0, 2 ) ) );
        }

        QString pxTypesArray { result.PQgetvalue( 0, 3 ) };
        pxTypesArray.chop( 1 );
        const QStringList pxTypes { pxTypesArray.mid( 1 ).split( ',' ) };

        QString noDataValuesArray { result.PQgetvalue( 0, 4 ) };
        noDataValuesArray.chop( 1 );
        const QStringList noDataValues { noDataValuesArray.mid( 1 ).split( ',' ) };

        if ( mBandCount != pxTypes.count( ) || mBandCount != noDataValues.count() )
        {
          throw QgsPostgresRasterProviderException( tr( "Band count and nodata items count differs" ) );
        }

        int i = 0;
        for ( const QString &t : std::as_const( pxTypes ) )
        {
          Qgis::DataType type { pixelTypeFromString( t ) };
          if ( type == Qgis::DataType::UnknownDataType )
          {
            throw QgsPostgresRasterProviderException( tr( "Unsupported data type: '%1'" ).arg( t ) );
          }
          mDataTypes.push_back( type );
          mDataSizes.push_back( QgsRasterBlock::typeSize( type ) );
          double nodataValue { noDataValues.at( i ).toDouble( &ok ) };
          if ( ! ok )
          {
            if ( noDataValues.at( i ) != QLatin1String( "NULL" ) )
            {
              QgsMessageLog::logMessage( tr( "Cannot convert nodata value '%1' to double" )
                                         .arg( noDataValues.at( i ) ),
                                         QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Info );
            }
            mSrcHasNoDataValue.append( false );
            mUseSrcNoDataValue.append( false );
            nodataValue = std::numeric_limits<double>::min();
          }
          else
          {
            mSrcHasNoDataValue.append( true );
            mUseSrcNoDataValue.append( true );
          }
          mSrcNoDataValue.append( QgsRaster::representableValue( nodataValue, type ) );
          ++i;
        }

        // Extent
        QgsPolygon p;
        // Strip \x
        const QByteArray hexAscii { result.PQgetvalue( 0, 5 ).toLatin1().mid( 2 ) };
        const QByteArray hexBin = QByteArray::fromHex( hexAscii );
        QgsConstWkbPtr ptr { hexBin };

        if ( hexAscii.isEmpty() || ! p.fromWkb( ptr ) )
        {
          // Try to determine extent from raster
          const QString extentSql = QStringLiteral( "SELECT ST_Envelope( %1 ) "
                                    "FROM %2 WHERE %3" )
                                    .arg( quotedIdentifier( mRasterColumn ),
                                          mQuery,
                                          subsetString().isEmpty() ? "'t'" : subsetString() );

          QgsPostgresResult extentResult( connectionRO()->PQexec( extentSql ) );
          const QByteArray extentHexAscii { extentResult.PQgetvalue( 0, 0 ).toLatin1() };
          const QByteArray extentHexBin = QByteArray::fromHex( extentHexAscii );
          QgsConstWkbPtr extentPtr { extentHexBin };
          if ( extentHexAscii.isEmpty() || ! p.fromWkb( extentPtr ) )
          {
            throw QgsPostgresRasterProviderException( tr( "Cannot get extent from raster" ) );
          }
        }

        mExtent = p.boundingBox();

        // Tile size
        mTileWidth = result.PQgetvalue( 0, 6 ).toInt( &ok );

        if ( ! ok )
        {
          throw QgsPostgresRasterProviderException( tr( "Cannot convert width '%1' to int" ).arg( result.PQgetvalue( 0, 6 ) ) );
        }

        mTileHeight = result.PQgetvalue( 0, 7 ).toInt( &ok );

        if ( ! ok )
        {
          throw QgsPostgresRasterProviderException( tr( "Cannot convert height '%1' to int" ).arg( result.PQgetvalue( 0, 7 ) ) );
        }

        mIsOutOfDb = result.PQgetvalue( 0, 8 ) == 't';
        mScaleX = result.PQgetvalue( 0, 10 ).toDouble( &ok );

        if ( ! ok )
        {
          throw QgsPostgresRasterProviderException( tr( "Cannot convert scale X '%1' to double" ).arg( result.PQgetvalue( 0, 10 ) ) );
        }

        mScaleY = result.PQgetvalue( 0, 11 ).toDouble( &ok );

        if ( ! ok )
        {
          throw QgsPostgresRasterProviderException( tr( "Cannot convert scale Y '%1' to double" ).arg( result.PQgetvalue( 0, 11 ) ) );
        }

        // Compute raster size
        mHeight = static_cast<long>( std::round( mExtent.height() / std::abs( mScaleY ) ) );
        mWidth = static_cast<long>( std::round( mExtent.width() / std::abs( mScaleX ) ) );
        // is tiled?
        mIsTiled = ( mWidth != mTileWidth ) || ( mHeight != mTileHeight );

        // Detect overviews
        findOverviews();
        return initFieldsAndTemporal( );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "An error occurred while fetching raster metadata for table %1: %2\nSQL: %3" )
                                   .arg( mQuery )
                                   .arg( result.PQresultErrorMessage() )
                                   .arg( sql ),
                                   QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
      }
    }
    catch ( QgsPostgresRasterProviderException &ex )
    {
      QgsMessageLog::logMessage( tr( "An error occurred while fetching raster metadata for %1, proceeding with (possibly very slow) raster data analysis: %2\n"
                                     "Please consider adding raster constraints with PostGIS function AddRasterConstraints." )
                                 .arg( mQuery )
                                 .arg( ex.message ),
                                 QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
    }
  }

  // TODO: query layers + mHasSpatialIndex in case metadata are not used

  // ///////////////////////////////////////////////////////////////////
  // Go the hard and slow way: fetch information directly from the layer
  //
  if ( mRasterColumn.isEmpty() )
  {
    const QString sql = QStringLiteral( "SELECT column_name FROM information_schema.columns WHERE "
                                        "table_name = %1 AND table_schema = %2 AND udt_name = 'raster'" )
                        .arg( quotedValue( mTableName ), quotedValue( mSchemaName ) );

    QgsPostgresResult result( connectionRO()->PQexec( sql ) );

    if ( PGRES_TUPLES_OK == result.PQresultStatus() && result.PQntuples() > 0 )
    {
      if ( result.PQntuples() > 1 )
      {
        QgsMessageLog::logMessage( tr( "Multiple raster column detected, using the first one" ),
                                   QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );

      }
      mRasterColumn = result.PQgetvalue( 0, 0 );
    }
    else
    {
      QgsMessageLog::logMessage( tr( "An error occurred while fetching raster column" ),
                                 QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
      return false;
    }
  }

  // Get the full raster and extract information
  // Note: this can be very slow
  // Use oveviews if we can, even if they are probably missing for unconstrained tables

  findOverviews();

  QString tableToQuery { mQuery };

  if ( ! mOverViews.isEmpty() )
  {
    tableToQuery = mOverViews.last();
  }

  QString where;
  if ( ! subsetString().isEmpty() )
  {
    where = QStringLiteral( "WHERE %1" ).arg( subsetString() );
  }

  // Unfortunately we cannot safely assume that the raster is untiled and just LIMIT 1
  // Fastest SQL: fetch all metadata in one pass
  //   0           1          3           3        4       5         6       7       8       9      10          11           12           13      14
  // encode | upperleftx | upperlefty | width | height | scalex | scaley | skewx | skewy | srid | numbands | pixeltype | nodatavalue | isoutdb | path
  const QString sql = QStringLiteral( R"(
      WITH cte_filtered_raster AS ( SELECT %1 AS filtered_rast FROM %2 %3 ),
           cte_rast AS ( SELECT ST_Union( cte_filtered_raster.filtered_rast ) AS united_raster FROM cte_filtered_raster ),
           cte_bandno AS ( SELECT * FROM generate_series(1, ST_NumBands ( ( SELECT cte_rast.united_raster FROM cte_rast ) ) ) AS bandno ),
           cte_band AS ( SELECT ST_Band( united_raster, bandno ) AS band FROM cte_rast, cte_bandno )
                      SELECT ENCODE( ST_AsBinary( ST_Envelope( band ) ), 'hex'),
                        (ST_Metadata( band  )).*,
                        (ST_BandMetadata( band )).*
                      FROM cte_band)" ).arg( quotedIdentifier( mRasterColumn ), tableToQuery, where );

  QgsDebugMsgLevel( QStringLiteral( "Raster information sql: %1" ).arg( sql ), 4 );

  QgsPostgresResult result( connectionRO()->PQexec( sql ) );
  if ( PGRES_TUPLES_OK == result.PQresultStatus() && result.PQntuples() > 0 )
  {

    // These may have been filled with defaults in the fast track
    mSrcNoDataValue.clear();
    mSrcHasNoDataValue.clear();
    mUseSrcNoDataValue.clear();
    mBandCount = result.PQntuples();

    bool ok;

    // Extent
    QgsPolygon p;
    try
    {
      const QByteArray hexBin = QByteArray::fromHex( result.PQgetvalue( 0, 0 ).toLatin1() );
      QgsConstWkbPtr ptr { hexBin };
      if ( ! p.fromWkb( ptr ) )
      {
        QgsMessageLog::logMessage( tr( "Cannot get extent from raster" ),
                                   QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
        return false;
      }
    }
    catch ( ... )
    {
      QgsMessageLog::logMessage( tr( "Cannot get metadata from raster" ),
                                 QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
      return false;
    }

    mExtent = p.boundingBox();

    // Tile size (in this path the raster is considered untiled, so this is actually the whole size
    mTileWidth = result.PQgetvalue( 0, 3 ).toInt( &ok );

    if ( ! ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot convert width '%1' to int" ).arg( result.PQgetvalue( 0, 3 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
      return false;
    }

    mTileHeight = result.PQgetvalue( 0, 4 ).toInt( &ok );

    if ( ! ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot convert height '%1' to int" ).arg( result.PQgetvalue( 0, 4 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
      return false;
    }

    mScaleX = result.PQgetvalue( 0, 5 ).toDouble( &ok );

    if ( ! ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot convert scale X '%1' to double" ).arg( result.PQgetvalue( 0, 5 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
      return false;
    }

    mScaleY = result.PQgetvalue( 0, 6 ).toDouble( &ok );

    if ( ! ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot convert scale Y '%1' to double" ).arg( result.PQgetvalue( 0, 6 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
      return false;
    }

    // Compute raster size, it is untiled so just take tile dimensions
    mHeight = mTileHeight;
    mWidth = mTileWidth;
    mIsTiled = false;

    mCrs = QgsCoordinateReferenceSystem();
    // FIXME: from Nyall's comment:
    // keep in mind that postgis crs handling is rather broken in QGIS and needs to be rethought for 4.0
    // (we should be retrieving the definition from the server corresponding to the reported postgis srs,
    // and not using any qgis internal databases for this ... which may or may not have any match to the
    // server's definitions.)
    Q_NOWARN_DEPRECATED_PUSH
    mCrs.createFromSrid( result.PQgetvalue( 0, 9 ).toLong( &ok ) );
    Q_NOWARN_DEPRECATED_PUSH

    if ( ! ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot create CRS from EPSG: '%1'" ).arg( result.PQgetvalue( 0, 9 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
      return false;
    }

    mDetectedSrid = result.PQgetvalue( 0, 9 );

    // Fetch band data types
    for ( int rowNumber = 0; rowNumber < result.PQntuples(); ++rowNumber )
    {
      Qgis::DataType type { pixelTypeFromString( result.PQgetvalue( rowNumber, 11 ) ) };

      if ( type == Qgis::DataType::UnknownDataType )
      {
        QgsMessageLog::logMessage( tr( "Unsupported data type: '%1'" ).arg( result.PQgetvalue( rowNumber, 11 ) ),
                                   QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
        return false;
      }

      mDataTypes.push_back( type );
      mDataSizes.push_back( QgsRasterBlock::typeSize( type ) );
      double nodataValue { result.PQgetvalue( rowNumber, 12 ).toDouble( &ok ) };

      if ( ! ok )
      {
        QgsMessageLog::logMessage( tr( "Cannot convert nodata value '%1' to double, default to: %2" )
                                   .arg( result.PQgetvalue( rowNumber, 2 ) )
                                   .arg( std::numeric_limits<double>::min() ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Info );
        nodataValue = std::numeric_limits<double>::min();
      }

      mSrcNoDataValue.append( QgsRaster::representableValue( nodataValue, type ) );
      mSrcHasNoDataValue.append( true );
      mUseSrcNoDataValue.append( true );
    }
    mIsOutOfDb = result.PQgetvalue( 0, 13 ) == 't';
  }
  else
  {
    QgsMessageLog::logMessage( tr( "An error occurred while fetching raster metadata" ),
                               QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
    return false;
  }

  return initFieldsAndTemporal( );
}

bool QgsPostgresRasterProvider::initFieldsAndTemporal( )
{
  // Populate fields
  if ( ! loadFields() )
  {
    QgsMessageLog::logMessage( tr( "An error occurred while fetching raster fields information" ),
                               QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Critical );
    return false;
  }

  QString where;
  if ( ! subsetString().isEmpty() )
  {
    where = QStringLiteral( "WHERE %1" ).arg( subsetString() );
  }

  // Temporal capabilities
  // Setup temporal properties for layer, do not fail if something goes wrong but log a warning
  if ( mUri.hasParam( QStringLiteral( "temporalFieldIndex" ) ) )
  {
    bool ok;
    const int temporalFieldIndex { mUri.param( QStringLiteral( "temporalFieldIndex" ) ).toInt( &ok ) };
    if ( ok && mAttributeFields.exists( temporalFieldIndex ) )
    {
      const QString temporalFieldName { mAttributeFields.field( temporalFieldIndex ).name() };
      // Calculate the range
      const QString sql =  QStringLiteral( "SELECT MIN(%1::timestamp), MAX(%1::timestamp) "
                                           "FROM %2 %3" ).arg( quotedIdentifier( temporalFieldName ),
                                               mQuery,
                                               where );

      QgsPostgresResult result( connectionRO()->PQexec( sql ) );

      if ( PGRES_TUPLES_OK == result.PQresultStatus() && result.PQntuples() == 1 )
      {
        const QDateTime minTime { QDateTime::fromString( result.PQgetvalue( 0, 0 ), Qt::DateFormat::ISODate ) };
        const QDateTime maxTime { QDateTime::fromString( result.PQgetvalue( 0, 1 ), Qt::DateFormat::ISODate ) };
        if ( minTime.isValid() && maxTime.isValid() && !( minTime > maxTime ) )
        {
          mTemporalFieldIndex = temporalFieldIndex;
          temporalCapabilities()->setHasTemporalCapabilities( true );
          temporalCapabilities()->setAvailableTemporalRange( { minTime, maxTime } );
          temporalCapabilities()->setIntervalHandlingMethod( Qgis::TemporalIntervalMatchMethod::FindClosestMatchToStartOfRange );
          QgsDebugMsgLevel( QStringLiteral( "Raster temporal range for field %1: %2 - %3" ).arg( QString::number( mTemporalFieldIndex ), minTime.toString(), maxTime.toString() ), 3 );

          if ( mUri.hasParam( QStringLiteral( "temporalDefaultTime" ) ) )
          {
            const QDateTime defaultDateTime { QDateTime::fromString( mUri.param( QStringLiteral( "temporalDefaultTime" ) ), Qt::DateFormat::ISODate ) };
            if ( defaultDateTime.isValid() )
            {
              mTemporalDefaultTime = defaultDateTime;
            }
            else
            {
              QgsMessageLog::logMessage( tr( "Invalid default date in raster temporal capabilities for field %1: %2" ).arg( temporalFieldName, mUri.param( QStringLiteral( "temporalDefaultTime" ) ) ),
                                         QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
            }
          }

          // Set temporal ranges
          QList< QgsDateTimeRange > allRanges;
          const QString sql =  QStringLiteral( "SELECT DISTINCT %1::timestamp "
                                               "FROM %2 %3 ORDER BY %1::timestamp" ).arg( quotedIdentifier( temporalFieldName ),
                                                   mQuery,
                                                   where );

          QgsPostgresResult result( connectionRO()->PQexec( sql ) );
          if ( PGRES_TUPLES_OK == result.PQresultStatus() && result.PQntuples() > 0 )
          {
            for ( qlonglong row = 0; row < result.PQntuples(); ++row )
            {
              const QDateTime date = QDateTime::fromString( result.PQgetvalue( row, 0 ), Qt::DateFormat::ISODate );
              allRanges.push_back( QgsDateTimeRange( date, date ) );
            }
            temporalCapabilities()->setAllAvailableTemporalRanges( allRanges );
          }
          else
          {
            QgsMessageLog::logMessage( tr( "No temporal ranges detected in raster temporal capabilities for field %1: %2" ).arg( temporalFieldName, mUri.param( QStringLiteral( "temporalDefaultTime" ) ) ),
                                       QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Info );
          }
        }
        else
        {
          QgsMessageLog::logMessage( tr( "Invalid temporal range in raster temporal capabilities for field %1: %2 - %3" ).arg( temporalFieldName, minTime.toString(), maxTime.toString() ),
                                     QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "An error occurred while fetching raster temporal capabilities for field: %1" ).arg( temporalFieldName ),
                                   QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );

      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Invalid field index for raster temporal capabilities: %1" )
                                 .arg( QString::number( temporalFieldIndex ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
    }
  }
  return true;
}

bool QgsPostgresRasterProvider::loadFields()
{

  if ( !mIsQuery )
  {
    QgsDebugMsgLevel( QStringLiteral( "Loading fields for table %1" ).arg( mTableName ), 2 );

    // Get the relation oid for use in later queries
    QString sql = QStringLiteral( "SELECT regclass(%1)::oid" ).arg( quotedValue( mQuery ) );
    QgsPostgresResult tresult( connectionRO()->PQexec( sql ) );
    QString tableoid = tresult.PQgetvalue( 0, 0 );

    // Get the table description
    sql = QStringLiteral( "SELECT description FROM pg_description WHERE objoid=%1 AND objsubid=0" ).arg( tableoid );
    tresult = connectionRO()->PQexec( sql );
    if ( tresult.PQntuples() > 0 )
    {
      mDataComment = tresult.PQgetvalue( 0, 0 );
      mLayerMetadata.setAbstract( mDataComment );
    }
  }
  else
  {
    // Not supported for now
    return true;
  }

  // Populate the field vector for this layer. The field vector contains
  // field name, type, length, and precision (if numeric)
  QString sql = QStringLiteral( "SELECT * FROM %1 LIMIT 0" ).arg( mQuery );

  QgsPostgresResult result( connectionRO()->PQexec( sql ) );

  // Collect type info
  sql = QStringLiteral( "SELECT oid,typname,typtype,typelem,typlen FROM pg_type" );
  QgsPostgresResult typeResult( connectionRO()->PQexec( sql ) );

  QMap<Oid, PGTypeInfo> typeMap;
  for ( int i = 0; i < typeResult.PQntuples(); ++i )
  {
    PGTypeInfo typeInfo =
    {
      /* typeName = */ typeResult.PQgetvalue( i, 1 ),
      /* typeType = */ typeResult.PQgetvalue( i, 2 ),
      /* typeElem = */ typeResult.PQgetvalue( i, 3 ),
      /* typeLen = */ typeResult.PQgetvalue( i, 4 ).toInt()
    };
    typeMap.insert( typeResult.PQgetvalue( i, 0 ).toUInt(), typeInfo );
  }


  QMap<Oid, QMap<int, QString> > fmtFieldTypeMap, descrMap, defValMap, identityMap;
  QMap<Oid, QMap<int, Oid> > attTypeIdMap;
  QMap<Oid, QMap<int, bool> > notNullMap, uniqueMap;
  if ( result.PQnfields() > 0 )
  {
    // Collect table oids
    QSet<Oid> tableoids;
    for ( int i = 0; i < result.PQnfields(); i++ )
    {
      Oid tableoid = result.PQftable( i );
      if ( tableoid > 0 )
      {
        tableoids.insert( tableoid );
      }
    }

    if ( !tableoids.isEmpty() )
    {
      QStringList tableoidsList;
      const auto constTableoids = tableoids;
      for ( Oid tableoid : constTableoids )
      {
        tableoidsList.append( QString::number( tableoid ) );
      }

      QString tableoidsFilter = '(' + tableoidsList.join( QLatin1Char( ',' ) ) + ')';

      // Collect formatted field types
      sql = QStringLiteral(
              "SELECT attrelid, attnum, pg_catalog.format_type(atttypid,atttypmod), pg_catalog.col_description(attrelid,attnum), pg_catalog.pg_get_expr(adbin,adrelid), atttypid, attnotnull::int, indisunique::int%1"
              " FROM pg_attribute"
              " LEFT OUTER JOIN pg_attrdef ON attrelid=adrelid AND attnum=adnum"

              // find unique constraints if present. Text cast required to handle int2vector comparison. Distinct required as multiple unique constraints may exist
              " LEFT OUTER JOIN ( SELECT DISTINCT indrelid, indkey, indisunique FROM pg_index WHERE indisunique ) uniq ON attrelid=indrelid AND attnum::text=indkey::text "

              " WHERE attrelid IN %2"
            ).arg( connectionRO()->pgVersion() >= 100000 ? QStringLiteral( ", attidentity" ) : QString() ).arg( tableoidsFilter );

      QgsPostgresResult fmtFieldTypeResult( connectionRO()->PQexec( sql ) );
      for ( int i = 0; i < fmtFieldTypeResult.PQntuples(); ++i )
      {
        Oid attrelid = fmtFieldTypeResult.PQgetvalue( i, 0 ).toUInt();
        int attnum = fmtFieldTypeResult.PQgetvalue( i, 1 ).toInt(); // Int2
        QString formatType = fmtFieldTypeResult.PQgetvalue( i, 2 );
        QString descr = fmtFieldTypeResult.PQgetvalue( i, 3 );
        QString defVal = fmtFieldTypeResult.PQgetvalue( i, 4 );
        Oid attType = fmtFieldTypeResult.PQgetvalue( i, 5 ).toUInt();
        bool attNotNull = fmtFieldTypeResult.PQgetvalue( i, 6 ).toInt();
        bool uniqueConstraint = fmtFieldTypeResult.PQgetvalue( i, 7 ).toInt();
        QString attIdentity = connectionRO()->pgVersion() >= 100000 ? fmtFieldTypeResult.PQgetvalue( i, 8 ) : " ";
        fmtFieldTypeMap[attrelid][attnum] = formatType;
        descrMap[attrelid][attnum] = descr;
        defValMap[attrelid][attnum] = defVal;
        attTypeIdMap[attrelid][attnum] = attType;
        notNullMap[attrelid][attnum] = attNotNull;
        uniqueMap[attrelid][attnum] = uniqueConstraint;
        identityMap[attrelid][attnum] = attIdentity.isEmpty() ? " " : attIdentity;
      }
    }
  }

  QSet<QString> fields;
  mAttributeFields.clear();
  mIdentityFields.clear();
  for ( int i = 0; i < result.PQnfields(); i++ )
  {
    QString fieldName = result.PQfname( i );
    if ( fieldName == mRasterColumn )
      continue;

    Oid fldtyp = result.PQftype( i );
    int fldMod = result.PQfmod( i );
    int fieldPrec = 0;
    Oid tableoid = result.PQftable( i );
    int attnum = result.PQftablecol( i );
    Oid atttypid = attTypeIdMap[tableoid][attnum];

    const PGTypeInfo &typeInfo = typeMap.value( fldtyp );
    QString fieldTypeName = typeInfo.typeName;
    QString fieldTType = typeInfo.typeType;
    int fieldSize = typeInfo.typeLen;

    bool isDomain = ( typeMap.value( atttypid ).typeType == QLatin1String( "d" ) );

    QString formattedFieldType = fmtFieldTypeMap[tableoid][attnum];
    QString originalFormattedFieldType = formattedFieldType;
    if ( isDomain )
    {
      // get correct formatted field type for domain
      sql = QStringLiteral( "SELECT format_type(%1, %2)" ).arg( fldtyp ).arg( fldMod );
      QgsPostgresResult fmtFieldModResult( connectionRO()->PQexec( sql ) );
      if ( fmtFieldModResult.PQntuples() > 0 )
      {
        formattedFieldType = fmtFieldModResult.PQgetvalue( 0, 0 );
      }
    }

    QString fieldComment = descrMap[tableoid][attnum];

    QVariant::Type fieldType;
    QVariant::Type fieldSubType = QVariant::Invalid;

    if ( fieldTType == QLatin1String( "b" ) )
    {
      bool isArray = fieldTypeName.startsWith( '_' );

      if ( isArray )
        fieldTypeName = fieldTypeName.mid( 1 );

      if ( fieldTypeName == QLatin1String( "int8" ) || fieldTypeName == QLatin1String( "serial8" ) )
      {
        fieldType = QVariant::LongLong;
        fieldSize = -1;
        fieldPrec = 0;
      }
      else if ( fieldTypeName == QLatin1String( "int2" ) || fieldTypeName == QLatin1String( "int4" ) ||
                fieldTypeName == QLatin1String( "oid" ) || fieldTypeName == QLatin1String( "serial" ) )
      {
        fieldType = QVariant::Int;
        fieldSize = -1;
        fieldPrec = 0;
      }
      else if ( fieldTypeName == QLatin1String( "real" ) || fieldTypeName == QLatin1String( "double precision" ) ||
                fieldTypeName == QLatin1String( "float4" ) || fieldTypeName == QLatin1String( "float8" ) )
      {
        fieldType = QVariant::Double;
        fieldSize = -1;
        fieldPrec = 0;
      }
      else if ( fieldTypeName == QLatin1String( "numeric" ) )
      {
        fieldType = QVariant::Double;

        if ( formattedFieldType == QLatin1String( "numeric" ) || formattedFieldType.isEmpty() )
        {
          fieldSize = -1;
          fieldPrec = 0;
        }
        else
        {
          QRegularExpression re( QRegularExpression::anchoredPattern( QStringLiteral( "numeric\\((\\d+),(\\d+)\\)" ) ) );
          const QRegularExpressionMatch match = re.match( formattedFieldType );
          if ( match.hasMatch() )
          {
            fieldSize = match.captured( 1 ).toInt();
            fieldPrec = match.captured( 2 ).toInt();
          }
          else if ( formattedFieldType != QLatin1String( "numeric" ) )
          {
            QgsMessageLog::logMessage( tr( "Unexpected formatted field type '%1' for field %2" )
                                       .arg( formattedFieldType,
                                             fieldName ),
                                       tr( "PostGIS" ) );
            fieldSize = -1;
            fieldPrec = 0;
          }
        }
      }
      else if ( fieldTypeName == QLatin1String( "varchar" ) )
      {
        fieldType = QVariant::String;

        const QRegularExpression re( QRegularExpression::anchoredPattern( QStringLiteral( "character varying\\((\\d+)\\)" ) ) );
        const QRegularExpressionMatch match = re.match( formattedFieldType );
        if ( match.hasMatch() )
        {
          fieldSize = match.captured( 1 ).toInt();
        }
        else
        {
          fieldSize = -1;
        }
      }
      else if ( fieldTypeName == QLatin1String( "date" ) )
      {
        fieldType = QVariant::Date;
        fieldSize = -1;
      }
      else if ( fieldTypeName == QLatin1String( "time" ) )
      {
        fieldType = QVariant::Time;
        fieldSize = -1;
      }
      else if ( fieldTypeName == QLatin1String( "timestamp" ) )
      {
        fieldType = QVariant::DateTime;
        fieldSize = -1;
      }
      else if ( fieldTypeName == QLatin1String( "bytea" ) )
      {
        fieldType = QVariant::ByteArray;
        fieldSize = -1;
      }
      else if ( fieldTypeName == QLatin1String( "text" ) ||
                fieldTypeName == QLatin1String( "citext" ) ||
                fieldTypeName == QLatin1String( "geometry" ) ||
                fieldTypeName == QLatin1String( "inet" ) ||
                fieldTypeName == QLatin1String( "money" ) ||
                fieldTypeName == QLatin1String( "ltree" ) ||
                fieldTypeName == QLatin1String( "uuid" ) ||
                fieldTypeName == QLatin1String( "xml" ) ||
                fieldTypeName.startsWith( QLatin1String( "time" ) ) ||
                fieldTypeName.startsWith( QLatin1String( "date" ) ) )
      {
        fieldType = QVariant::String;
        fieldSize = -1;
      }
      else if ( fieldTypeName == QLatin1String( "bpchar" ) )
      {
        // although postgres internally uses "bpchar", this is exposed to users as character in postgres
        fieldTypeName = QStringLiteral( "character" );

        fieldType = QVariant::String;

        const QRegularExpression re( QRegularExpression::anchoredPattern( QStringLiteral( "character\\((\\d+)\\)" ) ) );
        const QRegularExpressionMatch match = re.match( formattedFieldType );
        if ( match.hasMatch() )
        {
          fieldSize = match.captured( 1 ).toInt();
        }
        else
        {
          QgsDebugMsg( QStringLiteral( "Unexpected formatted field type '%1' for field %2" )
                       .arg( formattedFieldType,
                             fieldName ) );
          fieldSize = -1;
          fieldPrec = 0;
        }
      }
      else if ( fieldTypeName == QLatin1String( "char" ) )
      {
        fieldType = QVariant::String;

        const QRegularExpression re( QRegularExpression::anchoredPattern( QStringLiteral( "char\\((\\d+)\\)" ) ) );
        const QRegularExpressionMatch match = re.match( formattedFieldType );
        if ( match.hasMatch() )
        {
          fieldSize = match.captured( 1 ).toInt();
        }
        else
        {
          QgsMessageLog::logMessage( tr( "Unexpected formatted field type '%1' for field %2" )
                                     .arg( formattedFieldType,
                                           fieldName ) );
          fieldSize = -1;
          fieldPrec = 0;
        }
      }
      else if ( fieldTypeName == QLatin1String( "hstore" ) ||  fieldTypeName == QLatin1String( "json" ) || fieldTypeName == QLatin1String( "jsonb" ) )
      {
        fieldType = QVariant::Map;
        fieldSubType = QVariant::String;
        fieldSize = -1;
      }
      else if ( fieldTypeName == QLatin1String( "bool" ) )
      {
        // enum
        fieldType = QVariant::Bool;
        fieldSize = -1;
      }
      else
      {
        // be tolerant in case of views: this might be a field used as a key
        const QgsPostgresProvider::Relkind type = relkind();
        if ( ( type == QgsPostgresProvider::Relkind::View || type == QgsPostgresProvider::Relkind::MaterializedView )
             && parseUriKey( mUri.keyColumn( ) ).contains( fieldName ) )
        {
          // Assume it is convertible to text
          fieldType = QVariant::String;
          fieldSize = -1;
        }
        else
        {
          QgsMessageLog::logMessage( tr( "Field %1 ignored, because of unsupported type %2" ).arg( fieldName, fieldTType ), tr( "PostGIS" ) );
          continue;
        }
      }

      if ( isArray )
      {
        fieldTypeName = '_' + fieldTypeName;
        fieldSubType = fieldType;
        fieldType = ( fieldType == QVariant::String ? QVariant::StringList : QVariant::List );
        fieldSize = -1;
      }
    }
    else if ( fieldTType == QLatin1String( "e" ) )
    {
      // enum
      fieldType = QVariant::String;
      fieldSize = -1;
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Field %1 ignored, because of unsupported type %2" ).arg( fieldName, fieldTType ), tr( "PostGIS" ) );
      continue;
    }

    if ( fields.contains( fieldName ) )
    {
      QgsMessageLog::logMessage( tr( "Duplicate field %1 found\n" ).arg( fieldName ), tr( "PostGIS" ) );
      return false;
    }

    fields << fieldName;

    if ( isDomain )
    {
      //field was defined using domain, so use domain type name for fieldTypeName
      fieldTypeName = originalFormattedFieldType;
    }

    // If this is an identity field with constraints and there is no default, let's look for a sequence:
    // we might have a default value created by a sequence named <table>_<field>_seq
    if ( ! identityMap[tableoid ][ attnum ].isEmpty()
         && notNullMap[tableoid][ attnum ]
         && uniqueMap[tableoid][attnum]
         && defValMap[tableoid][attnum].isEmpty() )
    {
      const QString seqName { mTableName + '_' + fieldName + QStringLiteral( "_seq" ) };
      const QString seqSql = QStringLiteral( "SELECT c.oid "
                                             "  FROM pg_class c "
                                             "  LEFT JOIN pg_namespace n "
                                             "    ON ( n.oid = c.relnamespace ) "
                                             "  WHERE c.relkind = 'S' "
                                             "    AND c.relname = %1 "
                                             "    AND n.nspname = %2" )
                             .arg( quotedValue( seqName ),
                                   quotedValue( mSchemaName ) );
      QgsPostgresResult seqResult( connectionRO()->PQexec( seqSql ) );
      if ( seqResult.PQntuples() == 1 )
      {
        defValMap[tableoid][attnum] = QStringLiteral( "nextval(%1::regclass)" ).arg( quotedIdentifier( seqName ) );
      }
    }

    mDefaultValues.insert( mAttributeFields.size(), defValMap[tableoid][attnum] );

    QgsField newField = QgsField( fieldName, fieldType, fieldTypeName, fieldSize, fieldPrec, fieldComment, fieldSubType );

    QgsFieldConstraints constraints;
    if ( notNullMap[tableoid][attnum] || ( mPrimaryKeyAttrs.size() == 1 && mPrimaryKeyAttrs[0] == fieldName ) || identityMap[tableoid][attnum] != ' ' )
      constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
    if ( uniqueMap[tableoid][attnum] || ( mPrimaryKeyAttrs.size() == 1 && mPrimaryKeyAttrs[0] == fieldName ) || identityMap[tableoid][attnum] != ' ' )
      constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
    newField.setConstraints( constraints );

    mIdentityFields.insert( mAttributeFields.size(), identityMap[tableoid][attnum][0].toLatin1() );
    mAttributeFields.append( newField );
  }

  return true;
}

/* static */
QStringList QgsPostgresRasterProvider::parseUriKey( const QString &key )
{
  if ( key.isEmpty() ) return QStringList();

  QStringList cols;

  // remove quotes from key list
  if ( key.startsWith( '"' ) && key.endsWith( '"' ) )
  {
    int i = 1;
    QString col;
    while ( i < key.size() )
    {
      if ( key[i] == '"' )
      {
        if ( i + 1 < key.size() && key[i + 1] == '"' )
        {
          i++;
        }
        else
        {
          cols << col;
          col.clear();

          if ( ++i == key.size() )
            break;

          Q_ASSERT( key[i] == ',' );
          i++;
          Q_ASSERT( key[i] == '"' );
          i++;
          col.clear();
          continue;
        }
      }

      col += key[i++];
    }
  }
  else if ( key.contains( ',' ) )
  {
    cols = key.split( ',' );
  }
  else
  {
    cols << key;
  }

  return cols;
}

QgsPostgresProvider::Relkind QgsPostgresRasterProvider::relkind() const
{
  if ( mIsQuery || !connectionRO() )
    return QgsPostgresProvider::Relkind::Unknown;

  QString sql = QStringLiteral( "SELECT relkind FROM pg_class WHERE oid=regclass(%1)::oid" ).arg( quotedValue( mQuery ) );
  QgsPostgresResult res( connectionRO()->PQexec( sql ) );
  QString type = res.PQgetvalue( 0, 0 );

  QgsPostgresProvider::Relkind kind = QgsPostgresProvider::Relkind::Unknown;

  if ( type == 'r' )
  {
    kind = QgsPostgresProvider::Relkind::OrdinaryTable;
  }
  else if ( type == 'i' )
  {
    kind = QgsPostgresProvider::Relkind::Index;
  }
  else if ( type == 's' )
  {
    kind = QgsPostgresProvider::Relkind::Sequence;
  }
  else if ( type == 'v' )
  {
    kind = QgsPostgresProvider::Relkind::View;
  }
  else if ( type == 'm' )
  {
    kind = QgsPostgresProvider::Relkind::MaterializedView;
  }
  else if ( type == 'c' )
  {
    kind = QgsPostgresProvider::Relkind::CompositeType;
  }
  else if ( type == 't' )
  {
    kind = QgsPostgresProvider::Relkind::ToastTable;
  }
  else if ( type == 'f' )
  {
    kind = QgsPostgresProvider::Relkind::ForeignTable;
  }
  else if ( type == 'p' )
  {
    kind = QgsPostgresProvider::Relkind::PartitionedTable;
  }

  return kind;
}

bool QgsPostgresRasterProvider::determinePrimaryKey()
{

  // check to see if there is an unique index on the relation, which
  // can be used as a key into the table. Primary keys are always
  // unique indices, so we catch them as well.

  QString sql;

  mPrimaryKeyAttrs.clear();

  if ( !mIsQuery )
  {
    sql = QStringLiteral( "SELECT count(*) FROM pg_inherits WHERE inhparent=%1::regclass" ).arg( quotedValue( mQuery ) );
    QgsDebugMsgLevel( QStringLiteral( "Checking whether %1 is a parent table" ).arg( sql ), 4 );
    QgsPostgresResult res( connectionRO()->PQexec( sql ) );
    bool isParentTable( res.PQntuples() == 0 || res.PQgetvalue( 0, 0 ).toInt() > 0 );

    sql = QStringLiteral( "SELECT indexrelid FROM pg_index WHERE indrelid=%1::regclass AND (indisprimary OR indisunique) ORDER BY CASE WHEN indisprimary THEN 1 ELSE 2 END LIMIT 1" ).arg( quotedValue( mQuery ) );
    QgsDebugMsgLevel( QStringLiteral( "Retrieving first primary or unique index: %1" ).arg( sql ), 4 );

    res = connectionRO()->PQexec( sql );
    QgsDebugMsgLevel( QStringLiteral( "Got %1 rows." ).arg( res.PQntuples() ), 4 );

    QStringList log;

    // no primary or unique indices found
    if ( res.PQntuples() == 0 )
    {
      QgsDebugMsgLevel( QStringLiteral( "Relation has no primary key -- investigating alternatives" ), 4 );

      // Two options here. If the relation is a table, see if there is
      // an oid column that can be used instead.
      // If the relation is a view try to find a suitable column to use as
      // the primary key.

      const QgsPostgresProvider::Relkind type = relkind();

      if ( type == QgsPostgresProvider::Relkind::OrdinaryTable || type == QgsPostgresProvider::Relkind::PartitionedTable )
      {
        QgsDebugMsgLevel( QStringLiteral( "Relation is a table. Checking to see if it has an oid column." ), 4 );

        mPrimaryKeyAttrs.clear();
        mPrimaryKeyType = PktUnknown;

        if ( connectionRO()->pgVersion() >= 100000 )
        {
          // If there is an generated id on the table, use that instead,
          sql = QStringLiteral( "SELECT attname FROM pg_attribute WHERE attidentity IN ('a','d') AND attrelid=regclass(%1) LIMIT 1" ).arg( quotedValue( mQuery ) );
          res = connectionRO()->PQexec( sql );
          if ( res.PQntuples() == 1 )
          {
            // Could warn the user here that performance will suffer if
            // attribute isn't indexed (and that they may want to add a
            // primary key to the table)
            mPrimaryKeyAttrs << res.PQgetvalue( 0, 0 );
          }
        }

        if ( mPrimaryKeyType == PktUnknown )
        {
          // If there is an oid on the table, use that instead,
          sql = QStringLiteral( "SELECT attname FROM pg_attribute WHERE attname='oid' AND attrelid=regclass(%1)" ).arg( quotedValue( mQuery ) );

          res = connectionRO()->PQexec( sql );
          if ( res.PQntuples() == 1 )
          {
            // Could warn the user here that performance will suffer if
            // oid isn't indexed (and that they may want to add a
            // primary key to the table)
            mPrimaryKeyType = PktOid;
            mPrimaryKeyAttrs << QStringLiteral( "oid" );
          }
        }

        if ( mPrimaryKeyType == PktUnknown )
        {
          sql = QStringLiteral( "SELECT attname FROM pg_attribute WHERE attname='ctid' AND attrelid=regclass(%1)" ).arg( quotedValue( mQuery ) );

          res = connectionRO()->PQexec( sql );
          if ( res.PQntuples() == 1 )
          {
            mPrimaryKeyType = PktTid;
            QgsMessageLog::logMessage( tr( "Primary key is ctid - changing of existing features disabled (%1; %2)" ).arg( mRasterColumn, mQuery ) );
            // TODO: set capabilities to RO when writing will be implemented
            mPrimaryKeyAttrs << QStringLiteral( "ctid" );
          }
        }

        if ( mPrimaryKeyType == PktUnknown )
        {
          QgsMessageLog::logMessage( tr( "The table has no column suitable for use as a key. QGIS requires a primary key, a PostgreSQL oid column or a ctid for tables." ), tr( "PostGIS" ) );
        }
      }
      else if ( type == QgsPostgresProvider::Relkind::View || type == QgsPostgresProvider::Relkind::MaterializedView
                || type == QgsPostgresProvider::Relkind::ForeignTable )
      {
        determinePrimaryKeyFromUriKeyColumn();
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Unexpected relation type." ), tr( "PostGIS" ) );
      }
    }
    else
    {
      // have a primary key or unique index
      QString indrelid = res.PQgetvalue( 0, 0 );
      sql = QStringLiteral( "SELECT attname, attnotnull, data_type FROM pg_index, pg_attribute "
                            "JOIN information_schema.columns ON (column_name = attname AND table_name = %1 AND table_schema = %2) "
                            "WHERE indexrelid=%3 AND indrelid=attrelid AND pg_attribute.attnum=any(pg_index.indkey)" )
            .arg( quotedValue( mTableName ) )
            .arg( quotedValue( mSchemaName ) )
            .arg( indrelid );

      QgsDebugMsgLevel( "Retrieving key columns: " + sql, 4 );
      res = connectionRO()->PQexec( sql );
      QgsDebugMsgLevel( QStringLiteral( "Got %1 rows." ).arg( res.PQntuples() ), 4 );

      bool mightBeNull = false;
      QString primaryKey;
      QString delim;

      mPrimaryKeyType = PktFidMap; // map by default, will downgrade if needed
      for ( int i = 0; i < res.PQntuples(); i++ )
      {
        const QString name = res.PQgetvalue( i, 0 );
        if ( res.PQgetvalue( i, 1 ).startsWith( 'f' ) )
        {
          QgsMessageLog::logMessage( tr( "Unique column '%1' doesn't have a NOT NULL constraint." ).arg( name ), tr( "PostGIS" ) );
          mightBeNull = true;
        }

        primaryKey += delim + quotedIdentifier( name );
        delim = ',';

        QgsPostgresPrimaryKeyType pkType { QgsPostgresPrimaryKeyType::PktUnknown };
        const QString fieldTypeName { res.PQgetvalue( i, 2 ) };

        if ( fieldTypeName == QLatin1String( "oid" ) )
        {
          pkType = QgsPostgresPrimaryKeyType::PktOid;
        }
        else if ( fieldTypeName == QLatin1String( "integer" ) )
        {
          pkType = QgsPostgresPrimaryKeyType::PktInt;
        }
        else if ( fieldTypeName == QLatin1String( "bigint" ) )
        {
          pkType = QgsPostgresPrimaryKeyType::PktUint64;
        }
        else if ( fieldTypeName == QLatin1String( "text" ) )
        {
          pkType = QgsPostgresPrimaryKeyType::PktFidMap;
        }
        // Always use PktFidMap for multi-field keys
        mPrimaryKeyType = i ? QgsPostgresPrimaryKeyType::PktFidMap : pkType;
        mPrimaryKeyAttrs << name;
      }

      if ( mightBeNull || isParentTable )
      {
        QgsMessageLog::logMessage( tr( "Ignoring key candidate because of NULL values or inherited table" ), tr( "PostGIS" ) );
        mPrimaryKeyType = PktUnknown;
        mPrimaryKeyAttrs.clear();
      }
    }
  }
  else
  {
    determinePrimaryKeyFromUriKeyColumn();
  }

  if ( mPrimaryKeyAttrs.size() == 0 )
  {
    QgsMessageLog::logMessage( tr( "Could not find a primary key for PostGIS raster table %1" ).arg( mQuery ), tr( "PostGIS" ) );
    mPrimaryKeyType = PktUnknown;
  }

  return mPrimaryKeyType != PktUnknown;
}



void QgsPostgresRasterProvider::determinePrimaryKeyFromUriKeyColumn()
{
  mPrimaryKeyAttrs.clear();
  const QString keyCandidate {  mUri.keyColumn() };
  QgsPostgresPrimaryKeyType pkType { QgsPostgresPrimaryKeyType::PktUnknown };
  const QString sql = QStringLiteral( "SELECT data_type FROM information_schema.columns "
                                      "WHERE column_name = %1 AND table_name = %2 AND table_schema = %3" )
                      .arg( keyCandidate, mTableName,  mSchemaName );
  QgsPostgresResult result( connectionRO()->PQexec( sql ) );
  if ( PGRES_TUPLES_OK == result.PQresultStatus() )
  {
    const QString fieldTypeName { result.PQgetvalue( 0, 0 ) };

    if ( fieldTypeName == QLatin1String( "oid" ) )
    {
      pkType = QgsPostgresPrimaryKeyType::PktOid;
    }
    else if ( fieldTypeName == QLatin1String( "integer" ) )
    {
      pkType = QgsPostgresPrimaryKeyType::PktInt;
    }
    else if ( fieldTypeName == QLatin1String( "bigint" ) )
    {
      pkType = QgsPostgresPrimaryKeyType::PktUint64;
    }
    mPrimaryKeyAttrs.push_back( mUri.keyColumn() );
    mPrimaryKeyType = pkType;
  }
}

QString QgsPostgresRasterProvider::pkSql()
{
  Q_ASSERT_X( ! mPrimaryKeyAttrs.isEmpty(), "QgsPostgresRasterProvider::pkSql()",  "No PK is defined!" );
  if ( mPrimaryKeyAttrs.count( ) > 1 )
  {
    QStringList pkeys;
    for ( const QString &k : std::as_const( mPrimaryKeyAttrs ) )
    {
      pkeys.push_back( quotedIdentifier( k ) );
    }
    return pkeys.join( ',' ).prepend( '(' ).append( ')' );
  }
  return quotedIdentifier( mPrimaryKeyAttrs.first() );
}

QString QgsPostgresRasterProvider::dataComment() const
{
  return mDataComment;
}

void QgsPostgresRasterProvider::findOverviews()
{
  const QString sql = QStringLiteral( "SELECT overview_factor, o_table_schema, o_table_name, o_raster_column "
                                      "FROM raster_overviews WHERE r_table_schema = %1 AND r_table_name = %2" ).arg( quotedValue( mSchemaName ),
                                          quotedValue( mTableName ) );

  //QgsDebugMsg( QStringLiteral( "Raster overview information sql: %1" ).arg( sql ) );
  QgsPostgresResult result( connectionRO()->PQexec( sql ) );
  if ( PGRES_TUPLES_OK == result.PQresultStatus() )
  {
    for ( int i = 0; i < result.PQntuples(); ++i )
    {
      bool ok;
      const unsigned int overViewFactor { static_cast< unsigned int>( result.PQgetvalue( i, 0 ).toInt( & ok ) ) };
      if ( ! ok )
      {
        QgsMessageLog::logMessage( tr( "Cannot convert overview factor '%1' to int" ).arg( result.PQgetvalue( i, 0 ) ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
        return;
      }
      const QString schema { result.PQgetvalue( i, 1 ) };
      const QString table { result.PQgetvalue( i, 2 ) };
      if ( table.isEmpty() || schema.isEmpty() )
      {
        QgsMessageLog::logMessage( tr( "Table or schema is empty" ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
        return;
      }
      mOverViews[ overViewFactor ] = QStringLiteral( "%1.%2" ).arg( quotedIdentifier( schema ) ).arg( quotedIdentifier( table ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Error fetching overviews information: %1" ).arg( result.PQresultErrorMessage() ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
  }
  if ( mOverViews.isEmpty() )
  {
    QgsMessageLog::logMessage( tr( "No overviews found, performances may be affected for %1" ).arg( mQuery ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Info );
  }
}

int QgsPostgresRasterProvider::xSize() const
{
  return static_cast<int>( mWidth );
}

int QgsPostgresRasterProvider::ySize() const
{
  return static_cast<int>( mHeight );
}

Qgis::DataType QgsPostgresRasterProvider::sourceDataType( int bandNo ) const
{
  if ( bandNo <= mBandCount && static_cast<unsigned long>( bandNo ) <= mDataTypes.size() )
  {
    return mDataTypes[ static_cast<unsigned long>( bandNo - 1 ) ];
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Data type is unknown" ), QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
    return Qgis::DataType::UnknownDataType;
  }
}

int QgsPostgresRasterProvider::xBlockSize() const
{
  if ( mInput )
  {
    return mInput->xBlockSize();
  }
  else
  {
    return static_cast<int>( mWidth );
  }
}

int QgsPostgresRasterProvider::yBlockSize() const
{
  if ( mInput )
  {
    return mInput->yBlockSize();
  }
  else
  {
    return static_cast<int>( mHeight );
  }
}

QgsRasterBandStats QgsPostgresRasterProvider::bandStatistics( int bandNo, int stats, const QgsRectangle &extent, int sampleSize, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( feedback )
  QgsRasterBandStats rasterBandStats;
  const auto constMStatistics = mStatistics;
  initStatistics( rasterBandStats, bandNo, stats, extent, sampleSize );
  for ( const QgsRasterBandStats &stats : constMStatistics )
  {
    if ( stats.contains( rasterBandStats ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "Using cached statistics." ), 4 );
      return stats;
    }
  }

  QString tableToQuery { mQuery };
  const double pixelsRatio { static_cast<double>( sampleSize ) / ( mWidth * mHeight ) };
  double statsRatio { pixelsRatio };

  // Decide if overviews can be used here
  if ( subsetString().isEmpty() && ! mIsQuery && mIsTiled && extent.isEmpty() )
  {
    const unsigned int desiredOverviewFactor { static_cast<unsigned int>( 1.0 / sqrt( pixelsRatio ) ) };
    const auto ovKeys { mOverViews.keys( ) };
    QList<unsigned int>::const_reverse_iterator rit { ovKeys.rbegin() };
    for ( ; rit != ovKeys.rend(); ++rit )
    {
      if ( *rit <= desiredOverviewFactor )
      {
        tableToQuery = mOverViews[ *rit ];
        // This should really be: *= *rit * *rit;
        // but we are already approximating, let's get decent statistics
        statsRatio = 1;
        QgsDebugMsgLevel( QStringLiteral( "Using overview for statistics read: %1" ).arg( tableToQuery ), 3 );
        break;
      }
    }
  }

  // Query the backend
  QString where { extent.isEmpty() ? QString() : QStringLiteral( "WHERE %1 && ST_GeomFromText( %2, %3 )" )
                  .arg( quotedIdentifier( mRasterColumn ) )
                  .arg( quotedValue( extent.asWktPolygon() ) )
                  .arg( mCrs.postgisSrid() ) };

  if ( ! subsetString().isEmpty() )
  {
    where.append( where.isEmpty() ? QStringLiteral( "WHERE %1" ).arg( subsetString() ) :
                  QStringLiteral( " AND %1" ).arg( subsetString() ) );
  }

  const QString sql = QStringLiteral( "SELECT (ST_SummaryStatsAgg( %1, %2, TRUE, %3 )).* "
                                      "FROM %4 %5" ).arg( quotedIdentifier( mRasterColumn ) )
                      .arg( bandNo )
                      .arg( std::max<double>( 0, std::min<double>( 1, statsRatio ) ) )
                      .arg( tableToQuery, where );

  QgsPostgresResult result( connectionRO()->PQexec( sql ) );

  if ( PGRES_TUPLES_OK == result.PQresultStatus() && result.PQntuples() == 1 )
  {
    // count   |     sum     |       mean       |      stddev      | min | max
    rasterBandStats.sum = result.PQgetvalue( 0, 1 ).toDouble( );
    rasterBandStats.mean = result.PQgetvalue( 0, 2 ).toDouble( );
    rasterBandStats.stdDev = result.PQgetvalue( 0, 3 ).toDouble( );
    rasterBandStats.minimumValue = result.PQgetvalue( 0, 4 ).toDouble( );
    rasterBandStats.maximumValue = result.PQgetvalue( 0, 5 ).toDouble( );
    rasterBandStats.range = rasterBandStats.maximumValue - rasterBandStats.minimumValue;
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Error fetching statistics for %1: %2\nSQL: %3" )
                               .arg( mQuery )
                               .arg( result.PQresultErrorMessage() )
                               .arg( sql ),
                               QStringLiteral( "PostGIS" ), Qgis::MessageLevel::Warning );
  }

  QgsDebugMsgLevel( QStringLiteral( "************ STATS **************" ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "MIN %1" ).arg( rasterBandStats.minimumValue ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "MAX %1" ).arg( rasterBandStats.maximumValue ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "RANGE %1" ).arg( rasterBandStats.range ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "MEAN %1" ).arg( rasterBandStats.mean ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "STDDEV %1" ).arg( rasterBandStats.stdDev ), 4 );

  mStatistics.append( rasterBandStats );
  return rasterBandStats;
}


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsPostgresRasterProviderMetadata();
}
#endif


QgsPostgresRasterProviderException::QgsPostgresRasterProviderException( const QString &msg )
  : message( msg )
{}

QgsFields QgsPostgresRasterProvider::fields() const
{
  return mAttributeFields;
}
