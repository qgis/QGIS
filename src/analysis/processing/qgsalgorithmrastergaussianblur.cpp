/***************************************************************************
                         qgsalgorithmrastergaussianblur.cpp
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

#include "qgsalgorithmrastergaussianblur.h"

#include "qgsrasterfilewriter.h"

///@cond PRIVATE

QString QgsRasterGaussianBlurAlgorithm::name() const
{
  return QStringLiteral( "rastergaussianblur" );
}

QString QgsRasterGaussianBlurAlgorithm::displayName() const
{
  return QObject::tr( "Gaussian blur" );
}

QStringList QgsRasterGaussianBlurAlgorithm::tags() const
{
  return QObject::tr( "smooth,filter,denoise" ).split( ',' );
}

QString QgsRasterGaussianBlurAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterGaussianBlurAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

QString QgsRasterGaussianBlurAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm applies a Gaussian blur filter to an input raster layer.\n\n"
                      "The radius parameter controls the strength of the blur. "
                      "A larger radius results in a smoother output, at the cost of execution time." );
}

QString QgsRasterGaussianBlurAlgorithm::shortDescription() const
{
  return QObject::tr( "Applies a Gaussian blur filter to a raster layer." );
}

void QgsRasterGaussianBlurAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterBand( QStringLiteral( "BAND" ), QObject::tr( "Band number" ), 1, QStringLiteral( "INPUT" ) ) );

  auto sigmaParam = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "RADIUS" ), QObject::tr( "Blur radius (pixels)" ), Qgis::ProcessingNumberParameterType::Integer, 2.0, false, 1, 512 );
  addParameter( sigmaParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( QStringLiteral( "CREATION_OPTIONS" ), QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setMetadata( QVariantMap( { { QStringLiteral( "widget_wrapper" ), QVariantMap( { { QStringLiteral( "widget_type" ), QStringLiteral( "rasteroptions" ) } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  auto outputLayerParam = std::make_unique<QgsProcessingParameterRasterDestination>( QStringLiteral( "OUTPUT" ), QObject::tr( "Output layer" ) );
  addParameter( outputLayerParam.release() );
}

QgsRasterGaussianBlurAlgorithm *QgsRasterGaussianBlurAlgorithm::createInstance() const
{
  return new QgsRasterGaussianBlurAlgorithm();
}

bool QgsRasterGaussianBlurAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT" ), context );
  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT" ) ) );

  const int band = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );

  mBand = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( layer->bandCount() ) );

  mInterface.reset( layer->dataProvider()->clone() );
  mHasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( band );
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

QVariantMap QgsRasterGaussianBlurAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const int radius = parameterAsInt( parameters, QStringLiteral( "RADIUS" ), context );

  const QString creationOptions = parameterAsString( parameters, QStringLiteral( "CREATION_OPTIONS" ), context ).trimmed();

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, QStringLiteral( "OUTPUT" ), context );

  auto outputWriter = std::make_unique<QgsRasterFileWriter>( outputFile );
  outputWriter->setOutputProviderKey( QStringLiteral( "gdal" ) );
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

  // create 1D Gaussian kernel
  const int kernelSize = 2 * radius + 1;
  std::vector<double> kernel( kernelSize );

  double sum = 0.0;
  const double sigma = radius / 3.0;
  const double expCoefficient = -1.0 / ( 2.0 * sigma * sigma );

  for ( int i = -radius; i <= radius; ++i )
  {
    double result = std::exp( i * i * expCoefficient );
    kernel[i + radius] = result;
    sum += result;
  }
  // normalize kernel
  for ( double &w : kernel )
  {
    w /= sum;
  }

  QgsRasterIterator iter( mInterface.get(), radius );
  iter.startRasterRead( mBand, mLayerWidth, mLayerHeight, mExtent );
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  int tileLeft = 0;
  int tileTop = 0;
  int tileCols = 0;
  int tileRows = 0;

  QgsRectangle blockExtent;

  std::vector<double> horizBuffer;
  std::vector<qint8> horizNoData;
  // reserve max potential size to avoid reallocations
  const std::size_t maxBufferSize = static_cast< std::size_t >( QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH + 2 * radius ) * static_cast< std::size_t >( QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT + 2 * radius );
  horizBuffer.reserve( maxBufferSize );
  horizNoData.reserve( maxBufferSize );

  std::unique_ptr<QgsRasterBlock> inputBlock;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, inputBlock, iterLeft, iterTop, &blockExtent, &tileCols, &tileRows, &tileLeft, &tileTop ) )
  {
    feedback->setProgress( 100 * iter.progress( mBand, 0 ) );

    if ( feedback->isCanceled() )
      break;

    auto outputBlock = std::make_unique<QgsRasterBlock>( mDataType, tileCols, tileRows );

    const int tileBoundaryLeft = tileLeft - iterLeft;
    const int tileBoundaryTop = tileTop - iterTop;

    // first pass -- horizontally blurred buffer. Take CAREFUL note of the different sizes used here -- for the
    // first pass we calculate the blur over the whole vertical height of the tile (i.e. every row), but EXCLUDE the horizontal padding
    const int horizWidth = tileCols;

    const std::size_t bufferSize = static_cast< std::size_t>( horizWidth ) * iterRows;
    horizBuffer.resize( bufferSize );
    horizNoData.assign( bufferSize, 0 ); // reset to 0 (false)

    bool isNoData = false;
    double *horizPtr = horizBuffer.data();
    qint8 *noDataPtr = horizNoData.data();

    for ( int r = 0; r < iterRows; ++r )
    {
      feedback->setProgress( 100 * iter.progress( mBand, r / static_cast< double >( iterRows ) * 0.5 ) );

      if ( feedback->isCanceled() )
        break;

      for ( int c = 0; c < tileCols; ++c )
      {
        const int inputColumn = c + tileBoundaryLeft;
        inputBlock->valueAndNoData( r, inputColumn, isNoData );

        if ( isNoData )
        {
          *noDataPtr++ = 1; // mark as no-data
          horizPtr++;
          continue;
        }

        double sumValues = 0.0;
        double sumWeight = 0.0;

        for ( int k = -radius; k <= radius; ++k )
        {
          const int neighborColumn = inputColumn + k;
          if ( neighborColumn < 0 || neighborColumn >= iterCols )
            continue;

          const double val = inputBlock->valueAndNoData( r, neighborColumn, isNoData );
          if ( !isNoData )
          {
            double w = kernel[k + radius];
            sumValues += val * w;
            sumWeight += w;
          }
        }

        if ( sumWeight > 0.0 )
        {
          // must re-normalize summed values -- we may have hit edges or nodata pixels
          *horizPtr++ = sumValues / sumWeight;
          // leave no-data flag as zero (not no-data)
          noDataPtr++;
        }
        else
        {
          // should theoretically not happen if center is valid, unless all weights are 0
          *noDataPtr++ = 1; // mark as no-data
          horizPtr++;
        }
      }
    }

    // second pass -- vertical blur
    // unlike the first pass, here we ONLY need to calculate the blur for pixels which aren't block padding
    for ( int r = 0; r < tileRows; ++r )
    {
      feedback->setProgress( 100 * iter.progress( mBand, 0.5 + r / static_cast< double >( tileRows ) * 0.5 ) );

      if ( feedback->isCanceled() )
        break;

      const int inputRow = r + tileBoundaryTop;
      for ( int c = 0; c < tileCols; ++c )
      {
        const std::size_t targetPixelBufferIndex = inputRow * static_cast< std::size_t>( horizWidth ) + c;
        if ( horizNoData[targetPixelBufferIndex] )
        {
          outputBlock->setValue( r, c, mNoData );
          continue;
        }

        double sumValues = 0.0;
        double sumWeight = 0.0;

        for ( int k = -radius; k <= radius; ++k )
        {
          const int neighborRow = inputRow + k;
          if ( neighborRow < 0 || neighborRow >= iterRows )
            continue;

          const std::size_t neighborIndex = static_cast< std::size_t>( neighborRow ) * horizWidth + c;
          if ( !horizNoData[neighborIndex] )
          {
            double w = kernel[k + radius];
            sumValues += horizBuffer[neighborIndex] * w;
            sumWeight += w;
          }
        }

        if ( sumWeight > 0.0 )
        {
          outputBlock->setValue( r, c, sumValues / sumWeight );
        }
        else
        {
          outputBlock->setValue( r, c, mNoData );
        }
      }
    }

    if ( !destProvider->writeBlock( outputBlock.get(), 1, tileLeft, tileTop ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destProvider->error().summary() ) );
    }
  }
  destProvider->setEditable( false );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}


///@endcond
