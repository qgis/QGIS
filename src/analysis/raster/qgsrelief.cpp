/***************************************************************************
                          qgsrelief.cpp  -  description
                          ---------------------------
    begin                : November 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrelief.h"
#include "qgsaspectfilter.h"
#include "qgshillshadefilter.h"
#include "qgsslopefilter.h"
#include "qgis.h"
#include "cpl_string.h"
#include <QProgressDialog>
#include <cfloat>

#include <QFile>
#include <QTextStream>

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8(x) (x).toUtf8().constData()
#else
#define TO8(x) (x).toLocal8Bit().constData()
#endif

QgsRelief::QgsRelief( const QString& inputFile, const QString& outputFile, const QString& outputFormat ): \
    mInputFile( inputFile ), mOutputFile( outputFile ), mOutputFormat( outputFormat ), mZFactor( 1.0 )
{
  mSlopeFilter = new QgsSlopeFilter( inputFile, outputFile, outputFormat );
  mAspectFilter = new QgsAspectFilter( inputFile, outputFile, outputFormat );
  mHillshadeFilter285 = new QgsHillshadeFilter( inputFile, outputFile, outputFormat, 285, 30 );
  mHillshadeFilter300 = new QgsHillshadeFilter( inputFile, outputFile, outputFormat, 300, 30 );
  mHillshadeFilter315 = new QgsHillshadeFilter( inputFile, outputFile, outputFormat, 315, 30 );

  /*mReliefColors = calculateOptimizedReliefClasses();
    setDefaultReliefColors();*/
}

QgsRelief::~QgsRelief()
{
  delete mSlopeFilter;
  delete mAspectFilter;
  delete mHillshadeFilter285;
  delete mHillshadeFilter300;
  delete mHillshadeFilter315;
}

void QgsRelief::clearReliefColors()
{
  mReliefColors.clear();
}

void QgsRelief::addReliefColorClass( const ReliefColor& color )
{
  mReliefColors.push_back( color );
}

void QgsRelief::setDefaultReliefColors()
{
  clearReliefColors();
  addReliefColorClass( ReliefColor( QColor( 9, 176, 76 ), 0, 200 ) );
  addReliefColorClass( ReliefColor( QColor( 20, 228, 128 ), 200, 500 ) );
  addReliefColorClass( ReliefColor( QColor( 167, 239, 153 ), 500, 1000 ) );
  addReliefColorClass( ReliefColor( QColor( 218, 188, 143 ), 1000, 2000 ) );
  addReliefColorClass( ReliefColor( QColor( 233, 158, 91 ), 2000, 4000 ) );
  addReliefColorClass( ReliefColor( QColor( 255, 255, 255 ), 4000, 9000 ) );
}

