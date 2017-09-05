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
#include "qgsdatasourceuri.h"
#include "qgsfeaturestore.h"
#include "qgsgeometry.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrasterlayer.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmapsettings.h"
#include "qgsmessageoutput.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsnetworkreplyparser.h"
#include "qgstilecache.h"
#include "qgsgml.h"
#include "qgsgmlschema.h"
#include "qgswmscapabilities.h"
#include "qgsexception.h"
#include "qgssettings.h"


#ifdef HAVE_GUI
#include "qgswmssourceselect.h"
#include "qgssourceselectprovider.h"
#endif


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
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QNetworkDiskCache>
#include <QTimer>

#include <ogr_api.h>

#ifdef QGISDEBUG
#include <QFile>
#include <QDir>
#endif

#define ERR(message) QGS_ERROR_MESSAGE(message,"WMS provider")
#define QGS_ERROR(message) QgsError(message,"WMS provider")

static QString WMS_KEY = QStringLiteral( "wms" );
static QString WMS_DESCRIPTION = QStringLiteral( "OGC Web Map Service version 1.3 data provider" );

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


QgsWmsProvider::QgsWmsProvider( QString const &uri, const QgsWmsCapabilities *capabilities )
  : QgsRasterDataProvider( uri )
  , mHttpGetLegendGraphicResponse( nullptr )
  , mGetLegendGraphicImage()
  , mGetLegendGraphicScale( 0.0 )
  , mImageCrs( DEFAULT_LATLON_CRS )
  , mIdentifyReply( nullptr )
  , mExtentDirty( true )
  , mTileReqNo( 0 )
  , mTileLayer( nullptr )
  , mTileMatrixSet( nullptr )
{
  QgsDebugMsg( "constructing with uri '" + uri + "'." );

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

  if ( mSettings.mXyz )
  {
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

    // Make sure we have capabilities - other functions here may need them
    if ( !retrieveServerCapabilities() )
    {
      return;
    }
  }

  // setImageCrs is using mTiled !!!
  if ( !setImageCrs( mSettings.mCrsId ) )
  {
    appendError( ERR( tr( "Cannot set CRS" ) ) );
    return;
  }
  mCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mSettings.mCrsId );

  if ( !calculateExtent() || mLayerExtent.isEmpty() )
  {
    appendError( ERR( tr( "Cannot calculate extent" ) ) );
    return;
  }

  // URL can be in 3 forms:
  // 1) http://xxx.xxx.xx/yyy/yyy
  // 2) http://xxx.xxx.xx/yyy/yyy?
  // 3) http://xxx.xxx.xx/yyy/yyy?zzz=www

  mValid = true;
  QgsDebugMsg( "exiting constructor." );
}


QString QgsWmsProvider::prepareUri( QString uri )
{
  if ( uri.contains( QLatin1String( "SERVICE=WMTS" ) ) || uri.contains( QLatin1String( "/WMTSCapabilities.xml" ) ) )
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
  QgsDebugMsg( "deconstructing." );
}

QgsWmsProvider *QgsWmsProvider::clone() const
{
  QgsWmsProvider *provider = new QgsWmsProvider( dataSourceUri(), mCaps.isValid() ? &mCaps : nullptr );
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
  Q_FOREACH ( const QgsWmsStyleProperty &s, styles )
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

  return url.isEmpty() ? url : prepareUri( url );
}

bool QgsWmsProvider::addLayers()
{
  QgsDebugMsg( "Entering: layers:" + mSettings.mActiveSubLayers.join( ", " ) + ", styles:" + mSettings.mActiveSubStyles.join( ", " ) );

  if ( mSettings.mActiveSubLayers.size() != mSettings.mActiveSubStyles.size() )
  {
    QgsMessageLog::logMessage( tr( "Number of layers and styles don't match" ), tr( "WMS" ) );
    return false;
  }

  // Set the visibility of these new layers on by default
  Q_FOREACH ( const QString &layer, mSettings.mActiveSubLayers )
  {
    mActiveSubLayerVisibility[ layer ] = true;
    QgsDebugMsg( "set visibility of layer '" + layer + "' to true." );
  }

  // now that the layers have changed, the extent will as well.
  mExtentDirty = true;

  if ( mSettings.mTiled )
    mTileLayer = nullptr;

  QgsDebugMsg( "Exiting." );

  return true;
}

void QgsWmsProvider::setConnectionName( QString const &connName )
{
  mConnectionName = connName;
}

void QgsWmsProvider::setLayerOrder( QStringList const &layers )
{
  QgsDebugMsg( "Entering." );

  if ( layers.size() != mSettings.mActiveSubLayers.size() )
  {
    QgsDebugMsg( "Invalid layer list length" );
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
      QgsDebugMsg( QString( "Layer %1 not found" ).arg( layers[i] ) );
      return;
    }
  }

  mSettings.mActiveSubLayers = layers;
  mSettings.mActiveSubStyles.clear();
  for ( int i = 0; i < layers.size(); i++ )
  {
    mSettings.mActiveSubStyles.append( styleMap[ layers[i] ] );
  }

  QgsDebugMsg( "Exiting." );
}


void QgsWmsProvider::setSubLayerVisibility( QString const &name, bool vis )
{
  if ( !mActiveSubLayerVisibility.contains( name ) )
  {
    QgsDebugMsg( QString( "Layer %1 not found." ).arg( name ) );
    return;
  }

  mActiveSubLayerVisibility[name] = vis;
}


bool QgsWmsProvider::setImageCrs( QString const &crs )
{
  QgsDebugMsg( "Setting image CRS to " + crs + '.' );

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

    QgsDebugMsg( QString( "mTileLayersSupported.size() = %1" ).arg( mCaps.mTileLayersSupported.size() ) );
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
          QgsDebugMsg( QString( "tile matrix set '%1' not found." ).arg( tms ) );
          continue;
        }

        if ( mCaps.mTileMatrixSets[ tms ].crs != mImageCrs )
        {
          QgsDebugMsg( QString( "tile matrix set '%1' has crs %2 instead of %3." ).arg( tms, mCaps.mTileMatrixSets[ tms ].crs, mImageCrs ) );
          continue;
        }

        // fill in generate matrix for WMS-C
        mSettings.mTileMatrixSetId = tms;
      }

      mTileLayer = tl;
      break;
    }

    QList<QVariant> resolutions;
    if ( mCaps.mTileMatrixSets.contains( mSettings.mTileMatrixSetId ) )
    {
      mTileMatrixSet = &mCaps.mTileMatrixSets[ mSettings.mTileMatrixSetId ];
      QList<double> keys = mTileMatrixSet->tileMatrices.keys();
      std::sort( keys.begin(), keys.end() );
      Q_FOREACH ( double key, keys )
      {
        resolutions << key;
      }
      if ( !mTileMatrixSet->tileMatrices.empty() )
      {
        setProperty( "tileWidth", mTileMatrixSet->tileMatrices.values().first().tileWidth );
        setProperty( "tileHeight", mTileMatrixSet->tileMatrices.values().first().tileHeight );
      }
    }
    else
    {
      QgsDebugMsg( QString( "Expected tile matrix set '%1' not found." ).arg( mSettings.mTileMatrixSetId ) );
      mTileMatrixSet = nullptr;
    }

    setProperty( "resolutions", resolutions );

    if ( !mTileLayer || !mTileMatrixSet )
    {
      appendError( ERR( tr( "Tile layer or tile matrix set not found" ) ) );
      return false;
    }
  }
  return true;
}

void QgsWmsProvider::setQueryItem( QUrl &url, const QString &item, const QString &value )
{
  url.removeQueryItem( item );
  url.addQueryItem( item, value );
}

