/***************************************************************************
  qgswmsprovider.cpp  -  QGIS Data provider for
                         OGC Web Map Service layers
                             -------------------
    begin                : 17 Mar, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au

    wms-c/wmts support   : Jürgen E. Fischer < jef at norbit dot de >, norBIT GmbH

    tile retry support   : Luigi Pirelli < luipir at gmail dot com >
                           (funded by Regione Toscana-SITA)

    contextual wms legend: Sandro Santilli < strk at keybit dot net >

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswmsprovider.h"

#include <ogr_api.h>

#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsdatasourceuri.h"
#include "qgsexception.h"
#include "qgsfeaturestore.h"
#include "qgsgdalutils.h"
#include "qgsgeometry.h"
#include "qgsgml.h"
#include "qgsgmlschema.h"
#include "qgslogger.h"
#include "qgsmapsettings.h"
#include "qgsmbtiles.h"
#include "qgsmessagelog.h"
#include "qgsmessageoutput.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsnetworkreplyparser.h"
#include "qgsogrutils.h"
#include "qgsproviderregistry.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrectangle.h"
#include "qgsruntimeprofiler.h"
#include "qgssetrequestinitiator_p.h"
#include "qgssettings.h"
#include "qgssettingsregistrycore.h"
#include "qgstilecache.h"
#include "qgstiledownloadmanager.h"
#include "qgswmscapabilities.h"
#include "qgswmsdataitems.h"

#include <QEventLoop>
#include <QImage>
#include <QImageReader>
#include <QJsonArray>
#include <QNetworkDiskCache>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringBuilder>
#include <QTextCodec>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

#include "moc_qgswmsprovider.cpp"

#ifdef QGISDEBUG
#include <QFile>
#include <QDir>
#include <memory>
#endif

#define ERR( message ) QGS_ERROR_MESSAGE( message, "WMS provider" )
#define QGS_ERROR( message ) QgsError( message, "WMS provider" )

QString QgsWmsProvider::WMS_KEY = u"wms"_s;
QString QgsWmsProvider::WMS_DESCRIPTION = u"OGC Web Map Service version 1.3 data provider"_s;

QMap<QString, QgsWmsStatistics::Stat> QgsWmsStatistics::sData;

//! a helper class for ordering tile requests according to the distance from view center
struct LessThanTileRequest
{
    QgsPointXY center;
    bool operator()( const QgsWmsProvider::TileRequest &req1, const QgsWmsProvider::TileRequest &req2 )
    {
      QPointF p1 = req1.rect.center();
      QPointF p2 = req2.rect.center();
      // using chessboard distance (loading order more natural than euclidean/manhattan distance)
      double d1 = std::max( std::fabs( center.x() - p1.x() ), std::fabs( center.y() - p1.y() ) );
      double d2 = std::max( std::fabs( center.x() - p2.x() ), std::fabs( center.y() - p2.y() ) );
      return d1 < d2;
    }
};


QgsWmsProvider::QgsWmsProvider( QString const &uri, const ProviderOptions &options, const QgsWmsCapabilities *capabilities )
  : QgsRasterDataProvider( uri, options )
  , mHttpGetLegendGraphicResponse( nullptr )
  , mImageCrs( DEFAULT_LATLON_CRS )
{
  QgsDebugMsgLevel( "constructing with uri '" + uri + "'.", 4 );

  mSupportedGetFeatureFormats = QStringList() << u"text/html"_s << u"text/plain"_s << u"text/xml"_s << u"application/vnd.ogc.gml"_s << u"application/json"_s;

  mValid = false;

  // URL may contain username/password information for a WMS
  // requiring authentication. In this case the URL is prefixed
  // with username=user,password=pass,url=http://xxx.xxx.xx/yyy...

  if ( !mSettings.parseUri( uri ) )
  {
    appendError( ERR( tr( "Cannot parse URI" ) ) );
    return;
  }

  if ( !addLayers() )
    return;

  std::unique_ptr<QgsScopedRuntimeProfile> profile;

  if ( mSettings.mIsMBTiles )
  {
    if ( QgsApplication::profiler()->groupIsActive( u"projectload"_s ) )
      profile = std::make_unique<QgsScopedRuntimeProfile>( tr( "Setup tile capabilities" ), u"projectload"_s );

    // we are dealing with a local MBTiles file
    if ( !setupMBTilesCapabilities( uri ) )
    {
      appendError( ERR( tr( "Cannot open MBTiles database" ) ) );
      return;
    }
  }
  else if ( mSettings.mXyz )
  {
    if ( QgsApplication::profiler()->groupIsActive( u"projectload"_s ) )
      profile = std::make_unique<QgsScopedRuntimeProfile>( tr( "Setup tile capabilities" ), u"projectload"_s );

    // we are working with XYZ tiles
    // no need to get capabilities, the whole definition is in URI
    // so we just generate a dummy WMTS definition
    if ( !setupXyzCapabilities( uri ) )
    {
      return;
    }
  }
  else
  {
    // we are working with WMS / WMTS server

    // if there are already parsed capabilities, use them!
    if ( capabilities )
      mCaps = *capabilities;

    if ( QgsApplication::profiler()->groupIsActive( u"projectload"_s ) )
      profile = std::make_unique<QgsScopedRuntimeProfile>( tr( "Retrieve server capabilities" ), u"projectload"_s );

    // Make sure we have capabilities - other functions here may need them
    if ( !retrieveServerCapabilities() )
    {
      return;
    }

    // Setup temporal properties for layers in WMS-T
    if ( mSettings.mIsTemporal )
    {
      QgsRasterDataProviderTemporalCapabilities *lTemporalCapabilities = temporalCapabilities();
      Q_ASSERT_X( lTemporalCapabilities, "QgsWmsProvider::QgsWmsProvider()", "Data provider temporal capabilities object does not exist" );
      lTemporalCapabilities->setHasTemporalCapabilities( true );
      lTemporalCapabilities->setAvailableTemporalRange( mSettings.mFixedRange );
      lTemporalCapabilities->setAllAvailableTemporalRanges( mSettings.mAllRanges );
      lTemporalCapabilities->setDefaultInterval( mSettings.mDefaultInterval );

      lTemporalCapabilities->setIntervalHandlingMethod( Qgis::TemporalIntervalMatchMethod::MatchExactUsingStartOfRange );

      if ( mSettings.mIsBiTemporal )
      {
        lTemporalCapabilities->setAvailableReferenceTemporalRange( mSettings.mFixedReferenceRange );
      }
    }

    if ( mSettings.mCrsId.isEmpty() && !mSettings.mActiveSubLayers.empty() )
    {
      // if crs not specified via layer uri, use the first available from server capabilities
      for ( const QgsWmsLayerProperty &property : std::as_const( mCaps.mLayersSupported ) )
      {
        if ( property.name == mSettings.mActiveSubLayers[0] )
        {
          mSettings.mCrsId = property.preferredAvailableCrs();
          break;
        }
      }
    }
  }

  // setImageCrs is using mTiled !!!
  if ( !setImageCrs( mSettings.mCrsId ) )
  {
    appendError( ERR( tr( "Cannot set CRS" ) ) );
    return;
  }
  mCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mSettings.mCrsId );

  if ( mSettings.mTiled )
  {
    // WMTS - may have time dimension
    if ( !mTileLayer->allTimeRanges.empty() )
    {
      QgsRasterDataProviderTemporalCapabilities *lTemporalCapabilities = temporalCapabilities();
      Q_ASSERT_X( lTemporalCapabilities, "QgsWmsProvider::QgsWmsProvider()", "Data provider temporal capabilities object does not exist" );
      lTemporalCapabilities->setHasTemporalCapabilities( true );

      lTemporalCapabilities->setAvailableTemporalRange( mTileLayer->temporalExtent );
      lTemporalCapabilities->setAllAvailableTemporalRanges( mTileLayer->allTimeRanges );
      lTemporalCapabilities->setDefaultInterval( mTileLayer->temporalInterval );
      lTemporalCapabilities->setFlags( mTileLayer->temporalCapabilityFlags );
      lTemporalCapabilities->setIntervalHandlingMethod( Qgis::TemporalIntervalMatchMethod::FindClosestMatchToStartOfRange );
    }

    if ( !mSettings.mXyz )
    {
      switch ( mSettings.mTilePixelRatio )
      {
        case Qgis::TilePixelRatio::Undefined:
          mTileLayer->dpi = -1;
          break;
        case Qgis::TilePixelRatio::StandardDpi:
          mTileLayer->dpi = 96;
          break;
        case Qgis::TilePixelRatio::HighDpi:
          mTileLayer->dpi = 192;
          break;
      }
    }
  }

  if ( profile )
    profile->switchTask( tr( "Calculate extent" ) );

  if ( !calculateExtent() || mLayerExtent.isEmpty() )
  {
    appendError( ERR( tr( "Cannot calculate extent" ) ) );
    return;
  }

  // URL can be in 3 forms:
  // 1) http://xxx.xxx.xx/yyy/yyy
  // 2) http://xxx.xxx.xx/yyy/yyy?
  // 3) http://xxx.xxx.xx/yyy/yyy?zzz=www


  mConverter = QgsWmsInterpretationConverter::createConverter( mSettings.mInterpretation );
  if ( mConverter && mConverter->representsElevation() )
  {
    elevationProperties()->setContainsElevationData( true );
  }

  mValid = true;
  QgsDebugMsgLevel( u"exiting constructor."_s, 4 );
}


QString QgsWmsProvider::prepareUri( QString uri )
{
  // some services provide a percent/url encoded (legend) uri string, always decode here
  uri = QUrl::fromPercentEncoding( uri.toUtf8() );

  if ( isUrlForWMTS( uri ) )
  {
    return uri;
  }

  if ( !uri.contains( "?"_L1 ) )
  {
    uri.append( '?' );
  }
  else if ( uri.right( 1 ) != "?"_L1 && uri.right( 1 ) != "&"_L1 )
  {
    uri.append( '&' );
  }

  return uri;
}

QgsWmsProvider::~QgsWmsProvider()
{
  QgsDebugMsgLevel( u"deconstructing."_s, 4 );
}

//! Returns the destination extent in image coordinate of a tile image defined by its extent
static QRect destinationRect( const QgsRectangle &destinationExtent, const QRectF &tileImageExtent, int imagePixelWidth )
{
  const double mapUnitsPerPixel = destinationExtent.width() / imagePixelWidth;

  // note -- we round to exact pixel boundaries here, as we know that we'll always be drawing the
  // tiles using a painter with a pixel-based image device. We always expand out the rect to the nearest
  // pixel (instead of shrinking), in order to avoid any chance of visible gaps between tiles

  const int left = static_cast< int >( std::floor( ( tileImageExtent.left() - destinationExtent.xMinimum() ) / mapUnitsPerPixel ) );
  const int right = static_cast< int >( std::ceil( ( tileImageExtent.right() - destinationExtent.xMinimum() ) / mapUnitsPerPixel ) );
  const int top = static_cast< int >( std::floor( ( destinationExtent.yMaximum() - tileImageExtent.bottom() ) / mapUnitsPerPixel ) );
  const int bottom = static_cast< int >( std::ceil( ( destinationExtent.yMaximum() - tileImageExtent.top() ) / mapUnitsPerPixel ) );

  return QRect( left, top, right - left, bottom - top );
}

QgsWmsProvider *QgsWmsProvider::clone() const
{
  QgsDataProvider::ProviderOptions options;
  QgsWmsProvider *provider = new QgsWmsProvider( dataSourceUri(), options, mCaps.isValid() ? &mCaps : nullptr );
  provider->copyBaseSettings( *this );
  return provider;
}

QString QgsWmsProvider::getMapUrl() const
{
  return mCaps.mCapabilities.capability.request.getMap.dcpType.isEmpty()
           ? mSettings.mBaseUrl
           : prepareUri( mCaps.mCapabilities.capability.request.getMap.dcpType.front().http.get.onlineResource.xlinkHref );
}


QString QgsWmsProvider::getFeatureInfoUrl() const
{
  return mCaps.mCapabilities.capability.request.getFeatureInfo.dcpType.isEmpty()
           ? mSettings.mBaseUrl
           : prepareUri( mCaps.mCapabilities.capability.request.getFeatureInfo.dcpType.front().http.get.onlineResource.xlinkHref );
}

QString QgsWmsProvider::getTileUrl() const
{
  if ( mCaps.mCapabilities.capability.request.getTile.dcpType.isEmpty() || ( !mCaps.mCapabilities.capability.request.getTile.allowedEncodings.isEmpty() && !mCaps.mCapabilities.capability.request.getTile.allowedEncodings.contains( u"KVP"_s ) ) )
  {
    return QString();
  }
  else
  {
    return prepareUri( mCaps.mCapabilities.capability.request.getTile.dcpType.front().http.get.onlineResource.xlinkHref );
  }
}

static bool isValidLegend( const QgsWmsLegendUrlProperty &l )
{
  return l.format.startsWith( "image/"_L1 );
}

/**
 * Picks a usable legend URL for a given style.
 */
static QString pickLegend( const QgsWmsStyleProperty &s )
{
  QString url;
  for ( int k = 0; k < s.legendUrl.size() && url.isEmpty(); k++ )
  {
    const QgsWmsLegendUrlProperty &l = s.legendUrl[k];
    if ( isValidLegend( l ) )
    {
      url = l.onlineResource.xlinkHref;
    }
  }
  return url;
}

static const QgsWmsStyleProperty *searchStyle( const QVector<QgsWmsStyleProperty> &styles, const QString &name )
{
  for ( const QgsWmsStyleProperty &s : styles )
    if ( s.name == name )
      return &s;
  return nullptr;
}

QString QgsWmsProvider::getLegendGraphicUrl() const
{
  QString url;

  for ( int i = 0; i < mCaps.mLayersSupported.size() && url.isEmpty(); i++ )
  {
    const QgsWmsLayerProperty &l = mCaps.mLayersSupported[i];

    if ( l.name == mSettings.mActiveSubLayers[0] )
    {
      if ( !mSettings.mActiveSubStyles[0].isEmpty() && mSettings.mActiveSubStyles[0] != "default"_L1 )
      {
        const QgsWmsStyleProperty *s = searchStyle( l.style, mSettings.mActiveSubStyles[0] );
        if ( s )
          url = pickLegend( *s );
      }
      else
      {
        // QGIS wants the default style, but GetCapabilities doesn't give us a
        // way to know what is the default style. So we look for the onlineResource
        // only if there is a single style available or if there is a style called "default".
        if ( l.style.size() == 1 )
        {
          url = pickLegend( l.style[0] );
        }
        else
        {
          const QgsWmsStyleProperty *s = searchStyle( l.style, u"default"_s );
          if ( s )
            url = pickLegend( *s );
        }
      }
      break;
    }
  }

  if ( url.isEmpty() && !mCaps.mCapabilities.capability.request.getLegendGraphic.dcpType.isEmpty() )
  {
    url = mCaps.mCapabilities.capability.request.getLegendGraphic.dcpType.front().http.get.onlineResource.xlinkHref;
  }

  if ( url.isEmpty() )
  {
    for ( const QgsWmtsTileLayer &l : mCaps.mTileLayersSupported )
    {
      if ( l.identifier != mSettings.mActiveSubLayers[0] )
        continue;

      QHash<QString, QgsWmtsStyle>::const_iterator it = l.styles.constFind( mSettings.mActiveSubStyles[0] );
      if ( it == l.styles.constEnd() )
        if ( it == l.styles.end() )
          continue;

      for ( const QgsWmtsLegendURL &u : it.value().legendURLs )
      {
        if ( u.format == mSettings.mImageMimeType )
          url = u.href;
      }
      if ( url.isEmpty() && !it.value().legendURLs.isEmpty() )
        url = it.value().legendURLs.front().href;

      if ( !url.isEmpty() )
        break;
    }
  }

  return url.isEmpty() ? url : prepareUri( url );
}

bool QgsWmsProvider::addLayers()
{
  QgsDebugMsgLevel( "Entering: layers:" + mSettings.mActiveSubLayers.join( ", " ) + ", styles:" + mSettings.mActiveSubStyles.join( ", " ), 4 );

  if ( mSettings.mActiveSubLayers.size() != mSettings.mActiveSubStyles.size() )
  {
    QgsMessageLog::logMessage( tr( "Number of layers and styles don't match" ), tr( "WMS" ) );
    return false;
  }

  // Set the visibility of these new layers on by default
  for ( const QString &layer : std::as_const( mSettings.mActiveSubLayers ) )
  {
    mActiveSubLayerVisibility[layer] = true;
    QgsDebugMsgLevel( u"set visibility of layer '%1' to true."_s.arg( layer ), 3 );
  }

  // now that the layers have changed, the extent will as well.
  mExtentDirty = true;

  if ( mSettings.mTiled )
    mTileLayer = nullptr;

  QgsDebugMsgLevel( u"Exiting."_s, 4 );

  return true;
}

void QgsWmsProvider::setConnectionName( QString const &connName )
{
  mConnectionName = connName;
}

Qgis::DataProviderFlags QgsWmsProvider::flags() const
{
  Qgis::DataProviderFlags res;
  if ( mSettings.mXyz )
  {
    // always consider XYZ tiles as basemap sources
    res |= Qgis::DataProviderFlag::IsBasemapSource;
  }
  return res;
}

void QgsWmsProvider::setLayerOrder( QStringList const &layers )
{
  if ( layers.size() != mSettings.mActiveSubLayers.size() )
  {
    QgsDebugError( u"Invalid layer list length"_s );
    return;
  }

  QMap<QString, QString> styleMap;
  for ( int i = 0; i < mSettings.mActiveSubLayers.size(); i++ )
  {
    styleMap.insert( mSettings.mActiveSubLayers[i], mSettings.mActiveSubStyles[i] );
  }

  for ( int i = 0; i < layers.size(); i++ )
  {
    if ( !styleMap.contains( layers[i] ) )
    {
      QgsDebugError( u"Layer %1 not found"_s.arg( layers[i] ) );
      return;
    }
  }

  mSettings.mActiveSubLayers = layers;
  mSettings.mActiveSubStyles.clear();
  for ( int i = 0; i < layers.size(); i++ )
  {
    mSettings.mActiveSubStyles.append( styleMap[layers[i]] );
  }
}


void QgsWmsProvider::setSubLayerVisibility( QString const &name, bool vis )
{
  if ( !mActiveSubLayerVisibility.contains( name ) )
  {
    QgsDebugError( u"Layer %1 not found."_s.arg( name ) );
    return;
  }

  mActiveSubLayerVisibility[name] = vis;
}


bool QgsWmsProvider::setImageCrs( QString const &crs )
{
  QgsDebugMsgLevel( "Setting image CRS to " + crs + '.', 3 );

  if ( crs != mImageCrs && !crs.isEmpty() )
  {
    mExtentDirty = true;
    mImageCrs = crs;
  }

  if ( mSettings.mTiled )
  {
    if ( mSettings.mActiveSubLayers.size() != 1 )
    {
      appendError( ERR( tr( "Number of tile layers must be one" ) ) );
      return false;
    }

    QgsDebugMsgLevel( u"mTileLayersSupported.size() = %1"_s.arg( mCaps.mTileLayersSupported.size() ), 2 );
    if ( mCaps.mTileLayersSupported.isEmpty() )
    {
      appendError( ERR( tr( "Tile layer not found" ) ) );
      return false;
    }

    for ( int i = 0; i < mCaps.mTileLayersSupported.size(); i++ )
    {
      QgsWmtsTileLayer *tl = &mCaps.mTileLayersSupported[i];

      if ( tl->identifier != mSettings.mActiveSubLayers[0] )
        continue;

      if ( mSettings.mTileMatrixSetId.isEmpty() && tl->setLinks.size() == 1 )
      {
        QString tms = tl->setLinks.constBegin().key();

        if ( !mCaps.mTileMatrixSets.contains( tms ) )
        {
          QgsDebugError( u"tile matrix set '%1' not found."_s.arg( tms ) );
          continue;
        }

        if ( mCaps.mTileMatrixSets[tms].crs != mImageCrs )
        {
          QgsDebugError( u"tile matrix set '%1' has crs %2 instead of %3."_s.arg( tms, mCaps.mTileMatrixSets[tms].crs, mImageCrs ) );
          continue;
        }

        // fill in generate matrix for WMS-C
        mSettings.mTileMatrixSetId = tms;
      }

      mTileLayer = tl;
      break;
    }

    mNativeResolutions.clear();

    if ( mSettings.mTileMatrixSetId.isEmpty() && !mCaps.mFirstTileMatrixSetId.isEmpty() )
    {
      // if no explicit tile matrix set specified, use first listed
      mSettings.mTileMatrixSetId = mCaps.mFirstTileMatrixSetId;
      for ( int i = 0; i < mCaps.mTileLayersSupported.size(); i++ )
      {
        QgsWmtsTileLayer *tl = &mCaps.mTileLayersSupported[i];
        if ( tl->identifier != mSettings.mActiveSubLayers[0] )
          continue;

        mTileLayer = tl;
        break;
      }
    }

    if ( mCaps.mTileMatrixSets.contains( mSettings.mTileMatrixSetId ) )
    {
      mTileMatrixSet = &mCaps.mTileMatrixSets[mSettings.mTileMatrixSetId];

      if ( crs.isEmpty() )
      {
        // if CRS is not specified, use default
        mSettings.mCrsId = mTileMatrixSet->crs;
        mExtentDirty = true;
        mImageCrs = mSettings.mCrsId;
      }
      if ( mSettings.mImageMimeType.isEmpty() && mTileLayer )
      {
        // if format is not specified, use first available
        mSettings.mImageMimeType = mTileLayer->formats.value( 0 );
      }

      QList<double> keys = mTileMatrixSet->tileMatrices.keys();
      std::sort( keys.begin(), keys.end() );
      for ( double key : std::as_const( keys ) )
      {
        mNativeResolutions << key;
      }
      if ( !mTileMatrixSet->tileMatrices.empty() )
      {
        setProperty( "tileWidth", mTileMatrixSet->tileMatrices.first().tileWidth );
        setProperty( "tileHeight", mTileMatrixSet->tileMatrices.first().tileHeight );
      }
    }
    else
    {
      QgsDebugError( u"Expected tile matrix set '%1' not found."_s.arg( mSettings.mTileMatrixSetId ) );
      mTileMatrixSet = nullptr;
    }

    if ( !mTileLayer || !mTileMatrixSet )
    {
      appendError( ERR( tr( "Tile layer or tile matrix set not found" ) ) );
      return false;
    }
  }
  return true;
}

void QgsWmsProvider::setQueryItem( QUrlQuery &url, const QString &item, const QString &value )
{
  QString key = QUrl::toPercentEncoding( item );
  url.removeQueryItem( key );
  if ( value.isNull() )
    url.addQueryItem( key, "" );
  else
    url.addQueryItem( key, QUrl::toPercentEncoding( value ) );
}

void QgsWmsProvider::setFormatQueryItem( QUrlQuery &url )
{
  url.removeQueryItem( u"FORMAT"_s );
  if ( mSettings.mImageMimeType.contains( '+' ) )
  {
    QString format( mSettings.mImageMimeType );
    format.replace( '+', "%2b"_L1 );
    url.addQueryItem( "FORMAT", format );
  }
  else
    setQueryItem( url, u"FORMAT"_s, mSettings.mImageMimeType );
}


static bool _fuzzyContainsRect( const QRectF &r1, const QRectF &r2 )
{
  double significantDigits = std::log10( std::max( r1.width(), r1.height() ) );
  double epsilon = std::pow( 10.0, significantDigits - 5 ); // floats have 6-9 significant digits
  return r1.contains( r2.adjusted( epsilon, epsilon, -epsilon, -epsilon ) );
}

