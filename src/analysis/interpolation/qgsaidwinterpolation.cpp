/***************************************************************************
  qgsaidwinterpolation.cpp - QgsAidwInterpolation

 ---------------------
 begin                : 19.7.2021
 copyright            : (C) 2021 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaidwinterpolation.h"
#include "qgsopenclutils.h"
#include "qgsfeedback.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsattributes.h"


#include <QObject>

QgsAidwInterpolation::QgsAidwInterpolation( QgsVectorLayer *dataLayer, QString &dataAttributeName, QgsRasterLayer *interpolatedLayer )
  : mDataLayer( dataLayer )
  , mInterpolatedLayer( interpolatedLayer )
  , mDataAttributeName( dataAttributeName )
{

}

void QgsAidwInterpolation::interpolate( QgsFeedback *feedback )
{

  const int attrId { mDataLayer->fields().lookupField( mDataAttributeName ) };
  Q_ASSERT( attrId >= 0 );

  int tileWidth;
  int tileHeight;
  int tileXOffset = 0;
  int tileYOffset = 0;

  // TODO: Calculate tile size
  tileWidth = mInterpolatedLayer->width();
  tileHeight = mInterpolatedLayer->height();
  unsigned long tileSize = tileWidth * tileHeight;
  std::size_t resultBlockSize( sizeof( float ) * tileSize );
  // TODO: This must be dynamically calculated given the available GPU memory
  const unsigned long maxGpuMemory { 1000000 };
  std::size_t dataSize { std::min<unsigned long>( mDataLayer->featureCount() * sizeof( float ) * 3, maxGpuMemory )};

  // Prepare context and queue
  cl::Context ctx = QgsOpenClUtils::context();
  cl::CommandQueue queue = QgsOpenClUtils::commandQueue();

  QgsOpenClUtils::CPLAllocator<float> resultBlock( resultBlockSize );
  QgsOpenClUtils::CPLAllocator<float> data( dataSize );

  // Fill up the data array
  QgsFeatureRequest req;
  req.setSubsetOfAttributes( QgsAttributeList() << attrId );

  // Create I/O buffers
  cl::Buffer resultBuffer( ctx, CL_MEM_WRITE_ONLY, resultBlockSize, nullptr, nullptr );
  cl::Buffer dataBuffer( ctx, CL_MEM_READ_ONLY, dataSize, nullptr, nullptr );

  // Create a program from the kernel source
  const QString source( QgsOpenClUtils::sourceFromBaseName( QStringLiteral( "aidw" ) ) );
  cl::Program program( QgsOpenClUtils::buildProgram( source, QgsOpenClUtils::ExceptionBehavior::Throw ) );

  // Create the OpenCL kernel
  auto kernel = cl::KernelFunctor <
                cl::Buffer &,
                cl::Buffer &
                > ( program, "aidw" );

  // Calculate
  queue.enqueueWriteBuffer( dataBuffer, CL_TRUE, 0, dataSize, data.get() );

  kernel( cl::EnqueueArgs(
            queue,
            cl::NDRange( tileSize )
          ),
          dataBuffer,
          resultBuffer
        );

  queue.enqueueReadBuffer( resultBuffer, CL_TRUE, 0, resultBlockSize, resultBlock.get() );

  // TODO: reproject if needed

  mInterpolatedLayer->dataProvider()->write( static_cast<void *>( data.get() ), 0, tileWidth, tileHeight, tileXOffset, tileYOffset );
}

void QgsAidwInterpolation::process( QgsFeedback *feedback )
{
  // TODO: check that data layer is a point layer

  const int attrId { mDataLayer->fields().lookupField( mDataAttributeName ) };
  if ( attrId < 0 )
  {
    throw QgsProcessingException( QObject::tr( "Attribute '%1' was not found in data layer" ).arg( mDataAttributeName ) );
  }

  if ( ! QgsOpenClUtils::enabled() || ! QgsOpenClUtils::available() )
  {
    throw QgsProcessingException( QObject::tr( "AIDW interpolation requires OpenCL which is not available" ) );
  }
  const QString source( QgsOpenClUtils::sourceFromBaseName( QStringLiteral( "aidw" ) ) );
  if ( ! source.isEmpty() )
  {
    try
    {
      QgsDebugMsg( QObject::tr( "Running OpenCL program: AIDW" ) );
      // Run program here
      interpolate( feedback );
    }
    catch ( cl::Error &e )
    {
      QString err = QObject::tr( "Error running OpenCL program: %1 - %2" ).arg( e.what( ), QgsOpenClUtils::errorText( e.err( ) ) );
      QgsMessageLog::logMessage( err, QgsOpenClUtils::LOGMESSAGE_TAG, Qgis::MessageLevel::Critical );
      throw QgsProcessingException( err );
    }
  }
  else
  {
    QString err = QObject::tr( "Error loading OpenCL program AIDW" );
    QgsMessageLog::logMessage( err,
                               QgsOpenClUtils::LOGMESSAGE_TAG, Qgis::MessageLevel::Critical );
    throw QgsProcessingException( err );
  }
}
