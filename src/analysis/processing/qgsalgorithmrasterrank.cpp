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
#include "qgsrasterfilewriter.h"
#include "qgsrasterprojector.h"

///@cond PRIVATE

QString QgsRasterRankAlgorithm::name() const
{
  return QStringLiteral( "rasterrank" );
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
  return QStringLiteral( "rasteranalysis" );
}

QString QgsRasterRankAlgorithm::shortHelpString() const
{
  return QObject::tr( "Performing a cell-by-cell analysis in which output values match the rank of a sorted list of overlapping cell values from input layers." );
}

QgsRasterRankAlgorithm *QgsRasterRankAlgorithm::createInstance() const
{
  return new QgsRasterRankAlgorithm();
}

void QgsRasterRankAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::Raster ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "RANKS" ), QObject::tr( "Rank (separate multiple ranks using commas)" ), 1 ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "NODATA_HANDLING" ), QObject::tr( "NoData value handling" ), QStringList() << QObject::tr( "Exclude NoData from values lists" ) << QObject::tr( "Presence of NoData in a values list results in NoData output cell" ), false, 0 ) );

  auto extentParam = std::make_unique<QgsProcessingParameterExtent>( QStringLiteral( "EXTENT" ), QObject::tr( "Output extent" ), QVariant(), true );
  extentParam->setHelp( QObject::tr( "Extent of the output layer. If not specified, the extent will be the overall extent of all input layers" ) );
  extentParam->setFlags( extentParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( extentParam.release() );
  auto cellSizeParam = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "CELL_SIZE" ), QObject::tr( "Output cell size" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0.0 );
  cellSizeParam->setHelp( QObject::tr( "Cell size of the output layer. If not specified, the smallest cell size from the input layers will be used" ) );
  cellSizeParam->setFlags( cellSizeParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( cellSizeParam.release() );
  auto crsParam = std::make_unique<QgsProcessingParameterCrs>( QStringLiteral( "CRS" ), QObject::tr( "Output CRS" ), QVariant(), true );
  crsParam->setHelp( QObject::tr( "CRS of the output layer. If not specified, the CRS of the first input layer will be used" ) );
  crsParam->setFlags( crsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( crsParam.release() );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Calculated" ) ) );
}

bool QgsRasterRankAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QStringList rankStrings = parameterAsString( parameters, QStringLiteral( "RANKS" ), context ).split( QLatin1String( "," ) );
  for ( const QString &rankString : rankStrings )
  {
    bool ok = false;
    const int rank = rankString.toInt( &ok );
    if ( ok && rank != 0 )
    {
      mRanks << rank;
    }
  }

  if ( mRanks.isEmpty() )
  {
    feedback->reportError( QObject::tr( "No valid non-zero rank value(s) provided" ), false );
    return false;
  }

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context );
  for ( const QgsMapLayer *layer : std::as_const( layers ) )
  {
    if ( !qobject_cast<const QgsRasterLayer *>( layer ) || !layer->dataProvider() )
      continue;

    QgsMapLayer *clonedLayer = layer->clone();
    clonedLayer->moveToThread( nullptr );
    mLayers << clonedLayer;
  }

  if ( mLayers.isEmpty() )
  {
    feedback->reportError( QObject::tr( "No raster layers selected" ), false );
    return false;
  }

  return true;
}


