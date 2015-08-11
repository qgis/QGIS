/***************************************************************************
  qgsalignraster.cpp
  --------------------------------------
  Date                 : June 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalignraster.h"

#include <gdalwarper.h>
#include <ogr_spatialref.h>
#include <cpl_conv.h>
#include <limits>

#include <qmath.h>
#include <QPair>
#include <QString>

#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"


static double ceil_with_tolerance( double value )
{
  if ( qAbs( value - qRound( value ) ) < 1e-6 )
    return qRound( value );
  else
    return qCeil( value );
}

static double floor_with_tolerance( double value )
{
  if ( qAbs( value - qRound( value ) ) < 1e-6 )
    return qRound( value );
  else
    return qFloor( value );
}

static double fmod_with_tolerance( double num, double denom )
{
  return num - floor_with_tolerance( num / denom ) * denom;
}


static QgsRectangle transform_to_extent( const double* geotransform, double xSize, double ySize )
{
  QgsRectangle r( geotransform[0],
                  geotransform[3],
                  geotransform[0] + geotransform[1] * xSize,
                  geotransform[3] + geotransform[5] * ySize );
  r.normalize();
  return r;
}


static int CPL_STDCALL _progress( double dfComplete, const char* pszMessage, void* pProgressArg )
{
  Q_UNUSED( pszMessage );

  QgsAlignRaster::ProgressHandler* handler = (( QgsAlignRaster* ) pProgressArg )->progressHandler();
  if ( handler )
    return handler->progress( dfComplete );
  else
    return true;
}


static CPLErr rescalePreWarpChunkProcessor( void* pKern, void* pArg )
{
  GDALWarpKernel* kern = ( GDALWarpKernel* ) pKern;
  double cellsize = (( double* )pArg )[0];

  for ( int nBand = 0; nBand < kern->nBands; ++nBand )
  {
    float* bandData = ( float * ) kern->papabySrcImage[nBand];
    for ( int nLine = 0; nLine < kern->nSrcYSize; ++nLine )
    {
      for ( int nPixel = 0; nPixel < kern->nSrcXSize; ++nPixel )
      {
        bandData[nPixel] /= cellsize;
      }
      bandData += kern->nSrcXSize;
    }
  }
  return CE_None;
}


static CPLErr rescalePostWarpChunkProcessor( void* pKern, void* pArg )
{
  GDALWarpKernel* kern = ( GDALWarpKernel* ) pKern;
  double cellsize = (( double* )pArg )[1];

  for ( int nBand = 0; nBand < kern->nBands; ++nBand )
  {
    float* bandData = ( float * ) kern->papabyDstImage[nBand];
    for ( int nLine = 0; nLine < kern->nDstYSize; ++nLine )
    {
      for ( int nPixel = 0; nPixel < kern->nDstXSize; ++nPixel )
      {
        bandData[nPixel] *= cellsize;
      }
      bandData += kern->nDstXSize;
    }
  }
  return CE_None;
}



QgsAlignRaster::QgsAlignRaster()
    : mProgressHandler( 0 )
{
  // parameters
  mCellSizeX = mCellSizeY = 0;
  mGridOffsetX = mGridOffsetY = 0;
  mClipExtent[0] = mClipExtent[1] = mClipExtent[2] = mClipExtent[3] = 0;

  // derived variables
  mXSize = mYSize = 0;
  for ( int i = 0; i < 6; ++i )
    mGeoTransform[i] = 0;
}

void QgsAlignRaster::setClipExtent( double xmin, double ymin, double xmax, double ymax )
{
  mClipExtent[0] = xmin;
  mClipExtent[1] = ymin;
  mClipExtent[2] = xmax;
  mClipExtent[3] = ymax;
}

void QgsAlignRaster::setClipExtent( const QgsRectangle& extent )
{
  setClipExtent( extent.xMinimum(), extent.yMinimum(),
                 extent.xMaximum(), extent.yMaximum() );
}

QgsRectangle QgsAlignRaster::clipExtent() const
{
  return QgsRectangle( mClipExtent[0], mClipExtent[1],
                       mClipExtent[2], mClipExtent[3] );
}


bool QgsAlignRaster::setParametersFromRaster( const QString& filename, const QString& destWkt, QSizeF customCellSize, QPointF customGridOffset )
{
  return setParametersFromRaster( RasterInfo( filename ), destWkt, customCellSize, customGridOffset );
}

bool QgsAlignRaster::setParametersFromRaster( const RasterInfo& rasterInfo, const QString& customCRSWkt, QSizeF customCellSize, QPointF customGridOffset )
{
  if ( customCRSWkt.isEmpty() || customCRSWkt == rasterInfo.crs() )
  {
    // use ref. layer to init input
    mCrsWkt = rasterInfo.crs();

    if ( !customCellSize.isValid() )
    {
      mCellSizeX = rasterInfo.cellSize().width();
      mCellSizeY = rasterInfo.cellSize().height();
    }
    else
    {
      mCellSizeX = customCellSize.width();
      mCellSizeY = customCellSize.height();
    }

    if ( customGridOffset.x() < 0 || customGridOffset.y() < 0 )
    {
      if ( !customCellSize.isValid() )
      {
        // using original raster's grid offset to be aligned with origin
        mGridOffsetX = rasterInfo.gridOffset().x();
        mGridOffsetY = rasterInfo.gridOffset().y();
      }
      else
      {
        // if using custom cell size: offset so that we are aligned
        // with the original raster's origin point
        mGridOffsetX = fmod_with_tolerance( rasterInfo.origin().x(), customCellSize.width() );
        mGridOffsetY = fmod_with_tolerance( rasterInfo.origin().y(), customCellSize.height() );
      }
    }
    else
    {
      mGridOffsetX = customGridOffset.x();
      mGridOffsetY = customGridOffset.x();
    }
  }
  else
  {
    QSizeF cs;
    QPointF go;
    if ( !suggestedWarpOutput( rasterInfo, customCRSWkt, &cs, &go ) )
    {
      mCrsWkt = "_error_";
      mCellSizeX = mCellSizeY = 0;
      mGridOffsetX = mGridOffsetY = 0;
      return false;
    }

    mCrsWkt = customCRSWkt;

    if ( !customCellSize.isValid() )
    {
      mCellSizeX = cs.width();
      mCellSizeY = cs.height();
    }
    else
    {
      mCellSizeX = customCellSize.width();
      mCellSizeY = customCellSize.height();
    }

    if ( customGridOffset.x() < 0 || customGridOffset.y() < 0 )
    {
      mGridOffsetX = go.x();
      mGridOffsetY = go.y();
    }
    else
    {
      mGridOffsetX = customGridOffset.x();
      mGridOffsetY = customGridOffset.x();
    }
  }
  return true;
}


bool QgsAlignRaster::checkInputParameters()
{
  mErrorMessage.clear();

  if ( mCrsWkt == "_error_" )
  {
    mErrorMessage = QObject::tr( "Unable to reproject." );
    return false;
  }

  if ( mCellSizeX == 0 || mCellSizeY == 0 )
  {
    mErrorMessage = QObject::tr( "Cell size must not be zero." );
    return false;
  }

  mXSize = mYSize = 0;
  for ( int i = 0; i < 6; ++i )
    mGeoTransform[i] = 0;

  double finalExtent[4] = { 0, 0, 0, 0 };

  // for each raster: determine their extent in projected cfg
  for ( int i = 0; i < mRasters.count(); ++i )
  {
    Item& r = mRasters[i];

    RasterInfo info( r.inputFilename );

    QSizeF cs;
    QgsRectangle extent;
    if ( !suggestedWarpOutput( info, mCrsWkt, &cs, 0, &extent ) )
    {
      mErrorMessage = QString( "Failed to get suggested warp output.\n\n"
                               "File:\n%1\n\n"
                               "Source WKT:\n%2\n\nDestination WKT:\n%3" )
                      .arg( r.inputFilename )
                      .arg( info.mCrsWkt )
                      .arg( mCrsWkt );
      return false;
    }

    r.srcCellSizeInDestCRS = cs.width() * cs.height();

    if ( finalExtent[0] == 0 && finalExtent[1] == 0 && finalExtent[2] == 0 && finalExtent[3] == 0 )
    {
      // initialize with the first layer
      finalExtent[0] = extent.xMinimum();
      finalExtent[1] = extent.yMinimum();
      finalExtent[2] = extent.xMaximum();
      finalExtent[3] = extent.yMaximum();
    }
    else
    {
      // use intersection of rects
      if ( extent.xMinimum() > finalExtent[0] ) finalExtent[0] = extent.xMinimum();
      if ( extent.yMinimum() > finalExtent[1] ) finalExtent[1] = extent.yMinimum();
      if ( extent.xMaximum() < finalExtent[2] ) finalExtent[2] = extent.xMaximum();
      if ( extent.yMaximum() < finalExtent[3] ) finalExtent[3] = extent.yMaximum();
    }
  }

  // count in extra clip extent (if present)
  // 1. align requested rect to the grid - extend the rect if necessary
  // 2. intersect with clip extent with final extent

  if ( !( mClipExtent[0] == 0 && mClipExtent[1] == 0 && mClipExtent[2] == 0 && mClipExtent[3] == 0 ) )
  {
    // extend clip extent to grid
    double clipX0 = floor_with_tolerance(( mClipExtent[0] - mGridOffsetX ) / mCellSizeX ) * mCellSizeX + mGridOffsetX;
    double clipY0 = floor_with_tolerance(( mClipExtent[1] - mGridOffsetY ) / mCellSizeY ) * mCellSizeY + mGridOffsetY;
    double clipX1 = ceil_with_tolerance(( mClipExtent[2] - clipX0 ) / mCellSizeX ) * mCellSizeX + clipX0;
    double clipY1 = ceil_with_tolerance(( mClipExtent[3] - clipY0 ) / mCellSizeY ) * mCellSizeY + clipY0;
    if ( clipX0 > finalExtent[0] ) finalExtent[0] = clipX0;
    if ( clipY0 > finalExtent[1] ) finalExtent[1] = clipY0;
    if ( clipX1 < finalExtent[2] ) finalExtent[2] = clipX1;
    if ( clipY1 < finalExtent[3] ) finalExtent[3] = clipY1;
  }

  // align to grid - shrink the rect if necessary
  // output raster grid configuration (with no rotation/shear)
  // ... and raster width/height

  double originX = ceil_with_tolerance(( finalExtent[0] - mGridOffsetX ) / mCellSizeX ) * mCellSizeX + mGridOffsetX;;
  double originY = ceil_with_tolerance(( finalExtent[1] - mGridOffsetY ) / mCellSizeY ) * mCellSizeY + mGridOffsetY;
  int xSize = floor_with_tolerance(( finalExtent[2] - originX ) / mCellSizeX );
  int ySize = floor_with_tolerance(( finalExtent[3] - originY ) / mCellSizeY );

  if ( xSize <= 0 || ySize <= 0 )
  {
    mErrorMessage = QObject::tr( "No common intersecting area." );
    return false;
  }

  mXSize = xSize;
  mYSize = ySize;

  // build final geotransform...
  mGeoTransform[0] = originX;
  mGeoTransform[1] = mCellSizeX;
  mGeoTransform[2] = 0;
  mGeoTransform[3] = originY + ( mCellSizeY * ySize );
  mGeoTransform[4] = 0;
  mGeoTransform[5] = -mCellSizeY;

  return true;
}


QSize QgsAlignRaster::alignedRasterSize() const
{
  return QSize( mXSize, mYSize );
}

QgsRectangle QgsAlignRaster::alignedRasterExtent() const
{
  return transform_to_extent( mGeoTransform, mXSize, mYSize );
}


bool QgsAlignRaster::run()
{
  mErrorMessage.clear();

  // consider extent of all layers and setup geotransform and output grid size
  if ( !checkInputParameters() )
    return false;

  //dump();

  foreach ( const Item& r, mRasters )
  {
    if ( !createAndWarp( r ) )
      return false;
  }
  return true;
}


void QgsAlignRaster::dump() const
{
  qDebug( "---ALIGN------------------" );
  qDebug( "wkt %s", mCrsWkt.toAscii().constData() );
  qDebug( "w/h %d,%d", mXSize, mYSize );
  qDebug( "transform" );
  qDebug( "%6.2f %6.2f %6.2f", mGeoTransform[0], mGeoTransform[1], mGeoTransform[2] );
  qDebug( "%6.2f %6.2f %6.2f", mGeoTransform[3], mGeoTransform[4], mGeoTransform[5] );

  QgsRectangle e = transform_to_extent( mGeoTransform, mXSize, mYSize );
  qDebug( "extent %s", e.toString().toAscii().constData() );
}

int QgsAlignRaster::suggestedReferenceLayer() const
{
  int bestIndex = -1;
  double bestCellArea = qInf();
  QSizeF cs;
  int i = 0;

  // using WGS84 as a destination CRS... but maybe some projected CRS
  // would be a better a choice to more accurately compute areas?
  // (Why earth is not flat???)
  QgsCoordinateReferenceSystem destCRS( "EPSG:4326" );
  QString destWkt = destCRS.toWkt();

  foreach ( const Item& raster, mRasters )
  {
    if ( !suggestedWarpOutput( RasterInfo( raster.inputFilename ), destWkt, &cs ) )
      return false;

    double cellArea = cs.width() * cs.height();
    if ( cellArea < bestCellArea )
    {
      bestCellArea = cellArea;
      bestIndex = i;
    }
    ++i;
  }

  return bestIndex;
}


bool QgsAlignRaster::createAndWarp( const Item& raster )
{
  GDALDriverH hDriver = GDALGetDriverByName( "GTiff" );
  if ( !hDriver )
  {
    mErrorMessage = QString( "GDALGetDriverByName(GTiff) failed." );
    return false;
  }

  // Open the source file.
  GDALDatasetH hSrcDS = GDALOpen( raster.inputFilename.toLocal8Bit().constData(), GA_ReadOnly );
  if ( !hSrcDS )
  {
    mErrorMessage = QObject::tr( "Unable to open input file: " ) + raster.inputFilename;
    return false;
  }

  // Create output with same datatype as first input band.

  int bandCount = GDALGetRasterCount( hSrcDS );
  GDALDataType eDT = GDALGetRasterDataType( GDALGetRasterBand( hSrcDS, 1 ) );

  // Create the output file.
  GDALDatasetH hDstDS;
  hDstDS = GDALCreate( hDriver, raster.outputFilename.toLocal8Bit().constData(), mXSize, mYSize,
                       bandCount, eDT, NULL );
  if ( !hDstDS )
  {
    GDALClose( hSrcDS );
    mErrorMessage = QObject::tr( "Unable to create output file: " ) + raster.outputFilename;
    return false;
  }

  // Write out the projection definition.
  GDALSetProjection( hDstDS, mCrsWkt.toAscii().constData() );
  GDALSetGeoTransform( hDstDS, ( double* )mGeoTransform );

  // Copy the color table, if required.
  GDALColorTableH hCT = GDALGetRasterColorTable( GDALGetRasterBand( hSrcDS, 1 ) );
  if ( hCT != NULL )
    GDALSetRasterColorTable( GDALGetRasterBand( hDstDS, 1 ), hCT );

  // -----------------------------------------------------------------------

  // Setup warp options.
  GDALWarpOptions* psWarpOptions = GDALCreateWarpOptions();
  psWarpOptions->hSrcDS = hSrcDS;
  psWarpOptions->hDstDS = hDstDS;

  psWarpOptions->nBandCount = GDALGetRasterCount( hSrcDS );
  psWarpOptions->panSrcBands = ( int * ) CPLMalloc( sizeof( int ) * psWarpOptions->nBandCount );
  psWarpOptions->panDstBands = ( int * ) CPLMalloc( sizeof( int ) * psWarpOptions->nBandCount );
  for ( int i = 0; i < psWarpOptions->nBandCount; ++i )
  {
    psWarpOptions->panSrcBands[i] = i + 1;
    psWarpOptions->panDstBands[i] = i + 1;
  }

  psWarpOptions->eResampleAlg = ( GDALResampleAlg ) raster.resampleMethod;

  // our progress function
  psWarpOptions->pfnProgress = _progress;
  psWarpOptions->pProgressArg = this;

  // Establish reprojection transformer.
  psWarpOptions->pTransformerArg =
    GDALCreateGenImgProjTransformer( hSrcDS, GDALGetProjectionRef( hSrcDS ),
                                     hDstDS, GDALGetProjectionRef( hDstDS ),
                                     FALSE, 0.0, 1 );
  psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

  double rescaleArg[2];
  if ( raster.rescaleValues )
  {
    rescaleArg[0] = raster.srcCellSizeInDestCRS; // source cell size
    rescaleArg[1] = mCellSizeX * mCellSizeY;  // destination cell size
    psWarpOptions->pfnPreWarpChunkProcessor = rescalePreWarpChunkProcessor;
    psWarpOptions->pfnPostWarpChunkProcessor = rescalePostWarpChunkProcessor;
    psWarpOptions->pPreWarpProcessorArg = rescaleArg;
    psWarpOptions->pPostWarpProcessorArg = rescaleArg;
    // force use of float32 data type as that is what our pre/post-processor uses
    psWarpOptions->eWorkingDataType = GDT_Float32;
  }

  // Initialize and execute the warp operation.
  GDALWarpOperation oOperation;
  oOperation.Initialize( psWarpOptions );
  oOperation.ChunkAndWarpImage( 0, 0, mXSize, mYSize );

  GDALDestroyGenImgProjTransformer( psWarpOptions->pTransformerArg );
  GDALDestroyWarpOptions( psWarpOptions );

  GDALClose( hDstDS );
  GDALClose( hSrcDS );
  return true;
}

bool QgsAlignRaster::suggestedWarpOutput( const QgsAlignRaster::RasterInfo& info, const QString& destWkt, QSizeF* cellSize, QPointF* gridOffset, QgsRectangle* rect )
{
  // Create a transformer that maps from source pixel/line coordinates
  // to destination georeferenced coordinates (not destination
  // pixel line).  We do that by omitting the destination dataset
  // handle (setting it to NULL).
  void* hTransformArg = GDALCreateGenImgProjTransformer( info.mDataset, info.mCrsWkt.toAscii().constData(), NULL, destWkt.toAscii().constData(), FALSE, 0, 1 );
  if ( !hTransformArg )
    return false;

  // Get approximate output georeferenced bounds and resolution for file.
  double adfDstGeoTransform[6];
  double extents[4];
  int nPixels = 0, nLines = 0;
  CPLErr eErr;
  eErr = GDALSuggestedWarpOutput2( info.mDataset,
                                   GDALGenImgProjTransform, hTransformArg,
                                   adfDstGeoTransform, &nPixels, &nLines, extents, 0 );
  GDALDestroyGenImgProjTransformer( hTransformArg );

  if ( eErr != CE_None )
    return false;

  QSizeF cs( qAbs( adfDstGeoTransform[1] ), qAbs( adfDstGeoTransform[5] ) );

  if ( rect )
    *rect = QgsRectangle( extents[0], extents[1], extents[2], extents[3] );
  if ( cellSize )
    *cellSize = cs;
  if ( gridOffset )
    *gridOffset = QPointF( fmod_with_tolerance( adfDstGeoTransform[0], cs.width() ),
                           fmod_with_tolerance( adfDstGeoTransform[3], cs.height() ) );
  return true;
}


//----------


QgsAlignRaster::RasterInfo::RasterInfo( const QString& layerpath )
    : mXSize( 0 )
    , mYSize( 0 )
    , mBandCnt( 0 )
{
  mDataset = GDALOpen( layerpath.toLocal8Bit().constData(), GA_ReadOnly );
  if ( !mDataset )
    return;

  mXSize = GDALGetRasterXSize( mDataset );
  mYSize = GDALGetRasterYSize( mDataset );

  ( void ) GDALGetGeoTransform( mDataset, mGeoTransform );

  // TODO: may be null or empty string
  mCrsWkt = QString::fromAscii( GDALGetProjectionRef( mDataset ) );

  mBandCnt = GDALGetBandNumber( mDataset );
}

QgsAlignRaster::RasterInfo::~RasterInfo()
{
  if ( mDataset )
    GDALClose( mDataset );
}

QSizeF QgsAlignRaster::RasterInfo::cellSize() const
{
  return QSizeF( qAbs( mGeoTransform[1] ), qAbs( mGeoTransform[5] ) );
}

QPointF QgsAlignRaster::RasterInfo::gridOffset() const
{
  return QPointF( fmod_with_tolerance( mGeoTransform[0], cellSize().width() ),
                  fmod_with_tolerance( mGeoTransform[3], cellSize().height() ) );
}

QgsRectangle QgsAlignRaster::RasterInfo::extent() const
{
  return transform_to_extent( mGeoTransform, mXSize, mYSize );
}

QPointF QgsAlignRaster::RasterInfo::origin() const
{
  return QPointF( mGeoTransform[0], mGeoTransform[3] );
}

void QgsAlignRaster::RasterInfo::dump() const
{
  qDebug( "---RASTER INFO------------------" );
  qDebug( "wkt %s", mCrsWkt.toAscii().constData() );
  qDebug( "w/h %d,%d", mXSize, mYSize );
  qDebug( "cell x/y %f,%f", cellSize().width(), cellSize().width() );

  QgsRectangle r = extent();
  qDebug( "extent %s", r.toString().toAscii().constData() );

  qDebug( "transform" );
  qDebug( "%6.2f %6.2f %6.2f", mGeoTransform[0], mGeoTransform[1], mGeoTransform[2] );
  qDebug( "%6.2f %6.2f %6.2f", mGeoTransform[3], mGeoTransform[4], mGeoTransform[5] );
}

double QgsAlignRaster::RasterInfo::identify( double mx, double my )
{
  GDALRasterBandH hBand = GDALGetRasterBand( mDataset, 1 );

  // must not be rotated in order for this to work
  int px = int(( mx - mGeoTransform[0] ) / mGeoTransform[1] );
  int py = int(( my - mGeoTransform[3] ) / mGeoTransform[5] );

  float* pafScanline = ( float * ) CPLMalloc( sizeof( float ) );
  CPLErr err = GDALRasterIO( hBand, GF_Read, px, py, 1, 1,
                             pafScanline, 1, 1, GDT_Float32, 0, 0 );
  double value = err == CE_None ? pafScanline[0] : std::numeric_limits<double>::quiet_NaN();
  CPLFree( pafScanline );

  return value;
}
