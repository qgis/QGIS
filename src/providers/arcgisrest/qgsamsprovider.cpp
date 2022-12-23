/***************************************************************************
    qgsamsprovider.cpp - ArcGIS MapServer Raster Provider
     ----------------------------------------------------
    Date                 : Nov 24, 2015
    Copyright            : (C) 2015 by Sandro Mani
    email                : manisandro@gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsamsprovider.h"
#include "qgsarcgisrestutils.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsrasteridentifyresult.h"
#include "qgsfeaturestore.h"
#include "qgsgeometry.h"
#include "qgsapplication.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssettings.h"
#include "qgsmessagelog.h"
#include "qgsauthmanager.h"
#include "qgstilecache.h"
#include "qgsstringutils.h"
#include "qgsarcgisrestquery.h"

#include <cstring>
#include <QFontMetrics>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPainter>
#include <QNetworkCacheMetaData>
#include <QUrlQuery>
#include <QDir>
#include <QTimer>

const QString QgsAmsProvider::AMS_PROVIDER_KEY = QStringLiteral( "arcgismapserver" );
const QString QgsAmsProvider::AMS_PROVIDER_DESCRIPTION = QStringLiteral( "ArcGIS Map Service data provider" );


//! a helper class for ordering tile requests according to the distance from view center
struct LessThanTileRequest
{
  QgsPointXY center;
  bool operator()( const QgsAmsProvider::TileRequest &req1, const QgsAmsProvider::TileRequest &req2 )
  {
    QPointF p1 = req1.mapExtent.center();
    QPointF p2 = req2.mapExtent.center();
    // using chessboard distance (loading order more natural than euclidean/manhattan distance)
    double d1 = std::max( std::fabs( center.x() - p1.x() ), std::fabs( center.y() - p1.y() ) );
    double d2 = std::max( std::fabs( center.x() - p2.x() ), std::fabs( center.y() - p2.y() ) );
    return d1 < d2;
  }
};

QgsAmsLegendFetcher::QgsAmsLegendFetcher( QgsAmsProvider *provider, const QImage &fetchedImage )
  : QgsImageFetcher( provider )
  , mProvider( provider )
  , mLegendImage( fetchedImage )
{
  mQuery = new QgsArcGisAsyncQuery( this );
  connect( mQuery, &QgsArcGisAsyncQuery::finished, this, &QgsAmsLegendFetcher::handleFinished );
  connect( mQuery, &QgsArcGisAsyncQuery::failed, this, &QgsAmsLegendFetcher::handleError );
}

void QgsAmsLegendFetcher::start()
{
  if ( mLegendImage.isNull() )
  {
    // http://resources.arcgis.com/en/help/rest/apiref/mslegend.html
    // http://sampleserver5.arcgisonline.com/arcgis/rest/services/CommunityAddressing/MapServer/legend?f=pjson
    QgsDataSourceUri dataSource( mProvider->dataSourceUri() );
    const QString authCfg = dataSource.authConfigId();

    QUrl queryUrl( dataSource.param( QStringLiteral( "url" ) ) + "/legend" );
    QUrlQuery query( queryUrl );
    query.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "json" ) );
    queryUrl.setQuery( query );
    mQuery->start( queryUrl, authCfg, &mQueryReply, false, dataSource.httpHeaders() );
  }
  else
  {
    QTimer::singleShot( 1, this, &QgsAmsLegendFetcher::sendCachedImage );
  }
}

void QgsAmsLegendFetcher::handleError( const QString &errorTitle, const QString &errorMsg )
{
  mErrorTitle = errorTitle;
  mError = errorMsg;
  emit error( errorTitle + ": " + errorMsg );
}

void QgsAmsLegendFetcher::sendCachedImage()
{
  emit finish( mLegendImage );
}

void QgsAmsLegendFetcher::handleFinished()
{
  // Parse result
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson( mQueryReply, &err );
  if ( doc.isNull() )
  {
    emit error( QStringLiteral( "Parsing error: %1" ).arg( err.errorString() ) );
  }
  QVariantMap queryResults = doc.object().toVariantMap();
  QgsDataSourceUri dataSource( mProvider->dataSourceUri() );
  QVector< QPair<QString, QImage> > legendEntries;
  const QVariantList layersList = queryResults.value( QStringLiteral( "layers" ) ).toList();
  for ( const QVariant &result : layersList )
  {
    QVariantMap queryResultMap = result.toMap();
    QString layerId = queryResultMap[QStringLiteral( "layerId" )].toString();
    if ( !dataSource.param( QStringLiteral( "layer" ) ).isNull()
         && layerId != dataSource.param( QStringLiteral( "layer" ) )
         && !mProvider->subLayers().contains( layerId ) )
    {
      continue;
    }
    const QVariantList legendSymbols = queryResultMap[QStringLiteral( "legend" )].toList();
    for ( const QVariant &legendEntry : legendSymbols )
    {
      QVariantMap legendEntryMap = legendEntry.toMap();
      QString label = legendEntryMap[QStringLiteral( "label" )].toString();
      if ( label.isEmpty() && legendSymbols.size() == 1 )
        label = queryResultMap[QStringLiteral( "layerName" )].toString();
      QByteArray imageData = QByteArray::fromBase64( legendEntryMap[QStringLiteral( "imageData" )].toByteArray() );
      legendEntries.append( qMakePair( label, QImage::fromData( imageData ) ) );
    }
  }
  if ( !legendEntries.isEmpty() )
  {
    int padding = 5;
    int imageSize = 20;

    QgsSettings settings;
    QFont font = qApp->font();
    int fontSize = settings.value( QStringLiteral( "/qgis/stylesheet/fontPointSize" ), font.pointSize() ).toInt();
    font.setPointSize( fontSize );
    QString fontFamily = settings.value( QStringLiteral( "/qgis/stylesheet/fontFamily" ), font.family() ).toString();
    font.setFamily( fontFamily );
    QFontMetrics fm( font );
    int textWidth = 0;
    int textHeight = fm.ascent();

    int verticalSize = std::max( imageSize, textHeight );
    int verticalPadding = 1;

    typedef QPair<QString, QImage> LegendEntry_t;
    QSize maxImageSize( 0, 0 );
    for ( const LegendEntry_t &legendEntry : std::as_const( legendEntries ) )
    {
      maxImageSize.setWidth( std::max( maxImageSize.width(), legendEntry.second.width() ) );
      maxImageSize.setHeight( std::max( maxImageSize.height(), legendEntry.second.height() ) );
      textWidth = std::max( textWidth, fm.boundingRect( legendEntry.first ).width() + 10 );
    }
    double scaleFactor = maxImageSize.width() == 0 || maxImageSize.height() == 0 ? 1.0 :
                         std::min( 1., std::min( double( imageSize ) / maxImageSize.width(), double( imageSize ) / maxImageSize.height() ) );

    mLegendImage = QImage( imageSize + padding + textWidth, verticalPadding + legendEntries.size() * ( verticalSize + verticalPadding ), QImage::Format_ARGB32 );
    mLegendImage.fill( Qt::transparent );
    QPainter painter( &mLegendImage );
    painter.setFont( font );
    int i = 0;
    for ( const LegendEntry_t &legendEntry : std::as_const( legendEntries ) )
    {
      QImage symbol = legendEntry.second.scaled( legendEntry.second.width() * scaleFactor, legendEntry.second.height() * scaleFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation );
      painter.drawImage( 0, verticalPadding + i * ( verticalSize + verticalPadding ) + ( verticalSize - symbol.height() ), symbol );
      painter.drawText( imageSize + padding, verticalPadding + i * ( verticalSize + verticalPadding ), textWidth, verticalSize, Qt::AlignLeft | Qt::AlignVCenter, legendEntry.first );
      ++i;
    }
  }
  emit fetchedNew( mLegendImage );
  emit finish( mLegendImage );
}

///////////////////////////////////////////////////////////////////////////////

QgsAmsProvider::QgsAmsProvider( const QString &uri, const ProviderOptions &options, QgsDataProvider::ReadFlags flags )
  : QgsRasterDataProvider( uri, options, flags )
{
  QgsDataSourceUri dataSource( dataSourceUri() );
  mRequestHeaders = dataSource.httpHeaders();

  mLegendFetcher = new QgsAmsLegendFetcher( this, QImage() );


  const QString authcfg = dataSource.authConfigId();

  const QString serviceUrl = dataSource.param( QStringLiteral( "url" ) );
  if ( !serviceUrl.isEmpty() )
    mServiceInfo = QgsArcGisRestQueryUtils::getServiceInfo( serviceUrl, authcfg, mErrorTitle, mError, mRequestHeaders );

  QString layerUrl;
  if ( dataSource.param( QStringLiteral( "layer" ) ).isEmpty() )
  {
    layerUrl = serviceUrl;
    mLayerInfo = mServiceInfo;
    if ( mServiceInfo.value( QStringLiteral( "serviceDataType" ) ).toString().startsWith( QLatin1String( "esriImageService" ) ) )
      mImageServer = true;
  }
  else
  {
    layerUrl = dataSource.param( QStringLiteral( "url" ) ) + "/" + dataSource.param( QStringLiteral( "layer" ) );
    mLayerInfo = QgsArcGisRestQueryUtils::getLayerInfo( layerUrl, authcfg, mErrorTitle, mError, mRequestHeaders );
  }

  QVariantMap extentData;
  if ( mLayerInfo.contains( QStringLiteral( "extent" ) ) )
  {
    extentData = mLayerInfo.value( QStringLiteral( "extent" ) ).toMap();
  }
  else
  {
    extentData = mLayerInfo.value( QStringLiteral( "fullExtent" ) ).toMap();
  }
  mExtent.setXMinimum( extentData[QStringLiteral( "xmin" )].toDouble() );
  mExtent.setYMinimum( extentData[QStringLiteral( "ymin" )].toDouble() );
  mExtent.setXMaximum( extentData[QStringLiteral( "xmax" )].toDouble() );
  mExtent.setYMaximum( extentData[QStringLiteral( "ymax" )].toDouble() );
  mCrs = QgsArcGisRestUtils::convertSpatialReference( extentData[QStringLiteral( "spatialReference" )].toMap() );
  if ( !mCrs.isValid() )
  {
    appendError( QgsErrorMessage( tr( "Could not parse spatial reference" ), QStringLiteral( "AMSProvider" ) ) );
    return;
  }

  QgsLayerMetadata::SpatialExtent spatialExtent;
  spatialExtent.bounds = QgsBox3d( mExtent );
  spatialExtent.extentCrs = mCrs;
  QgsLayerMetadata::Extent metadataExtent;
  metadataExtent.setSpatialExtents( QList<  QgsLayerMetadata::SpatialExtent >() << spatialExtent );
  mLayerMetadata.setExtent( metadataExtent );
  mLayerMetadata.setCrs( mCrs );

  mTiled = mServiceInfo.value( QStringLiteral( "singleFusedMapCache" ) ).toBool();
  if ( dataSource.param( QStringLiteral( "tiled" ) ).toLower() == "false" || dataSource.param( QStringLiteral( "tiled" ) ) == "0" )
  {
    mTiled = false;
  }

  if ( mServiceInfo.contains( QStringLiteral( "maxImageWidth" ) ) )
    mMaxImageWidth = mServiceInfo.value( QStringLiteral( "maxImageWidth" ) ).toInt();
  if ( mServiceInfo.contains( QStringLiteral( "maxImageHeight" ) ) )
    mMaxImageHeight = mServiceInfo.value( QStringLiteral( "maxImageHeight" ) ).toInt();

  const QVariantList subLayersList = mLayerInfo.value( QStringLiteral( "subLayers" ) ).toList();
  mSubLayers.reserve( subLayersList.size() );
  for ( const QVariant &sublayer : subLayersList )
  {
    mSubLayers.append( sublayer.toMap()[QStringLiteral( "id" )].toString() );
    mSubLayerVisibilities.append( true );
  }

  mTimestamp = QDateTime::currentDateTime();
  mValid = true;

  // layer metadata

  mLayerMetadata.setIdentifier( layerUrl );
  mLayerMetadata.setParentIdentifier( serviceUrl );
  mLayerMetadata.setType( QStringLiteral( "dataset" ) );
  mLayerMetadata.setTitle( mLayerInfo.value( QStringLiteral( "name" ) ).toString() );
  mLayerMetadata.setAbstract( mLayerInfo.value( QStringLiteral( "description" ) ).toString() );
  const QString copyright = mLayerInfo.value( QStringLiteral( "copyrightText" ) ).toString();
  if ( !copyright.isEmpty() )
    mLayerMetadata.setRights( QStringList() << copyright );
  mLayerMetadata.addLink( QgsAbstractMetadataBase::Link( tr( "Source" ), QStringLiteral( "WWW:LINK" ), layerUrl ) );
  const QVariantMap docInfo = mServiceInfo.value( QStringLiteral( "documentInfo" ) ).toMap();
  const QStringList keywords = docInfo.value( QStringLiteral( "Keywords" ) ).toString().split( ',' );
  if ( !keywords.empty() )
  {
    mLayerMetadata.addKeywords( QStringLiteral( "keywords" ), keywords );
  }
  const QString category = docInfo.value( QStringLiteral( "Category" ) ).toString();
  if ( !category.isEmpty() )
    mLayerMetadata.setCategories( QStringList() << category );
  const QString author = docInfo.value( QStringLiteral( "Author" ) ).toString();
  if ( !author.isEmpty() )
  {
    QgsAbstractMetadataBase::Contact contact( author );
    contact.role = QStringLiteral( "author" );
    mLayerMetadata.addContact( contact );
  }

  if ( mTiled )
  {
    const QVariantMap tileInfo = mServiceInfo.value( QStringLiteral( "tileInfo" ) ).toMap();
    const QList<QVariant> lodEntries = tileInfo[QStringLiteral( "lods" )].toList();
    for ( const QVariant &lodEntry : lodEntries )
    {
      const QVariantMap lodEntryMap = lodEntry.toMap();
      mResolutions << lodEntryMap[QStringLiteral( "resolution" )].toDouble();
    }
    std::sort( mResolutions.begin(), mResolutions.end() );
  }
}

QgsAmsProvider::QgsAmsProvider( const QgsAmsProvider &other, const QgsDataProvider::ProviderOptions &providerOptions )
  : QgsRasterDataProvider( other.dataSourceUri(), providerOptions )
  , mValid( other.mValid )
// intentionally omitted:
// - mLegendFetcher
  , mServiceInfo( other.mServiceInfo )
  , mLayerInfo( other.mLayerInfo )
  , mCrs( other.mCrs )
  , mExtent( other.mExtent )
  , mSubLayers( other.mSubLayers )
  , mSubLayerVisibilities( other.mSubLayerVisibilities )
  , mRequestHeaders( other.mRequestHeaders )
  , mTiled( other.mTiled )
  , mImageServer( other.mImageServer )
  , mMaxImageWidth( other.mMaxImageWidth )
  , mMaxImageHeight( other.mMaxImageHeight )
  , mLayerMetadata( other.mLayerMetadata )
  , mResolutions( other.mResolutions )
// intentionally omitted:
// - mErrorTitle
// - mError
// - mCachedImage
// - mCachedImageExtent
{
  mLegendFetcher = new QgsAmsLegendFetcher( this, other.mLegendFetcher->getImage() );

  // is this needed?
  mTimestamp = QDateTime::currentDateTime();
}

QgsRasterDataProvider::ProviderCapabilities QgsAmsProvider::providerCapabilities() const
{
  return ProviderCapability::ReadLayerMetadata | ProviderCapability::ReloadData;
}

QString QgsAmsProvider::name() const { return AMS_PROVIDER_KEY; }

QString QgsAmsProvider::providerKey() { return AMS_PROVIDER_KEY; }

QString QgsAmsProvider::description() const { return AMS_PROVIDER_DESCRIPTION; }

QStringList QgsAmsProvider::subLayerStyles() const
{
  QStringList styles;
  styles.reserve( mSubLayers.size() );
  for ( int i = 0, n = mSubLayers.size(); i < n; ++i )
  {
    styles.append( QString() );
  }
  return styles;
}

void QgsAmsProvider::setLayerOrder( const QStringList &layers )
{
  QStringList oldSubLayers = mSubLayers;
  QList<bool> oldSubLayerVisibilities = mSubLayerVisibilities;
  mSubLayers.clear();
  mSubLayerVisibilities.clear();
  for ( const QString &layer : layers )
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

void QgsAmsProvider::setSubLayerVisibility( const QString &name, bool vis )
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

void QgsAmsProvider::reloadProviderData()
{
  mCachedImage = QImage();
}

bool QgsAmsProvider::renderInPreview( const QgsDataProvider::PreviewContext &context )
{
  if ( mTiled )
    return true;

  return QgsRasterDataProvider::renderInPreview( context );
}

QgsLayerMetadata QgsAmsProvider::layerMetadata() const
{
  return mLayerMetadata;
}

QgsAmsProvider *QgsAmsProvider::clone() const
{
  QgsDataProvider::ProviderOptions options;
  options.transformContext = transformContext();
  QgsAmsProvider *provider = new QgsAmsProvider( *this, options );
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

QString QgsAmsProvider::htmlMetadata()
{
  // This must return the content of a HTML table starting by tr and ending by tr
  return dumpVariantMap( mServiceInfo, tr( "Service Info" ) ) + dumpVariantMap( mLayerInfo, tr( "Layer Info" ) );
}

static bool _fuzzyContainsRect( const QRectF &r1, const QRectF &r2 )
{
  double significantDigits = std::log10( std::max( r1.width(), r1.height() ) );
  double epsilon = std::pow( 10.0, significantDigits - 5 ); // floats have 6-9 significant digits
  return r1.contains( r2.adjusted( epsilon, epsilon, -epsilon, -epsilon ) );
}

QImage QgsAmsProvider::draw( const QgsRectangle &viewExtent, int pixelWidth, int pixelHeight, QgsRasterBlockFeedback *feedback )
{
  QgsDataSourceUri dataSource( dataSourceUri() );
  const QString authcfg = dataSource.authConfigId();

  if ( mTiled )
  {
    mTileReqNo++;

    // Compute ideal resolution
    // - Measure distance in meters along lower and upper edge of bounding box
    // - Target resolution is the coarser resolution (resolution = distMeters / pixelWidth)
    double width = viewExtent.xMaximum() - viewExtent.xMinimum();
    double targetRes = width / ( pixelWidth );

    // Tiles available, assemble image from tiles
    QVariantMap tileInfo = mServiceInfo.value( QStringLiteral( "tileInfo" ) ).toMap();
    int tileWidth = tileInfo[QStringLiteral( "cols" )].toInt();
    int tileHeight = tileInfo[QStringLiteral( "rows" )].toInt();
    QVariantMap origin = tileInfo[QStringLiteral( "origin" )].toMap();
    double ox = origin[QStringLiteral( "x" )].toDouble();
    double oy = origin[QStringLiteral( "y" )].toDouble();

    // Search matching resolution (tile resolution <= targetRes)
    QList<QVariant> lodEntries = tileInfo[QStringLiteral( "lods" )].toList();
    if ( lodEntries.isEmpty() )
    {
      return QImage();
    }
    std::sort( lodEntries.begin(), lodEntries.end(), []( const QVariant & a, const QVariant & b )
    {
      return a.toMap().value( QStringLiteral( "resolution" ) ).toDouble() > b.toMap().value( QStringLiteral( "resolution" ) ).toDouble();
    } );
    int level = 0;
    int foundLevel = -1;

    QMap< int, double > levelToResMap;
    for ( const QVariant &lodEntry : lodEntries )
    {
      QVariantMap lodEntryMap = lodEntry.toMap();

      level = lodEntryMap[QStringLiteral( "level" )].toInt();
      double resolution = lodEntryMap[QStringLiteral( "resolution" )].toDouble();
      levelToResMap.insert( level, resolution );
      if ( foundLevel < 0 && resolution <= 1.5 * targetRes )
      {
        foundLevel = level;
      }
    }
    if ( foundLevel >= 0 )
    {
      level = foundLevel;
    }
    else
    {
      // just use best resolution available
      level = lodEntries.constLast().toMap().value( QStringLiteral( "level" ) ).toInt();
    }

    auto getRequests = [&levelToResMap, &viewExtent, tileWidth, tileHeight, ox, oy, targetRes, &dataSource]( int level, TileRequests & requests )
    {
      const double resolution = levelToResMap.value( level );

      // Get necessary tiles to fill extent
      // tile_x = ox + i * (resolution * tileWidth)
      // tile_y = oy - j * (resolution * tileHeight)
      int ixStart = static_cast< int >( std::floor( ( viewExtent.xMinimum() - ox ) / ( tileWidth * resolution ) ) );
      int iyStart = static_cast< int >( std::floor( ( oy - viewExtent.yMaximum() ) / ( tileHeight * resolution ) ) );
      int ixEnd = static_cast< int >( std::ceil( ( viewExtent.xMaximum() - ox ) / ( tileWidth * resolution ) ) );
      int iyEnd = static_cast< int >( std::ceil( ( oy - viewExtent.yMinimum() ) / ( tileHeight * resolution ) ) );
      double imX = ( viewExtent.xMinimum() - ox ) / resolution;
      double imY = ( oy - viewExtent.yMaximum() ) / resolution;

      int i = 0;
      const double resScale = resolution / targetRes;
      for ( int iy = iyStart; iy <= iyEnd; ++iy )
      {
        for ( int ix = ixStart; ix <= ixEnd; ++ix )
        {
          const QUrl url = QUrl( dataSource.param( QStringLiteral( "url" ) ) + QStringLiteral( "/tile/%1/%2/%3" ).arg( level ).arg( iy ).arg( ix ) );
          const QRectF tileImageRect( ( ix * tileWidth - imX ) * resScale,
                                      ( iy * tileHeight - imY ) * resScale,
                                      tileWidth * resScale,
                                      tileHeight * resScale );

          const QRectF worldRect( ox + ix * ( resolution * tileWidth ), oy - iy * ( resolution * tileHeight ), tileWidth * resolution, tileHeight * resolution );

          requests.push_back( TileRequest( url, tileImageRect, i, worldRect ) );
          i++;
        }
      }
    };

    // Get necessary tiles at ideal level to fill extent
    TileRequests requests;
    getRequests( level, requests );

    QList<TileImage> tileImages;  // in the correct resolution
    QList<QRectF> missing;  // rectangles (in map coords) of missing tiles for this view

    QImage image( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    TileRequests requestsFinal;
    tileImages.reserve( requests.size() );
    missing.reserve( requests.size() );
    requestsFinal.reserve( requests.size() );
    for ( const TileRequest &r : std::as_const( requests ) )
    {
      QImage localImage;
      if ( QgsTileCache::tile( r.url, localImage ) )
      {
        // if image size is "close enough" to destination size, don't smooth it out. Instead try for pixel-perfect placement!
        bool disableSmoothing = ( qgsDoubleNear( r.rect.width(), tileWidth, 2 ) && qgsDoubleNear( r.rect.height(), tileHeight, 2 ) );
        tileImages << TileImage( r.rect, localImage, !disableSmoothing );
      }
      else
      {
        missing << r.rect;

        // need to make a request
        requestsFinal << r;
      }
    }

    QPainter p( &image );

    // draw other res tiles if preview
    if ( feedback && feedback->isPreviewOnly() && missing.count() > 0 )
    {
      // some tiles are still missing, so let's see if we have any cached tiles
      // from lower or higher resolution available to give the user a bit of context
      // while loading the right resolution
      p.setCompositionMode( QPainter::CompositionMode_Source );
      p.setRenderHint( QPainter::SmoothPixmapTransform, false );  // let's not waste time with bilinear filtering

      auto fetchOtherResTiles = [&getRequests]( int otherLevel, QList< TileImage> &otherResTiles, QList< QRectF > &missingRects )
      {
        TileRequests otherRequests;
        getRequests( otherLevel, otherRequests );
        QList<QRectF> missingRectsToDelete;
        for ( const TileRequest &r : std::as_const( otherRequests ) )
        {
          QImage localImage;
          if ( ! QgsTileCache::tile( r.url, localImage ) )
            continue;

          otherResTiles << TileImage( r.rect, localImage, false );

          // see if there are any missing rects that are completely covered by this tile
          for ( const QRectF &missingRect : std::as_const( missingRects ) )
          {
            // we need to do a fuzzy "contains" check because the coordinates may not align perfectly
            // due to numerical errors and/or transform of coords from double to floats
            if ( _fuzzyContainsRect( r.rect, missingRect ) )
            {
              missingRectsToDelete << missingRect;
            }
          }
        }

        // remove all the rectangles we have completely covered by tiles from this resolution
        // so we will not use tiles from multiple resolutions for one missing tile (to save time)
        for ( const QRectF &rectToDelete : std::as_const( missingRectsToDelete ) )
        {
          missingRects.removeOne( rectToDelete );
        }
      };

      QList<TileImage> lowerResTiles, lowerResTiles2, higherResTiles;
      // first we check lower resolution tiles: one level back, then two levels back (if there is still some are not covered),
      // finally (in the worst case we use one level higher resolution tiles). This heuristic should give
      // good overviews while not spending too much time drawing cached tiles from resolutions far away.
      fetchOtherResTiles( level - 1, lowerResTiles, missing );
      fetchOtherResTiles( level - 2, lowerResTiles2, missing );
      fetchOtherResTiles( level + 1, higherResTiles, missing );

      // draw the cached tiles lowest to highest resolution
      const auto constLowerResTiles2 = lowerResTiles2;
      for ( const TileImage &ti : constLowerResTiles2 )
      {
        p.drawImage( ti.rect, ti.img );
      }
      const auto constLowerResTiles = lowerResTiles;
      for ( const TileImage &ti : constLowerResTiles )
      {
        p.drawImage( ti.rect, ti.img );
      }
      const auto constHigherResTiles = higherResTiles;
      for ( const TileImage &ti : constHigherResTiles )
      {
        p.drawImage( ti.rect, ti.img );
      }
    }

    // draw composite in this resolution
    for ( const TileImage &ti : std::as_const( tileImages ) )
    {
      if ( ti.smooth )
        p.setRenderHint( QPainter::SmoothPixmapTransform, true );
      p.drawImage( ti.rect, ti.img );
    }

    p.end();

    if ( feedback && feedback->isPreviewOnly() )
    {
      // preview job only, so don't request any new tiles
    }
    else if ( !requestsFinal.isEmpty() )
    {
      // let the feedback object know about the tiles we have already
      if ( feedback && feedback->renderPartialOutput() )
        feedback->onNewData();

      // order tile requests according to the distance from view center
      LessThanTileRequest cmp;
      cmp.center = viewExtent.center();
      std::sort( requestsFinal.begin(), requestsFinal.end(), cmp );

      QgsAmsTiledImageDownloadHandler handler( authcfg, mRequestHeaders, mTileReqNo, requestsFinal, &image, viewExtent, feedback );
      handler.downloadBlocking();
    }

    return image;
  }
  else
  {
    if ( !mCachedImage.isNull() && mCachedImageExtent == viewExtent )
    {
      return mCachedImage;
    }

    mCachedImage = QImage( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
    mCachedImage.fill( Qt::transparent );
    QPainter p( &mCachedImage );

    int maxWidth = mMaxImageWidth > 0 ? mMaxImageWidth : pixelWidth;
    int maxHeight = mMaxImageHeight > 0 ? mMaxImageHeight : pixelHeight;
    int nbStepWidth = std::ceil( ( float )pixelWidth / maxWidth );
    int nbStepHeight = std::ceil( ( float )pixelHeight / maxHeight );
    for ( int currentStepWidth = 0; currentStepWidth < nbStepWidth; currentStepWidth++ )
    {
      for ( int currentStepHeight = 0; currentStepHeight < nbStepHeight; currentStepHeight++ )
      {
        int width = currentStepWidth == nbStepWidth - 1 ? pixelWidth % maxWidth : maxWidth;
        int height = currentStepHeight == nbStepHeight - 1 ? pixelHeight % maxHeight : maxHeight;
        QgsRectangle extent;
        extent.setXMinimum( viewExtent.xMinimum() + viewExtent.width() / pixelWidth * ( currentStepWidth * maxWidth ) );
        extent.setXMaximum( viewExtent.xMinimum() + viewExtent.width() / pixelWidth * ( currentStepWidth * maxWidth + width ) );
        extent.setYMinimum( viewExtent.yMinimum() + viewExtent.height() / pixelHeight * ( currentStepHeight * maxHeight ) );
        extent.setYMaximum( viewExtent.yMinimum() + viewExtent.height() / pixelHeight * ( currentStepHeight * maxHeight + height ) );

        QUrl requestUrl( dataSource.param( QStringLiteral( "url" ) ) + ( mImageServer ? "/exportImage" : "/export" ) );
        QUrlQuery query( requestUrl );
        query.addQueryItem( QStringLiteral( "bbox" ), QStringLiteral( "%1,%2,%3,%4" ).arg( extent.xMinimum(), 0, 'f', -1 ).arg( extent.yMinimum(), 0, 'f', -1 ).arg( extent.xMaximum(), 0, 'f', -1 ).arg( extent.yMaximum(), 0, 'f', -1 ) );
        query.addQueryItem( QStringLiteral( "size" ), QStringLiteral( "%1,%2" ).arg( width ).arg( height ) );
        query.addQueryItem( QStringLiteral( "format" ), dataSource.param( QStringLiteral( "format" ) ) );
        query.addQueryItem( QStringLiteral( "layers" ), QStringLiteral( "show:%1" ).arg( dataSource.param( QStringLiteral( "layer" ) ) ) );
        query.addQueryItem( QStringLiteral( "transparent" ), QStringLiteral( "true" ) );
        query.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "image" ) );
        requestUrl.setQuery( query );
        mError.clear();
        mErrorTitle.clear();
        QString contentType;
        QByteArray reply = QgsArcGisRestQueryUtils::queryService( requestUrl, authcfg, mErrorTitle, mError, mRequestHeaders, feedback, &contentType );
        if ( !mError.isEmpty() )
        {
          p.end();
          mCachedImage = QImage();
          if ( feedback )
            feedback->appendError( QStringLiteral( "%1: %2" ).arg( mErrorTitle, mError ) );
          return QImage();
        }
        else if ( contentType.startsWith( QLatin1String( "application/json" ) ) )
        {
          // if we get a JSON response, something went wrong (e.g. authentication error)
          p.end();
          mCachedImage = QImage();

          QJsonParseError err;
          QJsonDocument doc = QJsonDocument::fromJson( reply, &err );
          if ( doc.isNull() )
          {
            mErrorTitle = QObject::tr( "Error" );
            mError = reply;
          }
          else
          {
            const QVariantMap res = doc.object().toVariantMap();
            if ( res.contains( QStringLiteral( "error" ) ) )
            {
              const QVariantMap error = res.value( QStringLiteral( "error" ) ).toMap();
              mError = error.value( QStringLiteral( "message" ) ).toString();
              mErrorTitle = QObject::tr( "Error %1" ).arg( error.value( QStringLiteral( "code" ) ).toString() );
            }
          }

          if ( feedback )
            feedback->appendError( QStringLiteral( "%1: %2" ).arg( mErrorTitle, mError ) );
          return QImage();
        }
        else
        {
          QImage img = QImage::fromData( reply, dataSource.param( QStringLiteral( "format" ) ).toLatin1() );
          p.drawImage( QPoint( currentStepWidth * maxWidth, currentStepHeight * maxHeight ), img );
        }
      }
    }
    p.end();
    return mCachedImage;
  }
}

QImage QgsAmsProvider::getLegendGraphic( double /*scale*/, bool forceRefresh, const QgsRectangle * /*visibleExtent*/ )
{
  if ( mLegendFetcher->haveImage() && !forceRefresh )
  {
    return mLegendFetcher->getImage();
  }
  mLegendFetcher->clear();
  QEventLoop evLoop;
  connect( mLegendFetcher, &QgsImageFetcher::finish, &evLoop, &QEventLoop::quit );
  connect( mLegendFetcher, &QgsImageFetcher::error, &evLoop, &QEventLoop::quit );
  mLegendFetcher->start();
  evLoop.exec( QEventLoop::ExcludeUserInputEvents );
  if ( !mLegendFetcher->errorTitle().isEmpty() )
  {
    mErrorTitle = mLegendFetcher->errorTitle();
    mError = mLegendFetcher->errorMessage();
    return QImage();
  }
  else
  {
    return mLegendFetcher->getImage();
  }
}

