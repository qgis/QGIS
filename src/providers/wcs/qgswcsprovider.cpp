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
#include <QRegExp>
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

// TODO: colortable - use comon baseclass with gdal, mapserver does not support http://trac.osgeo.org/mapserver/ticket/1671

QgsWcsProvider::QgsWcsProvider( QString const &uri )
    : QgsRasterDataProvider( uri )
    , mHttpUri( QString::null )
    , mCoverageCrs( DEFAULT_LATLON_CRS )
    , mCacheReply( 0 )
    , mCachedViewExtent( 0 )
    , mCoordinateTransform( 0 )
    , mExtentDirty( true )
    , mGetFeatureInfoUrlBase( "" )
    , mErrors( 0 )
    , mUserName( QString::null )
    , mPassword( QString::null )
    , mCoverageSummary( 0 )
    , mCachedGdalDataset( 0 )
    , mCachedMemFile( 0 )
    , mWidth( 0 )
    , mHeight( 0 )
    , mXBlockSize( 0 )
    , mYBlockSize( 0 )
    , mHasSize( false )
    , mBandCount( 0 )
{
  QgsDebugMsg( "constructing with uri '" + mHttpUri + "'." );

  mValid = false;

  parseUri( uri );

  // GetCapabilities and DescribeCoverage
  // TODO(?): do only DescribeCoverage to avoid one request
  // We need to get at least server version, which is not set in of URI (if not part of url)
  // and probably also rangeSet

  QgsDataSourceURI capabilitiesUri;
  capabilitiesUri.setEncodedUri( uri );
  // remove non relevant params
  capabilitiesUri.removeParam( "identifier" );
  capabilitiesUri.removeParam( "crs" );
  capabilitiesUri.removeParam( "format" );
  // TODO: check if successful (add return to capabilities)
  mCapabilities.setUri( capabilitiesUri );

  // 1.0 get additional coverage info
  if ( !mCapabilities.describeCoverage( mIdentifier ) )
  {
    QgsDebugMsg( "Cannot describe coverage" );
    return;
  }

  mCoverageSummary = mCapabilities.coverageSummary( mIdentifier );
  if ( !mCoverageSummary )
  {
    QgsDebugMsg( "coverage not found" );
    return;
  }
  mWidth = mCoverageSummary->width;
  mHeight = mCoverageSummary->height;
  mHasSize = mCoverageSummary->hasSize;

  QgsDebugMsg( QString( "mWidth = %1 mHeight = %2" ).arg( mWidth ).arg( mHeight ) ) ;

  if ( !calculateExtent() )
  {
    QgsDebugMsg( "Cannot calculate extent" );
    return;
  }

  mCachedMemFilename = QString( "/vsimem/qgis/wcs/%0.dat" ).arg(( qlonglong )this );

  // Get small piece of coverage to find GDAL data type and nubmer of bands
  int bandNo = 0; // All bands
  double xRes, yRes;
  int size = 5; // to be requested
  int width = 1000; // just some number to get smaller piece of coverage
  int height = 1000;
  if ( mHasSize )
  {
    width = mWidth;
    height = mHeight;
  }
  xRes = mCoverageExtent.width() / width;
  yRes = mCoverageExtent.height() / height;
  QgsRectangle extent = mCoverageExtent;
  extent.setXMaximum( extent.xMinimum() + size * xRes );
  extent.setYMaximum( extent.yMinimum() + size * yRes );
  getCache( bandNo, extent, size, size );

  if ( !mCachedGdalDataset )
  {
    QgsDebugMsg( "Cannot get test dataset." );
    return;
  }

  mBandCount = GDALGetRasterCount( mCachedGdalDataset );
  QgsDebugMsg( QString( "mBandCount = %1" ).arg( mBandCount ) ) ;

  // Get types
  // TODO: we are using the same data types like GDAL (not wider like GDAL provider)
  // with expectation to replace 'no data' values by NaN
  for ( int i = 1; i <= mBandCount; i++ )
  {
    GDALRasterBandH gdalBand = GDALGetRasterBand( mCachedGdalDataset, i );
    GDALDataType myGdalDataType = GDALGetRasterDataType( gdalBand );

    mSrcGdalDataType.append( myGdalDataType );
    // TODO: This could be shared with GDAL provider
    int isValid = false;

    // UMN Mapserver does not automaticaly set null value, METADATA wcs_rangeset_nullvalue must be used
    // http://lists.osgeo.org/pipermail/mapserver-users/2010-April/065328.html

    // TODO:

    double myNoDataValue = GDALGetRasterNoDataValue( gdalBand, &isValid );
    if ( isValid )
    {
      QgsDebugMsg( QString( "GDALGetRasterNoDataValue = %1" ).arg( myNoDataValue ) ) ;
      mGdalDataType.append( myGdalDataType );
    }
    else
    {
      // But we need a null value in case of reprojection and BTW also for
      // aligned margines
      switch ( dataTypeFormGdal( myGdalDataType ) )
      {

        case QgsRasterDataProvider::Byte:
          // Use longer data type to avoid conflict with real data
          myNoDataValue = -32768.0;
          mGdalDataType.append( GDT_Int16 );
          break;
        case QgsRasterDataProvider::Int16:
          myNoDataValue = -2147483648.0;
          mGdalDataType.append( GDT_Int32 );
          break;
        case QgsRasterDataProvider::UInt16:
          myNoDataValue = -2147483648.0;
          mGdalDataType.append( GDT_Int32 );
          break;
        case QgsRasterDataProvider::Int32:
          myNoDataValue = -2147483648.0;
          mGdalDataType.append( myGdalDataType );
          break;
        case QgsRasterDataProvider::UInt32:
          myNoDataValue = 4294967295.0;
          mGdalDataType.append( myGdalDataType );
          break;
        default:
          myNoDataValue = std::numeric_limits<int>::max();
          // Would NaN work well?
          //myNoDataValue = std::numeric_limits<double>::quiet_NaN();
          mGdalDataType.append( myGdalDataType );
      }
    }
    mNoDataValue.append( myNoDataValue );
    mValidNoDataValue = true;

    QgsDebugMsg( QString( "mSrcGdalDataType[%1] = %2" ).arg( i - 1 ).arg( mSrcGdalDataType[i-1] ) );
    QgsDebugMsg( QString( "mGdalDataType[%1] = %2" ).arg( i - 1 ).arg( mGdalDataType[i-1] ) );
    QgsDebugMsg( QString( "mNoDataValue[%1] = %2" ).arg( i - 1 ).arg( mNoDataValue[i-1] ) );
  }

  clearCache();

  // Block size is used for for statistics
  // TODO: How to find maximum block size supported by server?
  if ( mHasSize )
  {
    // This is taken from GDAL, how they come to these numbers?
    if ( mWidth > 1800 ) mXBlockSize = 1024;
    else mXBlockSize = mWidth;

    if ( mHeight > 900 ) mYBlockSize = 512;
    else mYBlockSize = mHeight;
  }

  mValid = true;
  QgsDebugMsg( "Constructed ok, provider valid." );
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
  clearCache();

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

  if ( crs != mCoverageCrs && !crs.isEmpty() )
  {
    // delete old coordinate transform as it is no longer valid
    if ( mCoordinateTransform )
    {
      delete mCoordinateTransform;
      mCoordinateTransform = 0;
    }

    mExtentDirty = true;

    mCoverageCrs = crs;
  }
}

