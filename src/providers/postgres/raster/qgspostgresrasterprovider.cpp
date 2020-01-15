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

const QString QgsPostgresRasterProvider::PG_RASTER_PROVIDER_KEY = QStringLiteral( "postgresraster" );
const QString QgsPostgresRasterProvider::PG_RASTER_PROVIDER_DESCRIPTION =  QStringLiteral( "Postgres raster provider" );


QgsPostgresRasterProvider::QgsPostgresRasterProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions )
  : QgsRasterDataProvider( uri, providerOptions )
  , mShared( new QgsPostgresRasterSharedData )
{
  mUri = QgsDataSourceUri( uri );

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

  if ( !init() ) // gets srid and data type
  {
    // the table is not a raster table
    QgsMessageLog::logMessage( tr( "Invalid PostgreSQL raster layer" ), tr( "PostGIS" ) );
    disconnectDb();
    return;
  }

  // Check if requested srid and detected srid match
  if ( ! mDetectedSrid.isEmpty() && mRequestedSrid != mDetectedSrid )
  {
    QgsMessageLog::logMessage( tr( "Requested SRID (%1) and detected SRID (%2) differ" )
                               .arg( mRequestedSrid )
                               .arg( mDetectedSrid ),
                               QStringLiteral( "PostGIS" ), Qgis::Warning );
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
  , mDetectedSrid( other.mDetectedSrid )
  , mRequestedSrid( other.mRequestedSrid )
  , mConnectionRO( other.mConnectionRO )
  , mConnectionRW( other.mConnectionRW )
  , mPrimaryKeyType( other.mPrimaryKeyType )
  , mPrimaryKeyAttrs( other.mPrimaryKeyAttrs )
  , mShared( other.mShared )
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
    QgsMessageLog::logMessage( tr( "Invalid band number '%1" ).arg( bandNo ), QStringLiteral( "PostGIS" ), Qgis::Critical );
    return false;
  }

  const QgsRectangle rasterExtent = viewExtent.intersect( mExtent );
  if ( rasterExtent.isEmpty() )
  {
    QgsMessageLog::logMessage( tr( "Requested extent is not valid" ), QStringLiteral( "PostGIS" ), Qgis::Critical );
    return false;
  }

  const bool isSingleValue {  width == 1 && height == 1 };
  QString tableToQuery { mQuery };

  QString whereAnd;
  if ( ! mSqlWhereClause.isEmpty() )
  {
    whereAnd = QStringLiteral( "%1 AND " ).arg( mSqlWhereClause );
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
    const Qgis::DataType dataType { mDataTypes[ static_cast<size_t>( bandNo - 1 ) ] };
    switch ( dataType )
    {
      case Qgis::DataType::Byte:
      {
        const unsigned short byte { val.toUShort( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to byte" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
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
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to unsigned int" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
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
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to unsigned long" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
          return false;
        }
        std::memcpy( data, &ulong, sizeof( unsigned long ) );
        break;
      }
      case Qgis::DataType::Int16:
      {
        const int _int { val.toInt( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to unsigned long" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
          return false;
        }
        std::memcpy( data, &_int, sizeof( int ) );
        break;
      }
      case Qgis::DataType::Int32:
      {
        const long _long { val.toInt( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to long" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
          return false;
        }
        std::memcpy( data, &_long, sizeof( long ) );
        break;
      }
      case Qgis::DataType::Float32:
      {
        const float _float { val.toFloat( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to float" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
          return false;
        }
        std::memcpy( data, &_float, sizeof( float ) );
        break;
      }
      case Qgis::DataType::Float64:
      {
        const double _double { val.toDouble( &ok ) };
        if ( ! ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to double" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
          return false;
        }
        std::memcpy( data, &_double, sizeof( double ) );
        break;
      }
      default:
      {
        QgsMessageLog::logMessage( tr( "Unknown identified data type" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
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
    if ( mSqlWhereClause.isEmpty() )
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
    const QString dataSql { QStringLiteral( "SELECT %3, ENCODE( ST_AsBinary( %1, TRUE ), 'hex') "
                                            "FROM %2 WHERE %4 %3 IN " )
                            .arg( quotedIdentifier( mRasterColumn ) )
                            .arg( tableToQuery )
                            .arg( quotedIdentifier( pkSql() ) )
                            .arg( whereAnd )};

    const QString indexSql { QStringLiteral( "SELECT %1, (ST_Metadata( %2 )).* FROM %3 "
                             "WHERE %6 %2 && ST_GeomFromText( %5, %4 )" )
                             .arg( quotedIdentifier( pkSql() ) )
                             .arg( quotedIdentifier( mRasterColumn ) )
                             .arg( tableToQuery )
                             .arg( mCrs.postgisSrid() )
                             .arg( quotedValue( QStringLiteral( "###__POLYGON_WKT__###" ) ) )
                             .arg( whereAnd )
                           };


    const QgsPostgresRasterSharedData::TilesRequest tilesRequest
    {
      bandNo,
      rasterExtent,
      overviewFactor,
      indexSql,
      dataSql,
      connectionRO()
    };

    const QgsPostgresRasterSharedData::TilesResponse tileResponse
    {
      mShared->tiles( tilesRequest )
    };

    if ( tileResponse.tiles.isEmpty() )
    {
      QgsMessageLog::logMessage( tr( "No tiles available in table %1 for the requested extent: %2" )
                                 .arg( tableToQuery, rasterExtent.toString( ) ), tr( "PostGIS" ), Qgis::Critical );
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
                                   .arg( tableToQuery ), tr( "PostGIS" ), Qgis::Critical );
        return false;
      }
    }

    // Write tiles to the temporary raster
    CPLErrorReset();
    for ( auto &tile : qgis::as_const( tileResponse.tiles ) )
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
                                   .arg( tableToQuery, lastError ), tr( "PostGIS" ), Qgis::Critical );
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
    if ( !  dstDS )
    {
      const QString lastError = QString::fromUtf8( CPLGetLastErrorMsg() ) ;
      QgsMessageLog::logMessage( tr( "Unable to create destination raster for tiles from %1: %2" )
                                 .arg( tableToQuery, lastError ), tr( "PostGIS" ), Qgis::Critical );
      return false;
    }

    // Resample the raster to the final bounds and resolution
    QgsGdalUtils::resampleSingleBandRaster( tmpDS.get(), dstDS.get(), GDALResampleAlg::GRA_NearestNeighbour );

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
                                 .arg( mQuery, lastError ), tr( "PostGIS" ), Qgis::Critical );
      return false;
    }

#if 0
    GDALGetRasterStatistics( GDALGetRasterBand( dstDS.get(), 1 ), 0, 1, &pdfMin, &pdfMax, &pdfMean, &pdfStdDev );
    qDebug() << pdfMin << pdfMax << pdfMean << pdfStdDev;

    // Spit it out float data
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
  if ( mDataTypes.size() < static_cast<unsigned long>( bandNo ) )
  {
    QgsMessageLog::logMessage( tr( "Data type size for band %1 could not be found: num bands is: %2 and the type size map for bands contains: %3 items" )
                               .arg( bandNo )
                               .arg( mBandCount )
                               .arg( mDataSizes.size() ),
                               QStringLiteral( "PostGIS" ), Qgis::Warning );
    return Qgis::DataType::UnknownDataType;
  }
  // Band is 1-based
  return mDataTypes[ static_cast<unsigned long>( bandNo ) - 1 ];
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
      result += QStringLiteral( "</ul></td></tr>" );
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
    { tr( "Where Clause SQL" ), mSqlWhereClause },
    { tr( "Pixel Size" ), QStringLiteral( "%1, %2" ).arg( mScaleX ).arg( mScaleY ) },
    { tr( "Overviews" ),  overviews },
    { tr( "Primary Keys SQL" ),  pkSql() },
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

bool QgsPostgresRasterProvider::init()
{

  // WARNING: multiple failure return points!

  if ( !determinePrimaryKey() )
  {
    QgsMessageLog::logMessage( tr( "PostgreSQL raster layer has no primary key." ), tr( "PostGIS" ) );
    return false;
  }

  // We first try to collect raster information using raster_columns information
  // unless:
  // - it is a query layer (unsupported at the moment)
  // - use estimated metadata is false
  // - there is a WHERE condition
  // If previous conditions are not met or the first method fail try to fetch information
  // directly from the raster data. This can be very slow.

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

  // ///////////////////////////////////////////////////////////////////
  // First method: get information from metadata
  if ( ! mIsQuery && mUseEstimatedMetadata && mSqlWhereClause.isEmpty() )
  {
    try
    {
      const QString sql { QStringLiteral( "SELECT r_raster_column, srid,"
                                          "num_bands, pixel_types, nodata_values, ST_AsBinary(extent), blocksize_x, blocksize_y,"
                                          "out_db, spatial_index, scale_x, scale_y, same_alignment,"
                                          "regular_blocking "
                                          "FROM raster_columns WHERE "
                                          "r_table_name = %1 AND r_table_schema = %2" )
                          .arg( quotedValue( mTableName ) )
                          .arg( quotedValue( mSchemaName ) )  };

      QgsPostgresResult result( connectionRO()->PQexec( sql ) );

      if ( ( PGRES_TUPLES_OK == result.PQresultStatus() ) && ( result.PQntuples() > 0 ) )
      {
        mRasterColumn = result.PQgetvalue( 0, 0 );
        mHasSpatialIndex = result.PQgetvalue( 0, 9 ) == 't';
        bool ok;

        mCrs = QgsCoordinateReferenceSystem::fromEpsgId( result.PQgetvalue( 0, 1 ).toLong( &ok ) );

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
        for ( const QString &t : qgis::as_const( pxTypes ) )
        {
          Qgis::DataType type { pixelTypeFromString( t ) };
          if ( t == Qgis::DataType::UnknownDataType )
          {
            throw QgsPostgresRasterProviderException( tr( "Unsupported data type: '%1'" ).arg( t ) );
          }
          mDataTypes.push_back( type );
          mDataSizes.push_back( QgsRasterBlock::typeSize( type ) );
          double nodataValue { noDataValues.at( i ).toDouble( &ok ) };
          if ( ! ok )
          {
            if ( noDataValues.at( i ) != QStringLiteral( "NULL" ) )
            {
              QgsMessageLog::logMessage( tr( "Cannot convert nodata value '%1' to double" )
                                         .arg( noDataValues.at( i ) ),
                                         QStringLiteral( "PostGIS" ), Qgis::Info );
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
          mSrcNoDataValue.append( nodataValue );
          ++i;
        }

        // Extent
        QgsPolygon p;
        // Strip \x
        const QByteArray hexAscii { result.PQgetvalue( 0, 5 ).toAscii().mid( 2 ) };
        QgsConstWkbPtr ptr { QByteArray::fromHex( hexAscii ) };

        if ( ! p.fromWkb( ptr ) )
        {
          throw QgsPostgresRasterProviderException( tr( "Cannot get extent from raster_columns" ) );
        }

        mExtent = p.boundingBox();

        // Size
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
        mHeight = static_cast<long>( mExtent.height() / std::abs( mScaleY ) );
        mWidth = static_cast<long>( mExtent.width() / std::abs( mScaleX ) );
        // is tiled?
        mIsTiled = ( mWidth != mTileWidth ) || ( mHeight != mTileHeight );

        // Detect overviews
        findOverviews();

        return true;
      }
      else
      {
        QgsMessageLog::logMessage( tr( "An error occurred while fetching raster metadata for table %1: %2\nSQL: %3" )
                                   .arg( mQuery )
                                   .arg( result.PQresultErrorMessage() )
                                   .arg( sql ),
                                   QStringLiteral( "PostGIS" ), Qgis::Warning );
      }
    }
    catch ( QgsPostgresRasterProviderException &ex )
    {
      QgsMessageLog::logMessage( tr( "An error occurred while fetching raster metadata for %1, proceeding with (possibly very slow) raster data analysis: %2\n"
                                     "Please consider adding raster constraints with PostGIS function AddRasterConstraints." )
                                 .arg( mQuery )
                                 .arg( ex.message ),
                                 QStringLiteral( "PostGIS" ), Qgis::Warning );
    }
  }

  // TODO: query layers + mHasSpatialIndex in case metadata are not used

  // ///////////////////////////////////////////////////////////////////
  // Go the hard and slow way: fetch information directly from the layer
  //
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
        QgsMessageLog::logMessage( tr( "Multiple raster column detected, using the first one" ),
                                   QStringLiteral( "PostGIS" ), Qgis::Warning );

      }
      mRasterColumn = result.PQgetvalue( 0, 0 );
    }
    else
    {
      QgsMessageLog::logMessage( tr( "An error occurred while fetching raster column" ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
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
  if ( ! mSqlWhereClause.isEmpty() )
  {
    where = QStringLiteral( "WHERE %1" ).arg( mSqlWhereClause );
  }

  const QString sql { QStringLiteral( "SELECT ENCODE( ST_AsBinary( ST_Envelope( foo.bar) ), 'hex'), ( ST_Metadata( foo.bar ) ).* "
                                      "FROM ( SELECT ST_Union ( %1 ) AS bar FROM %2 %3) AS foo" )
                      .arg( quotedIdentifier( mRasterColumn ) )
                      .arg( tableToQuery )
                      .arg( where )};

  QgsDebugMsg( QStringLiteral( "Raster information sql: %1" ).arg( sql ) );

  QgsPostgresResult result( connectionRO()->PQexec( sql ) );
  if ( PGRES_TUPLES_OK == result.PQresultStatus() && result.PQntuples() > 0 )
  {
    bool ok;
    QgsPolygon p;

    // envelope | upperleftx | upperlefty | width | height | scalex | scaley | skewx | skewy | srid | numbands

    // Extent
    try
    {
      QgsConstWkbPtr ptr { QByteArray::fromHex( result.PQgetvalue( 0, 0 ).toAscii() ) };
      if ( ! p.fromWkb( ptr ) )
      {
        QgsMessageLog::logMessage( tr( "Cannot get extent from raster" ),
                                   QStringLiteral( "PostGIS" ), Qgis::Critical );
        return false;
      }
    }
    catch ( ... )
    {
      QgsMessageLog::logMessage( tr( "Cannot get metadata from raster" ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }

    mExtent = p.boundingBox();

    // Size
    mTileWidth = result.PQgetvalue( 0, 3 ).toInt( &ok );

    if ( ! ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot convert width '%1' to int" ).arg( result.PQgetvalue( 0, 3 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }

    mTileHeight = result.PQgetvalue( 0, 4 ).toInt( &ok );

    if ( ! ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot convert height '%1' to int" ).arg( result.PQgetvalue( 0, 4 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }

    mScaleX = result.PQgetvalue( 0, 5 ).toDouble( &ok );

    if ( ! ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot convert scale X '%1' to double" ).arg( result.PQgetvalue( 0, 5 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }

    mScaleY = result.PQgetvalue( 0, 6 ).toDouble( &ok );

    if ( ! ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot convert scale Y '%1' to double" ).arg( result.PQgetvalue( 0, 6 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }

    // Compute raster size
    mHeight = static_cast<long>( mExtent.height() / std::abs( mScaleY ) );
    mWidth = static_cast<long>( mExtent.width() / std::abs( mScaleX ) );
    mIsTiled = ( mWidth != mTileWidth ) || ( mHeight != mTileHeight );

    mCrs = QgsCoordinateReferenceSystem::fromEpsgId( result.PQgetvalue( 0, 9 ).toLong( &ok ) );

    if ( ! ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot create CRS from EPSG: '%1'" ).arg( result.PQgetvalue( 0, 9 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }

    mDetectedSrid = result.PQgetvalue( 0, 9 );
    mBandCount = result.PQgetvalue( 0, 10 ).toInt( &ok );
    if ( ! ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot convert band count '%1' to int" ).arg( result.PQgetvalue( 0, 10 ) ),
                                 QStringLiteral( "PostGIS" ), Qgis::Critical );
      return false;
    }

    // Fetch band data types
    for ( int band = 1; band <= mBandCount; ++band )
    {
      // pixeltype | nodatavalue | isoutdb | path
      const QString sql { QStringLiteral( "SELECT ( ST_BandMetadata( foo.bar, %1 ) ).* "
                                          "FROM ( SELECT ST_Union ( ST_Band ( %2, %1 ) ) AS bar FROM %3 %4 ) AS foo" )
                          .arg( band )
                          .arg( quotedIdentifier( mRasterColumn ) )
                          .arg( mQuery )
                          .arg( where )};

      QgsPostgresResult result( connectionRO()->PQexec( sql ) );
      if ( PGRES_TUPLES_OK == result.PQresultStatus() && result.PQntuples() > 0 )
      {

        Qgis::DataType type { pixelTypeFromString( result.PQgetvalue( 0, 0 ) ) };

        if ( type == Qgis::DataType::UnknownDataType )
        {
          QgsMessageLog::logMessage( tr( "Unsupported data type: '%1'" ).arg( result.PQgetvalue( 0, 0 ) ),
                                     QStringLiteral( "PostGIS" ), Qgis::Critical );
          return false;
        }

        mDataTypes.push_back( type );
        mDataSizes.push_back( QgsRasterBlock::typeSize( type ) );
        double nodataValue { result.PQgetvalue( 0, 1 ).toDouble( &ok ) };

        if ( ! ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert nodata value '%1' to double, default to: %2" )
                                     .arg( result.PQgetvalue( 0, 1 ) )
                                     .arg( std::numeric_limits<double>::min() ), QStringLiteral( "PostGIS" ), Qgis::Info );
          nodataValue = std::numeric_limits<double>::min();
        }

        mSrcNoDataValue.append( nodataValue );
        mSrcHasNoDataValue.append( true );
        mUseSrcNoDataValue.append( true );
        mIsOutOfDb = result.PQgetvalue( 0, 2 ) == 't';
      }
      else
      {
        QgsMessageLog::logMessage( tr( "An error occurred while fetching raster band metadata" ),
                                   QStringLiteral( "PostGIS" ), Qgis::Critical );
        return false;
      }
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "An error occurred while fetching raster metadata" ),
                               QStringLiteral( "PostGIS" ), Qgis::Critical );
    return false;
  }

  return true;
}

QgsPostgresProvider::Relkind QgsPostgresRasterProvider::relkind() const
{
  if ( mIsQuery || !connectionRO() )
    return QgsPostgresProvider::Relkind::Unknown;

  QString sql = QStringLiteral( "SELECT relkind FROM pg_class WHERE oid=regclass(%1)::oid" ).arg( quotedValue( mQuery ) );
  QgsPostgresResult res( connectionRO()->PQexec( sql ) );
  QString type = res.PQgetvalue( 0, 0 );

  QgsPostgresProvider::Relkind kind = QgsPostgresProvider::Relkind::Unknown;

  if ( type == QLatin1String( "r" ) )
  {
    kind = QgsPostgresProvider::Relkind::OrdinaryTable;
  }
  else if ( type == QLatin1String( "i" ) )
  {
    kind = QgsPostgresProvider::Relkind::Index;
  }
  else if ( type == QLatin1String( "s" ) )
  {
    kind = QgsPostgresProvider::Relkind::Sequence;
  }
  else if ( type == QLatin1String( "v" ) )
  {
    kind = QgsPostgresProvider::Relkind::View;
  }
  else if ( type == QLatin1String( "m" ) )
  {
    kind = QgsPostgresProvider::Relkind::MaterializedView;
  }
  else if ( type == QLatin1String( "c" ) )
  {
    kind = QgsPostgresProvider::Relkind::CompositeType;
  }
  else if ( type == QLatin1String( "t" ) )
  {
    kind = QgsPostgresProvider::Relkind::ToastTable;
  }
  else if ( type == QLatin1String( "f" ) )
  {
    kind = QgsPostgresProvider::Relkind::ForeignTable;
  }
  else if ( type == QLatin1String( "p" ) )
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
  if ( !mIsQuery )
  {
    sql = QStringLiteral( "SELECT count(*) FROM pg_inherits WHERE inhparent=%1::regclass" ).arg( quotedValue( mQuery ) );
    QgsDebugMsg( QStringLiteral( "Checking whether %1 is a parent table" ).arg( sql ) );
    QgsPostgresResult res( connectionRO()->PQexec( sql ) );
    bool isParentTable( res.PQntuples() == 0 || res.PQgetvalue( 0, 0 ).toInt() > 0 );

    sql = QStringLiteral( "SELECT indexrelid FROM pg_index WHERE indrelid=%1::regclass AND (indisprimary OR indisunique) ORDER BY CASE WHEN indisprimary THEN 1 ELSE 2 END LIMIT 1" ).arg( quotedValue( mQuery ) );
    QgsDebugMsg( QStringLiteral( "Retrieving first primary or unique index: %1" ).arg( sql ) );

    res = connectionRO()->PQexec( sql );
    QgsDebugMsg( QStringLiteral( "Got %1 rows." ).arg( res.PQntuples() ) );

    QStringList log;

    // no primary or unique indices found
    if ( res.PQntuples() == 0 )
    {
      QgsDebugMsg( QStringLiteral( "Relation has no primary key -- investigating alternatives" ) );

      // Two options here. If the relation is a table, see if there is
      // an oid column that can be used instead.
      // If the relation is a view try to find a suitable column to use as
      // the primary key.

      const QgsPostgresProvider::Relkind type = relkind();

      if ( type == QgsPostgresProvider::Relkind::OrdinaryTable || type == QgsPostgresProvider::Relkind::PartitionedTable )
      {
        QgsDebugMsg( QStringLiteral( "Relation is a table. Checking to see if it has an oid column." ) );

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

      QgsDebugMsg( "Retrieving key columns: " + sql );
      res = connectionRO()->PQexec( sql );
      QgsDebugMsg( QStringLiteral( "Got %1 rows." ).arg( res.PQntuples() ) );

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
  const QString sql { QStringLiteral( "SELECT data_type FROM information_schema.columns "
                                      "WHERE column_name = %1 AND table_name = %2 AND table_schema = %3" )
                      .arg( keyCandidate )
                      .arg( mTableName )
                      .arg( mSchemaName ) };
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
  Q_ASSERT( ! mPrimaryKeyAttrs.isEmpty() );
  if ( mPrimaryKeyAttrs.count( ) > 1 )
  {
    return mPrimaryKeyAttrs.join( ',' ).prepend( '(' ).append( ')' );
  }
  return mPrimaryKeyAttrs.first();
}


void QgsPostgresRasterProvider::findOverviews()
{
  const QString sql { QStringLiteral( "SELECT overview_factor, o_table_schema, o_table_name, o_raster_column "
                                      "FROM raster_overviews WHERE r_table_schema = %1 AND r_table_name = %2" )
                      .arg( quotedValue( mSchemaName ) )
                      .arg( quotedValue( mTableName ) ) };

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
        QgsMessageLog::logMessage( tr( "Cannot convert overview factor '%1' to int" ).arg( result.PQgetvalue( i, 0 ) ), QStringLiteral( "PostGIS" ), Qgis::Warning );
        return;
      }
      const QString schema { result.PQgetvalue( i, 1 ) };
      const QString table { result.PQgetvalue( i, 2 ) };
      if ( table.isEmpty() || schema.isEmpty() )
      {
        QgsMessageLog::logMessage( tr( "Table or schema is empty" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
        return;
      }
      mOverViews[ overViewFactor ] = QStringLiteral( "%1.%2" ).arg( quotedIdentifier( schema ) ).arg( quotedIdentifier( table ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Error fetching overviews information: %1" ).arg( result.PQresultErrorMessage() ), QStringLiteral( "PostGIS" ), Qgis::Warning );
  }
  if ( mOverViews.isEmpty() )
  {
    QgsMessageLog::logMessage( tr( "No overviews found, performances may be affected" ), QStringLiteral( "PostGIS" ), Qgis::Info );
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
    QgsMessageLog::logMessage( tr( "Data type is unknown" ), QStringLiteral( "PostGIS" ), Qgis::Warning );
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
      QgsDebugMsg( QStringLiteral( "Using cached statistics." ) );
      return stats;
    }
  }

  QString tableToQuery { mQuery };
  const double pixelsRatio { static_cast<double>( sampleSize ) / ( mWidth * mHeight ) };
  double statsRatio { pixelsRatio };

  // Decide if overviews can be used here
  if ( mSqlWhereClause.isEmpty() && ! mIsQuery && mIsTiled && extent.isEmpty() )
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
                  .arg( extent.asWktPolygon() )
                  .arg( mCrs.postgisSrid() ) };

  if ( ! mSqlWhereClause.isEmpty() )
  {
    where.append( where.isEmpty() ? QStringLiteral( "WHERE %1" ).arg( mSqlWhereClause ) :
                  QStringLiteral( " AND %1" ).arg( mSqlWhereClause ) );
  }

  const QString sql { QStringLiteral( "SELECT (ST_SummaryStatsAgg( %1, %2, TRUE, %3 )).* "
                                      "FROM %4 %5" )
                      .arg( quotedIdentifier( mRasterColumn ) )
                      .arg( bandNo )
                      .arg( std::max<double>( 0, std::min<double>( 1, statsRatio ) ) )
                      .arg( tableToQuery )
                      .arg( where )
                    };

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
                               QStringLiteral( "PostGIS" ), Qgis::Warning );
  }

  QgsDebugMsg( QStringLiteral( "************ STATS **************" ) );
  QgsDebugMsg( QStringLiteral( "MIN %1" ).arg( rasterBandStats.minimumValue ) );
  QgsDebugMsg( QStringLiteral( "MAX %1" ).arg( rasterBandStats.maximumValue ) );
  QgsDebugMsg( QStringLiteral( "RANGE %1" ).arg( rasterBandStats.range ) );
  QgsDebugMsg( QStringLiteral( "MEAN %1" ).arg( rasterBandStats.mean ) );
  QgsDebugMsg( QStringLiteral( "STDDEV %1" ).arg( rasterBandStats.stdDev ) );

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
