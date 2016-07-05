/***************************************************************************
  qgsgdalprovider.cpp  -  QGIS Data provider for
                           GDAL rasters
                             -------------------
    begin                : November, 2010
    copyright            : (C) 2010 by Radim Blazek
    email                : radim dot blazek at gmail dot com
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
#include "qgsgdalproviderbase.h"
#include "qgsgdalprovider.h"
#include "qgsconfig.h"

#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsdataitem.h"
#include "qgsdatasourceuri.h"
#include "qgsmessagelog.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrasterbandstats.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrasterlayer.h"
#include "qgsrasterpyramid.h"
#include "qgscrscache.h"
#include "qgspoint.h"

#include <QImage>
#include <QSettings>
#include <QColor>
#include <QProcess>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QHash>
#include <QTime>
#include <QTextDocument>
#include <QDebug>

#include "gdalwarper.h"
#include "ogr_spatialref.h"
#include "cpl_conv.h"
#include "cpl_string.h"

#define ERRMSG(message) QGS_ERROR_MESSAGE(message,"GDAL provider")
#define ERR(message) QgsError(message,"GDAL provider")

static QString PROVIDER_KEY = "gdal";
static QString PROVIDER_DESCRIPTION = "GDAL provider";

struct QgsGdalProgress
{
  int type;
  QgsGdalProvider *provider;
};
//
// global callback function
//
int CPL_STDCALL progressCallback( double dfComplete,
                                  const char * pszMessage,
                                  void * pProgressArg )
{
  static double dfLastComplete = -1.0;

  QgsGdalProgress *prog = static_cast<QgsGdalProgress *>( pProgressArg );
  QgsGdalProvider *mypProvider = prog->provider;

  if ( dfLastComplete > dfComplete )
  {
    if ( dfLastComplete >= 1.0 )
      dfLastComplete = -1.0;
    else
      dfLastComplete = dfComplete;
  }

  if ( floor( dfLastComplete*10 ) != floor( dfComplete*10 ) )
  {
    mypProvider->emitProgress( prog->type, dfComplete * 100, QString( pszMessage ) );
    mypProvider->emitProgressUpdate( dfComplete * 100 );
  }
  dfLastComplete = dfComplete;

  return true;
}

QgsGdalProvider::QgsGdalProvider( const QString &uri, QgsError error )
    : QgsRasterDataProvider( uri )
    , mUpdate( false )
    , mValid( false )
    , mHasPyramids( false )
    , mWidth( 0 )
    , mHeight( 0 )
    , mXBlockSize( 0 )
    , mYBlockSize( 0 )
    , mGdalBaseDataset( nullptr )
    , mGdalDataset( nullptr )
{
  mGeoTransform[0] =  0;
  mGeoTransform[1] =  1;
  mGeoTransform[2] =  0;
  mGeoTransform[3] =  0;
  mGeoTransform[4] =  0;
  mGeoTransform[5] = -1;
  setError( error );
}

QgsGdalProvider::QgsGdalProvider( const QString &uri, bool update )
    : QgsRasterDataProvider( uri )
    , QgsGdalProviderBase()
    , mUpdate( update )
    , mValid( false )
    , mHasPyramids( false )
    , mWidth( 0 )
    , mHeight( 0 )
    , mXBlockSize( 0 )
    , mYBlockSize( 0 )
    , mGdalBaseDataset( nullptr )
    , mGdalDataset( nullptr )
{
  mGeoTransform[0] =  0;
  mGeoTransform[1] =  1;
  mGeoTransform[2] =  0;
  mGeoTransform[3] =  0;
  mGeoTransform[4] =  0;
  mGeoTransform[5] = -1;

  QgsDebugMsg( "constructing with uri '" + uri + "'." );

  QgsGdalProviderBase::registerGdalDrivers();

  if ( !CPLGetConfigOption( "AAIGRID_DATATYPE", nullptr ) )
  {
    // GDAL tends to open AAIGrid as Float32 which results in lost precision
    // and confusing values shown to users, force Float64
    CPLSetConfigOption( "AAIGRID_DATATYPE", "Float64" );
  }

  // To get buildSupportedRasterFileFilter the provider is called with empty uri
  if ( uri.isEmpty() )
  {
    return;
  }

  mGdalDataset = nullptr;

  // Try to open using VSIFileHandler (see qgsogrprovider.cpp)
  QString vsiPrefix = QgsZipItem::vsiPrefix( uri );
  if ( vsiPrefix != "" )
  {
    if ( !uri.startsWith( vsiPrefix ) )
      setDataSourceUri( vsiPrefix + uri );
    QgsDebugMsg( QString( "Trying %1 syntax, uri= %2" ).arg( vsiPrefix, dataSourceUri() ) );
  }

  QString gdalUri = dataSourceUri();

  CPLErrorReset();
  mGdalBaseDataset = gdalOpen( TO8F( gdalUri ), mUpdate ? GA_Update : GA_ReadOnly );

  if ( !mGdalBaseDataset )
  {
    QString msg = QString( "Cannot open GDAL dataset %1:\n%2" ).arg( dataSourceUri(), QString::fromUtf8( CPLGetLastErrorMsg() ) );
    appendError( ERRMSG( msg ) );
    return;
  }

  QgsDebugMsg( "GdalDataset opened" );
  initBaseDataset();
}

QgsGdalProvider* QgsGdalProvider::clone() const
{
  QgsGdalProvider * provider = new QgsGdalProvider( dataSourceUri() );
  provider->copyBaseSettings( *this );
  return provider;
}

bool QgsGdalProvider::crsFromWkt( const char *wkt )
{

  OGRSpatialReferenceH hCRS = OSRNewSpatialReference( nullptr );

  if ( OSRImportFromWkt( hCRS, ( char ** ) &wkt ) == OGRERR_NONE )
  {
    if ( OSRAutoIdentifyEPSG( hCRS ) == OGRERR_NONE )
    {
      QString authid = QString( "%1:%2" )
                       .arg( OSRGetAuthorityName( hCRS, nullptr ),
                             OSRGetAuthorityCode( hCRS, nullptr ) );
      QgsDebugMsg( "authid recognized as " + authid );
      mCrs = QgsCRSCache::instance()->crsByOgcWmsCrs( authid );
    }
    else
    {
      // get the proj4 text
      char *pszProj4;
      OSRExportToProj4( hCRS, &pszProj4 );
      QgsDebugMsg( pszProj4 );
      OGRFree( pszProj4 );

      char *pszWkt = nullptr;
      OSRExportToWkt( hCRS, &pszWkt );
      QString myWktString = QString( pszWkt );
      OGRFree( pszWkt );

      // create CRS from Wkt
      mCrs = QgsCRSCache::instance()->crsByWkt( myWktString );
    }
  }

  OSRRelease( hCRS );

  return mCrs.isValid();
}

QgsGdalProvider::~QgsGdalProvider()
{
  if ( mGdalBaseDataset )
  {
    GDALDereferenceDataset( mGdalBaseDataset );
  }
  if ( mGdalDataset )
  {
    GDALClose( mGdalDataset );
  }
}


// This was used by raster layer to reload data
void QgsGdalProvider::closeDataset()
{
  if ( !mValid )
  {
    return;
  }
  mValid = false;

  GDALDereferenceDataset( mGdalBaseDataset );
  mGdalBaseDataset = nullptr;

  GDALClose( mGdalDataset );
  mGdalDataset = nullptr;
}

QString QgsGdalProvider::metadata()
{
  QString myMetadata;
  myMetadata += QString( GDALGetDescription( GDALGetDatasetDriver( mGdalDataset ) ) );
  myMetadata += "<br>";
  myMetadata += QString( GDALGetMetadataItem( GDALGetDatasetDriver( mGdalDataset ), GDAL_DMD_LONGNAME, nullptr ) );

  // my added code (MColetti)

  myMetadata += "<p class=\"glossy\">";
  myMetadata += tr( "Dataset Description" );
  myMetadata += "</p>\n";
  myMetadata += "<p>";
  myMetadata += FROM8( GDALGetDescription( mGdalDataset ) );
  myMetadata += "</p>\n";


  char ** GDALmetadata = GDALGetMetadata( mGdalDataset, nullptr );

  if ( GDALmetadata )
  {
    QStringList metadata = cStringList2Q_( GDALmetadata );
    myMetadata += QgsRasterDataProvider::makeTableCells( metadata );
  }
  else
  {
    QgsDebugMsg( "dataset has no metadata" );
  }

  for ( int i = 1; i <= GDALGetRasterCount( mGdalDataset ); ++i )
  {
    myMetadata += "<p class=\"glossy\">" + tr( "Band %1" ).arg( i ) + "</p>\n";
    GDALRasterBandH gdalBand = GDALGetRasterBand( mGdalDataset, i );
    GDALmetadata = GDALGetMetadata( gdalBand, nullptr );

    if ( GDALmetadata )
    {
      QStringList metadata = cStringList2Q_( GDALmetadata );
      myMetadata += QgsRasterDataProvider::makeTableCells( metadata );
    }
    else
    {
      QgsDebugMsg( "band " + QString::number( i ) + " has no metadata" );
    }

    char ** GDALcategories = GDALGetRasterCategoryNames( gdalBand );

    if ( GDALcategories )
    {
      QStringList categories = cStringList2Q_( GDALcategories );
      myMetadata += QgsRasterDataProvider::makeTableCells( categories );
    }
    else
    {
      QgsDebugMsg( "band " + QString::number( i ) + " has no categories" );
    }

  }

  // end my added code

  myMetadata += "<p class=\"glossy\">";
  myMetadata += tr( "Dimensions" );
  myMetadata += "</p>\n";
  myMetadata += "<p>";
  myMetadata += tr( "X: %1 Y: %2 Bands: %3" )
                .arg( GDALGetRasterXSize( mGdalDataset ) )
                .arg( GDALGetRasterYSize( mGdalDataset ) )
                .arg( GDALGetRasterCount( mGdalDataset ) );
  myMetadata += "</p>\n";

  //just use the first band
  if ( GDALGetRasterCount( mGdalDataset ) > 0 )
  {
    GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, 1 );
    if ( GDALGetOverviewCount( myGdalBand ) > 0 )
    {
      int myOverviewInt;
      for ( myOverviewInt = 0;
            myOverviewInt < GDALGetOverviewCount( myGdalBand );
            myOverviewInt++ )
      {
        GDALRasterBandH myOverview;
        myOverview = GDALGetOverview( myGdalBand, myOverviewInt );
        myMetadata += "<p>X : " + QString::number( GDALGetRasterBandXSize( myOverview ) );
        myMetadata += ",Y " + QString::number( GDALGetRasterBandYSize( myOverview ) ) + "</p>";
      }
    }
  }

  if ( GDALGetGeoTransform( mGdalDataset, mGeoTransform ) != CE_None )
  {
    // if the raster does not have a valid transform we need to use
    // a pixel size of (1,-1), but GDAL returns (1,1)
    mGeoTransform[5] = -1;
  }
  else
  {
    myMetadata += "<p class=\"glossy\">";
    myMetadata += tr( "Origin" );
    myMetadata += "</p>\n";
    myMetadata += "<p>";
    myMetadata += QString::number( mGeoTransform[0] );
    myMetadata += ',';
    myMetadata += QString::number( mGeoTransform[3] );
    myMetadata += "</p>\n";

    myMetadata += "<p class=\"glossy\">";
    myMetadata += tr( "Pixel Size" );
    myMetadata += "</p>\n";
    myMetadata += "<p>";
    myMetadata += QString::number( mGeoTransform[1] );
    myMetadata += ',';
    myMetadata += QString::number( mGeoTransform[5] );
    myMetadata += "</p>\n";
  }

  return myMetadata;
}


// Not supported by GDAL
QImage* QgsGdalProvider::draw( QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight )
{
  Q_UNUSED( viewExtent );
  QgsDebugMsg( "pixelWidth = "  + QString::number( pixelWidth ) );
  QgsDebugMsg( "pixelHeight = "  + QString::number( pixelHeight ) );
  QgsDebugMsg( "viewExtent: " + viewExtent.toString() );

  QImage *image = new QImage( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
  image->fill( QColor( Qt::gray ).rgb() );

  return image;
}

QgsRasterBlock* QgsGdalProvider::block( int theBandNo, const QgsRectangle &theExtent, int theWidth, int theHeight )
{
  //QgsRasterBlock *block = new QgsRasterBlock( dataType( theBandNo ), theWidth, theHeight, noDataValue( theBandNo ) );
  QgsRasterBlock *block;
  if ( srcHasNoDataValue( theBandNo ) && useSrcNoDataValue( theBandNo ) )
  {
    block = new QgsRasterBlock( dataType( theBandNo ), theWidth, theHeight, srcNoDataValue( theBandNo ) );
  }
  else
  {
    block = new QgsRasterBlock( dataType( theBandNo ), theWidth, theHeight );
  }

  if ( block->isEmpty() )
  {
    return block;
  }

  if ( !mExtent.contains( theExtent ) )
  {
    QRect subRect = QgsRasterBlock::subRect( theExtent, theWidth, theHeight, mExtent );
    block->setIsNoDataExcept( subRect );
  }
  readBlock( theBandNo, theExtent, theWidth, theHeight, block->bits() );
  // apply scale and offset
  block->applyScaleOffset( bandScale( theBandNo ), bandOffset( theBandNo ) );
  block->applyNoDataValues( userNoDataValues( theBandNo ) );
  return block;
}

void QgsGdalProvider::readBlock( int theBandNo, int xBlock, int yBlock, void *block )
{
  // TODO!!!: Check data alignment!!! May it happen that nearest value which
  // is not nearest is assigned to an output cell???


  //QgsDebugMsg( "yBlock = "  + QString::number( yBlock ) );

  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  //GDALReadBlock( myGdalBand, xBlock, yBlock, block );

  // We have to read with correct data type consistent with other readBlock functions
  int xOff = xBlock * mXBlockSize;
  int yOff = yBlock * mYBlockSize;
  gdalRasterIO( myGdalBand, GF_Read, xOff, yOff, mXBlockSize, mYBlockSize, block, mXBlockSize, mYBlockSize, ( GDALDataType ) mGdalDataType.at( theBandNo - 1 ), 0, 0 );
}

void QgsGdalProvider::readBlock( int theBandNo, QgsRectangle  const & theExtent, int thePixelWidth, int thePixelHeight, void *theBlock )
{
  QgsDebugMsg( "thePixelWidth = "  + QString::number( thePixelWidth ) );
  QgsDebugMsg( "thePixelHeight = "  + QString::number( thePixelHeight ) );
  QgsDebugMsg( "theExtent: " + theExtent.toString() );

  for ( int i = 0 ; i < 6; i++ )
  {
    QgsDebugMsg( QString( "transform : %1" ).arg( mGeoTransform[i] ) );
  }

  int dataSize = dataTypeSize( theBandNo );

  // moved to block()
#if 0
  if ( !mExtent.contains( theExtent ) )
  {
    // fill with null values
    QByteArray ba = QgsRasterBlock::valueBytes( dataType( theBandNo ), noDataValue( theBandNo ) );
    char *nodata = ba.data();
    char *block = ( char * ) theBlock;
    for ( int i = 0; i < thePixelWidth * thePixelHeight; i++ )
    {
      memcpy( block, nodata, dataSize );
      block += dataSize;
    }
  }
#endif

  QgsRectangle myRasterExtent = theExtent.intersect( &mExtent );
  if ( myRasterExtent.isEmpty() )
  {
    QgsDebugMsg( "draw request outside view extent." );
    return;
  }
  QgsDebugMsg( "mExtent: " + mExtent.toString() );
  QgsDebugMsg( "myRasterExtent: " + myRasterExtent.toString() );

  double xRes = theExtent.width() / thePixelWidth;
  double yRes = theExtent.height() / thePixelHeight;

  // Find top, bottom rows and left, right column the raster extent covers
  // These are limits in target grid space
#if 0
  int top = 0;
  int bottom = thePixelHeight - 1;
  int left = 0;
  int right = thePixelWidth - 1;

  if ( myRasterExtent.yMaximum() < theExtent.yMaximum() )
  {
    top = qRound(( theExtent.yMaximum() - myRasterExtent.yMaximum() ) / yRes );
  }
  if ( myRasterExtent.yMinimum() > theExtent.yMinimum() )
  {
    bottom = qRound(( theExtent.yMaximum() - myRasterExtent.yMinimum() ) / yRes ) - 1;
  }

  if ( myRasterExtent.xMinimum() > theExtent.xMinimum() )
  {
    left = qRound(( myRasterExtent.xMinimum() - theExtent.xMinimum() ) / xRes );
  }
  if ( myRasterExtent.xMaximum() < theExtent.xMaximum() )
  {
    right = qRound(( myRasterExtent.xMaximum() - theExtent.xMinimum() ) / xRes ) - 1;
  }
#endif
  QRect subRect = QgsRasterBlock::subRect( theExtent, thePixelWidth, thePixelHeight, myRasterExtent );
  int top = subRect.top();
  int bottom = subRect.bottom();
  int left = subRect.left();
  int right = subRect.right();
  QgsDebugMsg( QString( "top = %1 bottom = %2 left = %3 right = %4" ).arg( top ).arg( bottom ).arg( left ).arg( right ) );


  // We want to avoid another resampling, so we read data approximately with
  // the same resolution as requested and exactly the width/height we need.

  // Calculate rows/cols limits in raster grid space

  // Set readable names
  double srcXRes = mGeoTransform[1];
  double srcYRes = mGeoTransform[5]; // may be negative?
  QgsDebugMsg( QString( "xRes = %1 yRes = %2 srcXRes = %3 srcYRes = %4" ).arg( xRes ).arg( yRes ).arg( srcXRes ).arg( srcYRes ) );

  // target size in pizels
  int width = right - left + 1;
  int height = bottom - top + 1;

  int srcLeft = 0; // source raster x offset
  int srcTop = 0; // source raster x offset
  int srcBottom = ySize() - 1;
  int srcRight = xSize() - 1;

  // Note: original approach for xRes < srcXRes || yRes < qAbs( srcYRes ) was to avoid
  // second resampling and read with GDALRasterIO to another temporary data block
  // extended to fit src grid. The problem was that with src resolution much bigger
  // than dst res, the target could become very large
  // in theory it was going to infinity when zooming in ...

  // Note: original approach for xRes > srcXRes, yRes > srcYRes was to read directly with GDALRasterIO
  // but we would face this problem:
  // If the edge of the source is greater than the edge of destination:
  // src:        | | | | | | | | |
  // dst:     |      |     |     |
  // We have 2 options for resampling:
  //  a) 'Stretch' the src and align the start edge of src to the start edge of dst.
  //     That means however, that to the target cells may be assigned values of source
  //     which are not nearest to the center of dst cells. Usualy probably not a problem
  //     but we are not precise. The shift is in maximum ... TODO
  //  b) We could cut the first destination column and left only the second one which is
  //     completely covered by src. No (significant) stretching is applied in that
  //     case, but the first column may be rendered as without values event if its center
  //     is covered by src column. That could result in wrongly rendered (missing) edges
  //     which could be easily noticed by user

  // Because of problems mentioned above we read to another temporary block and do i
  // another resampling here which appeares to be quite fast

  // Get necessary src extent aligned to src resolution
  if ( mExtent.xMinimum() < myRasterExtent.xMinimum() )
  {
    srcLeft = static_cast<int>( floor(( myRasterExtent.xMinimum() - mExtent.xMinimum() ) / srcXRes ) );
  }
  if ( mExtent.xMaximum() > myRasterExtent.xMaximum() )
  {
    srcRight = static_cast<int>( floor(( myRasterExtent.xMaximum() - mExtent.xMinimum() ) / srcXRes ) );
  }

  // GDAL states that mGeoTransform[3] is top, may it also be bottom and mGeoTransform[5] positive?
  if ( mExtent.yMaximum() > myRasterExtent.yMaximum() )
  {
    srcTop = static_cast<int>( floor( -1. * ( mExtent.yMaximum() - myRasterExtent.yMaximum() ) / srcYRes ) );
  }
  if ( mExtent.yMinimum() < myRasterExtent.yMinimum() )
  {
    srcBottom = static_cast<int>( floor( -1. * ( mExtent.yMaximum() - myRasterExtent.yMinimum() ) / srcYRes ) );
  }

  QgsDebugMsg( QString( "srcTop = %1 srcBottom = %2 srcLeft = %3 srcRight = %4" ).arg( srcTop ).arg( srcBottom ).arg( srcLeft ).arg( srcRight ) );

  int srcWidth = srcRight - srcLeft + 1;
  int srcHeight = srcBottom - srcTop + 1;

  QgsDebugMsg( QString( "width = %1 height = %2 srcWidth = %3 srcHeight = %4" ).arg( width ).arg( height ).arg( srcWidth ).arg( srcHeight ) );

  int tmpWidth = srcWidth;
  int tmpHeight = srcHeight;

  if ( xRes > srcXRes )
  {
    tmpWidth = static_cast<int>( qRound( srcWidth * srcXRes / xRes ) );
  }
  if ( yRes > fabs( srcYRes ) )
  {
    tmpHeight = static_cast<int>( qRound( -1.*srcHeight * srcYRes / yRes ) );
  }

  double tmpXMin = mExtent.xMinimum() + srcLeft * srcXRes;
  double tmpYMax = mExtent.yMaximum() + srcTop * srcYRes;
  QgsDebugMsg( QString( "tmpXMin = %1 tmpYMax = %2 tmpWidth = %3 tmpHeight = %4" ).arg( tmpXMin ).arg( tmpYMax ).arg( tmpWidth ).arg( tmpHeight ) );

  // Allocate temporary block
  char *tmpBlock = ( char * )qgsMalloc( dataSize * tmpWidth * tmpHeight );
  if ( ! tmpBlock )
  {
    QgsDebugMsg( QString( "Coudn't allocate temporary buffer of %1 bytes" ).arg( dataSize * tmpWidth * tmpHeight ) );
    return;
  }
  GDALRasterBandH gdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  GDALDataType type = ( GDALDataType )mGdalDataType.at( theBandNo - 1 );
  CPLErrorReset();
  CPLErr err = gdalRasterIO( gdalBand, GF_Read,
                             srcLeft, srcTop, srcWidth, srcHeight,
                             ( void * )tmpBlock,
                             tmpWidth, tmpHeight, type,
                             0, 0 );

  if ( err != CPLE_None )
  {
    QgsLogger::warning( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
    qgsFree( tmpBlock );
    return;
  }

  double tmpXRes = srcWidth * srcXRes / tmpWidth;
  double tmpYRes = srcHeight * srcYRes / tmpHeight; // negative

  double y = myRasterExtent.yMaximum() - 0.5 * yRes;
  for ( int row = 0; row < height; row++ )
  {
    int tmpRow = static_cast<int>( floor( -1. * ( tmpYMax - y ) / tmpYRes ) );

    char *srcRowBlock = tmpBlock + dataSize * tmpRow * tmpWidth;
    char *dstRowBlock = ( char * )theBlock + dataSize * ( top + row ) * thePixelWidth;

    double x = ( myRasterExtent.xMinimum() + 0.5 * xRes - tmpXMin ) / tmpXRes; // cell center
    double increment = xRes / tmpXRes;

    char* dst = dstRowBlock + dataSize * left;
    char* src = srcRowBlock;
    int tmpCol = 0;
    int lastCol = 0;
    for ( int col = 0; col < width; ++col )
    {
      // floor() is quite slow! Use just cast to int.
      tmpCol = static_cast<int>( x );
      if ( tmpCol > lastCol )
      {
        src += ( tmpCol - lastCol ) * dataSize;
        lastCol = tmpCol;
      }
      memcpy( dst, src, dataSize );
      dst += dataSize;
      x += increment;
    }
    y -= yRes;
  }

  qgsFree( tmpBlock );
  return;
}

//void * QgsGdalProvider::readBlock( int bandNo, QgsRectangle  const & extent, int width, int height )
//{
//  return 0;
//}

// this is old version which was using GDALWarpOperation, unfortunately
// it may be very slow on large datasets
#if 0
void QgsGdalProvider::readBlock( int theBandNo, QgsRectangle  const & theExtent, int thePixelWidth, int thePixelHeight, void *theBlock )
{
  QgsDebugMsg( "thePixelWidth = "  + QString::number( thePixelWidth ) );
  QgsDebugMsg( "thePixelHeight = "  + QString::number( thePixelHeight ) );
  QgsDebugMsg( "theExtent: " + theExtent.toString() );

  QString myMemDsn;
  myMemDsn.sprintf( "DATAPOINTER = %p", theBlock );
  QgsDebugMsg( myMemDsn );

  //myMemDsn.sprintf( "MEM:::DATAPOINTER=%lu,PIXELS=%d,LINES=%d,BANDS=1,DATATYPE=%s,PIXELOFFSET=0,LINEOFFSET=0,BANDOFFSET=0", ( long )theBlock, thePixelWidth, thePixelHeight, GDALGetDataTypeName(( GDALDataType )mGdalDataType[theBandNo-1] ) );
  char szPointer[64];
  memset( szPointer, 0, sizeof( szPointer ) );
  CPLPrintPointer( szPointer, theBlock, sizeof( szPointer ) );

  myMemDsn.sprintf( "MEM:::DATAPOINTER=%s,PIXELS=%d,LINES=%d,BANDS=1,DATATYPE=%s,PIXELOFFSET=0,LINEOFFSET=0,BANDOFFSET=0", szPointer, thePixelWidth, thePixelHeight, GDALGetDataTypeName(( GDALDataType )mGdalDataType[theBandNo-1] ) );

  QgsDebugMsg( "Open GDAL MEM : " + myMemDsn );

  CPLErrorReset();
  GDALDatasetH myGdalMemDataset = GDALOpen( TO8F( myMemDsn ), GA_Update );

  if ( !myGdalMemDataset )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QObject::tr( "Cannot open GDAL MEM dataset %1: %2" ).arg( myMemDsn ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    return;
  }

  //GDALSetProjection( myGdalMemDataset, theDestCRS.toWkt().toAscii().constData() );

  double myMemGeoTransform[6];
  myMemGeoTransform[0] = theExtent.xMinimum(); // top left x
  myMemGeoTransform[1] = theExtent.width() / thePixelWidth; // w-e pixel resolution
  myMemGeoTransform[2] = 0; // rotation, 0 if image is "north up"
  myMemGeoTransform[3] = theExtent.yMaximum(); // top left y
  myMemGeoTransform[4] = 0; // rotation, 0 if image is "north up"
  myMemGeoTransform[5] = -1. *  theExtent.height() / thePixelHeight; // n-s pixel resolution

  double myGeoTransform[6];
  GDALGetGeoTransform( mGdalDataset, myGeoTransform );
  // TODO:
  // Attention: GDALCreateGenImgProjTransformer failes if source data source
  // is not georeferenced, e.g. matrix 0,1,0,0,0,1/-1
  // as a workaround in such case we have to set some different value - really ugly
  myGeoTransform[0] = DBL_MIN;
  GDALSetGeoTransform( mGdalDataset, myGeoTransform );

  GDALSetGeoTransform( myGdalMemDataset, myMemGeoTransform );

  for ( int i = 0 ; i < 6; i++ )
  {
    QgsDebugMsg( QString( "transform : %1 %2" ).arg( myGeoTransform[i] ).arg( myMemGeoTransform[i] ) );
  }

  GDALWarpOptions *myWarpOptions = GDALCreateWarpOptions();

  myWarpOptions->hSrcDS = mGdalDataset;
  myWarpOptions->hDstDS = myGdalMemDataset;

  myWarpOptions->nBandCount = 1;
  myWarpOptions->panSrcBands =
    ( int * ) qgsMalloc( sizeof( int ) * myWarpOptions->nBandCount );
  myWarpOptions->panSrcBands[0] = theBandNo;
  myWarpOptions->panDstBands =
    ( int * ) qgsMalloc( sizeof( int ) * myWarpOptions->nBandCount );
  myWarpOptions->panDstBands[0] = 1;

  // TODO move here progressCallback and use it
  myWarpOptions->pfnProgress = GDALTermProgress;

  QgsDebugMsg( "src wkt: " +  QString( GDALGetProjectionRef( mGdalDataset ) ) );
  QgsDebugMsg( "dst wkt: " +  QString( GDALGetProjectionRef( myGdalMemDataset ) ) );
  myWarpOptions->pTransformerArg =
    GDALCreateGenImgProjTransformer(
      mGdalDataset,
      nullptr,
      myGdalMemDataset,
      nullptr,
      false, 0.0, 1
    );
#if 0
  myWarpOptions->pTransformerArg =
    GDALCreateGenImgProjTransformer2(
      mGdalDataset,
      myGdalMemDataset,
      nullptr
    );
#endif
  if ( !myWarpOptions->pTransformerArg )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QObject::tr( "Cannot GDALCreateGenImgProjTransformer: " )
                          + QString::fromUtf8( CPLGetLastErrorMsg() ) );
    return;

  }

  //CPLAssert( myWarpOptions->pTransformerArg );
  myWarpOptions->pfnTransformer = GDALGenImgProjTransform;

  myWarpOptions->padfDstNoDataReal = ( double * ) qgsMalloc( myWarpOptions->nBandCount * sizeof( double ) );
  myWarpOptions->padfDstNoDataImag = ( double * ) qgsMalloc( myWarpOptions->nBandCount * sizeof( double ) );

  myWarpOptions->padfDstNoDataReal[0] = mNoDataValue[theBandNo-1];
  myWarpOptions->padfDstNoDataImag[0] = 0.0;

  GDALSetRasterNoDataValue( GDALGetRasterBand( myGdalMemDataset,
                            myWarpOptions->panDstBands[0] ),
                            myWarpOptions->padfDstNoDataReal[0] );

  // TODO optimize somehow to avoid no data init if not necessary
  // i.e. no projection, but there is also the problem with margine
  myWarpOptions->papszWarpOptions =
    CSLSetNameValue( myWarpOptions->papszWarpOptions, "INIT_DEST", "NO_DATA" );

  myWarpOptions->eResampleAlg = GRA_NearestNeighbour;

  GDALWarpOperation myOperation;

  if ( myOperation.Initialize( myWarpOptions ) != CE_None )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QObject::tr( "Cannot inittialize GDALWarpOperation : " )
                          + QString::fromUtf8( CPLGetLastErrorMsg() ) );
    return;

  }
  CPLErrorReset();
  CPLErr myErr;
  myErr = myOperation.ChunkAndWarpImage( 0, 0, thePixelWidth, thePixelHeight );
  if ( myErr != CPLE_None )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QObject::tr( "Cannot ChunkAndWarpImage: %1" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    return;
  }

  GDALDestroyGenImgProjTransformer( myWarpOptions->pTransformerArg );
  GDALDestroyWarpOptions( myWarpOptions );

  // flush should not be necessary
  //GDALFlushCache  (  myGdalMemDataset );
  // this was causing crash ???
  // The MEM driver does not free() the memory passed as DATAPOINTER so we can closee the dataset
  GDALClose( myGdalMemDataset );

}
#endif

#if 0
bool QgsGdalProvider::srcHasNoDataValue( int bandNo ) const
{
  if ( mGdalDataset )
  {
    GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, bandNo );
    if ( myGdalBand )
    {
      int ok;
      GDALGetRasterNoDataValue( myGdalBand, &ok );
      return ok;
    }
  }
  return false;
}

double  QgsGdalProvider::noDataValue() const
{
  if ( !mNoDataValue.isEmpty() )
  {
    return mNoDataValue[0];
  }
  return std::numeric_limits<int>::max(); // should not happen or be used
}
#endif

#if 0
void QgsGdalProvider::computeMinMax( int theBandNo ) const
{
  QgsDebugMsg( QString( "theBandNo = %1 mMinMaxComputed = %2" ).arg( theBandNo ).arg( mMinMaxComputed[theBandNo-1] ) );
  if ( mMinMaxComputed[theBandNo-1] )
  {
    return;
  }
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  int             bGotMin, bGotMax;
  double          adfMinMax[2];
  adfMinMax[0] = GDALGetRasterMinimum( myGdalBand, &bGotMin );
  adfMinMax[1] = GDALGetRasterMaximum( myGdalBand, &bGotMax );
  if ( !( bGotMin && bGotMax ) )
  {
    GDALComputeRasterMinMax( myGdalBand, true, adfMinMax );
  }
  mMinimum[theBandNo-1] = adfMinMax[0];
  mMaximum[theBandNo-1] = adfMinMax[1];
}
#endif

/**
 * @param theBandNumber the number of the band for which you want a color table
 * @param theList a pointer the object that will hold the color table
 * @return true of a color table was able to be read, false otherwise
 */