int QgsRelief::processRaster( QProgressDialog* p )
{
  //open input file
  int xSize, ySize;
  GDALDatasetH  inputDataset = openInputFile( xSize, ySize );
  if ( inputDataset == NULL )
  {
    return 1; //opening of input file failed
  }

  //output driver
  GDALDriverH outputDriver = openOutputDriver();
  if ( outputDriver == 0 )
  {
    return 2;
  }

  GDALDatasetH outputDataset = openOutputFile( inputDataset, outputDriver );
  if ( outputDataset == NULL )
  {
    return 3; //create operation on output file failed
  }

  //initialize dependency filters with cell sizes
  mHillshadeFilter285->setCellSizeX( mCellSizeX );
  mHillshadeFilter285->setCellSizeY( mCellSizeY );
  mHillshadeFilter285->setZFactor( mZFactor );
  mHillshadeFilter300->setCellSizeX( mCellSizeX );
  mHillshadeFilter300->setCellSizeY( mCellSizeY );
  mHillshadeFilter300->setZFactor( mZFactor );
  mHillshadeFilter315->setCellSizeX( mCellSizeX );
  mHillshadeFilter315->setCellSizeY( mCellSizeY );
  mHillshadeFilter315->setZFactor( mZFactor );
  mSlopeFilter->setCellSizeX( mCellSizeX );
  mSlopeFilter->setCellSizeY( mCellSizeY );
  mSlopeFilter->setZFactor( mZFactor );
  mAspectFilter->setCellSizeX( mCellSizeX );
  mAspectFilter->setCellSizeY( mCellSizeY );
  mAspectFilter->setZFactor( mZFactor );

  //open first raster band for reading (operation is only for single band raster)
  GDALRasterBandH rasterBand = GDALGetRasterBand( inputDataset, 1 );
  if ( rasterBand == NULL )
  {
    GDALClose( inputDataset );
    GDALClose( outputDataset );
    return 4;
  }
  mInputNodataValue = GDALGetRasterNoDataValue( rasterBand, NULL );
  mSlopeFilter->setInputNodataValue( mInputNodataValue );
  mAspectFilter->setInputNodataValue( mInputNodataValue );
  mHillshadeFilter285->setInputNodataValue( mInputNodataValue );
  mHillshadeFilter300->setInputNodataValue( mInputNodataValue );
  mHillshadeFilter315->setInputNodataValue( mInputNodataValue );

  GDALRasterBandH outputRedBand = GDALGetRasterBand( outputDataset, 1 );
  GDALRasterBandH outputGreenBand = GDALGetRasterBand( outputDataset, 2 );
  GDALRasterBandH outputBlueBand = GDALGetRasterBand( outputDataset, 3 );

  if ( outputRedBand == NULL || outputGreenBand == NULL || outputBlueBand == NULL )
  {
    GDALClose( inputDataset );
    GDALClose( outputDataset );
    return 5;
  }
  //try to set -9999 as nodata value
  GDALSetRasterNoDataValue( outputRedBand, -9999 );
  GDALSetRasterNoDataValue( outputGreenBand, -9999 );
  GDALSetRasterNoDataValue( outputBlueBand, -9999 );
  mOutputNodataValue = GDALGetRasterNoDataValue( outputRedBand, NULL );
  mSlopeFilter->setOutputNodataValue( mOutputNodataValue );
  mAspectFilter->setOutputNodataValue( mOutputNodataValue );
  mHillshadeFilter285->setOutputNodataValue( mOutputNodataValue );
  mHillshadeFilter300->setOutputNodataValue( mOutputNodataValue );
  mHillshadeFilter315->setOutputNodataValue( mOutputNodataValue );

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

  int* resultRedLine = ( int * ) CPLMalloc( sizeof( int ) * xSize );
  int* resultGreenLine = ( int * ) CPLMalloc( sizeof( int ) * xSize );
  int* resultBlueLine = ( int * ) CPLMalloc( sizeof( int ) * xSize );

  if ( p )
  {
    p->setMaximum( ySize );
  }

  bool resultOk;

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
      GDALRasterIO( rasterBand, GF_Read, 0, 0, xSize, 1, scanLine2, xSize, 1, GDT_Float32, 0, 0 );
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
      GDALRasterIO( rasterBand, GF_Read, 0, i + 1, xSize, 1, scanLine3, xSize, 1, GDT_Float32, 0, 0 );
    }

    for ( int j = 0; j < xSize; ++j )
    {
      if ( j == 0 )
      {
        resultOk = processNineCellWindow( &mInputNodataValue, &scanLine1[j], &scanLine1[j+1], &mInputNodataValue, &scanLine2[j], \
                                          &scanLine2[j+1], &mInputNodataValue, &scanLine3[j], &scanLine3[j+1], \
                                          &resultRedLine[j], &resultGreenLine[j], &resultBlueLine[j] );
      }
      else if ( j == xSize - 1 )
      {
        resultOk = processNineCellWindow( &scanLine1[j-1], &scanLine1[j], &mInputNodataValue, &scanLine2[j-1], &scanLine2[j], \
                                          &mInputNodataValue, &scanLine3[j-1], &scanLine3[j], &mInputNodataValue, \
                                          &resultRedLine[j], &resultGreenLine[j], &resultBlueLine[j] );
      }
      else
      {
        resultOk = processNineCellWindow( &scanLine1[j-1], &scanLine1[j], &scanLine1[j+1], &scanLine2[j-1], &scanLine2[j], \
                                          &scanLine2[j+1], &scanLine3[j-1], &scanLine3[j], &scanLine3[j+1], \
                                          &resultRedLine[j], &resultGreenLine[j], &resultBlueLine[j] );
      }

      if ( !resultOk )
      {
        resultRedLine[j] = mOutputNodataValue;
        resultGreenLine[j] = mOutputNodataValue;
        resultBlueLine[j] = mOutputNodataValue;
      }
    }

    GDALRasterIO( outputRedBand, GF_Write, 0, i, xSize, 1, resultRedLine, xSize, 1, GDT_Int32, 0, 0 );
    GDALRasterIO( outputGreenBand, GF_Write, 0, i, xSize, 1, resultGreenLine, xSize, 1, GDT_Int32, 0, 0 );
    GDALRasterIO( outputBlueBand, GF_Write, 0, i, xSize, 1, resultBlueLine, xSize, 1, GDT_Int32, 0, 0 );
  }

  if ( p )
  {
    p->setValue( ySize );
  }

  CPLFree( resultRedLine );
  CPLFree( resultBlueLine );
  CPLFree( resultGreenLine );
  CPLFree( scanLine1 );
  CPLFree( scanLine2 );
  CPLFree( scanLine3 );

  GDALClose( inputDataset );

  if ( p && p->wasCanceled() )
  {
    //delete the dataset without closing (because it is faster)
    GDALDeleteDataset( outputDriver, mOutputFile.toLocal8Bit().data() );
    return 7;
  }
  GDALClose( outputDataset );

  return 0;
}

