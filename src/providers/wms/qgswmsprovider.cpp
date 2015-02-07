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

#include <QTimer>

#include <typeinfo>

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
#include <QNetworkDiskCache>

#include <QtXmlPatterns/QXmlSchema>
#include <QtXmlPatterns/QXmlSchemaValidator>

#include <QUrl>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QSet>
#include <QSettings>
#include <QEventLoop>
#include <QCoreApplication>
#include <QTextCodec>
#include <QTime>
#include <QThread>

#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>

#include <ogr_api.h>

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

QMap<QString, QgsWmsStatistics::Stat> QgsWmsStatistics::sData;

QgsWmsProvider::QgsWmsProvider( QString const& uri, const QgsWmsCapabilities* capabilities )
    : QgsRasterDataProvider( uri )
    , mHttpGetLegendGraphicResponse( 0 )
    , mGetLegendGraphicImage()
    , mGetLegendGraphicScale( 0.0 )
    , mImageCrs( DEFAULT_LATLON_CRS )
    , mCachedImage( 0 )
    , mIdentifyReply( 0 )
    , mCachedViewExtent( 0 )
    , mCachedViewWidth( 0 )
    , mCachedViewHeight( 0 )
    , mCoordinateTransform( NULL )
    , mExtentDirty( true )
    , mTileReqNo( 0 )
    , mTileLayer( 0 )
    , mTileMatrixSet( 0 )
{
  QgsDebugMsg( "constructing with uri '" + uri + "'." );

  mSupportedGetFeatureFormats = QStringList() << "text/html" << "text/plain" << "text/xml" << "application/vnd.ogc.gml" << "application/json";

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
}

