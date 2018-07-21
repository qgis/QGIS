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

#ifdef HAVE_OPENCL
#include "qgsopenclutils.h"
#endif

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

// TODO: return an anum instead of an int
int QgsNineCellFilter::processRaster( QgsFeedback *feedback )
{
#ifdef HAVE_OPENCL
  if ( QgsOpenClUtils::enabled() && QgsOpenClUtils::available() && ! openClProgramBaseName( ).isEmpty() )
  {
    // Load the program sources
    QString source( QgsOpenClUtils::sourceFromBaseName( openClProgramBaseName( ) ) );
    if ( ! source.isEmpty() )
    {
      try
      {
        QgsDebugMsg( QObject::tr( "Running OpenCL program: %1" )
                     .arg( openClProgramBaseName( ) ), QgsOpenClUtils::LOGMESSAGE_TAG, Qgis::Info );
        return processRasterGPU( source, feedback );
      }
      catch ( cl::Error &e )
      {
        QString err = QObject::tr( "Error running OpenCL program: %1 - %2" ).arg( e.what( ) ).arg( QgsOpenClUtils::errorText( e.err( ) ) );
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

#ifdef HAVE_OPENCL

// TODO: return an anum instead of an int
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

  // Prepare context and queue
  cl::Context ctx = QgsOpenClUtils::context();
<<<<<<< 3bad167572f04c553d1e3d60f9c15d3f8511365f
<<<<<<< 16a49cddaa18cb6d0b12335fe24c68cda183e1c0
  cl::CommandQueue queue( ctx );

  //keep only three scanlines in memory at a time, make room for initial and final nodata
  QgsOpenClUtils::CPLAllocator<float> scanLine( xSize + 2 );
=======
  cl::Context::setDefault( ctx );
  cl::CommandQueue queue( ctx );

  //keep only three scanlines in memory at a time, make room for initial and final nodata
  QgsOpenClUtils::CPLAllocator<float> scanLine1( xSize + 2 );
  QgsOpenClUtils::CPLAllocator<float> scanLine2( xSize + 2 );
  QgsOpenClUtils::CPLAllocator<float> scanLine3( xSize + 2 );
>>>>>>> Use OpenCL command queue
  QgsOpenClUtils::CPLAllocator<float> resultLine( xSize );

<<<<<<< 12fa896554321892e88ca05407cf125a7ccf92c1
=======
  cl_int errorCode = 0;

>>>>>>> [opencl] Test with image2d
=======
  cl::CommandQueue queue( ctx );

  //keep only three scanlines in memory at a time, make room for initial and final nodata
  QgsOpenClUtils::CPLAllocator<float> scanLine( xSize + 2 );
  QgsOpenClUtils::CPLAllocator<float> resultLine( xSize );

>>>>>>> [opencl] Reduce memory footprint and optimize
  // Cast to float (because double just crashes on some GPUs)
  std::vector<float> rasterParams;

  rasterParams.push_back( mInputNodataValue ); //  0
  rasterParams.push_back( mOutputNodataValue ); // 1
  rasterParams.push_back( mZFactor ); // 2
  rasterParams.push_back( mCellSizeX ); // 3
  rasterParams.push_back( mCellSizeY ); // 4

  // Allow subclasses to add extra params needed for computation:
  // used to pass additional args to opencl program
  addExtraRasterParams( rasterParams );

  std::size_t bufferSize( sizeof( float ) * ( xSize + 2 ) );
<<<<<<< a73bbbad21629d81b9b1d4217a096a930473eb5c
<<<<<<< 3bad167572f04c553d1e3d60f9c15d3f8511365f
  std::size_t inputSize( sizeof( float ) * ( xSize ) );
=======
>>>>>>> [opencl] Reduce memory footprint and optimize
=======
  std::size_t inputSize( sizeof( float ) * ( xSize ) );
>>>>>>> [opencl] Use fast formula for hillshade

  cl::Buffer rasterParamsBuffer( queue, rasterParams.begin(), rasterParams.end(), true, false, nullptr );
  cl::Buffer scanLine1Buffer( ctx, CL_MEM_READ_ONLY, bufferSize, nullptr, nullptr );
  cl::Buffer scanLine2Buffer( ctx, CL_MEM_READ_ONLY, bufferSize, nullptr, nullptr );
  cl::Buffer scanLine3Buffer( ctx, CL_MEM_READ_ONLY, bufferSize, nullptr, nullptr );
<<<<<<< a73bbbad21629d81b9b1d4217a096a930473eb5c
<<<<<<< 3bad167572f04c553d1e3d60f9c15d3f8511365f
  cl::Buffer *scanLineBuffer[3] = {&scanLine1Buffer, &scanLine2Buffer, &scanLine3Buffer};
  cl::Buffer resultLineBuffer( ctx, CL_MEM_WRITE_ONLY, inputSize, nullptr, nullptr );
=======
  cl::Buffer resultLineBuffer( ctx, CL_MEM_WRITE_ONLY, sizeof( float ) * xSize, nullptr, nullptr );
>>>>>>> [opencl] Reduce memory footprint and optimize
=======
  cl::Buffer *scanLineBuffer[3] = {&scanLine1Buffer, &scanLine2Buffer, &scanLine3Buffer};
  cl::Buffer resultLineBuffer( ctx, CL_MEM_WRITE_ONLY, inputSize, nullptr, nullptr );
>>>>>>> [opencl] Use fast formula for hillshade

  // Create a program from the kernel source
  cl::Program program( QgsOpenClUtils::buildProgram( ctx, source, QgsOpenClUtils::ExceptionBehavior::Throw ) );

  // Create the OpenCL kernel
  auto kernel = cl::KernelFunctor <
                cl::Buffer &,
                cl::Buffer &,
                cl::Buffer &,
                cl::Buffer &,
                cl::Buffer &
                > ( program, "processNineCellWindow" );

<<<<<<< a73bbbad21629d81b9b1d4217a096a930473eb5c
<<<<<<< 3bad167572f04c553d1e3d60f9c15d3f8511365f
  // Rotate buffer index
  std::vector<int> rowIndex = {0, 1, 2};

=======
>>>>>>> [opencl] Reduce memory footprint and optimize
=======
  // Rotate buffer index
  std::vector<int> rowIndex = {0, 1, 2};

>>>>>>> [opencl] Use fast formula for hillshade
  // values outside the layer extent (if the 3x3 window is on the border) are sent to the processing method as (input) nodata values
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
<<<<<<< a73bbbad21629d81b9b1d4217a096a930473eb5c
<<<<<<< 3bad167572f04c553d1e3d60f9c15d3f8511365f
      // Fill scanline 1 with (input) nodata for the values above the first row and
      // feed scanline2 with the first actual data row
=======
      // Fill scanline 1 with (input) nodata for the values above the first row and feed scanline2 with the first row
>>>>>>> [opencl] Reduce memory footprint and optimize
=======
      // Fill scanline 1 with (input) nodata for the values above the first row and
      // feed scanline2 with the first actual data row
>>>>>>> [opencl] Use fast formula for hillshade
      for ( int a = 0; a < xSize + 2 ; ++a )
      {
        scanLine[a] = mInputNodataValue;
      }
      queue.enqueueWriteBuffer( scanLine1Buffer, CL_TRUE, 0, bufferSize, scanLine.get() );

<<<<<<< a73bbbad21629d81b9b1d4217a096a930473eb5c
<<<<<<< 3bad167572f04c553d1e3d60f9c15d3f8511365f
      // Read scanline2: first real raster row
      if ( GDALRasterIO( rasterBand, GF_Read, 0, i, xSize, 1, &scanLine[1], xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
      {
        QgsDebugMsg( "Raster IO Error" );
      }
<<<<<<< 16a49cddaa18cb6d0b12335fe24c68cda183e1c0
      queue.enqueueWriteBuffer( scanLine2Buffer, CL_TRUE, 0, bufferSize, scanLine.get() );
=======
    }
    else
    {
      //normally fetch only scanLine3 and release scanline 1 if we move forward one row
      scanLine1.reset( scanLine2.release() );
      scanLine2.reset( scanLine3.release() );
      scanLine3.reset( xSize + 2 );
    }
>>>>>>> Use OpenCL command queue

      // Read scanline3: second real raster row
      if ( GDALRasterIO( rasterBand, GF_Read, 0, i + 1, xSize, 1, &scanLine[1], xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
=======
      // Read scanline2
      if ( GDALRasterIO( rasterBand, GF_Read, 0, 0, xSize, 1, &scanLine[1], xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
=======
      // Read scanline2: first real raster row
      if ( GDALRasterIO( rasterBand, GF_Read, 0, i, xSize, 1, &scanLine[1], xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
>>>>>>> [opencl] Use fast formula for hillshade
      {
        QgsDebugMsg( "Raster IO Error" );
      }
      queue.enqueueWriteBuffer( scanLine2Buffer, CL_TRUE, 0, bufferSize, scanLine.get() );

<<<<<<< a73bbbad21629d81b9b1d4217a096a930473eb5c
      // Read scanline3
      if ( GDALRasterIO( rasterBand, GF_Read, 0, 0, xSize, 1, &scanLine[1], xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
>>>>>>> [opencl] Reduce memory footprint and optimize
=======
      // Read scanline3: second real raster row
      if ( GDALRasterIO( rasterBand, GF_Read, 0, i + 1, xSize, 1, &scanLine[1], xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
>>>>>>> [opencl] Use fast formula for hillshade
      {
        QgsDebugMsg( "Raster IO Error" );
      }
      queue.enqueueWriteBuffer( scanLine3Buffer, CL_TRUE, 0, bufferSize, scanLine.get() );
    }
    else
    {
      // Normally fetch only scanLine3 and move forward one row
<<<<<<< a73bbbad21629d81b9b1d4217a096a930473eb5c
<<<<<<< 3bad167572f04c553d1e3d60f9c15d3f8511365f
      // Read scanline 3, fill the last row with nodata values if it's the last iteration
=======
      queue.enqueueCopyBuffer( scanLine2Buffer, scanLine1Buffer, 0, 0, bufferSize, nullptr, nullptr );
      queue.enqueueCopyBuffer( scanLine3Buffer, scanLine2Buffer, 0, 0, bufferSize, nullptr, nullptr );

      // Read scanline 3
>>>>>>> [opencl] Reduce memory footprint and optimize
=======
      // Read scanline 3, fill the last row with nodata values if it's the last iteration
>>>>>>> [opencl] Use fast formula for hillshade
      if ( i == ySize - 1 ) //fill the row below the bottom with nodata values
      {
        for ( int a = 0; a < xSize + 2; ++a )
        {
          scanLine[a] = mInputNodataValue;
        }
<<<<<<< a73bbbad21629d81b9b1d4217a096a930473eb5c
<<<<<<< 3bad167572f04c553d1e3d60f9c15d3f8511365f
        queue.enqueueWriteBuffer( *scanLineBuffer[rowIndex[2]], CL_TRUE, 0, bufferSize, scanLine.get() ); // row 0
      }
      else // Read line i + 1 and put it into scanline 3
        // Overwrite from input, skip first and last
=======
        queue.enqueueWriteBuffer( scanLine3Buffer, CL_TRUE, 0, bufferSize, scanLine.get() ); // row 0
=======
        queue.enqueueWriteBuffer( *scanLineBuffer[rowIndex[2]], CL_TRUE, 0, bufferSize, scanLine.get() ); // row 0
>>>>>>> [opencl] Use fast formula for hillshade
      }
<<<<<<< 573283f0dcf022e84bd615e84fd2656043a9722b
      else // Overwrite from input, skip first and last
>>>>>>> [opencl] Reduce memory footprint and optimize
=======
      else // Read line i + 1 and put it into scanline 3
        // Overwrite from input, skip first and last
>>>>>>> [opencl] Fix small OpenCL alg issues
      {
        if ( GDALRasterIO( rasterBand, GF_Read, 0, i + 1, xSize, 1, &scanLine[1], xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
        {
          QgsDebugMsg( "Raster IO Error" );
        }
<<<<<<< a73bbbad21629d81b9b1d4217a096a930473eb5c
<<<<<<< 3bad167572f04c553d1e3d60f9c15d3f8511365f
        queue.enqueueWriteBuffer( *scanLineBuffer[rowIndex[2]], CL_TRUE, 0, bufferSize, scanLine.get() ); // row 0
      }
    }
<<<<<<< 16a49cddaa18cb6d0b12335fe24c68cda183e1c0
=======
    // Set first and last extra colums to nodata
    scanLine1[0] = scanLine1[xSize + 1] = mInputNodataValue;
    scanLine2[0] = scanLine2[xSize + 1] = mInputNodataValue;
    scanLine3[0] = scanLine3[xSize + 1] = mInputNodataValue;

    // TODO: There is room for further optimization here: instead of replacing the buffers
    //       we could just replace just the new one (the top row) and switch the order
    //       of buffer arguments in the kernell call.
    errorCode = cl::enqueueWriteBuffer( scanLine1Buffer, CL_TRUE, 0,
                                        sizeof( float ) * ( xSize + 2 ), scanLine1.get() );
    errorCode = cl::enqueueWriteBuffer( scanLine2Buffer, CL_TRUE, 0,
                                        sizeof( float ) * ( xSize + 2 ), scanLine2.get() );
    errorCode = cl::enqueueWriteBuffer( scanLine3Buffer, CL_TRUE, 0,
                                        sizeof( float ) * ( xSize + 2 ), scanLine3.get() );
>>>>>>> Use OpenCL command queue
=======
        queue.enqueueWriteBuffer( scanLine3Buffer, CL_TRUE, 0, bufferSize, scanLine.get() ); // row 0
=======
        queue.enqueueWriteBuffer( *scanLineBuffer[rowIndex[2]], CL_TRUE, 0, bufferSize, scanLine.get() ); // row 0
>>>>>>> [opencl] Use fast formula for hillshade
      }
    }
>>>>>>> [opencl] Reduce memory footprint and optimize

    kernel( cl::EnqueueArgs(
              queue,
              cl::NDRange( xSize )
            ),
            *scanLineBuffer[rowIndex[0]],
            *scanLineBuffer[rowIndex[1]],
            *scanLineBuffer[rowIndex[2]],
            resultLineBuffer,
            rasterParamsBuffer
          );

<<<<<<< a73bbbad21629d81b9b1d4217a096a930473eb5c
<<<<<<< 3bad167572f04c553d1e3d60f9c15d3f8511365f
    queue.enqueueReadBuffer( resultLineBuffer, CL_TRUE, 0, inputSize, resultLine.get() );
=======
    queue.enqueueReadBuffer( resultLineBuffer, CL_TRUE, 0, xSize * sizeof( float ), resultLine.get() );
>>>>>>> [opencl] Reduce memory footprint and optimize
=======
    queue.enqueueReadBuffer( resultLineBuffer, CL_TRUE, 0, inputSize, resultLine.get() );
>>>>>>> [opencl] Use fast formula for hillshade

    if ( GDALRasterIO( outputRasterBand, GF_Write, 0, i, xSize, 1, resultLine.get(), xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
    {
      QgsDebugMsg( "Raster IO Error" );
    }
    std::rotate( rowIndex.begin(), rowIndex.begin() + 1, rowIndex.end() );
  }

  if ( feedback && feedback->isCanceled() )
  {
    //delete the dataset without closing (because it is faster)
    gdal::fast_delete_and_close( outputDataset, outputDriver, mOutputFile );
    return 7;
  }
  return 0;
}
#endif
<<<<<<< 573283f0dcf022e84bd615e84fd2656043a9722b

=======
>>>>>>> [opencl] Fix small OpenCL alg issues

// TODO: return an anum instead of an int
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
  std::size_t bufferSize( sizeof( float ) * ( xSize + 2 ) );
  float *scanLine1 = ( float * ) CPLMalloc( bufferSize );
  float *scanLine2 = ( float * ) CPLMalloc( bufferSize );
  float *scanLine3 = ( float * ) CPLMalloc( bufferSize );

  float *resultLine = ( float * ) CPLMalloc( sizeof( float ) * xSize );

  //values outside the layer extent (if the 3x3 window is on the border) are sent to the processing method as (input) nodata values
  for ( int yIndex = 0; yIndex < ySize; ++yIndex )
  {
    if ( feedback && feedback->isCanceled() )
    {
      break;
    }

    if ( feedback )
    {
      feedback->setProgress( 100.0 * static_cast< double >( yIndex ) / ySize );
    }

    if ( yIndex == 0 )
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
      scanLine3 = ( float * ) CPLMalloc( bufferSize );
    }

    // Read scanline 3
    if ( yIndex == ySize - 1 ) //fill the row below the bottom with nodata values
    {
      for ( int a = 0; a < xSize + 2; ++a )
      {
        scanLine3[a] = mInputNodataValue;
      }
    }
    else
    {
      if ( GDALRasterIO( rasterBand, GF_Read, 0, yIndex + 1, xSize, 1, &scanLine3[1], xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
      {
        QgsDebugMsg( "Raster IO Error" );
      }
    }
    // Set first and last extra columns to nodata
    scanLine1[0] = scanLine1[xSize + 1] = mInputNodataValue;
    scanLine2[0] = scanLine2[xSize + 1] = mInputNodataValue;
    scanLine3[0] = scanLine3[xSize + 1] = mInputNodataValue;



    // j is the x axis index, skip 0 and last cell that have been filled with nodata
    for ( int xIndex = 0; xIndex < xSize ; ++xIndex )
    {
      // cells(x, y) x11, x21, x31, x12, x22, x32, x13, x23, x33
      resultLine[ xIndex ] = processNineCellWindow( &scanLine1[ xIndex ], &scanLine1[ xIndex + 1 ], &scanLine1[ xIndex + 2 ],
                             &scanLine2[ xIndex ], &scanLine2[ xIndex + 1 ], &scanLine2[ xIndex + 2 ],
                             &scanLine3[ xIndex ], &scanLine3[ xIndex + 1 ], &scanLine3[ xIndex + 2 ] );

    }

    if ( GDALRasterIO( outputRasterBand, GF_Write, 0, yIndex, xSize, 1, resultLine, xSize, 1, GDT_Float32, 0, 0 ) != CE_None )
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
