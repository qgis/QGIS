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
#include "qgsfeedback.h"
#include "qgsogrutils.h"
#include "qgsmessagelog.h"
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <iterator>


QgsNineCellFilter::QgsNineCellFilter( const QString &inputFile, const QString &outputFile, const QString &outputFormat )
  : mInputFile( inputFile )
  , mOutputFile( outputFile )
  , mOutputFormat( outputFormat )
{

}

int QgsNineCellFilter::processRaster( QgsFeedback *feedback )
{
#ifdef HAVE_OPENCL
  if ( QgsOpenClUtils::enabled() && QgsOpenClUtils::available() && ! openClProgramBaseName( ).isEmpty() )
  {
    // Load the program sources
    QString source( QgsOpenClUtils::sourceFromPath( QStringLiteral( "/home/ale/dev/QGIS/src/analysis/raster/%1.cl" ).arg( openClProgramBaseName( ) ) ) );
    if ( ! source.isEmpty() )
    {
      try
      {
        QgsMessageLog::logMessage( QObject::tr( "Running OpenCL program: %1" )
                                   .arg( openClProgramBaseName( ) ), QgsOpenClUtils::LOGMESSAGE_TAG, Qgis::Info );
        return processRasterGPU( source, feedback );
      }
      catch ( cl::BuildError &e )
      {
        cl::BuildLogType build_logs = e.getBuildLog();
        QString build_log;
        if ( build_logs.size() > 0 )
          build_log = QString::fromStdString( build_logs[0].second );
        else
          build_log = QObject::tr( "Build logs not available!" );
        QString err = QObject::tr( "Error building OpenCL program: %1" )
                      .arg( build_log );
        QgsMessageLog::logMessage( err, QgsOpenClUtils::LOGMESSAGE_TAG, Qgis::Critical );
        throw QgsProcessingException( err );
      }
      catch ( cl::Error &e )
      {
        QString err = QObject::tr( "Error %1 running OpenCL program in %2" )
                      .arg( QgsOpenClUtils::errorText( e.err() ), QString::fromStdString( e.what() ) );
        QgsMessageLog::logMessage( err, QgsOpenClUtils::LOGMESSAGE_TAG, Qgis::Critical );
        throw QgsProcessingException( err );
      }
    }
    else
    {
      QString err = QObject::tr( "Error loading OpenCL program sources" );
      QgsMessageLog::logMessage( err,
                                 QgsOpenClUtils::LOGMESSAGE_TAG, Qgis::Critical );
      throw QgsProcessingException( err );
    }
  }
  else
  {
    return processRasterCPU( feedback );
  }
  return 1;
#else
  return processRasterCPU( feedback );
#endif
}

gdal::dataset_unique_ptr QgsNineCellFilter::openInputFile( int &nCellsX, int &nCellsY )
{
  gdal::dataset_unique_ptr inputDataset( GDALOpen( mInputFile.toUtf8().constData(), GA_ReadOnly ) );
  if ( inputDataset )
  {
    nCellsX = GDALGetRasterXSize( inputDataset.get() );
    nCellsY = GDALGetRasterYSize( inputDataset.get() );

    //we need at least one band
    if ( GDALGetRasterCount( inputDataset.get() ) < 1 )
    {
      return nullptr;
    }
  }
  return inputDataset;
}

