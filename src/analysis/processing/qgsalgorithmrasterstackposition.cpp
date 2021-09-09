/***************************************************************************
                         qgsrasterstackposition.cpp
                         ---------------------
    begin                : July 2020
    copyright            : (C) 2020 by Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmrasterstackposition.h"
#include "qgsrasterprojector.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasteranalysisutils.h"

///@cond PRIVATE

//
//QgsRasterFrequencyByComparisonOperatorBase
//

QString QgsRasterStackPositionAlgorithmBase::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterStackPositionAlgorithmBase::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

void QgsRasterStackPositionAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "INPUT_RASTERS" ),
                QObject::tr( "Input raster layers" ), QgsProcessing::TypeRaster ) );

  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "REFERENCE_LAYER" ), QObject::tr( "Reference layer" ) ) );

  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "IGNORE_NODATA" ), QObject::tr( "Ignore NoData values" ), false ) );

  std::unique_ptr< QgsProcessingParameterNumber > output_nodata_parameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "OUTPUT_NODATA_VALUE" ), QObject::tr( "Output NoData value" ), QgsProcessingParameterNumber::Double, -9999, true );
  output_nodata_parameter->setFlags( output_nodata_parameter->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( output_nodata_parameter.release() );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ),
                QObject::tr( "Output layer" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "CRS_AUTHID" ), QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "WIDTH_IN_PIXELS" ), QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "HEIGHT_IN_PIXELS" ), QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TOTAL_PIXEL_COUNT" ), QObject::tr( "Total pixel count" ) ) );
}

bool QgsRasterStackPositionAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *referenceLayer = parameterAsRasterLayer( parameters, QStringLiteral( "REFERENCE_LAYER" ), context );
  if ( !referenceLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "REFERENCE_LAYER" ) ) );

  mIgnoreNoData = parameterAsBool( parameters, QStringLiteral( "IGNORE_NODATA" ), context );
  mNoDataValue = parameterAsDouble( parameters, QStringLiteral( "OUTPUT_NODATA_VALUE" ), context );

  mCrs = referenceLayer->crs();
  mRasterUnitsPerPixelX = referenceLayer->rasterUnitsPerPixelX();
  mRasterUnitsPerPixelY = referenceLayer->rasterUnitsPerPixelY();
  mLayerWidth = referenceLayer->width();
  mLayerHeight = referenceLayer->height();
  mExtent = referenceLayer->extent();

  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "INPUT_RASTERS" ), context );
  QList< QgsRasterLayer * > rasterLayers;
  rasterLayers.reserve( layers.count() );
  for ( QgsMapLayer *l : layers )
  {
    if ( feedback->isCanceled() )
      break; //in case some slow data sources are loaded

    if ( l->type() == QgsMapLayerType::RasterLayer )
    {
      QgsRasterLayer *layer = qobject_cast< QgsRasterLayer * >( l );
      QgsRasterAnalysisUtils::RasterLogicInput input;
      const int band = 1; //could be made dynamic
      input.hasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( band );
      input.sourceDataProvider.reset( layer->dataProvider()->clone() );
      input.interface = input.sourceDataProvider.get();
      // add projector if necessary
      if ( layer->crs() != mCrs )
      {
        input.projector = std::make_unique< QgsRasterProjector >();
        input.projector->setInput( input.sourceDataProvider.get() );
        input.projector->setCrs( layer->crs(), mCrs, context.transformContext() );
        input.interface = input.projector.get();
      }
      mInputs.emplace_back( std::move( input ) );
    }
  }

  return true;
}

QVariantMap QgsRasterStackPositionAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  std::unique_ptr< QgsRasterFileWriter > writer = std::make_unique< QgsRasterFileWriter >( outputFile );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider > provider( writer->createOneBandRaster( Qgis::DataType::Int32, mLayerWidth, mLayerHeight, mExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  provider->setNoDataValue( 1, mNoDataValue );
  const qgssize layerSize = static_cast< qgssize >( mLayerWidth ) * static_cast< qgssize >( mLayerHeight );

  const int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  const int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;
  const int nbBlocksWidth = static_cast< int>( std::ceil( 1.0 * mLayerWidth / maxWidth ) );
  const int nbBlocksHeight = static_cast< int >( std::ceil( 1.0 * mLayerHeight / maxHeight ) );
  const int nbBlocks = nbBlocksWidth * nbBlocksHeight;
  provider->setEditable( true );

  QgsRasterIterator iter( provider.get() );
  iter.startRasterRead( 1, mLayerWidth, mLayerHeight, mExtent );
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  QgsRectangle blockExtent;

  std::unique_ptr< QgsRasterBlock > outputBlock;
  while ( iter.readNextRasterPart( 1, iterCols, iterRows, outputBlock, iterLeft, iterTop, &blockExtent ) )
  {
    std::vector< std::unique_ptr< QgsRasterBlock > > inputBlocks;
    for ( const QgsRasterAnalysisUtils::RasterLogicInput &i : mInputs )
    {
      if ( feedback->isCanceled() )
        break; //in case some slow data sources are loaded
      for ( const int band : i.bands )
      {
        if ( feedback->isCanceled() )
          break; //in case some slow data sources are loaded
        std::unique_ptr< QgsRasterBlock > b( i.interface->block( band, blockExtent, iterCols, iterRows ) );
        inputBlocks.emplace_back( std::move( b ) );
      }
    }

    feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback->isCanceled() )
        break;

      for ( int col = 0; col < iterCols; col++ )
      {
        bool noDataInStack = false;

        if ( !inputBlocks.empty() )
        {
          const int position = findPosition( inputBlocks, row, col, noDataInStack );

          if ( position == -1 || ( noDataInStack && !mIgnoreNoData ) )
          {
            //output cell will always be NoData if NoData occurs the current raster cell
            //of the input blocks and NoData is not ignored
            //this saves unnecessary iterations on the cellValueStack
            outputBlock->setValue( row, col, mNoDataValue );
          }
          else
          {
            outputBlock->setValue( row, col, position );
          }
        }
        else
        {
          outputBlock->setValue( row, col, mNoDataValue );
        }
      }
    }
    provider->writeBlock( outputBlock.get(), 1, iterLeft, iterTop );
  }
  provider->setEditable( false );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "EXTENT" ), mExtent.toString() );
  outputs.insert( QStringLiteral( "CRS_AUTHID" ), mCrs.authid() );
  outputs.insert( QStringLiteral( "WIDTH_IN_PIXELS" ), mLayerWidth );
  outputs.insert( QStringLiteral( "HEIGHT_IN_PIXELS" ), mLayerHeight );
  outputs.insert( QStringLiteral( "TOTAL_PIXEL_COUNT" ), layerSize );
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );

  return outputs;
}

//
// QgsRasterStackLowestPositionAlgorithm
//
QString QgsRasterStackLowestPositionAlgorithm::displayName() const
{
  return QObject::tr( "Lowest position in raster stack" );
}

QString QgsRasterStackLowestPositionAlgorithm::name() const
{
  return QStringLiteral( "lowestpositioninrasterstack" );
}

QStringList QgsRasterStackLowestPositionAlgorithm::tags() const
{
  return QObject::tr( "cell,lowest,position,pixel,stack" ).split( ',' );
}

QString QgsRasterStackLowestPositionAlgorithm::shortHelpString() const
{
  return QObject::tr( "The lowest position algorithm evaluates on a cell-by-cell basis the position "
                      "of the raster with the lowest value in a stack of rasters. Position counts start "
                      "with 1 and range to the total number of input rasters. The order of the input "
                      "rasters is relevant for the algorithm. If multiple rasters feature the lowest value, "
                      "the first raster will be used for the position value.\n "
                      "If multiband rasters are used in the data raster stack, the algorithm will always "
                      "perform the analysis on the first band of the rasters - use GDAL to use other bands in the analysis. "
                      "Any NoData cells in the raster layer stack will result in a NoData cell "
                      "in the output raster unless the \"ignore NoData\" parameter is checked. "
                      "The output NoData value can be set manually. The output rasters extent and resolution "
                      "is defined by a reference raster layer and is always of int32 type." );
}

QgsRasterStackLowestPositionAlgorithm *QgsRasterStackLowestPositionAlgorithm::createInstance() const
{
  return new QgsRasterStackLowestPositionAlgorithm();
}

int QgsRasterStackLowestPositionAlgorithm::findPosition( std::vector< std::unique_ptr<QgsRasterBlock> > &inputBlocks, int &row, int &col, bool &noDataInRasterBlockStack )
{
  int lowestPosition = 0;

  //auxiliary variables
  const int inputBlocksCount = inputBlocks.size();
  int currentPosition = 0;
  int noDataCount = 0;
  double firstValue = mNoDataValue;
  bool firstValueIsNoData = true;

  while ( firstValueIsNoData && ( currentPosition < inputBlocksCount ) )
  {
    //check if all blocks are nodata/invalid
    std::unique_ptr<QgsRasterBlock> &firstBlock = inputBlocks.at( currentPosition );
    firstValue = firstBlock->valueAndNoData( row, col, firstValueIsNoData );

    if ( !firstBlock->isValid() || firstValueIsNoData )
    {
      noDataInRasterBlockStack = true;
      noDataCount++;
    }
    else
    {
      lowestPosition = currentPosition;
    }
    currentPosition++;
  }

  if ( noDataCount == inputBlocksCount )
  {
    noDataInRasterBlockStack = true;
    return -1; //all blocks are NoData
  }
  else
  {
    //scan for the lowest value
    while ( currentPosition < inputBlocksCount )
    {
      std::unique_ptr< QgsRasterBlock > &currentBlock = inputBlocks.at( currentPosition );

      bool currentValueIsNoData = false;
      const double currentValue = currentBlock->valueAndNoData( row, col, currentValueIsNoData );

      if ( !currentBlock->isValid() || currentValueIsNoData )
      {
        noDataInRasterBlockStack = true;
        noDataCount++;
      }
      else
      {
        if ( currentValue < firstValue )
        {
          firstValue = currentValue;
          lowestPosition = currentPosition;
        }
      }
      currentPosition++;
    }
  }
  //the ArcGIS implementation uses 1 for first position value instead of 0 as in standard c++
  return ++lowestPosition; //therefore ++
}

//
// QgsRasterStackHighestPositionAlgorithmAlgorithm
//

QString QgsRasterStackHighestPositionAlgorithm::displayName() const
{
  return QObject::tr( "Highest position in raster stack" );
}

QString QgsRasterStackHighestPositionAlgorithm::name() const
{
  return QStringLiteral( "highestpositioninrasterstack" );
}

QStringList QgsRasterStackHighestPositionAlgorithm::tags() const
{
  return QObject::tr( "cell,highest,position,pixel,stack" ).split( ',' );
}

QString QgsRasterStackHighestPositionAlgorithm::shortHelpString() const
{
  return QObject::tr( "The highest position algorithm evaluates on a cell-by-cell basis the position "
                      "of the raster with the highest value in a stack of rasters. Position counts start "
                      "with 1 and range to the total number of input rasters. The order of the input "
                      "rasters is relevant for the algorithm. If multiple rasters feature the highest value, "
                      "the first raster will be used for the position value.\n "
                      "If multiband rasters are used in the data raster stack, the algorithm will always "
                      "perform the analysis on the first band of the rasters - use GDAL to use other bands in the analysis. "
                      "Any NoData cells in the raster layer stack will result in a NoData cell "
                      "in the output raster unless the \"ignore NoData\" parameter is checked. "
                      "The output NoData value can be set manually. The output rasters extent and resolution "
                      "is defined by a reference raster layer and is always of int32 type." );
}

QgsRasterStackHighestPositionAlgorithm *QgsRasterStackHighestPositionAlgorithm::createInstance() const
{
  return new QgsRasterStackHighestPositionAlgorithm();
}

int QgsRasterStackHighestPositionAlgorithm::findPosition( std::vector< std::unique_ptr< QgsRasterBlock> > &inputBlocks, int &row, int &col, bool &noDataInRasterBlockStack )
{
  int highestPosition = 0;

  //auxiliary variables
  const int inputBlocksCount = inputBlocks.size();
  int currentPosition = 0;
  int noDataCount = 0;
  double firstValue = mNoDataValue;
  bool firstValueIsNoData = true;

  while ( firstValueIsNoData && ( currentPosition < inputBlocksCount ) )
  {
    //check if all blocks are nodata/invalid
    std::unique_ptr<QgsRasterBlock> &firstBlock = inputBlocks.at( currentPosition );
    firstValue = firstBlock->valueAndNoData( row, col, firstValueIsNoData );

    if ( !firstBlock->isValid() || firstValueIsNoData )
    {
      noDataInRasterBlockStack = true;
      noDataCount++;
    }
    else
    {
      highestPosition = currentPosition;
    }

    currentPosition++;
  }

  if ( noDataCount == inputBlocksCount )
  {
    noDataInRasterBlockStack = true;
    return -1; //all blocks are NoData
  }
  else
  {
    //scan for the lowest value
    while ( currentPosition < inputBlocksCount )
    {
      std::unique_ptr< QgsRasterBlock > &currentBlock = inputBlocks.at( currentPosition );

      bool currentValueIsNoData = false;
      const double currentValue = currentBlock->valueAndNoData( row, col, currentValueIsNoData );

      if ( !currentBlock->isValid() || currentValueIsNoData )
      {
        noDataInRasterBlockStack = true;
        noDataCount++;
      }
      else
      {
        if ( currentValue > firstValue )
        {
          firstValue = currentValue;
          highestPosition = currentPosition;
        }
      }
      currentPosition++;
    }
  }
  //the ArcGIS implementation uses 1 for first position value instead of 0 as in standard c++
  return ++highestPosition; //therefore ++
}

///@endcond

