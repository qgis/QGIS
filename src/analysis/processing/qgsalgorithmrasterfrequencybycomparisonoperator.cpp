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
#include "qgsrasterprojector.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasteranalysisutils.h"

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
  return QStringLiteral( "rasteranalysis" );
}

void QgsRasterFrequencyByComparisonOperatorBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT_VALUE_RASTER" ), QObject::tr( "Input value raster" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "INPUT_VALUE_RASTER_BAND" ), QObject::tr( "Value raster band" ), 1, QStringLiteral( "INPUT_VALUE_RASTER" ) ) );


  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "INPUT_RASTERS" ),
                QObject::tr( "Input raster layers" ), QgsProcessing::TypeRaster ) );

  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "IGNORE_NODATA" ), QObject::tr( "Ignore NoData values" ), false ) );

  std::unique_ptr< QgsProcessingParameterNumber > output_nodata_parameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "OUTPUT_NODATA_VALUE" ), QObject::tr( "Output NoData value" ), QgsProcessingParameterNumber::Double, -9999, true );
  output_nodata_parameter->setFlags( output_nodata_parameter->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( output_nodata_parameter.release() );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ),
                QObject::tr( "Output layer" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "OCCURRENCE_COUNT" ), QObject::tr( "Count of value occurrences" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "FOUND_LOCATIONS_COUNT" ), QObject::tr( "Count of cells with equal value occurrences" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "MEAN_FREQUENCY_PER_LOCATION" ), QObject::tr( "Mean frequency at valid cell locations" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "CRS_AUTHID" ), QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "WIDTH_IN_PIXELS" ), QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "HEIGHT_IN_PIXELS" ), QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TOTAL_PIXEL_COUNT" ), QObject::tr( "Total pixel count" ) ) );
}

bool QgsRasterFrequencyByComparisonOperatorBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *inputValueRaster = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT_VALUE_RASTER" ), context );
  if ( !inputValueRaster )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT_VALUE_RASTER" ) ) );

  mInputValueRasterBand = parameterAsInt( parameters, QStringLiteral( "INPUT_VALUE_RASTER_BAND" ), context );
  mIgnoreNoData = parameterAsBool( parameters, QStringLiteral( "IGNORE_NODATA" ), context );

  mInputValueRasterInterface.reset( inputValueRaster->dataProvider()->clone() );
  mNoDataValue = parameterAsDouble( parameters, QStringLiteral( "OUTPUT_NODATA_VALUE" ), context );
  mCrs = inputValueRaster->crs();
  mRasterUnitsPerPixelX = inputValueRaster->rasterUnitsPerPixelX();
  mRasterUnitsPerPixelY = inputValueRaster->rasterUnitsPerPixelY();
  mLayerWidth = inputValueRaster->width();
  mLayerHeight = inputValueRaster->height();
  mExtent = inputValueRaster->extent();

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

QVariantMap QgsRasterFrequencyByComparisonOperatorBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  QgsRasterIterator iter( mInputValueRasterInterface.get() );
  iter.startRasterRead( mInputValueRasterBand, mLayerWidth, mLayerHeight, mExtent );
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  QgsRectangle blockExtent;

  unsigned long long occurrenceCount = 0;
  unsigned long long noDataLocationsCount = 0;
  std::unique_ptr< QgsRasterBlock > inputBlock;
  while ( iter.readNextRasterPart( 1, iterCols, iterRows, inputBlock, iterLeft, iterTop, &blockExtent ) )
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

    std::unique_ptr< QgsRasterBlock > outputBlock = std::make_unique<QgsRasterBlock>( Qgis::DataType::Int32, iterCols, iterRows );
    feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
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
    provider->writeBlock( outputBlock.get(), 1, iterLeft, iterTop );
  }
  provider->setEditable( false );

  const unsigned long long foundLocationsCount = layerSize - noDataLocationsCount;
  const double meanEqualCountPerValidLocation = static_cast<double>( occurrenceCount ) / static_cast<double>( foundLocationsCount * mInputs.size() );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OCCURRENCE_COUNT" ), occurrenceCount );
  outputs.insert( QStringLiteral( "FOUND_LOCATIONS_COUNT" ), foundLocationsCount );
  outputs.insert( QStringLiteral( "MEAN_FREQUENCY_PER_LOCATION" ),  meanEqualCountPerValidLocation );
  outputs.insert( QStringLiteral( "EXTENT" ), mExtent.toString() );
  outputs.insert( QStringLiteral( "CRS_AUTHID" ), mCrs.authid() );
  outputs.insert( QStringLiteral( "WIDTH_IN_PIXELS" ), mLayerWidth );
  outputs.insert( QStringLiteral( "HEIGHT_IN_PIXELS" ), mLayerHeight );
  outputs.insert( QStringLiteral( "TOTAL_PIXEL_COUNT" ), layerSize );
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );

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
  return QStringLiteral( "equaltofrequency" );
}

QStringList QgsRasterFrequencyByEqualOperatorAlgorithm::tags() const
{
  return QObject::tr( "cell,equal,frequency,pixel,stack" ).split( ',' );
}

QString QgsRasterFrequencyByEqualOperatorAlgorithm::shortHelpString() const
{
  return QObject::tr( "The Equal to frequency algorithm evaluates on a cell-by-cell basis the frequency "
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
  return QStringLiteral( "greaterthanfrequency" );
}

QStringList QgsRasterFrequencyByGreaterThanOperatorAlgorithm::tags() const
{
  return QObject::tr( "cell,greater,frequency,pixel,stack" ).split( ',' );
}

QString QgsRasterFrequencyByGreaterThanOperatorAlgorithm::shortHelpString() const
{
  return QObject::tr( "The Greater than frequency algorithm evaluates on a cell-by-cell basis the frequency "
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

QgsRasterFrequencyByGreaterThanOperatorAlgorithm *QgsRasterFrequencyByGreaterThanOperatorAlgorithm::createInstance() const
{
  return new QgsRasterFrequencyByGreaterThanOperatorAlgorithm();
}

int QgsRasterFrequencyByGreaterThanOperatorAlgorithm::applyComparisonOperator( double searchValue, std::vector<double> cellValueStack )
{
  return static_cast<int>( std::count_if( cellValueStack.begin(), cellValueStack.end(), [&]( double const & stackValue ) { return stackValue > searchValue; } ) );
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
  return QStringLiteral( "lessthanfrequency" );
}

QStringList QgsRasterFrequencyByLessThanOperatorAlgorithm::tags() const
{
  return QObject::tr( "cell,less,lower,frequency,pixel,stack" ).split( ',' );
}

QString QgsRasterFrequencyByLessThanOperatorAlgorithm::shortHelpString() const
{
  return QObject::tr( "The Less than frequency algorithm evaluates on a cell-by-cell basis the frequency "
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

QgsRasterFrequencyByLessThanOperatorAlgorithm *QgsRasterFrequencyByLessThanOperatorAlgorithm::createInstance() const
{
  return new QgsRasterFrequencyByLessThanOperatorAlgorithm();
}

int QgsRasterFrequencyByLessThanOperatorAlgorithm::applyComparisonOperator( double searchValue, std::vector<double> cellValueStack )
{
  return static_cast<int>( std::count_if( cellValueStack.begin(), cellValueStack.end(), [&]( double const & stackValue ) { return stackValue < searchValue; } ) );
}

///@endcond