void QgsWmsProvider::fetchOtherResTiles( QgsTileMode tileMode, const QgsRectangle &viewExtent, int imageWidth, QList<QRectF> &missingRects, double tres, int resOffset, QList<TileImage> &otherResTiles, QgsRasterBlockFeedback *feedback )
{
  if ( !mTileMatrixSet )
    return; // there is no tile matrix set defined for ordinary WMS (with user-specified tile size)

  const QgsWmtsTileMatrix *tmOther = mTileMatrixSet->findOtherResolution( tres, resOffset );
  if ( !tmOther )
    return;

  QSet<TilePosition> tilesSet;
  for ( const QRectF &missingTileRect : std::as_const( missingRects ) )
  {
    int c0, r0, c1, r1;
    tmOther->viewExtentIntersection( QgsRectangle( missingTileRect ), nullptr, c0, r0, c1, r1 );

    for ( int row = r0; row <= r1; row++ )
    {
      for ( int col = c0; col <= c1; col++ )
      {
        tilesSet << TilePosition( row, col );
      }
    }
  }

  // get URLs of tiles because their URLs are used as keys in the tile cache
  TilePositions tiles = qgis::setToList( tilesSet );
  TileRequests requests;
  switch ( tileMode )
  {
    case WMSC:
      createTileRequestsWMSC( tmOther, tiles, requests );
      break;

    case WMTS:
      createTileRequestsWMTS( tmOther, tiles, requests );
      break;

    case XYZ:
      createTileRequestsXYZ( tmOther, tiles, requests, feedback );
      break;
  }

  if ( feedback && feedback->isCanceled() )
    return;

  QList<QRectF> missingRectsToDelete;
  for ( const TileRequest &r : std::as_const( requests ) )
  {
    if ( feedback && feedback->isCanceled() )
      return;

    QImage localImage;
    if ( !QgsTileCache::tile( r.url, localImage ) )
      continue;

    const QRect dst = destinationRect( viewExtent, r.rect, imageWidth );
    otherResTiles << TileImage( dst, localImage, false );

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

  if ( feedback && feedback->isCanceled() )
    return;

  // remove all the rectangles we have completely covered by tiles from this resolution
  // so we will not use tiles from multiple resolutions for one missing tile (to save time)
  for ( const QRectF &rectToDelete : std::as_const( missingRectsToDelete ) )
  {
    missingRects.removeOne( rectToDelete );
  }

  QgsDebugMsgLevel( u"Other resolution tiles: offset %1, res %2, missing rects %3, remaining rects %4, added tiles %5"_s.arg( resOffset ).arg( tmOther->tres ).arg( missingRects.count() + missingRectsToDelete.count() ).arg( missingRects.count() ).arg( otherResTiles.count() ), 3 );
}

uint qHash( QgsWmsProvider::TilePosition tp )
{
  return ( uint ) tp.col + ( ( uint ) tp.row << 16 );
}

static void _drawDebugRect( QPainter &p, const QRectF &rect, const QColor &color )
{
#if 0 // good for debugging how tiles from various resolutions are used
  QPainter::CompositionMode oldMode = p.compositionMode();
  p.setCompositionMode( QPainter::CompositionMode_SourceOver );
  QColor c = color;
  c.setAlpha( 100 );
  p.fillRect( rect, QBrush( c, Qt::DiagCrossPattern ) );
  p.setCompositionMode( oldMode );
#else
  Q_UNUSED( p )
  Q_UNUSED( rect )
  Q_UNUSED( color )
#endif
}

/**
 * Resets an image with a buffered image (add 2 pixels on each side) to take account of external pixels considering the resolution,
 * Returns the map extent of the buffered image.
 */
static QgsRectangle initializeBufferedImage( const QgsRectangle &viewExtent, double resolution, QImage *image )
{
  int pixelX = std::ceil( viewExtent.width() / resolution ) + 4;
  int pixelY = std::ceil( viewExtent.height() / resolution ) + 4;

  *image = QImage( pixelX, pixelY, QImage::Format_ARGB32 );
  image->fill( 0 );

  return QgsRectangle( viewExtent.xMinimum() - 2 * resolution, viewExtent.yMinimum() - 2 * resolution, viewExtent.xMinimum() + ( image->width() - 2 ) * resolution, viewExtent.yMinimum() + ( image->height() - 2 ) * resolution );
}

QImage QgsWmsProvider::draw( const QgsRectangle &viewExtent, int pixelWidth, int pixelHeight, QgsRectangle &effectiveViewExtent, double &sourceResolution, QgsRasterBlockFeedback *feedback )
{
  if ( qApp && qApp->thread() == QThread::currentThread() )
  {
    QgsDebugError( u"Trying to draw a WMS image on the main thread. Stop it!"_s );
  }

  // compose the URL query string for the WMS server.

  QImage image( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
  if ( image.isNull() )
    return image;

  image.fill( 0 );

  const QSize maxTileSize { maximumTileSize() };

  const int maxWidth { maxTileSize.width() };
  const int maxHeight { maxTileSize.height() };

  if ( !mSettings.mTiled && pixelWidth <= maxWidth && pixelHeight <= maxHeight )
  {
    QUrl url = createRequestUrlWMS( viewExtent, pixelWidth, pixelHeight );

    // cache some details for if the user wants to do an identifyAsHtml() later
    QgsWmsImageDownloadHandler handler( dataSourceUri(), url, mSettings.authorization(), &image, feedback );
    handler.downloadBlocking();
  }
  else
  {
    mTileReqNo++;

    double vres = viewExtent.width() / pixelWidth;

    const QgsWmtsTileMatrix *tm = nullptr;
    std::unique_ptr<QgsWmtsTileMatrix> tempTm;
    enum QgsTileMode tileMode;

    if ( mSettings.mTiled )
    {
      Q_ASSERT( mTileLayer );
      Q_ASSERT( mTileMatrixSet );
      if ( mTileMatrixSet->tileMatrices.isEmpty() )
      {
        QgsDebugError( u"WMTS tile set is empty!"_s );
        return image;
      }

      // if we know both source and output DPI, let's scale the tiles
      if ( mDpi != -1 && mTileLayer->dpi != -1 )
      {
        vres *= static_cast<double>( mDpi ) / mTileLayer->dpi;
      }

      // find nearest resolution
      tm = mTileMatrixSet->findNearestResolution( vres );
      Q_ASSERT( tm );

      tileMode = mTileLayer->tileMode;
    }
    else if ( ( mSettings.mMaxWidth != 0 && mSettings.mMaxHeight != 0 ) || pixelWidth > maxWidth || pixelHeight > maxHeight )
    {
      int w = mSettings.mMaxWidth != 0 && mSettings.mMaxWidth < maxWidth ? mSettings.mMaxWidth : maxWidth;
      int h = mSettings.mMaxHeight != 0 && mSettings.mMaxHeight < maxHeight ? mSettings.mMaxHeight : maxHeight;

      // this is an ordinary WMS server, but the user requested tiled approach
      // so we will pretend it is a WMS-C server with just one tile matrix
      tempTm = std::make_unique<QgsWmtsTileMatrix>();
      tempTm->topLeft = QgsPointXY( mLayerExtent.xMinimum(), mLayerExtent.yMaximum() );
      tempTm->tileWidth = w;
      tempTm->tileHeight = h;
      tempTm->matrixWidth = std::ceil( mLayerExtent.width() / w / vres );
      tempTm->matrixHeight = std::ceil( mLayerExtent.height() / h / vres );
      tempTm->tres = vres;
      tm = tempTm.get();

      tileMode = WMSC;
    }
    else
    {
      QgsDebugError( u"empty tile size"_s );
      return image;
    }

    QgsDebugMsgLevel( u"layer extent: %1,%2,%3,%4 %5x%6"_s.arg( qgsDoubleToString( mLayerExtent.xMinimum() ), qgsDoubleToString( mLayerExtent.yMinimum() ) ).arg( qgsDoubleToString( mLayerExtent.xMaximum() ), qgsDoubleToString( mLayerExtent.yMaximum() ) ).arg( mLayerExtent.width() ).arg( mLayerExtent.height() ), 3 );

    QgsDebugMsgLevel( u"view extent: %1,%2,%3,%4 %5x%6  res:%7"_s.arg( qgsDoubleToString( viewExtent.xMinimum() ), qgsDoubleToString( viewExtent.yMinimum() ) ).arg( qgsDoubleToString( viewExtent.xMaximum() ), qgsDoubleToString( viewExtent.yMaximum() ) ).arg( viewExtent.width() ).arg( viewExtent.height() ).arg( vres, 0, 'f' ), 3 );

    QgsDebugMsgLevel( u"tile matrix %1,%2 res:%3 tilesize:%4x%5 matrixsize:%6x%7 id:%8"_s.arg( tm->topLeft.x() ).arg( tm->topLeft.y() ).arg( tm->tres ).arg( tm->tileWidth ).arg( tm->tileHeight ).arg( tm->matrixWidth ).arg( tm->matrixHeight ).arg( tm->identifier ), 3 );

    const QgsWmtsTileMatrixLimits *tml = nullptr;

    if ( mTileLayer && mTileLayer->setLinks.contains( mTileMatrixSet->identifier ) && mTileLayer->setLinks[mTileMatrixSet->identifier].limits.contains( tm->identifier ) )
    {
      tml = &mTileLayer->setLinks[mTileMatrixSet->identifier].limits[tm->identifier];
    }

    // calculate tile coordinates
    int col0, col1, row0, row1;

    if ( mConverter && mProviderResamplingEnabled ) // if resampling we need some exterior pixel depending of the algorithm, max is 2 with cubic resampling
      tm->viewExtentIntersection( viewExtent.buffered( tm->tres * 2 ), tml, col0, row0, col1, row1 );
    else
      tm->viewExtentIntersection( viewExtent, tml, col0, row0, col1, row1 );

#ifdef QGISDEBUG
    int n = ( col1 - col0 + 1 ) * ( row1 - row0 + 1 );
    QgsDebugMsgLevel( u"tile number: %1x%2 = %3"_s.arg( col1 - col0 + 1 ).arg( row1 - row0 + 1 ).arg( n ), 3 );
    if ( n > 256 && !mSettings.mIsMBTiles )
    {
      emit statusChanged( u"current view would need %1 tiles. tile request per draw limited to 256."_s.arg( n ) );
      return image;
    }
#endif

    TilePositions tiles;
    for ( int row = row0; row <= row1; row++ )
    {
      for ( int col = col0; col <= col1; col++ )
      {
        tiles << TilePosition( row, col );
      }
    }

    TileRequests requests;
    switch ( tileMode )
    {
      case WMSC:
        createTileRequestsWMSC( tm, tiles, requests );
        break;

      case WMTS:
        createTileRequestsWMTS( tm, tiles, requests );
        break;

      case XYZ:
        createTileRequestsXYZ( tm, tiles, requests, feedback );
        break;

      default:
        QgsDebugError( u"unexpected tile mode %1"_s.arg( mTileLayer->tileMode ) );
        return image;
    }

    if ( feedback && feedback->isCanceled() )
      return image;

    QList<TileImage> tileImages; // in the correct resolution
    QList<QRectF> missing;       // rectangles (in map coords) of missing tiles for this view

    std::unique_ptr<QgsMbTiles> mbtilesReader;
    if ( mSettings.mIsMBTiles )
    {
      mbtilesReader = std::make_unique<QgsMbTiles>( QUrl( mSettings.mBaseUrl ).path() );
      mbtilesReader->open();
    }

    QElapsedTimer t;
    t.start();
    TileRequests requestsFinal;
    effectiveViewExtent = viewExtent;
    for ( const TileRequest &r : std::as_const( requests ) )
    {
      if ( feedback && feedback->isCanceled() )
        return image;

      QImage localImage;

      if ( mbtilesReader && !QgsTileCache::tile( r.url, localImage ) )
      {
        QUrlQuery query( r.url );
        QImage img = mbtilesReader->tileDataAsImage( query.queryItemValue( "z" ).toInt(), query.queryItemValue( "x" ).toInt(), query.queryItemValue( "y" ).toInt() );
        if ( img.isNull() )
          continue;
        QgsTileCache::insertTile( r.url, img );
      }

      if ( QgsTileCache::tile( r.url, localImage ) )
      {
        if ( mConverter && mProviderResamplingEnabled )
        {
          if ( sourceResolution < 0 )
          {
            sourceResolution = r.rect.width() / localImage.width();
            effectiveViewExtent = initializeBufferedImage( viewExtent, sourceResolution, &image );
          }
        }

        const QRect dst = destinationRect( effectiveViewExtent, r.rect, image.width() );

        // if image size is "close enough" to destination size, don't smooth it out. Instead try for pixel-perfect placement!
        bool disableSmoothing = mConverter || ( qgsDoubleNear( dst.width(), tm->tileWidth, 2 ) && qgsDoubleNear( dst.height(), tm->tileHeight, 2 ) );
        tileImages << TileImage( dst, localImage, !disableSmoothing );
      }
      else
      {
        missing << r.rect;

        // need to make a request
        requestsFinal << r;
      }
    }

    if ( feedback && feedback->isCanceled() )
      return image;

    if ( sourceResolution < 0 )
      effectiveViewExtent = viewExtent;

    int t0 = t.elapsed();
    // draw other res tiles if preview
    QPainter p( &image );
    if ( feedback && feedback->isPreviewOnly() && missing.count() > 0 )
    {
      // some tiles are still missing, so let's see if we have any cached tiles
      // from lower or higher resolution available to give the user a bit of context
      // while loading the right resolution

      p.setCompositionMode( QPainter::CompositionMode_Source );
#if 0 // for debugging
      p.fillRect( image->rect(), QBrush( Qt::lightGray, Qt::CrossPattern ) );
#endif
      p.setRenderHint( QPainter::SmoothPixmapTransform, false ); // let's not waste time with bilinear filtering

      QList<TileImage> lowerResTiles, lowerResTiles2, higherResTiles;
      // first we check lower resolution tiles: one level back, then two levels back (if there is still some area not covered),
      // finally (in the worst case we use one level higher resolution tiles). This heuristic should give
      // good overviews while not spending too much time drawing cached tiles from resolutions far away.
      fetchOtherResTiles( tileMode, effectiveViewExtent, pixelWidth, missing, tm->tres, 1, lowerResTiles, feedback );
      fetchOtherResTiles( tileMode, effectiveViewExtent, pixelWidth, missing, tm->tres, 2, lowerResTiles2, feedback );
      fetchOtherResTiles( tileMode, effectiveViewExtent, pixelWidth, missing, tm->tres, -1, higherResTiles, feedback );

      if ( feedback && feedback->isCanceled() )
      {
        p.end();
        return image;
      }

      // draw the cached tiles lowest to highest resolution
      for ( const TileImage &ti : std::as_const( lowerResTiles2 ) )
      {
        p.drawImage( ti.rect, ti.img );
        _drawDebugRect( p, ti.rect, Qt::blue );
      }
      for ( const TileImage &ti : std::as_const( lowerResTiles ) )
      {
        p.drawImage( ti.rect, ti.img );
        _drawDebugRect( p, ti.rect, Qt::yellow );
      }
      for ( const TileImage &ti : std::as_const( higherResTiles ) )
      {
        p.drawImage( ti.rect, ti.img );
        _drawDebugRect( p, ti.rect, Qt::red );
      }
    }

    int t1 = t.elapsed() - t0;

    if ( feedback && feedback->isCanceled() )
    {
      p.end();
      return image;
    }

    // draw composite in this resolution
    for ( const TileImage &ti : std::as_const( tileImages ) )
    {
      if ( feedback && feedback->isCanceled() )
      {
        p.end();
        return image;
      }

      if ( ti.smooth && mSettings.mSmoothPixmapTransform )
        p.setRenderHint( QPainter::SmoothPixmapTransform, true );
      p.drawImage( ti.rect, ti.img );

      if ( feedback && feedback->isPreviewOnly() )
        _drawDebugRect( p, ti.rect, Qt::green );
    }
    p.end();

    int t2 = t.elapsed() - t1;
    Q_UNUSED( t2 ) // only used in debug build

    if ( feedback && feedback->isPreviewOnly() )
    {
      QgsDebugMsgLevel( u"PREVIEW - CACHED: %1 / MISSING: %2"_s.arg( tileImages.count() ).arg( requests.count() - tileImages.count() ), 4 );
      QgsDebugMsgLevel( u"PREVIEW - TIME: this res %1 ms | other res %2 ms | TOTAL %3 ms"_s.arg( t0 + t2 ).arg( t1 ).arg( t0 + t1 + t2 ), 4 );
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

      QgsWmsTiledImageDownloadHandler handler(
        dataSourceUri(),
        mSettings.authorization(),
        mTileReqNo,
        requestsFinal,
        &image,
        effectiveViewExtent,
        sourceResolution,
        mSettings.mSmoothPixmapTransform,
        mProviderResamplingEnabled,
        feedback
      );

      handler.downloadBlocking();

      if ( feedback && !handler.error().isEmpty() )
        feedback->appendError( handler.error() );

      effectiveViewExtent = handler.effectiveViewExtent();
      sourceResolution = handler.sourceResolution();
    }

    QgsDebugMsgLevel( u"TILE CACHE total: %1 / %2"_s.arg( QgsTileCache::totalCost() ).arg( QgsTileCache::maxCost() ), 3 );

#if 0
    const QgsWmsStatistics::Stat &stat = QgsWmsStatistics::statForUri( dataSourceUri() );
    emit statusChanged( tr( "%n tile request(s) in background", "tile request count", requests.count() )
                        + tr( ", %n cache hit(s)", "tile cache hits", stat.cacheHits )
                        + tr( ", %n cache misses.", "tile cache missed", stat.cacheMisses )
                        + tr( ", %n error(s).", "errors", stat.errors )
                      );
#endif
  }

  return image;
}

bool QgsWmsProvider::readBlock( int bandNo, QgsRectangle const &viewExtent, int pixelWidth, int pixelHeight, void *block, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo )
  // TODO: optimize to avoid writing to QImage
  QgsRectangle effectiveExtent;
  double sourceResolution = -1;
  const QImage image( draw( viewExtent, pixelWidth, pixelHeight, effectiveExtent, sourceResolution, feedback ) );
  if ( image.isNull() ) // should not happen
  {
    QgsMessageLog::logMessage( tr( "image is NULL" ), tr( "WMS" ) );
    return false;
  }

  QgsDebugMsgLevel( u"image height = %1 bytesPerLine = %2"_s.arg( image.height() ).arg( image.bytesPerLine() ), 3 );
  size_t pixelsCount;
  if ( mConverter && mProviderResamplingEnabled )
    pixelsCount = static_cast<size_t>( image.width() ) * image.height();
  else
    pixelsCount = static_cast<size_t>( pixelWidth ) * pixelHeight;

  size_t myExpectedSize = pixelsCount * 4;
  size_t myImageSize = static_cast< size_t >( image.height() ) * image.bytesPerLine();
  if ( myExpectedSize != myImageSize ) // should not happen
  {
    QgsMessageLog::logMessage( tr( "unexpected image size" ), tr( "WMS" ) );
    return false;
  }

  const uchar *ptr = image.bits();
  if ( ptr )
  {
    // If image is too large, ptr can be NULL
    if ( mConverter && ( image.format() == QImage::Format_ARGB32 || image.format() == QImage::Format_RGB32 ) )
    {
      std::vector<float> data;
      data.resize( pixelsCount );
      const QRgb *inputPtr = reinterpret_cast<const QRgb *>( image.constBits() );
      float *outputPtr = data.data();
      for ( size_t i = 0; i < pixelsCount; ++i )
      {
        mConverter->convert( *inputPtr, outputPtr );
        inputPtr++;
        outputPtr++;
      }

      if ( mProviderResamplingEnabled )
      {
        const double resamplingFactor = ( viewExtent.width() / pixelWidth ) / sourceResolution;

        GDALResampleAlg alg;
        if ( resamplingFactor < 1 || qgsDoubleNear( resamplingFactor, 1.0 ) )
          alg = QgsGdalUtils::gdalResamplingAlgorithm( mZoomedInResamplingMethod );
        else
          alg = QgsGdalUtils::gdalResamplingAlgorithm( mZoomedOutResamplingMethod );

        gdal::dataset_unique_ptr gdalDsInput = QgsGdalUtils::blockToSingleBandMemoryDataset( image.width(), image.height(), effectiveExtent, data.data(), GDT_Float32 );
        gdal::dataset_unique_ptr gdalDsOutput = QgsGdalUtils::blockToSingleBandMemoryDataset( pixelWidth, pixelHeight, viewExtent, block, GDT_Float32 );
        return QgsGdalUtils::resampleSingleBandRaster( gdalDsInput.get(), gdalDsOutput.get(), alg, nullptr );
      }

      memcpy( block, data.data(), myExpectedSize );
      return true;
    }
    else
    {
      memcpy( block, ptr, myExpectedSize );
    }

    return true;
  }
  else
  {
    return false;
  }
}

QUrl QgsWmsProvider::createRequestUrlWMS( const QgsRectangle &viewExtent, int pixelWidth, int pixelHeight )
{
  // Calculate active layers that are also visible.

  bool changeXY = mCaps.shouldInvertAxisOrientation( mImageCrs );

  QgsDebugMsgLevel( "Active layer list of " + mSettings.mActiveSubLayers.join( ", " ) + " and style list of " + mSettings.mActiveSubStyles.join( ", " ), 2 );

  QStringList visibleLayers = QStringList();
  QStringList visibleStyles = QStringList();

  QStringList::const_iterator it2 = mSettings.mActiveSubStyles.constBegin();

  for ( QStringList::const_iterator it = mSettings.mActiveSubLayers.constBegin();
        it != mSettings.mActiveSubLayers.constEnd();
        ++it )
  {
    if ( mActiveSubLayerVisibility.constFind( *it ).value() )
    {
      visibleLayers += *it;
      visibleStyles += *it2;
    }

    ++it2;
  }

  QString layers = visibleLayers.join( ',' );
  layers = layers.isNull() ? QString() : layers;
  QString styles = visibleStyles.join( ',' );
  styles = styles.isNull() ? QString() : styles;

  QgsDebugMsgLevel( "Visible layer list of " + layers + " and style list of " + styles, 2 );

  QString bbox = toParamValue( viewExtent, changeXY );

  QUrl url( mSettings.mIgnoreGetMapUrl ? mSettings.mBaseUrl : getMapUrl() );
  QUrlQuery query( url );
  setQueryItem( query, u"SERVICE"_s, u"WMS"_s );
  setQueryItem( query, u"VERSION"_s, mCaps.mCapabilities.version );
  setQueryItem( query, u"REQUEST"_s, u"GetMap"_s );
  setQueryItem( query, u"BBOX"_s, bbox );
  setSRSQueryItem( query );
  setQueryItem( query, u"WIDTH"_s, QString::number( pixelWidth ) );
  setQueryItem( query, u"HEIGHT"_s, QString::number( pixelHeight ) );
  setQueryItem( query, u"LAYERS"_s, layers );
  setQueryItem( query, u"STYLES"_s, styles );
  QStringList opacityList = mSettings.mOpacities;
  if ( !opacityList.isEmpty() )
  {
    setQueryItem( query, u"OPACITIES"_s, mSettings.mOpacities.join( ',' ) );
  }

  if ( !mSettings.mFilter.isEmpty() )
  {
    setQueryItem( query, u"FILTER"_s, mSettings.mFilter );
  }

  // For WMS-T layers
  if ( temporalCapabilities() && temporalCapabilities()->hasTemporalCapabilities() )
  {
    addWmstParameters( query );
  }

  setFormatQueryItem( query );

  if ( mDpi != -1 )
  {
    if ( mSettings.mDpiMode & DpiQGIS )
      setQueryItem( query, u"DPI"_s, QString::number( mDpi ) );
    if ( mSettings.mDpiMode & DpiUMN )
      setQueryItem( query, u"MAP_RESOLUTION"_s, QString::number( mDpi ) );
    if ( mSettings.mDpiMode & DpiGeoServer )
      setQueryItem( query, u"FORMAT_OPTIONS"_s, u"dpi:%1"_s.arg( mDpi ) );
  }

  //MH: jpeg does not support transparency and some servers complain if jpg and transparent=true
  if ( mSettings.mImageMimeType == "image/x-jpegorpng"_L1 || ( !mSettings.mImageMimeType.contains( "jpeg"_L1, Qt::CaseInsensitive ) && !mSettings.mImageMimeType.contains( "jpg"_L1, Qt::CaseInsensitive ) ) )
  {
    setQueryItem( query, u"TRANSPARENT"_s, u"TRUE"_s ); // some servers giving error for 'true' (lowercase)
  }

  url.setQuery( query );

  QgsDebugMsgLevel( u"getmap: %1"_s.arg( url.toString() ), 2 );
  return url;
}

void QgsWmsProvider::addWmstParameters( QUrlQuery &query )
{
  QgsDateTimeRange range = temporalCapabilities()->requestedTemporalRange();

  QString format { u"yyyy-MM-ddThh:mm:ssZ"_s };
  bool dateOnly = false;

  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"wms"_s );

  QVariantMap uri = metadata->decodeUri( dataSourceUri() );

  // Skip fetching if updates are not allowed
  if ( !uri.value( u"allowTemporalUpdates"_s, true ).toBool() )
    return;

  if ( range.isInfinite() )
  {
    if ( uri.contains( u"time"_s ) && !uri.value( u"time"_s ).toString().isEmpty() )
    {
      QString time = uri.value( u"time"_s ).toString();
      QStringList timeParts = time.split( '/' );

      QDateTime start = QDateTime::fromString( timeParts.at( 0 ), Qt::ISODateWithMs );
      QDateTime end = QDateTime::fromString( timeParts.at( 1 ), Qt::ISODateWithMs );

      range = QgsDateTimeRange( start, end );
    }
  }

  if ( !uri.value( u"enableTime"_s, true ).toBool() )
  {
    format = u"yyyy-MM-dd"_s;
    dateOnly = true;
  }

  if ( range.begin().isValid() && range.end().isValid() )
  {
    switch ( temporalCapabilities()->intervalHandlingMethod() )
    {
      case Qgis::TemporalIntervalMatchMethod::MatchUsingWholeRange:
        break;
      case Qgis::TemporalIntervalMatchMethod::MatchExactUsingStartOfRange:
        range = QgsDateTimeRange( range.begin(), range.begin() );
        break;
      case Qgis::TemporalIntervalMatchMethod::MatchExactUsingEndOfRange:
        range = QgsDateTimeRange( range.end(), range.end() );
        break;
      case Qgis::TemporalIntervalMatchMethod::FindClosestMatchToStartOfRange:
      {
        QDateTime dateTimeStart = mSettings.findLeastClosestDateTime( range.begin(), dateOnly );
        range = QgsDateTimeRange( dateTimeStart, dateTimeStart );
        break;
      }

      case Qgis::TemporalIntervalMatchMethod::FindClosestMatchToEndOfRange:
      {
        QDateTime dateTimeEnd = mSettings.findLeastClosestDateTime( range.end(), dateOnly );
        range = QgsDateTimeRange( dateTimeEnd, dateTimeEnd );
        break;
      }
    }

    if ( range.begin() == range.end() )
      setQueryItem( query, u"TIME"_s, range.begin().toString( format ) );
    else
    {
      QString extent = range.begin().toString( format );
      extent.append( "/" );
      extent.append( range.end().toString( format ) );

      setQueryItem( query, u"TIME"_s, extent );
    }
  }

  // If the data provider has bi-temporal properties and they are enabled
  if ( uri.contains( u"referenceTime"_s ) && !uri.value( u"referenceTime"_s ).toString().isEmpty() )
  {
    QString time = uri.value( u"referenceTime"_s ).toString();

    QDateTime dateTime = QDateTime::fromString( time, Qt::ISODateWithMs );

    if ( dateTime.isValid() )
    {
      setQueryItem( query, u"DIM_REFERENCE_TIME"_s, dateTime.toString( format ) );
    }
  }
}

