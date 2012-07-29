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
#include <QRegExp>
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
    , QgsGdalProviderBase()
    , mHttpUri( QString::null )
    , mCoverageSummary()
    , mWidth( 0 )
    , mHeight( 0 )
    , mXBlockSize( 0 )
    , mYBlockSize( 0 )
    , mHasSize( false )
    , mBandCount( 0 )
    , mCoverageCrs()
    , mCacheReply( 0 )
    , mCachedMemFile( 0 )
    , mCachedGdalDataset( 0 )
    , mCachedViewExtent( 0 )
    , mCoordinateTransform( 0 )
    , mExtentDirty( true )
    , mGetFeatureInfoUrlBase( "" )
    , mErrors( 0 )
    , mUserName( QString::null )
    , mPassword( QString::null )
{
  QgsDebugMsg( "constructing with uri '" + mHttpUri + "'." );

  mValid = false;

  if ( !parseUri( uri ) ) return;

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
    QgsMessageLog::logMessage( tr( "Cannot describe coverage" ), tr( "WCS" ) );
    return;
  }

  mCoverageSummary = mCapabilities.coverage( mIdentifier );
  if ( !mCoverageSummary.valid )
  {
    // Should not happen if describeCoverage() did not fail
    QgsMessageLog::logMessage( tr( "Coverage not found" ), tr( "WCS" ) );
    return;
  }

  // It may happen that format is empty (e.g. uri created in python script),
  // in that casei select one from available formats
  if ( mFormat.isEmpty() )
  {
    // TIFF is known by GDAL
    mFormat = mCoverageSummary.supportedFormat.filter( "tif", Qt::CaseInsensitive ).value( 0 );
  }
  if ( mFormat.isEmpty() )
  {
    // Take the first if TIFF was not found
    mFormat = mCoverageSummary.supportedFormat.value( 0 );
  }

  // We cannot continue without format, it is required
  if ( mFormat.isEmpty() ) return;

  // It could happen (usually not with current QgsWCSSourceSelect if at least
  // one CRS is available) that crs is not set in uri, in that case we
  // use the native, if available or WGS84 or the first supported
  if ( mCoverageCrs.isEmpty() )
  {
    QgsDebugMsg( "nativeCrs = " + mCoverageSummary.nativeCrs );
    QgsDebugMsg( "supportedCrs = " + mCoverageSummary.supportedCrs.join( "," ) );
    if ( !mCoverageSummary.nativeCrs.isEmpty() )
    {
      setCoverageCrs( mCoverageSummary.nativeCrs );
    }
    else if ( mCoverageSummary.supportedCrs.contains( "EPSG:4326", Qt::CaseInsensitive ) )
    {
      setCoverageCrs( "EPSG:4326" );
    }
    else if ( mCoverageSummary.supportedCrs.size() > 0 )
    {
      setCoverageCrs( mCoverageSummary.supportedCrs.value( 0 ) );
    }
  }
  QgsDebugMsg( "mCoverageCrs = " + mCoverageCrs );
  // We cannot continue without CRS
  if ( mCoverageCrs.isEmpty() ) return;

  mWidth = mCoverageSummary.width;
  mHeight = mCoverageSummary.height;
  mHasSize = mCoverageSummary.hasSize;

  QgsDebugMsg( QString( "mWidth = %1 mHeight = %2" ).arg( mWidth ).arg( mHeight ) ) ;

  if ( !calculateExtent() )
  {
    QgsMessageLog::logMessage( tr( "Cannot calculate extent" ), tr( "WCS" ) );
    return;
  }

  mCachedMemFilename = QString( "/vsimem/qgis/wcs/%0.dat" ).arg(( qlonglong )this );

  // Get small piece of coverage to find GDAL data type and nubmer of bands
  // Using non native CRS (if we don't know which is native) it could easily happen,
  // that a small part of bbox in request CRS near margin falls outside
  // coverage native bbox and server reports error => take a piece from center

  int bandNo = 0; // All bands
  int width;
  int height;
  QString crs;
  QgsRectangle box; // box to use to calc extent
  // Prefer native CRS
  if ( !mCoverageSummary.nativeCrs.isEmpty() &&
       !mCoverageSummary.nativeBoundingBox.isEmpty() &&
       mCoverageSummary.supportedCrs.contains( mCoverageSummary.nativeCrs ) &&
       mHasSize )
  {
    box = mCoverageSummary.nativeBoundingBox;
    width = mWidth;
    height = mHeight;
    crs = mCoverageSummary.nativeCrs;
  }
  else
  {
    box = mCoverageExtent;
    if ( mHasSize )
    {
      width = mWidth;
      height = mHeight;
    }
    else
    {
      // just a number to get smaller piece of coverage
      width = 1000;
      height = 1000;
    }
  }
  double xRes = box.width() / width;
  double yRes = box.height() / height;
  QgsPoint p = box.center();

  int half = 2; // to be requested

  // extent to be used for test request
  QgsRectangle extent = QgsRectangle( p.x() - half * xRes, p.y() - half * yRes, p.x() + half * xRes, p.y() + half * yRes );

  getCache( bandNo, extent, 2*half, 2*half, crs );

  if ( !mCachedGdalDataset )
  {
    QgsMessageLog::logMessage( tr( "Cannot get test dataset." ), tr( "WCS" ) );
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

    QgsDebugMsg( QString( "myGdalDataType[%1] = %2" ).arg( i - 1 ).arg( myGdalDataType ) );
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
      switch ( dataTypeFromGdal( myGdalDataType ) )
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

    // TODO: what to do if null values from DescribeCoverage differ?
    if ( !mCoverageSummary.nullValues.contains( myNoDataValue ) )
    {
      QgsDebugMsg( QString( "noDataValue %1 is missing in nullValues from CoverageDescription" ).arg( myNoDataValue ) );
    }

    mValidNoDataValue = true;

    QgsDebugMsg( QString( "mSrcGdalDataType[%1] = %2" ).arg( i - 1 ).arg( mSrcGdalDataType[i-1] ) );
    QgsDebugMsg( QString( "mGdalDataType[%1] = %2" ).arg( i - 1 ).arg( mGdalDataType[i-1] ) );
    QgsDebugMsg( QString( "mNoDataValue[%1] = %2" ).arg( i - 1 ).arg( mNoDataValue[i-1] ) );

    // Create and store color table
    // TODO: never tested because mapserver (6.0.3) does not support color tables
    mColorTables.append( QgsGdalProviderBase::colorTable( mCachedGdalDataset, i ) );
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

bool QgsWcsProvider::parseUri( QString uriString )
{

  QgsDebugMsg( "uriString = " + uriString );
  QgsDataSourceURI uri;
  uri.setEncodedUri( uriString );

  mMaxWidth = 0;
  mMaxHeight = 0;

  mHttpUri = uri.param( "url" );
  mBaseUrl = prepareUri( mHttpUri );
  QgsDebugMsg( "mBaseUrl = " + mBaseUrl );

  mIgnoreGetCoverageUrl = uri.hasParam( "IgnoreGetMapUrl" );
  mIgnoreAxisOrientation = uri.hasParam( "IgnoreAxisOrientation" ); // must be before parsing!
  mInvertAxisOrientation = uri.hasParam( "InvertAxisOrientation" ); // must be before parsing!

  mUserName = uri.param( "username" );
  QgsDebugMsg( "set username to " + mUserName );

  mPassword = uri.param( "password" );
  QgsDebugMsg( "set password to " + mPassword );

  mIdentifier = uri.param( "identifier" );

  setFormat( uri.param( "format" ) );

  if ( !uri.param( "crs" ).isEmpty() )
  {
    setCoverageCrs( uri.param( "crs" ) );
  }

  return true;
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

    mCrs.createFromOgcWmsCrs( mCoverageCrs );
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

  // TODO: set block to null values, move that to function and call only if fails
  memset( block, 0, pixelWidth * pixelHeight * typeSize( dataType( bandNo ) ) / 8 );

  // Requested extent must at least partialy overlap coverage extent, otherwise
  // server gives error. QGIS usually does not request blocks outside raster extent
  // (higher level checks) but it is better to do check here as well
  if ( !viewExtent.intersects( mCoverageExtent ) )
  {
    return;
  }

  // abort running (untiled) request
  if ( mCacheReply )
  {
    mCacheReply->abort();
    delete mCacheReply;
    mCacheReply = 0;
  }

  // Can we reuse the previously cached coverage?
  if ( !mCachedGdalDataset ||
       mCachedViewExtent != viewExtent ||
       mCachedViewWidth != pixelWidth ||
       mCachedViewHeight != pixelHeight )
  {
    getCache( bandNo, viewExtent, pixelWidth, pixelHeight );
  }

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
      QgsMessageLog::logMessage( tr( "Received coverage has wrong size" ), tr( "WCS" ) );
      return;
    }
  }
}

