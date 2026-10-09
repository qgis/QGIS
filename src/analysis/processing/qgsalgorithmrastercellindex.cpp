/***************************************************************************
                         qgsalgorithmrastercellindex.cpp
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

#include "qgsalgorithmrastercellindex.h"

#include "qgsrasterfilewriter.h"
#include "qgssortedrasterblockindex.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsRasterCellIndexAlgorithm::name() const
{
  return u"rastercellindex"_s;
}

QString QgsRasterCellIndexAlgorithm::displayName() const
{
  return QObject::tr( "Raster grid cell index" );
}

QStringList QgsRasterCellIndexAlgorithm::tags() const
{
  return QObject::tr( "grid,cell,index,sort,rank,order,ascending,descending" ).split( ',' );
}

QString QgsRasterCellIndexAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterCellIndexAlgorithm::groupId() const
{
  return u"rasteranalysis"_s;
}

QString QgsRasterCellIndexAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm ranks valid non-NoData raster grid cells according to their value, "
    "outputting a new grid where each cell contains its 0-based sorted index (rank).\n\n"
    "NoData cells in the input layer are preserved as NoData in the output layer.\n\n"
    "This algorithm is a port of the SAGA 'Grid Cell Index' tool."
  );
}

QString QgsRasterCellIndexAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates an index raster according to the cell values in either ascending or descending order." );
}

void QgsRasterCellIndexAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterBand( u"BAND"_s, QObject::tr( "Band number" ), 1, u"INPUT"_s ) );

  const QStringList orders = { QObject::tr( "Ascending" ), QObject::tr( "Descending" ) };
  auto orderParam = std::make_unique<QgsProcessingParameterEnum>( u"ORDER"_s, QObject::tr( "Sort order" ), orders, false, 0 );
  orderParam->setHelp( QObject::tr( "Sort order: Ascending (assigns 0 to the lowest value) or Descending (assigns 0 to the highest value)." ) );
  addParameter( orderParam.release() );

  auto outputNodataParam = std::make_unique<QgsProcessingParameterNumber>( u"NODATA"_s, QObject::tr( "Output NoData value" ), Qgis::ProcessingNumberParameterType::Integer, -9999 );
  outputNodataParam->setHelp( QObject::tr( "The NODATA value to use in the output raster." ) );
  outputNodataParam->setFlags( outputNodataParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( outputNodataParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATION_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setHelp( QObject::tr( "The raster creation options for the output raster. These options control things like colorimetry, compression, etc." ) );
  creationOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  auto outputParam = std::make_unique<QgsProcessingParameterRasterDestination>( u"OUTPUT"_s, QObject::tr( "Output layer" ) );
  addParameter( outputParam.release() );
}

QgsRasterCellIndexAlgorithm *QgsRasterCellIndexAlgorithm::createInstance() const
{
  return new QgsRasterCellIndexAlgorithm();
}

bool QgsRasterCellIndexAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  mBand = parameterAsInt( parameters, u"BAND"_s, context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( layer->bandCount() ) );

  mInterface.reset( layer->dataProvider()->clone() );
  mLayerWidth = layer->width();
  mLayerHeight = layer->height();
  mExtent = layer->extent();
  mCrs = layer->crs();

  return true;
}

QVariantMap QgsRasterCellIndexAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  const Qt::SortOrder sortOrder = ( parameterAsInt( parameters, u"ORDER"_s, context ) == 0 ) ? Qt::AscendingOrder : Qt::DescendingOrder;

  const QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  const int outputNodata = parameterAsInt( parameters, u"NODATA"_s, context );

  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  std::unique_ptr<QgsRasterBlock> inputBlock( mInterface->block( 1, mExtent, mLayerWidth, mLayerHeight ) );
  if ( !inputBlock )
    throw QgsProcessingException( QObject::tr( "Could not read input raster block." ) );

  const QgsSortedRasterBlockIndex sortedIndex( inputBlock.get() );
  const qgssize count = sortedIndex.sortedCount();

  const qgssize totalCells = static_cast<qgssize>( mLayerWidth ) * mLayerHeight;
  auto outputBlock = std::make_unique< QgsRasterBlock >( Qgis::DataType::Int32, mLayerWidth, mLayerHeight );
  outputBlock->setNoDataValue( outputNodata );

  int32_t *outputData = reinterpret_cast<int32_t *>( outputBlock->bits() );
  std::fill_n( outputData, totalCells, static_cast<int32_t>( outputNodata ) );

  for ( qgssize rank = 0; rank < count; ++rank )
  {
    if ( feedback->isCanceled() )
      return {};

    feedback->setProgress( 100.0 * static_cast<double>( rank ) / count );

    const qgssize outputBlockIndex = sortedIndex.sortedIndex( rank, sortOrder );
    outputData[outputBlockIndex] = static_cast<int32_t>( rank );
  }

  auto outputWriter = std::make_unique<QgsRasterFileWriter>( outputFile );
  outputWriter->setOutputProviderKey( u"gdal"_s );
  if ( !creationOptions.isEmpty() )
  {
    outputWriter->setCreationOptions( creationOptions.split( '|' ) );
  }
  outputWriter->setOutputFormat( outputFormat );

  std::unique_ptr<QgsRasterDataProvider> destProvider( outputWriter->createOneBandRaster( Qgis::DataType::Int32, mLayerWidth, mLayerHeight, mExtent, mCrs ) );
  if ( !destProvider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !destProvider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, destProvider->error().message( QgsErrorMessage::Text ) ) );

  destProvider->setNoDataValue( 1, outputNodata );
  destProvider->setEditable( true );
  if ( !destProvider->writeBlock( outputBlock.get(), 1 ) )
  {
    throw QgsProcessingException( QObject::tr( "Could not write raster block: %1" ).arg( destProvider->error().summary() ) );
  }
  destProvider->setEditable( false );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}


///@endcond
