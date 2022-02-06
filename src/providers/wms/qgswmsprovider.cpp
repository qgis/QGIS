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

#include "qgslogger.h"
#include "qgswmsprovider.h"
#include "qgswmsconnection.h"
#include "qgscoordinatetransform.h"
#include "qgswmsdataitems.h"
#include "qgsdatasourceuri.h"
#include "qgsfeaturestore.h"
#include "qgsgeometry.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrasterlayer.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmapsettings.h"
#include "qgsmbtiles.h"
#include "qgsmessageoutput.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsnetworkreplyparser.h"
#include "qgstilecache.h"
#include "qgsgdalutils.h"
#include "qgsgml.h"
#include "qgsgmlschema.h"
#include "qgswmscapabilities.h"
#include "qgsexception.h"
#include "qgssettings.h"
#include "qgsogrutils.h"
#include "qgsproviderregistry.h"
#include "qgsruntimeprofiler.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QUrl>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QEventLoop>
#include <QTextCodec>
#include <QThread>
#include <QNetworkDiskCache>
#include <QTimer>
#include <QStringBuilder>
#include <QUrlQuery>
#include <QJsonArray>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <ogr_api.h>

#ifdef QGISDEBUG
#include <QFile>
#include <QDir>
#endif

#define ERR(message) QGS_ERROR_MESSAGE(message,"WMS provider")
#define QGS_ERROR(message) QgsError(message,"WMS provider")

QString QgsWmsProvider::WMS_KEY = QStringLiteral( "wms" );
QString QgsWmsProvider::WMS_DESCRIPTION = QStringLiteral( "OGC Web Map Service version 1.3 data provider" );

static QString DEFAULT_LATLON_CRS = QStringLiteral( "CRS:84" );

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

  mSupportedGetFeatureFormats = QStringList() << QStringLiteral( "text/html" ) << QStringLiteral( "text/plain" ) << QStringLiteral( "text/xml" ) << QStringLiteral( "application/vnd.ogc.gml" ) << QStringLiteral( "application/json" );

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

  std::unique_ptr< QgsScopedRuntimeProfile > profile;

  if ( mSettings.mIsMBTiles )
  {
    if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
      profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Setup tile capabilities" ), QStringLiteral( "projectload" ) );

    // we are dealing with a local MBTiles file
    if ( !setupMBTilesCapabilities( uri ) )
    {
      appendError( ERR( tr( "Cannot open MBTiles database" ) ) );
      return;
    }
  }
  else if ( mSettings.mXyz )
  {
    if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
      profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Setup tile capabilities" ), QStringLiteral( "projectload" ) );

    // we are working with XYZ tiles
    // no need to get capabilities, the whole definition is in URI
    // so we just generate a dummy WMTS definition
    setupXyzCapabilities( uri );
  }
  else
  {
    // we are working with WMS / WMTS server

    // if there are already parsed capabilities, use them!
    if ( capabilities )
      mCaps = *capabilities;

    if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
      profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Retrieve server capabilities" ), QStringLiteral( "projectload" ) );

    // Make sure we have capabilities - other functions here may need them
    if ( !retrieveServerCapabilities() )
    {
      return;
    }

    // Setup temporal properties for layers in WMS-T
    if ( mSettings.mIsTemporal )
    {
      Q_ASSERT_X( temporalCapabilities(), "QgsWmsProvider::QgsWmsProvider()", "Data provider temporal capabilities object does not exist" );
      temporalCapabilities()->setHasTemporalCapabilities( true );
      temporalCapabilities()->setAvailableTemporalRange( mSettings.mFixedRange );
      temporalCapabilities()->setAllAvailableTemporalRanges( mSettings.mAllRanges );
      temporalCapabilities()->setDefaultInterval( mSettings.mDefaultInterval );

      temporalCapabilities()->setIntervalHandlingMethod( Qgis::TemporalIntervalMatchMethod::MatchExactUsingStartOfRange );

      if ( mSettings.mIsBiTemporal )
      {
        temporalCapabilities()->setAvailableReferenceTemporalRange( mSettings.mFixedReferenceRange );
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

  mValid = true;
  QgsDebugMsgLevel( QStringLiteral( "exiting constructor." ), 4 );
}


QString QgsWmsProvider::prepareUri( QString uri )
{
  // some services provide a percent/url encoded (legend) uri string, always decode here
  uri = QUrl::fromPercentEncoding( uri.toUtf8() );

  if ( isUrlForWMTS( uri ) )
  {
    return uri;
  }

  if ( !uri.contains( QLatin1String( "?" ) ) )
  {
    uri.append( '?' );
  }
  else if ( uri.right( 1 ) != QLatin1String( "?" ) && uri.right( 1 ) != QLatin1String( "&" ) )
  {
    uri.append( '&' );
  }

  return uri;
}

QgsWmsProvider::~QgsWmsProvider()
{
  QgsDebugMsgLevel( QStringLiteral( "deconstructing." ), 4 );
}

//! Returns the destination extent in image coordinate of a tile image defined by its extent
static QRectF destinationRect( const QgsRectangle &destinationExtent, const QRectF &tileImageExtent, int imagePixelWidth )
{
  double cr = destinationExtent.width() / imagePixelWidth;

  return QRectF( ( tileImageExtent.left() - destinationExtent.xMinimum() ) / cr,
                 ( destinationExtent.yMaximum() - tileImageExtent.bottom() ) / cr,
                 tileImageExtent.width() / cr,
                 tileImageExtent.height() / cr );
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
  if ( mCaps.mCapabilities.capability.request.getTile.dcpType.isEmpty() ||
       ( !mCaps.mCapabilities.capability.request.getTile.allowedEncodings.isEmpty() &&
         !mCaps.mCapabilities.capability.request.getTile.allowedEncodings.contains( QStringLiteral( "KVP" ) ) ) )
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
  return l.format.startsWith( QLatin1String( "image/" ) );
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
      if ( !mSettings.mActiveSubStyles[0].isEmpty() && mSettings.mActiveSubStyles[0] != QLatin1String( "default" ) )
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
          const QgsWmsStyleProperty *s = searchStyle( l.style, QStringLiteral( "default" ) );
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
    mActiveSubLayerVisibility[ layer ] = true;
    QgsDebugMsgLevel( QStringLiteral( "set visibility of layer '%1' to true." ).arg( layer ), 3 );
  }

  // now that the layers have changed, the extent will as well.
  mExtentDirty = true;

  if ( mSettings.mTiled )
    mTileLayer = nullptr;

  QgsDebugMsgLevel( QStringLiteral( "Exiting." ), 4 );

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
    QgsDebugMsg( QStringLiteral( "Invalid layer list length" ) );
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
      QgsDebugMsg( QStringLiteral( "Layer %1 not found" ).arg( layers[i] ) );
      return;
    }
  }

  mSettings.mActiveSubLayers = layers;
  mSettings.mActiveSubStyles.clear();
  for ( int i = 0; i < layers.size(); i++ )
  {
    mSettings.mActiveSubStyles.append( styleMap[ layers[i] ] );
  }
}


void QgsWmsProvider::setSubLayerVisibility( QString const &name, bool vis )
{
  if ( !mActiveSubLayerVisibility.contains( name ) )
  {
    QgsDebugMsg( QStringLiteral( "Layer %1 not found." ).arg( name ) );
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

    QgsDebugMsgLevel( QStringLiteral( "mTileLayersSupported.size() = %1" ).arg( mCaps.mTileLayersSupported.size() ), 2 );
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
        QString tms = tl->setLinks.keys()[0];

        if ( !mCaps.mTileMatrixSets.contains( tms ) )
        {
          QgsDebugMsg( QStringLiteral( "tile matrix set '%1' not found." ).arg( tms ) );
          continue;
        }

        if ( mCaps.mTileMatrixSets[ tms ].crs != mImageCrs )
        {
          QgsDebugMsg( QStringLiteral( "tile matrix set '%1' has crs %2 instead of %3." ).arg( tms, mCaps.mTileMatrixSets[ tms ].crs, mImageCrs ) );
          continue;
        }

        // fill in generate matrix for WMS-C
        mSettings.mTileMatrixSetId = tms;
      }

      mTileLayer = tl;
      break;
    }

    mNativeResolutions.clear();
    if ( mCaps.mTileMatrixSets.contains( mSettings.mTileMatrixSetId ) )
    {
      mTileMatrixSet = &mCaps.mTileMatrixSets[ mSettings.mTileMatrixSetId ];
      QList<double> keys = mTileMatrixSet->tileMatrices.keys();
      std::sort( keys.begin(), keys.end() );
      const auto constKeys = keys;
      for ( double key : constKeys )
      {
        mNativeResolutions << key;
      }
      if ( !mTileMatrixSet->tileMatrices.empty() )
      {
        setProperty( "tileWidth", mTileMatrixSet->tileMatrices.values().first().tileWidth );
        setProperty( "tileHeight", mTileMatrixSet->tileMatrices.values().first().tileHeight );
      }
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Expected tile matrix set '%1' not found." ).arg( mSettings.mTileMatrixSetId ) );
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
  url.removeQueryItem( item );
  if ( value.isNull() )
    url.addQueryItem( item, "" );
  else
    url.addQueryItem( item, value );
}

void QgsWmsProvider::setFormatQueryItem( QUrlQuery &url )
{
  url.removeQueryItem( QStringLiteral( "FORMAT" ) );
  if ( mSettings.mImageMimeType.contains( '+' ) )
  {
    QString format( mSettings.mImageMimeType );
    format.replace( '+', QLatin1String( "%2b" ) );
    url.addQueryItem( "FORMAT", format );
  }
  else
    setQueryItem( url, QStringLiteral( "FORMAT" ), mSettings.mImageMimeType );
}


static bool _fuzzyContainsRect( const QRectF &r1, const QRectF &r2 )
{
  double significantDigits = std::log10( std::max( r1.width(), r1.height() ) );
  double epsilon = std::pow( 10.0, significantDigits - 5 ); // floats have 6-9 significant digits
  return r1.contains( r2.adjusted( epsilon, epsilon, -epsilon, -epsilon ) );
}

void QgsWmsProvider::fetchOtherResTiles( QgsTileMode tileMode,
    const QgsRectangle &viewExtent,
    int imageWidth,
    QList<QRectF> &missingRects,
    double tres,
    int resOffset,
    QList<TileImage> &otherResTiles,
    QgsRasterBlockFeedback *feedback )
{
  if ( !mTileMatrixSet )
    return;  // there is no tile matrix set defined for ordinary WMS (with user-specified tile size)

  const QgsWmtsTileMatrix *tmOther = mTileMatrixSet->findOtherResolution( tres, resOffset );
  if ( !tmOther )
    return;

  QSet<TilePosition> tilesSet;
  const auto constMissingRects = missingRects;
  for ( const QRectF &missingTileRect : constMissingRects )
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

  QList<QRectF> missingRectsToDelete;
  const auto constRequests = requests;
  for ( const TileRequest &r : constRequests )
  {
    QImage localImage;
    if ( ! QgsTileCache::tile( r.url, localImage ) )
      continue;

    QRectF dst = destinationRect( viewExtent, r.rect, imageWidth );
    otherResTiles << TileImage( dst, localImage, false );

    // see if there are any missing rects that are completely covered by this tile
    const auto constMissingRects = missingRects;
    for ( const QRectF &missingRect : constMissingRects )
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
  const auto constMissingRectsToDelete = missingRectsToDelete;
  for ( const QRectF &rectToDelete : constMissingRectsToDelete )
  {
    missingRects.removeOne( rectToDelete );
  }

  QgsDebugMsgLevel( QStringLiteral( "Other resolution tiles: offset %1, res %2, missing rects %3, remaining rects %4, added tiles %5" )
                    .arg( resOffset )
                    .arg( tmOther->tres )
                    .arg( missingRects.count() + missingRectsToDelete.count() )
                    .arg( missingRects.count() )
                    .arg( otherResTiles.count() ), 3 );
}

uint qHash( QgsWmsProvider::TilePosition tp )
{
  return ( uint ) tp.col + ( ( uint ) tp.row << 16 );
}

static void _drawDebugRect( QPainter &p, const QRectF &rect, const QColor &color )
{
#if 0  // good for debugging how tiles from various resolutions are used
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

  return  QgsRectangle( viewExtent.xMinimum() - 2 * resolution,
                        viewExtent.yMinimum() - 2 * resolution,
                        viewExtent.xMinimum() + ( image->width() - 2 ) * resolution,
                        viewExtent.yMinimum() + ( image->height() - 2 ) * resolution );
}

QImage *QgsWmsProvider::draw( const QgsRectangle &viewExtent, int pixelWidth, int pixelHeight, QgsRectangle &effectiveViewExtent, double &sourceResolution, QgsRasterBlockFeedback *feedback )
{
  if ( qApp && qApp->thread() == QThread::currentThread() )
  {
    QgsDebugMsg( QStringLiteral( "Trying to draw a WMS image on the main thread. Stop it!" ) );
  }

  // compose the URL query string for the WMS server.

  QImage *image = new QImage( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
  image->fill( 0 );

  int maxWidth  = mCaps.mCapabilities.service.maxWidth == 0 ? std::numeric_limits<int>::max() : mCaps.mCapabilities.service.maxWidth;
  int maxHeight = mCaps.mCapabilities.service.maxHeight == 0 ? std::numeric_limits<int>::max() : mCaps.mCapabilities.service.maxHeight;

  if ( !mSettings.mTiled && mSettings.mMaxWidth == 0 && mSettings.mMaxHeight == 0 && pixelWidth <= maxWidth && pixelHeight <= maxHeight )
  {
    QUrl url = createRequestUrlWMS( viewExtent, pixelWidth, pixelHeight );

    // cache some details for if the user wants to do an identifyAsHtml() later
    QgsWmsImageDownloadHandler handler( dataSourceUri(), url, mSettings.authorization(), image, feedback );
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
        QgsDebugMsg( QStringLiteral( "WMTS tile set is empty!" ) );
        return image;
      }

      // if we know both source and output DPI, let's scale the tiles
      if ( mDpi != -1 && mTileLayer->dpi != -1 )
        vres *= static_cast<double>( mDpi ) / mTileLayer->dpi;

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
      tempTm.reset( new QgsWmtsTileMatrix );
      tempTm->topLeft      = QgsPointXY( mLayerExtent.xMinimum(), mLayerExtent.yMaximum() );
      tempTm->tileWidth    = w;
      tempTm->tileHeight   = h;
      tempTm->matrixWidth  = std::ceil( mLayerExtent.width() / w / vres );
      tempTm->matrixHeight = std::ceil( mLayerExtent.height() / h / vres );
      tempTm->tres = vres;
      tm = tempTm.get();

      tileMode = WMSC;
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "empty tile size" ) );
      return image;
    }

    QgsDebugMsgLevel( QStringLiteral( "layer extent: %1,%2,%3,%4 %5x%6" )
                      .arg( qgsDoubleToString( mLayerExtent.xMinimum() ),
                            qgsDoubleToString( mLayerExtent.yMinimum() ) )
                      .arg( qgsDoubleToString( mLayerExtent.xMaximum() ),
                            qgsDoubleToString( mLayerExtent.yMaximum() ) )
                      .arg( mLayerExtent.width() )
                      .arg( mLayerExtent.height() ), 3
                    );

    QgsDebugMsgLevel( QStringLiteral( "view extent: %1,%2,%3,%4 %5x%6  res:%7" )
                      .arg( qgsDoubleToString( viewExtent.xMinimum() ),
                            qgsDoubleToString( viewExtent.yMinimum() ) )
                      .arg( qgsDoubleToString( viewExtent.xMaximum() ),
                            qgsDoubleToString( viewExtent.yMaximum() ) )
                      .arg( viewExtent.width() )
                      .arg( viewExtent.height() )
                      .arg( vres, 0, 'f' ), 3
                    );

    QgsDebugMsgLevel( QStringLiteral( "tile matrix %1,%2 res:%3 tilesize:%4x%5 matrixsize:%6x%7 id:%8" )
                      .arg( tm->topLeft.x() ).arg( tm->topLeft.y() ).arg( tm->tres )
                      .arg( tm->tileWidth ).arg( tm->tileHeight )
                      .arg( tm->matrixWidth ).arg( tm->matrixHeight )
                      .arg( tm->identifier ), 3
                    );

    const QgsWmtsTileMatrixLimits *tml = nullptr;

    if ( mTileLayer &&
         mTileLayer->setLinks.contains( mTileMatrixSet->identifier ) &&
         mTileLayer->setLinks[ mTileMatrixSet->identifier ].limits.contains( tm->identifier ) )
    {
      tml = &mTileLayer->setLinks[ mTileMatrixSet->identifier ].limits[ tm->identifier ];
    }

    // calculate tile coordinates
    int col0, col1, row0, row1;

    if ( mConverter && mProviderResamplingEnabled ) // if resampling we need some exterior pixel depending of the algorithm, max is 2 with cubic resampling
      tm->viewExtentIntersection( viewExtent.buffered( tm->tres * 2 ), tml, col0, row0, col1, row1 );
    else
      tm->viewExtentIntersection( viewExtent, tml, col0, row0, col1, row1 );