QgsImageFetcher *QgsAmsProvider::getLegendGraphicFetcher( const QgsMapSettings * /*mapSettings*/ )
{
  QgsAmsLegendFetcher *fetcher = new QgsAmsLegendFetcher( this, mLegendFetcher->getImage() );
  connect( fetcher, &QgsAmsLegendFetcher::fetchedNew, this, [ = ]( const QImage & fetched )
  {
    mLegendFetcher->setImage( fetched );
  } );
  return fetcher;
}

QgsRasterIdentifyResult QgsAmsProvider::identify( const QgsPointXY &point, Qgis::RasterIdentifyFormat format, const QgsRectangle &extent, int width, int height, int dpi )
{
  // http://resources.arcgis.com/en/help/rest/apiref/identify.html
  QgsDataSourceUri dataSource( dataSourceUri() );
  QUrl queryUrl( dataSource.param( QStringLiteral( "url" ) ) + "/identify" );
  QUrlQuery query( queryUrl );
  query.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "json" ) );
  query.addQueryItem( QStringLiteral( "geometryType" ), QStringLiteral( "esriGeometryPoint" ) );
  query.addQueryItem( QStringLiteral( "geometry" ), QStringLiteral( "{x: %1, y: %2}" ).arg( point.x(), 0, 'f' ).arg( point.y(), 0, 'f' ) );