bool QgsRelief::processNineCellWindow( float* x1, float* x2, float* x3, float* x4, float* x5, float* x6, float* x7, float* x8, float* x9,
                                       int* red, int* green, int* blue )
{
  //1. component: colour and hillshade from 300 degrees
  int r = 0;
  int g = 0;
  int b = 0;

  float hillShadeValue300 = mHillshadeFilter300->processNineCellWindow( x1, x2, x3, x4, x5, x6, x7, x8, x9 );
  if ( hillShadeValue300 != mOutputNodataValue )
  {
    if ( !setElevationColor( *x5, &r, &g, &b ) )
    {
      r = hillShadeValue300;
      g = hillShadeValue300;
      b = hillShadeValue300;
    }
    else
    {
      r = r / 2.0 + hillShadeValue300 / 2.0;
      g = g / 2.0 + hillShadeValue300 / 2.0;
      b = b / 2.0 + hillShadeValue300 / 2.0;
    }
  }

  //2. component: hillshade and slope
  float hillShadeValue315 = mHillshadeFilter315->processNineCellWindow( x1, x2, x3, x4, x5, x6, x7, x8, x9 );
  float slope = mSlopeFilter->processNineCellWindow( x1, x2, x3, x4, x5, x6, x7, x8, x9 );
  if ( hillShadeValue315 != mOutputNodataValue && slope != mOutputNodataValue )
  {
    int r2, g2, b2;
    if ( slope > 15 )
    {
      r2 = 0 / 2.0 + hillShadeValue315 / 2.0;
      g2 = 0 / 2.0 + hillShadeValue315 / 2.0;
      b2 = 0 / 2.0 + hillShadeValue315 / 2.0;
    }
    else if ( slope >= 1 )
    {
      int slopeValue = 255 - ( slope / 15.0 * 255.0 );
      r2 = slopeValue / 2.0 + hillShadeValue315 / 2.0;
      g2 = slopeValue / 2.0 + hillShadeValue315 / 2.0;
      b2 = slopeValue / 2.0 + hillShadeValue315 / 2.0;
    }
    else
    {
      r2 = hillShadeValue315; g2 = hillShadeValue315; b2 = hillShadeValue315;
    }

    //combine with r,g,b with 70 percentage coverage
    r = r * 0.7 + r2 * 0.3;
    g = g * 0.7 + g2 * 0.3;
    b = b * 0.7 + b2 * 0.3;
  }

  //3. combine yellow aspect with 10% transparency, illumination from 285 degrees
  float hillShadeValue285 = mHillshadeFilter285->processNineCellWindow( x1, x2, x3, x4, x5, x6, x7, x8, x9 );
  float aspect = mAspectFilter->processNineCellWindow( x1, x2, x3, x4, x5, x6, x7, x8, x9 );
  if ( hillShadeValue285 != mOutputNodataValue && aspect != mOutputNodataValue )
  {
    double angle_diff = qAbs( 285 - aspect );
    if ( angle_diff > 180 )
    {
      angle_diff -= 180;
    }

    int r3, g3, b3;
    if ( angle_diff < 90 )
    {
      int aspectVal = ( 1 - cos( angle_diff * M_PI / 180 ) ) * 255;
      r3 = 0.5 * 255 + hillShadeValue315 * 0.5;
      g3 = 0.5 * 255 + hillShadeValue315 * 0.5;
      b3 = 0.5 * aspectVal + hillShadeValue315 * 0.5;
    }
    else //white
    {
      r3 = 0.5 * 255 + hillShadeValue315 * 0.5;
      g3 = 0.5 * 255 + hillShadeValue315 * 0.5;
      b3 = 0.5 * 255 + hillShadeValue315 * 0.5;
    }

    r = r3 * 0.1 + r * 0.9;
    g = g3 * 0.1 + g * 0.9;
    b = b3 * 0.1 + b * 0.9;
  }

  *red = r;
  *green = g;
  *blue = b;
  return true;
}

