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

#include <cstring>

#include "qgsapplication.h"
#include "qgsgdalutils.h"
#include "qgsmessagelog.h"
#include "qgspolygon.h"
#include "qgspostgresprovidermetadatautils.h"
#include "qgspostgresutils.h"
#include "qgsraster.h"
#include "qgsrectangle.h"
#include "qgsstringutils.h"

#include <QRegularExpression>

#include "moc_qgspostgresrasterprovider.cpp"

const QString QgsPostgresRasterProvider::PG_RASTER_PROVIDER_KEY = u"postgresraster"_s;
const QString QgsPostgresRasterProvider::PG_RASTER_PROVIDER_DESCRIPTION = u"Postgres raster provider"_s;

QgsPostgresRasterProvider::QgsPostgresRasterProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, Qgis::DataProviderReadFlags flags )
  : QgsRasterDataProvider( uri, providerOptions, flags )
  , mShared( new QgsPostgresRasterSharedData )
{
  mUri = uri;

  // populate members from the uri structure
  mSchemaName = mUri.schema();
  if ( mSchemaName.isEmpty() )
  {
    mSchemaName = u"public"_s;
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

  QgsDebugMsgLevel( u"Connection info is %1"_s.arg( QgsPostgresConn::connectionInfo( mUri, false ) ), 4 );
  QgsDebugMsgLevel( u"Schema is: %1"_s.arg( mSchemaName ), 4 );
  QgsDebugMsgLevel( u"Table name is: %1"_s.arg( mTableName ), 4 );
  QgsDebugMsgLevel( u"Query is: %1"_s.arg( mQuery ), 4 );
  QgsDebugMsgLevel( u"Where clause is: %1"_s.arg( mSqlWhereClause ), 4 );

  // no table/query passed, the provider could be used to get tables
  if ( mQuery.isEmpty() )
  {
    return;
  }

  mConnectionRO = QgsPostgresConn::connectDb( mUri, true );
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
  if ( !mDetectedSrid.isEmpty() && !mRequestedSrid.isEmpty() && mRequestedSrid != mDetectedSrid )
  {
    QgsMessageLog::logMessage( tr( "Requested SRID (%1) and detected SRID (%2) differ" ).arg( mRequestedSrid ).arg( mDetectedSrid ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
  }

  // Try to load metadata
  const QString schemaQuery = u"SELECT table_schema FROM information_schema.tables WHERE table_name = 'qgis_layer_metadata'"_s;
  QgsPostgresResult res( mConnectionRO->LoggedPQexec( "QgsPostgresRasterProvider", schemaQuery ) );
  if ( res.PQntuples() > 0 )
  {
    const QString schemaName = res.PQgetvalue( 0, 0 );
    // TODO: also filter CRS?
    const QString selectQuery = QStringLiteral( R"SQL(
            SELECT
              qmd
           FROM %4.qgis_layer_metadata
             WHERE
                f_table_schema=%1
                AND f_table_name=%2
                AND f_geometry_column %3
                AND layer_type='raster'
           )SQL" )
                                  .arg( QgsPostgresConn::quotedValue( mUri.schema() ) )
                                  .arg( QgsPostgresConn::quotedValue( mUri.table() ) )
                                  .arg( mUri.geometryColumn().isEmpty() ? u"IS NULL"_s : u"=%1"_s.arg( QgsPostgresConn::quotedValue( mUri.geometryColumn() ) ) )
                                  .arg( QgsPostgresConn::quotedIdentifier( schemaName ) );

    QgsPostgresResult res( mConnectionRO->LoggedPQexec( "QgsPostgresRasterProvider", selectQuery ) );
    if ( res.PQntuples() > 0 )
    {
      QgsLayerMetadata metadata;
      QDomDocument doc;
      doc.setContent( res.PQgetvalue( 0, 0 ) );
      mLayerMetadata.readMetadataXml( doc.documentElement() );
      QgsMessageLog::logMessage( tr( "PostgreSQL raster layer metadata loaded from the database." ), tr( "PostGIS" ) );
    }
  }

  mLayerMetadata.setType( u"dataset"_s );
  mLayerMetadata.setCrs( crs() );


  mValid = true;
}

QgsPostgresRasterProvider::QgsPostgresRasterProvider( const QgsPostgresRasterProvider &other, const QgsDataProvider::ProviderOptions &providerOptions, Qgis::DataProviderReadFlags flags )
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

Qgis::DataProviderFlags QgsPostgresRasterProvider::flags() const
{
  return Qgis::DataProviderFlag::FastExtent2D;
}

bool QgsPostgresRasterProvider::hasSufficientPermsAndCapabilities()
{
  QgsDebugMsgLevel( u"Checking for permissions on the relation"_s, 4 );

  QgsPostgresResult testAccess;
  if ( !mIsQuery )
  {
    // Check that we can read from the table (i.e., we have select permission).
    QString sql = u"SELECT * FROM %1 LIMIT 1"_s.arg( mQuery );
    QgsPostgresResult testAccess( connectionRO()->PQexec( sql ) );
    if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Unable to access the %1 relation.\nThe error message from the database was:\n%2.\nSQL: %3" ).arg( mQuery, testAccess.PQresultErrorMessage(), sql ), tr( "PostGIS" ) );
      return false;
    }

    if ( connectionRO()->pgVersion() >= 90000 )
    {
      testAccess = connectionRO()->PQexec( u"SELECT pg_is_in_recovery()"_s );
      if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK || testAccess.PQgetvalue( 0, 0 ) == "t"_L1 )
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
    QgsMessageLog::logMessage( tr( "Invalid band number '%1" ).arg( bandNo ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
    return false;
  }

  const QgsRectangle rasterExtent = viewExtent.intersect( mExtent );
  if ( rasterExtent.isEmpty() )
  {
    QgsMessageLog::logMessage( tr( "Requested extent is not valid" ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
    return false;
  }

  const bool isSingleValue { width == 1 && height == 1 };
  QString tableToQuery { mQuery };

  QString whereAnd { subsetStringWithTemporalRange() };
  if ( !whereAnd.isEmpty() )
  {
    whereAnd = whereAnd.append( u" AND "_s );
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
      QgsMessageLog::logMessage( tr( "Unable to access the %1 relation.\nThe error message from the database was:\n%2.\nSQL: %3" ).arg( mQuery, result.PQresultErrorMessage(), sql ), tr( "PostGIS" ) );
      return false;
    }

    bool ok;
    QString val { result.PQntuples() > 0 ? result.PQgetvalue( 0, 0 ) : QString() };

    if ( val.isNull() && mUseSrcNoDataValue[bandNo - 1] )
    {
      // sparse rasters can have null values
      val = QString::number( mSrcNoDataValue[bandNo - 1] );
    }


    const Qgis::DataType dataType { mDataTypes[static_cast<unsigned int>( bandNo - 1 )] };
    switch ( dataType )
    {
      case Qgis::DataType::Byte:
      {
        const unsigned short byte { val.toUShort( &ok ) };
        if ( !ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to byte" ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
          return false;
        }
        std::memcpy( data, &byte, sizeof( unsigned short ) );
        break;
      }
      case Qgis::DataType::UInt16:
      {
        const unsigned int uint { val.toUInt( &ok ) };
        if ( !ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to unsigned int" ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
          return false;
        }
        std::memcpy( data, &uint, sizeof( unsigned int ) );
        break;
      }
      case Qgis::DataType::UInt32:
      {
        const unsigned long ulong { val.toULong( &ok ) };
        if ( !ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to unsigned long" ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
          return false;
        }
        std::memcpy( data, &ulong, sizeof( unsigned long ) );
        break;
      }
      case Qgis::DataType::Int16:
      {
        const int intVal { val.toInt( &ok ) };
        if ( !ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to int" ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
          return false;
        }
        std::memcpy( data, &intVal, sizeof( int ) );
        break;
      }
      case Qgis::DataType::Int32:
      {
        const long longVal { val.toLong( &ok ) };
        if ( !ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to long" ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
          return false;
        }
        std::memcpy( data, &longVal, sizeof( long ) );
        break;
      }
      case Qgis::DataType::Float32:
      {
        const float floatVal { val.toFloat( &ok ) };
        if ( !ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to float" ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
          return false;
        }
        std::memcpy( data, &floatVal, sizeof( float ) );
        break;
      }
      case Qgis::DataType::Float64:
      {
        const double doubleVal { val.toDouble( &ok ) };
        if ( !ok )
        {
          QgsMessageLog::logMessage( tr( "Cannot convert identified value to double" ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
          return false;
        }
        std::memcpy( data, &doubleVal, sizeof( double ) );
        break;
      }
      default:
      {
        QgsMessageLog::logMessage( tr( "Unknown identified data type" ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
        return false;
      }
    }
  }
  else // Fetch block
  {
    const Qgis::DataType dataType { mDataTypes[bandNo - 1] };
    const GDALDataType gdalDataType = QgsGdalUtils::gdalDataTypeFromQgisDataType( dataType );
    const double noDataValue { mSrcNoDataValue[bandNo - 1] };

    const double xRes = viewExtent.width() / width;
    const double yRes = viewExtent.height() / height;

    // Find overview
    const double minPixelSize { std::min( xRes, yRes ) };

    // TODO: round?
    const unsigned int desiredOverviewFactor { static_cast<unsigned int>( minPixelSize / std::max( std::abs( mScaleX ), std::abs( mScaleY ) ) ) };

    unsigned int overviewFactor { 1 }; // no overview

    // Cannot use overviews if there is a where condition
    if ( whereAnd.isEmpty() )
    {
      const auto ovKeys { mOverViews.keys() };
      QList<unsigned int>::const_reverse_iterator rit { ovKeys.rbegin() };
      for ( ; rit != ovKeys.rend(); ++rit )
      {
        if ( *rit <= desiredOverviewFactor )
        {
          tableToQuery = mOverViews[*rit];
          overviewFactor = *rit;
          QgsDebugMsgLevel( u"Using overview for block read: %1"_s.arg( tableToQuery ), 3 );
          break;
        }
      }
    }

    //qDebug() << "Overview desired: " << desiredOverviewFactor << "found:" << overviewFactor << tableToQuery;
    //qDebug() << "View extent" << viewExtent.toString( 1 ) << width << height << minPixelSize;

    // Get the the tiles we need to build the block
    const QgsPostgresRasterSharedData::TilesRequest tilesRequest {
      bandNo,
      rasterExtent,
      overviewFactor,
      pkSql(), // already quoted
      quotedIdentifier( mRasterColumn ),
      tableToQuery,
      QString::number( mCrs.postgisSrid() ),
      whereAnd,
      connectionRO()
    };

    const QgsPostgresRasterSharedData::TilesResponse tileResponse {
      mShared->tiles( tilesRequest )
    };

    if ( tileResponse.tiles.isEmpty() )
    {
      // rasters can be sparse by omitting some of the blocks/tiles
      // so we should not log an error here but make sure
      // the result buffer is filled with nodata
      gdal::dataset_unique_ptr dstDS { QgsGdalUtils::createSingleBandMemoryDataset(
        gdalDataType, viewExtent, width, height, mCrs
      ) };
      if ( !dstDS )
      {
        const QString lastError = QString::fromUtf8( CPLGetLastErrorMsg() );
        QgsMessageLog::logMessage( tr( "Unable to create destination raster for tiles from %1: %2" ).arg( tableToQuery, lastError ), tr( "PostGIS" ), Qgis::MessageLevel::Critical );
        return false;
      }

      GDALSetRasterNoDataValue( GDALGetRasterBand( dstDS.get(), 1 ), noDataValue );
      // fill with nodata
      GDALFillRaster( GDALGetRasterBand( dstDS.get(), 1 ), noDataValue, 0 );

      // copy to the result buffer
      CPLErrorReset();
      CPLErr err = GDALRasterIO( GDALGetRasterBand( dstDS.get(), 1 ), GF_Read, 0, 0, width, height, data, width, height, gdalDataType, 0, 0 );
      if ( err != CE_None )
      {
        const QString lastError = QString::fromUtf8( CPLGetLastErrorMsg() );
        QgsMessageLog::logMessage( tr( "Unable to write raster to block from %1: %2" ).arg( mQuery, lastError ), tr( "PostGIS" ), Qgis::MessageLevel::Critical );
        return false;
      }

      return true;
    }

    // Finally merge the tiles
    // We must have at least one tile at this point (we checked for that before)

    const QgsRectangle &tilesExtent { tileResponse.extent };

    // Prepare tmp output raster
    const int tmpWidth = static_cast<int>( std::round( tilesExtent.width() / tileResponse.tiles.first().scaleX ) );
    const int tmpHeight = static_cast<int>( std::round( tilesExtent.height() / std::fabs( tileResponse.tiles.first().scaleY ) ) );

    //qDebug() << "Creating output raster: " << tilesExtent.toString() << tmpWidth << tmpHeight;

    gdal::dataset_unique_ptr tmpDS { QgsGdalUtils::createSingleBandMemoryDataset(
      gdalDataType, tilesExtent, tmpWidth, tmpHeight, mCrs
    ) };
    if ( !tmpDS )
    {
      {
        QgsMessageLog::logMessage( tr( "Unable to create temporary raster for tiles from %1" ).arg( tableToQuery ), tr( "PostGIS" ), Qgis::MessageLevel::Critical );
        return false;
      }
    }

    GDALSetRasterNoDataValue( GDALGetRasterBand( tmpDS.get(), 1 ), noDataValue );

    // Write tiles to the temporary raster
    CPLErrorReset();

    for ( auto &tile : std::as_const( tileResponse.tiles ) )
    {
      // Offset in px from the base raster
      const int xOff { static_cast<int>( std::round( ( tile.upperLeftX - tilesExtent.xMinimum() ) / tile.scaleX ) ) };
      const int yOff { static_cast<int>( std::round( ( tilesExtent.yMaximum() - tile.extent.yMaximum() ) / std::fabs( tile.scaleY ) ) ) };

      //qDebug() << "Merging tile output raster: " << tile.tileId << xOff << yOff << tile.width << tile.height ;

      CPLErr err = GDALRasterIO( GDALGetRasterBand( tmpDS.get(), 1 ), GF_Write, xOff, yOff, static_cast<int>( tile.width ), static_cast<int>( tile.height ),
                                 ( void * ) ( tile.data.constData() ), // old-style because of const
                                 static_cast<int>( tile.width ), static_cast<int>( tile.height ), gdalDataType, 0, 0 );
      if ( err != CE_None )
      {
        const QString lastError = QString::fromUtf8( CPLGetLastErrorMsg() );
        QgsMessageLog::logMessage( tr( "Unable to write tile to temporary raster from %1: %2" ).arg( tableToQuery, lastError ), tr( "PostGIS" ), Qgis::MessageLevel::Critical );
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
      gdalDataType, viewExtent, width, height, mCrs
    ) };
    if ( !dstDS )
    {
      const QString lastError = QString::fromUtf8( CPLGetLastErrorMsg() );
      QgsMessageLog::logMessage( tr( "Unable to create destination raster for tiles from %1: %2" ).arg( tableToQuery, lastError ), tr( "PostGIS" ), Qgis::MessageLevel::Critical );
      return false;
    }

    GDALSetRasterNoDataValue( GDALGetRasterBand( dstDS.get(), 1 ), noDataValue );

    // Resample the raster to the final bounds and resolution
    if ( !QgsGdalUtils::resampleSingleBandRaster( tmpDS.get(), dstDS.get(), GDALResampleAlg::GRA_NearestNeighbour, nullptr ) )
    {
      const QString lastError = QString::fromUtf8( CPLGetLastErrorMsg() );
      QgsMessageLog::logMessage( tr( "Unable to resample and transform destination raster for tiles from %1: %2" ).arg( tableToQuery, lastError ), tr( "PostGIS" ), Qgis::MessageLevel::Critical );
      return false;
    }

    // Copy to result buffer
    CPLErrorReset();
    CPLErr err = GDALRasterIO( GDALGetRasterBand( dstDS.get(), 1 ), GF_Read, 0, 0, width, height, data, width, height, gdalDataType, 0, 0 );
    if ( err != CE_None )
    {
      const QString lastError = QString::fromUtf8( CPLGetLastErrorMsg() );
      QgsMessageLog::logMessage( tr( "Unable to write raster to block from %1: %2" ).arg( mQuery, lastError ), tr( "PostGIS" ), Qgis::MessageLevel::Critical );
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

QIcon QgsPostgresRasterProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconPostgis.svg"_s );
}

QVariantMap QgsPostgresRasterProviderMetadata::decodeUri( const QString &uri ) const
{
  const QgsDataSourceUri dsUri { uri };
  QVariantMap decoded;

  if ( !dsUri.database().isEmpty() )
  {
    decoded[u"dbname"_s] = dsUri.database();
  }
  if ( !dsUri.host().isEmpty() )
  {
    decoded[u"host"_s] = dsUri.host();
  }
  if ( !dsUri.port().isEmpty() )
  {
    decoded[u"port"_s] = dsUri.port();
  }
  if ( !dsUri.service().isEmpty() )
  {
    decoded[u"service"_s] = dsUri.service();
  }
  if ( !dsUri.username().isEmpty() )
  {
    decoded[u"username"_s] = dsUri.username();
  }
  if ( !dsUri.password().isEmpty() )
  {
    decoded[u"password"_s] = dsUri.password();
  }
  if ( !dsUri.authConfigId().isEmpty() )
  {
    decoded[u"authcfg"_s] = dsUri.authConfigId();
  }
  if ( !dsUri.schema().isEmpty() )
  {
    decoded[u"schema"_s] = dsUri.schema();
  }
  if ( !dsUri.table().isEmpty() )
  {
    decoded[u"table"_s] = dsUri.table();
  }
  if ( !dsUri.keyColumn().isEmpty() )
  {
    decoded[u"key"_s] = dsUri.keyColumn();
  }
  if ( !dsUri.srid().isEmpty() )
  {
    decoded[u"srid"_s] = dsUri.srid();
  }
  if ( uri.contains( u"estimatedmetadata="_s, Qt::CaseSensitivity::CaseInsensitive ) )
  {
    decoded[u"estimatedmetadata"_s] = dsUri.useEstimatedMetadata();
  }
  if ( uri.contains( u"sslmode="_s, Qt::CaseSensitivity::CaseInsensitive ) )
  {
    decoded[u"sslmode"_s] = dsUri.sslMode();
  }
  // Do not add sql if it's empty
  if ( !dsUri.sql().isEmpty() )
  {
    decoded[u"sql"_s] = dsUri.sql();
  }
  if ( !dsUri.geometryColumn().isEmpty() )
  {
    decoded[u"geometrycolumn"_s] = dsUri.geometryColumn();
  }

  // Params
  const static QStringList params { { u"temporalFieldIndex"_s, u"temporalDefaultTime"_s, u"enableTime"_s } };

  for ( const QString &pname : std::as_const( params ) )
  {
    if ( dsUri.hasParam( pname ) )
    {
      decoded[pname] = dsUri.param( pname );
    }
  }

  return decoded;
}


QString QgsPostgresRasterProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  if ( parts.contains( u"dbname"_s ) )
    dsUri.setDatabase( parts.value( u"dbname"_s ).toString() );
  if ( parts.contains( u"port"_s ) )
    dsUri.setParam( u"port"_s, parts.value( u"port"_s ).toString() );
  if ( parts.contains( u"host"_s ) )
    dsUri.setParam( u"host"_s, parts.value( u"host"_s ).toString() );
  if ( parts.contains( u"service"_s ) )
    dsUri.setParam( u"service"_s, parts.value( u"service"_s ).toString() );
  if ( parts.contains( u"username"_s ) )
    dsUri.setUsername( parts.value( u"username"_s ).toString() );
  if ( parts.contains( u"password"_s ) )
    dsUri.setPassword( parts.value( u"password"_s ).toString() );
  if ( parts.contains( u"authcfg"_s ) )
    dsUri.setAuthConfigId( parts.value( u"authcfg"_s ).toString() );
  if ( parts.contains( u"selectatid"_s ) )
    dsUri.setParam( u"selectatid"_s, parts.value( u"selectatid"_s ).toString() );
  if ( parts.contains( u"table"_s ) )
    dsUri.setTable( parts.value( u"table"_s ).toString() );
  if ( parts.contains( u"schema"_s ) )
    dsUri.setSchema( parts.value( u"schema"_s ).toString() );
  if ( parts.contains( u"key"_s ) )
    dsUri.setParam( u"key"_s, parts.value( u"key"_s ).toString() );
  if ( parts.contains( u"srid"_s ) )
    dsUri.setSrid( parts.value( u"srid"_s ).toString() );
  if ( parts.contains( u"estimatedmetadata"_s ) )
    dsUri.setParam( u"estimatedmetadata"_s, parts.value( u"estimatedmetadata"_s ).toString() );
  if ( parts.contains( u"sslmode"_s ) )
    dsUri.setParam( u"sslmode"_s, QgsDataSourceUri::encodeSslMode( static_cast<QgsDataSourceUri::SslMode>( parts.value( u"sslmode"_s ).toInt() ) ) );
  if ( parts.contains( u"sql"_s ) )
    dsUri.setSql( parts.value( u"sql"_s ).toString() );
  if ( parts.contains( u"geometrycolumn"_s ) )
    dsUri.setGeometryColumn( parts.value( u"geometrycolumn"_s ).toString() );
  if ( parts.contains( u"temporalFieldIndex"_s ) )
    dsUri.setParam( u"temporalFieldIndex"_s, parts.value( u"temporalFieldIndex"_s ).toString() );
  if ( parts.contains( u"temporalDefaultTime"_s ) )
    dsUri.setParam( u"temporalDefaultTime"_s, parts.value( u"temporalDefaultTime"_s ).toString() );
  if ( parts.contains( u"enableTime"_s ) )
    dsUri.setParam( u"enableTime"_s, parts.value( u"enableTime"_s ).toString() );
  return dsUri.uri( false );
}

QList<Qgis::LayerType> QgsPostgresRasterProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Raster };
}

bool QgsPostgresRasterProviderMetadata::saveLayerMetadata( const QString &uri, const QgsLayerMetadata &metadata, QString &errorMessage )
{
  return QgsPostgresProviderMetadataUtils::saveLayerMetadata( Qgis::LayerType::Raster, uri, metadata, errorMessage );
}

QgsProviderMetadata::ProviderCapabilities QgsPostgresRasterProviderMetadata::providerCapabilities() const
{
  return QgsProviderMetadata::ProviderCapability::SaveLayerMetadata;
}

QgsPostgresRasterProvider *QgsPostgresRasterProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  return new QgsPostgresRasterProvider( uri, options, flags );
}


Qgis::DataType QgsPostgresRasterProvider::dataType( int bandNo ) const
{
  if ( mDataTypes.size() < static_cast<unsigned long>( bandNo ) )
  {
    QgsMessageLog::logMessage( tr( "Data type size for band %1 could not be found: num bands is: %2 and the type size map for bands contains: %n item(s)", nullptr, mDataSizes.size() ).arg( bandNo ).arg( mBandCount ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
    return Qgis::DataType::UnknownDataType;
  }
  // Band is 1-based
  return mDataTypes[static_cast<unsigned long>( bandNo ) - 1];
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

Qgis::RasterProviderCapabilities QgsPostgresRasterProvider::providerCapabilities() const
{
  return Qgis::RasterProviderCapability::ReadLayerMetadata;
}

QString QgsPostgresRasterProvider::htmlMetadata() const
{
  // This must return the content of a HTML table starting by tr and ending by tr
  QVariantMap overviews;
  for ( auto it = mOverViews.constBegin(); it != mOverViews.constEnd(); ++it )
  {
    overviews.insert( QString::number( it.key() ), it.value() );
  }

  const QVariantMap additionalInformation {
    { tr( "Is Tiled" ), mIsTiled },
    { tr( "Where Clause SQL" ), subsetString() },
    { tr( "Pixel Size" ), u"%1, %2"_s.arg( mScaleX ).arg( mScaleY ) },
    { tr( "Overviews" ), overviews },
    { tr( "Primary Keys SQL" ), pkSql() },
    { tr( "Temporal Column" ), mTemporalFieldIndex >= 0 && mAttributeFields.exists( mTemporalFieldIndex ) ? mAttributeFields.field( mTemporalFieldIndex ).name() : QString() },
  };
  return QgsPostgresUtils::variantMapToHtml( additionalInformation, tr( "Additional information" ) );
}

QString QgsPostgresRasterProvider::lastErrorTitle()
{
  return mErrorTitle;
}

QString QgsPostgresRasterProvider::lastError()
{
  return mError;
}

Qgis::RasterInterfaceCapabilities QgsPostgresRasterProvider::capabilities() const
{
  const Qgis::RasterInterfaceCapabilities capability = Qgis::RasterInterfaceCapability::Identify
                                                       | Qgis::RasterInterfaceCapability::IdentifyValue
                                                       | Qgis::RasterInterfaceCapability::Size
                                                       // TODO:| QgsRasterDataProvider::BuildPyramids
                                                       | Qgis::RasterInterfaceCapability::Prefetch;
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
    mConnectionRW = QgsPostgresConn::connectDb( mUri, false );
  }
  return mConnectionRW;
}

bool QgsPostgresRasterProvider::supportsSubsetString() const
{
  return true;
}

QString QgsPostgresRasterProvider::subsetStringDialect() const
{
  return tr( "PostgreSQL WHERE clause" );
}

QString QgsPostgresRasterProvider::subsetStringHelpUrl() const
{
  return u"https://www.postgresql.org/docs/current/sql-expressions.html"_s;
}

QString QgsPostgresRasterProvider::subsetString() const
{
  return mSqlWhereClause;
}

QString QgsPostgresRasterProvider::defaultTimeSubsetString( const QDateTime &defaultTime ) const
{
  if ( defaultTime.isValid() && mTemporalFieldIndex >= 0 && mAttributeFields.exists( mTemporalFieldIndex ) )
  {
    const QgsField temporalField { mAttributeFields.field( mTemporalFieldIndex ) };
    const QString typeCast { temporalField.type() != QMetaType::Type::QDateTime ? u"::timestamp"_s : QString() };
    const QString temporalFieldName { temporalField.name() };
    return { u"%1%2 = %3"_s
               .arg( quotedIdentifier( temporalFieldName ), typeCast, quotedValue( defaultTime.toString( Qt::DateFormat::ISODate ) ) ) };
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
    const QString typeCast { temporalField.type() != QMetaType::Type::QDateTime ? u"::timestamp"_s : QString() };
    const QString temporalFieldName { temporalField.name() };

    if ( temporalCapabilities()->hasTemporalCapabilities() )
    {
      QString temporalClause;
      const QgsTemporalRange<QDateTime> requestedRange { temporalCapabilities()->requestedTemporalRange() };
      if ( !requestedRange.isEmpty() && !requestedRange.isInfinite() )
      {
        if ( requestedRange.isInstant() )
        {
          temporalClause = u"%1%2 = %3"_s
                             .arg( quotedIdentifier( temporalFieldName ), typeCast, quotedValue( requestedRange.begin().toString( Qt::DateFormat::ISODate ) ) );
        }
        else
        {
          if ( requestedRange.begin().isValid() )
          {
            temporalClause = u"%1%2 %3 %4"_s
                               .arg( quotedIdentifier( temporalFieldName ), typeCast, requestedRange.includeBeginning() ? ">=" : ">", quotedValue( requestedRange.begin().toString( Qt::DateFormat::ISODate ) ) );
          }
          if ( requestedRange.end().isValid() )
          {
            if ( !temporalClause.isEmpty() )
            {
              temporalClause.append( u" AND "_s );
            }
            temporalClause.append( u"%1%2 %3 %4"_s
                                     .arg( quotedIdentifier( temporalFieldName ), typeCast, requestedRange.includeEnd() ? "<=" : "<", quotedValue( requestedRange.end().toString( Qt::DateFormat::ISODate ) ) ) );
          }
        }
        return mSqlWhereClause.isEmpty() ? temporalClause : u"%1 AND (%2)"_s.arg( mSqlWhereClause, temporalClause );
      }
      const QString defaultTimeSubset { defaultTimeSubsetString( mTemporalDefaultTime ) };
      if ( !defaultTimeSubset.isEmpty() )
      {
        return mSqlWhereClause.isEmpty() ? defaultTimeSubset : u"%1 AND (%2)"_s.arg( mSqlWhereClause, defaultTimeSubset );
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

  mOverViews.clear();

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
  auto pixelTypeFromString = []( const QString &t ) -> Qgis::DataType {
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
    if ( t == "8BUI"_L1 )
    {
      type = Qgis::DataType::Byte;
    }
    else if ( t == "16BUI"_L1 )
    {
      type = Qgis::DataType::UInt16;
    }
    else if ( t == "16BSI"_L1 )
    {
      type = Qgis::DataType::Int16;
    }
    else if ( t == "32BSI"_L1 )
    {
      type = Qgis::DataType::Int32;
    }
    else if ( t == "32BUI"_L1 )
    {
      type = Qgis::DataType::UInt32;
    }
    else if ( t == "32BF"_L1 )
    {
      type = Qgis::DataType::Float32;
    }
    else if ( t == "64BF"_L1 )
    {
      type = Qgis::DataType::Float64;
    }
    return type;
  };

  // ///////////////////////////////////////////////////////////////////
  // First method: get information from metadata
  if ( !mIsQuery && mUseEstimatedMetadata && subsetString().isEmpty() )
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

        if ( !ok )
        {
          throw QgsPostgresRasterProviderException( tr( "Cannot create CRS from EPSG: '%1'" ).arg( result.PQgetvalue( 0, 1 ) ) );
        }

        mDetectedSrid = result.PQgetvalue( 0, 1 );
        mBandCount = result.PQgetvalue( 0, 2 ).toInt( &ok );

        if ( !ok )
        {
          throw QgsPostgresRasterProviderException( tr( "Cannot get band count from value: '%1'" ).arg( result.PQgetvalue( 0, 2 ) ) );
        }

        QString pxTypesArray { result.PQgetvalue( 0, 3 ) };
        pxTypesArray.chop( 1 );
        const QStringList pxTypes { pxTypesArray.mid( 1 ).split( ',' ) };

        QString noDataValuesArray { result.PQgetvalue( 0, 4 ) };
        noDataValuesArray.chop( 1 );
        const QStringList noDataValues { noDataValuesArray.mid( 1 ).split( ',' ) };

        if ( mBandCount != pxTypes.count() || mBandCount != noDataValues.count() )
        {
          throw QgsPostgresRasterProviderException( tr( "Band count and NoData items count differ" ) );
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
          if ( !ok )
          {
            if ( noDataValues.at( i ) != "NULL"_L1 )
            {
              QgsMessageLog::logMessage( tr( "Cannot convert NoData value '%1' to double" ).arg( noDataValues.at( i ) ), u"PostGIS"_s, Qgis::MessageLevel::Info );
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

        if ( hexAscii.isEmpty() || !p.fromWkb( ptr ) )
        {
          // Try to determine extent from raster
          const QString extentSql = QStringLiteral( "SELECT ST_Envelope( %1 ) "
                                                    "FROM %2 WHERE %3" )
                                      .arg( quotedIdentifier( mRasterColumn ), mQuery, subsetString().isEmpty() ? "'t'" : subsetString() );

          QgsPostgresResult extentResult( connectionRO()->PQexec( extentSql ) );
          const QByteArray extentHexAscii { extentResult.PQgetvalue( 0, 0 ).toLatin1() };
          const QByteArray extentHexBin = QByteArray::fromHex( extentHexAscii );
          QgsConstWkbPtr extentPtr { extentHexBin };
          if ( extentHexAscii.isEmpty() || !p.fromWkb( extentPtr ) )
          {
            throw QgsPostgresRasterProviderException( tr( "Cannot get extent from raster" ) );
          }
        }

        mExtent = p.boundingBox();

        // Tile size
        mTileWidth = result.PQgetvalue( 0, 6 ).toInt( &ok );

        if ( !ok )
        {
          throw QgsPostgresRasterProviderException( tr( "Cannot convert width '%1' to int" ).arg( result.PQgetvalue( 0, 6 ) ) );
        }

        mTileHeight = result.PQgetvalue( 0, 7 ).toInt( &ok );

        if ( !ok )
        {
          throw QgsPostgresRasterProviderException( tr( "Cannot convert height '%1' to int" ).arg( result.PQgetvalue( 0, 7 ) ) );
        }

        mIsOutOfDb = result.PQgetvalue( 0, 8 ) == 't';
        mScaleX = result.PQgetvalue( 0, 10 ).toDouble( &ok );

        if ( !ok )
        {
          throw QgsPostgresRasterProviderException( tr( "Cannot convert scale X '%1' to double" ).arg( result.PQgetvalue( 0, 10 ) ) );
        }

        mScaleY = result.PQgetvalue( 0, 11 ).toDouble( &ok );

        if ( !ok )
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
        return initFieldsAndTemporal();
      }
      else
      {
        QgsMessageLog::logMessage( tr( "An error occurred while fetching raster metadata for table %1: %2\nSQL: %3" ).arg( mQuery ).arg( result.PQresultErrorMessage() ).arg( sql ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
      }
    }
    catch ( QgsPostgresRasterProviderException &ex )
    {
      QgsMessageLog::logMessage( tr( "An error occurred while fetching raster metadata for %1, proceeding with (possibly very slow) raster data analysis: %2\n"
                                     "Please consider adding raster constraints with PostGIS function AddRasterConstraints." )
                                   .arg( mQuery )
                                   .arg( ex.message ),
                                 u"PostGIS"_s, Qgis::MessageLevel::Warning );
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
        QgsMessageLog::logMessage( tr( "Multiple raster column detected, using the first one" ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
      }
      mRasterColumn = result.PQgetvalue( 0, 0 );
    }
    else
    {
      QgsMessageLog::logMessage( tr( "An error occurred while fetching raster column" ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
      return false;
    }
  }

  // Get the full raster and extract information
  // Note: this can be very slow
  // Use oveviews if we can, even if they are probably missing for unconstrained tables.
  // Overviews are useless if there is a filter.
  if ( subsetString().isEmpty() )
  {
    findOverviews();
  }

  QString tableToQuery { mQuery };

  if ( !mOverViews.isEmpty() )
  {
    tableToQuery = mOverViews.last();
  }

  QString where;
  if ( !subsetString().isEmpty() )
  {
    where = u"WHERE %1"_s.arg( subsetString() );
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
                      FROM cte_band)" )
                        .arg( quotedIdentifier( mRasterColumn ), tableToQuery, where );

  QgsDebugMsgLevel( u"Raster information sql: %1"_s.arg( sql ), 4 );

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
      if ( !p.fromWkb( ptr ) )
      {
        QgsMessageLog::logMessage( tr( "Cannot get extent from raster" ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
        return false;
      }
    }
    catch ( ... )
    {
      QgsMessageLog::logMessage( tr( "Cannot get metadata from raster" ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
      return false;
    }

    mExtent = p.boundingBox();

    // Tile size (in this path the raster is considered untiled, so this is actually the whole size)
    mTileWidth = result.PQgetvalue( 0, 3 ).toInt( &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot convert width '%1' to int" ).arg( result.PQgetvalue( 0, 3 ) ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
      return false;
    }

    mTileHeight = result.PQgetvalue( 0, 4 ).toInt( &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot convert height '%1' to int" ).arg( result.PQgetvalue( 0, 4 ) ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
      return false;
    }

    mScaleX = result.PQgetvalue( 0, 5 ).toDouble( &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot convert scale X '%1' to double" ).arg( result.PQgetvalue( 0, 5 ) ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
      return false;
    }

    mScaleY = result.PQgetvalue( 0, 6 ).toDouble( &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot convert scale Y '%1' to double" ).arg( result.PQgetvalue( 0, 6 ) ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
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

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Cannot create CRS from EPSG: '%1'" ).arg( result.PQgetvalue( 0, 9 ) ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
      return false;
    }

    mDetectedSrid = result.PQgetvalue( 0, 9 );

    // Fetch band data types
    for ( int rowNumber = 0; rowNumber < result.PQntuples(); ++rowNumber )
    {
      Qgis::DataType type { pixelTypeFromString( result.PQgetvalue( rowNumber, 11 ) ) };

      if ( type == Qgis::DataType::UnknownDataType )
      {
        QgsMessageLog::logMessage( tr( "Unsupported data type: '%1'" ).arg( result.PQgetvalue( rowNumber, 11 ) ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
        return false;
      }

      mDataTypes.push_back( type );
      mDataSizes.push_back( QgsRasterBlock::typeSize( type ) );
      double nodataValue { result.PQgetvalue( rowNumber, 12 ).toDouble( &ok ) };

      if ( !ok )
      {
        QgsMessageLog::logMessage( tr( "Cannot convert NoData value '%1' to double, default to: %2" ).arg( result.PQgetvalue( rowNumber, 2 ) ).arg( std::numeric_limits<double>::min() ), u"PostGIS"_s, Qgis::MessageLevel::Info );
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
    QgsMessageLog::logMessage( tr( "An error occurred while fetching raster metadata" ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
    return false;
  }

  return initFieldsAndTemporal();
}

bool QgsPostgresRasterProvider::initFieldsAndTemporal()
{
  // Populate fields
  if ( !loadFields() )
  {
    QgsMessageLog::logMessage( tr( "An error occurred while fetching raster fields information" ), u"PostGIS"_s, Qgis::MessageLevel::Critical );
    return false;
  }

  QString where;
  if ( !subsetString().isEmpty() )
  {
    where = u"WHERE %1"_s.arg( subsetString() );
  }

  // Temporal capabilities
  // Setup temporal properties for layer, do not fail if something goes wrong but log a warning
  if ( mUri.hasParam( u"temporalFieldIndex"_s ) )
  {
    bool ok;
    const int temporalFieldIndex { mUri.param( u"temporalFieldIndex"_s ).toInt( &ok ) };
    if ( ok && mAttributeFields.exists( temporalFieldIndex ) )
    {
      const QString temporalFieldName { mAttributeFields.field( temporalFieldIndex ).name() };
      // Calculate the range
      const QString sql = QStringLiteral( "SELECT MIN(%1::timestamp), MAX(%1::timestamp) "
                                          "FROM %2 %3" )
                            .arg( quotedIdentifier( temporalFieldName ), mQuery, where );

      QgsPostgresResult result( connectionRO()->PQexec( sql ) );

      if ( PGRES_TUPLES_OK == result.PQresultStatus() && result.PQntuples() == 1 )
      {
        const QDateTime minTime { QDateTime::fromString( result.PQgetvalue( 0, 0 ), Qt::DateFormat::ISODate ) };
        const QDateTime maxTime { QDateTime::fromString( result.PQgetvalue( 0, 1 ), Qt::DateFormat::ISODate ) };
        if ( minTime.isValid() && maxTime.isValid() && !( minTime > maxTime ) )
        {
          mTemporalFieldIndex = temporalFieldIndex;
          QgsRasterDataProviderTemporalCapabilities *lTemporalCapabilities = temporalCapabilities();
          lTemporalCapabilities->setHasTemporalCapabilities( true );
          lTemporalCapabilities->setAvailableTemporalRange( { minTime, maxTime } );
          lTemporalCapabilities->setIntervalHandlingMethod( Qgis::TemporalIntervalMatchMethod::FindClosestMatchToStartOfRange );
          QgsDebugMsgLevel( u"Raster temporal range for field %1: %2 - %3"_s.arg( QString::number( mTemporalFieldIndex ), minTime.toString(), maxTime.toString() ), 3 );

          if ( mUri.hasParam( u"temporalDefaultTime"_s ) )
          {
            const QDateTime defaultDateTime { QDateTime::fromString( mUri.param( u"temporalDefaultTime"_s ), Qt::DateFormat::ISODate ) };
            if ( defaultDateTime.isValid() )
            {
              mTemporalDefaultTime = defaultDateTime;
            }
            else
            {
              QgsMessageLog::logMessage( tr( "Invalid default date in raster temporal capabilities for field %1: %2" ).arg( temporalFieldName, mUri.param( u"temporalDefaultTime"_s ) ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
            }
          }

          // Set temporal ranges
          QList<QgsDateTimeRange> allRanges;
          const QString sql = QStringLiteral( "SELECT DISTINCT %1::timestamp "
                                              "FROM %2 %3 ORDER BY %1::timestamp" )
                                .arg( quotedIdentifier( temporalFieldName ), mQuery, where );

          QgsPostgresResult result( connectionRO()->PQexec( sql ) );
          if ( PGRES_TUPLES_OK == result.PQresultStatus() && result.PQntuples() > 0 )
          {
            for ( qlonglong row = 0; row < result.PQntuples(); ++row )
            {
              const QDateTime date = QDateTime::fromString( result.PQgetvalue( row, 0 ), Qt::DateFormat::ISODate );
              allRanges.push_back( QgsDateTimeRange( date, date ) );
            }
            lTemporalCapabilities->setAllAvailableTemporalRanges( allRanges );
          }
          else
          {
            QgsMessageLog::logMessage( tr( "No temporal ranges detected in raster temporal capabilities for field %1: %2" ).arg( temporalFieldName, mUri.param( u"temporalDefaultTime"_s ) ), u"PostGIS"_s, Qgis::MessageLevel::Info );
          }
        }
        else
        {
          QgsMessageLog::logMessage( tr( "Invalid temporal range in raster temporal capabilities for field %1: %2 - %3" ).arg( temporalFieldName, minTime.toString(), maxTime.toString() ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "An error occurred while fetching raster temporal capabilities for field: %1" ).arg( temporalFieldName ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Invalid field index for raster temporal capabilities: %1" ).arg( QString::number( temporalFieldIndex ) ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
    }
  }
  return true;
}

bool QgsPostgresRasterProvider::loadFields()
{
  if ( !mIsQuery )
  {
    QgsDebugMsgLevel( u"Loading fields for table %1"_s.arg( mTableName ), 2 );

    // Get the relation oid for use in later queries
    QString sql = u"SELECT regclass(%1)::oid"_s.arg( quotedValue( mQuery ) );
    QgsPostgresResult tresult( connectionRO()->PQexec( sql ) );
    QString tableoid = tresult.PQgetvalue( 0, 0 );

    // Get the table description
    sql = u"SELECT description FROM pg_description WHERE objoid=%1 AND objsubid=0"_s.arg( tableoid );
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
  QString sql = u"SELECT * FROM %1 LIMIT 0"_s.arg( mQuery );

  QgsPostgresResult result( connectionRO()->PQexec( sql ) );

  // Collect type info
  sql = u"SELECT oid,typname,typtype,typelem,typlen FROM pg_type"_s;
  QgsPostgresResult typeResult( connectionRO()->PQexec( sql ) );

  QMap<Oid, PGTypeInfo> typeMap;
  for ( int i = 0; i < typeResult.PQntuples(); ++i )
  {
    PGTypeInfo typeInfo = {
      /* typeName = */ typeResult.PQgetvalue( i, 1 ),
      /* typeType = */ typeResult.PQgetvalue( i, 2 ),
      /* typeElem = */ typeResult.PQgetvalue( i, 3 ),
      /* typeLen = */ typeResult.PQgetvalue( i, 4 ).toInt()
    };
    typeMap.insert( typeResult.PQgetvalue( i, 0 ).toUInt(), typeInfo );
  }


  QMap<Oid, QMap<int, QString>> fmtFieldTypeMap, descrMap, defValMap, identityMap;
  QMap<Oid, QMap<int, Oid>> attTypeIdMap;
  QMap<Oid, QMap<int, bool>> notNullMap, uniqueMap;
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
      )
              .arg( connectionRO()->pgVersion() >= 100000 ? u", attidentity"_s : QString() )
              .arg( tableoidsFilter );

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

    bool isDomain = ( typeMap.value( atttypid ).typeType == "d"_L1 );

    QString formattedFieldType = fmtFieldTypeMap[tableoid][attnum];
    QString originalFormattedFieldType = formattedFieldType;
    if ( isDomain )
    {
      // get correct formatted field type for domain
      sql = u"SELECT format_type(%1, %2)"_s.arg( fldtyp ).arg( fldMod );
      QgsPostgresResult fmtFieldModResult( connectionRO()->PQexec( sql ) );
      if ( fmtFieldModResult.PQntuples() > 0 )
      {
        formattedFieldType = fmtFieldModResult.PQgetvalue( 0, 0 );
      }
    }

    QString fieldComment = descrMap[tableoid][attnum];

    QMetaType::Type fieldType;
    QMetaType::Type fieldSubType = QMetaType::Type::UnknownType;

    if ( fieldTType == "b"_L1 )
    {
      bool isArray = fieldTypeName.startsWith( '_' );

      if ( isArray )
        fieldTypeName = fieldTypeName.mid( 1 );

      if ( fieldTypeName == "int8"_L1 || fieldTypeName == "serial8"_L1 )
      {
        fieldType = QMetaType::Type::LongLong;
        fieldSize = -1;
        fieldPrec = 0;
      }
      else if ( fieldTypeName == "int2"_L1 || fieldTypeName == "int4"_L1 || fieldTypeName == "oid"_L1 || fieldTypeName == "serial"_L1 )
      {
        fieldType = QMetaType::Type::Int;
        fieldSize = -1;
        fieldPrec = 0;
      }
      else if ( fieldTypeName == "real"_L1 || fieldTypeName == "double precision"_L1 || fieldTypeName == "float4"_L1 || fieldTypeName == "float8"_L1 )
      {
        fieldType = QMetaType::Type::Double;
        fieldSize = -1;
        fieldPrec = 0;
      }
      else if ( fieldTypeName == "numeric"_L1 )
      {
        fieldType = QMetaType::Type::Double;

        if ( formattedFieldType == "numeric"_L1 || formattedFieldType.isEmpty() )
        {
          fieldSize = -1;
          fieldPrec = 0;
        }
        else
        {
          const thread_local QRegularExpression re( QRegularExpression::anchoredPattern( u"numeric\\((\\d+),(\\d+)\\)"_s ) );
          const QRegularExpressionMatch match = re.match( formattedFieldType );
          if ( match.hasMatch() )
          {
            fieldSize = match.captured( 1 ).toInt();
            fieldPrec = match.captured( 2 ).toInt();
          }
          else if ( formattedFieldType != "numeric"_L1 )
          {
            QgsMessageLog::logMessage( tr( "Unexpected formatted field type '%1' for field %2" ).arg( formattedFieldType, fieldName ), tr( "PostGIS" ) );
            fieldSize = -1;
            fieldPrec = 0;
          }
        }
      }
      else if ( fieldTypeName == "money"_L1 )
      {
        fieldType = QMetaType::Type::Double;
        fieldSize = -1;
        fieldPrec = 2;
      }
      else if ( fieldTypeName == "varchar"_L1 )
      {
        fieldType = QMetaType::Type::QString;

        const thread_local QRegularExpression re( QRegularExpression::anchoredPattern( u"character varying\\((\\d+)\\)"_s ) );
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
      else if ( fieldTypeName == "date"_L1 )
      {
        fieldType = QMetaType::Type::QDate;
        fieldSize = -1;
      }
      else if ( fieldTypeName == "time"_L1 )
      {
        fieldType = QMetaType::Type::QTime;
        fieldSize = -1;
      }
      else if ( fieldTypeName == "timestamp"_L1 )
      {
        fieldType = QMetaType::Type::QDateTime;
        fieldSize = -1;
      }
      else if ( fieldTypeName == "bytea"_L1 )
      {
        fieldType = QMetaType::Type::QByteArray;
        fieldSize = -1;
      }
      else if ( fieldTypeName == "text"_L1 || fieldTypeName == "citext"_L1 || fieldTypeName == "geometry"_L1 || fieldTypeName == "inet"_L1 || fieldTypeName == "ltree"_L1 || fieldTypeName == "uuid"_L1 || fieldTypeName == "xml"_L1 || fieldTypeName.startsWith( "time"_L1 ) || fieldTypeName.startsWith( "date"_L1 ) )
      {
        fieldType = QMetaType::Type::QString;
        fieldSize = -1;
      }
      else if ( fieldTypeName == "bpchar"_L1 )
      {
        // although postgres internally uses "bpchar", this is exposed to users as character in postgres
        fieldTypeName = u"character"_s;

        fieldType = QMetaType::Type::QString;

        const thread_local QRegularExpression re( QRegularExpression::anchoredPattern( u"character\\((\\d+)\\)"_s ) );
        const QRegularExpressionMatch match = re.match( formattedFieldType );
        if ( match.hasMatch() )
        {
          fieldSize = match.captured( 1 ).toInt();
        }
        else
        {
          QgsDebugError( u"Unexpected formatted field type '%1' for field %2"_s
                           .arg( formattedFieldType, fieldName ) );
          fieldSize = -1;
          fieldPrec = 0;
        }
      }
      else if ( fieldTypeName == "char"_L1 )
      {
        fieldType = QMetaType::Type::QString;

        const thread_local QRegularExpression re( QRegularExpression::anchoredPattern( u"char\\((\\d+)\\)"_s ) );
        const QRegularExpressionMatch match = re.match( formattedFieldType );
        if ( match.hasMatch() )
        {
          fieldSize = match.captured( 1 ).toInt();
        }
        else
        {
          QgsMessageLog::logMessage( tr( "Unexpected formatted field type '%1' for field %2" )
                                       .arg( formattedFieldType, fieldName ) );
          fieldSize = -1;
          fieldPrec = 0;
        }
      }
      else if ( fieldTypeName == "hstore"_L1 || fieldTypeName == "json"_L1 || fieldTypeName == "jsonb"_L1 )
      {
        fieldType = QMetaType::Type::QVariantMap;
        fieldSubType = QMetaType::Type::QString;
        fieldSize = -1;
      }
      else if ( fieldTypeName == "bool"_L1 )
      {
        // enum
        fieldType = QMetaType::Type::Bool;
        fieldSize = -1;
      }
      else
      {
        // be tolerant in case of views: this might be a field used as a key
        const Qgis::PostgresRelKind type = relkind();
        if ( ( type == Qgis::PostgresRelKind::View || type == Qgis::PostgresRelKind::MaterializedView )
             && parseUriKey( mUri.keyColumn() ).contains( fieldName ) )
        {
          // Assume it is convertible to text
          fieldType = QMetaType::Type::QString;
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
        fieldType = ( fieldType == QMetaType::Type::QString ? QMetaType::Type::QStringList : QMetaType::Type::QVariantList );
        fieldSize = -1;
      }
    }
    else if ( fieldTType == "e"_L1 )
    {
      // enum
      fieldType = QMetaType::Type::QString;
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
    if ( !identityMap[tableoid][attnum].isEmpty()
         && notNullMap[tableoid][attnum]
         && uniqueMap[tableoid][attnum]
         && defValMap[tableoid][attnum].isEmpty() )
    {
      const QString seqName { mTableName + '_' + fieldName + u"_seq"_s };
      const QString seqSql = QStringLiteral( "SELECT c.oid "
                                             "  FROM pg_class c "
                                             "  LEFT JOIN pg_namespace n "
                                             "    ON ( n.oid = c.relnamespace ) "
                                             "  WHERE c.relkind = 'S' "
                                             "    AND c.relname = %1 "
                                             "    AND n.nspname = %2" )
                               .arg( quotedValue( seqName ), quotedValue( mSchemaName ) );
      QgsPostgresResult seqResult( connectionRO()->PQexec( seqSql ) );
      if ( seqResult.PQntuples() == 1 )
      {
        defValMap[tableoid][attnum] = u"nextval(%1::regclass)"_s.arg( quotedIdentifier( seqName ) );
      }
    }

    mDefaultValues.insert( mAttributeFields.size(), defValMap[tableoid][attnum] );

    QgsField newField = QgsField( fieldName, fieldType, fieldTypeName, fieldSize, fieldPrec, fieldComment, fieldSubType );

    QgsFieldConstraints constraints;
    if ( notNullMap[tableoid][attnum] || ( mPrimaryKeyAttrs.size() == 1 && mPrimaryKeyAttrs[0] == i ) || identityMap[tableoid][attnum] != ' ' )
      constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
    if ( uniqueMap[tableoid][attnum] || ( mPrimaryKeyAttrs.size() == 1 && mPrimaryKeyAttrs[0] == i ) || identityMap[tableoid][attnum] != ' ' )
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
  if ( key.isEmpty() )
    return QStringList();

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

Qgis::PostgresRelKind QgsPostgresRasterProvider::relkind() const
{
  if ( mIsQuery || !connectionRO() )
    return Qgis::PostgresRelKind::Unknown;

  QString sql = u"SELECT relkind FROM pg_class WHERE oid=regclass(%1)::oid"_s.arg( quotedValue( mQuery ) );
  QgsPostgresResult res( connectionRO()->PQexec( sql ) );
  QString type = res.PQgetvalue( 0, 0 );

  return QgsPostgresConn::relKindFromValue( type );
}

bool QgsPostgresRasterProvider::determinePrimaryKey()
{
  if ( !loadFields() )
  {
    return false;
  }

  // check to see if there is an unique index on the relation, which
  // can be used as a key into the table. Primary keys are always
  // unique indices, so we catch them as well.

  QString sql;

  mPrimaryKeyAttrs.clear();

  if ( !mIsQuery )
  {
    sql = u"SELECT count(*) FROM pg_inherits WHERE inhparent=%1::regclass"_s.arg( quotedValue( mQuery ) );
    QgsDebugMsgLevel( u"Checking whether %1 is a parent table"_s.arg( sql ), 4 );
    QgsPostgresResult res( connectionRO()->PQexec( sql ) );
    bool isParentTable( res.PQntuples() == 0 || res.PQgetvalue( 0, 0 ).toInt() > 0 );

    sql = u"SELECT indexrelid FROM pg_index WHERE indrelid=%1::regclass AND (indisprimary OR indisunique) ORDER BY CASE WHEN indisprimary THEN 1 ELSE 2 END LIMIT 1"_s.arg( quotedValue( mQuery ) );
    QgsDebugMsgLevel( u"Retrieving first primary or unique index: %1"_s.arg( sql ), 4 );

    res = connectionRO()->PQexec( sql );
    QgsDebugMsgLevel( u"Got %1 rows."_s.arg( res.PQntuples() ), 4 );

    // no primary or unique indices found
    if ( res.PQntuples() == 0 )
    {
      QgsDebugMsgLevel( u"Relation has no primary key -- investigating alternatives"_s, 4 );

      // Two options here. If the relation is a table, see if there is
      // an oid column that can be used instead.
      // If the relation is a view try to find a suitable column to use as
      // the primary key.

      const Qgis::PostgresRelKind type = relkind();

      if ( type == Qgis::PostgresRelKind::OrdinaryTable || type == Qgis::PostgresRelKind::PartitionedTable )
      {
        QgsDebugMsgLevel( u"Relation is a table. Checking to see if it has an oid column."_s, 4 );

        mPrimaryKeyAttrs.clear();
        mPrimaryKeyType = PktUnknown;

        if ( connectionRO()->pgVersion() >= 100000 )
        {
          // If there is an generated id on the table, use that instead,
          sql = u"SELECT attname FROM pg_attribute WHERE attidentity IN ('a','d') AND attrelid=regclass(%1) LIMIT 1"_s.arg( quotedValue( mQuery ) );
          res = connectionRO()->PQexec( sql );
          if ( res.PQntuples() == 1 )
          {
            // Could warn the user here that performance will suffer if
            // attribute isn't indexed (and that they may want to add a
            // primary key to the table)
            const QString keyName { res.PQgetvalue( 0, 0 ) };
            Q_ASSERT( mAttributeFields.indexFromName( keyName ) >= 0 );
            mPrimaryKeyAttrs << mAttributeFields.indexFromName( keyName );
          }
        }

        if ( mPrimaryKeyType == PktUnknown )
        {
          // If there is an oid on the table, use that instead,
          sql = u"SELECT attname FROM pg_attribute WHERE attname='oid' AND attrelid=regclass(%1)"_s.arg( quotedValue( mQuery ) );

          res = connectionRO()->PQexec( sql );
          if ( res.PQntuples() == 1 )
          {
            // Could warn the user here that performance will suffer if
            // oid isn't indexed (and that they may want to add a
            // primary key to the table)
            mPrimaryKeyType = PktOid;
            mPrimaryKeyAttrs.clear();
          }
        }

        if ( mPrimaryKeyType == PktUnknown )
        {
          sql = u"SELECT attname FROM pg_attribute WHERE attname='ctid' AND attrelid=regclass(%1)"_s.arg( quotedValue( mQuery ) );

          res = connectionRO()->PQexec( sql );
          if ( res.PQntuples() == 1 )
          {
            mPrimaryKeyType = PktTid;
            QgsMessageLog::logMessage( tr( "Primary key is ctid - changing of existing features disabled (%1; %2)" ).arg( mRasterColumn, mQuery ) );
            // TODO: set capabilities to RO when writing will be implemented
            mPrimaryKeyAttrs.clear();
          }
        }

        if ( mPrimaryKeyType == PktUnknown )
        {
          QgsMessageLog::logMessage( tr( "The table has no column suitable for use as a key. QGIS requires a primary key, a PostgreSQL oid column or a ctid for tables." ), tr( "PostGIS" ) );
        }
      }
      else if ( type == Qgis::PostgresRelKind::View || type == Qgis::PostgresRelKind::MaterializedView
                || type == Qgis::PostgresRelKind::ForeignTable )
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
      QgsDebugMsgLevel( u"Got %1 rows."_s.arg( res.PQntuples() ), 4 );

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

        if ( fieldTypeName == "oid"_L1 )
        {
          pkType = QgsPostgresPrimaryKeyType::PktOid;
        }
        else if ( fieldTypeName == "integer"_L1 )
        {
          pkType = QgsPostgresPrimaryKeyType::PktInt;
        }
        else if ( fieldTypeName == "bigint"_L1 )
        {
          pkType = QgsPostgresPrimaryKeyType::PktUint64;
        }
        else if ( fieldTypeName == "text"_L1 )
        {
          pkType = QgsPostgresPrimaryKeyType::PktFidMap;
        }
        // Always use PktFidMap for multi-field keys
        mPrimaryKeyType = i ? QgsPostgresPrimaryKeyType::PktFidMap : pkType;
        Q_ASSERT( mAttributeFields.indexFromName( name ) >= 0 );
        mPrimaryKeyAttrs << mAttributeFields.indexFromName( name );
      }

      if ( mightBeNull || isParentTable )
      {
        QgsMessageLog::logMessage( tr( "Ignoring key candidate because of NULL values or inherited table" ), tr( "PostGIS" ), Qgis::MessageLevel::Info );
        mPrimaryKeyType = PktUnknown;
        mPrimaryKeyAttrs.clear();
      }
    }
  }
  else
  {
    determinePrimaryKeyFromUriKeyColumn();
  }

  return mPrimaryKeyType != PktUnknown;
}

void QgsPostgresRasterProvider::determinePrimaryKeyFromUriKeyColumn()
{
  QString primaryKey = mUri.keyColumn();
  mPrimaryKeyType = PktUnknown;

  if ( !primaryKey.isEmpty() )
  {
    const QStringList cols = parseUriKey( primaryKey );

    primaryKey.clear();
    QString del;
    for ( const QString &col : cols )
    {
      primaryKey += del + quotedIdentifier( col );
      del = u","_s;
    }

    for ( const QString &col : cols )
    {
      int idx = fields().lookupField( col );
      if ( idx < 0 )
      {
        QgsMessageLog::logMessage( tr( "Key field '%1' for view/query not found." ).arg( col ), tr( "PostGIS" ) );
        mPrimaryKeyAttrs.clear();
        break;
      }

      mPrimaryKeyAttrs << idx;
    }

    if ( !mPrimaryKeyAttrs.isEmpty() )
    {
      if ( mUseEstimatedMetadata )
      {
        mPrimaryKeyType = PktFidMap; // Map by default
        if ( mPrimaryKeyAttrs.size() == 1 )
        {
          QgsField fld = mAttributeFields.at( mPrimaryKeyAttrs.at( 0 ) );
          mPrimaryKeyType = pkType( fld );
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Primary key field '%1' for view/query not unique." ).arg( primaryKey ), tr( "PostGIS" ) );
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Keys for view/query undefined." ), tr( "PostGIS" ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "No key field for view/query given." ), tr( "PostGIS" ) );
  }
}


QString QgsPostgresRasterProvider::pkSql() const
{
  switch ( mPrimaryKeyType )
  {
    case QgsPostgresPrimaryKeyType::PktOid:
      return u"oid"_s;
    case QgsPostgresPrimaryKeyType::PktTid:
      return u"ctid"_s;
    default:
    {
      if ( mPrimaryKeyAttrs.count() > 1 )
      {
        QStringList pkeys;
        for ( const int &keyIndex : std::as_const( mPrimaryKeyAttrs ) )
        {
          if ( mAttributeFields.exists( keyIndex ) )
          {
            pkeys.push_back( quotedIdentifier( mAttributeFields.at( keyIndex ).name() ) );
          }
          else
          {
            QgsDebugError( u"Attribute not found %1"_s.arg( keyIndex ) );
          }
        }
        return pkeys.join( ',' ).prepend( '(' ).append( ')' );
      }
      return mAttributeFields.exists( mPrimaryKeyAttrs.first() ) ? quotedIdentifier( mAttributeFields.at( mPrimaryKeyAttrs.first() ).name() ) : QString();
    }
  }
}

QString QgsPostgresRasterProvider::dataComment() const
{
  return mDataComment;
}

void QgsPostgresRasterProvider::findOverviews()
{
  const QString sql = QStringLiteral( "SELECT overview_factor, o_table_schema, o_table_name, o_raster_column "
                                      "FROM raster_overviews WHERE r_table_schema = %1 AND r_table_name = %2" )
                        .arg( quotedValue( mSchemaName ), quotedValue( mTableName ) );

  //QgsDebugMsgLevel( u"Raster overview information sql: %1"_s.arg( sql ), 2 );
  QgsPostgresResult result( connectionRO()->PQexec( sql ) );
  if ( PGRES_TUPLES_OK == result.PQresultStatus() )
  {
    for ( int i = 0; i < result.PQntuples(); ++i )
    {
      bool ok;
      const unsigned int overViewFactor { static_cast<unsigned int>( result.PQgetvalue( i, 0 ).toInt( &ok ) ) };
      if ( !ok )
      {
        QgsMessageLog::logMessage( tr( "Cannot convert overview factor '%1' to int" ).arg( result.PQgetvalue( i, 0 ) ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
        return;
      }
      const QString schema { result.PQgetvalue( i, 1 ) };
      const QString table { result.PQgetvalue( i, 2 ) };
      if ( table.isEmpty() || schema.isEmpty() )
      {
        QgsMessageLog::logMessage( tr( "Table or schema is empty" ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
        return;
      }
      mOverViews[overViewFactor] = u"%1.%2"_s.arg( quotedIdentifier( schema ) ).arg( quotedIdentifier( table ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Error fetching overviews information: %1" ).arg( result.PQresultErrorMessage() ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
  }
  if ( mOverViews.isEmpty() )
  {
    QgsMessageLog::logMessage( tr( "No overviews found, performances may be affected for %1" ).arg( mQuery ), u"PostGIS"_s, Qgis::MessageLevel::Info );
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

QgsPostgresPrimaryKeyType QgsPostgresRasterProvider::pkType( const QgsField &fld )
{
  switch ( fld.type() )
  {
    case QMetaType::Type::LongLong:
      // PostgreSQL doesn't have native "unsigned" types.
      // Unsigned primary keys are emulated by the serial/bigserial
      // pseudo-types, in which autogenerated values are always > 0;
      // however, the database accepts manually inserted 0 and negative values
      // in these fields.
      return PktInt64;

    case QMetaType::Type::Int:
      return PktInt;

    default:
      return PktFidMap;
  }
}

Qgis::DataType QgsPostgresRasterProvider::sourceDataType( int bandNo ) const
{
  if ( bandNo <= mBandCount && static_cast<unsigned long>( bandNo ) <= mDataTypes.size() )
  {
    return mDataTypes[static_cast<unsigned long>( bandNo - 1 )];
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Data type is unknown" ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
    return Qgis::DataType::UnknownDataType;
  }
}

int QgsPostgresRasterProvider::xBlockSize() const
{
  return mTileWidth;
}

int QgsPostgresRasterProvider::yBlockSize() const
{
  return mTileHeight;
}

QgsRasterBandStats QgsPostgresRasterProvider::bandStatistics( int bandNo, Qgis::RasterBandStatistics stats, const QgsRectangle &extent, int sampleSize, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( feedback )
  QgsRasterBandStats rasterBandStats;
  const auto constMStatistics = mStatistics;
  initStatistics( rasterBandStats, bandNo, stats, extent, sampleSize );
  for ( const QgsRasterBandStats &stats : constMStatistics )
  {
    if ( stats.contains( rasterBandStats ) )
    {
      QgsDebugMsgLevel( u"Using cached statistics."_s, 4 );
      return stats;
    }
  }

  QString tableToQuery { mQuery };
  const double pixelsRatio { static_cast<double>( sampleSize ) / ( mWidth * mHeight ) };
  double statsRatio { pixelsRatio };

  // Decide if overviews can be used here
  if ( subsetString().isEmpty() && !mIsQuery && mIsTiled && extent.isEmpty() )
  {
    const unsigned int desiredOverviewFactor { static_cast<unsigned int>( 1.0 / sqrt( pixelsRatio ) ) };
    const auto ovKeys { mOverViews.keys() };
    QList<unsigned int>::const_reverse_iterator rit { ovKeys.rbegin() };
    for ( ; rit != ovKeys.rend(); ++rit )
    {
      if ( *rit <= desiredOverviewFactor )
      {
        tableToQuery = mOverViews[*rit];
        // This should really be: *= *rit * *rit;
        // but we are already approximating, let's get decent statistics
        statsRatio = 1;
        QgsDebugMsgLevel( u"Using overview for statistics read: %1"_s.arg( tableToQuery ), 3 );
        break;
      }
    }
  }

  // Query the backend
  QString where { extent.isEmpty() ? QString() : u"WHERE %1 && ST_GeomFromText( %2, %3 )"_s.arg( quotedIdentifier( mRasterColumn ) ).arg( quotedValue( extent.asWktPolygon() ) ).arg( mCrs.postgisSrid() ) };

  if ( !subsetString().isEmpty() )
  {
    where.append( where.isEmpty() ? u"WHERE %1"_s.arg( subsetString() ) : u" AND %1"_s.arg( subsetString() ) );
  }

  const QString sql = QStringLiteral( "SELECT (ST_SummaryStatsAgg( %1, %2, TRUE, %3 )).* "
                                      "FROM %4 %5" )
                        .arg( quotedIdentifier( mRasterColumn ) )
                        .arg( bandNo )
                        .arg( std::max<double>( 0, std::min<double>( 1, statsRatio ) ) )
                        .arg( tableToQuery, where );

  QgsPostgresResult result( connectionRO()->PQexec( sql ) );

  if ( PGRES_TUPLES_OK == result.PQresultStatus() && result.PQntuples() == 1 )
  {
    // count   |     sum     |       mean       |      stddev      | min | max
    rasterBandStats.sum = result.PQgetvalue( 0, 1 ).toDouble();
    rasterBandStats.mean = result.PQgetvalue( 0, 2 ).toDouble();
    rasterBandStats.stdDev = result.PQgetvalue( 0, 3 ).toDouble();
    rasterBandStats.minimumValue = result.PQgetvalue( 0, 4 ).toDouble();
    rasterBandStats.maximumValue = result.PQgetvalue( 0, 5 ).toDouble();
    rasterBandStats.range = rasterBandStats.maximumValue - rasterBandStats.minimumValue;
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Error fetching statistics for %1: %2\nSQL: %3" ).arg( mQuery ).arg( result.PQresultErrorMessage() ).arg( sql ), u"PostGIS"_s, Qgis::MessageLevel::Warning );
  }

  QgsDebugMsgLevel( u"************ STATS **************"_s, 4 );
  QgsDebugMsgLevel( u"MIN %1"_s.arg( rasterBandStats.minimumValue ), 4 );
  QgsDebugMsgLevel( u"MAX %1"_s.arg( rasterBandStats.maximumValue ), 4 );
  QgsDebugMsgLevel( u"RANGE %1"_s.arg( rasterBandStats.range ), 4 );
  QgsDebugMsgLevel( u"MEAN %1"_s.arg( rasterBandStats.mean ), 4 );
  QgsDebugMsgLevel( u"STDDEV %1"_s.arg( rasterBandStats.stdDev ), 4 );

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

QgsLayerMetadata QgsPostgresRasterProvider::layerMetadata() const
{
  return mLayerMetadata;
}

Qgis::ProviderStyleStorageCapabilities QgsPostgresRasterProvider::styleStorageCapabilities() const
{
  Qgis::ProviderStyleStorageCapabilities storageCapabilities;
  if ( isValid() )
  {
    storageCapabilities |= Qgis::ProviderStyleStorageCapability::SaveToDatabase;
    storageCapabilities |= Qgis::ProviderStyleStorageCapability::LoadFromDatabase;
    storageCapabilities |= Qgis::ProviderStyleStorageCapability::DeleteFromDatabase;
  }
  return storageCapabilities;
}


bool QgsPostgresRasterProviderMetadata::styleExists( const QString &uri, const QString &styleId, QString &errorCause )
{
  errorCause.clear();

  QgsDataSourceUri dsUri( uri );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, true );
  if ( !conn )
  {
    errorCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  if ( !QgsPostgresUtils::tableExists( conn, u"public"_s, u"layer_styles"_s ) || !QgsPostgresUtils::columnExists( conn, u"public"_s, u"layer_styles"_s, u"type"_s ) || !QgsPostgresUtils::columnExists( conn, u"public"_s, u"layer_styles"_s, u"r_raster_column"_s ) )
  {
    return false;
  }

  if ( dsUri.database().isEmpty() ) // typically when a service file is used
  {
    dsUri.setDatabase( conn->currentDatabase() );
  }

  const QString checkQuery = QString( "SELECT styleName"
                                      " FROM layer_styles"
                                      " WHERE f_table_catalog=%1"
                                      " AND f_table_schema=%2"
                                      " AND f_table_name=%3"
                                      " AND f_geometry_column IS NULL"
                                      " AND (type=%4 OR type IS NULL)"
                                      " AND styleName=%5"
                                      " AND r_raster_column %6" )
                               .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                               .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                               .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                               .arg( QgsPostgresConn::quotedValue( mType ) )
                               .arg( QgsPostgresConn::quotedValue( styleId.isEmpty() ? dsUri.table() : styleId ) )
                               .arg( dsUri.geometryColumn().isEmpty() ? u"IS NULL"_s : u"= %1"_s.arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) ) );

  QgsPostgresResult res( conn->LoggedPQexec( u"QgsPostgresRasterProviderMetadata"_s, checkQuery ) );
  if ( res.PQresultStatus() == PGRES_TUPLES_OK )
  {
    return res.PQntuples() > 0;
  }
  else
  {
    errorCause = res.PQresultErrorMessage();
    return false;
  }
}

bool QgsPostgresRasterProviderMetadata::saveStyle( const QString &uri, const QString &qmlStyleIn, const QString &sldStyleIn, const QString &styleName, const QString &styleDescription, const QString &uiFileContent, bool useAsDefault, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );

  // Replace invalid XML characters
  QString qmlStyle { qmlStyleIn };
  QgsPostgresUtils::replaceInvalidXmlChars( qmlStyle );
  QString sldStyle { sldStyleIn };
  QgsPostgresUtils::replaceInvalidXmlChars( sldStyle );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  if ( !QgsPostgresUtils::tableExists( conn, u"public"_s, u"layer_styles"_s ) )
  {
    if ( !QgsPostgresUtils::createStylesTable( conn, u"QgsPostgresRasterProviderMetadata"_s ) )
    {
      errCause = QObject::tr( "Unable to save layer style. It's not possible to create the destination table on the database. Maybe this is due to table permissions (user=%1). Please contact your database admin" ).arg( dsUri.username() );
      conn->unref();
      return false;
    }
  }
  else
  {
    if ( !QgsPostgresUtils::columnExists( conn, u"public"_s, u"layer_styles"_s, u"type"_s ) )
    {
      QgsPostgresResult res( conn->LoggedPQexec( u"QgsPostgresRasterProviderMetadata"_s, "ALTER TABLE layer_styles ADD COLUMN type varchar NULL" ) );
      if ( res.PQresultStatus() != PGRES_COMMAND_OK )
      {
        errCause = QObject::tr( "Unable to add column type to layer_styles table. Maybe this is due to table permissions (user=%1). Please contact your database admin" ).arg( dsUri.username() );
        conn->unref();
        return false;
      }
    }
  }

  if ( !QgsPostgresUtils::columnExists( conn, u"public"_s, u"layer_styles"_s, u"r_raster_column"_s ) )
  {
    QgsPostgresResult res( conn->LoggedPQexec( u"QgsPostgresRasterProviderMetadata"_s, "ALTER TABLE layer_styles ADD COLUMN r_raster_column varchar NULL" ) );
    if ( res.PQresultStatus() != PGRES_COMMAND_OK )
    {
      errCause = QObject::tr( "Unable to add column r_raster_column to layer_styles table. Maybe this is due to table permissions (user=%1). Please contact your database admin" ).arg( dsUri.username() );
      conn->unref();
      return false;
    }
  }

  if ( dsUri.database().isEmpty() ) // typically when a service file is used
  {
    dsUri.setDatabase( conn->currentDatabase() );
  }

  QString uiFileColumn;
  QString uiFileValue;
  if ( !uiFileContent.isEmpty() )
  {
    uiFileColumn = u",ui"_s;
    uiFileValue = u",XMLPARSE(DOCUMENT %1)"_s.arg( QgsPostgresConn::quotedValue( uiFileContent ) );
  }

  // Note: in the construction of the INSERT and UPDATE strings the qmlStyle and sldStyle values
  // can contain user entered strings, which may themselves include %## values that would be
  // replaced by the QString.arg function.  To ensure that the final SQL string is not corrupt these
  // two values are both replaced in the final .arg call of the string construction.

  QString sql = QString( "INSERT INTO layer_styles("
                         "f_table_catalog,f_table_schema,f_table_name,f_geometry_column,styleName,styleQML,styleSLD,useAsDefault,description,owner,type%12,r_raster_column"
                         ") VALUES ("
                         "%1,%2,%3,%4,%5,XMLPARSE(DOCUMENT %16),XMLPARSE(DOCUMENT %17),%8,%9,%10,%11%13,%14"
                         ")" )
                  .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                  .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                  .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                  .arg( "NULL"_L1 )
                  .arg( QgsPostgresConn::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) )
                  .arg( useAsDefault ? "true" : "false" )
                  .arg( QgsPostgresConn::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
                  .arg( "CURRENT_USER" )
                  .arg( uiFileColumn )
                  .arg( uiFileValue )
                  .arg( QgsPostgresConn::quotedValue( mType ) )
                  .arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) )
                  // Must be the final .arg replacement - see above
                  .arg( QgsPostgresConn::quotedValue( qmlStyle ), QgsPostgresConn::quotedValue( sldStyle ) );


  QString checkQuery = QString( "SELECT styleName"
                                " FROM layer_styles"
                                " WHERE f_table_catalog=%1"
                                " AND f_table_schema=%2"
                                " AND f_table_name=%3"
                                " AND f_geometry_column IS NULL"
                                " AND (type=%4 OR type IS NULL)"
                                " AND styleName=%5"
                                " AND r_raster_column %6" )
                         .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                         .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                         .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                         .arg( QgsPostgresConn::quotedValue( mType ) )
                         .arg( QgsPostgresConn::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) )
                         .arg( dsUri.geometryColumn().isEmpty() ? u"IS NULL"_s : u"= %1"_s.arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) ) );

  QgsPostgresResult res( conn->LoggedPQexec( "QgsPostgresRasterProviderMetadata", checkQuery ) );
  if ( res.PQntuples() > 0 )
  {
    sql = QString( "UPDATE layer_styles"
                   " SET useAsDefault=%1"
                   ",styleQML=XMLPARSE(DOCUMENT %12)"
                   ",styleSLD=XMLPARSE(DOCUMENT %13)"
                   ",description=%4"
                   ",owner=%5"
                   ",type=%2"
                   " WHERE f_table_catalog=%6"
                   " AND f_table_schema=%7"
                   " AND f_table_name=%8"
                   " AND f_geometry_column IS NULL"
                   " AND styleName=%9"
                   " AND (type=%2 OR type IS NULL)"
                   " AND r_raster_column %14" )
            .arg( useAsDefault ? "true" : "false" )
            .arg( QgsPostgresConn::quotedValue( mType ) )
            .arg( QgsPostgresConn::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
            .arg( "CURRENT_USER" )
            .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
            .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
            .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
            .arg( QgsPostgresConn::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) )
            // Must be the final .arg replacement - see above
            .arg( QgsPostgresConn::quotedValue( qmlStyle ), QgsPostgresConn::quotedValue( sldStyle ) )
            .arg( dsUri.geometryColumn().isEmpty() ? u"IS NULL"_s : u"= %1"_s.arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) ) );
  }

  if ( useAsDefault )
  {
    QString removeDefaultSql = QString( "UPDATE layer_styles"
                                        " SET useAsDefault=false"
                                        " WHERE f_table_catalog=%1"
                                        " AND f_table_schema=%2"
                                        " AND f_table_name=%3"
                                        " AND f_geometry_column IS NULL"
                                        " AND (type=%4 OR type IS NULL)"
                                        " AND r_raster_column %5" )
                                 .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                                 .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                                 .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                                 .arg( QgsPostgresConn::quotedValue( mType ) )
                                 .arg( dsUri.geometryColumn().isEmpty() ? u"IS NULL"_s : u"= %1"_s.arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) ) );

    sql = u"BEGIN; %1; %2; COMMIT;"_s.arg( removeDefaultSql, sql );
  }

  res = conn->LoggedPQexec( "QgsPostgresRasterProviderMetadata", sql );

  bool saved = res.PQresultStatus() == PGRES_COMMAND_OK;
  if ( !saved )
    errCause = QObject::tr( "Unable to save layer style. It's not possible to insert a new record into the style table. Maybe this is due to table permissions (user=%1). Please contact your database administrator." ).arg( dsUri.username() );

  conn->unref();

  return saved;
}


QString QgsPostgresRasterProviderMetadata::loadStyle( const QString &uri, QString &errCause )
{
  QString styleName;
  return loadStoredStyle( uri, styleName, errCause );
}

QString QgsPostgresRasterProviderMetadata::loadStoredStyle( const QString &uri, QString &styleName, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString selectQmlQuery;

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, true );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return QString();
  }

  if ( dsUri.database().isEmpty() ) // typically when a service file is used
  {
    dsUri.setDatabase( conn->currentDatabase() );
  }

  if ( !QgsPostgresUtils::tableExists( conn, u"public"_s, u"layer_styles"_s ) )
  {
    conn->unref();
    return QString();
  }
  else if ( !QgsPostgresUtils::columnExists( conn, u"public"_s, u"layer_styles"_s, u"r_raster_column"_s ) )
  {
    return QString();
  }

  // support layer_styles without type column < 3.14
  if ( !QgsPostgresUtils::columnExists( conn, u"public"_s, u"layer_styles"_s, u"type"_s ) )
  {
    selectQmlQuery = QString( "SELECT styleName, styleQML"
                              " FROM layer_styles"
                              " WHERE f_table_catalog=%1"
                              " AND f_table_schema=%2"
                              " AND f_table_name=%3"
                              " AND f_geometry_column IS NULL"
                              " AND r_raster_column %4"
                              " ORDER BY CASE WHEN useAsDefault THEN 1 ELSE 2 END"
                              ",update_time DESC LIMIT 1" )
                       .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                       .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                       .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                       .arg( dsUri.geometryColumn().isEmpty() ? u"IS NULL"_s : u"= %1"_s.arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) ) );
  }
  else
  {
    selectQmlQuery = QString( "SELECT styleName, styleQML"
                              " FROM layer_styles"
                              " WHERE f_table_catalog=%1"
                              " AND f_table_schema=%2"
                              " AND f_table_name=%3"
                              " AND f_geometry_column IS NULL"
                              " AND (type=%4 OR type IS NULL)"
                              " AND r_raster_column %5"
                              " ORDER BY CASE WHEN useAsDefault THEN 1 ELSE 2 END"
                              ",update_time DESC LIMIT 1" )
                       .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                       .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                       .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                       .arg( QgsPostgresConn::quotedValue( mType ) )
                       .arg( dsUri.geometryColumn().isEmpty() ? u"IS NULL"_s : u"= %1"_s.arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) ) );
  }

  QgsPostgresResult result( conn->LoggedPQexec( u"QgsPostgresRasterProviderMetadata"_s, selectQmlQuery ) );

  styleName = result.PQntuples() == 1 ? result.PQgetvalue( 0, 0 ) : QString();
  QString style = result.PQntuples() == 1 ? result.PQgetvalue( 0, 1 ) : QString();
  conn->unref();

  QgsPostgresUtils::restoreInvalidXmlChars( style );

  return style;
}

int QgsPostgresRasterProviderMetadata::listStyles( const QString &uri, QStringList &ids, QStringList &names, QStringList &descriptions, QString &errCause )
{
  errCause.clear();
  QgsDataSourceUri dsUri( uri );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, true );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed using username: %1" ).arg( dsUri.username() );
    return -1;
  }

  if ( !QgsPostgresUtils::tableExists( conn, u"public"_s, u"layer_styles"_s ) )
  {
    return -1;
  }

  if ( !QgsPostgresUtils::columnExists( conn, u"public"_s, u"layer_styles"_s, u"r_raster_column"_s ) )
  {
    return false;
  }

  if ( dsUri.database().isEmpty() ) // typically when a service file is used
  {
    dsUri.setDatabase( conn->currentDatabase() );
  }

  QString selectRelatedQuery = QString( "SELECT id,styleName,description"
                                        " FROM layer_styles"
                                        " WHERE f_table_catalog=%1"
                                        " AND f_table_schema=%2"
                                        " AND f_table_name=%3"
                                        " AND f_geometry_column is NULL"
                                        " AND (type=%4 OR type IS NULL)"
                                        " AND r_raster_column %5"
                                        " ORDER BY useasdefault DESC, update_time DESC" )
                                 .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                                 .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                                 .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                                 .arg( QgsPostgresConn::quotedValue( mType ) )
                                 .arg( dsUri.geometryColumn().isEmpty() ? u"IS NULL"_s : u"= %1"_s.arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) ) );

  QgsPostgresResult result( conn->LoggedPQexec( u"QgsPostgresRasterProviderMetadata"_s, selectRelatedQuery ) );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectRelatedQuery ) );
    errCause = QObject::tr( "Error executing the select query for related styles. The query was logged" );
    conn->unref();
    return -1;
  }

  int numberOfRelatedStyles = result.PQntuples();
  for ( int i = 0; i < numberOfRelatedStyles; i++ )
  {
    ids.append( result.PQgetvalue( i, 0 ) );
    names.append( result.PQgetvalue( i, 1 ) );
    descriptions.append( result.PQgetvalue( i, 2 ) );
  }

  QString selectOthersQuery = QString( "SELECT id,styleName,description"
                                       " FROM layer_styles"
                                       " WHERE NOT (f_table_catalog=%1 AND f_table_schema=%2 AND f_table_name=%3 AND f_geometry_column IS NULL AND type=%4 AND r_raster_column=%5)"
                                       " ORDER BY update_time DESC" )
                                .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                                .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                                .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                                .arg( QgsPostgresConn::quotedValue( mType ) )
                                .arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) );

  result = conn->LoggedPQexec( u"QgsPostgresRasterProviderMetadata"_s, selectOthersQuery );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectOthersQuery ) );
    errCause = QObject::tr( "Error executing the select query for unrelated styles. The query was logged" );
    conn->unref();
    return -1;
  }

  for ( int i = 0; i < result.PQntuples(); i++ )
  {
    ids.append( result.PQgetvalue( i, 0 ) );
    names.append( result.PQgetvalue( i, 1 ) );
    descriptions.append( result.PQgetvalue( i, 2 ) );
  }

  conn->unref();

  return numberOfRelatedStyles;
}

bool QgsPostgresRasterProviderMetadata::deleteStyleById( const QString &uri, const QString &styleId, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  bool deleted;

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed using username: %1" ).arg( dsUri.username() );
    deleted = false;
  }
  else
  {
    QString deleteStyleQuery = u"DELETE FROM layer_styles WHERE id=%1"_s.arg( QgsPostgresConn::quotedValue( styleId ) );
    QgsPostgresResult result( conn->LoggedPQexec( u"QgsPostgresRasterProviderMetadata"_s, deleteStyleQuery ) );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
    {
      QgsDebugError(
        QString( "PQexec of this query returning != PGRES_COMMAND_OK (%1 != expected %2): %3" )
          .arg( result.PQresultStatus() )
          .arg( PGRES_COMMAND_OK )
          .arg( deleteStyleQuery )
      );
      QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( deleteStyleQuery ) );
      errCause = QObject::tr( "Error executing the delete query. The query was logged" );
      deleted = false;
    }
    else
    {
      deleted = true;
    }
    conn->unref();
  }
  return deleted;
}

QString QgsPostgresRasterProviderMetadata::getStyleById( const QString &uri, const QString &styleId, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, true );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed using username: %1" ).arg( dsUri.username() );
    return QString();
  }

  QString style;
  QString selectQmlQuery = u"SELECT styleQml FROM layer_styles WHERE id=%1"_s.arg( QgsPostgresConn::quotedValue( styleId ) );
  QgsPostgresResult result( conn->LoggedPQexec( u"QgsPostgresRasterProviderMetadata"_s, selectQmlQuery ) );
  if ( result.PQresultStatus() == PGRES_TUPLES_OK )
  {
    if ( result.PQntuples() == 1 )
      style = result.PQgetvalue( 0, 0 );
    else
      errCause = QObject::tr( "Consistency error in table '%1'. Style id should be unique" ).arg( "layer_styles"_L1 );
  }
  else
  {
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectQmlQuery ) );
    errCause = QObject::tr( "Error executing the select query. The query was logged" );
  }

  conn->unref();

  QgsPostgresUtils::restoreInvalidXmlChars( style );

  return style;
}
