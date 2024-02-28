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


QString QgsCellStatisticsAlgorithmBase::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsCellStatisticsAlgorithmBase::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

void QgsCellStatisticsAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::Raster ) );

  addSpecificAlgorithmParams();

  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "IGNORE_NODATA" ), QObject::tr( "Ignore NoData values" ), true ) );

  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "REFERENCE_LAYER" ), QObject::tr( "Reference layer" ) ) );

  std::unique_ptr< QgsProcessingParameterNumber > output_nodata_parameter = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "OUTPUT_NODATA_VALUE" ), QObject::tr( "Output NoData value" ), Qgis::ProcessingNumberParameterType::Double, -9999, false );
  output_nodata_parameter->setFlags( output_nodata_parameter->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( output_nodata_parameter.release() );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ),
                QObject::tr( "Output layer" ) ) );

  addOutput( new QgsProcessingOutputString( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "CRS_AUTHID" ), QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "WIDTH_IN_PIXELS" ), QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "HEIGHT_IN_PIXELS" ), QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TOTAL_PIXEL_COUNT" ), QObject::tr( "Total pixel count" ) ) );
}

bool QgsCellStatisticsAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "INPUT" ), context );
  QList< QgsRasterLayer * > rasterLayers;
  rasterLayers.reserve( layers.count() );
  for ( QgsMapLayer *l : layers )
  {
    if ( feedback->isCanceled() )
      break; //in case some slow data sources are loaded

    if ( l->type() == Qgis::LayerType::Raster )
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

  //determine output raster data type
  //initially raster data type to most primitive data type that is possible
  mDataType = Qgis::DataType::Byte;
  for ( const QgsRasterAnalysisUtils::RasterLogicInput &i : std::as_const( mInputs ) )
  {
    for ( int band : i.bands )
    {
      Qgis::DataType inputDataType = i.interface->dataType( band );
      if ( static_cast<int>( mDataType ) < static_cast<int>( inputDataType ) )
        mDataType = inputDataType; //if raster data type is more potent, set it as new data type
    }
  }

  prepareSpecificAlgorithmParameters( parameters, context, feedback );

  return true;
}


QVariantMap QgsCellStatisticsAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  std::unique_ptr< QgsRasterFileWriter > writer = std::make_unique< QgsRasterFileWriter >( outputFile );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( outputFormat );
  mOutputRasterDataProvider.reset( writer->createOneBandRaster( mDataType, mLayerWidth, mLayerHeight, mExtent, mCrs ) );
  if ( !mOutputRasterDataProvider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !mOutputRasterDataProvider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, mOutputRasterDataProvider->error().message( QgsErrorMessage::Text ) ) );

  mOutputRasterDataProvider->setNoDataValue( 1, mNoDataValue );
  qgssize layerSize = static_cast< qgssize >( mLayerWidth ) * static_cast< qgssize >( mLayerHeight );

  //call child statistics method
  processRasterStack( feedback );

  mOutputRasterDataProvider.reset();

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
//QgsCellStatisticsAlgorithm
//
QString QgsCellStatisticsAlgorithm::displayName() const
{
  return QObject::tr( "Cell statistics" );
}

QString QgsCellStatisticsAlgorithm::name() const
{
  return QStringLiteral( "cellstatistics" );
}

QStringList QgsCellStatisticsAlgorithm::tags() const
{
  return QObject::tr( "cell,pixel,statistic,count,mean,sum,majority,minority,variance,variety,range,median,minimum,maximum" ).split( ',' );
}

