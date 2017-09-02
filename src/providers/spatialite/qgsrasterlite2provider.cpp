/***************************************************************************
     qgsrasterlite2provider.cpp
     --------------------------------------
    Date                 : 2017-08-29
    Copyright         : (C) 2017 by Mark Johnson, Berlin Germany
    Email                : mj10777 at googlemail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterlite2provider.h"
#include "qgsspatialiteconnpool.h"
#include "qgsdatasourceuri.h"
#include "qgsfeaturestore.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsrasteridentifyresult.h"
#include "qgssettings.h"

#include <cstring>
#if 0
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPainter>
#endif
#include <qmath.h>

//----------------------------------------------------------
//-- Mandatory functions for each Provider
//--> each Provider must be created in an extra library
//----------------------------------------------------------
const QString RASTERLITE2_KEY = QStringLiteral( "rasterlite2" );
const QString RASTERLITE2_DESCRIPTION = QStringLiteral( "Spatialite RasterLite2 data provider" );

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return RASTERLITE2_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return RASTERLITE2_DESCRIPTION;
}

/**
 * Class factory to return a pointer to a newly created
 * QgsRasterLite2Provider object
 */
QGISEXTERN QgsRasterLite2Provider *classFactory( const QString *uri )
{
  return new QgsRasterLite2Provider( *uri );
}
//----------------------------------------------------------
QgsRasterLite2Provider::QgsRasterLite2Provider( const QString &uri )
  : QgsRasterDataProvider( uri )
  , mValid( false )
  , mUriTableName( QString::null )
  , mUriSqlitePath( QString::null )
  , mDefaultImageBackground( QString( "#ffffff" ) )
{
  if ( dataSourceUri().startsWith( QStringLiteral( "RASTERLITE2:" ) ) )
  {
    QStringList sa_list_info = dataSourceUri().split( SpatialiteDbInfo::ParseSeparatorUris );
    if ( sa_list_info.size() == 3 )
    {
      // Note: the gdal notation will be retained, so that only one connection string exists. QgsDataSourceUri will fail.
      if ( sa_list_info.at( 0 ) == QStringLiteral( "RASTERLITE2" ) )
      {
        mUriSqlitePath = sa_list_info.at( 1 );
        mUriTableName = sa_list_info.at( 2 );
      }
    }
  }
  else
  {
    QgsDataSourceUri anUri = QgsDataSourceUri( dataSourceUri() );
    mUriTableName = anUri.table();
    mUriSqlitePath = anUri.database();
  }
  if ( ( !mUriSqlitePath.isEmpty() ) && ( !mUriSqlitePath.isEmpty() ) )
  {
    // trying to open the SQLite DB
    bool bShared = true;
    bool bLoadLayers = true;
    if ( setSqliteHandle( QgsSqliteHandle::openDb( mUriSqlitePath, bShared, mUriTableName, bLoadLayers ) ) )
    {
      mValid = true;
    }
  }
  mTimestamp = QDateTime::currentDateTime();
}

QgsRasterLite2Provider::~QgsRasterLite2Provider()
{
  closeDb();
  invalidateConnections( mUriSqlitePath );
}

QString QgsRasterLite2Provider::name() const
{
  return RASTERLITE2_KEY;
}

QString QgsRasterLite2Provider::description() const
{
  return RASTERLITE2_DESCRIPTION;
}

void QgsRasterLite2Provider::closeDb()
{
// trying to close the SQLite DB
  if ( mHandle )
  {
    QgsSqliteHandle::closeDb( mHandle );
    mHandle = nullptr;
  }
}

void QgsRasterLite2Provider::invalidateConnections( const QString &connection )
{
  QgsSpatiaLiteConnPool::instance()->invalidateConnections( connection );
}

int QgsRasterLite2Provider::capabilities() const
{
  int capability = QgsRasterDataProvider::Identify
                   | QgsRasterDataProvider::IdentifyValue
                   | QgsRasterDataProvider::Size
                   | QgsRasterDataProvider::BuildPyramids
                   | QgsRasterDataProvider::Create
                   | QgsRasterDataProvider::Remove;
  return capability;
}

