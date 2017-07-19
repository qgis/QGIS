/***************************************************************************
  qgskde.cpp
  ----------
  Date                 : October 2016
  Copyright            : (C) 2016 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QByteArray>

#include "qgskde.h"
#include "qgsfeaturesource.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsrasterfilewriter.h"

#define NO_DATA -9999

QgsKernelDensityEstimation::QgsKernelDensityEstimation( const QgsKernelDensityEstimation::Parameters &parameters, const QString &outputFile, const QString &outputFormat )
  : mSource( parameters.source )
  , mOutputFile( outputFile )
  , mOutputFormat( outputFormat )
  , mRadiusField( -1 )
  , mWeightField( -1 )
  , mRadius( parameters.radius )
  , mPixelSize( parameters.pixelSize )
  , mShape( parameters.shape )
  , mDecay( parameters.decayRatio )
  , mOutputValues( parameters.outputValues )
  , mBufferSize( -1 )
{
  if ( !parameters.radiusField.isEmpty() )
    mRadiusField = mSource->fields().lookupField( parameters.radiusField );
  if ( !parameters.weightField.isEmpty() )
    mWeightField = mSource->fields().lookupField( parameters.weightField );
}

QgsKernelDensityEstimation::Result QgsKernelDensityEstimation::run()
{
  Result result = prepare();
  if ( result != Success )
    return result;

  QgsAttributeList requiredAttributes;

  if ( mRadiusField >= 0 )
    requiredAttributes << mRadiusField;

  if ( mWeightField >= 0 )
    requiredAttributes << mWeightField;

  QgsFeatureIterator fit = mSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( requiredAttributes ) );

  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    addFeature( f );
  }

  return finalise();
}

QgsKernelDensityEstimation::Result QgsKernelDensityEstimation::prepare()
{
  if ( !mSource )
    return InvalidParameters;

  mBounds = calculateBounds();
  if ( mBounds.isNull() )
    return InvalidParameters;

  int rows = std::max( std::ceil( mBounds.height() / mPixelSize ) + 1, 1.0 );
  int cols = std::max( std::ceil( mBounds.width() / mPixelSize ) + 1, 1.0 );

  // create empty raster and fill it with nodata values
  QgsRasterFileWriter writer( mOutputFile );
  writer.setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer.setOutputFormat( mOutputFormat );
  mProvider = writer.createOneBandRaster( Qgis::Float32, cols, rows, mBounds, mSource->sourceCrs() );
  if ( !mProvider )
  {
    return FileCreationError;
  }

  if ( !mProvider->setNoDataValue( 1, NO_DATA ) )
  {
    return FileCreationError;
  }

  int dataTypeSize = QgsRasterBlock::typeSize( Qgis::Float32 );
  std::vector<float> line( cols );
  std::fill( line.begin(), line.end(), NO_DATA );
  QgsRasterBlock block( Qgis::Float32, cols, 1 );
  block.setData( QByteArray::fromRawData( ( char * )&line[0], dataTypeSize * cols ) );
  for ( int i = 0; i < rows ; i++ )
  {
    mProvider->writeBlock( &block, 1, 0, i );
  }

  mBufferSize = -1;
  if ( mRadiusField < 0 )
    mBufferSize = radiusSizeInPixels( mRadius );

  return Success;
}

QgsKernelDensityEstimation::Result QgsKernelDensityEstimation::addFeature( const QgsFeature &feature )
{
  QgsGeometry featureGeometry = feature.geometry();
  if ( featureGeometry.isNull() )
  {
    return Success;
  }

  // convert the geometry to multipoint
  QgsMultiPointXY multiPoints;
  if ( !featureGeometry.isMultipart() )
  {
    QgsPointXY p = featureGeometry.asPoint();
    // avoiding any empty points or out of extent points
    if ( !mBounds.contains( p ) )
      return Success;

    multiPoints << p;
  }
  else
  {
    multiPoints = featureGeometry.asMultiPoint();
  }

  // if radius is variable then fetch it and calculate new pixel buffer size
  double radius = mRadius;
  int buffer = mBufferSize;
  if ( mRadiusField >= 0 )
  {
    radius = feature.attribute( mRadiusField ).toDouble();
    buffer = radiusSizeInPixels( radius );
  }
  int blockSize = 2 * buffer + 1; //Block SIDE would be more appropriate

  // calculate weight
  double weight = 1.0;
  if ( mWeightField >= 0 )
  {
    weight = feature.attribute( mWeightField ).toDouble();
  }

  Result result = Success;

  //loop through all points in multipoint
  for ( QgsMultiPointXY::const_iterator pointIt = multiPoints.constBegin(); pointIt != multiPoints.constEnd(); ++pointIt )
  {
    // avoiding any empty points or out of extent points
    if ( !mBounds.contains( *pointIt ) )
    {
      continue;
    }

    // calculate the pixel position
    unsigned int xPosition = ( ( ( *pointIt ).x() - mBounds.xMinimum() ) / mPixelSize ) - buffer;
    unsigned int yPosition = ( ( ( *pointIt ).y() - mBounds.yMinimum() ) / mPixelSize ) - buffer;
    unsigned int yPositionIO = ( ( mBounds.yMaximum() - ( *pointIt ).y() ) / mPixelSize ) - buffer;


    // calculate buffer around pixel
    QgsRectangle extent( ( *pointIt ).x() - radius, ( *pointIt ).y() - radius, ( *pointIt ).x() + radius, ( *pointIt ).y() + radius );

    // get the data
    QgsRasterBlock *block = mProvider->block( 1, extent, blockSize, blockSize );
    QByteArray blockData = block->data();
    float *dataBuffer = ( float * ) blockData.data();

    for ( int xp = 0; xp < blockSize; xp++ )
    {
      for ( int yp = 0; yp < blockSize; yp++ )
      {
        double pixelCentroidX = ( xPosition + xp + 0.5 ) * mPixelSize + mBounds.xMinimum();
        double pixelCentroidY = ( yPosition + yp + 0.5 ) * mPixelSize + mBounds.yMinimum();

        double distance = std::sqrt( std::pow( pixelCentroidX - ( *pointIt ).x(), 2.0 ) + std::pow( pixelCentroidY - ( *pointIt ).y(), 2.0 ) );

        // is pixel outside search bandwidth of feature?
        if ( distance > radius )
        {
          continue;
        }

        double pixelValue = weight * calculateKernelValue( distance, radius, mShape, mOutputValues );
        int pos = xp + blockSize * yp;
        if ( dataBuffer[ pos ] == NO_DATA )
        {
          dataBuffer[ pos ] = 0;
        }
        dataBuffer[ pos ] += pixelValue;
      }
    }

    block->setData( blockData );
    if ( !mProvider->writeBlock( block, 1, xPosition, yPositionIO ) )
    {
      result = RasterIoError;
    }

    delete block;
  }

  return result;
}

QgsKernelDensityEstimation::Result QgsKernelDensityEstimation::finalise()
{
  mProvider->setEditable( false );
  mProvider = nullptr;

  return Success;
}

int QgsKernelDensityEstimation::radiusSizeInPixels( double radius ) const
{
  int buffer = radius / mPixelSize;
  if ( radius - ( mPixelSize * buffer ) > 0.5 )
  {
    ++buffer;
  }
  return buffer;
}

double QgsKernelDensityEstimation::calculateKernelValue( const double distance, const double bandwidth, const QgsKernelDensityEstimation::KernelShape shape, const QgsKernelDensityEstimation::OutputValues outputType ) const
{
  switch ( shape )
  {
    case KernelTriangular:
      return triangularKernel( distance, bandwidth, outputType );

    case KernelUniform:
      return uniformKernel( distance, bandwidth, outputType );

    case KernelQuartic:
      return quarticKernel( distance, bandwidth, outputType );

    case KernelTriweight:
      return triweightKernel( distance, bandwidth, outputType );

    case KernelEpanechnikov:
      return epanechnikovKernel( distance, bandwidth, outputType );
  }
  return 0; //no warnings
}

/* The kernel functions below are taken from "Kernel Smoothing" by Wand and Jones (1995), p. 175
 *
 * Each kernel is multiplied by a normalizing constant "k", which normalizes the kernel area
 * to 1 for a given bandwidth size.
 *
 * k is calculated by polar double integration of the kernel function
 * between a radius of 0 to the specified bandwidth and equating the area to 1. */

