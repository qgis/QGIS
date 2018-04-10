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
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <iterator>

#ifdef HAVE_OPENCL
#include <CL/cl.hpp>
#include <CL/cl.h>
#endif


QgsNineCellFilter::QgsNineCellFilter( const QString &inputFile, const QString &outputFile, const QString &outputFormat )
  : mInputFile( inputFile )
  , mOutputFile( outputFile )
  , mOutputFormat( outputFormat )
{

}

int QgsNineCellFilter::processRaster( QgsFeedback *feedback )
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

#ifdef HAVE_OPENCL

  cl_int errorCode = 0;

  // Get platform and device information
//  cl_platform_id platform_id = NULL;
//  cl_device_id device_id = NULL;
//  cl_uint ret_num_devices;
//  cl_uint ret_num_platforms;
//  cl_int ret = clGetPlatformIDs( 1, &platform_id, &ret_num_platforms );
//  ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1,
//                        &device_id, &ret_num_devices );

//  // Create an OpenCL context
//  cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret );

//  // Create a command queue
//  cl_command_queue command_queue = clCreateCommandQueue( context, device_id, 0, &ret );

//  // Create memory buffers on the device for each vector
//  cl_mem scanLine1Buffer = clCreateBuffer( context, CL_MEM_READ_ONLY,
//                             sizeof( float ) * ( xSize + 2 ), NULL, &ret );
//  cl_mem scanLine2Buffer = clCreateBuffer( context, CL_MEM_READ_ONLY,
//                             sizeof( float ) * ( xSize + 2 ), NULL, &ret );
//  cl_mem scanLine3Buffer = clCreateBuffer( context, CL_MEM_READ_ONLY,
//                             sizeof( float ) * ( xSize + 2 ), NULL, &ret );

  // TODO: constants


//  cl_mem inputNodataValueBuffer = clCreateBuffer( context, CL_MEM_READ_ONLY,
//                                    sizeof( float ), NULL, &ret );
//  cl_mem outputNodataValueBuffer = clCreateBuffer( context, CL_MEM_READ_ONLY,
//                                     sizeof( float ), NULL, &ret );
//  cl_mem zFactorBuffer = clCreateBuffer( context, CL_MEM_READ_ONLY,
//                           sizeof( double ), NULL, &ret );
//  cl_mem cellSizeXBuffer = clCreateBuffer( context, CL_MEM_READ_ONLY,
//                             sizeof( double ), NULL, &ret );
//  cl_mem cellSizeYBuffer = clCreateBuffer( context, CL_MEM_READ_ONLY,
//                             sizeof( double ), NULL, &ret );

//  cl_mem resultLineBuffer = clCreateBuffer( context, CL_MEM_WRITE_ONLY,
//                              sizeof( float ) *  xSize, NULL, &ret );

  std::vector<double> rasterParams;

  rasterParams.push_back( mInputNodataValue );
  rasterParams.push_back( mOutputNodataValue );
  rasterParams.push_back( mZFactor );
  rasterParams.push_back( mCellSizeX );
  rasterParams.push_back( mCellSizeY );

//  cl::Buffer inputNodataValueBuffer( CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
//                                      sizeof( float ), mInputNodataValue , &ret );
//  cl::Buffer outputNodataValueBuffer( CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
//                                       sizeof( float ), mOutputNodataValue, &ret );
//  cl::Buffer zFactorBuffer ( CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
//                             sizeof( double ), mZFactor, &ret );
//  cl::Buffer cellSizeXBuffer ( CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
//                               sizeof( double ), mCellSizeX, &ret );
//  cl::Buffer cellSizeYBuffer ( CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
//                               sizeof( double ), mCellSizeY, &ret );

  cl::Buffer rasterParamsBuffer( std::begin( rasterParams ), std::end( rasterParams ), true, false, &errorCode );

  cl::Buffer resultLineBuffer( CL_MEM_WRITE_ONLY, sizeof( float ) * xSize, nullptr, &errorCode );

  cl::Buffer scanLine1Buffer( CL_MEM_READ_ONLY, sizeof( float ) * ( xSize + 2 ), nullptr, &errorCode );
  cl::Buffer scanLine2Buffer( CL_MEM_READ_ONLY, sizeof( float ) * ( xSize + 2 ), nullptr, &errorCode );
  cl::Buffer scanLine3Buffer( CL_MEM_READ_ONLY, sizeof( float ) * ( xSize + 2 ), nullptr, &errorCode );


  char *source_str = new char [QFileInfo( "/home/ale/dev/QGIS/src/analysis/raster/slope.cl" ).size() + 1];

  QFile file( "/home/ale/dev/QGIS/src/analysis/raster/slope.cl" );
  file.open( QIODevice::ReadOnly | QIODevice::Text );

  file.read( source_str, file.size() );
  source_str[QFileInfo( "/home/ale/dev/QGIS/src/analysis/raster/slope.cl" ).size()] = '\0';
  file.close();

  // Create a program from the kernel source
  cl::Program program( source_str, true, &errorCode );

  auto buildInfo = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>( cl::Device::getDefault(), &errorCode );
  for ( auto &pair : buildInfo )
  {
    qDebug() << pair;
  }

  // Create the OpenCL kernel
  auto kernel =
    cl::make_kernel <
    cl::Buffer &,
    cl::Buffer &,
    cl::Buffer &,
    cl::Buffer &,
    cl::Buffer &
    > ( program, "processNineCellWindow" );

