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

/* $Id$ */

#define WMS_THRESHOLD 200  // time to wait for an answer without emitting dataChanged() 

#include "qgslogger.h"
#include "qgswmsprovider.h"

#include <math.h>

#include "qgscoordinatetransform.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>

#if QT_VERSION >= 0x40500
#include <QNetworkDiskCache>
#endif

#include <QUrl>
#include <QImage>
#include <QPainter>
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
    , mGetFeatureInfoUrlBase( 0 )
    , mLayerCount( -1 )
    , mTileReqNo( 0 )
    , mCacheHits( 0 )
    , mCacheMisses( 0 )
    , mErrors( 0 )
{
  if ( !smNAM )
  {
    QList<QByteArray> propertyNames = QCoreApplication::instance()->dynamicPropertyNames();
    foreach( QByteArray name, propertyNames )
    {
      QgsDebugMsg( QString( "property name: %1" ).arg( QString::fromUtf8( name ) ) );
    }

    if ( propertyNames.contains( "qgisNetworkAccessManager" ) )
    {
      smNAM = qobject_cast<QNetworkAccessManager*>( QCoreApplication::instance()->property( "qgisNetworkAccessManager" ).value<QObject*>() );

      if ( smNAM )
      {
        QNetworkProxy proxy = smNAM->proxy();
        QgsDebugMsg( QString( "proxy host:%1:%2 type:%3 user:%4 password:%5 capabilities:%6" )
                     .arg( proxy.hostName() ).arg( proxy.port() )
                     .arg( proxy.type() )
                     .arg( proxy.user() ).arg( proxy.password() )
                     .arg( proxy.capabilities() )
                   );

      }
    }

#if QT_VERSION >= 0x40500
    if ( !smNAM )
    {
      QgsDebugMsg( "application doesn't have a network access manager - creating wmscache" );
      smNAM = new QNetworkAccessManager( this );
      QNetworkDiskCache *ndc = new QNetworkDiskCache( this );
      ndc->setCacheDirectory( "wmsCache" );
      smNAM->setCache( ndc );
    }
#endif
  }

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

  baseUrl = prepareUri( httpuri );

  QgsDebugMsg( "baseUrl = " + baseUrl );

  QgsDebugMsg( "exiting constructor." );
}

void QgsWmsProvider::parseUri( QString uri )
{
  // Strip off and store the user name and password (if they exist)
  if ( !uri.startsWith( " http:" ) )
  {
    mUserName = "";
    mPassword = "";
    mTiled = false;
    mTileWidth = 0;
    mTileHeight = 0;
    mResolutions.clear();

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
      else if ( item.startsWith( "url=" ) )
      {
        // strip the authentication information from the front of the uri
        httpuri = item.mid( 4 );
        QgsDebugMsg( "set httpuri to " + httpuri );
      }
    }

  }
}

QString QgsWmsProvider::prepareUri( QString uri )
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