void QgsWmsProvider::setFormatQueryItem( QUrl &url )
{
  url.removeQueryItem( QStringLiteral( "FORMAT" ) );
  if ( mSettings.mImageMimeType.contains( '+' ) )
  {
    QString format( mSettings.mImageMimeType );
    format.replace( '+', QLatin1String( "%2b" ) );
    url.addEncodedQueryItem( "FORMAT", format.toUtf8() );
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

void QgsWmsProvider::fetchOtherResTiles( QgsTileMode tileMode, const QgsRectangle &viewExtent, int imageWidth, QList<QRectF> &missingRects, double tres, int resOffset, QList<TileImage> &otherResTiles )
{
  if ( !mTileMatrixSet )
    return;  // there is no tile matrix set defined for ordinary WMS (with user-specified tile size)

  const QgsWmtsTileMatrix *tmOther = mTileMatrixSet->findOtherResolution( tres, resOffset );
  if ( !tmOther )
    return;

  QSet<TilePosition> tilesSet;
  Q_FOREACH ( const QRectF &missingTileRect, missingRects )
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
  TilePositions tiles = tilesSet.toList();
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
      createTileRequestsXYZ( tmOther, tiles, requests );
      break;
  }

  QList<QRectF> missingRectsToDelete;
  Q_FOREACH ( const TileRequest &r, requests )
  {
    QImage localImage;
    if ( ! QgsTileCache::tile( r.url, localImage ) )
      continue;

    double cr = viewExtent.width() / imageWidth;
    QRectF dst( ( r.rect.left() - viewExtent.xMinimum() ) / cr,
                ( viewExtent.yMaximum() - r.rect.bottom() ) / cr,
                r.rect.width() / cr,
                r.rect.height() / cr );
    otherResTiles << TileImage( dst, localImage );

    // see if there are any missing rects that are completely covered by this tile
    Q_FOREACH ( const QRectF &missingRect, missingRects )
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
  Q_FOREACH ( const QRectF &rectToDelete, missingRectsToDelete )
  {
    missingRects.removeOne( rectToDelete );
  }

  QgsDebugMsg( QString( "Other resolution tiles: offset %1, res %2, missing rects %3, remaining rects %4, added tiles %5" )
               .arg( resOffset )
               .arg( tmOther->tres )
               .arg( missingRects.count() + missingRectsToDelete.count() )
               .arg( missingRects.count() )
               .arg( otherResTiles.count() ) );
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
  Q_UNUSED( p );
  Q_UNUSED( rect );
  Q_UNUSED( color );
#endif
}

QImage *QgsWmsProvider::draw( QgsRectangle const &viewExtent, int pixelWidth, int pixelHeight, QgsRasterBlockFeedback *feedback )
{
  QgsDebugMsg( "Entering." );

  // compose the URL query string for the WMS server.

  QImage *image = new QImage( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
  image->fill( 0 );

  if ( !mSettings.mTiled && mSettings.mMaxWidth == 0 && mSettings.mMaxHeight == 0 )
  {
    QUrl url = createRequestUrlWMS( viewExtent, pixelWidth, pixelHeight );

    // cache some details for if the user wants to do an identifyAsHtml() later

    emit statusChanged( tr( "Getting map via WMS." ) );

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
      Q_ASSERT( !mTileMatrixSet->tileMatrices.isEmpty() );

      // find nearest resolution
      tm = mTileMatrixSet->findNearestResolution( vres );
      Q_ASSERT( tm );

      tileMode = mTileLayer->tileMode;
    }
    else if ( mSettings.mMaxWidth != 0 && mSettings.mMaxHeight != 0 )
    {
      // this is an ordinary WMS server, but the user requested tiled approach
      // so we will pretend it is a WMS-C server with just one tile matrix
      tempTm.reset( new QgsWmtsTileMatrix );
      tempTm->topLeft      = QgsPointXY( mLayerExtent.xMinimum(), mLayerExtent.yMaximum() );
      tempTm->tileWidth    = mSettings.mMaxWidth;
      tempTm->tileHeight   = mSettings.mMaxHeight;
      tempTm->matrixWidth  = std::ceil( mLayerExtent.width() / mSettings.mMaxWidth / vres );
      tempTm->matrixHeight = std::ceil( mLayerExtent.height() / mSettings.mMaxHeight / vres );
      tempTm->tres = vres;
      tm = tempTm.get();

      tileMode = WMSC;
    }
    else
    {
      QgsDebugMsg( "empty tile size" );
      return image;
    }

    QgsDebugMsg( QString( "layer extent: %1,%2 %3x%4" )
                 .arg( qgsDoubleToString( mLayerExtent.xMinimum() ),
                       qgsDoubleToString( mLayerExtent.yMinimum() ) )
                 .arg( mLayerExtent.width() )
                 .arg( mLayerExtent.height() )
               );

    QgsDebugMsg( QString( "view extent: %1,%2 %3x%4  res:%5" )
                 .arg( qgsDoubleToString( viewExtent.xMinimum() ),
                       qgsDoubleToString( viewExtent.yMinimum() ) )
                 .arg( viewExtent.width() )
                 .arg( viewExtent.height() )
                 .arg( vres, 0, 'f' )
               );

    QgsDebugMsg( QString( "tile matrix %1,%2 res:%3 tilesize:%4x%5 matrixsize:%6x%7 id:%8" )
                 .arg( tm->topLeft.x() ).arg( tm->topLeft.y() ).arg( tm->tres )
                 .arg( tm->tileWidth ).arg( tm->tileHeight )
                 .arg( tm->matrixWidth ).arg( tm->matrixHeight )
                 .arg( tm->identifier )
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
    tm->viewExtentIntersection( viewExtent, tml, col0, row0, col1, row1 );

#if QGISDEBUG
    int n = ( col1 - col0 + 1 ) * ( row1 - row0 + 1 );
    QgsDebugMsg( QString( "tile number: %1x%2 = %3" ).arg( col1 - col0 + 1 ).arg( row1 - row0 + 1 ).arg( n ) );
    if ( n > 256 )
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
        createTileRequestsXYZ( tm, tiles, requests );
        break;

      default:
        QgsDebugMsg( QString( "unexpected tile mode %1" ).arg( mTileLayer->tileMode ) );
        return image;
    }

    emit statusChanged( tr( "Getting tiles." ) );

    QList<TileImage> tileImages;  // in the correct resolution
    QList<QRectF> missing;  // rectangles (in map coords) of missing tiles for this view

    QTime t;
    t.start();
    TileRequests requestsFinal;
    Q_FOREACH ( const TileRequest &r, requests )
    {
      QImage localImage;
      if ( QgsTileCache::tile( r.url, localImage ) )
      {
        double cr = viewExtent.width() / image->width();

        QRectF dst( ( r.rect.left() - viewExtent.xMinimum() ) / cr,
                    ( viewExtent.yMaximum() - r.rect.bottom() ) / cr,
                    r.rect.width() / cr,
                    r.rect.height() / cr );
        tileImages << TileImage( dst, localImage );
      }
      else
      {
        missing << r.rect;

        // need to make a request
        requestsFinal << r;
      }
    }
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
      // first we check lower resolution tiles: one level back, then two levels back (if there is still some are not covered),
      // finally (in the worst case we use one level higher resolution tiles). This heuristic should give
      // good overviews while not spending too much time drawing cached tiles from resolutions far away.
      fetchOtherResTiles( tileMode, viewExtent, image->width(), missing, tm->tres, 1, lowerResTiles );
      fetchOtherResTiles( tileMode, viewExtent, image->width(), missing, tm->tres, 2, lowerResTiles2 );
      fetchOtherResTiles( tileMode, viewExtent, image->width(), missing, tm->tres, -1, higherResTiles );

      // draw the cached tiles lowest to highest resolution
      Q_FOREACH ( const TileImage &ti, lowerResTiles2 )
      {
        p.drawImage( ti.rect, ti.img );
        _drawDebugRect( p, ti.rect, Qt::blue );
      }
      Q_FOREACH ( const TileImage &ti, lowerResTiles )
      {
        p.drawImage( ti.rect, ti.img );
        _drawDebugRect( p, ti.rect, Qt::yellow );
      }
      Q_FOREACH ( const TileImage &ti, higherResTiles )
      {
        p.drawImage( ti.rect, ti.img );
        _drawDebugRect( p, ti.rect, Qt::red );
      }
    }

    int t1 = t.elapsed() - t0;

    // draw composite in this resolution
    Q_FOREACH ( const TileImage &ti, tileImages )
    {
      if ( mSettings.mSmoothPixmapTransform )
        p.setRenderHint( QPainter::SmoothPixmapTransform, true );
      p.drawImage( ti.rect, ti.img );

      if ( feedback && feedback->isPreviewOnly() )
        _drawDebugRect( p, ti.rect, Qt::green );
    }
    p.end();

    int t2 = t.elapsed() - t1;
    Q_UNUSED( t2 ); // only used in debug build

    if ( feedback && feedback->isPreviewOnly() )
    {
      QgsDebugMsg( QString( "PREVIEW - CACHED: %1 / MISSING: %2" ).arg( tileImages.count() ).arg( requests.count() - tileImages.count() ) );
      QgsDebugMsg( QString( "PREVIEW - TIME: this res %1 ms | other res %2 ms | TOTAL %3 ms" ).arg( t0 + t2 ).arg( t1 ).arg( t0 + t1 + t2 ) );
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

      QgsWmsTiledImageDownloadHandler handler( dataSourceUri(), mSettings.authorization(), mTileReqNo, requestsFinal, image, viewExtent, mSettings.mSmoothPixmapTransform, feedback );
      handler.downloadBlocking();
    }

    QgsDebugMsg( QString( "TILE CACHE total: %1 / %2" ).arg( QgsTileCache::totalCost() ).arg( QgsTileCache::maxCost() ) );

#if 0
    const QgsWmsStatistics::Stat &stat = QgsWmsStatistics::statForUri( dataSourceUri() );
    emit statusChanged( tr( "%n tile requests in background", "tile request count", requests.count() )
                        + tr( ", %n cache hits", "tile cache hits", stat.cacheHits )
                        + tr( ", %n cache misses.", "tile cache missed", stat.cacheMisses )
                        + tr( ", %n errors.", "errors", stat.errors )
                      );
#endif
  }

  return image;
}

void QgsWmsProvider::readBlock( int bandNo, QgsRectangle  const &viewExtent, int pixelWidth, int pixelHeight, void *block, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo );
  // TODO: optimize to avoid writing to QImage
  QImage *image = draw( viewExtent, pixelWidth, pixelHeight, feedback );
  if ( !image )   // should not happen
  {
    QgsMessageLog::logMessage( tr( "image is NULL" ), tr( "WMS" ) );
    return;
  }

  QgsDebugMsg( QString( "image height = %1 bytesPerLine = %2" ).arg( image->height() ) . arg( image->bytesPerLine() ) );
  size_t myExpectedSize = pixelWidth * pixelHeight * 4;
  size_t myImageSize = image->height() *  image->bytesPerLine();
  if ( myExpectedSize != myImageSize )   // should not happen
  {
    QgsMessageLog::logMessage( tr( "unexpected image size" ), tr( "WMS" ) );
    delete image;
    return;
  }

  uchar *ptr = image->bits();
  if ( ptr )
  {
    // If image is too large, ptr can be NULL
    memcpy( block, ptr, myExpectedSize );
  }

  delete image;
}

QUrl QgsWmsProvider::createRequestUrlWMS( const QgsRectangle &viewExtent, int pixelWidth, int pixelHeight )
{
  // Calculate active layers that are also visible.

  bool changeXY = mCaps.shouldInvertAxisOrientation( mImageCrs );

  QgsDebugMsg( "Active layer list of "  + mSettings.mActiveSubLayers.join( ", " )
               + " and style list of "  + mSettings.mActiveSubStyles.join( ", " ) );

  QStringList visibleLayers = QStringList();
  QStringList visibleStyles = QStringList();

  QStringList::const_iterator it2  = mSettings.mActiveSubStyles.constBegin();

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

  QString layers = visibleLayers.join( QStringLiteral( "," ) );
  layers = layers.isNull() ? QLatin1String( "" ) : layers;
  QString styles = visibleStyles.join( QStringLiteral( "," ) );
  styles = styles.isNull() ? QLatin1String( "" ) : styles;

  QgsDebugMsg( "Visible layer list of " + layers + " and style list of " + styles );

  QString bbox = toParamValue( viewExtent, changeXY );

  QUrl url( mSettings.mIgnoreGetMapUrl ? mSettings.mBaseUrl : getMapUrl() );
  setQueryItem( url, QStringLiteral( "SERVICE" ), QStringLiteral( "WMS" ) );
  setQueryItem( url, QStringLiteral( "VERSION" ), mCaps.mCapabilities.version );
  setQueryItem( url, QStringLiteral( "REQUEST" ), QStringLiteral( "GetMap" ) );
  setQueryItem( url, QStringLiteral( "BBOX" ), bbox );
  setSRSQueryItem( url );
  setQueryItem( url, QStringLiteral( "WIDTH" ), QString::number( pixelWidth ) );
  setQueryItem( url, QStringLiteral( "HEIGHT" ), QString::number( pixelHeight ) );
  setQueryItem( url, QStringLiteral( "LAYERS" ), layers );
  setQueryItem( url, QStringLiteral( "STYLES" ), styles );
  setFormatQueryItem( url );

  if ( mDpi != -1 )
  {
    if ( mSettings.mDpiMode & DpiQGIS )
      setQueryItem( url, QStringLiteral( "DPI" ), QString::number( mDpi ) );
    if ( mSettings.mDpiMode & DpiUMN )
      setQueryItem( url, QStringLiteral( "MAP_RESOLUTION" ), QString::number( mDpi ) );
    if ( mSettings.mDpiMode & DpiGeoServer )
      setQueryItem( url, QStringLiteral( "FORMAT_OPTIONS" ), QStringLiteral( "dpi:%1" ).arg( mDpi ) );
  }

  //MH: jpeg does not support transparency and some servers complain if jpg and transparent=true
  if ( mSettings.mImageMimeType == QLatin1String( "image/x-jpegorpng" ) ||
       ( !mSettings.mImageMimeType.contains( QLatin1String( "jpeg" ), Qt::CaseInsensitive ) &&
         !mSettings.mImageMimeType.contains( QLatin1String( "jpg" ), Qt::CaseInsensitive ) ) )
  {
    setQueryItem( url, QStringLiteral( "TRANSPARENT" ), QStringLiteral( "TRUE" ) );  // some servers giving error for 'true' (lowercase)
  }

  QgsDebugMsg( QString( "getmap: %1" ).arg( url.toString() ) );
  return url;
}


