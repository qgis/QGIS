/***************************************************************************
  qgswcsprovider.cpp  -  QGIS Data provider for
                         OGC Web Coverage Service layers
                             -------------------
    begin                : 2 July, 2012
    copyright            : (C) (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail.com

    Based on qgswmsprovider.cpp written by Brendan Morley.

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

// time to wait for an answer without emitting dataChanged()
#define WCS_THRESHOLD 200

#include "qgslogger.h"
#include "qgswcsprovider.h"
//#include "qgswcsconnection.h"
#include "qgscoordinatetransform.h"
#include "qgsdatasourceuri.h"
#include "qgsrasterlayer.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsmessageoutput.h"
#include "qgsmessagelog.h"

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

#ifdef QGISDEBUG
#include <QFile>
#include <QDir>
#endif

#include "gdalwarper.h"
#include "ogr_spatialref.h"
#include "cpl_conv.h"
#include "cpl_string.h"

static QString WCS_KEY = "wcs";
static QString WCS_DESCRIPTION = "OGC Web Coverage Service version 1.0/1.1 data provider";

static QString DEFAULT_LATLON_CRS = "CRS:84";

QgsWcsProvider::QgsWcsProvider( QString const &uri )
    : QgsRasterDataProvider( uri )
    , mHttpUri( QString::null )
    , mImageCrs( DEFAULT_LATLON_CRS )
    , mCachedImage( 0 )
    , mCacheReply( 0 )
    , mCachedViewExtent( 0 )
    , mCoordinateTransform( 0 )
    , mExtentDirty( true )
    , mGetFeatureInfoUrlBase( "" )
    , mErrors( 0 )
    , mUserName( QString::null )
    , mPassword( QString::null )
{
  QgsDebugMsg( "constructing with uri '" + mHttpUri + "'." );

  // assume this is a valid layer until we determine otherwise
  mValid = true;

  parseUri( uri );

  QgsDebugMsg( "exiting constructor." );
}

void QgsWcsProvider::parseUri( QString uriString )
{

  QgsDebugMsg( "uriString = " + uriString );
  QgsDataSourceURI uri;
  uri.setEncodedUri( uriString );

  mMaxWidth = 0;
  mMaxHeight = 0;

  mHttpUri = uri.param( "url" );
  mBaseUrl = prepareUri( mHttpUri );
  QgsDebugMsg( "mBaseUrl = " + mBaseUrl );

  mIgnoreGetMapUrl = uri.hasParam( "IgnoreGetMapUrl" );
  mIgnoreAxisOrientation = uri.hasParam( "IgnoreAxisOrientation" ); // must be before parsing!
  mInvertAxisOrientation = uri.hasParam( "InvertAxisOrientation" ); // must be before parsing!

  mUserName = uri.param( "username" );
  QgsDebugMsg( "set username to " + mUserName );

  mPassword = uri.param( "password" );
  QgsDebugMsg( "set password to " + mPassword );

  mIdentifier = uri.param( "identifier" );

  setFormat( uri.param( "format" ) );

  setCoverageCrs( uri.param( "crs" ) );
  mCrs.createFromOgcWmsCrs( uri.param( "crs" ) );
}

QString QgsWcsProvider::prepareUri( QString uri ) const
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

QgsWcsProvider::~QgsWcsProvider()
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
}

QString QgsWcsProvider::baseUrl() const
{
  return mBaseUrl;
}

QString QgsWcsProvider::getMapUrl() const
{
  return "TODO";
  /*
  return mCapabilities.capability.request.getMap.dcpType.size() == 0
         ? mBaseUrl
         : prepareUri( mCapabilities.capability.request.getMap.dcpType.front().http.get.onlineResource.xlinkHref );
  */
}