QString QgsCellStatisticsAlgorithm::shortHelpString() const
{
  return QObject::tr( "The Cell statistics algorithm computes a value for each cell of the "
                      "output raster. At each cell location, "
                      "the output value is defined as a function of all overlaid cell values of the "
                      "input rasters.\n\n"
                      "The output raster's extent and resolution is defined by a reference "
                      "raster. The following functions can be applied on the input "
                      "raster cells per output raster cell location:\n"
                      "<ul> "
                      "   <li>Sum</li>"
                      "   <li>Count</li>"
                      "   <li>Mean</li>"
                      "   <li>Median</li>"
                      "   <li>Standard deviation</li>"
                      "   <li>Variance</li>"
                      "   <li>Minimum</li>"
                      "   <li>Maximum</li>"
                      "   <li>Minority (least frequent value)</li>"
                      "   <li>Majority (most frequent value)</li>"
                      "   <li>Range (max-min)</li>"
                      "   <li>Variety (count of unique values)</li>"
                      "</ul> "
                      "Input raster layers that do not match the cell size of the reference raster layer will be "
                      "resampled using nearest neighbor resampling. The output raster data type will be set to "
                      "the most complex data type present in the input datasets except when using the functions "
                      "Mean, Standard deviation and Variance (data type is always Float32/Float64 depending on input float type) or Count and Variety (data type is always Int32).\n"
                      "<i>Calculation details - general:</i> NoData values in any of the input layers will result in a NoData cell output if the Ignore NoData parameter is not set.\n"
                      "<i>Calculation details - Count:</i> Count will always result in the number of cells without NoData values at the current cell location.\n"
                      "<i>Calculation details - Median:</i> If the number of input layers is even, the median will be calculated as the "
                      "arithmetic mean of the two middle values of the ordered cell input values. In this case the output data type is Float32.\n"
                      "<i>Calculation details - Minority/Majority:</i> If no unique minority or majority could be found, the result is NoData, except all "
                      "input cell values are equal." );
}

QgsCellStatisticsAlgorithm *QgsCellStatisticsAlgorithm::createInstance() const
{
  return new QgsCellStatisticsAlgorithm();
}

void QgsCellStatisticsAlgorithm::addSpecificAlgorithmParams()
{
  QStringList statistics = QStringList();
  statistics << QObject::tr( "Sum" )
             << QObject::tr( "Count" )
             << QObject::tr( "Mean" )
             << QObject::tr( "Median" )
             << QObject::tr( "Standard deviation" )
             << QObject::tr( "Variance" )
             << QObject::tr( "Minimum" )
             << QObject::tr( "Maximum" )
             << QObject::tr( "Minority" )
             << QObject::tr( "Majority" )
             << QObject::tr( "Range" )
             << QObject::tr( "Variety" );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "STATISTIC" ), QObject::tr( "Statistic" ),  statistics, false, 0, false ) );
}

bool QgsCellStatisticsAlgorithm::prepareSpecificAlgorithmParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )
  //obtain statistic method
  mMethod = static_cast<QgsRasterAnalysisUtils::CellValueStatisticMethods>( parameterAsEnum( parameters, QStringLiteral( "STATISTIC" ), context ) );

  //force data types on specific functions in the cellstatistics alg if input data types don't match
  if (
    mMethod == QgsRasterAnalysisUtils::Mean ||
    mMethod == QgsRasterAnalysisUtils::StandardDeviation ||
    mMethod == QgsRasterAnalysisUtils::Variance ||
    ( mMethod == QgsRasterAnalysisUtils::Median && ( mInputs.size() % 2 == 0 ) )
  )
  {
    if ( static_cast<int>( mDataType ) < 6 )
      mDataType = Qgis::DataType::Float32; //force float on mean, stddev and median with equal number of input layers if all inputs are integer
  }
  else if ( mMethod == QgsRasterAnalysisUtils::Count || mMethod == QgsRasterAnalysisUtils::Variety ) //count, variety
  {
    if ( static_cast<int>( mDataType ) > 5 ) //if is floating point type
      mDataType = Qgis::DataType::Int32; //force integer on variety if all inputs are float or complex
  }
  return true;
}