void QgsWcsProvider::setQueryItem( QUrl &url, QString item, QString value )
{
  url.removeQueryItem( item );
  url.addQueryItem( item, value );
}

void QgsWcsProvider::readBlock( int bandNo, QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight, void *block )
{
  QgsDebugMsg( "Entered" );

  /*
  if ( myExpectedSize != myImageSize )   // should not happen
  {
    QgsMessageLog::logMessage( tr( "unexpected image size" ), tr( "WCS" ) );
    return;
  }
  */

  // abort running (untiled) request
  if ( mCacheReply )
  {
    mCacheReply->abort();
    delete mCacheReply;
    mCacheReply = 0;
  }

  // Can we reuse the previously cached image?
  if ( mCachedGdalDataset &&
       mCachedViewExtent == viewExtent &&
       mCachedViewWidth == pixelWidth &&
       mCachedViewHeight == pixelHeight )
  {
    //return mCachedImage;
  }

  getCache( bandNo, viewExtent, pixelWidth, pixelHeight );

  if ( mCachedGdalDataset )
  {
    // TODO band
    int width = GDALGetRasterXSize( mCachedGdalDataset );
    int height = GDALGetRasterYSize( mCachedGdalDataset );
    QgsDebugMsg( QString( "cached data width = %1 height = %2" ).arg( width ).arg( height ) );
    // TODO: check type? , check band count?
    if ( width == pixelWidth && height == pixelHeight )
    {
      GDALRasterBandH gdalBand = GDALGetRasterBand( mCachedGdalDataset, bandNo );
      GDALRasterIO( gdalBand, GF_Read, 0, 0, pixelWidth, pixelHeight, block, pixelWidth, pixelHeight, ( GDALDataType ) mGdalDataType[bandNo-1], 0, 0 );
      QgsDebugMsg( tr( "Block read OK" ) );
    }
    else
    {
      QgsDebugMsg( tr( "Recieved coverage has wrong size" ) );
      return;
    }
  }
}

