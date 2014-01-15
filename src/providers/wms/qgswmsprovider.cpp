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

#include "qgslogger.h"
#include "qgswmsprovider.h"
#include "qgswmsconnection.h"
#include "qgscoordinatetransform.h"
#include "qgsdatasourceuri.h"
#include "qgsfeaturestore.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrasterlayer.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmessageoutput.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsnetworkreplyparser.h"
#include "qgsgml.h"
#include "qgsgmlschema.h"
#include "qgswmscapabilities.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>

#if QT_VERSION >= 0x40500
#include <QNetworkDiskCache>
#endif

#if QT_VERSION >= 0x40600
#include <QtXmlPatterns/QXmlSchema>
#include <QtXmlPatterns/QXmlSchemaValidator>
#endif

#include <QUrl>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QSet>
#include <QSettings>
#include <QEventLoop>
#include <QCoreApplication>
#include <QTime>
#include <QThread>

#ifdef QGISDEBUG
#include <QFile>
#include <QDir>
#endif

#define ERR(message) QGS_ERROR_MESSAGE(message,"WMS provider")
#define SRVERR(message) QGS_ERROR_MESSAGE(message,"WMS server")
#define ERROR(message) QgsError(message,"WMS provider")

static QString WMS_KEY = "wms";
static QString WMS_DESCRIPTION = "OGC Web Map Service version 1.3 data provider";

static QString DEFAULT_LATLON_CRS = "CRS:84";

QgsWmsProvider::QgsWmsProvider( QString const& uri, const QgsWmsCapabilities* capabilities )
    : QgsRasterDataProvider( uri )
    , mHttpGetLegendGraphicResponse( 0 )
    , mGetLegendGraphicImage()
    , mGetLegendGraphicScale( 0 )
    , mImageCrs( DEFAULT_LATLON_CRS )
    , mCachedImage( 0 )
    , mCacheReply( 0 )
    , mCachedViewExtent( 0 )
    , mCoordinateTransform( 0 )
    , mExtentDirty( true )
    , mGetFeatureInfoUrlBase( "" )
    , mTileReqNo( 0 )
    , mCacheHits( 0 )
    , mCacheMisses( 0 )
    , mErrors( 0 )
    , mTileLayer( 0 )
    , mTileMatrixSet( 0 )
    , mNAM( 0 )
{
  QgsDebugMsg( "constructing with uri '" + uri + "'." );

  mSupportedGetFeatureFormats = QStringList() << "text/html" << "text/plain" << "text/xml" << "application/vnd.ogc.gml";

  mValid = false;

  // URL may contain username/password information for a WMS
  // requiring authentication. In this case the URL is prefixed
  // with username=user,password=pass,url=http://xxx.xxx.xx/yyy...

  if ( !mSettings.parseUri( uri ) )
  {
    appendError( ERR( tr( "Cannot parse URI" ) ) );
    return;
  }

  addLayers();

  // if there are already parsed capabilities, use them!
  if ( capabilities )
    mCaps = *capabilities;

  // Make sure we have capabilities - other functions here may need them
  if ( !retrieveServerCapabilities() )
  {
    return;
  }

  // setImageCrs is using mTiled !!!
  if ( !setImageCrs( mSettings.mCrsId ) )
  {
    appendError( ERR( tr( "Cannot set CRS" ) ) );
    return;
  }
  mCrs.createFromOgcWmsCrs( mSettings.mCrsId );

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
  if ( uri.contains( "SERVICE=WMTS" ) || uri.contains( "/WMTSCapabilities.xml" ) )
  {
    return uri;
  }

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
  if ( mCachedImage )
  {
    delete mCachedImage;
    mCachedImage = 0;
  }

  if ( mCoordinateTransform )
  {
    delete mCoordinateTransform;
    mCoordinateTransform = 0;
  }

  if ( mCacheReply )
  {
    mCacheReply->deleteLater();
    mCacheReply = 0;
  }

  while ( !mTileReplies.isEmpty() )
  {
    mTileReplies.takeFirst()->deleteLater();
  }

  mNAM->deleteLater();

}

QgsRasterInterface * QgsWmsProvider::clone() const
{
  QgsWmsProvider * provider = new QgsWmsProvider( dataSourceUri(), mCaps.isValid() ? &mCaps : 0 );
  return provider;
}


QString QgsWmsProvider::getMapUrl() const
{
  return mCaps.mCapabilities.capability.request.getMap.dcpType.size() == 0
         ? mSettings.mBaseUrl
         : prepareUri( mCaps.mCapabilities.capability.request.getMap.dcpType.front().http.get.onlineResource.xlinkHref );
}


QString QgsWmsProvider::getFeatureInfoUrl() const
{
  return mCaps.mCapabilities.capability.request.getFeatureInfo.dcpType.size() == 0
         ? mSettings.mBaseUrl
         : prepareUri( mCaps.mCapabilities.capability.request.getFeatureInfo.dcpType.front().http.get.onlineResource.xlinkHref );
}

QString QgsWmsProvider::getTileUrl() const
{
  if ( mCaps.mCapabilities.capability.request.getTile.dcpType.size() == 0 ||
       ( mCaps.mCapabilities.capability.request.getTile.allowedEncodings.size() > 0 &&
         !mCaps.mCapabilities.capability.request.getTile.allowedEncodings.contains( "KVP" ) ) )
  {
    return QString::null;
  }
  else
  {
    return prepareUri( mCaps.mCapabilities.capability.request.getTile.dcpType.front().http.get.onlineResource.xlinkHref );
  }
}

void QgsWmsProvider::addLayers()
{
  QgsDebugMsg( "Entering: layers:" + mSettings.mActiveSubLayers.join( ", " ) + ", styles:" + mSettings.mActiveSubStyles.join( ", " ) );

  if ( mSettings.mActiveSubLayers.size() != mSettings.mActiveSubStyles.size() )
  {
    QgsMessageLog::logMessage( tr( "Number of layers and styles don't match" ), tr( "WMS" ) );
    mValid = false;
    return;
  }

  // Set the visibility of these new layers on by default
  foreach ( const QString &layer, mSettings.mActiveSubLayers )
  {
    mActiveSubLayerVisibility[ layer ] = true;
    QgsDebugMsg( "set visibility of layer '" + layer + "' to true." );
  }

  // now that the layers have changed, the extent will as well.
  mExtentDirty = true;

  if ( mSettings.mTiled )
    mTileLayer = 0;

  QgsDebugMsg( "Exiting." );
}

void QgsWmsProvider::setConnectionName( QString const &connName )
{
  mConnectionName = connName;
}

void QgsWmsProvider::setLayerOrder( QStringList const &layers )
{
  QgsDebugMsg( "Entering." );

  // is this ever used anywhere???

  mSettings.mActiveSubLayers = layers;

  QgsDebugMsg( "Exiting." );
}


void QgsWmsProvider::setSubLayerVisibility( QString const & name, bool vis )
{
  mActiveSubLayerVisibility[name] = vis;
}