QString QgsWmsProvider::calculateWmtsTimeDimensionValue() const
{
  QgsDateTimeRange range = temporalCapabilities()->requestedTemporalRange();

  QString format { u"yyyy-MM-ddThh:mm:ssZ"_s };
  bool dateOnly = false;

  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"wms"_s );

  QVariantMap uri = metadata->decodeUri( dataSourceUri() );

  // Skip fetching if updates are not allowed
  if ( !uri.value( u"allowTemporalUpdates"_s, true ).toBool() )
    return QString();

  if ( range.isInfinite() )
  {
    if ( uri.contains( u"time"_s ) && !uri.value( u"time"_s ).toString().isEmpty() )
    {
      QString time = uri.value( u"time"_s ).toString();
      QStringList timeParts = time.split( '/' );

      QDateTime start = QDateTime::fromString( timeParts.at( 0 ), Qt::ISODateWithMs );
      QDateTime end = QDateTime::fromString( timeParts.at( 1 ), Qt::ISODateWithMs );

      range = QgsDateTimeRange( start, end );
    }
  }

  if ( !uri.value( u"enableTime"_s, true ).toBool() )
  {
    format = u"yyyy-MM-dd"_s;
    dateOnly = true;
  }

  if ( range.begin().isValid() && range.end().isValid() )
  {
    QDateTime time;
    switch ( temporalCapabilities()->intervalHandlingMethod() )
    {
      case Qgis::TemporalIntervalMatchMethod::MatchUsingWholeRange:
        time = range.begin();
        break;
      case Qgis::TemporalIntervalMatchMethod::MatchExactUsingStartOfRange:
        time = range.begin();
        break;
      case Qgis::TemporalIntervalMatchMethod::MatchExactUsingEndOfRange:
        time = range.end();
        break;
      case Qgis::TemporalIntervalMatchMethod::FindClosestMatchToStartOfRange:
        time = mSettings.findLeastClosestDateTime( range.begin(), dateOnly );
        break;
      case Qgis::TemporalIntervalMatchMethod::FindClosestMatchToEndOfRange:
        time = mSettings.findLeastClosestDateTime( range.end(), dateOnly );
        break;
    }

    QString formattedTime;
    switch ( mTileLayer->timeFormat )
    {
      case QgsWmtsTileLayer::WmtsTimeFormat::yyyyMMdd:
        formattedTime = time.toString( u"yyyyMMdd"_s );
        break;
      case QgsWmtsTileLayer::WmtsTimeFormat::yyyy_MM_dd:
        formattedTime = time.toString( u"yyyy-MM-dd"_s );
        break;
      case QgsWmtsTileLayer::WmtsTimeFormat::yyyy:
        formattedTime = time.toString( u"yyyy"_s );
        break;
      case QgsWmtsTileLayer::WmtsTimeFormat::yyyyMMddyyyyMMddPxx:
        formattedTime = time.toString( u"yyyy-MM-ddThh:mm:ssZ"_s );
        break;
      case QgsWmtsTileLayer::WmtsTimeFormat::yyyyMMddThhmmssZ:
        formattedTime = time.toString( u"yyyy-MM-ddThh:mm:ssZ"_s );
        break;
    }

    return formattedTime;
  }

  return mTileLayer->defaultTimeDimensionValue;
}

void QgsWmsProvider::createTileRequestsWMSC( const QgsWmtsTileMatrix *tm, const QgsWmsProvider::TilePositions &tiles, QgsWmsProvider::TileRequests &requests )
{
  bool changeXY = mCaps.shouldInvertAxisOrientation( mImageCrs );

  // add WMS request
  QUrl url( mSettings.mIgnoreGetMapUrl ? mSettings.mBaseUrl : getMapUrl() );
  QUrlQuery query( url );
  setQueryItem( query, u"SERVICE"_s, u"WMS"_s );
  setQueryItem( query, u"VERSION"_s, mCaps.mCapabilities.version );
  setQueryItem( query, u"REQUEST"_s, u"GetMap"_s );
  setQueryItem( query, u"LAYERS"_s, mSettings.mActiveSubLayers.join( ','_L1 ) );
  setQueryItem( query, u"STYLES"_s, mSettings.mActiveSubStyles.join( ','_L1 ) );
  setQueryItem( query, u"WIDTH"_s, QString::number( tm->tileWidth ) );
  setQueryItem( query, u"HEIGHT"_s, QString::number( tm->tileHeight ) );
  setFormatQueryItem( query );

  setSRSQueryItem( query );

  if ( mSettings.mTiled )
  {
    setQueryItem( query, u"TILED"_s, u"true"_s );
  }

  if ( mDpi != -1 )
  {
    if ( mSettings.mDpiMode & DpiQGIS )
      setQueryItem( query, u"DPI"_s, QString::number( mDpi ) );
    if ( mSettings.mDpiMode & DpiUMN )
      setQueryItem( query, u"MAP_RESOLUTION"_s, QString::number( mDpi ) );
    if ( mSettings.mDpiMode & DpiGeoServer )
      setQueryItem( query, u"FORMAT_OPTIONS"_s, u"dpi:%1"_s.arg( mDpi ) );
  }

  if ( mSettings.mImageMimeType == "image/x-jpegorpng"_L1 || ( !mSettings.mImageMimeType.contains( "jpeg"_L1, Qt::CaseInsensitive ) && !mSettings.mImageMimeType.contains( "jpg"_L1, Qt::CaseInsensitive ) ) )
  {
    setQueryItem( query, u"TRANSPARENT"_s, u"TRUE"_s ); // some servers giving error for 'true' (lowercase)
  }

  // For WMSC-T layers
  if ( temporalCapabilities() && temporalCapabilities()->hasTemporalCapabilities() )
  {
    addWmstParameters( query );
  }

  url.setQuery( query );

  int i = 0;
  for ( const TilePosition &tile : tiles )
  {
    QgsRectangle bbox( tm->tileBBox( tile.col, tile.row ) );
    QString turl;
    turl += url.toString();
    turl += QString( changeXY ? "&BBOX=%2,%1,%4,%3" : "&BBOX=%1,%2,%3,%4" )
              .arg( qgsDoubleToString( bbox.xMinimum() ), qgsDoubleToString( bbox.yMinimum() ), qgsDoubleToString( bbox.xMaximum() ), qgsDoubleToString( bbox.yMaximum() ) );

    QgsDebugMsgLevel( u"tileRequest %1 %2/%3 (%4,%5): %6"_s.arg( mTileReqNo ).arg( i ).arg( tiles.count() ).arg( tile.row ).arg( tile.col ).arg( turl ), 2 );
    requests << TileRequest( turl, tm->tileRect( tile.col, tile.row ), i );
    ++i;
  }
}


void QgsWmsProvider::createTileRequestsWMTS( const QgsWmtsTileMatrix *tm, const QgsWmsProvider::TilePositions &tiles, QgsWmsProvider::TileRequests &requests )
{
  if ( !getTileUrl().isNull() )
  {
    // KVP
    QString baseUrl = mSettings.mIgnoreGetMapUrl ? mSettings.mBaseUrl : getTileUrl();


    QUrl url( baseUrl );
    QUrlQuery query( url );

    // compose static request arguments.
    setQueryItem( query, u"SERVICE"_s, u"WMTS"_s );
    setQueryItem( query, u"REQUEST"_s, u"GetTile"_s );
    setQueryItem( query, u"VERSION"_s, mCaps.mCapabilities.version );
    setQueryItem( query, u"LAYER"_s, mSettings.mActiveSubLayers[0] );
    setQueryItem( query, u"STYLE"_s, mSettings.mActiveSubStyles[0] );
    setQueryItem( query, u"FORMAT"_s, mSettings.mImageMimeType );
    setQueryItem( query, u"TILEMATRIXSET"_s, mTileMatrixSet->identifier );
    setQueryItem( query, u"TILEMATRIX"_s, tm->identifier );

    if ( temporalCapabilities() && temporalCapabilities()->hasTemporalCapabilities() && !mTileLayer->timeDimensionIdentifier.isEmpty() )
    {
      // time dimension gets special handling
      const QString formattedTime = calculateWmtsTimeDimensionValue();
      if ( !formattedTime.isEmpty() )
        setQueryItem( query, mTileLayer->timeDimensionIdentifier, formattedTime );
    }

    for ( QHash<QString, QString>::const_iterator it = mSettings.mTileDimensionValues.constBegin(); it != mSettings.mTileDimensionValues.constEnd(); ++it )
    {
      setQueryItem( query, it.key(), it.value() );
    }

    query.removeQueryItem( u"TILEROW"_s );
    query.removeQueryItem( u"TILECOL"_s );
    url.setQuery( query );

    int i = 0;
    for ( const TilePosition &tile : tiles )
    {
      QString turl;
      turl += url.toString();
      turl += u"&TILEROW=%1&TILECOL=%2"_s.arg( tile.row ).arg( tile.col );

      QgsDebugMsgLevel( u"tileRequest %1 %2/%3 (%4,%5): %6"_s.arg( mTileReqNo ).arg( i ).arg( tiles.count() ).arg( tile.row ).arg( tile.col ).arg( turl ), 2 );
      requests << TileRequest( turl, tm->tileRect( tile.col, tile.row ), i );
      ++i;
    }
  }
  else
  {
    // REST
    QString url = mTileLayer->getTileURLs[mSettings.mImageMimeType];

    url.replace( "{layer}"_L1, mSettings.mActiveSubLayers[0], Qt::CaseInsensitive );
    url.replace( "{style}"_L1, mSettings.mActiveSubStyles[0], Qt::CaseInsensitive );
    url.replace( "{tilematrixset}"_L1, mTileMatrixSet->identifier, Qt::CaseInsensitive );
    url.replace( "{tilematrix}"_L1, tm->identifier, Qt::CaseInsensitive );

    for ( QHash<QString, QString>::const_iterator it = mSettings.mTileDimensionValues.constBegin(); it != mSettings.mTileDimensionValues.constEnd(); ++it )
    {
      url.replace( "{" + it.key() + "}", it.value(), Qt::CaseInsensitive );
    }

    if ( temporalCapabilities() && temporalCapabilities()->hasTemporalCapabilities() && !mTileLayer->timeDimensionIdentifier.isEmpty() )
    {
      // time dimension gets special handling
      const QString formattedTime = calculateWmtsTimeDimensionValue();
      if ( !formattedTime.isEmpty() )
        url.replace( u"{%1}"_s.arg( mTileLayer->timeDimensionIdentifier ), formattedTime );
    }

    int i = 0;
    for ( const TilePosition &tile : tiles )
    {
      QString turl( url );
      turl.replace( "{tilerow}"_L1, QString::number( tile.row ), Qt::CaseInsensitive );
      turl.replace( "{tilecol}"_L1, QString::number( tile.col ), Qt::CaseInsensitive );

      QgsDebugMsgLevel( u"tileRequest %1 %2/%3 (%4,%5): %6"_s.arg( mTileReqNo ).arg( i ).arg( tiles.count() ).arg( tile.row ).arg( tile.col ).arg( turl ), 2 );
      requests << TileRequest( turl, tm->tileRect( tile.col, tile.row ), i );
      ++i;
    }
  }
}


// support for Bing Maps tile system
// https://msdn.microsoft.com/en-us/library/bb259689.aspx
static QString _tile2quadkey( int tileX, int tileY, int z )
{
  QString quadKey;
  for ( int i = z; i > 0; i-- )
  {
    char digit = '0';
    int mask = 1 << ( i - 1 );
    if ( tileX & mask )
      digit++;
    if ( tileY & mask )
      digit += 2;
    quadKey.append( QChar( digit ) );
  }
  return quadKey;
}


void QgsWmsProvider::createTileRequestsXYZ( const QgsWmtsTileMatrix *tm, const QgsWmsProvider::TilePositions &tiles, QgsWmsProvider::TileRequests &requests, QgsRasterBlockFeedback *feedback )
{
  int z = tm->identifier.toInt();
  QString url = mSettings.mBaseUrl;
  int i = 0;
  for ( const TilePosition &tile : tiles )
  {
    ++i;
    QString turl( url );

    if ( turl.contains( "{q}"_L1 ) ) // used in Bing maps
      turl.replace( "{q}"_L1, _tile2quadkey( tile.col, tile.row, z ) );

    turl.replace( "{x}"_L1, QString::number( tile.col ), Qt::CaseInsensitive );
    // inverted Y axis
    if ( turl.contains( "{-y}"_L1 ) )
    {
      turl.replace( "{-y}"_L1, QString::number( tm->matrixHeight - tile.row - 1 ), Qt::CaseInsensitive );
    }
    else
    {
      turl.replace( "{y}"_L1, QString::number( tile.row ), Qt::CaseInsensitive );
    }
    turl.replace( "{z}"_L1, QString::number( z ), Qt::CaseInsensitive );

    if ( turl.contains( "{usage}"_L1 ) && feedback )
    {
      switch ( feedback->renderContext().rendererUsage() )
      {
        case Qgis::RendererUsage::View:
          turl.replace( "{usage}"_L1, "view"_L1 );
          break;
        case Qgis::RendererUsage::Export:
          turl.replace( "{usage}"_L1, "export"_L1 );
          break;
        case Qgis::RendererUsage::Unknown:
          turl.replace( "{usage}"_L1, QString() );
          break;
      }
    }

    QgsDebugMsgLevel( u"tileRequest %1 %2/%3 (%4,%5): %6"_s.arg( mTileReqNo ).arg( i ).arg( tiles.count() ).arg( tile.row ).arg( tile.col ).arg( turl ), 2 );
    requests << TileRequest( turl, tm->tileRect( tile.col, tile.row ), i );
  }
}


bool QgsWmsProvider::retrieveServerCapabilities( bool forceRefresh )
{
  if ( !mCaps.isValid() )
  {
    QgsWmsCapabilitiesDownload downloadCaps( mSettings.baseUrl(), mSettings.authorization(), forceRefresh );
    if ( !downloadCaps.downloadCapabilities() )
    {
      mErrorFormat = u"text/plain"_s;
      mError = downloadCaps.lastError();
      return false;
    }

    QgsWmsCapabilities caps( transformContext(), mSettings.baseUrl() );
    if ( !caps.parseResponse( downloadCaps.response(), mSettings.parserSettings() ) )
    {
      mErrorFormat = caps.lastErrorFormat();
      mError = caps.lastError();
      return false;
    }

    mCaps = caps;
  }

  Q_ASSERT( mCaps.isValid() );

  return true;
}


bool QgsWmsProvider::setupXyzCapabilities( const QString &uri, const QgsRectangle &sourceExtent, int sourceMinZoom, int sourceMaxZoom, double sourceTilePixelRatio )
{
  QgsDataSourceUri parsedUri;
  parsedUri.setEncodedUri( uri );

  QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ), QgsCoordinateReferenceSystem( mSettings.mCrsId ), transformContext() );

  // the whole world is projected to a square:
  // X going from 180 W to 180 E
  // Y going from ~85 N to ~85 S  (=atan(sinh(pi)) ... to get a square)
  QgsPointXY topLeftLonLat( -180, 180.0 / M_PI * std::atan( std::sinh( M_PI ) ) );
  QgsPointXY bottomRightLonLat( 180, 180.0 / M_PI * std::atan( std::sinh( -M_PI ) ) );
  QgsPointXY topLeft;
  QgsPointXY bottomRight;
  try
  {
    topLeft = ct.transform( topLeftLonLat );
    bottomRight = ct.transform( bottomRightLonLat );
  }
  catch ( const QgsCsException & )
  {
    QgsDebugError( u"setupXyzCapabilities: failed to reproject corner coordinates"_s );
    return false;
  }
  double xspan = ( bottomRight.x() - topLeft.x() );

  QgsWmsBoundingBoxProperty bbox;
  bbox.crs = mSettings.mCrsId;
  bbox.box = sourceExtent.isNull() ? QgsRectangle( topLeft.x(), bottomRight.y(), bottomRight.x(), topLeft.y() ) : sourceExtent;

  // metadata
  if ( mSettings.mXyz )
  {
    if ( parsedUri.param( u"url"_s ).contains( "openstreetmap"_L1, Qt::CaseInsensitive ) )
    {
      mLayerMetadata.setTitle( tr( "OpenStreetMap tiles" ) );
      mLayerMetadata.setIdentifier( tr( "OpenStreetMap tiles" ) );
      mLayerMetadata.setAbstract( tr( "OpenStreetMap is built by a community of mappers that contribute and maintain data about roads, trails, cafés, railway stations, and much more, all over the world." ) );

      QStringList licenses;
      licenses << tr( "Open Data Commons Open Database License (ODbL)" );
      if ( parsedUri.param( u"url"_s ).contains( "tile.openstreetmap.org"_L1, Qt::CaseInsensitive ) )
      {
        // OSM tiles have a different attribution requirement to OpenStreetMap data - see https://www.openstreetmap.org/copyright
        mLayerMetadata.setRights( QStringList() << tr( "Base map and data from OpenStreetMap and OpenStreetMap Foundation (CC-BY-SA). © https://www.openstreetmap.org and contributors." ) );
        licenses << tr( "Creative Commons Attribution-ShareAlike (CC-BY-SA)" );
      }
      else
        mLayerMetadata.setRights( QStringList() << tr( "© OpenStreetMap and contributors (https://www.openstreetmap.org/copyright)." ) );
      mLayerMetadata.setLicenses( licenses );

      QgsLayerMetadata::SpatialExtent spatialExtent;
      spatialExtent.bounds = QgsBox3D( QgsRectangle( topLeftLonLat.x(), bottomRightLonLat.y(), bottomRightLonLat.x(), topLeftLonLat.y() ) );
      spatialExtent.extentCrs = QgsCoordinateReferenceSystem( u"EPSG:4326"_s );
      QgsLayerMetadata::Extent metadataExtent;
      metadataExtent.setSpatialExtents( QList<QgsLayerMetadata::SpatialExtent>() << spatialExtent );
      mLayerMetadata.setExtent( metadataExtent );
      mLayerMetadata.setCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );

      mLayerMetadata.addLink( QgsAbstractMetadataBase::Link( tr( "Source" ), u"WWW:LINK"_s, u"https://www.openstreetmap.org/"_s ) );
    }
  }
  mLayerMetadata.setType( u"dataset"_s );

  QgsWmtsTileLayer tl;
  tl.tileMode = XYZ;
  tl.identifier = u"xyz"_s; // as set in parseUri
  tl.boundingBoxes << bbox;
  // suppress cppcheck warnings
  tl.dpi = -1;
  tl.timeFormat = QgsWmtsTileLayer::WmtsTimeFormat::yyyyMMdd;

  double tilePixelRatio = sourceTilePixelRatio; // by default 0 = unknown
  if ( parsedUri.hasParam( u"tilePixelRatio"_s ) )
    tilePixelRatio = parsedUri.param( u"tilePixelRatio"_s ).toDouble();

  if ( tilePixelRatio != 0 )
  {
    // known tile pixel ratio - will be doing auto-scaling of tiles based on output DPI
    tl.dpi = 96 * tilePixelRatio; // TODO: is 96 correct base DPI ?
  }
  else
  {
    // unknown tile pixel ratio - no scaling of tiles based on output DPI
    tilePixelRatio = 1;
  }

  mCaps.mTileLayersSupported.append( tl );

  QgsWmtsTileMatrixSetLink &tmsLinkRef = mCaps.mTileLayersSupported.last().setLinks[u"tms0"_s];
  tmsLinkRef.tileMatrixSet = u"tms0"_s;

  QgsWmtsTileMatrixSet tms;
  tms.identifier = u"tms0"_s; // as set in parseUri
  tms.crs = mSettings.mCrsId;
  mCaps.mTileMatrixSets[tms.identifier] = tms;

  int minZoom = sourceMinZoom == -1 ? 0 : sourceMinZoom;
  int maxZoom = sourceMaxZoom == -1 ? 18 : sourceMaxZoom;
  if ( parsedUri.hasParam( u"zmin"_s ) )
    minZoom = parsedUri.param( u"zmin"_s ).toInt();
  if ( parsedUri.hasParam( u"zmax"_s ) )
    maxZoom = parsedUri.param( u"zmax"_s ).toInt();

  // zoom 0 is one tile for the whole world
  for ( int zoom = minZoom; zoom <= maxZoom; ++zoom )
  {
    QgsWmtsTileMatrix tm;
    tm.identifier = QString::number( zoom );
    tm.topLeft = topLeft;
    tm.tileWidth = tm.tileHeight = 256 * tilePixelRatio;
    tm.matrixWidth = tm.matrixHeight = 1 << zoom;
    tm.tres = xspan / ( static_cast<qgssize>( tm.tileWidth ) * tm.matrixWidth );
    tm.scaleDenom = 0.0;

    mCaps.mTileMatrixSets[tms.identifier].tileMatrices[tm.tres] = tm;

    if ( !sourceExtent.isNull() )
    {
      // set limits based on bounding box
      QgsWmtsTileMatrixLimits limits;
      limits.tileMatrix = tm.identifier;
      tm.viewExtentIntersection( sourceExtent, nullptr, limits.minTileCol, limits.minTileRow, limits.maxTileCol, limits.maxTileRow );
      tmsLinkRef.limits[tm.identifier] = limits;
    }
  }
  return true;
}

bool QgsWmsProvider::setupMBTilesCapabilities( const QString &uri )
{
  // if it is MBTiles source, let's prepare the reader to get some metadata
  QgsMbTiles mbtilesReader( QUrl( mSettings.mBaseUrl ).path() );
  if ( !mbtilesReader.open() )
    return false;

  // We expect something like "mbtiles:///path/to/my/file.mbtiles" as the URL for tiles in MBTiles specs.
  // Here we just add extra x,y,z query items as an implementation detail (it uses TMS tiling scheme - that's why {-y})
  mSettings.mBaseUrl += "?x={x}&y={-y}&z={z}";

  QgsRectangle sourceExtent;
  QgsRectangle sourceExtentWgs84 = mbtilesReader.extent();
  if ( !sourceExtentWgs84.isNull() )
  {
    QgsPointXY customTopLeft, customBottomRight;
    QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ), QgsCoordinateReferenceSystem( mSettings.mCrsId ), transformContext() );
    try
    {
      customTopLeft = ct.transform( QgsPointXY( sourceExtentWgs84.xMinimum(), sourceExtentWgs84.yMaximum() ) );
      customBottomRight = ct.transform( QgsPointXY( sourceExtentWgs84.xMaximum(), sourceExtentWgs84.yMinimum() ) );
    }
    catch ( const QgsCsException & )
    {
      QgsDebugError( u"Failed to reproject extent from MBTiles metadata"_s );
      return false;
    }
    sourceExtent = QgsRectangle( customTopLeft.x(), customBottomRight.y(), customBottomRight.x(), customTopLeft.y() );
  }

  // minzoom/maxzoom SHOULD be in MBTiles (since spec v1.3) - but does not have to be there...
  int sourceMinZoom = -1, sourceMaxZoom = -1;
  QString sourceMinZoomStr = mbtilesReader.metadataValue( "minzoom" );
  QString sourceMaxZoomStr = mbtilesReader.metadataValue( "maxzoom" );
  if ( !sourceMinZoomStr.isEmpty() && !sourceMaxZoomStr.isEmpty() )
  {
    sourceMinZoom = sourceMinZoomStr.toInt();
    sourceMaxZoom = sourceMaxZoomStr.toInt();
  }

  // Assuming tiles with resolution of 256x256 pixels at 96 DPI.
  // This can be overridden by "tilePixelRatio" in URI. Unfortunately
  // MBTiles spec does not say anything about resolutions...
  double sourceTilePixelRatio = 1;

  return setupXyzCapabilities( uri, sourceExtent, sourceMinZoom, sourceMaxZoom, sourceTilePixelRatio );
}


Qgis::DataType QgsWmsProvider::dataType( int bandNo ) const
{
  return sourceDataType( bandNo );
}

Qgis::DataType QgsWmsProvider::sourceDataType( int bandNo ) const
{
  Q_UNUSED( bandNo )
  if ( mConverter )
    return mConverter->dataType();
  else
    return Qgis::DataType::ARGB32;
}

int QgsWmsProvider::bandCount() const
{
  return 1;
}


static const QgsWmsLayerProperty *_findNestedLayerProperty( const QString &layerName, const QgsWmsLayerProperty *prop )
{
  if ( prop->name == layerName )
    return prop;

  for ( const QgsWmsLayerProperty &child : std::as_const( prop->layer ) )
  {
    if ( const QgsWmsLayerProperty *res = _findNestedLayerProperty( layerName, &child ) )
      return res;
  }

  return nullptr;
}


bool QgsWmsProvider::extentForNonTiledLayer( const QString &layerName, const QString &crs, QgsRectangle &extent ) const
{
  const QgsWmsLayerProperty *layerProperty = nullptr;
  for ( const QgsWmsLayerProperty &toplevelLayer : std::as_const( mCaps.mCapabilities.capability.layers ) )
  {
    layerProperty = _findNestedLayerProperty( layerName, &toplevelLayer );
    if ( layerProperty )
      break;
  }
  if ( !layerProperty )
    return false;

  // see if we can refine the bounding box with the CRS-specific bounding boxes
  for ( int i = 0; i < layerProperty->boundingBoxes.size(); i++ )
  {
    if ( layerProperty->boundingBoxes[i].crs == crs )
    {
      // exact bounding box is provided for this CRS
      extent = layerProperty->boundingBoxes[i].box;
      return true;
    }
  }

  // exact bounding box for given CRS is not listed - we need to pick a different
  // bounding box definition - either the coarse bounding box (in WGS84)
  // or one of the alternative bounding box definitions for the layer

  // Use the coarse bounding box
  extent = layerProperty->ex_GeographicBoundingBox;

  for ( int i = 0; i < layerProperty->boundingBoxes.size(); i++ )
  {
    if ( layerProperty->boundingBoxes[i].crs == DEFAULT_LATLON_CRS )
    {
      if ( layerProperty->boundingBoxes[i].box.contains( extent ) )
        continue; // this bounding box is less specific (probably inherited from parent)

      // this BBox is probably better than the one in ex_GeographicBoundingBox
      extent = layerProperty->boundingBoxes[i].box;
      break;
    }
  }

  // transform it to requested CRS

  QgsCoordinateReferenceSystem dst = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );
  QgsCoordinateReferenceSystem wgs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( DEFAULT_LATLON_CRS );
  if ( !wgs.isValid() || !dst.isValid() )
    return false;

  QgsDebugMsgLevel( u"transforming layer extent %1"_s.arg( extent.toString( true ) ), 2 );
  try
  {
    QgsCoordinateTransform xform( wgs, dst, transformContext() );
    xform.setBallparkTransformsAreAppropriate( true );
    extent = xform.transformBoundingBox( extent );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse )
    return false;
  }
  QgsDebugMsgLevel( u"transformed layer extent %1"_s.arg( extent.toString( true ) ), 2 );

  //make sure extent does not contain 'inf' or 'nan'
  if ( !extent.isFinite() )
  {
    return false;
  }

  return true;
}