bool QgsRelief::setElevationColor( double elevation, int* red, int* green, int* blue )
{
  QList< ReliefColor >::const_iterator reliefColorIt =  mReliefColors.constBegin();
  for ( ; reliefColorIt != mReliefColors.constEnd(); ++reliefColorIt )
  {
    if ( elevation >= reliefColorIt->minElevation && elevation <= reliefColorIt->maxElevation )
    {
      const QColor& c = reliefColorIt->color;
      *red = c.red();
      *green = c.green();
      *blue = c.blue();

      return true;
    }
  }
  return false;
}

//duplicated from QgsNineCellFilter. Todo: make common base class
GDALDatasetH QgsRelief::openInputFile( int& nCellsX, int& nCellsY )
{
  GDALDatasetH inputDataset = GDALOpen( TO8( mInputFile ), GA_ReadOnly );
  if ( inputDataset != NULL )
  {
    nCellsX = GDALGetRasterXSize( inputDataset );
    nCellsY = GDALGetRasterYSize( inputDataset );

    //we need at least one band
    if ( GDALGetRasterCount( inputDataset ) < 1 )
    {
      GDALClose( inputDataset );
      return NULL;
    }
  }
  return inputDataset;
}

GDALDriverH QgsRelief::openOutputDriver()
{
  char **driverMetadata;

  //open driver
  GDALDriverH outputDriver = GDALGetDriverByName( mOutputFormat.toLocal8Bit().data() );

  if ( outputDriver == NULL )
  {
    return outputDriver; //return NULL, driver does not exist
  }

  driverMetadata = GDALGetMetadata( outputDriver, NULL );
  if ( !CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE, false ) )
  {
    return NULL; //driver exist, but it does not support the create operation
  }

  return outputDriver;
}