QList<QgsColorRampShader::ColorRampItem> QgsGdalProvider::colorTable( int theBandNumber )const
{
  return QgsGdalProviderBase::colorTable( mGdalDataset, theBandNumber );
}

QgsCoordinateReferenceSystem QgsGdalProvider::crs()
{
  return mCrs;
}

QgsRectangle QgsGdalProvider::extent()
{
  //TODO
  //mExtent = QgsGdal::extent( mGisdbase, mLocation, mMapset, mMapName, QgsGdal::Raster );
  return mExtent;
}

// this is only called once when statistics are calculated
// TODO
int QgsGdalProvider::xBlockSize() const
{
  return mXBlockSize;
}
int QgsGdalProvider::yBlockSize() const
{
  return mYBlockSize;
}

int QgsGdalProvider::xSize() const { return mWidth; }
int QgsGdalProvider::ySize() const { return mHeight; }

QString QgsGdalProvider::generateBandName( int theBandNumber ) const
{
#ifdef GDAL_COMPUTE_VERSION /* only available in GDAL 1.10 or later */
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,10,0)
  if ( strcmp( GDALGetDriverShortName( GDALGetDatasetDriver( mGdalDataset ) ), "netCDF" ) == 0 )
  {
    char ** GDALmetadata = GDALGetMetadata( mGdalDataset, nullptr );

    if ( GDALmetadata )
    {
      QStringList metadata = cStringList2Q_( GDALmetadata );
      QStringList dimExtraValues;
      QMap< QString, QString > unitsMap;
      for ( QStringList::const_iterator i = metadata.begin();
            i != metadata.end(); ++i )
      {
        QString val( *i );
        if ( !val.startsWith( "NETCDF_DIM_EXTRA" ) && !val.contains( "#units=" ) )
          continue;
        QStringList values = val.split( '=' );
        val = values.at( 1 );
        if ( values.at( 0 ) == "NETCDF_DIM_EXTRA" )
        {
          dimExtraValues = val.replace( QString( "{" ), QString() ).replace( QString( "}" ), QString() ).split( ',' );
          //http://qt-project.org/doc/qt-4.8/qregexp.html#capturedTexts
        }
        else
        {
          unitsMap[ values.at( 0 ).split( '#' ).at( 0 )] = val;
        }
      }
      if ( !dimExtraValues.isEmpty() )
      {
        QStringList bandNameValues;
        GDALRasterBandH gdalBand = GDALGetRasterBand( mGdalDataset, theBandNumber );
        GDALmetadata = GDALGetMetadata( gdalBand, nullptr );

        if ( GDALmetadata )
        {
          metadata = cStringList2Q_( GDALmetadata );
          for ( QStringList::const_iterator i = metadata.begin();
                i != metadata.end(); ++i )
          {
            QString val( *i );
            if ( !val.startsWith( "NETCDF_DIM_" ) )
              continue;
            QStringList values = val.split( '=' );
            for ( QStringList::const_iterator j = dimExtraValues.begin();
                  j != dimExtraValues.end(); ++j )
            {
              QString dim = ( *j );
              if ( values.at( 0 ) != "NETCDF_DIM_" + dim )
                continue;
              if ( unitsMap.contains( dim ) && unitsMap[ dim ] != "" && unitsMap[ dim ] != "none" )
                bandNameValues.append( dim + '=' + values.at( 1 ) + " (" + unitsMap[ dim ] + ')' );
              else
                bandNameValues.append( dim + '=' + values.at( 1 ) );
            }
          }
        }

        if ( !bandNameValues.isEmpty() )
          return tr( "Band" ) + QString( " %1 / %2" ) .arg( theBandNumber, 1 + ( int ) log10(( float ) bandCount() ), 10, QChar( '0' ) ).arg( bandNameValues.join( " / " ) );
      }
    }
  }