QString QgsWcsProvider::getFeatureInfoUrl() const
{
  return "TODO";
  /*
  return mCapabilities.capability.request.getFeatureInfo.dcpType.size() == 0
         ? mBaseUrl
         : prepareUri( mCapabilities.capability.request.getFeatureInfo.dcpType.front().http.get.onlineResource.xlinkHref );
  */
}

QString QgsWcsProvider::format() const
{
  return mFormat;
}

void QgsWcsProvider::setFormat( QString const & format )
{
  QgsDebugMsg( "Setting format to " + format + "." );
  mFormat = format;
}


void QgsWcsProvider::setCoverageCrs( QString const & crs )
{
  QgsDebugMsg( "Setting coverage CRS to " + crs + "." );

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
}

void QgsWcsProvider::setQueryItem( QUrl &url, QString item, QString value )
{
  url.removeQueryItem( item );
  url.addQueryItem( item, value );
}

QImage *QgsWcsProvider::draw( QgsRectangle  const &viewExtent, int pixelWidth, int pixelHeight )
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

  //according to the WCS spec for 1.3, some CRS have inverted axis
  bool changeXY = false;
  // TODO
  //if ( !mIgnoreAxisOrientation && ( mCapabilities.version == "1.3.0" || mCapabilities.version == "1.3" ) )
  //{
    ////create CRS from string
    //QgsCoordinateReferenceSystem theSrs;
    //if ( theSrs.createFromOgcWcsCrs( mImageCrs ) && theSrs.axisInverted() )
    //{
      //changeXY = true;
    //}
  //}

  if ( mInvertAxisOrientation )
    changeXY = !changeXY;

  // compose the URL query string for the WCS server.
  QString crsKey = "SRS"; //SRS in 1.1.1 and CRS in 1.3.0
  // TODO
  /*
  if ( mCapabilities.version == "1.3.0" || mCapabilities.version == "1.3" )
  {
    crsKey = "CRS";
  }
  */
  // Bounding box in WMS format (Warning: does not work with scientific notation)
  QString bbox = QString( changeXY ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
                 .arg( viewExtent.xMinimum(), 0, 'f', 16 )
                 .arg( viewExtent.yMinimum(), 0, 'f', 16 )
                 .arg( viewExtent.xMaximum(), 0, 'f', 16 )
                 .arg( viewExtent.yMaximum(), 0, 'f', 16 );

  QUrl url( mIgnoreGetMapUrl ? mBaseUrl : getMapUrl() );
  setQueryItem( url, "SERVICE", "WCS" );
  // TODO
  //setQueryItem( url, "VERSION", mCapabilities.version );
  setQueryItem( url, "REQUEST", "GetMap" );
  setQueryItem( url, "BBOX", bbox );
  setQueryItem( url, crsKey, mImageCrs );
  setQueryItem( url, "WIDTH", QString::number( pixelWidth ) );
  setQueryItem( url, "HEIGHT", QString::number( pixelHeight ) );
  setQueryItem( url, "IDENTIFIER", mIdentifier );
  setQueryItem( url, "FORMAT", mFormat );

  //DPI parameter is accepted by QGIS mapserver (and ignored by the other WCS servers)
  if ( mDpi != -1 )
  {
    setQueryItem( url, "DPI", QString::number( mDpi ) );
  }

  //MH: jpeg does not support transparency and some servers complain if jpg and transparent=true
  if ( !mFormat.contains( "jpeg", Qt::CaseInsensitive ) &&
       !mFormat.contains( "jpg", Qt::CaseInsensitive ) )
  {
    setQueryItem( url, "TRANSPARENT", "TRUE" );  // some servers giving error for 'true' (lowercase)
  }

  QgsDebugMsg( QString( "getmap: %1" ).arg( url.toString() ) );

  // cache some details for if the user wants to do an identifyAsHtml() later

  //mGetFeatureInfoUrlBase = mIgnoreGetFeatureInfoUrl ? mBaseUrl : getFeatureInfoUrl();

  QNetworkRequest request( url );
  setAuthorization( request );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  mCacheReply = QgsNetworkAccessManager::instance()->get( request );
  connect( mCacheReply, SIGNAL( finished() ), this, SLOT( cacheReplyFinished() ) );
  connect( mCacheReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( cacheReplyProgress( qint64, qint64 ) ) );

  emit statusChanged( tr( "Getting map via WCS." ) );

  mWaiting = true;

  QTime t;
  t.start();

  QSettings s;
  bool bkLayerCaching = s.value( "/qgis/enable_render_caching", false ).toBool();

  while ( mCacheReply && ( !bkLayerCaching || t.elapsed() < WCS_THRESHOLD ) )
  {
    QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, WCS_THRESHOLD );
  }

  mWaiting = false;
  return mCachedImage;
}