bool QgsRasterLite2Provider::setSqliteHandle( QgsSqliteHandle *sqliteHandle )
{
  bool bRc = false;
  mHandle = sqliteHandle;
  bool bLoadLayer = true;
  if ( !mHandle )
  {
    return bRc;
  }
  if ( mHandle )
  {
    mSpatialiteDbInfo = mHandle->getSpatialiteDbInfo();
    if ( ( mSpatialiteDbInfo ) && ( isDbValid() )  && ( isDbRasterLite2() ) )
    {
      // -- ---------------------------------- --
      // The combination isDbValid() and isDbRasterLite2()
      //  - means that the given Layer is supported by the QgsRasterLite2Provider
      //  --> i.e. not GeoPackage, MBTiles etc.
      //  - RasterLite1 will return isDbGdalOgr() == 1
      //  -> which renders the RasterLayers with gdal
      //  --> so a check must be done later
      //  ---> that the layer is not a RasterLite1-Layer
      // -- ---------------------------------- --
      bRc = setDbLayer( mSpatialiteDbInfo->getSpatialiteDbLayer( mUriTableName, bLoadLayer ) );
    }
  }
  return bRc;
}
bool QgsRasterLite2Provider::setDbLayer( SpatialiteDbLayer *dbLayer )
{
  bool bRc = false;
  if ( ( dbLayer ) && ( dbLayer->isLayerValid() ) && ( dbLayer->isLayerRasterLite2() ) )
  {
    mDbLayer = dbLayer;
    mCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mDbLayer->getSridEpsg() );
    setLayerBandsInfo( mDbLayer->getLayerGetBandStatistics(), mDbLayer->getLayerBandsHistograms() );
    mDefaultRasterStyle = mDbLayer->getLayerStyleSelected();
    mStyleLayersInfo = mDbLayer->getLayerCoverageStylesInfo();
    mDefaultImageBackground = mDbLayer->getLayerDefaultImageBackground();
    QgsSettings *userSettings = new QgsSettings();
    // Set the default Image-Backround to the color used for the canvas background
    mDefaultImageBackground = QString( "#%1%2%3" )
                              .arg( QString::number( userSettings->value( QStringLiteral( "/qgis/default_canvas_color_red" ), 255 ).toInt(), 16 ) )
                              .arg( QString::number( userSettings->value( QStringLiteral( "/qgis/default_canvas_color_green" ), 255 ).toInt(), 16 ) )
                              .arg( QString::number( userSettings->value( QStringLiteral( "/qgis/default_canvas_color_blue" ), 255 ).toInt(), 16 ) );
    createLayerMetadata( 1 );
    bRc = true;
  }
  return bRc;
}
void QgsRasterLite2Provider::setLayerBandsInfo( QStringList layerBandsInfo, QMap<int, QImage> layerBandsHistograms )
{
  mLayerBandsInfo = layerBandsInfo;
  mLayerBandsHistograms = layerBandsHistograms;
  for ( int i = 0; i < mLayerBandsInfo.size(); i++ )
  {
    QStringList sa_list_info = mLayerBandsInfo.at( i ).split( SpatialiteDbInfo::ParseSeparatorGeneral );
    if ( sa_list_info.size() == 8 )
    {
      mLayerBandsNodata.insert( i, sa_list_info.at( 0 ).toDouble() );
      mSrcNoDataValue.append( sa_list_info.at( 0 ).toDouble() );
      // We want to inforn the world of the values of these Nodata pixels
      mSrcHasNoDataValue.append( true );
      // Nodata is returned from RasterLite2 when supported by the given format
      mUseSrcNoDataValue.append( false );
      mLayerBandsPixelMin.insert( i, sa_list_info.at( 1 ).toDouble() );
      mLayerBandsPixelMax.insert( i, sa_list_info.at( 2 ).toDouble() );
      mLayerBandsPixelAverage.insert( i, sa_list_info.at( 3 ).toDouble() );
      mLayerBandsPixelVariance.insert( i, sa_list_info.at( 4 ).toDouble() );
      mLayerBandsPixelStandardDeviation.insert( i, sa_list_info.at( 5 ).toDouble() );
      mLayerBandsPixelValidPixelsCount.insert( i, sa_list_info.at( 6 ).toInt() );
    }
  }
}
int QgsRasterLite2Provider::createLayerMetadata( int i_debug )
{
  int i_status = 0;
  QString sMetadata = "";
  QString sMetadataBands = "";
  //-----------------------------------------------------------
  sMetadataBands += QString( "Layer-Name: %1\n" ).arg( getLayerName() );
  sMetadataBands += QString( "Title      : %1\n" ).arg( getTitle() );
  sMetadataBands += QString( "Abstract  : %1\n" ).arg( getAbstract() );
  sMetadataBands += QString( "Copyright : %1\n" ).arg( getCopyright() );
  sMetadataBands += QString( "Srid\t  : %1\n" ).arg( getSridEpsg() );
  sMetadataBands += QString( "Extent\t  : %1\n" ).arg( getDbLayer()->getLayerExtentEWKT() );
  sMetadataBands += QString( "\tExtent - Width : %1\n" ).arg( QString::number( getDbLayer()->getLayerExtentWidth(), 'f', 7 ) );
  sMetadataBands += QString( "\tExtent - Height: %1\n" ).arg( QString::number( getDbLayer()->getLayerExtentHeight(), 'f', 7 ) );
  sMetadataBands += QString( "Image-Size: %1x%2\n" ).arg( getDbLayer()->getLayerImageWidth() ).arg( getDbLayer()->getLayerImageHeight() );
  sMetadataBands += QString( "\tPixels - Total  : %1\n" ).arg( getDbLayer()->getLayerCountImagePixels() );
  sMetadataBands += QString( "\tPixels - Valid  : %1\n" ).arg( getDbLayer()->getLayerCountImageValidPixels() );
  sMetadataBands += QString( "\tPixels - NoData : %1\n" ).arg( getDbLayer()->getLayerCountImageNodataPixels() );
  sMetadataBands += QString( "Bands\t  : %1\n" ).arg( getDbLayer()->getLayerNumBands() );
  // 3319x2701 = 8964619-7917158=1047461
  //-----------------------------------------------------------
  for ( int i = 0; i < getDbLayer()->getLayerNumBands(); i++ )
  {
    // [generateBandName is 1 based, thus adding '+1'] RL2_GetPixelValue for nodata
    QString sBandInfo = QString( " %1\t: nodata[%2] " ).arg( generateBandName( i + 1 ) ).arg( ( int )mLayerBandsNodata.value( i ) );
    // RL2_GetBandStatistics_Min, RL2_GetBandStatistics_Max
    sBandInfo += QString( "Min[%1] Max[%2] " ).arg( ( int )mLayerBandsPixelMin.value( i ) ).arg( ( int )mLayerBandsPixelMax.value( i ) );
    // RL2_GetRasterStatistics_ValidPixelsCount, range (max-min)
    sBandInfo += QString( "Range[%1] " ).arg( ( int )( mLayerBandsPixelMax.value( i ) - mLayerBandsPixelMin.value( i ) ) );
    // RL2_GetBandStatistics_Avg, RL2_GetBandStatistics_StdDev
    sBandInfo += QString( "Average/Mean[%1] StandardDeviation[%2] " ).arg( ( int )mLayerBandsPixelAverage.value( i ) ).arg( ( int )mLayerBandsPixelStandardDeviation.value( i ) );
    // RL2_GetBandStatistics_Var [estimated Variance value as double]
    sBandInfo += QString( "Variance[%1] " ).arg( ( int )mLayerBandsPixelVariance.value( i ) );
    sMetadataBands += QString( "\t%1\n" ).arg( sBandInfo );
  }
  sMetadata += QString( "%1\n" ).arg( sMetadataBands );
  sMetadata += QString( "Data-Type   : %1 [%2]\n" ).arg( getDbLayer()->getLayerRasterSampleType() ).arg( getDbLayer()->getLayerRasterDataTypeString() );
  sMetadata += QString( "Pixel-Type  : %1\n" ).arg( getDbLayer()->getLayerRasterPixelType() );
  sMetadata += QString( "Compression : %1\n" ).arg( getDbLayer()->getLayerRasterCompressionType() );
  sMetadata += QString( "Tile-Size   : %1,%2\n" ).arg( getDbLayer()->getLayerTileWidth() ).arg( getDbLayer()->getLayerTileHeight() );
  sMetadata += QString( "Resolution  : horz=%1 vert=%2\n" ).arg( QString::number( getDbLayer()->getLayerImageResolutionX(), 'f', 7 ) ).arg( QString::number( getDbLayer()->getLayerImageResolutionY(), 'f', 7 ) );
  sMetadata += QString( "User-Default canvas background : '%1'\n" ).arg( mDefaultImageBackground );
  sMetadata += QString( "NoData ImageBackground : '%1'\n" ).arg( mDbLayer->getLayerDefaultImageBackground() );
  sMetadata += QString( "DefaultRasterStyle : '%1' of %2 Styles\n" ).arg( mDbLayer->getLayerStyleSelected() ).arg( mStyleLayersInfo.count() );
  if ( i_debug > 0 )
  {
    qDebug().noquote() << sMetadata;
  }
  //-----------------------------------------------------------
  mMetadata = sMetadata;
  //-----------------------------------------------------------
  return i_status;
}

