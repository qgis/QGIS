/***************************************************************************
                          qgsninecellfilter.h  -  description
                             -------------------
    begin                : August 6th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsninecellfilter.h"
#include "qgslogger.h"
#include "cpl_string.h"
#include <QProgressDialog>
#include <QFile>

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8F(x) (x).toUtf8().constData()
#else
#define TO8F(x) QFile::encodeName( x ).constData()
#endif

QgsNineCellFilter::QgsNineCellFilter( const QString& inputFile, const QString& outputFile, const QString& outputFormat )
    : mInputFile( inputFile )
    , mOutputFile( outputFile )
    , mOutputFormat( outputFormat )
    , mCellSizeX( -1.0 )
    , mCellSizeY( -1.0 )
    , mInputNodataValue( -1.0 )
    , mOutputNodataValue( -1.0 )
    , mZFactor( 1.0 )
{

}

QgsNineCellFilter::QgsNineCellFilter()
    : mCellSizeX( -1.0 )
    , mCellSizeY( -1.0 )
    , mInputNodataValue( -1.0 )
    , mOutputNodataValue( -1.0 )
    , mZFactor( 1.0 )
{
}

QgsNineCellFilter::~QgsNineCellFilter()
{

}

int QgsNineCellFilter::processRaster( QProgressDialog* p )
{
  GDALAllRegister();

  //open input file
  int xSize, ySize;
  GDALDatasetH  inputDataset = openInputFile( xSize, ySize );
  if ( !inputDataset )
  {
    return 1; //opening of input file failed
  }

  //output driver
  GDALDriverH outputDriver = openOutputDriver();
  if ( !outputDriver )
  {
    return 2;
  }

  GDALDatasetH outputDataset = openOutputFile( inputDataset, outputDriver );
  if ( !outputDataset )
  {
    return 3; //create operation on output file failed
  }

  //open first raster band for reading (operation is only for single band raster)
  GDALRasterBandH rasterBand = GDALGetRasterBand( inputDataset, 1 );
  if ( !rasterBand )
  {
    GDALClose( inputDataset );
    GDALClose( outputDataset );
    return 4;
  }
  mInputNodataValue = GDALGetRasterNoDataValue( rasterBand, nullptr );

  GDALRasterBandH outputRasterBand = GDALGetRasterBand( outputDataset, 1 );
  if ( !outputRasterBand )
  {
    GDALClose( inputDataset );
    GDALClose( outputDataset );
    return 5;
  }
  //try to set -9999 as nodata value
  GDALSetRasterNoDataValue( outputRasterBand, -9999 );
  mOutputNodataValue = GDALGetRasterNoDataValue( outputRasterBand, nullptr );

  if ( ySize < 3 ) //we require at least three rows (should be true for most datasets)
  {
    GDALClose( inputDataset );
    GDALClose( outputDataset );
    return 6;
  }

  //keep only three scanlines in memory at a time
  float* scanLine1 = ( float * ) CPLMalloc( sizeof( float ) * xSize );
  float* scanLine2 = ( float * ) CPLMalloc( sizeof( float ) * xSize );
  float* scanLine3 = ( float * ) CPLMalloc( sizeof( float ) * xSize );

  float* resultLine = ( float * ) CPLMalloc( sizeof( float ) * xSize );

  if ( p )
  {
    p->setMaximum( ySize );
  }

  //values outside the layer extent (if the 3x3 window is on the border) are sent to the processing method as (input) nodata values
  for ( int i = 0; i < ySize; ++i )
  {
    if ( p )
    {
      p->setValue( i );
    }

    if ( p && p->wasCanceled() )
    {
      break;
    }

    if ( i == 0 )
    {
      //fill scanline 1 with (input) nodata for the values above the first row and feed scanline2 with the first row
      for ( int a = 0; a < xSize; ++a )
      {
        scanLine1[a] = mInputNodataValue;
      }
      if ( GDALRasterIO( rasterBand, GF_Read, 0, 0, xSize, 1, scanLine2, xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
      {
        QgsDebugMsg( "Raster IO Error" );
      }
    }
    else
    {
      //normally fetch only scanLine3 and release scanline 1 if we move forward one row
      CPLFree( scanLine1 );
      scanLine1 = scanLine2;
      scanLine2 = scanLine3;
      scanLine3 = ( float * ) CPLMalloc( sizeof( float ) * xSize );
    }

    if ( i == ySize - 1 ) //fill the row below the bottom with nodata values
    {
      for ( int a = 0; a < xSize; ++a )
      {
        scanLine3[a] = mInputNodataValue;
      }
    }
    else
    {
      if ( GDALRasterIO( rasterBand, GF_Read, 0, i + 1, xSize, 1, scanLine3, xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
      {
        QgsDebugMsg( "Raster IO Error" );
      }
    }

    for ( int j = 0; j < xSize; ++j )
    {
      if ( j == 0 )
      {
        resultLine[j] = processNineCellWindow( &mInputNodataValue, &scanLine1[j], &scanLine1[j+1], &mInputNodataValue, &scanLine2[j],
                                               &scanLine2[j+1], &mInputNodataValue, &scanLine3[j], &scanLine3[j+1] );
      }
      else if ( j == xSize - 1 )
      {
        resultLine[j] = processNineCellWindow( &scanLine1[j-1], &scanLine1[j], &mInputNodataValue, &scanLine2[j-1], &scanLine2[j],
                                               &mInputNodataValue, &scanLine3[j-1], &scanLine3[j], &mInputNodataValue );
      }
      else
      {
        resultLine[j] = processNineCellWindow( &scanLine1[j-1], &scanLine1[j], &scanLine1[j+1], &scanLine2[j-1], &scanLine2[j],
                                               &scanLine2[j+1], &scanLine3[j-1], &scanLine3[j], &scanLine3[j+1] );
      }
    }

    if ( GDALRasterIO( outputRasterBand, GF_Write, 0, i, xSize, 1, resultLine, xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
    {
      QgsDebugMsg( "Raster IO Error" );
    }
  }

  if ( p )
  {
    p->setValue( ySize );
  }

  CPLFree( resultLine );
  CPLFree( scanLine1 );
  CPLFree( scanLine2 );
  CPLFree( scanLine3 );

  GDALClose( inputDataset );

  if ( p && p->wasCanceled() )
  {
    //delete the dataset without closing (because it is faster)
    GDALDeleteDataset( outputDriver, TO8F( mOutputFile ) );
    return 7;
  }
  GDALClose( outputDataset );

  return 0;
}

GDALDatasetH QgsNineCellFilter::openInputFile( int& nCellsX, int& nCellsY )
{
  GDALDatasetH inputDataset = GDALOpen( TO8F( mInputFile ), GA_ReadOnly );
  if ( inputDataset )
  {
    nCellsX = GDALGetRasterXSize( inputDataset );
    nCellsY = GDALGetRasterYSize( inputDataset );

    //we need at least one band
    if ( GDALGetRasterCount( inputDataset ) < 1 )
    {
      GDALClose( inputDataset );
      return nullptr;
    }
  }
  return inputDataset;
}

GDALDriverH QgsNineCellFilter::openOutputDriver()
{
  char **driverMetadata;

  //open driver
  GDALDriverH outputDriver = GDALGetDriverByName( mOutputFormat.toLocal8Bit().data() );

  if ( !outputDriver )
  {
    return outputDriver; //return nullptr, driver does not exist
  }

  driverMetadata = GDALGetMetadata( outputDriver, nullptr );
  if ( !CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE, false ) )
  {
    return nullptr; //driver exist, but it does not support the create operation
  }

  return outputDriver;
}

GDALDatasetH QgsNineCellFilter::openOutputFile( GDALDatasetH inputDataset, GDALDriverH outputDriver )
{
  if ( !inputDataset )
  {
    return nullptr;
  }

  int xSize = GDALGetRasterXSize( inputDataset );
  int ySize = GDALGetRasterYSize( inputDataset );

  //open output file
  char **papszOptions = nullptr;
  GDALDatasetH outputDataset = GDALCreate( outputDriver, TO8F( mOutputFile ), xSize, ySize, 1, GDT_Float32, papszOptions );
  if ( !outputDataset )
  {
    return outputDataset;
  }

  //get geotransform from inputDataset
  double geotransform[6];
  if ( GDALGetGeoTransform( inputDataset, geotransform ) != CE_None )
  {
    GDALClose( outputDataset );
    return nullptr;
  }
  GDALSetGeoTransform( outputDataset, geotransform );

  //make sure mCellSizeX and mCellSizeY are always > 0
  mCellSizeX = geotransform[1];
  if ( mCellSizeX < 0 )
  {
    mCellSizeX = -mCellSizeX;
  }
  mCellSizeY = geotransform[5];
  if ( mCellSizeY < 0 )
  {
    mCellSizeY = -mCellSizeY;
  }

  const char* projection = GDALGetProjectionRef( inputDataset );
  GDALSetProjection( outputDataset, projection );

  return outputDataset;
}