//  query.addQueryItem( "sr", mCrs.postgisSrid() );
  query.addQueryItem( QStringLiteral( "layers" ), QStringLiteral( "all:%1" ).arg( dataSource.param( QStringLiteral( "layer" ) ) ) );
  query.addQueryItem( QStringLiteral( "imageDisplay" ), QStringLiteral( "%1,%2,%3" ).arg( width ).arg( height ).arg( dpi ) );
  query.addQueryItem( QStringLiteral( "mapExtent" ), QStringLiteral( "%1,%2,%3,%4" ).arg( extent.xMinimum(), 0, 'f' ).arg( extent.yMinimum(), 0, 'f' ).arg( extent.xMaximum(), 0, 'f' ).arg( extent.yMaximum(), 0, 'f' ) );
  query.addQueryItem( QStringLiteral( "tolerance" ), QStringLiteral( "10" ) );
  queryUrl.setQuery( query );

  const QString authcfg = dataSource.authConfigId();
  const QVariantList queryResults = QgsArcGisRestQueryUtils::queryServiceJSON( queryUrl, authcfg, mErrorTitle, mError ).value( QStringLiteral( "results" ) ).toList();

  QMap<int, QVariant> entries;

  if ( format == Qgis::RasterIdentifyFormat::Text )
  {
    for ( const QVariant &result : queryResults )
    {
      const QVariantMap resultMap = result.toMap();
      QVariantMap attributesMap = resultMap[QStringLiteral( "attributes" )].toMap();
      QString valueStr;
      for ( auto it = attributesMap.constBegin(); it != attributesMap.constEnd(); ++it )
      {
        valueStr += QStringLiteral( "%1 = %2\n" ).arg( it.key(), it.value().toString() );
      }
      entries.insert( entries.size(), valueStr );
    }
  }
  else if ( format == Qgis::RasterIdentifyFormat::Feature )
  {
    for ( const QVariant &result : queryResults )
    {
      const QVariantMap resultMap = result.toMap();

      QgsFields fields;
      const QVariantMap attributesMap = resultMap[QStringLiteral( "attributes" )].toMap();
      QgsAttributes featureAttributes;
      for ( auto it = attributesMap.constBegin(); it != attributesMap.constEnd(); ++it )
      {
        fields.append( QgsField( it.key(), QVariant::String, QStringLiteral( "string" ) ) );
        featureAttributes.append( it.value().toString() );
      }
      QgsCoordinateReferenceSystem crs;
      std::unique_ptr< QgsAbstractGeometry > geometry( QgsArcGisRestUtils::convertGeometry( resultMap[QStringLiteral( "geometry" )].toMap(), resultMap[QStringLiteral( "geometryType" )].toString(), false, false, &crs ) );
      QgsFeature feature( fields );
      feature.setGeometry( QgsGeometry( std::move( geometry ) ) );
      feature.setAttributes( featureAttributes );
      feature.setValid( true );
      QgsFeatureStore store( fields, crs );
      QMap<QString, QVariant> params;
      params[QStringLiteral( "sublayer" )] = resultMap[QStringLiteral( "layerName" )].toString();
      params[QStringLiteral( "featureType" )] = attributesMap[resultMap[QStringLiteral( "displayFieldName" )].toString()].toString();
      store.setParams( params );
      store.addFeature( feature );
      entries.insert( entries.size(), QVariant::fromValue( QgsFeatureStoreList() << store ) );
    }
  }
  return QgsRasterIdentifyResult( format, entries );
}

