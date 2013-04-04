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
#include "qgsnetworkreplyparser.h"
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
#include <QFile>

#ifdef QGISDEBUG
#include <QDir>
#endif

#include "gdalwarper.h"
#include "ogr_spatialref.h"
#include "cpl_conv.h"
#include "cpl_string.h"

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8F(x) (x).toUtf8().constData()
#define FROM8(x) QString::fromUtf8(x)
#else
#define TO8F(x) QFile::encodeName( x ).constData()
#define FROM8(x) QString::fromLocal8Bit(x)
#endif

#define ERR(message) QGS_ERROR_MESSAGE(message,"WCS provider")
#define SRVERR(message) QGS_ERROR_MESSAGE(message,"WCS server")

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
    , mFixBox( false )
    , mFixRotate( false )
    , mCacheLoadControl( QNetworkRequest::PreferNetwork )
{
  QgsDebugMsg( "constructing with uri '" + mHttpUri + "'." );

  mValid = false;
  mCachedMemFilename = QString( "/vsimem/qgis/wcs/%0.dat" ).arg(( qlonglong )this );

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
    appendError( ERR( tr( "Cannot describe coverage" ) ) );
    return;
  }

  mCoverageSummary = mCapabilities.coverage( mIdentifier );
  if ( !mCoverageSummary.valid )
  {
    // Should not happen if describeCoverage() did not fail
    appendError( ERR( tr( "Coverage not found" ) ) );
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

  // It may happen that coverage CRS is not given or it is unknown
  // in that case we continue without CRS and user is asked for it
  //if ( mCoverageCrs.isEmpty() ) return;

  // Native size
  mWidth = mCoverageSummary.width;
  mHeight = mCoverageSummary.height;
  mHasSize = mCoverageSummary.hasSize;

  QgsDebugMsg( QString( "mWidth = %1 mHeight = %2" ).arg( mWidth ).arg( mHeight ) );

  // TODO: Consider if/how to recalculate mWidth, mHeight if non native CRS is used

  if ( !calculateExtent() )
  {
    appendError( ERR( tr( "Cannot calculate extent" ) ) );
    return;
  }

  // Get small piece of coverage to find GDAL data type and nubmer of bands
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

  // width and height different to recognize rotation
  int requestWidth = 6;
  int requestHeight = 3;

  // extent to be used for test request
  double halfWidth = xRes * ( requestWidth / 2. );
  double halfHeight = yRes * ( requestHeight / 2. );

  // Using non native CRS (if we don't know which is native) it could easily happen,
  // that a small part of bbox in request CRS near margin falls outside
  // coverage native bbox and server reports error => take a piece from center
  QgsRectangle extent = QgsRectangle( p.x() - halfWidth, p.y() - halfHeight, p.x() + halfWidth, p.y() + halfHeight );

  getCache( bandNo, extent, requestWidth, requestHeight, crs );

  if ( !mCachedGdalDataset )
  {
    setError( mCachedError );
    appendError( ERR( tr( "Cannot get test dataset." ) ) );
    return;
  }

  mBandCount = GDALGetRasterCount( mCachedGdalDataset );
  QgsDebugMsg( QString( "mBandCount = %1" ).arg( mBandCount ) ) ;

  // Check for server particularities (bbox, rotation)
  int responseWidth = GDALGetRasterXSize( mCachedGdalDataset );
  int responseHeight = GDALGetRasterYSize( mCachedGdalDataset );

  QgsDebugMsg( QString( "requestWidth = %1 requestHeight = %2 responseWidth = %3 responseHeight = %4)" ).arg( requestWidth ).arg( requestHeight ).arg( responseWidth ).arg( responseHeight ) );
  // GeoServer and ArcGIS are using for 1.1 box "pixel" edges
  // Mapserver is using pixel centers according to 1.1. specification
  if (( responseWidth == requestWidth - 1 && responseHeight == requestHeight - 1 ) ||
      ( responseWidth == requestHeight - 1 && responseHeight == requestWidth - 1 ) )
  {
    mFixBox = true;
    QgsDebugMsg( "Test response size is smaller by pixel, using mFixBox" );
  }
  // Geoserver is giving rotated raster for geographic CRS - switched axis,
  // Geoserver developers argue that changed axis order applies also to
  // returned raster, that is exagerated IMO but we have to handle that.
  if (( responseWidth == requestHeight && responseHeight == requestWidth ) ||
      ( responseWidth == requestHeight - 1 && responseHeight == requestWidth - 1 ) )
  {
    mFixRotate = true;
    QgsDebugMsg( "Test response is rotated, using mFixRotate" );
  }

  // Get types
  // TODO: we are using the same data types like GDAL (not wider like GDAL provider)
  // with expectation to replace 'no data' values by NaN
  for ( int i = 1; i <= mBandCount; i++ )
  {
    GDALRasterBandH gdalBand = GDALGetRasterBand( mCachedGdalDataset, i );
    GDALDataType myGdalDataType = GDALGetRasterDataType( gdalBand );

    QgsDebugMsg( QString( "myGdalDataType[%1] = %2" ).arg( i - 1 ).arg( myGdalDataType ) );
    mSrcGdalDataType.append( myGdalDataType );

    // UMN Mapserver does not automaticaly set null value, METADATA wcs_rangeset_nullvalue must be used
    // http://lists.osgeo.org/pipermail/mapserver-users/2010-April/065328.html

    // TODO: This could be shared with GDAL provider
    int isValid = false;
    double myNoDataValue = GDALGetRasterNoDataValue( gdalBand, &isValid );
    if ( isValid )
    {
      QgsDebugMsg( QString( "GDALGetRasterNoDataValue = %1" ).arg( myNoDataValue ) ) ;
      mSrcNoDataValue.append( myNoDataValue );
      mSrcHasNoDataValue.append( true );
      mUseSrcNoDataValue.append( true );
    }
    else
    {
      mSrcNoDataValue.append( std::numeric_limits<double>::quiet_NaN() );
      mSrcHasNoDataValue.append( false );
      mUseSrcNoDataValue.append( false );
    }
    // It may happen that nodata value given by GDAL is wrong and it has to be
    // disabled by user, in that case we need another value to be used for nodata
    // (for reprojection for example) -> always internaly represent as wider type
    // with mInternalNoDataValue in reserve.
    int myInternalGdalDataType = myGdalDataType;
    double myInternalNoDataValue;
    switch ( srcDataType( i ) )
    {
      case QGis::Byte:
        myInternalNoDataValue = -32768.0;
        myInternalGdalDataType = GDT_Int16;
        break;
      case QGis::Int16:
        myInternalNoDataValue = -2147483648.0;
        myInternalGdalDataType = GDT_Int32;
        break;
      case QGis::UInt16:
        myInternalNoDataValue = -2147483648.0;
        myInternalGdalDataType = GDT_Int32;
        break;
      case QGis::Int32:
        // We believe that such values is no used in real data
        myInternalNoDataValue = -2147483648.0;
        break;
      case QGis::UInt32:
        // We believe that such values is no used in real data
        myInternalNoDataValue = 4294967295.0;
        break;
      default: // Float32, Float64
        //myNoDataValue = std::numeric_limits<int>::max();
        // NaN should work well
        myInternalNoDataValue = std::numeric_limits<double>::quiet_NaN();
    }
    mGdalDataType.append( myInternalGdalDataType );
    mInternalNoDataValue.append( myInternalNoDataValue );
    QgsDebugMsg( QString( "mInternalNoDataValue[%1] = %2" ).arg( i - 1 ).arg( mInternalNoDataValue[i-1] ) );

    // TODO: what to do if null values from DescribeCoverage differ?
    if ( !mCoverageSummary.nullValues.contains( myNoDataValue ) )
    {
      QgsDebugMsg( QString( "noDataValue %1 is missing in nullValues from CoverageDescription" ).arg( myNoDataValue ) );
    }

    QgsDebugMsg( QString( "mSrcGdalDataType[%1] = %2" ).arg( i - 1 ).arg( mSrcGdalDataType[i-1] ) );
    QgsDebugMsg( QString( "mGdalDataType[%1] = %2" ).arg( i - 1 ).arg( mGdalDataType[i-1] ) );
    QgsDebugMsg( QString( "mSrcNoDataValue[%1] = %2" ).arg( i - 1 ).arg( mSrcNoDataValue[i-1] ) );

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

  mTime = uri.param( "time" );

  setFormat( uri.param( "format" ) );

  if ( !uri.param( "crs" ).isEmpty() )
  {
    setCoverageCrs( uri.param( "crs" ) );
  }

  QString cache = uri.param( "cache" );
  if ( !cache.isEmpty() )
  {
    mCacheLoadControl = QgsNetworkAccessManager::cacheLoadControlFromName( cache );
  }
  QgsDebugMsg( QString( "mCacheLoadControl = %1" ).arg( mCacheLoadControl ) ) ;

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

QgsRasterInterface * QgsWcsProvider::clone() const
{
  QgsWcsProvider * provider = new QgsWcsProvider( dataSourceUri() );
  return provider;
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
  memset( block, 0, pixelWidth * pixelHeight * QgsRasterBlock::typeSize( dataType( bandNo ) ) );

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
    // It may happen (Geoserver) that if requested BBOX is larger than coverage
    // extent, the returned data cover intersection of requested BBOX and coverage
    // extent scaled to requested WIDTH/HEIGHT => check extent
    // Unfortunately if received raster does not hac CRS, the extent is raster size
    // and in that case it cannot be used to verify extent
    QgsCoordinateReferenceSystem cacheCrs;
    if ( !cacheCrs.createFromWkt( GDALGetProjectionRef( mCachedGdalDataset ) ) &&
         !cacheCrs.createFromWkt( GDALGetGCPProjection( mCachedGdalDataset ) ) )
    {
      QgsDebugMsg( "Cached does not have CRS" );
    }
    QgsDebugMsg( "Cache CRS: " + cacheCrs.authid() + " " + cacheCrs.description() );

    QgsRectangle cacheExtent = QgsGdalProviderBase::extent( mCachedGdalDataset );
    QgsDebugMsg( "viewExtent = " + viewExtent.toString() );
    QgsDebugMsg( "cacheExtent = " + cacheExtent.toString() );
    // TODO: check also rotated
    if ( cacheCrs.isValid() && !mFixRotate )
    {
      // using doubleNear is too precise, example accetable difference:
      // 179.9999999306699863 x 179.9999999306700431
      if ( !doubleNearSig( cacheExtent.xMinimum(), viewExtent.xMinimum(), 10 ) ||
           !doubleNearSig( cacheExtent.yMinimum(), viewExtent.yMinimum(), 10 ) ||
           !doubleNearSig( cacheExtent.xMaximum(), viewExtent.xMaximum(), 10 ) ||
           !doubleNearSig( cacheExtent.yMaximum(), viewExtent.yMaximum(), 10 ) )
      {
        QgsDebugMsg( "cacheExtent and viewExtent differ" );
        QgsMessageLog::logMessage( tr( "Received coverage has wrong extent %1 (expected %2)" ).arg( cacheExtent.toString() ).arg( viewExtent.toString() ), tr( "WCS" ) );
        // We are doing all possible to avoid this situation,
        // If it happens, it would be possible to rescale the portion we get
        // to only part of the data block, but it is better to left it
        // blank, so that the problem may be discovered in its origin.
        return;
      }
    }

    int width = GDALGetRasterXSize( mCachedGdalDataset );
    int height = GDALGetRasterYSize( mCachedGdalDataset );
    QgsDebugMsg( QString( "cached data width = %1 height = %2 (expected %3 x %4)" ).arg( width ).arg( height ).arg( pixelWidth ).arg( pixelHeight ) );

    GDALRasterBandH gdalBand = GDALGetRasterBand( mCachedGdalDataset, bandNo );
    // TODO: check type? , check band count?
    if ( mFixRotate && width == pixelHeight && height == pixelWidth )
    {
      // Rotate counter clockwise
      // If GridOffsets With GeoServer,
      QgsDebugMsg( tr( "Rotating raster" ) );
      int pixelSize = QgsRasterBlock::typeSize( dataType( bandNo ) );
      QgsDebugMsg( QString( "pixelSize = %1" ).arg( pixelSize ) );
      int size = width * height * pixelSize;
      void * tmpData = malloc( size );
      if ( ! tmpData )
      {
        QgsDebugMsg( QString( "Couldn't allocate memory of %1 bytes" ).arg( size ) );
        return;
      }
      GDALRasterIO( gdalBand, GF_Read, 0, 0, width, height, tmpData, width, height, ( GDALDataType ) mGdalDataType[bandNo-1], 0, 0 );
      for ( int i = 0; i < pixelHeight; i++ )
      {
        for ( int j = 0; j < pixelWidth; j++ )
        {
          int destIndex = pixelSize * ( i * pixelWidth + j );
          int srcIndex = pixelSize * ( j * width + ( width - i - 1 ) );
          memcpy(( char* )block + destIndex, ( char* )tmpData + srcIndex, pixelSize );
        }
      }
      free( tmpData );
    }
    else if ( width == pixelWidth && height == pixelHeight )
    {
      GDALRasterIO( gdalBand, GF_Read, 0, 0, pixelWidth, pixelHeight, block, pixelWidth, pixelHeight, ( GDALDataType ) mGdalDataType[bandNo-1], 0, 0 );
      QgsDebugMsg( tr( "Block read OK" ) );
    }
    else
    {
      // This should not happen, but it is better to give distorted result + warning
      GDALRasterIO( gdalBand, GF_Read, 0, 0, width, height, block, pixelWidth, pixelHeight, ( GDALDataType ) mGdalDataType[bandNo-1], 0, 0 );
      QgsMessageLog::logMessage( tr( "Received coverage has wrong size %1 x %2 (expected %3 x %4)" ).arg( width ).arg( height ).arg( pixelWidth ).arg( pixelHeight ), tr( "WCS" ) );
    }
  }
}

void QgsWcsProvider::getCache( int bandNo, QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight, QString crs )
{
  Q_UNUSED( bandNo );
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
  // WCS 1.1 grid is using grid points (surrounded by sample spaces) and
  // "The spatial extent of a grid coverage extends only as far as the outermost
  // grid points contained in the bounding box. It does NOT include any area
  // (partial or whole grid cells or sample spaces) beyond those grid points."
  // Mapserver and GDAL are using bbox defined by grid points, i.e. shrinked
  // by 1 pixel, but Geoserver and ArcGIS are using full bbox including
  // the space around edge grid points.
  if ( mCapabilities.version().startsWith( "1.1" ) && !mFixBox )
  {
    // shrink the extent to border cells centers by half cell size
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
    if ( !mTime.isEmpty() )
    {
      // It seems that Mmapserver (6.0.3) WCS 1.1 completely ignores
      // TemporalDomain. Some code (copy-pasted from 1.0) is commented in
      // msWCSDescribeCoverage_CoverageDescription11() and GetCoverage
      // TimeSequence param is not supported at all. If a coverage is defined
      // with timeposition in mapfile, the result of GetCoverage is empty
      // raster (all values 0).
      setQueryItem( url, "TIME", mTime );
    }
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

    if ( !mTime.isEmpty() )
    {
      setQueryItem( url, "TIMESEQUENCE", mTime );
    }

    setQueryItem( url, "BOUNDINGBOX", bbox );

    //  Example:
    //   GridBaseCRS=urn:ogc:def:crs:SG:6.6:32618
    //   GridType=urn:ogc:def:method:CS:1.1:2dGridIn2dCrs
    //   GridCS=urn:ogc:def:cs:OGC:0Grid2dSquareCS
    //   GridOrigin=0,0
    //   GridOffsets=0.0707,-0.0707,0.1414,0.1414&

    setQueryItem( url, "GRIDBASECRS", crsUrn ); // response CRS

    setQueryItem( url, "GRIDCS", "urn:ogc:def:cs:OGC:0.0:Grid2dSquareCS" );

    setQueryItem( url, "GRIDTYPE", "urn:ogc:def:method:WCS:1.1:2dSimpleGrid" );

    // GridOrigin is BBOX minx, maxy
    // Note: shifting origin to cell center (not realy necessary nor making sense)
    // does not work with Mapserver 6.0.3
    // Mapserver 6.0.3 does not work with origin on yMinimum (lower left)
    // Geoserver works OK with yMinimum (lower left)
    QString gridOrigin = QString( changeXY ? "%2,%1" : "%1,%2" )
                         .arg( extent.xMinimum(), 0, 'f', 16 )
                         .arg( extent.yMaximum(), 0, 'f', 16 );
    setQueryItem( url, "GRIDORIGIN", gridOrigin );

    // GridOffsets WCS 1.1:
    // GridType urn:ogc:def:method:WCS:1.1:2dSimpleGrid : 2 values
    // GridType urn:ogc:def:method:WCS:1.1:2dGridIn2dCrs : 4 values, the center two of these offsets will be zero when the GridCRS is not rotated or skewed in the GridBaseCRS.
    // The yOff must be negative because origin is in upper corner
    // WCS 1.1.2: BaseX = origin(1) + offsets(1) * GridX
    //            BaseY = origin(2) + offsets(2) * GridY
    // otherwise GeoServer gives mirrored result, however
    // Mapserver works OK with both positive and negative
    // OTOH, the yOff offset must not be negative with mFixRotate and Geoserver 2.1-SNAPSHOT
    // but it must be negative with GeoServer 2.1.3 and mFixRotate. I am not sure
    // at this moment 100% -> disabling positive yOff for now - TODO: try other servers
    //double yOff = mFixRotate ? yRes : -yRes; // this was working with some servers I think
    double yOff = -yRes;
    QString gridOffsets = QString( changeXY ? "%2,%1" : "%1,%2" )
                          //setQueryItem( url, "GRIDTYPE", "urn:ogc:def:method:WCS:1.1:2dGridIn2dCrs" );
                          //QString gridOffsets = QString( changeXY ? "%2,0,0,%1" : "%1,0,0,%2" )
                          .arg( xRes, 0, 'f', 16 )
                          .arg( yOff, 0, 'f', 16 );
    setQueryItem( url, "GRIDOFFSETS", gridOffsets );
  }

  QgsDebugMsg( QString( "GetCoverage: %1" ).arg( url.toString() ) );

  // cache some details for if the user wants to do an identifyAsHtml() later

  //mGetFeatureInfoUrlBase = mIgnoreGetFeatureInfoUrl ? mBaseUrl : getFeatureInfoUrl();

  QNetworkRequest request( url );
  setAuthorization( request );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, mCacheLoadControl );
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
  QgsDebugMsg( QString( "mCacheReply->error() = %1" ).arg( mCacheReply->error() ) );
  if ( mCacheReply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = mCacheReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      mCacheReply->deleteLater();

      QgsDebugMsg( QString( "redirected getmap: %1" ).arg( redirect.toString() ) );
      mCacheReply = QgsNetworkAccessManager::instance()->get( QNetworkRequest( redirect.toUrl() ) );
      connect( mCacheReply, SIGNAL( finished() ), this, SLOT( cacheReplyFinished() ) );
      connect( mCacheReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( cacheReplyProgress( qint64, qint64 ) ) );
      return;
    }

    QVariant status = mCacheReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    QgsDebugMsg( QString( "status = %1" ).arg( status.toInt() ) );
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
    // Content type examples: text/xml
    //                        application/vnd.ogc.se_xml;charset=UTF-8
    //                        application/xml
    if ( contentType.startsWith( "text/", Qt::CaseInsensitive ) ||
         contentType.toLower() == "application/xml" ||
         contentType.startsWith( "application/vnd.ogc.se_xml", Qt::CaseInsensitive ) )
    {
      QByteArray text = mCacheReply->readAll();
      if (( contentType.toLower() == "text/xml" ||
            contentType.toLower() == "application/xml" ||
            contentType.startsWith( "application/vnd.ogc.se_xml", Qt::CaseInsensitive ) )
          && parseServiceExceptionReportDom( text ) )
      {
        mCachedError.append( SRVERR( tr( "Map request error:<br>Title: %1<br>Error: %2<br>URL: <a href='%3'>%3</a>)" )
                                     .arg( mErrorCaption ).arg( mError )
                                     .arg( mCacheReply->url().toString() ) ) );
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
    if ( QgsNetworkReplyParser::isMultipart( mCacheReply ) )
    {
      QgsDebugMsg( "reply is multipart" );
      QgsNetworkReplyParser parser( mCacheReply );

      if ( !parser.isValid() )
      {
        QgsMessageLog::logMessage( tr( "Cannot parse multipart response: %1" ).arg( parser.error() ), tr( "WCS" ) );
        clearCache();
        mCacheReply->deleteLater();
        mCacheReply = 0;
        return;
      }

      if ( parser.parts() < 2 )
      {
        QgsMessageLog::logMessage( tr( "Expected 2 parts, %1 received" ).arg( parser.parts() ), tr( "WCS" ) );
        clearCache();
        mCacheReply->deleteLater();
        mCacheReply = 0;
        return;
      }
      else if ( parser.parts() > 2 )
      {
        // We will try the second one
        QgsMessageLog::logMessage( tr( "More than 2 parts (%1) received" ).arg( parser.parts() ), tr( "WCS" ) );
      }

      QString transferEncoding = parser.rawHeader( 1, QString( "Content-Transfer-Encoding" ).toAscii() );
      QgsDebugMsg( "transferEncoding = " + transferEncoding );

      // It may happen (GeoServer) that in part header is for example
      // Content-Type: image/tiff and Content-Transfer-Encoding: base64
      // but content is xml ExceptionReport which is not in base64
      QByteArray body = parser.body( 1 );
      if ( body.startsWith( "<?xml" ) )
      {
        if ( parseServiceExceptionReportDom( body ) )
        {
          QgsMessageLog::logMessage( tr( "Map request error (Title:%1; Error:%2; URL: %3)" )
                                     .arg( mErrorCaption ).arg( mError )
                                     .arg( mCacheReply->url().toString() ), tr( "WCS" ) );
        }
        else
        {
          QgsMessageLog::logMessage( tr( "Map request error (Response: %1; URL:%2)" )
                                     .arg( QString::fromUtf8( body ) )
                                     .arg( mCacheReply->url().toString() ), tr( "WCS" ) );
        }

        mCacheReply->deleteLater();
        mCacheReply = 0;
        return;
      }

      if ( transferEncoding == "binary" )
      {
        mCachedData = body;
      }
      else if ( transferEncoding == "base64" )
      {
        mCachedData = QByteArray::fromBase64( body );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Content-Transfer-Encoding %1 not supported" ).arg( transferEncoding ), tr( "WCS" ) );
        clearCache();
        mCacheReply->deleteLater();
        mCacheReply = 0;
        return;
      }
    }
    else
    {
      // Mime types for WCS 1.0 response should be usually image/<format>
      // (image/tiff, image/png, image/jpeg, image/png; mode=8bit, etc.)
      // but other mime types (like application/*) may probably also appear

      // Create memory file
      mCachedData = mCacheReply->readAll();
    }

    QgsDebugMsg( QString( "%1 bytes received" ).arg( mCachedData.size() ) );
    if ( mCachedData.size() == 0 )
    {
      QgsMessageLog::logMessage( tr( "No data received" ), tr( "WCS" ) );
      clearCache();
      mCacheReply->deleteLater();
      mCacheReply = 0;
      return;
    }

#if 0
    QFile myFile( "/tmp/qgiswcscache.dat" );
    if ( myFile.open( QIODevice::WriteOnly ) )
    {
      QDataStream myStream( &myFile );
      myStream.writeRawData( mCachedData.data(),  mCachedData.size() );
      myFile.close();
    }
#endif

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
    // Resend request if AlwaysCache
    QNetworkRequest request = mCacheReply->request();
    if ( request.attribute( QNetworkRequest::CacheLoadControlAttribute ).toInt() == QNetworkRequest::AlwaysCache )
    {
      QgsDebugMsg( "Resend request with PreferCache" );
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );

      mCacheReply->deleteLater();

      mCacheReply = QgsNetworkAccessManager::instance()->get( request );
      connect( mCacheReply, SIGNAL( finished() ), this, SLOT( cacheReplyFinished() ) );
      connect( mCacheReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( cacheReplyProgress( qint64, qint64 ) ) );
      return;
    }

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
QGis::DataType QgsWcsProvider::srcDataType( int bandNo ) const
{
  if ( bandNo < 0 || bandNo > mSrcGdalDataType.size() )
  {
    return QGis::UnknownDataType;
  }

  return dataTypeFromGdal( mSrcGdalDataType[bandNo-1] );
}

QGis::DataType QgsWcsProvider::dataType( int bandNo ) const
{
  if ( bandNo < 0 || bandNo > mGdalDataType.size() )
  {
    return QGis::UnknownDataType;
  }

  return dataTypeFromGdal( mGdalDataType[bandNo-1] );
}

int QgsWcsProvider::bandCount() const
{
  return mBandCount;
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
  mCachedError.clear();
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
  QgsDebugMsgLevel( msg, 3 );
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
  exceptions["MissingParameterValue"] = tr( "Request does not include a parameter value, and the server instance did not declare a default value for that dimension." );
  exceptions["InvalidParameterValue"] = tr( "Request contains an invalid parameter value." );
  // 1.1
  exceptions["NoApplicableCode"] = tr( "No other exceptionCode specified by this service and server applies to this exception." );
  exceptions["UnsupportedCombination"] = tr( "Operation request contains an output CRS that can not be used within the output format." );
  exceptions["NotEnoughStorage"] = tr( "Operation request specifies to \"store\" the result, but not enough storage is available to do this." );

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
  // transformation from WGS84 enlarges the extent
  mCoverageExtent = mCoverageSummary.boundingBoxes.value( mCoverageCrs );
  QgsDebugMsg( "mCoverageCrs = " + mCoverageCrs + " mCoverageExtent = " + mCoverageExtent.toString() );
  if ( !mCoverageExtent.isEmpty() && mCoverageExtent.isFinite() )
  {
    QgsDebugMsg( "mCoverageExtent = " + mCoverageExtent.toString() );
    //return true;
  }
  else
  {
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
  }

  QgsDebugMsg( "mCoverageExtent = " + mCoverageExtent.toString() );

  // It may happen (GeoServer) that extent reported in spatialDomain.Envelope is larger
  // than the coverage. Then if that larger BBOX is requested, the server returns
  // request BBOX intersected with coverage box scaled to requested WIDTH and HEIGHT.
  // GDAL WCS client does not suffer from this probably because it probably takes
  // extent from lonLatEnvelope (it probably does not calculate it from
  // spatialDomain.RectifiedGrid because calculated value is slightly different).

  // To get the true extent (it can also be smaller than real if reported Envelope is
  // than real smaller, but smaller is safer because data cannot be shifted) we make
  // request of the whole extent cut the extent from spatialDomain.Envelope if
  // necessary

  getCache( 1, mCoverageExtent, 10, 10 );
  if ( mCachedGdalDataset )
  {
    QgsRectangle cacheExtent = QgsGdalProviderBase::extent( mCachedGdalDataset );
    QgsDebugMsg( "mCoverageExtent = " + mCoverageExtent.toString() );
    QgsDebugMsg( "cacheExtent = " + cacheExtent.toString() );
    QgsCoordinateReferenceSystem cacheCrs;
    if ( !cacheCrs.createFromWkt( GDALGetProjectionRef( mCachedGdalDataset ) ) &&
         !cacheCrs.createFromWkt( GDALGetGCPProjection( mCachedGdalDataset ) ) )
    {
      QgsDebugMsg( "Cached does not have CRS" );
    }
    QgsDebugMsg( "Cache CRS: " + cacheCrs.authid() + " " + cacheCrs.description() );

    // We can only verify extent if CRS is set
    // If dataset comes rotated, GDAL probably cuts latitude extend, disable
    // extent check for rotated, TODO: verify
    if ( cacheCrs.isValid() && !mFixRotate )
    {
      if ( !doubleNearSig( cacheExtent.xMinimum(), mCoverageExtent.xMinimum(), 10 ) ||
           !doubleNearSig( cacheExtent.yMinimum(), mCoverageExtent.yMinimum(), 10 ) ||
           !doubleNearSig( cacheExtent.xMaximum(), mCoverageExtent.xMaximum(), 10 ) ||
           !doubleNearSig( cacheExtent.yMaximum(), mCoverageExtent.yMaximum(), 10 ) )
      {
        QgsDebugMsg( "cacheExtent and mCoverageExtent differ, mCoverageExtent cut to cacheExtent" );
        mCoverageExtent = cacheExtent;
      }
    }
  }
  else
  {
    // Unfortunately it may also happen that a server (cubewerx.com) does not have
    // overviews and it is not able to respond for the whole extent within timeout.
    // It returns timeout error.
    // In that case (if request failed) we do not report error to allow to work
    // with such servers on smaller portions of extent
    // (http://lists.osgeo.org/pipermail/qgis-developer/2013-January/024019.html)

    // Unfortunately even if we get over this 10x10 check, QGIS also requests
    // 32x32 thumbnail where it is waiting for another timeout

    QgsDebugMsg( "Cannot get cache to verify extent" );
    QgsMessageLog::logMessage( tr( "Cannot verify coverage full extent: %1" ).arg( mCachedError.message() ), tr( "WCS" ) );
  }

  return true;
}


int QgsWcsProvider::capabilities() const
{
  int capability = NoCapabilities;
  capability |= QgsRasterDataProvider::Identify;
  capability |= QgsRasterDataProvider::IdentifyValue;
  capability |= QgsRasterDataProvider::Histogram;

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
#if 0
  // We don't have size, nativeCrs, nativeBoundingBox etc. until describe coverage which would be heavy for all coverages
  metadata += htmlRow( tr( "Fixed Width" ), QString::number( coverage.width ) );
  metadata += htmlRow( tr( "Fixed Height" ), QString::number( coverage.height ) );
  metadata += htmlRow( tr( "Native CRS" ), coverage.nativeCrs );
  metadata += htmlRow( tr( "Native Bounding Box" ), coverage.nativeBoundingBox.toString() );
#endif

  metadata += htmlRow( tr( "WGS 84 Bounding Box" ), coverage.wgs84BoundingBox.toString() );

  // Layer Coordinate Reference Systems
  // TODO(?): supportedCrs and supportedFormat are not available in 1.0
  // until coverage is described - it would be confusing to show it only if available
#if 0
  for ( int j = 0; j < qMin( coverage.supportedCrs.size(), 10 ); j++ )
  {
    metadata += htmlRow( tr( "Available in CRS" ), coverage.supportedCrs.value( j ) );
  }

  if ( coverage.supportedCrs.size() > 10 )
  {
    metadata += htmlRow( tr( "Available in CRS" ), tr( "(and %n more)", "crs", coverage.supportedCrs.size() - 10 ) );
  }

  for ( int j = 0; j < qMin( coverage.supportedFormat.size(), 10 ); j++ )
  {
    metadata += htmlRow( tr( "Available in format" ), coverage.supportedFormat.value( j ) );
  }

  if ( coverage.supportedFormat.size() > 10 )
  {
    metadata += htmlRow( tr( "Available in format" ), tr( "(and %n more)", "crs", coverage.supportedFormat.size() - 10 ) );
  }
#endif

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

#if 0
#if QT_VERSION >= 0x40500
  // TODO
  metadata += "<a href=\"#cachestats\">";
  metadata += tr( "Cache Stats" );
  metadata += "</a> ";
#endif
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
#if 0
  // TODO: probably apply stylesheet in QgsWcsCapabilities and save as html
  metadata += htmlRow( tr( "Keywords" ), mCapabilities.service.keywordList.join( "<br />" ) );
  metadata += htmlRow( tr( "Online Resource" ), "-" );
  metadata += htmlRow( tr( "Contact Person" ),
                       mCapabilities.service.contactInformation.contactPersonPrimary.contactPerson
                       + "<br />" + mCapabilities.service.contactInformation.contactPosition;
                       + "<br />" + mCapabilities.service.contactInformation.contactPersonPrimary.contactOrganization );
  metadata += htmlRow( tr( "Fees" ), mCapabilities.service.fees );
  metadata += htmlRow( tr( "Access Constraints" ), mCapabilities.service.accessConstraints );
  metadata += htmlRow( tr( "Image Formats" ), mCapabilities.capability.request.getMap.format.join( "<br />" ) );
  metadata += htmlRow( tr( "GetCapabilitiesUrl" ), mBaseUrl );
#endif
  metadata += htmlRow( tr( "Get Coverage Url" ), mCapabilities.getCoverageUrl() + ( mIgnoreGetCoverageUrl ? tr( "&nbsp;<font color=\"red\">(advertised but ignored)</font>" ) : "" ) );

  // Close the nested table
  metadata += "</table>";
  metadata += "</td></tr>";

  // Coverage properties
  metadata += "<tr><th class=\"glossy\"><a name=\"coverages\"></a>";
  metadata += tr( "Coverages" );
  metadata += "</th></tr>";

  // Dialog takes too long to open if there are too many coverages (1000 for example)
  int count = 0;
  foreach ( QgsWcsCoverageSummary c, mCapabilities.coverages() )
  {
    metadata += coverageMetadata( c );
    count++;
    if ( count >= 100 ) break;
  }
  metadata += "</table>";
  if ( count < mCapabilities.coverages().size() )
  {
    metadata += tr( "And %1 more coverages" ).arg( mCapabilities.coverages().size() - count );
  }

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

QMap<int, QVariant> QgsWcsProvider::identify( const QgsPoint & thePoint, IdentifyFormat theFormat, const QgsRectangle &theExtent, int theWidth, int theHeight )
{
  QgsDebugMsg( QString( "thePoint =  %1 %2" ).arg( thePoint.x(), 0, 'g', 10 ).arg( thePoint.y(), 0, 'g', 10 ) );
  QgsDebugMsg( QString( "theWidth = %1 theHeight = %2" ).arg( theWidth ).arg( theHeight ) );
  QgsDebugMsg( "theExtent = " + theExtent.toString() );
  QMap<int, QVariant> results;

  if ( theFormat != IdentifyFormatValue ) return results;

  if ( !extent().contains( thePoint ) )
  {
    // Outside the raster
    for ( int i = 1; i <= bandCount(); i++ )
    {
      results.insert( i, noDataValue( i ) );
    }
    return results;
  }

  QgsRectangle myExtent = theExtent;
  int maxSize = 2000;
  // if context size is to large we have to cut it, in that case caching big
  // big part does not make sense
  if ( myExtent.isEmpty() || theWidth == 0 || theHeight == 0 ||
       theWidth > maxSize || theHeight > maxSize )
  {
    // context missing, use an area around the point and highest resolution if known

    // 1000 is bad in any case, either not precise or too much data requests
    if ( theWidth == 0 ) theWidth = mHasSize ? mWidth : 1000;
    if ( theHeight == 0 ) theHeight = mHasSize ? mHeight : 1000;

    if ( myExtent.isEmpty() )  myExtent = extent();

    double xRes = myExtent.width() / theWidth;
    double yRes = myExtent.height() / theHeight;

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
  }
  else
  {
    // Use context -> effective caching (usually, if context is constant)
    QgsDebugMsg( "Using context extent and resolution" );
    if ( !mCachedGdalDataset ||
         mCachedViewExtent != theExtent ||
         mCachedViewWidth != theWidth ||
         mCachedViewHeight != theHeight )
    {
      getCache( 1, theExtent, theWidth, theHeight );
    }
  }

  if ( !mCachedGdalDataset ||
       !mCachedViewExtent.contains( thePoint ) )
  {
    return results; // should not happen
  }

  double x = thePoint.x();
  double y = thePoint.y();

  // Calculate the row / column where the point falls
  double xRes = mCachedViewExtent.width() / mCachedViewWidth;
  double yRes = mCachedViewExtent.height() / mCachedViewHeight;

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
      results.clear();
      return results;
    }

    // Apply user no data
    QList<QgsRasterBlock::Range> myNoDataRangeList = userNoDataValue( i );
    if ( QgsRasterBlock::valueInRange( value, myNoDataRangeList ) )
    {
      value = noDataValue( i );
    }

    results.insert( i, value );
  }

  return results;
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