void QgsWmsProvider::addLayers( QStringList const &layers,
                                QStringList const &styles )
{
  QgsDebugMsg( "Entering with layer list of " + layers.join( ", " )
               + " and style list of " + styles.join( ", " ) );

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

  // Bounding box in WMS format
  QString bbox;

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

  QString url;
  QVector<QgsWmsDcpTypeProperty> dcpType = mCapabilities.capability.request.getMap.dcpType;
  if ( dcpType.size() < 1 )
  {
    url = baseUrl;
  }
  else
  {
    url = prepareUri( dcpType.front().http.get.onlineResource.xlinkHref );
  }

  cachedImage = new QImage( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
  cachedImage->fill( 0 );
  cachedViewExtent = viewExtent;
  cachedViewWidth = pixelWidth;
  cachedViewHeight = pixelHeight;

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

    // Warning: does not work with scientific notation
    bbox = QString( changeXY ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
           .arg( viewExtent.xMinimum(), 0, 'f' )
           .arg( viewExtent.yMinimum(), 0, 'f' )
           .arg( viewExtent.xMaximum(), 0, 'f' )
           .arg( viewExtent.yMaximum(), 0, 'f' );

    // Width in WMS format
    QString width;
    width = width.setNum( pixelWidth );

    // Height in WMS format
    QString height;
    height = height.setNum( pixelHeight );

    url += "SERVICE=WMS";
    url += "&VERSION=" + mCapabilities.version;
    url += "&REQUEST=GetMap";
    url += "&BBOX=" + bbox;
    url += "&" + crsKey + "=" + imageCrs;
    url += "&WIDTH=" + width;
    url += "&HEIGHT=" + height;
    url += "&LAYERS=" + layers;
    url += "&STYLES=" + styles;
    url += "&FORMAT=" + imageMimeType;

    //DPI parameter is accepted by QGIS mapserver (and ignored by the other WMS servers)
    if ( mDpi != -1 )
    {
      url += "&DPI=" + QString::number( mDpi );
    }

    //MH: jpeg does not support transparency and some servers complain if jpg and transparent=true
    if ( !imageMimeType.contains( "jpeg", Qt::CaseInsensitive ) && !imageMimeType.contains( "jpg", Qt::CaseInsensitive ) )
    {
      url += "&TRANSPARENT=true";
    }

    dcpType = mCapabilities.capability.request.getFeatureInfo.dcpType;
    if ( dcpType.size() < 1 )
    {
      mGetFeatureInfoUrlBase = baseUrl;
    }
    else
    {
      mGetFeatureInfoUrlBase = prepareUri( dcpType.front().http.get.onlineResource.xlinkHref );
    }

    // cache some details for if the user wants to do an identifyAsHtml() later
    mGetFeatureInfoUrlBase += "SERVICE=WMS";
    mGetFeatureInfoUrlBase += "&VERSION=" + mCapabilities.version;
    mGetFeatureInfoUrlBase += "&REQUEST=GetFeatureInfo";
    mGetFeatureInfoUrlBase += "&BBOX=" + bbox;
    mGetFeatureInfoUrlBase += "&" + crsKey + "=" + imageCrs;
    mGetFeatureInfoUrlBase += "&WIDTH=" + width;
    mGetFeatureInfoUrlBase += "&HEIGHT=" + height;
    mGetFeatureInfoUrlBase += "&LAYERS=" + layers;
    mGetFeatureInfoUrlBase += "&STYLES=" + styles;
    mGetFeatureInfoUrlBase += "&FORMAT=" + imageMimeType;

    if ( !imageMimeType.contains( "jpeg", Qt::CaseInsensitive ) && !imageMimeType.contains( "jpg", Qt::CaseInsensitive ) )
    {
      mGetFeatureInfoUrlBase += "&TRANSPARENT=true";
    }

    QgsDebugMsg( QString( "getmap: %1" ).arg( url ) );
    cacheReply = smNAM->get( QNetworkRequest( url ) );
    connect( cacheReply, SIGNAL( finished() ), this, SLOT( cacheReplyFinished() ) );
    connect( cacheReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( cacheReplyProgress( qint64, qint64 ) ) );

    mWaiting = true;

    QTime t;
    t.start();

    while ( cacheReply && t.elapsed() < WMS_THRESHOLD )
    {
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, WMS_THRESHOLD );
    }

    mWaiting = false;
  }
  else
  {
    mTileReqNo++;
    double vres = viewExtent.width() / pixelWidth;

    // find nearest resolution
    int i;
    for ( i = 0; i < mResolutions.size() && mResolutions[i] < vres; i++ )
      QgsDebugMsg( QString( "skipped res: %1:%2" ).arg( i ).arg( mResolutions[i] ) );

    if ( i == mResolutions.size() ||
         ( i > 0 && vres - mResolutions[i-1] < mResolutions[i] - vres ) )
    {
      QgsDebugMsg( "back to previous res" );
      i--;
    }

    double tres = mResolutions[i];
    double dx = mTileWidth * tres;
    double dy = mTileHeight * tres;

    // clip view extent to layer extent
    double xmin = std::max( viewExtent.xMinimum(), layerExtent.xMinimum() );
    double ymin = std::max( viewExtent.yMinimum(), layerExtent.yMinimum() );
    double xmax = std::min( viewExtent.xMaximum(), layerExtent.xMaximum() );
    double ymax = std::min( viewExtent.yMaximum(), layerExtent.yMaximum() );

    // snap to tile coordinates
    double x0 = floor(( xmin - layerExtent.xMinimum() ) / dx ) * dx + layerExtent.xMinimum();
    double y0 = floor(( ymin - layerExtent.yMinimum() ) / dy ) * dy + layerExtent.yMinimum();

#ifdef QGISDEBUG
    // calculate number of tiles
    int n = ceil(( xmax - xmin ) / dx ) * ceil(( ymax - ymin ) / dy );
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
                 .arg( dx, 0, 'f' ).arg( dy, 0, 'f' )
                 .arg( mTileWidth ).arg( mTileHeight )
                 .arg( tres, 0, 'f' )
               );
    QgsDebugMsg( QString( "tile number: %1x%2 = %3" )
                 .arg( ceil(( xmax - xmin ) / dx ) )
                 .arg( ceil(( ymax - ymin ) / dy ) )
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
    url += QString( "SERVICE=WMS&VERSION=%1&REQUEST=GetMap" ).arg( mCapabilities.version );

    // compose static request arguments.
    QString urlargs;
    urlargs += QString( "&%1=%2" ).arg( crsKey ).arg( imageCrs );
    urlargs += QString( "&WIDTH=%1" ).arg( mTileWidth );
    urlargs += QString( "&HEIGHT=%1" ).arg( mTileHeight );
    urlargs += QString( "&LAYERS=%1" ).arg( activeSubLayers.join( "," ) );
    urlargs += QString( "&STYLES=%1" ).arg( activeSubStyles.join( "," ) );
    urlargs += QString( "&FORMAT=%1" ).arg( imageMimeType );
    urlargs += QString( "&TILED=true" );

    int j = 0;
    for ( double y = y0; y < ymax; y += dy )
    {
      for ( double x = x0; x <= xmax; x += dx )
      {
        QString turl;
        turl += url;
        turl += QString( changeXY ? "&BBOX=%2,%1,%4,%3" : "&BBOX=%1,%2,%3,%4" )
                .arg( x, 0, 'f' )
                .arg( y, 0, 'f' )
                .arg( x + dx, 0, 'f' )
                .arg( y + dy, 0, 'f' );
        turl += urlargs;

        QNetworkRequest request( turl );
        QgsDebugMsg( QString( "tileRequest %1 %2/%3: %4" ).arg( mTileReqNo ).arg( j++ ).arg( n ).arg( turl ) );
        request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
        request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
        request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 0 ), mTileReqNo );
        request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ), j );
        request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 2 ), QRectF( x, y, dx, dy ) );

        QgsDebugMsg( QString( "gettile: %1" ).arg( turl ) );
        QNetworkReply *reply = smNAM->get( request );
        tileReplies << reply;
        connect( reply, SIGNAL( finished() ), this, SLOT( tileReplyFinished() ) );
      }
    }

    mWaiting = true;

    QTime t;
    t.start();

    // draw everything that is retrieved within a second
    // and the rest asynchronously
    while ( !tileReplies.isEmpty() && t.elapsed() < WMS_THRESHOLD )
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