bool QgsWmsProvider::parseServiceExceptionReportDom( QByteArray const &xml, QString &errorTitle, QString &errorText )
{
#ifdef QGISDEBUG
  //test the content of the QByteArray
  QString responsestring( xml );
  QgsDebugMsgLevel( "received the following data: " + responsestring, 2 );
#endif

  // Convert completed document into a Dom
  QDomDocument doc;
  QString errorMsg;
  int errorLine;
  int errorColumn;
  bool contentSuccess = doc.setContent( xml, false, &errorMsg, &errorLine, &errorColumn );

  if ( !contentSuccess )
  {
    errorTitle = tr( "Dom Exception" );
    errorText = tr( "Could not get WMS Service Exception: %1 at line %2 column %3\n\nResponse was:\n\n%4" )
                  .arg( errorMsg )
                  .arg( errorLine )
                  .arg( errorColumn )
                  .arg( QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + errorText );

    return false;
  }

  QDomElement docElem = doc.documentElement();

  // TODO: Assert the docElem.tagName() is "ServiceExceptionReport"

  // serviceExceptionProperty.version = docElem.attribute("version");

  // Start walking through XML.
  QDomNode n = docElem.firstChild();

  while ( !n.isNull() )
  {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if ( !e.isNull() )
    {
      QgsDebugMsgLevel( e.tagName(), 2 ); // the node really is an element.

      QString tagName = e.tagName();
      if ( tagName.startsWith( "wms:"_L1 ) || tagName.startsWith( "ogc:"_L1 ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "ServiceException"_L1 )
      {
        QgsDebugError( u"  ServiceException."_s );
        parseServiceException( e, errorTitle, errorText );
      }
    }
    n = n.nextSibling();
  }

  return true;
}


void QgsWmsProvider::parseServiceException( QDomElement const &e, QString &errorTitle, QString &errorText )
{
  QString seCode = e.attribute( u"code"_s );
  QString seText = e.text();

  errorTitle = tr( "Service Exception" );

  // set up friendly descriptions for the service exception
  if ( seCode == "InvalidFormat"_L1 )
  {
    errorText = tr( "Request contains a format not offered by the server." );
  }
  else if ( seCode == "InvalidCRS"_L1 )
  {
    errorText = tr( "Request contains a CRS not offered by the server for one or more of the Layers in the request." );
  }
  else if ( seCode == "InvalidSRS"_L1 ) // legacy WMS < 1.3.0
  {
    errorText = tr( "Request contains a SRS not offered by the server for one or more of the Layers in the request." );
  }
  else if ( seCode == "LayerNotDefined"_L1 )
  {
    errorText = tr( "GetMap request is for a Layer not offered by the server, "
                    "or GetFeatureInfo request is for a Layer not shown on the map." );
  }
  else if ( seCode == "StyleNotDefined"_L1 )
  {
    errorText = tr( "Request is for a Layer in a Style not offered by the server." );
  }
  else if ( seCode == "LayerNotQueryable"_L1 )
  {
    errorText = tr( "GetFeatureInfo request is applied to a Layer which is not declared queryable." );
  }
  else if ( seCode == "InvalidPoint"_L1 )
  {
    errorText = tr( "GetFeatureInfo request contains invalid X or Y value." );
  }
  else if ( seCode == "CurrentUpdateSequence"_L1 )
  {
    errorText = tr( "Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to "
                    "current value of service metadata update sequence number." );
  }
  else if ( seCode == "InvalidUpdateSequence"_L1 )
  {
    errorText = tr( "Value of (optional) UpdateSequence parameter in GetCapabilities request is greater "
                    "than current value of service metadata update sequence number." );
  }
  else if ( seCode == "MissingDimensionValue"_L1 )
  {
    errorText = tr( "Request does not include a sample dimension value, and the server did not declare a "
                    "default value for that dimension." );
  }
  else if ( seCode == "InvalidDimensionValue"_L1 )
  {
    errorText = tr( "Request contains an invalid sample dimension value." );
  }
  else if ( seCode == "OperationNotSupported"_L1 )
  {
    errorText = tr( "Request is for an optional operation that is not supported by the server." );
  }
  else if ( seCode == "NoMatch"_L1 )
  {
    QString locator = e.attribute( u"locator"_s );
    if ( locator == "time"_L1 )
      errorText = tr( "Request contains a time value that does not match any available layer in the server." );
    else
      errorText = tr( "Request contains some parameter values that do not match any available layer in the server" );
  }
  else if ( seCode.isEmpty() )
  {
    errorText = tr( "(No error code was reported)" );
  }
  else
  {
    errorText = seCode + ' ' + tr( "(Unknown error code)" );
  }

  errorText += '\n' + tr( "The WMS vendor also reported: " );
  errorText += seText;

  // TODO = e.attribute("locator");

  QgsDebugMsgLevel( u"exiting with composed error message '%1'."_s.arg( errorText ), 2 );
}

QgsRectangle QgsWmsProvider::extent() const
{
  if ( mExtentDirty )
  {
    if ( calculateExtent() )
    {
      mExtentDirty = false;
    }
  }

  return mLayerExtent;
}

bool QgsWmsProvider::isValid() const
{
  return mValid;
}

QString QgsWmsProvider::wmsVersion()
{
  // TODO
  return QString();
}


QStringList QgsWmsProvider::subLayers() const
{
  return mSettings.mActiveSubLayers;
}


QStringList QgsWmsProvider::subLayerStyles() const
{
  return mSettings.mActiveSubStyles;
}

bool QgsWmsProvider::calculateExtent() const
{
  //! \todo Make this handle non-geographic CRSs (e.g. std::floor plans) as per WMS spec


  if ( mSettings.mTiled )
  {
    if ( mTileLayer )
    {
      int i;
      for ( i = 0; i < mTileLayer->boundingBoxes.size() && mTileLayer->boundingBoxes[i].crs != mImageCrs; i++ )
      {
        QgsDebugMsgLevel( u"Skip %1 [%2]"_s.arg( mTileLayer->boundingBoxes.at( i ).crs, mImageCrs ), 2 );
      }

      if ( i < mTileLayer->boundingBoxes.size() )
      {
        mLayerExtent = mTileLayer->boundingBoxes[i].box;
      }
      else
      {
        QgsCoordinateReferenceSystem qgisSrsDest = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mImageCrs );

        // pick the first that transforms fin(it)e
        for ( i = 0; i < mTileLayer->boundingBoxes.size(); i++ )
        {
          QgsCoordinateReferenceSystem qgisSrsSource = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mTileLayer->boundingBoxes[i].crs );

          QgsDebugMsgLevel( u"ct: %1 => %2"_s.arg( mTileLayer->boundingBoxes.at( i ).crs, mImageCrs ), 2 );

          try
          {
            QgsCoordinateTransform ct( qgisSrsSource, qgisSrsDest, transformContext() );
            ct.setBallparkTransformsAreAppropriate( true );
            QgsRectangle extent = ct.transformBoundingBox( mTileLayer->boundingBoxes.at( i ).box, Qgis::TransformDirection::Forward );

            //make sure extent does not contain 'inf' or 'nan'
            if ( extent.isFinite() )
            {
              mLayerExtent = extent;
              break;
            }
          }
          catch ( QgsCsException &cse )
          {
            Q_UNUSED( cse )
          }
        }
      }

      QgsDebugMsgLevel( "exiting with '" + mLayerExtent.toString() + "'.", 3 );

      return true;
    }

    QgsDebugMsgLevel( u"no extent returned"_s, 2 );
    return false;
  }
  else
  {
    bool firstLayer = true; //flag to know if a layer is the first to be successfully transformed
    for ( QStringList::const_iterator it = mSettings.mActiveSubLayers.constBegin();
          it != mSettings.mActiveSubLayers.constEnd();
          ++it )
    {
      QgsDebugMsgLevel( "Sublayer iterator: " + *it, 2 );

      QgsRectangle extent;
      if ( !extentForNonTiledLayer( *it, mImageCrs, extent ) )
      {
        QgsDebugMsgLevel( "extent for " + *it + " is invalid! (ignoring)", 2 );
        continue;
      }

      QgsDebugMsgLevel( "extent for " + *it + " is " + extent.toString( 3 ) + '.', 2 );

      // add to the combined extent of all the active sublayers
      if ( firstLayer )
      {
        mLayerExtent = extent;
      }
      else
      {
        mLayerExtent.combineExtentWith( extent );
      }

      firstLayer = false;

      QgsDebugMsgLevel( "combined extent is '" + mLayerExtent.toString() + "' after '" + ( *it ) + "'.", 2 );
    }

    QgsDebugMsgLevel( "exiting with '" + mLayerExtent.toString() + "'.", 2 );
    return true;
  }
}


Qgis::RasterInterfaceCapabilities QgsWmsProvider::capabilities() const
{
  Qgis::RasterInterfaceCapabilities capability = Qgis::RasterInterfaceCapability::NoCapabilities;
  bool canIdentify = false;

  if ( mSettings.mTiled && mTileLayer )
  {
    QgsDebugMsgLevel( u"Tiled."_s, 2 );
    canIdentify = !mTileLayer->getFeatureInfoURLs.isEmpty() || !getFeatureInfoUrl().isNull();
  }
  else
  {
    QgsDebugMsgLevel( u"Not tiled."_s, 2 );
    // Test for the ability to use the Identify map tool
    for ( QStringList::const_iterator it = mSettings.mActiveSubLayers.begin();
          it != mSettings.mActiveSubLayers.end();
          ++it )
    {
      // Is sublayer visible?
      if ( mActiveSubLayerVisibility.find( *it ).value() )
      {
        // Is sublayer queryable?
        if ( mCaps.mQueryableForLayer.find( *it ).value() )
        {
          QgsDebugMsgLevel( '\'' + ( *it ) + "' is queryable.", 2 );
          canIdentify = true;
        }
      }
    }
  }

  if ( canIdentify )
  {
    capability = mCaps.identifyCapabilities();
    if ( capability )
    {
      capability |= Qgis::RasterInterfaceCapability::Identify;
    }
  }

  bool enablePrefetch = QgsSettingsRegistryCore::settingsEnableWMSTilePrefetching->value();
  if ( mSettings.mXyz || enablePrefetch )
  {
    capability |= Qgis::RasterInterfaceCapability::Prefetch;
  }

  QgsDebugMsgLevel( u"capability = %1"_s.arg( static_cast<int>( capability ) ), 2 );
  return capability;
}

QString QgsWmsProvider::layerMetadata( const QgsWmsLayerProperty &layer ) const
{
  QString metadata =
    // Layer Properties section
    // Use a nested table
    QStringLiteral( "<tr><td>"
                    "<table width=\"100%\" class=\"tabular-view\">"

                    // Table header
                    "<tr><th class=\"strong\">" )
    % tr( "Property" ) % QStringLiteral( "</th>"
                                         "<th class=\"strong\">" )
    % tr( "Value" ) % QStringLiteral( "</th></tr>"

                                      // Name
                                      "<tr><td>" )
    % tr( "Name" ) % QStringLiteral( "</td>"
                                     "<td>" )
    % layer.name % QStringLiteral( "</td></tr>"

                                   // Layer Visibility (as managed by this provider)
                                   "<tr><td>" )
    % tr( "Visibility" ) % QStringLiteral( "</td>"
                                           "<td>" )
    % ( mActiveSubLayerVisibility.find( layer.name ).value() ? tr( "Visible" ) : tr( "Hidden" ) ) % QStringLiteral( "</td></tr>"

                                                                                                                    // Layer Title
                                                                                                                    "<tr><td>" )
    % tr( "Title" ) % QStringLiteral( "</td>"
                                      "<td>" )
    % layer.title % QStringLiteral( "</td></tr>"

                                    // Layer Abstract
                                    "<tr><td>" )
    % tr( "Abstract" ) % QStringLiteral( "</td>"
                                         "<td>" )
    % layer.abstract % QStringLiteral( "</td></tr>"

                                       // Layer Queryability
                                       "<tr><td>" )
    % tr( "Can Identify" ) % QStringLiteral( "</td>"
                                             "<td>" )
    % ( layer.queryable ? tr( "Yes" ) : tr( "No" ) ) % QStringLiteral( "</td></tr>"

                                                                       // Layer Opacity
                                                                       "<tr><td>" )
    % tr( "Can be Transparent" ) % QStringLiteral( "</td>"
                                                   "<td>" )
    % ( layer.opaque ? tr( "No" ) : tr( "Yes" ) ) % QStringLiteral( "</td></tr>"

                                                                    // Layer Subsetability
                                                                    "<tr><td>" )
    % tr( "Can Zoom In" ) % QStringLiteral( "</td>"
                                            "<td>" )
    % ( layer.noSubsets ? tr( "No" ) : tr( "Yes" ) ) % QStringLiteral( "</td></tr>"

                                                                       // Layer Server Cascade Count
                                                                       "<tr><td>" )
    % tr( "Cascade Count" ) % QStringLiteral( "</td>"
                                              "<td>" )
    % QString::number( layer.cascaded ) % QStringLiteral( "</td></tr>"

                                                          // Layer Fixed Width
                                                          "<tr><td>" )
    % tr( "Fixed Width" ) % QStringLiteral( "</td>"
                                            "<td>" )
    % QString::number( layer.fixedWidth ) % QStringLiteral( "</td></tr>"

                                                            // Layer Fixed Height
                                                            "<tr><td>" )
    % tr( "Fixed Height" ) % QStringLiteral( "</td>"
                                             "<td>" )
    % QString::number( layer.fixedHeight ) % u"</td></tr>"_s;

  // Dimensions
  if ( !layer.dimensions.isEmpty() )
  {
    metadata += u"<tr><th>"_s % tr( "Dimensions" ) % QStringLiteral( "</th>"
                                                                     "<td><table class=\"tabular-view\">"
                                                                     "<tr><th>" )
                % tr( "Name" ) % u"</th><th>"_s % tr( "Unit" ) % u"</th><th>"_s % tr( "Extent" ) % u"</th></tr>"_s;

    for ( const QgsWmsDimensionProperty &d : std::as_const( layer.dimensions ) )
    {
      metadata += u"<tr><td>"_s % d.name % u"</td><td>"_s % d.units % u"</td><td>"_s % d.extent % u"</td></tr>"_s;
    }
    metadata += QStringLiteral( "</table>"
                                "</td></tr>" );
  }
  // Metadata URLs
  if ( !layer.metadataUrl.isEmpty() )
  {
    metadata += u"<tr><th>"_s % tr( "Metadata URLs" ) % QStringLiteral( "</th>"
                                                                        "<td><table class=\"tabular-view\">"
                                                                        "<tr><th>" )
                % tr( "Format" ) % u"</th><th>"_s % tr( "URL" ) % u"</th></tr>"_s;

    for ( const QgsWmsMetadataUrlProperty &l : std::as_const( layer.metadataUrl ) )
    {
      metadata += u"<tr><td>"_s % l.format % u"</td><td>"_s % l.onlineResource.xlinkHref % u"</td></tr>"_s;
    }
    metadata += QStringLiteral( "</table>"
                                "</td></tr>" );
  }

  // Layer Coordinate Reference Systems
  for ( int j = 0; j < std::min( static_cast<int>( layer.crs.size() ), 10 ); j++ )
  {
    metadata += u"<tr><td>"_s % tr( "Available in CRS" ) % QStringLiteral( "</td>"
                                                                           "<td>" )
                % layer.crs[j] % u"</td></tr>"_s;
  }

  if ( layer.crs.size() > 10 )
  {
    metadata += u"<tr><td>"_s % tr( "Available in CRS" ) % QStringLiteral( "</td>"
                                                                           "<td>" )
                % tr( "(and %n more)", "crs", layer.crs.size() - 10 ) % u"</td></tr>"_s;
  }

  // Layer Styles
  for ( int j = 0; j < layer.style.size(); j++ )
  {
    const QgsWmsStyleProperty &style = layer.style.at( j );

    metadata += u"<tr><td>"_s % tr( "Available in style" ) % QStringLiteral( "</td>"
                                                                             "<td>" )
                %

                // Nested table.
                QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">"

                                // Layer Style Name
                                "<tr><th class=\"strong\">" )
                % tr( "Name" ) % QStringLiteral( "</th>"
                                                 "<td>" )
                % style.name % QStringLiteral( "</td></tr>"

                                               // Layer Style Title
                                               "<tr><th class=\"strong\">" )
                % tr( "Title" ) % QStringLiteral( "</th>"
                                                  "<td>" )
                % style.title % QStringLiteral( "</td></tr>"

                                                // Layer Style Abstract
                                                "<tr><th class=\"strong\">" )
                % tr( "Abstract" ) % QStringLiteral( "</th>"
                                                     "<td>" )
                % style.abstract % u"</td></tr>"_s;

    // LegendURLs
    if ( !style.legendUrl.isEmpty() )
    {
      metadata += u"<tr><th class=\"strong\">"_s % tr( "LegendURLs" ) % QStringLiteral( "</th>"
                                                                                        "<td><table class=\"tabular-view\">"
                                                                                        "<tr><th>Format</th><th>URL</th></tr>" );
      for ( int k = 0; k < style.legendUrl.size(); k++ )
      {
        const QgsWmsLegendUrlProperty &l = style.legendUrl[k];
        metadata += u"<tr><td>"_s % l.format % u"</td><td>"_s % l.onlineResource.xlinkHref % u"</td></tr>"_s;
      }
      metadata += "</table></td></tr>"_L1;
    }

    // Close the nested table
    metadata += QStringLiteral( "</table>"
                                "</td></tr>" );
  }

  // Close the nested table
  metadata += QStringLiteral( "</table>"
                              "</td></tr>" );
  return metadata;
}