QStringList QgsRasterLite2Provider::subLayerStyles() const
{
  QStringList styles;
  for ( int i = 0, n = mSubLayers.size(); i < n; ++i )
  {
    styles.append( QString() );
  }
  return styles;
}

void QgsRasterLite2Provider::setLayerOrder( const QStringList &layers )
{
  QStringList oldSubLayers = mSubLayers;
  QList<bool> oldSubLayerVisibilities = mSubLayerVisibilities;
  mSubLayers.clear();
  mSubLayerVisibilities.clear();
  foreach ( const QString &layer, layers )
  {
    // Search for match
    for ( int i = 0, n = oldSubLayers.size(); i < n; ++i )
    {
      if ( oldSubLayers[i] == layer )
      {
        mSubLayers.append( layer );
        oldSubLayers.removeAt( i );
        mSubLayerVisibilities.append( oldSubLayerVisibilities[i] );
        oldSubLayerVisibilities.removeAt( i );
        break;
      }
    }
  }
  // Add remaining at bottom
  mSubLayers.append( oldSubLayers );
  mSubLayerVisibilities.append( oldSubLayerVisibilities );
}

void QgsRasterLite2Provider::setSubLayerVisibility( const QString &name, bool vis )
{
  for ( int i = 0, n = mSubLayers.size(); i < n; ++i )
  {
    if ( mSubLayers[i] == name )
    {
      mSubLayerVisibilities[i] = vis;
      break;
    }
  }
}