void QgsWmsProvider::tileReplyFinished()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply*>( sender() );

#if QT_VERSION >= 0x40500
  bool fromCache = reply->attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool();
  if ( fromCache )
    mCacheHits++;
  else
    mCacheMisses++;
#endif
  int tileReqNo = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 0 ) ).toInt();
  int tileNo = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ) ).toInt();
  QRectF r = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 2 ) ).toRectF();

  QgsDebugMsg( QString( "tile reply %1 (%2) tile:%3 rect:%4,%5 %6x%7) fromcache:%8 error:%9" )
               .arg( tileReqNo ).arg( mTileReqNo ).arg( tileNo )
               .arg( r.left(), 0, 'f' ).arg( r.bottom(), 0, 'f' ).arg( r.width(), 0, 'f' ).arg( r.height(), 0, 'f' )
               .arg( fromCache )
               .arg( reply->errorString() )
             );

  if ( reply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      QNetworkRequest request( redirect.toUrl() );
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
      request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 0 ), tileReqNo );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ), tileNo );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 2 ), r );

      tileReplies.removeOne( reply );
      reply->deleteLater();

      QgsDebugMsg( QString( "redirected gettile: %1" ).arg( redirect.toString() ) );
      reply = smNAM->get( request );
      tileReplies << reply;

      connect( reply, SIGNAL( finished() ), this, SLOT( tileReplyFinished() ) );

      return;
    }

    QVariant status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
    {
      QVariant phrase = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
      mError = tr( "tile request err %1: %2" ).arg( status.toInt() ).arg( phrase.toString() );
      emit statusChanged( mError );

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

      QPainter p( cachedImage );
      p.drawImage( dst, myLocalImage );

      // p.drawRect( dst ); // show tile bounds
      // p.drawText( dst.center(), QString( "(%1)\n%2,%3\n%4x%5" ).arg( tileNo ).arg( r.left() ).arg( r.bottom() ).arg( r.width() ).arg( r.height() ) );
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

#if QGISDEBUG
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
      cacheReply = smNAM->get( QNetworkRequest( redirect.toUrl() ) );
      connect( cacheReply, SIGNAL( finished() ), this, SLOT( cacheReplyFinished() ) );
      return;
    }

    QVariant status = cacheReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
    {
      QVariant phrase = cacheReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
      mError = tr( "map request error %1: %2" ).arg( status.toInt() ).arg( phrase.toString() );
      emit statusChanged( mError );

      cacheReply->deleteLater();
      cacheReply = 0;

      return;
    }

    {
      QImage myLocalImage = QImage::fromData( cacheReply->readAll() );
      QPainter p( cachedImage );
      p.drawImage( 0, 0, myLocalImage );
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
    QString url = baseUrl + "SERVICE=WMS&REQUEST=GetCapabilities";

    QNetworkRequest request( url );
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

    QgsDebugMsg( QString( "getcapabilities: %1" ).arg( url ) );
    mCapabilitiesReply = smNAM->get( request );

    connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( capabilitiesReplyFinished() ) );
    connect( mCapabilitiesReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( capabilitiesReplyProgress( qint64, qint64 ) ) );

    while ( mCapabilitiesReply )
    {
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
    }

    if ( httpcapabilitiesresponse.isEmpty() )
    {
      QgsDebugMsg( "empty capabilities: " + mError );
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
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
      request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

      mCapabilitiesReply->deleteLater();
      QgsDebugMsg( QString( "redirected getcapabilities: %1" ).arg( redirect.toString() ) );
      mCapabilitiesReply = smNAM->get( request );

      connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( capabilitiesReplyFinished() ) );
      connect( mCapabilitiesReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( capabilitiesReplyProgress( qint64, qint64 ) ) );
      return;
    }

    httpcapabilitiesresponse = mCapabilitiesReply->readAll();

    if ( httpcapabilitiesresponse.isEmpty() )
    {
      mError = tr( "empty of capabilities: %1" ).arg( mCapabilitiesReply->errorString() );
    }
  }
  else
  {
    mError = tr( "Download of capabilities failed: %1" ).arg( mCapabilitiesReply->errorString() );
    QgsDebugMsg( "error: " + mError );
    httpcapabilitiesresponse.clear();
  }


  mCapabilitiesReply->deleteLater();
  mCapabilitiesReply = 0;
}

