/***************************************************************************
                         qgsalgorithmrasterfrequencybycomparisonoperator.cpp
                         ---------------------
    begin                : June 2020
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

#include "qgsalgorithmrasterfrequencybycomparisonoperator.h"

#include "qgsrasteranalysisutils.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasterprojector.h"

///@cond PRIVATE

//
//QgsRasterFrequencyByComparisonOperatorBase
//

QString QgsRasterFrequencyByComparisonOperatorBase::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterFrequencyByComparisonOperatorBase::groupId() const
{
  return u"rasteranalysis"_s;
}

void QgsRasterFrequencyByComparisonOperatorBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT_VALUE_RASTER"_s, QObject::tr( "Input value raster" ) ) );
  addParameter( new QgsProcessingParameterBand( u"INPUT_VALUE_RASTER_BAND"_s, QObject::tr( "Value raster band" ), 1, u"INPUT_VALUE_RASTER"_s ) );
  addParameter( new QgsProcessingParameterMultipleLayers( u"INPUT_RASTERS"_s, QObject::tr( "Input raster layers" ), Qgis::ProcessingSourceType::Raster ) );
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
  addOutput( new QgsProcessingOutputNumber( u"OCCURRENCE_COUNT"_s, QObject::tr( "Count of value occurrences" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"FOUND_LOCATIONS_COUNT"_s, QObject::tr( "Count of cells with equal value occurrences" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MEAN_FREQUENCY_PER_LOCATION"_s, QObject::tr( "Mean frequency at valid cell locations" ) ) );
  addOutput( new QgsProcessingOutputString( u"EXTENT"_s, QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputString( u"CRS_AUTHID"_s, QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"WIDTH_IN_PIXELS"_s, QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"HEIGHT_IN_PIXELS"_s, QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"TOTAL_PIXEL_COUNT"_s, QObject::tr( "Total pixel count" ) ) );
}

bool QgsRasterFrequencyByComparisonOperatorBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *inputValueRaster = parameterAsRasterLayer( parameters, u"INPUT_VALUE_RASTER"_s, context );
  if ( !inputValueRaster )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT_VALUE_RASTER"_s ) );

  mInputValueRasterBand = parameterAsInt( parameters, u"INPUT_VALUE_RASTER_BAND"_s, context );
  mIgnoreNoData = parameterAsBool( parameters, u"IGNORE_NODATA"_s, context );

  mInputValueRasterInterface.reset( inputValueRaster->dataProvider()->clone() );
  mNoDataValue = parameterAsDouble( parameters, u"OUTPUT_NODATA_VALUE"_s, context );
  mCrs = inputValueRaster->crs();
  mRasterUnitsPerPixelX = inputValueRaster->rasterUnitsPerPixelX();
  mRasterUnitsPerPixelY = inputValueRaster->rasterUnitsPerPixelY();
  mLayerWidth = inputValueRaster->width();
  mLayerHeight = inputValueRaster->height();
  mExtent = inputValueRaster->extent();

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

QVariantMap QgsRasterFrequencyByComparisonOperatorBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  const int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  const int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;
  const int nbBlocksWidth = static_cast<int>( std::ceil( 1.0 * mLayerWidth / maxWidth ) );
  const int nbBlocksHeight = static_cast<int>( std::ceil( 1.0 * mLayerHeight / maxHeight ) );
  const int nbBlocks = nbBlocksWidth * nbBlocksHeight;
  provider->setEditable( true );

  QgsRasterIterator iter( mInputValueRasterInterface.get() );
  iter.startRasterRead( mInputValueRasterBand, mLayerWidth, mLayerHeight, mExtent );
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  QgsRectangle blockExtent;

  const bool hasReportsDuringClose = provider->hasReportsDuringClose();
  const double maxProgressDuringBlockWriting = hasReportsDuringClose ? 50.0 : 100.0;

  unsigned long long occurrenceCount = 0;
  unsigned long long noDataLocationsCount = 0;
  std::unique_ptr<QgsRasterBlock> inputBlock;
  while ( iter.readNextRasterPart( 1, iterCols, iterRows, inputBlock, iterLeft, iterTop, &blockExtent ) )
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

    auto outputBlock = std::make_unique<QgsRasterBlock>( Qgis::DataType::Int32, iterCols, iterRows );
    feedback->setProgress( maxProgressDuringBlockWriting * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback->isCanceled() )
        break;

      for ( int col = 0; col < iterCols; col++ )
      {
        bool valueRasterCellIsNoData = false;
        const double value = inputBlock->valueAndNoData( row, col, valueRasterCellIsNoData );

        if ( valueRasterCellIsNoData && !mIgnoreNoData )
        {
          //output cell will always be NoData if NoData occurs in valueRaster or cellValueStack and NoData is not ignored
          //this saves unnecessary iterations on the cellValueStack
          outputBlock->setValue( row, col, mNoDataValue );
          noDataLocationsCount++;
        }
        else
        {
          bool noDataInStack = false;
          const std::vector<double> cellValues = QgsRasterAnalysisUtils::getCellValuesFromBlockStack( inputBlocks, row, col, noDataInStack );

          if ( noDataInStack && !mIgnoreNoData )
          {
            outputBlock->setValue( row, col, mNoDataValue );
            noDataLocationsCount++;
          }
          else
          {
            const int frequency = applyComparisonOperator( value, cellValues );
            outputBlock->setValue( row, col, frequency );
            occurrenceCount += frequency;
          }
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

  const unsigned long long foundLocationsCount = layerSize - noDataLocationsCount;
  const double meanEqualCountPerValidLocation = static_cast<double>( occurrenceCount ) / static_cast<double>( foundLocationsCount * mInputs.size() );

  QVariantMap outputs;
  outputs.insert( u"OCCURRENCE_COUNT"_s, occurrenceCount );
  outputs.insert( u"FOUND_LOCATIONS_COUNT"_s, foundLocationsCount );
  outputs.insert( u"MEAN_FREQUENCY_PER_LOCATION"_s, meanEqualCountPerValidLocation );
  outputs.insert( u"EXTENT"_s, mExtent.toString() );
  outputs.insert( u"CRS_AUTHID"_s, mCrs.authid() );
  outputs.insert( u"WIDTH_IN_PIXELS"_s, mLayerWidth );
  outputs.insert( u"HEIGHT_IN_PIXELS"_s, mLayerHeight );
  outputs.insert( u"TOTAL_PIXEL_COUNT"_s, layerSize );
  outputs.insert( u"OUTPUT"_s, outputFile );

  return outputs;
}

//
// QgsRasterFrequencyByEqualOperatorAlgorithm
//

QString QgsRasterFrequencyByEqualOperatorAlgorithm::displayName() const
{
  return QObject::tr( "Equal to frequency" );
}

QString QgsRasterFrequencyByEqualOperatorAlgorithm::name() const
{
  return u"equaltofrequency"_s;
}

QStringList QgsRasterFrequencyByEqualOperatorAlgorithm::tags() const
{
  return QObject::tr( "cell,equal,frequency,pixel,stack" ).split( ',' );
}

QString QgsRasterFrequencyByEqualOperatorAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm evaluates on a cell-by-cell basis the frequency "
                      "(number of times) the values of an input stack of rasters are equal "
                      "to the value of a value raster. \n "
                      "If multiband rasters are used in the data raster stack, the algorithm will always "
                      "perform the analysis on the first band of the rasters - use GDAL to use other bands in the analysis. "
                      "The input value layer serves as reference layer for the sample layers. "
                      "Any NoData cells in the value raster or the data layer stack will result in a NoData cell "
                      "in the output raster if the ignore NoData parameter is not checked. "
                      "The output NoData value can be set manually. The output rasters extent and resolution "
                      "is defined by the input raster layer and is always of int32 type." );
}

QString QgsRasterFrequencyByEqualOperatorAlgorithm::shortDescription() const
{
  return QObject::tr( "Evaluates on a cell-by-cell basis the frequency (number of times) "
                      "the values of an input stack of rasters are equal to the value of a value raster." );
}

QgsRasterFrequencyByEqualOperatorAlgorithm *QgsRasterFrequencyByEqualOperatorAlgorithm::createInstance() const
{
  return new QgsRasterFrequencyByEqualOperatorAlgorithm();
}

int QgsRasterFrequencyByEqualOperatorAlgorithm::applyComparisonOperator( double searchValue, std::vector<double> cellValueStack )
{
  return static_cast<int>( std::count( cellValueStack.begin(), cellValueStack.end(), searchValue ) );
}

//
// QgsRasterFrequencyByGreaterThanOperatorAlgorithm
//

QString QgsRasterFrequencyByGreaterThanOperatorAlgorithm::displayName() const
{
  return QObject::tr( "Greater than frequency" );
}

QString QgsRasterFrequencyByGreaterThanOperatorAlgorithm::name() const
{
  return u"greaterthanfrequency"_s;
}

QStringList QgsRasterFrequencyByGreaterThanOperatorAlgorithm::tags() const
{
  return QObject::tr( "cell,greater,frequency,pixel,stack" ).split( ',' );
}

QString QgsRasterFrequencyByGreaterThanOperatorAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm evaluates on a cell-by-cell basis the frequency "
                      "(number of times) the values of an input stack of rasters are greater than "
                      "the value of a value raster. \n "
                      "If multiband rasters are used in the data raster stack, the algorithm will always "
                      "perform the analysis on the first band of the rasters - use GDAL to use other bands in the analysis. "
                      "The input value layer serves as reference layer for the sample layers. "
                      "Any NoData cells in the value raster or the data layer stack will result in a NoData cell "
                      "in the output raster if the ignore NoData parameter is not checked. "
                      "The output NoData value can be set manually. The output rasters extent and resolution "
                      "is defined by the input raster layer and is always of int32 type." );
}

QString QgsRasterFrequencyByGreaterThanOperatorAlgorithm::shortDescription() const
{
  return QObject::tr( "Evaluates on a cell-by-cell basis the frequency (number of times) "
                      "the values of an input stack of rasters are greater than the value of a value raster." );
}

QgsRasterFrequencyByGreaterThanOperatorAlgorithm *QgsRasterFrequencyByGreaterThanOperatorAlgorithm::createInstance() const
{
  return new QgsRasterFrequencyByGreaterThanOperatorAlgorithm();
}

int QgsRasterFrequencyByGreaterThanOperatorAlgorithm::applyComparisonOperator( double searchValue, std::vector<double> cellValueStack )
{
  return static_cast<int>( std::count_if( cellValueStack.begin(), cellValueStack.end(), [&]( double const &stackValue ) { return stackValue > searchValue; } ) );
}

//
// QgsRasterFrequencyByLessThanOperatorAlgorithm
//

QString QgsRasterFrequencyByLessThanOperatorAlgorithm::displayName() const
{
  return QObject::tr( "Less than frequency" );
}

QString QgsRasterFrequencyByLessThanOperatorAlgorithm::name() const
{
  return u"lessthanfrequency"_s;
}

QStringList QgsRasterFrequencyByLessThanOperatorAlgorithm::tags() const
{
  return QObject::tr( "cell,less,lower,frequency,pixel,stack" ).split( ',' );
}

QString QgsRasterFrequencyByLessThanOperatorAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm evaluates on a cell-by-cell basis the frequency "
                      "(number of times) the values of an input stack of rasters are less than "
                      "the value of a value raster. \n "
                      "If multiband rasters are used in the data raster stack, the algorithm will always "
                      "perform the analysis on the first band of the rasters - use GDAL to use other bands in the analysis. "
                      "The input value layer serves as reference layer for the sample layers. "
                      "Any NoData cells in the value raster or the data layer stack will result in a NoData cell "
                      "in the output raster if the ignore NoData parameter is not checked. "
                      "The output NoData value can be set manually. The output rasters extent and resolution "
                      "is defined by the input raster layer and is always of int32 type." );
}

QString QgsRasterFrequencyByLessThanOperatorAlgorithm::shortDescription() const
{
  return QObject::tr( "Evaluates on a cell-by-cell basis the frequency (number of times) "
                      "the values of an input stack of rasters are less than the value of a value raster." );
}

QgsRasterFrequencyByLessThanOperatorAlgorithm *QgsRasterFrequencyByLessThanOperatorAlgorithm::createInstance() const
{
  return new QgsRasterFrequencyByLessThanOperatorAlgorithm();
}

int QgsRasterFrequencyByLessThanOperatorAlgorithm::applyComparisonOperator( double searchValue, std::vector<double> cellValueStack )
{
  return static_cast<int>( std::count_if( cellValueStack.begin(), cellValueStack.end(), [&]( double const &stackValue ) { return stackValue < searchValue; } ) );
}

///@endcond
