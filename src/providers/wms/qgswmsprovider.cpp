/***************************************************************************
  qgswmsprovider.cpp  -  QGIS Data provider for
                         OGC Web Map Service layers
                             -------------------
    begin                : 17 Mar, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au

    wms-c support        : Jürgen E. Fischer < jef at norbit dot de >, norBIT GmbH

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <typeinfo>

#define WMS_THRESHOLD 200  // time to wait for an answer without emitting dataChanged() 

#include "qgslogger.h"
#include "qgswmsprovider.h"
#include "qgswmsconnection.h"

#include <cmath>

#include "qgscoordinatetransform.h"
#include "qgsrasterlayer.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsnetworkaccessmanager.h"
#include <qgsmessageoutput.h>
#include <qgsmessagelog.h>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>

#if QT_VERSION >= 0x40500
#include <QNetworkDiskCache>
#endif

#include <QUrl>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QPixmap>
#include <QSet>
#include <QSettings>
#include <QEventLoop>
#include <QCoreApplication>
#include <QTime>

#ifdef _MSC_VER
#include <float.h>
#define isfinite(x) _finite(x)
#endif

#ifdef QGISDEBUG
#include <QFile>
#include <QDir>
#endif

static QString WMS_KEY = "wms";
static QString WMS_DESCRIPTION = "OGC Web Map Service version 1.3 data provider";

static QString DEFAULT_LATLON_CRS = "CRS:84";


QgsWmsProvider::QgsWmsProvider( QString const &uri )
    : QgsRasterDataProvider( uri )
    , httpuri( uri )
    , httpcapabilitiesresponse( 0 )
    , imageCrs( DEFAULT_LATLON_CRS )
    , cachedImage( 0 )
    , cacheReply( 0 )
    , cachedViewExtent( 0 )
    , mCoordinateTransform( 0 )
    , extentDirty( true )
    , mGetFeatureInfoUrlBase( "" )
    , mLayerCount( -1 )
    , mTileReqNo( 0 )
    , mCacheHits( 0 )
    , mCacheMisses( 0 )
    , mErrors( 0 )
    , mUserName( QString::null )
    , mPassword( QString::null )
    , mFeatureCount( 0 )
{
  // URL may contain username/password information for a WMS
  // requiring authentication. In this case the URL is prefixed
  // with username=user,password=pass,url=http://xxx.xxx.xx/yyy...
  parseUri( uri );

  QgsDebugMsg( "constructing with uri '" + httpuri + "'." );

  // assume this is a valid layer until we determine otherwise
  valid = true;


  // URL can be in 3 forms:
  // 1) http://xxx.xxx.xx/yyy/yyy
  // 2) http://xxx.xxx.xx/yyy/yyy?
  // 3) http://xxx.xxx.xx/yyy/yyy?zzz=www

  mBaseUrl = prepareUri( httpuri );

  mSupportedGetFeatureFormats = QStringList() << "text/html" << "text/plain" << "text/xml";

  QgsDebugMsg( "mBaseUrl = " + mBaseUrl );

  QgsDebugMsg( "exiting constructor." );
}

void QgsWmsProvider::parseUri( QString uri )
{
  // Strip off and store the user name and password (if they exist)
  if ( !uri.startsWith( " http:" ) )
  {
    mTiled = false;
    mTileWidth = 0;
    mTileHeight = 0;
    mResolutions.clear();

    mIgnoreGetMapUrl = false;
    mIgnoreGetFeatureInfoUrl = false;

    // uri potentially contains username and password
    QStringList parts = uri.split( "," );
    QStringListIterator iter( parts );
    while ( iter.hasNext() )
    {
      QString item = iter.next();
      QgsDebugMsg( "testing for creds: " + item );
      if ( item.startsWith( "username=" ) )
      {
        mUserName = item.mid( 9 );
        QgsDebugMsg( "set username to " + mUserName );
      }
      else if ( item.startsWith( "password=" ) )
      {
        mPassword = item.mid( 9 );
        QgsDebugMsg( "set password to " + mPassword );
      }
      else if ( item.startsWith( "tiled=" ) )
      {
        QStringList params = item.mid( 6 ).split( ";" );

        mTiled = true;
        mTileWidth = params.takeFirst().toInt();
        mTileHeight = params.takeFirst().toInt();

        mResolutions.clear();
        foreach( QString r, params )
        {
          mResolutions << r.toDouble();
        }
        qSort( mResolutions );
      }
      else if ( item.startsWith( "featureCount=" ) )
      {
        mFeatureCount = item.mid( 13 ).toInt();
      }
      else if ( item.startsWith( "url=" ) )
      {
        // strip the authentication information from the front of the uri
        httpuri = item.mid( 4 );
        QgsDebugMsg( "set httpuri to " + httpuri );
      }
      else if ( item.startsWith( "ignoreUrl=" ) )
      {
        foreach( QString param, item.mid( 10 ).split( ";" ) )
        {
          if ( param == "GetMap" )
          {
            mIgnoreGetMapUrl = true;
          }
          else if ( param == "GetFeatureInfo" )
          {
            mIgnoreGetFeatureInfoUrl = true;
          }
        }
      }
    }
  }
}

QString QgsWmsProvider::prepareUri( QString uri ) const
{
  if ( !uri.contains( "?" ) )
  {
    uri.append( "?" );
  }
  else if ( uri.right( 1 ) != "?" && uri.right( 1 ) != "&" )
  {
    uri.append( "&" );
  }

  return uri;
}

QgsWmsProvider::~QgsWmsProvider()
{
  QgsDebugMsg( "deconstructing." );

  // Dispose of any cached image as created by draw()
  if ( cachedImage )
  {
    delete cachedImage;
  }

  if ( mCoordinateTransform )
  {
    delete mCoordinateTransform;
  }

  if ( cacheReply )
  {
    cacheReply->deleteLater();
    cacheReply = 0;
  }

  while ( !tileReplies.isEmpty() )
  {
    tileReplies.takeFirst()->deleteLater();
  }
}


bool QgsWmsProvider::supportedLayers( QVector<QgsWmsLayerProperty> &layers )
{
  QgsDebugMsg( "Entering." );

  // Allow the provider to collect the capabilities first.
  if ( !retrieveServerCapabilities() )
  {
    return false;
  }

  layers = layersSupported;

  QgsDebugMsg( "Exiting." );

  return true;
}

bool QgsWmsProvider::supportedTileSets( QVector<QgsWmsTileSetProfile> &tilesets )
{
  QgsDebugMsg( "Entering." );

  // Allow the provider to collect the capabilities first.
  if ( !retrieveServerCapabilities() )
  {
    return false;
  }

  tilesets = tilesetsSupported;

  QgsDebugMsg( "Exiting." );

  return true;
}

size_t QgsWmsProvider::layerCount() const
{
  return 1;                   // XXX properly return actual number of layers
} // QgsWmsProvider::layerCount()

bool QgsWmsProvider::hasTiles() const
{
  return mCapabilities.capability.tileSetProfiles.size() > 0;
}

QString QgsWmsProvider::baseUrl() const
{
  return mBaseUrl;
}

QString QgsWmsProvider::getMapUrl() const
{
  return mCapabilities.capability.request.getMap.dcpType.size() == 0
         ? mBaseUrl
         : prepareUri( mCapabilities.capability.request.getMap.dcpType.front().http.get.onlineResource.xlinkHref );
}

QString QgsWmsProvider::getFeatureInfoUrl() const
{
  return mCapabilities.capability.request.getFeatureInfo.dcpType.size() == 0
         ? mBaseUrl
         : prepareUri( mCapabilities.capability.request.getFeatureInfo.dcpType.front().http.get.onlineResource.xlinkHref );
}

void QgsWmsProvider::addLayers( QStringList const &layers,
                                QStringList const &styles )
{
  QgsDebugMsg( "Entering with layer list of " + layers.join( ", " )
               + " and style list of " + styles.join( ", " ) );

  if ( layers.size() != styles.size() )
  {
    QgsMessageLog::logMessage( tr( "number of layers and styles don't match" ), tr( "WMS" ) );
    valid = false;
    return;
  }

  // TODO: Make activeSubLayers a std::map in order to avoid duplicates
  activeSubLayers += layers;
  activeSubStyles += styles;

  // Set the visibility of these new layers on by default
  for ( QStringList::const_iterator it  = layers.begin();
        it != layers.end();
        ++it )
  {
    activeSubLayerVisibility[*it] = true;

    QgsDebugMsg( "set visibility of layer '" + ( *it ) + "' to true." );
  }

  // now that the layers have changed, the extent will as well.
  extentDirty = true;

  QgsDebugMsg( "Exiting." );
}

void QgsWmsProvider::setConnectionName( QString const &connName )
{
  connectionName = connName;
}

void QgsWmsProvider::setLayerOrder( QStringList const &layers )
{
  QgsDebugMsg( "Entering." );

  activeSubLayers = layers;

  QgsDebugMsg( "Exiting." );
}


void QgsWmsProvider::setSubLayerVisibility( QString const & name, bool vis )
{
  activeSubLayerVisibility[name] = vis;
}


QString QgsWmsProvider::imageEncoding() const
{
  return imageMimeType;
}


void QgsWmsProvider::setImageEncoding( QString const & mimeType )
{
  QgsDebugMsg( "Setting image encoding to " + mimeType + "." );
  imageMimeType = mimeType;
}


void QgsWmsProvider::setImageCrs( QString const & crs )
{
  QgsDebugMsg( "Setting image CRS to " + crs + "." );

  if ( crs != imageCrs && !crs.isEmpty() )
  {
    // delete old coordinate transform as it is no longer valid
    if ( mCoordinateTransform )
    {
      delete mCoordinateTransform;
    }

    extentDirty = true;

    imageCrs = crs;
  }
}

void QgsWmsProvider::setQueryItem( QUrl &url, QString item, QString value )
{
  url.removeQueryItem( item );
  url.addQueryItem( item, value );
}

QImage *QgsWmsProvider::draw( QgsRectangle  const &viewExtent, int pixelWidth, int pixelHeight )
{
  QgsDebugMsg( "Entering." );

  // Can we reuse the previously cached image?
  if ( cachedImage &&
       cachedViewExtent == viewExtent &&
       cachedViewWidth == pixelWidth &&
       cachedViewHeight == pixelHeight )
  {
    return cachedImage;
  }

  // delete cached image and create network request(s) to fill it
  if ( cachedImage )
  {
    delete cachedImage;
    cachedImage = 0;
  }

  // abort running (untiled) request
  if ( cacheReply )
  {
    cacheReply->abort();
    delete cacheReply;
    cacheReply = 0;
  }

  //according to the WMS spec for 1.3, the order of x - and y - coordinates is inverted for geographical CRS
  bool changeXY = false;
  if ( mCapabilities.version == "1.3.0" || mCapabilities.version == "1.3" )
  {
    //create CRS from string
    QgsCoordinateReferenceSystem theSrs;
    if ( theSrs.createFromOgcWmsCrs( imageCrs ) && theSrs.geographicFlag() )
    {
      changeXY = true;
    }
  }

  // compose the URL query string for the WMS server.
  QString crsKey = "SRS"; //SRS in 1.1.1 and CRS in 1.3.0
  if ( mCapabilities.version == "1.3.0" || mCapabilities.version == "1.3" )
  {
    crsKey = "CRS";
  }

  // Bounding box in WMS format (Warning: does not work with scientific notation)
  QString bbox = QString( changeXY ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
                 .arg( viewExtent.xMinimum(), 0, 'f' )
                 .arg( viewExtent.yMinimum(), 0, 'f' )
                 .arg( viewExtent.xMaximum(), 0, 'f' )
                 .arg( viewExtent.yMaximum(), 0, 'f' );

  cachedImage = new QImage( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
  cachedImage->fill( 0 );
  cachedViewExtent = viewExtent;
  cachedViewWidth = pixelWidth;
  cachedViewHeight = pixelHeight;

  QSettings s;
  bool bkLayerCaching = s.value( "/qgis/enable_render_caching", false ).toBool();

  if ( !mTiled )
  {
    // Calculate active layers that are also visible.

    QgsDebugMsg( "Active layer list of "  + activeSubLayers.join( ", " )
                 + " and style list of "  + activeSubStyles.join( ", " ) );

    QStringList visibleLayers = QStringList();
    QStringList visibleStyles = QStringList();

    QStringList::Iterator it2  = activeSubStyles.begin();

    for ( QStringList::Iterator it  = activeSubLayers.begin();
          it != activeSubLayers.end();
          ++it )
    {
      if ( activeSubLayerVisibility.find( *it ).value() )
      {
        visibleLayers += *it;
        visibleStyles += *it2;
      }

      ++it2;
    }

    QString layers = visibleLayers.join( "," );
    QString styles = visibleStyles.join( "," );

    QgsDebugMsg( "Visible layer list of " + layers + " and style list of " + styles );

    QUrl url( mIgnoreGetMapUrl ? mBaseUrl : getMapUrl() );
    setQueryItem( url, "SERVICE", "WMS" );
    setQueryItem( url, "VERSION", mCapabilities.version );
    setQueryItem( url, "REQUEST", "GetMap" );
    setQueryItem( url, "BBOX", bbox );
    setQueryItem( url, crsKey, imageCrs );
    setQueryItem( url, "WIDTH", QString::number( pixelWidth ) );
    setQueryItem( url, "HEIGHT", QString::number( pixelHeight ) );
    setQueryItem( url, "LAYERS", layers );
    setQueryItem( url, "STYLES", styles );
    setQueryItem( url, "FORMAT", imageMimeType );

    //DPI parameter is accepted by QGIS mapserver (and ignored by the other WMS servers)
    if ( mDpi != -1 )
    {
      setQueryItem( url, "DPI", QString::number( mDpi ) );
    }

    //MH: jpeg does not support transparency and some servers complain if jpg and transparent=true
    if ( !imageMimeType.contains( "jpeg", Qt::CaseInsensitive ) && !imageMimeType.contains( "jpg", Qt::CaseInsensitive ) )
    {
      setQueryItem( url, "TRANSPARENT", "TRUE" );  // some servers giving error for 'true' (lowercase)
    }

    QgsDebugMsg( QString( "getmap: %1" ).arg( url.toString() ) );

    // cache some details for if the user wants to do an identifyAsHtml() later

    mGetFeatureInfoUrlBase = mIgnoreGetFeatureInfoUrl ? mBaseUrl : getFeatureInfoUrl();

    QNetworkRequest request( url );
    setAuthorization( request );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
    cacheReply = QgsNetworkAccessManager::instance()->get( request );
    connect( cacheReply, SIGNAL( finished() ), this, SLOT( cacheReplyFinished() ) );
    connect( cacheReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( cacheReplyProgress( qint64, qint64 ) ) );

    emit statusChanged( tr( "Getting map via WMS." ) );

    mWaiting = true;

    QTime t;
    t.start();

    while ( cacheReply && ( !bkLayerCaching || t.elapsed() < WMS_THRESHOLD ) )
    {
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, WMS_THRESHOLD );
    }

    mWaiting = false;
  }
  else
  {
    mTileReqNo++;

    double vres = viewExtent.width() / pixelWidth;

    double tres = vres;
    int i;
    if ( mResolutions.size() > 0 )
    {

      // find nearest resolution
      for ( i = 0; i < mResolutions.size() && mResolutions[i] < vres; i++ )
        QgsDebugMsg( QString( "skipped res: %1:%2" ).arg( i ).arg( mResolutions[i] ) );

      if ( i == mResolutions.size() ||
           ( i > 0 && vres - mResolutions[i-1] < mResolutions[i] - vres ) )
      {
        QgsDebugMsg( "back to previous res" );
        i--;
      }

      tres = mResolutions[i];
    }

    // clip view extent to layer extent
    double xmin = qMax( viewExtent.xMinimum(), layerExtent.xMinimum() );
    double ymin = qMax( viewExtent.yMinimum(), layerExtent.yMinimum() );
    double xmax = qMin( viewExtent.xMaximum(), layerExtent.xMaximum() );
    double ymax = qMin( viewExtent.yMaximum(), layerExtent.yMaximum() );

    // snap to tile coordinates
    double x0 = floor(( xmin - layerExtent.xMinimum() ) / mTileWidth / tres ) * mTileWidth * tres + layerExtent.xMinimum() + mTileWidth * tres * 0.001;
    double y0 = floor(( ymin - layerExtent.yMinimum() ) / mTileHeight / tres ) * mTileHeight * tres + layerExtent.yMinimum() + mTileHeight * tres * 0.001;

#ifdef QGISDEBUG
    // calculate number of tiles
    int n = ceil(( xmax - xmin ) / mTileWidth / tres ) * ceil(( ymax - ymin ) / mTileHeight / tres );
#endif

    QgsDebugMsg( QString( "layer extent: %1,%2 %3x%4" )
                 .arg( layerExtent.xMinimum(), 0, 'f' )
                 .arg( layerExtent.yMinimum(), 0, 'f' )
                 .arg( layerExtent.width() )
                 .arg( layerExtent.height() )
               );
    QgsDebugMsg( QString( "view extent: %1,%2 %3x%4  res:%5" )
                 .arg( viewExtent.xMinimum(), 0, 'f' )
                 .arg( viewExtent.yMinimum(), 0, 'f' )
                 .arg( viewExtent.width() )
                 .arg( viewExtent.height() )
                 .arg( vres, 0, 'f' )
               );
    QgsDebugMsg( QString( "tile extent: %1,%2 %3x%4 pixel:%5x%6 res:%7" )
                 .arg( x0, 0, 'f' ).arg( y0, 0, 'f' )
                 .arg( mTileWidth * tres, 0, 'f' ).arg( mTileHeight * tres, 0, 'f' )
                 .arg( mTileWidth ).arg( mTileHeight )
                 .arg( tres, 0, 'f' )
               );
    QgsDebugMsg( QString( "tile number: %1x%2 = %3" )
                 .arg( ceil(( xmax - xmin ) / mTileWidth / tres ) )
                 .arg( ceil(( ymax - ymin ) / mTileHeight / tres ) )
                 .arg( n )
               );

#if 0
    if ( n > 100 )
    {
      emit statusChanged( QString( "current view would need %1 tiles. tile request per draw limited to 100." ).arg( n ) );
      return cachedImage;
    }
#endif

    // add WMS request
    QUrl url( mIgnoreGetMapUrl ? mBaseUrl : getMapUrl() );
    setQueryItem( url, "SERVICE", "WMS" );
    setQueryItem( url, "VERSION", mCapabilities.version );
    setQueryItem( url, "REQUEST", "GetMap" );

    url.removeQueryItem( crsKey );
    url.removeQueryItem( "WIDTH" );
    url.removeQueryItem( "HEIGHT" );
    url.removeQueryItem( "LAYERS" );
    url.removeQueryItem( "STYLES" );
    url.removeQueryItem( "FORMAT" );
    url.removeQueryItem( "TILED" );


    // compose static request arguments.
    QString urlargs;
    urlargs += QString( "&%1=%2" ).arg( crsKey ).arg( imageCrs );
    urlargs += QString( "&WIDTH=%1" ).arg( mTileWidth );
    urlargs += QString( "&HEIGHT=%1" ).arg( mTileHeight );
    urlargs += QString( "&LAYERS=%1" ).arg( activeSubLayers.join( "," ) );
    urlargs += QString( "&STYLES=%1" ).arg( activeSubStyles.join( "," ) );
    urlargs += QString( "&FORMAT=%1" ).arg( imageMimeType );
    urlargs += QString( "&TILED=true" );

    i = 0;
    int j = 0;
    double y = y0;
    while ( y < ymax )
    {
      int k = 0;
      double x = x0;
      while ( x < xmax )
      {
        QString turl;
        turl += url.toString();
        turl += QString( changeXY ? "&BBOX=%2,%1,%4,%3" : "&BBOX=%1,%2,%3,%4" )
                .arg( x, 0, 'f' )
                .arg( y, 0, 'f' )
                .arg( x + mTileWidth * tres, 0, 'f' )
                .arg( y + mTileHeight * tres, 0, 'f' );
        turl += urlargs;

        QNetworkRequest request( turl );
        setAuthorization( request );
        QgsDebugMsg( QString( "tileRequest %1 %2/%3: %4" ).arg( mTileReqNo ).arg( i++ ).arg( n ).arg( turl ) );
        request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
        request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
        request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 0 ), mTileReqNo );
        request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ), i );
        request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 2 ), QRectF( x, y, mTileWidth * tres, mTileHeight * tres ) );

        QgsDebugMsg( QString( "gettile: %1" ).arg( turl ) );
        QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( request );
        tileReplies << reply;
        connect( reply, SIGNAL( finished() ), this, SLOT( tileReplyFinished() ) );

        x = x0 + ++k * mTileWidth * tres;
      }
      y = y0 + ++j * mTileHeight * tres;
    }

    emit statusChanged( tr( "Getting tiles via WMS." ) );

    mWaiting = true;

    QTime t;
    t.start();

    // draw everything that is retrieved within a second
    // and the rest asynchronously
    while ( !tileReplies.isEmpty() && ( !bkLayerCaching || t.elapsed() < WMS_THRESHOLD ) )
    {
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, WMS_THRESHOLD );
    }

    mWaiting = false;

#ifdef QGISDEBUG
    emit statusChanged( tr( "%n tile requests in background", "tile request count", tileReplies.count() )
                        + tr( ", %n cache hits", "tile cache hits", mCacheHits )
                        + tr( ", %n cache misses.", "tile cache missed", mCacheMisses )
                        + tr( ", %n errors.", "errors", mErrors )
                      );
#endif
  }

  return cachedImage;
}

//void QgsWmsProvider::readBlock( int bandNo, QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight, QgsCoordinateReferenceSystem theSrcCRS, QgsCoordinateReferenceSystem theDestCRS, void *block )
void QgsWmsProvider::readBlock( int bandNo, QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight, void *block )
{
  Q_UNUSED( bandNo );
  QgsDebugMsg( "Entered" );
  // TODO: optimize to avoid writing to QImage
  QImage* image = draw( viewExtent, pixelWidth, pixelHeight );

  if ( ! image )   // should not happen
  {
    QgsMessageLog::logMessage( tr( "image is NULL" ), tr( "WMS" ) );
    return;
  }
  QgsDebugMsg( QString( "image height = %1 bytesPerLine = %2" ).arg( image->height() ) . arg( image->bytesPerLine() ) ) ;
  int myExpectedSize = pixelWidth * pixelHeight * 4;
  int myImageSize = image->height() *  image->bytesPerLine();
  if ( myExpectedSize != myImageSize )   // should not happen
  {
    QgsMessageLog::logMessage( tr( "unexpected image size" ), tr( "WMS" ) );
    return;
  }

  uchar * ptr = image->bits( ) ;
  memcpy( block, ptr, myExpectedSize );
  // do not delete the image, it is handled by draw()
  //delete image;
}

void QgsWmsProvider::tileReplyFinished()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply*>( sender() );

#if defined(QGISDEBUG) && (QT_VERSION >= 0x40500)
  bool fromCache = reply->attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool();
  if ( fromCache )
    mCacheHits++;
  else
    mCacheMisses++;
#endif
#if defined(QGISDEBUG) && (QT_VERSION >= 0x40700)
  QgsDebugMsgLevel( "raw headers:", 3 );
  foreach( const QNetworkReply::RawHeaderPair &pair, reply->rawHeaderPairs() )
  {
    QgsDebugMsgLevel( QString( " %1:%2" )
                      .arg( QString::fromUtf8( pair.first ) )
                      .arg( QString::fromUtf8( pair.second ) ), 3 );
  }
#endif
  int tileReqNo = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 0 ) ).toInt();
  int tileNo = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ) ).toInt();
  QRectF r = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 2 ) ).toRectF();

#if QT_VERSION >= 0x40500
  QgsDebugMsg( QString( "tile reply %1 (%2) tile:%3 rect:%4,%5 %6x%7) fromcache:%8 error:%9" )
               .arg( tileReqNo ).arg( mTileReqNo ).arg( tileNo )
               .arg( r.left(), 0, 'f' ).arg( r.bottom(), 0, 'f' ).arg( r.width(), 0, 'f' ).arg( r.height(), 0, 'f' )
               .arg( fromCache )
               .arg( reply->errorString() )
             );
#else
  QgsDebugMsg( QString( "tile reply %1 (%2) tile:%3 rect:%4,%5 %6x%7) error:%8" )
               .arg( tileReqNo ).arg( mTileReqNo ).arg( tileNo )
               .arg( r.left(), 0, 'f' ).arg( r.bottom(), 0, 'f' ).arg( r.width(), 0, 'f' ).arg( r.height(), 0, 'f' )
               .arg( reply->errorString() )
             );
#endif

  if ( reply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      QNetworkRequest request( redirect.toUrl() );
      setAuthorization( request );
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
      request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 0 ), tileReqNo );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ), tileNo );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 2 ), r );

      tileReplies.removeOne( reply );
      reply->deleteLater();

      QgsDebugMsg( QString( "redirected gettile: %1" ).arg( redirect.toString() ) );
      reply = QgsNetworkAccessManager::instance()->get( request );
      tileReplies << reply;

      connect( reply, SIGNAL( finished() ), this, SLOT( tileReplyFinished() ) );

      return;
    }

    QVariant status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
    {
      QVariant phrase = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );

      showMessageBox( tr( "Tile request error" ), tr( "Status: %1\nReason phrase: %2" ).arg( status.toInt() ).arg( phrase.toString() ) );

      tileReplies.removeOne( reply );
      reply->deleteLater();

      return;
    }

    QString contentType = reply->header( QNetworkRequest::ContentTypeHeader ).toString();
    QgsDebugMsg( "contentType: " + contentType );
    if ( !contentType.startsWith( "image/" ) )
    {
      QByteArray text = reply->readAll();
      if ( contentType == "text/xml" && parseServiceExceptionReportDom( text ) )
      {
        showMessageBox( mErrorCaption, mError );
      }
      else
      {
        showMessageBox( "Tile request error", tr( "response: %1" ).arg( QString::fromUtf8( text ) ) );
      }

      tileReplies.removeOne( reply );
      reply->deleteLater();

      return;
    }

    // only take results from current request number
    if ( mTileReqNo == tileReqNo )
    {
      double cr = cachedViewExtent.width() / cachedViewWidth;

      QRectF dst(( r.left() - cachedViewExtent.xMinimum() ) / cr,
                 ( cachedViewExtent.yMaximum() - r.bottom() ) / cr,
                 r.width() / cr,
                 r.height() / cr );

      QgsDebugMsg( QString( "tile reply: %1" ).arg( reply->bytesAvailable() ) );
      QImage myLocalImage = QImage::fromData( reply->readAll() );

      // myLocalImage.save( QString( "%1/%2-tile-%3.png" ).arg( QDir::tempPath() ).arg( mTileReqNo ).arg( tileNo ) );

      if ( !myLocalImage.isNull() )
      {
        QPainter p( cachedImage );
        p.drawImage( dst, myLocalImage );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Returned image is flawed [%1]" ).arg( reply->url().toString() ), tr( "WMS" ) );
      }

#if 0
      p.drawRect( dst ); // show tile bounds
      p.drawText( dst, Qt::AlignCenter, QString( "(%1)\n%2,%3\n%4,%5\n%6x%7" )
                  .arg( tileNo )
                  .arg( r.left() ).arg( r.bottom() )
                  .arg( r.right() ).arg( r.top() )
                  .arg( r.width() ).arg( r.height() ) );
#endif
    }

    tileReplies.removeOne( reply );
    reply->deleteLater();

    if ( !mWaiting )
    {
      QgsDebugMsg( "emit dataChanged()" );
      emit dataChanged();
    }
  }
  else
  {
    tileReplies.removeOne( reply );
    reply->deleteLater();
    mErrors++;
  }

#ifdef QGISDEBUG
  emit statusChanged( tr( "%n tile requests in background", "tile request count", tileReplies.count() )
                      + tr( ", %n cache hits", "tile cache hits", mCacheHits )
                      + tr( ", %n cache misses.", "tile cache missed", mCacheMisses )
                      + tr( ", %n errors.", "errors", mErrors )
                    );
#endif
}

void QgsWmsProvider::cacheReplyFinished()
{
  if ( cacheReply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = cacheReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      cacheReply->deleteLater();

      QgsDebugMsg( QString( "redirected getmap: %1" ).arg( redirect.toString() ) );
      cacheReply = QgsNetworkAccessManager::instance()->get( QNetworkRequest( redirect.toUrl() ) );
      connect( cacheReply, SIGNAL( finished() ), this, SLOT( cacheReplyFinished() ) );
      return;
    }

    QVariant status = cacheReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
    {
      QVariant phrase = cacheReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );

      showMessageBox( tr( "Map request error" ), tr( "Status: %1\nReason phrase: %2" ).arg( status.toInt() ).arg( phrase.toString() ) );

      cacheReply->deleteLater();
      cacheReply = 0;

      return;
    }

    QString contentType = cacheReply->header( QNetworkRequest::ContentTypeHeader ).toString();
    QgsDebugMsg( "contentType: " + contentType );
    if ( contentType.startsWith( "image/" ) )
    {
      QImage myLocalImage = QImage::fromData( cacheReply->readAll() );
      if ( !myLocalImage.isNull() )
      {
        QPainter p( cachedImage );
        p.drawImage( 0, 0, myLocalImage );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Returned image is flawed [%1]" ).arg( cacheReply->url().toString() ), tr( "WMS" ) );
      }
    }
    else
    {
      QByteArray text = cacheReply->readAll();
      if ( contentType == "text/xml" && parseServiceExceptionReportDom( text ) )
      {
        showMessageBox( mErrorCaption, mError );
      }
      else
      {
        showMessageBox( tr( "Map request error" ), tr( "Response: %1" ).arg( QString::fromUtf8( text ) ) );
      }

      cacheReply->deleteLater();
      cacheReply = 0;

      return;
    }

    cacheReply->deleteLater();
    cacheReply = 0;

    if ( !mWaiting )
    {
      QgsDebugMsg( "emit dataChanged()" );
      emit dataChanged();
    }
  }
  else
  {
    cacheReply->deleteLater();
    cacheReply = 0;
    mErrors++;
  }
}

bool QgsWmsProvider::retrieveServerCapabilities( bool forceRefresh )
{
  QgsDebugMsg( "entering." );

  if ( httpcapabilitiesresponse.isNull() || forceRefresh )
  {
    QString url = mBaseUrl + "SERVICE=WMS&REQUEST=GetCapabilities";

    mError = "";

    QNetworkRequest request( url );
    setAuthorization( request );
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

    QgsDebugMsg( QString( "getcapabilities: %1" ).arg( url ) );
    mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );

    connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( capabilitiesReplyFinished() ) );
    connect( mCapabilitiesReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( capabilitiesReplyProgress( qint64, qint64 ) ) );

    while ( mCapabilitiesReply )
    {
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
    }

    if ( httpcapabilitiesresponse.isEmpty() )
    {
      if ( mError.isEmpty() )
      {
        mErrorFormat = "text/plain";
        mError = tr( "empty capabilities document" );
      }
      return false;
    }

    if ( httpcapabilitiesresponse.startsWith( "<html>" ) ||
         httpcapabilitiesresponse.startsWith( "<HTML>" ) )
    {
      mErrorFormat = "text/html";
      mError = httpcapabilitiesresponse;
      return false;
    }

    QgsDebugMsg( "Converting to Dom." );

    bool domOK;
    domOK = parseCapabilitiesDom( httpcapabilitiesresponse, mCapabilities );

    if ( !domOK )
    {
      // We had an Dom exception -
      // mErrorCaption and mError are pre-filled by parseCapabilitiesDom

      mError += tr( "\nTried URL: %1" ).arg( url );

      QgsDebugMsg( "!domOK: " + mError );

      return false;
    }

  }

  QgsDebugMsg( "exiting." );

  return true;
}

void QgsWmsProvider::capabilitiesReplyFinished()
{
  if ( mCapabilitiesReply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = mCapabilitiesReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      emit statusChanged( tr( "Capabilities request redirected." ) );

      QNetworkRequest request( redirect.toUrl() );
      setAuthorization( request );
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
      request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

      mCapabilitiesReply->deleteLater();
      QgsDebugMsg( QString( "redirected getcapabilities: %1" ).arg( redirect.toString() ) );
      mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );

      connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( capabilitiesReplyFinished() ) );
      connect( mCapabilitiesReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( capabilitiesReplyProgress( qint64, qint64 ) ) );
      return;
    }

    httpcapabilitiesresponse = mCapabilitiesReply->readAll();

    if ( httpcapabilitiesresponse.isEmpty() )
    {
      mErrorFormat = "text/plain";
      mError = tr( "empty of capabilities: %1" ).arg( mCapabilitiesReply->errorString() );
    }
  }
  else
  {
    mErrorFormat = "text/plain";
    mError = tr( "Download of capabilities failed: %1" ).arg( mCapabilitiesReply->errorString() );
    QgsMessageLog::logMessage( mError, tr( "WMS" ) );
    httpcapabilitiesresponse.clear();
  }


  mCapabilitiesReply->deleteLater();
  mCapabilitiesReply = 0;
}

int QgsWmsProvider::dataType( int bandNo ) const
{
  return srcDataType( bandNo );
}

int QgsWmsProvider::srcDataType( int bandNo ) const
{
  Q_UNUSED( bandNo );
  return QgsRasterDataProvider::ARGBDataType;
}

int QgsWmsProvider::bandCount() const
{
  return 1;
}

void QgsWmsProvider::capabilitiesReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of capabilities downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsg( msg );
  emit statusChanged( msg );
}

void QgsWmsProvider::cacheReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of map downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsg( msg );
  emit statusChanged( msg );
}

bool QgsWmsProvider::parseCapabilitiesDom( QByteArray const &xml, QgsWmsCapabilitiesProperty& capabilitiesProperty )
{
  QgsDebugMsg( "entering." );

#ifdef QGISDEBUG
  QFile file( QDir::tempPath() + "/qgis-wmsprovider-capabilities.xml" );
  if ( file.open( QIODevice::WriteOnly ) )
  {
    file.write( xml );
    file.close();
  }
#endif

  // Convert completed document into a Dom
  QString errorMsg;
  int errorLine;
  int errorColumn;
  bool contentSuccess = capabilitiesDom.setContent( xml, false, &errorMsg, &errorLine, &errorColumn );

  if ( !contentSuccess )
  {
    mErrorCaption = tr( "Dom Exception" );
    mErrorFormat = "text/plain";
    mError = tr( "Could not get WMS capabilities: %1 at line %2 column %3\nThis is probably due to an incorrect WMS Server URL.\nResponse was:\n\n%4" )
             .arg( errorMsg )
             .arg( errorLine )
             .arg( errorColumn )
             .arg( QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  QDomElement docElem = capabilitiesDom.documentElement();

  // Assert that the DTD is what we expected (i.e. a WMS Capabilities document)
  QgsDebugMsg( "testing tagName " + docElem.tagName() );

  if (
    docElem.tagName() != "WMS_Capabilities"  && // (1.3 vintage)
    docElem.tagName() != "WMT_MS_Capabilities"  // (1.1.1 vintage)
  )
  {
    mErrorCaption = tr( "Dom Exception" );
    mErrorFormat = "text/plain";
    mError = tr( "Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found.\nThis might be due to an incorrect WMS Server URL.\nTag:%3\nResponse was:\n%4" )
             .arg( "WMS_Capabilities" )
             .arg( "WMT_MS_Capabilities" )
             .arg( docElem.tagName() )
             .arg( QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  capabilitiesProperty.version = docElem.attribute( "version" );

  // Start walking through XML.
  QDomNode n = docElem.firstChild();

  while ( !n.isNull() )
  {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if ( !e.isNull() )
    {
      //QgsDebugMsg(e.tagName() ); // the node really is an element.

      if ( e.tagName() == "Service" )
      {
        QgsDebugMsg( "  Service." );
        parseService( e, capabilitiesProperty.service );
      }
      else if ( e.tagName() == "Capability" )
      {
        QgsDebugMsg( "  Capability." );
        parseCapability( e, capabilitiesProperty.capability );
      }

    }
    n = n.nextSibling();
  }

  QgsDebugMsg( "exiting." );

  return true;
}


void QgsWmsProvider::parseService( QDomElement const & e, QgsWmsServiceProperty& serviceProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      // QgsDebugMsg( "  "  + e1.tagName() ); // the node really is an element.
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Title" )
      {
        serviceProperty.title = e1.text();
      }
      else if ( tagName == "Abstract" )
      {
        serviceProperty.abstract = e1.text();
      }
      else if ( tagName == "KeywordList" )
      {
        parseKeywordList( e1, serviceProperty.keywordList );
      }
      else if ( tagName == "OnlineResource" )
      {
        parseOnlineResource( e1, serviceProperty.onlineResource );
      }
      else if ( tagName == "ContactInformation" )
      {
        parseContactInformation( e1, serviceProperty.contactInformation );
      }
      else if ( tagName == "Fees" )
      {
        serviceProperty.fees = e1.text();
      }
      else if ( tagName == "AccessConstraints" )
      {
        serviceProperty.accessConstraints = e1.text();
      }
      else if ( tagName == "LayerLimit" )
      {
        serviceProperty.layerLimit = e1.text().toUInt();
      }
      else if ( tagName == "MaxWidth" )
      {
        serviceProperty.maxWidth = e1.text().toUInt();
      }
      else if ( tagName == "MaxHeight" )
      {
        serviceProperty.maxHeight = e1.text().toUInt();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseCapability( QDomElement const & e, QgsWmsCapabilityProperty& capabilityProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      QgsDebugMsg( "  "  + e1.tagName() ); // the node really is an element.

      if ( tagName == "Request" )
      {
        parseRequest( e1, capabilityProperty.request );
      }
      else if ( tagName == "Layer" )
      {
        parseLayer( e1, capabilityProperty.layer );
      }
      else if ( tagName == "VendorSpecificCapabilities" )
      {
        for ( int i = 0; i < e1.childNodes().size(); i++ )
        {
          QDomNode n2 = e1.childNodes().item( i );
          QDomElement e2 = n2.toElement();

          QString tagName = e2.tagName();
          if ( tagName.startsWith( "wms:" ) )
            tagName = tagName.mid( 4 );

          if ( tagName == "TileSet" )
          {
            parseTileSetProfile( e2, capabilityProperty.tileSetProfiles );
          }
        }
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseContactPersonPrimary( QDomElement const & e, QgsWmsContactPersonPrimaryProperty& contactPersonPrimaryProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "ContactPerson" )
      {
        contactPersonPrimaryProperty.contactPerson = e1.text();
      }
      else if ( tagName == "ContactOrganization" )
      {
        contactPersonPrimaryProperty.contactOrganization = e1.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseContactAddress( QDomElement const & e, QgsWmsContactAddressProperty& contactAddressProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "AddressType" )
      {
        contactAddressProperty.addressType = e1.text();
      }
      else if ( tagName == "Address" )
      {
        contactAddressProperty.address = e1.text();
      }
      else if ( tagName == "City" )
      {
        contactAddressProperty.city = e1.text();
      }
      else if ( tagName == "StateOrProvince" )
      {
        contactAddressProperty.stateOrProvince = e1.text();
      }
      else if ( tagName == "PostCode" )
      {
        contactAddressProperty.postCode = e1.text();
      }
      else if ( tagName == "Country" )
      {
        contactAddressProperty.country = e1.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseContactInformation( QDomElement const & e, QgsWmsContactInformationProperty& contactInformationProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "ContactPersonPrimary" )
      {
        parseContactPersonPrimary( e1, contactInformationProperty.contactPersonPrimary );
      }
      else if ( tagName == "ContactPosition" )
      {
        contactInformationProperty.contactPosition = e1.text();
      }
      else if ( tagName == "ContactAddress" )
      {
        parseContactAddress( e1, contactInformationProperty.contactAddress );
      }
      else if ( tagName == "ContactVoiceTelephone" )
      {
        contactInformationProperty.contactVoiceTelephone = e1.text();
      }
      else if ( tagName == "ContactFacsimileTelephone" )
      {
        contactInformationProperty.contactFacsimileTelephone = e1.text();
      }
      else if ( tagName == "ContactElectronicMailAddress" )
      {
        contactInformationProperty.contactElectronicMailAddress = e1.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseOnlineResource( QDomElement const & e, QgsWmsOnlineResourceAttribute& onlineResourceAttribute )
{
  QgsDebugMsg( "entering." );

  onlineResourceAttribute.xlinkHref = e.attribute( "xlink:href" );

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseKeywordList( QDomElement  const & e, QStringList& keywordListProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Keyword" )
      {
        QgsDebugMsg( "      Keyword." );
        keywordListProperty += e1.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseGet( QDomElement const & e, QgsWmsGetProperty& getProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "OnlineResource" )
      {
        QgsDebugMsg( "      OnlineResource." );
        parseOnlineResource( e1, getProperty.onlineResource );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parsePost( QDomElement const & e, QgsWmsPostProperty& postProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "OnlineResource" )
      {
        QgsDebugMsg( "      OnlineResource." );
        parseOnlineResource( e1, postProperty.onlineResource );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseHttp( QDomElement const & e, QgsWmsHttpProperty& httpProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == "Get" )
      {
        QgsDebugMsg( "      Get." );
        parseGet( e1, httpProperty.get );
      }
      else if ( e1.tagName() == "Post" )
      {
        QgsDebugMsg( "      Post." );
        parsePost( e1, httpProperty.post );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseDcpType( QDomElement const & e, QgsWmsDcpTypeProperty& dcpType )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == "HTTP" )
      {
        QgsDebugMsg( "      HTTP." );
        parseHttp( e1, dcpType.http );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseOperationType( QDomElement const & e, QgsWmsOperationType& operationType )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Format" )
      {
        QgsDebugMsg( "      Format." );
        operationType.format += e1.text();
      }
      else if ( tagName == "DCPType" )
      {
        QgsDebugMsg( "      DCPType." );
        QgsWmsDcpTypeProperty dcp;
        parseDcpType( e1, dcp );
        operationType.dcpType.push_back( dcp );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseRequest( QDomElement const & e, QgsWmsRequestProperty& requestProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == "GetMap" )
      {
        QgsDebugMsg( "      GetMap." );
        parseOperationType( e1, requestProperty.getMap );
      }
      else if ( e1.tagName() == "GetFeatureInfo" )
      {
        QgsDebugMsg( "      GetFeatureInfo." );
        parseOperationType( e1, requestProperty.getFeatureInfo );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseLegendUrl( QDomElement const & e, QgsWmsLegendUrlProperty& legendUrlProperty )
{
  QgsDebugMsg( "entering." );

  legendUrlProperty.width  = e.attribute( "width" ).toUInt();
  legendUrlProperty.height = e.attribute( "height" ).toUInt();

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Format" )
      {
        legendUrlProperty.format = e1.text();
      }
      else if ( tagName == "OnlineResource" )
      {
        parseOnlineResource( e1, legendUrlProperty.onlineResource );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseStyle( QDomElement const & e, QgsWmsStyleProperty& styleProperty )
{
//  QgsDebugMsg("entering.");

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Name" )
      {
        styleProperty.name = e1.text();
      }
      else if ( tagName == "Title" )
      {
        styleProperty.title = e1.text();
      }
      else if ( tagName == "Abstract" )
      {
        styleProperty.abstract = e1.text();
      }
      else if ( tagName == "LegendURL" )
      {
        // TODO
      }
      else if ( tagName == "StyleSheetURL" )
      {
        // TODO
      }
      else if ( tagName == "StyleURL" )
      {
        // TODO
      }
    }
    n1 = n1.nextSibling();
  }

//  QgsDebugMsg("exiting.");
}


void QgsWmsProvider::parseLayer( QDomElement const & e, QgsWmsLayerProperty& layerProperty,
                                 QgsWmsLayerProperty *parentProperty )
{
//  QgsDebugMsg("entering.");

// TODO: Delete this stanza completely, depending on success of "Inherit things into the sublayer" below.
//  // enforce WMS non-inheritance rules
//  layerProperty.name =        QString::null;
//  layerProperty.title =       QString::null;
//  layerProperty.abstract =    QString::null;
//  layerProperty.keywordList.clear();
  layerProperty.orderId     = ++mLayerCount;
  layerProperty.queryable   = e.attribute( "queryable" ).toUInt();
  layerProperty.cascaded    = e.attribute( "cascaded" ).toUInt();
  layerProperty.opaque      = e.attribute( "opaque" ).toUInt();
  layerProperty.noSubsets   = e.attribute( "noSubsets" ).toUInt();
  layerProperty.fixedWidth  = e.attribute( "fixedWidth" ).toUInt();
  layerProperty.fixedHeight = e.attribute( "fixedHeight" ).toUInt();

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      //QgsDebugMsg( "    "  + e1.tagName() ); // the node really is an element.

      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Layer" )
      {
        QgsDebugMsg( "      Nested layer." );

        QgsWmsLayerProperty subLayerProperty;

        // Inherit things into the sublayer
        //   Ref: 7.2.4.8 Inheritance of layer properties
        subLayerProperty.style                    = layerProperty.style;
        subLayerProperty.crs                      = layerProperty.crs;
        subLayerProperty.boundingBox              = layerProperty.boundingBox;
        subLayerProperty.ex_GeographicBoundingBox = layerProperty.ex_GeographicBoundingBox;
        // TODO

        parseLayer( e1, subLayerProperty, &layerProperty );

        layerProperty.layer.push_back( subLayerProperty );
      }
      else if ( tagName == "Name" )
      {
        layerProperty.name = e1.text();
      }
      else if ( tagName == "Title" )
      {
        layerProperty.title = e1.text();
      }
      else if ( tagName == "Abstract" )
      {
        layerProperty.abstract = e1.text();
      }
      else if ( tagName == "KeywordList" )
      {
        parseKeywordList( e1, layerProperty.keywordList );
      }
      else if ( tagName == "SRS" || tagName == "CRS" )
      {
        // CRS can contain several definitions separated by whitespace
        // though this was deprecated in WMS 1.1.1
        foreach( QString srs, e1.text().split( QRegExp( "\\s+" ) ) )
        {
          layerProperty.crs.push_back( srs );
        }
      }
      else if ( tagName == "LatLonBoundingBox" )      // legacy from earlier versions of WMS
      {
        layerProperty.ex_GeographicBoundingBox = QgsRectangle(
              e1.attribute( "minx" ).toDouble(),
              e1.attribute( "miny" ).toDouble(),
              e1.attribute( "maxx" ).toDouble(),
              e1.attribute( "maxy" ).toDouble()
            );

        if ( e1.hasAttribute( "SRS" ) && e1.attribute( "SRS" ) != DEFAULT_LATLON_CRS )
        {
          try
          {
            QgsCoordinateReferenceSystem src;
            src.createFromOgcWmsCrs( e1.attribute( "SRS" ) );

            QgsCoordinateReferenceSystem dst;
            dst.createFromOgcWmsCrs( DEFAULT_LATLON_CRS );

            QgsCoordinateTransform ct( src, dst );
            layerProperty.ex_GeographicBoundingBox = ct.transformBoundingBox( layerProperty.ex_GeographicBoundingBox );
          }
          catch ( QgsCsException &cse )
          {
            Q_UNUSED( cse );
          }
        }
      }
      else if ( tagName == "EX_GeographicBoundingBox" ) //for WMS 1.3
      {
        QDomElement wBoundLongitudeElem, eBoundLongitudeElem, sBoundLatitudeElem, nBoundLatitudeElem;

        if ( e1.tagName() == "wms:EX_GeographicBoundingBox" )
        {
          wBoundLongitudeElem = n1.namedItem( "wms:westBoundLongitude" ).toElement();
          eBoundLongitudeElem = n1.namedItem( "wms:eastBoundLongitude" ).toElement();
          sBoundLatitudeElem = n1.namedItem( "wms:southBoundLatitude" ).toElement();
          nBoundLatitudeElem = n1.namedItem( "wms:northBoundLatitude" ).toElement();
        }
        else
        {
          wBoundLongitudeElem = n1.namedItem( "westBoundLongitude" ).toElement();
          eBoundLongitudeElem = n1.namedItem( "eastBoundLongitude" ).toElement();
          sBoundLatitudeElem = n1.namedItem( "southBoundLatitude" ).toElement();
          nBoundLatitudeElem = n1.namedItem( "northBoundLatitude" ).toElement();
        }

        double wBLong, eBLong, sBLat, nBLat;
        bool wBOk, eBOk, sBOk, nBOk;
        wBLong = wBoundLongitudeElem.text().toDouble( &wBOk );
        eBLong = eBoundLongitudeElem.text().toDouble( &eBOk );
        sBLat = sBoundLatitudeElem.text().toDouble( &sBOk );
        nBLat = nBoundLatitudeElem.text().toDouble( &nBOk );
        if ( wBOk && eBOk && sBOk && nBOk )
        {
          layerProperty.ex_GeographicBoundingBox = QgsRectangle( wBLong, sBLat, eBLong, nBLat );
        }
      }
      else if ( tagName == "BoundingBox" )
      {
        // TODO: overwrite inherited
        QgsWmsBoundingBoxProperty bbox;
        bbox.box = QgsRectangle( e1.attribute( "minx" ).toDouble(),
                                 e1.attribute( "miny" ).toDouble(),
                                 e1.attribute( "maxx" ).toDouble(),
                                 e1.attribute( "maxy" ).toDouble()
                               );
        if ( e1.hasAttribute( "CRS" ) || e1.hasAttribute( "SRS" ) )
        {
          if ( e1.hasAttribute( "CRS" ) )
            bbox.crs = e1.attribute( "CRS" );
          else if ( e1.hasAttribute( "SRS" ) )
            bbox.crs = e1.attribute( "SRS" );

          layerProperty.boundingBox.push_back( bbox );
        }
        else
        {
          QgsDebugMsg( "CRS/SRS attribute note found in BoundingBox" );
        }
      }
      else if ( tagName == "Dimension" )
      {
        // TODO
      }
      else if ( tagName == "Attribution" )
      {
        // TODO
      }
      else if ( tagName == "AuthorityURL" )
      {
        // TODO
      }
      else if ( tagName == "Identifier" )
      {
        // TODO
      }
      else if ( tagName == "MetadataURL" )
      {
        // TODO
      }
      else if ( tagName == "DataURL" )
      {
        // TODO
      }
      else if ( tagName == "FeatureListURL" )
      {
        // TODO
      }
      else if ( tagName == "Style" )
      {
        QgsWmsStyleProperty styleProperty;

        parseStyle( e1, styleProperty );

        layerProperty.style.push_back( styleProperty );
      }
      else if ( tagName == "MinScaleDenominator" )
      {
        // TODO
      }
      else if ( tagName == "MaxScaleDenominator" )
      {
        // TODO
      }
      // If we got here then it's not in the WMS 1.3 standard

    }
    n1 = n1.nextSibling();
  }

  if ( parentProperty )
  {
    mLayerParents[ layerProperty.orderId ] = parentProperty->orderId;
  }

  if ( !layerProperty.name.isEmpty() )
  {
    // We have all the information we need to properly evaluate a layer definition
    // TODO: Save this somewhere

    // Store if the layer is queryable
    mQueryableForLayer[ layerProperty.name ] = layerProperty.queryable;

    // Store the available Coordinate Reference Systems for the layer so that it
    // can be combined with others later in supportedCrsForLayers()
    crsForLayer[ layerProperty.name ] = layerProperty.crs;

    // Store the WGS84 (CRS:84) extent so that it can be combined with others later
    // in calculateExtent()

    // Apply the coarse bounding box first
    extentForLayer[ layerProperty.name ] = layerProperty.ex_GeographicBoundingBox;

    // see if we can refine the bounding box with the CRS-specific bounding boxes
    for ( int i = 0; i < layerProperty.boundingBox.size(); i++ )
    {
      QgsDebugMsg( "testing bounding box CRS which is "
                   + layerProperty.boundingBox[i].crs + "." );

      if ( layerProperty.boundingBox[i].crs == DEFAULT_LATLON_CRS )
      {
        extentForLayer[ layerProperty.name ] = layerProperty.boundingBox[i].box;
      }
    }

    QgsDebugMsg( "extent for "
                 + layerProperty.name  + " is "
                 + extentForLayer[ layerProperty.name ].toString( 3 )  + "." );

    // Insert into the local class' registry
    layersSupported.push_back( layerProperty );

    //if there are several <Layer> elements without a parent layer, the style list needs to be cleared
    if ( layerProperty.layer.empty() )
    {
      layerProperty.style.clear();
    }
  }

  if ( !layerProperty.layer.empty() )
  {
    mLayerParentNames[ layerProperty.orderId ] = QStringList() << layerProperty.name << layerProperty.title << layerProperty.abstract;
  }

  if ( !parentProperty )
  {
    // Why clear()? I need top level access. Seems to work in standard select dialog without clear.
    //layerProperty.layer.clear();
    layerProperty.crs.clear();
  }

//  QgsDebugMsg("exiting.");
}

void QgsWmsProvider::parseTileSetProfile( QDomElement const &e, QVector<QgsWmsTileSetProfile> &tileSet )
{
  Q_UNUSED( tileSet );
  QgsWmsTileSetProfile tsp;

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QgsDebugMsg( "    "  + e1.tagName() ); // the node really is an element.

      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Layers" )
      {
        tsp.layers << e1.text();
      }
      else if ( tagName == "Styles" )
      {
        tsp.styles << e1.text();
      }
      else if ( tagName == "Width" )
      {
        tsp.tileWidth = e1.text().toInt();
      }
      else if ( tagName == "Height" )
      {
        tsp.tileHeight = e1.text().toInt();
      }
      else if ( tagName == "SRS" )
      {
        tsp.crs = e1.text();
      }
      else if ( tagName == "Format" )
      {
        tsp.format = e1.text();
      }
      else if ( tagName == "BoundingBox" )
      {
        tsp.boundingBox.box = QgsRectangle(
                                e1.attribute( "minx" ).toDouble(),
                                e1.attribute( "miny" ).toDouble(),
                                e1.attribute( "maxx" ).toDouble(),
                                e1.attribute( "maxy" ).toDouble()
                              );
        if ( e1.hasAttribute( "SRS" ) )
          tsp.boundingBox.crs = e1.attribute( "SRS" );
        else if ( e1.hasAttribute( "srs" ) )
          tsp.boundingBox.crs = e1.attribute( "srs" );
        else if ( e1.hasAttribute( "CRS" ) )
          tsp.boundingBox.crs = e1.attribute( "CRS" );
        else if ( e1.hasAttribute( "crs" ) )
          tsp.boundingBox.crs = e1.attribute( "crs" );

      }
      else if ( tagName == "Resolutions" )
      {
        tsp.resolutions = e1.text().trimmed().split( " ", QString::SkipEmptyParts );
      }
      else
      {
        QgsDebugMsg( QString( "tileset tag %1 ignored" ).arg( e1.tagName() ) );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( QString( "extent for %1/%2 is %3:%4." )
               .arg( tsp.layers.join( "," ) )
               .arg( tsp.styles.join( "," ) )
               .arg( tsp.boundingBox.crs )
               .arg( tsp.boundingBox.box.toString( 3 ) ) );

  tilesetsSupported << tsp;
}

void QgsWmsProvider::layerParents( QMap<int, int> &parents, QMap<int, QStringList> &parentNames ) const
{
  parents = mLayerParents;
  parentNames = mLayerParentNames;
}

bool QgsWmsProvider::parseServiceExceptionReportDom( QByteArray const & xml )
{
  QgsDebugMsg( "entering." );

#ifdef QGISDEBUG
  //test the content of the QByteArray
  QString responsestring( xml );
  QgsDebugMsg( "received the following data: " + responsestring );
#endif

  // Convert completed document into a Dom
  QString errorMsg;
  int errorLine;
  int errorColumn;
  bool contentSuccess = serviceExceptionReportDom.setContent( xml, false, &errorMsg, &errorLine, &errorColumn );

  if ( !contentSuccess )
  {
    mErrorCaption = tr( "Dom Exception" );
    mErrorFormat = "text/plain";
    mError = tr( "Could not get WMS Service Exception at %1: %2 at line %3 column %4\n\nResponse was:\n\n%5" )
             .arg( mBaseUrl )
             .arg( errorMsg )
             .arg( errorLine )
             .arg( errorColumn )
             .arg( QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  QDomElement docElem = serviceExceptionReportDom.documentElement();

  // TODO: Assert the docElem.tagName() is "ServiceExceptionReport"

  // serviceExceptionProperty.version = docElem.attribute("version");

  // Start walking through XML.
  QDomNode n = docElem.firstChild();

  while ( !n.isNull() )
  {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if ( !e.isNull() )
    {
      //QgsDebugMsg(e.tagName() ); // the node really is an element.

      QString tagName = e.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "ServiceException" )
      {
        QgsDebugMsg( "  ServiceException." );
        parseServiceException( e );
      }

    }
    n = n.nextSibling();
  }

  QgsDebugMsg( "exiting." );

  return true;
}


void QgsWmsProvider::parseServiceException( QDomElement const & e )
{
  QgsDebugMsg( "entering." );

  QString seCode = e.attribute( "code" );
  QString seText = e.text();

  mErrorFormat = "text/plain";

  // set up friendly descriptions for the service exception
  if ( seCode == "InvalidFormat" )
  {
    mError = tr( "Request contains a format not offered by the server." );
  }
  else if ( seCode == "InvalidCRS" )
  {
    mError = tr( "Request contains a CRS not offered by the server for one or more of the Layers in the request." );
  }
  else if ( seCode == "InvalidSRS" )  // legacy WMS < 1.3.0
  {
    mError = tr( "Request contains a SRS not offered by the server for one or more of the Layers in the request." );
  }
  else if ( seCode == "LayerNotDefined" )
  {
    mError = tr( "GetMap request is for a Layer not offered by the server, "
                 "or GetFeatureInfo request is for a Layer not shown on the map." );
  }
  else if ( seCode == "StyleNotDefined" )
  {
    mError = tr( "Request is for a Layer in a Style not offered by the server." );
  }
  else if ( seCode == "LayerNotQueryable" )
  {
    mError = tr( "GetFeatureInfo request is applied to a Layer which is not declared queryable." );
  }
  else if ( seCode == "InvalidPoint" )
  {
    mError = tr( "GetFeatureInfo request contains invalid X or Y value." );
  }
  else if ( seCode == "CurrentUpdateSequence" )
  {
    mError = tr( "Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to "
                 "current value of service metadata update sequence number." );
  }
  else if ( seCode == "InvalidUpdateSequence" )
  {
    mError = tr( "Value of (optional) UpdateSequence parameter in GetCapabilities request is greater "
                 "than current value of service metadata update sequence number." );
  }
  else if ( seCode == "MissingDimensionValue" )
  {
    mError = tr( "Request does not include a sample dimension value, and the server did not declare a "
                 "default value for that dimension." );
  }
  else if ( seCode == "InvalidDimensionValue" )
  {
    mError = tr( "Request contains an invalid sample dimension value." );
  }
  else if ( seCode == "OperationNotSupported" )
  {
    mError = tr( "Request is for an optional operation that is not supported by the server." );
  }
  else if ( seCode.isEmpty() )
  {
    mError = tr( "(No error code was reported)" );
  }
  else
  {
    mError = seCode + " " + tr( "(Unknown error code)" );
  }

  mError += "\n" + tr( "The WMS vendor also reported: " );
  mError += seText;

  // TODO = e.attribute("locator");

  QgsMessageLog::logMessage( tr( "composed error message '%1'." ).arg( mError ), tr( "WMS" ) );
  QgsDebugMsg( "exiting." );
}



QgsRectangle QgsWmsProvider::extent()
{
  if ( extentDirty )
  {
    if ( calculateExtent() )
    {
      extentDirty = false;
    }
  }

  return layerExtent;
}

bool QgsWmsProvider::isValid()
{
  return valid;
}


QString QgsWmsProvider::wmsVersion()
{
  // TODO
  return NULL;
}

QStringList QgsWmsProvider::supportedImageEncodings()
{
  return mCapabilities.capability.request.getMap.format;
}


QStringList QgsWmsProvider::subLayers() const
{
  return activeSubLayers;
}


QStringList QgsWmsProvider::subLayerStyles() const
{
  return activeSubStyles;
}

bool QgsWmsProvider::calculateExtent()
{
  //! \todo Make this handle non-geographic CRSs (e.g. floor plans) as per WMS spec

  QgsDebugMsg( "entered." );

  // Make sure we know what extents are available
  if ( !retrieveServerCapabilities() )
  {
    return false;
  }

  if ( mTiled && mResolutions.size() > 0 )
  {
    QString layers = activeSubLayers.join( "," );
    QString styles = activeSubStyles.join( "," );

    QgsDebugMsg( QString( "looking for tileset with layers=%1, styles=%2 and crs=%3." )
                 .arg( layers ).arg( styles ).arg( imageCrs ) );
    for ( int i = 0; i < tilesetsSupported.size(); i++ )
    {
      if ( tilesetsSupported[i].layers.join( "," ) == layers &&
           tilesetsSupported[i].styles.join( "," ) == styles &&
           tilesetsSupported[i].crs == imageCrs )
      {
        layerExtent = tilesetsSupported[i].boundingBox.box;
        return true;
      }

      QgsMessageLog::logMessage( tr( "mismatch layers=%1, styles=%2 and crs=%3." )
                                 .arg( tilesetsSupported[i].layers.join( "," ) )
                                 .arg( tilesetsSupported[i].styles.join( "," ) )
                                 .arg( tilesetsSupported[i].crs ), tr( "WMS" ) );
    }

    QgsMessageLog::logMessage( tr( "no extent for layer" ), tr( "WMS" ) );
    return false;
  }

  // Set up the coordinate transform from the WMS standard CRS:84 bounding
  // box to the user's selected CRS
  if ( !mCoordinateTransform )
  {
    QgsCoordinateReferenceSystem qgisSrsSource;
    QgsCoordinateReferenceSystem qgisSrsDest;

    qgisSrsSource.createFromOgcWmsCrs( DEFAULT_LATLON_CRS );
    qgisSrsDest  .createFromOgcWmsCrs( imageCrs );

    mCoordinateTransform = new QgsCoordinateTransform( qgisSrsSource, qgisSrsDest );
  }

  bool firstLayer = true; //flag to know if a layer is the first to be successfully transformed
  for ( QStringList::Iterator it  = activeSubLayers.begin();
        it != activeSubLayers.end();
        ++it )
  {
    QgsDebugMsg( "Sublayer Iterator: " + *it );
    // This is the extent for the layer name in *it
    QgsRectangle extent = extentForLayer.find( *it ).value();

    // Convert to the user's CRS as required
    try
    {
      extent = mCoordinateTransform->transformBoundingBox( extent, QgsCoordinateTransform::ForwardTransform );
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse );
      continue; //ignore extents of layers which cannot be transformed info the required CRS
    }

    //make sure extent does not contain 'inf' or 'nan'
    if ( !extent.isFinite() )
    {
      continue;
    }

    // add to the combined extent of all the active sublayers
    if ( firstLayer )
    {
      layerExtent = extent;
    }
    else
    {
      layerExtent.combineExtentWith( &extent );
    }

    firstLayer = false;

    QgsDebugMsg( "combined extent is '"  + layerExtent.toString()
                 + "' after '"  + ( *it ) + "'." );

  }

  QgsDebugMsg( "exiting with '"  + layerExtent.toString() + "'." );

  return true;

}


int QgsWmsProvider::capabilities() const
{
  int capability = NoCapabilities;
  bool canIdentify = false;

  QgsDebugMsg( "entering." );

  // Test for the ability to use the Identify map tool
  for ( QStringList::const_iterator it  = activeSubLayers.begin();
        it != activeSubLayers.end();
        ++it )
  {
    // Is sublayer visible?
    if ( activeSubLayerVisibility.find( *it ).value() )
    {
      // Is sublayer queryable?
      if ( mQueryableForLayer.find( *it ).value() )
      {
        QgsDebugMsg( "'"  + ( *it )  + "' is queryable." );
        canIdentify = true;
      }
    }
  }

  if ( canIdentify )
  {
    foreach( QString f, mCapabilities.capability.request.getFeatureInfo.format )
    {
      if ( mSupportedGetFeatureFormats.contains( f ) )
      {
        // Collect all the test results into one bitmask
        capability |= QgsRasterDataProvider::Identify;
        break;
      }
    }
  }

  //QgsDebugMsg( "exiting with '"  + QString( capability )  + "'." );

  return capability;
}

QString QgsWmsProvider::layerMetadata( QgsWmsLayerProperty &layer )
{
  QString myMetadataQString;

  // Layer Properties section

  // Use a nested table
  myMetadataQString += "<tr><td>";
  myMetadataQString += "<table width=\"100%\">";

  // Table header
  myMetadataQString += "<tr><th class=\"glossy\">";
  myMetadataQString += tr( "Property" );
  myMetadataQString += "</th>";
  myMetadataQString += "<th class=\"glossy\">";
  myMetadataQString += tr( "Value" );
  myMetadataQString += "</th></tr>";

  // Name
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Name" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += layer.name;
  myMetadataQString += "</td></tr>";

  // Layer Visibility (as managed by this provider)
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Visibility" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += activeSubLayerVisibility.find( layer.name ).value() ? tr( "Visible" ) : tr( "Hidden" );
  myMetadataQString += "</td></tr>";

  // Layer Title
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Title" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += layer.title;
  myMetadataQString += "</td></tr>";

  // Layer Abstract
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Abstract" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += layer.abstract;
  myMetadataQString += "</td></tr>";

  // Layer Queryability
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Can Identify" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += layer.queryable ? tr( "Yes" ) : tr( "No" );
  myMetadataQString += "</td></tr>";

  // Layer Opacity
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Can be Transparent" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += layer.opaque ? tr( "No" ) : tr( "Yes" );
  myMetadataQString += "</td></tr>";

  // Layer Subsetability
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Can Zoom In" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += layer.noSubsets ? tr( "No" ) : tr( "Yes" );
  myMetadataQString += "</td></tr>";

  // Layer Server Cascade Count
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Cascade Count" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += QString::number( layer.cascaded );
  myMetadataQString += "</td></tr>";

  // Layer Fixed Width
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Fixed Width" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += QString::number( layer.fixedWidth );
  myMetadataQString += "</td></tr>";

  // Layer Fixed Height
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Fixed Height" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += QString::number( layer.fixedHeight );
  myMetadataQString += "</td></tr>";

  // Layer Fixed Height
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "WGS 84 Bounding Box" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += extentForLayer[ layer.name ].toString();
  myMetadataQString += "</td></tr>";

  // Layer Coordinate Reference Systems
  for ( int j = 0; j < qMin( layer.crs.size(), 10 ); j++ )
  {
    myMetadataQString += "<tr><td>";
    myMetadataQString += tr( "Available in CRS" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td>";
    myMetadataQString += layer.crs[j];
    myMetadataQString += "</td></tr>";
  }

  if ( layer.crs.size() > 10 )
  {
    myMetadataQString += "<tr><td>";
    myMetadataQString += tr( "Available in CRS" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td>";
    myMetadataQString += tr( "(and %n more)", "crs", layer.crs.size() - 10 );
    myMetadataQString += "</td></tr>";
  }

  // Layer Styles
  for ( int j = 0; j < layer.style.size(); j++ )
  {
    myMetadataQString += "<tr><td>";
    myMetadataQString += tr( "Available in style" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td>";

    // Nested table.
    myMetadataQString += "<table width=\"100%\">";

    // Layer Style Name
    myMetadataQString += "<tr><th class=\"glossy\">";
    myMetadataQString += tr( "Name" );
    myMetadataQString += "</th>";
    myMetadataQString += "<td>";
    myMetadataQString += layer.style[j].name;
    myMetadataQString += "</td></tr>";

    // Layer Style Title
    myMetadataQString += "<tr><th class=\"glossy\">";
    myMetadataQString += tr( "Title" );
    myMetadataQString += "</th>";
    myMetadataQString += "<td>";
    myMetadataQString += layer.style[j].title;
    myMetadataQString += "</td></tr>";

    // Layer Style Abstract
    myMetadataQString += "<tr><th class=\"glossy\">";
    myMetadataQString += tr( "Abstract" );
    myMetadataQString += "</th>";
    myMetadataQString += "<td>";
    myMetadataQString += layer.style[j].abstract;
    myMetadataQString += "</td></tr>";

    // Close the nested table
    myMetadataQString += "</table>";
    myMetadataQString += "</td></tr>";
  }

  // Close the nested table
  myMetadataQString += "</table>";
  myMetadataQString += "</td></tr>";

  return myMetadataQString;
}

QString QgsWmsProvider::metadata()
{
  QString myMetadataQString = "";

  myMetadataQString += "<tr><td>";

  myMetadataQString += "<a href=\"#serverproperties\">";
  myMetadataQString += tr( "Server Properties" );
  myMetadataQString += "</a> ";

  myMetadataQString += "&nbsp;<a href=\"#selectedlayers\">";
  myMetadataQString += tr( "Selected Layers" );
  myMetadataQString += "</a>&nbsp;<a href=\"#otherlayers\">";
  myMetadataQString += tr( "Other Layers" );
  myMetadataQString += "</a>";

  if ( tilesetsSupported.size() > 0 )
  {
    myMetadataQString += "<a href=\"#tilesetproperties\">";
    myMetadataQString += tr( "Tileset Properties" );
    myMetadataQString += "</a> ";

#if QT_VERSION >= 0x40500
    myMetadataQString += "<a href=\"#cachestats\">";
    myMetadataQString += tr( "Cache Stats" );
    myMetadataQString += "</a> ";
#endif
  }

  myMetadataQString += "</td></tr>";

  // Server Properties section
  myMetadataQString += "<tr><th class=\"glossy\"><a name=\"serverproperties\"></a>";
  myMetadataQString += tr( "Server Properties" );
  myMetadataQString += "</th></tr>";

  // Use a nested table
  myMetadataQString += "<tr><td>";
  myMetadataQString += "<table width=\"100%\">";

  // Table header
  myMetadataQString += "<tr><th class=\"glossy\">";
  myMetadataQString += tr( "Property" );
  myMetadataQString += "</th>";
  myMetadataQString += "<th class=\"glossy\">";
  myMetadataQString += tr( "Value" );
  myMetadataQString += "</th></tr>";

  // WMS Version
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "WMS Version" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += mCapabilities.version;
  myMetadataQString += "</td></tr>";

  // Service Title
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Title" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += mCapabilities.service.title;
  myMetadataQString += "</td></tr>";

  // Service Abstract
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Abstract" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += mCapabilities.service.abstract;
  myMetadataQString += "</td></tr>";

  // Service Keywords
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Keywords" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += mCapabilities.service.keywordList.join( "<br />" );
  myMetadataQString += "</td></tr>";

  // Service Online Resource
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Online Resource" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += "-";
  myMetadataQString += "</td></tr>";

  // Service Contact Information
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Contact Person" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += mCapabilities.service.contactInformation.contactPersonPrimary.contactPerson;
  myMetadataQString += "<br />";
  myMetadataQString += mCapabilities.service.contactInformation.contactPosition;
  myMetadataQString += "<br />";
  myMetadataQString += mCapabilities.service.contactInformation.contactPersonPrimary.contactOrganization;
  myMetadataQString += "</td></tr>";

  // Service Fees
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Fees" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += mCapabilities.service.fees;
  myMetadataQString += "</td></tr>";

  // Service Access Constraints
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Access Constraints" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += mCapabilities.service.accessConstraints;
  myMetadataQString += "</td></tr>";

  // GetMap Request Formats
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Image Formats" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += mCapabilities.capability.request.getMap.format.join( "<br />" );
  myMetadataQString += "</td></tr>";

  // GetFeatureInfo Request Formats
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Identify Formats" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += mCapabilities.capability.request.getFeatureInfo.format.join( "<br />" );
  myMetadataQString += "</td></tr>";

  // Layer Count (as managed by this provider)
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "Layer Count" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += QString::number( layersSupported.size() );
  myMetadataQString += "</td></tr>";

  // Tileset Count (as managed by this provider)
  if ( tilesetsSupported.size() > 0 )
  {
    myMetadataQString += "<tr><td>";
    myMetadataQString += tr( "Tileset Count" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td>";
    myMetadataQString += QString::number( tilesetsSupported.size() );
    myMetadataQString += "</td></tr>";
  }

  // Base URL
  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "GetCapabilitiesUrl" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += mBaseUrl;
  myMetadataQString += "</td></tr>";

  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "GetMapUrl" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += getMapUrl() + ( mIgnoreGetMapUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : "" );
  myMetadataQString += "</td></tr>";

  myMetadataQString += "<tr><td>";
  myMetadataQString += tr( "GetFeatureInfoUrl" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td>";
  myMetadataQString += getFeatureInfoUrl() + ( mIgnoreGetFeatureInfoUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : "" );
  myMetadataQString += "</td></tr>";

  // Close the nested table
  myMetadataQString += "</table>";
  myMetadataQString += "</td></tr>";

  // Layer properties
  myMetadataQString += "<tr><th class=\"glossy\"><a name=\"selectedlayers\"></a>";
  myMetadataQString += tr( "Selected Layers" );
  myMetadataQString += "</th></tr>";

  for ( int i = 0; i < layersSupported.size(); i++ )
  {
    if ( !mTiled && activeSubLayers.indexOf( layersSupported[i].name ) >= 0 )
    {
      myMetadataQString += layerMetadata( layersSupported[i] );
    }
  } // for each layer

  // Layer properties
  myMetadataQString += "<tr><th class=\"glossy\"><a name=\"otherlayers\"></a>";
  myMetadataQString += tr( "Other Layers" );
  myMetadataQString += "</th></tr>";

  for ( int i = 0; i < layersSupported.size(); i++ )
  {
    if ( activeSubLayers.indexOf( layersSupported[i].name ) < 0 )
    {
      myMetadataQString += layerMetadata( layersSupported[i] );
    }
  } // for each layer

  // Tileset properties
  if ( tilesetsSupported.size() > 0 )
  {
    myMetadataQString += "<tr><th class=\"glossy\"><a name=\"tilesetproperties\"></a>";
    myMetadataQString += tr( "Tileset Properties" );
    myMetadataQString += "</th></tr>";

    // Iterate through tilesets
    myMetadataQString += "<tr><td>";
    myMetadataQString += "<table width=\"100%\">";

    for ( int i = 0; i < tilesetsSupported.size(); i++ )
    {
      myMetadataQString += "<tr><td colspan=\"2\">";
      myMetadataQString += tilesetsSupported[i].layers.join( ", " );
      myMetadataQString += "</td></tr>";

      // Table header
      myMetadataQString += "<tr><th class=\"glossy\">";
      myMetadataQString += tr( "Property" );
      myMetadataQString += "</th>";
      myMetadataQString += "<th class=\"glossy\">";
      myMetadataQString += tr( "Value" );
      myMetadataQString += "</th></tr>";

      myMetadataQString += "<tr><td class=\"glossy\">";
      myMetadataQString += tr( "Selected" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td class=\"glossy\">";
      myMetadataQString += mTiled && tilesetsSupported[i].layers.join( "," ) == activeSubLayers.join( "," ) ? tr( "Yes" ) : tr( "No" );
      myMetadataQString += "</td></tr>";

      if ( tilesetsSupported[i].styles.size() > 0 )
      {
        myMetadataQString += "<tr><td class=\"glossy\">";
        myMetadataQString += tr( "Styles" );
        myMetadataQString += "</td>";
        myMetadataQString += "<td class=\"glossy\">";
        myMetadataQString += tilesetsSupported[i].styles.join( ", " );
        myMetadataQString += "</td></tr>";
      }

      myMetadataQString += "<tr><td class=\"glossy\">";
      myMetadataQString += tr( "CRS" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td class=\"glossy\">";
      myMetadataQString += tilesetsSupported[i].boundingBox.crs;
      myMetadataQString += "</td></tr>";

      myMetadataQString += "<tr><td class=\"glossy\">";
      myMetadataQString += tr( "Bounding Box" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td class=\"glossy\">";
      myMetadataQString += tilesetsSupported[i].boundingBox.box.toString();
      myMetadataQString += "</td></tr>";

      myMetadataQString += "<tr><td class=\"glossy\">";
      myMetadataQString += tr( "Available in Resolutions" );
      myMetadataQString += "</td><td class=\"glossy\">";

      for ( int j = 0; j < tilesetsSupported[i].resolutions.size(); j++ )
      {
        myMetadataQString += tilesetsSupported[i].resolutions[j] + "<br>";
      }

      myMetadataQString += "</td></tr>";
    }

    myMetadataQString += "</table></td></tr>";

#if QT_VERSION >= 0x40500
    if ( mTiled )
    {
      myMetadataQString += "<tr><th class=\"glossy\"><a name=\"cachestats\"></a>";
      myMetadataQString += tr( "Cache stats" );
      myMetadataQString += "</th></tr>";

      // Iterate through tilesets
      myMetadataQString += "<tr><td>";
      myMetadataQString += "<table width=\"100%\">";

      myMetadataQString += "<tr><th class=\"glossy\">";
      myMetadataQString += tr( "Property" );
      myMetadataQString += "</th>";
      myMetadataQString += "<th class=\"glossy\">";
      myMetadataQString += tr( "Value" );
      myMetadataQString += "</th></tr>";

      myMetadataQString += "<tr><td>";
      myMetadataQString += tr( "Hits" );
      myMetadataQString += "</td><td>";
      myMetadataQString += QString::number( mCacheHits );
      myMetadataQString += "</td></tr>";

      myMetadataQString += "<tr><td>";
      myMetadataQString += tr( "Misses" );
      myMetadataQString += "</td><td>";
      myMetadataQString += QString::number( mCacheMisses );
      myMetadataQString += "</td></tr>";

      myMetadataQString += "<tr><td>";
      myMetadataQString += tr( "Errors" );
      myMetadataQString += "</td><td>";
      myMetadataQString += QString::number( mErrors );
      myMetadataQString += "</td></tr>";

      myMetadataQString += "</table></td></tr>";
    }
#endif
  }

  myMetadataQString += "</table>";

  QgsDebugMsg( "exiting with '"  + myMetadataQString  + "'." );

  return myMetadataQString;
}

QStringList QgsWmsProvider::identifyAs( const QgsPoint& point, QString format )
{
  QgsDebugMsg( "Entering." );
  QStringList results;

  // Collect which layers to query on

  QStringList queryableLayers = QStringList();
  QString text = "";

  //according to the WMS spec for 1.3, the order of x - and y - coordinates is inverted for geographical CRS
  bool changeXY = false;
  if ( mCapabilities.version == "1.3.0" || mCapabilities.version == "1.3" )
  {
    //create CRS from string
    QgsCoordinateReferenceSystem theSrs;
    if ( theSrs.createFromOgcWmsCrs( imageCrs ) && theSrs.geographicFlag() )
    {
      changeXY = true;
    }
  }

  // compose the URL query string for the WMS server.
  QString crsKey = "SRS"; //SRS in 1.1.1 and CRS in 1.3.0
  if ( mCapabilities.version == "1.3.0" || mCapabilities.version == "1.3" )
  {
    crsKey = "CRS";
  }

  // Compose request to WMS server
  QString bbox = QString( changeXY ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
                 .arg( cachedViewExtent.xMinimum(), 0, 'f' )
                 .arg( cachedViewExtent.yMinimum(), 0, 'f' )
                 .arg( cachedViewExtent.xMaximum(), 0, 'f' )
                 .arg( cachedViewExtent.yMaximum(), 0, 'f' );

  // Test for which layers are suitable for querying with
  for ( QStringList::const_iterator
        layers  = activeSubLayers.begin(),
        styles = activeSubStyles.begin();
        layers != activeSubLayers.end();
        ++layers, ++styles )
  {
    // Is sublayer visible?
    if ( activeSubLayerVisibility.find( *layers ).value() )
    {
      // Is sublayer queryable?
      if ( mQueryableForLayer.find( *layers ).value() )
      {
        QgsDebugMsg( "Layer '" + *layers + "' is queryable." );

        QUrl requestUrl( mGetFeatureInfoUrlBase );
        setQueryItem( requestUrl, "SERVICE", "WMS" );
        setQueryItem( requestUrl, "VERSION", mCapabilities.version );
        setQueryItem( requestUrl, "REQUEST", "GetFeatureInfo" );
        setQueryItem( requestUrl, "BBOX", bbox );
        setQueryItem( requestUrl, crsKey, imageCrs );
        setQueryItem( requestUrl, "WIDTH", QString::number( cachedViewWidth ) );
        setQueryItem( requestUrl, "HEIGHT", QString::number( cachedViewHeight ) );
        setQueryItem( requestUrl, "LAYERS", *layers );
        setQueryItem( requestUrl, "STYLES", *styles );
        setQueryItem( requestUrl, "FORMAT", imageMimeType );
        setQueryItem( requestUrl, "QUERY_LAYERS", *layers );
        setQueryItem( requestUrl, "INFO_FORMAT", format );
        setQueryItem( requestUrl, "X", QString::number( point.x() ) );
        setQueryItem( requestUrl, "Y", QString::number( point.y() ) );

        if ( mFeatureCount > 0 )
        {
          setQueryItem( requestUrl, "FEATURE_COUNT", QString::number( mFeatureCount ) );
        }

        // X,Y in WMS 1.1.1; I,J in WMS 1.3.0
        //   requestUrl += QString( "&I=%1&J=%2" ).arg( point.x() ).arg( point.y() );

        QgsDebugMsg( QString( "getfeatureinfo: %1" ).arg( requestUrl.toString() ) );
        QNetworkRequest request( requestUrl );
        setAuthorization( request );
        mIdentifyReply = QgsNetworkAccessManager::instance()->get( request );
        connect( mIdentifyReply, SIGNAL( finished() ), this, SLOT( identifyReplyFinished() ) );

        while ( mIdentifyReply )
        {
          QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
        }

        results << mIdentifyResult;
      }
    }
  }

  QgsDebugMsg( "Exiting with: " + results.join( "\n------\n" ) );
  return results;
}

QString QgsWmsProvider::identifyAsText( const QgsPoint &point )
{
  if ( !mCapabilities.capability.request.getFeatureInfo.format.contains( "text/plain" ) )
    return tr( "Layer cannot be queried in plain text." );

  QStringList list = identifyAs( point, "text/plain" );

  if ( list.isEmpty() )
  {
    return tr( "Layer cannot be queried." );
  }
  else
  {
    return list.join( "\n-------------\n" );
  }
}

QString QgsWmsProvider::identifyAsHtml( const QgsPoint &point )
{
  QString format;

  foreach( QString f, mSupportedGetFeatureFormats )
  {
    if ( mCapabilities.capability.request.getFeatureInfo.format.contains( f ) )
    {
      format = f;
      break;
    }
  }

  Q_ASSERT( !format.isEmpty() );

  QStringList results = identifyAs( point, format );

  if ( format == "text/html" )
  {
    return "<table>\n<tr><td>" + results.join( "</td></tr>\n<tr><td>" ) + "</td></tr>\n</table>";
  }
  else
  {
    // TODO format text/xml
    return "<table>\n<tr><td><pre>\n" + results.join( "\n</pre></td></tr>\n<tr><td><pre>\n" ) + "\n</pre></td></tr>\n</table>";
  }
}

void QgsWmsProvider::identifyReplyFinished()
{
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
      connect( mIdentifyReply, SIGNAL( finished() ), this, SLOT( identifyReplyFinished() ) );

      return;
    }

    QVariant status = mIdentifyReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
    {
      QVariant phrase = mIdentifyReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
      mErrorFormat = "text/plain";
      mError = tr( "Map getfeatureinfo error %1: %2" ).arg( status.toInt() ).arg( phrase.toString() );
      emit statusChanged( mError );

      mIdentifyResult = "";
    }

    mIdentifyResult = QString::fromUtf8( mIdentifyReply->readAll() );
  }
  else
  {
    mIdentifyResult = tr( "ERROR: GetFeatureInfo failed" );
    QgsMessageLog::logMessage( tr( "Map getfeatureinfo error: %1 [%2]" ).arg( mIdentifyReply->errorString() ).arg( mIdentifyReply->url().toString() ), tr( "WMS" ) );
  }

  mIdentifyReply->deleteLater();
  mIdentifyReply = 0;
}


QgsCoordinateReferenceSystem QgsWmsProvider::crs()
{
  // TODO: implement
  return QgsCoordinateReferenceSystem();
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
  delete cachedImage;
  cachedImage = 0;
}

void QgsWmsProvider::setAuthorization( QNetworkRequest &request ) const
{
  if ( !mUserName.isNull() || !mPassword.isNull() )
  {
    request.setRawHeader( "Authorization", "Basic " + QString( "%1:%2" ).arg( mUserName ).arg( mPassword ).toAscii().toBase64() );
  }
}

QVector<QgsWmsSupportedFormat> QgsWmsProvider::supportedFormats()
{
  QVector<QgsWmsSupportedFormat> formats;
  QStringList mFormats, mLabels;


  QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();

  if ( supportedFormats.contains( "png" ) )
  {
    QgsWmsSupportedFormat p1 = { "image/png", "PNG" };
    QgsWmsSupportedFormat p2 = { "image/png; mode=24bit", "PNG24" }; // UMN mapserver
    QgsWmsSupportedFormat p3 = { "image/png8", "PNG8" }; // used by geoserver
    QgsWmsSupportedFormat p4 = { "png", "PNG" }; // used by french IGN geoportail
    QgsWmsSupportedFormat p5 = { "pngt", "PNGT" }; // used by french IGN geoportail

    formats << p1 << p2 << p3 << p4 << p5;
  }

  if ( supportedFormats.contains( "jpg" ) )
  {
    QgsWmsSupportedFormat j1 = { "image/jpeg", "JPEG" };
    QgsWmsSupportedFormat j2 = { "jpeg", "JPEG" }; // used by french IGN geoportail
    formats << j1 << j2;
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

  return formats;
}

void QgsWmsProvider::showMessageBox( const QString& title, const QString& text )
{
  QgsMessageOutput *message = QgsMessageOutput::createMessageOutput();
  message->setTitle( title );
  message->setMessage( text, QgsMessageOutput::MessageText );
  message->showMessage();
}

/**
 * Class factory to return a pointer to a newly created
 * QgsWmsProvider object
 */
QGISEXTERN QgsWmsProvider * classFactory( const QString *uri )
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