double QgsKernelDensityEstimation::uniformKernel( const double distance, const double bandwidth, const QgsKernelDensityEstimation::OutputValues outputType ) const
{
  Q_UNUSED( distance )
  switch ( outputType )
  {
    case OutputScaled:
    {
      // Normalizing constant
      double k = 2. / ( M_PI * bandwidth );

      // Derived from Wand and Jones (1995), p. 175
      return k * ( 0.5 / bandwidth );
    }
    case OutputRaw:
      return 1.0;
  }
  return 0.0; // NO warnings!!!!!
}

double QgsKernelDensityEstimation::quarticKernel( const double distance, const double bandwidth, const QgsKernelDensityEstimation::OutputValues outputType ) const
{
  switch ( outputType )
  {
    case OutputScaled:
    {
      // Normalizing constant
      double k = 116. / ( 5. * M_PI * std::pow( bandwidth, 2 ) );

      // Derived from Wand and Jones (1995), p. 175
      return k * ( 15. / 16. ) * std::pow( 1. - std::pow( distance / bandwidth, 2 ), 2 );
    }
    case OutputRaw:
      return std::pow( 1. - std::pow( distance / bandwidth, 2 ), 2 );
  }
  return 0.0; //no, seriously, I told you NO WARNINGS!
}

double QgsKernelDensityEstimation::triweightKernel( const double distance, const double bandwidth, const QgsKernelDensityEstimation::OutputValues outputType ) const
{
  switch ( outputType )
  {
    case OutputScaled:
    {
      // Normalizing constant
      double k = 128. / ( 35. * M_PI * std::pow( bandwidth, 2 ) );

      // Derived from Wand and Jones (1995), p. 175
      return k * ( 35. / 32. ) * std::pow( 1. - std::pow( distance / bandwidth, 2 ), 3 );
    }
    case OutputRaw:
      return std::pow( 1. - std::pow( distance / bandwidth, 2 ), 3 );
  }
  return 0.0; // this is getting ridiculous... don't you ever listen to a word I say?
}