void QgsWmsProvider::createTileRequestsWMSC( const QgsWmtsTileMatrix *tm, const QgsWmsProvider::TilePositions &tiles, QgsWmsProvider::TileRequests &requests )
{
  bool changeXY = mCaps.shouldInvertAxisOrientation( mImageCrs );

  // add WMS request
  QUrl url( mSettings.mIgnoreGetMapUrl ? mSettings.mBaseUrl : getMapUrl() );
  setQueryItem( url, QStringLiteral( "SERVICE" ), QStringLiteral( "WMS" ) );
  setQueryItem( url, QStringLiteral( "VERSION" ), mCaps.mCapabilities.version );
  setQueryItem( url, QStringLiteral( "REQUEST" ), QStringLiteral( "GetMap" ) );
  setQueryItem( url, QStringLiteral( "LAYERS" ), mSettings.mActiveSubLayers.join( QStringLiteral( "," ) ) );
  setQueryItem( url, QStringLiteral( "STYLES" ), mSettings.mActiveSubStyles.join( QStringLiteral( "," ) ) );
  setQueryItem( url, QStringLiteral( "WIDTH" ), QString::number( tm->tileWidth ) );
  setQueryItem( url, QStringLiteral( "HEIGHT" ), QString::number( tm->tileHeight ) );
  setFormatQueryItem( url );

  setSRSQueryItem( url );

  if ( mSettings.mTiled )
  {
    setQueryItem( url, QStringLiteral( "TILED" ), QStringLiteral( "true" ) );
  }

  if ( mDpi != -1 )
  {
    if ( mSettings.mDpiMode & DpiQGIS )
      setQueryItem( url, QStringLiteral( "DPI" ), QString::number( mDpi ) );
    if ( mSettings.mDpiMode & DpiUMN )
      setQueryItem( url, QStringLiteral( "MAP_RESOLUTION" ), QString::number( mDpi ) );
    if ( mSettings.mDpiMode & DpiGeoServer )
      setQueryItem( url, QStringLiteral( "FORMAT_OPTIONS" ), QStringLiteral( "dpi:%1" ).arg( mDpi ) );
  }

  if ( mSettings.mImageMimeType == QLatin1String( "image/x-jpegorpng" ) ||
       ( !mSettings.mImageMimeType.contains( QLatin1String( "jpeg" ), Qt::CaseInsensitive ) &&
         !mSettings.mImageMimeType.contains( QLatin1String( "jpg" ), Qt::CaseInsensitive ) ) )
  {
    setQueryItem( url, QStringLiteral( "TRANSPARENT" ), QStringLiteral( "TRUE" ) );  // some servers giving error for 'true' (lowercase)
  }

  int i = 0;
  Q_FOREACH ( const TilePosition &tile, tiles )
  {
    QgsRectangle bbox( tm->tileBBox( tile.col, tile.row ) );
    QString turl;
    turl += url.toString();
    turl += QString( changeXY ? "&BBOX=%2,%1,%4,%3" : "&BBOX=%1,%2,%3,%4" )
            .arg( qgsDoubleToString( bbox.xMinimum() ),
                  qgsDoubleToString( bbox.yMinimum() ),
                  qgsDoubleToString( bbox.xMaximum() ),
                  qgsDoubleToString( bbox.yMaximum() ) );

    QgsDebugMsg( QString( "tileRequest %1 %2/%3 (%4,%5): %6" ).arg( mTileReqNo ).arg( i ).arg( tiles.count() ).arg( tile.row ).arg( tile.col ).arg( turl ) );
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

    // compose static request arguments.
    setQueryItem( url, QStringLiteral( "SERVICE" ), QStringLiteral( "WMTS" ) );
    setQueryItem( url, QStringLiteral( "REQUEST" ), QStringLiteral( "GetTile" ) );
    setQueryItem( url, QStringLiteral( "VERSION" ), mCaps.mCapabilities.version );
    setQueryItem( url, QStringLiteral( "LAYER" ), mSettings.mActiveSubLayers[0] );
    setQueryItem( url, QStringLiteral( "STYLE" ), mSettings.mActiveSubStyles[0] );
    setQueryItem( url, QStringLiteral( "FORMAT" ), mSettings.mImageMimeType );
    setQueryItem( url, QStringLiteral( "TILEMATRIXSET" ), mTileMatrixSet->identifier );
    setQueryItem( url, QStringLiteral( "TILEMATRIX" ), tm->identifier );

    for ( QHash<QString, QString>::const_iterator it = mSettings.mTileDimensionValues.constBegin(); it != mSettings.mTileDimensionValues.constEnd(); ++it )
    {
      setQueryItem( url, it.key(), it.value() );
    }

    url.removeQueryItem( QStringLiteral( "TILEROW" ) );
    url.removeQueryItem( QStringLiteral( "TILECOL" ) );

    int i = 0;
    Q_FOREACH ( const TilePosition &tile, tiles )
    {
      QString turl;
      turl += url.toString();
      turl += QStringLiteral( "&TILEROW=%1&TILECOL=%2" ).arg( tile.row ).arg( tile.col );

      QgsDebugMsg( QString( "tileRequest %1 %2/%3 (%4,%5): %6" ).arg( mTileReqNo ).arg( i ).arg( tiles.count() ).arg( tile.row ).arg( tile.col ).arg( turl ) );
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
    Q_FOREACH ( const TilePosition &tile, tiles )
    {
      QString turl( url );
      turl.replace( QLatin1String( "{tilerow}" ), QString::number( tile.row ), Qt::CaseInsensitive );
      turl.replace( QLatin1String( "{tilecol}" ), QString::number( tile.col ), Qt::CaseInsensitive );

      QgsDebugMsgLevel( QString( "tileRequest %1 %2/%3 (%4,%5): %6" ).arg( mTileReqNo ).arg( i ).arg( tiles.count() ).arg( tile.row ).arg( tile.col ).arg( turl ), 2 );
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


void QgsWmsProvider::createTileRequestsXYZ( const QgsWmtsTileMatrix *tm, const QgsWmsProvider::TilePositions &tiles, QgsWmsProvider::TileRequests &requests )
{
  int z = tm->identifier.toInt();
  QString url = mSettings.mBaseUrl;
  int i = 0;
  Q_FOREACH ( const TilePosition &tile, tiles )
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

    QgsDebugMsgLevel( QString( "tileRequest %1 %2/%3 (%4,%5): %6" ).arg( mTileReqNo ).arg( i ).arg( tiles.count() ).arg( tile.row ).arg( tile.col ).arg( turl ), 2 );
    requests << TileRequest( turl, tm->tileRect( tile.col, tile.row ), i );
  }
}


bool QgsWmsProvider::retrieveServerCapabilities( bool forceRefresh )
{
  QgsDebugMsg( QString( "entering: forceRefresh=%1" ).arg( forceRefresh ) );

  if ( !mCaps.isValid() )
  {
    QgsWmsCapabilitiesDownload downloadCaps( mSettings.baseUrl(), mSettings.authorization(), forceRefresh );
    if ( !downloadCaps.downloadCapabilities() )
    {
      mErrorFormat = QStringLiteral( "text/plain" );
      mError = downloadCaps.lastError();
      return false;
    }

    QgsWmsCapabilities caps;
    if ( !caps.parseResponse( downloadCaps.response(), mSettings.parserSettings() ) )
    {
      mErrorFormat = caps.lastErrorFormat();
      mError = caps.lastError();
      return false;
    }

    mCaps = caps;
  }

  Q_ASSERT( mCaps.isValid() );

  QgsDebugMsg( "exiting." );

  return true;
}


void QgsWmsProvider::setupXyzCapabilities( const QString &uri )
{
  QgsDataSourceUri parsedUri;
  parsedUri.setEncodedUri( uri );

  QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), QgsCoordinateReferenceSystem( mSettings.mCrsId ) );
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
  bbox.box = QgsRectangle( topLeft.x(), bottomRight.y(), bottomRight.x(), topLeft.y() );

  QgsWmtsTileLayer tl;
  tl.tileMode = XYZ;
  tl.identifier = QStringLiteral( "xyz" );  // as set in parseUri
  tl.boundingBoxes << bbox;
  mCaps.mTileLayersSupported.append( tl );

  QgsWmtsTileMatrixSet tms;
  tms.identifier = QStringLiteral( "tms0" );  // as set in parseUri
  tms.crs = mSettings.mCrsId;
  mCaps.mTileMatrixSets[tms.identifier] = tms;

  int minZoom = 0;
  int maxZoom = 18;
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
    tm.tileWidth = tm.tileHeight = 256;
    tm.matrixWidth = tm.matrixHeight = 1 << zoom;
    tm.tres = xspan / ( tm.tileWidth * tm.matrixWidth );
    tm.scaleDenom = 0.0;

    mCaps.mTileMatrixSets[tms.identifier].tileMatrices[tm.tres] = tm;
  }
}


Qgis::DataType QgsWmsProvider::dataType( int bandNo ) const
{
  return sourceDataType( bandNo );
}

Qgis::DataType QgsWmsProvider::sourceDataType( int bandNo ) const
{
  Q_UNUSED( bandNo );
  return Qgis::ARGB32;
}

int QgsWmsProvider::bandCount() const
{
  return 1;
}


static const QgsWmsLayerProperty *_findNestedLayerProperty( const QString &layerName, const QgsWmsLayerProperty *prop )
{
  if ( prop->name == layerName )
    return prop;

  Q_FOREACH ( const QgsWmsLayerProperty &child, prop->layer )
  {
    if ( const QgsWmsLayerProperty *res = _findNestedLayerProperty( layerName, &child ) )
      return res;
  }

  return nullptr;
}


bool QgsWmsProvider::extentForNonTiledLayer( const QString &layerName, const QString &crs, QgsRectangle &extent ) const
{
  const QgsWmsLayerProperty *layerProperty = nullptr;
  Q_FOREACH ( const QgsWmsLayerProperty &toplevelLayer, mCaps.mCapabilities.capability.layers )
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

  QgsCoordinateTransform xform( wgs, dst );
  QgsDebugMsg( QString( "transforming layer extent %1" ).arg( extent.toString( true ) ) );
  try
  {
    extent = xform.transformBoundingBox( extent );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    return false;
  }
  QgsDebugMsg( QString( "transformed layer extent %1" ).arg( extent.toString( true ) ) );

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
  QgsDebugMsg( "received the following data: " + responsestring );
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
      QgsDebugMsg( e.tagName() ); // the node really is an element.

      QString tagName = e.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "ServiceException" ) )
      {
        QgsDebugMsg( "  ServiceException." );
        parseServiceException( e, errorTitle, errorText );
      }

    }
    n = n.nextSibling();
  }

  QgsDebugMsg( "exiting." );

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

  QgsDebugMsg( QString( "exiting with composed error message '%1'." ).arg( errorText ) );
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
        QgsDebugMsg( QString( "Skip %1 [%2]" ).arg( mTileLayer->boundingBoxes.at( i ).crs, mImageCrs ) );

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

          QgsCoordinateTransform ct( qgisSrsSource, qgisSrsDest );

          QgsDebugMsg( QString( "ct: %1 => %2" ).arg( mTileLayer->boundingBoxes.at( i ).crs, mImageCrs ) );

          try
          {
            QgsRectangle extent = ct.transformBoundingBox( mTileLayer->boundingBoxes.at( i ).box, QgsCoordinateTransform::ForwardTransform );

            //make sure extent does not contain 'inf' or 'nan'
            if ( extent.isFinite() )
            {
              mLayerExtent = extent;
              break;
            }
          }
          catch ( QgsCsException &cse )
          {
            Q_UNUSED( cse );
          }
        }
      }

      QgsDebugMsg( "exiting with '"  + mLayerExtent.toString() + "'." );

      return true;
    }

    QgsDebugMsg( "no extent returned" );
    return false;
  }
  else
  {
    bool firstLayer = true; //flag to know if a layer is the first to be successfully transformed
    for ( QStringList::const_iterator it  = mSettings.mActiveSubLayers.constBegin();
          it != mSettings.mActiveSubLayers.constEnd();
          ++it )
    {
      QgsDebugMsg( "Sublayer iterator: " + *it );

      QgsRectangle extent;
      if ( !extentForNonTiledLayer( *it, mImageCrs, extent ) )
      {
        QgsDebugMsg( "extent for " + *it + " is invalid! (ignoring)" );
        continue;
      }

      QgsDebugMsg( "extent for " + *it  + " is " + extent.toString( 3 )  + '.' );

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

      QgsDebugMsg( "combined extent is '"  + mLayerExtent.toString()
                   + "' after '"  + ( *it ) + "'." );

    }

    QgsDebugMsg( "exiting with '"  + mLayerExtent.toString() + "'." );
    return true;
  }
}


int QgsWmsProvider::capabilities() const
{
  int capability = NoCapabilities;
  bool canIdentify = false;


  if ( mSettings.mTiled && mTileLayer )
  {
    QgsDebugMsg( "Tiled." );
    canIdentify = !mTileLayer->getFeatureInfoURLs.isEmpty() || !getFeatureInfoUrl().isNull();
  }
  else
  {
    QgsDebugMsg( "Not tiled." );
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
          QgsDebugMsg( '\''  + ( *it )  + "' is queryable." );
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
      capability |= Identify;
    }
  }

  QgsDebugMsg( QString( "capability = %1" ).arg( capability ) );
  return capability;
}