void QgsWmsProvider::capabilitiesReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  emit statusChanged( tr( "%1 of %2 bytes of capabilities downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) ) );
}

void QgsWmsProvider::cacheReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  emit statusChanged( tr( "%1 of %2 bytes of map downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) ) );
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
    mError = tr( "Could not get WMS capabilities: %1 at line %2 column %3\n" )
             .arg( errorMsg ).arg( errorLine ).arg( errorColumn )
             + tr( "This is probably due to an incorrect WMS Server URL." );

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
    mError = tr( "Could not get WMS capabilities in the "
                 "expected format (DTD): no %1 or %2 found\n" )
             .arg( "WMS_Capabilities" ).arg( "WMT_MS_Capabilities" )
             + tr( "This is probably due to an incorrect WMS Server URL." );

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

      if ( e1.tagName() == "Title" )
      {
        serviceProperty.title = e1.text();
      }
      else if ( e1.tagName() == "Abstract" )
      {
        serviceProperty.abstract = e1.text();
      }
      else if ( e1.tagName() == "KeywordList" )
      {
        parseKeywordList( e1, serviceProperty.keywordList );
      }
      else if ( e1.tagName() == "OnlineResource" )
      {
        parseOnlineResource( e1, serviceProperty.onlineResource );
      }
      else if ( e1.tagName() == "ContactInformation" )
      {
        parseContactInformation( e1, serviceProperty.contactInformation );
      }
      else if ( e1.tagName() == "Fees" )
      {
        serviceProperty.fees = e1.text();
      }
      else if ( e1.tagName() == "AccessConstraints" )
      {
        serviceProperty.accessConstraints = e1.text();
      }
      else if ( e1.tagName() == "LayerLimit" )
      {
        serviceProperty.layerLimit = e1.text().toUInt();
      }
      else if ( e1.tagName() == "MaxWidth" )
      {
        serviceProperty.maxWidth = e1.text().toUInt();
      }
      else if ( e1.tagName() == "MaxHeight" )
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
      QgsDebugMsg( "  "  + e1.tagName() ); // the node really is an element.

      if ( e1.tagName() == "Request" )
      {
        parseRequest( e1, capabilityProperty.request );
      }
      else if ( e1.tagName() == "Layer" )
      {
        parseLayer( e1, capabilityProperty.layer );
      }
      else if ( e1.tagName() == "VendorSpecificCapabilities" )
      {
        for ( int i = 0; i < e1.childNodes().size(); i++ )
        {
          QDomNode n2 = e1.childNodes().item( i );
          QDomElement e2 = n2.toElement();
          if ( e2.tagName() == "TileSet" )
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
      if ( e1.tagName() == "ContactPerson" )
      {
        contactPersonPrimaryProperty.contactPerson = e1.text();
      }
      else if ( e1.tagName() == "ContactOrganization" )
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
      if ( e1.tagName() == "AddressType" )
      {
        contactAddressProperty.addressType = e1.text();
      }
      else if ( e1.tagName() == "Address" )
      {
        contactAddressProperty.address = e1.text();
      }
      else if ( e1.tagName() == "City" )
      {
        contactAddressProperty.city = e1.text();
      }
      else if ( e1.tagName() == "StateOrProvince" )
      {
        contactAddressProperty.stateOrProvince = e1.text();
      }
      else if ( e1.tagName() == "PostCode" )
      {
        contactAddressProperty.postCode = e1.text();
      }
      else if ( e1.tagName() == "Country" )
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
      if ( e1.tagName() == "ContactPersonPrimary" )
      {
        parseContactPersonPrimary( e1, contactInformationProperty.contactPersonPrimary );
      }
      else if ( e1.tagName() == "ContactPosition" )
      {
        contactInformationProperty.contactPosition = e1.text();
      }
      else if ( e1.tagName() == "ContactAddress" )
      {
        parseContactAddress( e1, contactInformationProperty.contactAddress );
      }
      else if ( e1.tagName() == "ContactVoiceTelephone" )
      {
        contactInformationProperty.contactVoiceTelephone = e1.text();
      }
      else if ( e1.tagName() == "ContactFacsimileTelephone" )
      {
        contactInformationProperty.contactFacsimileTelephone = e1.text();
      }
      else if ( e1.tagName() == "ContactElectronicMailAddress" )
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
      if ( e1.tagName() == "Keyword" )
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
      if ( e1.tagName() == "OnlineResource" )
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
      if ( e1.tagName() == "OnlineResource" )
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
      if ( e1.tagName() == "Format" )
      {
        QgsDebugMsg( "      Format." );
        operationType.format += e1.text();
      }
      else if ( e1.tagName() == "DCPType" )
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
      if ( e1.tagName() == "Format" )
      {
        legendUrlProperty.format = e1.text();
      }
      else if ( e1.tagName() == "OnlineResource" )
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
      if ( e1.tagName() == "Name" )
      {
        styleProperty.name = e1.text();
      }
      else if ( e1.tagName() == "Title" )
      {
        styleProperty.title = e1.text();
      }
      else if ( e1.tagName() == "Abstract" )
      {
        styleProperty.abstract = e1.text();
      }
      else if ( e1.tagName() == "LegendURL" )
      {
        // TODO
      }
      else if ( e1.tagName() == "StyleSheetURL" )
      {
        // TODO
      }
      else if ( e1.tagName() == "StyleURL" )
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
      QgsDebugMsg( "    "  + e1.tagName() ); // the node really is an element.

      if ( e1.tagName() == "Layer" )
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
      else if ( e1.tagName() == "Name" )
      {
        layerProperty.name = e1.text();
      }
      else if ( e1.tagName() == "Title" )
      {
        layerProperty.title = e1.text();
      }
      else if ( e1.tagName() == "Abstract" )
      {
        layerProperty.abstract = e1.text();
      }
      else if ( e1.tagName() == "KeywordList" )
      {
        parseKeywordList( e1, layerProperty.keywordList );
      }
      else if ( e1.tagName() == "CRS" )
      {
        layerProperty.crs.push_back( e1.text() );
      }
      else if ( e1.tagName() == "SRS" )      // legacy from earlier versions of WMS
      {
        // CRS can contain several definitions separated by whitespace
        // though this was deprecated in WMS 1.1.1
        QStringList srsList = e1.text().split( QRegExp( "\\s+" ) );

        QStringList::const_iterator i;
        for ( i = srsList.constBegin(); i != srsList.constEnd(); ++i )
        {
          layerProperty.crs.push_back( *i );
        }
      }
      else if ( e1.tagName() == "LatLonBoundingBox" )      // legacy from earlier versions of WMS
      {
        layerProperty.ex_GeographicBoundingBox = QgsRectangle(
              e1.attribute( "minx" ).toDouble(),
              e1.attribute( "miny" ).toDouble(),
              e1.attribute( "maxx" ).toDouble(),
              e1.attribute( "maxy" ).toDouble()
            );
      }
      else if ( e1.tagName() == "EX_GeographicBoundingBox" ) //for WMS 1.3
      {
        QDomElement wBoundLongitudeElem = n1.namedItem( "westBoundLongitude" ).toElement();
        QDomElement eBoundLongitudeElem = n1.namedItem( "eastBoundLongitude" ).toElement();
        QDomElement sBoundLatitudeElem = n1.namedItem( "southBoundLatitude" ).toElement();
        QDomElement nBoundLatitudeElem = n1.namedItem( "northBoundLatitude" ).toElement();
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
      else if ( e1.tagName() == "BoundingBox" )
      {
        // TODO: overwrite inherited
        QgsWmsBoundingBoxProperty bbox;
        bbox.box = QgsRectangle( e1.attribute( "minx" ).toDouble(),
                                 e1.attribute( "miny" ).toDouble(),
                                 e1.attribute( "maxx" ).toDouble(),
                                 e1.attribute( "maxy" ).toDouble()
                               );
        bbox.crs = e1.attribute( "CRS" );
        layerProperty.boundingBox.push_back( bbox );
      }
      else if ( e1.tagName() == "Dimension" )
      {
        // TODO
      }
      else if ( e1.tagName() == "Attribution" )
      {
        // TODO
      }
      else if ( e1.tagName() == "AuthorityURL" )
      {
        // TODO
      }
      else if ( e1.tagName() == "Identifier" )
      {
        // TODO
      }
      else if ( e1.tagName() == "MetadataURL" )
      {
        // TODO
      }
      else if ( e1.tagName() == "DataURL" )
      {
        // TODO
      }
      else if ( e1.tagName() == "FeatureListURL" )
      {
        // TODO
      }
      else if ( e1.tagName() == "Style" )
      {
        QgsWmsStyleProperty styleProperty;

        parseStyle( e1, styleProperty );

        layerProperty.style.push_back( styleProperty );
      }
      else if ( e1.tagName() == "MinScaleDenominator" )
      {
        // TODO
      }
      else if ( e1.tagName() == "MaxScaleDenominator" )
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

  if ( layerProperty.layer.empty() )
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
  else
  {
    mLayerParentNames[ layerProperty.orderId ] = QStringList() << layerProperty.name << layerProperty.title << layerProperty.abstract;
  }

//  QgsDebugMsg("exiting.");
}

void QgsWmsProvider::parseTileSetProfile( QDomElement const &e, QVector<QgsWmsTileSetProfile> &tileSet )
{
  QgsWmsTileSetProfile tsp;

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QgsDebugMsg( "    "  + e1.tagName() ); // the node really is an element.

      if ( e1.tagName() == "Layers" )
      {
        tsp.layers << e1.text();
      }
      else if ( e1.tagName() == "Styles" )
      {
        tsp.styles << e1.text();
      }
      else if ( e1.tagName() == "Width" )
      {
        tsp.tileWidth = e1.text().toInt();
      }
      else if ( e1.tagName() == "Height" )
      {
        tsp.tileHeight = e1.text().toInt();
      }
      else if ( e1.tagName() == "SRS" )
      {
        tsp.crs = e1.text();
      }
      else if ( e1.tagName() == "Format" )
      {
        tsp.format = e1.text();
      }
      else if ( e1.tagName() == "BoundingBox" )
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
      else if ( e1.tagName() == "Resolutions" )
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
    mError = tr( "Could not get WMS Service Exception at %1: %2 at line %3 column %4" )
             .arg( baseUrl )
             .arg( errorMsg )
             .arg( errorLine )
             .arg( errorColumn );

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

      if ( e.tagName() == "ServiceException" )
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

  // set up friendly descriptions for the service exception
  if ( seCode == "InvalidFormat" )
  {
    mError = tr( "Request contains a Format not offered by the server." );
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

  QgsDebugMsg( "composed error message '"  + mError  + "'." );
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

  if ( mTiled )
  {
    QString layers = activeSubLayers.join( "," );
    QString styles = activeSubStyles.join( "," );

    QgsDebugMsg( QString( "looking for tileset with layers=%1, styles=%2 and crs=%3." )
                 .arg( layers ).arg( styles ).arg( imageCrs ) );
    for ( int i = 0; i < tilesetsSupported.size(); i++ )
    {
      if ( tilesetsSupported[i].layers.join( "," ) == layers &&
           tilesetsSupported[i].styles.join( "," ) == styles &&
           tilesetsSupported[i].boundingBox.crs == imageCrs )
      {
        layerExtent = tilesetsSupported[i].boundingBox.box;
        return true;
      }

      QgsDebugMsg( QString( "mismatch layers=%1, styles=%2 and crs=%3." )
                   .arg( tilesetsSupported[i].layers.join( "," ) )
                   .arg( tilesetsSupported[i].styles.join( "," ) )
                   .arg( tilesetsSupported[i].boundingBox.crs ) );
    }

    QgsDebugMsg( "no extent for layer" );
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
  int capability = 0;
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

  // Collect all the test results into one bitmask
  if ( canIdentify )
  {
    capability = ( capability | QgsRasterDataProvider::Identify );
  }

  QgsDebugMsg( "exiting with '"  + QString( capability )  + "'." );

  return capability;
}


QString QgsWmsProvider::metadata()
{
  QString myMetadataQString = "";

  myMetadataQString += "<tr><td>";

  myMetadataQString += "<a href=\"#serverproperties\">";
  myMetadataQString += tr( "Server Properties" );
  myMetadataQString += "</a> ";

  myMetadataQString += "<a href=\"#layerproperties\">";
  myMetadataQString += tr( "Layer Properties" );
  myMetadataQString += "</a> ";

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
  myMetadataQString += "<tr><td bgcolor=\"gray\"><a name=\"serverproperties\"></a>";
  myMetadataQString += tr( "Server Properties" );
  myMetadataQString += "</td></tr>";

  // Use a nested table
  myMetadataQString += "<tr><td bgcolor=\"white\">";
  myMetadataQString += "<table width=\"100%\">";

  // Table header
  myMetadataQString += "<tr><th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr( "Property" ) + "</font>";
  myMetadataQString += "</th>";
  myMetadataQString += "<th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr( "Value" ) + "</font>";
  myMetadataQString += "</th></tr>";

  // WMS Version
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "WMS Version" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.version;
  myMetadataQString += "</td></tr>";

  // Service Title
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Title" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.title;
  myMetadataQString += "</td></tr>";

  // Service Abstract
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Abstract" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.abstract;
  myMetadataQString += "</td></tr>";

  // Service Keywords
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Keywords" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.keywordList.join( "<br />" );
  myMetadataQString += "</td></tr>";

  // Service Online Resource
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Online Resource" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += "-";
  myMetadataQString += "</td></tr>";

  // Service Contact Information
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Contact Person" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.contactInformation.contactPersonPrimary.contactPerson;
  myMetadataQString += "<br />";
  myMetadataQString += mCapabilities.service.contactInformation.contactPosition;
  myMetadataQString += "<br />";
  myMetadataQString += mCapabilities.service.contactInformation.contactPersonPrimary.contactOrganization;
  myMetadataQString += "</td></tr>";

  // Service Fees
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Fees" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.fees;
  myMetadataQString += "</td></tr>";

  // Service Access Constraints
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Access Constraints" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.accessConstraints;
  myMetadataQString += "</td></tr>";

  // GetMap Request Formats
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Image Formats" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.capability.request.getMap.format.join( "<br />" );
  myMetadataQString += "</td></tr>";

  // GetFeatureInfo Request Formats
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Identify Formats" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.capability.request.getFeatureInfo.format.join( "<br />" );
  myMetadataQString += "</td></tr>";

  // Layer Count (as managed by this provider)
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Layer Count" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += QString::number( layersSupported.size() );
  myMetadataQString += "</td></tr>";

  // Tileset Count (as managed by this provider)
  if ( tilesetsSupported.size() > 0 )
  {
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Tileset Count" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += QString::number( tilesetsSupported.size() );
    myMetadataQString += "</td></tr>";
  }

  // Base URL
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "GetFeatureInfoUrl" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mGetFeatureInfoUrlBase;
  myMetadataQString += "</td></tr>";

  // Close the nested table
  myMetadataQString += "</table>";
  myMetadataQString += "</td></tr>";

  // Layer properties
  myMetadataQString += "<tr><td bgcolor=\"gray\"><a name=\"layerproperties\"></a>";
  myMetadataQString += tr( "Layer Properties:" );
  myMetadataQString += "</td></tr>";

  // Iterate through layers

  for ( int i = 0; i < layersSupported.size(); i++ )
  {
    // TODO: Handle nested layers
    QString layerName = layersSupported[i].name;   // for aesthetic convenience

    // Layer Properties section
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += layerName;
    myMetadataQString += "</td></tr>";

    // Use a nested table
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += "<table width=\"100%\">";

    // Table header
    myMetadataQString += "<tr><th bgcolor=\"black\">";
    myMetadataQString += "<font color=\"white\">" + tr( "Property" ) + "</font>";
    myMetadataQString += "</th>";
    myMetadataQString += "<th bgcolor=\"black\">";
    myMetadataQString += "<font color=\"white\">" + tr( "Value" ) + "</font>";
    myMetadataQString += "</th></tr>";

    bool selected = !mTiled && activeSubLayers.indexOf( layerName ) >= 0;

    // Layer Selectivity (as managed by this provider)
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Selected" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += selected ? tr( "Yes" ) : tr( "No" );
    myMetadataQString += "</td></tr>";

    // Layer Visibility (as managed by this provider)
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Visibility" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    if ( selected )
    {
      myMetadataQString += activeSubLayerVisibility.find( layerName ).value() ? tr( "Visible" ) : tr( "Hidden" );
    }
    else
    {
      myMetadataQString += tr( "n/a" );
    }
    myMetadataQString += "</td></tr>";

    // Layer Title
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Title" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += layersSupported[i].title;
    myMetadataQString += "</td></tr>";

    // Layer Abstract
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Abstract" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += layersSupported[i].abstract;
    myMetadataQString += "</td></tr>";

    // Layer Queryability
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Can Identify" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += (( layersSupported[i].queryable ) ? tr( "Yes" ) : tr( "No" ) );
    myMetadataQString += "</td></tr>";

    // Layer Opacity
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Can be Transparent" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += (( layersSupported[i].opaque ) ? tr( "No" ) : tr( "Yes" ) );
    myMetadataQString += "</td></tr>";

    // Layer Subsetability
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Can Zoom In" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += (( layersSupported[i].noSubsets ) ? tr( "No" ) : tr( "Yes" ) );
    myMetadataQString += "</td></tr>";

    // Layer Server Cascade Count
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Cascade Count" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += QString::number( layersSupported[i].cascaded );
    myMetadataQString += "</td></tr>";

    // Layer Fixed Width
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Fixed Width" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += QString::number( layersSupported[i].fixedWidth );
    myMetadataQString += "</td></tr>";

    // Layer Fixed Height
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Fixed Height" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += QString::number( layersSupported[i].fixedHeight );
    myMetadataQString += "</td></tr>";

    // Layer Fixed Height
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "WGS 84 Bounding Box" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += extentForLayer[ layerName ].toString();
    myMetadataQString += "</td></tr>";

    // Layer Coordinate Reference Systems
    for ( int j = 0; j < std::min( layersSupported[i].crs.size(), 10 ); j++ )
    {
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Available in CRS" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += layersSupported[i].crs[j];
      myMetadataQString += "</td></tr>";
    }

    if ( layersSupported[i].crs.size() > 10 )
    {
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Available in CRS" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += tr( "(and more)" );
      myMetadataQString += "</td></tr>";
    }

    // Layer Styles
    for ( int j = 0; j < layersSupported[i].style.size(); j++ )
    {
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Available in style" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td>";

      // Nested table.
      myMetadataQString += "<table width=\"100%\">";

      // Layer Style Name
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Name" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += layersSupported[i].style[j].name;
      myMetadataQString += "</td></tr>";

      // Layer Style Title
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Title" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += layersSupported[i].style[j].title;
      myMetadataQString += "</td></tr>";

      // Layer Style Abstract
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Abstract" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += layersSupported[i].style[j].abstract;
      myMetadataQString += "</td></tr>";

      // Close the nested table
      myMetadataQString += "</table>";
      myMetadataQString += "</td></tr>";
    }

    // Close the nested table
    myMetadataQString += "</table>";
    myMetadataQString += "</td></tr>";
  } // for each layer

  // Tileset properties
  if ( tilesetsSupported.size() > 0 )
  {
    myMetadataQString += "<tr><td bgcolor=\"gray\"><a name=\"tilesetproperties\"></a>";
    myMetadataQString += tr( "Tileset Properties" );
    myMetadataQString += "</td></tr>";

    // Iterate through tilesets
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += "<table width=\"100%\">";

    for ( int i = 0; i < tilesetsSupported.size(); i++ )
    {
      myMetadataQString += "<tr><td bgcolor=\"white\" colspan=\"2\">";
      myMetadataQString += tilesetsSupported[i].layers.join( ", " );
      myMetadataQString += "</td></tr>";

      // Table header
      myMetadataQString += "<tr><th bgcolor=\"black\">";
      myMetadataQString += "<font color=\"white\">" + tr( "Property" ) + "</font>";
      myMetadataQString += "</th>";
      myMetadataQString += "<th bgcolor=\"black\">";
      myMetadataQString += "<font color=\"white\">" + tr( "Value" ) + "</font>";
      myMetadataQString += "</th></tr>";

      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Selected" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += mTiled && tilesetsSupported[i].layers.join( "," ) == activeSubLayers.join( "," ) ? tr( "Yes" ) : tr( "No" );
      myMetadataQString += "</td></tr>";

      if ( tilesetsSupported[i].styles.size() > 0 )
      {
        myMetadataQString += "<tr><td bgcolor=\"gray\">";
        myMetadataQString += tr( "Styles" );
        myMetadataQString += "</td>";
        myMetadataQString += "<td bgcolor=\"gray\">";
        myMetadataQString += tilesetsSupported[i].styles.join( ", " );
        myMetadataQString += "</td></tr>";
      }

      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "CRS" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += tilesetsSupported[i].boundingBox.crs;
      myMetadataQString += "</td></tr>";

      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Bounding Box" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += tilesetsSupported[i].boundingBox.box.toString();
      myMetadataQString += "</td></tr>";

      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Available in Resolutions" );
      myMetadataQString += "</td><td bgcolor=\"gray\">";

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
      myMetadataQString += "<tr><td bgcolor=\"gray\"><a name=\"cachestats\"></a>";
      myMetadataQString += tr( "Cache stats" );
      myMetadataQString += "</td></tr>";

      // Iterate through tilesets
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += "<table width=\"100%\">";

      myMetadataQString += "<tr><th bgcolor=\"black\">";
      myMetadataQString += "<font color=\"white\">" + tr( "Property" ) + "</font>";
      myMetadataQString += "</th>";
      myMetadataQString += "<th bgcolor=\"black\">";
      myMetadataQString += "<font color=\"white\">" + tr( "Value" ) + "</font>";
      myMetadataQString += "</th></tr>";

      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Hits" );
      myMetadataQString += "</td><td bgcolor=\"gray\">";
      myMetadataQString += QString::number( mCacheHits );
      myMetadataQString += "</td></tr>";

      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Misses" );
      myMetadataQString += "</td><td bgcolor=\"gray\">";
      myMetadataQString += QString::number( mCacheMisses );
      myMetadataQString += "</td></tr>";

      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Errors" );
      myMetadataQString += "</td><td bgcolor=\"gray\">";
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


QString QgsWmsProvider::identifyAsText( const QgsPoint& point )
{
  QgsDebugMsg( "Entering." );

  // Collect which layers to query on

  QStringList queryableLayers = QStringList();
  QString text = "";

  // Test for which layers are suitable for querying with
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
        QgsDebugMsg( "Layer '" + *it + "' is queryable." );
        // Compose request to WMS server

        QString requestUrl = mGetFeatureInfoUrlBase;
        QString layer = QUrl::toPercentEncoding( *it );

        //! \todo Need to tie this into the options provided by GetCapabilities
        requestUrl += QString( "&QUERY_LAYERS=%1" ).arg( layer );
        requestUrl += QString( "&INFO_FORMAT=text/plain&X=%1&Y=%2" )
                      .arg( point.x() ).arg( point.y() );

        // X,Y in WMS 1.1.1; I,J in WMS 1.3.0
        //   requestUrl += QString( "&I=%1&J=%2" ).arg( point.x() ).arg( point.y() );

        QgsDebugMsg( QString( "getfeatureinfo: %1" ).arg( requestUrl ) );
        mIdentifyReply = smNAM->get( QNetworkRequest( requestUrl ) );
        connect( mIdentifyReply, SIGNAL( finished() ), this, SLOT( identifyReplyFinished() ) );

        while ( mIdentifyReply )
        {
          QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
        }

        text += "---------------\n" + mIdentifyResult;
      }
    }
  }

  if ( text.isEmpty() )
  {
    // No layers were queryably. This can happen if identify tool was
    // active when this non-queriable layer was selected.
    // Return a descriptive text.

    text = tr( "Layer cannot be queried." );
  }

  QgsDebugMsg( "Exiting with: " + text );
  return text;
}

void QgsWmsProvider::identifyReplyFinished()
{
  if ( mIdentifyReply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = mIdentifyReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      emit statusChanged( QString( "identify request redirected to %1" ).arg( redirect.toString() ) );
      emit statusChanged( tr( "identify request redirected." ) );

      mIdentifyReply->deleteLater();

      QgsDebugMsg( QString( "redirected getfeatureinfo: %1" ).arg( redirect.toString() ) );
      mIdentifyReply = smNAM->get( QNetworkRequest( redirect.toUrl() ) );
      connect( mIdentifyReply, SIGNAL( finished() ), this, SLOT( identifyReplyFinished() ) );

      return;
    }

    QVariant status = mIdentifyReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
    {
      QVariant phrase = mIdentifyReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
      mError = tr( "map request error %1: %2" ).arg( status.toInt() ).arg( phrase.toString() );
      emit statusChanged( mError );

      mIdentifyResult = "";
    }

    mIdentifyResult = QString::fromUtf8( mIdentifyReply->readAll() );
  }
  else
  {
    mIdentifyResult = "";
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


QString  QgsWmsProvider::name() const
{
  return WMS_KEY;
} //  QgsWmsProvider::name()



QString  QgsWmsProvider::description() const
{
  return WMS_DESCRIPTION;
} //  QgsWmsProvider::description()

QNetworkAccessManager *QgsWmsProvider::smNAM = 0;


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