#endif


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

#ifdef HAVE_OPENCL
    // Copy the scan lines to their respective memory buffers
//    ret = clEnqueueWriteBuffer( command_queue, scanLine1Buffer, CL_TRUE, 0,
//                                sizeof( float ) * ( xSize + 2 ), scanLine1, 0, NULL, NULL );
//    ret = clEnqueueWriteBuffer( command_queue, scanLine2Buffer, CL_TRUE, 0,
//                                sizeof( float ) * ( xSize + 2 ), scanLine2, 0, NULL, NULL );
//    ret = clEnqueueWriteBuffer( command_queue, scanLine3Buffer, CL_TRUE, 0,
//                                sizeof( float ) * ( xSize + 2 ), scanLine3, 0, NULL, NULL );

//    ret = clEnqueueWriteBuffer( command_queue, inputNodataValueBuffer, CL_TRUE, 0,
//                                sizeof( float ), &mInputNodataValue, 0, NULL, NULL );
//    ret = clEnqueueWriteBuffer( command_queue, outputNodataValueBuffer, CL_TRUE, 0,
//                                sizeof( float ), &mOutputNodataValue, 0, NULL, NULL );
//    ret = clEnqueueWriteBuffer( command_queue, zFactorBuffer, CL_TRUE, 0,
//                                sizeof( double ), &mZFactor, 0, NULL, NULL );
//    ret = clEnqueueWriteBuffer( command_queue, cellSizeXBuffer, CL_TRUE, 0,
//                                sizeof( double ), &mCellSizeX, 0, NULL, NULL );
//    ret = clEnqueueWriteBuffer( command_queue, cellSizeYBuffer, CL_TRUE, 0,
//                                sizeof( double ), &mCellSizeY, 0, NULL, NULL );


//    // Set the arguments of the kernel
//    ret = ret || clSetKernelArg( kernel, 0, sizeof( cl_mem ), ( void * )&scanLine1Buffer );
//    ret = ret || clSetKernelArg( kernel, 1, sizeof( cl_mem ), ( void * )&scanLine2Buffer );
//    ret = ret || clSetKernelArg( kernel, 2, sizeof( cl_mem ), ( void * )&scanLine3Buffer );
//    ret = ret || clSetKernelArg( kernel, 3, sizeof( cl_mem ), ( void * )&resultLineBuffer );
//    ret = ret || clSetKernelArg( kernel, 4, sizeof( cl_mem ), ( void * )&inputNodataValueBuffer );
//    ret = ret || clSetKernelArg( kernel, 5, sizeof( cl_mem ), ( void * )&outputNodataValueBuffer );
//    ret = ret || clSetKernelArg( kernel, 6, sizeof( cl_mem ), ( void * )&zFactorBuffer );
//    ret = ret || clSetKernelArg( kernel, 7, sizeof( cl_mem ), ( void * )&cellSizeXBuffer );
//    ret = ret || clSetKernelArg( kernel, 8, sizeof( cl_mem ), ( void * )&cellSizeYBuffer );


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
//    // Execute the OpenCL kernel on the scan line
//    size_t global_item_size = xSize; // Process the entire lists
//    //size_t local_item_size = 64; // Process in groups of 64 (or NULL for auto)
//    //ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
//    //        &global_item_size, &local_item_size, 0, NULL, NULL);
//    ret = clEnqueueNDRangeKernel( command_queue, kernel, 1, NULL,
//                                  &global_item_size, NULL, 0, NULL, NULL );

    //Q_ASSERT( ret == 0 );

    //const cl_command_queue command_queue = cl::CommandQueue::getDefault()();
    //ret = clEnqueueReadBuffer( command_queue , resultLineBuffer(), CL_TRUE, 0,
    //                           xSize * sizeof( float ), resultLine, 0, NULL, NULL );

    cl::enqueueReadBuffer( resultLineBuffer, CL_TRUE, 0, xSize * sizeof( float ), resultLine );

    if ( GDALRasterIO( outputRasterBand, GF_Write, 0, i, xSize, 1, resultLine, xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
    {
      QgsDebugMsg( "Raster IO Error" );
    }

  }

#else

    // j is the x axis index, skip 0 and last cell that hve been filled with nodata
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
#endif

  CPLFree( resultLine );
  CPLFree( scanLine1 );
  CPLFree( scanLine2 );
  CPLFree( scanLine3 );

  delete source_str;

  if ( feedback && feedback->isCanceled() )
  {
    //delete the dataset without closing (because it is faster)
    gdal::fast_delete_and_close( outputDataset, outputDriver, mOutputFile );
    return 7;
  }
  return 0;
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

