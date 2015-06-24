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

#include <qmath.h>
#include <QPair>
#include <QString>

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
  mCellSizeX = mCellSizeY = 0;
  mGridOffsetX = mGridOffsetY = 0;
  mXSize = mYSize = 0;

  mClipExtent[0] = mClipExtent[1] = mClipExtent[2] = mClipExtent[3] = 0;
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


void QgsAlignRaster::setParametersFromRaster( const QString& filename )
{
  setParametersFromRaster( RasterInfo( filename ) );
}

void QgsAlignRaster::setParametersFromRaster( const RasterInfo& rasterInfo )
{
  // use ref. layer to init input
  mCrsWkt = rasterInfo.crs();
  mCellSizeX = rasterInfo.cellSize().width();
  mCellSizeY = rasterInfo.cellSize().height();
  mGridOffsetX = rasterInfo.gridOffset().x();
  mGridOffsetY = rasterInfo.gridOffset().y();
}


bool QgsAlignRaster::determineTransformAndSize()
{
  double finalExtent[4] = { 0, 0, 0, 0 };

  // for each raster: determine their extent in projected cfg
  for ( int i = 0; i < mRasters.count(); ++i )
  {
    Item& r = mRasters[i];

    RasterInfo info( r.inputFilename );

    // Create a transformer that maps from source pixel/line coordinates
    // to destination georeferenced coordinates (not destination
    // pixel line).  We do that by omitting the destination dataset
    // handle (setting it to NULL).
    void* hTransformArg = GDALCreateGenImgProjTransformer( info.mDataset, info.mCrsWkt.constData(), NULL, mCrsWkt.constData(), FALSE, 0, 1 );
    if ( !hTransformArg )
    {
      mErrorMessage = QString( "GDALCreateGenImgProjTransformer failed.\n\n"
                               "Source WKT:\n%1\n\nDestination WKT:\n%2" )
                      .arg( QString::fromAscii( info.mCrsWkt ) )
                      .arg( QString::fromAscii( mCrsWkt ) );
      return false;
    }

    // Get approximate output georeferenced bounds and resolution for file.
    double adfDstGeoTransform[6];
    double extents[4];
    int nPixels = 0, nLines = 0;
    CPLErr eErr;
    eErr = GDALSuggestedWarpOutput2( info.mDataset,
                                     GDALGenImgProjTransform, hTransformArg,
                                     adfDstGeoTransform, &nPixels, &nLines, extents, 0 );
    GDALDestroyGenImgProjTransformer( hTransformArg );
    r.srcCellSizeInDestCRS = fabs( adfDstGeoTransform[1] * adfDstGeoTransform[5] );

    if ( eErr != CE_None )
    {
      mErrorMessage = QString( "GDALSuggestedWarpOutput2 failed.\n\n" + r.inputFilename );
      return false;
    }

    if ( finalExtent[0] == 0 && finalExtent[1] == 0 && finalExtent[2] == 0 && finalExtent[3] == 0 )
    {
      // initialize with the first layer
      finalExtent[0] = extents[0];
      finalExtent[1] = extents[1];
      finalExtent[2] = extents[2];
      finalExtent[3] = extents[3];
    }
    else
    {
      // use intersection of rects
      if ( extents[0] > finalExtent[0] ) finalExtent[0] = extents[0];
      if ( extents[1] > finalExtent[1] ) finalExtent[1] = extents[1];
      if ( extents[2] < finalExtent[2] ) finalExtent[2] = extents[2];
      if ( extents[3] < finalExtent[3] ) finalExtent[3] = extents[3];
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
  mXSize = floor_with_tolerance(( finalExtent[2] - originX ) / mCellSizeX );
  mYSize = floor_with_tolerance(( finalExtent[3] - originY ) / mCellSizeY );

  if ( mXSize <= 0 || mYSize <= 0 )
  {
    mErrorMessage = QObject::tr( "Configured inputs have no common intersecting area." );
    return false;
  }

  // build final geotransform...
  mGeoTransform[0] = originX;
  mGeoTransform[1] = mCellSizeX;
  mGeoTransform[2] = 0;
  mGeoTransform[3] = originY;
  mGeoTransform[4] = 0;
  mGeoTransform[5] = mCellSizeY;

  return true;
}


bool QgsAlignRaster::run()
{
  mErrorMessage.clear();

  // consider extent of all layers and setup geotransform and output grid size
  if ( !determineTransformAndSize() )
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
  qDebug( "wkt %s", mCrsWkt.constData() );
  qDebug( "w/h %d,%d", mXSize, mYSize );
  qDebug( "transform" );
  qDebug( "%6.2f %6.2f %6.2f", mGeoTransform[0], mGeoTransform[1], mGeoTransform[2] );
  qDebug( "%6.2f %6.2f %6.2f", mGeoTransform[3], mGeoTransform[4], mGeoTransform[5] );

  QgsRectangle e = transform_to_extent( mGeoTransform, mXSize, mYSize );
  qDebug( "extent %s", e.toString().toAscii().constData() );
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
  GDALSetProjection( hDstDS, mCrsWkt.constData() );
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


//----------


QgsAlignRaster::RasterInfo::RasterInfo( const QString& layerpath )
{
  mDataset = GDALOpen( layerpath.toLocal8Bit().constData(), GA_ReadOnly );
  if ( !mDataset )
    return;

  mXSize = GDALGetRasterXSize( mDataset );
  mYSize = GDALGetRasterYSize( mDataset );

  GDALGetGeoTransform( mDataset, mGeoTransform );

  // TODO: may be null or empty string
  mCrsWkt = QByteArray( GDALGetProjectionRef( mDataset ) );

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

void QgsAlignRaster::RasterInfo::dump() const
{
  qDebug( "---RASTER INFO------------------" );
  qDebug( "wkt %s", mCrsWkt.constData() );
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
  double value = err == CE_None ? pafScanline[0] : NAN;
  CPLFree( pafScanline );

  return value;
}
