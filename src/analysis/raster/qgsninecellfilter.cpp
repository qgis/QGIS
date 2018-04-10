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
#include "qdebug.h"

#ifdef HAVE_OPENCL
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
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
  // TODO: move to utils and check for errors

  // Get platform and device information
  cl_platform_id platform_id = NULL;
  cl_device_id device_id = NULL;
  cl_uint ret_num_devices;
  cl_uint ret_num_platforms;
  cl_int ret = clGetPlatformIDs( 1, &platform_id, &ret_num_platforms );
  ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1,
                        &device_id, &ret_num_devices );

  // Create an OpenCL context
  cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret );

  // Create a command queue
  cl_command_queue command_queue = clCreateCommandQueue( context, device_id, 0, &ret );

  // Create memory buffers on the device for each vector
  cl_mem scanLine1_mem_obj = clCreateBuffer( context, CL_MEM_READ_ONLY,
                             sizeof( float ) * ( xSize + 2 ), NULL, &ret );
  cl_mem scanLine2_mem_obj = clCreateBuffer( context, CL_MEM_READ_ONLY,
                             sizeof( float ) * ( xSize + 2 ), NULL, &ret );
  cl_mem scanLine3_mem_obj = clCreateBuffer( context, CL_MEM_READ_ONLY,
                             sizeof( float ) * ( xSize + 2 ), NULL, &ret );

  // TODO: constants
  cl_mem inputNodataValue_mem_obj = clCreateBuffer( context, CL_MEM_READ_ONLY,
                                    sizeof( float ), NULL, &ret );
  cl_mem outputNodataValue_mem_obj = clCreateBuffer( context, CL_MEM_READ_ONLY,
                                     sizeof( float ), NULL, &ret );
  cl_mem zFactor_mem_obj = clCreateBuffer( context, CL_MEM_READ_ONLY,
                           sizeof( float ), NULL, &ret );
  cl_mem cellSizeX_mem_obj = clCreateBuffer( context, CL_MEM_READ_ONLY,
                             sizeof( float ), NULL, &ret );
  cl_mem cellSizeY_mem_obj = clCreateBuffer( context, CL_MEM_READ_ONLY,
                             sizeof( float ), NULL, &ret );

  cl_mem resultLine_mem_obj = clCreateBuffer( context, CL_MEM_WRITE_ONLY,
                              sizeof( float ) *  xSize, NULL, &ret );


  const char *source_str = R"pgm(

   #pragma OPENCL EXTENSION cl_khr_fp64 : enable

   float calcFirstDer( float x11, float x21, float x31, float x12, float x22, float x32, float x13, float x23, float x33,
                           float mInputNodataValue, float mOutputNodataValue, float mZFactor, float mCellSize )
   {
     //the basic formula would be simple, but we need to test for nodata values...
     //X: return (( (x31 - x11) + 2 * (x32 - x12) + (x33 - x13) ) / (8 * mCellSizeX));
     //Y: return (((x11 - x13) + 2 * (x21 - x23) + (x31 - x33)) / ( 8 * mCellSizeY));

     int weight = 0;
     double sum = 0;

     //first row
     if ( x31 != mInputNodataValue && x11 != mInputNodataValue ) //the normal case
     {
       sum += ( x31 - x11 );
       weight += 2;
     }
     else if ( x31 == mInputNodataValue && x11 != mInputNodataValue && x21 != mInputNodataValue ) //probably 3x3 window is at the border
     {
       sum += ( x21 - x11 );
       weight += 1;
     }
     else if ( x11 == mInputNodataValue && x31 != mInputNodataValue && x21 != mInputNodataValue ) //probably 3x3 window is at the border
     {
       sum += ( x31 - x21 );
       weight += 1;
     }

     //second row
     if ( x32 != mInputNodataValue && x12 != mInputNodataValue ) //the normal case
     {
       sum += 2 * ( x32 - x12 );
       weight += 4;
     }
     else if ( x32 == mInputNodataValue && x12 != mInputNodataValue && x22 != mInputNodataValue )
     {
       sum += 2 * ( x22 - x12 );
       weight += 2;
     }
     else if ( x12 == mInputNodataValue && x32 != mInputNodataValue && x22 != mInputNodataValue )
     {
       sum += 2 * ( x32 - x22 );
       weight += 2;
     }

     //third row
     if ( x33 != mInputNodataValue && x13 != mInputNodataValue ) //the normal case
     {
       sum += ( x33 - x13 );
       weight += 2;
     }
     else if ( x33 == mInputNodataValue && x13 != mInputNodataValue && x23 != mInputNodataValue )
     {
       sum += ( x23 - x13 );
       weight += 1;
     }
     else if ( x13 == mInputNodataValue && x33 != mInputNodataValue && x23 != mInputNodataValue )
     {
       sum += ( x33 - x23 );
       weight += 1;
     }

     if ( weight == 0 )
     {
       return mOutputNodataValue;
     }

     return sum / ( weight * mCellSize ) * mZFactor;
   }


  __kernel void processNineCellWindow( __global float *scanLine1,
                                       __global float *scanLine2,
                                       __global float *scanLine3,
                                       __global float *resultLine,
                                       __global float *mInputNodataValue,
                                       __global float *mOutputNodataValue,
                                       __global float *mZFactor,
                                       __global float *mCellSizeX,
                                       __global float *mCellSizeY
                           ) {

      // Get the index of the current element
      int i = get_global_id(0);

      // Do the operation
      //return (( (x31 - x11) + 2 * (x32 - x12) + (x33 - x13) ) / (8 * mCellSizeX))
      float derX = calcFirstDer(   scanLine1[i],   scanLine2[i],   scanLine3[i],
                                   scanLine1[i+1], scanLine2[i+1], scanLine3[i+1],
                                   scanLine1[i+2], scanLine2[i+2], scanLine3[i+2],
                                   *mInputNodataValue, *mOutputNodataValue, *mZFactor, *mCellSizeX
                                 );
      //return (((x11 - x13) + 2 * (x21 - x23) + (x31 - x33)) / ( 8 * mCellSizeY));
      float derY = calcFirstDer(   scanLine1[i+2], scanLine1[i+1], scanLine1[i],
                                   scanLine2[i+2], scanLine2[i+1], scanLine2[i],
                                   scanLine3[i+2], scanLine3[i+1], scanLine3[i],
                                   *mInputNodataValue, *mOutputNodataValue, *mZFactor, *mCellSizeY
                                 );

      if ( derX == *mOutputNodataValue || derY == *mOutputNodataValue )
      {
        resultLine[i] = *mOutputNodataValue;
      }
      else
      {
        resultLine[i] = atan( sqrt( derX * derX + derY * derY ) ) * 180.0 / M_PI;
      }
  }
  )pgm";
  // Create a program from the kernel source

  Q_ASSERT( ret == 0 );

  size_t source_size = strlen( source_str );
  cl_program program = clCreateProgramWithSource( context, 1,
                       ( const char ** )&source_str, ( const size_t * )&source_size, &ret );

  // Build the program
  ret = clBuildProgram( program, 1, &device_id, NULL, NULL, NULL );

  if ( ret != 0 )
  {
    char *program_log;
    size_t log_size;
    /* Find size of log and print to std output */
    clGetProgramBuildInfo( program, device_id, CL_PROGRAM_BUILD_LOG,
                           0, NULL, &log_size );
    program_log = ( char * ) malloc( log_size + 1 );
    program_log[log_size] = '\0';
    clGetProgramBuildInfo( program, device_id, CL_PROGRAM_BUILD_LOG,
                           log_size + 1, program_log, NULL );
    qDebug() << program_log;
    free( program_log );
  }

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

    scanLine1[0] = scanLine1[xSize + 1] = mInputNodataValue;
    scanLine2[0] = scanLine2[xSize + 1] = mInputNodataValue;
    scanLine3[0] = scanLine3[xSize + 1] = mInputNodataValue;

