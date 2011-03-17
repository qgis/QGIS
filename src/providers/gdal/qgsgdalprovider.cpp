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

/* $Id: qgsgdalprovider.cpp 11522 2009-08-28 14:49:22Z jef $ */

#include "qgslogger.h"
#include "qgsgdalprovider.h"
#include "qgsconfig.h"

#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterpyramid.h"

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

#include "gdalwarper.h"
#include "ogr_spatialref.h"
#include "cpl_conv.h"
#include "cpl_string.h"

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8F(x) (x).toUtf8().constData()
#else
#define TO8F(x) QFile::encodeName( x ).constData()
#endif


static QString PROVIDER_KEY = "gdal";
static QString PROVIDER_DESCRIPTION = "GDAL provider";

//
// global callback function
//
int CPL_STDCALL progressCallback( double dfComplete,
                                  const char * pszMessage,
                                  void * pProgressArg )
{
  // TODO: add signals to providers
  static double dfLastComplete = -1.0;

  if ( dfLastComplete > dfComplete )
  {
    if ( dfLastComplete >= 1.0 )
      dfLastComplete = -1.0;
    else
      dfLastComplete = dfComplete;
  }

  if ( floor( dfLastComplete*10 ) != floor( dfComplete*10 ) )
  {
    int    nPercent = ( int ) floor( dfComplete * 100 );

    if ( nPercent == 0 && pszMessage != NULL )
    {
      //fprintf( stdout, "%s:", pszMessage );
    }

    if ( nPercent == 100 )
    {
      fprintf( stdout, "%d - done.\n", ( int ) floor( dfComplete*100 ) );
      //mypLayer->showProgress( 100 );
    }
    else
    {
      int myProgress = ( int ) floor( dfComplete * 100 );
      fprintf( stdout, "%d.", myProgress );
      //mypLayer->showProgress( myProgress );
      fflush( stdout );
    }
  }
  dfLastComplete = dfComplete;

  return true;
}


