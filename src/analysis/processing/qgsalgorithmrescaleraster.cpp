/***************************************************************************
                         qgsalgorithmrescaleraster.cpp
                         ---------------------
    begin                : July 2020
    copyright            : (C) 2020 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>
#include "math.h"
#include "qgsalgorithmrescaleraster.h"
#include "qgsrasterfilewriter.h"

///@cond PRIVATE

QString QgsRescaleRasterAlgorithm::name() const
{
  return QStringLiteral( "rescaleraster" );
}

QString QgsRescaleRasterAlgorithm::displayName() const
{
  return QObject::tr( "Rescale raster" );
}

QStringList QgsRescaleRasterAlgorithm::tags() const
{
  return QObject::tr( "raster,rescale,minimum,maximum,range" ).split( ',' );
}

QString QgsRescaleRasterAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRescaleRasterAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

QString QgsRescaleRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "Rescales raster layer to a new value range, while preserving the shape "
                      "(distribution) of the raster's histogram (pixel values). Input values "
                      "are mapped using a linear interpolation from the source raster's minimum "
                      "and maximum pixel values to the destination minimum and maximum pixel range.\n\n"
                      "By default the algorithm preserves original the NODATA value, but there is "
                      "an option to override it." );
}

QgsRescaleRasterAlgorithm *QgsRescaleRasterAlgorithm::createInstance() const
{
  return new QgsRescaleRasterAlgorithm();
}

void QgsRescaleRasterAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ), QStringLiteral( "Input raster" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "BAND" ), QObject::tr( "Band number" ), 1, QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MINIMUM" ), QObject::tr( "New minimum value" ), QgsProcessingParameterNumber::Double, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MAXIMUM" ), QObject::tr( "New maximum value" ), QgsProcessingParameterNumber::Double, 255 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "NODATA" ), QObject::tr( "New NODATA value" ), QgsProcessingParameterNumber::Double, QVariant(), true ) );
  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Rescaled" ) ) );
}

bool QgsRescaleRasterAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT" ), context );
  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT" ) ) );

  mBand = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" )
                                  .arg( mBand )
                                  .arg( layer->bandCount() ) );

  mMinimum = parameterAsDouble( parameters, QStringLiteral( "MINIMUM" ), context );
  mMaximum = parameterAsDouble( parameters, QStringLiteral( "MAXIMUM" ), context );

  mInterface.reset( layer->dataProvider()->clone() );

  mCrs = layer->crs();
  mLayerWidth = layer->width();
  mLayerHeight = layer->height();
  mExtent = layer->extent();
  if ( parameters.value( QStringLiteral( "NODATA" ) ).isValid() )
  {
    mNoData = parameterAsDouble( parameters, QStringLiteral( "NODATA" ), context );
  }
  else
  {
    mNoData = layer->dataProvider()->sourceNoDataValue( mBand );
  }
  mXSize = mInterface->xSize();
  mYSize = mInterface->ySize();

  return true;
}

QVariantMap QgsRescaleRasterAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  feedback->pushInfo( QObject::tr( "Calculating raster minimum and maximum values…" ) );
  QgsRasterBandStats stats = mInterface->bandStatistics( mBand, QgsRasterBandStats::Min | QgsRasterBandStats::Max, QgsRectangle(), 0 );

  feedback->pushInfo( QObject::tr( "Rescaling values…" ) );
  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );
  std::unique_ptr< QgsRasterFileWriter > writer = qgis::make_unique< QgsRasterFileWriter >( outputFile );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider > provider( writer->createOneBandRaster( Qgis::Float32, mXSize, mYSize, mExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  QgsRasterDataProvider *destProvider = provider.get();
  destProvider->setEditable( true );
  destProvider->setNoDataValue( 1, mNoData );

  int blockWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  int blockHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;
  int numBlocksX = static_cast< int >( std::ceil( 1.0 * mLayerWidth / blockWidth ) );
  int numBlocksY = static_cast< int >( std::ceil( 1.0 * mLayerHeight / blockHeight ) );
  int numBlocks = numBlocksX * numBlocksY;

  QgsRasterIterator iter( mInterface.get() );
  iter.startRasterRead( mBand, mLayerWidth, mLayerHeight, mExtent );
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  std::unique_ptr< QgsRasterBlock > block;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, block, iterLeft, iterTop ) )
  {
    feedback->setProgress( 100 * ( ( iterTop / blockHeight * numBlocksX ) + iterLeft / blockWidth ) / numBlocks );

    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback->isCanceled() )
        break;

      for ( int col = 0; col < iterCols; col++ )
      {
        bool isNoData = false;
        double val = block->valueAndNoData( row, col, isNoData );
        if ( isNoData )
        {
          block->setValue( row, col, mNoData );
        }
        else
        {
          double newValue = ( ( val - stats.minimumValue ) * ( mMaximum - mMinimum ) / ( stats.maximumValue - stats.minimumValue ) ) + mMinimum;
          block->setValue( row, col, newValue );
        }
      }
    }
    destProvider->writeBlock( block.get(), mBand, iterLeft, iterTop );
  }
  destProvider->setEditable( false );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

///@endcond