QString QgsWmsProvider::htmlMetadata() const
{
  QString metadata;

  metadata += u"<tr><td class=\"highlight\">"_s % tr( "WMS Info" ) % u"</td><td><div>"_s;

  if ( !mSettings.mTiled )
  {
    // Use also HTML anchors for use in QTextBrowser / mMetadataTextBrowser https://github.com/qgis/QGIS/issues/39689
    metadata += u"&nbsp;<a href=\"#selectedlayers\" onclick=\"document.getElementById('selectedlayers').scrollIntoView(); return false;\">"_s % tr( "Selected Layers" ) % u"</a>&nbsp;<a href=\"#otherlayers\" onclick=\"document.getElementById('otherlayers').scrollIntoView(); return false;\">"_s % tr( "Other Layers" ) % u"</a>"_s;
  }
  else
  {
    metadata += u"&nbsp;<a href=\"#tilesetproperties\" onclick=\"document.getElementById('tilesetproperties').scrollIntoView(); return false;\">"_s % tr( "Tile Layer Properties" ) % QStringLiteral( "</a> "
                                                                                                                                                                                                      "&nbsp;<a href=\"#cachestats\" onclick=\"document.getElementById('cachestats').scrollIntoView(); return false;\">" )
                % tr( "Cache Stats" ) % u"</a> "_s;
  }

  metadata += QStringLiteral( "<br /><table class=\"tabular-view\">" // Nested table 1
                              // Server Properties section
                              "<tr><th class=\"strong\" id=\"serverproperties\">" )
              % tr( "Server Properties" ) % QStringLiteral( "</th></tr>"

                                                            // Use a nested table
                                                            "<tr><td>"
                                                            "<table width=\"100%\" class=\"tabular-view\">" ); // Nested table 2

  // Table header
  metadata += u"<tr><th class=\"strong\">"_s % tr( "Property" ) % QStringLiteral( "</th>"
                                                                                  "<th class=\"strong\">" )
              % tr( "Value" ) % u"</th></tr>"_s;

  // WMS Version
  metadata += u"<tr><td>"_s % tr( "WMS Version" ) % QStringLiteral( "</td>"
                                                                    "<td>" )
              % mCaps.mCapabilities.version % u"</td></tr>"_s;

  // Service Title
  metadata += u"<tr><td>"_s % tr( "Title" ) % QStringLiteral( "</td>"
                                                              "<td>" )
              % mCaps.mCapabilities.service.title % u"</td></tr>"_s;

  // Service Abstract
  metadata += u"<tr><td>"_s % tr( "Abstract" ) % QStringLiteral( "</td>"
                                                                 "<td>" )
              % mCaps.mCapabilities.service.abstract % u"</td></tr>"_s;

  // Service Keywords
  metadata += u"<tr><td>"_s % tr( "Keywords" ) % QStringLiteral( "</td>"
                                                                 "<td>" )
              % mCaps.mCapabilities.service.keywordList.join( "<br />"_L1 ) % u"</td></tr>"_s;

  // Service Online Resource
  metadata += u"<tr><td>"_s % tr( "Online Resource" ) % QStringLiteral( "</td>"
                                                                        "<td>" )
              % '-' % u"</td></tr>"_s;

  // Service Contact Information
  metadata += u"<tr><td>"_s % tr( "Contact Person" ) % QStringLiteral( "</td>"
                                                                       "<td>" )
              % mCaps.mCapabilities.service.contactInformation.contactPersonPrimary.contactPerson % u"<br />"_s % mCaps.mCapabilities.service.contactInformation.contactPosition % u"<br />"_s % mCaps.mCapabilities.service.contactInformation.contactPersonPrimary.contactOrganization % u"</td></tr>"_s;

  // Service Fees
  metadata += u"<tr><td>"_s % tr( "Fees" ) % QStringLiteral( "</td>"
                                                             "<td>" )
              % mCaps.mCapabilities.service.fees % u"</td></tr>"_s;

  // Service Access Constraints
  metadata += u"<tr><td>"_s % tr( "Access Constraints" ) % QStringLiteral( "</td>"
                                                                           "<td>" )
              % mCaps.mCapabilities.service.accessConstraints % u"</td></tr>"_s;

  // Base URL
  metadata += u"<tr><td>"_s % tr( "GetCapabilitiesUrl" ) % QStringLiteral( "</td>"
                                                                           "<td>" )
              % mSettings.mBaseUrl % u"</td></tr>"_s;

  metadata += u"<tr><td>"_s % tr( "GetMapUrl" ) % QStringLiteral( "</td>"
                                                                  "<td>" )
              % getMapUrl() % ( mSettings.mIgnoreGetMapUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : QString() ) % u"</td></tr>"_s;

  metadata += u"<tr><td>"_s % tr( "GetFeatureInfoUrl" ) % QStringLiteral( "</td>"
                                                                          "<td>" )
              % getFeatureInfoUrl() % ( mSettings.mIgnoreGetFeatureInfoUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : QString() ) % u"</td></tr>"_s;

  metadata += u"<tr><td>"_s % tr( "GetLegendGraphic" ) % QStringLiteral( "</td>"
                                                                         "<td>" )
              % getLegendGraphicUrl() % ( mSettings.mIgnoreGetMapUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : QString() ) % u"</td></tr>"_s;

  if ( mSettings.mTiled )
  {
    metadata += u"<tr><td>"_s % tr( "Tile Layer Count" ) % QStringLiteral( "</td>"
                                                                           "<td>" )
                % QString::number( mCaps.mTileLayersSupported.size() ) % QStringLiteral( "</td></tr>"
                                                                                         "<tr><td>" )
                % tr( "GetTileUrl" ) % QStringLiteral( "</td>"
                                                       "<td>" )
                % getTileUrl() % u"</td></tr>"_s;

    if ( mTileLayer )
    {
      metadata += u"<tr><td>"_s % tr( "Tile templates" ) % QStringLiteral( "</td>"
                                                                           "<td>" );
      for ( QHash<QString, QString>::const_iterator it = mTileLayer->getTileURLs.constBegin();
            it != mTileLayer->getTileURLs.constEnd();
            ++it )
      {
        metadata += u"%1:%2<br>"_s.arg( it.key(), it.value() );
      }
      metadata += QStringLiteral( "</td></tr>"

                                  "<tr><td>" )
                  % tr( "FeatureInfo templates" ) % QStringLiteral( "</td>"
                                                                    "<td>" );
      for ( QHash<QString, QString>::const_iterator it = mTileLayer->getFeatureInfoURLs.constBegin();
            it != mTileLayer->getFeatureInfoURLs.constEnd();
            ++it )
      {
        metadata += u"%1:%2<br>"_s.arg( it.key(), it.value() );
      }
      metadata += "</td></tr>"_L1;

      // GetFeatureInfo Request Formats
      metadata += u"<tr><td>"_s % tr( "Identify Formats" ) % QStringLiteral( "</td>"
                                                                             "<td>" )
                  % mTileLayer->infoFormats.join( "<br />"_L1 ) % u"</td></tr>"_s;
    }
  }
  else
  {
    // GetMap Request Formats
    metadata += u"<tr><td>"_s % tr( "Image Formats" ) % QStringLiteral( "</td>"
                                                                        "<td>" )
                % mCaps.mCapabilities.capability.request.getMap.format.join( "<br />"_L1 ) % QStringLiteral( "</td></tr>"

                                                                                                             // GetFeatureInfo Request Formats
                                                                                                             "<tr><td>" )
                % tr( "Identify Formats" ) % QStringLiteral( "</td>"
                                                             "<td>" )
                % mCaps.mCapabilities.capability.request.getFeatureInfo.format.join( "<br />"_L1 ) % QStringLiteral( "</td></tr>"

                                                                                                                     // Layer Count (as managed by this provider)
                                                                                                                     "<tr><td>" )
                % tr( "Layer Count" ) % QStringLiteral( "</td>"
                                                        "<td>" )
                % QString::number( mCaps.mLayersSupported.size() ) % u"</td></tr>"_s;
  }

  // Close the nested table 2
  metadata += QStringLiteral( "</table>"
                              "</td></tr>" );

  // Layer properties
  if ( !mSettings.mTiled )
  {
    metadata += u"<tr><th class=\"strong\" id=\"selectedlayers\">"_s % tr( "Selected Layers" ) % u"</th></tr>"_s;

    int n = 0;
    for ( int i = 0; i < mCaps.mLayersSupported.size(); i++ )
    {
      if ( mSettings.mActiveSubLayers.contains( mCaps.mLayersSupported.at( i ).name ) )
      {
        metadata += layerMetadata( mCaps.mLayersSupported[i] );
        n++;
      }
    } // for each layer

    // Layer properties
    if ( n < mCaps.mLayersSupported.size() )
    {
      metadata += u"<tr><th class=\"strong\" id=\"otherlayers\">"_s % tr( "Other Layers" ) % u"</th></tr>"_s;

      for ( int i = 0; i < mCaps.mLayersSupported.size(); i++ )
      {
        if ( !mSettings.mActiveSubLayers.contains( mCaps.mLayersSupported[i].name ) )
        {
          metadata += layerMetadata( mCaps.mLayersSupported[i] );
        }
      } // for each layer
    }
  }
  else
  {
    // Tileset properties
    metadata += u"<tr><th class=\"strong\" id=\"tilesetproperties\">"_s % tr( "Tileset Properties" ) % QStringLiteral( "</th></tr>"

                                                                                                                       // Iterate through tilesets
                                                                                                                       "<tr><td>"

                                                                                                                       "<table width=\"100%\" class=\"tabular-view\">" ); // Nested table 3

    for ( const QgsWmtsTileLayer &l : std::as_const( mCaps.mTileLayersSupported ) )
    {
      metadata += u"<tr><th class=\"strong\">"_s % tr( "Identifier" ) % u"</th><th class=\"strong\">"_s % tr( "Tile mode" ) % QStringLiteral( "</th></tr>"

                                                                                                                                              "<tr><td>" )
                  % l.identifier % u"</td><td class=\"strong\">"_s;

      if ( l.tileMode == WMTS )
      {
        metadata += tr( "WMTS" );
      }
      else if ( l.tileMode == WMSC )
      {
        metadata += tr( "WMS-C" );
      }
      else if ( l.tileMode == XYZ )
      {
        metadata += tr( "XYZ" );
      }
      else
      {
        metadata += tr( "Invalid tile mode" );
      }

      metadata += QStringLiteral( "</td></tr>"

                                  // Table header
                                  "<tr><th class=\"strong\">" )
                  % tr( "Property" ) % QStringLiteral( "</th>"
                                                       "<th class=\"strong\">" )
                  % tr( "Value" ) % QStringLiteral( "</th></tr>"

                                                    "<tr><td class=\"strong\">" )
                  % tr( "Title" ) % QStringLiteral( "</td>"
                                                    "<td>" )
                  % l.title % QStringLiteral( "</td></tr>"

                                              "<tr><td class=\"strong\">" )
                  % tr( "Abstract" ) % QStringLiteral( "</td>"
                                                       "<td>" )
                  % l.abstract % QStringLiteral( "</td></tr>"

                                                 "<tr><td class=\"strong\">" )
                  % tr( "Selected" ) % QStringLiteral( "</td>"
                                                       "<td class=\"strong\">" )
                  % ( l.identifier == mSettings.mActiveSubLayers.join( ','_L1 ) ? tr( "Yes" ) : tr( "No" ) ) % u"</td></tr>"_s;

      if ( !l.styles.isEmpty() )
      {
        metadata += u"<tr><td class=\"strong\">"_s % tr( "Available Styles" ) % QStringLiteral( "</td>"
                                                                                                "<td class=\"strong\">" );
        QStringList styles;
        for ( const QgsWmtsStyle &style : std::as_const( l.styles ) )
        {
          styles << style.identifier;
        }
        metadata += styles.join( ", "_L1 ) % u"</td></tr>"_s;
      }

      metadata += u"<tr><td class=\"strong\">"_s % tr( "CRS" ) % QStringLiteral( "</td>"
                                                                                 "<td>"
                                                                                 "<table class=\"tabular-view\"><tr>" // Nested table 4
                                                                                 "<td class=\"strong\">" )
                  % tr( "CRS" ) % QStringLiteral( "</td>"
                                                  "<td class=\"strong\">" )
                  % tr( "Bounding Box" ) % u"</td>"_s;
      for ( int i = 0; i < l.boundingBoxes.size(); i++ )
      {
        metadata += u"<tr><td>"_s % l.boundingBoxes[i].crs % u"</td><td>"_s % l.boundingBoxes[i].box.toString() % u"</td></tr>"_s;
      }
      metadata += QStringLiteral( "</table></td></tr>" // End nested table 4
                                  "<tr><td class=\"strong\">" )
                  % tr( "Available Tilesets" ) % u"</td><td class=\"strong\">"_s;

      for ( const QgsWmtsTileMatrixSetLink &setLink : std::as_const( l.setLinks ) )
      {
        metadata += setLink.tileMatrixSet + "<br>";
      }

      metadata += "</td></tr>"_L1;
    }

    metadata += "</table></td></tr>"_L1; // End nested table 3

    if ( mTileMatrixSet )
    {
      // Iterate through tilesets
      metadata += QStringLiteral( "<tr><td><table width=\"100%\" class=\"tabular-view\">" // Nested table 3

                                  "<tr><th colspan=14 class=\"strong\">%1 %2</th></tr>"
                                  "<tr>"
                                  "<th rowspan=2 class=\"strong\">%3</th>"
                                  "<th colspan=2 class=\"strong\">%4</th>"
                                  "<th colspan=2 class=\"strong\">%5</th>"
                                  "<th colspan=2 class=\"strong\">%6</th>"
                                  "<th colspan=2 class=\"strong\">%7</th>"
                                  "<th colspan=4 class=\"strong\">%8</th>"
                                  "</tr><tr>"
                                  "<th class=\"strong\">%9</th><th class=\"strong\">%10</th>"
                                  "<th class=\"strong\">%9</th><th class=\"strong\">%10</th>"
                                  "<th class=\"strong\">%9</th><th class=\"strong\">%10</th>"
                                  "<th class=\"strong\">%9</th><th class=\"strong\">%10</th>"
                                  "<th class=\"strong\">%11</th>"
                                  "<th class=\"strong\">%12</th>"
                                  "<th class=\"strong\">%13</th>"
                                  "<th class=\"strong\">%14</th>"
                                  "</tr>" )
                    .arg( tr( "Selected tile matrix set " ), mSettings.mTileMatrixSetId, tr( "Scale" ), tr( "Tile size [px]" ), tr( "Tile size [mu]" ), tr( "Matrix size" ), tr( "Matrix extent [mu]" ) )
                    .arg( tr( "Bounds" ), tr( "Width" ), tr( "Height" ), tr( "Top" ), tr( "Left" ), tr( "Bottom" ), tr( "Right" ) );

      for ( const double key : mNativeResolutions )
      {
        QgsWmtsTileMatrix &tm = mTileMatrixSet->tileMatrices[key];

        double tw = key * tm.tileWidth;
        double th = key * tm.tileHeight;

        QgsRectangle r( tm.topLeft.x(), tm.topLeft.y() - tw * tm.matrixWidth, tm.topLeft.x() + th * tm.matrixHeight, tm.topLeft.y() );

        metadata += QStringLiteral( "<tr>"
                                    "<td>%1</td>"
                                    "<td>%2</td><td>%3</td>"
                                    "<td>%4</td><td>%5</td>"
                                    "<td>%6</td><td>%7</td>"
                                    "<td>%8</td><td>%9</td>" )
                      .arg( tm.scaleDenom )
                      .arg( tm.tileWidth )
                      .arg( tm.tileHeight )
                      .arg( tw )
                      .arg( th )
                      .arg( tm.matrixWidth )
                      .arg( tm.matrixHeight )
                      .arg( tw * tm.matrixWidth, 0, 'f' )
                      .arg( th * tm.matrixHeight, 0, 'f' );

        // top
        if ( mLayerExtent.yMaximum() > r.yMaximum() )
        {
          metadata += u"<td title=\"%1<br>%2\"><font color=\"red\">%3</font></td>"_s
                        .arg( tr( "%n missing row(s)", nullptr, ( int ) std::ceil( ( mLayerExtent.yMaximum() - r.yMaximum() ) / th ) ), tr( "Layer's upper bound: %1" ).arg( mLayerExtent.yMaximum(), 0, 'f' ) )
                        .arg( r.yMaximum(), 0, 'f' );
        }
        else
        {
          metadata += u"<td>%1</td>"_s.arg( r.yMaximum(), 0, 'f' );
        }

        // left
        if ( mLayerExtent.xMinimum() < r.xMinimum() )
        {
          metadata += u"<td title=\"%1<br>%2\"><font color=\"red\">%3</font></td>"_s
                        .arg( tr( "%n missing column(s)", nullptr, ( int ) std::ceil( ( r.xMinimum() - mLayerExtent.xMinimum() ) / tw ) ), tr( "Layer's left bound: %1" ).arg( mLayerExtent.xMinimum(), 0, 'f' ) )
                        .arg( r.xMinimum(), 0, 'f' );
        }
        else
        {
          metadata += u"<td>%1</td>"_s.arg( r.xMinimum(), 0, 'f' );
        }

        // bottom
        if ( mLayerExtent.yMaximum() > r.yMaximum() )
        {
          metadata += u"<td title=\"%1<br>%2\"><font color=\"red\">%3</font></td>"_s
                        .arg( tr( "%n missing row(s)", nullptr, ( int ) std::ceil( ( mLayerExtent.yMaximum() - r.yMaximum() ) / th ) ), tr( "Layer's lower bound: %1" ).arg( mLayerExtent.yMaximum(), 0, 'f' ) )
                        .arg( r.yMaximum(), 0, 'f' );
        }
        else
        {
          metadata += u"<td>%1</td>"_s.arg( r.yMaximum(), 0, 'f' );
        }

        // right
        if ( mLayerExtent.xMaximum() > r.xMaximum() )
        {
          metadata += u"<td title=\"%1<br>%2\"><font color=\"red\">%3</font></td>"_s
                        .arg( tr( "%n missing column(s)", nullptr, ( int ) std::ceil( ( mLayerExtent.xMaximum() - r.xMaximum() ) / tw ) ), tr( "Layer's right bound: %1" ).arg( mLayerExtent.xMaximum(), 0, 'f' ) )
                        .arg( r.xMaximum(), 0, 'f' );
        }
        else
        {
          metadata += u"<td>%1</td>"_s.arg( r.xMaximum(), 0, 'f' );
        }

        metadata += "</tr>"_L1;
      }

      metadata += "</table></td></tr>"_L1; // End nested table 3
    }

    const QgsWmsStatistics::Stat stat = QgsWmsStatistics::statForUri( dataSourceUri() );

    metadata += u"<tr><th class=\"strong\" id=\"cachestats\">"_s % tr( "Cache stats" ) % QStringLiteral( "</th></tr>"

                                                                                                         "<tr><td><table width=\"100%\" class=\"tabular-view\">" // Nested table 3

                                                                                                         "<tr><th class=\"strong\">" )
                % tr( "Property" ) % QStringLiteral( "</th>"
                                                     "<th class=\"strong\">" )
                % tr( "Value" ) % QStringLiteral( "</th></tr>"

                                                  "<tr><td>" )
                % tr( "Hits" ) % u"</td><td>"_s % QString::number( stat.cacheHits ) % QStringLiteral( "</td></tr>"

                                                                                                      "<tr><td>" )
                % tr( "Misses" ) % u"</td><td>"_s % QString::number( stat.cacheMisses ) % QStringLiteral( "</td></tr>"

                                                                                                          "<tr><td>" )
                % tr( "Errors" ) % u"</td><td>"_s % QString::number( stat.errors ) % QStringLiteral( "</td></tr>"

                                                                                                     "</table></td></tr>" ); // End nested table 3
  }

  metadata += QStringLiteral( "</table>"                      // End nested table 2
                              "</table></div></td></tr>\n" ); // End nested table 1

  return metadata;
}

double QgsWmsProvider::sample( const QgsPointXY &point, int band, bool *ok, const QgsRectangle &boundingBox, int width, int height, int dpi )
{
  if ( ok )
    *ok = false;

  Qgis::DataType bandDataType = sourceDataType( band );
  if ( mSettings.mTiled && mTileMatrixSet && ( bandDataType != Qgis::DataType::UnknownDataType && bandDataType != Qgis::DataType::ARGB32 && bandDataType != Qgis::DataType::ARGB32_Premultiplied ) )
  {
    const double maximumNativeResolution = mNativeResolutions.at( 0 );
    const double xMin = point.x() - std::fmod( point.x(), maximumNativeResolution );
    const double yMin = point.y() - std::fmod( point.y(), maximumNativeResolution );
    QgsRectangle rect( xMin, yMin, xMin + maximumNativeResolution, yMin + maximumNativeResolution );


    std::unique_ptr<QgsRasterBlock> b( block( band, rect, 1, 1 ) );
    if ( b->isValid() )
    {
      bool isNoData = true;
      const double value = b->valueAndNoData( 0, 0, isNoData );
      if ( !isNoData )
      {
        if ( ok )
          *ok = true;

        return value;
      }
    }

    return std::numeric_limits<double>::quiet_NaN();
  }

  return QgsRasterDataProvider::sample( point, band, ok, boundingBox, width, height, dpi );
}