void QgsRasterLite2Provider::reloadData()
{
  mCachedImage = QImage();
}

QgsRasterInterface *QgsRasterLite2Provider::clone() const
{
  QgsRasterLite2Provider *provider = new QgsRasterLite2Provider( dataSourceUri() );
  provider->copyBaseSettings( *this );
  return provider;
}

QString QgsRasterLite2Provider::metadata()
{
  return mMetadata;
}

QgsRasterIdentifyResult QgsRasterLite2Provider::identify( const QgsPointXY &point, QgsRaster::IdentifyFormat format, const QgsRectangle &extent, int width, int height, int dpi )
{
  Q_UNUSED( point );
  Q_UNUSED( format );
  Q_UNUSED( extent );
  Q_UNUSED( width );
  Q_UNUSED( height );
  Q_UNUSED( dpi ); // for now
  QMap<int, QVariant> entries;
#if 0
  // http://resources.arcgis.com/en/help/rest/apiref/identify.html
  QgsDataSourceUri dataSource( dataSourceUri() );
  QUrl queryUrl( dataSource.param( QStringLiteral( "url" ) ) + "/identify" );
  queryUrl.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "json" ) );
  queryUrl.addQueryItem( QStringLiteral( "geometryType" ), QStringLiteral( "esriGeometryPoint" ) );
  queryUrl.addQueryItem( QStringLiteral( "geometry" ), QStringLiteral( "{x: %1, y: %2}" ).arg( point.x(), 0, 'f' ).arg( point.y(), 0, 'f' ) );