QgsGdalProvider::QgsGdalProvider( QString const & uri )
    : QgsRasterDataProvider( uri )
    , mValid( true )
{
  QgsDebugMsg( "QgsGdalProvider: constructing with uri '" + uri + "'." );

  mValid = false;
  mGdalBaseDataset = 0;
  mGdalDataset = 0;

  registerGdalDrivers();

  // To get buildSupportedRasterFileFilter the provider is called with empty uri
  if ( uri.isEmpty() ) return;

  mGdalDataset = NULL;

  //mGdalBaseDataset = GDALOpen( QFile::encodeName( uri ).constData(), GA_ReadOnly );
  mGdalBaseDataset = GDALOpen( TO8F( uri ), GA_ReadOnly );

  CPLErrorReset();
  if ( mGdalBaseDataset == NULL )
  {
    QgsDebugMsg( QString( "Cannot open GDAL dataset %1: %2" ).arg( uri ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    return;
  }

  QgsDebugMsg( "GdalDataset opened" );

  for ( int i = 0; i < GDALGetRasterCount( mGdalBaseDataset ); i++ )
  {
    mMinMaxComputed.append( false );
    mMinimum.append( 0 );
    mMaximum.append( 0 );
  }

  // Check if we need a warped VRT for this file.
  bool hasGeoTransform = GDALGetGeoTransform( mGdalBaseDataset, mGeoTransform ) == CE_None;
  if (( hasGeoTransform
        && ( mGeoTransform[1] < 0.0
             || mGeoTransform[2] != 0.0
             || mGeoTransform[4] != 0.0
             || mGeoTransform[5] > 0.0 ) )
      || GDALGetGCPCount( mGdalBaseDataset ) > 0 )
  {
    QgsLogger::warning( "Creating Warped VRT." );

    mGdalDataset =
      GDALAutoCreateWarpedVRT( mGdalBaseDataset, NULL, NULL,
                               GRA_NearestNeighbour, 0.2, NULL );

    if ( mGdalDataset == NULL )
    {
      QgsLogger::warning( "Warped VRT Creation failed." );
      mGdalDataset = mGdalBaseDataset;
      GDALReferenceDataset( mGdalDataset );
    }
    else
    {
      GDALGetGeoTransform( mGdalDataset, mGeoTransform );
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

  //check if this file has pyramids
  CPLErrorReset();
  GDALRasterBandH myGDALBand = GDALGetRasterBand( mGdalDataset, 1 ); //just use the first band
  if ( myGDALBand == NULL )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QObject::tr( "Cannot get GDAL raster band: %1" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );

    GDALDereferenceDataset( mGdalBaseDataset );
    mGdalBaseDataset = NULL;

    GDALClose( mGdalDataset );
    mGdalDataset = NULL;
    return;
  }

  mHasPyramids = GDALGetOverviewCount( myGDALBand ) > 0;

  // Get the layer's projection info and set up the
  // QgsCoordinateTransform for this layer
  // NOTE: we must do this before metadata is called

  if ( !crsFromWkt( GDALGetProjectionRef( mGdalDataset ) ) &&
       !crsFromWkt( GDALGetGCPProjection( mGdalDataset ) ) )
  {
    QgsDebugMsg( "No valid CRS identified" );
    mCrs.validate();
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
  mValidNoDataValue = true;
  for ( int i = 1; i <= GDALGetRasterCount( mGdalBaseDataset ); i++ )
  {
    computeMinMax( i );
    GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, i );
    GDALDataType myGdalDataType = GDALGetRasterDataType( myGdalBand );
    int isValid = false;
    double myNoDataValue = GDALGetRasterNoDataValue( GDALGetRasterBand( mGdalDataset, i ), &isValid );
    if ( isValid )
    {
      QgsDebugMsg( QString( "GDALGetRasterNoDataValue = %1" ).arg( myNoDataValue ) ) ;
      mGdalDataType.append( myGdalDataType );

    }
    else
    {
      // But we need a null value in case of reprojection and BTW also for
      // aligned margines

      switch ( srcDataType( i ) )
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
    QgsDebugMsg( QString( "mNoDataValue[%1] = %1" ).arg( i - 1 ).arg( mNoDataValue[i-1] ) );
  }

  // This block of code was in old version in QgsRasterLayer::bandStatistics
  //ifdefs below to remove compiler warning about unused vars
#ifdef QGISDEBUG
#if 0
  int success;
  double GDALminimum = GDALGetRasterMinimum( myGdalBand, &success );

  if ( ! success )
  {
    QgsDebugMsg( "myGdalBand->GetMinimum() failed" );
  }

  double GDALmaximum = GDALGetRasterMaximum( myGdalBand, &success );

  if ( ! success )
  {
    QgsDebugMsg( "myGdalBand->GetMaximum() failed" );
  }

  double GDALnodata = GDALGetRasterNoDataValue( myGdalBand, &success );

  if ( ! success )
  {
    QgsDebugMsg( "myGdalBand->GetNoDataValue() failed" );
  }

  QgsLogger::debug( "GDALminium: ", GDALminimum, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "GDALmaximum: ", GDALmaximum, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "GDALnodata: ", GDALnodata, __FILE__, __FUNCTION__, __LINE__ );

  double GDALrange[2];          // calculated min/max, as opposed to the
  // dataset provided

  GDALComputeRasterMinMax( myGdalBand, 1, GDALrange );
  QgsLogger::debug( "approximate computed GDALminium:", GDALrange[0], __FILE__, __FUNCTION__, __LINE__, 1 );
  QgsLogger::debug( "approximate computed GDALmaximum:", GDALrange[1], __FILE__, __FUNCTION__, __LINE__, 1 );

  GDALComputeRasterMinMax( myGdalBand, 0, GDALrange );
  QgsLogger::debug( "exactly computed GDALminium:", GDALrange[0] );
  QgsLogger::debug( "exactly computed GDALmaximum:", GDALrange[1] );

  QgsDebugMsg( "starting manual stat computation" );
#endif
#endif

  mValid = true;
  QgsDebugMsg( "end" );
}

bool QgsGdalProvider::crsFromWkt( const char *wkt )
{
  void *hCRS = OSRNewSpatialReference( NULL );

  if ( OSRImportFromWkt( hCRS, ( char ** ) &wkt ) == OGRERR_NONE )
  {
    if ( OSRAutoIdentifyEPSG( hCRS ) == OGRERR_NONE )
    {
      QString authid = QString( "%1:%2" )
                       .arg( OSRGetAuthorityName( hCRS, NULL ) )
                       .arg( OSRGetAuthorityCode( hCRS, NULL ) );
      QgsDebugMsg( "authid recognized as " + authid );
      mCrs.createFromOgcWmsCrs( authid );
    }
    else
    {
      // get the proj4 text
      char *pszProj4;
      OSRExportToProj4( hCRS, &pszProj4 );
      QgsDebugMsg( pszProj4 );
      OGRFree( pszProj4 );

      char *pszWkt = NULL;
      OSRExportToWkt( hCRS, &pszWkt );
      QString myWktString = QString( pszWkt );
      OGRFree( pszWkt );

      // create CRS from Wkt
      mCrs.createFromWkt( myWktString );
    }
  }

  OSRRelease( hCRS );

  return mCrs.isValid();
}

QgsGdalProvider::~QgsGdalProvider()
{
  QgsDebugMsg( "QgsGdalProvider: deconstructing." );
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
  if ( !mValid ) return;
  mValid = false;

  GDALDereferenceDataset( mGdalBaseDataset );
  mGdalBaseDataset = NULL;

  GDALClose( mGdalDataset );
  mGdalDataset = NULL;
}

QString QgsGdalProvider::metadata()
{
  QString myMetadata ;
  myMetadata += QString( GDALGetDescription( GDALGetDatasetDriver( mGdalDataset ) ) );
  myMetadata += "<br>";
  myMetadata += QString( GDALGetMetadataItem( GDALGetDatasetDriver( mGdalDataset ), GDAL_DMD_LONGNAME, NULL ) );

  // my added code (MColetti)

  myMetadata += "<p class=\"glossy\">";
  myMetadata += tr( "Dataset Description" );
  myMetadata += "</p>\n";
  myMetadata += "<p>";
  myMetadata += QFile::decodeName( GDALGetDescription( mGdalDataset ) );
  myMetadata += "</p>\n";


  char ** GDALmetadata = GDALGetMetadata( mGdalDataset, NULL );

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
    GDALmetadata = GDALGetMetadata( gdalBand, NULL );

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
  myMetadata += tr( "Dimensions:" );
  myMetadata += "</p>\n";
  myMetadata += "<p>";
  myMetadata += tr( "X: %1 Y: %2 Bands: %3" )
                .arg( GDALGetRasterXSize( mGdalDataset ) )
                .arg( GDALGetRasterYSize( mGdalDataset ) )
                .arg( GDALGetRasterCount( mGdalDataset ) );
  myMetadata += "</p>\n";

  //just use the first band
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
  myMetadata += "</p>\n";

  if ( GDALGetGeoTransform( mGdalDataset, mGeoTransform ) != CE_None )
  {
    // if the raster does not have a valid transform we need to use
    // a pixel size of (1,-1), but GDAL returns (1,1)
    mGeoTransform[5] = -1;
  }
  else
  {
    myMetadata += "<p class=\"glossy\">";
    myMetadata += tr( "Origin:" );
    myMetadata += "</p>\n";
    myMetadata += "<p>";
    myMetadata += QString::number( mGeoTransform[0] );
    myMetadata += ",";
    myMetadata += QString::number( mGeoTransform[3] );
    myMetadata += "</p>\n";

    myMetadata += "<p class=\"glossy\">";
    myMetadata += tr( "Pixel Size:" );
    myMetadata += "</p>\n";
    myMetadata += "<p>";
    myMetadata += QString::number( mGeoTransform[1] );
    myMetadata += ",";
    myMetadata += QString::number( mGeoTransform[5] );
    myMetadata += "</p>\n";
  }

  return myMetadata;
}


// Not supported by GDAL
QImage* QgsGdalProvider::draw( QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight )
{
  QgsDebugMsg( "pixelWidth = "  + QString::number( pixelWidth ) );
  QgsDebugMsg( "pixelHeight = "  + QString::number( pixelHeight ) );
  QgsDebugMsg( "viewExtent: " + viewExtent.toString() );

  QImage *image = new QImage( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
  image->fill( QColor( Qt::gray ).rgb() );

  return image;
}


void QgsGdalProvider::readBlock( int theBandNo, int xBlock, int yBlock, void *block )
{
  // TODO!!!: Check data alignment!!! May it happen that nearest value which
  // is not nearest is assigned to an output cell???

  QgsDebugMsg( "Entered" );

  QgsDebugMsg( "yBlock = "  + QString::number( yBlock ) );

  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  //GDALReadBlock( myGdalBand, xBlock, yBlock, block );

  /* We have to read with correct data type consistent with other readBlock functions */
  int xOff = xBlock * mXBlockSize;
  int yOff = yBlock * mYBlockSize;
  GDALRasterIO( myGdalBand, GF_Read, xOff, yOff, mXBlockSize, mYBlockSize, block, mXBlockSize, mYBlockSize, ( GDALDataType ) mGdalDataType[theBandNo-1], 0, 0 );
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

  int dataSize = dataTypeSize( theBandNo ) / 8;

  // fill with null values
  QByteArray ba = noValueBytes( theBandNo );
  char *nodata = ba.data();
  char *block = ( char * ) theBlock;
  for ( int i = 0; i < thePixelWidth * thePixelHeight; i++ )
  {
    memcpy( block, nodata, dataSize );
    block += dataSize;
  }

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

  // Find top, bottom rows and  left, right column the raster extent covers
  // These are limits in target grid space
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

  QTime time;
  time.start();
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

  if ( xRes > srcXRes ) tmpWidth = width;
  if ( yRes > srcYRes ) tmpHeight = height;

  // Allocate temporary block
  char *tmpBlock = ( char * )malloc( dataSize * tmpWidth * tmpHeight );

  GDALRasterBandH gdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  GDALDataType type = ( GDALDataType )mGdalDataType[theBandNo-1];
  CPLErrorReset();
  CPLErr err = GDALRasterIO( gdalBand, GF_Read,
                             srcLeft, srcTop, srcWidth, srcHeight,
                             ( void * )tmpBlock,
                             tmpWidth, tmpHeight, type,
                             0, 0 );

  if ( err != CPLE_None )
  {
    QgsLogger::warning( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
    QgsDebugMsg( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
    free( tmpBlock );
    return;
  }

  QgsDebugMsg( QString( "GDALRasterIO time (ms): %1" ).arg( time.elapsed() ) );
  time.start();

  double tmpXMin = mExtent.xMinimum() + srcLeft * srcXRes;
  double tmpYMax = mExtent.yMaximum() + srcTop * srcYRes;
  double tmpXRes = srcWidth * srcXRes / tmpWidth;
  double tmpYRes = srcHeight * srcYRes / tmpHeight; // negative

  QgsDebugMsg( QString( "tmpXMin = %1 tmpYMax = %2 tmpWidth = %3 tmpHeight = %4" ).arg( tmpXMin ).arg( tmpYMax ).arg( tmpWidth ).arg( tmpHeight ) );

  for ( int row = 0; row < height; row++ )
  {
    double y = myRasterExtent.yMaximum() - ( row + 0.5 ) * yRes;
    int tmpRow = static_cast<int>( floor( -1. * ( tmpYMax - y ) / tmpYRes ) );

    char *srcRowBlock = tmpBlock + dataSize * tmpRow * tmpWidth;
    char *dstRowBlock = ( char * )theBlock + dataSize * ( top + row ) * thePixelWidth;
    for ( int col = 0; col < width; col++ )
    {
      // cell center
      double x = myRasterExtent.xMinimum() + ( col + 0.5 ) * xRes;
      // floor() is quite slow! Use just cast to int.
      int tmpCol = static_cast<int>(( x - tmpXMin ) / tmpXRes ) ;
      //QgsDebugMsg( QString( "row = %1 col = %2 tmpRow = %3 tmpCol = %4" ).arg(row).arg(col).arg(tmpRow).arg(tmpCol) );

      char *src = srcRowBlock + dataSize * tmpCol;
      char *dst = dstRowBlock + dataSize * ( left + col );
      memcpy( dst, src, dataSize );
    }
  }

  free( tmpBlock );
  QgsDebugMsg( QString( "resample time (ms): %1" ).arg( time.elapsed() ) );

  return;
}

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

  //myMemDsn.sprintf( "MEM:::DATAPOINTER=%lu,PIXELS=%d,LINES=%d,BANDS=1,DATATYPE=%s,PIXELOFFSET=0,LINEOFFSET=0,BANDOFFSET=0", ( long )theBlock, thePixelWidth, thePixelHeight,  GDALGetDataTypeName(( GDALDataType )mGdalDataType[theBandNo-1] ) );
  char szPointer[64];
  memset( szPointer, 0, sizeof( szPointer ) );
  CPLPrintPointer( szPointer, theBlock, sizeof( szPointer ) );

  myMemDsn.sprintf( "MEM:::DATAPOINTER=%s,PIXELS=%d,LINES=%d,BANDS=1,DATATYPE=%s,PIXELOFFSET=0,LINEOFFSET=0,BANDOFFSET=0", szPointer, thePixelWidth, thePixelHeight,  GDALGetDataTypeName(( GDALDataType )mGdalDataType[theBandNo-1] ) );

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
  myMemGeoTransform[0] = theExtent.xMinimum(); /* top left x */
  myMemGeoTransform[1] = theExtent.width() / thePixelWidth; /* w-e pixel resolution */
  myMemGeoTransform[2] = 0; /* rotation, 0 if image is "north up" */
  myMemGeoTransform[3] = theExtent.yMaximum(); /* top left y */
  myMemGeoTransform[4] = 0; /* rotation, 0 if image is "north up" */
  myMemGeoTransform[5] = -1. *  theExtent.height() / thePixelHeight; /* n-s pixel resolution */

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
    ( int * ) CPLMalloc( sizeof( int ) * myWarpOptions->nBandCount );
  myWarpOptions->panSrcBands[0] = theBandNo;
  myWarpOptions->panDstBands =
    ( int * ) CPLMalloc( sizeof( int ) * myWarpOptions->nBandCount );
  myWarpOptions->panDstBands[0] = 1;

  // TODO move here progressCallback and use it
  myWarpOptions->pfnProgress = GDALTermProgress;

  QgsDebugMsg( "src wkt: " +  QString( GDALGetProjectionRef( mGdalDataset ) ) );
  QgsDebugMsg( "dst wkt: " +  QString( GDALGetProjectionRef( myGdalMemDataset ) ) );
  myWarpOptions->pTransformerArg =
    GDALCreateGenImgProjTransformer(
      mGdalDataset,
      NULL,
      myGdalMemDataset,
      NULL,
      FALSE, 0.0, 1
    );
  /*
  myWarpOptions->pTransformerArg =
    GDALCreateGenImgProjTransformer2(
      mGdalDataset,
      myGdalMemDataset,
      NULL
    );
  */
  if ( !myWarpOptions->pTransformerArg )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QObject::tr( "Cannot GDALCreateGenImgProjTransformer: " )
                          + QString::fromUtf8( CPLGetLastErrorMsg() ) );
    return;

  }

  //CPLAssert( myWarpOptions->pTransformerArg  != NULL );
  myWarpOptions->pfnTransformer = GDALGenImgProjTransform;

  myWarpOptions->padfDstNoDataReal = ( double * ) CPLMalloc( myWarpOptions->nBandCount * sizeof( double ) );
  myWarpOptions->padfDstNoDataImag = ( double * ) CPLMalloc( myWarpOptions->nBandCount * sizeof( double ) );

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

double  QgsGdalProvider::noDataValue() const
{
  if ( mNoDataValue.size() > 0 )
  {
    return mNoDataValue[0];
  }
  return std::numeric_limits<int>::max(); // should not happen or be used
}

void QgsGdalProvider::computeMinMax( int theBandNo )
{
  QgsDebugMsg( QString( "theBandNo = %1 mMinMaxComputed = %2" ).arg( theBandNo ).arg( mMinMaxComputed[theBandNo-1] ) );
  if ( mMinMaxComputed[theBandNo-1] ) return;
  double GDALrange[2];
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  GDALComputeRasterMinMax( myGdalBand, 1, GDALrange ); //Approximate
  QgsDebugMsg( QString( "GDALrange[0] = %1 GDALrange[1] = %2" ).arg( GDALrange[0] ).arg( GDALrange[1] ) );
  mMinimum[theBandNo-1] = GDALrange[0];
  mMaximum[theBandNo-1] = GDALrange[1];
}

double  QgsGdalProvider::minimumValue( int theBandNo ) const
{
  QgsDebugMsg( QString( "theBandNo = %1" ).arg( theBandNo ) );
  //computeMinMax ( theBandNo );
  return  mMinimum[theBandNo-1];
}
double  QgsGdalProvider::maximumValue( int theBandNo ) const
{
  QgsDebugMsg( QString( "theBandNo = %1" ).arg( theBandNo ) );
  //computeMinMax ( theBandNo );
  return  mMaximum[theBandNo-1];
}

/**
 * @param theBandNumber the number of the band for which you want a color table
 * @param theList a pointer the object that will hold the color table
 * @return true of a color table was able to be read, false otherwise
 */
QList<QgsColorRampShader::ColorRampItem> QgsGdalProvider::colorTable( int theBandNumber )const
{
  QgsDebugMsg( "entered." );
  QList<QgsColorRampShader::ColorRampItem> ct;


  //Invalid band number, segfault prevention
  if ( 0 >= theBandNumber )
  {
    QgsDebugMsg( "Invalid parameter" );
    return ct;
  }

  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNumber );
  GDALColorTableH myGdalColorTable = GDALGetRasterColorTable( myGdalBand );

  if ( myGdalColorTable )
  {
    QgsDebugMsg( "Color table found" );
    int myEntryCount = GDALGetColorEntryCount( myGdalColorTable );
    GDALColorInterp myColorInterpretation =  GDALGetRasterColorInterpretation( myGdalBand );
    QgsDebugMsg( "Color Interpretation: " + QString::number(( int )myColorInterpretation ) );
    GDALPaletteInterp myPaletteInterpretation  = GDALGetPaletteInterpretation( myGdalColorTable );
    QgsDebugMsg( "Palette Interpretation: " + QString::number(( int )myPaletteInterpretation ) );

    const GDALColorEntry* myColorEntry = 0;
    for ( int myIterator = 0; myIterator < myEntryCount; myIterator++ )
    {
      myColorEntry = GDALGetColorEntry( myGdalColorTable, myIterator );

      if ( !myColorEntry )
      {
        continue;
      }
      else
      {
        //Branch on the color interpretation type
        if ( myColorInterpretation == GCI_GrayIndex )
        {
          QgsColorRampShader::ColorRampItem myColorRampItem;
          myColorRampItem.label = "";
          myColorRampItem.value = ( double )myIterator;
          myColorRampItem.color = QColor::fromRgb( myColorEntry->c1, myColorEntry->c1, myColorEntry->c1, myColorEntry->c4 );
          ct.append( myColorRampItem );
        }
        else if ( myColorInterpretation == GCI_PaletteIndex )
        {
          QgsColorRampShader::ColorRampItem myColorRampItem;
          myColorRampItem.label = "";
          myColorRampItem.value = ( double )myIterator;
          //Branch on palette interpretation
          if ( myPaletteInterpretation  == GPI_RGB )
          {
            myColorRampItem.color = QColor::fromRgb( myColorEntry->c1, myColorEntry->c2, myColorEntry->c3, myColorEntry->c4 );
          }
          else if ( myPaletteInterpretation  == GPI_CMYK )
          {
            myColorRampItem.color = QColor::fromCmyk( myColorEntry->c1, myColorEntry->c2, myColorEntry->c3, myColorEntry->c4 );
          }
          else if ( myPaletteInterpretation  == GPI_HLS )
          {
            myColorRampItem.color = QColor::fromHsv( myColorEntry->c1, myColorEntry->c3, myColorEntry->c2, myColorEntry->c4 );
          }
          else
          {
            myColorRampItem.color = QColor::fromRgb( myColorEntry->c1, myColorEntry->c1, myColorEntry->c1, myColorEntry->c4 );
          }
          ct.append( myColorRampItem );
        }
        else
        {
          QgsDebugMsg( "Color interpretation type not supported yet" );
          return ct;
        }
      }
    }
  }
  else
  {
    QgsDebugMsg( "No color table found for band " + QString::number( theBandNumber ) );
    return ct;
  }

  QgsDebugMsg( "Color table loaded successfully" );
  return ct;
}

QgsCoordinateReferenceSystem QgsGdalProvider::crs()
{
  QgsDebugMsg( "Entered" );
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

bool QgsGdalProvider::identify( const QgsPoint& thePoint, QMap<QString, QString>& theResults )
{
  QgsDebugMsg( "Entered" );
  if ( !mExtent.contains( thePoint ) )
  {
    // Outside the raster
    for ( int i = 1; i <= GDALGetRasterCount( mGdalDataset ); i++ )
    {
      theResults[ generateBandName( i )] = tr( "out of extent" );
    }
  }
  else
  {
    double x = thePoint.x();
    double y = thePoint.y();

    /* Calculate the row / column where the point falls */
    double xres = ( mExtent.xMaximum() - mExtent.xMinimum() ) / mWidth;
    double yres = ( mExtent.yMaximum() - mExtent.yMinimum() ) / mHeight;

    // Offset, not the cell index -> flor
    int col = ( int ) floor(( x - mExtent.xMinimum() ) / xres );
    int row = ( int ) floor(( mExtent.yMaximum() - y ) / yres );

    QgsDebugMsg( "row = " + QString::number( row ) + " col = " + QString::number( col ) );

    for ( int i = 1; i <= GDALGetRasterCount( mGdalDataset ); i++ )
    {
      GDALRasterBandH gdalBand = GDALGetRasterBand( mGdalDataset, i );
      double value;

      CPLErr err = GDALRasterIO( gdalBand, GF_Read, col, row, 1, 1,
                                 &value, 1, 1, GDT_Float64, 0, 0 );

      if ( err != CPLE_None )
      {
        QgsLogger::warning( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
      }

      //double value = readValue( data, type, 0 );
#ifdef QGISDEBUG
      QgsLogger::debug( "value", value, 1, __FILE__, __FUNCTION__, __LINE__ );
#endif
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

      //CPLFree( data );
    }
  }

  return true;
}

int QgsGdalProvider::capabilities() const
{
  int capability = QgsRasterDataProvider::Identify
                   | QgsRasterDataProvider::ExactResolution
                   | QgsRasterDataProvider::EstimatedMinimumMaximum
                   | QgsRasterDataProvider::BuildPyramids
                   | QgsRasterDataProvider::Histogram
                   | QgsRasterDataProvider::Size;
  return capability;
}

int QgsGdalProvider::dataTypeFormGdal( int theGdalDataType ) const
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

int QgsGdalProvider::srcDataType( int bandNo ) const
{
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, bandNo );
  GDALDataType myGdalDataType = GDALGetRasterDataType( myGdalBand );
  return dataTypeFormGdal( myGdalDataType );
}

int QgsGdalProvider::dataType( int bandNo ) const
{
  return dataTypeFormGdal( mGdalDataType[bandNo-1] );
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
  return GDALGetRasterColorInterpretation( myGdalBand );
}

void QgsGdalProvider::registerGdalDrivers()
{
  if ( GDALGetDriverCount() == 0 )
    GDALAllRegister();
}


bool QgsGdalProvider::isValid()
{
  QgsDebugMsg( QString( "valid = %1" ).arg( mValid ) );
  return mValid;
}

QString QgsGdalProvider::identifyAsText( const QgsPoint& point )
{
  return QString( "Not implemented" );
}

QString QgsGdalProvider::identifyAsHtml( const QgsPoint& point )
{
  return QString( "Not implemented" );
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
QStringList subLayers_( GDALDatasetH dataset )
{
  QStringList subLayers;

  char **metadata = GDALGetMetadata( dataset, "SUBDATASETS" );
  if ( metadata )
  {
    for ( int i = 0; metadata[i] != NULL; i++ )
    {
      QString layer = QString::fromUtf8( metadata[i] );

      int pos = layer.indexOf( "_NAME=" );
      if ( pos >= 0 )
      {
        subLayers << layer.mid( pos + 6 );
      }
    }
  }

  QgsDebugMsg( "sublayers:\n  " + subLayers.join( "\n  " ) );

  return subLayers;
}

void QgsGdalProvider::populateHistogram( int theBandNo,   QgsRasterBandStats & theBandStats, int theBinCount, bool theIgnoreOutOfRangeFlag, bool theHistogramEstimatedFlag )
{
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  //QgsRasterBandStats myRasterBandStats = bandStatistics( theBandNo );
  //calculate the histogram for this band
  //we assume that it only needs to be calculated if the length of the histogram
  //vector is not equal to the number of bins
  //i.e if the histogram has never previously been generated or the user has
  //selected a new number of bins.
  if ( theBandStats.histogramVector->size() != theBinCount ||
       theIgnoreOutOfRangeFlag != theBandStats.isHistogramOutOfRange ||
       theHistogramEstimatedFlag != theBandStats.isHistogramEstimated )
  {
    theBandStats.histogramVector->clear();
    theBandStats.isHistogramEstimated = theHistogramEstimatedFlag;
    theBandStats.isHistogramOutOfRange = theIgnoreOutOfRangeFlag;
    int *myHistogramArray = new int[theBinCount];

    /*
     *  CPLErr GDALRasterBand::GetHistogram (
     *          double       dfMin,
     *          double      dfMax,
     *          int     nBuckets,
     *          int *   panHistogram,
     *          int     bIncludeOutOfRange,
     *          int     bApproxOK,
     *          GDALProgressFunc    pfnProgress,
     *          void *      pProgressData
     *          )
     */
    double myerval = ( theBandStats.maximumValue - theBandStats.minimumValue ) / theBinCount;
    GDALGetRasterHistogram( myGdalBand, theBandStats.minimumValue - 0.1*myerval,
                            theBandStats.maximumValue + 0.1*myerval, theBinCount, myHistogramArray,
                            theIgnoreOutOfRangeFlag, theHistogramEstimatedFlag, progressCallback,
                            this ); //this is the arg for our custome gdal progress callback

    for ( int myBin = 0; myBin < theBinCount; myBin++ )
    {
      theBandStats.histogramVector->push_back( myHistogramArray[myBin] );
      QgsDebugMsg( "Added " + QString::number( myHistogramArray[myBin] ) + " to histogram vector" );
    }

  }
  QgsDebugMsg( ">>>>> Histogram vector now contains " + QString::number( theBandStats.histogramVector->size() ) +
               " elements" );
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
QString QgsGdalProvider::buildPyramids( QList<QgsRasterPyramid> const & theRasterPyramidList,
                                        QString const & theResamplingMethod, bool theTryInternalFlag )
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
  //first test if the file is writeable
  //QFileInfo myQFile( mDataSource );
  QFileInfo myQFile( dataSourceUri() );

  if ( !myQFile.isWritable() )
  {
    return "ERROR_WRITE_ACCESS";
  }

  if ( mGdalDataset != mGdalBaseDataset )
  {
    QgsLogger::warning( "Pyramid building not currently supported for 'warped virtual dataset'." );
    return "ERROR_VIRTUAL";
  }

  if ( theTryInternalFlag )
  {
    // libtiff < 4.0 has a bug that prevents safe building of overviews on JPEG compressed files
    // we detect libtiff < 4.0 by checking that BIGTIFF is not in the creation options of the GTiff driver
    // see https://trac.osgeo.org/qgis/ticket/1357
    const char* pszGTiffCreationOptions =
      GDALGetMetadataItem( GDALGetDriverByName( "GTiff" ), GDAL_DMD_CREATIONOPTIONLIST, "" );
    if ( strstr( pszGTiffCreationOptions, "BIGTIFF" ) == NULL )
    {
      QString myCompressionType = QString( GDALGetMetadataItem( mGdalDataset, "COMPRESSION", "IMAGE_STRUCTURE" ) );
      if ( "JPEG" == myCompressionType )
      {
        return "ERROR_JPEG_COMPRESSION";
      }
    }

    //close the gdal dataset and reopen it in read / write mode
    GDALClose( mGdalDataset );
    //mGdalBaseDataset = GDALOpen( QFile::encodeName( dataSourceUri() ).constData(), GA_Update );
    mGdalBaseDataset = GDALOpen( TO8F( dataSourceUri() ), GA_Update );

    // if the dataset couldn't be opened in read / write mode, tell the user
    if ( !mGdalBaseDataset )
    {
      //mGdalBaseDataset = GDALOpen( QFile::encodeName( mDataSource ).constData(), GA_ReadOnly );
      //mGdalBaseDataset = GDALOpen( QFile::encodeName( dataSourceUri()).constData(), GA_ReadOnly );
      mGdalBaseDataset = GDALOpen( TO8F( dataSourceUri() ), GA_ReadOnly );
      //Since we are not a virtual warped dataset, mGdalDataSet and mGdalBaseDataset are supposed to be the same
      mGdalDataset = mGdalBaseDataset;
      return "ERROR_WRITE_FORMAT";
    }
  }

  //
  // Iterate through the Raster Layer Pyramid Vector, building any pyramid
  // marked as exists in eaxh RasterPyramid struct.
  //
  CPLErr myError; //in case anything fails
  int myCount = 1;
  QList<QgsRasterPyramid>::const_iterator myRasterPyramidIterator;
  for ( myRasterPyramidIterator = theRasterPyramidList.begin();
        myRasterPyramidIterator != theRasterPyramidList.end();
        ++myRasterPyramidIterator )
  {
#ifdef QGISDEBUG
    QgsLogger::debug( "Build pyramids:: Level", ( *myRasterPyramidIterator ).level, 1, __FILE__, __FUNCTION__, __LINE__ );
    QgsLogger::debug( "x", ( *myRasterPyramidIterator ).xDim, 1, __FILE__, __FUNCTION__, __LINE__ );
    QgsLogger::debug( "y", ( *myRasterPyramidIterator ).yDim, 1, __FILE__, __FUNCTION__, __LINE__ );
    QgsLogger::debug( "exists :", ( *myRasterPyramidIterator ).exists,  1, __FILE__, __FUNCTION__, __LINE__ );
#endif
    if (( *myRasterPyramidIterator ).build )
    {
      QgsDebugMsg( "Building....." );
      //emit drawingProgress( myCount, myTotal );
      int myOverviewLevelsArray[1] = {( *myRasterPyramidIterator ).level };
      /* From : http://remotesensing.org/gdal/classGDALDataset.html#a23
       * pszResampling : one of "NEAREST", "AVERAGE" or "MODE" controlling the downsampling method applied.
       * nOverviews : number of overviews to build.
       * panOverviewList : the list of overview decimation factors to build.
       * nBand : number of bands to build overviews for in panBandList. Build for all bands if this is 0.
       * panBandList : list of band numbers.
       * pfnProgress : a function to call to report progress, or NULL.
       * pProgressData : application data to pass to the progress function.
       */
      //build the pyramid and show progress to console
      try
      {

        //build the pyramid and show progress to console
        //NOTE this (magphase) is disabled in the gui since it tends
        //to create corrupted images. The images can be repaired
        //by running one of the other resampling strategies below.
        //see ticket #284
        if ( theResamplingMethod == tr( "Average Magphase" ) )
        {
          myError = GDALBuildOverviews( mGdalBaseDataset, "MODE", 1, myOverviewLevelsArray, 0, NULL,
                                        progressCallback, this ); //this is the arg for the gdal progress callback
        }
        else if ( theResamplingMethod == tr( "Average" ) )

        {
          myError = GDALBuildOverviews( mGdalBaseDataset, "AVERAGE", 1, myOverviewLevelsArray, 0, NULL,
                                        progressCallback, this ); //this is the arg for the gdal progress callback
        }
        else // fall back to nearest neighbor
        {
          myError = GDALBuildOverviews( mGdalBaseDataset, "NEAREST", 1, myOverviewLevelsArray, 0, NULL,
                                        progressCallback, this ); //this is the arg for the gdal progress callback
        }

        if ( myError == CE_Failure || CPLGetLastErrorNo() == CPLE_NotSupported )
        {
          //something bad happenend
          //QString myString = QString (CPLGetLastError());
          GDALClose( mGdalBaseDataset );
          //mGdalBaseDataset = GDALOpen( QFile::encodeName( mDataSource ).constData(), GA_ReadOnly );
          //mGdalBaseDataset = GDALOpen( QFile::encodeName( dataSourceUri() ).constData(), GA_ReadOnly );
          mGdalBaseDataset = GDALOpen( TO8F( dataSourceUri() ), GA_ReadOnly );
          //Since we are not a virtual warped dataset, mGdalDataSet and mGdalBaseDataset are supposed to be the same
          mGdalDataset = mGdalBaseDataset;

          //emit drawingProgress( 0, 0 );
          return "FAILED_NOT_SUPPORTED";
        }
        else
        {
          //make sure the raster knows it has pyramids
          mHasPyramids = true;
        }
        myCount++;

      }
      catch ( CPLErr )
      {
        QgsLogger::warning( "Pyramid overview building failed!" );
      }
    }
  }

  QgsDebugMsg( "Pyramid overviews built" );
  if ( theTryInternalFlag )
  {
    //close the gdal dataset and reopen it in read only mode
    GDALClose( mGdalBaseDataset );
    mGdalBaseDataset = GDALOpen( TO8F( dataSourceUri() ), GA_ReadOnly );
    //Since we are not a virtual warped dataset, mGdalDataSet and mGdalBaseDataset are supposed to be the same
    mGdalDataset = mGdalBaseDataset;
  }

  //emit drawingProgress( 0, 0 );
  return NULL; // returning null on success
}

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
#ifdef QGISDEBUG
    QgsLogger::debug( "Pyramid", myRasterPyramid.level, 1, __FILE__, __FUNCTION__, __LINE__ );
    QgsLogger::debug( "xDim", myRasterPyramid.xDim, 1, __FILE__, __FUNCTION__, __LINE__ );
    QgsLogger::debug( "yDim", myRasterPyramid.yDim, 1, __FILE__, __FUNCTION__, __LINE__ );
#endif

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

QStringList QgsGdalProvider::subLayers() const
{
  return subLayers_( mGdalDataset );
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
  Builds the list of file filter strings to later be used by
  QgisApp::addRasterLayer()

  We query GDAL for a list of supported raster formats; we then build
  a list of file filter strings from that list.  We return a string
  that contains this list that is suitable for use in a
  QFileDialog::getOpenFileNames() call.

*/
QGISEXTERN void buildSupportedRasterFileFilter( QString & theFileFiltersString )
{
  QgsDebugMsg( "Entered" );
  // first get the GDAL driver manager
  QgsGdalProvider::registerGdalDrivers();

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

  GDALDriverH jp2Driver = NULL; // first JPEG2000 driver found

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

  // start with the default case
  theFileFiltersString = QObject::tr( "[GDAL] All files (*)" );

  for ( int i = 0; i < GDALGetDriverCount(); ++i )
  {
    myGdalDriver = GDALGetDriver( i );

    Q_CHECK_PTR( myGdalDriver );

    if ( !myGdalDriver )
    {
      QgsLogger::warning( "unable to get driver " + QString::number( i ) );
      continue;
    }
    // now we need to see if the driver is for something currently
    // supported; if not, we give it a miss for the next driver

    myGdalDriverDescription = GDALGetDescription( myGdalDriver );

    // QgsDebugMsg(QString("got driver string %1").arg(myGdalDriverDescription));

    myGdalDriverMetadata = GDALGetMetadata( myGdalDriver, NULL );

    // presumably we know we've run out of metadta if either the
    // address is 0, or the first character is null
    while ( myGdalDriverMetadata && '\0' != myGdalDriverMetadata[0] )
    {
      metadataTokens = QString( *myGdalDriverMetadata ).split( "=", QString::SkipEmptyParts );
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
        QString glob = "*." + myGdalDriverExtension.replace( "/", " *." );
        // Add only the first JP2 driver found to the filter list (it's the one GDAL uses)
        if ( myGdalDriverDescription == "JPEG2000" ||
             myGdalDriverDescription.startsWith( "JP2" ) ) // JP2ECW, JP2KAK, JP2MrSID
        {
          if ( jp2Driver )
            break; // skip if already found a JP2 driver

          jp2Driver = myGdalDriver;   // first JP2 driver found
          glob += " *.j2k";           // add alternate extension
        }
        else if ( myGdalDriverDescription == "GTiff" )
        {
          glob += " *.tiff";
        }
        else if ( myGdalDriverDescription == "JPEG" )
        {
          glob += " *.jpeg";
        }

        theFileFiltersString += ";;[GDAL] " + myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ")";

        break;            // ... to next driver, if any.
      }

      ++myGdalDriverMetadata;

    }                       // each metadata item

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
        QString glob = "*.dem";
        theFileFiltersString += ";;[GDAL] " + myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ")";
      }
      else if ( myGdalDriverDescription.startsWith( "DTED" ) )
      {
        // DTED use "*.dt0, *.dt1, *.dt2"
        QString glob = "*.dt0";
        glob += " *.dt1";
        glob += " *.dt2";
        theFileFiltersString += ";;[GDAL] " + myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ")";
      }
      else if ( myGdalDriverDescription.startsWith( "MrSID" ) )
      {
        // MrSID use "*.sid"
        QString glob = "*.sid";
        theFileFiltersString += ";;[GDAL] " + myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ")";
      }
      else
      {
        catchallFilter << QString( GDALGetDescription( myGdalDriver ) );
      }
    }

    myGdalDriverExtension = myGdalDriverLongName = "";  // reset for next driver

  }                           // each loaded GDAL driver

  QgsDebugMsg( "Raster filter list built: " + theFileFiltersString );
}                               // buildSupportedRasterFileFilter_()

QGISEXTERN bool isValidRasterFileName( QString const & theFileNameQString, QString & retErrMsg )
{
  GDALDatasetH myDataset;

  QgsGdalProvider::registerGdalDrivers();

  CPLErrorReset();

  //open the file using gdal making sure we have handled locale properly
  //myDataset = GDALOpen( QFile::encodeName( theFileNameQString ).constData(), GA_ReadOnly );
  myDataset = GDALOpen( TO8F( theFileNameQString ), GA_ReadOnly );
  if ( myDataset == NULL )
  {
    if ( CPLGetLastErrorNo() != CPLE_OpenFailed )
      retErrMsg = QString::fromUtf8( CPLGetLastErrorMsg() );
    return false;
  }
  else if ( GDALGetRasterCount( myDataset ) == 0 )
  {
    QStringList layers = subLayers_( myDataset );
    if ( layers.size() == 0 )
    {
      GDALClose( myDataset );
      myDataset = NULL;
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