#endif
#endif

  return QgsRasterDataProvider::generateBandName( theBandNumber );
}

QgsRasterIdentifyResult QgsGdalProvider::identify( const QgsPoint & thePoint, QgsRaster::IdentifyFormat theFormat, const QgsRectangle &theExtent, int theWidth, int theHeight , int /*theDpi*/ )
{
  QgsDebugMsg( QString( "thePoint =  %1 %2" ).arg( thePoint.x(), 0, 'g', 10 ).arg( thePoint.y(), 0, 'g', 10 ) );

  QMap<int, QVariant> results;

  if ( theFormat != QgsRaster::IdentifyFormatValue )
  {
    return QgsRasterIdentifyResult( ERR( tr( "Format not supported" ) ) );
  }

  if ( !extent().contains( thePoint ) )
  {
    // Outside the raster
    for ( int bandNo = 1; bandNo <= bandCount(); bandNo++ )
    {
      results.insert( bandNo, QVariant() ); // null QVariant represents no data
    }
    return QgsRasterIdentifyResult( QgsRaster::IdentifyFormatValue, results );
  }

  QgsRectangle myExtent = theExtent;
  if ( myExtent.isEmpty() )  myExtent = extent();

  QgsDebugMsg( "myExtent = " + myExtent.toString() );

  if ( theWidth == 0 ) theWidth = xSize();
  if ( theHeight == 0 ) theHeight = ySize();

  QgsDebugMsg( QString( "theWidth = %1 theHeight = %2" ).arg( theWidth ).arg( theHeight ) );

  // Calculate the row / column where the point falls
  double xres = ( myExtent.width() ) / theWidth;
  double yres = ( myExtent.height() ) / theHeight;

  // Offset, not the cell index -> floor
  int col = ( int ) floor(( thePoint.x() - myExtent.xMinimum() ) / xres );
  int row = ( int ) floor(( myExtent.yMaximum() - thePoint.y() ) / yres );

  QgsDebugMsg( QString( "row = %1 col = %2" ).arg( row ).arg( col ) );

  // QgsDebugMsg( "row = " + QString::number( row ) + " col = " + QString::number( col ) );

  int r = 0;
  int c = 0;
  int width = 1;
  int height = 1;

  // GDAL ECW driver in GDAL <  1.9.2 read whole row if single pixel (nYSize == 1)
  // was requested which made identify very slow -> use 2x2 matrix
  // but other drivers may be optimised for 1x1 -> conditional
#if !defined(GDAL_VERSION_NUM) || GDAL_VERSION_NUM < 1920
  if ( strcmp( GDALGetDriverShortName( GDALGetDatasetDriver( mGdalDataset ) ), "ECW" ) == 0 )
  {
    width = 2;
    height = 2;
    if ( col == mWidth - 1 && mWidth > 1 )
    {
      col--;
      c++;
    }
    if ( row == mHeight - 1 && mHeight > 1 )
    {
      row--;
      r++;
    }
  }
#endif

  double xMin = myExtent.xMinimum() + col * xres;
  double xMax = xMin + xres * width;
  double yMax = myExtent.yMaximum() - row * yres;
  double yMin = yMax - yres * height;
  QgsRectangle pixelExtent( xMin, yMin, xMax, yMax );

  for ( int i = 1; i <= bandCount(); i++ )
  {
    QgsRasterBlock * myBlock = block( i, pixelExtent, width, height );

    if ( !myBlock )
    {
      return QgsRasterIdentifyResult( ERR( tr( "Cannot read data" ) ) );
    }

    double value = myBlock->value( r, c );

    if (( srcHasNoDataValue( i ) && useSrcNoDataValue( i ) &&
          ( qIsNaN( value ) || qgsDoubleNear( value, srcNoDataValue( i ) ) ) ) ||
        ( QgsRasterRange::contains( value, userNoDataValues( i ) ) ) )
    {
      results.insert( i, QVariant() ); // null QVariant represents no data
    }
    else
    {
      if ( srcDataType( i ) == QGis::Float32 )
      {
        // Insert a float QVariant so that QgsMapToolIdentify::identifyRasterLayer()
        // can print a string without an excessive precision
        results.insert( i, static_cast<float>( value ) );
      }
      else
        results.insert( i, value );
    }
    delete myBlock;
  }
  return QgsRasterIdentifyResult( QgsRaster::IdentifyFormatValue, results );
}

int QgsGdalProvider::capabilities() const
{
  int capability = QgsRasterDataProvider::Identify
                   | QgsRasterDataProvider::IdentifyValue
                   | QgsRasterDataProvider::Size
                   | QgsRasterDataProvider::BuildPyramids
                   | QgsRasterDataProvider::Create
                   | QgsRasterDataProvider::Remove;
  GDALDriverH myDriver = GDALGetDatasetDriver( mGdalDataset );
  QString name = GDALGetDriverShortName( myDriver );
  QgsDebugMsg( "driver short name = " + name );
  if ( name != "WMS" )
  {
    capability |= QgsRasterDataProvider::Size;
  }
  return capability;
}

QGis::DataType QgsGdalProvider::srcDataType( int bandNo ) const
{
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, bandNo );
  GDALDataType myGdalDataType = GDALGetRasterDataType( myGdalBand );
  QGis::DataType myDataType = dataTypeFromGdal( myGdalDataType );

  // define if the band has scale and offset to apply
  double myScale = bandScale( bandNo );
  double myOffset = bandOffset( bandNo );
  if ( myScale != 1.0 || myOffset != 0.0 )
  {
    // if the band has scale or offset to apply change dataType
    switch ( myDataType )
    {
      case QGis::UnknownDataType:
      case QGis::ARGB32:
      case QGis::ARGB32_Premultiplied:
        return myDataType;
      case QGis::Byte:
      case QGis::UInt16:
      case QGis::Int16:
      case QGis::UInt32:
      case QGis::Int32:
      case QGis::Float32:
      case QGis::CInt16:
        myDataType = QGis::Float32;
        break;
      case QGis::Float64:
      case QGis::CInt32:
      case QGis::CFloat32:
        myDataType = QGis::Float64;
        break;
      case QGis::CFloat64:
        return myDataType;
    }
  }
  return myDataType;
}

QGis::DataType QgsGdalProvider::dataType( int bandNo ) const
{
  if ( bandNo <= 0 || bandNo > mGdalDataType.count() ) return QGis::UnknownDataType;

  return dataTypeFromGdal( mGdalDataType[bandNo-1] );
}

double QgsGdalProvider::bandScale( int bandNo ) const
{
#if GDAL_VERSION_NUM >= 1800
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, bandNo );
  int bGotScale;
  double myScale = GDALGetRasterScale( myGdalBand, &bGotScale );
  if ( bGotScale )
    return myScale;
  else
    return 1.0;
#else
  Q_UNUSED( bandNo );
  return 1.0;
#endif
}