QgsRasterIdentifyResult QgsWmsProvider::identify( const QgsPointXY &point, Qgis::RasterIdentifyFormat format, const QgsRectangle &boundingBox, int width, int height, int /*dpi*/ )
{
  QgsDebugMsgLevel( u"format = %1"_s.arg( qgsEnumValueToKey( format ) ), 2 );

  QString formatStr;
  formatStr = mCaps.mIdentifyFormats.value( format );
  if ( formatStr.isEmpty() )
  {
    return QgsRasterIdentifyResult( QGS_ERROR( tr( "Format not supported" ) ) );
  }

  QgsDebugMsgLevel( u"format = %1 format = %2"_s.arg( qgsEnumValueToKey( format ) ).arg( formatStr ), 2 );

  QMap<int, QVariant> results;
  if ( !extent().contains( point ) )
  {
    results.insert( 1, "" );
    return QgsRasterIdentifyResult( format, results );
  }

  QgsRectangle myExtent = boundingBox;

  if ( !myExtent.isEmpty() )
  {
    // we cannot reliably identify WMS if boundingBox is specified but width or theHeight
    // are not, because we don't know original resolution
    if ( width == 0 || height == 0 )
    {
      return QgsRasterIdentifyResult( QGS_ERROR( tr( "Context not fully specified (extent was defined but width and/or height was not)." ) ) );
    }
  }
  else // context (boundingBox, width, height) not defined
  {
    // We don't know original source resolution, so we take some small extent around the point.

    // Warning: this does not work well with poin/line vector layers where search rectangle
    // is based on pixel size (e.g. UMN Mapserver is using TOLERANCE layer param)

    double xRes = 0.001; // expecting meters

    // TODO: add CRS as class member
    QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mImageCrs );
    if ( crs.isValid() )
    {
      // set resolution approximately to 1mm
      switch ( crs.mapUnits() )
      {
        case Qgis::DistanceUnit::Meters:
          xRes = 0.001;
          break;
        case Qgis::DistanceUnit::Feet:
          xRes = 0.003;
          break;
        case Qgis::DistanceUnit::Degrees:
          // max length of degree of latitude on pole is 111694 m
          xRes = 1e-8;
          break;
        default:
          xRes = 0.001; // expecting meters
      }
    }

    // Keep resolution in both axis equal! Otherwise silly server (like QGIS mapserver)
    // fail to calculate coordinate because it is using single resolution average!!!
    double yRes = xRes;

    // 1x1 should be sufficient but at least we know that GDAL ECW was very unefficient
    // so we use 2x2 (until we find that it is too small for some server)
    width = height = 2;

    myExtent = QgsRectangle( point.x() - xRes, point.y() - yRes, point.x() + xRes, point.y() + yRes );
  }

  // Point in BBOX/WIDTH/HEIGHT coordinates
  // No need to fiddle with extent origin not covered by layer extent, I believe
  double xRes = myExtent.width() / width;
  double yRes = myExtent.height() / height;


  // Mapserver (6.0.3, for example) does not seem to work with 1x1 pixel box
  // (seems to be a different issue, not the slownes of GDAL with ECW mentioned above)
  // so we have to enlarge it a bit
  if ( width == 1 )
  {
    width += 1;
    myExtent.setXMaximum( myExtent.xMaximum() + xRes );
  }

  if ( height == 1 )
  {
    height += 1;
    myExtent.setYMaximum( myExtent.yMaximum() + yRes );
  }

  QgsDebugMsgLevel( "myExtent = " + myExtent.toString(), 2 );
  QgsDebugMsgLevel( u"theWidth = %1 height = %2"_s.arg( width ).arg( height ), 2 );
  QgsDebugMsgLevel( u"xRes = %1 yRes = %2"_s.arg( xRes ).arg( yRes ), 2 );

  QgsPointXY finalPoint;
  finalPoint.setX( std::floor( ( point.x() - myExtent.xMinimum() ) / xRes ) );
  finalPoint.setY( std::floor( ( myExtent.yMaximum() - point.y() ) / yRes ) );

  QgsDebugMsgLevel( u"point = %1 %2"_s.arg( finalPoint.x() ).arg( finalPoint.y() ), 2 );

  QgsDebugMsgLevel( u"recalculated orig point (corner) = %1 %2"_s.arg( myExtent.xMinimum() + finalPoint.x() * xRes ).arg( myExtent.yMaximum() - finalPoint.y() * yRes ), 2 );

  // Collect which layers to query on
  //according to the WMS spec for 1.3, the order of x - and y - coordinates is inverted for geographical CRS
  bool changeXY = mCaps.shouldInvertAxisOrientation( mImageCrs );

  // compose the URL query string for the WMS server.
  QString crsKey = u"SRS"_s; //SRS in 1.1.1 and CRS in 1.3.0
  if ( mCaps.mCapabilities.version == "1.3.0"_L1 || mCaps.mCapabilities.version == "1.3"_L1 )
  {
    crsKey = u"CRS"_s;
  }

  // Compose request to WMS server
  QString bbox = QString( changeXY ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
                   .arg( qgsDoubleToString( myExtent.xMinimum() ), qgsDoubleToString( myExtent.yMinimum() ), qgsDoubleToString( myExtent.xMaximum() ), qgsDoubleToString( myExtent.yMaximum() ) );

  //QgsFeatureList featureList;

  QList<QUrl> urls;
  QStringList layerList;

  if ( !mSettings.mTiled )
  {
    // Test for which layers are suitable for querying with
    for ( QStringList::const_iterator
            layers
          = mSettings.mActiveSubLayers.constBegin(),
          styles = mSettings.mActiveSubStyles.constBegin();
          layers != mSettings.mActiveSubLayers.constEnd();
          ++layers, ++styles )
    {
      // Is sublayer visible?
      if ( !mActiveSubLayerVisibility.find( *layers ).value() )
      {
        // TODO: something better?
        // we need to keep all sublayers so that we can get their names in identify tool
        results.insert( urls.size(), false );
        continue;
      }


      // Is sublayer queryable?
      if ( !mCaps.mQueryableForLayer.find( *layers ).value() )
      {
        results.insert( urls.size(), false );
        continue;
      }

      QgsDebugMsgLevel( "Layer '" + *layers + "' is queryable.", 2 );

      QUrl requestUrl( mSettings.mIgnoreGetFeatureInfoUrl ? mSettings.mBaseUrl : getFeatureInfoUrl() );
      QUrlQuery query( requestUrl );
      setQueryItem( query, u"SERVICE"_s, u"WMS"_s );
      setQueryItem( query, u"VERSION"_s, mCaps.mCapabilities.version );
      setQueryItem( query, u"REQUEST"_s, u"GetFeatureInfo"_s );
      setQueryItem( query, u"BBOX"_s, bbox );
      setSRSQueryItem( query );
      setQueryItem( query, u"WIDTH"_s, QString::number( width ) );
      setQueryItem( query, u"HEIGHT"_s, QString::number( height ) );
      setQueryItem( query, u"LAYERS"_s, *layers );
      setQueryItem( query, u"STYLES"_s, *styles );
      setFormatQueryItem( query );
      setQueryItem( query, u"QUERY_LAYERS"_s, *layers );
      setQueryItem( query, u"INFO_FORMAT"_s, formatStr );

      if ( mCaps.mCapabilities.version == "1.3.0"_L1 || mCaps.mCapabilities.version == "1.3"_L1 )
      {
        setQueryItem( query, u"I"_s, QString::number( finalPoint.x() ) );
        setQueryItem( query, u"J"_s, QString::number( finalPoint.y() ) );
      }
      else
      {
        setQueryItem( query, u"X"_s, QString::number( finalPoint.x() ) );
        setQueryItem( query, u"Y"_s, QString::number( finalPoint.y() ) );
      }

      if ( mSettings.mFeatureCount > 0 )
      {
        setQueryItem( query, u"FEATURE_COUNT"_s, QString::number( mSettings.mFeatureCount ) );
      }

      // For WMS-T layers
      if ( temporalCapabilities() && temporalCapabilities()->hasTemporalCapabilities() )
      {
        addWmstParameters( query );
      }

      requestUrl.setQuery( query );

      layerList << *layers;
      urls << requestUrl;
    }
  }
  else if ( mTileLayer && mTileLayer->tileMode == WMTS )
  {
    // WMTS FeatureInfo
    double vres = boundingBox.width() / width;
    double tres = vres;

    const QgsWmtsTileMatrix *tm = nullptr;

    Q_ASSERT( mTileMatrixSet );
    Q_ASSERT( !mTileMatrixSet->tileMatrices.isEmpty() );

    QMap<double, QgsWmtsTileMatrix> &m = mTileMatrixSet->tileMatrices;

    // find nearest resolution
    QMap<double, QgsWmtsTileMatrix>::const_iterator prev, it = m.constBegin();
    while ( it != m.constEnd() && it.key() < vres )
    {
      QgsDebugMsgLevel( u"res:%1 >= %2"_s.arg( it.key() ).arg( vres ), 2 );
      prev = it;
      ++it;
    }

    if ( it == m.constEnd() || ( it != m.constBegin() && vres - prev.key() < it.key() - vres ) )
    {
      QgsDebugMsgLevel( u"back to previous res"_s, 2 );
      it = prev;
    }

    tres = it.key();
    tm = &it.value();

    QgsDebugMsgLevel( u"layer extent: %1,%2,%3,%4 %5x%6"_s.arg( qgsDoubleToString( mLayerExtent.xMinimum() ), qgsDoubleToString( mLayerExtent.yMinimum() ) ).arg( qgsDoubleToString( mLayerExtent.xMaximum() ), qgsDoubleToString( mLayerExtent.yMaximum() ) ).arg( mLayerExtent.width() ).arg( mLayerExtent.height() ), 2 );

    QgsDebugMsgLevel( u"view extent: %1,%2,%3,%4 %5x%6  res:%7"_s.arg( qgsDoubleToString( boundingBox.xMinimum() ), qgsDoubleToString( boundingBox.yMinimum() ) ).arg( qgsDoubleToString( boundingBox.xMaximum() ), qgsDoubleToString( boundingBox.yMaximum() ) ).arg( boundingBox.width() ).arg( boundingBox.height() ).arg( vres, 0, 'f' ), 2 );

    QgsDebugMsgLevel( u"tile matrix %1,%2 res:%3 tilesize:%4x%5 matrixsize:%6x%7 id:%8"_s.arg( tm->topLeft.x() ).arg( tm->topLeft.y() ).arg( tres ).arg( tm->tileWidth ).arg( tm->tileHeight ).arg( tm->matrixWidth ).arg( tm->matrixHeight ).arg( tm->identifier ), 2 );

    // calculate tile coordinates
    double twMap = tm->tileWidth * tres;
    double thMap = tm->tileHeight * tres;
    QgsDebugMsgLevel( u"tile map size: %1,%2"_s.arg( qgsDoubleToString( twMap ), qgsDoubleToString( thMap ) ), 2 );

    int col = ( int ) std::floor( ( point.x() - tm->topLeft.x() ) / twMap );
    int row = ( int ) std::floor( ( tm->topLeft.y() - point.y() ) / thMap );
    double tx = tm->topLeft.x() + col * twMap;
    double ty = tm->topLeft.y() - row * thMap;
    int i = ( point.x() - tx ) / tres;
    int j = ( ty - point.y() ) / tres;

    QgsDebugMsgLevel( u"col=%1 row=%2 i=%3 j=%4 tx=%5 ty=%6"_s.arg( col ).arg( row ).arg( i ).arg( j ).arg( tx, 0, 'f', 1 ).arg( ty, 0, 'f', 1 ), 2 );

    if ( mTileLayer->getFeatureInfoURLs.contains( formatStr ) )
    {
      // REST


      QString url = mTileLayer->getFeatureInfoURLs[formatStr];

      if ( mSettings.mIgnoreGetFeatureInfoUrl )
      {
        // rewrite the URL if the one in the capabilities document is incorrect
        // strip every thing after the ? from the base url
        const QStringList parts = mSettings.mBaseUrl.split( QRegularExpression( "\\?" ) );
        const QString base = parts.isEmpty() ? mSettings.mBaseUrl : parts.first();
        // and strip everything before the `rest` element (at least for GeoServer)
        const int index = url.length() - url.lastIndexOf( "rest"_L1 ) + 1; // +1 for the /
        url = base + url.right( index );
      }

      QgsDebugMsgLevel( u"getfeatureinfo: %1"_s.arg( url ), 2 );

      url.replace( "{layer}"_L1, mSettings.mActiveSubLayers[0], Qt::CaseInsensitive );
      url.replace( "{style}"_L1, mSettings.mActiveSubStyles[0], Qt::CaseInsensitive );
      url.replace( "{tilematrixset}"_L1, mTileMatrixSet->identifier, Qt::CaseInsensitive );
      url.replace( "{tilematrix}"_L1, tm->identifier, Qt::CaseInsensitive );
      url.replace( "{tilerow}"_L1, QString::number( row ), Qt::CaseInsensitive );
      url.replace( "{tilecol}"_L1, QString::number( col ), Qt::CaseInsensitive );
      url.replace( "{i}"_L1, QString::number( i ), Qt::CaseInsensitive );
      url.replace( "{j}"_L1, QString::number( j ), Qt::CaseInsensitive );

      for ( QHash<QString, QString>::const_iterator it = mSettings.mTileDimensionValues.constBegin(); it != mSettings.mTileDimensionValues.constEnd(); ++it )
      {
        url.replace( "{" + it.key() + "}", it.value(), Qt::CaseInsensitive );
      }

      urls << QUrl( url );
      layerList << mSettings.mActiveSubLayers[0];
    }
    else if ( !getFeatureInfoUrl().isNull() )
    {
      // KVP
      QUrl url( mSettings.mIgnoreGetFeatureInfoUrl ? mSettings.mBaseUrl : getFeatureInfoUrl() );
      QUrlQuery query( url );

      // compose static request arguments.
      setQueryItem( query, u"SERVICE"_s, u"WMTS"_s );
      setQueryItem( query, u"REQUEST"_s, u"GetFeatureInfo"_s );
      setQueryItem( query, u"VERSION"_s, mCaps.mCapabilities.version );
      setQueryItem( query, u"LAYER"_s, mSettings.mActiveSubLayers[0] );
      setQueryItem( query, u"STYLE"_s, mSettings.mActiveSubStyles[0] );
      setQueryItem( query, u"INFOFORMAT"_s, formatStr );
      setQueryItem( query, u"TILEMATRIXSET"_s, mTileMatrixSet->identifier );
      setQueryItem( query, u"TILEMATRIX"_s, tm->identifier );

      for ( QHash<QString, QString>::const_iterator it = mSettings.mTileDimensionValues.constBegin(); it != mSettings.mTileDimensionValues.constEnd(); ++it )
      {
        setQueryItem( query, it.key(), it.value() );
      }

      setQueryItem( query, u"TILEROW"_s, QString::number( row ) );
      setQueryItem( query, u"TILECOL"_s, QString::number( col ) );
      setQueryItem( query, u"I"_s, qgsDoubleToString( i ) );
      setQueryItem( query, u"J"_s, qgsDoubleToString( j ) );
      url.setQuery( query );

      urls << url;
      layerList << mSettings.mActiveSubLayers[0];
    }
    else
    {
      QgsDebugError( u"No KVP and no feature info url for format %1"_s.arg( formatStr ) );
    }
  }

  for ( int count = 0; count < urls.size(); count++ )
  {
    const QUrl &requestUrl = urls[count];

    QgsDebugMsgLevel( u"getfeatureinfo: %1"_s.arg( requestUrl.toString() ), 2 );
    QNetworkRequest request( requestUrl );
    request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );
    QgsSetRequestInitiatorClass( request, u"QgsWmsProvider"_s );
    QgsSetRequestInitiatorId( request, u"identify %1,%2"_s.arg( point.x() ).arg( point.y() ) );
    mSettings.authorization().setAuthorization( request );
    mIdentifyReply = QgsNetworkAccessManager::instance()->get( request );
    connect( mIdentifyReply, &QNetworkReply::finished, this, &QgsWmsProvider::identifyReplyFinished );

    QEventLoop loop;
    mIdentifyReply->setProperty( "eventLoop", QVariant::fromValue( qobject_cast<QObject *>( &loop ) ) );
    loop.exec( QEventLoop::ExcludeUserInputEvents );

    if ( mIdentifyResultBodies.isEmpty() ) // no result
    {
      QgsDebugError( u"mIdentifyResultBodies is empty"_s );
      continue;
    }
    else if ( mIdentifyResultBodies.size() == 1 )
    {
      // Check for service exceptions (exceptions with ogr/gml are in the body)
      bool isXml = false;
      bool isGml = false;

      const QgsNetworkReplyParser::RawHeaderMap &headers = mIdentifyResultHeaders.value( 0 );
      for ( auto it = headers.constBegin(); it != headers.constEnd(); ++it )
      {
        if ( QString( it.key() ).compare( "Content-Type"_L1, Qt::CaseInsensitive ) == 0 )
        {
          isXml = QString( it.value() ).compare( "text/xml"_L1, Qt::CaseInsensitive ) == 0;
          isGml = QString( it.value() ).compare( "ogr/gml"_L1, Qt::CaseInsensitive ) == 0;
          if ( isXml || isGml )
            break;
        }
      }

      if ( isGml || isXml )
      {
        QByteArray body = mIdentifyResultBodies.value( 0 );

        if ( isGml && body.startsWith( "Content-Type: text/xml\r\n\r\n" ) )
        {
          body = body.data() + strlen( "Content-Type: text/xml\r\n\r\n" );
          isXml = true;
        }

        if ( isXml && parseServiceExceptionReportDom( body, mErrorCaption, mError ) )
        {
          if ( !mError.isEmpty() ) //xml is not necessarily an exception
          {
            QgsMessageLog::logMessage( tr( "Get feature info request error (Title: %1; Error: %2; URL: %3)" ).arg( mErrorCaption, mError, requestUrl.toString() ), tr( "WMS" ) );
            continue;
          }
        }
      }
    }

    if ( format == Qgis::RasterIdentifyFormat::Html || format == Qgis::RasterIdentifyFormat::Text )
    {
      results.insert( results.size(), QString::fromUtf8( mIdentifyResultBodies.value( 0 ) ) );
    }
    else if ( format == Qgis::RasterIdentifyFormat::Feature ) // GML
    {
      // The response maybe
      // 1) simple GML
      //    To get also geometry from UMN Mapserver, it must be enabled for layer, e.g.:
      //      LAYER
      //        METADATA
      //          "ows_geometries" "mygeom"
      //          "ows_mygeom_type" "polygon"
      //        END
      //      END

      // 2) multipart GML + XSD
      //    Multipart is supplied by UMN Mapserver following format is used
      //      OUTPUTFORMAT
      //        NAME "OGRGML"
      //        DRIVER "OGR/GML"
      //        FORMATOPTION "FORM=multipart"
      //      END
      //      WEB
      //        METADATA
      //          "wms_getfeatureinfo_formatlist" "OGRGML,text/html"
      //        END
      //      END
      //    GetFeatureInfo multipart response does not seem to be defined in
      //    OGC specification.

      int gmlPart = -1;
      int xsdPart = -1;
      int jsonPart = -1;

      // Headers are case insensitive

      for ( int i = 0; i < mIdentifyResultHeaders.size(); i++ )
      {
        QgsNetworkReplyParser::RawHeaderMap identifyResultHeader;
        for ( auto it = mIdentifyResultHeaders.at( i ).cbegin(); it != mIdentifyResultHeaders.at( i ).cend(); ++it )
        {
          identifyResultHeader.insert( it.key().toLower(), it.value() );
        }

        if ( xsdPart == -1 && identifyResultHeader.value( "content-disposition" ).contains( ".xsd" ) )
        {
          xsdPart = i;
        }
        else if ( gmlPart == -1 && identifyResultHeader.value( "content-disposition" ).contains( ".dat" ) )
        {
          gmlPart = i;
        }
        else if ( jsonPart == -1 && identifyResultHeader.value( "content-type" ).contains( "json" ) )
        {
          jsonPart = i;
        }

        if ( gmlPart != -1 && xsdPart != -1 && jsonPart != -1 )
          break;
      }

      if ( xsdPart == -1 && gmlPart == -1 && jsonPart == -1 )
      {
        if ( mIdentifyResultBodies.size() == 1 ) // GML
        {
          gmlPart = 0;
        }
        if ( mIdentifyResultBodies.size() == 2 ) // GML+XSD
        {
          QgsDebugMsgLevel( u"Multipart with 2 parts - expected GML + XSD"_s, 2 );
          // How to find which part is GML and which XSD? Both have
          // Content-Type: application/binary
          // different are Content-Disposition but it is not reliable.
          // We could analyze beginning of bodies...
          gmlPart = 0;
          xsdPart = 1;
        }
      }
      QgsDebugMsgLevel( u"jsonPart = %1 gmlPart = %2 xsdPart = %3"_s.arg( jsonPart ).arg( gmlPart ).arg( xsdPart ), 2 );

      if ( gmlPart >= 0 )
      {
        QByteArray gmlByteArray = mIdentifyResultBodies.value( gmlPart );
        QgsDebugMsgLevel( "GML (first 2000 bytes):\n" + gmlByteArray.left( 2000 ), 2 );

        // QgsGmlSchema.guessSchema() and QgsGml::getFeatures() are using Expat
        // which only accepts UTF-8, UTF-16, ISO-8859-1
        // http://sourceforge.net/p/expat/bugs/498/
        QDomDocument dom;
        dom.setContent( gmlByteArray ); // gets XML encoding
        gmlByteArray.clear();
        QTextStream stream( &gmlByteArray );
        dom.save( stream, 4, QDomNode::EncodingFromTextStream );

        QgsDebugMsgLevel( "GML UTF-8 (first 2000 bytes):\n" + gmlByteArray.left( 2000 ), 2 );

        Qgis::WkbType wkbType;
        QgsGmlSchema gmlSchema;

        if ( xsdPart >= 0 ) // XSD available
        {
          QgsDebugMsgLevel( "GML XSD (first 4000 bytes):\n" + QString::fromUtf8( mIdentifyResultBodies.value( xsdPart ).left( 4000 ) ), 2 );
          gmlSchema.parseXSD( mIdentifyResultBodies.value( xsdPart ) );
        }
        else
        {
          // guess from GML
          bool ok = gmlSchema.guessSchema( gmlByteArray );
          if ( !ok )
          {
            QgsError err = gmlSchema.error();
            err.append( tr( "Cannot identify" ) );
            QgsDebugError( "guess schema error: " + err.message() );
            return QgsRasterIdentifyResult( err );
          }
        }

        const QStringList featureTypeNames = gmlSchema.typeNames();
        QgsDebugMsgLevel( u"%1 featureTypeNames found"_s.arg( featureTypeNames.size() ), 2 );

        // Each sublayer may have more features of different types, for example
        // if GROUP of multiple vector layers is used with UMN MapServer
        // Note: GROUP of layers in UMN MapServer is not queryable by default
        // (and I could not find a way to force it), it is possible however
        // to add another RASTER layer with the same name as group which is queryable
        // and has no DATA defined. Then such a layer may be add to QGIS and both
        // GetMap and GetFeatureInfo will return data for the group of the same name.
        // https://github.com/mapserver/mapserver/issues/318#issuecomment-4923208
        QgsFeatureStoreList featureStoreList;
        for ( const QString &featureTypeName : featureTypeNames )
        {
          QgsDebugMsgLevel( u"featureTypeName = %1"_s.arg( featureTypeName ), 2 );

          QString geometryAttribute = gmlSchema.geometryAttributes( featureTypeName ).value( 0 );
          QList<QgsField> fieldList = gmlSchema.fields( featureTypeName );
          QgsDebugMsgLevel( u"%1 fields found"_s.arg( fieldList.size() ), 2 );
          QgsFields fields;
          for ( int i = 0; i < fieldList.size(); i++ )
          {
            fields.append( fieldList[i] );
          }
          QgsGml gml( featureTypeName, geometryAttribute, fields );
          // TODO: avoid converting to string and back
          int ret = gml.getFeatures( gmlByteArray, &wkbType );
#ifdef QGISDEBUG
          QgsDebugMsgLevel( u"parsing result = %1"_s.arg( ret ), 2 );
#else
          Q_UNUSED( ret )
#endif
          // TODO: all features coming from this layer should probably have the same CRS
          // the same as this layer, because layerExtentToOutputExtent() may be used
          // for results -> verify CRS and reprojects if necessary
          QMap<QgsFeatureId, QgsFeature *> features = gml.featuresMap();
          QgsCoordinateReferenceSystem featuresCrs = gml.crs();
          QgsDebugMsgLevel( u"%1 features read, crs: %2"_s.arg( features.size() ).arg( featuresCrs.userFriendlyIdentifier() ), 2 );
          QgsCoordinateTransform coordinateTransform;
          if ( featuresCrs.isValid() && featuresCrs != crs() )
          {
            coordinateTransform = QgsCoordinateTransform( featuresCrs, crs(), transformContext() );
          }
          QgsFeatureStore featureStore( fields, crs() );
          QMap<QString, QVariant> params;
          params.insert( u"sublayer"_s, layerList[count] );
          params.insert( u"featureType"_s, featureTypeName );
          params.insert( u"getFeatureInfoUrl"_s, requestUrl.toString() );
          featureStore.setParams( params );
          QMap<QgsFeatureId, QgsFeature *>::const_iterator featIt = features.constBegin();
          for ( ; featIt != features.constEnd(); ++featIt )
          {
            QgsFeature *feature = featIt.value();

            QgsDebugMsgLevel( u"feature id = %1 : %2 attributes"_s.arg( featIt.key() ).arg( feature->attributeCount() ), 2 );

            if ( coordinateTransform.isValid() && feature->hasGeometry() )
            {
              QgsGeometry g = feature->geometry();
              g.transform( coordinateTransform );
              feature->setGeometry( g );
            }
            featureStore.addFeature( *feature );
          }
          featureStoreList.append( featureStore );
        }
        // It is suspicious if we guessed feature types from GML but could not get
        // features from it. Either we geuessed wrong schema or parsing features failed.
        // Report it as error so that user can switch to another format in results dialog.
        if ( xsdPart < 0 && !featureTypeNames.isEmpty() && featureStoreList.isEmpty() )
        {
          QgsError err = QGS_ERROR( tr( "Cannot identify" ) );
          err.append( tr( "Result parsing failed. %n feature type(s) were guessed from gml (%2) but no features were parsed.", nullptr, featureTypeNames.size() ).arg( featureTypeNames.join( ','_L1 ) ) );
          QgsDebugError( "parsing GML error: " + err.message() );
          return QgsRasterIdentifyResult( err );
        }
        results.insert( results.size(), QVariant::fromValue( featureStoreList ) );
      }
      else if ( jsonPart != -1 )
      {
        QString json = QString::fromUtf8( mIdentifyResultBodies.value( jsonPart ) );

        QgsFeatureStoreList featureStoreList;
        QgsCoordinateTransform coordinateTransform;

        try
        {
          QJsonDocument doc = QJsonDocument::fromJson( json.toUtf8() );
          if ( doc.isNull() )
            throw u"Doc expected"_s;
          if ( !doc.isObject() )
            throw u"Object expected"_s;

          QJsonObject result = doc.object();
          if ( result.value( "type"_L1 ).toString() != "FeatureCollection"_L1 )
            throw u"Type FeatureCollection expected: %1"_s.arg( result.value( "type"_L1 ).toString() );

          if ( result.value( "crs"_L1 ).isObject() )
          {
            QString crsType = result.value( "crs"_L1 ).toObject().value( "type"_L1 ).toString();
            QString crsText;
            if ( crsType == "name"_L1 )
              crsText = result.value( u"crs"_s ).toObject().value( "properties"_L1 ).toObject().value( "name"_L1 ).toString();
            else if ( crsType == "EPSG"_L1 )
              crsText = u"%1:%2"_s.arg( crsType, result.value( "crs"_L1 ).toObject().value( "properties"_L1 ).toObject().value( u"code"_s ).toString() );
            else
            {
              QgsDebugError( u"crs not supported:%1"_s.arg( result.value( "crs"_L1 ).toString() ) );
            }

            QgsCoordinateReferenceSystem featuresCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsText );

            if ( !featuresCrs.isValid() )
              throw u"CRS %1 invalid"_s.arg( crsText );

            if ( featuresCrs.isValid() && featuresCrs != crs() )
            {
              coordinateTransform = QgsCoordinateTransform( featuresCrs, crs(), transformContext() );
            }
          }

          const QJsonValue fc = result.value( "features"_L1 );
          if ( !fc.isArray() )
            throw u"FeatureCollection array expected"_s;

          const QJsonArray features = fc.toArray();

          int i = -1;
          for ( const QJsonValue &fv : features )
          {
            ++i;
            const QJsonObject f = fv.toObject();
            const QJsonValue props = f.value( "properties"_L1 );
            if ( !props.isObject() )
            {
              QgsDebugMsgLevel( u"no properties found"_s, 2 );
              continue;
            }

            QgsFields fields;

            const QJsonObject properties = props.toObject();
            auto fieldIterator = properties.constBegin();

            for ( ; fieldIterator != properties.constEnd(); ++fieldIterator )
            {
              fields.append( QgsField( fieldIterator.key(), QMetaType::Type::QString ) );
            }

            QgsFeature feature( fields );

            if ( f.value( "geometry"_L1 ).isObject() )
            {
              QJsonDocument serializer( f.value( "geometry"_L1 ).toObject() );
              QString geom = serializer.toJson( QJsonDocument::JsonFormat::Compact );

              gdal::ogr_geometry_unique_ptr ogrGeom( OGR_G_CreateGeometryFromJson( geom.toUtf8() ) );
              if ( ogrGeom )
              {
                int wkbSize = OGR_G_WkbSize( ogrGeom.get() );
                unsigned char *wkb = new unsigned char[wkbSize];
                OGR_G_ExportToWkb( ogrGeom.get(), ( OGRwkbByteOrder ) QgsApplication::endian(), wkb );

                QgsGeometry g;
                g.fromWkb( wkb, wkbSize );
                feature.setGeometry( g );

                if ( coordinateTransform.isValid() && feature.hasGeometry() )
                {
                  QgsGeometry transformed = feature.geometry();
                  transformed.transform( coordinateTransform );
                  feature.setGeometry( transformed );
                }
              }
            }

            int j = 0;
            fieldIterator = properties.constBegin();
            for ( ; fieldIterator != properties.constEnd(); ++fieldIterator )
            {
              feature.setAttribute( j++, fieldIterator.value().toVariant() );
            }

            QgsFeatureStore featureStore( fields, crs() );

            QVariantMap params;
            params.insert( u"sublayer"_s, layerList[count] );
            params.insert( u"featureType"_s, u"%1_%2"_s.arg( count ).arg( i ) );
            params.insert( u"getFeatureInfoUrl"_s, requestUrl.toString() );
            featureStore.setParams( params );

            // Try to parse and set feature id if matches "<some string>.<integer>"
            if ( f.value( "id"_L1 ).isString() )
            {
              const thread_local QRegularExpression re { R"raw(\.(\d+)$)raw" };
              const QString idVal { f.value( "id"_L1 ).toString() };
              const QRegularExpressionMatch match { re.match( idVal ) };
              if ( match.hasMatch() )
              {
                bool ok;
                QgsFeatureId id { match.captured( 1 ).toLongLong( &ok ) };
                if ( ok )
                {
                  feature.setId( id );
                }
              }
            }

            feature.setValid( true );
            featureStore.addFeature( feature );

            featureStoreList.append( featureStore );
          }
        }
        catch ( const QString &err )
        {
          QgsDebugError( u"JSON error: %1\nResult: %2"_s.arg( err, QString::fromUtf8( mIdentifyResultBodies.value( jsonPart ) ) ) );
          results.insert( results.size(), err ); // string returned for format type "feature" means error
        }

        results.insert( results.size(), QVariant::fromValue( featureStoreList ) );
      }
    }
  }

  return QgsRasterIdentifyResult( format, results );
}

void QgsWmsProvider::identifyReplyFinished()
{
  QgsDebugMsgLevel( u"Entered."_s, 4 );
  mIdentifyResultHeaders.clear();
  mIdentifyResultBodies.clear();

  QEventLoop *loop = qobject_cast<QEventLoop *>( sender()->property( "eventLoop" ).value<QObject *>() );

  if ( mIdentifyReply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = mIdentifyReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !QgsVariantUtils::isNull( redirect ) )
    {
      QgsDebugMsgLevel( u"identify request redirected to %1"_s.arg( redirect.toString() ), 2 );

      mIdentifyReply->deleteLater();

      QgsDebugMsgLevel( u"redirected getfeatureinfo: %1"_s.arg( redirect.toString() ), 2 );
      QNetworkRequest request( redirect.toUrl() );
      request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );
      mIdentifyReply = QgsNetworkAccessManager::instance()->get( request );
      mSettings.authorization().setAuthorizationReply( mIdentifyReply );
      mIdentifyReply->setProperty( "eventLoop", QVariant::fromValue( qobject_cast<QObject *>( loop ) ) );
      connect( mIdentifyReply, &QNetworkReply::finished, this, &QgsWmsProvider::identifyReplyFinished );
      return;
    }

    QVariant status = mIdentifyReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !QgsVariantUtils::isNull( status ) && status.toInt() >= 400 )
    {
      QVariant phrase = mIdentifyReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
      mErrorFormat = u"text/plain"_s;
      mError = tr( "Map getfeatureinfo error %1: %2" ).arg( status.toInt() ).arg( phrase.toString() );
      emit statusChanged( mError );
    }

    QgsNetworkReplyParser parser( mIdentifyReply );
    if ( !parser.isValid() )
    {
      QgsDebugError( u"Cannot parse reply"_s );
      mErrorFormat = u"text/plain"_s;
      mError = tr( "Cannot parse getfeatureinfo: %1" ).arg( parser.error() );
      emit statusChanged( mError );
    }
    else
    {
      // TODO: check headers, xsd ...
      QgsDebugMsgLevel( u"%1 parts"_s.arg( parser.parts() ), 2 );
      mIdentifyResultBodies = parser.bodies();
      mIdentifyResultHeaders = parser.headers();
    }
  }
  else
  {
    //mIdentifyResult = tr( "ERROR: GetFeatureInfo failed" );
    mErrorFormat = u"text/plain"_s;
    mError = tr( "Map getfeatureinfo error: %1 [%2]" ).arg( mIdentifyReply->errorString(), mIdentifyReply->url().toString() );
    emit statusChanged( mError );
    QgsMessageLog::logMessage( mError, tr( "WMS" ) );
  }

  if ( loop )
    QMetaObject::invokeMethod( loop, "quit", Qt::QueuedConnection );

  mIdentifyReply->deleteLater();
  mIdentifyReply = nullptr;
}


QgsCoordinateReferenceSystem QgsWmsProvider::crs() const
{
  return mCrs;
}

Qgis::RasterProviderCapabilities QgsWmsProvider::providerCapabilities() const
{
  Qgis::RasterProviderCapabilities capabilities;
  if ( mConverter )
    capabilities = Qgis::RasterProviderCapability::ReadLayerMetadata | Qgis::RasterProviderCapability::ProviderHintBenefitsFromResampling | Qgis::RasterProviderCapability::ProviderHintCanPerformProviderResampling;
  else
    capabilities = Qgis::RasterProviderCapability::ReadLayerMetadata;

  if ( mSettings.mTiled || mSettings.mXyz )
  {
    capabilities |= Qgis::RasterProviderCapability::DpiDependentData;
  }

  return capabilities;
}

QString QgsWmsProvider::lastErrorTitle()
{
  return mErrorCaption;
}


QString QgsWmsProvider::lastError()
{
  return mError;
}

QString QgsWmsProvider::lastErrorFormat()
{
  return mErrorFormat;
}

QString QgsWmsProvider::name() const
{
  return WMS_KEY;
}

QString QgsWmsProvider::providerKey()
{
  return WMS_KEY;
}

QString QgsWmsProvider::description() const
{
  return WMS_DESCRIPTION;
}

bool QgsWmsProvider::renderInPreview( const QgsDataProvider::PreviewContext &context )
{
  if ( mSettings.mTiled || mSettings.mXyz )
    return true;

  return QgsRasterDataProvider::renderInPreview( context );
}

QList<double> QgsWmsProvider::nativeResolutions() const
{
  return mNativeResolutions;
}

QgsLayerMetadata QgsWmsProvider::layerMetadata() const
{
  return mLayerMetadata;
}

QSize QgsWmsProvider::maximumTileSize() const
{
  const int capsMaxHeight { static_cast<int>( mCaps.mCapabilities.service.maxHeight ) };
  const int capsMaxWidth { static_cast<int>( mCaps.mCapabilities.service.maxWidth ) };

  if ( mSettings.mMaxHeight > 0 && mSettings.mMaxWidth > 0 )
  {
    if ( capsMaxHeight > 0 && capsMaxWidth > 0 )
    {
      return QSize( std::min( mSettings.mMaxWidth, capsMaxWidth ), std::min( mSettings.mMaxHeight, capsMaxHeight ) );
    }
    else
    {
      return QSize( mSettings.mMaxWidth, mSettings.mMaxHeight );
    }
  }
  else if ( capsMaxHeight > 0 && capsMaxWidth > 0 )
  {
    return QSize( capsMaxWidth, capsMaxHeight );
  }
  else if ( mSettings.mStepWidth > 0 && mSettings.mStepHeight > 0 ) //The chosen step size can be higher than the default max size
  {
    return QSize( mSettings.mStepWidth, mSettings.mStepHeight );
  }
  else // default fallback
  {
    return QgsRasterDataProvider::maximumTileSize();
  }
}

