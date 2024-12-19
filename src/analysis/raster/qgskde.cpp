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

#include "qgskde.h"
#include "qgsfeaturesource.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"

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
  , mDatasetH( nullptr )
  , mRasterBandH( nullptr )
{
  if ( !parameters.radiusField.isEmpty() )
    mRadiusField = mSource->fields().lookupField( parameters.radiusField );
  if ( !parameters.weightField.isEmpty() )
    mWeightField = mSource->fields().lookupField( parameters.weightField );
}

QgsKernelDensityEstimation::Result QgsKernelDensityEstimation::run()
{
  const Result result = prepare();
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
  GDALAllRegister();

  GDALDriverH driver = GDALGetDriverByName( mOutputFormat.toUtf8() );
  if ( !driver )
  {
    return DriverError;
  }

  if ( !mSource )
    return InvalidParameters;

  mBounds = calculateBounds();
  if ( mBounds.isNull() )
    return InvalidParameters;

  const int rows = std::max( std::ceil( mBounds.height() / mPixelSize ) + 1, 1.0 );
  const int cols = std::max( std::ceil( mBounds.width() / mPixelSize ) + 1, 1.0 );

  if ( !createEmptyLayer( driver, mBounds, rows, cols ) )
    return FileCreationError;

  // open the raster in GA_Update mode
  mDatasetH.reset( GDALOpen( mOutputFile.toUtf8().constData(), GA_Update ) );
  if ( !mDatasetH )
    return FileCreationError;
  mRasterBandH = GDALGetRasterBand( mDatasetH.get(), 1 );
  if ( !mRasterBandH )
    return FileCreationError;

  mBufferSize = -1;
  if ( mRadiusField < 0 )
    mBufferSize = radiusSizeInPixels( mRadius );

  return Success;
}

QgsKernelDensityEstimation::Result QgsKernelDensityEstimation::addFeature( const QgsFeature &feature )
{
  const QgsGeometry featureGeometry = feature.geometry();
  if ( featureGeometry.isNull() )
  {
    return Success;
  }

  // convert the geometry to multipoint
  QgsMultiPointXY multiPoints;
  if ( !featureGeometry.isMultipart() )
  {
    const QgsPointXY p = featureGeometry.asPoint();
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
  const int blockSize = 2 * buffer + 1; //Block SIDE would be more appropriate

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
    const unsigned int xPosition = ( ( ( *pointIt ).x() - mBounds.xMinimum() ) / mPixelSize ) - buffer;
    const unsigned int yPosition = ( ( ( *pointIt ).y() - mBounds.yMinimum() ) / mPixelSize ) - buffer;
    const unsigned int yPositionIO = ( ( mBounds.yMaximum() - ( *pointIt ).y() ) / mPixelSize ) - buffer;


    // get the data
    float *dataBuffer = ( float * ) CPLMalloc( sizeof( float ) * blockSize * blockSize );
    if ( GDALRasterIO( mRasterBandH, GF_Read, xPosition, yPositionIO, blockSize, blockSize,
                       dataBuffer, blockSize, blockSize, GDT_Float32, 0, 0 ) != CE_None )
    {
      result = RasterIoError;
    }

    for ( int xp = 0; xp < blockSize; xp++ )
    {
      for ( int yp = 0; yp < blockSize; yp++ )
      {
        const double pixelCentroidX = ( xPosition + xp + 0.5 ) * mPixelSize + mBounds.xMinimum();
        const double pixelCentroidY = ( yPosition + yp + 0.5 ) * mPixelSize + mBounds.yMinimum();

        const double distance = std::sqrt( std::pow( pixelCentroidX - ( *pointIt ).x(), 2.0 ) + std::pow( pixelCentroidY - ( *pointIt ).y(), 2.0 ) );

        // is pixel outside search bandwidth of feature?
        if ( distance > radius )
        {
          continue;
        }

        const double pixelValue = weight * calculateKernelValue( distance, radius, mShape, mOutputValues );
        const int pos = xp + blockSize * yp;
        if ( dataBuffer[ pos ] == NO_DATA )
        {
          dataBuffer[ pos ] = 0;
        }
        dataBuffer[ pos ] += pixelValue;
      }
    }
    if ( GDALRasterIO( mRasterBandH, GF_Write, xPosition, yPositionIO, blockSize, blockSize,
                       dataBuffer, blockSize, blockSize, GDT_Float32, 0, 0 ) != CE_None )
    {
      result = RasterIoError;
    }
    CPLFree( dataBuffer );
  }

  return result;
}

QgsKernelDensityEstimation::Result QgsKernelDensityEstimation::finalise()
{
  mDatasetH.reset();
  mRasterBandH = nullptr;
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

bool QgsKernelDensityEstimation::createEmptyLayer( GDALDriverH driver, const QgsRectangle &bounds, int rows, int columns ) const
{
  double geoTransform[6] = { bounds.xMinimum(), mPixelSize, 0, bounds.yMaximum(), 0, -mPixelSize };
  const gdal::dataset_unique_ptr emptyDataset( GDALCreate( driver, mOutputFile.toUtf8(), columns, rows, 1, GDT_Float32, nullptr ) );
  if ( !emptyDataset )
    return false;

  if ( GDALSetGeoTransform( emptyDataset.get(), geoTransform ) != CE_None )
    return false;

  // Set the projection on the raster destination to match the input layer
  if ( GDALSetProjection( emptyDataset.get(), mSource->sourceCrs().toWkt().toLocal8Bit().data() ) != CE_None )
    return false;

  GDALRasterBandH poBand = GDALGetRasterBand( emptyDataset.get(), 1 );
  if ( !poBand )
    return false;

  if ( GDALSetRasterNoDataValue( poBand, NO_DATA ) != CE_None )
    return false;

  float *line = static_cast< float * >( CPLMalloc( sizeof( float ) * columns ) );
  for ( int i = 0; i < columns; i++ )
  {
    line[i] = NO_DATA;
  }
  // Write the empty raster
  for ( int i = 0; i < rows ; i++ )
  {
    if ( GDALRasterIO( poBand, GF_Write, 0, i, columns, 1, line, columns, 1, GDT_Float32, 0, 0 ) != CE_None )
    {
      return false;
    }
  }

  CPLFree( line );
  return true;
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
      const double k = 2. / ( M_PI * bandwidth );

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
      const double k = 116. / ( 5. * M_PI * std::pow( bandwidth, 2 ) );

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
      const double k = 128. / ( 35. * M_PI * std::pow( bandwidth, 2 ) );

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
      const double k = 8. / ( 3. * M_PI * std::pow( bandwidth, 2 ) );

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
        const double k = 3. / ( ( 1. + 2. * mDecay ) * M_PI * std::pow( bandwidth, 2 ) );

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