//void QgsWcsProvider::readBlock( int bandNo, QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight, QgsCoordinateReferenceSystem theSrcCRS, QgsCoordinateReferenceSystem theDestCRS, void *block )
void QgsWcsProvider::readBlock( int bandNo, QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight, void *block )
{
  Q_UNUSED( bandNo );
  QgsDebugMsg( "Entered" );
  // TODO: optimize to avoid writing to QImage
  QImage* image = draw( viewExtent, pixelWidth, pixelHeight );

  if ( ! image )   // should not happen
  {
    QgsMessageLog::logMessage( tr( "image is NULL" ), tr( "WCS" ) );
    return;
  }
  QgsDebugMsg( QString( "image height = %1 bytesPerLine = %2" ).arg( image->height() ) . arg( image->bytesPerLine() ) ) ;
  int myExpectedSize = pixelWidth * pixelHeight * 4;
  int myImageSize = image->height() *  image->bytesPerLine();
  if ( myExpectedSize != myImageSize )   // should not happen
  {
    QgsMessageLog::logMessage( tr( "unexpected image size" ), tr( "WCS" ) );
    return;
  }

  uchar * ptr = image->bits() ;
  memcpy( block, ptr, myExpectedSize );
  // do not delete the image, it is handled by draw()
  //delete image;
}

void QgsWcsProvider::cacheReplyFinished()
{
  if ( mCacheReply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = mCacheReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      mCacheReply->deleteLater();

      QgsDebugMsg( QString( "redirected getmap: %1" ).arg( redirect.toString() ) );
      mCacheReply = QgsNetworkAccessManager::instance()->get( QNetworkRequest( redirect.toUrl() ) );
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
                                 .arg( mCacheReply->url().toString() ), tr( "WCS" ) );

      mCacheReply->deleteLater();
      mCacheReply = 0;

      return;
    }

    QString contentType = mCacheReply->header( QNetworkRequest::ContentTypeHeader ).toString();
    QgsDebugMsg( "contentType: " + contentType );
    if ( contentType.startsWith( "image/", Qt::CaseInsensitive ) )
    {
      QImage myLocalImage = QImage::fromData( mCacheReply->readAll() );
      if ( !myLocalImage.isNull() )
      {
        QPainter p( mCachedImage );
        p.drawImage( 0, 0, myLocalImage );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Returned image is flawed [%1]" ).arg( mCacheReply->url().toString() ), tr( "WCS" ) );
      }
    }
    else
    {
      QByteArray text = mCacheReply->readAll();
      if ( contentType.toLower() == "text/xml" && parseServiceExceptionReportDom( text ) )
      {
        QgsMessageLog::logMessage( tr( "Map request error (Title:%1; Error:%2; URL: %3)" )
                                   .arg( mErrorCaption ).arg( mError )
                                   .arg( mCacheReply->url().toString() ), tr( "WCS" ) );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Map request error (Status: %1; Response: %2; URL:%3)" )
                                   .arg( status.toInt() )
                                   .arg( QString::fromUtf8( text ) )
                                   .arg( mCacheReply->url().toString() ), tr( "WCS" ) );
      }

      mCacheReply->deleteLater();
      mCacheReply = 0;

      return;
    }

    mCacheReply->deleteLater();
    mCacheReply = 0;

    if ( !mWaiting )
    {
      QgsDebugMsg( "emit dataChanged()" );
      emit dataChanged();
    }
  }
  else
  {
    mErrors++;
    if ( mErrors < 100 )
    {
      QgsMessageLog::logMessage( tr( "Map request failed [error:%1 url:%2]" ).arg( mCacheReply->errorString() ).arg( mCacheReply->url().toString() ), tr( "WCS" ) );
    }
    else if ( mErrors == 100 )
    {
      QgsMessageLog::logMessage( tr( "Not logging more than 100 request errors." ), tr( "WCS" ) );
    }

    mCacheReply->deleteLater();
    mCacheReply = 0;
  }
}

