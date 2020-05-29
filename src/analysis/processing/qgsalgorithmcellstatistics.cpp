/***************************************************************************
                         qgsalgorithmcellstatistics.cpp
                         ---------------------
    begin                : May 2020
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

#include "qgsalgorithmcellstatistics.h"
#include "qgsrasterprojector.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasteranalysisutils.h"

///@cond PRIVATE

QString QgsCellStatisticsAlgorithm::displayName() const
{
  return QObject::tr( "Cell statistics" );
}

QString QgsCellStatisticsAlgorithm::name() const
{
  return QObject::tr( "cellstatistics" );
}

QStringList QgsCellStatisticsAlgorithm::tags() const
{
  return QObject::tr( "cell,pixe,statistic,mean,sum,majority,minority,variance,variety,range,median,minimum,maximum" ).split( ',' );
}

QString QgsCellStatisticsAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsCellStatisticsAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

QString QgsCellStatisticsAlgorithm::shortHelpString() const
{
  return QObject::tr( "The Cell statistics algorithm computes a value for each cell of the "
                      "output raster. At each cell location, "
                      "the output value is defined as a function of all overlaid cell values of the "
                      "input rasters.\n "
                      "The output rasters extent and resolution is defined by a reference "
                      "raster. The following functions can be applied on the input "
                      "raster cells per output raster cell location: "
                      "<ul> "
                      "   <li>Sum</li>"
                      "   <li>Mean</li>"
                      "   <li>Median</li>"
                      "   <li>Standard deviation</li>"
                      "   <li>Variance</li>"
                      "   <li>Minimum</li>"
                      "   <li>Maximum</li>"
                      "   <li>Minority (most frequent value)</li>"
                      "   <li>Majority (least frequent value)</li>"
                      "   <li>Range (max-min)</li>"
                      "   <li>Variety (count of unique values)</li>"
                      "</ul> "
                      "Input raster layers that do not match the cell size of the reference raster layer will be "
                      "resampled using nearest neighbor resampling.\n"
                      "<i>Calculation details - general:</i> NoData values in any of the input layers will result in a NoData cell output.\n"
                      "<i>Calculation details - Median:</i> If the number of input layers is even, the median will be calculated as the "
                      "arithmetic mean of the two middle values of the ordered cell input values.\n"
                      "<i>Calculation details - Minority/Majority:</i> If no unique minority or majority could be found, the result is NoData, except all "
                      "input cell values are equal." );
}

QgsCellStatisticsAlgorithm *QgsCellStatisticsAlgorithm::createInstance() const
{
  return new QgsCellStatisticsAlgorithm();
}

void QgsCellStatisticsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "INPUTS" ),
                QObject::tr( "Input layers" ), QgsProcessing::TypeRaster ) );

  QStringList statistics = QStringList();
  statistics << QStringLiteral( "Sum" )
             << QStringLiteral( "Mean" )
             << QStringLiteral( "Median" )
             << QStringLiteral( "Standard deviation" )
             << QStringLiteral( "Variance" )
             << QStringLiteral( "Minimum" )
             << QStringLiteral( "Maximum" )
             << QStringLiteral( "Minority" )
             << QStringLiteral( "Majority" )
             << QStringLiteral( "Range" )
             << QStringLiteral( "Variety" );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "STATISTIC" ), QObject::tr( "Statistic" ),  statistics, false, 0, false ) );

  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "REF_LAYER" ), QObject::tr( "Reference layer" ) ) );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ),
                QObject::tr( "Output layer" ) ) );

  addOutput( new QgsProcessingOutputString( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "CRS_AUTHID" ), QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "WIDTH_IN_PIXELS" ), QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "HEIGHT_IN_PIXELS" ), QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TOTAL_PIXEL_COUNT" ), QObject::tr( "Total pixel count" ) ) );
}

bool QgsCellStatisticsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *referenceLayer = parameterAsRasterLayer( parameters, QStringLiteral( "REF_LAYER" ), context );
  if ( !referenceLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "REF_LAYER" ) ) );
  mCrs = referenceLayer->crs();
  mRasterUnitsPerPixelX = referenceLayer->rasterUnitsPerPixelX();
  mRasterUnitsPerPixelY = referenceLayer->rasterUnitsPerPixelY();
  mLayerWidth = referenceLayer->width();
  mLayerHeight = referenceLayer->height();
  mExtent = referenceLayer->extent();

  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "INPUTS" ), context );
  QList< QgsRasterLayer * > rasterLayers;
  rasterLayers.reserve( layers.count() );
  for ( QgsMapLayer *l : layers )
  {
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
        input.projector = qgis::make_unique< QgsRasterProjector >();
        input.projector->setInput( input.sourceDataProvider.get() );
        input.projector->setCrs( layer->crs(), mCrs, context.transformContext() );
        input.interface = input.projector.get();
      }
      mInputs.emplace_back( std::move( input ) );
    }
  }

  return true;
}

QVariantMap QgsCellStatisticsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  //obtain statistic method
  int statisticMethodIdx = parameterAsInt( parameters, QStringLiteral( "STATISTIC" ), context );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  std::unique_ptr< QgsRasterFileWriter > writer = qgis::make_unique< QgsRasterFileWriter >( outputFile );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider > provider( writer->createOneBandRaster( mDataType, mLayerWidth, mLayerHeight, mExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  provider->setNoDataValue( 1, mNoDataValue );
  qgssize layerSize = static_cast< qgssize >( mLayerWidth ) * static_cast< qgssize >( mLayerHeight );

  int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;
  int nbBlocksWidth = static_cast< int>( std::ceil( 1.0 * mLayerWidth / maxWidth ) );
  int nbBlocksHeight = static_cast< int >( std::ceil( 1.0 * mLayerHeight / maxHeight ) );
  int nbBlocks = nbBlocksWidth * nbBlocksHeight;
  provider->setEditable( true );
  QgsRasterIterator outputIter( provider.get() );
  outputIter.startRasterRead( 1, mLayerWidth, mLayerHeight, mExtent );

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  QgsRectangle blockExtent;
  std::unique_ptr< QgsRasterBlock > outputBlock;
  while ( outputIter.readNextRasterPart( 1, iterCols, iterRows, outputBlock, iterLeft, iterTop, &blockExtent ) )
  {
    std::vector< std::unique_ptr< QgsRasterBlock > > inputBlocks;
    for ( const QgsRasterAnalysisUtils::RasterLogicInput &i : mInputs )
    {
      for ( int band : i.bands )
      {
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
        double result = 0;
        bool hasNoData = false;
        std::vector<double> cellValues = getCellValues( inputBlocks, row, col, hasNoData );
        int cellValueStackSize = cellValues.size();

        if ( hasNoData )
        {
          //output cell will always be NoData if NoData occurs in cellValueStack
          //this saves unnecessary iterations on the cellValueStack
          outputBlock->setValue( row, col, mNoDataValue );
        }
        else
        {
          switch ( statisticMethodIdx )
          {
            case 0: //sum
              result = std::accumulate( cellValues.begin(), cellValues.end(), 0.0 );
              break;
            case 1: //mean
              result = QgsRasterAnalysisUtils::meanFromCellValues( cellValues, cellValueStackSize );
              break;
            case 2: //median
              result = QgsRasterAnalysisUtils::medianFromCellValues( cellValues, cellValueStackSize );
              break;
            case 3: //stddev
              result = QgsRasterAnalysisUtils::stddevFromCellValues( cellValues, cellValueStackSize );
              break;
            case 4: //variance
              result = QgsRasterAnalysisUtils::varianceFromCellValues( cellValues, cellValueStackSize );
              break;
            case 5: //min
              result = QgsRasterAnalysisUtils::minimumFromCellValues( cellValues );
              break;
            case 6: //max
              result = QgsRasterAnalysisUtils::maximumFromCellValues( cellValues );
              break;
            case 7: //minority
              result = QgsRasterAnalysisUtils::minorityFromCellValues( cellValues, mNoDataValue, cellValueStackSize );
              break;
            case 8: //majority
              result = QgsRasterAnalysisUtils::majorityFromCellValues( cellValues, mNoDataValue, cellValueStackSize );
              break;
            case 9: //range
              result = QgsRasterAnalysisUtils::rangeFromCellValues( cellValues );
              break;
            case 10: //variety
              result = QgsRasterAnalysisUtils::varietyFromCellValues( cellValues );
              break;
            default:
              break;
          }
          outputBlock->setValue( row, col, result );
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

std::vector<double> QgsCellStatisticsAlgorithm::getCellValues( const std::vector< std::unique_ptr< QgsRasterBlock > > &inputBlocks, int &row, int &col, bool &hasNoData )
{
  //get all values from inputBlocks
  std::vector<double> cellValues;
  for ( auto &block : inputBlocks )
  {
    double value = 0;
    if ( !block || !block->isValid() )
    {
      hasNoData = true;
      break;
    }
    else
    {
      value = block->valueAndNoData( row, col, hasNoData );
      if ( hasNoData )
        break;
      cellValues.push_back( value );
    }
  }
  return cellValues;
}

///@endcond