bool QgsWmsProvider::setImageCrs( QString const & crs )
{
  QgsDebugMsg( "Setting image CRS to " + crs + "." );

  if ( crs != mImageCrs && !crs.isEmpty() )
  {
    // delete old coordinate transform as it is no longer valid
    if ( mCoordinateTransform )
    {
      delete mCoordinateTransform;
      mCoordinateTransform = 0;
    }

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
    if ( mCaps.mTileLayersSupported.size() == 0 )
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
          QgsDebugMsg( QString( "tile matrix set '%1' has crs %2 instead of %3." ).arg( tms ).arg( mCaps.mTileMatrixSets[ tms ].crs ).arg( mImageCrs ) );
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
      qSort( keys );
      foreach ( double key, keys )
      {
        resolutions << key;
      }
    }
    else
    {
      QgsDebugMsg( QString( "Expected tile matrix set '%1' not found." ).arg( mSettings.mTileMatrixSetId ) );
      mTileMatrixSet = 0;
    }

    setProperty( "resolutions", resolutions );

    if ( mTileLayer == 0 && mTileMatrixSet == 0 )
    {
      appendError( ERR( tr( "Tile layer or tile matrix set not found" ) ) );
      return false;
    }
  }
  return true;
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
  if ( mCachedImage &&
       mCachedViewExtent == viewExtent &&
       mCachedViewWidth == pixelWidth &&
       mCachedViewHeight == pixelHeight )
  {
    return mCachedImage;
  }

  // delete cached image and create network request(s) to fill it
  if ( mCachedImage )
  {
    delete mCachedImage;
    mCachedImage = 0;
  }

  // abort running (untiled) request
  if ( mCacheReply )
  {
    mCacheReply->abort();
    delete mCacheReply;
    mCacheReply = 0;
  }

  //according to the WMS spec for 1.3, some CRS have inverted axis
  bool changeXY = false;
  if ( !mCaps.mParserSettings.ignoreAxisOrientation && ( mCaps.mCapabilities.version == "1.3.0" || mCaps.mCapabilities.version == "1.3" ) )
  {
    //create CRS from string
    QgsCoordinateReferenceSystem theSrs;
    if ( theSrs.createFromOgcWmsCrs( mImageCrs ) && theSrs.axisInverted() )
    {
      changeXY = true;
    }
  }

  if ( mCaps.mParserSettings.invertAxisOrientation )
    changeXY = !changeXY;

  // compose the URL query string for the WMS server.
  QString crsKey = "SRS"; //SRS in 1.1.1 and CRS in 1.3.0
  if ( mCaps.mCapabilities.version == "1.3.0" || mCaps.mCapabilities.version == "1.3" )
  {
    crsKey = "CRS";
  }

  // Bounding box in WMS format (Warning: does not work with scientific notation)
  QString bbox = QString( changeXY ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
                 .arg( qgsDoubleToString( viewExtent.xMinimum() ) )
                 .arg( qgsDoubleToString( viewExtent.yMinimum() ) )
                 .arg( qgsDoubleToString( viewExtent.xMaximum() ) )
                 .arg( qgsDoubleToString( viewExtent.yMaximum() ) );

  mCachedImage = new QImage( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
  mCachedImage->fill( 0 );
  mCachedViewExtent = viewExtent;
  mCachedViewWidth = pixelWidth;
  mCachedViewHeight = pixelHeight;

  if ( !mSettings.mTiled && !mSettings.mMaxWidth && !mSettings.mMaxHeight )
  {
    // Calculate active layers that are also visible.

    QgsDebugMsg( "Active layer list of "  + mSettings.mActiveSubLayers.join( ", " )
                 + " and style list of "  + mSettings.mActiveSubStyles.join( ", " ) );

    QStringList visibleLayers = QStringList();
    QStringList visibleStyles = QStringList();

    QStringList::Iterator it2  = mSettings.mActiveSubStyles.begin();

    for ( QStringList::Iterator it = mSettings.mActiveSubLayers.begin();
          it != mSettings.mActiveSubLayers.end();
          ++it )
    {
      if ( mActiveSubLayerVisibility.find( *it ).value() )
      {
        visibleLayers += *it;
        visibleStyles += *it2;
      }

      ++it2;
    }

    QString layers = visibleLayers.join( "," );
    QString styles = visibleStyles.join( "," );

    QgsDebugMsg( "Visible layer list of " + layers + " and style list of " + styles );

    QUrl url( mSettings.mIgnoreGetMapUrl ? mSettings.mBaseUrl : getMapUrl() );
    setQueryItem( url, "SERVICE", "WMS" );
    setQueryItem( url, "VERSION", mCaps.mCapabilities.version );
    setQueryItem( url, "REQUEST", "GetMap" );
    setQueryItem( url, "BBOX", bbox );
    setQueryItem( url, crsKey, mImageCrs );
    setQueryItem( url, "WIDTH", QString::number( pixelWidth ) );
    setQueryItem( url, "HEIGHT", QString::number( pixelHeight ) );
    setQueryItem( url, "LAYERS", layers );
    setQueryItem( url, "STYLES", styles );
    setQueryItem( url, "FORMAT", mSettings.mImageMimeType );

    if ( mDpi != -1 )
    {
      if ( mSettings.mDpiMode & dpiQGIS )
        setQueryItem( url, "DPI", QString::number( mDpi ) );
      if ( mSettings.mDpiMode & dpiUMN )
        setQueryItem( url, "MAP_RESOLUTION", QString::number( mDpi ) );
      if ( mSettings.mDpiMode & dpiGeoServer )
        setQueryItem( url, "FORMAT_OPTIONS", QString( "dpi:%1" ).arg( mDpi ) );
    }

    //MH: jpeg does not support transparency and some servers complain if jpg and transparent=true
    if ( mSettings.mImageMimeType == "image/x-jpegorpng" ||
         ( !mSettings.mImageMimeType.contains( "jpeg", Qt::CaseInsensitive ) &&
           !mSettings.mImageMimeType.contains( "jpg", Qt::CaseInsensitive ) ) )
    {
      setQueryItem( url, "TRANSPARENT", "TRUE" );  // some servers giving error for 'true' (lowercase)
    }

    QgsDebugMsg( QString( "getmap: %1" ).arg( url.toString() ) );

    // cache some details for if the user wants to do an identifyAsHtml() later

    mGetFeatureInfoUrlBase = mSettings.mIgnoreGetFeatureInfoUrl ? mSettings.mBaseUrl : getFeatureInfoUrl();

    QNetworkRequest request( url );
    mSettings.authorization().setAuthorization( request );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
    mCacheReply = nam()->get( request );
    connect( mCacheReply, SIGNAL( finished() ), this, SLOT( cacheReplyFinished() ), Qt::DirectConnection );
    connect( mCacheReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( cacheReplyProgress( qint64, qint64 ) ), Qt::DirectConnection );

    emit statusChanged( tr( "Getting map via WMS." ) );

    //QTime t;
    //t.start();

    Q_ASSERT( mCacheReply->thread() == QThread::currentThread() );

    QEventLoop loop;
    connect(mCacheReply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec( QEventLoop::ExcludeUserInputEvents );

    qDebug("LOOP FINISHED");

    Q_ASSERT( mCacheReply == 0 );
  }
  else
  {
    mTileReqNo++;

    double vres = viewExtent.width() / pixelWidth;
    double tres = vres;

    const QgsWmtsTileMatrix *tm = 0;
    enum QgsTileMode tileMode;

    if ( mSettings.mTiled )
    {
      Q_ASSERT( mTileLayer );
      Q_ASSERT( mTileMatrixSet );
      Q_ASSERT( mTileMatrixSet->tileMatrices.size() > 0 );

      QMap<double, QgsWmtsTileMatrix> &m =  mTileMatrixSet->tileMatrices;

      // find nearest resolution
      QMap<double, QgsWmtsTileMatrix>::const_iterator prev, it = m.constBegin();
      while ( it != m.constEnd() && it.key() < vres )
      {
        QgsDebugMsg( QString( "res:%1 >= %2" ).arg( it.key() ).arg( vres ) );
        prev = it;
        it++;
      }

      if ( it == m.constEnd() ||
           ( it != m.constBegin() && vres - prev.key() < it.key() - vres ) )
      {
        QgsDebugMsg( "back to previous res" );
        it = prev;
      }

      tres = it.key();
      tm = &it.value();

      tileMode = mTileLayer->tileMode;
    }
    else
    {
      static QgsWmtsTileMatrix tempTm;
      tempTm.topLeft      = QgsPoint( mLayerExtent.xMinimum(), mLayerExtent.yMaximum() );
      tempTm.tileWidth    = mSettings.mMaxWidth;
      tempTm.tileHeight   = mSettings.mMaxHeight;
      tempTm.matrixWidth  = ceil( mLayerExtent.width() / mSettings.mMaxWidth / vres );
      tempTm.matrixHeight = ceil( mLayerExtent.height() / mSettings.mMaxHeight / vres );
      tm = &tempTm;

      tileMode = WMSC;
    }

    QgsDebugMsg( QString( "layer extent: %1,%2 %3x%4" )
                 .arg( qgsDoubleToString( mLayerExtent.xMinimum() ) )
                 .arg( qgsDoubleToString( mLayerExtent.yMinimum() ) )
                 .arg( mLayerExtent.width() )
                 .arg( mLayerExtent.height() )
               );

    QgsDebugMsg( QString( "view extent: %1,%2 %3x%4  res:%5" )
                 .arg( qgsDoubleToString( viewExtent.xMinimum() ) )
                 .arg( qgsDoubleToString( viewExtent.yMinimum() ) )
                 .arg( viewExtent.width() )
                 .arg( viewExtent.height() )
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
    QgsDebugMsg( QString( "tile map size: %1,%2" ).arg( qgsDoubleToString( twMap ) ).arg( qgsDoubleToString( thMap ) ) );

    int minTileCol = 0;
    int maxTileCol = tm->matrixWidth - 1;
    int minTileRow = 0;
    int maxTileRow = tm->matrixHeight - 1;


    if ( mTileLayer &&
         mTileLayer->setLinks.contains( mTileMatrixSet->identifier ) &&
         mTileLayer->setLinks[ mTileMatrixSet->identifier ].limits.contains( tm->identifier ) )
    {
      const QgsWmtsTileMatrixLimits &tml = mTileLayer->setLinks[ mTileMatrixSet->identifier ].limits[ tm->identifier ];
      minTileCol = tml.minTileCol;
      maxTileCol = tml.maxTileCol;
      minTileRow = tml.minTileRow;
      maxTileRow = tml.maxTileRow;
      QgsDebugMsg( QString( "%1 %2: TileMatrixLimits col %3-%4 row %5-%6" )
                   .arg( mTileMatrixSet->identifier )
                   .arg( tm->identifier )
                   .arg( minTileCol ).arg( maxTileCol )
                   .arg( minTileRow ).arg( maxTileRow ) );
    }

    int col0 = qBound( minTileCol, ( int ) floor(( viewExtent.xMinimum() - tm->topLeft.x() ) / twMap ), maxTileCol );
    int row0 = qBound( minTileRow, ( int ) floor(( tm->topLeft.y() - viewExtent.yMaximum() ) / thMap ), maxTileRow );
    int col1 = qBound( minTileCol, ( int ) floor(( viewExtent.xMaximum() - tm->topLeft.x() ) / twMap ), maxTileCol );
    int row1 = qBound( minTileRow, ( int ) floor(( tm->topLeft.y() - viewExtent.yMinimum() ) / thMap ), maxTileRow );

#if QGISDEBUG
    int n = ( col1 - col0 + 1 ) * ( row1 - row0 + 1 );
    QgsDebugMsg( QString( "tile number: %1x%2 = %3" ).arg( col1 - col0 + 1 ).arg( row1 - row0 + 1 ).arg( n ) );
    if ( n > 100 )
    {
      emit statusChanged( QString( "current view would need %1 tiles. tile request per draw limited to 100." ).arg( n ) );
      return mCachedImage;
    }
#endif

    switch ( tileMode )
    {
      case WMSC:
      {
        // add WMS request
        QUrl url( mSettings.mIgnoreGetMapUrl ? mSettings.mBaseUrl : getMapUrl() );
        setQueryItem( url, "SERVICE", "WMS" );
        setQueryItem( url, "VERSION", mCaps.mCapabilities.version );
        setQueryItem( url, "REQUEST", "GetMap" );
        setQueryItem( url, "WIDTH", QString::number( tm->tileWidth ) );
        setQueryItem( url, "HEIGHT", QString::number( tm->tileHeight ) );
        setQueryItem( url, "LAYERS", mSettings.mActiveSubLayers.join( "," ) );
        setQueryItem( url, "STYLES", mSettings.mActiveSubStyles.join( "," ) );
        setQueryItem( url, "FORMAT", mSettings.mImageMimeType );
        setQueryItem( url, crsKey, mImageCrs );

        if ( mSettings.mTiled )
        {
          setQueryItem( url, "TILED", "true" );
        }

        if ( mDpi != -1 )
        {
          if ( mSettings.mDpiMode & dpiQGIS )
            setQueryItem( url, "DPI", QString::number( mDpi ) );
          if ( mSettings.mDpiMode & dpiUMN )
            setQueryItem( url, "MAP_RESOLUTION", QString::number( mDpi ) );
          if ( mSettings.mDpiMode & dpiGeoServer )
            setQueryItem( url, "FORMAT_OPTIONS", QString( "dpi:%1" ).arg( mDpi ) );
        }

        if ( mSettings.mImageMimeType == "image/x-jpegorpng" ||
             ( !mSettings.mImageMimeType.contains( "jpeg", Qt::CaseInsensitive ) &&
               !mSettings.mImageMimeType.contains( "jpg", Qt::CaseInsensitive ) ) )
        {
          setQueryItem( url, "TRANSPARENT", "TRUE" );  // some servers giving error for 'true' (lowercase)
        }

        int i = 0;
        for ( int row = row0; row <= row1; row++ )
        {
          for ( int col = col0; col <= col1; col++ )
          {
            QString turl;
            turl += url.toString();
            turl += QString( changeXY ? "&BBOX=%2,%1,%4,%3" : "&BBOX=%1,%2,%3,%4" )
                    .arg( qgsDoubleToString( tm->topLeft.x() +         col * twMap /* + twMap * 0.001 */ ) )
                    .arg( qgsDoubleToString( tm->topLeft.y() - ( row + 1 ) * thMap /* - thMap * 0.001 */ ) )
                    .arg( qgsDoubleToString( tm->topLeft.x() + ( col + 1 ) * twMap /* - twMap * 0.001 */ ) )
                    .arg( qgsDoubleToString( tm->topLeft.y() -         row * thMap /* + thMap * 0.001 */ ) );

            QNetworkRequest request( turl );
            mSettings.authorization().setAuthorization( request );
            QgsDebugMsg( QString( "tileRequest %1 %2/%3 (%4,%5): %6" ).arg( mTileReqNo ).arg( i++ ).arg( n ).arg( row ).arg( col ).arg( turl ) );
            request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
            request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
            request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ), mTileReqNo );
            request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileIndex ), i );
            request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRect ),
                                  QRectF( tm->topLeft.x() + col * twMap, tm->topLeft.y() - ( row + 1 ) * thMap, twMap, thMap ) );
            request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), 0 );

            QgsDebugMsg( QString( "gettile: %1" ).arg( turl ) );
            QNetworkReply *reply = nam()->get( request );
            mTileReplies << reply;
            connect( reply, SIGNAL( finished() ), this, SLOT( tileReplyFinished() ) );
          }
        }
      }
      break;

      case WMTS:
      {
        if ( !getTileUrl().isNull() )
        {
          // KVP
          QUrl url( mSettings.mIgnoreGetMapUrl ? mSettings.mBaseUrl : getTileUrl() );

          // compose static request arguments.
          setQueryItem( url, "SERVICE", "WMTS" );
          setQueryItem( url, "REQUEST", "GetTile" );
          setQueryItem( url, "VERSION", mCaps.mCapabilities.version );
          setQueryItem( url, "LAYER", mSettings.mActiveSubLayers[0] );
          setQueryItem( url, "STYLE", mSettings.mActiveSubStyles[0] );
          setQueryItem( url, "FORMAT", mSettings.mImageMimeType );
          setQueryItem( url, "TILEMATRIXSET", mTileMatrixSet->identifier );
          setQueryItem( url, "TILEMATRIX", tm->identifier );

          for ( QHash<QString, QString>::const_iterator it = mSettings.mTileDimensionValues.constBegin(); it != mSettings.mTileDimensionValues.constEnd(); ++it )
          {
            setQueryItem( url, it.key(), it.value() );
          }

          url.removeQueryItem( "TILEROW" );
          url.removeQueryItem( "TILECOL" );

          int i = 0;
          for ( int row = row0; row <= row1; row++ )
          {
            for ( int col = col0; col <= col1; col++ )
            {
              QString turl;
              turl += url.toString();
              turl += QString( "&TILEROW=%1&TILECOL=%2" ).arg( row ).arg( col );

              QNetworkRequest request( turl );
              mSettings.authorization().setAuthorization( request );
              QgsDebugMsg( QString( "tileRequest %1 %2/%3 (%4,%5): %6" ).arg( mTileReqNo ).arg( i++ ).arg( n ).arg( row ).arg( col ).arg( turl ) );
              request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
              request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
              request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ), mTileReqNo );
              request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileIndex ), i );
              request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRect ),
                                    QRectF( tm->topLeft.x() + col * twMap, tm->topLeft.y() - ( row + 1 ) * thMap, twMap, thMap ) );
              request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), 0 );

              QgsDebugMsg( QString( "gettile: %1" ).arg( turl ) );
              QNetworkReply *reply = nam()->get( request );
              mTileReplies << reply;
              connect( reply, SIGNAL( finished() ), this, SLOT( tileReplyFinished() ) );
            }
          }
        }
        else
        {
          // REST
          QString url = mTileLayer->getTileURLs[ mSettings.mImageMimeType ];

          url.replace( "{style}", mSettings.mActiveSubStyles[0], Qt::CaseInsensitive );
          url.replace( "{tilematrixset}", mTileMatrixSet->identifier, Qt::CaseInsensitive );
          url.replace( "{tilematrix}", tm->identifier, Qt::CaseInsensitive );

          for ( QHash<QString, QString>::const_iterator it = mSettings.mTileDimensionValues.constBegin(); it != mSettings.mTileDimensionValues.constEnd(); ++it )
          {
            url.replace( "{" + it.key() + "}", it.value(), Qt::CaseInsensitive );
          }

          int i = 0;
          for ( int row = row0; row <= row1; row++ )
          {
            for ( int col = col0; col <= col1; col++ )
            {
              QString turl( url );
              turl.replace( "{tilerow}", QString::number( row ), Qt::CaseInsensitive );
              turl.replace( "{tilecol}", QString::number( col ), Qt::CaseInsensitive );

              QNetworkRequest request( turl );
              mSettings.authorization().setAuthorization( request );
              QgsDebugMsg( QString( "tileRequest %1 %2/%3 (%4,%5): %6" ).arg( mTileReqNo ).arg( i++ ).arg( n ).arg( row ).arg( col ).arg( turl ) );
              request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
              request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
              request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ), mTileReqNo );
              request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileIndex ), i );
              request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRect ),
                                    QRectF( tm->topLeft.x() + col * twMap, tm->topLeft.y() - ( row + 1 ) * thMap, twMap, thMap ) );
              request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), 0 );

              QgsDebugMsg( QString( "gettile: %1" ).arg( turl ) );
              QNetworkReply *reply = nam()->get( request );
              mTileReplies << reply;
              connect( reply, SIGNAL( finished() ), this, SLOT( tileReplyFinished() ) );
            }
          }
        }
      }
      break;

      default:
        QgsDebugMsg( QString( "unexpected tile mode %1" ).arg( mTileLayer->tileMode ) );
        return mCachedImage;
        break;
    }

    emit statusChanged( tr( "Getting tiles." ) );

    QTime t;
    t.start();

    // draw everything that is retrieved within a second
    // and the rest asynchronously
    while ( !mTileReplies.isEmpty() )
    {
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
    }