QVariantMap QgsRasterRankAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    layer->moveToThread( QThread::currentThread() );
  }

  QgsCoordinateReferenceSystem outputCrs;
  if ( parameters.value( QStringLiteral( "CRS" ) ).isValid() )
  {
    outputCrs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );
  }
  else
  {
    outputCrs = mLayers.at( 0 )->crs();
  }


  const Qgis::DataType outputDataType = qobject_cast<QgsRasterLayer *>( mLayers.at( 0 ) )->dataProvider()->dataType( 1 );
  double outputNoData = 0.0;
  if ( qobject_cast<QgsRasterLayer *>( mLayers.at( 0 ) )->dataProvider()->sourceHasNoDataValue( 1 ) )
  {
    outputNoData = qobject_cast<QgsRasterLayer *>( mLayers.at( 0 ) )->dataProvider()->sourceNoDataValue( 1 );
  }
  else
  {
    outputNoData = -FLT_MAX;
  }
  const bool outputNoDataOverride = parameterAsInt( parameters, QStringLiteral( "NODATA_HANDLING" ), context ) == 1;

  QgsRectangle outputExtent;
  if ( parameters.value( QStringLiteral( "EXTENT" ) ).isValid() )
  {
    outputExtent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, outputCrs );
  }
  else
  {
    outputExtent = QgsProcessingUtils::combineLayerExtents( mLayers, outputCrs, context );
  }

  double minCellSizeX = 1e9;
  double minCellSizeY = 1e9;
  std::map<QString, std::unique_ptr<QgsRasterBlock>> inputBlocks;
  for ( QgsMapLayer *layer : mLayers )
  {
    QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( layer );

    QgsRectangle extent = rLayer->extent();
    if ( rLayer->crs() != outputCrs )
    {
      QgsCoordinateTransform ct( rLayer->crs(), outputCrs, context.transformContext() );
      extent = ct.transformBoundingBox( extent );
    }

    minCellSizeX = std::min( minCellSizeX, ( extent.xMaximum() - extent.xMinimum() ) / rLayer->width() );
    minCellSizeY = std::min( minCellSizeY, ( extent.yMaximum() - extent.yMinimum() ) / rLayer->height() );

    inputBlocks[rLayer->id()] = std::make_unique<QgsRasterBlock>();
  }

  double outputCellSizeX = parameterAsDouble( parameters, QStringLiteral( "CELL_SIZE" ), context );
  double outputCellSizeY = outputCellSizeX;
  if ( outputCellSizeX == 0 )
  {
    outputCellSizeX = minCellSizeX;
    outputCellSizeY = minCellSizeY;
  }

  double cols = std::round( ( outputExtent.xMaximum() - outputExtent.xMinimum() ) / outputCellSizeX );
  double rows = std::round( ( outputExtent.yMaximum() - outputExtent.yMinimum() ) / outputCellSizeY );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  auto writer = std::make_unique<QgsRasterFileWriter>( outputFile );
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider> provider( writer->createMultiBandRaster( outputDataType, cols, rows, outputExtent, outputCrs, mRanks.size() ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );
  provider->setNoDataValue( 1, outputNoData );


  std::vector<std::unique_ptr<QgsRasterBlock>> outputBlocks;
  for ( int i = 0; i < mRanks.size(); i++ )
  {
    outputBlocks.push_back( std::make_unique<QgsRasterBlock>() );
  }

  const double step = rows > 0 ? 100.0 / rows : 1;
  for ( int row = 0; row < rows; row++ )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    for ( int i = 0; i < mRanks.size(); i++ )
    {
      outputBlocks[i].reset( new QgsRasterBlock( outputDataType, cols, 1 ) );
      outputBlocks[i]->setNoDataValue( outputNoData );
    }

    // Calculates the rect for a single row read
    QgsRectangle rowExtent( outputExtent );
    rowExtent.setYMaximum( rowExtent.yMaximum() - outputCellSizeY * row );
    rowExtent.setYMinimum( rowExtent.yMaximum() - outputCellSizeY );

    for ( QgsMapLayer *layer : mLayers )
    {
      QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( layer );

      if ( rLayer->crs() != outputCrs )
      {
        QgsRasterProjector proj;
        proj.setCrs( rLayer->crs(), outputCrs, context.transformContext() );
        proj.setInput( rLayer->dataProvider() );
        proj.setPrecision( QgsRasterProjector::Exact );
        inputBlocks[rLayer->id()].reset( proj.block( 1, rowExtent, cols, 1 ) );
      }
      else
      {
        inputBlocks[rLayer->id()].reset( rLayer->dataProvider()->block( 1, rowExtent, cols, 1 ) );
      }
    }

    for ( int col = 0; col < cols; col++ )
    {
      QList<double> values;
      for ( const auto &inputBlock : inputBlocks )
      {
        bool isNoData = false;
        const double value = inputBlock.second->valueAndNoData( 0, col, isNoData );
        if ( !isNoData )
        {
          values << value;
        }
        else if ( outputNoDataOverride )
        {
          values.clear();
          break;
        }
      }
      std::sort( values.begin(), values.end() );

      for ( int i = 0; i < mRanks.size(); i++ )
      {
        if ( values.size() >= std::abs( mRanks[i] ) )
        {
          outputBlocks[i]->setValue( 0, col, values.at( mRanks[i] > 0 ? mRanks[i] - 1 : values.size() + mRanks[i] ) );
        }
        else
        {
          outputBlocks[i]->setValue( 0, col, outputNoData );
        }
      }
    }

    for ( int i = 0; i < mRanks.size(); i++ )
    {
      provider->writeBlock( outputBlocks[i].get(), i + 1, 0, row );
    }
    feedback->setProgress( row * step );
  }

  qDeleteAll( mLayers );
  mLayers.clear();

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

///@endcond
