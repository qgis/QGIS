/***************************************************************************
    qgsimageserverprovider.cpp
     ----------------------------------------------------
    Date                 : April 2026
    Copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsimageserverprovider.h"

#include <cpl_vsi.h>
#include <gdal.h>

#include "qgsapplication.h"
#include "qgsarcgisrestquery.h"
#include "qgsarcgisrestutils.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsdatasourceuri.h"
#include "qgsgdalutils.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsrasteridentifyresult.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsstringutils.h"
#include "qgstilecache.h"

#include <QDir>
#include <QFontMetrics>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkCacheMetaData>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QString>
#include <QTimer>
#include <QUrlQuery>

#include "moc_qgsimageserverprovider.cpp"

using namespace Qt::StringLiterals;

const QString QgsImageServerProvider::IMS_PROVIDER_KEY = u"arcgisimageserver"_s;
const QString QgsImageServerProvider::IMS_PROVIDER_DESCRIPTION = u"ArcGIS Image Server data provider"_s;


QgsImageServerProvider::QgsImageServerProvider( const QString &uri, const ProviderOptions &options, Qgis::DataProviderReadFlags flags )
  : QgsRasterDataProvider( uri, options, flags )
{
  QgsDataSourceUri dataSource( dataSourceUri() );
  mRequestHeaders = dataSource.httpHeaders();
  mUrlPrefix = dataSource.param( u"urlprefix"_s );
  mAuthCfg = dataSource.authConfigId();

  const QString serviceUrl = dataSource.param( u"url"_s );
  if ( !serviceUrl.isEmpty() )
    mServiceInfo = QgsArcGisRestQueryUtils::getServiceInfo( serviceUrl, mAuthCfg, mErrorTitle, mError, mRequestHeaders, mUrlPrefix );

  mCapabilities = QgsArcGisRestUtils::serviceCapabilitiesFromString( mServiceInfo.value( u"capabilities"_s ).toString() );
  if ( !mCapabilities.testFlag( Qgis::ArcGisRestServiceCapability::Image ) )
  {
    // not an image service
    appendError( QgsErrorMessage( tr( "Service does not have Image capability -- it is likely not an ESRI ImageService" ) ) );
    QgsMessageLog::logMessage( tr( "Service does not have Image capability -- it is likely not an ESRI ImageService" ), tr( "ImageServer" ) );
    mValid = false;
    return;
  }

  if ( mCapabilities.testFlag( Qgis::ArcGisRestServiceCapability::TilesOnly ) )
  {
    // not an image service
    appendError( QgsErrorMessage( tr( "Service has support for Image Tiles only -- this is currently not supported" ) ) );
    QgsMessageLog::logMessage( tr( "Service has support for Image Tiles only -- this is currently not supported" ), tr( "ImageServer" ) );
    mValid = false;
    return;
  }

  mHasRat = mServiceInfo.value( u"hasRasterAttributeTable"_s ).toBool();

  bool ok = false;

  // determine if GDAL was built was lerc support, and the service supports it
  if ( QgsGdalUtils::supportsMrfLercCompression() )
  {
    const double serverVersion = mServiceInfo.value( u"currentVersion"_s ).toDouble( &ok );
    if ( ok && serverVersion >= 10.5 )
    {
      // version 2 was added in 10.5
      mMaximumLercVersionSupported = 2;
    }
    else if ( ok && serverVersion >= 10.3 )
    {
      // version 1 was added in 10.3
      mMaximumLercVersionSupported = 1;
    }
    else
    {
      // no support for lerc in earlier versions
      mMaximumLercVersionSupported = 0;
    }
  }
  else
  {
    QgsDebugMsgLevel( u"GDAL version does not support LERC decoding, disabling"_s, 2 );
  }

  QString layerUrl;
  if ( dataSource.param( u"layer"_s ).isEmpty() )
  {
    layerUrl = serviceUrl;
    mLayerInfo = mServiceInfo;
    if ( !mServiceInfo.value( u"serviceDataType"_s ).toString().startsWith( "esriImageService"_L1 ) )
    {
      // not an image service
      appendError( QgsErrorMessage( tr( "Service is not an ESRI ImageService" ) ) );
      mValid = false;
      return;
    }
  }
  else
  {
    layerUrl = dataSource.param( u"url"_s ) + "/" + dataSource.param( u"layer"_s );
    mLayerInfo = QgsArcGisRestQueryUtils::getLayerInfo( layerUrl, mAuthCfg, mErrorTitle, mError, mRequestHeaders, mUrlPrefix );
  }

  QVariantMap extentData;
  if ( mLayerInfo.contains( u"fullExtent"_s ) )
  {
    extentData = mLayerInfo.value( u"fullExtent"_s ).toMap();
  }
  else
  {
    extentData = mLayerInfo.value( u"extent"_s ).toMap();
  }
  mExtent.setXMinimum( extentData[u"xmin"_s].toDouble() );
  mExtent.setYMinimum( extentData[u"ymin"_s].toDouble() );
  mExtent.setXMaximum( extentData[u"xmax"_s].toDouble() );
  mExtent.setYMaximum( extentData[u"ymax"_s].toDouble() );
  mCrs = QgsArcGisRestUtils::convertSpatialReference( extentData[u"spatialReference"_s].toMap() );
  if ( !mCrs.isValid() )
  {
    appendError( QgsErrorMessage( tr( "Could not parse spatial reference" ) ) );
    return;
  }
  mPixelSizeX = mLayerInfo.value( u"pixelSizeX"_s ).toDouble( &ok );
  if ( !ok )
    mPixelSizeX = 1;
  mPixelSizeY = mLayerInfo.value( u"pixelSizeY"_s ).toDouble( &ok );
  if ( !ok )
    mPixelSizeY = 1;

  mPixelType = mLayerInfo.value( u"pixelType"_s ).toString();
  mDataType = QgsArcGisRestUtils::dataTypeFromString( mPixelType );
  mBandCount = mLayerInfo.value( u"bandCount"_s ).toInt( &ok );
  if ( !ok )
    mBandCount = 1;
  mBandNames = mLayerInfo.value( u"bandNames"_s ).toStringList();

  // nodata handling
  if ( mLayerInfo.contains( u"noDataValues"_s ) )
  {
    const QVariantList noDataValues = mLayerInfo.value( u"noDataValues"_s ).toList();
    for ( const QVariant &noDataValue : noDataValues )
    {
      mSrcNoDataValue.append( noDataValue.toDouble() );
      mSrcHasNoDataValue.append( true );
      mUseSrcNoDataValue.append( true );
    }
  }

  // for integer data types, we enforce that there is a nodata value for every band.
  // we require this as we can't fallback to a nan value for padding raster blocks
  // with nodata when required
  switch ( mDataType )
  {
    case Qgis::DataType::UnknownDataType:
      break;

    case Qgis::DataType::Byte:
    case Qgis::DataType::UInt16:
    case Qgis::DataType::Int16:
    case Qgis::DataType::UInt32:
    case Qgis::DataType::Int32:
    case Qgis::DataType::Int8:
      while ( mSrcHasNoDataValue.size() < mBandCount )
      {
        mSrcNoDataValue.append( QgsArcGisRestUtils::defaultNoDataForDataType( mDataType, ok ) );
        mSrcHasNoDataValue.append( true );
        mUseSrcNoDataValue.append( true );
      }
      break;

    case Qgis::DataType::Float32:
    case Qgis::DataType::Float64:
    case Qgis::DataType::CInt16:
    case Qgis::DataType::CInt32:
    case Qgis::DataType::CFloat32:
    case Qgis::DataType::CFloat64:
    case Qgis::DataType::ARGB32:
    case Qgis::DataType::ARGB32_Premultiplied:
      break;
  }

  for ( const QVariant &min : mLayerInfo.value( u"minValues"_s ).toList() )
  {
    mMinValues.append( min.toDouble() );
  }
  for ( const QVariant &max : mLayerInfo.value( u"maxValues"_s ).toList() )
  {
    mMaxValues.append( max.toDouble() );
  }
  for ( const QVariant &mean : mLayerInfo.value( u"meanValues"_s ).toList() )
  {
    mMeanValues.append( mean.toDouble() );
  }
  for ( const QVariant &stdDev : mLayerInfo.value( u"stdvValues"_s ).toList() )
  {
    mStdevValues.append( stdDev.toDouble() );
  }

  if ( mLayerInfo.value( u"serviceDataType"_s ).toString().compare( "esriImageServiceDataTypeElevation"_L1, Qt::CaseInsensitive ) == 0 )
  {
    elevationProperties()->setContainsElevationData( true );
  }

  QgsLayerMetadata::SpatialExtent spatialExtent;
  spatialExtent.bounds = QgsBox3D( mExtent );
  spatialExtent.extentCrs = mCrs;
  QgsLayerMetadata::Extent metadataExtent;
  metadataExtent.setSpatialExtents( QList<QgsLayerMetadata::SpatialExtent>() << spatialExtent );
  mLayerMetadata.setExtent( metadataExtent );
  mLayerMetadata.setCrs( mCrs );

  mTiled = mServiceInfo.value( u"singleFusedMapCache"_s ).toBool();
  if ( dataSource.param( u"tiled"_s ).toLower() == "false" || dataSource.param( u"tiled"_s ) == "0" )
  {
    mTiled = false;
  }

  if ( mServiceInfo.contains( u"maxImageWidth"_s ) )
    mMaxImageWidth = mServiceInfo.value( u"maxImageWidth"_s ).toInt();
  if ( mServiceInfo.contains( u"maxImageHeight"_s ) )
    mMaxImageHeight = mServiceInfo.value( u"maxImageHeight"_s ).toInt();

  mTimestamp = QDateTime::currentDateTime();
  mValid = true;

  // layer metadata

  mLayerMetadata.setIdentifier( layerUrl );
  mLayerMetadata.setParentIdentifier( serviceUrl );
  mLayerMetadata.setType( u"dataset"_s );
  mLayerMetadata.setTitle( mLayerInfo.value( u"name"_s ).toString() );
  mLayerMetadata.setAbstract( mLayerInfo.value( u"description"_s ).toString() );
  const QString copyright = mLayerInfo.value( u"copyrightText"_s ).toString();
  if ( !copyright.isEmpty() )
    mLayerMetadata.setRights( QStringList() << copyright );
  mLayerMetadata.addLink( QgsAbstractMetadataBase::Link( tr( "Source" ), u"WWW:LINK"_s, layerUrl ) );

  if ( mTiled )
  {
    const QVariantMap tileInfo = mServiceInfo.value( u"tileInfo"_s ).toMap();
    const QList<QVariant> lodEntries = tileInfo[u"lods"_s].toList();
    for ( const QVariant &lodEntry : lodEntries )
    {
      const QVariantMap lodEntryMap = lodEntry.toMap();
      mResolutions << lodEntryMap[u"resolution"_s].toDouble();
    }
    std::sort( mResolutions.begin(), mResolutions.end() );
  }
}

QgsImageServerProvider::QgsImageServerProvider( const QgsImageServerProvider &other, const QgsDataProvider::ProviderOptions &providerOptions )
  : QgsRasterDataProvider( other.dataSourceUri(), providerOptions )
  , mValid( other.mValid )
  , mServiceInfo( other.mServiceInfo )
  , mLayerInfo( other.mLayerInfo )
  , mCapabilities( other.mCapabilities )
  , mCrs( other.mCrs )
  , mExtent( other.mExtent )
  , mPixelSizeX( other.mPixelSizeX )
  , mPixelSizeY( other.mPixelSizeY )
  , mPixelType( other.mPixelType )
  , mDataType( other.mDataType )
  , mBandCount( other.mBandCount )
  , mBandNames( other.mBandNames )
  , mMinValues( other.mMinValues )
  , mMaxValues( other.mMaxValues )
  , mMeanValues( other.mMeanValues )
  , mStdevValues( other.mStdevValues )
  , mRequestHeaders( other.mRequestHeaders )
  , mTiled( other.mTiled )
  , mMaxImageWidth( other.mMaxImageWidth )
  , mMaxImageHeight( other.mMaxImageHeight )
  , mLayerMetadata( other.mLayerMetadata )
  , mResolutions( other.mResolutions )
  , mUrlPrefix( other.mUrlPrefix )
  , mAuthCfg( other.mAuthCfg )
  , mMaximumLercVersionSupported( other.mMaximumLercVersionSupported )
  , mHasRat( other.mHasRat )
// intentionally omitted:
// - mErrorTitle
// - mError
// - mCachedImage
// - mCachedImageExtent
{
  // is this needed?
  mTimestamp = QDateTime::currentDateTime();
}

Qgis::DataProviderFlags QgsImageServerProvider::flags() const
{
  return Qgis::DataProviderFlag::FastExtent2D;
}

Qgis::RasterProviderCapabilities QgsImageServerProvider::providerCapabilities() const
{
  return Qgis::RasterProviderCapability::ReadLayerMetadata | Qgis::RasterProviderCapability::ReloadData;
}

QString QgsImageServerProvider::name() const
{
  return IMS_PROVIDER_KEY;
}

QString QgsImageServerProvider::providerKey()
{
  return IMS_PROVIDER_KEY;
}

int QgsImageServerProvider::bandCount() const
{
  return mBandCount;
}

QString QgsImageServerProvider::generateBandName( int bandNumber ) const
{
  return mBandNames.value( bandNumber - 1, QgsRasterDataProvider::generateBandName( bandNumber ) );
}

Qgis::RasterColorInterpretation QgsImageServerProvider::colorInterpretation( int bandNo ) const
{
  if ( bandNo < 1 || bandNo > mBandNames.size() )
    return Qgis::RasterColorInterpretation::Undefined;

  const QString bandName = mBandNames.at( bandNo - 1 );
  return QgsArcGisRestUtils::colorInterpretationFromBandName( bandName );
}

Qgis::RasterInterfaceCapabilities QgsImageServerProvider::capabilities() const
{
  return Qgis::RasterInterfaceCapability::Size | Qgis::RasterInterfaceCapability::Identify | Qgis::RasterInterfaceCapability::IdentifyValue;
}

int QgsImageServerProvider::xSize() const
{
  return mPixelSizeX > 0 ? static_cast< int >( std::round( mExtent.width() / mPixelSizeX ) ) : 0;
}

int QgsImageServerProvider::ySize() const
{
  return mPixelSizeY > 0 ? static_cast< int >( std::round( mExtent.height() / mPixelSizeY ) ) : 0;
}

bool QgsImageServerProvider::hasStatistics( int bandNo, Qgis::RasterBandStatistics stats, const QgsRectangle &boundingBox, int sampleSize )
{
  // First check if cached in mStatistics
  if ( QgsRasterDataProvider::hasStatistics( bandNo, stats, boundingBox, sampleSize ) )
  {
    return true;
  }

  QgsRasterBandStats rasterBandStats;
  initStatistics( rasterBandStats, bandNo, stats, boundingBox, sampleSize );

  if ( ( sourceHasNoDataValue( bandNo ) && !useSourceNoDataValue( bandNo ) ) || !userNoDataValues( bandNo ).isEmpty() )
  {
    QgsDebugMsgLevel( u"Custom NoData values -> provider statistics not sufficient."_s, 2 );
    return false;
  }

  // If not cached, check if supported by provider
  Qgis::RasterBandStatistics supportedStats = Qgis::RasterBandStatistic::Min
                                              | Qgis::RasterBandStatistic::Max
                                              | Qgis::RasterBandStatistic::Range
                                              | Qgis::RasterBandStatistic::Mean
                                              | Qgis::RasterBandStatistic::StdDev;

  if ( rasterBandStats.extent != extent() || ( stats & ( ~supportedStats ) ) )
  {
    QgsDebugMsgLevel( u"Custom extent or not supported stats -> provider statistics not sufficient."_s, 2 );
    return false;
  }

  QgsDebugMsgLevel( u"Looking for provider statistics"_s, 2 );
  if ( stats.testFlag( Qgis::RasterBandStatistic::Max ) && mMaxValues.size() < bandNo )
  {
    QgsDebugMsgLevel( u"No max value for band available"_s, 2 );
    return false;
  }
  if ( stats.testFlag( Qgis::RasterBandStatistic::Min ) && mMinValues.size() < bandNo )
  {
    QgsDebugMsgLevel( u"No min value for band available"_s, 2 );
    return false;
  }
  if ( stats.testFlag( Qgis::RasterBandStatistic::Range ) && ( mMinValues.size() < bandNo || mMaxValues.size() < bandNo ) )
  {
    QgsDebugMsgLevel( u"No range value for band available"_s, 2 );
    return false;
  }
  if ( stats.testFlag( Qgis::RasterBandStatistic::Mean ) && mMeanValues.size() < bandNo )
  {
    QgsDebugMsgLevel( u"No mean value for band available"_s, 2 );
    return false;
  }
  if ( stats.testFlag( Qgis::RasterBandStatistic::StdDev ) && mStdevValues.size() < bandNo )
  {
    QgsDebugMsgLevel( u"No std deviation value for band available"_s, 2 );
    return false;
  }

  return true;
}

QgsRasterBandStats QgsImageServerProvider::bandStatistics( int bandNo, Qgis::RasterBandStatistics stats, const QgsRectangle &boundingBox, int sampleSize, QgsRasterBlockFeedback *feedback )
{
  QgsRasterBandStats rasterBandStats;
  initStatistics( rasterBandStats, bandNo, stats, boundingBox, sampleSize );

  for ( const QgsRasterBandStats &stats : std::as_const( mStatistics ) )
  {
    if ( stats.contains( rasterBandStats ) )
    {
      QgsDebugMsgLevel( u"Using cached statistics."_s, 2 );
      return stats;
    }
  }

  // We cannot use provider stats if user disabled src no data value or set
  // custom no data values
  if ( ( sourceHasNoDataValue( bandNo ) && !useSourceNoDataValue( bandNo ) ) || !userNoDataValues( bandNo ).isEmpty() )
  {
    QgsDebugMsgLevel( u"Custom NoData values, using generic statistics."_s, 2 );
    return QgsRasterDataProvider::bandStatistics( bandNo, stats, boundingBox, sampleSize, feedback );
  }

  const Qgis::RasterBandStatistics supportedStats = Qgis::RasterBandStatistic::Min
                                                    | Qgis::RasterBandStatistic::Max
                                                    | Qgis::RasterBandStatistic::Range
                                                    | Qgis::RasterBandStatistic::Mean
                                                    | Qgis::RasterBandStatistic::StdDev;

  QgsDebugMsgLevel( u"theStats = %1 supportedStats = %2"_s.arg( static_cast<int>( stats ), 0, 2 ).arg( static_cast<int>( supportedStats ), 0, 2 ), 2 );

  if ( rasterBandStats.extent != extent() || ( stats & ( ~supportedStats ) ) )
  {
    QgsDebugMsgLevel( u"Statistics not supported by provider, using generic statistics."_s, 2 );
    return QgsRasterDataProvider::bandStatistics( bandNo, stats, boundingBox, sampleSize, feedback );
  }

  QgsDebugMsgLevel( u"Using provider statistics."_s, 2 );

  rasterBandStats.bandNumber = bandNo;
  rasterBandStats.range = mMaxValues.value( bandNo - 1 ) - mMinValues.value( bandNo - 1 );
  rasterBandStats.minimumValue = mMinValues.value( bandNo - 1 );
  rasterBandStats.maximumValue = mMaxValues.value( bandNo - 1 );
  rasterBandStats.mean = mMeanValues.value( bandNo - 1 );
  rasterBandStats.sum = 0;          //not available via provider
  rasterBandStats.elementCount = 0; //not available via provider
  rasterBandStats.sumOfSquares = 0; //not available via provider
  rasterBandStats.stdDev = mStdevValues.value( bandNo - 1 );
  rasterBandStats.statsGathered = Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max | Qgis::RasterBandStatistic::Range | Qgis::RasterBandStatistic::Mean | Qgis::RasterBandStatistic::StdDev;

  mStatistics.append( rasterBandStats );
  return rasterBandStats;
}

Qgis::DataType QgsImageServerProvider::dataType( int ) const
{
  return mDataType;
}

Qgis::DataType QgsImageServerProvider::sourceDataType( int ) const
{
  return mDataType;
}

QString QgsImageServerProvider::description() const
{
  return IMS_PROVIDER_DESCRIPTION;
}

void QgsImageServerProvider::reloadProviderData()
{
  mCachedImage = QImage();
}

bool QgsImageServerProvider::renderInPreview( const QgsDataProvider::PreviewContext &context )
{
  if ( mTiled )
    return true;

  return QgsRasterDataProvider::renderInPreview( context );
}

QgsLayerMetadata QgsImageServerProvider::layerMetadata() const
{
  return mLayerMetadata;
}

QgsImageServerProvider *QgsImageServerProvider::clone() const
{
  QgsDataProvider::ProviderOptions options;
  options.transformContext = transformContext();
  QgsImageServerProvider *provider = new QgsImageServerProvider( *this, options );
  provider->copyBaseSettings( *this );
  return provider;
}

QString QgsImageServerProvider::htmlMetadata() const
{
  return QgsVariantUtils::variantToHtml( mServiceInfo, tr( "Service Info" ) ) + QgsVariantUtils::variantToHtml( mLayerInfo, tr( "Layer Info" ) );
}

QgsRasterIdentifyResult QgsImageServerProvider::identify( const QgsPointXY &point, Qgis::RasterIdentifyFormat format, const QgsRectangle &, int, int, int )
{
  if ( format != Qgis::RasterIdentifyFormat::Value )
  {
    return QgsRasterIdentifyResult( QgsError( tr( "Format not supported" ), "ImageServer Provider" ) );
  }

  QMap<int, QVariant> results;
  if ( !extent().contains( point ) )
  {
    // Outside the raster
    for ( int bandNo = 1; bandNo <= bandCount(); bandNo++ )
    {
      results.insert( bandNo, QVariant() ); // null QVariant represents no data
    }
    return QgsRasterIdentifyResult( Qgis::RasterIdentifyFormat::Value, results );
  }

  // https://developers.arcgis.com/rest/services-reference/enterprise/identify-image-service/
  QgsDataSourceUri dataSource( dataSourceUri() );
  QUrl queryUrl( dataSource.param( u"url"_s ) + "/identify" );
  QUrlQuery query( queryUrl );
  query.addQueryItem( u"f"_s, u"json"_s );
  query.addQueryItem( u"geometryType"_s, u"esriGeometryPoint"_s );
  query.addQueryItem( u"geometry"_s, u"%1,%2"_s.arg( point.x(), 0, 'f' ).arg( point.y(), 0, 'f' ) );
  queryUrl.setQuery( query );

  const QVariantMap queryResults = QgsArcGisRestQueryUtils::queryServiceJSON( queryUrl, mAuthCfg, mErrorTitle, mError, mRequestHeaders, nullptr, mUrlPrefix );
  const QStringList values = queryResults.value( u"value"_s ).toString().split( ',' );
  for ( int bandNo = 1; bandNo <= bandCount(); bandNo++ )
  {
    bool ok = false;
    const double value = values.value( bandNo - 1 ).toDouble( &ok );
    if ( !ok )
    {
      results.insert( bandNo, QVariant() ); // null QVariant represents no data
      continue;
    }

    if ( ( sourceHasNoDataValue( bandNo ) && useSourceNoDataValue( bandNo ) && ( std::isnan( value ) || qgsDoubleNear( value, sourceNoDataValue( bandNo ) ) ) )
         || ( QgsRasterRange::contains( value, userNoDataValues( bandNo ) ) ) )
    {
      results.insert( bandNo, QVariant() ); // null QVariant represents no data
    }
    else
    {
      if ( sourceDataType( bandNo ) == Qgis::DataType::Float32 )
      {
        results.insert( bandNo, static_cast<float>( value ) );
      }
      else
        results.insert( bandNo, value );
    }
  }
  return QgsRasterIdentifyResult( Qgis::RasterIdentifyFormat::Value, results );
}

QList<double> QgsImageServerProvider::nativeResolutions() const
{
  return mResolutions;
}

bool QgsImageServerProvider::readBlock( int bandNo, const QgsRectangle &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback )
{
  if ( !mValid || width <= 0 || height <= 0 )
    return false;

  QgsDataSourceUri dataSource( dataSourceUri() );
  QString url = dataSource.param( u"url"_s );
  if ( !dataSource.param( u"layer"_s ).isEmpty() )
  {
    url += u"/"_s + dataSource.param( u"layer"_s );
  }
  url += "/exportImage"_L1;

  QUrl queryUrl( url );
  QUrlQuery query;

  const QgsRectangle requestExtent = viewExtent.intersect( mExtent );
  if ( requestExtent.isEmpty() )
  {
    QgsDebugMsgLevel( u"draw request outside view extent."_s, 2 );
    return false;
  }

  QRect blockSubRectPixels( 0, 0, width, height ); // Hoisted and defaults to full size
  if ( requestExtent != viewExtent )
  {
    blockSubRectPixels = QgsRasterBlock::subRect( viewExtent, width, height, requestExtent );
    query.addQueryItem( u"bbox"_s, u"%1,%2,%3,%4"_s.arg( requestExtent.xMinimum(), 0, 'f' ).arg( requestExtent.yMinimum(), 0, 'f' ).arg( requestExtent.xMaximum(), 0, 'f' ).arg( requestExtent.yMaximum(), 0, 'f' ) );
    query.addQueryItem( u"size"_s, u"%1,%2"_s.arg( blockSubRectPixels.width() ).arg( blockSubRectPixels.height() ) );
  }
  else
  {
    query.addQueryItem( u"bbox"_s, u"%1,%2,%3,%4"_s.arg( viewExtent.xMinimum(), 0, 'f' ).arg( viewExtent.yMinimum(), 0, 'f' ).arg( viewExtent.xMaximum(), 0, 'f' ).arg( viewExtent.yMaximum(), 0, 'f' ) );
    query.addQueryItem( u"size"_s, u"%1,%2"_s.arg( width ).arg( height ) );
  }

  query.addQueryItem( u"f"_s, u"image"_s );
  query.addQueryItem( u"bandIds"_s, QString::number( bandNo - 1 ) );
  query.addQueryItem( u"interpretation"_s, u"RSP_BilinearInterpolation"_s );
  query.addQueryItem( u"pixelType"_s, mPixelType );

  // determine the request format
  // default to tiff as a safe option since it handles all bit depths and nodata.
  QString requestFormat = u"tiff"_s;
  QString vsiExtension = u".tif"_s;
  if ( !dataSource.param( u"format"_s ).isEmpty() )
  {
    // provider has a hardcoded format, use that
    requestFormat = dataSource.param( u"format"_s );
    vsiExtension.clear();
  }
  else if ( mMaximumLercVersionSupported > 0 && ( mDataType == Qgis::DataType::Float32 || mDataType == Qgis::DataType::Float64 ) )
  {
    // if the data is floating point and both server and client support it, request LERC compression for smaller transfers
    requestFormat = u"lerc"_s;
    vsiExtension.clear();
    if ( mMaximumLercVersionSupported >= 2 )
    {
      // for now GDAL can only read v2 lerc, so don't request a later version then that.
      // if lerc version 1 is maximum supported, then the server won't support the lercVersion query item at all
      query.addQueryItem( u"lercVersion"_s, u"2"_s );
    }
    query.addQueryItem( u"compression"_s, u"LERC"_s );
    // we want lossless compression
    query.addQueryItem( u"compressionTolerance"_s, u"0"_s );
  }
  query.addQueryItem( u"format"_s, requestFormat );

  queryUrl.setQuery( query );

  queryUrl = QgsArcGisRestQueryUtils::parseUrl( queryUrl );

  QgsBlockingNetworkRequest networkRequest;
  networkRequest.setAuthCfg( mAuthCfg );
  QNetworkRequest req( queryUrl );
  mRequestHeaders.updateNetworkRequest( req );
  QgsSetRequestInitiatorClass( req, u"QgsImageServerProvider"_s );

  if ( networkRequest.get( req, false, feedback ) != QgsBlockingNetworkRequest::NoError )
  {
    if ( !feedback || !feedback->isCanceled() )
    {
      QgsDebugMsgLevel( u"Network request failed: %1"_s.arg( networkRequest.errorMessage() ), 2 );
    }
    return false;
  }

  const QgsNetworkReplyContent reply = networkRequest.reply();
  QByteArray rawData = reply.content();
  if ( rawData.isEmpty() || ( feedback && feedback->isCanceled() ) )
    return false;

  // use GDAL to read raw raster data, and handle eg lerc decompression
  // first create in-memory dataset
  const QString vsimemFilename = u"/vsimem/ims_%1%2"_s.arg( QUuid::createUuid().toString( QUuid::WithoutBraces ), vsiExtension );
  VSIFCloseL( VSIFileFromMemBuffer( vsimemFilename.toUtf8().constData(), reinterpret_cast<GByte *>( rawData.data() ), rawData.size(), false ) );
  // open the in-memory dataset
  GDALDatasetH hDS = GDALOpen( vsimemFilename.toUtf8().constData(), GA_ReadOnly );
  if ( !hDS || ( feedback && feedback->isCanceled() ) )
  {
    QgsDebugMsgLevel( u"GDAL failed to open the downloaded image payload."_s, 2 );
    VSIUnlink( vsimemFilename.toUtf8().constData() );
    return false;
  }

  // we'll always have a single-band raster, so we request the first band
  GDALRasterBandH hBand = GDALGetRasterBand( hDS, 1 );
  if ( !hBand || ( feedback && feedback->isCanceled() ) )
  {
    GDALClose( hDS );
    VSIUnlink( vsimemFilename.toUtf8().constData() );
    return false;
  }

  const GDALDataType gdalType = QgsGdalUtils::gdalDataTypeFromQgisDataType( mDataType );
  const int elementSize = GDALGetDataTypeSizeBytes( gdalType );
  const std::size_t pixelCount = static_cast< std::size_t >( width ) * static_cast< std::size_t >( height );

  // pre-fill the entire buffer with NoData (ie pad the exterior of the subrect containing the actual data from the service)
  const bool hasNoData = mSrcHasNoDataValue.size() >= bandNo && mSrcHasNoDataValue.at( bandNo - 1 );
  const double noDataVal = hasNoData ? mSrcNoDataValue.at( bandNo - 1 ) : 0.0;
  switch ( mDataType )
  {
    case Qgis::DataType::Byte:
      std::fill_n( static_cast<quint8 *>( data ), pixelCount, static_cast<quint8>( noDataVal ) );
      break;
    case Qgis::DataType::Int8:
      std::fill_n( static_cast<qint8 *>( data ), pixelCount, static_cast<qint8>( noDataVal ) );
      break;
    case Qgis::DataType::UInt16:
      std::fill_n( static_cast<quint16 *>( data ), pixelCount, static_cast<quint16>( noDataVal ) );
      break;
    case Qgis::DataType::Int16:
      std::fill_n( static_cast<qint16 *>( data ), pixelCount, static_cast<qint16>( noDataVal ) );
      break;
    case Qgis::DataType::UInt32:
      std::fill_n( static_cast<quint32 *>( data ), pixelCount, static_cast<quint32>( noDataVal ) );
      break;
    case Qgis::DataType::Int32:
      std::fill_n( static_cast<qint32 *>( data ), pixelCount, static_cast<qint32>( noDataVal ) );
      break;

    case Qgis::DataType::Float32:
    {
      float fillVal = hasNoData ? static_cast<float>( noDataVal ) : std::numeric_limits<float>::quiet_NaN();
      std::fill_n( static_cast<float *>( data ), pixelCount, fillVal );
      break;
    }
    case Qgis::DataType::Float64:
    {
      double fillVal = hasNoData ? noDataVal : std::numeric_limits<double>::quiet_NaN();
      std::fill_n( static_cast<double *>( data ), pixelCount, fillVal );
      break;
    }

    case Qgis::DataType::UnknownDataType:
    case Qgis::DataType::CInt16:
    case Qgis::DataType::CInt32:
    case Qgis::DataType::CFloat32:
    case Qgis::DataType::CFloat64:
    case Qgis::DataType::ARGB32:
    case Qgis::DataType::ARGB32_Premultiplied:
      memset( data, 0, static_cast<size_t>( pixelCount ) * elementSize );
      break;
  }

  if ( feedback && feedback->isCanceled() )
  {
    GDALClose( hDS );
    VSIUnlink( vsimemFilename.toUtf8().constData() );
    return false;
  }

  GDALRasterIOExtraArg extraArg;
  INIT_RASTERIO_EXTRA_ARG( extraArg );
  std::unique_ptr<QgsGdalProgressAdapter> progressAdapter;
  if ( feedback )
  {
    progressAdapter = std::make_unique<QgsGdalProgressAdapter>( feedback );
    extraArg.pfnProgress = QgsGdalProgressAdapter::progressCallback;
    extraArg.pProgressData = progressAdapter.get();
  }

  // write actual data into valid subRect
  void *targetData = static_cast<GByte *>( data ) + ( static_cast<GSpacing>( blockSubRectPixels.y() ) * width + blockSubRectPixels.x() ) * elementSize;
  const GSpacing pixelSpace = elementSize;
  const GSpacing lineSpace = static_cast<GSpacing>( width ) * elementSize;
  const CPLErr err
    = GDALRasterIOEx( hBand, GF_Read, 0, 0, GDALGetRasterXSize( hDS ), GDALGetRasterYSize( hDS ), targetData, blockSubRectPixels.width(), blockSubRectPixels.height(), gdalType, pixelSpace, lineSpace, &extraArg );

  GDALClose( hDS );
  VSIUnlink( vsimemFilename.toUtf8().constData() );

  if ( feedback && feedback->isCanceled() )
  {
    return false;
  }

  return err == CE_None;
}

bool QgsImageServerProvider::readNativeAttributeTable( QString *errorMessage )
{
  if ( !mHasRat || !mValid )
    return false;

  QgsDataSourceUri dataSource( dataSourceUri() );
  QString url = dataSource.param( u"url"_s );
  if ( !dataSource.param( u"layer"_s ).isEmpty() )
  {
    url += u"/"_s + dataSource.param( u"layer"_s );
  }
  url += "/rasterAttributeTable"_L1;

  QUrl queryUrl( url );
  QUrlQuery query( queryUrl );
  query.addQueryItem( u"f"_s, u"json"_s );
  queryUrl.setQuery( query );

  QString errorTitle;
  QString errorText;
  const QVariantMap results = QgsArcGisRestQueryUtils::queryServiceJSON( queryUrl, mAuthCfg, errorTitle, errorText, mRequestHeaders, nullptr, mUrlPrefix );
  if ( !errorText.isEmpty() )
  {
    if ( errorMessage && !errorText.isEmpty() )
      *errorMessage = errorText;
    return false;
  }

  auto rat = std::make_unique<QgsRasterAttributeTable>();

  const QVariantList jsonFields = results.value( u"fields"_s ).toList();
  QStringList fieldNames;
  for ( const QVariant &jsonField : jsonFields )
  {
    const QVariantMap fieldDef = jsonField.toMap();
    const QString name = fieldDef.value( u"name"_s ).toString();
    fieldNames << name;
    const QString type = fieldDef.value( u"type"_s ).toString();
    const QMetaType::Type metaType = QgsArcGisRestUtils::convertFieldType( type );

    Qgis::RasterAttributeTableFieldUsage usage = Qgis::RasterAttributeTableFieldUsage::Generic;
    if ( name.compare( "Value"_L1, Qt::CaseInsensitive ) == 0 )
    {
      usage = Qgis::RasterAttributeTableFieldUsage::MinMax;
    }
    else if ( name.compare( "Count"_L1, Qt::CaseInsensitive ) == 0 )
    {
      usage = Qgis::RasterAttributeTableFieldUsage::PixelCount;
    }
    else if ( name.compare( "Red"_L1, Qt::CaseInsensitive ) == 0 )
    {
      usage = Qgis::RasterAttributeTableFieldUsage::Red;
    }
    else if ( name.compare( "Green"_L1, Qt::CaseInsensitive ) == 0 )
    {
      usage = Qgis::RasterAttributeTableFieldUsage::Green;
    }
    else if ( name.compare( "Blue"_L1, Qt::CaseInsensitive ) == 0 )
    {
      usage = Qgis::RasterAttributeTableFieldUsage::Blue;
    }
    else if ( name.compare( "ClassName"_L1, Qt::CaseInsensitive ) == 0 )
    {
      usage = Qgis::RasterAttributeTableFieldUsage::Name;
    }

    rat->appendField( name, usage, metaType );
  }

  const QVariantList featureList = results.value( u"features"_s ).toList();
  for ( const QVariant &feature : featureList )
  {
    QVariantList rowData;
    const QVariantMap featureData = feature.toMap();
    const QVariantMap attributes = featureData.value( u"attributes"_s ).toMap();
    for ( const QString &fieldName : std::as_const( fieldNames ) )
    {
      rowData << attributes.value( fieldName );
    }
    rat->appendRow( rowData );
  }

  if ( !rat->isValid( errorMessage ) )
  {
    return false;
  }
  rat->setDirty( false );

  setAttributeTable( 1, rat.release() );
  return true;
}

QgsImageServerProviderMetadata::QgsImageServerProviderMetadata()
  : QgsProviderMetadata( QgsImageServerProvider::IMS_PROVIDER_KEY, QgsImageServerProvider::IMS_PROVIDER_DESCRIPTION )
{}

QIcon QgsImageServerProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconAms.svg"_s );
}

QgsProviderMetadata::ProviderCapabilities QgsImageServerProviderMetadata::providerCapabilities() const
{
  return QgsProviderMetadata::ProviderCapability::ParallelCreateProvider;
}

QgsImageServerProvider *QgsImageServerProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  return new QgsImageServerProvider( uri, options, flags );
}

QVariantMap QgsImageServerProviderMetadata::decodeUri( const QString &uri ) const
{
  QgsDataSourceUri dsUri = QgsDataSourceUri( uri );

  QVariantMap components;
  components.insert( u"url"_s, dsUri.param( u"url"_s ) );

  dsUri.httpHeaders().updateMap( components );

  if ( !dsUri.param( u"crs"_s ).isEmpty() )
  {
    components.insert( u"crs"_s, dsUri.param( u"crs"_s ) );
  }
  if ( !dsUri.authConfigId().isEmpty() )
  {
    components.insert( u"authcfg"_s, dsUri.authConfigId() );
  }
  if ( !dsUri.param( u"format"_s ).isEmpty() )
  {
    components.insert( u"format"_s, dsUri.param( u"format"_s ) );
  }
  if ( !dsUri.param( u"layer"_s ).isEmpty() )
  {
    components.insert( u"layer"_s, dsUri.param( u"layer"_s ) );
  }

  return components;
}

QString QgsImageServerProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setParam( u"url"_s, parts.value( u"url"_s ).toString() );

  if ( !parts.value( u"crs"_s ).toString().isEmpty() )
  {
    dsUri.setParam( u"crs"_s, parts.value( u"crs"_s ).toString() );
  }

  dsUri.httpHeaders().setFromMap( parts );

  if ( !parts.value( u"authcfg"_s ).toString().isEmpty() )
  {
    dsUri.setAuthConfigId( parts.value( u"authcfg"_s ).toString() );
  }
  if ( !parts.value( u"format"_s ).toString().isEmpty() )
  {
    dsUri.setParam( u"format"_s, parts.value( u"format"_s ).toString() );
  }
  if ( !parts.value( u"layer"_s ).toString().isEmpty() )
  {
    dsUri.setParam( u"layer"_s, parts.value( u"layer"_s ).toString() );
  }

  return dsUri.uri( false );
}

QList<Qgis::LayerType> QgsImageServerProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Raster };
}

#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsImageServerProviderMetadata();
}
#endif