GDALDatasetH QgsRelief::openOutputFile( GDALDatasetH inputDataset, GDALDriverH outputDriver )
{
  if ( inputDataset == NULL )
  {
    return NULL;
  }

  int xSize = GDALGetRasterXSize( inputDataset );
  int ySize = GDALGetRasterYSize( inputDataset );;

  //open output file
  char **papszOptions = NULL;

  //use PACKBITS compression for tiffs by default
  papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "PACKBITS" );

  //create three band raster (reg, green, blue)
  GDALDatasetH outputDataset = GDALCreate( outputDriver, mOutputFile.toLocal8Bit().data(), xSize, ySize, 3, GDT_Int32, papszOptions );
  if ( outputDataset == NULL )
  {
    return outputDataset;
  }

  //get geotransform from inputDataset
  double geotransform[6];
  if ( GDALGetGeoTransform( inputDataset, geotransform ) != CE_None )
  {
    GDALClose( outputDataset );
    return NULL;
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

//this function is mainly there for debugging
bool QgsRelief::exportFrequencyDistributionToCsv( const QString& file )
{
  int nCellsX, nCellsY;
  GDALDatasetH inputDataset = openInputFile( nCellsX, nCellsY );
  if ( inputDataset == NULL )
  {
    return false;
  }

  //open first raster band for reading (elevation raster is always single band)
  GDALRasterBandH elevationBand = GDALGetRasterBand( inputDataset, 1 );
  if ( elevationBand == NULL )
  {
    GDALClose( inputDataset );
    return false;
  }

  //1. get minimum and maximum of elevation raster -> 252 elevation classes
  int minOk, maxOk;
  double minMax[2];
  minMax[0] = GDALGetRasterMinimum( elevationBand, &minOk );
  minMax[1] = GDALGetRasterMaximum( elevationBand, &maxOk );

  if ( !minOk || !maxOk )
  {
    GDALComputeRasterMinMax( elevationBand, TRUE, minMax );
  }

  //2. go through raster cells and get frequency of classes

  //store elevation frequency in 256 elevation classes
  double frequency[252];
  double frequencyClassRange = ( minMax[1] - minMax[0] ) / 252.0;
  //initialize to zero
  for ( int i = 0; i < 252; ++i )
  {
    frequency[i] = 0;
  }

  float* scanLine = ( float * ) CPLMalloc( sizeof( float ) *  nCellsX );
  int elevationClass = -1;

  for ( int i = 0; i < nCellsY; ++i )
  {
    GDALRasterIO( elevationBand, GF_Read, 0, i, nCellsX, 1,
                  scanLine, nCellsX, 1, GDT_Float32,
                  0, 0 );
    for ( int j = 0; j < nCellsX; ++j )
    {
      elevationClass = frequencyClassForElevation( scanLine[j], minMax[0], frequencyClassRange );
      if ( elevationClass >= 0 )
      {
        frequency[elevationClass] += 1.0;
      }
    }
  }

  CPLFree( scanLine );

  //log10 transformation for all frequency values
  for ( int i = 0; i < 252; ++i )
  {
    frequency[i] = log10( frequency[i] );
  }

  //write out frequency values to csv file for debugging
  QFile outFile( file );
  if ( !outFile.open( QIODevice::WriteOnly ) )
  {
    return false;
  }

  QTextStream outstream( &outFile );
  for ( int i = 0; i < 252; ++i )
  {
    outstream << QString::number( i ) + "," + QString::number( frequency[i] ) << endl;
  }
  outFile.close();
  return true;
}

QList< QgsRelief::ReliefColor > QgsRelief::calculateOptimizedReliefClasses()
{
  QList< QgsRelief::ReliefColor > resultList;

  int nCellsX, nCellsY;
  GDALDatasetH inputDataset = openInputFile( nCellsX, nCellsY );
  if ( inputDataset == NULL )
  {
    return resultList;
  }

  //open first raster band for reading (elevation raster is always single band)
  GDALRasterBandH elevationBand = GDALGetRasterBand( inputDataset, 1 );
  if ( elevationBand == NULL )
  {
    GDALClose( inputDataset );
    return resultList;
  }

  //1. get minimum and maximum of elevation raster -> 252 elevation classes
  int minOk, maxOk;
  double minMax[2];
  minMax[0] = GDALGetRasterMinimum( elevationBand, &minOk );
  minMax[1] = GDALGetRasterMaximum( elevationBand, &maxOk );

  if ( !minOk || !maxOk )
  {
    GDALComputeRasterMinMax( elevationBand, TRUE, minMax );
  }

  //2. go through raster cells and get frequency of classes

  //store elevation frequency in 256 elevation classes
  double frequency[252];
  double frequencyClassRange = ( minMax[1] - minMax[0] ) / 252.0;
  //initialize to zero
  for ( int i = 0; i < 252; ++i )
  {
    frequency[i] = 0;
  }

  float* scanLine = ( float * ) CPLMalloc( sizeof( float ) *  nCellsX );
  int elevationClass = -1;

  for ( int i = 0; i < nCellsY; ++i )
  {
    GDALRasterIO( elevationBand, GF_Read, 0, i, nCellsX, 1,
                  scanLine, nCellsX, 1, GDT_Float32,
                  0, 0 );
    for ( int j = 0; j < nCellsX; ++j )
    {
      elevationClass = frequencyClassForElevation( scanLine[j], minMax[0], frequencyClassRange );
      if ( elevationClass < 0 )
      {
        elevationClass = 0;
      }
      else if ( elevationClass >= 252 )
      {
        elevationClass = 251;
      }
      frequency[elevationClass] += 1.0;
    }
  }

  CPLFree( scanLine );

  //log10 transformation for all frequency values
  for ( int i = 0; i < 252; ++i )
  {
    frequency[i] = log10( frequency[i] );
  }

  //start with 9 uniformly distributed classes
  QList<int> classBreaks;
  classBreaks.append( 0 );
  classBreaks.append( 28 );
  classBreaks.append( 56 );
  classBreaks.append( 84 );
  classBreaks.append( 112 );
  classBreaks.append( 140 );
  classBreaks.append( 168 );
  classBreaks.append( 196 );
  classBreaks.append( 224 );
  classBreaks.append( 252 );

  for ( int i = 0; i < 10; ++i )
  {
    optimiseClassBreaks( classBreaks, frequency );
  }

  //debug, print out all the classbreaks
  for ( int i = 0; i < classBreaks.size(); ++i )
  {
    qWarning( "%d", classBreaks[i] );
  }

  //set colors according to optimised class breaks
  QList<QColor> colorList;
  colorList.push_back( QColor( 7, 165, 144 ) );
  colorList.push_back( QColor( 12, 221, 162 ) );
  colorList.push_back( QColor( 33, 252, 183 ) );
  colorList.push_back( QColor( 247, 252, 152 ) );
  colorList.push_back( QColor( 252, 196, 8 ) );
  colorList.push_back( QColor( 252, 166, 15 ) );
  colorList.push_back( QColor( 175, 101, 15 ) );
  colorList.push_back( QColor( 255, 133, 92 ) );
  colorList.push_back( QColor( 204, 204, 204 ) );

  for ( int i = 1; i < classBreaks.size(); ++i )
  {
    double minElevation = minMax[0] + classBreaks[i - 1] * frequencyClassRange;
    double maxElevation = minMax[0] + classBreaks[i] * frequencyClassRange;
    resultList.push_back( QgsRelief::ReliefColor( colorList.at( i - 1 ), minElevation, maxElevation ) );
  }

  return resultList;
}

void QgsRelief::optimiseClassBreaks( QList<int>& breaks, double* frequencies )
{
  int nClasses = breaks.size() - 1;
  double* a = new double[nClasses]; //slopes
  double* b = new double[nClasses]; //y-offsets

  for ( int i = 0; i < nClasses; ++i )
  {
    //get all the values between the class breaks into input
    QList< QPair < int, double > > regressionInput;
    for ( int j = breaks.at( i ); j < breaks.at( i + 1 ); ++j )
    {
      regressionInput.push_back( qMakePair( j, frequencies[j] ) );
    }

    double aParam, bParam;
    if ( regressionInput.size() > 0 && calculateRegression( regressionInput, aParam, bParam ) )
    {
      a[i] = aParam;
      b[i] = bParam;
    }
    else
    {
      a[i] = 0;
      b[i] = 0; //better default value
    }
  }

  QList<int> classesToRemove;

  //shift class boundaries or eliminate classes which fall together
  for ( int i = 1; i < nClasses ; ++i )
  {
    if ( breaks[i] == breaks[ i - 1 ] )
    {
      continue;
    }

    if ( doubleNear( a[i - 1 ], a[i] ) )
    {
      continue;
    }
    else
    {
      int newX = ( b[i - 1] - b[ i ] ) / ( a[ i ] - a[ i - 1 ] );

      if ( newX <= breaks[i - 1] )
      {
        newX = breaks[i - 1];
        //  classesToRemove.push_back( i );//remove this class later as it falls together with the preceding one
      }
      else if ( i < nClasses - 1 && newX >= breaks[i + 1] )
      {
        newX = breaks[i + 1];
        //  classesToRemove.push_back( i );//remove this class later as it falls together with the next one
      }

      breaks[i] = newX;
    }
  }

  for ( int i = classesToRemove.size() - 1; i >= 0; --i )
  {
    breaks.removeAt( classesToRemove.at( i ) );
  }

  delete[] a;
  delete[] b;
}

int QgsRelief::frequencyClassForElevation( double elevation, double minElevation, double elevationClassRange )
{
  return ( elevation - minElevation ) / elevationClassRange;
}

bool QgsRelief::calculateRegression( const QList< QPair < int, double > >& input, double& a, double& b )
{
  double xMean, yMean;
  double xSum = 0;
  double ySum = 0;
  QList< QPair < int, double > >::const_iterator inputIt = input.constBegin();
  for ( ; inputIt != input.constEnd(); ++inputIt )
  {
    xSum += inputIt->first;
    ySum += inputIt->second;
  }
  xMean = xSum / input.size();
  yMean = ySum / input.size();

  double sumCounter = 0;
  double sumDenominator = 0;
  inputIt = input.constBegin();
  for ( ; inputIt != input.constEnd(); ++inputIt )
  {
    sumCounter += (( inputIt->first - xMean ) * ( inputIt->second - yMean ) );
    sumDenominator += (( inputIt->first - xMean ) * ( inputIt->first - xMean ) );
  }

  a = sumCounter / sumDenominator;
  b = yMean - a * xMean;

  return true;
}