QgsRasterBandStats QgsWmsProvider::bandStatistics(
  int bandNo,
  Qgis::RasterBandStatistics stats,
  const QgsRectangle &extent,
  int sampleSize,
  QgsRasterBlockFeedback *feedback
)
{
  if ( mConverter )
    return mConverter->statistics( bandNo, stats, extent, sampleSize, feedback );
  else
    return QgsRasterBandStats();
}

QgsRasterHistogram QgsWmsProvider::histogram(
  int bandNo,
  int binCount,
  double minimum,
  double maximum,
  const QgsRectangle &extent,
  int sampleSize,
  bool includeOutOfRange,
  QgsRasterBlockFeedback *feedback
)
{
  if ( mConverter )
    return mConverter->histogram( bandNo, binCount, minimum, maximum, extent, sampleSize, includeOutOfRange, feedback );
  else
    return QgsRasterHistogram();
}

QVector<QgsWmsSupportedFormat> QgsWmsProvider::supportedFormats()
{
  QVector<QgsWmsSupportedFormat> formats;

  QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();

  if ( supportedFormats.contains( "png" ) )
  {
    QgsWmsSupportedFormat p1 = { "image/png", "PNG" };
    QgsWmsSupportedFormat p2 = { "image/png; mode=24bit", "PNG24" }; // UMN mapserver
    QgsWmsSupportedFormat p3 = { "image/png8", "PNG8" };             // used by geoserver
    QgsWmsSupportedFormat p4 = { "image/png; mode=8bit", "PNG8" };   // used by QGIS server and UMN mapserver
    QgsWmsSupportedFormat p5 = { "png", "PNG" };                     // used by french IGN geoportail
    QgsWmsSupportedFormat p6 = { "pngt", "PNGT" };                   // used by french IGN geoportail

    formats << p1 << p2 << p3 << p4 << p5 << p6;
  }

  if ( supportedFormats.contains( "webp" ) )
  {
    QgsWmsSupportedFormat p1 = { "image/webp", "WebP" };
    formats << p1;
  }

  if ( supportedFormats.contains( "jpg" ) )
  {
    QgsWmsSupportedFormat j1 = { "image/jpeg", "JPEG" };
    QgsWmsSupportedFormat j2 = { "image/jpg", "JPEG" };
    QgsWmsSupportedFormat j3 = { "jpeg", "JPEG" }; // used by french IGN geoportail
    formats << j1 << j2 << j3;
  }

  if ( supportedFormats.contains( "png" ) && supportedFormats.contains( "jpg" ) )
  {
    QgsWmsSupportedFormat g1 = { "image/x-jpegorpng", "JPEG/PNG" }; // used by cubewerx
    QgsWmsSupportedFormat g2 = { "image/jpgpng", "JPEG/PNG" };      // used by ESRI
    formats << g1 << g2;
  }

  if ( supportedFormats.contains( "gif" ) )
  {
    QgsWmsSupportedFormat g1 = { "image/gif", "GIF" };
    formats << g1;
  }

  if ( supportedFormats.contains( "tiff" ) )
  {
    QgsWmsSupportedFormat t1 = { "image/tiff", "TIFF" };
    formats << t1;
  }

  if ( supportedFormats.contains( "svg" ) )
  {
    QgsWmsSupportedFormat s1 = { "image/svg", "SVG" };
    QgsWmsSupportedFormat s2 = { "image/svgz", "SVG" };
    QgsWmsSupportedFormat s3 = { "image/svg+xml", "SVG" };
    formats << s1 << s2 << s3;
  }

  return formats;
}

QString QgsWmsProvider::nodeAttribute( const QDomElement &e, const QString &name, const QString &defValue )
{
  if ( e.hasAttribute( name ) )
    return e.attribute( name );

  QDomNamedNodeMap map( e.attributes() );
  for ( int i = 0; i < map.size(); i++ )
  {
    QDomAttr attr( map.item( i ).toElement().toAttr() );
    if ( attr.name().compare( name, Qt::CaseInsensitive ) == 0 )
      return attr.value();
  }

  return defValue;
}

void QgsWmsProvider::showMessageBox( const QString &title, const QString &text )
{
  QgsMessageOutput *message = QgsMessageOutput::createMessageOutput();
  message->setTitle( title );
  message->setMessage( text, QgsMessageOutput::MessageText );
  message->showMessage();
}

QUrl QgsWmsProvider::getLegendGraphicFullURL( double scale, const QgsRectangle &visibleExtent )
{
  bool useContextualWMSLegend = mSettings.mEnableContextualLegend;

  QString lurl;
  if ( mSettings.mIgnoreGetMapUrl )
  {
    lurl = mSettings.mBaseUrl;
  }
  else
  {
    lurl = getLegendGraphicUrl();
  }

  if ( lurl.isEmpty() )
  {
    QgsDebugMsgLevel( u"getLegendGraphic url is empty"_s, 2 );
    return QUrl();
  }

  QgsDebugMsgLevel( u"visibleExtent is %1"_s.arg( visibleExtent.toString() ), 2 );

  QUrl url( lurl );
  QUrlQuery query( url );

  if ( isUrlForWMTS( dataSourceUri() ) )
  {
    QgsDebugMsgLevel( QString( "getlegendgraphicrequest: %1" ).arg( url.toString() ), 2 );
    return url;
  }

  // query names are NOT case-sensitive, so make an uppercase list for proper comparison
  QStringList qnames = QStringList();
  for ( int i = 0; i < query.queryItems().size(); i++ )
  {
    qnames << query.queryItems().at( i ).first.toUpper();
  }
  if ( !qnames.contains( u"SERVICE"_s ) )
    setQueryItem( query, u"SERVICE"_s, u"WMS"_s );
  if ( !qnames.contains( u"VERSION"_s ) )
    setQueryItem( query, u"VERSION"_s, mCaps.mCapabilities.version );
  if ( !qnames.contains( u"SLD_VERSION"_s ) )
    setQueryItem( query, u"SLD_VERSION"_s, u"1.1.0"_s ); // can not determine SLD_VERSION
  if ( !qnames.contains( u"REQUEST"_s ) )
    setQueryItem( query, u"REQUEST"_s, u"GetLegendGraphic"_s );
  if ( !qnames.contains( u"FORMAT"_s ) )
    setFormatQueryItem( query );
  if ( !qnames.contains( u"LAYER"_s ) )
    setQueryItem( query, u"LAYER"_s, mSettings.mActiveSubLayers[0] );
  if ( !qnames.contains( u"STYLE"_s ) )
    setQueryItem( query, u"STYLE"_s, mSettings.mActiveSubStyles[0] );
  // by setting TRANSPARENT=true, even too big legend images will look good
  if ( !qnames.contains( u"TRANSPARENT"_s ) )
    setQueryItem( query, u"TRANSPARENT"_s, u"true"_s );

  // add config parameter related to resolution
  QgsSettings s;
  int defaultLegendGraphicResolution = s.value( u"qgis/defaultLegendGraphicResolution"_s, 0 ).toInt();
  QgsDebugMsgLevel( u"defaultLegendGraphicResolution: %1"_s.arg( defaultLegendGraphicResolution ), 2 );
  if ( defaultLegendGraphicResolution )
  {
    if ( mSettings.mDpiMode & DpiQGIS )
      setQueryItem( query, u"DPI"_s, QString::number( defaultLegendGraphicResolution ) );
    if ( mSettings.mDpiMode & DpiUMN )
    {
      setQueryItem( query, u"MAP_RESOLUTION"_s, QString::number( defaultLegendGraphicResolution ) );
      setQueryItem( query, u"SCALE"_s, QString::number( scale, 'f' ) );
    }
    if ( mSettings.mDpiMode & DpiGeoServer )
    {
      setQueryItem( query, u"FORMAT_OPTIONS"_s, u"dpi:%1"_s.arg( defaultLegendGraphicResolution ) );
      setQueryItem( query, u"SCALE"_s, QString::number( scale, 'f' ) );
    }
  }

  if ( useContextualWMSLegend )
  {
    bool changeXY = mCaps.shouldInvertAxisOrientation( mImageCrs );
    setQueryItem( query, u"BBOX"_s, toParamValue( visibleExtent, changeXY ) );
    setSRSQueryItem( query );
  }
  url.setQuery( query );

  QgsDebugMsgLevel( u"getlegendgraphicrequest: %1"_s.arg( url.toString() ), 2 );
  return QUrl( url );
}

QImage QgsWmsProvider::getLegendGraphic( double scale, bool forceRefresh, const QgsRectangle *visibleExtent )
{
  // TODO manage return basing of getCapability => avoid call if service is not available
  // some services doesn't expose getLegendGraphic in capabilities but adding LegendURL in
  // the layer tags inside capabilities

  QString lurl = getLegendGraphicUrl();

  if ( lurl.isEmpty() )
  {
    QgsDebugMsgLevel( u"getLegendGraphic url is empty"_s, 2 );
    return QImage();
  }

  forceRefresh |= mGetLegendGraphicImage.isNull() || mGetLegendGraphicScale != scale;

  QgsRectangle mapExtent = visibleExtent ? *visibleExtent : extent();
  forceRefresh |= mGetLegendGraphicExtent != mapExtent;

  if ( !forceRefresh )
    return mGetLegendGraphicImage;

  mError.clear();

  QUrl url( getLegendGraphicFullURL( scale, mGetLegendGraphicExtent ) );
  if ( !url.isValid() )
    return QImage();

  Q_ASSERT( !mLegendGraphicFetcher ); // or we could just remove it instead, hopefully will cancel download
  mLegendGraphicFetcher = std::make_unique<QgsWmsLegendDownloadHandler>( *QgsNetworkAccessManager::instance(), mSettings, url );
  if ( !mLegendGraphicFetcher )
    return QImage();

  connect( mLegendGraphicFetcher.get(), &QgsWmsLegendDownloadHandler::finish, this, &QgsWmsProvider::getLegendGraphicReplyFinished );
  connect( mLegendGraphicFetcher.get(), &QgsWmsLegendDownloadHandler::error, this, &QgsWmsProvider::getLegendGraphicReplyErrored );
  connect( mLegendGraphicFetcher.get(), &QgsWmsLegendDownloadHandler::progress, this, &QgsWmsProvider::getLegendGraphicReplyProgress );
  mLegendGraphicFetcher->start();

  QEventLoop loop;
  mLegendGraphicFetcher->setProperty( "eventLoop", QVariant::fromValue( qobject_cast<QObject *>( &loop ) ) );
  mLegendGraphicFetcher->setProperty( "legendScale", QVariant::fromValue( scale ) );
  mLegendGraphicFetcher->setProperty( "legendExtent", QVariant::fromValue( mapExtent.toRectF() ) );
  loop.exec( QEventLoop::ExcludeUserInputEvents );

  return mGetLegendGraphicImage;
}

QgsImageFetcher *QgsWmsProvider::getLegendGraphicFetcher( const QgsMapSettings *mapSettings )
{
  if ( mLegendGraphicFetchErrored && !mSettings.mEnableContextualLegend )
  {
    // if a previous request to fetch the legend failed, don't bother trying to fetch it again!
    // Otherwise misconfigured services which return flawed images will just keep retrying to
    // fetch on every map extent change operation...
    return nullptr;
  }

  double scale;
  QgsRectangle mapExtent;
  if ( mapSettings && mSettings.mEnableContextualLegend )
  {
    scale = mapSettings->scale();
    mapExtent = mapSettings->visibleExtent();
    try
    {
      QgsCoordinateTransform ct { mapSettings->destinationCrs(), crs(), mapSettings->transformContext() };
      ct.setBallparkTransformsAreAppropriate( true );
      mapExtent = ct.transformBoundingBox( mapExtent );
    }
    catch ( QgsCsException & )
    {
      // Can't reproject
    }
  }
  else
  {
    scale = 0;
    mapExtent = extent();
  }

  if ( mSettings.mXyz )
  {
    // we are working with XYZ tiles: no legend graphics available
    return nullptr;
  }

  QUrl url = getLegendGraphicFullURL( scale, mapExtent );
  if ( !url.isValid() )
    return nullptr;

  if ( mapExtent == mGetLegendGraphicExtent && scale == mGetLegendGraphicScale && !mGetLegendGraphicImage.isNull() )
  {
    QgsDebugMsgLevel( u"Emitting cached image fetcher"_s, 2 );
    // return a cached image, skipping the load
    return new QgsCachedImageFetcher( mGetLegendGraphicImage );
  }
  else
  {
    QgsImageFetcher *fetcher = new QgsWmsLegendDownloadHandler( *QgsNetworkAccessManager::instance(), mSettings, url );
    fetcher->setProperty( "legendScale", QVariant::fromValue( scale ) );
    fetcher->setProperty( "legendExtent", QVariant::fromValue( mapExtent.toRectF() ) );
    connect( fetcher, &QgsImageFetcher::finish, this, &QgsWmsProvider::getLegendGraphicReplyFinished );
    connect( fetcher, &QgsImageFetcher::error, this, [this]( const QString & ) {
      mLegendGraphicFetchErrored = true;
    } );
    return fetcher;
  }
}

void QgsWmsProvider::getLegendGraphicReplyFinished( const QImage &img )
{
  QObject *reply = sender();

  if ( !img.isNull() )
  {
    mGetLegendGraphicImage = img;
    mGetLegendGraphicExtent = QgsRectangle( reply->property( "legendExtent" ).toRectF() );
    mGetLegendGraphicScale = reply->property( "legendScale" ).value<double>();

#if 0
    QString filename = QDir::tempPath() + "/GetLegendGraphic.png";
    mGetLegendGraphicImage.save( filename );
    QgsDebugMsgLevel( "saved GetLegendGraphic result in debug file: " + filename, 2 );
#endif
  }

  if ( reply == mLegendGraphicFetcher.get() )
  {
    QEventLoop *loop = qobject_cast<QEventLoop *>( reply->property( "eventLoop" ).value<QObject *>() );
    if ( loop )
      QMetaObject::invokeMethod( loop, "quit", Qt::QueuedConnection );
    mLegendGraphicFetcher.reset();
  }
}

void QgsWmsProvider::getLegendGraphicReplyErrored( const QString &message )
{
  Q_UNUSED( message )
  QgsDebugMsgLevel( u"get legend failed: %1"_s.arg( message ), 2 );

  QObject *reply = sender();

  if ( reply == mLegendGraphicFetcher.get() )
  {
    QEventLoop *loop = qobject_cast<QEventLoop *>( reply->property( "eventLoop" ).value<QObject *>() );
    if ( loop )
      QMetaObject::invokeMethod( loop, "quit", Qt::QueuedConnection );
    mLegendGraphicFetcher.reset();
  }
}

void QgsWmsProvider::getLegendGraphicReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of GetLegendGraphic downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? u"unknown number of"_s : QString::number( bytesTotal ) );
  QgsDebugMsgLevel( msg, 2 );
  emit statusChanged( msg );
}

bool QgsWmsProvider::isUrlForWMTS( const QString &url )
{
  // Do comparison in case insensitive way to match OGC KVP requirements
  return url.contains( "SERVICE=WMTS"_L1, Qt::CaseInsensitive ) || url.contains( "/WMTSCapabilities.xml"_L1, Qt::CaseInsensitive );
}

QVariantMap QgsWmsProvider::metadata() const
{
  QVariantMap metadata;
  metadata.insert( u"WmsVersion"_s, mCaps.mCapabilities.version );
  return metadata;
}


QgsWmsProvider *QgsWmsProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  Q_UNUSED( flags );
  return new QgsWmsProvider( uri, options );
}

QgsProviderMetadata::ProviderCapabilities QgsWmsProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

// -----------------

QgsWmsImageDownloadHandler::QgsWmsImageDownloadHandler( const QString &providerUri, const QUrl &url, const QgsAuthorizationSettings &auth, QImage *image, QgsRasterBlockFeedback *feedback )
  : mProviderUri( providerUri )
  , mCachedImage( image )
  , mEventLoop( new QEventLoop )
  , mFeedback( feedback )
{
  if ( feedback )
  {
    connect( feedback, &QgsFeedback::canceled, this, &QgsWmsImageDownloadHandler::canceled, Qt::QueuedConnection );

    // rendering could have been canceled before we started to listen to canceled() signal
    // so let's check before doing the download and maybe quit prematurely
    if ( feedback->isCanceled() )
      return;
  }

  QNetworkRequest request( url );
  request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );
  QgsSetRequestInitiatorClass( request, u"QgsWmsImageDownloadHandler"_s );
  auth.setAuthorization( request );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  mCacheReply = QgsNetworkAccessManager::instance()->get( request );
  connect( mCacheReply, &QNetworkReply::finished, this, &QgsWmsImageDownloadHandler::cacheReplyFinished );
  connect( mCacheReply, &QNetworkReply::downloadProgress, this, &QgsWmsImageDownloadHandler::cacheReplyProgress );

  Q_ASSERT( mCacheReply->thread() == QThread::currentThread() );
}

QgsWmsImageDownloadHandler::~QgsWmsImageDownloadHandler()
{
  delete mEventLoop;
}

void QgsWmsImageDownloadHandler::downloadBlocking()
{
  if ( mFeedback && mFeedback->isCanceled() )
    return; // nothing to do

  mEventLoop->exec( QEventLoop::ExcludeUserInputEvents );

  Q_ASSERT( !mCacheReply );
}

void QgsWmsImageDownloadHandler::cacheReplyFinished()
{
  if ( mCacheReply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = mCacheReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !QgsVariantUtils::isNull( redirect ) )
    {
      mCacheReply->deleteLater();

      QgsDebugMsgLevel( u"redirected getmap: %1"_s.arg( redirect.toString() ), 2 );
      QNetworkRequest request( redirect.toUrl() );
      request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );
      mCacheReply = QgsNetworkAccessManager::instance()->get( request );
      connect( mCacheReply, &QNetworkReply::finished, this, &QgsWmsImageDownloadHandler::cacheReplyFinished );
      return;
    }

    QVariant status = mCacheReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !QgsVariantUtils::isNull( status ) && status.toInt() >= 400 )
    {
      QVariant phrase = mCacheReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );

      QgsMessageLog::logMessage( tr( "Map request error (Status: %1; Reason phrase: %2; URL: %3)" ).arg( status.toInt() ).arg( phrase.toString(), mCacheReply->url().toString() ), tr( "WMS" ) );

      mCacheReply->deleteLater();
      mCacheReply = nullptr;

      finish();
      return;
    }

    QString contentType = mCacheReply->header( QNetworkRequest::ContentTypeHeader ).toString();
    QgsDebugMsgLevel( "contentType: " + contentType, 2 );
    QByteArray text = mCacheReply->readAll();
    QImage myLocalImage = QImage::fromData( text );
    if ( !myLocalImage.isNull() )
    {
      QPainter p( mCachedImage );
      p.drawImage( 0, 0, myLocalImage );
    }
    else if ( contentType.startsWith( "image/"_L1, Qt::CaseInsensitive ) || contentType.compare( "application/octet-stream"_L1, Qt::CaseInsensitive ) == 0 )
    {
      QgsMessageLog::logMessage( tr( "Returned image is flawed [Content-Type: %1; URL: %2]" ).arg( contentType, mCacheReply->url().toString() ), tr( "WMS" ) );
    }
    else
    {
      QString errorTitle, errorText;
      if ( contentType.compare( "text/xml"_L1, Qt::CaseInsensitive ) == 0 && QgsWmsProvider::parseServiceExceptionReportDom( text, errorTitle, errorText ) )
      {
        QgsMessageLog::logMessage( tr( "Map request error (Title: %1; Error: %2; URL: %3)" ).arg( errorTitle, errorText, mCacheReply->url().toString() ), tr( "WMS" ) );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Map request error (Status: %1; Response: %2; Content-Type: %3; URL: %4)" ).arg( status.toInt() ).arg( QString::fromUtf8( text ), contentType, mCacheReply->url().toString() ), tr( "WMS" ) );
      }

      mCacheReply->deleteLater();
      mCacheReply = nullptr;

      finish();
      return;
    }

    mCacheReply->deleteLater();
    mCacheReply = nullptr;

    finish();
  }
  else
  {
    // report any errors except for the one we have caused by canceling the request
    if ( mCacheReply->error() != QNetworkReply::OperationCanceledError )
    {
      QgsWmsStatistics::Stat &stat = QgsWmsStatistics::statForUri( mProviderUri );

      stat.errors++;
      if ( stat.errors < 100 )
      {
        QgsMessageLog::logMessage( tr( "Map request failed [error: %1 url: %2]" ).arg( mCacheReply->errorString(), mCacheReply->url().toString() ), tr( "WMS" ) );
      }
      else if ( stat.errors == 100 )
      {
        QgsMessageLog::logMessage( tr( "Not logging more than 100 request errors." ), tr( "WMS" ) );
      }
    }

    mCacheReply->deleteLater();
    mCacheReply = nullptr;

    finish();
  }
}

void QgsWmsImageDownloadHandler::cacheReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  Q_UNUSED( bytesReceived )
  Q_UNUSED( bytesTotal )
  QgsDebugMsgLevel( u"%1 of %2 bytes of map downloaded."_s.arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) ), 2 );
}

void QgsWmsImageDownloadHandler::canceled()
{
  QgsDebugMsgLevel( u"Caught canceled() signal"_s, 2 );
  if ( mCacheReply )
  {
    // abort the reply if it is still active
    QgsDebugMsgLevel( u"Aborting WMS network request"_s, 2 );
    mCacheReply->abort();
  }
}


// ----------


QgsWmsTiledImageDownloadHandler::QgsWmsTiledImageDownloadHandler( const QString &providerUri, const QgsAuthorizationSettings &auth, int tileReqNo, const QgsWmsProvider::TileRequests &requests, QImage *image, const QgsRectangle &viewExtent, double sourceResolution, bool smoothPixmapTransform, bool resamplingEnabled, QgsRasterBlockFeedback *feedback )
  : mProviderUri( providerUri )
  , mAuth( auth )
  , mImage( image )
  , mViewExtent( viewExtent )
  , mEventLoop( new QEventLoop )
  , mTileReqNo( tileReqNo )
  , mSmoothPixmapTransform( smoothPixmapTransform )
  , mFeedback( feedback )
  , mEffectiveViewExtent( viewExtent )
  , mSourceResolution( sourceResolution )
  , mResamplingEnabled( resamplingEnabled )
{
  if ( feedback )
  {
    connect( feedback, &QgsFeedback::canceled, this, &QgsWmsTiledImageDownloadHandler::canceled, Qt::QueuedConnection );

    // rendering could have been canceled before we started to listen to canceled() signal
    // so let's check before doing the download and maybe quit prematurely
    if ( feedback->isCanceled() )
      return;
  }

  for ( const QgsWmsProvider::TileRequest &r : requests )
  {
    QNetworkRequest request( r.url );
    QgsSetRequestInitiatorClass( request, u"QgsWmsTiledImageDownloadHandler"_s );
    auth.setAuthorization( request );
    request.setRawHeader( "Accept", "*/*" );
    request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ), mTileReqNo );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileIndex ), r.index );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRect ), r.rect );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), 0 );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileUrl ), r.url );

    QgsTileDownloadManagerReply *reply = QgsApplication::tileDownloadManager()->get( request );
    connect( reply, &QgsTileDownloadManagerReply::finished, this, &QgsWmsTiledImageDownloadHandler::tileReplyFinished );
    mReplies << reply;
  }
}

QgsWmsTiledImageDownloadHandler::~QgsWmsTiledImageDownloadHandler()
{
  delete mEventLoop;
}

void QgsWmsTiledImageDownloadHandler::downloadBlocking()
{
  if ( mFeedback && mFeedback->isCanceled() )
    return; // nothing to do

  mEventLoop->exec( QEventLoop::ExcludeUserInputEvents );

  Q_ASSERT( mReplies.isEmpty() );
}

void QgsWmsTiledImageDownloadHandler::tileReplyFinished()
{
  QgsTileDownloadManagerReply *reply = qobject_cast<QgsTileDownloadManagerReply *>( sender() );

#if defined( QGISDEBUG )
  bool fromCache = reply->attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool();
  QgsWmsStatistics::Stat &stat = QgsWmsStatistics::statForUri( mProviderUri );
  if ( fromCache )
    stat.cacheHits++;
  else
    stat.cacheMisses++;
#endif
#if defined( QGISDEBUG )
  QgsDebugMsgLevel( u"raw headers:"_s, 3 );
  const auto constRawHeaderPairs = reply->rawHeaderPairs();
  for ( const QNetworkReply::RawHeaderPair &pair : constRawHeaderPairs )
  {
    QgsDebugMsgLevel( u" %1:%2"_s.arg( QString::fromUtf8( pair.first ), QString::fromUtf8( pair.second ) ), 3 );
  }
#endif

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

    QgsDebugMsgLevel( u"expirationDate:%1"_s.arg( cmd.expirationDate().toString() ), 4 );
    if ( cmd.expirationDate().isNull() )
    {
      QgsSettings s;
      cmd.setExpirationDate( QDateTime::currentDateTime().addSecs( s.value( u"qgis/defaultTileExpiry"_s, "24" ).toInt() * 60 * 60 ) );
    }

    QgsNetworkAccessManager::instance()->cache()->updateMetaData( cmd );
  }

  int tileReqNo = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ) ).toInt();
  int tileNo = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( TileIndex ) ).toInt();
  QRectF r = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( TileRect ) ).toRectF();
#ifdef QGISDEBUG
  int retry = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( TileRetry ) ).toInt();