double QgsGdalProvider::bandOffset( int bandNo ) const
{
#if GDAL_VERSION_NUM >= 1800
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, bandNo );
  int bGotOffset;
  double myOffset = GDALGetRasterOffset( myGdalBand, &bGotOffset );
  if ( bGotOffset )
    return myOffset;
  else
    return 0.0;
#else
  Q_UNUSED( bandNo );
  return 0.0;
#endif
}

int QgsGdalProvider::bandCount() const
{
  if ( mGdalDataset )
    return GDALGetRasterCount( mGdalDataset );
  else
    return 1;
}

int QgsGdalProvider::colorInterpretation( int theBandNo ) const
{
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  return colorInterpretationFromGdal( GDALGetRasterColorInterpretation( myGdalBand ) );
}

bool QgsGdalProvider::isValid()
{
  QgsDebugMsg( QString( "valid = %1" ).arg( mValid ) );
  return mValid;
}

QString QgsGdalProvider::lastErrorTitle()
{
  return QString( "Not implemented" );
}

QString QgsGdalProvider::lastError()
{
  return QString( "Not implemented" );
}

QString QgsGdalProvider::name() const
{
  return PROVIDER_KEY;
}

QString QgsGdalProvider::description() const
{
  return PROVIDER_DESCRIPTION;
}

// This is used also by global isValidRasterFileName
QStringList QgsGdalProvider::subLayers( GDALDatasetH dataset )
{
  QStringList subLayers;

  if ( !dataset )
  {
    QgsDebugMsg( "dataset is nullptr" );
    return subLayers;
  }

  char **metadata = GDALGetMetadata( dataset, "SUBDATASETS" );

  if ( metadata )
  {
    for ( int i = 0; metadata[i]; i++ )
    {
      QString layer = QString::fromUtf8( metadata[i] );
      int pos = layer.indexOf( "_NAME=" );
      if ( pos >= 0 )
      {
        subLayers << layer.mid( pos + 6 );
      }
    }
  }

  if ( !subLayers.isEmpty() )
  {
    QgsDebugMsg( "sublayers:\n  " + subLayers.join( "\n  " ) );
  }

  return subLayers;
}

bool QgsGdalProvider::hasHistogram( int theBandNo,
                                    int theBinCount,
                                    double theMinimum, double theMaximum,
                                    const QgsRectangle & theExtent,
                                    int theSampleSize,
                                    bool theIncludeOutOfRange )
{
  QgsDebugMsg( QString( "theBandNo = %1 theBinCount = %2 theMinimum = %3 theMaximum = %4 theSampleSize = %5" ).arg( theBandNo ).arg( theBinCount ).arg( theMinimum ).arg( theMaximum ).arg( theSampleSize ) );

  // First check if cached in mHistograms
  if ( QgsRasterDataProvider::hasHistogram( theBandNo, theBinCount, theMinimum, theMaximum, theExtent, theSampleSize, theIncludeOutOfRange ) )
  {
    return true;
  }

  QgsRasterHistogram myHistogram;
  initHistogram( myHistogram, theBandNo, theBinCount, theMinimum, theMaximum, theExtent, theSampleSize, theIncludeOutOfRange );

  // If not cached, check if supported by GDAL
  if ( myHistogram.extent != extent() )
  {
    QgsDebugMsg( "Not supported by GDAL." );
    return false;
  }

  if (( srcHasNoDataValue( theBandNo ) && !useSrcNoDataValue( theBandNo ) ) ||
      !userNoDataValues( theBandNo ).isEmpty() )
  {
    QgsDebugMsg( "Custom no data values -> GDAL histogram not sufficient." );
    return false;
  }

  QgsDebugMsg( "Looking for GDAL histogram" );

  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  if ( ! myGdalBand )
  {
    return false;
  }

  // get default histogram with force=false to see if there is a cached histo
  double myMinVal, myMaxVal;
  int myBinCount;

#if GDAL_VERSION_MAJOR >= 2
  GUIntBig* myHistogramArray = 0;
  CPLErr myError = GDALGetDefaultHistogramEx( myGdalBand, &myMinVal, &myMaxVal,
                   &myBinCount, &myHistogramArray, false,
                   nullptr, nullptr );
#else
  int* myHistogramArray = nullptr;

  // TODO: GDALGetDefaultHistogram has no bIncludeOutOfRange and bApproxOK,
  //       consider consequences
  CPLErr myError = GDALGetDefaultHistogram( myGdalBand, &myMinVal, &myMaxVal,
                   &myBinCount, &myHistogramArray, false,
                   nullptr, nullptr );
#endif

  if ( myHistogramArray )
    VSIFree( myHistogramArray ); // use VSIFree because allocated by GDAL

  // if there was any error/warning assume the histogram is not valid or non-existent
  if ( myError != CE_None )
  {
    QgsDebugMsg( "Cannot get default GDAL histogram" );
    return false;
  }

  // This is fragile
  double myExpectedMinVal = myHistogram.minimum;
  double myExpectedMaxVal = myHistogram.maximum;

  double dfHalfBucket = ( myExpectedMaxVal - myExpectedMinVal ) / ( 2 * myHistogram.binCount );
  myExpectedMinVal -= dfHalfBucket;
  myExpectedMaxVal += dfHalfBucket;

  // min/max are stored as text in aux file => use threshold
  if ( myBinCount != myHistogram.binCount ||
       qAbs( myMinVal - myExpectedMinVal ) > qAbs( myExpectedMinVal ) / 10e6 ||
       qAbs( myMaxVal - myExpectedMaxVal ) > qAbs( myExpectedMaxVal ) / 10e6 )
  {
    QgsDebugMsg( QString( "Params do not match binCount: %1 x %2, minVal: %3 x %4, maxVal: %5 x %6" ).arg( myBinCount ).arg( myHistogram.binCount ).arg( myMinVal ).arg( myExpectedMinVal ).arg( myMaxVal ).arg( myExpectedMaxVal ) );
    return false;
  }

  QgsDebugMsg( "GDAL has cached histogram" );

  // This should be enough, possible call to histogram() should retrieve the histogram cached in GDAL

  return true;
}

QgsRasterHistogram QgsGdalProvider::histogram( int theBandNo,
    int theBinCount,
    double theMinimum, double theMaximum,
    const QgsRectangle & theExtent,
    int theSampleSize,
    bool theIncludeOutOfRange )
{
  QgsDebugMsg( QString( "theBandNo = %1 theBinCount = %2 theMinimum = %3 theMaximum = %4 theSampleSize = %5" ).arg( theBandNo ).arg( theBinCount ).arg( theMinimum ).arg( theMaximum ).arg( theSampleSize ) );

  QgsRasterHistogram myHistogram;
  initHistogram( myHistogram, theBandNo, theBinCount, theMinimum, theMaximum, theExtent, theSampleSize, theIncludeOutOfRange );

  // Find cached
  Q_FOREACH ( const QgsRasterHistogram& histogram, mHistograms )
  {
    if ( histogram == myHistogram )
    {
      QgsDebugMsg( "Using cached histogram." );
      return histogram;
    }
  }

  if (( srcHasNoDataValue( theBandNo ) && !useSrcNoDataValue( theBandNo ) ) ||
      !userNoDataValues( theBandNo ).isEmpty() )
  {
    QgsDebugMsg( "Custom no data values, using generic histogram." );
    return QgsRasterDataProvider::histogram( theBandNo, theBinCount, theMinimum, theMaximum, theExtent, theSampleSize, theIncludeOutOfRange );
  }

  if ( myHistogram.extent != extent() )
  {
    QgsDebugMsg( "Not full extent, using generic histogram." );
    return QgsRasterDataProvider::histogram( theBandNo, theBinCount, theMinimum, theMaximum, theExtent, theSampleSize, theIncludeOutOfRange );
  }

  QgsDebugMsg( "Computing GDAL histogram" );

  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );

  int bApproxOK = false;
  if ( theSampleSize > 0 )
  {
    // cast to double, integer could overflow
    if ((( double )xSize() * ( double )ySize() / theSampleSize ) > 2 )  // not perfect
    {
      QgsDebugMsg( "Approx" );
      bApproxOK = true;
    }
  }

  QgsDebugMsg( QString( "xSize() = %1 ySize() = %2 theSampleSize = %3 bApproxOK = %4" ).arg( xSize() ).arg( ySize() ).arg( theSampleSize ).arg( bApproxOK ) );

  QgsGdalProgress myProg;
  myProg.type = QgsRaster::ProgressHistogram;
  myProg.provider = this;

#if 0 // this is the old method

  double myerval = ( theBandStats.maximumValue - theBandStats.minimumValue ) / theBinCount;
  GDALGetRasterHistogram( myGdalBand, theBandStats.minimumValue - 0.1*myerval,
                          theBandStats.maximumValue + 0.1*myerval, theBinCount, myHistogramArray,
                          theIgnoreOutOfRangeFlag, theHistogramEstimatedFlag, progressCallback,
                          &myProg ); //this is the arg for our custom gdal progress callback

#else // this is the new method, which gets a "Default" histogram

  // calculate min/max like in GDALRasterBand::GetDefaultHistogram, but don't call it directly
  // because there is no bApproxOK argument - that is lacking from the API

  // Min/max, if not specified, are set by histogramDefaults, it does not
  // set however min/max shifted to avoid rounding errors

  double myMinVal = myHistogram.minimum;
  double myMaxVal = myHistogram.maximum;

  // unapply scale anf offset for min and max
  double myScale = bandScale( theBandNo );
  double myOffset = bandOffset( theBandNo );
  if ( myScale != 1.0 || myOffset != 0. )
  {
    myMinVal = ( myHistogram.minimum - myOffset ) / myScale;
    myMaxVal = ( myHistogram.maximum - myOffset ) / myScale;
  }

  double dfHalfBucket = ( myMaxVal - myMinVal ) / ( 2 * myHistogram.binCount );
  myMinVal -= dfHalfBucket;
  myMaxVal += dfHalfBucket;

#if 0
  const char* pszPixelType = GDALGetMetadataItem( myGdalBand, "PIXELTYPE", "IMAGE_STRUCTURE" );
  int bSignedByte = ( pszPixelType && EQUAL( pszPixelType, "SIGNEDBYTE" ) );

  if ( GDALGetRasterDataType( myGdalBand ) == GDT_Byte && !bSignedByte )
  {
    myMinVal = -0.5;
    myMaxVal = 255.5;
  }
  else
  {
    CPLErr eErr = CE_Failure;
    double dfHalfBucket = 0;
    eErr = GDALGetRasterStatistics( myGdalBand, true, true, &myMinVal, &myMaxVal, nullptr, nullptr );
    if ( eErr != CE_None )
    {
      delete [] myHistogramArray;
      return;
    }
    dfHalfBucket = ( myMaxVal - myMinVal ) / ( 2 * theBinCount );
    myMinVal -= dfHalfBucket;
    myMaxVal += dfHalfBucket;
  }
#endif

#if GDAL_VERSION_MAJOR >= 2
  GUIntBig* myHistogramArray = new GUIntBig[myHistogram.binCount];
  CPLErr myError = GDALGetRasterHistogramEx( myGdalBand, myMinVal, myMaxVal,
                   myHistogram.binCount, myHistogramArray,
                   theIncludeOutOfRange, bApproxOK, progressCallback,
                   &myProg ); //this is the arg for our custom gdal progress callback
#else
  int* myHistogramArray = new int[myHistogram.binCount];
  CPLErr myError = GDALGetRasterHistogram( myGdalBand, myMinVal, myMaxVal,
                   myHistogram.binCount, myHistogramArray,
                   theIncludeOutOfRange, bApproxOK, progressCallback,
                   &myProg ); //this is the arg for our custom gdal progress callback
#endif

  if ( myError != CE_None )
  {
    QgsDebugMsg( "Cannot get histogram" );
    delete [] myHistogramArray;
    return myHistogram;
  }

#endif

  for ( int myBin = 0; myBin < myHistogram.binCount; myBin++ )
  {
#if GDAL_VERSION_MAJOR < 2
    if ( myHistogramArray[myBin] < 0 ) //can't have less than 0 pixels of any value
    {
      myHistogram.histogramVector.push_back( 0 );
      // QgsDebugMsg( "Added 0 to histogram vector as freq was negative!" );
    }
    else
#endif
    {
      myHistogram.histogramVector.push_back( myHistogramArray[myBin] );
      myHistogram.nonNullCount += myHistogramArray[myBin];
      // QgsDebugMsg( "Added " + QString::number( myHistogramArray[myBin] ) + " to histogram vector" );
    }
  }

  myHistogram.valid = true;

  delete [] myHistogramArray;

  QgsDebugMsg( ">>>>> Histogram vector now contains " + QString::number( myHistogram.histogramVector.size() ) + " elements" );

  mHistograms.append( myHistogram );
  return myHistogram;
}

/*
 * This will speed up performance at the expense of hard drive space.
 * Also, write access to the file is required for creating internal pyramids,
 * and to the directory in which the files exists if external
 * pyramids (.ovr) are to be created. If no parameter is passed in
 * it will default to nearest neighbor resampling.
 *
 * @param theTryInternalFlag - Try to make the pyramids internal if supported (e.g. geotiff). If not supported it will revert to creating external .ovr file anyway.
 * @return null string on success, otherwise a string specifying error
 */