#ifdef QGISDEBUG
    int n = ( col1 - col0 + 1 ) * ( row1 - row0 + 1 );
    QgsDebugMsgLevel( QStringLiteral( "tile number: %1x%2 = %3" ).arg( col1 - col0 + 1 ).arg( row1 - row0 + 1 ).arg( n ), 3 );
    if ( n > 256 && !mSettings.mIsMBTiles )
    {
      emit statusChanged( QStringLiteral( "current view would need %1 tiles. tile request per draw limited to 256." ).arg( n ) );
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
        QgsDebugMsg( QStringLiteral( "unexpected tile mode %1" ).arg( mTileLayer->tileMode ) );
        return image;
    }

    QList<TileImage> tileImages;  // in the correct resolution
    QList<QRectF> missing;  // rectangles (in map coords) of missing tiles for this view

    std::unique_ptr<QgsMbTiles> mbtilesReader;
    if ( mSettings.mIsMBTiles )
    {
      mbtilesReader.reset( new QgsMbTiles( QUrl( mSettings.mBaseUrl ).path() ) );
      mbtilesReader->open();
    }

    QElapsedTimer t;
    t.start();
    TileRequests requestsFinal;
    effectiveViewExtent = viewExtent;
    const auto constRequests = requests;
    for ( const TileRequest &r : constRequests )
    {
      QImage localImage;

      if ( mbtilesReader && !QgsTileCache::tile( r.url, localImage ) )
      {
        QUrlQuery query( r.url );
        QImage img = mbtilesReader->tileDataAsImage( query.queryItemValue( "z" ).toInt(),
                     query.queryItemValue( "x" ).toInt(),
                     query.queryItemValue( "y" ).toInt() );
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
            effectiveViewExtent = initializeBufferedImage( viewExtent, sourceResolution, image );
          }

        }

        const QRectF dst = destinationRect( effectiveViewExtent, r.rect, image->width() );

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

    if ( sourceResolution < 0 )
      effectiveViewExtent = viewExtent;

    int t0 = t.elapsed();
    // draw other res tiles if preview
    QPainter p( image );
    if ( feedback && feedback->isPreviewOnly() && missing.count() > 0 )
    {
      // some tiles are still missing, so let's see if we have any cached tiles
      // from lower or higher resolution available to give the user a bit of context
      // while loading the right resolution

      p.setCompositionMode( QPainter::CompositionMode_Source );
#if 0 // for debugging
      p.fillRect( image->rect(), QBrush( Qt::lightGray, Qt::CrossPattern ) );
#endif
      p.setRenderHint( QPainter::SmoothPixmapTransform, false );  // let's not waste time with bilinear filtering

      QList<TileImage> lowerResTiles, lowerResTiles2, higherResTiles;
      // first we check lower resolution tiles: one level back, then two levels back (if there is still some area not covered),
      // finally (in the worst case we use one level higher resolution tiles). This heuristic should give
      // good overviews while not spending too much time drawing cached tiles from resolutions far away.
      fetchOtherResTiles( tileMode, effectiveViewExtent, image->width(), missing, tm->tres, 1, lowerResTiles );
      fetchOtherResTiles( tileMode, effectiveViewExtent, image->width(), missing, tm->tres, 2, lowerResTiles2 );
      fetchOtherResTiles( tileMode, effectiveViewExtent, image->width(), missing, tm->tres, -1, higherResTiles );

      // draw the cached tiles lowest to highest resolution
      const auto constLowerResTiles2 = lowerResTiles2;
      for ( const TileImage &ti : constLowerResTiles2 )
      {
        p.drawImage( ti.rect, ti.img );
        _drawDebugRect( p, ti.rect, Qt::blue );
      }
      const auto constLowerResTiles = lowerResTiles;
      for ( const TileImage &ti : constLowerResTiles )
      {
        p.drawImage( ti.rect, ti.img );
        _drawDebugRect( p, ti.rect, Qt::yellow );
      }
      const auto constHigherResTiles = higherResTiles;
      for ( const TileImage &ti : constHigherResTiles )
      {
        p.drawImage( ti.rect, ti.img );
        _drawDebugRect( p, ti.rect, Qt::red );
      }
    }

    int t1 = t.elapsed() - t0;

    // draw composite in this resolution
    const auto constTileImages = tileImages;
    for ( const TileImage &ti : constTileImages )
    {
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
      QgsDebugMsgLevel( QStringLiteral( "PREVIEW - CACHED: %1 / MISSING: %2" ).arg( tileImages.count() ).arg( requests.count() - tileImages.count() ), 4 );
      QgsDebugMsgLevel( QStringLiteral( "PREVIEW - TIME: this res %1 ms | other res %2 ms | TOTAL %3 ms" ).arg( t0 + t2 ).arg( t1 ).arg( t0 + t1 + t2 ), 4 );
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
        image,
        effectiveViewExtent,
        sourceResolution,
        mSettings.mSmoothPixmapTransform,
        mProviderResamplingEnabled,
        feedback );

      handler.downloadBlocking();

      if ( feedback && !handler.error().isEmpty() )
        feedback->appendError( handler.error() );

      effectiveViewExtent = handler.effectiveViewExtent();
      sourceResolution = handler.sourceResolution();
    }

    QgsDebugMsgLevel( QStringLiteral( "TILE CACHE total: %1 / %2" ).arg( QgsTileCache::totalCost() ).arg( QgsTileCache::maxCost() ), 3 );

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

static GDALResampleAlg getGDALResamplingAlg( QgsRasterDataProvider::ResamplingMethod method )
{
  GDALResampleAlg eResampleAlg = GRA_NearestNeighbour;
  switch ( method )
  {
    case QgsRasterDataProvider::ResamplingMethod::Nearest:
    case QgsRasterDataProvider::ResamplingMethod::Gauss: // Gauss not available in GDALResampleAlg
      eResampleAlg = GRA_NearestNeighbour;
      break;

    case QgsRasterDataProvider::ResamplingMethod::Bilinear:
      eResampleAlg = GRA_Bilinear;
      break;

    case QgsRasterDataProvider::ResamplingMethod::Cubic:
      eResampleAlg = GRA_Cubic;
      break;

    case QgsRasterDataProvider::ResamplingMethod::CubicSpline:
      eResampleAlg = GRA_CubicSpline;
      break;

    case QgsRasterDataProvider::ResamplingMethod::Lanczos:
      eResampleAlg = GRA_Lanczos;
      break;

    case QgsRasterDataProvider::ResamplingMethod::Average:
      eResampleAlg = GRA_Average;
      break;

    case QgsRasterDataProvider::ResamplingMethod::Mode:
      eResampleAlg = GRA_Mode;
      break;
  }

  return eResampleAlg;
}