QString QgsWmsProvider::layerMetadata( QgsWmsLayerProperty &layer )
{
  QString metadata;

  // Layer Properties section

  // Use a nested table
  metadata += QLatin1String( "<tr><td>" );
  metadata += QLatin1String( "<table width=\"100%\">" );

  // Table header
  metadata += QLatin1String( "<tr><th class=\"glossy\">" );
  metadata += tr( "Property" );
  metadata += QLatin1String( "</th>" );
  metadata += QLatin1String( "<th class=\"glossy\">" );
  metadata += tr( "Value" );
  metadata += QLatin1String( "</th></tr>" );

  // Name
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Name" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += layer.name;
  metadata += QLatin1String( "</td></tr>" );

  // Layer Visibility (as managed by this provider)
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Visibility" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += mActiveSubLayerVisibility.find( layer.name ).value() ? tr( "Visible" ) : tr( "Hidden" );
  metadata += QLatin1String( "</td></tr>" );

  // Layer Title
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Title" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += layer.title;
  metadata += QLatin1String( "</td></tr>" );

  // Layer Abstract
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Abstract" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += layer.abstract;
  metadata += QLatin1String( "</td></tr>" );

  // Layer Queryability
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Can Identify" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += layer.queryable ? tr( "Yes" ) : tr( "No" );
  metadata += QLatin1String( "</td></tr>" );

  // Layer Opacity
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Can be Transparent" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += layer.opaque ? tr( "No" ) : tr( "Yes" );
  metadata += QLatin1String( "</td></tr>" );

  // Layer Subsetability
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Can Zoom In" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += layer.noSubsets ? tr( "No" ) : tr( "Yes" );
  metadata += QLatin1String( "</td></tr>" );

  // Layer Server Cascade Count
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Cascade Count" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += QString::number( layer.cascaded );
  metadata += QLatin1String( "</td></tr>" );

  // Layer Fixed Width
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Fixed Width" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += QString::number( layer.fixedWidth );
  metadata += QLatin1String( "</td></tr>" );

  // Layer Fixed Height
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Fixed Height" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += QString::number( layer.fixedHeight );
  metadata += QLatin1String( "</td></tr>" );

  // Layer Coordinate Reference Systems
  for ( int j = 0; j < std::min( layer.crs.size(), 10 ); j++ )
  {
    metadata += QLatin1String( "<tr><td>" );
    metadata += tr( "Available in CRS" );
    metadata += QLatin1String( "</td>" );
    metadata += QLatin1String( "<td>" );
    metadata += layer.crs[j];
    metadata += QLatin1String( "</td></tr>" );
  }

  if ( layer.crs.size() > 10 )
  {
    metadata += QLatin1String( "<tr><td>" );
    metadata += tr( "Available in CRS" );
    metadata += QLatin1String( "</td>" );
    metadata += QLatin1String( "<td>" );
    metadata += tr( "(and %n more)", "crs", layer.crs.size() - 10 );
    metadata += QLatin1String( "</td></tr>" );
  }

  // Layer Styles
  for ( int j = 0; j < layer.style.size(); j++ )
  {
    const QgsWmsStyleProperty &style = layer.style.at( j );

    metadata += QLatin1String( "<tr><td>" );
    metadata += tr( "Available in style" );
    metadata += QLatin1String( "</td>" );
    metadata += QLatin1String( "<td>" );

    // Nested table.
    metadata += QLatin1String( "<table width=\"100%\">" );

    // Layer Style Name
    metadata += QLatin1String( "<tr><th class=\"glossy\">" );
    metadata += tr( "Name" );
    metadata += QLatin1String( "</th>" );
    metadata += QLatin1String( "<td>" );
    metadata += style.name;
    metadata += QLatin1String( "</td></tr>" );

    // Layer Style Title
    metadata += QLatin1String( "<tr><th class=\"glossy\">" );
    metadata += tr( "Title" );
    metadata += QLatin1String( "</th>" );
    metadata += QLatin1String( "<td>" );
    metadata += style.title;
    metadata += QLatin1String( "</td></tr>" );

    // Layer Style Abstract
    metadata += QLatin1String( "<tr><th class=\"glossy\">" );
    metadata += tr( "Abstract" );
    metadata += QLatin1String( "</th>" );
    metadata += QLatin1String( "<td>" );
    metadata += style.abstract;
    metadata += QLatin1String( "</td></tr>" );

    // LegendURLs
    if ( !style.legendUrl.isEmpty() )
    {
      metadata += QLatin1String( "<tr><th class=\"glossy\">" );
      metadata += tr( "LegendURLs" );
      metadata += QLatin1String( "</th>" );
      metadata += QLatin1String( "<td><table>" );
      metadata += QLatin1String( "<tr><th>Format</th><th>URL</th></tr>" );
      for ( int k = 0; k < style.legendUrl.size(); k++ )
      {
        const QgsWmsLegendUrlProperty &l = style.legendUrl[k];
        metadata += "<tr><td>" + l.format + "</td><td>" + l.onlineResource.xlinkHref + "</td></tr>";
      }
      metadata += QLatin1String( "</table></td></tr>" );
    }

    // Close the nested table
    metadata += QLatin1String( "</table>" );
    metadata += QLatin1String( "</td></tr>" );
  }

  // Close the nested table
  metadata += QLatin1String( "</table>" );
  metadata += QLatin1String( "</td></tr>" );

  return metadata;
}