void QgsCellStatisticsAlgorithm::processRasterStack( QgsProcessingFeedback *feedback )
{
  int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;
  int nbBlocksWidth = static_cast< int>( std::ceil( 1.0 * mLayerWidth / maxWidth ) );
  int nbBlocksHeight = static_cast< int >( std::ceil( 1.0 * mLayerHeight / maxHeight ) );
  int nbBlocks = nbBlocksWidth * nbBlocksHeight;
  mOutputRasterDataProvider->setEditable( true );
  QgsRasterIterator outputIter( mOutputRasterDataProvider.get() );
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
    for ( const QgsRasterAnalysisUtils::RasterLogicInput &i : std::as_const( mInputs ) )
    {
      if ( feedback->isCanceled() )
        break; //in case some slow data sources are loaded
      for ( int band : i.bands )
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
        double result = 0;
        bool noDataInStack = false;
        std::vector<double> cellValues = QgsRasterAnalysisUtils::getCellValuesFromBlockStack( inputBlocks, row, col, noDataInStack );
        int cellValueStackSize = cellValues.size();

        if ( noDataInStack && !mIgnoreNoData )
        {
          //output cell will always be NoData if NoData occurs in cellValueStack and NoData is not ignored
          //this saves unnecessary iterations on the cellValueStack
          if ( mMethod == QgsRasterAnalysisUtils::Count )
            outputBlock->setValue( row, col,  cellValueStackSize );
          else
          {
            outputBlock->setValue( row, col, mNoDataValue );
          }
        }
        else if ( !noDataInStack || ( mIgnoreNoData && cellValueStackSize > 0 ) )
        {
          switch ( mMethod )
          {
            case QgsRasterAnalysisUtils::Sum:
              result = std::accumulate( cellValues.begin(), cellValues.end(), 0.0 );
              break;
            case QgsRasterAnalysisUtils::Count:
              result = cellValueStackSize;
              break;
            case QgsRasterAnalysisUtils::Mean:
              result = QgsRasterAnalysisUtils::meanFromCellValues( cellValues, cellValueStackSize );
              break;
            case QgsRasterAnalysisUtils::Median:
              result = QgsRasterAnalysisUtils::medianFromCellValues( cellValues, cellValueStackSize );
              break;
            case QgsRasterAnalysisUtils::StandardDeviation:
              result = QgsRasterAnalysisUtils::stddevFromCellValues( cellValues, cellValueStackSize );
              break;
            case QgsRasterAnalysisUtils::Variance:
              result = QgsRasterAnalysisUtils::varianceFromCellValues( cellValues, cellValueStackSize );
              break;
            case QgsRasterAnalysisUtils::Minimum:
              result = QgsRasterAnalysisUtils::minimumFromCellValues( cellValues );
              break;
            case QgsRasterAnalysisUtils::Maximum:
              result = QgsRasterAnalysisUtils::maximumFromCellValues( cellValues );
              break;
            case QgsRasterAnalysisUtils::Minority:
              result = QgsRasterAnalysisUtils::minorityFromCellValues( cellValues, mNoDataValue, cellValueStackSize );
              break;
            case QgsRasterAnalysisUtils::Majority:
              result = QgsRasterAnalysisUtils::majorityFromCellValues( cellValues, mNoDataValue, cellValueStackSize );
              break;
            case QgsRasterAnalysisUtils::Range:
              result = QgsRasterAnalysisUtils::rangeFromCellValues( cellValues );
              break;
            case QgsRasterAnalysisUtils::Variety:
              result = QgsRasterAnalysisUtils::varietyFromCellValues( cellValues );
              break;
          }
          outputBlock->setValue( row, col, result );
        }
        else
        {
          //result is NoData if cellValueStack contains no valid values, eg. all cellValues are NoData
          outputBlock->setValue( row, col, mNoDataValue );
        }
      }
    }
    mOutputRasterDataProvider->writeBlock( outputBlock.get(), 1, iterLeft, iterTop );
  }
  mOutputRasterDataProvider->setEditable( false );
}

//
//QgsCellStatisticsPercentileAlgorithm
//
QString QgsCellStatisticsPercentileAlgorithm::displayName() const
{
  return QObject::tr( "Cell stack percentile" );
}

QString QgsCellStatisticsPercentileAlgorithm::name() const
{
  return QStringLiteral( "cellstackpercentile" );
}

QStringList QgsCellStatisticsPercentileAlgorithm::tags() const
{
  return QObject::tr( "cell,pixel,statistic,percentile,quantile,quartile" ).split( ',' );
}