bool QgsWmsProvider::readBlock( int bandNo, QgsRectangle  const &viewExtent, int pixelWidth, int pixelHeight, void *block, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo )
  // TODO: optimize to avoid writing to QImage
  QgsRectangle effectiveExtent;
  double sourceResolution = -1;
  std::unique_ptr< QImage > image( draw( viewExtent, pixelWidth, pixelHeight, effectiveExtent, sourceResolution, feedback ) );
  if ( !image )   // should not happen
  {
    QgsMessageLog::logMessage( tr( "image is NULL" ), tr( "WMS" ) );
    return false;
  }

  QgsDebugMsgLevel( QStringLiteral( "image height = %1 bytesPerLine = %2" ).arg( image->height() ) . arg( image->bytesPerLine() ), 3 );
  size_t pixelsCount;
  if ( mConverter && mProviderResamplingEnabled )
    pixelsCount = static_cast<size_t>( image->width() ) * image->height();
  else
    pixelsCount  = static_cast<size_t>( pixelWidth ) * pixelHeight;

  size_t myExpectedSize = pixelsCount * 4;
  size_t myImageSize = image->height() *  image->bytesPerLine();
  if ( myExpectedSize != myImageSize )   // should not happen
  {
    QgsMessageLog::logMessage( tr( "unexpected image size" ), tr( "WMS" ) );
    return false;
  }

  uchar *ptr = image->bits();
  if ( ptr )
  {
    // If image is too large, ptr can be NULL
    if ( mConverter && ( image->format() == QImage::Format_ARGB32 || image->format() == QImage::Format_RGB32 ) )
    {
      std::vector<float> data;
      data.resize( pixelsCount );
      const QRgb *inputPtr = reinterpret_cast<const QRgb *>( image->constBits() );
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
          alg = getGDALResamplingAlg( mZoomedInResamplingMethod );
        else
          alg = getGDALResamplingAlg( mZoomedOutResamplingMethod );

        gdal::dataset_unique_ptr gdalDsInput = QgsGdalUtils::blockToSingleBandMemoryDataset( image->width(), image->height(), effectiveExtent, data.data(), GDT_Float32 );
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

  QgsDebugMsgLevel( "Active layer list of "  + mSettings.mActiveSubLayers.join( ", " )
                    + " and style list of "  + mSettings.mActiveSubStyles.join( ", " ), 2 );

  QStringList visibleLayers = QStringList();
  QStringList visibleStyles = QStringList();

  QStringList::const_iterator it2  = mSettings.mActiveSubStyles.constBegin();

  for ( QStringList::const_iterator it = mSettings.mActiveSubLayers.constBegin();
        it != mSettings.mActiveSubLayers.constEnd();
        ++it )
  {
    if ( mActiveSubLayerVisibility.constFind( *it ).value() )
    {
      visibleLayers += QUrl::toPercentEncoding( *it );
      visibleStyles += QUrl::toPercentEncoding( *it2 );
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
  setQueryItem( query, QStringLiteral( "SERVICE" ), QStringLiteral( "WMS" ) );
  setQueryItem( query, QStringLiteral( "VERSION" ), mCaps.mCapabilities.version );
  setQueryItem( query, QStringLiteral( "REQUEST" ), QStringLiteral( "GetMap" ) );
  setQueryItem( query, QStringLiteral( "BBOX" ), bbox );
  setSRSQueryItem( query );
  setQueryItem( query, QStringLiteral( "WIDTH" ), QString::number( pixelWidth ) );
  setQueryItem( query, QStringLiteral( "HEIGHT" ), QString::number( pixelHeight ) );
  setQueryItem( query, QStringLiteral( "LAYERS" ), layers );
  setQueryItem( query, QStringLiteral( "STYLES" ), styles );
  QStringList opacityList = mSettings.mOpacities;
  if ( !opacityList.isEmpty() )
  {
    setQueryItem( query, QStringLiteral( "OPACITIES" ), mSettings.mOpacities.join( ',' ) );
  }

  // For WMS-T layers
  if ( temporalCapabilities() &&
       temporalCapabilities()->hasTemporalCapabilities() )
  {
    addWmstParameters( query );
  }

  setFormatQueryItem( query );

  if ( mDpi != -1 )
  {
    if ( mSettings.mDpiMode & DpiQGIS )
      setQueryItem( query, QStringLiteral( "DPI" ), QString::number( mDpi ) );
    if ( mSettings.mDpiMode & DpiUMN )
      setQueryItem( query, QStringLiteral( "MAP_RESOLUTION" ), QString::number( mDpi ) );
    if ( mSettings.mDpiMode & DpiGeoServer )
      setQueryItem( query, QStringLiteral( "FORMAT_OPTIONS" ), QStringLiteral( "dpi:%1" ).arg( mDpi ) );
  }

  //MH: jpeg does not support transparency and some servers complain if jpg and transparent=true
  if ( mSettings.mImageMimeType == QLatin1String( "image/x-jpegorpng" ) ||
       ( !mSettings.mImageMimeType.contains( QLatin1String( "jpeg" ), Qt::CaseInsensitive ) &&
         !mSettings.mImageMimeType.contains( QLatin1String( "jpg" ), Qt::CaseInsensitive ) ) )
  {
    setQueryItem( query, QStringLiteral( "TRANSPARENT" ), QStringLiteral( "TRUE" ) );  // some servers giving error for 'true' (lowercase)
  }

  url.setQuery( query );

  QgsDebugMsgLevel( QStringLiteral( "getmap: %1" ).arg( url.toString() ), 2 );
  return url;
}

void QgsWmsProvider::addWmstParameters( QUrlQuery &query )
{
  QgsDateTimeRange range = temporalCapabilities()->requestedTemporalRange();

  QString format { QStringLiteral( "yyyy-MM-ddThh:mm:ssZ" ) };
  bool dateOnly = false;

  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "wms" ) );

  QVariantMap uri = metadata->decodeUri( dataSourceUri() );

  // Skip fetching if updates are not allowed
  if ( !uri.value( QStringLiteral( "allowTemporalUpdates" ), true ).toBool() )
    return;

  if ( range.isInfinite() )
  {
    if ( uri.contains( QStringLiteral( "time" ) ) &&
         !uri.value( QStringLiteral( "time" ) ).toString().isEmpty() )
    {
      QString time = uri.value( QStringLiteral( "time" ) ).toString();
      QStringList timeParts = time.split( '/' );

      QDateTime start = QDateTime::fromString( timeParts.at( 0 ), Qt::ISODateWithMs );
      QDateTime end = QDateTime::fromString( timeParts.at( 1 ), Qt::ISODateWithMs );

      range = QgsDateTimeRange( start, end );
    }
  }

  if ( !uri.value( QStringLiteral( "enableTime" ), true ).toBool() )
  {
    format = QStringLiteral( "yyyy-MM-dd" );
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
      setQueryItem( query, QStringLiteral( "TIME" ),
                    range.begin().toString( format ) );
    else
    {
      QString extent = range.begin().toString( format );
      extent.append( "/" );
      extent.append( range.end().toString( format ) );

      setQueryItem( query, QStringLiteral( "TIME" ), extent );
    }
  }

  // If the data provider has bi-temporal properties and they are enabled
  if ( uri.contains( QStringLiteral( "referenceTime" ) ) &&
       !uri.value( QStringLiteral( "referenceTime" ) ).toString().isEmpty() )
  {
    QString time = uri.value( QStringLiteral( "referenceTime" ) ).toString();

    QDateTime dateTime = QDateTime::fromString( time, Qt::ISODateWithMs );

    if ( dateTime.isValid() )
    {
      setQueryItem( query, QStringLiteral( "DIM_REFERENCE_TIME" ),
                    dateTime.toString( format ) );
    }
  }
}

void QgsWmsProvider::createTileRequestsWMSC( const QgsWmtsTileMatrix *tm, const QgsWmsProvider::TilePositions &tiles, QgsWmsProvider::TileRequests &requests )
{
  bool changeXY = mCaps.shouldInvertAxisOrientation( mImageCrs );

  // add WMS request
  QUrl url( mSettings.mIgnoreGetMapUrl ? mSettings.mBaseUrl : getMapUrl() );
  QUrlQuery query( url );
  setQueryItem( query, QStringLiteral( "SERVICE" ), QStringLiteral( "WMS" ) );
  setQueryItem( query, QStringLiteral( "VERSION" ), mCaps.mCapabilities.version );
  setQueryItem( query, QStringLiteral( "REQUEST" ), QStringLiteral( "GetMap" ) );
  setQueryItem( query, QStringLiteral( "LAYERS" ), mSettings.mActiveSubLayers.join( QLatin1Char( ',' ) ) );
  setQueryItem( query, QStringLiteral( "STYLES" ), mSettings.mActiveSubStyles.join( QLatin1Char( ',' ) ) );
  setQueryItem( query, QStringLiteral( "WIDTH" ), QString::number( tm->tileWidth ) );
  setQueryItem( query, QStringLiteral( "HEIGHT" ), QString::number( tm->tileHeight ) );
  setFormatQueryItem( query );

  setSRSQueryItem( query );

  if ( mSettings.mTiled )
  {
    setQueryItem( query, QStringLiteral( "TILED" ), QStringLiteral( "true" ) );
  }

  if ( mDpi != -1 )
  {
    if ( mSettings.mDpiMode & DpiQGIS )
      setQueryItem( query, QStringLiteral( "DPI" ), QString::number( mDpi ) );
    if ( mSettings.mDpiMode & DpiUMN )
      setQueryItem( query, QStringLiteral( "MAP_RESOLUTION" ), QString::number( mDpi ) );
    if ( mSettings.mDpiMode & DpiGeoServer )
      setQueryItem( query, QStringLiteral( "FORMAT_OPTIONS" ), QStringLiteral( "dpi:%1" ).arg( mDpi ) );
  }

  if ( mSettings.mImageMimeType == QLatin1String( "image/x-jpegorpng" ) ||
       ( !mSettings.mImageMimeType.contains( QLatin1String( "jpeg" ), Qt::CaseInsensitive ) &&
         !mSettings.mImageMimeType.contains( QLatin1String( "jpg" ), Qt::CaseInsensitive ) ) )
  {
    setQueryItem( query, QStringLiteral( "TRANSPARENT" ), QStringLiteral( "TRUE" ) );  // some servers giving error for 'true' (lowercase)
  }

  // For WMSC-T layers
  if ( temporalCapabilities() &&
       temporalCapabilities()->hasTemporalCapabilities() )
  {
    addWmstParameters( query );
  }

  url.setQuery( query );

  int i = 0;
  const auto constTiles = tiles;
  for ( const TilePosition &tile : constTiles )
  {
    QgsRectangle bbox( tm->tileBBox( tile.col, tile.row ) );
    QString turl;
    turl += url.toString();
    turl += QString( changeXY ? "&BBOX=%2,%1,%4,%3" : "&BBOX=%1,%2,%3,%4" )
            .arg( qgsDoubleToString( bbox.xMinimum() ),
                  qgsDoubleToString( bbox.yMinimum() ),
                  qgsDoubleToString( bbox.xMaximum() ),
                  qgsDoubleToString( bbox.yMaximum() ) );

    QgsDebugMsgLevel( QStringLiteral( "tileRequest %1 %2/%3 (%4,%5): %6" ).arg( mTileReqNo ).arg( i ).arg( tiles.count() ).arg( tile.row ).arg( tile.col ).arg( turl ), 2 );
    requests << TileRequest( turl, tm->tileRect( tile.col, tile.row ), i );
    ++i;
  }
}


void QgsWmsProvider::createTileRequestsWMTS( const QgsWmtsTileMatrix *tm, const QgsWmsProvider::TilePositions &tiles, QgsWmsProvider::TileRequests &requests )
{
  if ( !getTileUrl().isNull() )
  {
    // KVP
    QUrl url( mSettings.mIgnoreGetMapUrl ? mSettings.mBaseUrl : getTileUrl() );
    QUrlQuery query( url );

    // compose static request arguments.
    setQueryItem( query, QStringLiteral( "SERVICE" ), QStringLiteral( "WMTS" ) );
    setQueryItem( query, QStringLiteral( "REQUEST" ), QStringLiteral( "GetTile" ) );
    setQueryItem( query, QStringLiteral( "VERSION" ), mCaps.mCapabilities.version );
    setQueryItem( query, QStringLiteral( "LAYER" ), mSettings.mActiveSubLayers[0] );
    setQueryItem( query, QStringLiteral( "STYLE" ), mSettings.mActiveSubStyles[0] );
    setQueryItem( query, QStringLiteral( "FORMAT" ), mSettings.mImageMimeType );
    setQueryItem( query, QStringLiteral( "TILEMATRIXSET" ), mTileMatrixSet->identifier );
    setQueryItem( query, QStringLiteral( "TILEMATRIX" ), tm->identifier );

    for ( QHash<QString, QString>::const_iterator it = mSettings.mTileDimensionValues.constBegin(); it != mSettings.mTileDimensionValues.constEnd(); ++it )
    {
      setQueryItem( query, it.key(), it.value() );
    }

    query.removeQueryItem( QStringLiteral( "TILEROW" ) );
    query.removeQueryItem( QStringLiteral( "TILECOL" ) );
    url.setQuery( query );

    int i = 0;
    const auto constTiles = tiles;
    for ( const TilePosition &tile : constTiles )
    {
      QString turl;
      turl += url.toString();
      turl += QStringLiteral( "&TILEROW=%1&TILECOL=%2" ).arg( tile.row ).arg( tile.col );

      QgsDebugMsgLevel( QStringLiteral( "tileRequest %1 %2/%3 (%4,%5): %6" ).arg( mTileReqNo ).arg( i ).arg( tiles.count() ).arg( tile.row ).arg( tile.col ).arg( turl ), 2 );
      requests << TileRequest( turl, tm->tileRect( tile.col, tile.row ), i );
      ++i;
    }
  }
  else
  {
    // REST
    QString url = mTileLayer->getTileURLs[ mSettings.mImageMimeType ];

    url.replace( QLatin1String( "{layer}" ), mSettings.mActiveSubLayers[0], Qt::CaseInsensitive );
    url.replace( QLatin1String( "{style}" ), mSettings.mActiveSubStyles[0], Qt::CaseInsensitive );
    url.replace( QLatin1String( "{tilematrixset}" ), mTileMatrixSet->identifier, Qt::CaseInsensitive );
    url.replace( QLatin1String( "{tilematrix}" ), tm->identifier, Qt::CaseInsensitive );

    for ( QHash<QString, QString>::const_iterator it = mSettings.mTileDimensionValues.constBegin(); it != mSettings.mTileDimensionValues.constEnd(); ++it )
    {
      url.replace( "{" + it.key() + "}", it.value(), Qt::CaseInsensitive );
    }

    int i = 0;
    const auto constTiles = tiles;
    for ( const TilePosition &tile : constTiles )
    {
      QString turl( url );
      turl.replace( QLatin1String( "{tilerow}" ), QString::number( tile.row ), Qt::CaseInsensitive );
      turl.replace( QLatin1String( "{tilecol}" ), QString::number( tile.col ), Qt::CaseInsensitive );

      QgsDebugMsgLevel( QStringLiteral( "tileRequest %1 %2/%3 (%4,%5): %6" ).arg( mTileReqNo ).arg( i ).arg( tiles.count() ).arg( tile.row ).arg( tile.col ).arg( turl ), 2 );
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
  const auto constTiles = tiles;
  for ( const TilePosition &tile : constTiles )
  {
    ++i;
    QString turl( url );

    if ( turl.contains( QLatin1String( "{q}" ) ) )  // used in Bing maps
      turl.replace( QLatin1String( "{q}" ), _tile2quadkey( tile.col, tile.row, z ) );

    turl.replace( QLatin1String( "{x}" ), QString::number( tile.col ), Qt::CaseInsensitive );
    // inverted Y axis
    if ( turl.contains( QLatin1String( "{-y}" ) ) )
    {
      turl.replace( QLatin1String( "{-y}" ), QString::number( tm->matrixHeight - tile.row - 1 ), Qt::CaseInsensitive );
    }
    else
    {
      turl.replace( QLatin1String( "{y}" ), QString::number( tile.row ), Qt::CaseInsensitive );
    }
    turl.replace( QLatin1String( "{z}" ), QString::number( z ), Qt::CaseInsensitive );

    if ( turl.contains( QLatin1String( "{usage}" ) ) && feedback )
    {
      switch ( feedback->renderContext().rendererUsage() )
      {
        case Qgis::RendererUsage::View:
          turl.replace( QLatin1String( "{usage}" ), QLatin1String( "view" ) );
          break;
        case Qgis::RendererUsage::Export:
          turl.replace( QLatin1String( "{usage}" ), QLatin1String( "export" ) );
          break;
        case Qgis::RendererUsage::Unknown:
          turl.replace( QLatin1String( "{usage}" ), QString() );
          break;
      }
    }

    QgsDebugMsgLevel( QStringLiteral( "tileRequest %1 %2/%3 (%4,%5): %6" ).arg( mTileReqNo ).arg( i ).arg( tiles.count() ).arg( tile.row ).arg( tile.col ).arg( turl ), 2 );
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
      mErrorFormat = QStringLiteral( "text/plain" );
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


void QgsWmsProvider::setupXyzCapabilities( const QString &uri, const QgsRectangle &sourceExtent, int sourceMinZoom, int sourceMaxZoom, double sourceTilePixelRatio )
{
  QgsDataSourceUri parsedUri;
  parsedUri.setEncodedUri( uri );

  QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), QgsCoordinateReferenceSystem( mSettings.mCrsId ),
                             transformContext() );

  // the whole world is projected to a square:
  // X going from 180 W to 180 E
  // Y going from ~85 N to ~85 S  (=atan(sinh(pi)) ... to get a square)
  QgsPointXY topLeftLonLat( -180, 180.0 / M_PI * std::atan( std::sinh( M_PI ) ) );
  QgsPointXY bottomRightLonLat( 180, 180.0 / M_PI * std::atan( std::sinh( -M_PI ) ) );
  QgsPointXY topLeft = ct.transform( topLeftLonLat );
  QgsPointXY bottomRight = ct.transform( bottomRightLonLat );
  double xspan = ( bottomRight.x() - topLeft.x() );

  QgsWmsBoundingBoxProperty bbox;
  bbox.crs = mSettings.mCrsId;
  bbox.box = sourceExtent.isNull() ? QgsRectangle( topLeft.x(), bottomRight.y(), bottomRight.x(), topLeft.y() ) : sourceExtent;

  // metadata
  if ( mSettings.mXyz )
  {
    if ( parsedUri.param( QStringLiteral( "url" ) ).contains( QLatin1String( "openstreetmap" ), Qt::CaseInsensitive ) )
    {
      mLayerMetadata.setTitle( tr( "OpenStreetMap tiles" ) );
      mLayerMetadata.setIdentifier( tr( "OpenStreetMap tiles" ) );
      mLayerMetadata.setAbstract( tr( "OpenStreetMap is built by a community of mappers that contribute and maintain data about roads, trails, cafés, railway stations, and much more, all over the world." ) );

      QStringList licenses;
      licenses << tr( "Open Data Commons Open Database License (ODbL)" );
      if ( parsedUri.param( QStringLiteral( "url" ) ).contains( QLatin1String( "tile.openstreetmap.org" ), Qt::CaseInsensitive ) )
      {
        // OSM tiles have a different attribution requirement to OpenStreetMap data - see https://www.openstreetmap.org/copyright
        mLayerMetadata.setRights( QStringList() << tr( "Base map and data from OpenStreetMap and OpenStreetMap Foundation (CC-BY-SA). © https://www.openstreetmap.org and contributors." ) );
        licenses << tr( "Creative Commons Attribution-ShareAlike (CC-BY-SA)" );
      }
      else
        mLayerMetadata.setRights( QStringList() << tr( "© OpenStreetMap and contributors (https://www.openstreetmap.org/copyright)." ) );
      mLayerMetadata.setLicenses( licenses );

      QgsLayerMetadata::SpatialExtent spatialExtent;
      spatialExtent.bounds = QgsBox3d( QgsRectangle( topLeftLonLat.x(), bottomRightLonLat.y(), bottomRightLonLat.x(), topLeftLonLat.y() ) );
      spatialExtent.extentCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
      QgsLayerMetadata::Extent metadataExtent;
      metadataExtent.setSpatialExtents( QList<  QgsLayerMetadata::SpatialExtent >() << spatialExtent );
      mLayerMetadata.setExtent( metadataExtent );
      mLayerMetadata.setCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );

      mLayerMetadata.addLink( QgsAbstractMetadataBase::Link( tr( "Source" ), QStringLiteral( "WWW:LINK" ), QStringLiteral( "https://www.openstreetmap.org/" ) ) );
    }
  }
  mLayerMetadata.setType( QStringLiteral( "dataset" ) );

  QgsWmtsTileLayer tl;
  tl.tileMode = XYZ;
  tl.identifier = QStringLiteral( "xyz" );  // as set in parseUri
  tl.boundingBoxes << bbox;

  double tilePixelRatio = sourceTilePixelRatio;  // by default 0 = unknown
  if ( parsedUri.hasParam( QStringLiteral( "tilePixelRatio" ) ) )
    tilePixelRatio = parsedUri.param( QStringLiteral( "tilePixelRatio" ) ).toDouble();

  if ( tilePixelRatio != 0 )
  {
    // known tile pixel ratio - will be doing auto-scaling of tiles based on output DPI
    tl.dpi = 96 * tilePixelRatio;  // TODO: is 96 correct base DPI ?
  }
  else
  {
    // unknown tile pixel ratio - no scaling of tiles based on output DPI
    tilePixelRatio = 1;
  }

  mCaps.mTileLayersSupported.append( tl );

  QgsWmtsTileMatrixSetLink &tmsLinkRef = mCaps.mTileLayersSupported.last().setLinks[QStringLiteral( "tms0" )];
  tmsLinkRef.tileMatrixSet = QStringLiteral( "tms0" );

  QgsWmtsTileMatrixSet tms;
  tms.identifier = QStringLiteral( "tms0" );  // as set in parseUri
  tms.crs = mSettings.mCrsId;
  mCaps.mTileMatrixSets[tms.identifier] = tms;

  int minZoom = sourceMinZoom == -1 ? 0 : sourceMinZoom;
  int maxZoom = sourceMaxZoom == -1 ? 18 : sourceMaxZoom;
  if ( parsedUri.hasParam( QStringLiteral( "zmin" ) ) )
    minZoom = parsedUri.param( QStringLiteral( "zmin" ) ).toInt();
  if ( parsedUri.hasParam( QStringLiteral( "zmax" ) ) )
    maxZoom = parsedUri.param( QStringLiteral( "zmax" ) ).toInt();

  // zoom 0 is one tile for the whole world
  for ( int zoom = minZoom; zoom <= maxZoom; ++zoom )
  {
    QgsWmtsTileMatrix tm;
    tm.identifier = QString::number( zoom );
    tm.topLeft = topLeft;
    tm.tileWidth = tm.tileHeight = 256 * tilePixelRatio;
    tm.matrixWidth = tm.matrixHeight = 1 << zoom;
    tm.tres = xspan / ( tm.tileWidth * tm.matrixWidth );
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
    QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), QgsCoordinateReferenceSystem( mSettings.mCrsId ),
                               transformContext() );
    try
    {
      customTopLeft = ct.transform( QgsPointXY( sourceExtentWgs84.xMinimum(), sourceExtentWgs84.yMaximum() ) );
      customBottomRight = ct.transform( QgsPointXY( sourceExtentWgs84.xMaximum(), sourceExtentWgs84.yMinimum() ) );
    }
    catch ( const QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Failed to reproject extent from MBTiles metadata" ) );
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

  setupXyzCapabilities( uri, sourceExtent, sourceMinZoom, sourceMaxZoom, sourceTilePixelRatio );
  return true;
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

  QgsDebugMsgLevel( QStringLiteral( "transforming layer extent %1" ).arg( extent.toString( true ) ), 2 );
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
  QgsDebugMsgLevel( QStringLiteral( "transformed layer extent %1" ).arg( extent.toString( true ) ), 2 );

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
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) ||
           tagName.startsWith( QLatin1String( "ogc:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "ServiceException" ) )
      {
        QgsDebugMsg( QStringLiteral( "  ServiceException." ) );
        parseServiceException( e, errorTitle, errorText );
      }

    }
    n = n.nextSibling();
  }

  return true;
}


void QgsWmsProvider::parseServiceException( QDomElement const &e, QString &errorTitle, QString &errorText )
{

  QString seCode = e.attribute( QStringLiteral( "code" ) );
  QString seText = e.text();

  errorTitle = tr( "Service Exception" );

  // set up friendly descriptions for the service exception
  if ( seCode == QLatin1String( "InvalidFormat" ) )
  {
    errorText = tr( "Request contains a format not offered by the server." );
  }
  else if ( seCode == QLatin1String( "InvalidCRS" ) )
  {
    errorText = tr( "Request contains a CRS not offered by the server for one or more of the Layers in the request." );
  }
  else if ( seCode == QLatin1String( "InvalidSRS" ) )  // legacy WMS < 1.3.0
  {
    errorText = tr( "Request contains a SRS not offered by the server for one or more of the Layers in the request." );
  }
  else if ( seCode == QLatin1String( "LayerNotDefined" ) )
  {
    errorText = tr( "GetMap request is for a Layer not offered by the server, "
                    "or GetFeatureInfo request is for a Layer not shown on the map." );
  }
  else if ( seCode == QLatin1String( "StyleNotDefined" ) )
  {
    errorText = tr( "Request is for a Layer in a Style not offered by the server." );
  }
  else if ( seCode == QLatin1String( "LayerNotQueryable" ) )
  {
    errorText = tr( "GetFeatureInfo request is applied to a Layer which is not declared queryable." );
  }
  else if ( seCode == QLatin1String( "InvalidPoint" ) )
  {
    errorText = tr( "GetFeatureInfo request contains invalid X or Y value." );
  }
  else if ( seCode == QLatin1String( "CurrentUpdateSequence" ) )
  {
    errorText = tr( "Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to "
                    "current value of service metadata update sequence number." );
  }
  else if ( seCode == QLatin1String( "InvalidUpdateSequence" ) )
  {
    errorText = tr( "Value of (optional) UpdateSequence parameter in GetCapabilities request is greater "
                    "than current value of service metadata update sequence number." );
  }
  else if ( seCode == QLatin1String( "MissingDimensionValue" ) )
  {
    errorText = tr( "Request does not include a sample dimension value, and the server did not declare a "
                    "default value for that dimension." );
  }
  else if ( seCode == QLatin1String( "InvalidDimensionValue" ) )
  {
    errorText = tr( "Request contains an invalid sample dimension value." );
  }
  else if ( seCode == QLatin1String( "OperationNotSupported" ) )
  {
    errorText = tr( "Request is for an optional operation that is not supported by the server." );
  }
  else if ( seCode == QLatin1String( "NoMatch" ) )
  {
    QString locator = e.attribute( QStringLiteral( "locator" ) );
    if ( locator == QLatin1String( "time" ) )
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

  QgsDebugMsgLevel( QStringLiteral( "exiting with composed error message '%1'." ).arg( errorText ), 2 );
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
        QgsDebugMsgLevel( QStringLiteral( "Skip %1 [%2]" ).arg( mTileLayer->boundingBoxes.at( i ).crs, mImageCrs ), 2 );
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

          QgsDebugMsgLevel( QStringLiteral( "ct: %1 => %2" ).arg( mTileLayer->boundingBoxes.at( i ).crs, mImageCrs ), 2 );

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

      QgsDebugMsgLevel( "exiting with '"  + mLayerExtent.toString() + "'.", 3 );

      return true;
    }

    QgsDebugMsgLevel( QStringLiteral( "no extent returned" ), 2 );
    return false;
  }
  else
  {
    bool firstLayer = true; //flag to know if a layer is the first to be successfully transformed
    for ( QStringList::const_iterator it  = mSettings.mActiveSubLayers.constBegin();
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

      QgsDebugMsgLevel( "extent for " + *it  + " is " + extent.toString( 3 )  + '.', 2 );

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

      QgsDebugMsgLevel( "combined extent is '"  + mLayerExtent.toString()
                        + "' after '"  + ( *it ) + "'.", 2 );

    }

    QgsDebugMsgLevel( "exiting with '"  + mLayerExtent.toString() + "'.", 2 );
    return true;
  }
}