void QgsWcsProvider::getCache( int bandNo, QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight, QString crs )
{
  QgsDebugMsg( "Entered" );
  // delete cached data
  clearCache();

  if ( crs.isEmpty() )
  {
    crs = mCoverageCrs;
  }

  mCachedViewExtent = viewExtent;
  mCachedViewWidth = pixelWidth;
  mCachedViewHeight = pixelHeight;

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
    if ( theSrs.createFromOgcWmsCrs( crs ) && theSrs.axisInverted() )
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

  QUrl url( mIgnoreGetCoverageUrl ? mBaseUrl : mCapabilities.getCoverageUrl() );

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
    setQueryItem( url, "CRS", crs ); // request BBOX CRS
    setQueryItem( url, "RESPONSE_CRS", crs ); // response CRS
    setQueryItem( url, "WIDTH", QString::number( pixelWidth ) );
    setQueryItem( url, "HEIGHT", QString::number( pixelHeight ) );
  }

  // Version 1.1.0, 1.1.2
  if ( mCapabilities.version().startsWith( "1.1" ) )
  {
    setQueryItem( url, "IDENTIFIER", mIdentifier );
    QString crsUrn = QString( "urn:ogc:def:crs:%1::%2" ).arg( crs.split( ':' ).value( 0 ) ).arg( crs.split( ':' ).value( 1 ) );
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
      QByteArray text = mCacheReply->readAll();
      if (( contentType.toLower() == "text/xml" || contentType.toLower() == "application/vnd.ogc.se_xml" )
          && parseServiceExceptionReportDom( text ) )
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
QgsRasterInterface::DataType QgsWcsProvider::srcDataType( int bandNo ) const
{
  if ( bandNo < 0 || bandNo > mSrcGdalDataType.size() )
  {
    return QgsRasterDataProvider::UnknownDataType;
  }

  return dataTypeFromGdal( mSrcGdalDataType[bandNo-1] );
}

QgsRasterInterface::DataType QgsWcsProvider::dataType( int bandNo ) const
{
  if ( bandNo < 0 || bandNo > mGdalDataType.size() )
  {
    return QgsRasterDataProvider::UnknownDataType;
  }

  return dataTypeFromGdal( mGdalDataType[bandNo-1] );
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

QList<QgsColorRampShader::ColorRampItem> QgsWcsProvider::colorTable( int theBandNumber )const
{
  QgsDebugMsg( "entered." );
  return mColorTables.value( theBandNumber - 1 );
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

  // TODO: Assert the docElem.tagName() is
  //  ServiceExceptionReport // 1.0
  //  ows:ExceptionReport  // 1.1

  //QString version = docElem.attribute("version");

  QDomElement e;
  if ( mCapabilities.version().startsWith( "1.0" ) )
  {
    e = QgsWcsCapabilities::domElement( docElem, "ServiceException" );
  }
  else // 1.1
  {
    e = QgsWcsCapabilities::domElement( docElem, "Exception" );
  }
  parseServiceException( e );

  QgsDebugMsg( "exiting." );

  return true;
}

void QgsWcsProvider::parseServiceException( QDomElement const & e )
{
  QgsDebugMsg( "entering." );


  QMap<QString, QString> exceptions;

  // Some codes are in both 1.0 and 1.1, in that case the 'meaning of code' is
  // taken from 1.0 specification

  // set up friendly descriptions for the service exception
  // 1.0
  exceptions["InvalidFormat"] = tr( "Request contains a format not offered by the server." );
  exceptions["CoverageNotDefined"] = tr( "Request is for a Coverage not offered by the service instance." );
  exceptions["CurrentUpdateSequence"] = tr( "Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number." );
  exceptions["InvalidUpdateSequence"] = tr( "Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number." );
  // 1.0, 1.1
  exceptions["MissingParameterValue"] = tr( "Request does not include a parameter value, and the servervice instance did not declare a default value for that dimension." );
  exceptions["InvalidParameterValue"] = tr( "Request contains an invalid parameter value." );
  // 1.1
  exceptions["NoApplicableCode"] = tr( "No other exceptionCode specified by this service and server applies to this exception." );
  exceptions["UnsupportedCombination"] = tr( "Operation request contains an output CRS that can not be used within the output format." );
  exceptions["NotEnoughStorage"] = tr( "Operation request specifies \"store\" the result, but not enough storage is available to do this." );

  QString seCode;
  QString seText;
  if ( mCapabilities.version().startsWith( "1.0" ) )
  {
    seCode = e.attribute( "code" );
    seText = e.text();
  }
  else
  {
    QStringList codes;
    seCode = e.attribute( "exceptionCode" );
    // UMN Mapserver (6.0.3) has messed/switched 'locator' and 'exceptionCode'
    if ( ! exceptions.contains( seCode ) )
    {
      seCode = e.attribute( "locator" );
      if ( ! exceptions.contains( seCode ) )
      {
        seCode = "";
      }
    }
    seText = QgsWcsCapabilities::firstChildText( e, "ExceptionText" );
  }

  mErrorFormat = "text/plain";

  if ( seCode.isEmpty() )
  {
    mError = tr( "(No error code was reported)" );
  }
  else if ( exceptions.contains( seCode ) )
  {
    mError = exceptions.value( seCode );
  }
  else
  {
    mError = seCode + " " + tr( "(Unknown error code)" );
  }

  mError += "\n" + tr( "The WCS vendor also reported: " );
  mError += seText;

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
  return mCapabilities.version();
}

bool QgsWcsProvider::calculateExtent()
{
  QgsDebugMsg( "entered." );

  // Make sure we know what extents are available
  if ( !mCoverageSummary.described )
  {
    return false;
  }

  // Prefer to use extent from capabilities / coverage description because
  // transformation from WGS84 increases the extent
  mCoverageExtent = mCoverageSummary.boundingBoxes.value( mCoverageCrs );
  if ( !mCoverageExtent.isEmpty() && !mCoverageExtent.isFinite() )
  {
    QgsDebugMsg( "mCoverageExtent = " + mCoverageExtent.toString() );
    return true;
  }

  // Set up the coordinate transform from the WCS standard CRS:84 bounding
  // box to the user's selected CRS
  if ( !mCoordinateTransform )
  {
    QgsCoordinateReferenceSystem qgisSrsSource;
    QgsCoordinateReferenceSystem qgisSrsDest;

    //qgisSrsSource.createFromOgcWmsCrs( DEFAULT_LATLON_CRS );
    qgisSrsSource.createFromOgcWmsCrs( "EPSG:4326" );
    //QgsDebugMsg( "qgisSrsSource: " + qgisSrsSource.toWkt() );
    qgisSrsDest.createFromOgcWmsCrs( mCoverageCrs );
    //QgsDebugMsg( "qgisSrsDest: " + qgisSrsDest.toWkt() );

    mCoordinateTransform = new QgsCoordinateTransform( qgisSrsSource, qgisSrsDest );
  }

  QgsDebugMsg( "mCoverageSummary.wgs84BoundingBox= " + mCoverageSummary.wgs84BoundingBox.toString() );
  // Convert to the user's CRS as required
  try
  {
    mCoverageExtent = mCoordinateTransform->transformBoundingBox( mCoverageSummary.wgs84BoundingBox, QgsCoordinateTransform::ForwardTransform );
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

QString QgsWcsProvider::coverageMetadata( QgsWcsCoverageSummary coverage )
{
  QString metadata;

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

  metadata += htmlRow( tr( "Name (identifier)" ), coverage.identifier );
  metadata += htmlRow( tr( "Title" ), coverage.title );
  metadata += htmlRow( tr( "Abstract" ), coverage.abstract );
  // We dont have size, nativeCrs, nativeBoundingBox etc. until describe coverage which would be heavy for all coverages
  //metadata += htmlRow ( tr( "Fixed Width" ), QString::number( coverage.width ) );
  //metadata += htmlRow ( tr( "Fixed Height" ), QString::number( coverage.height ) );
  //metadata += htmlRow ( tr( "Native CRS" ), coverage.nativeCrs );
  //metadata += htmlRow ( tr( "Native Bounding Box" ), coverage.nativeBoundingBox.toString() );

  metadata += htmlRow( tr( "WGS 84 Bounding Box" ), coverage.wgs84BoundingBox.toString() );

  // Layer Coordinate Reference Systems
  // TODO(?): supportedCrs and supportedFormat are not available in 1.0
  // until coverage is described - it would be confusing to show it only if available
  /*
  for ( int j = 0; j < qMin( coverage.supportedCrs.size(), 10 ); j++ )
  {
    metadata += htmlRow ( tr( "Available in CRS" ), coverage.supportedCrs.value(j) );
  }

  if ( coverage.supportedCrs.size() > 10 )
  {
    metadata += htmlRow ( tr( "Available in CRS" ), tr( "(and %n more)", "crs", coverage.supportedCrs.size() - 10 ) );
  }

  for ( int j = 0; j < qMin( coverage.supportedFormat.size(), 10 ); j++ )
  {
    metadata += htmlRow ( tr( "Available in format" ), coverage.supportedFormat.value(j) );
  }

  if ( coverage.supportedFormat.size() > 10 )
  {
    metadata += htmlRow ( tr( "Available in format" ), tr( "(and %n more)", "crs", coverage.supportedFormat.size() - 10 ) );
  }
  */

  // Close the nested table
  metadata += "</table>";
  metadata += "</td></tr>";

  return metadata;
}

QString QgsWcsProvider::metadata()
{
  QString metadata = "";

  metadata += "<tr><td>";

  metadata += "</a>&nbsp;<a href=\"#coverages\">";
  metadata += tr( "Coverages" );
  metadata += "</a>";

#if QT_VERSION >= 0x40500
  // TODO
  //metadata += "<a href=\"#cachestats\">";
  //metadata += tr( "Cache Stats" );
  //metadata += "</a> ";
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

  metadata += htmlRow(( "WCS Version" ), mCapabilities.version() );
  metadata += htmlRow( tr( "Title" ), mCapabilities.capabilities().title );
  metadata +=  htmlRow( tr( "Abstract" ), mCapabilities.capabilities().abstract );
  // TODO: probably apply stylesheet in QgsWcsCapabilities and save as html
  //metadata += htmlRow ( tr( "Keywords" ), mCapabilities.service.keywordList.join( "<br />" ) );
  //metadata += htmlRow (  tr( "Online Resource" ), "-" );
  //metadata += htmlRow (  tr( "Contact Person" ),
  //  mCapabilities.service.contactInformation.contactPersonPrimary.contactPerson
  //    + "<br />" + mCapabilities.service.contactInformation.contactPosition;
  //    + "<br />" + mCapabilities.service.contactInformation.contactPersonPrimary.contactOrganization );
  //metadata += htmlRow ( tr( "Fees" ), mCapabilities.service.fees );
  //metadata += htmlRow ( tr( "Access Constraints" ), mCapabilities.service.accessConstraints );
  //metadata += htmlRow ( tr( "Image Formats" ), mCapabilities.capability.request.getMap.format.join( "<br />" ) );
  //metadata += htmlRow (  tr( "GetCapabilitiesUrl" ), mBaseUrl );
  metadata += htmlRow( tr( "Get Coverage Url" ), mCapabilities.getCoverageUrl() + ( mIgnoreGetCoverageUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : "" ) );

  // Close the nested table
  metadata += "</table>";
  metadata += "</td></tr>";

  // Coverage properties
  metadata += "<tr><th class=\"glossy\"><a name=\"coverages\"></a>";
  metadata += tr( "Coverages" );
  metadata += "</th></tr>";

  foreach( QgsWcsCoverageSummary c, mCapabilities.coverages() )
  {
    metadata += coverageMetadata( c );
  }

  metadata += "</table>";

  QgsDebugMsg( "exiting with '"  + metadata  + "'." );

  return metadata;
}

QString QgsWcsProvider::htmlCell( const QString &text )
{
  return "<td>" + text + "</td>";
}

QString QgsWcsProvider:: htmlRow( const QString &text1, const QString &text2 )
{
  return "<tr>" + htmlCell( text1 ) +  htmlCell( text2 ) + "</tr>";
}

bool QgsWcsProvider::identify( const QgsPoint& thePoint, QMap<QString, QString>& theResults )
{
  QgsDebugMsg( "Entered" );
  theResults.clear();

  if ( !extent().contains( thePoint ) )
  {
    // Outside the raster
    for ( int i = 1; i <= bandCount(); i++ )
    {
      theResults[ generateBandName( i )] = tr( "out of extent" );
    }
    return true;
  }

  // It would be nice to use last cached block if possible, unfortunately we don't know
  // at which resolution identify() is called. It may happen, that user zoomed in with
  // layer switched off, canvas resolution increased, but provider cache was not refreshed.
  // In that case the resolution is too low and identify() could give wron results.
  // So we have to read always a block of data around the point on highest resolution
  // if not already cached.
  // TODO: change provider identify() prototype to pass also the resolution

  int width, height;
  if ( mHasSize )
  {
    width = mWidth;
    height = mHeight;
  }
  else
  {
    // Bad in any case, either not precise or too much data requests
    width = height = 1000;
  }
  double xRes = mCoverageExtent.width() / width;
  double yRes = mCoverageExtent.height() / height;

  if ( !mCachedGdalDataset ||
       !mCachedViewExtent.contains( thePoint ) ||
       mCachedViewWidth == 0 || mCachedViewHeight == 0 ||
       mCachedViewExtent.width() / mCachedViewWidth - xRes > TINY_VALUE ||
       mCachedViewExtent.height() / mCachedViewHeight - yRes > TINY_VALUE )
  {
    int half = 50;
    QgsRectangle extent( thePoint.x() - xRes * half, thePoint.y() - yRes * half,
                         thePoint.x() + xRes * half, thePoint.y() + yRes * half );
    getCache( 1, extent, 2*half, 2*half );

  }

  if ( !mCachedGdalDataset ||
       !mCachedViewExtent.contains( thePoint ) )
  {
    return false; // should not happen
  }

  double x = thePoint.x();
  double y = thePoint.y();

  // Calculate the row / column where the point falls
  xRes = ( mCachedViewExtent.xMaximum() - mCachedViewExtent.xMinimum() ) / mCachedViewWidth;
  yRes = ( mCachedViewExtent.yMaximum() - mCachedViewExtent.yMinimum() ) / mCachedViewHeight;

  // Offset, not the cell index -> flor
  int col = ( int ) floor(( x - mCachedViewExtent.xMinimum() ) / xRes );
  int row = ( int ) floor(( mCachedViewExtent.yMaximum() - y ) / yRes );

  QgsDebugMsg( "row = " + QString::number( row ) + " col = " + QString::number( col ) );

  for ( int i = 1; i <= GDALGetRasterCount( mCachedGdalDataset ); i++ )
  {
    GDALRasterBandH gdalBand = GDALGetRasterBand( mCachedGdalDataset, i );
    double value;

    CPLErr err = GDALRasterIO( gdalBand, GF_Read, col, row, 1, 1,
                               &value, 1, 1, GDT_Float64, 0, 0 );

    if ( err != CPLE_None )
    {
      QgsLogger::warning( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
    }

    QString v;

    if ( mValidNoDataValue && ( fabs( value - mNoDataValue[i-1] ) <= TINY_VALUE || value != value ) )
    {
      v = tr( "null (no data)" );
    }
    else
    {
      v.setNum( value );
    }

    theResults[ generateBandName( i )] = v;
  }

  return true;
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