QString QgsCellStatisticsPercentileAlgorithm::shortHelpString() const
{
  return QObject::tr( "The Cell stack percentile algorithm returns the cell-wise percentile value of a stack of rasters "
                      "and writes the results to an output raster. The percentile to return is determined by the percentile input value (ranges between 0 and 1). "
                      "At each cell location, the specified percentile is obtained using the respective value from "
                      "the stack of all overlaid and sorted cell values of the input rasters.\n\n"
                      "There are three methods for percentile calculation:"
                      "<ul> "
                      "   <li>Nearest rank</li>"
                      "   <li>Inclusive linear interpolation (PERCENTILE.INC)</li>"
                      "   <li>Exclusive linear interpolation (PERCENTILE.EXC)</li>"
                      "</ul> "
                      "While the output value can stay the same for the nearest rank method (obtains the value that is nearest to the "
                      "specified percentile), the linear interpolation method return unique values for different percentiles. Both interpolation "
                      "methods follow their counterpart methods implemented by LibreOffice or Microsoft Excel. \n\n"
                      "The output raster's extent and resolution is defined by a reference "
                      "raster. If the input raster layers that do not match the cell size of the reference raster layer will be "
                      "resampled using nearest neighbor resampling. NoData values in any of the input layers will result in a NoData cell output if the Ignore NoData parameter is not set. "
                      "The output raster data type will be set to the most complex data type present in the input datasets. " );
}

QgsCellStatisticsPercentileAlgorithm *QgsCellStatisticsPercentileAlgorithm::createInstance() const
{
  return new QgsCellStatisticsPercentileAlgorithm();
}

void QgsCellStatisticsPercentileAlgorithm::addSpecificAlgorithmParams()
{
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "METHOD" ), QObject::tr( "Method" ), QStringList() << QObject::tr( "Nearest rank" ) << QObject::tr( "Inclusive linear interpolation (PERCENTILE.INC)" ) << QObject::tr( "Exclusive linear interpolation (PERCENTILE.EXC)" ), false, 0, false ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "PERCENTILE" ), QObject::tr( "Percentile" ), Qgis::ProcessingNumberParameterType::Double, 0.25, false, 0.0, 1.0 ) );
}

bool QgsCellStatisticsPercentileAlgorithm::prepareSpecificAlgorithmParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )
  mMethod = static_cast< QgsRasterAnalysisUtils::CellValuePercentileMethods >( parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context ) );
  mPercentile = parameterAsDouble( parameters, QStringLiteral( "PERCENTILE" ), context );

  //default percentile output data type to float32 raster if interpolation method is chosen
  //otherwise use the most potent data type in the input raster stack (see prepareAlgorithm() in base class)
  if ( mMethod != QgsRasterAnalysisUtils::CellValuePercentileMethods::NearestRankPercentile && static_cast< int >( mDataType ) < 6 )
    mDataType = Qgis::DataType::Float32;

  return true;
}

void QgsCellStatisticsPercentileAlgorithm::processRasterStack( QgsProcessingFeedback *feedback )
{

  int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;
  int nbBlocksWidth = static_cast< int>( std::ceil( 1.0 * mLayerWidth / maxWidth ) );
  int nbBlocksHeight = static_cast< int >( std::ceil( 1.0 * mLayerHeight / maxHeight ) );
  int nbBlocks = nbBlocksWidth * nbBlocksHeight;
  mOutputRasterDataProvider->setEditable( true );
  QgsRasterIterator outputIter( mOutputRasterDataProvider.get() );
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
    for ( const QgsRasterAnalysisUtils::RasterLogicInput &i : std::as_const( mInputs ) )
    {
      if ( feedback->isCanceled() )
        break; //in case some slow data sources are loaded
      for ( int band : i.bands )
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
        double result = 0;
        bool noDataInStack = false;
        std::vector<double> cellValues = QgsRasterAnalysisUtils::getCellValuesFromBlockStack( inputBlocks, row, col, noDataInStack );
        int cellValueStackSize = cellValues.size();

        if ( noDataInStack && !mIgnoreNoData )
        {
          outputBlock->setValue( row, col, mNoDataValue );
        }
        else if ( !noDataInStack || ( mIgnoreNoData && cellValueStackSize > 0 ) )
        {
          switch ( mMethod )
          {
            case QgsRasterAnalysisUtils::NearestRankPercentile:
              result = QgsRasterAnalysisUtils::nearestRankPercentile( cellValues, cellValueStackSize, mPercentile );
              break;
            case QgsRasterAnalysisUtils::InterpolatedPercentileInc:
              result = QgsRasterAnalysisUtils::interpolatedPercentileInc( cellValues, cellValueStackSize, mPercentile );
              break;
            case QgsRasterAnalysisUtils::InterpolatedPercentileExc:
              result = QgsRasterAnalysisUtils::interpolatedPercentileExc( cellValues, cellValueStackSize, mPercentile, mNoDataValue );
              break;
          }
          outputBlock->setValue( row, col, result );
        }
        else
        {
          //result is NoData if cellValueStack contains no valid values, eg. all cellValues are NoData
          outputBlock->setValue( row, col, mNoDataValue );
        }
      }
    }
    mOutputRasterDataProvider->writeBlock( outputBlock.get(), 1, iterLeft, iterTop );
  }
  mOutputRasterDataProvider->setEditable( false );
}