#ifdef QGISDEBUG
    emit statusChanged( tr( "%n tile requests in background", "tile request count", mTileReplies.count() )
                        + tr( ", %n cache hits", "tile cache hits", mCacheHits )
                        + tr( ", %n cache misses.", "tile cache missed", mCacheMisses )
                        + tr( ", %n errors.", "errors", mErrors )
                      );
#endif
  }

  return mCachedImage;
}

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
  size_t myExpectedSize = pixelWidth * pixelHeight * 4;
  size_t myImageSize = image->height() *  image->bytesPerLine();
  if ( myExpectedSize != myImageSize )   // should not happen
  {
    QgsMessageLog::logMessage( tr( "unexpected image size" ), tr( "WMS" ) );
    return;
  }

  uchar * ptr = image->bits() ;
  if ( ptr )
  {
    // If image is too large, ptr can be NULL
    memcpy( block, ptr, myExpectedSize );
  }
  // do not delete the image, it is handled by draw()
  //delete image;
}

void QgsWmsProvider::repeatTileRequest( QNetworkRequest const &oldRequest )
{
  if ( mErrors == 100 )
  {
    QgsMessageLog::logMessage( tr( "Not logging more than 100 request errors." ), tr( "WMS" ) );
  }

  QNetworkRequest request( oldRequest );

  QString url = request.url().toString();
  int tileReqNo = request.attribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ) ).toInt();
  int tileNo = request.attribute( static_cast<QNetworkRequest::Attribute>( TileIndex ) ).toInt();
  int retry = request.attribute( static_cast<QNetworkRequest::Attribute>( TileRetry ) ).toInt();
  retry++;

  QSettings s;
  int maxRetry = s.value( "/qgis/defaultTileMaxRetry", "3" ).toInt();
  if ( retry > maxRetry )
  {
    if ( mErrors < 100 )
    {
      QgsMessageLog::logMessage( tr( "Tile request max retry error. Failed %1 requests for tile %2 of tileRequest %3 (url: %4)" )
                                 .arg( maxRetry ).arg( tileNo ).arg( tileReqNo ).arg( url ), tr( "WMS" ) );
    }
    return;
  }

  mSettings.authorization().setAuthorization( request );
  if ( mErrors < 100 )
  {
    QgsMessageLog::logMessage( tr( "repeat tileRequest %1 tile %2(retry %3)" )
                               .arg( tileReqNo ).arg( tileNo ).arg( retry ), tr( "WMS" ), QgsMessageLog::INFO );
  }
  QgsDebugMsg( QString( "repeat tileRequest %1 %2(retry %3) for url: %4" ).arg( tileReqNo ).arg( tileNo ).arg( retry ).arg( url ) );
  request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), retry );

  QNetworkReply *reply = nam()->get( request );
  mTileReplies << reply;
  connect( reply, SIGNAL( finished() ), this, SLOT( tileReplyFinished() ) );
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
  foreach ( const QNetworkReply::RawHeaderPair &pair, reply->rawHeaderPairs() )
  {
    QgsDebugMsgLevel( QString( " %1:%2" )
                      .arg( QString::fromUtf8( pair.first ) )
                      .arg( QString::fromUtf8( pair.second ) ), 3 );
  }
#endif

  if ( nam()->cache() )
  {
    QNetworkCacheMetaData cmd = nam()->cache()->metaData( reply->request().url() );

    QNetworkCacheMetaData::RawHeaderList hl;
    foreach ( const QNetworkCacheMetaData::RawHeader &h, cmd.rawHeaders() )
    {
      if ( h.first != "Cache-Control" )
        hl.append( h );
    }
    cmd.setRawHeaders( hl );

    QgsDebugMsg( QString( "expirationDate:%1" ).arg( cmd.expirationDate().toString() ) );
    if ( cmd.expirationDate().isNull() )
    {
      QSettings s;
      cmd.setExpirationDate( QDateTime::currentDateTime().addSecs( s.value( "/qgis/defaultTileExpiry", "24" ).toInt() * 60 * 60 ) );
    }

    nam()->cache()->updateMetaData( cmd );
  }

  int tileReqNo = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ) ).toInt();
  int tileNo = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( TileIndex ) ).toInt();
  QRectF r = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( TileRect ) ).toRectF();
  int retry = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( TileRetry ) ).toInt();