int QgsWcsProvider::dataType( int bandNo ) const
{
  return srcDataType( bandNo );
}

int QgsWcsProvider::srcDataType( int bandNo ) const
{
  Q_UNUSED( bandNo );
  return QgsRasterDataProvider::ARGBDataType;
}

int QgsWcsProvider::bandCount() const
{
  return 1;
}

void QgsWcsProvider::cacheReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of map downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsg( msg );
  emit statusChanged( msg );
}

bool QgsWcsProvider::parseServiceExceptionReportDom( QByteArray const & xml )
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
    mError = tr( "Could not get WCS Service Exception at %1: %2 at line %3 column %4\n\nResponse was:\n\n%5" )
             .arg( mBaseUrl )
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
      //QgsDebugMsg(e.tagName() ); // the node really is an element.

      QString tagName = e.tagName();
      if ( tagName.startsWith( "wcs:" ) )
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

void QgsWcsProvider::parseServiceException( QDomElement const & e )
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
  else if ( seCode == "InvalidSRS" )  // legacy WCS < 1.3.0
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

  mError += "\n" + tr( "The WCS vendor also reported: " );
  mError += seText;

  // TODO = e.attribute("locator");

  QgsMessageLog::logMessage( tr( "composed error message '%1'." ).arg( mError ), tr( "WCS" ) );
  QgsDebugMsg( "exiting." );
}



QgsRectangle QgsWcsProvider::extent()
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

bool QgsWcsProvider::isValid()
{
  return mValid;
}


QString QgsWcsProvider::wcsVersion()
{
  // TODO
  return NULL;
}

bool QgsWcsProvider::calculateExtent()
{
  //! \todo Make this handle non-geographic CRSs (e.g. floor plans) as per WCS spec

  QgsDebugMsg( "entered." );

  // Make sure we know what extents are available
  //if ( !retrieveServerCapabilities() )
  //{
    //return false;
  //}

  // Set up the coordinate transform from the WCS standard CRS:84 bounding
  // box to the user's selected CRS
  if ( !mCoordinateTransform )
  {
    QgsCoordinateReferenceSystem qgisSrsSource;
    QgsCoordinateReferenceSystem qgisSrsDest;

    qgisSrsSource.createFromOgcWmsCrs( DEFAULT_LATLON_CRS );

    qgisSrsDest.createFromOgcWmsCrs( mImageCrs );

    mCoordinateTransform = new QgsCoordinateTransform( qgisSrsSource, qgisSrsDest );
  }

  //TODO
  /* 
  bool firstLayer = true; //flag to know if a layer is the first to be successfully transformed
  for ( QStringList::Iterator it  = mActiveSubLayers.begin();
        it != mActiveSubLayers.end();
        ++it )
  {
    QgsDebugMsg( "Sublayer Iterator: " + *it );
    // This is the extent for the layer name in *it
    QgsRectangle extent = mExtentForLayer.find( *it ).value();

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
  */
  QgsDebugMsg( "exiting with '"  + mLayerExtent.toString() + "'." );
  return true;

  QgsDebugMsg( "exiting without extent." );
  return false;

}