int QgsWmsProvider::capabilities() const
{
  int capability = NoCapabilities;
  bool canIdentify = false;

  if ( mSettings.mTiled && mTileLayer )
  {
    QgsDebugMsgLevel( QStringLiteral( "Tiled." ), 2 );
    canIdentify = !mTileLayer->getFeatureInfoURLs.isEmpty() || !getFeatureInfoUrl().isNull();
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Not tiled." ), 2 );
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
          QgsDebugMsgLevel( '\''  + ( *it )  + "' is queryable.", 2 );
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
      capability |= Capability::Identify;
    }
  }

  // Prevent prefetch of XYZ openstreetmap images, see: https://github.com/qgis/QGIS/issues/34813
  // But also prevent prefetching if service is a true WMS (mSettings.mTiled = True)
  // See https://github.com/qgis/QGIS/issues/34813
  if ( mSettings.mTiled && !( mSettings.mXyz && dataSourceUri().contains( QStringLiteral( "openstreetmap.org" ) ) ) )
  {
    // March 2021: *never* prefetch tile based layers, see: https://github.com/qgis/QGIS/pull/41953
    // capability |= Capability::Prefetch;
  }

  if ( mSettings.mTiled || mSettings.mXyz )
  {
    capability |= DpiDependentData;
  }

  QgsDebugMsgLevel( QStringLiteral( "capability = %1" ).arg( capability ), 2 );
  return capability;
}