void QgsWcsProvider::getCache( int bandNo, QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight )
{
  // delete cached data
  clearCache();

  // --------------- BOUNDING BOX --------------------------------
  //according to the WCS spec for 1.1, some CRS have inverted axis
  // box:
  //  1.0.0: minx,miny,maxx,maxy
  //  1.1.0, 1.1.2: OGC 07-067r5 (WCS 1.1.2) referes to OGC 06-121r3 which says
  //  "The number of axes included, and the order of these axes, shall be as specified
  //  by the referenced CRS." That means inverted for geographic.
  bool changeXY = false;
  if ( !mIgnoreAxisOrientation && ( mCapabilities.version().startsWith( "1.1" ) ) )
  {
    //create CRS from string
    QgsCoordinateReferenceSystem theSrs;
    if ( theSrs.createFromOgcWmsCrs( mCoverageCrs ) && theSrs.axisInverted() )
    {
      changeXY = true;
    }
  }

  if ( mInvertAxisOrientation ) changeXY = !changeXY;

  double xRes = viewExtent.width() / pixelWidth;
  double yRes = viewExtent.height() / pixelHeight;
  QgsRectangle extent = viewExtent;
  if ( mCapabilities.version().startsWith( "1.1" ) )
  {
    // WCS 1.1 grid is using cell centers -> shrink the extent to border cells centers by half cell size
    extent = QgsRectangle( viewExtent.xMinimum() + xRes / 2., viewExtent.yMinimum() + yRes / 2., viewExtent.xMaximum() - xRes / 2., viewExtent.yMaximum() - yRes / 2. );
  }

  // Bounding box in WCS format (Warning: does not work with scientific notation)
  QString bbox = QString( changeXY ? "%2,%1,%4,%3" : "%1,%2,%3,%4" )
                 .arg( extent.xMinimum(), 0, 'f', 16 )
                 .arg( extent.yMinimum(), 0, 'f', 16 )
                 .arg( extent.xMaximum(), 0, 'f', 16 )
                 .arg( extent.yMaximum(), 0, 'f', 16 );

  QUrl url( mIgnoreGetMapUrl ? mBaseUrl : mCapabilities.getCoverageUrl() );

  // Version 1.0.0, 1.1.0, 1.1.2
  setQueryItem( url, "SERVICE", "WCS" );
  setQueryItem( url, "VERSION", mCapabilities.version() );
  setQueryItem( url, "REQUEST", "GetCoverage" );
  setQueryItem( url, "FORMAT", mFormat );

  // Version 1.0.0
  if ( mCapabilities.version().startsWith( "1.0" ) )
  {
    setQueryItem( url, "COVERAGE", mIdentifier );
    setQueryItem( url, "BBOX", bbox );
    setQueryItem( url, "CRS", mCoverageCrs ); // request BBOX CRS
    setQueryItem( url, "RESPONSE_CRS", mCoverageCrs ); // response CRS
    setQueryItem( url, "WIDTH", QString::number( pixelWidth ) );
    setQueryItem( url, "HEIGHT", QString::number( pixelHeight ) );
  }

  // Version 1.1.0, 1.1.2
  if ( mCapabilities.version().startsWith( "1.1" ) )
  {
    setQueryItem( url, "IDENTIFIER", mIdentifier );
    QString crsUrn = QString( "urn:ogc:def:crs:%1::%2" ).arg( mCoverageCrs.split( ':' ).value( 0 ) ).arg( mCoverageCrs.split( ':' ).value( 1 ) );
    bbox += "," + crsUrn;
    setQueryItem( url, "BOUNDINGBOX", bbox );

    //   GridBaseCRS=urn:ogc:def:crs:SG:6.6:32618
    //   GridType=urn:ogc:def:method:CS:1.1:2dGridIn2dCrs
    //   GridCS=urn:ogc:def:cs:OGC:0Grid2dSquareCS
    //   GridOrigin=0,0
    //   GridOffsets=0.0707,-0.0707,0.1414,0.1414&

    setQueryItem( url, "GRIDBASECRS", crsUrn ); // response CRS
    // GridOrigin is BBOX minx, maxy
    // TODO: invert axis, pixels centers
    QString gridOrigin = QString( changeXY ? "%2,%1" : "%1,%2" )
                         .arg( extent.xMinimum(), 0, 'f', 16 )
                         .arg( extent.yMaximum(), 0, 'f', 16 );
    setQueryItem( url, "GRIDORIGIN", gridOrigin );

    QString gridOffsets = QString( changeXY ? "%2,%1" : "%1,%2" )
                          .arg( xRes, 0, 'f', 16 )
                          .arg( yRes, 0, 'f', 16 );
    setQueryItem( url, "GRIDOFFSETS", gridOffsets );
  }


  QgsDebugMsg( QString( "GetCoverage: %1" ).arg( url.toString() ) );

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

}