#if QT_VERSION >= 0x40500
  QgsDebugMsg( QString( "tile reply %1 (%2) tile:%3(retry %4) rect:%5,%6 %7,%8) fromcache:%9 error:%10 url:%11" )
               .arg( tileReqNo ).arg( mTileReqNo ).arg( tileNo ).arg( retry )
               .arg( r.left(), 0, 'f' ).arg( r.bottom(), 0, 'f' ).arg( r.right(), 0, 'f' ).arg( r.top(), 0, 'f' )
               .arg( fromCache )
               .arg( reply->errorString() )
               .arg( reply->url().toString() )
             );
#else
  QgsDebugMsg( QString( "tile reply %1 (%2) tile:%3(retry %4) rect:%5,%6 %7,%8) error:%9 url:%10" )
               .arg( tileReqNo ).arg( mTileReqNo ).arg( tileNo ).arg( retry )
               .arg( r.left(), 0, 'f' ).arg( r.bottom(), 0, 'f' ).arg( r.right(), 0, 'f' ).arg( r.top(), 0, 'f' )
               .arg( reply->errorString() )
               .arg( reply->url().toString() )
             );
#endif

  if ( reply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      QNetworkRequest request( redirect.toUrl() );
      mSettings.authorization().setAuthorization( request );
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
      request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ), tileReqNo );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileIndex ), tileNo );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRect ), r );
      request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), 0 );

      mTileReplies.removeOne( reply );
      reply->deleteLater();

      QgsDebugMsg( QString( "redirected gettile: %1" ).arg( redirect.toString() ) );
      reply = nam()->get( request );
      mTileReplies << reply;

      connect( reply, SIGNAL( finished() ), this, SLOT( tileReplyFinished() ) );

      return;
    }

    QVariant status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
    {
      QVariant phrase = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );

      showMessageBox( tr( "Tile request error" ), tr( "Status: %1\nReason phrase: %2" ).arg( status.toInt() ).arg( phrase.toString() ) );

      mTileReplies.removeOne( reply );
      reply->deleteLater();

      return;
    }

    QString contentType = reply->header( QNetworkRequest::ContentTypeHeader ).toString();
    QgsDebugMsg( "contentType: " + contentType );
    if ( !contentType.startsWith( "image/", Qt::CaseInsensitive ) &&
         contentType.compare( "application/octet-stream", Qt::CaseInsensitive ) != 0 )
    {
      QByteArray text = reply->readAll();
      if ( contentType.toLower() == "text/xml" && parseServiceExceptionReportDom( text ) )
      {
        QgsMessageLog::logMessage( tr( "Tile request error (Title:%1; Error:%2; URL: %3)" )
                                   .arg( mErrorCaption ).arg( mError )
                                   .arg( reply->url().toString() ), tr( "WMS" ) );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Tile request error (Status:%1; Content-Type:%2; Length:%3; URL: %4)" )
                                   .arg( status.toString() )
                                   .arg( contentType )
                                   .arg( text.size() )
                                   .arg( reply->url().toString() ), tr( "WMS" ) );
#ifdef QGISDEBUG
        QFile file( QDir::tempPath() + "/broken-image.png" );
        if ( file.open( QIODevice::WriteOnly ) )
        {
          file.write( text );
          file.close();
        }
#endif
      }

      mTileReplies.removeOne( reply );
      reply->deleteLater();

      return;
    }

    // only take results from current request number
    if ( mTileReqNo == tileReqNo )
    {
      double cr = mCachedViewExtent.width() / mCachedViewWidth;

      QRectF dst(( r.left() - mCachedViewExtent.xMinimum() ) / cr,
                 ( mCachedViewExtent.yMaximum() - r.bottom() ) / cr,
                 r.width() / cr,
                 r.height() / cr );

      QgsDebugMsg( QString( "tile reply: length %1" ).arg( reply->bytesAvailable() ) );

      QImage myLocalImage = QImage::fromData( reply->readAll() );

      if ( !myLocalImage.isNull() )
      {
        QPainter p( mCachedImage );
        if ( mSettings.mSmoothPixmapTransform )
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
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Returned image is flawed [Content-Type:%1; URL: %2]" )
                                   .arg( contentType ).arg( reply->url().toString() ), tr( "WMS" ) );

        repeatTileRequest( reply->request() );
      }
    }
    else
    {
      QgsDebugMsg( QString( "Reply too late [%1]" ).arg( reply->url().toString() ) );
    }

    mTileReplies.removeOne( reply );
    reply->deleteLater();
  }
  else
  {
    mErrors++;

    repeatTileRequest( reply->request() );

    mTileReplies.removeOne( reply );
    reply->deleteLater();
  }

#ifdef QGISDEBUG
  emit statusChanged( tr( "%n tile requests in background", "tile request count", mTileReplies.count() )
                      + tr( ", %n cache hits", "tile cache hits", mCacheHits )
                      + tr( ", %n cache misses.", "tile cache missed", mCacheMisses )
                      + tr( ", %n errors.", "errors", mErrors )
                    );
#endif
}

void QgsWmsProvider::cacheReplyFinished()
{
  if ( mCacheReply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = mCacheReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      mCacheReply->deleteLater();

      QgsDebugMsg( QString( "redirected getmap: %1" ).arg( redirect.toString() ) );
      mCacheReply = nam()->get( QNetworkRequest( redirect.toUrl() ) );
      connect( mCacheReply, SIGNAL( finished() ), this, SLOT( cacheReplyFinished() ) );
      return;
    }

    QVariant status = mCacheReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
    {
      QVariant phrase = mCacheReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );

      QgsMessageLog::logMessage( tr( "Map request error (Status: %1; Reason phrase: %2; URL:%3)" )
                                 .arg( status.toInt() )
                                 .arg( phrase.toString() )
                                 .arg( mCacheReply->url().toString() ), tr( "WMS" ) );

      mCacheReply->deleteLater();
      mCacheReply = 0;

      return;
    }

    QString contentType = mCacheReply->header( QNetworkRequest::ContentTypeHeader ).toString();
    QgsDebugMsg( "contentType: " + contentType );
    if ( contentType.startsWith( "image/", Qt::CaseInsensitive ) ||
         contentType.compare( "application/octet-stream", Qt::CaseInsensitive ) == 0 )
    {
      qDebug("GOT PICTURE!");
      QImage myLocalImage = QImage::fromData( mCacheReply->readAll() );
      if ( !myLocalImage.isNull() )
      {
        QPainter p( mCachedImage );
        p.drawImage( 0, 0, myLocalImage );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Returned image is flawed [Content-Type:%1; URL:%2]" )
                                   .arg( contentType ).arg( mCacheReply->url().toString() ), tr( "WMS" ) );
      }
    }
    else
    {
      QByteArray text = mCacheReply->readAll();
      if ( contentType.toLower() == "text/xml" && parseServiceExceptionReportDom( text ) )
      {
        QgsMessageLog::logMessage( tr( "Map request error (Title:%1; Error:%2; URL: %3)" )
                                   .arg( mErrorCaption ).arg( mError )
                                   .arg( mCacheReply->url().toString() ), tr( "WMS" ) );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Map request error (Status: %1; Response: %2; Content-Type: %3; URL:%4)" )
                                   .arg( status.toInt() )
                                   .arg( QString::fromUtf8( text ) )
                                   .arg( contentType )
                                   .arg( mCacheReply->url().toString() ), tr( "WMS" ) );
      }

      mCacheReply->deleteLater();
      mCacheReply = 0;

      return;
    }

    mCacheReply->deleteLater();
    mCacheReply = 0;
    qDebug("DELETE REPLY");
  }
  else
  {
    mErrors++;
    if ( mErrors < 100 )
    {
      QgsMessageLog::logMessage( tr( "Map request failed [error:%1 url:%2]" ).arg( mCacheReply->errorString() ).arg( mCacheReply->url().toString() ), tr( "WMS" ) );
    }
    else if ( mErrors == 100 )
    {
      QgsMessageLog::logMessage( tr( "Not logging more than 100 request errors." ), tr( "WMS" ) );
    }

    mCacheReply->deleteLater();
    mCacheReply = 0;
  }
}