QString QgsWmsProvider::layerMetadata( QgsWmsLayerProperty &layer )
{
  QString metadata =
    // Layer Properties section
    // Use a nested table
    QStringLiteral( "<tr><td>"
                    "<table width=\"100%\" class=\"tabular-view\">"

                    // Table header
                    "<tr><th class=\"strong\">" ) %
    tr( "Property" ) %
    QStringLiteral( "</th>"
                    "<th class=\"strong\">" ) %
    tr( "Value" ) %
    QStringLiteral( "</th></tr>"

                    // Name
                    "<tr><td>" ) %
    tr( "Name" ) %
    QStringLiteral( "</td>"
                    "<td>" ) %
    layer.name %
    QStringLiteral( "</td></tr>"

                    // Layer Visibility (as managed by this provider)
                    "<tr><td>" ) %
    tr( "Visibility" ) %
    QStringLiteral( "</td>"
                    "<td>" ) %
    ( mActiveSubLayerVisibility.find( layer.name ).value() ? tr( "Visible" ) : tr( "Hidden" ) ) %
    QStringLiteral( "</td></tr>"

                    // Layer Title
                    "<tr><td>" ) %
    tr( "Title" ) %
    QStringLiteral( "</td>"
                    "<td>" ) %
    layer.title %
    QStringLiteral( "</td></tr>"

                    // Layer Abstract
                    "<tr><td>" ) %
    tr( "Abstract" ) %
    QStringLiteral( "</td>"
                    "<td>" ) %
    layer.abstract %
    QStringLiteral( "</td></tr>"

                    // Layer Queryability
                    "<tr><td>" ) %
    tr( "Can Identify" ) %
    QStringLiteral( "</td>"
                    "<td>" ) %
    ( layer.queryable ? tr( "Yes" ) : tr( "No" ) ) %
    QStringLiteral( "</td></tr>"

                    // Layer Opacity
                    "<tr><td>" ) %
    tr( "Can be Transparent" ) %
    QStringLiteral( "</td>"
                    "<td>" ) %
    ( layer.opaque ? tr( "No" ) : tr( "Yes" ) ) %
    QStringLiteral( "</td></tr>"

                    // Layer Subsetability
                    "<tr><td>" ) %
    tr( "Can Zoom In" ) %
    QStringLiteral( "</td>"
                    "<td>" ) %
    ( layer.noSubsets ? tr( "No" ) : tr( "Yes" ) ) %
    QStringLiteral( "</td></tr>"

                    // Layer Server Cascade Count
                    "<tr><td>" ) %
    tr( "Cascade Count" ) %
    QStringLiteral( "</td>"
                    "<td>" ) %
    QString::number( layer.cascaded ) %
    QStringLiteral( "</td></tr>"

                    // Layer Fixed Width
                    "<tr><td>" ) %
    tr( "Fixed Width" ) %
    QStringLiteral( "</td>"
                    "<td>" ) %
    QString::number( layer.fixedWidth ) %
    QStringLiteral( "</td></tr>"

                    // Layer Fixed Height
                    "<tr><td>" ) %
    tr( "Fixed Height" ) %
    QStringLiteral( "</td>"
                    "<td>" ) %
    QString::number( layer.fixedHeight ) %
    QStringLiteral( "</td></tr>" );

  // Dimensions
  if ( !layer.dimensions.isEmpty() )
  {
    metadata += QStringLiteral( "<tr><th>" ) %
                tr( "Dimensions" ) %
                QStringLiteral( "</th>"
                                "<td><table class=\"tabular-view\">"
                                "<tr><th>" ) %
                tr( "Name" ) %
                QStringLiteral( "</th><th>" ) %
                tr( "Unit" ) %
                QStringLiteral( "</th><th>" ) %
                tr( "Extent" ) %
                QStringLiteral( "</th></tr>" );

    for ( const QgsWmsDimensionProperty &d : std::as_const( layer.dimensions ) )
    {
      metadata += QStringLiteral( "<tr><td>" ) % d.name % QStringLiteral( "</td><td>" ) % d.units %  QStringLiteral( "</td><td>" ) % d.extent % QStringLiteral( "</td></tr>" );
    }
    metadata += QStringLiteral( "</table>"
                                "</td></tr>" );
  }
  // Metadata URLs
  if ( !layer.metadataUrl.isEmpty() )
  {
    metadata += QStringLiteral( "<tr><th>" ) %
                tr( "Metadata URLs" ) %
                QStringLiteral( "</th>"
                                "<td><table class=\"tabular-view\">"
                                "<tr><th>" ) %
                tr( "Format" ) %
                QStringLiteral( "</th><th>" ) %
                tr( "URL" ) %
                QStringLiteral( "</th></tr>" );

    for ( const QgsWmsMetadataUrlProperty &l : std::as_const( layer.metadataUrl ) )
    {
      metadata += QStringLiteral( "<tr><td>" ) % l.format % QStringLiteral( "</td><td>" ) % l.onlineResource.xlinkHref % QStringLiteral( "</td></tr>" );
    }
    metadata += QStringLiteral( "</table>"
                                "</td></tr>" );
  }

  // Layer Coordinate Reference Systems
  for ( int j = 0; j < std::min( static_cast< int >( layer.crs.size() ), 10 ); j++ )
  {
    metadata += QStringLiteral( "<tr><td>" ) %
                tr( "Available in CRS" ) %
                QStringLiteral( "</td>"
                                "<td>" ) %
                layer.crs[j] %
                QStringLiteral( "</td></tr>" );
  }

  if ( layer.crs.size() > 10 )
  {
    metadata += QStringLiteral( "<tr><td>" ) %
                tr( "Available in CRS" ) %
                QStringLiteral( "</td>"
                                "<td>" ) %
                tr( "(and %n more)", "crs", layer.crs.size() - 10 ) %
                QStringLiteral( "</td></tr>" );
  }

  // Layer Styles
  for ( int j = 0; j < layer.style.size(); j++ )
  {
    const QgsWmsStyleProperty &style = layer.style.at( j );

    metadata += QStringLiteral( "<tr><td>" ) %
                tr( "Available in style" ) %
                QStringLiteral( "</td>"
                                "<td>" ) %

                // Nested table.
                QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">"

                                // Layer Style Name
                                "<tr><th class=\"strong\">" ) %
                tr( "Name" ) %
                QStringLiteral( "</th>"
                                "<td>" ) %
                style.name %
                QStringLiteral( "</td></tr>"

                                // Layer Style Title
                                "<tr><th class=\"strong\">" ) %
                tr( "Title" ) %
                QStringLiteral( "</th>"
                                "<td>" ) %
                style.title %
                QStringLiteral( "</td></tr>"

                                // Layer Style Abstract
                                "<tr><th class=\"strong\">" ) %
                tr( "Abstract" ) %
                QStringLiteral( "</th>"
                                "<td>" ) %
                style.abstract %
                QStringLiteral( "</td></tr>" );

    // LegendURLs
    if ( !style.legendUrl.isEmpty() )
    {
      metadata += QStringLiteral( "<tr><th class=\"strong\">" ) %
                  tr( "LegendURLs" ) %
                  QStringLiteral( "</th>"
                                  "<td><table class=\"tabular-view\">"
                                  "<tr><th>Format</th><th>URL</th></tr>" );
      for ( int k = 0; k < style.legendUrl.size(); k++ )
      {
        const QgsWmsLegendUrlProperty &l = style.legendUrl[k];
        metadata += QStringLiteral( "<tr><td>" ) % l.format % QStringLiteral( "</td><td>" ) % l.onlineResource.xlinkHref % QStringLiteral( "</td></tr>" );
      }
      metadata += QLatin1String( "</table></td></tr>" );
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

QString QgsWmsProvider::htmlMetadata()
{
  QString metadata;

  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "WMS Info" ) % QStringLiteral( "</td><td><div>" );

  if ( !mSettings.mTiled )
  {
    metadata += QStringLiteral( "&nbsp;<a href=\"\" onclick=\"document.getElementById('selectedlayers').scrollIntoView(); return false;\">" ) %
                tr( "Selected Layers" ) %
                QStringLiteral( "</a>&nbsp;<a href=\"\" onclick=\"document.getElementById('otherlayers').scrollIntoView(); return false;\">" ) %
                tr( "Other Layers" ) %
                QStringLiteral( "</a>" );
  }
  else
  {
    metadata += QStringLiteral( "&nbsp;<a href=\"\" onclick=\"document.getElementById('tilesetproperties').scrollIntoView(); return false;\">" ) %
                tr( "Tile Layer Properties" ) %
                QStringLiteral( "</a> "
                                "&nbsp;<a href=\"\" onclick=\"document.getElementById('cachestats'); return false;\">" ) %
                tr( "Cache Stats" ) %
                QStringLiteral( "</a> " );
  }

  metadata += QStringLiteral( "<br /><table class=\"tabular-view\">" // Nested table 1
                              // Server Properties section
                              "<tr><th class=\"strong\" id=\"serverproperties\">" ) %
              tr( "Server Properties" ) %
              QStringLiteral( "</th></tr>"

                              // Use a nested table
                              "<tr><td>"
                              "<table width=\"100%\" class=\"tabular-view\">" ); // Nested table 2

  // Table header
  metadata += QStringLiteral( "<tr><th class=\"strong\">" ) %
              tr( "Property" ) %
              QStringLiteral( "</th>"
                              "<th class=\"strong\">" ) %
              tr( "Value" ) %
              QStringLiteral( "</th></tr>" );

  // WMS Version
  metadata += QStringLiteral( "<tr><td>" ) %
              tr( "WMS Version" ) %
              QStringLiteral( "</td>"
                              "<td>" ) %
              mCaps.mCapabilities.version %
              QStringLiteral( "</td></tr>" );

  // Service Title
  metadata += QStringLiteral( "<tr><td>" ) %
              tr( "Title" ) %
              QStringLiteral( "</td>"
                              "<td>" ) %
              mCaps.mCapabilities.service.title %
              QStringLiteral( "</td></tr>" );

  // Service Abstract
  metadata += QStringLiteral( "<tr><td>" ) %
              tr( "Abstract" ) %
              QStringLiteral( "</td>"
                              "<td>" ) %
              mCaps.mCapabilities.service.abstract %
              QStringLiteral( "</td></tr>" );

  // Service Keywords
  metadata += QStringLiteral( "<tr><td>" ) %
              tr( "Keywords" ) %
              QStringLiteral( "</td>"
                              "<td>" ) %
              mCaps.mCapabilities.service.keywordList.join( QLatin1String( "<br />" ) ) %
              QStringLiteral( "</td></tr>" );

  // Service Online Resource
  metadata += QStringLiteral( "<tr><td>" ) %
              tr( "Online Resource" ) %
              QStringLiteral( "</td>"
                              "<td>" ) %
              '-' %
              QStringLiteral( "</td></tr>" );

  // Service Contact Information
  metadata += QStringLiteral( "<tr><td>" ) %
              tr( "Contact Person" ) %
              QStringLiteral( "</td>"
                              "<td>" ) %
              mCaps.mCapabilities.service.contactInformation.contactPersonPrimary.contactPerson %
              QStringLiteral( "<br />" ) %
              mCaps.mCapabilities.service.contactInformation.contactPosition %
              QStringLiteral( "<br />" ) %
              mCaps.mCapabilities.service.contactInformation.contactPersonPrimary.contactOrganization %
              QStringLiteral( "</td></tr>" );

  // Service Fees
  metadata += QStringLiteral( "<tr><td>" ) %
              tr( "Fees" ) %
              QStringLiteral( "</td>"
                              "<td>" ) %
              mCaps.mCapabilities.service.fees %
              QStringLiteral( "</td></tr>" );

  // Service Access Constraints
  metadata += QStringLiteral( "<tr><td>" ) %
              tr( "Access Constraints" ) %
              QStringLiteral( "</td>"
                              "<td>" ) %
              mCaps.mCapabilities.service.accessConstraints %
              QStringLiteral( "</td></tr>" );

  // Base URL
  metadata += QStringLiteral( "<tr><td>" ) %
              tr( "GetCapabilitiesUrl" ) %
              QStringLiteral( "</td>"
                              "<td>" ) %
              mSettings.mBaseUrl %
              QStringLiteral( "</td></tr>" );

  metadata += QStringLiteral( "<tr><td>" ) %
              tr( "GetMapUrl" ) %
              QStringLiteral( "</td>"
                              "<td>" ) %
              getMapUrl() % ( mSettings.mIgnoreGetMapUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : QString() ) %
              QStringLiteral( "</td></tr>" );

  metadata += QStringLiteral( "<tr><td>" ) %
              tr( "GetFeatureInfoUrl" ) %
              QStringLiteral( "</td>"
                              "<td>" ) %
              getFeatureInfoUrl() % ( mSettings.mIgnoreGetFeatureInfoUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : QString() ) %
              QStringLiteral( "</td></tr>" );

  metadata += QStringLiteral( "<tr><td>" ) %
              tr( "GetLegendGraphic" ) %
              QStringLiteral( "</td>"
                              "<td>" ) %
              getLegendGraphicUrl() % ( mSettings.mIgnoreGetMapUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : QString() ) %
              QStringLiteral( "</td></tr>" );

  if ( mSettings.mTiled )
  {
    metadata += QStringLiteral( "<tr><td>" ) %
                tr( "Tile Layer Count" ) %
                QStringLiteral( "</td>"
                                "<td>" ) %
                QString::number( mCaps.mTileLayersSupported.size() ) %
                QStringLiteral( "</td></tr>"
                                "<tr><td>" ) %
                tr( "GetTileUrl" ) %
                QStringLiteral( "</td>"
                                "<td>" ) %
                getTileUrl() %
                QStringLiteral( "</td></tr>" );

    if ( mTileLayer )
    {
      metadata += QStringLiteral( "<tr><td>" ) %
                  tr( "Tile templates" ) %
                  QStringLiteral( "</td>"
                                  "<td>" );
      for ( QHash<QString, QString>::const_iterator it = mTileLayer->getTileURLs.constBegin();
            it != mTileLayer->getTileURLs.constEnd();
            ++it )
      {
        metadata += QStringLiteral( "%1:%2<br>" ).arg( it.key(), it.value() );
      }
      metadata += QStringLiteral( "</td></tr>"

                                  "<tr><td>" ) %
                  tr( "FeatureInfo templates" ) %
                  QStringLiteral( "</td>"
                                  "<td>" );
      for ( QHash<QString, QString>::const_iterator it = mTileLayer->getFeatureInfoURLs.constBegin();
            it != mTileLayer->getFeatureInfoURLs.constEnd();
            ++it )
      {
        metadata += QStringLiteral( "%1:%2<br>" ).arg( it.key(), it.value() );
      }
      metadata += QLatin1String( "</td></tr>" );

      // GetFeatureInfo Request Formats
      metadata += QStringLiteral( "<tr><td>" ) %
                  tr( "Identify Formats" ) %
                  QStringLiteral( "</td>"
                                  "<td>" ) %
                  mTileLayer->infoFormats.join( QLatin1String( "<br />" ) ) %
                  QStringLiteral( "</td></tr>" );
    }
  }
  else
  {
    // GetMap Request Formats
    metadata += QStringLiteral( "<tr><td>" ) %
                tr( "Image Formats" ) %
                QStringLiteral( "</td>"
                                "<td>" ) %
                mCaps.mCapabilities.capability.request.getMap.format.join( QLatin1String( "<br />" ) ) %
                QStringLiteral( "</td></tr>"

                                // GetFeatureInfo Request Formats
                                "<tr><td>" ) %
                tr( "Identify Formats" ) %
                QStringLiteral( "</td>"
                                "<td>" ) %
                mCaps.mCapabilities.capability.request.getFeatureInfo.format.join( QLatin1String( "<br />" ) ) %
                QStringLiteral( "</td></tr>"

                                // Layer Count (as managed by this provider)
                                "<tr><td>" ) %
                tr( "Layer Count" ) %
                QStringLiteral( "</td>"
                                "<td>" ) %
                QString::number( mCaps.mLayersSupported.size() ) %
                QStringLiteral( "</td></tr>" );
  }

  // Close the nested table 2
  metadata += QStringLiteral( "</table>"
                              "</td></tr>" );

  // Layer properties
  if ( !mSettings.mTiled )
  {
    metadata += QStringLiteral( "<tr><th class=\"strong\" id=\"selectedlayers\">" ) %
                tr( "Selected Layers" ) %
                QStringLiteral( "</th></tr>" );

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
      metadata += QStringLiteral( "<tr><th class=\"strong\" id=\"otherlayers\">" ) %
                  tr( "Other Layers" ) %
                  QStringLiteral( "</th></tr>" );

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
    metadata += QStringLiteral( "<tr><th class=\"strong\" id=\"tilesetproperties\">" ) %
                tr( "Tileset Properties" ) %
                QStringLiteral( "</th></tr>"

                                // Iterate through tilesets
                                "<tr><td>"

                                "<table width=\"100%\" class=\"tabular-view\">" );  // Nested table 3

    for ( const QgsWmtsTileLayer &l : std::as_const( mCaps.mTileLayersSupported ) )
    {
      metadata += QStringLiteral( "<tr><th class=\"strong\">" ) %
                  tr( "Identifier" ) %
                  QStringLiteral( "</th><th class=\"strong\">" ) %
                  tr( "Tile mode" ) %
                  QStringLiteral( "</th></tr>"

                                  "<tr><td>" ) %
                  l.identifier %
                  QStringLiteral( "</td><td class=\"strong\">" );

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
                                  "<tr><th class=\"strong\">" ) %
                  tr( "Property" ) %
                  QStringLiteral( "</th>"
                                  "<th class=\"strong\">" ) %
                  tr( "Value" ) %
                  QStringLiteral( "</th></tr>"

                                  "<tr><td class=\"strong\">" ) %
                  tr( "Title" ) %
                  QStringLiteral( "</td>"
                                  "<td>" ) %
                  l.title %
                  QStringLiteral( "</td></tr>"

                                  "<tr><td class=\"strong\">" ) %
                  tr( "Abstract" ) %
                  QStringLiteral( "</td>"
                                  "<td>" ) %
                  l.abstract %
                  QStringLiteral( "</td></tr>"

                                  "<tr><td class=\"strong\">" ) %
                  tr( "Selected" ) %
                  QStringLiteral( "</td>"
                                  "<td class=\"strong\">" ) %
                  ( l.identifier == mSettings.mActiveSubLayers.join( QLatin1Char( ',' ) ) ? tr( "Yes" ) : tr( "No" ) ) %
                  QStringLiteral( "</td></tr>" );

      if ( !l.styles.isEmpty() )
      {
        metadata += QStringLiteral( "<tr><td class=\"strong\">" ) %
                    tr( "Available Styles" ) %
                    QStringLiteral( "</td>"
                                    "<td class=\"strong\">" );
        QStringList styles;
        for ( const QgsWmtsStyle &style : std::as_const( l.styles ) )
        {
          styles << style.identifier;
        }
        metadata += styles.join( QLatin1String( ", " ) ) %
                    QStringLiteral( "</td></tr>" );
      }

      metadata += QStringLiteral( "<tr><td class=\"strong\">" ) %
                  tr( "CRS" ) %
                  QStringLiteral( "</td>"
                                  "<td>"
                                  "<table class=\"tabular-view\"><tr>" // Nested table 4
                                  "<td class=\"strong\">" ) %
                  tr( "CRS" ) %
                  QStringLiteral( "</td>"
                                  "<td class=\"strong\">" ) %
                  tr( "Bounding Box" ) %
                  QStringLiteral( "</td>" );
      for ( int i = 0; i < l.boundingBoxes.size(); i++ )
      {
        metadata += QStringLiteral( "<tr><td>" ) %
                    l.boundingBoxes[i].crs %
                    QStringLiteral( "</td><td>" ) %
                    l.boundingBoxes[i].box.toString() %
                    QStringLiteral( "</td></tr>" );
      }
      metadata += QStringLiteral( "</table></td></tr>"  // End nested table 4
                                  "<tr><td class=\"strong\">" ) %
                  tr( "Available Tilesets" ) %
                  QStringLiteral( "</td><td class=\"strong\">" );

      for ( const QgsWmtsTileMatrixSetLink &setLink : std::as_const( l.setLinks ) )
      {
        metadata += setLink.tileMatrixSet + "<br>";
      }

      metadata += QLatin1String( "</td></tr>" );
    }

    metadata += QLatin1String( "</table></td></tr>" ); // End nested table 3

    if ( mTileMatrixSet )
    {
      // Iterate through tilesets
      metadata += QStringLiteral( "<tr><td><table width=\"100%\" class=\"tabular-view\">"  // Nested table 3

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
                  .arg( tr( "Selected tile matrix set " ),
                        mSettings.mTileMatrixSetId,
                        tr( "Scale" ),
                        tr( "Tile size [px]" ),
                        tr( "Tile size [mu]" ),
                        tr( "Matrix size" ),
                        tr( "Matrix extent [mu]" ) )
                  .arg( tr( "Bounds" ),
                        tr( "Width" ),
                        tr( "Height" ),
                        tr( "Top" ),
                        tr( "Left" ),
                        tr( "Bottom" ),
                        tr( "Right" ) );

      for ( const double key : mNativeResolutions )
      {
        QgsWmtsTileMatrix &tm = mTileMatrixSet->tileMatrices[ key ];

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
                    .arg( tm.tileWidth ).arg( tm.tileHeight )
                    .arg( tw ).arg( th )
                    .arg( tm.matrixWidth ).arg( tm.matrixHeight )
                    .arg( tw * tm.matrixWidth, 0, 'f' )
                    .arg( th * tm.matrixHeight, 0, 'f' );

        // top
        if ( mLayerExtent.yMaximum() > r.yMaximum() )
        {
          metadata += QStringLiteral( "<td title=\"%1<br>%2\"><font color=\"red\">%3</font></td>" )
                      .arg( tr( "%n missing row(s)", nullptr, ( int ) std::ceil( ( mLayerExtent.yMaximum() - r.yMaximum() ) / th ) ),
                            tr( "Layer's upper bound: %1" ).arg( mLayerExtent.yMaximum(), 0, 'f' ) )
                      .arg( r.yMaximum(), 0, 'f' );
        }
        else
        {
          metadata += QStringLiteral( "<td>%1</td>" ).arg( r.yMaximum(), 0, 'f' );
        }

        // left
        if ( mLayerExtent.xMinimum() < r.xMinimum() )
        {
          metadata += QStringLiteral( "<td title=\"%1<br>%2\"><font color=\"red\">%3</font></td>" )
                      .arg( tr( "%n missing column(s)", nullptr, ( int ) std::ceil( ( r.xMinimum() - mLayerExtent.xMinimum() ) / tw ) ),
                            tr( "Layer's left bound: %1" ).arg( mLayerExtent.xMinimum(), 0, 'f' ) )
                      .arg( r.xMinimum(), 0, 'f' );
        }
        else
        {
          metadata += QStringLiteral( "<td>%1</td>" ).arg( r.xMinimum(), 0, 'f' );
        }

        // bottom
        if ( mLayerExtent.yMaximum() > r.yMaximum() )
        {
          metadata += QStringLiteral( "<td title=\"%1<br>%2\"><font color=\"red\">%3</font></td>" )
                      .arg( tr( "%n missing row(s)", nullptr, ( int ) std::ceil( ( mLayerExtent.yMaximum() - r.yMaximum() ) / th ) ),
                            tr( "Layer's lower bound: %1" ).arg( mLayerExtent.yMaximum(), 0, 'f' ) )
                      .arg( r.yMaximum(), 0, 'f' );
        }
        else
        {
          metadata += QStringLiteral( "<td>%1</td>" ).arg( r.yMaximum(), 0, 'f' );
        }

        // right
        if ( mLayerExtent.xMaximum() > r.xMaximum() )
        {
          metadata +=  QStringLiteral( "<td title=\"%1<br>%2\"><font color=\"red\">%3</font></td>" )
                       .arg( tr( "%n missing column(s)", nullptr, ( int ) std::ceil( ( mLayerExtent.xMaximum() - r.xMaximum() ) / tw ) ),
                             tr( "Layer's right bound: %1" ).arg( mLayerExtent.xMaximum(), 0, 'f' ) )
                       .arg( r.xMaximum(), 0, 'f' );
        }
        else
        {
          metadata += QStringLiteral( "<td>%1</td>" ).arg( r.xMaximum(), 0, 'f' );
        }

        metadata += QLatin1String( "</tr>" );
      }

      metadata += QLatin1String( "</table></td></tr>" );  // End nested table 3
    }

    const QgsWmsStatistics::Stat &stat = QgsWmsStatistics::statForUri( dataSourceUri() );

    metadata += QStringLiteral( "<tr><th class=\"strong\" id=\"cachestats\">" ) %
                tr( "Cache stats" ) %
                QStringLiteral( "</th></tr>"

                                "<tr><td><table width=\"100%\" class=\"tabular-view\">"  // Nested table 3

                                "<tr><th class=\"strong\">" ) %
                tr( "Property" ) %
                QStringLiteral( "</th>"
                                "<th class=\"strong\">" ) %
                tr( "Value" ) %
                QStringLiteral( "</th></tr>"

                                "<tr><td>" ) %
                tr( "Hits" ) %
                QStringLiteral( "</td><td>" ) %
                QString::number( stat.cacheHits ) %
                QStringLiteral( "</td></tr>"

                                "<tr><td>" ) %
                tr( "Misses" ) %
                QStringLiteral( "</td><td>" ) %
                QString::number( stat.cacheMisses ) %
                QStringLiteral( "</td></tr>"

                                "<tr><td>" ) %
                tr( "Errors" ) %
                QStringLiteral( "</td><td>" ) %
                QString::number( stat.errors ) %
                QStringLiteral( "</td></tr>"

                                "</table></td></tr>" );  // End nested table 3
  }

  metadata += QStringLiteral( "</table>" // End nested table 2
                              "</table></div></td></tr>\n" );  // End nested table 1

  return metadata;
}

QgsRasterIdentifyResult QgsWmsProvider::identify( const QgsPointXY &point, QgsRaster::IdentifyFormat format, const QgsRectangle &boundingBox, int width, int height, int /*dpi*/ )
{
  QgsDebugMsgLevel( QStringLiteral( "format = %1" ).arg( format ), 2 );

  QString formatStr;
  formatStr = mCaps.mIdentifyFormats.value( format );
  if ( formatStr.isEmpty() )
  {
    return QgsRasterIdentifyResult( QGS_ERROR( tr( "Format not supported" ) ) );
  }

  QgsDebugMsgLevel( QStringLiteral( "format = %1 format = %2" ).arg( format ).arg( formatStr ), 2 );

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
        case QgsUnitTypes::DistanceMeters:
          xRes = 0.001;
          break;
        case QgsUnitTypes::DistanceFeet:
          xRes = 0.003;
          break;
        case QgsUnitTypes::DistanceDegrees:
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

    myExtent = QgsRectangle( point.x() - xRes, point.y() - yRes,
                             point.x() + xRes, point.y() + yRes );
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
  QgsDebugMsgLevel( QStringLiteral( "theWidth = %1 height = %2" ).arg( width ).arg( height ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "xRes = %1 yRes = %2" ).arg( xRes ).arg( yRes ), 2 );

  QgsPointXY finalPoint;
  finalPoint.setX( std::floor( ( point.x() - myExtent.xMinimum() ) / xRes ) );
  finalPoint.setY( std::floor( ( myExtent.yMaximum() - point.y() ) / yRes ) );

  QgsDebugMsgLevel( QStringLiteral( "point = %1 %2" ).arg( finalPoint.x() ).arg( finalPoint.y() ), 2 );

  QgsDebugMsgLevel( QStringLiteral( "recalculated orig point (corner) = %1 %2" ).arg( myExtent.xMinimum() + finalPoint.x()*xRes ).arg( myExtent.yMaximum() - finalPoint.y()*yRes ), 2 );

  // Collect which layers to query on
  //according to the WMS spec for 1.3, the order of x - and y - coordinates is inverted for geographical CRS
  bool changeXY = mCaps.shouldInvertAxisOrientation( mImageCrs );

  // compose the URL query string for the WMS server.
  QString crsKey = QStringLiteral( "SRS" ); //SRS in 1.1.1 and CRS in 1.3.0
  if ( mCaps.mCapabilities.version == QLatin1String( "1.3.0" ) || mCaps.mCapabilities.version == QLatin1String( "1.3" ) )
  {
    crsKey = QStringLiteral( "CRS" );
  }

  // Compose request to WMS server
  QString bbox = QString( changeXY ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
                 .arg( qgsDoubleToString( myExtent.xMinimum() ),
                       qgsDoubleToString( myExtent.yMinimum() ),
                       qgsDoubleToString( myExtent.xMaximum() ),
                       qgsDoubleToString( myExtent.yMaximum() ) );

  //QgsFeatureList featureList;

  QList<QUrl> urls;
  QStringList layerList;

  if ( !mSettings.mTiled )
  {
    // Test for which layers are suitable for querying with
    for ( QStringList::const_iterator
          layers = mSettings.mActiveSubLayers.constBegin(),
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
      setQueryItem( query, QStringLiteral( "SERVICE" ), QStringLiteral( "WMS" ) );
      setQueryItem( query, QStringLiteral( "VERSION" ), mCaps.mCapabilities.version );
      setQueryItem( query, QStringLiteral( "REQUEST" ), QStringLiteral( "GetFeatureInfo" ) );
      setQueryItem( query, QStringLiteral( "BBOX" ), bbox );
      setSRSQueryItem( query );
      setQueryItem( query, QStringLiteral( "WIDTH" ), QString::number( width ) );
      setQueryItem( query, QStringLiteral( "HEIGHT" ), QString::number( height ) );
      setQueryItem( query, QStringLiteral( "LAYERS" ), *layers );
      setQueryItem( query, QStringLiteral( "STYLES" ), *styles );
      setFormatQueryItem( query );
      setQueryItem( query, QStringLiteral( "QUERY_LAYERS" ), *layers );
      setQueryItem( query, QStringLiteral( "INFO_FORMAT" ), formatStr );

      if ( mCaps.mCapabilities.version == QLatin1String( "1.3.0" ) || mCaps.mCapabilities.version == QLatin1String( "1.3" ) )
      {
        setQueryItem( query, QStringLiteral( "I" ), QString::number( finalPoint.x() ) );
        setQueryItem( query, QStringLiteral( "J" ), QString::number( finalPoint.y() ) );
      }
      else
      {
        setQueryItem( query, QStringLiteral( "X" ), QString::number( finalPoint.x() ) );
        setQueryItem( query, QStringLiteral( "Y" ), QString::number( finalPoint.y() ) );
      }

      if ( mSettings.mFeatureCount > 0 )
      {
        setQueryItem( query, QStringLiteral( "FEATURE_COUNT" ), QString::number( mSettings.mFeatureCount ) );
      }

      // For WMS-T layers
      if ( temporalCapabilities() &&
           temporalCapabilities()->hasTemporalCapabilities() )
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
      QgsDebugMsgLevel( QStringLiteral( "res:%1 >= %2" ).arg( it.key() ).arg( vres ), 2 );
      prev = it;
      ++it;
    }

    if ( it == m.constEnd() ||
         ( it != m.constBegin() && vres - prev.key() < it.key() - vres ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "back to previous res" ), 2 );
      it = prev;
    }

    tres = it.key();
    tm = &it.value();

    QgsDebugMsgLevel( QStringLiteral( "layer extent: %1,%2,%3,%4 %5x%6" )
                      .arg( qgsDoubleToString( mLayerExtent.xMinimum() ),
                            qgsDoubleToString( mLayerExtent.yMinimum() ) )
                      .arg( qgsDoubleToString( mLayerExtent.xMaximum() ),
                            qgsDoubleToString( mLayerExtent.yMaximum() ) )
                      .arg( mLayerExtent.width() )
                      .arg( mLayerExtent.height() ), 2
                    );

    QgsDebugMsgLevel( QStringLiteral( "view extent: %1,%2,%3,%4 %5x%6  res:%7" )
                      .arg( qgsDoubleToString( boundingBox.xMinimum() ),
                            qgsDoubleToString( boundingBox.yMinimum() ) )
                      .arg( qgsDoubleToString( boundingBox.xMaximum() ),
                            qgsDoubleToString( boundingBox.yMaximum() ) )
                      .arg( boundingBox.width() )
                      .arg( boundingBox.height() )
                      .arg( vres, 0, 'f' ), 2
                    );

    QgsDebugMsgLevel( QStringLiteral( "tile matrix %1,%2 res:%3 tilesize:%4x%5 matrixsize:%6x%7 id:%8" )
                      .arg( tm->topLeft.x() ).arg( tm->topLeft.y() ).arg( tres )
                      .arg( tm->tileWidth ).arg( tm->tileHeight )
                      .arg( tm->matrixWidth ).arg( tm->matrixHeight )
                      .arg( tm->identifier ), 2
                    );

    // calculate tile coordinates
    double twMap = tm->tileWidth * tres;
    double thMap = tm->tileHeight * tres;
    QgsDebugMsgLevel( QStringLiteral( "tile map size: %1,%2" ).arg( qgsDoubleToString( twMap ), qgsDoubleToString( thMap ) ), 2 );

    int col = ( int ) std::floor( ( point.x() - tm->topLeft.x() ) / twMap );
    int row = ( int ) std::floor( ( tm->topLeft.y() - point.y() ) / thMap );
    double tx = tm->topLeft.x() + col * twMap;
    double ty = tm->topLeft.y() - row * thMap;
    int i   = ( point.x() - tx ) / tres;
    int j   = ( ty - point.y() ) / tres;

    QgsDebugMsgLevel( QStringLiteral( "col=%1 row=%2 i=%3 j=%4 tx=%5 ty=%6" ).arg( col ).arg( row ).arg( i ).arg( j ).arg( tx, 0, 'f', 1 ).arg( ty, 0, 'f', 1 ), 2 );

    if ( mTileLayer->getFeatureInfoURLs.contains( formatStr ) )
    {
      // REST


      QString url = mTileLayer->getFeatureInfoURLs[ formatStr ];

      if ( mSettings.mIgnoreGetFeatureInfoUrl )
      {
        // rewrite the URL if the one in the capabilities document is incorrect
        // strip every thing after the ? from the base url
        const QStringList parts = mSettings.mBaseUrl.split( QRegularExpression( "\\?" ) );
        const QString base = parts.isEmpty() ? mSettings.mBaseUrl : parts.first();
        // and strip everything before the `rest` element (at least for GeoServer)
        const int index = url.length() - url.lastIndexOf( QLatin1String( "rest" ) ) + 1; // +1 for the /
        url = base + url.right( index );
      }

      QgsDebugMsgLevel( QStringLiteral( "getfeatureinfo: %1" ).arg( url ), 2 );

      url.replace( QLatin1String( "{layer}" ), mSettings.mActiveSubLayers[0], Qt::CaseInsensitive );
      url.replace( QLatin1String( "{style}" ), mSettings.mActiveSubStyles[0], Qt::CaseInsensitive );
      url.replace( QLatin1String( "{tilematrixset}" ), mTileMatrixSet->identifier, Qt::CaseInsensitive );
      url.replace( QLatin1String( "{tilematrix}" ), tm->identifier, Qt::CaseInsensitive );
      url.replace( QLatin1String( "{tilerow}" ), QString::number( row ), Qt::CaseInsensitive );
      url.replace( QLatin1String( "{tilecol}" ), QString::number( col ), Qt::CaseInsensitive );
      url.replace( QLatin1String( "{i}" ), QString::number( i ), Qt::CaseInsensitive );
      url.replace( QLatin1String( "{j}" ), QString::number( j ), Qt::CaseInsensitive );

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
      setQueryItem( query, QStringLiteral( "SERVICE" ), QStringLiteral( "WMTS" ) );
      setQueryItem( query, QStringLiteral( "REQUEST" ), QStringLiteral( "GetFeatureInfo" ) );
      setQueryItem( query, QStringLiteral( "VERSION" ), mCaps.mCapabilities.version );
      setQueryItem( query, QStringLiteral( "LAYER" ), mSettings.mActiveSubLayers[0] );
      setQueryItem( query, QStringLiteral( "STYLE" ), mSettings.mActiveSubStyles[0] );
      setQueryItem( query, QStringLiteral( "INFOFORMAT" ), formatStr );
      setQueryItem( query, QStringLiteral( "TILEMATRIXSET" ), mTileMatrixSet->identifier );
      setQueryItem( query, QStringLiteral( "TILEMATRIX" ), tm->identifier );

      for ( QHash<QString, QString>::const_iterator it = mSettings.mTileDimensionValues.constBegin(); it != mSettings.mTileDimensionValues.constEnd(); ++it )
      {
        setQueryItem( query, it.key(), it.value() );
      }

      setQueryItem( query, QStringLiteral( "TILEROW" ), QString::number( row ) );
      setQueryItem( query, QStringLiteral( "TILECOL" ), QString::number( col ) );
      setQueryItem( query, QStringLiteral( "I" ), qgsDoubleToString( i ) );
      setQueryItem( query, QStringLiteral( "J" ), qgsDoubleToString( j ) );
      url.setQuery( query );

      urls << url;
      layerList << mSettings.mActiveSubLayers[0];
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "No KVP and no feature info url for format %1" ).arg( formatStr ) );
    }
  }

  for ( int count = 0; count < urls.size(); count++ )
  {
    const QUrl &requestUrl = urls[count];

    QgsDebugMsgLevel( QStringLiteral( "getfeatureinfo: %1" ).arg( requestUrl.toString() ), 2 );
    QNetworkRequest request( requestUrl );
    QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsWmsProvider" ) );
    QgsSetRequestInitiatorId( request, QStringLiteral( "identify %1,%2" ).arg( point.x() ).arg( point.y() ) );
    mSettings.authorization().setAuthorization( request );
    mIdentifyReply = QgsNetworkAccessManager::instance()->get( request );
    connect( mIdentifyReply, &QNetworkReply::finished, this, &QgsWmsProvider::identifyReplyFinished );

    QEventLoop loop;
    mIdentifyReply->setProperty( "eventLoop", QVariant::fromValue( qobject_cast<QObject *>( &loop ) ) );
    loop.exec( QEventLoop::ExcludeUserInputEvents );

    if ( mIdentifyResultBodies.isEmpty() ) // no result
    {
      QgsDebugMsg( QStringLiteral( "mIdentifyResultBodies is empty" ) );
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
        if ( QString( it.key() ).compare( QLatin1String( "Content-Type" ), Qt::CaseInsensitive ) == 0 )
        {
          isXml = QString( it.value() ).compare( QLatin1String( "text/xml" ), Qt::CaseInsensitive ) == 0;
          isGml = QString( it.value() ).compare( QLatin1String( "ogr/gml" ), Qt::CaseInsensitive ) == 0;
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
          QgsMessageLog::logMessage( tr( "Get feature info request error (Title: %1; Error: %2; URL: %3)" )
                                     .arg( mErrorCaption, mError,
                                           requestUrl.toString() ), tr( "WMS" ) );
          continue;
        }
      }
    }

    if ( format == QgsRaster::IdentifyFormatHtml || format == QgsRaster::IdentifyFormatText )
    {
      results.insert( results.size(), QString::fromUtf8( mIdentifyResultBodies.value( 0 ) ) );
    }
    else if ( format == QgsRaster::IdentifyFormatFeature ) // GML
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
      for ( int i = 0; i < mIdentifyResultHeaders.size(); i++ )
      {
        if ( xsdPart == -1 && mIdentifyResultHeaders.at( i ).value( "Content-Disposition" ).contains( ".xsd" ) )
        {
          xsdPart = i;
        }
        else if ( gmlPart == -1 && mIdentifyResultHeaders.at( i ).value( "Content-Disposition" ).contains( ".dat" ) )
        {
          gmlPart = i;
        }
        else if ( jsonPart == -1 && mIdentifyResultHeaders.at( i ).value( "Content-Type" ).contains( "json" ) )
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
          QgsDebugMsg( QStringLiteral( "Multipart with 2 parts - expected GML + XSD" ) );
          // How to find which part is GML and which XSD? Both have
          // Content-Type: application/binary
          // different are Content-Disposition but it is not reliable.
          // We could analyze beginning of bodies...
          gmlPart = 0;
          xsdPart = 1;
        }
      }
      QgsDebugMsgLevel( QStringLiteral( "jsonPart = %1 gmlPart = %2 xsdPart = %3" ).arg( jsonPart ).arg( gmlPart ).arg( xsdPart ), 2 );

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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        stream.setCodec( QTextCodec::codecForName( "UTF-8" ) );
#endif
        dom.save( stream, 4, QDomNode::EncodingFromTextStream );

        QgsDebugMsgLevel( "GML UTF-8 (first 2000 bytes):\n" + gmlByteArray.left( 2000 ), 2 );

        QgsWkbTypes::Type wkbType;
        QgsGmlSchema gmlSchema;

        if ( xsdPart >= 0 )  // XSD available
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
            QgsDebugMsg( "guess schema error: " + err.message() );
            return QgsRasterIdentifyResult( err );
          }
        }

        QStringList featureTypeNames = gmlSchema.typeNames();
        QgsDebugMsgLevel( QStringLiteral( "%1 featureTypeNames found" ).arg( featureTypeNames.size() ), 2 );

        // Each sublayer may have more features of different types, for example
        // if GROUP of multiple vector layers is used with UMN MapServer
        // Note: GROUP of layers in UMN MapServer is not queryable by default
        // (and I could not find a way to force it), it is possible however
        // to add another RASTER layer with the same name as group which is queryable
        // and has no DATA defined. Then such a layer may be add to QGIS and both
        // GetMap and GetFeatureInfo will return data for the group of the same name.
        // https://github.com/mapserver/mapserver/issues/318#issuecomment-4923208
        QgsFeatureStoreList featureStoreList;
        const auto constFeatureTypeNames = featureTypeNames;
        for ( const QString &featureTypeName : constFeatureTypeNames )
        {
          QgsDebugMsgLevel( QStringLiteral( "featureTypeName = %1" ).arg( featureTypeName ), 2 );

          QString geometryAttribute = gmlSchema.geometryAttributes( featureTypeName ).value( 0 );
          QList<QgsField> fieldList = gmlSchema.fields( featureTypeName );
          QgsDebugMsgLevel( QStringLiteral( "%1 fields found" ).arg( fieldList.size() ), 2 );
          QgsFields fields;
          for ( int i = 0; i < fieldList.size(); i++ )
          {
            fields.append( fieldList[i] );
          }
          QgsGml gml( featureTypeName, geometryAttribute, fields );
          // TODO: avoid converting to string and back
          int ret = gml.getFeatures( gmlByteArray, &wkbType );
#ifdef QGISDEBUG
          QgsDebugMsgLevel( QStringLiteral( "parsing result = %1" ).arg( ret ), 2 );
#else
          Q_UNUSED( ret )
#endif
          // TODO: all features coming from this layer should probably have the same CRS
          // the same as this layer, because layerExtentToOutputExtent() may be used
          // for results -> verify CRS and reprojects if necessary
          QMap<QgsFeatureId, QgsFeature * > features = gml.featuresMap();
          QgsCoordinateReferenceSystem featuresCrs = gml.crs();
          QgsDebugMsgLevel( QStringLiteral( "%1 features read, crs: %2" ).arg( features.size() ).arg( featuresCrs.userFriendlyIdentifier() ), 2 );
          QgsCoordinateTransform coordinateTransform;
          if ( featuresCrs.isValid() && featuresCrs != crs() )
          {
            coordinateTransform = QgsCoordinateTransform( featuresCrs, crs(), transformContext() );
          }
          QgsFeatureStore featureStore( fields, crs() );
          QMap<QString, QVariant> params;
          params.insert( QStringLiteral( "sublayer" ), layerList[count] );
          params.insert( QStringLiteral( "featureType" ), featureTypeName );
          params.insert( QStringLiteral( "getFeatureInfoUrl" ), requestUrl.toString() );
          featureStore.setParams( params );
          QMap<QgsFeatureId, QgsFeature * >::const_iterator featIt = features.constBegin();
          for ( ; featIt != features.constEnd(); ++featIt )
          {
            QgsFeature *feature = featIt.value();

            QgsDebugMsgLevel( QStringLiteral( "feature id = %1 : %2 attributes" ).arg( featIt.key() ).arg( feature->attributes().size() ), 2 );

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
          err.append( tr( "Result parsing failed. %n feature type(s) were guessed from gml (%2) but no features were parsed.", nullptr, featureTypeNames.size() ).arg( featureTypeNames.join( QLatin1Char( ',' ) ) ) );
          QgsDebugMsg( "parsing GML error: " + err.message() );
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
            throw QStringLiteral( "Doc expected" );
          if ( !doc.isObject() )
            throw QStringLiteral( "Object expected" );

          QJsonObject result = doc.object();
          if ( result.value( QLatin1String( "type" ) ).toString() != QLatin1String( "FeatureCollection" ) )
            throw QStringLiteral( "Type FeatureCollection expected: %1" ).arg( result.value( QLatin1String( "type" ) ).toString() );

          if ( result.value( QLatin1String( "crs" ) ).isObject() )
          {
            QString crsType = result.value( QLatin1String( "crs" ) ).toObject().value( QLatin1String( "type" ) ).toString();
            QString crsText;
            if ( crsType == QLatin1String( "name" ) )
              crsText = result.value( QStringLiteral( "crs" ) ).toObject().value( QLatin1String( "properties" ) ).toObject().value( QLatin1String( "name" ) ).toString();
            else if ( crsType == QLatin1String( "EPSG" ) )
              crsText = QStringLiteral( "%1:%2" ).arg( crsType, result.value( QLatin1String( "crs" ) ).toObject().value( QLatin1String( "properties" ) ).toObject().value( QStringLiteral( "code" ) ).toString() );
            else
            {
              QgsDebugMsg( QStringLiteral( "crs not supported:%1" ).arg( result.value( QLatin1String( "crs" ) ).toString() ) );
            }

            QgsCoordinateReferenceSystem featuresCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsText );

            if ( !featuresCrs.isValid() )
              throw QStringLiteral( "CRS %1 invalid" ).arg( crsText );

            if ( featuresCrs.isValid() && featuresCrs != crs() )
            {
              coordinateTransform = QgsCoordinateTransform( featuresCrs, crs(), transformContext() );
            }
          }

          const QJsonValue fc = result.value( QLatin1String( "features" ) );
          if ( !fc.isArray() )
            throw QStringLiteral( "FeatureCollection array expected" );

          const QJsonArray features = fc.toArray();

          int i = -1;
          for ( const QJsonValue &fv : features )
          {
            ++i;
            const QJsonObject f = fv.toObject();
            const QJsonValue props = f.value( QLatin1String( "properties" ) );
            if ( !props.isObject() )
            {
              QgsDebugMsgLevel( QStringLiteral( "no properties found" ), 2 );
              continue;
            }

            QgsFields fields;

            const QJsonObject properties = props.toObject();
            auto fieldIterator = properties.constBegin();

            for ( ; fieldIterator != properties.constEnd(); ++fieldIterator )
            {
              fields.append( QgsField( fieldIterator.key(), QVariant::String ) );
            }

            QgsFeature feature( fields );

            if ( f.value( QLatin1String( "geometry" ) ).isObject() )
            {
              QJsonDocument serializer( f.value( QLatin1String( "geometry" ) ).toObject() );
              QString geom = serializer.toJson( QJsonDocument::JsonFormat::Compact );

              gdal::ogr_geometry_unique_ptr ogrGeom( OGR_G_CreateGeometryFromJson( geom.toUtf8() ) );
              if ( ogrGeom )
              {
                int wkbSize = OGR_G_WkbSize( ogrGeom.get() );
                unsigned char *wkb = new unsigned char[ wkbSize ];
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
            params.insert( QStringLiteral( "sublayer" ), layerList[count] );
            params.insert( QStringLiteral( "featureType" ), QStringLiteral( "%1_%2" ).arg( count ).arg( i ) );
            params.insert( QStringLiteral( "getFeatureInfoUrl" ), requestUrl.toString() );
            featureStore.setParams( params );

            // Try to parse and set feature id if matches "<some string>.<integer>"
            if ( f.value( QLatin1String( "id" ) ).isString() )
            {
              const thread_local QRegularExpression re{ R"raw(\.(\d+)$)raw" };
              const QString idVal { f.value( QLatin1String( "id" ) ).toString() };
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
          QgsDebugMsg( QStringLiteral( "JSON error: %1\nResult: %2" ).arg( err, QString::fromUtf8( mIdentifyResultBodies.value( jsonPart ) ) ) );
          results.insert( results.size(), err );  // string returned for format type "feature" means error
        }

        results.insert( results.size(), QVariant::fromValue( featureStoreList ) );
      }
    }
  }

  return QgsRasterIdentifyResult( format, results );
}