//  queryUrl.addQueryItem( "sr", mCrs.postgisSrid() );
  queryUrl.addQueryItem( QStringLiteral( "layers" ), QStringLiteral( "all:%1" ).arg( dataSource.param( QStringLiteral( "layer" ) ) ) );
  queryUrl.addQueryItem( QStringLiteral( "imageDisplay" ), QStringLiteral( "%1,%2,%3" ).arg( width ).arg( height ).arg( dpi ) );
  queryUrl.addQueryItem( QStringLiteral( "mapExtent" ), QStringLiteral( "%1,%2,%3,%4" ).arg( extent.xMinimum(), 0, 'f' ).arg( extent.yMinimum(), 0, 'f' ).arg( extent.xMaximum(), 0, 'f' ).arg( extent.yMaximum(), 0, 'f' ) );
  queryUrl.addQueryItem( QStringLiteral( "tolerance" ), QStringLiteral( "10" ) );
  QVariantList queryResults = QgsArcGisRestUtils::queryServiceJSON( queryUrl, mErrorTitle, mError ).value( QStringLiteral( "results" ) ).toList();

  if ( format == QgsRaster::IdentifyFormatText )
  {
    foreach ( const QVariant &result, queryResults )
    {
      QVariantMap resultMap = result.toMap();
      QVariantMap attributesMap = resultMap[QStringLiteral( "attributes" )].toMap();
      QString valueStr;
      foreach ( const QString &attribute, attributesMap.keys() )
      {
        valueStr += QStringLiteral( "%1 = %2\n" ).arg( attribute, attributesMap[attribute].toString() );
      }
      entries.insert( entries.size(), valueStr );
    }
  }
  else if ( format == QgsRaster::IdentifyFormatFeature )
  {
    foreach ( const QVariant &result, queryResults )
    {
      QVariantMap resultMap = result.toMap();

      QgsFields fields;
      QVariantMap attributesMap = resultMap[QStringLiteral( "attributes" )].toMap();
      QgsAttributes featureAttributes;
      foreach ( const QString &attribute, attributesMap.keys() )
      {
        fields.append( QgsField( attribute, QVariant::String, QStringLiteral( "string" ) ) );
        featureAttributes.append( attributesMap[attribute].toString() );
      }
      QgsCoordinateReferenceSystem crs;
      QgsAbstractGeometry *geometry = QgsArcGisRestUtils::parseEsriGeoJSON( resultMap[QStringLiteral( "geometry" )].toMap(), resultMap[QStringLiteral( "geometryType" )].toString(), false, false, &crs );
      QgsFeature feature( fields );
      feature.setGeometry( QgsGeometry( geometry ) );
      feature.setAttributes( featureAttributes );
      feature.setValid( true );
      QgsFeatureStore store( fields, crs );
      QMap<QString, QVariant> params;
      params[QStringLiteral( "sublayer" )] = resultMap[QStringLiteral( "layerName" )].toString();
      params[QStringLiteral( "featureType" )] = attributesMap[resultMap[QStringLiteral( "displayFieldName" )].toString()].toString();
      store.setParams( params );
      store.addFeature( feature );
      entries.insert( entries.size(), qVariantFromValue( QList<QgsFeatureStore>() << store ) );
    }
  }
#endif
  return QgsRasterIdentifyResult( format, entries );
}