int QgsWcsProvider::capabilities() const
{
  int capability = NoCapabilities;
  capability |= QgsRasterDataProvider::Identify;

  return capability;
}
//TODO
//QString QgsWcsProvider::layerMetadata( QgsWcsLayerProperty &layer )
QString QgsWcsProvider::layerMetadata( )
{
  QString metadata;
/*
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
  metadata += mExtentForLayer[ layer.name ].toString();
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
*/
  return metadata;
}

QString QgsWcsProvider::metadata()
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

#if QT_VERSION >= 0x40500
    metadata += "<a href=\"#cachestats\">";
    metadata += tr( "Cache Stats" );
    metadata += "</a> ";
#endif

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

  // WCS Version
  metadata += "<tr><td>";
  metadata += tr( "WCS Version" );
  metadata += "</td>";
  metadata += "<td>";
  // TODO
  //metadata += mCapabilities.version;
  metadata += "</td></tr>";

  // Service Title
  metadata += "<tr><td>";
  metadata += tr( "Title" );
  metadata += "</td>";
  metadata += "<td>";
  // TODO metadata += mCapabilities.service.title;
  metadata += "</td></tr>";

  // Service Abstract
  metadata += "<tr><td>";
  metadata += tr( "Abstract" );
  metadata += "</td>";
  metadata += "<td>";
  // TODO metadata += mCapabilities.service.abstract;
  metadata += "</td></tr>";

  // Service Keywords
  metadata += "<tr><td>";
  metadata += tr( "Keywords" );
  metadata += "</td>";
  metadata += "<td>";
  //metadata += mCapabilities.service.keywordList.join( "<br />" );
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
  //metadata += mCapabilities.service.contactInformation.contactPersonPrimary.contactPerson;
  metadata += "<br />";
  //metadata += mCapabilities.service.contactInformation.contactPosition;
  metadata += "<br />";
  //metadata += mCapabilities.service.contactInformation.contactPersonPrimary.contactOrganization;
  metadata += "</td></tr>";

  // Service Fees
  metadata += "<tr><td>";
  metadata += tr( "Fees" );
  metadata += "</td>";
  metadata += "<td>";
  //metadata += mCapabilities.service.fees;
  metadata += "</td></tr>";

  // Service Access Constraints
  metadata += "<tr><td>";
  metadata += tr( "Access Constraints" );
  metadata += "</td>";
  metadata += "<td>";
  //metadata += mCapabilities.service.accessConstraints;
  metadata += "</td></tr>";

  // GetMap Request Formats
  metadata += "<tr><td>";
  metadata += tr( "Image Formats" );
  metadata += "</td>";
  metadata += "<td>";
  //metadata += mCapabilities.capability.request.getMap.format.join( "<br />" );
  metadata += "</td></tr>";

  // GetFeatureInfo Request Formats
  metadata += "<tr><td>";
  metadata += tr( "Identify Formats" );
  metadata += "</td>";
  metadata += "<td>";
  //metadata += mCapabilities.capability.request.getFeatureInfo.format.join( "<br />" );
  metadata += "</td></tr>";

  // Layer Count (as managed by this provider)
  metadata += "<tr><td>";
  metadata += tr( "Layer Count" );
  metadata += "</td>";
  metadata += "<td>";
  // TODO
  //metadata += QString::number( mLayersSupported.size() );
  metadata += "</td></tr>";

  // Base URL
  metadata += "<tr><td>";
  metadata += tr( "GetCapabilitiesUrl" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += mBaseUrl;
  metadata += "</td></tr>";

  metadata += "<tr><td>";
  metadata += tr( "GetMapUrl" );
  metadata += "</td>";
  metadata += "<td>";
  metadata += getMapUrl() + ( mIgnoreGetMapUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : "" );
  metadata += "</td></tr>";

  // Close the nested table
  metadata += "</table>";
  metadata += "</td></tr>";

  // Layer properties
  metadata += "<tr><th class=\"glossy\"><a name=\"selectedlayers\"></a>";
  metadata += tr( "Selected Layers" );
  metadata += "</th></tr>";

  // Layer properties
  metadata += "<tr><th class=\"glossy\"><a name=\"otherlayers\"></a>";
  metadata += tr( "Other Layers" );
  metadata += "</th></tr>";

  /* TODO
  for ( int i = 0; i < mLayersSupported.size(); i++ )
  {
    if ( mActiveSubLayers.indexOf( mLayersSupported[i].name ) < 0 )
    {
      metadata += layerMetadata( mLayersSupported[i] );
    }
  } // for each layer
  */
  metadata += "</table>";

  QgsDebugMsg( "exiting with '"  + metadata  + "'." );

  return metadata;
}