void QgsWmsProvider::identifyReplyFinished()
{
  QgsDebugMsgLevel( QStringLiteral( "Entered." ), 4 );
  mIdentifyResultHeaders.clear();
  mIdentifyResultBodies.clear();

  QEventLoop *loop = qobject_cast< QEventLoop *>( sender()->property( "eventLoop" ).value< QObject *>() );

  if ( mIdentifyReply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = mIdentifyReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      QgsDebugMsgLevel( QStringLiteral( "identify request redirected to %1" ).arg( redirect.toString() ), 2 );

      mIdentifyReply->deleteLater();

      QgsDebugMsgLevel( QStringLiteral( "redirected getfeatureinfo: %1" ).arg( redirect.toString() ), 2 );
      mIdentifyReply = QgsNetworkAccessManager::instance()->get( QNetworkRequest( redirect.toUrl() ) );
      mSettings.authorization().setAuthorizationReply( mIdentifyReply );
      mIdentifyReply->setProperty( "eventLoop", QVariant::fromValue( qobject_cast<QObject *>( loop ) ) );
      connect( mIdentifyReply, &QNetworkReply::finished, this, &QgsWmsProvider::identifyReplyFinished );
      return;
    }

    QVariant status = mIdentifyReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
    {
      QVariant phrase = mIdentifyReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
      mErrorFormat = QStringLiteral( "text/plain" );
      mError = tr( "Map getfeatureinfo error %1: %2" ).arg( status.toInt() ).arg( phrase.toString() );
      emit statusChanged( mError );
    }

    QgsNetworkReplyParser parser( mIdentifyReply );
    if ( !parser.isValid() )
    {
      QgsDebugMsg( QStringLiteral( "Cannot parse reply" ) );
      mErrorFormat = QStringLiteral( "text/plain" );
      mError = tr( "Cannot parse getfeatureinfo: %1" ).arg( parser.error() );
      emit statusChanged( mError );
    }
    else
    {
      // TODO: check headers, xsd ...
      QgsDebugMsgLevel( QStringLiteral( "%1 parts" ).arg( parser.parts() ), 2 );
      mIdentifyResultBodies = parser.bodies();
      mIdentifyResultHeaders = parser.headers();
    }
  }
  else
  {
    //mIdentifyResult = tr( "ERROR: GetFeatureInfo failed" );
    mErrorFormat = QStringLiteral( "text/plain" );
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

QgsRasterDataProvider::ProviderCapabilities QgsWmsProvider::providerCapabilities() const
{
  if ( mConverter )
    return ProviderCapability::ReadLayerMetadata |
           ProviderCapability::ProviderHintBenefitsFromResampling |
           ProviderCapability::ProviderHintCanPerformProviderResampling;

  return ProviderCapability::ReadLayerMetadata;
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

QString  QgsWmsProvider::name() const
{
  return WMS_KEY;
}

QString QgsWmsProvider::providerKey()
{
  return WMS_KEY;
}

QString  QgsWmsProvider::description() const
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

QgsRasterBandStats QgsWmsProvider::bandStatistics(
  int bandNo,
  int stats,
  const QgsRectangle &extent,
  int sampleSize,
  QgsRasterBlockFeedback *feedback )
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
  QgsRasterBlockFeedback *feedback )
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
    QgsWmsSupportedFormat p3 = { "image/png8", "PNG8" }; // used by geoserver
    QgsWmsSupportedFormat p4 = { "image/png; mode=8bit", "PNG8" }; // used by QGIS server and UMN mapserver
    QgsWmsSupportedFormat p5 = { "png", "PNG" }; // used by french IGN geoportail
    QgsWmsSupportedFormat p6 = { "pngt", "PNGT" }; // used by french IGN geoportail

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
    QgsWmsSupportedFormat g2 = { "image/jpgpng", "JPEG/PNG" }; // used by ESRI
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
    QgsDebugMsgLevel( QStringLiteral( "getLegendGraphic url is empty" ), 2 );
    return QUrl();
  }

  QgsDebugMsgLevel( QStringLiteral( "visibleExtent is %1" ).arg( visibleExtent.toString() ), 2 );

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
  if ( !qnames.contains( QStringLiteral( "SERVICE" ) ) )
    setQueryItem( query, QStringLiteral( "SERVICE" ), QStringLiteral( "WMS" ) );
  if ( !qnames.contains( QStringLiteral( "VERSION" ) ) )
    setQueryItem( query, QStringLiteral( "VERSION" ), mCaps.mCapabilities.version );
  if ( !qnames.contains( QStringLiteral( "SLD_VERSION" ) ) )
    setQueryItem( query, QStringLiteral( "SLD_VERSION" ), QStringLiteral( "1.1.0" ) ); // can not determine SLD_VERSION
  if ( !qnames.contains( QStringLiteral( "REQUEST" ) ) )
    setQueryItem( query, QStringLiteral( "REQUEST" ), QStringLiteral( "GetLegendGraphic" ) );
  if ( !qnames.contains( QStringLiteral( "FORMAT" ) ) )
    setFormatQueryItem( query );
  if ( !qnames.contains( QStringLiteral( "LAYER" ) ) )
    setQueryItem( query, QStringLiteral( "LAYER" ), mSettings.mActiveSubLayers[0] );
  if ( !qnames.contains( QStringLiteral( "STYLE" ) ) )
    setQueryItem( query, QStringLiteral( "STYLE" ), mSettings.mActiveSubStyles[0] );
  // by setting TRANSPARENT=true, even too big legend images will look good
  if ( !qnames.contains( QStringLiteral( "TRANSPARENT" ) ) )
    setQueryItem( query, QStringLiteral( "TRANSPARENT" ), QStringLiteral( "true" ) );

  // add config parameter related to resolution
  QgsSettings s;
  int defaultLegendGraphicResolution = s.value( QStringLiteral( "qgis/defaultLegendGraphicResolution" ), 0 ).toInt();
  QgsDebugMsgLevel( QStringLiteral( "defaultLegendGraphicResolution: %1" ).arg( defaultLegendGraphicResolution ), 2 );
  if ( defaultLegendGraphicResolution )
  {
    if ( mSettings.mDpiMode & DpiQGIS )
      setQueryItem( query, QStringLiteral( "DPI" ), QString::number( defaultLegendGraphicResolution ) );
    if ( mSettings.mDpiMode & DpiUMN )
    {
      setQueryItem( query, QStringLiteral( "MAP_RESOLUTION" ), QString::number( defaultLegendGraphicResolution ) );
      setQueryItem( query, QStringLiteral( "SCALE" ), QString::number( scale, 'f' ) );
    }
    if ( mSettings.mDpiMode & DpiGeoServer )
    {
      setQueryItem( query, QStringLiteral( "FORMAT_OPTIONS" ), QStringLiteral( "dpi:%1" ).arg( defaultLegendGraphicResolution ) );
      setQueryItem( query, QStringLiteral( "SCALE" ), QString::number( scale, 'f' ) );
    }
  }

  if ( useContextualWMSLegend )
  {
    bool changeXY = mCaps.shouldInvertAxisOrientation( mImageCrs );
    setQueryItem( query, QStringLiteral( "BBOX" ), toParamValue( visibleExtent, changeXY ) );
    setSRSQueryItem( query );
  }
  url.setQuery( query );

  QgsDebugMsgLevel( QStringLiteral( "getlegendgraphicrequest: %1" ).arg( url.toString() ), 2 );
  return QUrl( url );
}

