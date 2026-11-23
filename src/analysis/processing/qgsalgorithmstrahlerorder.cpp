/***************************************************************************
                         qgsalgorithmstrahlerorder.cpp
                         ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#include "qgsalgorithmstrahlerorder.h"

#include "qgsrasteranalysisutils.h"
#include "qgsrasterfilewriter.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QStringList QgsStrahlerOrderAlgorithmBase::tags() const
{
  return QObject::tr( "dem,strahler,order,stream,channels,hydrology,network,catchment" ).split( ',' );
}

void QgsStrahlerOrderAlgorithmBase::addCommonParameters()
{
  addParameter( new QgsProcessingParameterNumber( u"THRESHOLD"_s, QObject::tr( "Minimum stream order threshold" ), Qgis::ProcessingNumberParameterType::Integer, 1, false, 1 ) );

  auto outputNodataParam = std::make_unique<QgsProcessingParameterNumber>( u"NODATA"_s, QObject::tr( "Output NoData value" ), Qgis::ProcessingNumberParameterType::Integer, -9999 );
  outputNodataParam->setHelp( QObject::tr( "The NODATA value to use in the output raster." ) );
  outputNodataParam->setFlags( outputNodataParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( outputNodataParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATION_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setHelp( QObject::tr( "The raster creation options for the output raster. These options control things like colorimetry, compression, etc." ) );
  creationOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  auto outputParam = std::make_unique<QgsProcessingParameterRasterDestination>( u"OUTPUT"_s, QObject::tr( "Strahler order" ) );
  addParameter( outputParam.release() );
}

QVariantMap QgsStrahlerOrderAlgorithmBase::writeOutputRaster( QgsRasterBlock *outputBlock, const QString &outputFile, const QString &outputFormat, const QString &creationOptions )
{
  auto outputWriter = std::make_unique<QgsRasterFileWriter>( outputFile );
  outputWriter->setOutputProviderKey( u"gdal"_s );
  if ( !creationOptions.isEmpty() )
  {
    outputWriter->setCreationOptions( creationOptions.split( '|' ) );
  }
  outputWriter->setOutputFormat( outputFormat );

  std::unique_ptr<QgsRasterDataProvider> destProvider( outputWriter->createOneBandRaster( Qgis::DataType::Int16, mLayerWidth, mLayerHeight, mExtent, mCrs ) );
  if ( !destProvider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !destProvider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, destProvider->error().message( QgsErrorMessage::Text ) ) );

  destProvider->setNoDataValue( 1, mOutputNoData );
  destProvider->setEditable( true );

  if ( !destProvider->writeBlock( outputBlock, 1 ) )
  {
    throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destProvider->error().summary() ) );
  }

  destProvider->setEditable( false );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

//
// QgsStrahlerOrderFromDemAlgorithm
//

QString QgsStrahlerOrderFromDemAlgorithm::name() const
{
  return u"strahlerorderfromdem"_s;
}

QString QgsStrahlerOrderFromDemAlgorithm::displayName() const
{
  return QObject::tr( "Strahler order from DEM" );
}

QString QgsStrahlerOrderFromDemAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates Strahler stream order directly from an input DEM raster." );
}

QString QgsStrahlerOrderFromDemAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm calculates Strahler stream order from an input elevation raster (DEM).\n\n"
    "D8 flow directions are computed internally to traverse channel trees topographically.\n"
    "Confluences of two stream channels of order N produce a downstream channel of order N + 1.\n"
    "When the threshold is set to 1, raw stream orders (1, 2, 3...) are calculated. "
    "Higher threshold values mask non-stream cells as NoData and offset stream orders.\n\n"
    "This algorithm is a port of the Strahler stream order calculation from SAGA 'Channel Network and Drainage Basins' tool."
  );
}

void QgsStrahlerOrderFromDemAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Elevation raster" ) ) );
  addCommonParameters();
}

QgsProcessingAlgorithm *QgsStrahlerOrderFromDemAlgorithm::createInstance() const
{
  return new QgsStrahlerOrderFromDemAlgorithm();
}

bool QgsStrahlerOrderFromDemAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  if ( !layer || !layer->dataProvider() )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  mDemInterface.reset( layer->dataProvider()->clone() );
  mLayerWidth = layer->width();
  mLayerHeight = layer->height();
  mExtent = layer->extent();
  mCrs = layer->crs();
  mCellSizeX = layer->rasterUnitsPerPixelX();
  mCellSizeY = layer->rasterUnitsPerPixelY();

  return true;
}

QVariantMap QgsStrahlerOrderFromDemAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  const int threshold = parameterAsInt( parameters, u"THRESHOLD"_s, context );
  const QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );
  mOutputNoData = parameterAsInt( parameters, u"NODATA"_s, context );

  std::unique_ptr<QgsRasterBlock> demBlock( mDemInterface->block( 1, mExtent, mLayerWidth, mLayerHeight ) );
  if ( !demBlock )
    throw QgsProcessingException( QObject::tr( "Could not read input DEM block." ) );

  const qgssize totalCells = static_cast<qgssize>( mLayerWidth ) * mLayerHeight;
  QgsProcessingMultiStepFeedback multiStepFeedback( 2, feedback );

  // pass 1: D8 flow direction calculation (matching SAGA CD8_Flow_Analysis::Get_Direction)
  multiStepFeedback.setCurrentStep( 0 );
  std::vector<int8_t> d8Directions( totalCells, -1 );
  for ( int row = 0; row < mLayerHeight; ++row )
  {
    if ( multiStepFeedback.isCanceled() )
      return {};

    multiStepFeedback.setProgress( 100.0 * static_cast<double>( row ) / mLayerHeight );
    const qgssize rowOffset = static_cast<qgssize>( row ) * mLayerWidth;
    for ( int col = 0; col < mLayerWidth; ++col )
    {
      const int dir = QgsRasterAnalysisUtils::steepestGradientDirection( demBlock.get(), row, col, mCellSizeX, mCellSizeY, true, true );
      d8Directions[rowOffset + col] = static_cast<int8_t>( dir );
    }
  }

  // pass 2: Strahler stream order calculation
  multiStepFeedback.setCurrentStep( 1 );
  auto outputBlock = std::make_unique<QgsRasterBlock>( Qgis::DataType::Int16, mLayerWidth, mLayerHeight );
  outputBlock->setNoDataValue( mOutputNoData );

  int16_t *outOrder = reinterpret_cast<int16_t *>( outputBlock->bits() );
  computeStrahlerOrder( demBlock.get(), d8Directions, mLayerWidth, mLayerHeight, threshold, outOrder, &multiStepFeedback, mOutputNoData );

  if ( multiStepFeedback.isCanceled() )
    return {};

  return writeOutputRaster( outputBlock.get(), outputFile, outputFormat, creationOptions );
}

//
// QgsStrahlerOrderFromFlowDirectionAlgorithm
//

QString QgsStrahlerOrderFromFlowDirectionAlgorithm::name() const
{
  return u"strahlerorderfromflowdirection"_s;
}

QString QgsStrahlerOrderFromFlowDirectionAlgorithm::displayName() const
{
  return QObject::tr( "Strahler order from DEM and flow direction" );
}

QString QgsStrahlerOrderFromFlowDirectionAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates Strahler stream order using an elevation raster (DEM) and a D8 flow direction raster." );
}

QString QgsStrahlerOrderFromFlowDirectionAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm calculates Strahler stream order from an input elevation raster (DEM) and a pre-computed D8 flow direction raster.\n\n"
    "Confluences of two stream channels of order N produce a downstream channel of order N + 1.\n"
    "When the threshold is set to 1, raw stream orders (1, 2, 3...) are calculated. "
    "Higher threshold values mask non-stream cells as NoData and offset stream orders.\n\n"
    "This algorithm is a port of the Strahler stream order calculation from SAGA 'Channel Network and Drainage Basins' tool."
  );
}

void QgsStrahlerOrderFromFlowDirectionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Elevation raster" ) ) );
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT_FLOW_DIRECTION"_s, QObject::tr( "Flow direction raster" ) ) );
  addCommonParameters();
}

QgsProcessingAlgorithm *QgsStrahlerOrderFromFlowDirectionAlgorithm::createInstance() const
{
  return new QgsStrahlerOrderFromFlowDirectionAlgorithm();
}

bool QgsStrahlerOrderFromFlowDirectionAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *demLayer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  if ( !demLayer || !demLayer->dataProvider() )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  QgsRasterLayer *flowDirLayer = parameterAsRasterLayer( parameters, u"INPUT_FLOW_DIRECTION"_s, context );
  if ( !flowDirLayer || !flowDirLayer->dataProvider() )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT_FLOW_DIRECTION"_s ) );

  mDemInterface.reset( demLayer->dataProvider()->clone() );
  mFlowDirInterface.reset( flowDirLayer->dataProvider()->clone() );
  mLayerWidth = demLayer->width();
  mLayerHeight = demLayer->height();
  mExtent = demLayer->extent();
  mCrs = demLayer->crs();

  return true;
}

QVariantMap QgsStrahlerOrderFromFlowDirectionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  const int threshold = parameterAsInt( parameters, u"THRESHOLD"_s, context );
  const QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );
  mOutputNoData = parameterAsInt( parameters, u"NODATA"_s, context );

  std::unique_ptr<QgsRasterBlock> demBlock( mDemInterface->block( 1, mExtent, mLayerWidth, mLayerHeight ) );
  if ( !demBlock )
    throw QgsProcessingException( QObject::tr( "Could not read input DEM block." ) );

  std::unique_ptr<QgsRasterBlock> flowDirBlock( mFlowDirInterface->block( 1, mExtent, mLayerWidth, mLayerHeight ) );
  if ( !flowDirBlock )
    throw QgsProcessingException( QObject::tr( "Could not read input flow direction block." ) );

  const qgssize totalCells = static_cast<qgssize>( mLayerWidth ) * mLayerHeight;

  std::vector<int8_t> d8Directions( totalCells, -1 );
  for ( int row = 0; row < mLayerHeight; ++row )
  {
    if ( feedback->isCanceled() )
      return {};

    bool isNoData = false;
    const qgssize rowOffset = static_cast<qgssize>( row ) * mLayerWidth;
    for ( int col = 0; col < mLayerWidth; ++col )
    {
      const double dir = flowDirBlock->valueAndNoData( row, col, isNoData );
      if ( !isNoData )
      {
        d8Directions[rowOffset + col] = static_cast<int8_t>( dir );
      }
    }
  }

  auto outputBlock = std::make_unique<QgsRasterBlock>( Qgis::DataType::Int16, mLayerWidth, mLayerHeight );
  outputBlock->setNoDataValue( mOutputNoData );

  int16_t *outOrder = reinterpret_cast<int16_t *>( outputBlock->bits() );
  computeStrahlerOrder( demBlock.get(), d8Directions, mLayerWidth, mLayerHeight, threshold, outOrder, feedback, mOutputNoData );

  if ( feedback->isCanceled() )
    return {};

  return writeOutputRaster( outputBlock.get(), outputFile, outputFormat, creationOptions );
}

///@endcond
