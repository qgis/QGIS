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

QgsAidwInterpolation::QgsAidwInterpolation( QgsVectorLayer *dataLayer, QString &dataAttributeName, QgsRasterLayer *interpolatedLayer, double coefficient )
  : mDataLayer( dataLayer )
  , mInterpolatedLayer( interpolatedLayer )
  , mDataAttributeName( dataAttributeName )
  , mCoefficient( coefficient )
{

}

void QgsAidwInterpolation::interpolate( QgsFeedback *feedback )
{

  const int attrId { mDataLayer->fields().lookupField( mDataAttributeName ) };
  Q_ASSERT( attrId >= 0 );

  long long featureCount { mDataLayer->featureCount() };

  if ( featureCount <= 0 )
  {
    throw QgsProcessingException( QObject::tr( "Feature count for data layer '%1' is not known or the layer is empty" ).arg( mDataLayer->name() ) );
  }

  if ( ! mInterpolatedLayer->dataProvider()->isEditable() && ! mInterpolatedLayer->dataProvider()->setEditable( true ) )
  {
    throw QgsProcessingException( QObject::tr( "Could not edit layer '%1': dataset is read only" ).arg( mInterpolatedLayer->name() ) );
  }

  Q_ASSERT( mInterpolatedLayer->dataProvider()->isEditable() );

  const int columnCount { mInterpolatedLayer->width() };
  const int rowCount = { mInterpolatedLayer->height() };

  if ( rowCount <= 0 || columnCount <= 0 )
  {
    throw QgsProcessingException( QObject::tr( "Could not write on an empty (%1x%2) layer '%3'" )
                                  .arg( columnCount )
                                  .arg( rowCount )
                                  .arg( mInterpolatedLayer->name() ) );
  }
  // Size of a reult row
  const std::size_t resultRowSize( sizeof( cl_double ) * columnCount );

  QgsRectangle requestedExtent { mInterpolatedLayer->extent( ) };

  // Output step size
  cl_double xStep { static_cast<cl_double>( requestedExtent.width() / mInterpolatedLayer->width() ) };
  cl_double yStep { static_cast<cl_double>( requestedExtent.height() / mInterpolatedLayer->height() ) };

  const cl_double xMin { static_cast<cl_double>( mInterpolatedLayer->extent( ).xMinimum() + xStep / 2 ) }; // Center x of first cell
  cl_double yMin { static_cast<cl_double>( mInterpolatedLayer->extent( ).yMinimum() + yStep / 2 ) }; // Center y of first cell, will be incremented on each row iteration

  // Prepare context and queue
  cl::Context ctx = QgsOpenClUtils::context();
  cl::CommandQueue queue = QgsOpenClUtils::commandQueue();

  QgsOpenClUtils::CPLAllocator<cl_double> resultBlock( resultRowSize );

  // Fill up the data array
  QgsFeatureRequest req;
  if ( mInterpolatedLayer->crs() != mDataLayer->crs( ) )
  {
    const QgsCoordinateTransform tranformer { mInterpolatedLayer->crs(), mDataLayer->crs( ), mInterpolatedLayer->transformContext() };
    requestedExtent = tranformer.transform( requestedExtent );
    req.setDestinationCrs( mInterpolatedLayer->crs(), mInterpolatedLayer->transformContext() );
  }
  req.setFilterRect( requestedExtent );
  req.setSubsetOfAttributes( QgsAttributeList() << attrId );

  struct VectorData
  {
    cl_double x;
    cl_double y;
    cl_double z;  //!< This is the interpolation value
  };

  std::vector<VectorData> vectorData;
  vectorData.reserve( featureCount );

  QgsFeature f;
  QgsFeatureIterator featureIterator { mDataLayer->getFeatures( req )};
  long long featureIdx { 0 };
  while ( featureIterator.nextFeature( f ) )
  {
    featureIdx++;
    if ( feedback )
    {
      if ( feedback->isCanceled() )
        return;
      feedback->setProgress( 100.0 * featureIdx / featureCount );
    }
    const QgsPointXY point { f.geometry().asPoint() };
    vectorData.push_back( VectorData{ static_cast<cl_double>( point.x() ), static_cast<cl_double>( point.y() ), static_cast<cl_double>( f.attribute( attrId ).toDouble() ) } );
  }


  // Create I/O buffers
  cl::Buffer resultBuffer( ctx, CL_MEM_WRITE_ONLY, resultRowSize );
  // Create and fill
  cl::Buffer dataBuffer( queue, vectorData.begin(), vectorData.end(), true, false );

  // Create a program from the kernel source
  const QString source( QgsOpenClUtils::sourceFromBaseName( QStringLiteral( "aidw" ) ) );
  cl::Program program( QgsOpenClUtils::buildProgram( source, QgsOpenClUtils::ExceptionBehavior::Throw ) );

  // Create the OpenCL kernel
  cl::Kernel kernel { cl::Kernel( program, "idw" ) };
  kernel.setArg( 0, sizeof( xStep ), &xStep );  // private
  kernel.setArg( 1, sizeof( yStep ), &yStep );  // private
  kernel.setArg( 2, sizeof( xMin ), &xMin );  // private
  //     setArg  3 yMin vary on each iteration
  kernel.setArg( 4, sizeof( columnCount ), &columnCount );  // private
  // Pass an unsigned long or NVidia compiler crashes :/
  const cl_ulong ulFeatureCount { static_cast<cl_ulong>( featureCount ) };
  kernel.setArg( 5, sizeof( ulFeatureCount ), &ulFeatureCount );  // private
  cl_double coefficient { static_cast<cl_double>( mCoefficient )};
  kernel.setArg( 6, sizeof( coefficient ), &coefficient );  // private
  kernel.setArg( 7, sizeof( dataBuffer ), &dataBuffer ); // global
  kernel.setArg( 8, sizeof( resultRowSize ), &resultBuffer ); // global

  // Process one row at a time
  for ( int row = 0; row < rowCount; ++row )
  {
    if ( feedback )
    {
      if ( feedback->isCanceled() )
        return;
      feedback->setProgress( 100.0 * row / rowCount );
    }

    kernel.setArg( 3, sizeof( yMin ), &yMin );  // private

    // Start calculation for a row: nd range is column count
    queue.enqueueNDRangeKernel( kernel, 0, columnCount );
    queue.enqueueReadBuffer( resultBuffer, CL_TRUE, 0, resultRowSize, resultBlock.get() );

    // TODO: convert to destination raster data type if different than double

    /* Debug
    for ( int i = 0; i < columnCount; i++ )
    {
      qDebug() << "Value" << i << resultBlock.get()[i];
    }
    //*/

    // Write result to destination raster
    const bool writeOperationResult { mInterpolatedLayer->dataProvider()->write( static_cast<void *>( resultBlock.get() ),
                                      1 /* band */,
                                      columnCount /* width */,
                                      1 /* height */,
                                      0 /* xoffset */,
                                      rowCount - row - 1 /* y offset */ ) };

    if ( ! writeOperationResult )
    {
      throw QgsProcessingException( QObject::tr( "Could not write interpolated data to destination raster '%1'" ).arg( mInterpolatedLayer->name() ) );
    }

    // Increment for the next row
    yMin += yStep;
  }

  mInterpolatedLayer->dataProvider()->reloadData();

}

void QgsAidwInterpolation::process( QgsFeedback *feedback )
{
  // check that data layer is a point layer
  if ( mDataLayer->geometryType() != QgsWkbTypes::GeometryType::PointGeometry )
  {
    throw QgsProcessingException( QObject::tr( "Input layer '%1' geometry type is not point" ).arg( mDataLayer->name() ) );
  }

  if ( mInterpolatedLayer->dataProvider()->dataType( 1 ) != Qgis::DataType::Float64 )
  {
    throw QgsProcessingException( QObject::tr( "Destination layer '%1' type is not Float64 (double precision)" ).arg( mInterpolatedLayer->name() ) );
  }

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