QImage QgsWmsProvider::getLegendGraphic( double scale, bool forceRefresh, const QgsRectangle *visibleExtent )
{
  // TODO manage return basing of getCapablity => avoid call if service is not available
  // some services doesn't expose getLegendGraphic in capabilities but adding LegendURL in
  // the layer tags inside capabilities

  QString lurl = getLegendGraphicUrl();

  if ( lurl.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "getLegendGraphic url is empty" ), 2 );
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
  mLegendGraphicFetcher.reset( new QgsWmsLegendDownloadHandler( *QgsNetworkAccessManager::instance(), mSettings, url ) );
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

  if ( mapExtent == mGetLegendGraphicExtent &&
       scale == mGetLegendGraphicScale &&
       !mGetLegendGraphicImage.isNull() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Emitting cached image fetcher" ), 2 );
    // return a cached image, skipping the load
    return new QgsCachedImageFetcher( mGetLegendGraphicImage );
  }
  else
  {
    QgsImageFetcher *fetcher = new QgsWmsLegendDownloadHandler( *QgsNetworkAccessManager::instance(), mSettings, url );
    fetcher->setProperty( "legendScale", QVariant::fromValue( scale ) );
    fetcher->setProperty( "legendExtent", QVariant::fromValue( mapExtent.toRectF() ) );
    connect( fetcher, &QgsImageFetcher::finish, this, &QgsWmsProvider::getLegendGraphicReplyFinished );
    connect( fetcher, &QgsImageFetcher::error, this, [ = ]( const QString & )
    {
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
    QEventLoop *loop = qobject_cast< QEventLoop *>( reply->property( "eventLoop" ).value< QObject *>() );
    if ( loop )
      QMetaObject::invokeMethod( loop, "quit", Qt::QueuedConnection );
    mLegendGraphicFetcher.reset();
  }
}

void QgsWmsProvider::getLegendGraphicReplyErrored( const QString &message )
{
  Q_UNUSED( message )
  QgsDebugMsgLevel( QStringLiteral( "get legend failed: %1" ).arg( message ), 2 );

  QObject *reply = sender();

  if ( reply == mLegendGraphicFetcher.get() )
  {
    QEventLoop *loop = qobject_cast< QEventLoop *>( reply->property( "eventLoop" ).value< QObject *>() );
    if ( loop )
      QMetaObject::invokeMethod( loop, "quit", Qt::QueuedConnection );
    mLegendGraphicFetcher.reset();
  }
}

void QgsWmsProvider::getLegendGraphicReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of GetLegendGraphic downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QStringLiteral( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsgLevel( msg, 2 );
  emit statusChanged( msg );
}

bool QgsWmsProvider::isUrlForWMTS( const QString &url )
{
  // Do comparison in case insensitive way to match OGC KVP requirements
  return  url.contains( QLatin1String( "SERVICE=WMTS" ), Qt::CaseInsensitive ) ||
          url.contains( QLatin1String( "/WMTSCapabilities.xml" ), Qt::CaseInsensitive );
}


QgsWmsProvider *QgsWmsProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  Q_UNUSED( flags );
  return new QgsWmsProvider( uri, options );
}