QString QgsGdalProvider::buildPyramids( const QList<QgsRasterPyramid> & theRasterPyramidList,
                                        const QString & theResamplingMethod, QgsRaster::RasterPyramidsFormat theFormat,
                                        const QStringList & theConfigOptions )
{
  //TODO: Consider making theRasterPyramidList modifyable by this method to indicate if the pyramid exists after build attempt
  //without requiring the user to rebuild the pyramid list to get the updated infomation

  //
  // Note: Make sure the raster is not opened in write mode
  // in order to force overviews to be written to a separate file.
  // Otherwise reoopen it in read/write mode to stick overviews
  // into the same file (if supported)
  //


  // TODO add signal and connect from rasterlayer
  //emit drawingProgress( 0, 0 );

  if ( mGdalDataset != mGdalBaseDataset )
  {
    QgsLogger::warning( "Pyramid building not currently supported for 'warped virtual dataset'." );
    return "ERROR_VIRTUAL";
  }

  // check if building internally
  if ( theFormat == QgsRaster::PyramidsInternal )
  {

    // test if the file is writable
    //QFileInfo myQFile( mDataSource );
    QFileInfo myQFile( dataSourceUri() );

    if ( !myQFile.isWritable() )
    {
      return "ERROR_WRITE_ACCESS";
    }

    // libtiff < 4.0 has a bug that prevents safe building of overviews on JPEG compressed files
    // we detect libtiff < 4.0 by checking that BIGTIFF is not in the creation options of the GTiff driver
    // see https://trac.osgeo.org/qgis/ticket/1357
    const char* pszGTiffCreationOptions =
      GDALGetMetadataItem( GDALGetDriverByName( "GTiff" ), GDAL_DMD_CREATIONOPTIONLIST, "" );
    if ( !strstr( pszGTiffCreationOptions, "BIGTIFF" ) )
    {
      QString myCompressionType = QString( GDALGetMetadataItem( mGdalDataset, "COMPRESSION", "IMAGE_STRUCTURE" ) );
      if ( "JPEG" == myCompressionType )
      {
        return "ERROR_JPEG_COMPRESSION";
      }
    }

    // if needed close the gdal dataset and reopen it in read / write mode
    // TODO this doesn't seem to work anymore... must fix it before 2.0!!!
    // no errors are reported, but pyramids are not present in file.
    if ( GDALGetAccess( mGdalDataset ) == GA_ReadOnly )
    {
      QgsDebugMsg( "re-opening the dataset in read/write mode" );
      GDALClose( mGdalDataset );
      //mGdalBaseDataset = GDALOpen( QFile::encodeName( dataSourceUri() ).constData(), GA_Update );

      mGdalBaseDataset = gdalOpen( TO8F( dataSourceUri() ), GA_Update );

      // if the dataset couldn't be opened in read / write mode, tell the user
      if ( !mGdalBaseDataset )
      {
        mGdalBaseDataset = gdalOpen( TO8F( dataSourceUri() ), GA_ReadOnly );
        //Since we are not a virtual warped dataset, mGdalDataSet and mGdalBaseDataset are supposed to be the same
        mGdalDataset = mGdalBaseDataset;
        return "ERROR_WRITE_FORMAT";
      }
    }
  }

  // are we using Erdas Imagine external overviews?
  QgsStringMap myConfigOptionsOld;
  myConfigOptionsOld[ "USE_RRD" ] = CPLGetConfigOption( "USE_RRD", "NO" );
  if ( theFormat == QgsRaster::PyramidsErdas )
    CPLSetConfigOption( "USE_RRD", "YES" );
  else
    CPLSetConfigOption( "USE_RRD", "NO" );

  // add any driver-specific configuration options, save values to be restored later
  if ( theFormat != QgsRaster::PyramidsErdas && ! theConfigOptions.isEmpty() )
  {
    Q_FOREACH ( const QString& option, theConfigOptions )
    {
      QStringList opt = option.split( '=' );
      if ( opt.size() == 2 )
      {
        QByteArray key = opt[0].toLocal8Bit();
        QByteArray value = opt[1].toLocal8Bit();
        // save previous value
        myConfigOptionsOld[ opt[0] ] = QString( CPLGetConfigOption( key.data(), nullptr ) );
        // set temp. value
        CPLSetConfigOption( key.data(), value.data() );
        QgsDebugMsg( QString( "set option %1=%2" ).arg( key.data(), value.data() ) );
      }
      else
      {
        QgsDebugMsg( QString( "invalid pyramid option: %1" ).arg( option ) );
      }
    }
  }

  //
  // Iterate through the Raster Layer Pyramid Vector, building any pyramid
  // marked as exists in each RasterPyramid struct.
  //
  CPLErr myError; //in case anything fails

  QVector<int> myOverviewLevelsVector;
  QList<QgsRasterPyramid>::const_iterator myRasterPyramidIterator;
  for ( myRasterPyramidIterator = theRasterPyramidList.begin();
        myRasterPyramidIterator != theRasterPyramidList.end();
        ++myRasterPyramidIterator )
  {
#ifdef QGISDEBUG
    QgsDebugMsg( QString( "Build pyramids:: Level %1" ).arg( myRasterPyramidIterator->level ) );
    QgsDebugMsg( QString( "x:%1" ).arg( myRasterPyramidIterator->xDim ) );
    QgsDebugMsg( QString( "y:%1" ).arg( myRasterPyramidIterator->yDim ) );
    QgsDebugMsg( QString( "exists : %1" ).arg( myRasterPyramidIterator->exists ) );
#endif
    if ( myRasterPyramidIterator->build )
    {
      QgsDebugMsg( QString( "adding overview at level %1 to list"
                          ).arg( myRasterPyramidIterator->level ) );
      myOverviewLevelsVector.append( myRasterPyramidIterator->level );
    }
  }
  /* From : http://www.gdal.org/classGDALDataset.html#a2aa6f88b3bbc840a5696236af11dde15
   * pszResampling : one of "NEAREST", "GAUSS", "CUBIC", "CUBICSPLINE" (GDAL >= 2.0),
   * "LANCZOS" ( GDAL >= 2.0), "AVERAGE", "MODE" or "NONE" controlling the downsampling method applied.
   * nOverviews : number of overviews to build.
   * panOverviewList : the list of overview decimation factors to build.
   * nListBands : number of bands to build overviews for in panBandList. Build for all bands if this is 0.
   * panBandList : list of band numbers.
   * pfnProgress : a function to call to report progress, or nullptr.
   * pProgressData : application data to pass to the progress function.
   */

  // resampling method is now passed directly, via QgsRasterDataProvider::pyramidResamplingArg()
  // average_mp and average_magphase have been removed from the gui
  QByteArray ba = theResamplingMethod.toLocal8Bit();
  const char *theMethod = ba.data();

  //build the pyramid and show progress to console
  QgsDebugMsg( QString( "Building overviews at %1 levels using resampling method %2"
                      ).arg( myOverviewLevelsVector.size() ).arg( theMethod ) );
  try
  {
    //build the pyramid and show progress to console
    QgsGdalProgress myProg;
    myProg.type = QgsRaster::ProgressPyramids;
    myProg.provider = this;
    myError = GDALBuildOverviews( mGdalBaseDataset, theMethod,
                                  myOverviewLevelsVector.size(), myOverviewLevelsVector.data(),
                                  0, nullptr,
                                  progressCallback, &myProg ); //this is the arg for the gdal progress callback

    if ( myError == CE_Failure || CPLGetLastErrorNo() == CPLE_NotSupported )
    {
      QgsDebugMsg( QString( "Building pyramids failed using resampling method [%1]" ).arg( theMethod ) );
      //something bad happenend
      //QString myString = QString (CPLGetLastError());
      GDALClose( mGdalBaseDataset );
      mGdalBaseDataset = gdalOpen( TO8F( dataSourceUri() ), mUpdate ? GA_Update : GA_ReadOnly );
      //Since we are not a virtual warped dataset, mGdalDataSet and mGdalBaseDataset are supposed to be the same
      mGdalDataset = mGdalBaseDataset;

      //emit drawingProgress( 0, 0 );

      // restore former configOptions
      for ( QgsStringMap::const_iterator it = myConfigOptionsOld.begin();
            it != myConfigOptionsOld.end(); ++it )
      {
        QByteArray key = it.key().toLocal8Bit();
        QByteArray value = it.value().toLocal8Bit();
        CPLSetConfigOption( key.data(), value.data() );
      }

      // TODO print exact error message
      return "FAILED_NOT_SUPPORTED";
    }
    else
    {
      QgsDebugMsg( "Building pyramids finished OK" );
      //make sure the raster knows it has pyramids
      mHasPyramids = true;
    }
  }
  catch ( CPLErr )
  {
    QgsLogger::warning( "Pyramid overview building failed!" );
  }

  // restore former configOptions
  for ( QgsStringMap::const_iterator it = myConfigOptionsOld.begin();
        it != myConfigOptionsOld.end(); ++it )
  {
    QByteArray key = it.key().toLocal8Bit();
    QByteArray value = it.value().toLocal8Bit();
    CPLSetConfigOption( key.data(), value.data() );
  }

  QgsDebugMsg( "Pyramid overviews built" );

  // Observed problem: if a *.rrd file exists and GDALBuildOverviews() is called,
  // the *.rrd is deleted and no overviews are created, if GDALBuildOverviews()
  // is called next time, it crashes somewhere in GDAL:
  // https://trac.osgeo.org/gdal/ticket/4831
  // Crash can be avoided if dataset is reopened, fixed in GDAL 1.9.2
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1920
  if ( theFormat == QgsRaster::PyramidsInternal )
#else
  if ( true ) // GDAL #4831 fix
#endif
  {
    QgsDebugMsg( "Reopening dataset ..." );
    //close the gdal dataset and reopen it in read only mode
    GDALClose( mGdalBaseDataset );
    mGdalBaseDataset = gdalOpen( TO8F( dataSourceUri() ), mUpdate ? GA_Update : GA_ReadOnly );
    //Since we are not a virtual warped dataset, mGdalDataSet and mGdalBaseDataset are supposed to be the same
    mGdalDataset = mGdalBaseDataset;
  }

  //emit drawingProgress( 0, 0 );
  return nullptr; // returning null on success
}

#if 0
QList<QgsRasterPyramid> QgsGdalProvider::buildPyramidList()
{
  //
  // First we build up a list of potential pyramid layers
  //
  int myWidth = mWidth;
  int myHeight = mHeight;
  int myDivisor = 2;

  GDALRasterBandH myGDALBand = GDALGetRasterBand( mGdalDataset, 1 ); //just use the first band

  mPyramidList.clear();
  QgsDebugMsg( "Building initial pyramid list" );
  while (( myWidth / myDivisor > 32 ) && (( myHeight / myDivisor ) > 32 ) )
  {

    QgsRasterPyramid myRasterPyramid;
    myRasterPyramid.level = myDivisor;
    myRasterPyramid.xDim = ( int )( 0.5 + ( myWidth / ( double )myDivisor ) );
    myRasterPyramid.yDim = ( int )( 0.5 + ( myHeight / ( double )myDivisor ) );
    myRasterPyramid.exists = false;

    QgsDebugMsg( QString( "Pyramid %1 xDim %2 yDim %3" ).arg( myRasterPyramid.level ).arg( myRasterPyramid.xDim ).arg( myRasterPyramid.yDim ) );

    //
    // Now we check if it actually exists in the raster layer
    // and also adjust the dimensions if the dimensions calculated
    // above are only a near match.
    //
    const int myNearMatchLimit = 5;
    if ( GDALGetOverviewCount( myGDALBand ) > 0 )
    {
      int myOverviewCount;
      for ( myOverviewCount = 0;
            myOverviewCount < GDALGetOverviewCount( myGDALBand );
            ++myOverviewCount )
      {
        GDALRasterBandH myOverview;
        myOverview = GDALGetOverview( myGDALBand, myOverviewCount );
        int myOverviewXDim = GDALGetRasterBandXSize( myOverview );
        int myOverviewYDim = GDALGetRasterBandYSize( myOverview );
        //
        // here is where we check if its a near match:
        // we will see if its within 5 cells either side of
        //
        QgsDebugMsg( "Checking whether " + QString::number( myRasterPyramid.xDim ) + " x " +
                     QString::number( myRasterPyramid.yDim ) + " matches " +
                     QString::number( myOverviewXDim ) + " x " + QString::number( myOverviewYDim ) );


        if (( myOverviewXDim <= ( myRasterPyramid.xDim + myNearMatchLimit ) ) &&
            ( myOverviewXDim >= ( myRasterPyramid.xDim - myNearMatchLimit ) ) &&
            ( myOverviewYDim <= ( myRasterPyramid.yDim + myNearMatchLimit ) ) &&
            ( myOverviewYDim >= ( myRasterPyramid.yDim - myNearMatchLimit ) ) )
        {
          //right we have a match so adjust the a / y before they get added to the list
          myRasterPyramid.xDim = myOverviewXDim;
          myRasterPyramid.yDim = myOverviewYDim;
          myRasterPyramid.exists = true;
          QgsDebugMsg( ".....YES!" );
        }
        else
        {
          //no match
          QgsDebugMsg( ".....no." );
        }
      }
    }
    mPyramidList.append( myRasterPyramid );
    //sqare the divisor each step
    myDivisor = ( myDivisor * 2 );
  }

  return mPyramidList;
}
#endif

QList<QgsRasterPyramid> QgsGdalProvider::buildPyramidList( QList<int> overviewList )
{
  int myWidth = mWidth;
  int myHeight = mHeight;
  GDALRasterBandH myGDALBand = GDALGetRasterBand( mGdalDataset, 1 ); //just use the first band

  mPyramidList.clear();

  // if overviewList is empty (default) build the pyramid list
  if ( overviewList.isEmpty() )
  {
    int myDivisor = 2;

    QgsDebugMsg( "Building initial pyramid list" );

    while (( myWidth / myDivisor > 32 ) && (( myHeight / myDivisor ) > 32 ) )
    {
      overviewList.append( myDivisor );
      //sqare the divisor each step
      myDivisor = ( myDivisor * 2 );
    }
  }

  // loop over pyramid list
  Q_FOREACH ( int myDivisor, overviewList )
  {
    //
    // First we build up a list of potential pyramid layers
    //

    QgsRasterPyramid myRasterPyramid;
    myRasterPyramid.level = myDivisor;
    myRasterPyramid.xDim = ( int )( 0.5 + ( myWidth / ( double )myDivisor ) );
    myRasterPyramid.yDim = ( int )( 0.5 + ( myHeight / ( double )myDivisor ) );
    myRasterPyramid.exists = false;

    QgsDebugMsg( QString( "Pyramid %1 xDim %2 yDim %3" ).arg( myRasterPyramid.level ).arg( myRasterPyramid.xDim ).arg( myRasterPyramid.yDim ) );

    //
    // Now we check if it actually exists in the raster layer
    // and also adjust the dimensions if the dimensions calculated
    // above are only a near match.
    //
    const int myNearMatchLimit = 5;
    if ( GDALGetOverviewCount( myGDALBand ) > 0 )
    {
      int myOverviewCount;
      for ( myOverviewCount = 0;
            myOverviewCount < GDALGetOverviewCount( myGDALBand );
            ++myOverviewCount )
      {
        GDALRasterBandH myOverview;
        myOverview = GDALGetOverview( myGDALBand, myOverviewCount );
        int myOverviewXDim = GDALGetRasterBandXSize( myOverview );
        int myOverviewYDim = GDALGetRasterBandYSize( myOverview );
        //
        // here is where we check if its a near match:
        // we will see if its within 5 cells either side of
        //
        QgsDebugMsg( "Checking whether " + QString::number( myRasterPyramid.xDim ) + " x " +
                     QString::number( myRasterPyramid.yDim ) + " matches " +
                     QString::number( myOverviewXDim ) + " x " + QString::number( myOverviewYDim ) );


        if (( myOverviewXDim <= ( myRasterPyramid.xDim + myNearMatchLimit ) ) &&
            ( myOverviewXDim >= ( myRasterPyramid.xDim - myNearMatchLimit ) ) &&
            ( myOverviewYDim <= ( myRasterPyramid.yDim + myNearMatchLimit ) ) &&
            ( myOverviewYDim >= ( myRasterPyramid.yDim - myNearMatchLimit ) ) )
        {
          //right we have a match so adjust the a / y before they get added to the list
          myRasterPyramid.xDim = myOverviewXDim;
          myRasterPyramid.yDim = myOverviewYDim;
          myRasterPyramid.exists = true;
          QgsDebugMsg( ".....YES!" );
        }
        else
        {
          //no match
          QgsDebugMsg( ".....no." );
        }
      }
    }
    mPyramidList.append( myRasterPyramid );
  }

  return mPyramidList;
}

