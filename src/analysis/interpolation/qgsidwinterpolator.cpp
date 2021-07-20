/***************************************************************************
                              qgsidwinterpolator.cpp
                              ----------------------
  begin                : Marco 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
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

#include "qgsidwinterpolator.h"
#include "qgis.h"
#include "qgsopenclutils.h"
#include "qgsmessagelog.h"
#include "qgsfeedback.h"
#include <cmath>
#include <limits>

QgsIDWInterpolator::QgsIDWInterpolator( const QList<LayerData> &layerData )
  : QgsInterpolator( layerData )
{

#ifdef HAVE_OPENCL

  if ( QgsOpenClUtils::enabled() || QgsOpenClUtils::available() )
  {
    // TODO: check if layer data fit in OpenCL memory
    const QString source( QgsOpenClUtils::sourceFromBaseName( QStringLiteral( "idw" ) ) );
    if ( ! source.isEmpty() )
    {
      // Check double precision support
      const QString extensions { QgsOpenClUtils::activeDeviceInfo( QgsOpenClUtils::Info::Extensions ) };
      if ( ! extensions.contains( QStringLiteral( "cl_khr_fp64" ) ) || ! extensions.contains( "cl_khr_int64_base_atomics" ) )
      {
        QgsMessageLog::logMessage( QObject::tr( "Active OpenCL device does not support double precision or double precision atomic operations, falling back to CPU implementation." ), QgsOpenClUtils::LOGMESSAGE_TAG, Qgis::MessageLevel::Warning );
      }
      else
      {
        mUseOpenCl = true;
      }
    }
  }
#endif

}


int QgsIDWInterpolator::interpolatePoint( double x, double y, double &result, QgsFeedback *feedback )
{

#ifdef HAVE_OPENCL
  // Buffer sizes
  std::size_t dataBufferSize;
  static const std::size_t resultBufferSize { sizeof( double ) * 2 };

#endif
  if ( !mDataIsCached )
  {
    cacheBaseData( feedback );
#ifdef HAVE_OPENCL
    if ( mUseOpenCl )
    {
      // Initialize OpenCL
      dataBufferSize = sizeof( double ) * 3 * mCachedBaseData.size();

      try
      {
        const QString source( QgsOpenClUtils::sourceFromBaseName( QStringLiteral( "idw" ) ) );
        Q_ASSERT( ! source.isEmpty() );
        mCtx = QgsOpenClUtils::context();
        mQueue = QgsOpenClUtils::commandQueue();
        cl::Program program( QgsOpenClUtils::buildProgram( source, QgsOpenClUtils::ExceptionBehavior::Throw ) );
        mKernel = std::make_shared<cl::KernelFunctor <
                  cl::Buffer &,  // input buffer 2 doubles: (x, y)
                  cl::Buffer &,  // data buffer, size of input data * double * 3
                  cl::Buffer &   // results buffer 2 doubles: sumCounter and sumDenominator
                  >> ( program, "calculateIDWDistance" );
        mDataBuffer = cl::Buffer( mCtx, CL_MEM_READ_ONLY, dataBufferSize, nullptr, nullptr ) ;
        // Enqueue data
        mQueue.enqueueWriteBuffer( mDataBuffer, CL_TRUE, 0, dataBufferSize, mCachedBaseData.data() );
      }
      catch ( cl::Error &e )
      {
        QString err = QObject::tr( "Error running OpenCL program: %1 - %2" ).arg( e.what( ), QgsOpenClUtils::errorText( e.err( ) ) );
        QgsMessageLog::logMessage( err, QgsOpenClUtils::LOGMESSAGE_TAG, Qgis::MessageLevel::Critical );
        mUseOpenCl = false;
      }
    }
#endif
  }

  if ( feedback && feedback->isCanceled() )
  {
    return 1;
  }

#ifdef HAVE_OPENCL
  if ( mUseOpenCl )
  {
    std::vector<double> inputParams;
    inputParams.push_back( x );
    inputParams.push_back( y );
    inputParams.push_back( mDistanceCoefficient );
    cl::Buffer inputBuffer( mQueue, inputParams.begin(), inputParams.end(), true, false, nullptr );
    cl::Buffer resultBuffer( mCtx, CL_MEM_READ_WRITE, resultBufferSize, nullptr, nullptr );
    const std::vector<double> zeros { 0, 0 };
    mQueue.enqueueWriteBuffer( resultBuffer, CL_TRUE, 0, resultBufferSize, &zeros );

    mKernel->operator()( cl::EnqueueArgs(
                           mQueue,
                           cl::NDRange( mCachedBaseData.size() )
                         ),
                         inputBuffer,
                         mDataBuffer,
                         resultBuffer
                       );

    double clResult[2];
    mQueue.enqueueReadBuffer( resultBuffer, CL_TRUE, 0, resultBufferSize, &clResult );
    const double sumCounter { clResult[0] };
    const double sumDenominator{ clResult[1] };
    result = sumCounter / sumDenominator;
    return 0;
  }
#endif

  double sumCounter = 0;
  double sumDenominator = 0;

  for ( const QgsInterpolatorVertexData &vertex : std::as_const( mCachedBaseData ) )
  {
    if ( feedback && feedback->isCanceled() )
    {
      return 1;
    }
    double distance = std::sqrt( ( vertex.x - x ) * ( vertex.x - x ) + ( vertex.y - y ) * ( vertex.y - y ) );

    if ( qgsDoubleNear( distance, 0.0 ) )
    {
      result = vertex.z;
      return 0;
    }
    double currentWeight = 1 / ( std::pow( distance, mDistanceCoefficient ) );
    sumCounter += ( currentWeight * vertex.z );
    sumDenominator += currentWeight;
    printf( "Distance: %f\n", distance );
    printf( "CurrentWeight: %f\n", currentWeight );
    printf( "SumCounter: %f\n", sumCounter );
    printf( "SumDenominator: %f\n", sumDenominator );
  }

  if ( sumDenominator == 0.0 )
  {
    return 1;
  }

  result = sumCounter / sumDenominator;
  return 0;
}