//
//QgsCellStatisticsPercentRankFromValueAlgorithm
//
QString QgsCellStatisticsPercentRankFromValueAlgorithm::displayName() const
{
  return QObject::tr( "Cell stack percent rank from value" );
}

QString QgsCellStatisticsPercentRankFromValueAlgorithm::name() const
{
  return QStringLiteral( "cellstackpercentrankfromvalue" );
}

QStringList QgsCellStatisticsPercentRankFromValueAlgorithm::tags() const
{
  return QObject::tr( "cell,pixel,statistic,percentrank,rank,percent,value" ).split( ',' );
}

QString QgsCellStatisticsPercentRankFromValueAlgorithm::shortHelpString() const
{
  return QObject::tr( "The Cell stack percentrank from value algorithm calculates the cell-wise percentrank value of a stack of rasters based on a single input value "
                      "and writes them to an output raster.\n\n"
                      "At each cell location, the specified value is ranked among the respective values in the stack of all overlaid and sorted cell values from the input rasters. "
                      "For values outside of the stack value distribution, the algorithm returns NoData because the value cannot be ranked among the cell values.\n\n"
                      "There are two methods for percentile calculation:"
                      "<ul> "
                      "   <li>Inclusive linearly interpolated percent rank (PERCENTRANK.INC)</li>"
                      "   <li>Exclusive linearly interpolated percent rank (PERCENTRANK.EXC)</li>"
                      "</ul> "
                      "The linear interpolation method return the unique percent rank for different values. Both interpolation "
                      "methods follow their counterpart methods implemented by LibreOffice or Microsoft Excel. \n\n"
                      "The output raster's extent and resolution is defined by a reference "
                      "raster. If the input raster layers that do not match the cell size of the reference raster layer will be "
                      "resampled using nearest neighbor resampling. NoData values in any of the input layers will result in a NoData cell output if the Ignore NoData parameter is not set. "
                      "The output raster data type will always be Float32." );
}

QgsCellStatisticsPercentRankFromValueAlgorithm *QgsCellStatisticsPercentRankFromValueAlgorithm::createInstance() const
{
  return new QgsCellStatisticsPercentRankFromValueAlgorithm();
}

void QgsCellStatisticsPercentRankFromValueAlgorithm::addSpecificAlgorithmParams()
{
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "METHOD" ), QObject::tr( "Method" ), QStringList() << QObject::tr( "Inclusive linear interpolation (PERCENTRANK.INC)" ) << QObject::tr( "Exclusive linear interpolation (PERCENTRANK.EXC)" ), false, 0, false ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "VALUE" ), QObject::tr( "Value" ), Qgis::ProcessingNumberParameterType::Double, 10, false ) );
}

bool QgsCellStatisticsPercentRankFromValueAlgorithm::prepareSpecificAlgorithmParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )
  mMethod = static_cast< QgsRasterAnalysisUtils::CellValuePercentRankMethods >( parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context ) );
  mValue = parameterAsDouble( parameters, QStringLiteral( "VALUE" ), context );

  //output data type always defaults to Float32 because result only ranges between 0 and 1
  mDataType = Qgis::DataType::Float32;
  return true;
}