QStringList QgsGdalProvider::subLayers() const
{
  return mSubLayers;
}

void QgsGdalProvider::emitProgress( int theType, double theProgress, QString theMessage )
{
  emit progress( theType, theProgress, theMessage );
}

void QgsGdalProvider::emitProgressUpdate( int theProgress )
{
  emit progressUpdate( theProgress );
}

/**
 * Class factory to return a pointer to a newly created
 * QgsGdalProvider object
 */
QGISEXTERN QgsGdalProvider * classFactory( const QString *uri )
{
  return new QgsGdalProvider( *uri );
}
/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return PROVIDER_KEY;
}
/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return PROVIDER_DESCRIPTION;
}
/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}
/**

  Convenience function for readily creating file filters.

  Given a long name for a file filter and a regular expression, return
  a file filter string suitable for use in a QFileDialog::OpenFiles()
  call.  The regular express, glob, will have both all lower and upper
  case versions added.

  @note

  Copied from qgisapp.cpp.

  @todo XXX This should probably be generalized and moved to a standard
            utility type thingy.

*/
static QString createFileFilter_( QString const &longName, QString const &glob )
{
  // return longName + " [GDAL] (" + glob.toLower() + ' ' + glob.toUpper() + ");;";
  return longName + " (" + glob.toLower() + ' ' + glob.toUpper() + ");;";
} // createFileFilter_

void buildSupportedRasterFileFilterAndExtensions( QString & theFileFiltersString, QStringList & theExtensions, QStringList & theWildcards )
{

  // then iterate through all of the supported drivers, adding the
  // corresponding file filter

  GDALDriverH myGdalDriver;           // current driver

  char **myGdalDriverMetadata;        // driver metadata strings

  QString myGdalDriverLongName( "" ); // long name for the given driver
  QString myGdalDriverExtension( "" );  // file name extension for given driver
  QString myGdalDriverDescription;    // QString wrapper of GDAL driver description

  QStringList metadataTokens;   // essentially the metadata string delimited by '='

  QStringList catchallFilter;   // for Any file(*.*), but also for those
  // drivers with no specific file filter

  GDALDriverH jp2Driver = nullptr; // first JPEG2000 driver found

  QgsGdalProviderBase::registerGdalDrivers();

  // Grind through all the drivers and their respective metadata.
  // We'll add a file filter for those drivers that have a file
  // extension defined for them; the others, well, even though
  // theoreticaly we can open those files because there exists a
  // driver for them, the user will have to use the "All Files" to
  // open datasets with no explicitly defined file name extension.
  // Note that file name extension strings are of the form
  // "DMD_EXTENSION=.*".  We'll also store the long name of the
  // driver, which will be found in DMD_LONGNAME, which will have the
  // same form.

  theFileFiltersString = "";

  QgsDebugMsg( QString( "GDAL driver count: %1" ).arg( GDALGetDriverCount() ) );

  for ( int i = 0; i < GDALGetDriverCount(); ++i )
  {
    myGdalDriver = GDALGetDriver( i );

    Q_CHECK_PTR( myGdalDriver );

    if ( !myGdalDriver )
    {
      QgsLogger::warning( "unable to get driver " + QString::number( i ) );
      continue;
    }

    // in GDAL 2.0 vector and mixed drivers are returned by GDALGetDriver, so filter out non-raster drivers
    // TODO also make sure drivers are not loaded unnecessarily (as GDALAllRegister() and OGRRegisterAll load all drivers)
#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,0,0)
    if ( QString( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_RASTER, nullptr ) ) != "YES" )
      continue;
#endif
#endif

    // now we need to see if the driver is for something currently
    // supported; if not, we give it a miss for the next driver

    myGdalDriverDescription = GDALGetDescription( myGdalDriver );
    // QgsDebugMsg(QString("got driver string %1").arg(myGdalDriverDescription));

    myGdalDriverExtension = myGdalDriverLongName = "";

    myGdalDriverMetadata = GDALGetMetadata( myGdalDriver, nullptr );

    // presumably we know we've run out of metadta if either the
    // address is 0, or the first character is null
    while ( myGdalDriverMetadata && myGdalDriverMetadata[0] )
    {
      metadataTokens = QString( *myGdalDriverMetadata ).split( '=', QString::SkipEmptyParts );
      // QgsDebugMsg(QString("\t%1").arg(*myGdalDriverMetadata));

      // XXX add check for malformed metadataTokens

      // Note that it's oddly possible for there to be a
      // DMD_EXTENSION with no corresponding defined extension
      // string; so we check that there're more than two tokens.

      if ( metadataTokens.count() > 1 )
      {
        if ( "DMD_EXTENSION" == metadataTokens[0] )
        {
          myGdalDriverExtension = metadataTokens[1];

        }
        else if ( "DMD_LONGNAME" == metadataTokens[0] )
        {
          myGdalDriverLongName = metadataTokens[1];

          // remove any superfluous (.*) strings at the end as
          // they'll confuse QFileDialog::getOpenFileNames()

          myGdalDriverLongName.remove( QRegExp( "\\(.*\\)$" ) );
        }
      }

      // if we have both the file name extension and the long name,
      // then we've all the information we need for the current
      // driver; therefore emit a file filter string and move to
      // the next driver
      if ( !( myGdalDriverExtension.isEmpty() || myGdalDriverLongName.isEmpty() ) )
      {
        // XXX add check for SDTS; in that case we want (*CATD.DDF)
        QString glob = "*." + myGdalDriverExtension.replace( '/', " *." );
        theExtensions << myGdalDriverExtension.remove( '/' ).remove( '*' ).remove( '.' );
        // Add only the first JP2 driver found to the filter list (it's the one GDAL uses)
        if ( myGdalDriverDescription == "JPEG2000" ||
             myGdalDriverDescription.startsWith( "JP2" ) ) // JP2ECW, JP2KAK, JP2MrSID
        {
          if ( jp2Driver )
            break; // skip if already found a JP2 driver

          jp2Driver = myGdalDriver;   // first JP2 driver found
          glob += " *.j2k";           // add alternate extension
          theExtensions << "j2k";
        }
        else if ( myGdalDriverDescription == "GTiff" )
        {
          glob += " *.tiff";
          theExtensions << "tiff";
        }
        else if ( myGdalDriverDescription == "JPEG" )
        {
          glob += " *.jpeg";
          theExtensions << "jpeg";
        }
        else if ( myGdalDriverDescription == "VRT" )
        {
          glob += " *.ovr";
          theExtensions << "ovr";
        }

        theFileFiltersString += createFileFilter_( myGdalDriverLongName, glob );

        break;            // ... to next driver, if any.
      }

      ++myGdalDriverMetadata;

    }                       // each metadata item

    //QgsDebugMsg(QString("got driver Desc=%1 LongName=%2").arg(myGdalDriverDescription).arg(myGdalDriverLongName));

    if ( myGdalDriverExtension.isEmpty() && !myGdalDriverLongName.isEmpty() )
    {
      // Then what we have here is a driver with no corresponding
      // file extension; e.g., GRASS.  In which case we append the
      // string to the "catch-all" which will match all file types.
      // (I.e., "*.*") We use the driver description intead of the
      // long time to prevent the catch-all line from getting too
      // large.

      // ... OTOH, there are some drivers with missing
      // DMD_EXTENSION; so let's check for them here and handle
      // them appropriately

      // USGS DEMs use "*.dem"
      if ( myGdalDriverDescription.startsWith( "USGSDEM" ) )
      {
        theFileFiltersString += createFileFilter_( myGdalDriverLongName, "*.dem" );
        theExtensions << "dem";
      }
      else if ( myGdalDriverDescription.startsWith( "DTED" ) )
      {
        // DTED use "*.dt0, *.dt1, *.dt2"
        QString glob = "*.dt0";
        glob += " *.dt1";
        glob += " *.dt2";
        theFileFiltersString += createFileFilter_( myGdalDriverLongName, glob );
        theExtensions << "dt0" << "dt1" << "dt2";
      }
      else if ( myGdalDriverDescription.startsWith( "MrSID" ) )
      {
        // MrSID use "*.sid"
        theFileFiltersString += createFileFilter_( myGdalDriverLongName, "*.sid" );
        theExtensions << "sid";
      }
      else if ( myGdalDriverDescription.startsWith( "EHdr" ) )
      {
        theFileFiltersString += createFileFilter_( myGdalDriverLongName, "*.bil" );
        theExtensions << "bil";
      }
      else if ( myGdalDriverDescription.startsWith( "AIG" ) )
      {
        theFileFiltersString += createFileFilter_( myGdalDriverLongName, "hdr.adf" );
        theWildcards << "hdr.adf";
      }
      else if ( myGdalDriverDescription == "HDF4" )
      {
        // HDF4 extension missing in driver metadata
        theFileFiltersString += createFileFilter_( myGdalDriverLongName, "*.hdf" );
        theExtensions << "hdf";
      }
      else
      {
        catchallFilter << QString( GDALGetDescription( myGdalDriver ) );
      }
    } // each loaded GDAL driver

  }                           // each loaded GDAL driver

  // sort file filters alphabetically
  QStringList filters = theFileFiltersString.split( ";;", QString::SkipEmptyParts );
  filters.sort();
  theFileFiltersString = filters.join( ";;" ) + ";;";

  // VSIFileHandler (see qgsogrprovider.cpp) - second
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1600
  QSettings settings;
  if ( settings.value( "/qgis/scanZipInBrowser2", "basic" ).toString() != "no" )
  {
    theFileFiltersString.prepend( createFileFilter_( QObject::tr( "GDAL/OGR VSIFileHandler" ), "*.zip *.gz *.tar *.tar.gz *.tgz" ) );
    theExtensions << "zip" << "gz" << "tar" << "tar.gz" << "tgz";
  }
#endif

  // can't forget the default case - first
  theFileFiltersString.prepend( QObject::tr( "All files" ) + " (*);;" );

  // cleanup
  if ( theFileFiltersString.endsWith( ";;" ) ) theFileFiltersString.chop( 2 );

  QgsDebugMsg( "Raster filter list built: " + theFileFiltersString );
  QgsDebugMsg( "Raster extension list built: " + theExtensions.join( " " ) );
}                               // buildSupportedRasterFileFilter_()

QGISEXTERN bool isValidRasterFileName( QString const & theFileNameQString, QString & retErrMsg )
{
  GDALDatasetH myDataset;

  QgsGdalProviderBase::registerGdalDrivers();

  CPLErrorReset();

  QString fileName = theFileNameQString;

  // Try to open using VSIFileHandler (see qgsogrprovider.cpp)
  // TODO suppress error messages and report in debug, like in OGR provider
  QString vsiPrefix = QgsZipItem::vsiPrefix( fileName );
  if ( vsiPrefix != "" )
  {
    if ( !fileName.startsWith( vsiPrefix ) )
      fileName = vsiPrefix + fileName;
    QgsDebugMsg( QString( "Trying %1 syntax, fileName= %2" ).arg( vsiPrefix, fileName ) );
  }

  //open the file using gdal making sure we have handled locale properly
  //myDataset = GDALOpen( QFile::encodeName( theFileNameQString ).constData(), GA_ReadOnly );
  myDataset = QgsGdalProviderBase::gdalOpen( TO8F( fileName ), GA_ReadOnly );
  if ( !myDataset )
  {
    if ( CPLGetLastErrorNo() != CPLE_OpenFailed )
      retErrMsg = QString::fromUtf8( CPLGetLastErrorMsg() );
    return false;
  }
  else if ( GDALGetRasterCount( myDataset ) == 0 )
  {
    QStringList layers = QgsGdalProvider::subLayers( myDataset );
    GDALClose( myDataset );
    myDataset = nullptr;
    if ( layers.isEmpty() )
    {
      retErrMsg = QObject::tr( "This raster file has no bands and is invalid as a raster layer." );
      return false;
    }
    return true;
  }
  else
  {
    GDALClose( myDataset );
    return true;
  }
}