QList<double> QgsAmsProvider::nativeResolutions() const
{
  return mResolutions;
}

bool QgsAmsProvider::readBlock( int /*bandNo*/, const QgsRectangle &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback )
{
  // TODO: optimize to avoid writing to QImage
  QImage res = draw( viewExtent, width, height, feedback );
  if ( res.isNull() )
  {
    return false;
  }
  else if ( res.width() != width || res.height() != height )
  {
    const QString error = tr( "Unexpected image size for block. Expected %1x%2, got %3x%4" ).arg( width ).arg( height ).arg( res.width() ).arg( res.height() );
    if ( feedback )
      feedback->appendError( error );

    QgsDebugMsg( error );
    return false;
  }
  else
  {
    std::memcpy( data, res.constBits(), res.bytesPerLine() * res.height() );
    return true;
  }
}


//
// QgsAmsTiledImageDownloadHandler
//

QgsAmsTiledImageDownloadHandler::QgsAmsTiledImageDownloadHandler( const QString &auth,  const QgsHttpHeaders &requestHeaders, int tileReqNo, const QgsAmsProvider::TileRequests &requests, QImage *image, const QgsRectangle &viewExtent, QgsRasterBlockFeedback *feedback )
  : mAuth( auth )
  , mRequestHeaders( requestHeaders )
  , mImage( image )
  , mViewExtent( viewExtent )
  , mEventLoop( new QEventLoop )
  , mTileReqNo( tileReqNo )
  , mFeedback( feedback )
{
  if ( feedback )
  {
    connect( feedback, &QgsFeedback::canceled, this, &QgsAmsTiledImageDownloadHandler::canceled, Qt::QueuedConnection );

    // rendering could have been canceled before we started to listen to canceled() signal
    // so let's check before doing the download and maybe quit prematurely
    if ( feedback->isCanceled() )
      return;
  }

  for ( const QgsAmsProvider::TileRequest &r : requests )
  {
    QNetworkRequest request( r.url );
    QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsAmsTiledImageDownloadHandler" ) );
    QgsSetRequestInitiatorId( request, QString::number( r.index ) );
    mRequestHeaders.updateNetworkRequest( request );
    if ( !mAuth.isEmpty() && !QgsApplication::authManager()->updateNetworkRequest( request, mAuth ) )
    {
      const QString error = tr( "network request update failed for authentication config" );
      // mErrors.append( error );
      QgsMessageLog::logMessage( error, tr( "Network" ) );
      continue;
    }
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ), mTileReqNo );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileIndex ), r.index );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRect ), r.rect );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), 0 );

    QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( request );
    connect( reply, &QNetworkReply::finished, this, &QgsAmsTiledImageDownloadHandler::tileReplyFinished );

    mReplies << reply;
  }
}