void QgsRasterLite2Provider::readBlock( int /*bandNo*/, const QgsRectangle &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( feedback );  // TODO: make use of the feedback object
  // viewExtent uses the srid of the given QgsCoordinateReferenceSystem.
  // Note Style: registered style used in the coverage if this remain empty.
  // Otherwise: set to 'default' or to another registered Style, otherwise 'default' will be used
  QString mimeType = QString( "image/png" ); // should never be changed.
  if ( !getDbLayer()->getMapImageFromRasterLite2( width, height, viewExtent, mDefaultRasterStyle, mimeType, mDefaultImageBackground, data, mError ) )
  {
    QgsDebugMsg( QString( "Retrieving image from RasterLite2 failed: [%1]" ).arg( mError ) );
    return;
  }
  return;
}

QString QgsRasterLite2Provider::generateBandName( int bandNumber ) const
{
  QString sLayerPixelType = getDbLayer()->getLayerRasterPixelType();
  if ( sLayerPixelType == QLatin1String( "RGB" ) )
  {
    switch ( bandNumber )
    {
      case 1:
        sLayerPixelType = QString( "%2 %1" ).arg( bandNumber ).arg( "Red  " );
        break;
      case 2:
        sLayerPixelType = QString( "%2 %1" ).arg( bandNumber ).arg( "Green" );
        break;
      case 3:
        sLayerPixelType = QString( "%2 %1" ).arg( bandNumber ).arg( "Blue " );
        break;
      case 4:
        sLayerPixelType = QString( "%2 %1" ).arg( bandNumber ).arg( "Alpha" );
        break;
      default:
        sLayerPixelType = QString( "Band %1" ).arg( bandNumber );
        break;
    }
  }
  else
  {
    sLayerPixelType = QString( "%2-Band %1" ).arg( bandNumber ).arg( sLayerPixelType );
  }
  return sLayerPixelType;
}
#if 0
bool  QgsRasterLite2Provider::hasStatistics( int bandNo,
    int stats,
    const QgsRectangle &boundingBox,
    int sampleSize )
{
  // First check if cached in mStatistics
  if ( QgsRasterDataProvider::hasStatistics( bandNo, stats, boundingBox, sampleSize ) )
  {
    return true;
  }
  // The gdal band number (starts at 1) - ours are 0 based, thus '(bandNo-1)'
  QgsRasterBandStats rasterBandStats;
  initStatistics( rasterBandStats, bandNo, stats, boundingBox, sampleSize );
  rasterBandStats.elementCount = mLayerBandsPixelValidPixelsCount.value( ( bandNo - 1 ) ); // RL2_GetRasterStatistics_ValidPixelsCount
  rasterBandStats.minimumValue = mLayerBandsPixelMin.value( ( bandNo - 1 ) ); // RL2_GetBandStatistics_Min
  rasterBandStats.maximumValue = mLayerBandsPixelMax.value( ( bandNo - 1 ) ); // RL2_GetBandStatistics_Max
  rasterBandStats.range = rasterBandStats.maximumValue - rasterBandStats.minimumValue;
  rasterBandStats.mean = mLayerBandsPixelAverage.value( ( bandNo - 1 ) ); // RL2_GetBandStatistics_Avg
  rasterBandStats.stdDev = mLayerBandsPixelStandardDeviation.value( ( bandNo - 1 ) ); // RL2_GetBandStatistics_StdDev
  // The sum of all cells in the band. NO_DATA values are excluded
  rasterBandStats.sum = 0; // info[QStringLiteral( "SUM" )].toDouble();
  // The sum of the squares. Used to calculate standard deviation.
  rasterBandStats.sumOfSquares = 0; // info[QStringLiteral( "SQSUM" )].toDouble();
  // RL2_GetBandStatistics_Var [estimated Variance value as double]
  int supportedStats = QgsRasterBandStats::Min | QgsRasterBandStats::Max
                       | QgsRasterBandStats::Range | QgsRasterBandStats::Mean
                       | QgsRasterBandStats::StdDev;
  rasterBandStats.statsGathered = supportedStats;
#ifdef QGISDEBUG
  QgsDebugMsg( QString( "************ STATS Band %1**************" ).arg( rasterBandStats.bandNumber ) );
  QgsDebugMsg( QString( "COUNT %1" ).arg( rasterBandStats.elementCount ) );
  QgsDebugMsg( QString( "MIN %1" ).arg( rasterBandStats.minimumValue ) );
  QgsDebugMsg( QString( "MAX %1" ).arg( rasterBandStats.maximumValue ) );
  QgsDebugMsg( QString( "RANGE %1" ).arg( rasterBandStats.range ) );
  QgsDebugMsg( QString( "MEAN %1" ).arg( rasterBandStats.mean ) );
  QgsDebugMsg( QString( "STDDEV %1" ).arg( rasterBandStats.stdDev ) );
#endif
  if ( rasterBandStats.extent != extent() ||
       ( stats & ( ~supportedStats ) ) )
  {
    QgsDebugMsg( "Not supported by RasterLite2." );
    return false;
  }
  return true;
}
QgsRasterBandStats QgsRasterLite2Provider::bandStatistics( int bandNo, int stats, const QgsRectangle &boundingBox, int sampleSize, QgsRasterBlockFeedback * )
{
  QgsRasterBandStats rasterBandStats;
  initStatistics( rasterBandStats, bandNo, stats, boundingBox, sampleSize );
  // First check if cached in mStatistics
  Q_FOREACH ( const QgsRasterBandStats &stats, mStatistics )
  {
    if ( stats.contains( rasterBandStats ) )
    {
      QgsDebugMsg( "Using cached statistics." );
      return stats;
    }
  }
  // The gdal band number (starts at 1) - ours are 0 based, thus '(bandNo-1)'
  initStatistics( rasterBandStats, bandNo, stats, boundingBox, sampleSize );
  rasterBandStats.elementCount = mLayerBandsPixelValidPixelsCount.value( ( bandNo - 1 ) ); // RL2_GetRasterStatistics_ValidPixelsCount
  rasterBandStats.minimumValue = mLayerBandsPixelMin.value( ( bandNo - 1 ) ); // RL2_GetBandStatistics_Min
  rasterBandStats.maximumValue = mLayerBandsPixelMax.value( ( bandNo - 1 ) ); // RL2_GetBandStatistics_Max
  rasterBandStats.range = rasterBandStats.maximumValue - rasterBandStats.minimumValue;
  rasterBandStats.mean = mLayerBandsPixelAverage.value( ( bandNo - 1 ) ); // RL2_GetBandStatistics_Avg
  rasterBandStats.stdDev = mLayerBandsPixelStandardDeviation.value( ( bandNo - 1 ) ); // RL2_GetBandStatistics_StdDev
  // The sum of all cells in the band. NO_DATA values are excluded
  rasterBandStats.sum = 0; // info[QStringLiteral( "SUM" )].toDouble();
  // The sum of the squares. Used to calculate standard deviation.
  rasterBandStats.sumOfSquares = 0; // info[QStringLiteral( "SQSUM" )].toDouble();
  // RL2_GetBandStatistics_Var [estimated Variance value as double]
  int supportedStats = QgsRasterBandStats::Min | QgsRasterBandStats::Max
                       | QgsRasterBandStats::Range | QgsRasterBandStats::Mean
                       | QgsRasterBandStats::StdDev;
  rasterBandStats.statsGathered = supportedStats;
#ifdef QGISDEBUG
  QgsDebugMsg( QString( "************ STATS Band %1 **************" ).arg( rasterBandStats.bandNumber ) );
  QgsDebugMsg( QString( "COUNT %1" ).arg( rasterBandStats.elementCount ) );
  QgsDebugMsg( QString( "MIN %1" ).arg( rasterBandStats.minimumValue ) );
  QgsDebugMsg( QString( "MAX %1" ).arg( rasterBandStats.maximumValue ) );
  QgsDebugMsg( QString( "RANGE %1" ).arg( rasterBandStats.range ) );
  QgsDebugMsg( QString( "MEAN %1" ).arg( rasterBandStats.mean ) );
  QgsDebugMsg( QString( "STDDEV %1" ).arg( rasterBandStats.stdDev ) );
#endif

  mStatistics.append( rasterBandStats );
  return rasterBandStats;
}
#endif