bool QgsGdalProvider::hasStatistics( int theBandNo,
                                     int theStats,
                                     const QgsRectangle & theExtent,
                                     int theSampleSize )
{
  QgsDebugMsg( QString( "theBandNo = %1 theSampleSize = %2" ).arg( theBandNo ).arg( theSampleSize ) );

  // First check if cached in mStatistics
  if ( QgsRasterDataProvider::hasStatistics( theBandNo, theStats, theExtent, theSampleSize ) )
  {
    return true;
  }

  QgsRasterBandStats myRasterBandStats;
  initStatistics( myRasterBandStats, theBandNo, theStats, theExtent, theSampleSize );

  if (( srcHasNoDataValue( theBandNo ) && !useSrcNoDataValue( theBandNo ) ) ||
      !userNoDataValues( theBandNo ).isEmpty() )
  {
    QgsDebugMsg( "Custom no data values -> GDAL statistics not sufficient." );
    return false;
  }

  // If not cached, check if supported by GDAL
  int supportedStats = QgsRasterBandStats::Min | QgsRasterBandStats::Max
                       | QgsRasterBandStats::Range | QgsRasterBandStats::Mean
                       | QgsRasterBandStats::StdDev;

  if ( myRasterBandStats.extent != extent() ||
       ( theStats & ( ~supportedStats ) ) )
  {
    QgsDebugMsg( "Not supported by GDAL." );
    return false;
  }

  QgsDebugMsg( "Looking for GDAL statistics" );

  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  if ( ! myGdalBand )
  {
    return false;
  }

  int bApproxOK = false;
  if ( theSampleSize > 0 )
  {
    if ((( double )xSize() * ( double )ySize() / theSampleSize ) > 2 )  // not perfect
    {
      bApproxOK = true;
    }
  }

  // Params in GDALGetRasterStatistics must not be nullptr otherwise GDAL returns
  // without error even if stats are not cached
  double dfMin, dfMax, dfMean, dfStdDev;
  double *pdfMin = &dfMin;
  double *pdfMax = &dfMax;
  double *pdfMean = &dfMean;
  double *pdfStdDev = &dfStdDev;

  if ( !( theStats & QgsRasterBandStats::Min ) ) pdfMin = nullptr;
  if ( !( theStats & QgsRasterBandStats::Max ) ) pdfMax = nullptr;
  if ( !( theStats & QgsRasterBandStats::Mean ) ) pdfMean = nullptr;
  if ( !( theStats & QgsRasterBandStats::StdDev ) ) pdfStdDev = nullptr;

  // try to fetch the cached stats (bForce=FALSE)
  // Unfortunately GDALGetRasterStatistics() does not work as expected according to
  // API doc, if bApproxOK=false and bForce=false/true and exact statistics
  // (from all raster pixels) are not available/cached, it should return CE_Warning.
  // Instead, it is giving estimated (from sample) cached statistics and it returns CE_None.
  // see above and https://trac.osgeo.org/gdal/ticket/4857
  // -> Cannot used cached GDAL stats for exact
  if ( !bApproxOK ) return false;

  CPLErr myerval = GDALGetRasterStatistics( myGdalBand, bApproxOK, true, pdfMin, pdfMax, pdfMean, pdfStdDev );

  if ( CE_None == myerval ) // CE_Warning if cached not found
  {
    QgsDebugMsg( "GDAL has cached statistics" );
    return true;
  }

  return false;
}

QgsRasterBandStats QgsGdalProvider::bandStatistics( int theBandNo, int theStats, const QgsRectangle & theExtent, int theSampleSize )
{
  QgsDebugMsg( QString( "theBandNo = %1 theSampleSize = %2" ).arg( theBandNo ).arg( theSampleSize ) );

  // TODO: null values set on raster layer!!!

  // Currently there is no API in GDAL to collect statistics of specified extent
  // or with defined sample size. We check first if we have cached stats, if not,
  // and it is not possible to use GDAL we call generic provider method,
  // otherwise we use GDAL (faster, cache)

  QgsRasterBandStats myRasterBandStats;
  initStatistics( myRasterBandStats, theBandNo, theStats, theExtent, theSampleSize );

  Q_FOREACH ( const QgsRasterBandStats& stats, mStatistics )
  {
    if ( stats.contains( myRasterBandStats ) )
    {
      QgsDebugMsg( "Using cached statistics." );
      return stats;
    }
  }

  // We cannot use GDAL stats if user disabled src no data value or set
  // custom  no data values
  if (( srcHasNoDataValue( theBandNo ) && !useSrcNoDataValue( theBandNo ) ) ||
      !userNoDataValues( theBandNo ).isEmpty() )
  {
    QgsDebugMsg( "Custom no data values, using generic statistics." );
    return QgsRasterDataProvider::bandStatistics( theBandNo, theStats, theExtent, theSampleSize );
  }

  int supportedStats = QgsRasterBandStats::Min | QgsRasterBandStats::Max
                       | QgsRasterBandStats::Range | QgsRasterBandStats::Mean
                       | QgsRasterBandStats::StdDev;

  QgsDebugMsg( QString( "theStats = %1 supportedStats = %2" ).arg( theStats, 0, 2 ).arg( supportedStats, 0, 2 ) );

  if ( myRasterBandStats.extent != extent() ||
       ( theStats & ( ~supportedStats ) ) )
  {
    QgsDebugMsg( "Statistics not supported by provider, using generic statistics." );
    return QgsRasterDataProvider::bandStatistics( theBandNo, theStats, theExtent, theSampleSize );
  }

  QgsDebugMsg( "Using GDAL statistics." );
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );

  //int bApproxOK = false; //as we asked for stats, don't get approx values
  // GDAL does not have sample size parameter in API, just bApproxOK or not,
  // we decide if approximation should be used according to
  // total size / sample size ration
  int bApproxOK = false;
  if ( theSampleSize > 0 )
  {
    if ((( double )xSize() * ( double )ySize() / theSampleSize ) > 2 )  // not perfect
    {
      bApproxOK = true;
    }
  }

  QgsDebugMsg( QString( "bApproxOK = %1" ).arg( bApproxOK ) );

  double pdfMin;
  double pdfMax;
  double pdfMean;
  double pdfStdDev;
  QgsGdalProgress myProg;
  myProg.type = QgsRaster::ProgressHistogram;
  myProg.provider = this;

  // try to fetch the cached stats (bForce=FALSE)
  // GDALGetRasterStatistics() do not work correctly with bApproxOK=false and bForce=false/true
  // see above and https://trac.osgeo.org/gdal/ticket/4857
  // -> Cannot used cached GDAL stats for exact

  CPLErr myerval =
    GDALGetRasterStatistics( myGdalBand, bApproxOK, true, &pdfMin, &pdfMax, &pdfMean, &pdfStdDev );

  QgsDebugMsg( QString( "myerval = %1" ).arg( myerval ) );

  // if cached stats are not found, compute them
  if ( !bApproxOK || CE_None != myerval )
  {
    QgsDebugMsg( "Calculating statistics by GDAL" );
    myerval = GDALComputeRasterStatistics( myGdalBand, bApproxOK,
                                           &pdfMin, &pdfMax, &pdfMean, &pdfStdDev,
                                           progressCallback, &myProg );
  }
  else
  {
    QgsDebugMsg( "Using GDAL cached statistics" );
  }

  // if stats are found populate the QgsRasterBandStats object
  if ( CE_None == myerval )
  {
    myRasterBandStats.bandNumber = theBandNo;
    myRasterBandStats.range =  pdfMax - pdfMin;
    myRasterBandStats.minimumValue = pdfMin;
    myRasterBandStats.maximumValue = pdfMax;
    //calculate the mean
    myRasterBandStats.mean = pdfMean;
    myRasterBandStats.sum = 0; //not available via gdal
    //myRasterBandStats.elementCount = mWidth * mHeight;
    // Sum of non NULL
    myRasterBandStats.elementCount = 0; //not available via gdal
    myRasterBandStats.sumOfSquares = 0; //not available via gdal
    myRasterBandStats.stdDev = pdfStdDev;
    myRasterBandStats.statsGathered = QgsRasterBandStats::Min | QgsRasterBandStats::Max
                                      | QgsRasterBandStats::Range | QgsRasterBandStats::Mean
                                      | QgsRasterBandStats::StdDev;

    // define if the band has scale and offset to apply
    double myScale = bandScale( theBandNo );
    double myOffset = bandOffset( theBandNo );
    if ( myScale != 1.0 || myOffset != 0.0 )
    {
      if ( myScale < 0.0 )
      {
        // update Min and Max value
        myRasterBandStats.minimumValue = pdfMax * myScale + myOffset;
        myRasterBandStats.maximumValue = pdfMin * myScale + myOffset;
        // update the range
        myRasterBandStats.range = ( pdfMin - pdfMax ) * myScale;
        // update standard deviation
        myRasterBandStats.stdDev = -1.0 * pdfStdDev * myScale;
      }
      else
      {
        // update Min and Max value
        myRasterBandStats.minimumValue = pdfMin * myScale + myOffset;
        myRasterBandStats.maximumValue = pdfMax * myScale + myOffset;
        // update the range
        myRasterBandStats.range = ( pdfMax - pdfMin ) * myScale;
        // update standard deviation
        myRasterBandStats.stdDev = pdfStdDev * myScale;
      }
      // update the mean
      myRasterBandStats.mean = pdfMean * myScale + myOffset;
    }

#ifdef QGISDEBUG
    QgsDebugMsg( "************ STATS **************" );
    QgsDebugMsg( QString( "MIN %1" ).arg( myRasterBandStats.minimumValue ) );
    QgsDebugMsg( QString( "MAX %1" ).arg( myRasterBandStats.maximumValue ) );
    QgsDebugMsg( QString( "RANGE %1" ).arg( myRasterBandStats.range ) );
    QgsDebugMsg( QString( "MEAN %1" ).arg( myRasterBandStats.mean ) );
    QgsDebugMsg( QString( "STDDEV %1" ).arg( myRasterBandStats.stdDev ) );
#endif
  }

  mStatistics.append( myRasterBandStats );
  return myRasterBandStats;

} // QgsGdalProvider::bandStatistics

void QgsGdalProvider::initBaseDataset()
{
#if 0
  for ( int i = 0; i < GDALGetRasterCount( mGdalBaseDataset ); i++ )
  {
    mMinMaxComputed.append( false );
    mMinimum.append( 0 );
    mMaximum.append( 0 );
  }
#endif
  // Check if we need a warped VRT for this file.
  bool hasGeoTransform = GDALGetGeoTransform( mGdalBaseDataset, mGeoTransform ) == CE_None;
  if (( hasGeoTransform
        && ( mGeoTransform[1] < 0.0
             || mGeoTransform[2] != 0.0
             || mGeoTransform[4] != 0.0
             || mGeoTransform[5] > 0.0 ) )
      || GDALGetGCPCount( mGdalBaseDataset ) > 0
      || GDALGetMetadata( mGdalBaseDataset, "RPC" ) )
  {
    QgsLogger::warning( "Creating Warped VRT." );

    mGdalDataset =
      GDALAutoCreateWarpedVRT( mGdalBaseDataset, nullptr, nullptr,
                               GRA_NearestNeighbour, 0.2, nullptr );

    if ( !mGdalDataset )
    {
      QgsLogger::warning( "Warped VRT Creation failed." );
      mGdalDataset = mGdalBaseDataset;
      GDALReferenceDataset( mGdalDataset );
    }
    else
    {
      hasGeoTransform = GDALGetGeoTransform( mGdalDataset, mGeoTransform ) == CE_None;
    }
  }
  else
  {
    mGdalDataset = mGdalBaseDataset;
    GDALReferenceDataset( mGdalDataset );
  }

  if ( !hasGeoTransform )
  {
    // Initialise the affine transform matrix
    mGeoTransform[0] =  0;
    mGeoTransform[1] =  1;
    mGeoTransform[2] =  0;
    mGeoTransform[3] =  0;
    mGeoTransform[4] =  0;
    mGeoTransform[5] = -1;
  }

  // get sublayers
  mSubLayers = QgsGdalProvider::subLayers( mGdalDataset );

  // check if this file has bands or subdatasets
  CPLErrorReset();
  GDALRasterBandH myGDALBand = GDALGetRasterBand( mGdalDataset, 1 ); //just use the first band
  if ( !myGDALBand )
  {
    QString msg = QString::fromUtf8( CPLGetLastErrorMsg() );

    // if there are no subdatasets, then close the dataset
    if ( mSubLayers.isEmpty() )
    {
      appendError( ERRMSG( tr( "Cannot get GDAL raster band: %1" ).arg( msg ) ) );

      GDALDereferenceDataset( mGdalBaseDataset );
      mGdalBaseDataset = nullptr;

      GDALClose( mGdalDataset );
      mGdalDataset = nullptr;
      return;
    }
    // if there are subdatasets, leave the dataset open for subsequent queries
    else
    {
      QgsDebugMsg( QObject::tr( "Cannot get GDAL raster band: %1" ).arg( msg ) +
                   QString( " but dataset has %1 subdatasets" ).arg( mSubLayers.size() ) );
      return;
    }
  }

  // check if this file has pyramids
  mHasPyramids = gdalGetOverviewCount( myGDALBand ) > 0;

  // Get the layer's projection info and set up the
  // QgsCoordinateTransform for this layer
  // NOTE: we must do this before metadata is called

  if ( !crsFromWkt( GDALGetProjectionRef( mGdalDataset ) ) &&
       !crsFromWkt( GDALGetGCPProjection( mGdalDataset ) ) )
  {
    if ( mGdalBaseDataset != mGdalDataset &&
         GDALGetMetadata( mGdalBaseDataset, "RPC" ) )
    {
      // Warped VRT of RPC is in EPSG:4326
      mCrs = QgsCRSCache::instance()->crsByOgcWmsCrs( "EPSG:4326" );
    }
    else
    {
      QgsDebugMsg( "No valid CRS identified" );
    }
  }

  //set up the coordinat transform - in the case of raster this is mainly used to convert
  //the inverese projection of the map extents of the canvas when zooming in etc. so
  //that they match the coordinate system of this layer
  //QgsDebugMsg( "Layer registry has " + QString::number( QgsMapLayerRegistry::instance()->count() ) + "layers" );

  //metadata();

  // Use the affine transform to get geo coordinates for
  // the corners of the raster
  double myXMax = mGeoTransform[0] +
                  GDALGetRasterXSize( mGdalDataset ) * mGeoTransform[1] +
                  GDALGetRasterYSize( mGdalDataset ) * mGeoTransform[2];
  double myYMin = mGeoTransform[3] +
                  GDALGetRasterXSize( mGdalDataset ) * mGeoTransform[4] +
                  GDALGetRasterYSize( mGdalDataset ) * mGeoTransform[5];

  mExtent.setXMaximum( myXMax );
  // The affine transform reduces to these values at the
  // top-left corner of the raster
  mExtent.setXMinimum( mGeoTransform[0] );
  mExtent.setYMaximum( mGeoTransform[3] );
  mExtent.setYMinimum( myYMin );

  //
  // Set up the x and y dimensions of this raster layer
  //
  mWidth = GDALGetRasterXSize( mGdalDataset );
  mHeight = GDALGetRasterYSize( mGdalDataset );


  GDALGetBlockSize( GDALGetRasterBand( mGdalDataset, 1 ), &mXBlockSize, &mYBlockSize );
  //
  // Determine the nodata value and data type
  //
  //mValidNoDataValue = true;
  for ( int i = 1; i <= GDALGetRasterCount( mGdalBaseDataset ); i++ )
  {
    GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, i );
    GDALDataType myGdalDataType = GDALGetRasterDataType( myGdalBand );

    int isValid = false;
    double myNoDataValue = GDALGetRasterNoDataValue( myGdalBand, &isValid );
    // We check that the double value we just got is representable in the
    // data type. In normal situations this should not be needed, but it happens
    // to have 8bit TIFFs with nan as the nodata value. If not checking against
    // the min/max bounds, it would be cast to 0 by representableValue().
    if ( isValid && !QgsRaster::isRepresentableValue( myNoDataValue, dataTypeFromGdal( myGdalDataType ) ) )
    {
      QgsDebugMsg( QString( "GDALGetRasterNoDataValue = %1 is not representable in data type, so ignoring it" ).arg( myNoDataValue ) );
      isValid = false;
    }
    if ( isValid )
    {
      QgsDebugMsg( QString( "GDALGetRasterNoDataValue = %1" ).arg( myNoDataValue ) );
      // The no data value double may be non representable by data type, it can result
      // in problems if that value is used to represent additional user defined no data
      // see #3840
      myNoDataValue = QgsRaster::representableValue( myNoDataValue, dataTypeFromGdal( myGdalDataType ) );
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
    // Not used
#if 0
    int myInternalGdalDataType = myGdalDataType;
    double myInternalNoDataValue = 123;
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
#endif
    //mGdalDataType.append( myInternalGdalDataType );

    // define if the band has scale and offset to apply
    double myScale = bandScale( i );
    double myOffset = bandOffset( i );
    if ( !qgsDoubleNear( myScale, 1.0 ) || !qgsDoubleNear( myOffset, 0.0 ) )
    {
      // if the band has scale or offset to apply change dataType
      switch ( myGdalDataType )
      {
        case GDT_Unknown:
        case GDT_TypeCount:
          break;
        case GDT_Byte:
        case GDT_UInt16:
        case GDT_Int16:
        case GDT_UInt32:
        case GDT_Int32:
        case GDT_Float32:
        case GDT_CInt16:
          myGdalDataType = GDT_Float32;
          break;
        case GDT_Float64:
        case GDT_CInt32:
        case GDT_CFloat32:
          myGdalDataType = GDT_Float64;
          break;
        case GDT_CFloat64:
          break;
      }
    }

    mGdalDataType.append( myGdalDataType );
    //mInternalNoDataValue.append( myInternalNoDataValue );
    //QgsDebugMsg( QString( "mInternalNoDataValue[%1] = %2" ).arg( i - 1 ).arg( mInternalNoDataValue[i-1] ) );
  }

  mValid = true;
}