#endif
  QUrl tileUrl = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( TileUrl ) ).value<QUrl>();

  QgsDebugMsgLevel( u"tile reply %1 (%2) tile:%3(retry %4) rect:%5,%6 %7,%8) fromcache:%9 %10 url:%11"_s.arg( tileReqNo ).arg( mTileReqNo ).arg( tileNo ).arg( retry ).arg( r.left(), 0, 'f' ).arg( r.bottom(), 0, 'f' ).arg( r.right(), 0, 'f' ).arg( r.top(), 0, 'f' ).arg( fromCache ).arg( reply->error() == QNetworkReply::NoError ? QString() : u"error: "_s + reply->errorString(), reply->url().toString() ), 4 );

  if ( reply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !QgsVariantUtils::isNull( redirect ) )
    {
      QNetworkRequest request( redirect.toUrl() );
      QgsSetRequestInitiatorClass( request, u"QgsWmsTiledImageDownloadHandler"_s );
      mAuth.setAuthorization( request );
      request.setRawHeader( "Accept", "*/*" );
      request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
      request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ), tileReqNo );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileIndex ), tileNo );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRect ), r );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), 0 );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileUrl ), tileUrl );

      mReplies.removeOne( reply );
      reply->deleteLater();

      QgsDebugMsgLevel( u"redirected gettile: %1"_s.arg( redirect.toString() ), 2 );

      QgsTileDownloadManagerReply *reply = QgsApplication::tileDownloadManager()->get( request );
      connect( reply, &QgsTileDownloadManagerReply::finished, this, &QgsWmsTiledImageDownloadHandler::tileReplyFinished );
      mReplies << reply;

      return;
    }

    QVariant status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !QgsVariantUtils::isNull( status ) && status.toInt() >= 400 )
    {
      QVariant phrase = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
      QgsWmsProvider::showMessageBox( tr( "Tile request error" ), tr( "Status: %1\nReason phrase: %2" ).arg( status.toInt() ).arg( phrase.toString() ) );

      mReplies.removeOne( reply );
      reply->deleteLater();

      if ( mReplies.isEmpty() )
        finish();

      return;
    }

    QString contentType = reply->header( QNetworkRequest::ContentTypeHeader ).toString();
    QgsDebugMsgLevel( "contentType: " + contentType, 3 );
    if ( !contentType.isEmpty() && !contentType.startsWith( "image/"_L1, Qt::CaseInsensitive ) && contentType.compare( "application/octet-stream"_L1, Qt::CaseInsensitive ) != 0 )
    {
      QByteArray text = reply->data();
      QString errorTitle, errorText;
      if ( contentType.compare( "text/xml"_L1, Qt::CaseInsensitive ) == 0 && QgsWmsProvider::parseServiceExceptionReportDom( text, errorTitle, errorText ) )
      {
        QgsMessageLog::logMessage( tr( "Tile request error (Title: %1; Error: %2; URL: %3)" ).arg( errorTitle, errorText, reply->url().toString() ), tr( "WMS" ) );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Tile request error (Status: %1; Content-Type: %2; Length: %3; URL: %4)" ).arg( status.toString(), contentType ).arg( text.size() ).arg( reply->url().toString() ), tr( "WMS" ) );
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
      QgsDebugMsgLevel( u"tile reply: length %1"_s.arg( reply->data().size() ), 2 );

      QImage myLocalImage = QImage::fromData( reply->data() );

      if ( !myLocalImage.isNull() )
      {
        if ( mResamplingEnabled && mSourceResolution < 0 )
        {
          mSourceResolution = r.width() / myLocalImage.width();
          mEffectiveViewExtent = initializeBufferedImage( mViewExtent, mSourceResolution, mImage );
        }

        const QRect dst = destinationRect( mEffectiveViewExtent, r, mImage->width() );

        QPainter p( mImage );
        // if image size is "close enough" to destination size, don't smooth it out. Instead try for pixel-perfect placement!
        const bool disableSmoothing = ( qgsDoubleNear( dst.width(), myLocalImage.width(), 2 ) && qgsDoubleNear( dst.height(), myLocalImage.height(), 2 ) );
        if ( !disableSmoothing && mSmoothPixmapTransform )
          p.setRenderHint( QPainter::SmoothPixmapTransform, true );
        p.drawImage( dst, myLocalImage );
        p.end();
#if 0
        myLocalImage.save( QString( "%1/%2-tile-%3.png" ).arg( QDir::tempPath() ).arg( mTileReqNo ).arg( tileNo ) );
        p.drawRect( dst ); // show tile bounds
        p.drawText( dst, Qt::AlignCenter, QString( "(%1)\n%2,%3\n%4,%5\n%6x%7" )
                    .arg( tileNo )
                    .arg( r.left() ).arg( r.bottom() )
                    .arg( r.right() ).arg( r.top() )
                    .arg( r.width() ).arg( r.height() ) );
#endif

        QgsTileCache::insertTile( tileUrl, myLocalImage );

        if ( mFeedback )
          mFeedback->onNewData();
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Returned image is flawed [Content-Type: %1; URL: %2]" ).arg( contentType, reply->url().toString() ), tr( "WMS" ) );
      }
    }
    else
    {
      QgsDebugMsgLevel( u"Reply too late [%1]"_s.arg( reply->url().toString() ), 2 );
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
        QgsWmsStatistics::Stat &stat = QgsWmsStatistics::statForUri( mProviderUri );
        stat.errors++;
        // if we reached timeout, let's try again (e.g. in case of slow connection or slow server)
        repeatTileRequest( reply->request() );

        if ( reply->error() == QNetworkReply::ContentAccessDenied )
        {
          const QString contentType = reply->header( QNetworkRequest::ContentTypeHeader ).toString();

          QString errorMessage;
          if ( contentType.startsWith( "text/plain"_L1 ) )
            errorMessage = reply->data();
          else
            errorMessage = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();

          mError = tr( "Access denied: %1" ).arg( errorMessage );
        }
      }
    }

    mReplies.removeOne( reply );
    reply->deleteLater();

    if ( mReplies.isEmpty() )
      finish();
  }

#if 0
  const QgsWmsStatistics::Stat &stat = QgsWmsStatistics::statForUri( mProviderUri );
  emit statusChanged( tr( "%n tile request(s) in background", "tile request count", mReplies.count() )
                      + tr( ", %n cache hit(s)", "tile cache hits", stat.cacheHits )
                      + tr( ", %n cache misses.", "tile cache missed", stat.cacheMisses )
                      + tr( ", %n error(s).", "errors", stat.errors )
                    );
#endif
}

void QgsWmsTiledImageDownloadHandler::canceled()
{
  QgsDebugMsgLevel( u"Caught canceled() signal"_s, 3 );
  qDeleteAll( mReplies );
  mReplies.clear();
  finish();
}


void QgsWmsTiledImageDownloadHandler::repeatTileRequest( QNetworkRequest const &oldRequest )
{
  QgsWmsStatistics::Stat &stat = QgsWmsStatistics::statForUri( mProviderUri );

  if ( stat.errors == 100 )
  {
    QgsMessageLog::logMessage( tr( "Not logging more than 100 request errors." ), tr( "WMS" ) );
  }

  QNetworkRequest request( oldRequest );
  QgsSetRequestInitiatorClass( request, u"QgsWmsTiledImageDownloadHandler"_s );

  QString url = request.url().toString();
  int tileReqNo = request.attribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ) ).toInt();
  int tileNo = request.attribute( static_cast<QNetworkRequest::Attribute>( TileIndex ) ).toInt();
  int retry = request.attribute( static_cast<QNetworkRequest::Attribute>( TileRetry ) ).toInt();
  retry++;

  QgsSettings s;
  int maxRetry = s.value( u"qgis/defaultTileMaxRetry"_s, "3" ).toInt();
  if ( retry > maxRetry )
  {
    if ( stat.errors < 100 )
    {
      QgsMessageLog::logMessage( tr( "Tile request max retry error. Failed %1 requests for tile %2 of tileRequest %3 (url: %4)" ).arg( maxRetry ).arg( tileNo ).arg( tileReqNo ).arg( url ), tr( "WMS" ) );
    }
    return;
  }

  mAuth.setAuthorization( request );
  if ( stat.errors < 100 )
  {
    QgsMessageLog::logMessage( tr( "repeat tileRequest %1 tile %2(retry %3)" ).arg( tileReqNo ).arg( tileNo ).arg( retry ), tr( "WMS" ), Qgis::MessageLevel::Info );
  }
  QgsDebugMsgLevel( u"repeat tileRequest %1 %2(retry %3) for url: %4"_s.arg( tileReqNo ).arg( tileNo ).arg( retry ).arg( url ), 2 );
  request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), retry );

  QgsTileDownloadManagerReply *reply = QgsApplication::tileDownloadManager()->get( request );
  connect( reply, &QgsTileDownloadManagerReply::finished, this, &QgsWmsTiledImageDownloadHandler::tileReplyFinished );
  mReplies << reply;
}

double QgsWmsTiledImageDownloadHandler::sourceResolution() const
{
  return mSourceResolution;
}

QgsRectangle QgsWmsTiledImageDownloadHandler::effectiveViewExtent() const
{
  return mEffectiveViewExtent;
}

QString QgsWmsTiledImageDownloadHandler::error() const
{
  return mError;
}

// Some servers like http://glogow.geoportal2.pl/map/wms/wms.php? do not BBOX
// to be formatted with excessive precision. As a double is exactly represented
// with 19 decimal figures, do not attempt to output more
static QString formatDouble( double x )
{
  if ( x == 0.0 )
    return u"0"_s;
  const int numberOfDecimals = std::max( 0, 19 - static_cast<int>( std::ceil( std::log10( std::fabs( x ) ) ) ) );
  return qgsDoubleToString( x, numberOfDecimals );
}

QString QgsWmsProvider::toParamValue( const QgsRectangle &rect, bool changeXY )
{
  return QString( changeXY ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
    .arg( formatDouble( rect.xMinimum() ), formatDouble( rect.yMinimum() ), formatDouble( rect.xMaximum() ), formatDouble( rect.yMaximum() ) );
}

void QgsWmsProvider::setSRSQueryItem( QUrlQuery &url )
{
  QString crsKey = u"SRS"_s; //SRS in 1.1.1 and CRS in 1.3.0
  if ( mCaps.mCapabilities.version == "1.3.0"_L1 || mCaps.mCapabilities.version == "1.3"_L1 )
  {
    crsKey = u"CRS"_s;
  }
  setQueryItem( url, crsKey, mImageCrs );
}

bool QgsWmsProvider::ignoreExtents() const
{
  return mSettings.mIgnoreReportedLayerExtents;
}

// ----------

QgsWmsLegendDownloadHandler::QgsWmsLegendDownloadHandler( QgsNetworkAccessManager &networkAccessManager, const QgsWmsSettings &settings, const QUrl &url )
  : mNetworkAccessManager( networkAccessManager )
  , mSettings( settings )
  , mInitialUrl( url )
{
}

QgsWmsLegendDownloadHandler::~QgsWmsLegendDownloadHandler()
{
  if ( mReply )
  {
    // Send finished if not done yet ?
    QgsDebugMsgLevel( u"WMSLegendDownloader destroyed while still processing reply"_s, 2 );
    mReply->deleteLater();
  }
  mReply = nullptr;
}

void QgsWmsLegendDownloadHandler::start()
{
  Q_ASSERT( mVisitedUrls.empty() );
  startUrl( mInitialUrl );
}

void QgsWmsLegendDownloadHandler::startUrl( const QUrl &url )
{
  Q_ASSERT( !mReply ); // don't call me twice from outside !
  Q_ASSERT( url.isValid() );

  if ( mVisitedUrls.contains( url ) )
  {
    QString err( tr( "Redirect loop detected: %1" ).arg( url.toString() ) );
    QgsMessageLog::logMessage( err, tr( "WMS" ) );
    sendError( err );
    return;
  }
  mVisitedUrls.insert( url );

  QgsDebugMsgLevel( u"legend url: %1"_s.arg( url.toString() ), 2 );

  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, u"QgsWmsLegendDownloadHandler"_s );
  mSettings.authorization().setAuthorization( request );
  request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  mReply = mNetworkAccessManager.get( request );
  mSettings.authorization().setAuthorizationReply( mReply );

  connect( mReply, &QNetworkReply::errorOccurred, this, &QgsWmsLegendDownloadHandler::errored );

  connect( mReply, &QNetworkReply::finished, this, &QgsWmsLegendDownloadHandler::finished );
  connect( mReply, &QNetworkReply::downloadProgress, this, &QgsWmsLegendDownloadHandler::progressed );
}

void QgsWmsLegendDownloadHandler::sendError( const QString &msg )
{
  QgsMessageLog::logMessage( msg, tr( "WMS" ) );
  Q_ASSERT( mReply );
  mReply->deleteLater();
  mReply = nullptr;
  emit error( msg );
}

void QgsWmsLegendDownloadHandler::sendSuccess( const QImage &img )
{
  QgsDebugMsgLevel( u"emitting finish: %1x%2 image"_s.arg( img.width() ).arg( img.height() ), 2 );
  Q_ASSERT( mReply );
  mReply->deleteLater();
  mReply = nullptr;
  emit finish( img );
}

void QgsWmsLegendDownloadHandler::errored( QNetworkReply::NetworkError )
{
  if ( !mReply )
    return;

  sendError( mReply->errorString() );
}

void QgsWmsLegendDownloadHandler::finished()
{
  if ( !mReply )
    return;

  // or ::errored() should have been called before ::finished
  Q_ASSERT( mReply->error() == QNetworkReply::NoError );

  QgsDebugMsgLevel( u"reply OK"_s, 2 );
  QVariant redirect = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
  if ( !QgsVariantUtils::isNull( redirect ) )
  {
    mReply->deleteLater();
    mReply = nullptr;
    startUrl( redirect.toUrl() );
    return;
  }

  QVariant status = mReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
  if ( !QgsVariantUtils::isNull( status ) && status.toInt() >= 400 )
  {
    QVariant phrase = mReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
    QString msg( tr( "GetLegendGraphic request error" ) );
    msg += " - "_L1;
    msg += tr( "Status: %1\nReason phrase: %2" ).arg( status.toInt() ).arg( phrase.toString() );
    sendError( msg );
    return;
  }

  QImage myLocalImage = QImage::fromData( mReply->readAll() );
  if ( myLocalImage.isNull() )
  {
    QString msg( tr( "Returned legend image is flawed [URL: %1]" )
                   .arg( mReply->url().toString() ) );
    sendError( msg );
    return;
  }

  sendSuccess( myLocalImage );
}

void QgsWmsLegendDownloadHandler::progressed( qint64 recv, qint64 tot )
{
  emit progress( recv, tot );
}

//------

QgsCachedImageFetcher::QgsCachedImageFetcher( const QImage &img )
  : _img( img )
{
}

void QgsCachedImageFetcher::start()
{
  QTimer::singleShot( 1, this, &QgsCachedImageFetcher::send );
}


// -----------------------


QgsWmsProviderMetadata::QgsWmsProviderMetadata()
  : QgsProviderMetadata( QgsWmsProvider::WMS_KEY, QgsWmsProvider::WMS_DESCRIPTION )
{
}

QIcon QgsWmsProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconWms.svg"_s );
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsWmsProviderMetadata::capabilities() const
{
  return ProviderMetadataCapability::LayerTypesForUri
         | ProviderMetadataCapability::PriorityForUri
         | ProviderMetadataCapability::QuerySublayers;
}

QList<QgsDataItemProvider *> QgsWmsProviderMetadata::dataItemProviders() const
{
  QList<QgsDataItemProvider *> providers;

  providers
    << new QgsWmsDataItemProvider
    << new QgsXyzTileDataItemProvider;

  return providers;
}

QVariantMap QgsWmsProviderMetadata::decodeUri( const QString &uri ) const
{
  const QUrlQuery query { uri };
  const QList<QPair<QString, QString>> constItems { query.queryItems() };
  QVariantMap decoded;
  for ( const QPair<QString, QString> &item : constItems )
  {
    if ( item.first == "url"_L1 )
    {
      const QUrl url( item.second );
      if ( url.isLocalFile() )
      {
        decoded[u"path"_s] = url.toLocalFile();
        // Also add the url to insure WMS source widget works properly with XYZ file:// URLs
        decoded[item.first] = item.second;
      }
      else if ( QFileInfo( item.second ).isFile() )
      {
        decoded[u"path"_s] = item.second;
      }
      else
      {
        decoded[item.first] = item.second;
      }
    }
    else
    {
      if ( decoded.contains( item.first ) )
      {
        if ( decoded[item.first].userType() == QMetaType::Type::QString )
        {
          decoded[item.first] = QStringList() << decoded[item.first].toString();
        }
        QStringList items = decoded[item.first].toStringList();
        items.append( item.second );
        decoded[item.first] = items;
      }
      else
      {
        decoded[item.first] = item.second;
      }
    }
  }
  return decoded;
}

QString QgsWmsProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QUrlQuery query;
  QList<QPair<QString, QString>> items;
  QList<QPair<QString, QStringList>> listItems;
  for ( auto it = parts.constBegin(); it != parts.constEnd(); ++it )
  {
    if ( it.key() == "path"_L1 )
    {
      items.push_back( { u"url"_s, QUrl::fromLocalFile( it.value().toString() ).toString() } );
    }
    else if ( it.key() == "url"_L1 )
    {
      if ( !parts.contains( "path"_L1 ) )
      {
        items.push_back( { it.key(), it.value().toString() } );
      }
    }
    else
    {
      if ( it.value().userType() == QMetaType::Type::QStringList )
      {
        listItems.push_back( { it.key(), it.value().toStringList() } );
      }
      else
      {
        items.push_back( { it.key(), it.value().toString() } );
      }
    }
  }
  query.setQueryItems( items );
  QString uri { query.toString() };
  // Add lists
  for ( auto it = listItems.constBegin(); it != listItems.constEnd(); ++it )
  {
    for ( auto itItem = it->second.constBegin(); itItem != it->second.constEnd(); ++itItem )
    {
      uri.append( '&' );
      uri.append( it->first );
      uri.append( '=' );
      uri.append( *itItem );
    }
  }
  return uri;
}

QList<QgsProviderSublayerDetails> QgsWmsProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags, QgsFeedback * ) const
{
  QString fileName;
  const QFileInfo fi( uri );
  if ( fi.isFile() )
  {
    fileName = uri;
  }
  else
  {
    const QVariantMap parts = decodeUri( uri );
    if ( !parts.contains( u"path"_s ) )
    {
      if ( parts.value( u"url"_s ).isValid() )
      {
        // online wms source
        QgsProviderSublayerDetails details;
        details.setUri( uri );
        details.setProviderKey( key() );
        details.setType( Qgis::LayerType::Raster );
        return { details };
      }
      else
      {
        // not a wms uri
        return {};
      }
    }

    fileName = parts.value( u"path"_s ).toString();
  }

  if ( fileName.isEmpty() )
    return {};

  if ( QFileInfo( fileName ).suffix().compare( "mbtiles"_L1, Qt::CaseInsensitive ) == 0 )
  {
    QVariantMap parts;
    parts.insert( u"path"_s, fileName );
    parts.insert( u"type"_s, u"mbtiles"_s );

    if ( flags & Qgis::SublayerQueryFlag::FastScan )
    {
      // fast scan -- assume raster tiles are available
      QgsProviderSublayerDetails details;
      details.setUri( encodeUri( parts ) );
      details.setProviderKey( key() );
      details.setType( Qgis::LayerType::Raster );
      details.setName( QgsProviderUtils::suggestLayerNameFromFilePath( fileName ) );
      details.setSkippedContainerScan( true );
      return { details };
    }
    else
    {
      // slower scan, check actual mbtiles format
      QgsMbTiles reader( fileName );
      if ( reader.open() )
      {
        if ( reader.metadataValue( "format" ) != "pbf"_L1 )
        {
          QgsProviderSublayerDetails details;
          details.setUri( encodeUri( parts ) );
          details.setProviderKey( key() );
          details.setType( Qgis::LayerType::Raster );
          details.setName( QgsProviderUtils::suggestLayerNameFromFilePath( fileName ) );
          return { details };
        }
      }
    }
  }
  else
  {
    const thread_local QRegularExpression re( u"{-?[xyzq]}"_s );
    if ( fileName.contains( re ) )
    {
      // local XYZ directory
      QgsProviderSublayerDetails details;
      details.setUri( uri );
      details.setProviderKey( key() );
      details.setType( Qgis::LayerType::Raster );
      return { details };
    }
  }

  return {};
}

int QgsWmsProviderMetadata::priorityForUri( const QString &uri ) const
{
  if ( validLayerTypesForUri( uri ).contains( Qgis::LayerType::Raster ) )
    return 100;

  return 0;
}

QList<Qgis::LayerType> QgsWmsProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QFileInfo fi( uri );
  if ( fi.isFile() && fi.suffix().compare( "mbtiles"_L1, Qt::CaseInsensitive ) == 0 )
  {
    return { Qgis::LayerType::Raster };
  }

  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( u"path"_s ).toString().endsWith( ".mbtiles", Qt::CaseSensitivity::CaseInsensitive ) )
    return { Qgis::LayerType::Raster };

  return {};
}

QString QgsWmsProviderMetadata::absoluteToRelativeUri( const QString &src, const QgsReadWriteContext &context ) const
{
  // handle relative paths to XYZ tiles
  QgsDataSourceUri uri;
  uri.setEncodedUri( src );
  const QUrl srcUrl( uri.param( u"url"_s ) );
  if ( srcUrl.isLocalFile() )
  {
    // relative path will become "file:./x.txt"
    const QString relSrcUrl = context.pathResolver().writePath( srcUrl.toLocalFile() );
    uri.removeParam( u"url"_s ); // needed because setParam() would insert second "url" key
    uri.setParam( u"url"_s, QUrl::fromLocalFile( relSrcUrl ).toString() );
    return uri.encodedUri();
  }

  return src;
}

QString QgsWmsProviderMetadata::relativeToAbsoluteUri( const QString &src, const QgsReadWriteContext &context ) const
{
  // handle relative paths to XYZ tiles
  QgsDataSourceUri uri;
  uri.setEncodedUri( src );
  const QUrl srcUrl( uri.param( u"url"_s ) );
  if ( srcUrl.isLocalFile() ) // file-based URL? convert to relative path
  {
    const QString absSrcUrl = context.pathResolver().readPath( srcUrl.toLocalFile() );
    uri.removeParam( u"url"_s ); // needed because setParam() would insert second "url" key
    uri.setParam( u"url"_s, QUrl::fromLocalFile( absSrcUrl ).toString() );
    return uri.encodedUri();
  }

  return src;
}

QList<Qgis::LayerType> QgsWmsProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Raster };
}

#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsWmsProviderMetadata();
}
#endif


std::unique_ptr<QgsWmsInterpretationConverter> QgsWmsInterpretationConverter::createConverter( const QString &key )
{
  if ( key == QgsWmsInterpretationConverterMapTilerTerrainRGB::interpretationKey() )
    return std::make_unique<QgsWmsInterpretationConverterMapTilerTerrainRGB>();
  else if ( key == QgsWmsInterpretationConverterTerrariumRGB::interpretationKey() )
    return std::make_unique<QgsWmsInterpretationConverterTerrariumRGB>();

  return nullptr;
}

Qgis::DataType QgsWmsInterpretationConverter::dataType() const
{
  return Qgis::DataType::Float32;
}

bool QgsWmsInterpretationConverter::representsElevation() const
{
  return false;
}

//
// QgsWmsInterpretationConverterMapTilerTerrainRGB
//

void QgsWmsInterpretationConverterMapTilerTerrainRGB::convert( const QRgb &color, float *converted ) const
{
  int R = qRed( color );
  int G = qGreen( color );
  int B = qBlue( color );

  if ( qAlpha( color ) == 255 )
  {
    *converted = -10000 + ( ( R * 256 * 256 + G * 256 + B ) ) * 0.1;
  }
  else
  {
    *converted = std::numeric_limits<float>::quiet_NaN();
  }
}

QgsRasterBandStats QgsWmsInterpretationConverterMapTilerTerrainRGB::statistics( int, int, const QgsRectangle &, int, QgsRasterBlockFeedback * ) const
{
  QgsRasterBandStats stat;
  stat.minimumValue = -10000;
  stat.maximumValue = 9000;
  stat.statsGathered = Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max;
  return stat;
}

QgsRasterHistogram QgsWmsInterpretationConverterMapTilerTerrainRGB::histogram( int, int, double, double, const QgsRectangle &, int, bool, QgsRasterBlockFeedback * ) const
{
  return QgsRasterHistogram();
}

bool QgsWmsInterpretationConverterMapTilerTerrainRGB::representsElevation() const
{
  return true;
}

//
// QgsWmsInterpretationConverterTerrariumRGB
//

void QgsWmsInterpretationConverterTerrariumRGB::convert( const QRgb &color, float *converted ) const
{
  // for description of the "terrarium" format:
  // https://github.com/tilezen/joerd/blob/master/docs/formats.md

  if ( qAlpha( color ) == 255 )
  {
    *converted = qRed( color ) * 256 + qGreen( color ) + qBlue( color ) / 256.f - 32768;
  }
  else
  {
    *converted = std::numeric_limits<float>::quiet_NaN();
  }
}

QgsRasterBandStats QgsWmsInterpretationConverterTerrariumRGB::statistics( int, int, const QgsRectangle &, int, QgsRasterBlockFeedback * ) const
{
  QgsRasterBandStats stat;
  stat.minimumValue = -11000;
  stat.maximumValue = 9000;
  stat.statsGathered = Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max;
  return stat;
}

QgsRasterHistogram QgsWmsInterpretationConverterTerrariumRGB::histogram( int, int, double, double, const QgsRectangle &, int, bool, QgsRasterBlockFeedback * ) const
{
  return QgsRasterHistogram();
}

bool QgsWmsInterpretationConverterTerrariumRGB::representsElevation() const
{
  return true;
}