QgsAmsTiledImageDownloadHandler::~QgsAmsTiledImageDownloadHandler()
{
  delete mEventLoop;
}

void QgsAmsTiledImageDownloadHandler::downloadBlocking()
{
  if ( mFeedback && mFeedback->isCanceled() )
    return; // nothing to do

  mEventLoop->exec( QEventLoop::ExcludeUserInputEvents );

  Q_ASSERT( mReplies.isEmpty() );
}


void QgsAmsTiledImageDownloadHandler::tileReplyFinished()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );

  if ( QgsNetworkAccessManager::instance()->cache() )
  {
    QNetworkCacheMetaData cmd = QgsNetworkAccessManager::instance()->cache()->metaData( reply->request().url() );

    QNetworkCacheMetaData::RawHeaderList hl;
    const auto constRawHeaders = cmd.rawHeaders();
    for ( const QNetworkCacheMetaData::RawHeader &h : constRawHeaders )
    {
      if ( h.first != "Cache-Control" )
        hl.append( h );
    }
    cmd.setRawHeaders( hl );

    QgsDebugMsgLevel( QStringLiteral( "expirationDate:%1" ).arg( cmd.expirationDate().toString() ), 2 );
    if ( cmd.expirationDate().isNull() )
    {
      QgsSettings s;
      cmd.setExpirationDate( QDateTime::currentDateTime().addSecs( s.value( QStringLiteral( "qgis/defaultTileExpiry" ), "24" ).toInt() * 60 * 60 ) );
    }

    QgsNetworkAccessManager::instance()->cache()->updateMetaData( cmd );
  }

  int tileReqNo = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ) ).toInt();
  int tileNo = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( TileIndex ) ).toInt();
  QRectF r = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( TileRect ) ).toRectF();

  if ( reply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !QgsVariantUtils::isNull( redirect ) )
    {
      QNetworkRequest request( redirect.toUrl() );
      QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsAmsTiledImageDownloadHandler" ) );
      QgsSetRequestInitiatorId( request, QString::number( tileReqNo ) );
      mRequestHeaders.updateNetworkRequest( request );
      if ( !mAuth.isEmpty() && !QgsApplication::authManager()->updateNetworkRequest( request, mAuth ) )
      {
        const QString error = tr( "network request update failed for authentication config" );
        // mErrors.append( error );
        QgsMessageLog::logMessage( error, tr( "Network" ) );
      }
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
      request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ), tileReqNo );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileIndex ), tileNo );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRect ), r );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), 0 );

      mReplies.removeOne( reply );
      reply->deleteLater();

      QgsDebugMsgLevel( QStringLiteral( "redirected gettile: %1" ).arg( redirect.toString() ), 2 );
      reply = QgsNetworkAccessManager::instance()->get( request );
      mReplies << reply;

      connect( reply, &QNetworkReply::finished, this, &QgsAmsTiledImageDownloadHandler::tileReplyFinished );

      return;
    }

    QVariant status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !QgsVariantUtils::isNull( status ) && status.toInt() >= 400 )
    {
      mReplies.removeOne( reply );
      reply->deleteLater();

      if ( mReplies.isEmpty() )
        finish();

      return;
    }

    QString contentType = reply->header( QNetworkRequest::ContentTypeHeader ).toString();
    QgsDebugMsgLevel( "contentType: " + contentType, 2 );
    if ( !contentType.isEmpty() && !contentType.startsWith( QLatin1String( "image/" ), Qt::CaseInsensitive ) &&
         contentType.compare( QLatin1String( "application/octet-stream" ), Qt::CaseInsensitive ) != 0 )
    {
      QByteArray text = reply->readAll();
      QString errorTitle, errorText;
      if ( contentType.startsWith( QLatin1String( "application/json" ), Qt::CaseInsensitive ) )
      {
        // if we get a JSON response, something went wrong (e.g. authentication error)
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson( text, &err );
        if ( doc.isNull() )
        {
          errorTitle = QObject::tr( "Error" );
          errorText = text;
        }
        else
        {
          const QVariantMap res = doc.object().toVariantMap();
          if ( res.contains( QStringLiteral( "error" ) ) )
          {
            const QVariantMap error = res.value( QStringLiteral( "error" ) ).toMap();
            errorText = error.value( QStringLiteral( "message" ) ).toString();
            errorTitle = QObject::tr( "Error %1" ).arg( error.value( QStringLiteral( "code" ) ).toString() );
          }
        }

        if ( mFeedback )
          mFeedback->appendError( QStringLiteral( "%1: %2" ).arg( errorTitle, errorText ) );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Tile request error (Status: %1; Content-Type: %2; Length: %3; URL: %4)" )
                                   .arg( status.toString(),
                                         contentType )
                                   .arg( text.size() )
                                   .arg( reply->url().toString() ), tr( "WMS" ) );
#ifdef QGISDEBUG
        QFile file( QDir::tempPath() + "/broken-image.png" );
        if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
        {
          file.write( text );
          file.close();
        }
#endif
      }

      mReplies.removeOne( reply );
      reply->deleteLater();

      if ( mReplies.isEmpty() )
        finish();

      return;
    }

    // only take results from current request number
    if ( mTileReqNo == tileReqNo )
    {
      QgsDebugMsgLevel( QStringLiteral( "tile reply: length %1" ).arg( reply->bytesAvailable() ), 2 );

      QImage myLocalImage = QImage::fromData( reply->readAll() );

      if ( !myLocalImage.isNull() )
      {
        QPainter p( mImage );
        // if image size is "close enough" to destination size, don't smooth it out. Instead try for pixel-perfect placement!
        const bool disableSmoothing = ( qgsDoubleNear( r.width(), myLocalImage.width(), 2 ) && qgsDoubleNear( r.height(), myLocalImage.height(), 2 ) );
        if ( !disableSmoothing )
          p.setRenderHint( QPainter::SmoothPixmapTransform, true );
        p.drawImage( r, myLocalImage );
        p.end();

        QgsTileCache::insertTile( reply->url(), myLocalImage );

        if ( mFeedback )
          mFeedback->onNewData();
      }
      else
      {
        QString errorText = tr( "Returned image is flawed [Content-Type: %1; URL: %2]" )
                            .arg( contentType, reply->url().toString() );
        if ( mFeedback )
          mFeedback->appendError( errorText );
      }
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "Reply too late [%1]" ).arg( reply->url().toString() ), 2 );
    }

    mReplies.removeOne( reply );
    reply->deleteLater();

    if ( mReplies.isEmpty() )
      finish();

  }
  else
  {
    if ( !( mFeedback && mFeedback->isPreviewOnly() ) )
    {
      // report any errors except for the one we have caused by canceling the request
      if ( reply->error() != QNetworkReply::OperationCanceledError )
      {
        // if we reached timeout, let's try again (e.g. in case of slow connection or slow server)
        if ( reply->error() == QNetworkReply::TimeoutError )
          repeatTileRequest( reply->request() );
      }
    }

    mReplies.removeOne( reply );
    reply->deleteLater();

    if ( mReplies.isEmpty() )
      finish();
  }
}