// For stats only, maybe change QgsRasterDataProvider::bandStatistics() to
// use standard readBlock with extent
void QgsWcsProvider::readBlock( int theBandNo, int xBlock, int yBlock, void *block )
{
  QgsDebugMsg( "Entered" );

  QgsDebugMsg( QString( "xBlock = %1 yBlock = %2" ).arg( xBlock ).arg( yBlock ) );

  if ( !mHasSize ) return;

  double xRes = mCoverageExtent.width() / mWidth;
  double yRes = mCoverageExtent.height() / mHeight;

  // blocks on edges may run out of extent, that should not be problem (at least for
  // stats - there is a check for it)
  double xMin = mCoverageExtent.xMinimum() + xRes * xBlock * mXBlockSize;
  double xMax = xMin + xRes * mXBlockSize;
  double yMax = mCoverageExtent.yMaximum() - yRes * yBlock * mYBlockSize;
  double yMin = yMax - yRes * mYBlockSize;
  //QgsDebugMsg( QString("yMin = %1 yMax = %2").arg(yMin).arg(yMax) );

  QgsRectangle extent( xMin, yMin, xMax, yMax );

  readBlock( theBandNo, extent, mXBlockSize, mYBlockSize, block );
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

    // Read response
    clearCache(); // should not be necessary

    QString contentType = mCacheReply->header( QNetworkRequest::ContentTypeHeader ).toString();
    QgsDebugMsg( "contentType: " + contentType );

    // Exception
    if ( contentType.startsWith( "text/", Qt::CaseInsensitive ) ||
         contentType.toLower() == "application/vnd.ogc.se_xml" )
    {
      // TODO: test also if it is really exception (from content)
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

    // WCS 1.1 gives response as multipart, e.g.
    //   multipart/mixed; boundary=wcs
    //   multipart/mixed; boundary="wcs"\n
    if ( contentType.startsWith( "multipart/", Qt::CaseInsensitive ) )
    {
      // It seams that Qt does not have currently support for multipart reply
      // and it is not even possible to create QNetworkReply from raw data
      // so we have to parse response
      QRegExp re( ".*boundary=\"?([^\"]+)\"?\\s?", Qt::CaseInsensitive );

      if ( !re.indexIn( contentType ) == 0 )
      {
        QgsMessageLog::logMessage( tr( "Cannot find boundary in multipart content type" ), tr( "WCS" ) );
        clearCache();
        mCacheReply->deleteLater();
        mCacheReply = 0;
        return;
      }

      QString boundary = re.cap( 1 );
      QgsDebugMsg( "boundary = " + boundary );
      boundary = "--" + boundary;

      // Lines should be terminated by CRLF ("\r\n") but any new line combination may appear
      QByteArray data = mCacheReply->readAll();
      int from, to;
      from = data.indexOf( boundary.toAscii(), 0 ) + boundary.length() + 1 ;
      QVector<QByteArray> partHeaders;
      QVector<QByteArray> partBodies;
      while ( true )
      {
        to = data.indexOf( boundary.toAscii(), from );
        if ( to < 0 ) break;
        QgsDebugMsg( QString( "part %1 - %2" ).arg( from ).arg( to ) );
        QByteArray part = data.mid( from, to - from );
        // Remove possible new line from beginning
        while ( part.size() > 0 && ( part.at( 0 ) == '\r' || part.at( 0 ) == '\n' ) )
        {
          part.remove( 0, 1 );
        }
        // Split header and data (find empty new line)
        // New lines should be CRLF, but we support also CRLFCRLF, LFLF to find empty line
        int pos = 0; // body start
        while ( pos < part.size() - 1 )
        {
          if ( part.at( pos ) == '\n' && ( part.at( pos + 1 ) == '\n' || part.at( pos + 1 ) == '\r' ) )
          {
            if ( part.at( pos + 1 ) == '\r' ) pos++;
            pos += 2;
            break;
          }
          pos++;
        }
        partHeaders.append( part.left( pos ) );
        partBodies.append( part.mid( pos ) );
        QgsDebugMsg( "Part header:\n" + partHeaders.last() );

        from = to + boundary.length() + 1;
      }
      if ( partBodies.size() < 2 )
      {
        QgsMessageLog::logMessage( tr( "Expected 2 parts, %1 recieved" ).arg( partBodies.size() ), tr( "WCS" ) );
        clearCache();
        mCacheReply->deleteLater();
        mCacheReply = 0;
        return;
      }
      else if ( partBodies.size() > 2 )
      {
        // We will try the second one
        QgsMessageLog::logMessage( tr( "More than 2 parts (%1) recieved" ).arg( partBodies.size() ), tr( "WCS" ) );
      }
      mCachedData = partBodies.value( 1 );
    }
    else
    {
      // Mime types for WCS 1.0 response should be usually image/<format>
      // (image/tiff, image/png, image/jpeg, image/png; mode=8bit, etc.)
      // but other mime types (like application/*) may probably also appear

      // Create memory file
      mCachedData = mCacheReply->readAll();
    }

    if ( mCachedData.size() == 0 )
    {
      QgsMessageLog::logMessage( tr( "No data recieved" ), tr( "WCS" ) );
      clearCache();
      mCacheReply->deleteLater();
      mCacheReply = 0;
      return;
    }
    QgsDebugMsg( QString( "%1 bytes recieved" ).arg( mCachedData.size() ) );

    mCachedMemFile = VSIFileFromMemBuffer( TO8F( mCachedMemFilename ),
                                           ( GByte* )mCachedData.data(),
                                           ( vsi_l_offset )mCachedData.size(),
                                           FALSE );

    if ( !mCachedMemFile )
    {
      QgsMessageLog::logMessage( tr( "Cannot create memory file" ), tr( "WCS" ) );
      clearCache();
      mCacheReply->deleteLater();
      mCacheReply = 0;
      return;
    }
    QgsDebugMsg( "Memory file created" );

    CPLErrorReset();
    mCachedGdalDataset = GDALOpen( TO8F( mCachedMemFilename ), GA_ReadOnly );
    if ( !mCachedGdalDataset )
    {
      QgsMessageLog::logMessage( QString::fromUtf8( CPLGetLastErrorMsg() ), tr( "WCS" ) );
      clearCache();
      mCacheReply->deleteLater();
      mCacheReply = 0;
      return;
    }
    QgsDebugMsg( "Dataset opened" );

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

// This could be shared with GDAL provider
int QgsWcsProvider::srcDataType( int bandNo ) const
{
  if ( bandNo < 0 || bandNo > mSrcGdalDataType.size() )
  {
    return QgsRasterDataProvider::UnknownDataType;
  }

  return dataTypeFormGdal( mSrcGdalDataType[bandNo-1] );
}

int QgsWcsProvider::dataType( int bandNo ) const
{
  if ( bandNo < 0 || bandNo > mGdalDataType.size() )
  {
    return QgsRasterDataProvider::UnknownDataType;
  }

  return dataTypeFormGdal( mGdalDataType[bandNo-1] );
}

int QgsWcsProvider::dataTypeFormGdal( int theGdalDataType ) const
{
  switch ( theGdalDataType )
  {
    case GDT_Unknown:
      return QgsRasterDataProvider::UnknownDataType;
      break;
    case GDT_Byte:
      return QgsRasterDataProvider::Byte;
      break;
    case GDT_UInt16:
      return QgsRasterDataProvider::UInt16;
      break;
    case GDT_Int16:
      return QgsRasterDataProvider::Int16;
      break;
    case GDT_UInt32:
      return QgsRasterDataProvider::UInt32;
      break;
    case GDT_Int32:
      return QgsRasterDataProvider::Int32;
      break;
    case GDT_Float32:
      return QgsRasterDataProvider::Float32;
      break;
    case GDT_Float64:
      return QgsRasterDataProvider::Float64;
      break;
    case GDT_CInt16:
      return QgsRasterDataProvider::CInt16;
      break;
    case GDT_CInt32:
      return QgsRasterDataProvider::CInt32;
      break;
    case GDT_CFloat32:
      return QgsRasterDataProvider::CFloat32;
      break;
    case GDT_CFloat64:
      return QgsRasterDataProvider::CFloat64;
      break;
    case GDT_TypeCount:
      // make gcc happy
      break;
  }
  return QgsRasterDataProvider::UnknownDataType;
}

int QgsWcsProvider::bandCount() const
{
  return mBandCount;
}

double  QgsWcsProvider::noDataValue() const
{
  if ( mNoDataValue.size() > 0 )
  {
    return mNoDataValue[0];
  }
  return std::numeric_limits<int>::max(); // should not happen or be used
}

// this is only called once when statistics are calculated
// TODO
int QgsWcsProvider::xBlockSize() const
{
  return mXBlockSize;
}
int QgsWcsProvider::yBlockSize() const
{
  return mYBlockSize;
}

int QgsWcsProvider::xSize() const { return mWidth; }
int QgsWcsProvider::ySize() const { return mHeight; }

void QgsWcsProvider::clearCache()
{
  QgsDebugMsg( "Entered" );
  if ( mCachedGdalDataset )
  {
    QgsDebugMsg( "Close mCachedGdalDataset" );
    GDALClose( mCachedGdalDataset );
    mCachedGdalDataset = 0;
    QgsDebugMsg( "Closed" );
  }
  if ( mCachedMemFile )
  {
    QgsDebugMsg( "Close mCachedMemFile" );
    VSIFCloseL( mCachedMemFile );
    mCachedMemFile = 0;
    QgsDebugMsg( "Closed" );
  }
  QgsDebugMsg( "Clear mCachedData" );
  mCachedData.clear();
  QgsDebugMsg( "Cleared" );
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

  return mCoverageExtent;
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
  QgsDebugMsg( "entered." );

  // Make sure we know what extents are available
  if ( !mCoverageSummary )
  {
    return false;
  }

  // Set up the coordinate transform from the WCS standard CRS:84 bounding
  // box to the user's selected CRS
  if ( !mCoordinateTransform )
  {
    QgsCoordinateReferenceSystem qgisSrsSource;
    QgsCoordinateReferenceSystem qgisSrsDest;

    qgisSrsSource.createFromOgcWmsCrs( DEFAULT_LATLON_CRS );

    qgisSrsDest.createFromOgcWmsCrs( mCoverageCrs );

    mCoordinateTransform = new QgsCoordinateTransform( qgisSrsSource, qgisSrsDest );
  }

  // Convert to the user's CRS as required
  try
  {
    mCoverageExtent = mCoordinateTransform->transformBoundingBox( mCoverageSummary->wgs84BoundingBox, QgsCoordinateTransform::ForwardTransform );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    return false;
  }

  //make sure extent does not contain 'inf' or 'nan'
  if ( !mCoverageExtent.isFinite() )
  {
    return false;
  }

  QgsDebugMsg( "mCoverageExtent = " + mCoverageExtent.toString() );
  return true;
}


int QgsWcsProvider::capabilities() const
{
  int capability = NoCapabilities;
  capability |= QgsRasterDataProvider::Identify;

  if ( mHasSize )
  {
    capability |= QgsRasterDataProvider::ExactResolution;
    capability |= QgsRasterDataProvider::Size;
  }

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
  //metadata += getMapUrl() + ( mIgnoreGetMapUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : "" );
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
  clearCache();
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

// Not supported by WCS
// TODO: remove from QgsRasterDataProvider?
QImage* QgsWcsProvider::draw( QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight )
{
  Q_UNUSED( viewExtent );
  QgsDebugMsg( "pixelWidth = "  + QString::number( pixelWidth ) );
  QgsDebugMsg( "pixelHeight = "  + QString::number( pixelHeight ) );
  QgsDebugMsg( "viewExtent: " + viewExtent.toString() );

  QImage *image = new QImage( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
  image->fill( QColor( Qt::gray ).rgb() );

  return image;
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