QString QgsWmsProvider::metadata()
{
  QString metadata = QLatin1String( "" );

  metadata += QLatin1String( "<tr><td>" );

  if ( !mSettings.mTiled )
  {
    metadata += QLatin1String( "&nbsp;<a href=\"#selectedlayers\">" );
    metadata += tr( "Selected Layers" );
    metadata += QLatin1String( "</a>&nbsp;<a href=\"#otherlayers\">" );
    metadata += tr( "Other Layers" );
    metadata += QLatin1String( "</a>" );
  }
  else
  {
    metadata += QLatin1String( "&nbsp;<a href=\"#tilesetproperties\">" );
    metadata += tr( "Tile Layer Properties" );
    metadata += QLatin1String( "</a> " );

    metadata += QLatin1String( "&nbsp;<a href=\"#cachestats\">" );
    metadata += tr( "Cache Stats" );
    metadata += QLatin1String( "</a> " );
  }

  metadata += QLatin1String( "</td></tr>" );

  // Server Properties section
  metadata += QLatin1String( "<tr><th class=\"glossy\"><a name=\"serverproperties\"></a>" );
  metadata += tr( "Server Properties" );
  metadata += QLatin1String( "</th></tr>" );

  // Use a nested table
  metadata += QLatin1String( "<tr><td>" );
  metadata += QLatin1String( "<table width=\"100%\">" );

  // Table header
  metadata += QLatin1String( "<tr><th class=\"glossy\">" );
  metadata += tr( "Property" );
  metadata += QLatin1String( "</th>" );
  metadata += QLatin1String( "<th class=\"glossy\">" );
  metadata += tr( "Value" );
  metadata += QLatin1String( "</th></tr>" );

  // WMS Version
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "WMS Version" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += mCaps.mCapabilities.version;
  metadata += QLatin1String( "</td></tr>" );

  // Service Title
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Title" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += mCaps.mCapabilities.service.title;
  metadata += QLatin1String( "</td></tr>" );

  // Service Abstract
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Abstract" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += mCaps.mCapabilities.service.abstract;
  metadata += QLatin1String( "</td></tr>" );

  // Service Keywords
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Keywords" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += mCaps.mCapabilities.service.keywordList.join( QStringLiteral( "<br />" ) );
  metadata += QLatin1String( "</td></tr>" );

  // Service Online Resource
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Online Resource" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += '-';
  metadata += QLatin1String( "</td></tr>" );

  // Service Contact Information
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Contact Person" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += mCaps.mCapabilities.service.contactInformation.contactPersonPrimary.contactPerson;
  metadata += QLatin1String( "<br />" );
  metadata += mCaps.mCapabilities.service.contactInformation.contactPosition;
  metadata += QLatin1String( "<br />" );
  metadata += mCaps.mCapabilities.service.contactInformation.contactPersonPrimary.contactOrganization;
  metadata += QLatin1String( "</td></tr>" );

  // Service Fees
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Fees" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += mCaps.mCapabilities.service.fees;
  metadata += QLatin1String( "</td></tr>" );

  // Service Access Constraints
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "Access Constraints" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += mCaps.mCapabilities.service.accessConstraints;
  metadata += QLatin1String( "</td></tr>" );

  // Base URL
  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "GetCapabilitiesUrl" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += mSettings.mBaseUrl;
  metadata += QLatin1String( "</td></tr>" );

  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "GetMapUrl" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += getMapUrl() + ( mSettings.mIgnoreGetMapUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : QLatin1String( "" ) );
  metadata += QLatin1String( "</td></tr>" );

  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "GetFeatureInfoUrl" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += getFeatureInfoUrl() + ( mSettings.mIgnoreGetFeatureInfoUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : QLatin1String( "" ) );
  metadata += QLatin1String( "</td></tr>" );

  metadata += QLatin1String( "<tr><td>" );
  metadata += tr( "GetLegendGraphic" );
  metadata += QLatin1String( "</td>" );
  metadata += QLatin1String( "<td>" );
  metadata += getLegendGraphicUrl() + ( mSettings.mIgnoreGetMapUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : QLatin1String( "" ) );
  metadata += QLatin1String( "</td></tr>" );

  if ( mSettings.mTiled )
  {
    metadata += QLatin1String( "<tr><td>" );
    metadata += tr( "Tile Layer Count" );
    metadata += QLatin1String( "</td>" );
    metadata += QLatin1String( "<td>" );
    metadata += QString::number( mCaps.mTileLayersSupported.size() );
    metadata += QLatin1String( "</td></tr>" );

    metadata += QLatin1String( "<tr><td>" );
    metadata += tr( "GetTileUrl" );
    metadata += QLatin1String( "</td>" );
    metadata += QLatin1String( "<td>" );
    metadata += getTileUrl();
    metadata += QLatin1String( "</td></tr>" );

    if ( mTileLayer )
    {
      metadata += QLatin1String( "<tr><td>" );
      metadata += tr( "Tile templates" );
      metadata += QLatin1String( "</td>" );
      metadata += QLatin1String( "<td>" );
      for ( QHash<QString, QString>::const_iterator it = mTileLayer->getTileURLs.constBegin();
            it != mTileLayer->getTileURLs.constEnd();
            ++it )
      {
        metadata += QStringLiteral( "%1:%2<br>" ).arg( it.key(), it.value() );
      }
      metadata += QLatin1String( "</td></tr>" );

      metadata += QLatin1String( "<tr><td>" );
      metadata += tr( "FeatureInfo templates" );
      metadata += QLatin1String( "</td>" );
      metadata += QLatin1String( "<td>" );
      for ( QHash<QString, QString>::const_iterator it = mTileLayer->getFeatureInfoURLs.constBegin();
            it != mTileLayer->getFeatureInfoURLs.constEnd();
            ++it )
      {
        metadata += QStringLiteral( "%1:%2<br>" ).arg( it.key(), it.value() );
      }
      metadata += QLatin1String( "</td></tr>" );
    }

    // GetFeatureInfo Request Formats
    metadata += QLatin1String( "<tr><td>" );
    metadata += tr( "Identify Formats" );
    metadata += QLatin1String( "</td>" );
    metadata += QLatin1String( "<td>" );
    metadata += mTileLayer->infoFormats.join( QStringLiteral( "<br />" ) );
    metadata += QLatin1String( "</td></tr>" );
  }
  else
  {
    // GetMap Request Formats
    metadata += QLatin1String( "<tr><td>" );
    metadata += tr( "Image Formats" );
    metadata += QLatin1String( "</td>" );
    metadata += QLatin1String( "<td>" );
    metadata += mCaps.mCapabilities.capability.request.getMap.format.join( QStringLiteral( "<br />" ) );
    metadata += QLatin1String( "</td></tr>" );

    // GetFeatureInfo Request Formats
    metadata += QLatin1String( "<tr><td>" );
    metadata += tr( "Identify Formats" );
    metadata += QLatin1String( "</td>" );
    metadata += QLatin1String( "<td>" );
    metadata += mCaps.mCapabilities.capability.request.getFeatureInfo.format.join( QStringLiteral( "<br />" ) );
    metadata += QLatin1String( "</td></tr>" );

    // Layer Count (as managed by this provider)
    metadata += QLatin1String( "<tr><td>" );
    metadata += tr( "Layer Count" );
    metadata += QLatin1String( "</td>" );
    metadata += QLatin1String( "<td>" );
    metadata += QString::number( mCaps.mLayersSupported.size() );
    metadata += QLatin1String( "</td></tr>" );
  }

  // Close the nested table
  metadata += QLatin1String( "</table>" );
  metadata += QLatin1String( "</td></tr>" );

  // Layer properties
  if ( !mSettings.mTiled )
  {
    metadata += QLatin1String( "<tr><th class=\"glossy\"><a name=\"selectedlayers\"></a>" );
    metadata += tr( "Selected Layers" );
    metadata += QLatin1String( "</th></tr>" );

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
      metadata += QLatin1String( "<tr><th class=\"glossy\"><a name=\"otherlayers\"></a>" );
      metadata += tr( "Other Layers" );
      metadata += QLatin1String( "</th></tr>" );

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
    metadata += QLatin1String( "<tr><th class=\"glossy\"><a name=\"tilesetproperties\"></a>" );
    metadata += tr( "Tileset Properties" );
    metadata += QLatin1String( "</th></tr>" );

    // Iterate through tilesets
    metadata += QLatin1String( "<tr><td>" );

    metadata += QLatin1String( "<table width=\"100%\">" );

    Q_FOREACH ( const QgsWmtsTileLayer &l, mCaps.mTileLayersSupported )
    {
      metadata += QLatin1String( "<tr><th class=\"glossy\">" );
      metadata += tr( "Identifier" );
      metadata += QLatin1String( "</th><th class=\"glossy\">" );
      metadata += tr( "Tile mode" );
      metadata += QLatin1String( "</th></tr>" );

      metadata += QLatin1String( "<tr><td>" );
      metadata += l.identifier;
      metadata += QLatin1String( "</td><td class=\"glossy\">" );

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

      metadata += QLatin1String( "</td></tr>" );

      // Table header
      metadata += QLatin1String( "<tr><th class=\"glossy\">" );
      metadata += tr( "Property" );
      metadata += QLatin1String( "</th>" );
      metadata += QLatin1String( "<th class=\"glossy\">" );
      metadata += tr( "Value" );
      metadata += QLatin1String( "</th></tr>" );

      metadata += QLatin1String( "<tr><td class=\"glossy\">" );
      metadata += tr( "Selected" );
      metadata += QLatin1String( "</td>" );
      metadata += QLatin1String( "<td class=\"glossy\">" );
      metadata += l.identifier == mSettings.mActiveSubLayers.join( QStringLiteral( "," ) ) ? tr( "Yes" ) : tr( "No" );
      metadata += QLatin1String( "</td></tr>" );

      if ( !l.styles.isEmpty() )
      {
        metadata += QLatin1String( "<tr><td class=\"glossy\">" );
        metadata += tr( "Available Styles" );
        metadata += QLatin1String( "</td>" );
        metadata += QLatin1String( "<td class=\"glossy\">" );
        QStringList styles;
        Q_FOREACH ( const QgsWmtsStyle &style, l.styles )
        {
          styles << style.identifier;
        }
        metadata += styles.join( QStringLiteral( ", " ) );
        metadata += QLatin1String( "</td></tr>" );
      }

      metadata += QLatin1String( "<tr><td class=\"glossy\">" );
      metadata += tr( "CRS" );
      metadata += QLatin1String( "</td>" );
      metadata += QLatin1String( "<td>" );
      metadata += QLatin1String( "<table><tr>" );
      metadata += QLatin1String( "<td class=\"glossy\">" );
      metadata += tr( "CRS" );
      metadata += QLatin1String( "</td>" );
      metadata += QLatin1String( "<td class=\"glossy\">" );
      metadata += tr( "Bounding Box" );
      metadata += QLatin1String( "</td>" );
      for ( int i = 0; i < l.boundingBoxes.size(); i++ )
      {
        metadata += QLatin1String( "<tr><td>" );
        metadata += l.boundingBoxes[i].crs;
        metadata += QLatin1String( "</td><td>" );
        metadata += l.boundingBoxes[i].box.toString();
        metadata += QLatin1String( "</td></tr>" );
      }
      metadata += QLatin1String( "</table></td></tr>" );

      metadata += QLatin1String( "<tr><td class=\"glossy\">" );
      metadata += tr( "Available Tilesets" );
      metadata += QLatin1String( "</td><td class=\"glossy\">" );

      Q_FOREACH ( const QgsWmtsTileMatrixSetLink &setLink, l.setLinks )
      {
        metadata += setLink.tileMatrixSet + "<br>";
      }

      metadata += QLatin1String( "</td></tr>" );
    }

    metadata += QLatin1String( "</table></td></tr>" );

    if ( mTileMatrixSet )
    {
      // Iterate through tilesets
      metadata += QLatin1String( "<tr><td><table width=\"100%\">" );

      metadata += QString( "<tr><th colspan=14 class=\"glossy\">%1 %2</th></tr>"
                           "<tr>"
                           "<th rowspan=2 class=\"glossy\">%3</th>"
                           "<th colspan=2 class=\"glossy\">%4</th>"
                           "<th colspan=2 class=\"glossy\">%5</th>"
                           "<th colspan=2 class=\"glossy\">%6</th>"
                           "<th colspan=2 class=\"glossy\">%7</th>"
                           "<th colspan=4 class=\"glossy\">%8</th>"
                           "</tr><tr>"
                           "<th class=\"glossy\">%9</th><th class=\"glossy\">%10</th>"
                           "<th class=\"glossy\">%9</th><th class=\"glossy\">%10</th>"
                           "<th class=\"glossy\">%9</th><th class=\"glossy\">%10</th>"
                           "<th class=\"glossy\">%9</th><th class=\"glossy\">%10</th>"
                           "<th class=\"glossy\">%11</th>"
                           "<th class=\"glossy\">%12</th>"
                           "<th class=\"glossy\">%13</th>"
                           "<th class=\"glossy\">%14</th>"
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

      Q_FOREACH ( const QVariant &res, property( "resolutions" ).toList() )
      {
        double key = res.toDouble();

        QgsWmtsTileMatrix &tm = mTileMatrixSet->tileMatrices[ key ];

        double tw = key * tm.tileWidth;
        double th = key * tm.tileHeight;

        QgsRectangle r( tm.topLeft.x(), tm.topLeft.y() - tw * tm.matrixWidth, tm.topLeft.x() + th * tm.matrixHeight, tm.topLeft.y() );

        metadata += QString( "<tr>"
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
          metadata += QStringLiteral( "<td title=\"%1<br>%2\"><font color=\"red\">%3</font></td>" )
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

      metadata += QLatin1String( "</table></td></tr>" );
    }

    const QgsWmsStatistics::Stat &stat = QgsWmsStatistics::statForUri( dataSourceUri() );

    metadata += QLatin1String( "<tr><th class=\"glossy\"><a name=\"cachestats\"></a>" );
    metadata += tr( "Cache stats" );
    metadata += QLatin1String( "</th></tr>" );

    metadata += QLatin1String( "<tr><td><table width=\"100%\">" );

    metadata += QLatin1String( "<tr><th class=\"glossy\">" );
    metadata += tr( "Property" );
    metadata += QLatin1String( "</th>" );
    metadata += QLatin1String( "<th class=\"glossy\">" );
    metadata += tr( "Value" );
    metadata += QLatin1String( "</th></tr>" );

    metadata += QLatin1String( "<tr><td>" );
    metadata += tr( "Hits" );
    metadata += QLatin1String( "</td><td>" );
    metadata += QString::number( stat.cacheHits );
    metadata += QLatin1String( "</td></tr>" );

    metadata += QLatin1String( "<tr><td>" );
    metadata += tr( "Misses" );
    metadata += QLatin1String( "</td><td>" );
    metadata += QString::number( stat.cacheMisses );
    metadata += QLatin1String( "</td></tr>" );

    metadata += QLatin1String( "<tr><td>" );
    metadata += tr( "Errors" );
    metadata += QLatin1String( "</td><td>" );
    metadata += QString::number( stat.errors );
    metadata += QLatin1String( "</td></tr>" );

    metadata += QLatin1String( "</table></td></tr>" );
  }

  metadata += QLatin1String( "</table>" );

  QgsDebugMsg( "exiting with '"  + metadata  + "'." );

  return metadata;
}

QgsRasterIdentifyResult QgsWmsProvider::identify( const QgsPointXY &point, QgsRaster::IdentifyFormat format, const QgsRectangle &boundingBox, int width, int height, int /*dpi*/ )
{
  QgsDebugMsg( QString( "format = %1" ).arg( format ) );

  QString formatStr;
  formatStr = mCaps.mIdentifyFormats.value( format );
  if ( formatStr.isEmpty() )
  {
    return QgsRasterIdentifyResult( QGS_ERROR( tr( "Format not supported" ) ) );
  }

  QgsDebugMsg( QString( "format = %1 format = %2" ).arg( format ).arg( formatStr ) );

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

  QgsDebugMsg( "myExtent = " + myExtent.toString() );
  QgsDebugMsg( QString( "theWidth = %1 height = %2" ).arg( width ).arg( height ) );
  QgsDebugMsg( QString( "xRes = %1 yRes = %2" ).arg( xRes ).arg( yRes ) );

  QgsPointXY finalPoint;
  finalPoint.setX( std::floor( ( finalPoint.x() - myExtent.xMinimum() ) / xRes ) );
  finalPoint.setY( std::floor( ( myExtent.yMaximum() - finalPoint.y() ) / yRes ) );

  QgsDebugMsg( QString( "point = %1 %2" ).arg( finalPoint.x() ).arg( finalPoint.y() ) );

  QgsDebugMsg( QString( "recalculated orig point (corner) = %1 %2" ).arg( myExtent.xMinimum() + finalPoint.x()*xRes ).arg( myExtent.yMaximum() - finalPoint.y()*yRes ) );

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

      QgsDebugMsg( "Layer '" + *layers + "' is queryable." );

      QUrl requestUrl( mSettings.mIgnoreGetFeatureInfoUrl ? mSettings.mBaseUrl : getFeatureInfoUrl() );
      setQueryItem( requestUrl, QStringLiteral( "SERVICE" ), QStringLiteral( "WMS" ) );
      setQueryItem( requestUrl, QStringLiteral( "VERSION" ), mCaps.mCapabilities.version );
      setQueryItem( requestUrl, QStringLiteral( "REQUEST" ), QStringLiteral( "GetFeatureInfo" ) );
      setQueryItem( requestUrl, QStringLiteral( "BBOX" ), bbox );
      setSRSQueryItem( requestUrl );
      setQueryItem( requestUrl, QStringLiteral( "WIDTH" ), QString::number( width ) );
      setQueryItem( requestUrl, QStringLiteral( "HEIGHT" ), QString::number( height ) );
      setQueryItem( requestUrl, QStringLiteral( "LAYERS" ), *layers );
      setQueryItem( requestUrl, QStringLiteral( "STYLES" ), *styles );
      setFormatQueryItem( requestUrl );
      setQueryItem( requestUrl, QStringLiteral( "QUERY_LAYERS" ), *layers );
      setQueryItem( requestUrl, QStringLiteral( "INFO_FORMAT" ), formatStr );

      if ( mCaps.mCapabilities.version == QLatin1String( "1.3.0" ) || mCaps.mCapabilities.version == QLatin1String( "1.3" ) )
      {
        setQueryItem( requestUrl, QStringLiteral( "I" ), QString::number( finalPoint.x() ) );
        setQueryItem( requestUrl, QStringLiteral( "J" ), QString::number( finalPoint.y() ) );
      }
      else
      {
        setQueryItem( requestUrl, QStringLiteral( "X" ), QString::number( finalPoint.x() ) );
        setQueryItem( requestUrl, QStringLiteral( "Y" ), QString::number( finalPoint.y() ) );
      }

      if ( mSettings.mFeatureCount > 0 )
      {
        setQueryItem( requestUrl, QStringLiteral( "FEATURE_COUNT" ), QString::number( mSettings.mFeatureCount ) );
      }

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

    QMap<double, QgsWmtsTileMatrix> &m =  mTileMatrixSet->tileMatrices;

    // find nearest resolution
    QMap<double, QgsWmtsTileMatrix>::const_iterator prev, it = m.constBegin();
    while ( it != m.constEnd() && it.key() < vres )
    {
      QgsDebugMsg( QString( "res:%1 >= %2" ).arg( it.key() ).arg( vres ) );
      prev = it;
      ++it;
    }

    if ( it == m.constEnd() ||
         ( it != m.constBegin() && vres - prev.key() < it.key() - vres ) )
    {
      QgsDebugMsg( "back to previous res" );
      it = prev;
    }

    tres = it.key();
    tm = &it.value();

    QgsDebugMsg( QString( "layer extent: %1,%2 %3x%4" )
                 .arg( qgsDoubleToString( mLayerExtent.xMinimum() ),
                       qgsDoubleToString( mLayerExtent.yMinimum() ) )
                 .arg( mLayerExtent.width() )
                 .arg( mLayerExtent.height() )
               );

    QgsDebugMsg( QString( "view extent: %1,%2 %3x%4  res:%5" )
                 .arg( qgsDoubleToString( boundingBox.xMinimum() ),
                       qgsDoubleToString( boundingBox.yMinimum() ) )
                 .arg( boundingBox.width() )
                 .arg( boundingBox.height() )
                 .arg( vres, 0, 'f' )
               );

    QgsDebugMsg( QString( "tile matrix %1,%2 res:%3 tilesize:%4x%5 matrixsize:%6x%7 id:%8" )
                 .arg( tm->topLeft.x() ).arg( tm->topLeft.y() ).arg( tres )
                 .arg( tm->tileWidth ).arg( tm->tileHeight )
                 .arg( tm->matrixWidth ).arg( tm->matrixHeight )
                 .arg( tm->identifier )
               );

    // calculate tile coordinates
    double twMap = tm->tileWidth * tres;
    double thMap = tm->tileHeight * tres;
    QgsDebugMsg( QString( "tile map size: %1,%2" ).arg( qgsDoubleToString( twMap ), qgsDoubleToString( thMap ) ) );

    int col = ( int ) std::floor( ( point.x() - tm->topLeft.x() ) / twMap );
    int row = ( int ) std::floor( ( tm->topLeft.y() - point.y() ) / thMap );
    double tx = tm->topLeft.x() + col * twMap;
    double ty = tm->topLeft.y() - row * thMap;
    int i   = ( point.x() - tx ) / tres;
    int j   = ( ty - point.y() ) / tres;

    QgsDebugMsg( QString( "col=%1 row=%2 i=%3 j=%4 tx=%5 ty=%6" ).arg( col ).arg( row ).arg( i ).arg( j ).arg( tx, 0, 'f', 1 ).arg( ty, 0, 'f', 1 ) );

    if ( mTileLayer->getFeatureInfoURLs.contains( formatStr ) )
    {
      // REST
      QString url = mTileLayer->getFeatureInfoURLs[ formatStr ];

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

      // compose static request arguments.
      setQueryItem( url, QStringLiteral( "SERVICE" ), QStringLiteral( "WMTS" ) );
      setQueryItem( url, QStringLiteral( "REQUEST" ), QStringLiteral( "GetFeatureInfo" ) );
      setQueryItem( url, QStringLiteral( "VERSION" ), mCaps.mCapabilities.version );
      setQueryItem( url, QStringLiteral( "LAYER" ), mSettings.mActiveSubLayers[0] );
      setQueryItem( url, QStringLiteral( "STYLE" ), mSettings.mActiveSubStyles[0] );
      setQueryItem( url, QStringLiteral( "INFOFORMAT" ), formatStr );
      setQueryItem( url, QStringLiteral( "TILEMATRIXSET" ), mTileMatrixSet->identifier );
      setQueryItem( url, QStringLiteral( "TILEMATRIX" ), tm->identifier );

      for ( QHash<QString, QString>::const_iterator it = mSettings.mTileDimensionValues.constBegin(); it != mSettings.mTileDimensionValues.constEnd(); ++it )
      {
        setQueryItem( url, it.key(), it.value() );
      }

      setQueryItem( url, QStringLiteral( "TILEROW" ), QString::number( row ) );
      setQueryItem( url, QStringLiteral( "TILECOL" ), QString::number( col ) );
      setQueryItem( url, QStringLiteral( "I" ), qgsDoubleToString( i ) );
      setQueryItem( url, QStringLiteral( "J" ), qgsDoubleToString( j ) );

      urls << url;
      layerList << mSettings.mActiveSubLayers[0];
    }
    else
    {
      QgsDebugMsg( QString( "No KVP and no feature info url for format %1" ).arg( formatStr ) );
    }
  }

  for ( int count = 0; count < urls.size(); count++ )
  {
    const QUrl &requestUrl = urls[count];

    QgsDebugMsg( QString( "getfeatureinfo: %1" ).arg( requestUrl.toString() ) );
    QNetworkRequest request( requestUrl );
    mSettings.authorization().setAuthorization( request );
    mIdentifyReply = QgsNetworkAccessManager::instance()->get( request );
    connect( mIdentifyReply, &QNetworkReply::finished, this, &QgsWmsProvider::identifyReplyFinished );

    QEventLoop loop;
    mIdentifyReply->setProperty( "eventLoop", QVariant::fromValue( qobject_cast<QObject *>( &loop ) ) );
    loop.exec( QEventLoop::ExcludeUserInputEvents );

    if ( mIdentifyResultBodies.isEmpty() ) // no result
    {
      QgsDebugMsg( "mIdentifyResultBodies is empty" );
      continue;
    }
    else if ( mIdentifyResultBodies.size() == 1 )
    {
      // Check for service exceptions (exceptions with ogr/gml are in the body)
      bool isXml = false;
      bool isGml = false;

      const QgsNetworkReplyParser::RawHeaderMap &headers = mIdentifyResultHeaders.value( 0 );
      Q_FOREACH ( const QByteArray &v, headers.keys() )
      {
        if ( QString( v ).compare( QLatin1String( "Content-Type" ), Qt::CaseInsensitive ) == 0 )
        {
          isXml = QString( headers.value( v ) ).compare( QLatin1String( "text/xml" ), Qt::CaseInsensitive ) == 0;
          isGml = QString( headers.value( v ) ).compare( QLatin1String( "ogr/gml" ), Qt::CaseInsensitive ) == 0;
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
          QgsMessageLog::logMessage( tr( "Get feature info request error (Title:%1; Error:%2; URL: %3)" )
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
          QgsDebugMsg( "Multipart with 2 parts - expected GML + XSD" );
          // How to find which part is GML and which XSD? Both have
          // Content-Type: application/binary
          // different are Content-Disposition but it is not reliable.
          // We could analyze beginning of bodies...
          gmlPart = 0;
          xsdPart = 1;
        }
      }
      QgsDebugMsg( QString( "jsonPart = %1 gmlPart = %2 xsdPart = %3" ).arg( jsonPart ).arg( gmlPart ).arg( xsdPart ) );

      if ( gmlPart >= 0 )
      {
        QByteArray gmlByteArray = mIdentifyResultBodies.value( gmlPart );
        QgsDebugMsg( "GML (first 2000 bytes):\n" + gmlByteArray.left( 2000 ) );

        // QgsGmlSchema.guessSchema() and QgsGml::getFeatures() are using Expat
        // which only accepts UTF-8, UTF-16, ISO-8859-1
        // http://sourceforge.net/p/expat/bugs/498/
        QDomDocument dom;
        dom.setContent( gmlByteArray ); // gets XML encoding
        gmlByteArray.clear();
        QTextStream stream( &gmlByteArray );
        stream.setCodec( QTextCodec::codecForName( "UTF-8" ) );
        dom.save( stream, 4, QDomNode::EncodingFromTextStream );

        QgsDebugMsg( "GML UTF-8 (first 2000 bytes):\n" + gmlByteArray.left( 2000 ) );

        QgsWkbTypes::Type wkbType;
        QgsGmlSchema gmlSchema;

        if ( xsdPart >= 0 )  // XSD available
        {
#if 0
          // Validate GML by schema
          // Loading schema takes ages! It needs to load all XSD referenced in the schema,
          // for example:
          // http://schemas.opengis.net/gml/2.1.2/feature.xsd
          // http://schemas.opengis.net/gml/2.1.2/gml.xsd
          // http://schemas.opengis.net/gml/2.1.2/geometry.xsd
          // http://www.w3.org/1999/xlink.xsd
          // http://www.w3.org/2001/xml.xsd <- this takes 30s to download (2/2013)

          QXmlSchema schema;
          schema.load( mIdentifyResultBodies.value( xsdPart ) );
          // Unfortunately the schema cannot be successfully loaded, it reports error
          // "Element {http://www.opengis.net/gml}_Feature already defined"
          // there is probably a bug in QXmlSchema:
          // https://bugreports.qt-project.org/browse/QTBUG-8394
          // xmlpatternsvalidator gives the same error on XSD generated by OGR
          if ( !schema.isValid() )
          {
            // TODO: return QgsError
            results.insert( count, tr( "GML schema is not valid" ) );
            continue;
          }
          QXmlSchemaValidator validator( schema );
          if ( !validator.validate( mIdentifyResultBodies.value( gmlPart ) ) )
          {
            results.insert( count, tr( "GML is not valid" ) );
            continue;
          }
#endif
          QgsDebugMsg( "GML XSD (first 4000 bytes):\n" + QString::fromUtf8( mIdentifyResultBodies.value( xsdPart ).left( 4000 ) ) );
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
        QgsDebugMsg( QString( "%1 featureTypeNames found" ).arg( featureTypeNames.size() ) );

        // Each sublayer may have more features of different types, for example
        // if GROUP of multiple vector layers is used with UMN MapServer
        // Note: GROUP of layers in UMN MapServer is not queryable by default
        // (and I could not find a way to force it), it is possible however
        // to add another RASTER layer with the same name as group which is queryable
        // and has no DATA defined. Then such a layer may be add to QGIS and both
        // GetMap and GetFeatureInfo will return data for the group of the same name.
        // https://github.com/mapserver/mapserver/issues/318#issuecomment-4923208
        QgsFeatureStoreList featureStoreList;
        Q_FOREACH ( const QString &featureTypeName, featureTypeNames )
        {
          QgsDebugMsg( QString( "featureTypeName = %1" ).arg( featureTypeName ) );

          QString geometryAttribute = gmlSchema.geometryAttributes( featureTypeName ).value( 0 );
          QList<QgsField> fieldList = gmlSchema.fields( featureTypeName );
          QgsDebugMsg( QString( "%1 fields found" ).arg( fieldList.size() ) );
          QgsFields fields;
          for ( int i = 0; i < fieldList.size(); i++ )
          {
            fields.append( fieldList[i] );
          }
          QgsGml gml( featureTypeName, geometryAttribute, fields );
          // TODO: avoid converting to string and back
          int ret = gml.getFeatures( gmlByteArray, &wkbType );
#ifdef QGISDEBUG
          QgsDebugMsg( QString( "parsing result = %1" ).arg( ret ) );
#else
          Q_UNUSED( ret );
#endif
          // TODO: all features coming from this layer should probably have the same CRS
          // the same as this layer, because layerExtentToOutputExtent() may be used
          // for results -> verify CRS and reprojects if necessary
          QMap<QgsFeatureId, QgsFeature * > features = gml.featuresMap();
          QgsCoordinateReferenceSystem featuresCrs = gml.crs();
          QgsDebugMsg( QString( "%1 features read, crs: %2 %3" ).arg( features.size() ).arg( featuresCrs.authid(), featuresCrs.description() ) );
          QgsCoordinateTransform coordinateTransform;
          if ( featuresCrs.isValid() && featuresCrs != crs() )
          {
            coordinateTransform = QgsCoordinateTransform( featuresCrs, crs() );
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

            QgsDebugMsg( QString( "feature id = %1 : %2 attributes" ).arg( featIt.key() ).arg( feature->attributes().size() ) );

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
          err.append( tr( "Result parsing failed. %1 feature types were guessed from gml (%2) but no features were parsed." ).arg( featureTypeNames.size() ).arg( featureTypeNames.join( QStringLiteral( "," ) ) ) );
          QgsDebugMsg( "parsing GML error: " + err.message() );
          return QgsRasterIdentifyResult( err );
        }
        results.insert( results.size(), qVariantFromValue( featureStoreList ) );
      }
      else if ( jsonPart != -1 )
      {
        QString json = QString::fromUtf8( mIdentifyResultBodies.value( jsonPart ) );
        json.prepend( '(' ).append( ')' );

        QScriptEngine engine;
        engine.evaluate( QStringLiteral( "function json_stringify(obj) { return JSON.stringify(obj); }" ) );
        QScriptValue json_stringify = engine.globalObject().property( QStringLiteral( "json_stringify" ) );
        Q_ASSERT( json_stringify.isFunction() );

        QScriptValue result = engine.evaluate( json );

        QgsFeatureStoreList featureStoreList;
        QgsCoordinateTransform coordinateTransform;

        try
        {
          QgsDebugMsg( QString( "result:%1" ).arg( result.toString() ) );

          if ( !result.isObject() )
            throw QStringLiteral( "object expected" );

          if ( result.property( QStringLiteral( "type" ) ).toString() != QLatin1String( "FeatureCollection" ) )
            throw QStringLiteral( "type FeatureCollection expected: %1" ).arg( result.property( QStringLiteral( "type" ) ).toString() );

          if ( result.property( QStringLiteral( "crs" ) ).isValid() )
          {
            QString crsType = result.property( QStringLiteral( "crs" ) ).property( QStringLiteral( "type" ) ).toString();
            QString crsText;
            if ( crsType == QLatin1String( "name" ) )
              crsText = result.property( QStringLiteral( "crs" ) ).property( QStringLiteral( "properties" ) ).property( QStringLiteral( "name" ) ).toString();
            else if ( crsType == QLatin1String( "EPSG" ) )
              crsText = QStringLiteral( "%1:%2" ).arg( crsType, result.property( QStringLiteral( "crs" ) ).property( QStringLiteral( "properties" ) ).property( QStringLiteral( "code" ) ).toString() );
            else
            {
              QgsDebugMsg( QString( "crs not supported:%1" ).arg( result.property( "crs" ).toString() ) );
            }

            QgsCoordinateReferenceSystem featuresCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsText );

            if ( !featuresCrs.isValid() )
              throw QStringLiteral( "CRS %1 invalid" ).arg( crsText );

            if ( featuresCrs.isValid() && featuresCrs != crs() )
            {
              coordinateTransform = QgsCoordinateTransform( featuresCrs, crs() );
            }
          }

          QScriptValue fc = result.property( QStringLiteral( "features" ) );
          if ( !fc.isArray() )
            throw QStringLiteral( "FeatureCollection array expected" );

          QScriptValue f;
          for ( int i = 0; f = fc.property( i ), f.isValid(); i++ )
          {
            QgsDebugMsg( QString( "feature %1" ).arg( i ) );

            QScriptValue props = f.property( QStringLiteral( "properties" ) );
            if ( !props.isObject() )
            {
              QgsDebugMsg( "no properties found" );
              continue;
            }

            QgsFields fields;
            QScriptValueIterator it( props );
            while ( it.hasNext() )
            {
              it.next();
              fields.append( QgsField( it.name(), QVariant::String ) );
            }

            QgsFeature feature( fields );

            if ( f.property( QStringLiteral( "geometry" ) ).isValid() )
            {
              QScriptValue geom = json_stringify.call( QScriptValue(), QScriptValueList() << f.property( QStringLiteral( "geometry" ) ) );
              if ( geom.isString() )
              {
                OGRGeometryH ogrGeom = OGR_G_CreateGeometryFromJson( geom.toString().toUtf8() );
                if ( ogrGeom )
                {
                  int wkbSize = OGR_G_WkbSize( ogrGeom );
                  unsigned char *wkb = new unsigned char[ wkbSize ];
                  OGR_G_ExportToWkb( ogrGeom, ( OGRwkbByteOrder ) QgsApplication::endian(), wkb );
                  OGR_G_DestroyGeometry( ogrGeom );

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
            }

            int j = 0;
            it.toFront();
            while ( it.hasNext() )
            {
              it.next();
              feature.setAttribute( j++, it.value().toString() );
            }

            QgsFeatureStore featureStore( fields, crs() );

            QMap<QString, QVariant> params;
            params.insert( QStringLiteral( "sublayer" ), layerList[count] );
            params.insert( QStringLiteral( "featureType" ), QStringLiteral( "%1_%2" ).arg( count ).arg( i ) );
            params.insert( QStringLiteral( "getFeatureInfoUrl" ), requestUrl.toString() );
            featureStore.setParams( params );

            feature.setValid( true );
            featureStore.addFeature( feature );

            featureStoreList.append( featureStore );
          }
        }
        catch ( const QString &err )
        {
          QgsDebugMsg( QString( "JSON error: %1\nResult: %2" ).arg( err, QString::fromUtf8( mIdentifyResultBodies.value( jsonPart ) ) ) );
          results.insert( results.size(), err );  // string returned for format type "feature" means error
        }

        results.insert( results.size(), qVariantFromValue( featureStoreList ) );
      }
    }
  }

  return QgsRasterIdentifyResult( format, results );
}

void QgsWmsProvider::identifyReplyFinished()
{
  QgsDebugMsg( "Entered." );
  mIdentifyResultHeaders.clear();
  mIdentifyResultBodies.clear();

  QEventLoop *loop = qobject_cast< QEventLoop *>( sender()->property( "eventLoop" ).value< QObject *>() );

  if ( mIdentifyReply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = mIdentifyReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      QgsDebugMsg( QString( "identify request redirected to %1" ).arg( redirect.toString() ) );
      emit statusChanged( tr( "identify request redirected." ) );

      mIdentifyReply->deleteLater();

      QgsDebugMsg( QString( "redirected getfeatureinfo: %1" ).arg( redirect.toString() ) );
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
      QgsDebugMsg( "Cannot parse reply" );
      mErrorFormat = QStringLiteral( "text/plain" );
      mError = tr( "Cannot parse getfeatureinfo: %1" ).arg( parser.error() );
      emit statusChanged( mError );
    }
    else
    {
      // TODO: check headers, xsd ...
      QgsDebugMsg( QString( "%1 parts" ).arg( parser.parts() ) );
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

QString QgsWmsProvider::lastErrorTitle()
{
  return mErrorCaption;
}


QString QgsWmsProvider::lastError()
{
  QgsDebugMsg( "returning '" + mError  + "'." );
  return mError;
}

QString QgsWmsProvider::lastErrorFormat()
{
  return mErrorFormat;
}

QString  QgsWmsProvider::name() const
{
  return WMS_KEY;
} //  QgsWmsProvider::name()


QString  QgsWmsProvider::description() const
{
  return WMS_DESCRIPTION;
} //  QgsWmsProvider::description()

void QgsWmsProvider::reloadData()
{
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

  QString lurl = getLegendGraphicUrl();

  if ( lurl.isEmpty() )
  {
    QgsDebugMsg( "getLegendGraphic url is empty" );
    return QUrl();
  }

  QgsDebugMsg( QString( "visibleExtent is %1" ).arg( visibleExtent.toString() ) );

  QUrl url( lurl );

  // query names are NOT case-sensitive, so make an uppercase list for proper comparison
  QStringList qnames = QStringList();
  for ( int i = 0; i < url.queryItems().size(); i++ )
  {
    qnames << url.queryItems().at( i ).first.toUpper();
  }
  if ( !qnames.contains( QStringLiteral( "SERVICE" ) ) )
    setQueryItem( url, QStringLiteral( "SERVICE" ), QStringLiteral( "WMS" ) );
  if ( !qnames.contains( QStringLiteral( "VERSION" ) ) )
    setQueryItem( url, QStringLiteral( "VERSION" ), mCaps.mCapabilities.version );
  if ( !qnames.contains( QStringLiteral( "SLD_VERSION" ) ) )
    setQueryItem( url, QStringLiteral( "SLD_VERSION" ), QStringLiteral( "1.1.0" ) ); // can not determine SLD_VERSION
  if ( !qnames.contains( QStringLiteral( "REQUEST" ) ) )
    setQueryItem( url, QStringLiteral( "REQUEST" ), QStringLiteral( "GetLegendGraphic" ) );
  if ( !qnames.contains( QStringLiteral( "FORMAT" ) ) )
    setFormatQueryItem( url );
  if ( !qnames.contains( QStringLiteral( "LAYER" ) ) )
    setQueryItem( url, QStringLiteral( "LAYER" ), mSettings.mActiveSubLayers[0] );
  if ( !qnames.contains( QStringLiteral( "STYLE" ) ) )
    setQueryItem( url, QStringLiteral( "STYLE" ), mSettings.mActiveSubStyles[0] );
  // by setting TRANSPARENT=true, even too big legend images will look good
  if ( !qnames.contains( QStringLiteral( "TRANSPARENT" ) ) )
    setQueryItem( url, QStringLiteral( "TRANSPARENT" ), QStringLiteral( "true" ) );

  // add config parameter related to resolution
  QgsSettings s;
  int defaultLegendGraphicResolution = s.value( QStringLiteral( "qgis/defaultLegendGraphicResolution" ), 0 ).toInt();
  QgsDebugMsg( QString( "defaultLegendGraphicResolution: %1" ).arg( defaultLegendGraphicResolution ) );
  if ( defaultLegendGraphicResolution )
  {
    if ( mSettings.mDpiMode & DpiQGIS )
      setQueryItem( url, QStringLiteral( "DPI" ), QString::number( defaultLegendGraphicResolution ) );
    if ( mSettings.mDpiMode & DpiUMN )
    {
      setQueryItem( url, QStringLiteral( "MAP_RESOLUTION" ), QString::number( defaultLegendGraphicResolution ) );
      setQueryItem( url, QStringLiteral( "SCALE" ), QString::number( scale, 'f' ) );
    }
    if ( mSettings.mDpiMode & DpiGeoServer )
    {
      setQueryItem( url, QStringLiteral( "FORMAT_OPTIONS" ), QStringLiteral( "dpi:%1" ).arg( defaultLegendGraphicResolution ) );
      setQueryItem( url, QStringLiteral( "SCALE" ), QString::number( scale, 'f' ) );
    }
  }

  if ( useContextualWMSLegend )
  {
    bool changeXY = mCaps.shouldInvertAxisOrientation( mImageCrs );
    setQueryItem( url, QStringLiteral( "BBOX" ), toParamValue( visibleExtent, changeXY ) );
    setSRSQueryItem( url );
  }

  QgsDebugMsg( QString( "getlegendgraphicrequest: %1" ).arg( url.toString() ) );
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
    QgsDebugMsg( "getLegendGraphic url is empty" );
    return QImage();
  }

  forceRefresh |= mGetLegendGraphicImage.isNull() || mGetLegendGraphicScale != scale;

  QgsRectangle mapExtent = visibleExtent ? *visibleExtent : extent();
  forceRefresh |= mGetLegendGraphicExtent != mapExtent;

  if ( !forceRefresh )
    return mGetLegendGraphicImage;

  mError = QLatin1String( "" );

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

  QgsDebugMsg( "exiting." );

  return mGetLegendGraphicImage;
}

QgsImageFetcher *QgsWmsProvider::getLegendGraphicFetcher( const QgsMapSettings *mapSettings )
{
  double scale;
  QgsRectangle mapExtent;
  if ( mapSettings && mSettings.mEnableContextualLegend )
  {
    scale = mapSettings->scale();
    mapExtent = mapSettings->visibleExtent();
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
    QgsDebugMsg( "Emitting cached image fetcher" );
    // return a cached image, skipping the load
    return new QgsCachedImageFetcher( mGetLegendGraphicImage );
  }
  else
  {
    QgsImageFetcher *fetcher =  new QgsWmsLegendDownloadHandler( *QgsNetworkAccessManager::instance(), mSettings, url );
    fetcher->setProperty( "legendScale", QVariant::fromValue( scale ) );
    fetcher->setProperty( "legendExtent", QVariant::fromValue( mapExtent.toRectF() ) );
    connect( fetcher, &QgsImageFetcher::finish, this, &QgsWmsProvider::getLegendGraphicReplyFinished );
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

#ifdef QGISDEBUG
    QString filename = QDir::tempPath() + "/GetLegendGraphic.png";
    mGetLegendGraphicImage.save( filename );
    QgsDebugMsg( "saved GetLegendGraphic result in debug file: " + filename );
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
  Q_UNUSED( message );
  QgsDebugMsg( QString( "get legend failed: %1" ).arg( message ) );

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
  QgsDebugMsg( msg );
  emit statusChanged( msg );
}



/**
 * Class factory to return a pointer to a newly created
 * QgsWmsProvider object
 */
QGISEXTERN QgsWmsProvider *classFactory( const QString *uri )
{
  return new QgsWmsProvider( *uri );
}

/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return WMS_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return WMS_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}


// -----------------

QgsWmsImageDownloadHandler::QgsWmsImageDownloadHandler( const QString &providerUri, const QUrl &url, const QgsWmsAuthorization &auth, QImage *image, QgsRasterBlockFeedback *feedback )
  : mProviderUri( providerUri )
  , mCacheReply( nullptr )
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

      QgsDebugMsg( QString( "redirected getmap: %1" ).arg( redirect.toString() ) );
      mCacheReply = QgsNetworkAccessManager::instance()->get( QNetworkRequest( redirect.toUrl() ) );
      connect( mCacheReply, &QNetworkReply::finished, this, &QgsWmsImageDownloadHandler::cacheReplyFinished );
      return;
    }

    QVariant status = mCacheReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
    {
      QVariant phrase = mCacheReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );

      QgsMessageLog::logMessage( tr( "Map request error (Status: %1; Reason phrase: %2; URL:%3)" )
                                 .arg( status.toInt() )
                                 .arg( phrase.toString(),
                                       mCacheReply->url().toString() ), tr( "WMS" ) );

      mCacheReply->deleteLater();
      mCacheReply = nullptr;

      finish();
      return;
    }

    QString contentType = mCacheReply->header( QNetworkRequest::ContentTypeHeader ).toString();
    QgsDebugMsg( "contentType: " + contentType );
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
      QgsMessageLog::logMessage( tr( "Returned image is flawed [Content-Type:%1; URL:%2]" )
                                 .arg( contentType, mCacheReply->url().toString() ), tr( "WMS" ) );
    }
    else
    {
      QString errorTitle, errorText;
      if ( contentType.toLower() == QLatin1String( "text/xml" ) && QgsWmsProvider::parseServiceExceptionReportDom( text, errorTitle, errorText ) )
      {
        QgsMessageLog::logMessage( tr( "Map request error (Title:%1; Error:%2; URL: %3)" )
                                   .arg( errorTitle, errorText,
                                         mCacheReply->url().toString() ), tr( "WMS" ) );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Map request error (Status: %1; Response: %2; Content-Type: %3; URL:%4)" )
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
        QgsMessageLog::logMessage( tr( "Map request failed [error:%1 url:%2]" ).arg( mCacheReply->errorString(), mCacheReply->url().toString() ), tr( "WMS" ) );
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
  Q_UNUSED( bytesReceived );
  Q_UNUSED( bytesTotal );
  QgsDebugMsg( tr( "%1 of %2 bytes of map downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) ) );
}

void QgsWmsImageDownloadHandler::canceled()
{
  QgsDebugMsg( "Caught canceled() signal" );
  if ( mCacheReply )
  {
    // abort the reply if it is still active
    QgsDebugMsg( "Aborting WMS network request" );
    mCacheReply->abort();
  }
}


// ----------


QgsWmsTiledImageDownloadHandler::QgsWmsTiledImageDownloadHandler( const QString &providerUri, const QgsWmsAuthorization &auth, int tileReqNo, const QgsWmsProvider::TileRequests &requests, QImage *image, const QgsRectangle &viewExtent, bool smoothPixmapTransform, QgsRasterBlockFeedback *feedback )
  : mProviderUri( providerUri )
  , mAuth( auth )
  , mImage( image )
  , mViewExtent( viewExtent )
  , mEventLoop( new QEventLoop )
  , mTileReqNo( tileReqNo )
  , mSmoothPixmapTransform( smoothPixmapTransform )
  , mFeedback( feedback )
{
  if ( feedback )
  {
    connect( feedback, &QgsFeedback::canceled, this, &QgsWmsTiledImageDownloadHandler::canceled, Qt::QueuedConnection );

    // rendering could have been canceled before we started to listen to canceled() signal
    // so let's check before doing the download and maybe quit prematurely
    if ( feedback->isCanceled() )
      return;
  }

  Q_FOREACH ( const QgsWmsProvider::TileRequest &r, requests )
  {
    QNetworkRequest request( r.url );
    auth.setAuthorization( request );
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ), mTileReqNo );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileIndex ), r.index );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRect ), r.rect );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), 0 );

    QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( request );
    connect( reply, &QNetworkReply::finished, this, &QgsWmsTiledImageDownloadHandler::tileReplyFinished );

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
  QgsDebugMsgLevel( "raw headers:", 3 );
  Q_FOREACH ( const QNetworkReply::RawHeaderPair &pair, reply->rawHeaderPairs() )
  {
    QgsDebugMsgLevel( QString( " %1:%2" )
                      .arg( QString::fromUtf8( pair.first ),
                            QString::fromUtf8( pair.second ) ), 3 );
  }
#endif

  if ( QgsNetworkAccessManager::instance()->cache() )
  {
    QNetworkCacheMetaData cmd = QgsNetworkAccessManager::instance()->cache()->metaData( reply->request().url() );

    QNetworkCacheMetaData::RawHeaderList hl;
    Q_FOREACH ( const QNetworkCacheMetaData::RawHeader &h, cmd.rawHeaders() )
    {
      if ( h.first != "Cache-Control" )
        hl.append( h );
    }
    cmd.setRawHeaders( hl );

    QgsDebugMsg( QString( "expirationDate:%1" ).arg( cmd.expirationDate().toString() ) );
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

  QgsDebugMsg( QString( "tile reply %1 (%2) tile:%3(retry %4) rect:%5,%6 %7,%8) fromcache:%9 error:%10 url:%11" )
               .arg( tileReqNo ).arg( mTileReqNo ).arg( tileNo ).arg( retry )
               .arg( r.left(), 0, 'f' ).arg( r.bottom(), 0, 'f' ).arg( r.right(), 0, 'f' ).arg( r.top(), 0, 'f' )
               .arg( fromCache )
               .arg( reply->errorString(),
                     reply->url().toString() )
             );

  if ( reply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      QNetworkRequest request( redirect.toUrl() );
      mAuth.setAuthorization( request );
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
      request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ), tileReqNo );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileIndex ), tileNo );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRect ), r );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), 0 );

      mReplies.removeOne( reply );
      reply->deleteLater();

      QgsDebugMsg( QString( "redirected gettile: %1" ).arg( redirect.toString() ) );
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
    QgsDebugMsg( "contentType: " + contentType );
    if ( !contentType.startsWith( QLatin1String( "image/" ), Qt::CaseInsensitive ) &&
         contentType.compare( QLatin1String( "application/octet-stream" ), Qt::CaseInsensitive ) != 0 )
    {
      QByteArray text = reply->readAll();
      QString errorTitle, errorText;
      if ( contentType.toLower() == QLatin1String( "text/xml" ) && QgsWmsProvider::parseServiceExceptionReportDom( text, errorTitle, errorText ) )
      {
        QgsMessageLog::logMessage( tr( "Tile request error (Title:%1; Error:%2; URL: %3)" )
                                   .arg( errorTitle, errorText,
                                         reply->url().toString() ), tr( "WMS" ) );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Tile request error (Status:%1; Content-Type:%2; Length:%3; URL: %4)" )
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
      double cr = mViewExtent.width() / mImage->width();

      QRectF dst( ( r.left() - mViewExtent.xMinimum() ) / cr,
                  ( mViewExtent.yMaximum() - r.bottom() ) / cr,
                  r.width() / cr,
                  r.height() / cr );

      QgsDebugMsg( QString( "tile reply: length %1" ).arg( reply->bytesAvailable() ) );

      QImage myLocalImage = QImage::fromData( reply->readAll() );

      if ( !myLocalImage.isNull() )
      {
        QPainter p( mImage );
        if ( mSmoothPixmapTransform )
          p.setRenderHint( QPainter::SmoothPixmapTransform, true );
        p.drawImage( dst, myLocalImage );
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
        QgsMessageLog::logMessage( tr( "Returned image is flawed [Content-Type:%1; URL: %2]" )
                                   .arg( contentType, reply->url().toString() ), tr( "WMS" ) );
      }
    }
    else
    {
      QgsDebugMsg( QString( "Reply too late [%1]" ).arg( reply->url().toString() ) );
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
        if ( reply->error() == QNetworkReply::TimeoutError )
          repeatTileRequest( reply->request() );
      }
    }

    mReplies.removeOne( reply );
    reply->deleteLater();

    if ( mReplies.isEmpty() )
      finish();
  }