void QgsAmsTiledImageDownloadHandler::canceled()
{
  QgsDebugMsgLevel( QStringLiteral( "Caught canceled() signal" ), 2 );
  const auto constMReplies = mReplies;
  for ( QNetworkReply *reply : constMReplies )
  {
    QgsDebugMsgLevel( QStringLiteral( "Aborting tiled network request" ), 2 );
    reply->abort();
  }
}


void QgsAmsTiledImageDownloadHandler::repeatTileRequest( QNetworkRequest const &oldRequest )
{
  QNetworkRequest request( oldRequest );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsAmsTiledImageDownloadHandler" ) );

  QString url = request.url().toString();
#ifdef QGISDEBUG
  int tileReqNo = request.attribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ) ).toInt();
  int tileNo = request.attribute( static_cast<QNetworkRequest::Attribute>( TileIndex ) ).toInt();
#endif
  int retry = request.attribute( static_cast<QNetworkRequest::Attribute>( TileRetry ) ).toInt();
  retry++;

  QgsSettings s;
  int maxRetry = s.value( QStringLiteral( "qgis/defaultTileMaxRetry" ), "3" ).toInt();
  if ( retry > maxRetry )
  {
    return;
  }

  mRequestHeaders.updateNetworkRequest( request );
  if ( !mAuth.isEmpty() && !QgsApplication::authManager()->updateNetworkRequest( request, mAuth ) )
  {
    const QString error = tr( "network request update failed for authentication config" );
    // mErrors.append( error );
    QgsMessageLog::logMessage( error, tr( "Network" ) );
    return;
  }
  QgsDebugMsgLevel( QStringLiteral( "repeat tileRequest %1 %2(retry %3) for url: %4" ).arg( tileReqNo ).arg( tileNo ).arg( retry ).arg( url ), 2 );
  request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), retry );

  QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( request );
  mReplies << reply;
  connect( reply, &QNetworkReply::finished, this, &QgsAmsTiledImageDownloadHandler::tileReplyFinished );
}



