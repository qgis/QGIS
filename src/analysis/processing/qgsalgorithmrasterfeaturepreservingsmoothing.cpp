/***************************************************************************
                         qgsalgorithmrasterfeaturepreservingsmoothing.cpp
                         ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmrasterfeaturepreservingsmoothing.h"

#include "qgsrasterfilewriter.h"

///@cond PRIVATE

struct Vector2D
{
    double x = 0;
    double y = 0;

    Vector2D( double x = 0, double y = 0 )
      : x( x )
      , y( y )
    {}

    double angleBetweenCos( const Vector2D &other ) const
    {
      const double dot = x * other.x + y * other.y + 1.0;
      const double magSelf = ( x * x + y * y + 1.0 );
      const double magOther = ( other.x * other.x + other.y * other.y + 1.0 );
      return dot / std::sqrt( magSelf * magOther );
    }
};

QString QgsRasterFeaturePreservingSmoothingAlgorithm::name() const
{
  return u"rasterfeaturepreservingsmoothing"_s;
}

QString QgsRasterFeaturePreservingSmoothingAlgorithm::displayName() const
{
  return QObject::tr( "Feature preserving DEM smoothing" );
}

QStringList QgsRasterFeaturePreservingSmoothingAlgorithm::tags() const
{
  return QObject::tr( "smooth,filter,denoise,fpdems,blur" ).split( ',' );
}

QString QgsRasterFeaturePreservingSmoothingAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterFeaturePreservingSmoothingAlgorithm::groupId() const
{
  return u"rasteranalysis"_s;
}

QString QgsRasterFeaturePreservingSmoothingAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm applies the Feature-Preserving DEM Smoothing (FPDEMS) method, as described by Lindsay et al. (2019).\n\n"
                      "It is effective at removing surface roughness from Digital Elevation Models (DEMs) without significantly altering sharp features such as breaks-in-slope, stream banks, or terrace scarps. "
                      "This makes it superior to standard low-pass filters (e.g., mean, median, Gaussian) or resampling, which often blur distinct topographic features.\n\n"
                      "The algorithm works in three steps:\n"
                      "1. Calculating surface normal 3D vectors for each grid cell.\n"
                      "2. Smoothing the normal vector field using a filter that applies more weight to neighbors with similar surface normals (preserving edges).\n"
                      "3. Iteratively updating the elevations in the DEM to match the smoothed normal field.\n\n"
                      "References: Lindsay, John et al (2019): LiDAR DEM Smoothing and the Preservation of Drainage Features. Remote Sensing, Vol. 11, Issue 16, https://doi.org/10.3390/rs11161926"
  );
}

QString QgsRasterFeaturePreservingSmoothingAlgorithm::shortDescription() const
{
  return QObject::tr( "Smooths a DEM while preserving topographic features." );
}

void QgsRasterFeaturePreservingSmoothingAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterBand( u"BAND"_s, QObject::tr( "Band number" ), 1, u"INPUT"_s ) );

  auto radiusParam = std::make_unique<QgsProcessingParameterNumber>( u"RADIUS"_s, QObject::tr( "Filter radius (pixels)" ), Qgis::ProcessingNumberParameterType::Integer, 5, false, 1, 50 );
  radiusParam->setHelp( QObject::tr( "Radius of the filter kernel. A radius of 5 results in an 11x11 kernel." ) );
  addParameter( radiusParam.release() );

  auto thresholdParam = std::make_unique<QgsProcessingParameterNumber>( u"THRESHOLD"_s, QObject::tr( "Normal difference threshold (degrees)" ), Qgis::ProcessingNumberParameterType::Double, 15.0, false, 0.0, 90.0 );
  thresholdParam->setHelp( QObject::tr( "Maximum angular difference (in degrees) between the normal vector of the center cell and a neighbor for the neighbor to be included in the filter. Higher values result in more neighbors being included, producing smoother surfaces. A range of 10-20 degrees is typically optimal." ) );
  addParameter( thresholdParam.release() );

  auto iterParam = std::make_unique<QgsProcessingParameterNumber>( u"ITERATIONS"_s, QObject::tr( "Elevation update iterations" ), Qgis::ProcessingNumberParameterType::Integer, 3, false, 1, 50 );
  iterParam->setHelp( QObject::tr( "Number of times the smoothing process (elevation update) is repeated. Increasing this value from the default of 3 will result in significantly greater smoothing." ) );
  addParameter( iterParam.release() );

  auto maxDiffParam = std::make_unique<QgsProcessingParameterNumber>( u"MAX_ELEVATION_CHANGE"_s, QObject::tr( "Maximum elevation change" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0.0 );
  maxDiffParam->setHelp( QObject::tr( "The allowed maximum height change of any cell in one iteration. If the calculated change exceeds this value, the elevation remains unchanged. This prevents excessive deviation from the original surface." ) );
  addParameter( maxDiffParam.release() );

  auto zFactorParam = std::make_unique<QgsProcessingParameterNumber>( u"Z_FACTOR"_s, QObject::tr( "Z factor" ), Qgis::ProcessingNumberParameterType::Double, 1.0, false, 0.00000001 );
  zFactorParam->setHelp( QObject::tr( "Multiplication factor to convert vertical Z units to horizontal XY units." ) );
  zFactorParam->setFlags( zFactorParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  zFactorParam->setMetadata(
    { QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"decimals"_s, 12 } } ) } } )
    }
  );
  addParameter( zFactorParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATION_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  auto outputLayerParam = std::make_unique<QgsProcessingParameterRasterDestination>( u"OUTPUT"_s, QObject::tr( "Output layer" ) );
  addParameter( outputLayerParam.release() );
}

QgsRasterFeaturePreservingSmoothingAlgorithm *QgsRasterFeaturePreservingSmoothingAlgorithm::createInstance() const
{
  return new QgsRasterFeaturePreservingSmoothingAlgorithm();
}

bool QgsRasterFeaturePreservingSmoothingAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  mBand = parameterAsInt( parameters, u"BAND"_s, context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( layer->bandCount() ) );

  mInterface.reset( layer->dataProvider()->clone() );
  mLayerWidth = layer->width();
  mLayerHeight = layer->height();
  mExtent = layer->extent();
  mCrs = layer->crs();
  mRasterUnitsPerPixelX = layer->rasterUnitsPerPixelX();
  mRasterUnitsPerPixelY = layer->rasterUnitsPerPixelY();
  mDataType = layer->dataProvider()->dataType( mBand );
  mNoData = layer->dataProvider()->sourceNoDataValue( mBand );
  return true;
}

QVariantMap QgsRasterFeaturePreservingSmoothingAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const int radius = parameterAsInt( parameters, u"RADIUS"_s, context );

  const double thresholdDegrees = parameterAsDouble( parameters, u"THRESHOLD"_s, context );
  const double cosThreshold = std::cos( thresholdDegrees * M_PI / 180.0 );

  const int iterations = parameterAsInt( parameters, u"ITERATIONS"_s, context );
  const bool hasMaxElevChange = parameters.value( u"MAX_ELEVATION_CHANGE"_s ).isValid();
  const double maxElevChange = hasMaxElevChange ? parameterAsDouble( parameters, u"MAX_ELEVATION_CHANGE"_s, context ) : 0;
  const double zFactor = parameterAsDouble( parameters, u"Z_FACTOR"_s, context );

  const double oneOver8ResX = 1 / ( 8.0 * mRasterUnitsPerPixelX );
  const double oneOver8ResY = 1 / ( 8.0 * mRasterUnitsPerPixelY );

  const QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  auto outputWriter = std::make_unique<QgsRasterFileWriter>( outputFile );
  outputWriter->setOutputProviderKey( u"gdal"_s );
  if ( !creationOptions.isEmpty() )
  {
    outputWriter->setCreationOptions( creationOptions.split( '|' ) );
  }
  outputWriter->setOutputFormat( outputFormat );

  std::unique_ptr<QgsRasterDataProvider> destProvider( outputWriter->createOneBandRaster( mDataType, mLayerWidth, mLayerHeight, mExtent, mCrs ) );
  if ( !destProvider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !destProvider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, destProvider->error().message( QgsErrorMessage::Text ) ) );

  destProvider->setNoDataValue( 1, mNoData );
  destProvider->setEditable( true );

  // block iteration padding must be large enough to cover all 3 steps, i.e. 1px (for normal calculation) + radius in pixels (for smoothing) + 1px for each iteration
  const int blockPadding = 1 + radius + iterations;
  QgsRasterIterator iter( mInterface.get(), blockPadding );
  iter.startRasterRead( mBand, mLayerWidth, mLayerHeight, mExtent );

  int iterCols = 0;
  int iterRows = 0;
  int iterLeft = 0;
  int iterTop = 0;
  int tileCols = 0;
  int tileRows = 0;
  int tileLeft = 0;
  int tileTop = 0;
  QgsRectangle blockExtent;

  std::unique_ptr<QgsRasterBlock> inputBlock;

  // buffers are re-used across different tiles to minimize allocations.
  // Resize them in advance to the largest possible required sizes.
  const size_t maxBufferSize = static_cast<std::size_t>( iter.maximumTileWidth() + 2 * blockPadding ) * static_cast< std::size_t >( iter.maximumTileHeight() + 2 * blockPadding );
  // pixel normals, using SCALED z values (accounting for z-factor)
  std::vector<Vector2D> normalBuffer( maxBufferSize );
  std::vector<Vector2D> smoothedNormalBuffer( maxBufferSize );
  std::vector<double> zBuffer( maxBufferSize );
  std::vector<qint8> noDataBuffer( maxBufferSize );

  bool isNoData = false;

  const bool hasReportsDuringClose = destProvider->hasReportsDuringClose();
  const double maxProgressDuringBlockWriting = hasReportsDuringClose ? 50.0 : 100.0;

  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, inputBlock, iterLeft, iterTop, &blockExtent, &tileCols, &tileRows, &tileLeft, &tileTop ) )
  {
    if ( feedback->isCanceled() )
      break;
    feedback->setProgress( maxProgressDuringBlockWriting * iter.progress( mBand, 0 ) );
    feedback->setProgressText( QObject::tr( "Calculating surface normals" ) );

    // copy raster values and NoData to buffers once in advance -- we will be retrieving
    // each individual value many times during surface normal calculation, so the cost
    // of this upfront step pays off...
    double *zBufferData = zBuffer.data();
    qint8 *noDataBufferData = noDataBuffer.data();
    for ( int r = 0; r < iterRows; ++r )
    {
      for ( int c = 0; c < iterCols; ++c )
      {
        *zBufferData++ = inputBlock->valueAndNoData( r, c, isNoData ) * zFactor;
        *noDataBufferData++ = isNoData ? 1 : 0;
      }
    }

    // step 1: calculate surface normals
    // we follow Lindsay's rust implementation of FPDEMS, and consider out-of-range pixels and no-data pixels
    // as the center pixel value when calculating the normal over the 3x3 matrix window.

    // if neighbor is out of range or is NoData, follow Lindsay and use center z
    auto getZ = [iterRows, iterCols, &noDataBuffer, &zBuffer]( int row, int col, double z ) -> double {
      if ( row < 0 || row >= iterRows || col < 0 || col >= iterCols )
        return z;
      std::size_t idx = static_cast<std::size_t>( row ) * iterCols + col;
      if ( noDataBuffer[idx] )
        return z;
      return zBuffer[idx];
    };

    for ( int r = 0; r < iterRows; ++r )
    {
      if ( feedback->isCanceled() )
        break;

      feedback->setProgress( maxProgressDuringBlockWriting * iter.progress( mBand, r / static_cast< double >( iterRows ) / 3.0 ) );

      for ( int c = 0; c < iterCols; ++c )
      {
        const std::size_t idx = static_cast<std::size_t>( r ) * iterCols + c;
        if ( noDataBuffer[idx] )
        {
          normalBuffer[idx] = Vector2D( 0, 0 );
          continue;
        }

        const double z = zBuffer[idx];
        const double z1 = getZ( r - 1, c - 1, z );
        const double z2 = getZ( r - 1, c, z );
        const double z3 = getZ( r - 1, c + 1, z );
        const double z4 = getZ( r, c - 1, z );
        const double z6 = getZ( r, c + 1, z );
        const double z7 = getZ( r + 1, c - 1, z );
        const double z8 = getZ( r + 1, c, z );
        const double z9 = getZ( r + 1, c + 1, z );

        // Horn 1981, adjusting for raster units per pixel
        const double dx = ( ( z3 - z1 + 2 * ( z6 - z4 ) + z9 - z7 ) * oneOver8ResX );
        const double dy = ( ( z7 - z1 + 2 * ( z8 - z2 ) + z9 - z3 ) * oneOver8ResY );
        normalBuffer[idx] = Vector2D( -dx, dy );
      }
    }
    if ( feedback->isCanceled() )
      break;

    // step 2: smooth normals
    feedback->setProgressText( QObject::tr( "Smoothing surface normals" ) );
    for ( int r = 0; r < iterRows; ++r )
    {
      if ( feedback->isCanceled() )
        break;

      feedback->setProgress( maxProgressDuringBlockWriting * iter.progress( mBand, 1.0 / 3.0 + r / static_cast< double >( iterRows ) / 3.0 ) );

      for ( int c = 0; c < iterCols; ++c )
      {
        const std::size_t idx = static_cast<std::size_t>( r ) * iterCols + c;
        if ( noDataBuffer[idx] )
          continue;

        const Vector2D &centerNormal = normalBuffer[idx];
        double summedWeights = 0.0;
        double summedX = 0;
        double summedY = 0;

        for ( int kernelY = -radius; kernelY <= radius; ++kernelY )
        {
          for ( int kernelX = -radius; kernelX <= radius; ++kernelX )
          {
            const int pixelRow = r + kernelY;
            const int pixelCol = c + kernelX;

            // skip pixels outside range, nodata pixels
            if ( pixelRow < 0 || pixelRow >= iterRows || pixelCol < 0 || pixelCol >= iterCols )
              continue;
            const std::size_t pixelIdx = static_cast<std::size_t>( pixelRow ) * iterCols + pixelCol;
            if ( noDataBuffer[pixelIdx] )
              continue;

            const Vector2D &neighNormal = normalBuffer[pixelIdx];

            const double cosAngle = centerNormal.angleBetweenCos( neighNormal );
            if ( cosAngle > cosThreshold )
            {
              const double w = ( cosAngle - cosThreshold ) * ( cosAngle - cosThreshold );
              summedWeights += w;
              summedX += w * neighNormal.x;
              summedY += w * neighNormal.y;
            }
          }
        }
        if ( !qgsDoubleNear( summedWeights, 0.0 ) )
        {
          summedX /= summedWeights;
          summedY /= summedWeights;
        }
        smoothedNormalBuffer[idx] = Vector2D( summedX, summedY );
      }
    }
    if ( feedback->isCanceled() )
      break;

    // step 3: update elevation
    // pixel offsets
    const int dx[8] = { 1, 1, 1, 0, -1, -1, -1, 0 };
    const int dy[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };

    // world offsets (accounting for units per pixel)
    const double worldX[8] = { -mRasterUnitsPerPixelX, -mRasterUnitsPerPixelX, -mRasterUnitsPerPixelX, 0.0, mRasterUnitsPerPixelX, mRasterUnitsPerPixelX, mRasterUnitsPerPixelX, 0.0 };
    const double worldY[8] = { -mRasterUnitsPerPixelY, 0.0, mRasterUnitsPerPixelY, mRasterUnitsPerPixelY, mRasterUnitsPerPixelY, 0.0, -mRasterUnitsPerPixelY, -mRasterUnitsPerPixelY };

    for ( int iteration = 0; iteration < iterations; ++iteration )
    {
      feedback->setProgressText( QObject::tr( "Updating elevation (iteration %1/%2)" ).arg( iteration + 1 ).arg( iterations ) );
      if ( feedback->isCanceled() )
        break;

      for ( int r = 0; r < iterRows; ++r )
      {
        feedback->setProgress( maxProgressDuringBlockWriting * iter.progress( mBand, 2.0 / 3.0 + ( ( static_cast< double >( iteration ) / iterations ) + ( r / static_cast< double >( iterRows ) ) / iterations ) / 3.0 ) );
        for ( int c = 0; c < iterCols; ++c )
        {
          const std::size_t idx = static_cast<std::size_t>( r ) * iterCols + c;
          if ( noDataBuffer[idx] )
            continue;

          const double originalZ = inputBlock->value( r, c );

          const Vector2D &centerNormal = smoothedNormalBuffer[idx];
          double sumWeights = 0.0;
          double sumZ = 0.0;
          for ( int n = 0; n < 8; ++n )
          {
            const int pixelX = c + dx[n];
            const int pixelY = r + dy[n];
            if ( pixelX < 0 || pixelX >= iterCols || pixelY < 0 || pixelY >= iterRows )
              continue;

            const std::size_t nIdx = static_cast<std::size_t>( pixelY ) * iterCols + pixelX;
            if ( noDataBuffer[nIdx] )
              continue;

            const double pixelZ = zBuffer[nIdx];

            const Vector2D &neighNormal = smoothedNormalBuffer[nIdx];
            const double cosAngle = centerNormal.angleBetweenCos( neighNormal );
            if ( cosAngle > cosThreshold )
            {
              const double w = ( cosAngle - cosThreshold ) * ( cosAngle - cosThreshold );
              sumWeights += w;
              sumZ += -( neighNormal.x * worldX[n] + neighNormal.y * worldY[n] - pixelZ ) * w;
            }
          }

          if ( sumWeights > 0.0 )
          {
            const double newZScaled = ( sumZ / sumWeights );
            const double newZUnscaled = newZScaled / zFactor;
            if ( !hasMaxElevChange || std::abs( newZUnscaled - originalZ ) <= maxElevChange )
            {
              zBuffer[idx] = newZScaled;
            }
            else
            {
              zBuffer[idx] = originalZ * zFactor;
            }
          }
          else
          {
            zBuffer[idx] = originalZ * zFactor;
          }
        }
      }
    }

    const int blockOffsetX = tileLeft - iterLeft;
    const int blockOffsetY = tileTop - iterTop;

    auto outputBlock = std::make_unique<QgsRasterBlock>( mDataType, tileCols, tileRows );

    for ( int r = 0; r < tileRows; ++r )
    {
      for ( int c = 0; c < tileCols; ++c )
      {
        const int pixelRow = r + blockOffsetY;
        const int pixelCol = c + blockOffsetX;

        const std::size_t idx = static_cast<std::size_t>( pixelRow ) * iterCols + pixelCol;
        if ( noDataBuffer[idx] )
          outputBlock->setValue( r, c, mNoData );
        else
          outputBlock->setValue( r, c, zBuffer[idx] / zFactor );
      }
    }

    if ( !destProvider->writeBlock( outputBlock.get(), 1, tileLeft, tileTop ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destProvider->error().summary() ) );
    }
  }
  destProvider->setEditable( false );

  if ( feedback && hasReportsDuringClose )
  {
    std::unique_ptr<QgsFeedback> scaledFeedback( QgsFeedback::createScaledFeedback( feedback, maxProgressDuringBlockWriting, 100.0 ) );
    if ( !destProvider->closeWithProgress( scaledFeedback.get() ) )
    {
      if ( feedback->isCanceled() )
        return {};
      throw QgsProcessingException( QObject::tr( "Could not write raster dataset" ) );
    }
  }

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}


///@endcond