#if 0
  const QgsWmsStatistics::Stat &stat = QgsWmsStatistics::statForUri( mProviderUri );
  emit statusChanged( tr( "%n tile requests in background", "tile request count", mReplies.count() )
                      + tr( ", %n cache hits", "tile cache hits", stat.cacheHits )
                      + tr( ", %n cache misses.", "tile cache missed", stat.cacheMisses )
                      + tr( ", %n errors.", "errors", stat.errors )
                    );
#endif
}

void QgsWmsTiledImageDownloadHandler::canceled()
{
  QgsDebugMsg( "Caught canceled() signal" );
  Q_FOREACH ( QNetworkReply *reply, mReplies )
  {
    QgsDebugMsg( "Aborting tiled network request" );
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
                               .arg( tileReqNo ).arg( tileNo ).arg( retry ), tr( "WMS" ), QgsMessageLog::INFO );
  }
  QgsDebugMsg( QString( "repeat tileRequest %1 %2(retry %3) for url: %4" ).arg( tileReqNo ).arg( tileNo ).arg( retry ).arg( url ) );
  request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), retry );

  QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( request );
  mReplies << reply;
  connect( reply, &QNetworkReply::finished, this, &QgsWmsTiledImageDownloadHandler::tileReplyFinished );
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

void QgsWmsProvider::setSRSQueryItem( QUrl &url )
{
  QString crsKey = QStringLiteral( "SRS" ); //SRS in 1.1.1 and CRS in 1.3.0
  if ( mCaps.mCapabilities.version == QLatin1String( "1.3.0" ) || mCaps.mCapabilities.version == QLatin1String( "1.3" ) )
  {
    crsKey = QStringLiteral( "CRS" );
  }
  setQueryItem( url, crsKey, mImageCrs );
}