QgsAmsProviderMetadata::QgsAmsProviderMetadata()
  : QgsProviderMetadata( QgsAmsProvider::AMS_PROVIDER_KEY, QgsAmsProvider::AMS_PROVIDER_DESCRIPTION )
{
}

QIcon QgsAmsProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconAms.svg" ) );
}

QgsAmsProvider *QgsAmsProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsAmsProvider( uri, options, flags );
}

QVariantMap QgsAmsProviderMetadata::decodeUri( const QString &uri ) const
{
  QgsDataSourceUri dsUri = QgsDataSourceUri( uri );

  QVariantMap components;
  components.insert( QStringLiteral( "url" ), dsUri.param( QStringLiteral( "url" ) ) );

  dsUri.httpHeaders().updateMap( components );

  if ( !dsUri.param( QStringLiteral( "crs" ) ).isEmpty() )
  {
    components.insert( QStringLiteral( "crs" ), dsUri.param( QStringLiteral( "crs" ) ) );
  }
  if ( !dsUri.authConfigId().isEmpty() )
  {
    components.insert( QStringLiteral( "authcfg" ), dsUri.authConfigId() );
  }
  if ( !dsUri.param( QStringLiteral( "format" ) ).isEmpty() )
  {
    components.insert( QStringLiteral( "format" ), dsUri.param( QStringLiteral( "format" ) ) );
  }
  if ( !dsUri.param( QStringLiteral( "layer" ) ).isEmpty() )
  {
    components.insert( QStringLiteral( "layer" ), dsUri.param( QStringLiteral( "layer" ) ) );
  }

  return components;
}