double QgsKernelDensityEstimation::epanechnikovKernel( const double distance, const double bandwidth, const QgsKernelDensityEstimation::OutputValues outputType ) const
{
  switch ( outputType )
  {
    case OutputScaled:
    {
      // Normalizing constant
      double k = 8. / ( 3. * M_PI * std::pow( bandwidth, 2 ) );

      // Derived from Wand and Jones (1995), p. 175
      return k * ( 3. / 4. ) * ( 1. - std::pow( distance / bandwidth, 2 ) );
    }
    case OutputRaw:
      return ( 1. - std::pow( distance / bandwidth, 2 ) );
  }

  return 0.0; // la la la i'm not listening
}

double QgsKernelDensityEstimation::triangularKernel( const double distance, const double bandwidth, const QgsKernelDensityEstimation::OutputValues outputType ) const
{
  switch ( outputType )
  {
    case OutputScaled:
    {
      // Normalizing constant. In this case it's calculated a little different
      // due to the inclusion of the non-standard "decay" parameter

      if ( mDecay >= 0 )
      {
        double k = 3. / ( ( 1. + 2. * mDecay ) * M_PI * std::pow( bandwidth, 2 ) );

        // Derived from Wand and Jones (1995), p. 175 (with addition of decay parameter)
        return k * ( 1. - ( 1. - mDecay ) * ( distance / bandwidth ) );
      }
      else
      {
        // Non-standard or mathematically valid negative decay ("coolmap")
        return ( 1. - ( 1. - mDecay ) * ( distance / bandwidth ) );
      }
    }
    case OutputRaw:
      return ( 1. - ( 1. - mDecay ) * ( distance / bandwidth ) );
  }
  return 0.0; // ....
}

QgsRectangle QgsKernelDensityEstimation::calculateBounds() const
{
  if ( !mSource )
    return QgsRectangle();

  QgsRectangle bbox = mSource->sourceExtent();

  double radius = 0;
  if ( mRadiusField >= 0 )
  {
    // if radius is using a field, find the max value
    radius = mSource->maximumValue( mRadiusField ).toDouble();
  }
  else
  {
    radius = mRadius;
  }
  // expand the bounds by the maximum search radius
  bbox.setXMinimum( bbox.xMinimum() - radius );
  bbox.setYMinimum( bbox.yMinimum() - radius );
  bbox.setXMaximum( bbox.xMaximum() + radius );
  bbox.setYMaximum( bbox.yMaximum() + radius );
  return bbox;
}