// ----------

QgsWmsLegendDownloadHandler::QgsWmsLegendDownloadHandler( QgsNetworkAccessManager &networkAccessManager, const QgsWmsSettings &settings, const QUrl &url )
  : mNetworkAccessManager( networkAccessManager )
  , mSettings( settings )
  , mReply( nullptr )
  , mInitialUrl( url )
{
}

QgsWmsLegendDownloadHandler::~QgsWmsLegendDownloadHandler()
{
  if ( mReply )
  {
    // Send finished if not done yet ?
    QgsDebugMsg( "WMSLegendDownloader destroyed while still processing reply" );
    mReply->deleteLater();
  }
  mReply = nullptr;
}

/* public */
void
QgsWmsLegendDownloadHandler::start()
{
  Q_ASSERT( mVisitedUrls.empty() );
  startUrl( mInitialUrl );
}

/* private */
void
QgsWmsLegendDownloadHandler::startUrl( const QUrl &url )
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

  QgsDebugMsg( QString( "legend url: %1" ).arg( url.toString() ) );

  QNetworkRequest request( url );
  mSettings.authorization().setAuthorization( request );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  mReply = mNetworkAccessManager.get( request );
  mSettings.authorization().setAuthorizationReply( mReply );
  connect( mReply, static_cast < void ( QNetworkReply::* )( QNetworkReply::NetworkError ) >( &QNetworkReply::error ), this, &QgsWmsLegendDownloadHandler::errored );
  connect( mReply, &QNetworkReply::finished, this, &QgsWmsLegendDownloadHandler::finished );
  connect( mReply, &QNetworkReply::downloadProgress, this, &QgsWmsLegendDownloadHandler::progressed );
}

