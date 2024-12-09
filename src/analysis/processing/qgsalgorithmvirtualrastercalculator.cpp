/***************************************************************************
                         qgsalgorithmvirtualrastercalculator.cpp
                         ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmvirtualrastercalculator.h"
#include "qgsrasterdataprovider.h"

///@cond PRIVATE

Qgis::ProcessingAlgorithmFlags QgsVirtualRasterCalculatorAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::HideFromModeler;
}

QString QgsVirtualRasterCalculatorAlgorithm::name() const
{
  return QStringLiteral( "virtualrastercalc" );
}

QString QgsVirtualRasterCalculatorAlgorithm::displayName() const
{
  return QObject::tr( "Raster calculator (virtual)" );
}

QStringList QgsVirtualRasterCalculatorAlgorithm::tags() const
{
  return QObject::tr( "raster,calculator,virtual" ).split( ',' );
}

QString QgsVirtualRasterCalculatorAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsVirtualRasterCalculatorAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

QString QgsVirtualRasterCalculatorAlgorithm::shortHelpString() const
{
  return QObject::tr( "Performs algebraic operations using raster layers and generates in-memory result." );
}

QgsVirtualRasterCalculatorAlgorithm *QgsVirtualRasterCalculatorAlgorithm::createInstance() const
{
  return new QgsVirtualRasterCalculatorAlgorithm();
}

void QgsVirtualRasterCalculatorAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::Raster ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "EXPRESSION" ), QObject::tr( "Expression" ), QVariant(), QStringLiteral( "LAYERS" ), false, Qgis::ExpressionType::RasterCalculator ) );
  std::unique_ptr<QgsProcessingParameterExtent> extentParam = std::make_unique<QgsProcessingParameterExtent>( QStringLiteral( "EXTENT" ), QObject::tr( "Output extent" ), QVariant(), true );
  extentParam->setHelp( QObject::tr( "Extent of the output layer. If not specified, the extent will be the overall extent of all input layers" ) );
  addParameter( extentParam.release() );
  std::unique_ptr<QgsProcessingParameterNumber> cellSizeParam = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "CELL_SIZE" ), QObject::tr( "Output cell size (leave empty to set automatically)" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0.0 );
  cellSizeParam->setHelp( QObject::tr( "Cell size of the output layer. If not specified, the smallest cell size from the input layers will be used" ) );
  addParameter( cellSizeParam.release() );
  std::unique_ptr<QgsProcessingParameterCrs> crsParam = std::make_unique<QgsProcessingParameterCrs>( QStringLiteral( "CRS" ), QObject::tr( "Output CRS" ), QVariant(), true );
  crsParam->setHelp( QObject::tr( "CRS of the output layer. If not specified, the CRS of the first input layer will be used" ) );
  addParameter( crsParam.release() );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "LAYER_NAME" ), QObject::tr( "Output layer name" ), QVariant(), false, true ) );
  addOutput( new QgsProcessingOutputRasterLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Calculated" ) ) );
}

QVariantMap QgsVirtualRasterCalculatorAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context );
  if ( layers.isEmpty() )
  {
    throw QgsProcessingException( QObject::tr( "No input layers selected" ) );
  }

  QgsCoordinateReferenceSystem crs;
  if ( parameters.value( QStringLiteral( "CRS" ) ).isValid() )
  {
    crs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );
  }
  else
  {
    crs = layers.at( 0 )->crs();
  }

  QgsRectangle bbox;
  if ( parameters.value( QStringLiteral( "EXTENT" ) ).isValid() )
  {
    bbox = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, crs );
  }
  else
  {
    bbox = QgsProcessingUtils::combineLayerExtents( layers, crs, context );
  }

  double minCellSize = 1e9;
  QgsRasterDataProvider::VirtualRasterParameters rasterParameters;

  for ( const QgsMapLayer *layer : layers )
  {
    const QgsRasterLayer *rLayer = qobject_cast<const QgsRasterLayer *>( layer );
    if ( !rLayer )
    {
      continue;
    }

    QgsRasterDataProvider::VirtualRasterInputLayers rasterLayer;
    rasterLayer.name = rLayer->name();
    rasterLayer.provider = rLayer->dataProvider()->name();
    rasterLayer.uri = rLayer->source();
    rasterParameters.rInputLayers.append( rasterLayer );

    QgsRectangle ext = rLayer->extent();
    if ( rLayer->crs() != crs )
    {
      QgsCoordinateTransform ct( rLayer->crs(), crs, context.transformContext() );
      ext = ct.transformBoundingBox( ext );
    }

    double cellSize = ( ext.xMaximum() - ext.xMinimum() ) / rLayer->width();
    if ( cellSize < minCellSize )
    {
      minCellSize = cellSize;
    }
  }

  double cellSize = parameterAsDouble( parameters, QStringLiteral( "CELL_SIZE" ), context );
  if ( cellSize == 0 )
  {
    cellSize = minCellSize;
  }

  const QString expression = parameterAsExpression( parameters, QStringLiteral( "EXPRESSION" ), context );
  QString layerName = parameterAsString( parameters, QStringLiteral( "LAYER_NAME" ), context );
  if ( layerName.isEmpty() )
  {
    layerName = expression;
  }

  double width = std::round( ( bbox.xMaximum() - bbox.xMinimum() ) / cellSize );
  double height = std::round( ( bbox.yMaximum() - bbox.yMinimum() ) / cellSize );

  rasterParameters.crs = crs;
  rasterParameters.extent = bbox;
  rasterParameters.width = width;
  rasterParameters.height = height;
  rasterParameters.formula = expression;

  std::unique_ptr<QgsRasterLayer> layer;
  layer = std::make_unique<QgsRasterLayer>( QgsRasterDataProvider::encodeVirtualRasterProviderUri( rasterParameters ), layerName, QStringLiteral( "virtualraster" ) );
  if ( !layer->isValid() )
  {
    feedback->reportError( QObject::tr( "Failed to create virtual raster layer" ) );
  }
  else
  {
  }
  const QString layerId = layer->id();
  const QgsProcessingContext::LayerDetails details( layer->name(), context.project(), QStringLiteral( "OUTPUT" ), QgsProcessingUtils::LayerHint::Raster );
  context.addLayerToLoadOnCompletion( layerId, details );
  context.temporaryLayerStore()->addMapLayer( layer.release() );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), layerId );
  return outputs;
}

Qgis::ProcessingAlgorithmFlags QgsVirtualRasterCalculatorModelerAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::HideFromToolbox;
}

QString QgsVirtualRasterCalculatorModelerAlgorithm::name() const
{
  return QStringLiteral( "modelervirtualrastercalc" );
}

QString QgsVirtualRasterCalculatorModelerAlgorithm::displayName() const
{
  return QObject::tr( "Raster calculator (virtual)" );
}

QStringList QgsVirtualRasterCalculatorModelerAlgorithm::tags() const
{
  return QObject::tr( "raster,calculator,virtual" ).split( ',' );
}

QString QgsVirtualRasterCalculatorModelerAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsVirtualRasterCalculatorModelerAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

QgsVirtualRasterCalculatorModelerAlgorithm *QgsVirtualRasterCalculatorModelerAlgorithm::createInstance() const
{
  return new QgsVirtualRasterCalculatorModelerAlgorithm();
}

QVariantMap QgsVirtualRasterCalculatorModelerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, QStringLiteral( "INPUT" ), context );
  if ( layers.isEmpty() )
  {
    throw QgsProcessingException( QObject::tr( "No input layers selected" ) );
  }

  QgsCoordinateReferenceSystem crs;
  if ( parameters.value( QStringLiteral( "CRS" ) ).isValid() )
  {
    crs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );
  }
  else
  {
    crs = layers.at( 0 )->crs();
  }

  QgsRectangle bbox;
  if ( parameters.value( QStringLiteral( "EXTENT" ) ).isValid() )
  {
    bbox = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, crs );
  }
  else
  {
    bbox = QgsProcessingUtils::combineLayerExtents( layers, crs, context );
  }

  double minCellSize = 1e9;
  QgsRasterDataProvider::VirtualRasterParameters rasterParameters;

  int n = 0;
  for ( const QgsMapLayer *layer : layers )
  {
    const QgsRasterLayer *rLayer = qobject_cast<const QgsRasterLayer *>( layer );
    if ( !rLayer )
    {
      continue;
    }

    n++;
    QgsRasterDataProvider::VirtualRasterInputLayers rasterLayer;
    rasterLayer.name = indexToName( n );
    rasterLayer.provider = rLayer->dataProvider()->name();
    rasterLayer.uri = rLayer->source();
    rasterParameters.rInputLayers.append( rasterLayer );

    QgsRectangle ext = rLayer->extent();
    if ( rLayer->crs() != crs )
    {
      QgsCoordinateTransform ct( rLayer->crs(), crs, context.transformContext() );
      ext = ct.transformBoundingBox( ext );
    }

    double cellSize = ( ext.xMaximum() - ext.xMinimum() ) / rLayer->width();
    if ( cellSize < minCellSize )
    {
      minCellSize = cellSize;
    }
  }

  double cellSize = parameterAsDouble( parameters, QStringLiteral( "CELL_SIZE" ), context );
  if ( cellSize == 0 )
  {
    cellSize = minCellSize;
  }

  const QString expression = parameterAsExpression( parameters, QStringLiteral( "EXPRESSION" ), context );
  QString layerName = parameterAsString( parameters, QStringLiteral( "LAYER_NAME" ), context );
  if ( layerName.isEmpty() )
  {
    layerName = expression;
  }

  double width = std::round( ( bbox.xMaximum() - bbox.xMinimum() ) / cellSize );
  double height = std::round( ( bbox.yMaximum() - bbox.yMinimum() ) / cellSize );

  rasterParameters.crs = crs;
  rasterParameters.extent = bbox;
  rasterParameters.width = width;
  rasterParameters.height = height;
  rasterParameters.formula = expression;

  std::unique_ptr<QgsRasterLayer> layer;
  layer = std::make_unique<QgsRasterLayer>( QgsRasterDataProvider::encodeVirtualRasterProviderUri( rasterParameters ), layerName, QStringLiteral( "virtualraster" ) );
  if ( !layer->isValid() )
  {
    feedback->reportError( QObject::tr( "Failed to create virtual raster layer" ) );
  }
  else
  {
  }
  const QString layerId = layer->id();
  const QgsProcessingContext::LayerDetails details( layer->name(), context.project(), QStringLiteral( "OUTPUT" ), QgsProcessingUtils::LayerHint::Raster );
  context.addLayerToLoadOnCompletion( layerId, details );
  context.temporaryLayerStore()->addMapLayer( layer.release() );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), layerId );
  return outputs;
}

QString QgsVirtualRasterCalculatorModelerAlgorithm::indexToName( int index ) const
{
  QString name;
  int div = index;
  int mod = 0;

  while ( div > 0 )
  {
    mod = ( div - 1 ) % 26;
    name = static_cast<char>( 65 + mod ) + name;
    div = ( int ) ( ( div - mod ) / 26 );
  }
  return name;
}

///@endcond