QgsRasterInterface * QgsWmsProvider::clone() const
{
  QgsWmsProvider* provider = new QgsWmsProvider( dataSourceUri(), mCaps.isValid() ? &mCaps : 0 );
  provider->copyBaseSettings( *this );
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

QString QgsWmsProvider::getLegendGraphicUrl() const
{
  QString url;

  for ( int i = 0; i < mCaps.mLayersSupported.size() && url.isEmpty(); i++ )
  {
    const QgsWmsLayerProperty &l = mCaps.mLayersSupported[i];

    if ( l.name != mSettings.mActiveSubLayers[0] )
      continue;

    for ( int j = 0; j < l.style.size() && url.isEmpty(); j++ )
    {
      const QgsWmsStyleProperty &s = l.style[j];

      if ( s.name != mSettings.mActiveSubStyles[0] )
        continue;

      for ( int k = 0; k < s.legendUrl.size() && url.isEmpty(); k++ )
      {
        const QgsWmsLegendUrlProperty &l = s.legendUrl[k];

        if ( l.format != mSettings.mImageMimeType )
          continue;

        url = l.onlineResource.xlinkHref;
      }
    }
  }

  if ( url.isEmpty() && mCaps.mCapabilities.capability.request.getLegendGraphic.dcpType.size() > 0 )
  {
    url = mCaps.mCapabilities.capability.request.getLegendGraphic.dcpType.front().http.get.onlineResource.xlinkHref;
  }

  return url.isEmpty() ? url : prepareUri( url );
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


void QgsWmsProvider::setSubLayerVisibility( QString const & name, bool vis )
{
  if ( !mActiveSubLayerVisibility.contains( name ) )
  {
    QgsDebugMsg( QString( "Layer %1 not found." ).arg( name ) );
    return;
  }

  mActiveSubLayerVisibility[name] = vis;
}


bool QgsWmsProvider::setImageCrs( QString const & crs )
{
  QgsDebugMsg( "Setting image CRS to " + crs + "." );

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
      if ( !mTileMatrixSet->tileMatrices.empty() )
      {
        setProperty( "tileWidth", mTileMatrixSet->tileMatrices.values().first().tileWidth );
        setProperty( "tileHeight", mTileMatrixSet->tileMatrices.values().first().tileHeight );
      }
    }
    else
    {
      QgsDebugMsg( QString( "Expected tile matrix set '%1' not found." ).arg( mSettings.mTileMatrixSetId ) );
      mTileMatrixSet = 0;
    }

    setProperty( "resolutions", resolutions );

    if ( mTileLayer == 0 || mTileMatrixSet == 0 )
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

void QgsWmsProvider::setFormatQueryItem( QUrl &url )
{
  url.removeQueryItem( "FORMAT" );
  if ( mSettings.mImageMimeType.contains( "+" ) )
  {
    QString format( mSettings.mImageMimeType );
    format.replace( "+", "%2b" );
    url.addEncodedQueryItem( "FORMAT", format.toUtf8() );
  }
  else
    setQueryItem( url, "FORMAT", mSettings.mImageMimeType );
}

QImage *QgsWmsProvider::draw( QgsRectangle const &viewExtent, int pixelWidth, int pixelHeight )
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


  bool changeXY = mCaps.shouldInvertAxisOrientation( mImageCrs );

  // compose the URL query string for the WMS server.

  QString bbox = toParamValue( viewExtent, changeXY );

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
    setSRSQueryItem( url );
    setQueryItem( url, "WIDTH", QString::number( pixelWidth ) );
    setQueryItem( url, "HEIGHT", QString::number( pixelHeight ) );
    setQueryItem( url, "LAYERS", layers );
    setQueryItem( url, "STYLES", styles );
    setFormatQueryItem( url );

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

    emit statusChanged( tr( "Getting map via WMS." ) );

    QgsWmsImageDownloadHandler handler( dataSourceUri(), url, mSettings.authorization(), mCachedImage );
    handler.downloadBlocking();

    //QTime t;
    //t.start();

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

    QList<QgsWmsTiledImageDownloadHandler::TileRequest> requests;

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
        setFormatQueryItem( url );

        setSRSQueryItem( url );

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

            QgsDebugMsg( QString( "tileRequest %1 %2/%3 (%4,%5): %6" ).arg( mTileReqNo ).arg( i++ ).arg( n ).arg( row ).arg( col ).arg( turl ) );
            QRectF rect( tm->topLeft.x() + col * twMap, tm->topLeft.y() - ( row + 1 ) * thMap, twMap, thMap );
            requests << QgsWmsTiledImageDownloadHandler::TileRequest( turl, rect, i );
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

              QgsDebugMsg( QString( "tileRequest %1 %2/%3 (%4,%5): %6" ).arg( mTileReqNo ).arg( i++ ).arg( n ).arg( row ).arg( col ).arg( turl ) );
              QRectF rect( tm->topLeft.x() + col * twMap, tm->topLeft.y() - ( row + 1 ) * thMap, twMap, thMap );
              requests << QgsWmsTiledImageDownloadHandler::TileRequest( turl, rect, i );
            }
          }
        }
        else
        {
          // REST
          QString url = mTileLayer->getTileURLs[ mSettings.mImageMimeType ];

          url.replace( "{layer}", mSettings.mActiveSubLayers[0], Qt::CaseInsensitive );
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

              QgsDebugMsg( QString( "tileRequest %1 %2/%3 (%4,%5): %6" ).arg( mTileReqNo ).arg( i++ ).arg( n ).arg( row ).arg( col ).arg( turl ) );
              QRectF rect( tm->topLeft.x() + col * twMap, tm->topLeft.y() - ( row + 1 ) * thMap, twMap, thMap );
              requests << QgsWmsTiledImageDownloadHandler::TileRequest( turl, rect, i );
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

    QgsWmsTiledImageDownloadHandler handler( dataSourceUri(), mSettings.authorization(), mTileReqNo, requests, mCachedImage, mCachedViewExtent, mSettings.mSmoothPixmapTransform );
    handler.downloadBlocking();


#if 0
    const QgsWmsStatistics::Stat& stat = QgsWmsStatistics::statForUri( dataSourceUri() );
    emit statusChanged( tr( "%n tile requests in background", "tile request count", requests.count() )
                        + tr( ", %n cache hits", "tile cache hits", stat.cacheHits )
                        + tr( ", %n cache misses.", "tile cache missed", stat.cacheMisses )
                        + tr( ", %n errors.", "errors", stat.errors )
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
  QImage *image = draw( viewExtent, pixelWidth, pixelHeight );
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
    return;
  }

  uchar * ptr = image->bits();
  if ( ptr )
  {
    // If image is too large, ptr can be NULL
    memcpy( block, ptr, myExpectedSize );
  }
  // do not delete the image, it is handled by draw()
  //delete image;
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


static const QgsWmsLayerProperty* _findNestedLayerProperty( const QString& layerName, const QgsWmsLayerProperty* prop )
{
  if ( prop->name == layerName )
    return prop;

  foreach ( const QgsWmsLayerProperty& child, prop->layer )
  {
    if ( const QgsWmsLayerProperty* res = _findNestedLayerProperty( layerName, &child ) )
      return res;
  }

  return 0;
}


bool QgsWmsProvider::extentForNonTiledLayer( const QString& layerName, const QString& crs, QgsRectangle& extent )
{
  const QgsWmsLayerProperty* layerProperty = _findNestedLayerProperty( layerName, &mCaps.mCapabilities.capability.layer );
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

  QgsCoordinateReferenceSystem dst, wgs;
  if ( !wgs.createFromOgcWmsCrs( DEFAULT_LATLON_CRS ) || !dst.createFromOgcWmsCrs( crs ) )
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


bool QgsWmsProvider::parseServiceExceptionReportDom( QByteArray const & xml, QString& errorTitle, QString& errorText )
{
  QgsDebugMsg( "entering." );

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
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "ServiceException" )
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


void QgsWmsProvider::parseServiceException( QDomElement const & e, QString& errorTitle, QString& errorText )
{
  QgsDebugMsg( "entering." );

  QString seCode = e.attribute( "code" );
  QString seText = e.text();

  errorTitle = tr( "Service Exception" );

  // set up friendly descriptions for the service exception
  if ( seCode == "InvalidFormat" )
  {
    errorText = tr( "Request contains a format not offered by the server." );
  }
  else if ( seCode == "InvalidCRS" )
  {
    errorText = tr( "Request contains a CRS not offered by the server for one or more of the Layers in the request." );
  }
  else if ( seCode == "InvalidSRS" )  // legacy WMS < 1.3.0
  {
    errorText = tr( "Request contains a SRS not offered by the server for one or more of the Layers in the request." );
  }
  else if ( seCode == "LayerNotDefined" )
  {
    errorText = tr( "GetMap request is for a Layer not offered by the server, "
                    "or GetFeatureInfo request is for a Layer not shown on the map." );
  }
  else if ( seCode == "StyleNotDefined" )
  {
    errorText = tr( "Request is for a Layer in a Style not offered by the server." );
  }
  else if ( seCode == "LayerNotQueryable" )
  {
    errorText = tr( "GetFeatureInfo request is applied to a Layer which is not declared queryable." );
  }
  else if ( seCode == "InvalidPoint" )
  {
    errorText = tr( "GetFeatureInfo request contains invalid X or Y value." );
  }
  else if ( seCode == "CurrentUpdateSequence" )
  {
    errorText = tr( "Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to "
                    "current value of service metadata update sequence number." );
  }
  else if ( seCode == "InvalidUpdateSequence" )
  {
    errorText = tr( "Value of (optional) UpdateSequence parameter in GetCapabilities request is greater "
                    "than current value of service metadata update sequence number." );
  }
  else if ( seCode == "MissingDimensionValue" )
  {
    errorText = tr( "Request does not include a sample dimension value, and the server did not declare a "
                    "default value for that dimension." );
  }
  else if ( seCode == "InvalidDimensionValue" )
  {
    errorText = tr( "Request contains an invalid sample dimension value." );
  }
  else if ( seCode == "OperationNotSupported" )
  {
    errorText = tr( "Request is for an optional operation that is not supported by the server." );
  }
  else if ( seCode.isEmpty() )
  {
    errorText = tr( "(No error code was reported)" );
  }
  else
  {
    errorText = seCode + " " + tr( "(Unknown error code)" );
  }

  errorText += "\n" + tr( "The WMS vendor also reported: " );
  errorText += seText;

  // TODO = e.attribute("locator");

  QgsDebugMsg( QString( "exiting with composed error message '%1'." ).arg( errorText ) );
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

  if ( mSettings.mTiled )
  {
    if ( mTileLayer )
    {
      int i;
      for ( i = 0; i < mTileLayer->boundingBoxes.size() && mTileLayer->boundingBoxes[i].crs != mImageCrs; i++ )
        QgsDebugMsg( QString( "Skip %1 [%2]" ).arg( mTileLayer->boundingBoxes[i].crs ).arg( mImageCrs ) );

      if ( i < mTileLayer->boundingBoxes.size() )
      {
        mLayerExtent = mTileLayer->boundingBoxes[i].box;
      }
      else
      {
        QgsCoordinateReferenceSystem qgisSrsDest;
        qgisSrsDest.createFromOgcWmsCrs( mImageCrs );

        // pick the first that transforms fin(it)e
        for ( i = 0; i < mTileLayer->boundingBoxes.size(); i++ )
        {
          QgsCoordinateReferenceSystem qgisSrsSource;
          qgisSrsSource.createFromOgcWmsCrs( mTileLayer->boundingBoxes[i].crs );

          QgsCoordinateTransform ct( qgisSrsSource, qgisSrsDest );

          QgsDebugMsg( QString( "ct: %1 => %2" ).arg( mTileLayer->boundingBoxes[i].crs ).arg( mImageCrs ) );

          try
          {
            QgsRectangle extent = ct.transformBoundingBox( mTileLayer->boundingBoxes[i].box, QgsCoordinateTransform::ForwardTransform );

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
    for ( QStringList::Iterator it  = mSettings.mActiveSubLayers.begin();
          it != mSettings.mActiveSubLayers.end();
          ++it )
    {
      QgsDebugMsg( "Sublayer iterator: " + *it );

      QgsRectangle extent;
      if ( !extentForNonTiledLayer( *it, mImageCrs, extent ) )
      {
        QgsDebugMsg( "extent for " + *it + " is invalid! (ignoring)" );
        continue;
      }

      QgsDebugMsg( "extent for " + *it  + " is " + extent.toString( 3 )  + "." );

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
          QgsDebugMsg( "'"  + ( *it )  + "' is queryable." );
          canIdentify = true;
        }
      }
    }
  }

  if ( canIdentify )
  {
    capability = identifyCapabilities();
    if ( capability )
    {
      capability |= Identify;
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
    const QgsWmsStyleProperty &style = layer.style[j];

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
    metadata += style.name;
    metadata += "</td></tr>";

    // Layer Style Title
    metadata += "<tr><th class=\"glossy\">";
    metadata += tr( "Title" );
    metadata += "</th>";
    metadata += "<td>";
    metadata += style.title;
    metadata += "</td></tr>";

    // Layer Style Abstract
    metadata += "<tr><th class=\"glossy\">";
    metadata += tr( "Abstract" );
    metadata += "</th>";
    metadata += "<td>";
    metadata += style.abstract;
    metadata += "</td></tr>";

    // LegendURLs
    if ( !style.legendUrl.isEmpty() )
    {
      metadata += "<tr><th class=\"glossy\">";
      metadata += tr( "LegendURLs" );
      metadata += "</th>";
      metadata += "<td><table>";
      metadata += "<tr><th>Format</th><th>URL</th></tr>";
      for ( int k = 0; k < style.legendUrl.size(); k++ )
      {
        const QgsWmsLegendUrlProperty &l = style.legendUrl[k];
        metadata += "<tr><td>" + l.format + "</td><td>" + l.onlineResource.xlinkHref + "</td></tr>";
      }
      metadata += "</table></td></tr>";
    }

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

  if ( !mSettings.mTiled )
  {
    metadata += "&nbsp;<a href=\"#selectedlayers\">";
    metadata += tr( "Selected Layers" );
    metadata += "</a>&nbsp;<a href=\"#otherlayers\">";
    metadata += tr( "Other Layers" );
    metadata += "</a>";
  }
  else
  {
    metadata += "&nbsp;<a href=\"#tilesetproperties\">";
    metadata += tr( "Tile Layer Properties" );
    metadata += "</a> ";

    metadata += "&nbsp;<a href=\"#cachestats\">";
    metadata += tr( "Cache Stats" );
    metadata += "</a> ";
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

  metadata += "<tr><td>";
  metadata += tr( "GetLegendGraphic" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += getLegendGraphicUrl() + ( mSettings.mIgnoreGetMapUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : "" );
  metadata += "</td></tr>";

  if ( mSettings.mTiled )
  {
    metadata += "<tr><td>";
    metadata += tr( "Tile Layer Count" );
    metadata += "</td>";
    metadata += "<td>";
    metadata += QString::number( mCaps.mTileLayersSupported.size() );
    metadata += "</td></tr>";

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
            ++it )
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
            ++it )
      {
        metadata += QString( "%1:%2<br>" ).arg( it.key() ).arg( it.value() );
      }
      metadata += "</td></tr>";
    }

    // GetFeatureInfo Request Formats
    metadata += "<tr><td>";
    metadata += tr( "Identify Formats" );
    metadata += "</td>";
    metadata += "<td>";
    metadata += mTileLayer->infoFormats.join( "<br />" );
    metadata += "</td></tr>";
  }
  else
  {
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
  }

  // Close the nested table
  metadata += "</table>";
  metadata += "</td></tr>";

  // Layer properties
  if ( !mSettings.mTiled )
  {
    metadata += "<tr><th class=\"glossy\"><a name=\"selectedlayers\"></a>";
    metadata += tr( "Selected Layers" );
    metadata += "</th></tr>";

    int n = 0;
    for ( int i = 0; i < mCaps.mLayersSupported.size(); i++ )
    {
      if ( mSettings.mActiveSubLayers.contains( mCaps.mLayersSupported[i].name ) )
      {
        metadata += layerMetadata( mCaps.mLayersSupported[i] );
        n++;
      }
    } // for each layer

    // Layer properties
    if ( n < mCaps.mLayersSupported.size() )
    {
      metadata += "<tr><th class=\"glossy\"><a name=\"otherlayers\"></a>";
      metadata += tr( "Other Layers" );
      metadata += "</th></tr>";

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
    metadata += "<tr><th class=\"glossy\"><a name=\"tilesetproperties\"></a>";
    metadata += tr( "Tileset Properties" );
    metadata += "</th></tr>";

    // Iterate through tilesets
    metadata += "<tr><td>";

    metadata += "<table width=\"100%\">";

    foreach ( const QgsWmtsTileLayer &l, mCaps.mTileLayersSupported )
    {
      metadata += "<tr><th class=\"glossy\">";
      metadata += tr( "Identifier" );
      metadata += "</th><th class=\"glossy\">";
      metadata += tr( "Tile mode" );
      metadata += "</th></tr>";

      metadata += "<tr><td>";
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
        metadata += tr( "Invalid tile mode" );
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
      metadata += l.identifier == mSettings.mActiveSubLayers.join( "," ) ? tr( "Yes" ) : tr( "No" );
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
      metadata += "<td>";
      metadata += "<table><tr>";
      metadata += "<td class=\"glossy\">";
      metadata += tr( "CRS" );
      metadata += "</td>";
      metadata += "<td class=\"glossy\">";
      metadata += tr( "Bounding Box" );
      metadata += "</td>";
      for ( int i = 0; i < l.boundingBoxes.size(); i++ )
      {
        metadata += "<tr><td>";
        metadata += l.boundingBoxes[i].crs;
        metadata += "</td><td>";
        metadata += l.boundingBoxes[i].box.toString();
        metadata += "</td></tr>";
      }
      metadata += "</table></td></tr>";

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

    if ( mTileMatrixSet )
    {
      // Iterate through tilesets
      metadata += "<tr><td><table width=\"100%\">";

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
                  .arg( tr( "Selected tile matrix set " ) ).arg( mSettings.mTileMatrixSetId )
                  .arg( tr( "Scale" ) )
                  .arg( tr( "Tile size [px]" ) )
                  .arg( tr( "Tile size [mu]" ) )
                  .arg( tr( "Matrix size" ) )
                  .arg( tr( "Matrix extent [mu]" ) )
                  .arg( tr( "Bounds" ) )
                  .arg( tr( "Width" ) ).arg( tr( "Height" ) )
                  .arg( tr( "Top" ) ).arg( tr( "Left" ) )
                  .arg( tr( "Bottom" ) ).arg( tr( "Right" ) );

      foreach ( QVariant res, property( "resolutions" ).toList() )
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
          metadata += QString( "<td title=\"%1<br>%2\"><font color=\"red\">%3</font></td>" )
                      .arg( tr( "%n missing row(s)", 0, ( int ) ceil(( mLayerExtent.yMaximum() - r.yMaximum() ) / th ) ) )
                      .arg( tr( "Layer's upper bound: %1" ).arg( mLayerExtent.yMaximum(), 0, 'f' ) )
                      .arg( r.yMaximum(), 0, 'f' );
        }
        else
        {
          metadata += QString( "<td>%1</td>" ).arg( r.yMaximum(), 0, 'f' );
        }

        // left
        if ( mLayerExtent.xMinimum() < r.xMinimum() )
        {
          metadata += QString( "<td title=\"%1<br>%2\"><font color=\"red\">%3</font></td>" )
                      .arg( tr( "%n missing column(s)", 0, ( int ) ceil(( r.xMinimum() - mLayerExtent.xMinimum() ) / tw ) ) )
                      .arg( tr( "Layer's left bound: %1" ).arg( mLayerExtent.xMinimum(), 0, 'f' ) )
                      .arg( r.xMinimum(), 0, 'f' );
        }
        else
        {
          metadata += QString( "<td>%1</td>" ).arg( r.xMinimum(), 0, 'f' );
        }

        // bottom
        if ( mLayerExtent.yMaximum() > r.yMaximum() )
        {
          metadata += QString( "<td title=\"%1<br>%2\"><font color=\"red\">%3</font></td>" )
                      .arg( tr( "%n missing row(s)", 0, ( int ) ceil(( mLayerExtent.yMaximum() - r.yMaximum() ) / th ) ) )
                      .arg( tr( "Layer's lower bound: %1" ).arg( mLayerExtent.yMaximum(), 0, 'f' ) )
                      .arg( r.yMaximum(), 0, 'f' );
        }
        else
        {
          metadata += QString( "<td>%1</td>" ).arg( r.yMaximum(), 0, 'f' );
        }

        // right
        if ( mLayerExtent.xMaximum() > r.xMaximum() )
        {
          metadata += QString( "<td title=\"%1<br>%2\"><font color=\"red\">%3</font></td>" )
                      .arg( tr( "%n missing column(s)", 0, ( int ) ceil(( mLayerExtent.xMaximum() - r.xMaximum() ) / tw ) ) )
                      .arg( tr( "Layer's right bound: %1" ).arg( mLayerExtent.xMaximum(), 0, 'f' ) )
                      .arg( r.xMaximum(), 0, 'f' );
        }
        else
        {
          metadata += QString( "<td>%1</td>" ).arg( r.xMaximum(), 0, 'f' );
        }

        metadata += "</tr>";
      }

      metadata += "</table></td></tr>";
    }

    const QgsWmsStatistics::Stat& stat = QgsWmsStatistics::statForUri( dataSourceUri() );

    metadata += "<tr><th class=\"glossy\"><a name=\"cachestats\"></a>";
    metadata += tr( "Cache stats" );
    metadata += "</th></tr>";

    metadata += "<tr><td><table width=\"100%\">";

    metadata += "<tr><th class=\"glossy\">";
    metadata += tr( "Property" );
    metadata += "</th>";
    metadata += "<th class=\"glossy\">";
    metadata += tr( "Value" );
    metadata += "</th></tr>";

    metadata += "<tr><td>";
    metadata += tr( "Hits" );
    metadata += "</td><td>";
    metadata += QString::number( stat.cacheHits );
    metadata += "</td></tr>";

    metadata += "<tr><td>";
    metadata += tr( "Misses" );
    metadata += "</td><td>";
    metadata += QString::number( stat.cacheMisses );
    metadata += "</td></tr>";

    metadata += "<tr><td>";
    metadata += tr( "Errors" );
    metadata += "</td><td>";
    metadata += QString::number( stat.errors );
    metadata += "</td></tr>";

    metadata += "</table></td></tr>";
  }

  metadata += "</table>";

  QgsDebugMsg( "exiting with '"  + metadata  + "'." );

  return metadata;
}

QgsRasterIdentifyResult QgsWmsProvider::identify( const QgsPoint & thePoint, QgsRaster::IdentifyFormat theFormat, const QgsRectangle &theExtent, int theWidth, int theHeight )
{
  QgsDebugMsg( QString( "theFormat = %1" ).arg( theFormat ) );

  QString format;
  format = mCaps.mIdentifyFormats.value( theFormat );
  if ( format.isEmpty() )
  {
    return QgsRasterIdentifyResult( ERROR( tr( "Format not supported" ) ) );
  }

  QgsDebugMsg( QString( "theFormat = %1 format = %2" ).arg( theFormat ).arg( format ) );

  QMap<int, QVariant> results;
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

    // Warning: this does not work well with poin/line vector layers where search rectangle
    // is based on pixel size (e.g. UMN Mapserver is using TOLERANCE layer param)

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

  // Mapserver (6.0.3, for example) does not seem to work with 1x1 pixel box
  // (seems to be a different issue, not the slownes of GDAL with ECW mentioned above)
  // so we have to enlarge it a bit
  if ( theWidth == 1 )
  {
    theWidth += 1;
    myExtent.setXMaximum( myExtent.xMaximum() + xRes );
  }

  if ( theHeight == 1 )
  {
    theHeight += 1;
    myExtent.setYMaximum( myExtent.yMaximum() + yRes );
  }

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
  bool changeXY = mCaps.shouldInvertAxisOrientation( mImageCrs );

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

  QList<QUrl> urls;
  QStringList layerList;

  if ( !mSettings.mTiled )
  {
    // Test for which layers are suitable for querying with
    for ( QStringList::const_iterator
          layers = mSettings.mActiveSubLayers.begin(),
          styles = mSettings.mActiveSubStyles.begin();
          layers != mSettings.mActiveSubLayers.end();
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
      setQueryItem( requestUrl, "SERVICE", "WMS" );
      setQueryItem( requestUrl, "VERSION", mCaps.mCapabilities.version );
      setQueryItem( requestUrl, "REQUEST", "GetFeatureInfo" );
      setQueryItem( requestUrl, "BBOX", bbox );
      setSRSQueryItem( requestUrl );
      setQueryItem( requestUrl, "WIDTH", QString::number( theWidth ) );
      setQueryItem( requestUrl, "HEIGHT", QString::number( theHeight ) );
      setQueryItem( requestUrl, "LAYERS", *layers );
      setQueryItem( requestUrl, "STYLES", *styles );
      setFormatQueryItem( requestUrl );
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

      layerList << *layers;
      urls << requestUrl;
    }
  }
  else if ( mTileLayer && mTileLayer->tileMode == WMTS )
  {
    // WMTS FeatureInfo
    double vres = theExtent.width() / theWidth;
    double tres = vres;

    const QgsWmtsTileMatrix *tm = 0;

    Q_ASSERT( mTileMatrixSet );
    Q_ASSERT( mTileMatrixSet->tileMatrices.size() > 0 );

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
                 .arg( qgsDoubleToString( mLayerExtent.xMinimum() ) )
                 .arg( qgsDoubleToString( mLayerExtent.yMinimum() ) )
                 .arg( mLayerExtent.width() )
                 .arg( mLayerExtent.height() )
               );

    QgsDebugMsg( QString( "view extent: %1,%2 %3x%4  res:%5" )
                 .arg( qgsDoubleToString( theExtent.xMinimum() ) )
                 .arg( qgsDoubleToString( theExtent.yMinimum() ) )
                 .arg( theExtent.width() )
                 .arg( theExtent.height() )
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

    int col = ( int ) floor(( thePoint.x() - tm->topLeft.x() ) / twMap );
    int row = ( int ) floor(( tm->topLeft.y() - thePoint.y() ) / thMap );
    double tx = tm->topLeft.x() + col * twMap;
    double ty = tm->topLeft.y() - row * thMap;
    int i   = ( thePoint.x() - tx ) / tres;
    int j   = ( ty - thePoint.y() ) / tres;

    QgsDebugMsg( QString( "col=%1 row=%2 i=%3 j=%4 tx=%5 ty=%6" ).arg( col ).arg( row ).arg( i ).arg( j ).arg( tx, 0, 'f', 1 ).arg( ty, 0, 'f', 1 ) );

    if ( mTileLayer->getFeatureInfoURLs.contains( format ) )
    {
      // REST
      QString url = mTileLayer->getFeatureInfoURLs[ format ];

      url.replace( "{layer}", mSettings.mActiveSubLayers[0], Qt::CaseInsensitive );
      url.replace( "{style}", mSettings.mActiveSubStyles[0], Qt::CaseInsensitive );
      url.replace( "{tilematrixset}", mTileMatrixSet->identifier, Qt::CaseInsensitive );
      url.replace( "{tilematrix}", tm->identifier, Qt::CaseInsensitive );
      url.replace( "{tilerow}", QString::number( row ), Qt::CaseInsensitive );
      url.replace( "{tilecol}", QString::number( col ), Qt::CaseInsensitive );
      url.replace( "{i}", QString::number( i ), Qt::CaseInsensitive );
      url.replace( "{j}", QString::number( j ), Qt::CaseInsensitive );

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
      setQueryItem( url, "SERVICE", "WMTS" );
      setQueryItem( url, "REQUEST", "GetFeatureInfo" );
      setQueryItem( url, "VERSION", mCaps.mCapabilities.version );
      setQueryItem( url, "LAYER", mSettings.mActiveSubLayers[0] );
      setQueryItem( url, "STYLE", mSettings.mActiveSubStyles[0] );
      setQueryItem( url, "INFOFORMAT", format );
      setQueryItem( url, "TILEMATRIXSET", mTileMatrixSet->identifier );
      setQueryItem( url, "TILEMATRIX", tm->identifier );

      for ( QHash<QString, QString>::const_iterator it = mSettings.mTileDimensionValues.constBegin(); it != mSettings.mTileDimensionValues.constEnd(); ++it )
      {
        setQueryItem( url, it.key(), it.value() );
      }

      setQueryItem( url, "TILEROW", QString::number( row ) );
      setQueryItem( url, "TILECOL", QString::number( col ) );
      setQueryItem( url, "I", qgsDoubleToString( i ) );
      setQueryItem( url, "J", qgsDoubleToString( j ) );

      urls << url;
      layerList << mSettings.mActiveSubLayers[0];
    }
    else
    {
      QgsDebugMsg( QString( "No KVP and no feature info url for format %1" ).arg( format ) );
    }
  }

  for ( int count = 0; count < urls.size(); count++ )
  {
    const QUrl &requestUrl = urls[count];

    QgsDebugMsg( QString( "getfeatureinfo: %1" ).arg( requestUrl.toString() ) );
    QNetworkRequest request( requestUrl );
    mSettings.authorization().setAuthorization( request );
    mIdentifyReply = QgsNetworkAccessManager::instance()->get( request );
    connect( mIdentifyReply, SIGNAL( finished() ), this, SLOT( identifyReplyFinished() ) );

    QEventLoop loop;
    mIdentifyReply->setProperty( "eventLoop", QVariant::fromValue( qobject_cast<QObject *>( &loop ) ) );
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

        if ( isXml && parseServiceExceptionReportDom( body, mErrorCaption, mError ) )
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
      results.insert( results.size(), QString::fromUtf8( mIdentifyResultBodies.value( 0 ) ) );
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
      int jsonPart = -1;
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
        else if ( jsonPart == -1 && mIdentifyResultHeaders[i].value( "Content-Type" ).contains( "json" ) )
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

        QGis::WkbType wkbType;
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
          if ( ! ok )
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
          int ret = gml.getFeatures( gmlByteArray, &wkbType );
#ifdef QGISDEBUG
          QgsDebugMsg( QString( "parsing result = %1" ).arg( ret ) );
#else
          Q_UNUSED( ret );
#endif
          // TODO: all features coming from this layer should probably have the same CRS
          // the same as this layer, because layerExtentToOutputExtent() may be used
          // for results -> verify CRS and reprojects if necessary
          QMap<QgsFeatureId, QgsFeature* > features = gml.featuresMap();
          QgsCoordinateReferenceSystem featuresCrs = gml.crs();
          QgsDebugMsg( QString( "%1 features read, crs: %2 %3" ).arg( features.size() ).arg( featuresCrs.authid() ).arg( featuresCrs.description() ) );
          QgsCoordinateTransform *coordinateTransform = 0;
          if ( featuresCrs.isValid() && featuresCrs != crs() )
          {
            coordinateTransform = new QgsCoordinateTransform( featuresCrs, crs() );
          }
          QgsFeatureStore featureStore( fields, crs() );
          QMap<QString, QVariant> params;
          params.insert( "sublayer", layerList[count] );
          params.insert( "featureType", featureTypeName );
          params.insert( "getFeatureInfoUrl", requestUrl.toString() );
          featureStore.setParams( params );
          foreach ( QgsFeatureId id, features.keys() )
          {
            QgsFeature * feature = features.value( id );

            QgsDebugMsg( QString( "feature id = %1 : %2 attributes" ).arg( id ).arg( feature->attributes().size() ) );

            if ( coordinateTransform && feature->geometry() )
            {
              feature->geometry()->transform( *coordinateTransform );
            }
            featureStore.features().append( QgsFeature( *feature ) );
          }
          featureStoreList.append( featureStore );
          delete coordinateTransform;
        }
        // It is suspicious if we guessed feature types from GML but could not get
        // features from it. Either we geuessed wrong schema or parsing features failed.
        // Report it as error so that user can switch to another format in results dialog.
        if ( xsdPart < 0 && !featureTypeNames.isEmpty() && featureStoreList.isEmpty() )
        {
          QgsError err = ERROR( tr( "Cannot identify" ) );
          err.append( tr( "Result parsing failed. %1 feature types were guessed from gml (%2) but no features were parsed." ).arg( featureTypeNames.size() ).arg( featureTypeNames.join( "," ) ) );
          QgsDebugMsg( "parsing GML error: " + err.message() );
          return QgsRasterIdentifyResult( err );
        }
        results.insert( results.size(), qVariantFromValue( featureStoreList ) );
      }
      else if ( jsonPart != -1 )
      {
        QString json = QString::fromUtf8( mIdentifyResultBodies.value( jsonPart ) );
        json.prepend( "(" ).append( ")" );

        QScriptEngine engine;
        engine.evaluate( "function json_stringify(obj) { return JSON.stringify(obj); }" );
        QScriptValue json_stringify = engine.globalObject().property( "json_stringify" );
        Q_ASSERT( json_stringify.isFunction() );

        QScriptValue result = engine.evaluate( json );

        QgsFeatureStoreList featureStoreList;
        QgsCoordinateTransform *coordinateTransform = 0;

        try
        {
          QgsDebugMsg( QString( "result:%1" ).arg( result.toString() ) );

          if ( !result.isObject() )
            throw QString( "object expected" );

          if ( result.property( "type" ).toString() != "FeatureCollection" )
            throw QString( "type FeatureCollection expected: %1" ).arg( result.property( "type" ).toString() );

          if ( result.property( "crs" ).isValid() )
          {
            QString crsType = result.property( "crs" ).property( "type" ).toString();
            QString crsText;
            if ( crsType == "name" )
              crsText = result.property( "crs" ).property( "name" ).toString();
            else if ( crsType == "EPSG" )
              crsText = QString( "%1:%2" ).arg( crsType ).arg( result.property( "crs" ).property( "properties" ).property( "code" ).toString() );
            else
            {
              QgsDebugMsg( QString( "crs not supported:%1" ).arg( result.property( "crs" ).toString() ) );
            }

            QgsCoordinateReferenceSystem featuresCrs;
            featuresCrs.createFromOgcWmsCrs( crsText );

            if ( !featuresCrs.isValid() )
              throw QString( "CRS %1 invalid" ).arg( crsText );

            if ( featuresCrs.isValid() && featuresCrs != crs() )
            {
              coordinateTransform = new QgsCoordinateTransform( featuresCrs, crs() );
            }
          }

          QScriptValue fc = result.property( "features" );
          if ( !fc.isArray() )
            throw QString( "FeatureCollection array expected" );

          QScriptValue f;
          for ( int i = 0; f = fc.property( i ), f.isValid(); i++ )
          {
            QgsDebugMsg( QString( "feature %1" ).arg( i ) );

            QScriptValue props = f.property( "properties" );
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

            if ( f.property( "geometry" ).isValid() )
            {
              QScriptValue geom = json_stringify.call( QScriptValue(), QScriptValueList() << f.property( "geometry" ) );
              if ( geom.isString() )
              {
                OGRGeometryH ogrGeom = OGR_G_CreateGeometryFromJson( geom.toString().toUtf8() );
                if ( ogrGeom )
                {
                  size_t wkbSize = OGR_G_WkbSize( ogrGeom );
                  unsigned char *wkb = new unsigned char[ wkbSize ];
                  OGR_G_ExportToWkb( ogrGeom, ( OGRwkbByteOrder ) QgsApplication::endian(), wkb );
                  OGR_G_DestroyGeometry( ogrGeom );

                  feature.setGeometryAndOwnership( wkb, wkbSize );

                  if ( coordinateTransform && feature.geometry() )
                  {
                    feature.geometry()->transform( *coordinateTransform );
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
            params.insert( "sublayer", layerList[count] );
            params.insert( "featureType", QString( "%1_%2" ).arg( count ).arg( i ) );
            params.insert( "getFeatureInfoUrl", requestUrl.toString() );
            featureStore.setParams( params );

            feature.setValid( true );
            featureStore.features().append( feature );

            featureStoreList.append( featureStore );
          }
        }
        catch ( const QString &err )
        {
          QgsDebugMsg( QString( "JSON error: %1\nResult: %2" ).arg( err ).arg( QString::fromUtf8( mIdentifyResultBodies.value( jsonPart ) ) ) );
        }

        delete coordinateTransform;

        results.insert( results.size(), qVariantFromValue( featureStoreList ) );
      }
    }
  }

  return QgsRasterIdentifyResult( theFormat, results );
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
      mIdentifyReply->setProperty( "eventLoop", QVariant::fromValue( qobject_cast<QObject *>( loop ) ) );
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
    }

    QgsNetworkReplyParser parser( mIdentifyReply );
    if ( !parser.isValid() )
    {
      QgsDebugMsg( "Cannot parse reply" );
      mErrorFormat = "text/plain";
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
    mErrorFormat = "text/plain";
    mError = tr( "Map getfeatureinfo error: %1 [%2]" ).arg( mIdentifyReply->errorString() ).arg( mIdentifyReply->url().toString() );
    emit statusChanged( mError );
    QgsMessageLog::logMessage( mError, tr( "WMS" ) );
  }

  if ( loop )
    QMetaObject::invokeMethod( loop, "quit", Qt::QueuedConnection );

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

QUrl QgsWmsProvider::getLegendGraphicFullURL( double scale, const QgsRectangle& visibleExtent )
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

  if ( !url.hasQueryItem( "SERVICE" ) )
    setQueryItem( url, "SERVICE", "WMS" );
  if ( !url.hasQueryItem( "VERSION" ) )
    setQueryItem( url, "VERSION", mCaps.mCapabilities.version );
  if ( !url.hasQueryItem( "SLD_VERSION" ) )
    setQueryItem( url, "SLD_VERSION", "1.1.0" ); // can not determine SLD_VERSION
  if ( !url.hasQueryItem( "REQUEST" ) )
    setQueryItem( url, "REQUEST", "GetLegendGraphic" );
  if ( !url.hasQueryItem( "FORMAT" ) )
    setFormatQueryItem( url );
  if ( !url.hasQueryItem( "LAYER" ) )
    setQueryItem( url, "LAYER", mSettings.mActiveSubLayers[0] );
  if ( !url.hasQueryItem( "STYLE" ) )
    setQueryItem( url, "STYLE", mSettings.mActiveSubStyles[0] );

  // add config parameter related to resolution
  QSettings s;
  int defaultLegendGraphicResolution = s.value( "/qgis/defaultLegendGraphicResolution", 0 ).toInt();
  QgsDebugMsg( QString( "defaultLegendGraphicResolution: %1" ).arg( defaultLegendGraphicResolution ) );
  if ( defaultLegendGraphicResolution )
  {
    if ( mSettings.mDpiMode & dpiQGIS )
      setQueryItem( url, "DPI", QString::number( defaultLegendGraphicResolution ) );
    if ( mSettings.mDpiMode & dpiUMN )
    {
      setQueryItem( url, "MAP_RESOLUTION", QString::number( defaultLegendGraphicResolution ) );
      setQueryItem( url, "SCALE", QString::number( scale, 'f' ) );
    }
    if ( mSettings.mDpiMode & dpiGeoServer )
    {
      setQueryItem( url, "FORMAT_OPTIONS", QString( "dpi:%1" ).arg( defaultLegendGraphicResolution ) );
      setQueryItem( url, "SCALE", QString::number( scale, 'f' ) );
    }
  }

  if ( useContextualWMSLegend )
  {
    bool changeXY = mCaps.shouldInvertAxisOrientation( mImageCrs );
    setQueryItem( url, "BBOX", toParamValue( visibleExtent, changeXY ) );
    setSRSQueryItem( url );
  }

  QgsDebugMsg( QString( "getlegendgraphicrequest: %1" ).arg( url.toString() ) );
  return QUrl( url );
}

QImage QgsWmsProvider::getLegendGraphic( double scale, bool forceRefresh, const QgsRectangle* visibleExtent )
{
  // TODO manage return basing of getCapablity => avoid call if service is not available
  // some services doesn't expose getLegendGraphic in capabilities but adding LegendURL in
  // the layer tags inside capabilities
  QgsDebugMsg( "entering." );

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

  mError = "";

  QUrl url( getLegendGraphicFullURL( scale, mGetLegendGraphicExtent ) );
  if ( ! url.isValid() ) return QImage();
  Q_ASSERT( ! mLegendGraphicFetcher ); // or we could just remove it instead, hopefully will cancel download
  mLegendGraphicFetcher.reset( new QgsWmsLegendDownloadHandler( *QgsNetworkAccessManager::instance(), mSettings, url ) );
  if ( ! mLegendGraphicFetcher ) return QImage();
  connect( mLegendGraphicFetcher.data(), SIGNAL( finish( const QImage& ) ), this, SLOT( getLegendGraphicReplyFinished( const QImage& ) ) );
  connect( mLegendGraphicFetcher.data(), SIGNAL( progress( qint64, qint64 ) ), this, SLOT( getLegendGraphicReplyProgress( qint64, qint64 ) ) );
  mLegendGraphicFetcher->start( );

  QEventLoop loop;
  mLegendGraphicFetcher->setProperty( "eventLoop", QVariant::fromValue( qobject_cast<QObject *>( &loop ) ) );
  mLegendGraphicFetcher->setProperty( "legendScale", QVariant::fromValue( scale ) );
  mLegendGraphicFetcher->setProperty( "legendExtent", QVariant::fromValue( mapExtent.toRectF() ) );
  loop.exec( QEventLoop::ExcludeUserInputEvents );

  QgsDebugMsg( "exiting." );

  return mGetLegendGraphicImage;
}

QgsImageFetcher* QgsWmsProvider::getLegendGraphicFetcher( const QgsMapSettings* mapSettings )
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

  QUrl url = getLegendGraphicFullURL( scale, mapExtent );
  if ( ! url.isValid() ) return 0;

  if ( mapExtent == mGetLegendGraphicExtent &&
       scale == mGetLegendGraphicScale &&
       ! mGetLegendGraphicImage.isNull() )
  {
    QgsDebugMsg( "Emitting cached image fetcher" );
    // return a cached image, skipping the load
    return new QgsCachedImageFetcher( mGetLegendGraphicImage );
  }
  else
  {
    QgsImageFetcher* fetcher =  new QgsWmsLegendDownloadHandler( *QgsNetworkAccessManager::instance(), mSettings, url );
    fetcher->setProperty( "legendScale", QVariant::fromValue( scale ) );
    fetcher->setProperty( "legendExtent", QVariant::fromValue( mapExtent.toRectF() ) );
    connect( fetcher, SIGNAL( finish( const QImage& ) ), this, SLOT( getLegendGraphicReplyFinished( const QImage& ) ) );
    return fetcher;
  }
}

void QgsWmsProvider::getLegendGraphicReplyFinished( const QImage& img )
{
  QgsDebugMsg( "entering." );

  QObject* reply = sender();

  if ( ! img.isNull() )
  {
    mGetLegendGraphicImage = img;
    mGetLegendGraphicExtent = QgsRectangle( reply->property( "legendExtent" ).value<QRectF>() );
    mGetLegendGraphicScale = reply->property( "legendScale" ).value<double>();

#ifdef QGISDEBUG
    QString filename = QDir::tempPath() + "/GetLegendGraphic.png";
    mGetLegendGraphicImage.save( filename );
    QgsDebugMsg( "saved GetLegendGraphic result in debug file: " + filename );
#endif
  }

  if ( reply == mLegendGraphicFetcher.data() )
  {
    QEventLoop *loop = qobject_cast< QEventLoop *>( reply->property( "eventLoop" ).value< QObject *>() );
    if ( loop )
      QMetaObject::invokeMethod( loop, "quit", Qt::QueuedConnection );
    mLegendGraphicFetcher.reset();
  }
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


// -----------------

QgsWmsImageDownloadHandler::QgsWmsImageDownloadHandler( const QString& providerUri, const QUrl& url, const QgsWmsAuthorization& auth, QImage* image )
    : mProviderUri( providerUri )
    , mCachedImage( image )
    , mEventLoop( new QEventLoop )
    , mNAM( new QgsNetworkAccessManager )
{
  mNAM->setupDefaultProxyAndCache();

  QNetworkRequest request( url );
  auth.setAuthorization( request );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  mCacheReply = mNAM->get( request );
  connect( mCacheReply, SIGNAL( finished() ), this, SLOT( cacheReplyFinished() ) );
  connect( mCacheReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( cacheReplyProgress( qint64, qint64 ) ) );

  Q_ASSERT( mCacheReply->thread() == QThread::currentThread() );

}

QgsWmsImageDownloadHandler::~QgsWmsImageDownloadHandler()
{
  delete mNAM;
  delete mEventLoop;
}

void QgsWmsImageDownloadHandler::downloadBlocking()
{
  mEventLoop->exec( QEventLoop::ExcludeUserInputEvents );

  Q_ASSERT( mCacheReply == 0 );
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
      mCacheReply = mNAM->get( QNetworkRequest( redirect.toUrl() ) );
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
    else if ( contentType.startsWith( "image/", Qt::CaseInsensitive ) ||
              contentType.compare( "application/octet-stream", Qt::CaseInsensitive ) == 0 )
    {
      QgsMessageLog::logMessage( tr( "Returned image is flawed [Content-Type:%1; URL:%2]" )
                                 .arg( contentType ).arg( mCacheReply->url().toString() ), tr( "WMS" ) );
    }
    else
    {
      QString errorTitle, errorText;
      if ( contentType.toLower() == "text/xml" && QgsWmsProvider::parseServiceExceptionReportDom( text, errorTitle, errorText ) )
      {
        QgsMessageLog::logMessage( tr( "Map request error (Title:%1; Error:%2; URL: %3)" )
                                   .arg( errorTitle ).arg( errorText )
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

      finish();
      return;
    }

    mCacheReply->deleteLater();
    mCacheReply = 0;

    finish();
  }
  else
  {
    QgsWmsStatistics::Stat& stat = QgsWmsStatistics::statForUri( mProviderUri );

    stat.errors++;
    if ( stat.errors < 100 )
    {
      QgsMessageLog::logMessage( tr( "Map request failed [error:%1 url:%2]" ).arg( mCacheReply->errorString() ).arg( mCacheReply->url().toString() ), tr( "WMS" ) );
    }
    else if ( stat.errors == 100 )
    {
      QgsMessageLog::logMessage( tr( "Not logging more than 100 request errors." ), tr( "WMS" ) );
    }

    mCacheReply->deleteLater();
    mCacheReply = 0;

    finish();
  }
}

void QgsWmsImageDownloadHandler::cacheReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of map downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsg( msg );
}


// ----------


QgsWmsTiledImageDownloadHandler::QgsWmsTiledImageDownloadHandler( const QString& providerUri, const QgsWmsAuthorization& auth, int tileReqNo, const QList<QgsWmsTiledImageDownloadHandler::TileRequest>& requests, QImage* cachedImage, const QgsRectangle& cachedViewExtent, bool smoothPixmapTransform )
    : mProviderUri( providerUri )
    , mAuth( auth )
    , mCachedImage( cachedImage )
    , mCachedViewExtent( cachedViewExtent )
    , mEventLoop( new QEventLoop )
    , mNAM( new QgsNetworkAccessManager )
    , mTileReqNo( tileReqNo )
    , mSmoothPixmapTransform( smoothPixmapTransform )
{
  mNAM->setupDefaultProxyAndCache();

  foreach ( const TileRequest& r, requests )
  {
    QNetworkRequest request( r.url );
    auth.setAuthorization( request );
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileReqNo ), mTileReqNo );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileIndex ), r.index );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRect ), r.rect );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( TileRetry ), 0 );

    QNetworkReply *reply = mNAM->get( request );
    connect( reply, SIGNAL( finished() ), this, SLOT( tileReplyFinished() ) );

    mReplies << reply;
  }
}