void
QgsWmsLegendDownloadHandler::sendError( const QString &msg )
{
  QgsDebugMsg( QString( "emitting error: %1" ).arg( msg ) );
  Q_ASSERT( mReply );
  mReply->deleteLater();
  mReply = nullptr;
  emit error( msg );
}

void
QgsWmsLegendDownloadHandler::sendSuccess( const QImage &img )
{
  QgsDebugMsg( QString( "emitting finish: %1x%2 image" ).arg( img.width() ).arg( img.height() ) );
  Q_ASSERT( mReply );
  mReply->deleteLater();
  mReply = nullptr;
  emit finish( img );
}

void
QgsWmsLegendDownloadHandler::errored( QNetworkReply::NetworkError /* code */ )
{
  if ( !mReply )
    return;

  sendError( mReply->errorString() );
}

void
QgsWmsLegendDownloadHandler::finished()
{
  if ( !mReply )
    return;

  // or ::errored() should have been called before ::finished
  Q_ASSERT( mReply->error() == QNetworkReply::NoError );

  QgsDebugMsg( "reply OK" );
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
    msg += QStringLiteral( " - " );
    msg += QString( tr( "Status: %1\nReason phrase: %2" ) ).arg( status.toInt() ).arg( phrase.toString() );
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

void
QgsWmsLegendDownloadHandler::progressed( qint64 recv, qint64 tot )
{
  emit progress( recv, tot );
}

//------

QgsCachedImageFetcher::QgsCachedImageFetcher( const QImage &img )
  : _img( img )
{
}

QgsCachedImageFetcher::~QgsCachedImageFetcher()
{
}
void
QgsCachedImageFetcher::start()
{
  QTimer::singleShot( 1, this, SLOT( send() ) );
}


#ifdef HAVE_GUI

//! Provider for WMS layers source select
class QgsWmsSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    virtual QString providerKey() const override { return QStringLiteral( "wms" ); }
    virtual QString text() const override { return QObject::tr( "WMS" ); }
    virtual int ordering() const override { return QgsSourceSelectProvider::OrderRemoteProvider + 10; }
    virtual QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddWmsLayer.svg" ) ); }
    virtual QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsWMSSourceSelect( parent, fl, widgetMode );
    }
};


QGISEXTERN QList<QgsSourceSelectProvider *> *sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> *providers = new QList<QgsSourceSelectProvider *>();

  *providers
      << new QgsWmsSourceSelectProvider;

  return providers;
}
#endif