void QgsCellStatisticsPercentRankFromValueAlgorithm::processRasterStack( QgsProcessingFeedback *feedback )
{

  int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;
  int nbBlocksWidth = static_cast< int>( std::ceil( 1.0 * mLayerWidth / maxWidth ) );
  int nbBlocksHeight = static_cast< int >( std::ceil( 1.0 * mLayerHeight / maxHeight ) );
  int nbBlocks = nbBlocksWidth * nbBlocksHeight;
  mOutputRasterDataProvider->setEditable( true );
  QgsRasterIterator outputIter( mOutputRasterDataProvider.get() );
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
    for ( const QgsRasterAnalysisUtils::RasterLogicInput &i : std::as_const( mInputs ) )
    {
      if ( feedback->isCanceled() )
        break; //in case some slow data sources are loaded
      for ( int band : i.bands )
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
        double result = 0;
        bool noDataInStack = false;
        std::vector<double> cellValues = QgsRasterAnalysisUtils::getCellValuesFromBlockStack( inputBlocks, row, col, noDataInStack );
        int cellValueStackSize = cellValues.size();

        if ( noDataInStack && !mIgnoreNoData )
        {
          outputBlock->setValue( row, col, mNoDataValue );
        }
        else if ( !noDataInStack || ( mIgnoreNoData && cellValueStackSize > 0 ) )
        {
          switch ( mMethod )
          {
            case QgsRasterAnalysisUtils::InterpolatedPercentRankInc:
              result = QgsRasterAnalysisUtils::interpolatedPercentRankInc( cellValues, cellValueStackSize, mValue, mNoDataValue );
              break;
            case QgsRasterAnalysisUtils::InterpolatedPercentRankExc:
              result = QgsRasterAnalysisUtils::interpolatedPercentRankExc( cellValues, cellValueStackSize, mValue, mNoDataValue );
              break;
          }
          outputBlock->setValue( row, col, result );
        }
        else
        {
          //result is NoData if cellValueStack contains no valid values, eg. all cellValues are NoData
          outputBlock->setValue( row, col, mNoDataValue );
        }
      }
    }
    mOutputRasterDataProvider->writeBlock( outputBlock.get(), 1, iterLeft, iterTop );
  }
  mOutputRasterDataProvider->setEditable( false );
}


//
//QgsCellStatisticsPercentRankFromRasterAlgorithm
//
QString QgsCellStatisticsPercentRankFromRasterAlgorithm::displayName() const
{
  return QObject::tr( "Cell stack percentrank from raster layer" );
}

QString QgsCellStatisticsPercentRankFromRasterAlgorithm::name() const
{
  return QStringLiteral( "cellstackpercentrankfromrasterlayer" );
}

QStringList QgsCellStatisticsPercentRankFromRasterAlgorithm::tags() const
{
  return QObject::tr( "cell,pixel,statistic,percentrank,rank,percent,value,raster" ).split( ',' );
}

QString QgsCellStatisticsPercentRankFromRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "The Cell stack percentrank from raster layer algorithm calculates the cell-wise percentrank value of a stack of rasters based on an input value raster "
                      "and writes them to an output raster.\n\n"
                      "At each cell location, the current value of the value raster is used ranked among the respective values in the stack of all overlaid and sorted cell values of the input rasters. "
                      "For values outside of the the stack value distribution, the algorithm returns NoData because the value cannot be ranked among the cell values.\n\n"
                      "There are two methods for percentile calculation:"
                      "<ul> "
                      "   <li>Inclusive linearly interpolated percent rank (PERCENTRANK.INC)</li>"
                      "   <li>Exclusive linearly interpolated percent rank (PERCENTRANK.EXC)</li>"
                      "</ul> "
                      "The linear interpolation method return the unique percent rank for different values. Both interpolation "
                      "methods follow their counterpart methods implemented by LibreOffice or Microsoft Excel. \n\n"
                      "The output raster's extent and resolution is defined by a reference "
                      "raster. If the input raster layers that do not match the cell size of the reference raster layer will be "
                      "resampled using nearest neighbor resampling.  NoData values in any of the input layers will result in a NoData cell output if the Ignore NoData parameter is not set. "
                      "The output raster data type will always be Float32." );
}

QgsCellStatisticsPercentRankFromRasterAlgorithm *QgsCellStatisticsPercentRankFromRasterAlgorithm::createInstance() const
{
  return new QgsCellStatisticsPercentRankFromRasterAlgorithm();
}