QgsWmsTiledImageDownloadHandler::~QgsWmsTiledImageDownloadHandler()
{
  delete mNAM;
  delete mEventLoop;
}

void QgsWmsTiledImageDownloadHandler::downloadBlocking()
{
  mEventLoop->exec( QEventLoop::ExcludeUserInputEvents );

  Q_ASSERT( mReplies.isEmpty() );
}


void QgsWmsTiledImageDownloadHandler::tileReplyFinished()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply*>( sender() );

#if defined(QGISDEBUG)
  bool fromCache = reply->attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool();
  QgsWmsStatistics::Stat& stat = QgsWmsStatistics::statForUri( mProviderUri );
  if ( fromCache )
    stat.cacheHits++;
  else
    stat.cacheMisses++;
#endif
#if defined(QGISDEBUG)
  QgsDebugMsgLevel( "raw headers:", 3 );
  foreach ( const QNetworkReply::RawHeaderPair &pair, reply->rawHeaderPairs() )
  {
    QgsDebugMsgLevel( QString( " %1:%2" )
                      .arg( QString::fromUtf8( pair.first ) )
                      .arg( QString::fromUtf8( pair.second ) ), 3 );
  }
#endif

  if ( mNAM->cache() )
  {
    QNetworkCacheMetaData cmd = mNAM->cache()->metaData( reply->request().url() );

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

    mNAM->cache()->updateMetaData( cmd );
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
               .arg( reply->errorString() )
               .arg( reply->url().toString() )
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
      reply = mNAM->get( request );
      mReplies << reply;

      connect( reply, SIGNAL( finished() ), this, SLOT( tileReplyFinished() ) );

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
    if ( !contentType.startsWith( "image/", Qt::CaseInsensitive ) &&
         contentType.compare( "application/octet-stream", Qt::CaseInsensitive ) != 0 )
    {
      QByteArray text = reply->readAll();
      QString errorTitle, errorText;
      if ( contentType.toLower() == "text/xml" && QgsWmsProvider::parseServiceExceptionReportDom( text, errorTitle, errorText ) )
      {
        QgsMessageLog::logMessage( tr( "Tile request error (Title:%1; Error:%2; URL: %3)" )
                                   .arg( errorTitle ).arg( errorText )
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

      mReplies.removeOne( reply );
      reply->deleteLater();

      if ( mReplies.isEmpty() )
        finish();

      return;
    }

    // only take results from current request number
    if ( mTileReqNo == tileReqNo )
    {
      double cr = mCachedViewExtent.width() / mCachedImage->width();

      QRectF dst(( r.left() - mCachedViewExtent.xMinimum() ) / cr,
                 ( mCachedViewExtent.yMaximum() - r.bottom() ) / cr,
                 r.width() / cr,
                 r.height() / cr );

      QgsDebugMsg( QString( "tile reply: length %1" ).arg( reply->bytesAvailable() ) );

      QImage myLocalImage = QImage::fromData( reply->readAll() );

      if ( !myLocalImage.isNull() )
      {
        QPainter p( mCachedImage );
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

    mReplies.removeOne( reply );
    reply->deleteLater();

    if ( mReplies.isEmpty() )
      finish();

  }
  else
  {
    QgsWmsStatistics::Stat& stat = QgsWmsStatistics::statForUri( mProviderUri );
    stat.errors++;

    repeatTileRequest( reply->request() );

    mReplies.removeOne( reply );
    reply->deleteLater();

    if ( mReplies.isEmpty() )
      finish();
  }

#if 0
  const QgsWmsStatistics::Stat& stat = QgsWmsStatistics::statForUri( mProviderUri );
  emit statusChanged( tr( "%n tile requests in background", "tile request count", mReplies.count() )
                      + tr( ", %n cache hits", "tile cache hits", stat.cacheHits )
                      + tr( ", %n cache misses.", "tile cache missed", stat.cacheMisses )
                      + tr( ", %n errors.", "errors", stat.errors )
                    );
#endif
}


void QgsWmsTiledImageDownloadHandler::repeatTileRequest( QNetworkRequest const &oldRequest )
{
  QgsWmsStatistics::Stat& stat = QgsWmsStatistics::statForUri( mProviderUri );

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

  QSettings s;
  int maxRetry = s.value( "/qgis/defaultTileMaxRetry", "3" ).toInt();
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

  QNetworkReply *reply = mNAM->get( request );
  mReplies << reply;
  connect( reply, SIGNAL( finished() ), this, SLOT( tileReplyFinished() ) );
}

QString QgsWmsProvider::toParamValue( const QgsRectangle& rect, bool changeXY )
{
  // Warning: does not work with scientific notation
  return QString( changeXY ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
         .arg( qgsDoubleToString( rect.xMinimum() ) )
         .arg( qgsDoubleToString( rect.yMinimum() ) )
         .arg( qgsDoubleToString( rect.xMaximum() ) )
         .arg( qgsDoubleToString( rect.yMaximum() ) );
}

void QgsWmsProvider::setSRSQueryItem( QUrl& url )
{
  QString crsKey = "SRS"; //SRS in 1.1.1 and CRS in 1.3.0
  if ( mCaps.mCapabilities.version == "1.3.0" || mCaps.mCapabilities.version == "1.3" )
  {
    crsKey = "CRS";
  }
  setQueryItem( url, crsKey, mImageCrs );
}

// ----------

QgsWmsLegendDownloadHandler::QgsWmsLegendDownloadHandler( QgsNetworkAccessManager& networkAccessManager, const QgsWmsSettings& settings, const QUrl& url )
    : mNetworkAccessManager( networkAccessManager )
    , mSettings( settings )
    , mReply( 0 )
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
  mReply = 0;
}

/* public */
void
QgsWmsLegendDownloadHandler::start( )
{
  Q_ASSERT( mVisitedUrls.empty() );
  startUrl( mInitialUrl );
}

/* private */
void
QgsWmsLegendDownloadHandler::startUrl( const QUrl& url )
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
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  mReply = mNetworkAccessManager.get( request );
  connect( mReply, SIGNAL( error( QNetworkReply::NetworkError ) ), this, SLOT( errored( QNetworkReply::NetworkError ) ) );
  connect( mReply, SIGNAL( finished() ), this, SLOT( finished() ) );
  connect( mReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( progressed( qint64, qint64 ) ) );
}

void
QgsWmsLegendDownloadHandler::sendError( const QString& msg )
{
  QgsDebugMsg( QString( "emitting error: %1" ).arg( msg ) );
  Q_ASSERT( mReply );
  mReply->deleteLater();
  mReply = 0;
  emit error( msg );
}

void
QgsWmsLegendDownloadHandler::sendSuccess( const QImage& img )
{
  QgsDebugMsg( QString( "emitting finish: %1x%2 image" ).arg( img.width() ).arg( img.height() ) );
  Q_ASSERT( mReply );
  mReply->deleteLater();
  mReply = 0;
  emit finish( img );
}

void
QgsWmsLegendDownloadHandler::errored( QNetworkReply::NetworkError /* code */ )
{
  if ( ! mReply ) return;

  sendError( mReply->errorString() );
}

void
QgsWmsLegendDownloadHandler::finished( )
{
  if ( ! mReply ) return;

  // or ::errored() should have been called before ::finished
  Q_ASSERT( mReply->error() == QNetworkReply::NoError );

  QgsDebugMsg( "reply ok" );
  QVariant redirect = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
  if ( !redirect.isNull() )
  {
    mReply->deleteLater();
    mReply = 0;
    startUrl( redirect.toUrl() );
    return;
  }

  QVariant status = mReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
  if ( !status.isNull() && status.toInt() >= 400 )
  {
    QVariant phrase = mReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
    QString msg( tr( "GetLegendGraphic request error" ) );
    msg += QString( " - " );
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

QgsCachedImageFetcher::QgsCachedImageFetcher( const QImage& img )
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