QString QgsAmsProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setParam( QStringLiteral( "url" ), parts.value( QStringLiteral( "url" ) ).toString() );

  if ( !parts.value( QStringLiteral( "crs" ) ).toString().isEmpty() )
  {
    dsUri.setParam( QStringLiteral( "crs" ), parts.value( QStringLiteral( "crs" ) ).toString() );
  }

  dsUri.httpHeaders().setFromMap( parts );

  if ( !parts.value( QStringLiteral( "authcfg" ) ).toString().isEmpty() )
  {
    dsUri.setAuthConfigId( parts.value( QStringLiteral( "authcfg" ) ).toString() );
  }
  if ( !parts.value( QStringLiteral( "format" ) ).toString().isEmpty() )
  {
    dsUri.setParam( QStringLiteral( "format" ), parts.value( QStringLiteral( "format" ) ).toString() );
  }
  if ( !parts.value( QStringLiteral( "layer" ) ).toString().isEmpty() )
  {
    dsUri.setParam( QStringLiteral( "layer" ), parts.value( QStringLiteral( "layer" ) ).toString() );
  }

  return dsUri.uri( false );
}

QList<QgsMapLayerType> QgsAmsProviderMetadata::supportedLayerTypes() const
{
  return { QgsMapLayerType::RasterLayer };
}

#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsAmsProviderMetadata();
}
#endif