void QgsCellStatisticsPercentRankFromRasterAlgorithm::addSpecificAlgorithmParams()
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT_VALUE_RASTER" ), QObject::tr( "Value raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "VALUE_RASTER_BAND" ), QObject::tr( "Value raster band" ), 1, QStringLiteral( "VALUE_LAYER" ) ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "METHOD" ), QObject::tr( "Method" ), QStringList() << QObject::tr( "Inclusive linear interpolation (PERCENTRANK.INC)" ) << QObject::tr( "Exclusive linear interpolation (PERCENTRANK.EXC)" ), false, 0, false ) );
}

bool QgsCellStatisticsPercentRankFromRasterAlgorithm::prepareSpecificAlgorithmParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )
  mMethod = static_cast< QgsRasterAnalysisUtils::CellValuePercentRankMethods >( parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context ) );

  QgsRasterLayer *inputValueRaster = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT_VALUE_RASTER" ), context );
  if ( !inputValueRaster )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT_VALUE_RASTER" ) ) );

  mValueRasterInterface.reset( inputValueRaster->dataProvider()->clone() );

  mValueRasterBand = parameterAsInt( parameters, QStringLiteral( "VALUE_RASTER_BAND" ), context );

  //output data type always defaults to Float32 because result only ranges between 0 and 1
  mDataType = Qgis::DataType::Float32;
  return true;
}

void QgsCellStatisticsPercentRankFromRasterAlgorithm::processRasterStack( QgsProcessingFeedback *feedback )
{
  int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;
  int nbBlocksWidth = static_cast< int>( std::ceil( 1.0 * mLayerWidth / maxWidth ) );
  int nbBlocksHeight = static_cast< int >( std::ceil( 1.0 * mLayerHeight / maxHeight ) );
  int nbBlocks = nbBlocksWidth * nbBlocksHeight;
  mOutputRasterDataProvider->setEditable( true );
  QgsRasterIterator outputIter( mOutputRasterDataProvider.get() );
  outputIter.startRasterRead( 1, mLayerWidth, mLayerHeight, mExtent );

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  QgsRectangle blockExtent;
  std::unique_ptr< QgsRasterBlock > outputBlock;
  while ( outputIter.readNextRasterPart( 1, iterCols, iterRows, outputBlock, iterLeft, iterTop, &blockExtent ) )
  {
    std::unique_ptr< QgsRasterBlock > valueBlock( mValueRasterInterface->block( mValueRasterBand, blockExtent, iterCols, iterRows ) );

    std::vector< std::unique_ptr< QgsRasterBlock > > inputBlocks;
    for ( const QgsRasterAnalysisUtils::RasterLogicInput &i : std::as_const( mInputs ) )
    {
      if ( feedback->isCanceled() )
        break; //in case some slow data sources are loaded
      for ( int band : i.bands )
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
        bool percentRankValueIsNoData = false;
        double percentRankValue = valueBlock->valueAndNoData( row, col, percentRankValueIsNoData );

        double result = 0;
        bool noDataInStack = false;
        std::vector<double> cellValues = QgsRasterAnalysisUtils::getCellValuesFromBlockStack( inputBlocks, row, col, noDataInStack );
        int cellValueStackSize = cellValues.size();

        if ( noDataInStack && !mIgnoreNoData && !percentRankValueIsNoData )
        {
          outputBlock->setValue( row, col, mNoDataValue );
        }
        else if ( !noDataInStack || ( !percentRankValueIsNoData && mIgnoreNoData && cellValueStackSize > 0 ) )
        {
          switch ( mMethod )
          {
            case QgsRasterAnalysisUtils::InterpolatedPercentRankInc:
              result = QgsRasterAnalysisUtils::interpolatedPercentRankInc( cellValues, cellValueStackSize, percentRankValue, mNoDataValue );
              break;
            case QgsRasterAnalysisUtils::InterpolatedPercentRankExc:
              result = QgsRasterAnalysisUtils::interpolatedPercentRankExc( cellValues, cellValueStackSize, percentRankValue, mNoDataValue );
              break;
          }
          outputBlock->setValue( row, col, result );
        }
        else
        {
          //result is NoData if cellValueStack contains no valid values, eg. all cellValues are NoData or percentRankValue is NoData
          outputBlock->setValue( row, col, mNoDataValue );
        }
      }
    }
    mOutputRasterDataProvider->writeBlock( outputBlock.get(), 1, iterLeft, iterTop );
  }
  mOutputRasterDataProvider->setEditable( false );
}

///@endcond