// -----------------

QgsWmsImageDownloadHandler::QgsWmsImageDownloadHandler( const QString &providerUri, const QUrl &url, const QgsWmsAuthorization &auth, QImage *image, QgsRasterBlockFeedback *feedback )
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
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsWmsImageDownloadHandler" ) );
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
    if ( !redirect.isNull() )
    {
      mCacheReply->deleteLater();

      QgsDebugMsgLevel( QStringLiteral( "redirected getmap: %1" ).arg( redirect.toString() ), 2 );
      mCacheReply = QgsNetworkAccessManager::instance()->get( QNetworkRequest( redirect.toUrl() ) );
      connect( mCacheReply, &QNetworkReply::finished, this, &QgsWmsImageDownloadHandler::cacheReplyFinished );
      return;
    }

    QVariant status = mCacheReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
    {
      QVariant phrase = mCacheReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );

      QgsMessageLog::logMessage( tr( "Map request error (Status: %1; Reason phrase: %2; URL: %3)" )
                                 .arg( status.toInt() )
                                 .arg( phrase.toString(),
                                       mCacheReply->url().toString() ), tr( "WMS" ) );

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
    else if ( contentType.startsWith( QLatin1String( "image/" ), Qt::CaseInsensitive ) ||
              contentType.compare( QLatin1String( "application/octet-stream" ), Qt::CaseInsensitive ) == 0 )
    {
      QgsMessageLog::logMessage( tr( "Returned image is flawed [Content-Type: %1; URL: %2]" )
                                 .arg( contentType, mCacheReply->url().toString() ), tr( "WMS" ) );
    }
    else
    {
      QString errorTitle, errorText;
      if ( contentType.compare( QLatin1String( "text/xml" ), Qt::CaseInsensitive ) == 0 && QgsWmsProvider::parseServiceExceptionReportDom( text, errorTitle, errorText ) )
      {
        QgsMessageLog::logMessage( tr( "Map request error (Title: %1; Error: %2; URL: %3)" )
                                   .arg( errorTitle, errorText,
                                         mCacheReply->url().toString() ), tr( "WMS" ) );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Map request error (Status: %1; Response: %2; Content-Type: %3; URL: %4)" )
                                   .arg( status.toInt() )
                                   .arg( QString::fromUtf8( text ),
                                         contentType,
                                         mCacheReply->url().toString() ), tr( "WMS" ) );
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
  QgsDebugMsgLevel( QStringLiteral( "%1 of %2 bytes of map downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) ), 2 );
}

void QgsWmsImageDownloadHandler::canceled()
{
  QgsDebugMsgLevel( QStringLiteral( "Caught canceled() signal" ), 2 );
  if ( mCacheReply )
  {
    // abort the reply if it is still active
    QgsDebugMsgLevel( QStringLiteral( "Aborting WMS network request" ), 2 );
    mCacheReply->abort();
  }
}


// ----------


QgsWmsTiledImageDownloadHandler::QgsWmsTiledImageDownloadHandler( const QString &providerUri,
    const QgsWmsAuthorization &auth,
    int tileReqNo,
    const QgsWmsProvider::TileRequests &requests,
    QImage *image,
    const QgsRectangle &viewExtent,
    double sourceResolution,
    bool smoothPixmapTransform,
    bool resamplingEnabled,
    QgsRasterBlockFeedback *feedback )
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

  const auto constRequests = requests;
  for ( const QgsWmsProvider::TileRequest &r : constRequests )
  {
    QNetworkRequest request( r.url );
    QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsWmsTiledImageDownloadHandler" ) );
    auth.setAuthorization( request );
    request.setRawHeader( "Accept", "*/*" );
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ), mTileReqNo );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileIndex ), r.index );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRect ), r.rect );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), 0 );

    QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( request );
    connect( reply, &QNetworkReply::finished, this, &QgsWmsTiledImageDownloadHandler::tileReplyFinished );

    QString reqString = r.url.url();

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
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );

#if defined(QGISDEBUG)
  bool fromCache = reply->attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool();
  QgsWmsStatistics::Stat &stat = QgsWmsStatistics::statForUri( mProviderUri );
  if ( fromCache )
    stat.cacheHits++;
  else
    stat.cacheMisses++;
#endif
#if defined(QGISDEBUG)
  QgsDebugMsgLevel( QStringLiteral( "raw headers:" ), 3 );
  const auto constRawHeaderPairs = reply->rawHeaderPairs();
  for ( const QNetworkReply::RawHeaderPair &pair : constRawHeaderPairs )
  {
    QgsDebugMsgLevel( QStringLiteral( " %1:%2" )
                      .arg( QString::fromUtf8( pair.first ),
                            QString::fromUtf8( pair.second ) ), 3 );
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

    QgsDebugMsgLevel( QStringLiteral( "expirationDate:%1" ).arg( cmd.expirationDate().toString() ), 4 );
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
#ifdef QGISDEBUG
  int retry = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( TileRetry ) ).toInt();
#endif

  QgsDebugMsgLevel( QStringLiteral( "tile reply %1 (%2) tile:%3(retry %4) rect:%5,%6 %7,%8) fromcache:%9 %10 url:%11" )
                    .arg( tileReqNo ).arg( mTileReqNo ).arg( tileNo ).arg( retry )
                    .arg( r.left(), 0, 'f' ).arg( r.bottom(), 0, 'f' ).arg( r.right(), 0, 'f' ).arg( r.top(), 0, 'f' )
                    .arg( fromCache )
                    .arg( reply->error() == QNetworkReply::NoError ? QString() : QStringLiteral( "error: " ) + reply->errorString(),
                          reply->url().toString() ), 4
                  );

  if ( reply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      QNetworkRequest request( redirect.toUrl() );
      QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsWmsTiledImageDownloadHandler" ) );
      mAuth.setAuthorization( request );
      request.setRawHeader( "Accept", "*/*" );
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

      connect( reply, &QNetworkReply::finished, this, &QgsWmsTiledImageDownloadHandler::tileReplyFinished );

      return;
    }

    QVariant status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
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
    if ( !contentType.isEmpty() && !contentType.startsWith( QLatin1String( "image/" ), Qt::CaseInsensitive ) &&
         contentType.compare( QLatin1String( "application/octet-stream" ), Qt::CaseInsensitive ) != 0 )
    {
      QByteArray text = reply->readAll();
      QString errorTitle, errorText;
      if ( contentType.compare( QLatin1String( "text/xml" ), Qt::CaseInsensitive ) == 0 && QgsWmsProvider::parseServiceExceptionReportDom( text, errorTitle, errorText ) )
      {
        QgsMessageLog::logMessage( tr( "Tile request error (Title: %1; Error: %2; URL: %3)" )
                                   .arg( errorTitle, errorText,
                                         reply->url().toString() ), tr( "WMS" ) );
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
        if ( mResamplingEnabled && mSourceResolution < 0 )
        {
          mSourceResolution = r.width() / myLocalImage.width();
          mEffectiveViewExtent = initializeBufferedImage( mViewExtent, mSourceResolution, mImage );
        }

        const QRectF dst = destinationRect( mEffectiveViewExtent, r, mImage->width() );

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

        QgsTileCache::insertTile( reply->url(), myLocalImage );

        if ( mFeedback )
          mFeedback->onNewData();
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Returned image is flawed [Content-Type: %1; URL: %2]" )
                                   .arg( contentType, reply->url().toString() ), tr( "WMS" ) );
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
        QgsWmsStatistics::Stat &stat = QgsWmsStatistics::statForUri( mProviderUri );
        stat.errors++;
        // if we reached timeout, let's try again (e.g. in case of slow connection or slow server)
        repeatTileRequest( reply->request() );

        if ( reply->error() == QNetworkReply::ContentAccessDenied )
        {
          const QString contentType = reply->header( QNetworkRequest::ContentTypeHeader ).toString();

          QString errorMessage;
          if ( contentType.startsWith( QLatin1String( "text/plain" ) ) )
            errorMessage = reply->readAll();
          else
            errorMessage = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();

          mError = tr( "Access denied: %1" ).
                   arg( errorMessage );
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
  QgsDebugMsgLevel( QStringLiteral( "Caught canceled() signal" ), 3 );
  const auto constMReplies = mReplies;
  for ( QNetworkReply *reply : constMReplies )
  {
    QgsDebugMsgLevel( QStringLiteral( "Aborting tiled network request" ), 3 );
    reply->abort();
  }
}


void QgsWmsTiledImageDownloadHandler::repeatTileRequest( QNetworkRequest const &oldRequest )
{
  QgsWmsStatistics::Stat &stat = QgsWmsStatistics::statForUri( mProviderUri );

  if ( stat.errors == 100 )
  {
    QgsMessageLog::logMessage( tr( "Not logging more than 100 request errors." ), tr( "WMS" ) );
  }

  QNetworkRequest request( oldRequest );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsWmsTiledImageDownloadHandler" ) );

  QString url = request.url().toString();
  int tileReqNo = request.attribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ) ).toInt();
  int tileNo = request.attribute( static_cast<QNetworkRequest::Attribute>( TileIndex ) ).toInt();
  int retry = request.attribute( static_cast<QNetworkRequest::Attribute>( TileRetry ) ).toInt();
  retry++;

  QgsSettings s;
  int maxRetry = s.value( QStringLiteral( "qgis/defaultTileMaxRetry" ), "3" ).toInt();
  if ( retry > maxRetry )
  {
    if ( stat.errors < 100 )
    {
      QgsMessageLog::logMessage( tr( "Tile request max retry error. Failed %1 requests for tile %2 of tileRequest %3 (url: %4)" )
                                 .arg( maxRetry ).arg( tileNo ).arg( tileReqNo ).arg( url ), tr( "WMS" ) );
    }
    return;
  }

  mAuth.setAuthorization( request );
  if ( stat.errors < 100 )
  {
    QgsMessageLog::logMessage( tr( "repeat tileRequest %1 tile %2(retry %3)" )
                               .arg( tileReqNo ).arg( tileNo ).arg( retry ), tr( "WMS" ), Qgis::MessageLevel::Info );
  }
  QgsDebugMsgLevel( QStringLiteral( "repeat tileRequest %1 %2(retry %3) for url: %4" ).arg( tileReqNo ).arg( tileNo ).arg( retry ).arg( url ), 2 );
  request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), retry );

  QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( request );
  mReplies << reply;
  connect( reply, &QNetworkReply::finished, this, &QgsWmsTiledImageDownloadHandler::tileReplyFinished );
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
    return QStringLiteral( "0" );
  const int numberOfDecimals = std::max( 0, 19 - static_cast<int>( std::ceil( std::log10( std::fabs( x ) ) ) ) );
  return qgsDoubleToString( x, numberOfDecimals );
}

QString QgsWmsProvider::toParamValue( const QgsRectangle &rect, bool changeXY )
{
  return QString( changeXY ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
         .arg( formatDouble( rect.xMinimum() ),
               formatDouble( rect.yMinimum() ),
               formatDouble( rect.xMaximum() ),
               formatDouble( rect.yMaximum() ) );
}

void QgsWmsProvider::setSRSQueryItem( QUrlQuery &url )
{
  QString crsKey = QStringLiteral( "SRS" ); //SRS in 1.1.1 and CRS in 1.3.0
  if ( mCaps.mCapabilities.version == QLatin1String( "1.3.0" ) || mCaps.mCapabilities.version == QLatin1String( "1.3" ) )
  {
    crsKey = QStringLiteral( "CRS" );
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
    QgsDebugMsgLevel( QStringLiteral( "WMSLegendDownloader destroyed while still processing reply" ), 2 );
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
  Q_ASSERT( !mReply );  // don't call me twice from outside !
  Q_ASSERT( url.isValid() );

  if ( mVisitedUrls.contains( url ) )
  {
    QString err( tr( "Redirect loop detected: %1" ).arg( url.toString() ) );
    QgsMessageLog::logMessage( err, tr( "WMS" ) );
    sendError( err );
    return;
  }
  mVisitedUrls.insert( url );

  QgsDebugMsgLevel( QStringLiteral( "legend url: %1" ).arg( url.toString() ), 2 );

  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsWmsLegendDownloadHandler" ) );
  mSettings.authorization().setAuthorization( request );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  mReply = mNetworkAccessManager.get( request );
  mSettings.authorization().setAuthorizationReply( mReply );

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  connect( mReply, static_cast < void ( QNetworkReply::* )( QNetworkReply::NetworkError ) >( &QNetworkReply::error ), this, &QgsWmsLegendDownloadHandler::errored );
#else
  connect( mReply, &QNetworkReply::errorOccurred, this, &QgsWmsLegendDownloadHandler::errored );
#endif

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
  QgsDebugMsgLevel( QStringLiteral( "emitting finish: %1x%2 image" ).arg( img.width() ).arg( img.height() ), 2 );
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

  QgsDebugMsgLevel( QStringLiteral( "reply OK" ), 2 );
  QVariant redirect = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
  if ( !redirect.isNull() )
  {
    mReply->deleteLater();
    mReply = nullptr;
    startUrl( redirect.toUrl() );
    return;
  }

  QVariant status = mReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
  if ( !status.isNull() && status.toInt() >= 400 )
  {
    QVariant phrase = mReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
    QString msg( tr( "GetLegendGraphic request error" ) );
    msg += QLatin1String( " - " );
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
  QTimer::singleShot( 1, this, SLOT( send() ) );
}


// -----------------------


QgsWmsProviderMetadata::QgsWmsProviderMetadata()
  : QgsProviderMetadata( QgsWmsProvider::WMS_KEY, QgsWmsProvider::WMS_DESCRIPTION )
{
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
  const auto constItems { query.queryItems() };
  QVariantMap decoded;
  for ( const auto &item : constItems )
  {
    if ( item.first == QLatin1String( "url" ) )
    {
      const QUrl url( item.second );
      if ( url.isLocalFile() )
      {
        decoded[ QStringLiteral( "path" ) ] = url.toLocalFile();
      }
      else
      {
        decoded[ item.first ] = item.second;
      }
    }
    else
    {
      decoded[ item.first ] = item.second;
    }
  }
  return decoded;
}

QString QgsWmsProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QUrlQuery query;
  QList<QPair<QString, QString> > items;
  for ( auto it = parts.constBegin(); it != parts.constEnd(); ++it )
  {
    if ( it.key() == QLatin1String( "path" ) )
    {
      items.push_back( { QStringLiteral( "url" ), QUrl::fromLocalFile( it.value().toString() ).toString() } );
    }
    else
    {
      items.push_back( { it.key(), it.value().toString() } );
    }
  }
  query.setQueryItems( items );
  return query.toString();
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
  stat.statsGathered = QgsRasterBandStats::Min | QgsRasterBandStats::Max;
  return stat;
}

QgsRasterHistogram QgsWmsInterpretationConverterMapTilerTerrainRGB::histogram( int, int, double, double, const QgsRectangle &, int, bool, QgsRasterBlockFeedback * ) const
{
  return QgsRasterHistogram();
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
  stat.statsGathered = QgsRasterBandStats::Min | QgsRasterBandStats::Max;
  return stat;
}

QgsRasterHistogram QgsWmsInterpretationConverterTerrariumRGB::histogram( int, int, double, double, const QgsRectangle &, int, bool, QgsRasterBlockFeedback * ) const
{
  return QgsRasterHistogram();
}
