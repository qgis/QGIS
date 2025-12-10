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

#include "qgsrasteranalysisutils.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasterprojector.h"

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
  return u"rasteranalysis"_s;
}

void QgsRasterStackPositionAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( u"INPUT_RASTERS"_s, QObject::tr( "Input raster layers" ), Qgis::ProcessingSourceType::Raster ) );

  addParameter( new QgsProcessingParameterRasterLayer( u"REFERENCE_LAYER"_s, QObject::tr( "Reference layer" ) ) );

  addParameter( new QgsProcessingParameterBoolean( u"IGNORE_NODATA"_s, QObject::tr( "Ignore NoData values" ), false ) );

  auto output_nodata_parameter = std::make_unique<QgsProcessingParameterNumber>( u"OUTPUT_NODATA_VALUE"_s, QObject::tr( "Output NoData value" ), Qgis::ProcessingNumberParameterType::Double, -9999, true );
  output_nodata_parameter->setFlags( output_nodata_parameter->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( output_nodata_parameter.release() );

  // backwards compatibility parameter
  // TODO QGIS 5: remove parameter and related logic
  auto createOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATE_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  createOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  createOptsParam->setFlags( createOptsParam->flags() | Qgis::ProcessingParameterFlag::Hidden );
  addParameter( createOptsParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATION_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Output layer" ) ) );
  addOutput( new QgsProcessingOutputString( u"EXTENT"_s, QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputString( u"CRS_AUTHID"_s, QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"WIDTH_IN_PIXELS"_s, QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"HEIGHT_IN_PIXELS"_s, QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"TOTAL_PIXEL_COUNT"_s, QObject::tr( "Total pixel count" ) ) );
}

bool QgsRasterStackPositionAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *referenceLayer = parameterAsRasterLayer( parameters, u"REFERENCE_LAYER"_s, context );
  if ( !referenceLayer )
    throw QgsProcessingException( invalidRasterError( parameters, u"REFERENCE_LAYER"_s ) );

  mIgnoreNoData = parameterAsBool( parameters, u"IGNORE_NODATA"_s, context );
  mNoDataValue = parameterAsDouble( parameters, u"OUTPUT_NODATA_VALUE"_s, context );

  mCrs = referenceLayer->crs();
  mRasterUnitsPerPixelX = referenceLayer->rasterUnitsPerPixelX();
  mRasterUnitsPerPixelY = referenceLayer->rasterUnitsPerPixelY();
  mLayerWidth = referenceLayer->width();
  mLayerHeight = referenceLayer->height();
  mExtent = referenceLayer->extent();

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"INPUT_RASTERS"_s, context );
  QList<QgsRasterLayer *> rasterLayers;
  rasterLayers.reserve( layers.count() );
  for ( QgsMapLayer *l : layers )
  {
    if ( feedback->isCanceled() )
      break; //in case some slow data sources are loaded

    if ( l->type() == Qgis::LayerType::Raster )
    {
      QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( l );
      QgsRasterAnalysisUtils::RasterLogicInput input;
      const int band = 1; //could be made dynamic
      input.hasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( band );
      input.sourceDataProvider.reset( layer->dataProvider()->clone() );
      input.interface = input.sourceDataProvider.get();
      // add projector if necessary
      if ( layer->crs() != mCrs )
      {
        input.projector = std::make_unique<QgsRasterProjector>();
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
  QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  // handle backwards compatibility parameter CREATE_OPTIONS
  const QString optionsString = parameterAsString( parameters, u"CREATE_OPTIONS"_s, context );
  if ( !optionsString.isEmpty() )
    creationOptions = optionsString;

  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  auto writer = std::make_unique<QgsRasterFileWriter>( outputFile );
  writer->setOutputProviderKey( u"gdal"_s );
  if ( !creationOptions.isEmpty() )
  {
    writer->setCreationOptions( creationOptions.split( '|' ) );
  }
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider> provider( writer->createOneBandRaster( Qgis::DataType::Int32, mLayerWidth, mLayerHeight, mExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  provider->setNoDataValue( 1, mNoDataValue );
  const qgssize layerSize = static_cast<qgssize>( mLayerWidth ) * static_cast<qgssize>( mLayerHeight );

  provider->setEditable( true );

  QgsRasterIterator iter( provider.get() );
  iter.startRasterRead( 1, mLayerWidth, mLayerHeight, mExtent );
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  QgsRectangle blockExtent;

  const bool hasReportsDuringClose = provider->hasReportsDuringClose();
  const double maxProgressDuringBlockWriting = hasReportsDuringClose ? 50.0 : 100.0;

  std::unique_ptr<QgsRasterBlock> outputBlock;
  while ( iter.readNextRasterPart( 1, iterCols, iterRows, outputBlock, iterLeft, iterTop, &blockExtent ) )
  {
    std::vector<std::unique_ptr<QgsRasterBlock>> inputBlocks;
    for ( const QgsRasterAnalysisUtils::RasterLogicInput &i : mInputs )
    {
      if ( feedback->isCanceled() )
        break; //in case some slow data sources are loaded
      for ( const int band : i.bands )
      {
        if ( feedback->isCanceled() )
          break; //in case some slow data sources are loaded
        std::unique_ptr<QgsRasterBlock> b( i.interface->block( band, blockExtent, iterCols, iterRows ) );
        inputBlocks.emplace_back( std::move( b ) );
      }
    }

    feedback->setProgress( maxProgressDuringBlockWriting * iter.progress( 1 ) );
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
    if ( !provider->writeBlock( outputBlock.get(), 1, iterLeft, iterTop ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( provider->error().summary() ) );
    }
  }
  provider->setEditable( false );

  if ( feedback && hasReportsDuringClose )
  {
    std::unique_ptr<QgsFeedback> scaledFeedback( QgsFeedback::createScaledFeedback( feedback, maxProgressDuringBlockWriting, 100.0 ) );
    if ( !provider->closeWithProgress( scaledFeedback.get() ) )
    {
      if ( feedback->isCanceled() )
        return {};
      throw QgsProcessingException( QObject::tr( "Could not write raster dataset" ) );
    }
  }

  QVariantMap outputs;
  outputs.insert( u"EXTENT"_s, mExtent.toString() );
  outputs.insert( u"CRS_AUTHID"_s, mCrs.authid() );
  outputs.insert( u"WIDTH_IN_PIXELS"_s, mLayerWidth );
  outputs.insert( u"HEIGHT_IN_PIXELS"_s, mLayerHeight );
  outputs.insert( u"TOTAL_PIXEL_COUNT"_s, layerSize );
  outputs.insert( u"OUTPUT"_s, outputFile );

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
  return u"lowestpositioninrasterstack"_s;
}

QStringList QgsRasterStackLowestPositionAlgorithm::tags() const
{
  return QObject::tr( "cell,lowest,position,pixel,stack" ).split( ',' );
}

QString QgsRasterStackLowestPositionAlgorithm::shortDescription() const
{
  return QObject::tr( "Evaluates on a cell-by-cell basis the position "
                      "of the raster with the lowest value in a stack of rasters." );
}

QString QgsRasterStackLowestPositionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm evaluates on a cell-by-cell basis the position "
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

int QgsRasterStackLowestPositionAlgorithm::findPosition( std::vector<std::unique_ptr<QgsRasterBlock>> &inputBlocks, int &row, int &col, bool &noDataInRasterBlockStack )
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
      std::unique_ptr<QgsRasterBlock> &currentBlock = inputBlocks.at( currentPosition );

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
  return u"highestpositioninrasterstack"_s;
}

QStringList QgsRasterStackHighestPositionAlgorithm::tags() const
{
  return QObject::tr( "cell,highest,position,pixel,stack" ).split( ',' );
}

QString QgsRasterStackHighestPositionAlgorithm::shortDescription() const
{
  return QObject::tr( "Evaluates on a cell-by-cell basis the position "
                      "of the raster with the highest value in a stack of rasters." );
}

QString QgsRasterStackHighestPositionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm evaluates on a cell-by-cell basis the position "
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

int QgsRasterStackHighestPositionAlgorithm::findPosition( std::vector<std::unique_ptr<QgsRasterBlock>> &inputBlocks, int &row, int &col, bool &noDataInRasterBlockStack )
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
      std::unique_ptr<QgsRasterBlock> &currentBlock = inputBlocks.at( currentPosition );

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