#ifdef HAVE_OPENCL
    // Copy the scan lines to their respective memory buffers
    ret = clEnqueueWriteBuffer( command_queue, scanLine1_mem_obj, CL_TRUE, 0,
                                sizeof( float ) * ( xSize + 2 ), scanLine1, 0, NULL, NULL );
    ret = clEnqueueWriteBuffer( command_queue, scanLine2_mem_obj, CL_TRUE, 0,
                                sizeof( float ) * ( xSize + 2 ), scanLine2, 0, NULL, NULL );
    ret = clEnqueueWriteBuffer( command_queue, scanLine3_mem_obj, CL_TRUE, 0,
                                sizeof( float ) * ( xSize + 2 ), scanLine3, 0, NULL, NULL );

    ret = clEnqueueWriteBuffer( command_queue, inputNodataValue_mem_obj, CL_TRUE, 0,
                                sizeof( float ), &mInputNodataValue, 0, NULL, NULL );
    ret = clEnqueueWriteBuffer( command_queue, outputNodataValue_mem_obj, CL_TRUE, 0,
                                sizeof( float ), &mOutputNodataValue, 0, NULL, NULL );
    ret = clEnqueueWriteBuffer( command_queue, zFactor_mem_obj, CL_TRUE, 0,
                                sizeof( float ), &mZFactor, 0, NULL, NULL );
    ret = clEnqueueWriteBuffer( command_queue, cellSizeX_mem_obj, CL_TRUE, 0,
                                sizeof( float ), &mCellSizeX, 0, NULL, NULL );
    ret = clEnqueueWriteBuffer( command_queue, cellSizeY_mem_obj, CL_TRUE, 0,
                                sizeof( float ), &mCellSizeY, 0, NULL, NULL );


    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel( program, "processNineCellWindow", &ret );

    Q_ASSERT( ret == 0 );

    // Set the arguments of the kernel
    ret = ret || clSetKernelArg( kernel, 0, sizeof( cl_mem ), ( void * )&scanLine1_mem_obj );
    ret = ret || clSetKernelArg( kernel, 1, sizeof( cl_mem ), ( void * )&scanLine2_mem_obj );
    ret = ret || clSetKernelArg( kernel, 2, sizeof( cl_mem ), ( void * )&scanLine3_mem_obj );
    ret = ret || clSetKernelArg( kernel, 3, sizeof( cl_mem ), ( void * )&resultLine_mem_obj );
    ret = ret || clSetKernelArg( kernel, 4, sizeof( cl_mem ), ( void * )&inputNodataValue_mem_obj );
    ret = ret || clSetKernelArg( kernel, 5, sizeof( cl_mem ), ( void * )&outputNodataValue_mem_obj );
    ret = ret || clSetKernelArg( kernel, 6, sizeof( cl_mem ), ( void * )&zFactor_mem_obj );
    ret = ret || clSetKernelArg( kernel, 7, sizeof( cl_mem ), ( void * )&cellSizeX_mem_obj );
    ret = ret || clSetKernelArg( kernel, 8, sizeof( cl_mem ), ( void * )&cellSizeY_mem_obj );

    Q_ASSERT( ret == 0 );

    // Execute the OpenCL kernel on the scan line
    size_t global_item_size = xSize; // Process the entire lists
    //size_t local_item_size = 64; // Process in groups of 64 (or NULL for auto)
    //ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
    //        &global_item_size, &local_item_size, 0, NULL, NULL);
    ret = clEnqueueNDRangeKernel( command_queue, kernel, 1, NULL,
                                  &global_item_size, NULL, 0, NULL, NULL );

    Q_ASSERT( ret == 0 );

    ret = clEnqueueReadBuffer( command_queue, resultLine_mem_obj, CL_TRUE, 0,
                               xSize * sizeof( float ), resultLine, 0, NULL, NULL );


    if ( GDALRasterIO( outputRasterBand, GF_Write, 0, i, xSize, 1, resultLine, xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
    {
      QgsDebugMsg( "Raster IO Error" );
    }

    qDebug() << resultLine[1];

    ret = clReleaseKernel( kernel );

  }

  // Clean up
  ret = clFlush( command_queue );
  ret = clFinish( command_queue );
  ret = clReleaseProgram( program );
  ret = clReleaseMemObject( scanLine1_mem_obj );
  ret = clReleaseMemObject( scanLine2_mem_obj );
  ret = clReleaseMemObject( scanLine3_mem_obj );
  ret = clReleaseMemObject( resultLine_mem_obj );
  ret = clReleaseCommandQueue( command_queue );
  ret = clReleaseContext( context );

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