char** papszFromStringList( const QStringList& list )
{
  char **papszRetList = nullptr;
  Q_FOREACH ( const QString& elem, list )
  {
    papszRetList = CSLAddString( papszRetList, elem.toLocal8Bit().constData() );
  }
  return papszRetList;
}

#if 0
bool QgsGdalProvider::create( const QString& format, int nBands,
                              QGis::DataType type,
                              int width, int height, double* geoTransform,
                              const QgsCoordinateReferenceSystem& crs,
                              QStringList createOptions )
#endif
QGISEXTERN QgsGdalProvider * create(
  const QString &uri,
  const QString& format, int nBands,
  QGis::DataType type,
  int width, int height, double* geoTransform,
  const QgsCoordinateReferenceSystem& crs,
  QStringList createOptions )
{
  //get driver
  GDALDriverH driver = GDALGetDriverByName( format.toLocal8Bit().data() );
  if ( !driver )
  {
    QgsError error( "Cannot load GDAL driver " + format, "GDAL provider" );
    return new QgsGdalProvider( uri, error );
  }

  QgsDebugMsg( "create options: " + createOptions.join( " " ) );

  //create dataset
  CPLErrorReset();
  char **papszOptions = papszFromStringList( createOptions );
  GDALDatasetH dataset = GDALCreate( driver, TO8F( uri ), width, height, nBands, ( GDALDataType )type, papszOptions );
  CSLDestroy( papszOptions );
  if ( !dataset )
  {
    QgsError error( QString( "Cannot create new dataset  %1:\n%2" ).arg( uri, QString::fromUtf8( CPLGetLastErrorMsg() ) ), "GDAL provider" );
    QgsDebugMsg( error.summary() );
    return new QgsGdalProvider( uri, error );
  }

  GDALSetGeoTransform( dataset, geoTransform );
  GDALSetProjection( dataset, crs.toWkt().toLocal8Bit().data() );
  GDALClose( dataset );

  return new QgsGdalProvider( uri, true );
}

bool QgsGdalProvider::write( void* data, int band, int width, int height, int xOffset, int yOffset )
{
  if ( !mGdalDataset )
  {
    return false;
  }

  GDALRasterBandH rasterBand = GDALGetRasterBand( mGdalDataset, band );
  if ( !rasterBand )
  {
    return false;
  }
  return gdalRasterIO( rasterBand, GF_Write, xOffset, yOffset, width, height, data, width, height, GDALGetRasterDataType( rasterBand ), 0, 0 ) == CE_None;
}

bool QgsGdalProvider::setNoDataValue( int bandNo, double noDataValue )
{
  if ( !mGdalDataset )
  {
    return false;
  }

  GDALRasterBandH rasterBand = GDALGetRasterBand( mGdalDataset, bandNo );
  CPLErrorReset();
  CPLErr err = GDALSetRasterNoDataValue( rasterBand, noDataValue );
  if ( err != CPLE_None )
  {
    QgsDebugMsg( "Cannot set no data value" );
    return false;
  }
  mSrcNoDataValue[bandNo-1] = noDataValue;
  mSrcHasNoDataValue[bandNo-1] = true;
  mUseSrcNoDataValue[bandNo-1] = true;
  return true;
}

bool QgsGdalProvider::remove()
{
  if ( mGdalDataset )
  {
    GDALDriverH driver = GDALGetDatasetDriver( mGdalDataset );
    GDALClose( mGdalDataset );
    mGdalDataset = nullptr;

    CPLErrorReset();
    CPLErr err = GDALDeleteDataset( driver, TO8F( dataSourceUri() ) );
    if ( err != CPLE_None )
    {
      QgsLogger::warning( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
      QgsDebugMsg( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
      return false;
    }
    QgsDebugMsg( "Raster dataset dataSourceUri() successfully deleted" );
    return true;
  }
  return false;
}

/**
  Builds the list of file filter strings to later be used by
  QgisApp::addRasterLayer()

  We query GDAL for a list of supported raster formats; we then build
  a list of file filter strings from that list.  We return a string
  that contains this list that is suitable for use in a
  QFileDialog::getOpenFileNames() call.

*/
QGISEXTERN void buildSupportedRasterFileFilter( QString & theFileFiltersString )
{
  QStringList exts;
  QStringList wildcards;
  buildSupportedRasterFileFilterAndExtensions( theFileFiltersString, exts, wildcards );
}

/**
  Gets creation options metadata for a given format
*/
QGISEXTERN QString helpCreationOptionsFormat( QString format )
{
  QString message;
  GDALDriverH myGdalDriver = GDALGetDriverByName( format.toLocal8Bit().constData() );
  if ( myGdalDriver )
  {
    // first report details and help page
    char ** GDALmetadata = GDALGetMetadata( myGdalDriver, nullptr );
    message += "Format Details:\n";
    message += QString( "  Extension: %1\n" ).arg( CSLFetchNameValue( GDALmetadata, GDAL_DMD_EXTENSION ) );
    message += QString( "  Short Name: %1" ).arg( GDALGetDriverShortName( myGdalDriver ) );
    message += QString( "  /  Long Name: %1\n" ).arg( GDALGetDriverLongName( myGdalDriver ) );
    message += QString( "  Help page:  http://www.gdal.org/%1\n\n" ).arg( CSLFetchNameValue( GDALmetadata, GDAL_DMD_HELPTOPIC ) );

    // next get creation options
    // need to serialize xml to get newlines, should we make the basic xml prettier?
    CPLXMLNode *psCOL = CPLParseXMLString( GDALGetMetadataItem( myGdalDriver,
                                           GDAL_DMD_CREATIONOPTIONLIST, "" ) );
    char *pszFormattedXML = CPLSerializeXMLTree( psCOL );
    if ( pszFormattedXML )
      message += QString( pszFormattedXML );
    if ( psCOL )
      CPLDestroyXMLNode( psCOL );
    if ( pszFormattedXML )
      CPLFree( pszFormattedXML );
  }
  return message;
}

/**
  Validates creation options for a given format, regardless of layer.
*/
QGISEXTERN QString validateCreationOptionsFormat( const QStringList& createOptions, QString format )
{
  GDALDriverH myGdalDriver = GDALGetDriverByName( format.toLocal8Bit().constData() );
  if ( ! myGdalDriver )
    return "invalid GDAL driver";

  char** papszOptions = papszFromStringList( createOptions );
  // get error string?
  int ok = GDALValidateCreationOptions( myGdalDriver, papszOptions );
  CSLDestroy( papszOptions );

  if ( !ok )
    return "Failed GDALValidateCreationOptions() test";
  return QString();
}

QString QgsGdalProvider::validateCreationOptions( const QStringList& createOptions, const QString& format )
{
  QString message;

  // first validate basic syntax with GDALValidateCreationOptions
  message = validateCreationOptionsFormat( createOptions, format );
  if ( !message.isNull() )
    return message;

  // next do specific validations, depending on format and dataset
  // only check certain destination formats
  QStringList formatsCheck;
  formatsCheck << "gtiff";
  if ( ! formatsCheck.contains( format.toLower() ) )
    return QString();

  // prepare a map for easier lookup
  QMap< QString, QString > optionsMap;
  Q_FOREACH ( const QString& option, createOptions )
  {
    QStringList opt = option.split( '=' );
    optionsMap[ opt[0].toUpper()] = opt[1];
    QgsDebugMsg( "option: " + option );
  }

  // gtiff files - validate PREDICTOR option
  // see gdal: frmts/gtiff/geotiff.cpp and libtiff: tif_predict.c)
  if ( format.toLower() == "gtiff" && optionsMap.contains( "PREDICTOR" ) )
  {
    QString value = optionsMap.value( "PREDICTOR" );
    GDALDataType nDataType = ( !mGdalDataType.isEmpty() ) ? ( GDALDataType ) mGdalDataType.at( 0 ) : GDT_Unknown;
    int nBitsPerSample = nDataType != GDT_Unknown ? GDALGetDataTypeSize( nDataType ) : 0;
    QgsDebugMsg( QString( "PREDICTOR: %1 nbits: %2 type: %3" ).arg( value ).arg( nBitsPerSample ).arg(( GDALDataType ) mGdalDataType.at( 0 ) ) );
    // PREDICTOR=2 only valid for 8/16/32 bits per sample
    // TODO check for NBITS option (see geotiff.cpp)
    if ( value == "2" )
    {
      if ( nBitsPerSample != 8 && nBitsPerSample != 16 &&
           nBitsPerSample != 32 )
      {
        message = QString( "PREDICTOR=%1 only valid for 8/16/32 bits per sample (using %2)" ).arg( value ).arg( nBitsPerSample );
      }
    }
    // PREDICTOR=3 only valid for float/double precision
    else if ( value == "3" )
    {
      if ( nDataType != GDT_Float32 && nDataType != GDT_Float64 )
        message = "PREDICTOR=3 only valid for float/double precision";
    }
  }

  return message;
}

QString QgsGdalProvider::validatePyramidsConfigOptions( QgsRaster::RasterPyramidsFormat pyramidsFormat,
    const QStringList & theConfigOptions, const QString & fileFormat )
{
  // Erdas Imagine format does not support config options
  if ( pyramidsFormat == QgsRaster::PyramidsErdas )
  {
    if ( ! theConfigOptions.isEmpty() )
      return "Erdas Imagine format does not support config options";
    else
      return QString();
  }
  // Internal pyramids format only supported for gtiff/georaster/hfa/jp2kak/mrsid/nitf files
  else if ( pyramidsFormat == QgsRaster::PyramidsInternal )
  {
    QStringList supportedFormats;
    supportedFormats << "gtiff" << "georaster" << "hfa" << "gpkg" << "rasterlite" << "nitf";
    if ( ! supportedFormats.contains( fileFormat.toLower() ) )
      return QString( "Internal pyramids format only supported for gtiff/georaster/gpkg/rasterlite/nitf files (using %1)" ).arg( fileFormat );
  }
  else
  {
    // for gtiff external pyramids, validate gtiff-specific values
    // PHOTOMETRIC_OVERVIEW=YCBCR requires a source raster with only 3 bands (RGB)
    if ( theConfigOptions.contains( "PHOTOMETRIC_OVERVIEW=YCBCR" ) )
    {
      if ( GDALGetRasterCount( mGdalDataset ) != 3 )
        return "PHOTOMETRIC_OVERVIEW=YCBCR requires a source raster with only 3 bands (RGB)";
    }
  }

  return QString();
}

// pyramids resampling

// see http://www.gdal.org/gdaladdo.html
//     http://www.gdal.org/classGDALDataset.html#a2aa6f88b3bbc840a5696236af11dde15
//     http://www.gdal.org/classGDALRasterBand.html#afaea945b13ec9c86c2d783b883c68432

// from http://www.gdal.org/gdaladdo.html
//   average_mp is unsuitable for use thus not included

// from qgsgdalprovider.cpp (removed)
//   NOTE magphase is disabled in the gui since it tends
//   to create corrupted images. The images can be repaired
//   by running one of the other resampling strategies below.
//   see ticket #284

QGISEXTERN QList<QPair<QString, QString> > *pyramidResamplingMethods()
{
  static QList<QPair<QString, QString> > methods;

  if ( methods.isEmpty() )
  {
    methods.append( QPair<QString, QString>( "NEAREST", QObject::tr( "Nearest Neighbour" ) ) );
    methods.append( QPair<QString, QString>( "AVERAGE", QObject::tr( "Average" ) ) );
    methods.append( QPair<QString, QString>( "GAUSS", QObject::tr( "Gauss" ) ) );
    methods.append( QPair<QString, QString>( "CUBIC", QObject::tr( "Cubic" ) ) );
#if GDAL_VERSION_MAJOR >= 2
    methods.append( QPair<QString, QString>( "CUBICSPLINE", QObject::tr( "Cubic Spline" ) ) );
    methods.append( QPair<QString, QString>( "LANCZOS", QObject::tr( "Lanczos" ) ) );
#endif
    methods.append( QPair<QString, QString>( "MODE", QObject::tr( "Mode" ) ) );
    methods.append( QPair<QString, QString>( "NONE", QObject::tr( "None" ) ) );
  }

  return &methods;
}

QGISEXTERN void cleanupProvider()
{
  // nothing to do here, QgsApplication takes care of
  // calling GDALDestroyDriverManager()
}