QString QgsWcsProvider::identifyAsText( const QgsPoint &point )
{
  Q_UNUSED( point );
  return QString( "Not implemented" );
}

QString QgsWcsProvider::identifyAsHtml( const QgsPoint &point )
{
  Q_UNUSED( point );
  return QString( "Not implemented" );
}

QgsCoordinateReferenceSystem QgsWcsProvider::crs()
{
  return mCrs;
}

QString QgsWcsProvider::lastErrorTitle()
{
  return mErrorCaption;
}

QString QgsWcsProvider::lastError()
{
  QgsDebugMsg( "returning '" + mError  + "'." );
  return mError;
}

QString QgsWcsProvider::lastErrorFormat()
{
  return mErrorFormat;
}

QString  QgsWcsProvider::name() const
{
  return WCS_KEY;
}

QString  QgsWcsProvider::description() const
{
  return WCS_DESCRIPTION;
}

void QgsWcsProvider::reloadData()
{
  delete mCachedImage;
  mCachedImage = 0;
}

void QgsWcsProvider::setAuthorization( QNetworkRequest &request ) const
{
  if ( !mUserName.isNull() || !mPassword.isNull() )
  {
    request.setRawHeader( "Authorization", "Basic " + QString( "%1:%2" ).arg( mUserName ).arg( mPassword ).toAscii().toBase64() );
  }
}

QString QgsWcsProvider::nodeAttribute( const QDomElement &e, QString name, QString defValue )
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

void QgsWcsProvider::showMessageBox( const QString& title, const QString& text )
{
  QgsMessageOutput *message = QgsMessageOutput::createMessageOutput();
  message->setTitle( title );
  message->setMessage( text, QgsMessageOutput::MessageText );
  message->showMessage();
}

QMap<QString, QString> QgsWcsProvider::supportedMimes()
{
  QMap<QString, QString> mimes;
  GDALAllRegister();

  QgsDebugMsg( QString( "GDAL drivers cont %1" ).arg( GDALGetDriverCount() ) );
  for ( int i = 0; i < GDALGetDriverCount(); ++i )
  {
    GDALDriverH driver = GDALGetDriver( i );
    Q_CHECK_PTR( driver );

    if ( !driver )
    {
      QgsLogger::warning( "unable to get driver " + QString::number( i ) );
      continue;
    }

    QString desc = GDALGetDescription( driver );

    QString mimeType = GDALGetMetadataItem( driver, "DMD_MIMETYPE", "" );

    if ( mimeType.isEmpty() ) continue;

    desc = desc.isEmpty() ? mimeType : desc;

    QgsDebugMsg( "add GDAL format " + mimeType + " " + desc );

    mimes[mimeType] = desc;
  }
  return mimes;
}

QGISEXTERN QgsWcsProvider * classFactory( const QString *uri )
{
  return new QgsWcsProvider( *uri );
}

QGISEXTERN QString providerKey()
{
  return WCS_KEY;
}

QGISEXTERN QString description()
{
  return WCS_DESCRIPTION;
}

QGISEXTERN bool isProvider()
{
  return true;
}
