/***************************************************************************
                         qgsalgorithmrasterrank.cpp
                         ---------------------
    begin                : February 2024
    copyright            : (C) 2024 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmrasterrank.h"

#include <memory>

#include "qgsrasterfilewriter.h"
#include "qgsrasterprojector.h"

///@cond PRIVATE

QString QgsRasterRankAlgorithm::name() const
{
  return u"rasterrank"_s;
}

QString QgsRasterRankAlgorithm::displayName() const
{
  return QObject::tr( "Raster rank" );
}

QStringList QgsRasterRankAlgorithm::tags() const
{
  return QObject::tr( "raster,rank" ).split( ',' );
}

QString QgsRasterRankAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterRankAlgorithm::groupId() const
{
  return u"rasteranalysis"_s;
}

QString QgsRasterRankAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm performs a cell-by-cell analysis in which output values match the rank of a "
                      "sorted list of overlapping cell values from input layers. The output raster "
                      "will be multi-band if multiple ranks are provided.\n"
                      "If multiband rasters are used in the data raster stack, the algorithm will always "
                      "perform the analysis on the first band of the rasters." );
}

QString QgsRasterRankAlgorithm::shortDescription() const
{
  return QObject::tr( "Performs a cell-by-cell analysis in which output values match the rank of a "
                      "sorted list of overlapping cell values from input layers." );
}

QgsRasterRankAlgorithm *QgsRasterRankAlgorithm::createInstance() const
{
  return new QgsRasterRankAlgorithm();
}

void QgsRasterRankAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( u"INPUT_RASTERS"_s, QObject::tr( "Input raster layers" ), Qgis::ProcessingSourceType::Raster ) );
  auto ranksParameter = std::make_unique<QgsProcessingParameterString>( u"RANKS"_s, QObject::tr( "Rank(s) (separate multiple ranks using commas)" ), 1 );
  ranksParameter->setHelp( QObject::tr( "A rank value must be numerical, with multiple ranks separated by commas. The rank will be used to "
                                        "generate output values from sorted lists of input layers’ cell values. A rank value of 1 will pick "
                                        "the first value from a given sorted input layers’ cell values list (i.e. the minimum value). "
                                        "Negative rank values are supported, and will behave like a negative index. A rank value of -2 will "
                                        "pick the second to last value in sorted input values lists, while a rank value of -1 will pick the "
                                        "last value (i.e. the maximum value)." ) );
  addParameter( ranksParameter.release() );
  addParameter( new QgsProcessingParameterEnum( u"NODATA_HANDLING"_s, QObject::tr( "NoData value handling" ), QStringList() << QObject::tr( "Exclude NoData from values lists" ) << QObject::tr( "Presence of NoData in a values list results in NoData output cell" ), false, 0 ) );

  auto extentParam = std::make_unique<QgsProcessingParameterExtent>( u"EXTENT"_s, QObject::tr( "Output extent" ), QVariant(), true );
  extentParam->setHelp( QObject::tr( "Extent of the output layer. If not specified, the extent will be the overall extent of all input layers" ) );
  extentParam->setFlags( extentParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( extentParam.release() );
  auto cellSizeParam = std::make_unique<QgsProcessingParameterNumber>( u"CELL_SIZE"_s, QObject::tr( "Output cell size" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0.0 );
  cellSizeParam->setHelp( QObject::tr( "Cell size of the output layer. If not specified, the smallest cell size from the input layers will be used" ) );
  cellSizeParam->setFlags( cellSizeParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( cellSizeParam.release() );
  auto crsParam = std::make_unique<QgsProcessingParameterCrs>( u"CRS"_s, QObject::tr( "Output CRS" ), QVariant(), true );
  crsParam->setHelp( QObject::tr( "CRS of the output layer. If not specified, the CRS of the first input layer will be used" ) );
  crsParam->setFlags( crsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( crsParam.release() );

  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Ranked" ) ) );
}

bool QgsRasterRankAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  const QStringList rankStrings = parameterAsString( parameters, u"RANKS"_s, context ).split( ","_L1 );
  for ( const QString &rankString : rankStrings )
  {
    bool ok = false;
    const int rank = rankString.toInt( &ok );
    if ( ok && rank != 0 )
    {
      mRanks << rank;
    }
    else
    {
      throw QgsProcessingException( QObject::tr( "Rank values must be integers (found \"%1\")" ).arg( rankString ) );
    }
  }

  if ( mRanks.isEmpty() )
  {
    throw QgsProcessingException( QObject::tr( "No valid non-zero rank value(s) provided" ) );
    return false;
  }

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"INPUT_RASTERS"_s, context );
  for ( const QgsMapLayer *layer : std::as_const( layers ) )
  {
    if ( !qobject_cast<const QgsRasterLayer *>( layer ) || !layer->dataProvider() )
      continue;

    std::unique_ptr<QgsMapLayer> clonedLayer;
    clonedLayer.reset( layer->clone() );
    clonedLayer->moveToThread( nullptr );
    mLayers.push_back( std::move( clonedLayer ) );
  }

  if ( mLayers.empty() )
  {
    throw QgsProcessingException( QObject::tr( "At least one raster input layer must be specified" ) );
    return false;
  }

  return true;
}


QVariantMap QgsRasterRankAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QList<QgsMapLayer *> layers; // Needed for QgsProcessingUtils::combineLayerExtents
  for ( auto &layer : mLayers )
  {
    layer->moveToThread( QThread::currentThread() );
    layers << layer.get();
  }

  QgsCoordinateReferenceSystem outputCrs;
  if ( parameters.value( u"CRS"_s ).isValid() )
  {
    outputCrs = parameterAsCrs( parameters, u"CRS"_s, context );
  }
  else
  {
    outputCrs = mLayers[0]->crs();
  }


  QgsRasterLayer *templateRasterLayer = qobject_cast<QgsRasterLayer *>( mLayers[0].get() );
  const Qgis::DataType outputDataType = templateRasterLayer->dataProvider()->dataType( 1 );
  double outputNoData = 0.0;
  if ( templateRasterLayer->dataProvider()->sourceHasNoDataValue( 1 ) )
  {
    outputNoData = templateRasterLayer->dataProvider()->sourceNoDataValue( 1 );
  }
  else
  {
    outputNoData = -FLT_MAX;
  }
  const bool outputNoDataOverride = parameterAsInt( parameters, u"NODATA_HANDLING"_s, context ) == 1;

  QgsRectangle outputExtent;
  if ( parameters.value( u"EXTENT"_s ).isValid() )
  {
    outputExtent = parameterAsExtent( parameters, u"EXTENT"_s, context, outputCrs );
  }
  else
  {
    outputExtent = QgsProcessingUtils::combineLayerExtents( layers, outputCrs, context );
  }

  double minCellSizeX = 1e9;
  double minCellSizeY = 1e9;
  for ( auto &layer : mLayers )
  {
    QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( layer.get() );

    QgsRectangle extent = rasterLayer->extent();
    if ( rasterLayer->crs() != outputCrs )
    {
      QgsCoordinateTransform ct( rasterLayer->crs(), outputCrs, context.transformContext() );
      extent = ct.transformBoundingBox( extent );
    }

    const int width = rasterLayer->width();
    const int height = rasterLayer->height();
    if ( width <= 0 || height <= 0 )
      throw QgsProcessingException( QObject::tr( "%1: raster is empty, cannot proceed" ).arg( rasterLayer->name() ) );

    minCellSizeX = std::min( minCellSizeX, ( extent.xMaximum() - extent.xMinimum() ) / width );
    minCellSizeY = std::min( minCellSizeY, ( extent.yMaximum() - extent.yMinimum() ) / height );
  }

  double outputCellSizeX = parameterAsDouble( parameters, u"CELL_SIZE"_s, context );
  double outputCellSizeY = outputCellSizeX;
  if ( outputCellSizeX == 0 )
  {
    outputCellSizeX = minCellSizeX;
    outputCellSizeY = minCellSizeY;
  }

  qgssize cols = static_cast<qgssize>( std::round( ( outputExtent.xMaximum() - outputExtent.xMinimum() ) / outputCellSizeX ) );
  qgssize rows = static_cast<qgssize>( std::round( ( outputExtent.yMaximum() - outputExtent.yMinimum() ) / outputCellSizeY ) );

  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  auto writer = std::make_unique<QgsRasterFileWriter>( outputFile );
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider> provider( writer->createMultiBandRaster( outputDataType, cols, rows, outputExtent, outputCrs, mRanks.size() ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );
  provider->setNoDataValue( 1, outputNoData );

  std::map<QString, std::unique_ptr<QgsRasterInterface>> newProjectorInterfaces;
  std::map<QString, QgsRasterInterface *> inputInterfaces;
  std::map<QString, std::unique_ptr<QgsRasterBlock>> inputBlocks;
  std::vector<std::unique_ptr<QgsRasterBlock>> outputBlocks;
  outputBlocks.resize( mRanks.size() );
  for ( auto &layer : mLayers )
  {
    QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( layer.get() );
    if ( rasterLayer->crs() != outputCrs )
    {
      QgsRasterProjector *projector = new QgsRasterProjector();
      projector->setCrs( rasterLayer->crs(), outputCrs, context.transformContext() );
      projector->setInput( rasterLayer->dataProvider() );
      projector->setPrecision( QgsRasterProjector::Exact );
      newProjectorInterfaces[rasterLayer->id()].reset( projector );
      inputInterfaces[rasterLayer->id()] = projector;
    }
    else
    {
      inputInterfaces[rasterLayer->id()] = rasterLayer->dataProvider();
    }
  }

  QgsRasterIterator rasterIterator( inputInterfaces[mLayers.at( 0 )->id()] );
  rasterIterator.startRasterRead( 1, cols, rows, outputExtent );
  int blockCount = static_cast<int>( rasterIterator.blockCount() );

  const double step = blockCount > 0 ? 100.0 / blockCount : 0;
  std::vector<double> inputValues;
  inputValues.resize( mLayers.size() );
  for ( int currentBlock = 0; currentBlock < blockCount; currentBlock++ )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }
    feedback->setProgress( currentBlock * step );

    int iterLeft = 0;
    int iterTop = 0;
    int iterCols = 0;
    int iterRows = 0;
    QgsRectangle blockExtent;
    rasterIterator.next( 1, iterCols, iterRows, iterLeft, iterTop, blockExtent );

    for ( const auto &inputInterface : inputInterfaces )
    {
      inputBlocks[inputInterface.first].reset( inputInterface.second->block( 1, blockExtent, iterCols, iterRows ) );
    }

    for ( int i = 0; i < mRanks.size(); i++ )
    {
      outputBlocks[i] = std::make_unique<QgsRasterBlock>( outputDataType, iterCols, iterRows );
      outputBlocks[i]->setNoDataValue( outputNoData );
    }

    for ( int row = 0; row < iterRows; row++ )
    {
      for ( int col = 0; col < iterCols; col++ )
      {
        int valuesCount = 0;
        for ( const auto &inputBlock : inputBlocks )
        {
          bool isNoData = false;
          const double value = inputBlock.second->valueAndNoData( row, col, isNoData );
          if ( !isNoData )
          {
            inputValues[valuesCount] = value;
            valuesCount++;
          }
          else if ( outputNoDataOverride )
          {
            valuesCount = 0;
            break;
          }
        }
        std::sort( inputValues.begin(), inputValues.begin() + valuesCount );

        for ( int i = 0; i < mRanks.size(); i++ )
        {
          if ( valuesCount >= std::abs( mRanks[i] ) )
          {
            outputBlocks[i]->setValue( row, col, inputValues[mRanks[i] > 0 ? mRanks[i] - 1 : valuesCount + mRanks[i]] );
          }
          else
          {
            outputBlocks[i]->setValue( row, col, outputNoData );
          }
        }
      }
    }

    for ( int i = 0; i < mRanks.size(); i++ )
    {
      if ( !provider->writeBlock( outputBlocks[i].get(), i + 1, iterLeft, iterTop ) )
      {
        throw QgsProcessingException( QObject::tr( "Could not write output raster block: %1" ).arg( provider->error().summary() ) );
      }
    }
  }

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

///@endcond