GDALDriverH QgsNineCellFilter::openOutputDriver()
{
  char **driverMetadata = nullptr;

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

gdal::dataset_unique_ptr QgsNineCellFilter::openOutputFile( GDALDatasetH inputDataset, GDALDriverH outputDriver )
{
  if ( !inputDataset )
  {
    return nullptr;
  }

  int xSize = GDALGetRasterXSize( inputDataset );
  int ySize = GDALGetRasterYSize( inputDataset );

  //open output file
  char **papszOptions = nullptr;
  gdal::dataset_unique_ptr outputDataset( GDALCreate( outputDriver, mOutputFile.toUtf8().constData(), xSize, ySize, 1, GDT_Float32, papszOptions ) );
  if ( !outputDataset )
  {
    return outputDataset;
  }

  //get geotransform from inputDataset
  double geotransform[6];
  if ( GDALGetGeoTransform( inputDataset, geotransform ) != CE_None )
  {
    return nullptr;
  }
  GDALSetGeoTransform( outputDataset.get(), geotransform );

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

  const char *projection = GDALGetProjectionRef( inputDataset );
  GDALSetProjection( outputDataset.get(), projection );

  return outputDataset;
}


int QgsNineCellFilter::processRasterGPU( const QString &source, QgsFeedback *feedback )
{

  GDALAllRegister();

  //open input file
  int xSize, ySize;
  gdal::dataset_unique_ptr inputDataset( openInputFile( xSize, ySize ) );
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

  gdal::dataset_unique_ptr outputDataset( openOutputFile( inputDataset.get(), outputDriver ) );
  if ( !outputDataset )
  {
    return 3; //create operation on output file failed
  }

  //open first raster band for reading (operation is only for single band raster)
  GDALRasterBandH rasterBand = GDALGetRasterBand( inputDataset.get(), 1 );
  if ( !rasterBand )
  {
    return 4;
  }
  mInputNodataValue = GDALGetRasterNoDataValue( rasterBand, nullptr );

  GDALRasterBandH outputRasterBand = GDALGetRasterBand( outputDataset.get(), 1 );
  if ( !outputRasterBand )
  {
    return 5;
  }
  //try to set -9999 as nodata value
  GDALSetRasterNoDataValue( outputRasterBand, -9999 );
  mOutputNodataValue = GDALGetRasterNoDataValue( outputRasterBand, nullptr );

  if ( ySize < 3 ) //we require at least three rows (should be true for most datasets)
  {
    return 6;
  }

  //keep only three scanlines in memory at a time, make room for initial and final nodata
  float *scanLine1 = ( float * ) CPLMalloc( sizeof( float ) * ( xSize + 2 ) );
  float *scanLine2 = ( float * ) CPLMalloc( sizeof( float ) * ( xSize + 2 ) );
  float *scanLine3 = ( float * ) CPLMalloc( sizeof( float ) * ( xSize + 2 ) );

  float *resultLine = ( float * ) CPLMalloc( sizeof( float ) * xSize );

  cl_int errorCode = 0;

  // Cast to float
  std::vector<float> rasterParams;

  rasterParams.push_back( mInputNodataValue );
  rasterParams.push_back( mOutputNodataValue );
  rasterParams.push_back( mZFactor );
  rasterParams.push_back( mCellSizeX );
  rasterParams.push_back( mCellSizeY );

  addExtraRasterParams( rasterParams );

  try
  {

    cl::Buffer rasterParamsBuffer( rasterParams.begin(), rasterParams.end(), true, false, &errorCode );
    cl::Buffer scanLine1Buffer( CL_MEM_READ_ONLY, sizeof( float ) * ( xSize + 2 ), nullptr, &errorCode );
    cl::Buffer scanLine2Buffer( CL_MEM_READ_ONLY, sizeof( float ) * ( xSize + 2 ), nullptr, &errorCode );
    cl::Buffer scanLine3Buffer( CL_MEM_READ_ONLY, sizeof( float ) * ( xSize + 2 ), nullptr, &errorCode );
    cl::Buffer resultLineBuffer( CL_MEM_WRITE_ONLY, sizeof( float ) * xSize, nullptr, &errorCode );

    // Create a program from the kernel source
    cl::Program program( source.toStdString() );
    // Use CL 1.1 for compatibility with older libs
    program.build( "-cl-std=CL1.1" );

    // Create the OpenCL kernel
    auto kernel = cl::KernelFunctor <
                  cl::Buffer &,
                  cl::Buffer &,
                  cl::Buffer &,
                  cl::Buffer &,
                  cl::Buffer &
                  > ( program, "processNineCellWindow" );

    //values outside the layer extent (if the 3x3 window is on the border) are sent to the processing method as (input) nodata values
    for ( int i = 0; i < ySize; ++i )
    {
      if ( feedback && feedback->isCanceled() )
      {
        break;
      }

      if ( feedback )
      {
        feedback->setProgress( 100.0 * static_cast< double >( i ) / ySize );
      }

      if ( i == 0 )
      {
        //fill scanline 1 with (input) nodata for the values above the first row and feed scanline2 with the first row
        for ( int a = 0; a < xSize + 2 ; ++a )
        {
          scanLine1[a] = mInputNodataValue;
        }
        // Read scanline2
        if ( GDALRasterIO( rasterBand, GF_Read, 0, 0, xSize, 1, &scanLine2[1], xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
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
        scanLine3 = ( float * ) CPLMalloc( sizeof( float ) * ( xSize + 2 ) );
      }

      // Read scanline 3
      if ( i == ySize - 1 ) //fill the row below the bottom with nodata values
      {
        for ( int a = 0; a < xSize + 2; ++a )
        {
          scanLine3[a] = mInputNodataValue;
        }
      }
      else
      {
        if ( GDALRasterIO( rasterBand, GF_Read, 0, i + 1, xSize, 1, &scanLine3[1], xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
        {
          QgsDebugMsg( "Raster IO Error" );
        }
      }
      // Set first and last extra colums to nodata
      scanLine1[0] = scanLine1[xSize + 1] = mInputNodataValue;
      scanLine2[0] = scanLine2[xSize + 1] = mInputNodataValue;
      scanLine3[0] = scanLine3[xSize + 1] = mInputNodataValue;

      errorCode = cl::enqueueWriteBuffer( scanLine1Buffer, CL_TRUE, 0,
                                          sizeof( float ) * ( xSize + 2 ), scanLine1 );
      errorCode = cl::enqueueWriteBuffer( scanLine2Buffer, CL_TRUE, 0,
                                          sizeof( float ) * ( xSize + 2 ), scanLine2 );
      errorCode = cl::enqueueWriteBuffer( scanLine3Buffer, CL_TRUE, 0,
                                          sizeof( float ) * ( xSize + 2 ), scanLine3 );

      kernel( cl::EnqueueArgs(
                cl::NDRange( xSize )
              ),
              scanLine1Buffer,
              scanLine2Buffer,
              scanLine3Buffer,
              resultLineBuffer,
              rasterParamsBuffer
            );

      cl::enqueueReadBuffer( resultLineBuffer, CL_TRUE, 0, xSize * sizeof( float ), resultLine );

      if ( GDALRasterIO( outputRasterBand, GF_Write, 0, i, xSize, 1, resultLine, xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
      {
        QgsDebugMsg( "Raster IO Error" );
      }

    }
  }
  catch ( cl::Error &e )
  {
    CPLFree( resultLine );
    CPLFree( scanLine1 );
    CPLFree( scanLine2 );
    CPLFree( scanLine3 );
    throw e;
  }

  CPLFree( resultLine );
  CPLFree( scanLine1 );
  CPLFree( scanLine2 );
  CPLFree( scanLine3 );

  if ( feedback && feedback->isCanceled() )
  {
    //delete the dataset without closing (because it is faster)
    gdal::fast_delete_and_close( outputDataset, outputDriver, mOutputFile );
    return 7;
  }
  return 0;
}


int QgsNineCellFilter::processRasterCPU( QgsFeedback *feedback )
{

  GDALAllRegister();

  //open input file
  int xSize, ySize;
  gdal::dataset_unique_ptr inputDataset( openInputFile( xSize, ySize ) );
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

  gdal::dataset_unique_ptr outputDataset( openOutputFile( inputDataset.get(), outputDriver ) );
  if ( !outputDataset )
  {
    return 3; //create operation on output file failed
  }

  //open first raster band for reading (operation is only for single band raster)
  GDALRasterBandH rasterBand = GDALGetRasterBand( inputDataset.get(), 1 );
  if ( !rasterBand )
  {
    return 4;
  }
  mInputNodataValue = GDALGetRasterNoDataValue( rasterBand, nullptr );

  GDALRasterBandH outputRasterBand = GDALGetRasterBand( outputDataset.get(), 1 );
  if ( !outputRasterBand )
  {
    return 5;
  }
  //try to set -9999 as nodata value
  GDALSetRasterNoDataValue( outputRasterBand, -9999 );
  mOutputNodataValue = GDALGetRasterNoDataValue( outputRasterBand, nullptr );

  if ( ySize < 3 ) //we require at least three rows (should be true for most datasets)
  {
    return 6;
  }

  //keep only three scanlines in memory at a time, make room for initial and final nodata
  float *scanLine1 = ( float * ) CPLMalloc( sizeof( float ) * ( xSize + 2 ) );
  float *scanLine2 = ( float * ) CPLMalloc( sizeof( float ) * ( xSize + 2 ) );
  float *scanLine3 = ( float * ) CPLMalloc( sizeof( float ) * ( xSize + 2 ) );

  float *resultLine = ( float * ) CPLMalloc( sizeof( float ) * xSize );

  //values outside the layer extent (if the 3x3 window is on the border) are sent to the processing method as (input) nodata values
  for ( int i = 0; i < ySize; ++i )
  {
    if ( feedback && feedback->isCanceled() )
    {
      break;
    }

    if ( feedback )
    {
      feedback->setProgress( 100.0 * static_cast< double >( i ) / ySize );
    }

    if ( i == 0 )
    {
      //fill scanline 1 with (input) nodata for the values above the first row and feed scanline2 with the first row
      for ( int a = 0; a < xSize + 2 ; ++a )
      {
        scanLine1[a] = mInputNodataValue;
      }
      // Read scanline2
      if ( GDALRasterIO( rasterBand, GF_Read, 0, 0, xSize, 1, &scanLine2[1], xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
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
      scanLine3 = ( float * ) CPLMalloc( sizeof( float ) * ( xSize + 2 ) );
    }

    // Read scanline 3
    if ( i == ySize - 1 ) //fill the row below the bottom with nodata values
    {
      for ( int a = 0; a < xSize + 2; ++a )
      {
        scanLine3[a] = mInputNodataValue;
      }
    }
    else
    {
      if ( GDALRasterIO( rasterBand, GF_Read, 0, i + 1, xSize, 1, &scanLine3[1], xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
      {
        QgsDebugMsg( "Raster IO Error" );
      }
    }
    // Set first and last extra colums to nodata
    scanLine1[0] = scanLine1[xSize + 1] = mInputNodataValue;
    scanLine2[0] = scanLine2[xSize + 1] = mInputNodataValue;
    scanLine3[0] = scanLine3[xSize + 1] = mInputNodataValue;



    // j is the x axis index, skip 0 and last cell that have been filled with nodata
    for ( int j = 0; j < xSize ; ++j )
    {
      resultLine[ j ] = processNineCellWindow( &scanLine1[ j ], &scanLine1[ j + 1 ], &scanLine1[ j + 2 ],
                        &scanLine2[ j ], &scanLine2[ j + 1 ], &scanLine2[ j + 2 ],
                        &scanLine3[ j ], &scanLine3[ j + 1 ], &scanLine3[ j + 2 ] );

    }

    if ( GDALRasterIO( outputRasterBand, GF_Write, 0, i, xSize, 1, resultLine, xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
    {
      QgsDebugMsg( "Raster IO Error" );
    }
  }

  CPLFree( resultLine );
  CPLFree( scanLine1 );
  CPLFree( scanLine2 );
  CPLFree( scanLine3 );

  if ( feedback && feedback->isCanceled() )
  {
    //delete the dataset without closing (because it is faster)
    gdal::fast_delete_and_close( outputDataset, outputDriver, mOutputFile );
    return 7;
  }
  return 0;
}