bool QgsWmsProvider::retrieveServerCapabilities( bool /*forceRefresh*/ )
{
  QgsDebugMsg( "entering." );

  if ( !mCaps.isValid() )
  {
    QgsWmsCapabilitiesDownload downloadCaps( mSettings.baseUrl(), mSettings.authorization() );
    if ( !downloadCaps.downloadCapabilities() )
    {
      mErrorFormat = "text/plain";
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


QGis::DataType QgsWmsProvider::dataType( int bandNo ) const
{
  return srcDataType( bandNo );
}

QGis::DataType QgsWmsProvider::srcDataType( int bandNo ) const
{
  Q_UNUSED( bandNo );
  return QGis::ARGB32;
}

int QgsWmsProvider::bandCount() const
{
  return 1;
}

void QgsWmsProvider::cacheReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of map downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsg( msg );
  emit statusChanged( msg );
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
  bool contentSuccess = mServiceExceptionReportDom.setContent( xml, false, &errorMsg, &errorLine, &errorColumn );

  if ( !contentSuccess )
  {
    mErrorCaption = tr( "Dom Exception" );
    mErrorFormat = "text/plain";
    mError = tr( "Could not get WMS Service Exception at %1: %2 at line %3 column %4\n\nResponse was:\n\n%5" )
             .arg( mSettings.mBaseUrl )
             .arg( errorMsg )
             .arg( errorLine )
             .arg( errorColumn )
             .arg( QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  QDomElement docElem = mServiceExceptionReportDom.documentElement();

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

  mErrorCaption = tr( "Service Exception" );
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

  QgsDebugMsg( QString( "exiting with composed error message '%1'." ).arg( mError ) );
}

QgsRectangle QgsWmsProvider::extent()
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

bool QgsWmsProvider::isValid()
{
  return mValid;
}


QString QgsWmsProvider::wmsVersion()
{
  // TODO
  return NULL;
}


QStringList QgsWmsProvider::subLayers() const
{
  return mSettings.mActiveSubLayers;
}


QStringList QgsWmsProvider::subLayerStyles() const
{
  return mSettings.mActiveSubStyles;
}

bool QgsWmsProvider::calculateExtent()
{
  //! \todo Make this handle non-geographic CRSs (e.g. floor plans) as per WMS spec

  QgsDebugMsg( "entered." );

  // Set up the coordinate transform from the WMS standard CRS:84 bounding
  // box to the user's selected CRS
  if ( !mCoordinateTransform )
  {
    QgsCoordinateReferenceSystem qgisSrsSource;
    QgsCoordinateReferenceSystem qgisSrsDest;

    if ( mSettings.mTiled && mTileLayer )
    {
      QgsDebugMsg( QString( "Tile layer's extent: %1 %2" ).arg( mTileLayer->boundingBox.box.toString() ).arg( mTileLayer->boundingBox.crs ) );
      qgisSrsSource.createFromOgcWmsCrs( mTileLayer->boundingBox.crs );
    }
    else
    {
      qgisSrsSource.createFromOgcWmsCrs( DEFAULT_LATLON_CRS );
    }

    qgisSrsDest.createFromOgcWmsCrs( mImageCrs );

    mCoordinateTransform = new QgsCoordinateTransform( qgisSrsSource, qgisSrsDest );
  }

  if ( mSettings.mTiled )
  {
    if ( mTileLayer )
    {
      try
      {
        QgsRectangle extent = mCoordinateTransform->transformBoundingBox( mTileLayer->boundingBox.box, QgsCoordinateTransform::ForwardTransform );

        //make sure extent does not contain 'inf' or 'nan'
        if ( extent.isFinite() )
        {
          QgsDebugMsg( "exiting with '"  + mLayerExtent.toString() + "'." );
          mLayerExtent = extent;
          return true;
        }
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
      }
    }

    QgsDebugMsg( "no extent returned" );
    return false;
  }
  else
  {
    bool firstLayer = true; //flag to know if a layer is the first to be successfully transformed
    for ( QStringList::Iterator it  = mSettings.mActiveSubLayers.begin();
          it != mSettings.mActiveSubLayers.end();
          ++it )
    {
      QgsDebugMsg( "Sublayer iterator: " + *it );
      // This is the extent for the layer name in *it
      if ( !mCaps.mExtentForLayer.contains( *it ) )
      {
        mLayerExtent = QgsRectangle();
        appendError( ERR( tr( "Extent for layer %1 not found in capabilities" ).arg( *it ) ) );
        continue;
      }

      QgsRectangle extent = mCaps.mExtentForLayer.find( *it ).value();

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
        mLayerExtent = extent;
      }
      else
      {
        mLayerExtent.combineExtentWith( &extent );
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

  QgsDebugMsg( "entering." );

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
        QgsDebugMsg( "'"  + ( *it )  + "' is queryable." );
        canIdentify = true;
      }
    }
  }

  if ( canIdentify )
  {
    if ( identifyCapabilities() )
    {
      capability |= identifyCapabilities() | Identify;
    }
  }
  QgsDebugMsg( QString( "capability = %1" ).arg( capability ) );
  return capability;
}

int QgsWmsProvider::identifyCapabilities() const
{
  int capability = NoCapabilities;

  foreach ( QgsRaster::IdentifyFormat f, mCaps.mIdentifyFormats.keys() )
  {
    capability |= identifyFormatToCapability( f );
  }

  QgsDebugMsg( QString( "capability = %1" ).arg( capability ) );
  return capability;
}

QString QgsWmsProvider::layerMetadata( QgsWmsLayerProperty &layer )
{
  QString metadata;

  // Layer Properties section

  // Use a nested table
  metadata += "<tr><td>";
  metadata += "<table width=\"100%\">";

  // Table header
  metadata += "<tr><th class=\"glossy\">";
  metadata += tr( "Property" );
  metadata += "</th>";
  metadata += "<th class=\"glossy\">";
  metadata += tr( "Value" );
  metadata += "</th></tr>";

  // Name
  metadata += "<tr><td>";
  metadata += tr( "Name" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += layer.name;
  metadata += "</td></tr>";

  // Layer Visibility (as managed by this provider)
  metadata += "<tr><td>";
  metadata += tr( "Visibility" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += mActiveSubLayerVisibility.find( layer.name ).value() ? tr( "Visible" ) : tr( "Hidden" );
  metadata += "</td></tr>";

  // Layer Title
  metadata += "<tr><td>";
  metadata += tr( "Title" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += layer.title;
  metadata += "</td></tr>";

  // Layer Abstract
  metadata += "<tr><td>";
  metadata += tr( "Abstract" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += layer.abstract;
  metadata += "</td></tr>";

  // Layer Queryability
  metadata += "<tr><td>";
  metadata += tr( "Can Identify" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += layer.queryable ? tr( "Yes" ) : tr( "No" );
  metadata += "</td></tr>";

  // Layer Opacity
  metadata += "<tr><td>";
  metadata += tr( "Can be Transparent" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += layer.opaque ? tr( "No" ) : tr( "Yes" );
  metadata += "</td></tr>";

  // Layer Subsetability
  metadata += "<tr><td>";
  metadata += tr( "Can Zoom In" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += layer.noSubsets ? tr( "No" ) : tr( "Yes" );
  metadata += "</td></tr>";

  // Layer Server Cascade Count
  metadata += "<tr><td>";
  metadata += tr( "Cascade Count" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += QString::number( layer.cascaded );
  metadata += "</td></tr>";

  // Layer Fixed Width
  metadata += "<tr><td>";
  metadata += tr( "Fixed Width" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += QString::number( layer.fixedWidth );
  metadata += "</td></tr>";

  // Layer Fixed Height
  metadata += "<tr><td>";
  metadata += tr( "Fixed Height" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += QString::number( layer.fixedHeight );
  metadata += "</td></tr>";

  // Layer Fixed Height
  metadata += "<tr><td>";
  metadata += tr( "WGS 84 Bounding Box" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += mCaps.mExtentForLayer[ layer.name ].toString();
  metadata += "</td></tr>";

  // Layer Coordinate Reference Systems
  for ( int j = 0; j < qMin( layer.crs.size(), 10 ); j++ )
  {
    metadata += "<tr><td>";
    metadata += tr( "Available in CRS" );
    metadata += "</td>";
    metadata += "<td>";
    metadata += layer.crs[j];
    metadata += "</td></tr>";
  }

  if ( layer.crs.size() > 10 )
  {
    metadata += "<tr><td>";
    metadata += tr( "Available in CRS" );
    metadata += "</td>";
    metadata += "<td>";
    metadata += tr( "(and %n more)", "crs", layer.crs.size() - 10 );
    metadata += "</td></tr>";
  }

  // Layer Styles
  for ( int j = 0; j < layer.style.size(); j++ )
  {
    metadata += "<tr><td>";
    metadata += tr( "Available in style" );
    metadata += "</td>";
    metadata += "<td>";

    // Nested table.
    metadata += "<table width=\"100%\">";

    // Layer Style Name
    metadata += "<tr><th class=\"glossy\">";
    metadata += tr( "Name" );
    metadata += "</th>";
    metadata += "<td>";
    metadata += layer.style[j].name;
    metadata += "</td></tr>";

    // Layer Style Title
    metadata += "<tr><th class=\"glossy\">";
    metadata += tr( "Title" );
    metadata += "</th>";
    metadata += "<td>";
    metadata += layer.style[j].title;
    metadata += "</td></tr>";

    // Layer Style Abstract
    metadata += "<tr><th class=\"glossy\">";
    metadata += tr( "Abstract" );
    metadata += "</th>";
    metadata += "<td>";
    metadata += layer.style[j].abstract;
    metadata += "</td></tr>";

    // Close the nested table
    metadata += "</table>";
    metadata += "</td></tr>";
  }

  // Close the nested table
  metadata += "</table>";
  metadata += "</td></tr>";

  return metadata;
}

QString QgsWmsProvider::metadata()
{
  QString metadata = "";

  metadata += "<tr><td>";

  metadata += "<a href=\"#serverproperties\">";
  metadata += tr( "Server Properties" );
  metadata += "</a> ";

  metadata += "&nbsp;<a href=\"#selectedlayers\">";
  metadata += tr( "Selected Layers" );
  metadata += "</a>&nbsp;<a href=\"#otherlayers\">";
  metadata += tr( "Other Layers" );
  metadata += "</a>";

  if ( mCaps.mTileLayersSupported.size() > 0 )
  {
    metadata += "<a href=\"#tilelayerproperties\">";
    metadata += tr( "Tile Layer Properties" );
    metadata += "</a> ";

#if QT_VERSION >= 0x40500
    metadata += "<a href=\"#cachestats\">";
    metadata += tr( "Cache Stats" );
    metadata += "</a> ";
#endif
  }

  metadata += "</td></tr>";

  // Server Properties section
  metadata += "<tr><th class=\"glossy\"><a name=\"serverproperties\"></a>";
  metadata += tr( "Server Properties" );
  metadata += "</th></tr>";

  // Use a nested table
  metadata += "<tr><td>";
  metadata += "<table width=\"100%\">";

  // Table header
  metadata += "<tr><th class=\"glossy\">";
  metadata += tr( "Property" );
  metadata += "</th>";
  metadata += "<th class=\"glossy\">";
  metadata += tr( "Value" );
  metadata += "</th></tr>";

  // WMS Version
  metadata += "<tr><td>";
  metadata += tr( "WMS Version" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += mCaps.mCapabilities.version;
  metadata += "</td></tr>";

  // Service Title
  metadata += "<tr><td>";
  metadata += tr( "Title" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += mCaps.mCapabilities.service.title;
  metadata += "</td></tr>";

  // Service Abstract
  metadata += "<tr><td>";
  metadata += tr( "Abstract" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += mCaps.mCapabilities.service.abstract;
  metadata += "</td></tr>";

  // Service Keywords
  metadata += "<tr><td>";
  metadata += tr( "Keywords" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += mCaps.mCapabilities.service.keywordList.join( "<br />" );
  metadata += "</td></tr>";

  // Service Online Resource
  metadata += "<tr><td>";
  metadata += tr( "Online Resource" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += "-";
  metadata += "</td></tr>";

  // Service Contact Information
  metadata += "<tr><td>";
  metadata += tr( "Contact Person" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += mCaps.mCapabilities.service.contactInformation.contactPersonPrimary.contactPerson;
  metadata += "<br />";
  metadata += mCaps.mCapabilities.service.contactInformation.contactPosition;
  metadata += "<br />";
  metadata += mCaps.mCapabilities.service.contactInformation.contactPersonPrimary.contactOrganization;
  metadata += "</td></tr>";

  // Service Fees
  metadata += "<tr><td>";
  metadata += tr( "Fees" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += mCaps.mCapabilities.service.fees;
  metadata += "</td></tr>";

  // Service Access Constraints
  metadata += "<tr><td>";
  metadata += tr( "Access Constraints" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += mCaps.mCapabilities.service.accessConstraints;
  metadata += "</td></tr>";

  // GetMap Request Formats
  metadata += "<tr><td>";
  metadata += tr( "Image Formats" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += mCaps.mCapabilities.capability.request.getMap.format.join( "<br />" );
  metadata += "</td></tr>";

  // GetFeatureInfo Request Formats
  metadata += "<tr><td>";
  metadata += tr( "Identify Formats" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += mCaps.mCapabilities.capability.request.getFeatureInfo.format.join( "<br />" );
  metadata += "</td></tr>";

  // Layer Count (as managed by this provider)
  metadata += "<tr><td>";
  metadata += tr( "Layer Count" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += QString::number( mCaps.mLayersSupported.size() );
  metadata += "</td></tr>";

  // Tileset Count (as managed by this provider)
  if ( mCaps.mTileLayersSupported.size() > 0 )
  {
    metadata += "<tr><td>";
    metadata += tr( "Tile Layer Count" );
    metadata += "</td>";
    metadata += "<td>";
    metadata += QString::number( mCaps.mTileLayersSupported.size() );
    metadata += "</td></tr>";
  }

  // Base URL
  metadata += "<tr><td>";
  metadata += tr( "GetCapabilitiesUrl" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += mSettings.mBaseUrl;
  metadata += "</td></tr>";

  metadata += "<tr><td>";
  metadata += tr( "GetMapUrl" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += getMapUrl() + ( mSettings.mIgnoreGetMapUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : "" );
  metadata += "</td></tr>";

  metadata += "<tr><td>";
  metadata += tr( "GetFeatureInfoUrl" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += getFeatureInfoUrl() + ( mSettings.mIgnoreGetFeatureInfoUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : "" );
  metadata += "</td></tr>";

  if ( mSettings.mTiled )
  {
    metadata += "<tr><td>";
    metadata += tr( "GetTileUrl" );
    metadata += "</td>";
    metadata += "<td>";
    metadata += getTileUrl();
    metadata += "</td></tr>";

    if ( mTileLayer )
    {
      metadata += "<tr><td>";
      metadata += tr( "Tile templates" );
      metadata += "</td>";
      metadata += "<td>";
      for ( QHash<QString, QString>::const_iterator it = mTileLayer->getTileURLs.constBegin();
            it != mTileLayer->getTileURLs.constEnd();
            it++ )
      {
        metadata += QString( "%1:%2<br>" ).arg( it.key() ).arg( it.value() );
      }
      metadata += "</td></tr>";

      metadata += "<tr><td>";
      metadata += tr( "FeatureInfo templates" );
      metadata += "</td>";
      metadata += "<td>";
      for ( QHash<QString, QString>::const_iterator it = mTileLayer->getFeatureInfoURLs.constBegin();
            it != mTileLayer->getFeatureInfoURLs.constEnd();
            it++ )
      {
        metadata += QString( "%1:%2<br>" ).arg( it.key() ).arg( it.value() );
      }
      metadata += "</td></tr>";
    }
  }

  // Close the nested table
  metadata += "</table>";
  metadata += "</td></tr>";

  // Layer properties
  metadata += "<tr><th class=\"glossy\"><a name=\"selectedlayers\"></a>";
  metadata += tr( "Selected Layers" );
  metadata += "</th></tr>";

  for ( int i = 0; i < mCaps.mLayersSupported.size(); i++ )
  {
    if ( !mSettings.mTiled && mSettings.mActiveSubLayers.indexOf( mCaps.mLayersSupported[i].name ) >= 0 )
    {
      metadata += layerMetadata( mCaps.mLayersSupported[i] );
    }
  } // for each layer

  // Layer properties
  metadata += "<tr><th class=\"glossy\"><a name=\"otherlayers\"></a>";
  metadata += tr( "Other Layers" );
  metadata += "</th></tr>";

  for ( int i = 0; i < mCaps.mLayersSupported.size(); i++ )
  {
    if ( mSettings.mActiveSubLayers.indexOf( mCaps.mLayersSupported[i].name ) < 0 )
    {
      metadata += layerMetadata( mCaps.mLayersSupported[i] );
    }
  } // for each layer

  // Tileset properties
  if ( mCaps.mTileLayersSupported.size() > 0 )
  {
    metadata += "<tr><th class=\"glossy\"><a name=\"tilesetproperties\"></a>";
    metadata += tr( "Tileset Properties" );
    metadata += "</th></tr>";

    // Iterate through tilesets
    metadata += "<tr><td>";
    metadata += "<table width=\"100%\">";

    foreach ( const QgsWmtsTileLayer &l, mCaps.mTileLayersSupported )
    {
      metadata += "<tr><td colspan=\"2\">";
      metadata += l.identifier;
      metadata += "</td><td class=\"glossy\">";

      if ( l.tileMode == WMTS )
      {
        metadata += tr( "WMTS" );
      }
      else if ( l.tileMode == WMSC )
      {
        metadata += tr( "WMS-C" );
      }
      else
      {
        Q_ASSERT( l.tileMode == WMTS || l.tileMode == WMSC );
      }

      metadata += "</td></tr>";

      // Table header
      metadata += "<tr><th class=\"glossy\">";
      metadata += tr( "Property" );
      metadata += "</th>";
      metadata += "<th class=\"glossy\">";
      metadata += tr( "Value" );
      metadata += "</th></tr>";

      metadata += "<tr><td class=\"glossy\">";
      metadata += tr( "Selected" );
      metadata += "</td>";
      metadata += "<td class=\"glossy\">";
      metadata += mSettings.mTiled && l.identifier == mSettings.mActiveSubLayers.join( "," ) ? tr( "Yes" ) : tr( "No" );
      metadata += "</td></tr>";

      if ( l.styles.size() > 0 )
      {
        metadata += "<tr><td class=\"glossy\">";
        metadata += tr( "Available Styles" );
        metadata += "</td>";
        metadata += "<td class=\"glossy\">";
        QStringList styles;
        foreach ( const QgsWmtsStyle &style, l.styles )
        {
          styles << style.identifier;
        }
        metadata += styles.join( ", " );
        metadata += "</td></tr>";
      }

      metadata += "<tr><td class=\"glossy\">";
      metadata += tr( "CRS" );
      metadata += "</td>";
      metadata += "<td class=\"glossy\">";
      metadata += l.boundingBox.crs;
      metadata += "</td></tr>";

      metadata += "<tr><td class=\"glossy\">";
      metadata += tr( "Bounding Box" );
      metadata += "</td>";
      metadata += "<td class=\"glossy\">";
      metadata += l.boundingBox.box.toString();
      metadata += "</td></tr>";

      metadata += "<tr><td class=\"glossy\">";
      metadata += tr( "Available Tilesets" );
      metadata += "</td><td class=\"glossy\">";

      foreach ( const QgsWmtsTileMatrixSetLink &setLink, l.setLinks )
      {
        metadata += setLink.tileMatrixSet + "<br>";
      }

      metadata += "</td></tr>";
    }

    metadata += "</table></td></tr>";

#if QT_VERSION >= 0x40500
    if ( mSettings.mTiled )
    {
      metadata += "<tr><th class=\"glossy\"><a name=\"cachestats\"></a>";
      metadata += tr( "Cache stats" );
      metadata += "</th></tr>";

      // Iterate through tilesets
      metadata += "<tr><td>";
      metadata += "<table width=\"100%\">";

      metadata += "<tr><th class=\"glossy\">";
      metadata += tr( "Property" );
      metadata += "</th>";
      metadata += "<th class=\"glossy\">";
      metadata += tr( "Value" );
      metadata += "</th></tr>";

      metadata += "<tr><td>";
      metadata += tr( "Hits" );
      metadata += "</td><td>";
      metadata += QString::number( mCacheHits );
      metadata += "</td></tr>";

      metadata += "<tr><td>";
      metadata += tr( "Misses" );
      metadata += "</td><td>";
      metadata += QString::number( mCacheMisses );
      metadata += "</td></tr>";

      metadata += "<tr><td>";
      metadata += tr( "Errors" );
      metadata += "</td><td>";
      metadata += QString::number( mErrors );
      metadata += "</td></tr>";

      metadata += "</table></td></tr>";
    }
#endif
  }

  metadata += "</table>";

  QgsDebugMsg( "exiting with '"  + metadata  + "'." );

  return metadata;
}

QgsRasterIdentifyResult QgsWmsProvider::identify( const QgsPoint & thePoint, QgsRaster::IdentifyFormat theFormat, const QgsRectangle &theExtent, int theWidth, int theHeight )
{
  QgsDebugMsg( QString( "theFormat = %1" ).arg( theFormat ) );
  QStringList resultStrings;
  QMap<int, QVariant> results;

  QString format;
  format = mCaps.mIdentifyFormats.value( theFormat );
  if ( format.isEmpty() )
  {
    return QgsRasterIdentifyResult( ERROR( tr( "Format not supported" ) ) );
  }

  QgsDebugMsg( QString( "theFormat = %1 format = %2" ).arg( theFormat ).arg( format ) );

  if ( !extent().contains( thePoint ) )
  {
    results.insert( 1, "" );
    return QgsRasterIdentifyResult( theFormat, results );
  }

  QgsRectangle myExtent = theExtent;

  if ( !myExtent.isEmpty() )
  {
    // we cannot reliably identify WMS if theExtent is specified but theWidth or theHeight
    // are not, because we don't know original resolution
    if ( theWidth == 0 || theHeight == 0 )
    {
      return QgsRasterIdentifyResult( ERROR( tr( "Context not fully specified (extent was defined but width and/or height was not)." ) ) );
    }
  }
  else // context (theExtent, theWidth, theHeight) not defined
  {
    // We don't know original source resolution, so we take some small extent around the point.

    double xRes = 0.001; // expecting meters

    // TODO: add CRS as class member
    QgsCoordinateReferenceSystem crs;
    if ( crs.createFromOgcWmsCrs( mImageCrs ) )
    {
      // set resolution approximately to 1mm
      switch ( crs.mapUnits() )
      {
        case QGis::Meters:
          xRes = 0.001;
          break;
        case QGis::Feet:
          xRes = 0.003;
          break;
        case QGis::Degrees:
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
    theWidth = theHeight = 2;

    myExtent = QgsRectangle( thePoint.x() - xRes, thePoint.y() - yRes,
                             thePoint.x() + xRes, thePoint.y() + yRes );
  }

  // Point in BBOX/WIDTH/HEIGHT coordinates
  // No need to fiddle with extent origin not covered by layer extent, I believe
  double xRes = myExtent.width() / theWidth;
  double yRes = myExtent.height() / theHeight;

  QgsDebugMsg( "myExtent = " + myExtent.toString() );
  QgsDebugMsg( QString( "theWidth = %1 theHeight = %2" ).arg( theWidth ).arg( theHeight ) );
  QgsDebugMsg( QString( "xRes = %1 yRes = %2" ).arg( xRes ).arg( yRes ) );

  QgsPoint point;
  point.setX( floor(( thePoint.x() - myExtent.xMinimum() ) / xRes ) );
  point.setY( floor(( myExtent.yMaximum() - thePoint.y() ) / yRes ) );

  QgsDebugMsg( QString( "point = %1 %2" ).arg( point.x() ).arg( point.y() ) );

  QgsDebugMsg( QString( "recalculated orig point (corner) = %1 %2" ).arg( myExtent.xMinimum() + point.x()*xRes ).arg( myExtent.yMaximum() - point.y()*yRes ) );

  // Collect which layers to query on
  //according to the WMS spec for 1.3, the order of x - and y - coordinates is inverted for geographical CRS
  bool changeXY = false;
  if ( !mCaps.mParserSettings.ignoreAxisOrientation && ( mCaps.mCapabilities.version == "1.3.0" || mCaps.mCapabilities.version == "1.3" ) )
  {
    //create CRS from string
    QgsCoordinateReferenceSystem theSrs;
    if ( theSrs.createFromOgcWmsCrs( mImageCrs ) && theSrs.axisInverted() )
    {
      changeXY = true;
    }
  }

  if ( mCaps.mParserSettings.invertAxisOrientation )
    changeXY = !changeXY;

  // compose the URL query string for the WMS server.
  QString crsKey = "SRS"; //SRS in 1.1.1 and CRS in 1.3.0
  if ( mCaps.mCapabilities.version == "1.3.0" || mCaps.mCapabilities.version == "1.3" )
  {
    crsKey = "CRS";
  }

  // Compose request to WMS server
  QString bbox = QString( changeXY ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
                 .arg( qgsDoubleToString( myExtent.xMinimum() ) )
                 .arg( qgsDoubleToString( myExtent.yMinimum() ) )
                 .arg( qgsDoubleToString( myExtent.xMaximum() ) )
                 .arg( qgsDoubleToString( myExtent.yMaximum() ) );

  //QgsFeatureList featureList;

  int count = -1;
  // Test for which layers are suitable for querying with
  for ( QStringList::const_iterator
        layers  = mSettings.mActiveSubLayers.begin(),
        styles = mSettings.mActiveSubStyles.begin();
        layers != mSettings.mActiveSubLayers.end();
        ++layers, ++styles )
  {
    count++;

    // Is sublayer visible?
    if ( !mActiveSubLayerVisibility.find( *layers ).value() )
    {
      // TODO: something better?
      // we need to keep all sublayers so that we can get their names in identify tool
      results.insert( count, false );
      continue;
    }

    // Is sublayer queryable?
    if ( !mCaps.mQueryableForLayer.find( *layers ).value() )
    {
      results.insert( count, false );
      continue;
    }

    QgsDebugMsg( "Layer '" + *layers + "' is queryable." );

    QUrl requestUrl( mGetFeatureInfoUrlBase );
    setQueryItem( requestUrl, "SERVICE", "WMS" );
    setQueryItem( requestUrl, "VERSION", mCaps.mCapabilities.version );
    setQueryItem( requestUrl, "REQUEST", "GetFeatureInfo" );
    setQueryItem( requestUrl, "BBOX", bbox );
    setQueryItem( requestUrl, crsKey, mImageCrs );
    setQueryItem( requestUrl, "WIDTH", QString::number( theWidth ) );
    setQueryItem( requestUrl, "HEIGHT", QString::number( theHeight ) );
    setQueryItem( requestUrl, "LAYERS", *layers );
    setQueryItem( requestUrl, "STYLES", *styles );
    setQueryItem( requestUrl, "FORMAT", mSettings.mImageMimeType );
    setQueryItem( requestUrl, "QUERY_LAYERS", *layers );
    setQueryItem( requestUrl, "INFO_FORMAT", format );

    if ( mCaps.mCapabilities.version == "1.3.0" || mCaps.mCapabilities.version == "1.3" )
    {
      setQueryItem( requestUrl, "I", QString::number( point.x() ) );
      setQueryItem( requestUrl, "J", QString::number( point.y() ) );
    }
    else
    {
      setQueryItem( requestUrl, "X", QString::number( point.x() ) );
      setQueryItem( requestUrl, "Y", QString::number( point.y() ) );
    }

    if ( mSettings.mFeatureCount > 0 )
    {
      setQueryItem( requestUrl, "FEATURE_COUNT", QString::number( mSettings.mFeatureCount ) );
    }

    QgsDebugMsg( QString( "getfeatureinfo: %1" ).arg( requestUrl.toString() ) );
    QNetworkRequest request( requestUrl );
    mSettings.authorization().setAuthorization( request );
    mIdentifyReply = nam()->get( request );
    connect( mIdentifyReply, SIGNAL( finished() ), this, SLOT( identifyReplyFinished() ) );

    QEventLoop loop;
    connect(mIdentifyReply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec( QEventLoop::ExcludeUserInputEvents );

    if ( mIdentifyResultBodies.size() == 0 ) // no result
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
      foreach ( const QByteArray &v, headers.keys() )
      {
        if ( QString( v ).compare( "Content-Type", Qt::CaseInsensitive ) == 0 )
        {
          isXml = QString( headers.value( v ) ).compare( "text/xml", Qt::CaseInsensitive ) == 0;
          isGml = QString( headers.value( v ) ).compare( "ogr/gml", Qt::CaseInsensitive ) == 0;
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

        if ( isXml && parseServiceExceptionReportDom( body ) )
        {
          QgsMessageLog::logMessage( tr( "Get feature info request error (Title:%1; Error:%2; URL: %3)" )
                                     .arg( mErrorCaption ).arg( mError )
                                     .arg( requestUrl.toString() ), tr( "WMS" ) );
          continue;
        }
      }
    }

    if ( theFormat == QgsRaster::IdentifyFormatHtml || theFormat == QgsRaster::IdentifyFormatText )
    {
      //resultStrings << mIdentifyResult;
      //results.insert( count, mIdentifyResult );
      results.insert( count, QString::fromUtf8( mIdentifyResultBodies.value( 0 ) ) );
    }
    else if ( theFormat == QgsRaster::IdentifyFormatFeature ) // GML
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
      for ( int i = 0; i < mIdentifyResultHeaders.size(); i++ )
      {
        if ( xsdPart == -1 && mIdentifyResultHeaders[i].value( "Content-Disposition" ).contains( ".xsd" ) )
        {
          xsdPart = i;
        }
        else if ( gmlPart == -1 && mIdentifyResultHeaders[i].value( "Content-Disposition" ).contains( ".dat" ) )
        {
          gmlPart = i;
        }

        if ( gmlPart != -1 && xsdPart != -1 )
          break;
      }

      if ( xsdPart == -1 && gmlPart == -1 )
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

      QgsDebugMsg( "GML (first 2000 bytes):\n" + QString::fromUtf8( mIdentifyResultBodies.value( gmlPart ).left( 2000 ) ) );
      QGis::WkbType wkbType;
      QgsGmlSchema gmlSchema;

      if ( xsdPart >= 0 )  // XSD available
      {
#if QT_VERSION >= 0x40600
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
#endif
        QgsDebugMsg( "GML XSD (first 4000 bytes):\n" + QString::fromUtf8( mIdentifyResultBodies.value( xsdPart ).left( 4000 ) ) );
        gmlSchema.parseXSD( mIdentifyResultBodies.value( xsdPart ) );
      }
      else
      {
        // guess from GML
        bool ok = gmlSchema.guessSchema( mIdentifyResultBodies.value( gmlPart ) );
        if ( ! ok )
        {
          QgsError err = gmlSchema.error();
          err.append( tr( "Cannot identify" ) );
          QgsDebugMsg( "guess schema error: " +  err.message() );
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
      foreach ( QString featureTypeName, featureTypeNames )
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
        int ret = gml.getFeatures( mIdentifyResultBodies.value( gmlPart ), &wkbType );
#ifdef QGISDEBUG
        QgsDebugMsg( QString( "parsing result = %1" ).arg( ret ) );
#else
        Q_UNUSED( ret );
#endif
        // TODO: all features coming from this layer should probably have the same CRS
        // the same as this layer, because layerExtentToOutputExtent() may be used
        // for results -> verify CRS and reprojects if necessary
        QMap<QgsFeatureId, QgsFeature* > features = gml.featuresMap();
        QgsDebugMsg( QString( "%1 features read" ).arg( features.size() ) );
        QgsFeatureStore featureStore( fields, crs() );
        QMap<QString, QVariant> params;
        params.insert( "sublayer", *layers );
        params.insert( "featureType", featureTypeName );
        params.insert( "getFeatureInfoUrl", requestUrl.toString() );
        featureStore.setParams( params );
        foreach ( QgsFeatureId id, features.keys() )
        {
          QgsFeature * feature = features.value( id );

          QgsDebugMsg( QString( "feature id = %1 : %2 attributes" ).arg( id ).arg( feature->attributes().size() ) );

          featureStore.features().append( QgsFeature( *feature ) );
        }
        featureStoreList.append( featureStore );
      }
      results.insert( count, qVariantFromValue( featureStoreList ) );
    }
  }

#if 0
  QString str;

  if ( theFormat == QgsRaster::IdentifyFormatHtml )
  {
    str = "<table>\n<tr><td>" + resultStrings.join( "</td></tr>\n<tr><td>" ) + "</td></tr>\n</table>";
    results.insert( 1, str );
  }
  else if ( theFormat == QgsRaster::IdentifyFormatText )
  {
    str = resultStrings.join( "\n-------------\n" );
    results.insert( 1, str );
  }
#endif

  return QgsRasterIdentifyResult( theFormat, results );
}

void QgsWmsProvider::identifyReplyFinished()
{
  mIdentifyResultHeaders.clear();
  mIdentifyResultBodies.clear();

  if ( mIdentifyReply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = mIdentifyReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      QgsDebugMsg( QString( "identify request redirected to %1" ).arg( redirect.toString() ) );
      emit statusChanged( tr( "identify request redirected." ) );

      mIdentifyReply->deleteLater();

      QgsDebugMsg( QString( "redirected getfeatureinfo: %1" ).arg( redirect.toString() ) );
      mIdentifyReply = nam()->get( QNetworkRequest( redirect.toUrl() ) );
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

      //mIdentifyResult = "";
    }

    QgsNetworkReplyParser parser( mIdentifyReply );
    if ( !parser.isValid() )
    {
      QgsDebugMsg( "Cannot parse reply" );
      mErrorFormat = "text/plain";
      mError = tr( "Cannot parse getfeatureinfo: %1" ).arg( parser.error() );
      emit statusChanged( mError );
      //mIdentifyResult = "";
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
    mErrorFormat = "text/plain";
    mError = tr( "Map getfeatureinfo error: %1 [%2]" ).arg( mIdentifyReply->errorString() ).arg( mIdentifyReply->url().toString() );
    emit statusChanged( mError );
    QgsMessageLog::logMessage( mError, tr( "WMS" ) );
  }

  mIdentifyReply->deleteLater();
  mIdentifyReply = 0;
}


QgsCoordinateReferenceSystem QgsWmsProvider::crs()
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
  delete mCachedImage;
  mCachedImage = 0;
}

QgsNetworkAccessManager *QgsWmsProvider::nam()
{
  if ( mNAM && mNAM->thread() != QThread::currentThread() )
  {
    // TODO: check that no network connections are handled by the NAM?
    mNAM->deleteLater();
    mNAM = 0;
  }

  if ( !mNAM )
    mNAM = new QgsNetworkAccessManager();

  return mNAM;
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
    QgsWmsSupportedFormat j2 = { "jpeg", "JPEG" }; // used by french IGN geoportail
    formats << j1 << j2;
  }

  if ( supportedFormats.contains( "png" ) && supportedFormats.contains( "jpg" ) )
  {
    QgsWmsSupportedFormat g1 = { "image/x-jpegorpng", "JPEG/PNG" }; // used by cubewerx
    formats << g1;
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

QString QgsWmsProvider::nodeAttribute( const QDomElement &e, QString name, QString defValue )
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

void QgsWmsProvider::showMessageBox( const QString& title, const QString& text )
{
  QgsMessageOutput *message = QgsMessageOutput::createMessageOutput();
  message->setTitle( title );
  message->setMessage( text, QgsMessageOutput::MessageText );
  message->showMessage();
}

QImage QgsWmsProvider::getLegendGraphic( double scale, bool forceRefresh )
{
  // TODO manage return basing of getCapablity => avoid call if service is not available
  // some services dowsn't expose getLegendGraphic in capabilities but adding LegendURL in
  // the layer tags inside capabilities
  QgsDebugMsg( "entering." );

  if ( !scale && !mGetLegendGraphicScale )
  {
    QgsDebugMsg( QString( "No scale factor set" ) );
    return QImage();
  }

  if ( scale && scale != mGetLegendGraphicScale )
  {
    forceRefresh = true;
    QgsDebugMsg( QString( "Download again due to scale change from: %1 to: %2" ).arg( mGetLegendGraphicScale ).arg( scale ) );
  }

  if ( forceRefresh )
  {
    if ( scale )
    {
      mGetLegendGraphicScale = scale;
    }

    // if style is not defined, set as "default"
    QString currentStyle( "default" );
    if ( mSettings.mActiveSubStyles[0] != "" )
    {
      currentStyle = mSettings.mActiveSubStyles[0];
    }

#if 0
    // add WMS GetGraphicLegend request
    // TODO set sld version using instance var something like mSldVersion
    // TODO at this moment LSD version can be get from LegendURL in getCapability,but parsing of
    // this tag is not complete. Below the code that should work if parsing would correct

    if ( mActiveSubLayers[0] == mCapabilities.capability.layer.name )
    {
      foreach ( QgsWmsStyleProperty style,  mCapabilities.capability.layer.style )
      {
        if ( currentStyle == style.name )
        {
          url.setUrl( style.legendUrl[0].onlineResource.xlinkHref, QUrl::StrictMode );
        }
      }
    } // is a sublayer
    else if ( mActiveSubLayers[0].contains( mCapabilities.capability.layer.name ) )
    {
      foreach ( QgsWmsLayerProperty layerProperty, mCapabilities.capability.layer.layer )
      {
        if ( mActiveSubLayers[0] == layerProperty.name )
        {
          foreach ( QgsWmsStyleProperty style, layerProperty.style )
          {
            if ( currentStyle == style.name )
            {
              url.setUrl( style.legendUrl[0].onlineResource.xlinkHref, QUrl::StrictMode );
            }
          }
        }
      }
    }
#endif
    QUrl url( mSettings.mIgnoreGetMapUrl ? mSettings.mBaseUrl : getMapUrl(), QUrl::StrictMode );
    setQueryItem( url, "SERVICE", "WMS" );
    setQueryItem( url, "VERSION", mCaps.mCapabilities.version );
    setQueryItem( url, "SLD_VERSION", "1.1.0" ); // can not determine SLD_VERSION
    setQueryItem( url, "REQUEST", "GetLegendGraphic" );
    setQueryItem( url, "LAYER", mSettings.mActiveSubLayers[0] );
    setQueryItem( url, "STYLE", currentStyle );
    setQueryItem( url, "SCALE", QString::number( scale, 'f' ) );
    setQueryItem( url, "FORMAT", mSettings.mImageMimeType );

    // add config parameter related to resolution
    QSettings s;
    int defaultLegendGraphicResolution = s.value( "/qgis/defaultLegendGraphicResolution", 0 ).toInt();
    QgsDebugMsg( QString( "defaultLegendGraphicResolution: %1" ).arg( defaultLegendGraphicResolution ) );
    if ( defaultLegendGraphicResolution )
    {
      if ( url.queryItemValue( "map_resolution" ) != "" )
      {
        setQueryItem( url, "map_resolution", QString::number( defaultLegendGraphicResolution ) );
      }
      else if ( url.queryItemValue( "dpi" ) != "" )
      {
        setQueryItem( url, "dpi", QString::number( defaultLegendGraphicResolution ) );
      }
      else
      {
        QgsLogger::warning( tr( "getLegendGraphic: Can not determine resolution uri parameter [map_resolution | dpi]. No resolution parameter will be used" ) );
      }
    }

    mError = "";

    QNetworkRequest request( url );
    mSettings.authorization().setAuthorization( request );
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

    QgsDebugMsg( QString( "getlegendgraphics: %1" ).arg( url.toString() ) );
    mGetLegendGraphicReply = nam()->get( request );

    connect( mGetLegendGraphicReply, SIGNAL( finished() ), this, SLOT( getLegendGraphicReplyFinished() ), Qt::DirectConnection );
    connect( mGetLegendGraphicReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( getLegendGraphicReplyProgress( qint64, qint64 ) ), Qt::DirectConnection );

    QEventLoop loop;
    connect(mGetLegendGraphicReply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec( QEventLoop::ExcludeUserInputEvents );
  }
  else
  {
    QgsDebugMsg( "get cached pixmap." );
  }

  QgsDebugMsg( "exiting." );

  return mGetLegendGraphicImage;
}

void QgsWmsProvider::getLegendGraphicReplyFinished()
{
  QgsDebugMsg( "entering." );
  if ( mGetLegendGraphicReply->error() == QNetworkReply::NoError )
  {
    QgsDebugMsg( "reply ok" );
    QVariant redirect = mGetLegendGraphicReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      emit statusChanged( tr( "GetLegendGraphic request redirected." ) );

      const QUrl& toUrl = redirect.toUrl();
      mGetLegendGraphicReply->request();
      if ( toUrl == mGetLegendGraphicReply->url() )
      {
        mErrorFormat = "text/plain";
        mError = tr( "Redirect loop detected: %1" ).arg( toUrl.toString() );
        QgsMessageLog::logMessage( mError, tr( "WMS" ) );
        mHttpGetLegendGraphicResponse.clear();
      }
      else
      {
        QNetworkRequest request( toUrl );
        mSettings.authorization().setAuthorization( request );
        request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
        request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

        mGetLegendGraphicReply->deleteLater();
        QgsDebugMsg( QString( "redirected GetLegendGraphic: %1" ).arg( redirect.toString() ) );
        mGetLegendGraphicReply = nam()->get( request );

        connect( mGetLegendGraphicReply, SIGNAL( finished() ), this, SLOT( getLegendGraphicReplyFinished() ) );
        connect( mGetLegendGraphicReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( getLegendGraphicReplyProgress( qint64, qint64 ) ) );
        return;
      }
    }

    QVariant status = mGetLegendGraphicReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
    {
      QVariant phrase = mGetLegendGraphicReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
      showMessageBox( tr( "GetLegendGraphic request error" ), tr( "Status: %1\nReason phrase: %2" ).arg( status.toInt() ).arg( phrase.toString() ) );
    }
    else
    {
      QImage myLocalImage = QImage::fromData( mGetLegendGraphicReply->readAll() ) ;
      if ( myLocalImage.isNull() )
      {
        QgsMessageLog::logMessage( tr( "Returned legend image is flawed [URL: %1]" )
                                   .arg( mGetLegendGraphicReply->url().toString() ), tr( "WMS" ) );
      }
      else
      {
        mGetLegendGraphicImage = myLocalImage;

#ifdef QGISDEBUG
        QString filename = QDir::tempPath() + "/GetLegendGraphic.png";
        mGetLegendGraphicImage.save( filename );
        QgsDebugMsg( "saved GetLegendGraphic result in debug ile: " + filename );
#endif
      }
    }
  }
  else
  {
    mErrorFormat = "text/plain";
    mError = tr( "Download of GetLegendGraphic failed: %1" ).arg( mGetLegendGraphicReply->errorString() );
    QgsMessageLog::logMessage( mError, tr( "WMS" ) );
    mHttpGetLegendGraphicResponse.clear();
  }

  mGetLegendGraphicReply->deleteLater();
  mGetLegendGraphicReply = 0;
}

void QgsWmsProvider::getLegendGraphicReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of GetLegendGraphic downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsg( msg );
  emit statusChanged( msg );
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
