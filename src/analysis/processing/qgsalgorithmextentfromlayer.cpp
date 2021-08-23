/***************************************************************************
                         qgsalgorithmextentfromlayer.cpp
                         ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Alexander Bruy
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

#include "qgsalgorithmextentfromlayer.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsExtentFromLayerAlgorithm::name() const
{
  return QStringLiteral( "polygonfromlayerextent" );
}

QString QgsExtentFromLayerAlgorithm::displayName() const
{
  return QObject::tr( "Extract layer extent" );
}

QStringList QgsExtentFromLayerAlgorithm::tags() const
{
  return QObject::tr( "polygon,vector,raster,extent,envelope,bounds,bounding,boundary,layer,round,rounded" ).split( ',' );
}

QString QgsExtentFromLayerAlgorithm::group() const
{
  return QObject::tr( "Layer tools" );
}

QString QgsExtentFromLayerAlgorithm::groupId() const
{
  return QStringLiteral( "layertools" );
}

QString QgsExtentFromLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a map layer and generates a new vector "
                      "layer with the minimum bounding box (rectangle polygon with "
                      "N-S orientation) that covers the input layer. Optionally, the "
                      "extent can be enlarged to a rounded value." );
}

QString QgsExtentFromLayerAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmExtractLayerExtent.svg" ) );
}

QIcon QgsExtentFromLayerAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmExtractLayerExtent.svg" ) );
}

QgsExtentFromLayerAlgorithm *QgsExtentFromLayerAlgorithm::createInstance() const
{
  return new QgsExtentFromLayerAlgorithm();
}

void QgsExtentFromLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  auto roundParam = std::make_unique < QgsProcessingParameterDistance >( QStringLiteral( "ROUND_TO" ), QObject::tr( "Round values to" ), 0, QStringLiteral( "INPUT" ), 0 );
  roundParam->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( roundParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Extent" ), QgsProcessing::TypeVectorPolygon ) );
}

QVariantMap QgsExtentFromLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  const double roundTo = parameterAsDouble( parameters, QStringLiteral( "ROUND_TO" ), context );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "MINX" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "MINY" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "MAXX" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "MAXY" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "CNTX" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "CNTY" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "AREA" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "PERIM" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "HEIGHT" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "WIDTH" ), QVariant::Double ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::Polygon, layer->crs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( layer ) )
  {
    vl->updateExtents();
  }

  QgsRectangle rect = layer->extent();

  if ( roundTo > 0 )
  {
    rect.setXMinimum( std::floor( rect.xMinimum() / roundTo ) * roundTo );
    rect.setYMinimum( std::floor( rect.yMinimum() / roundTo ) * roundTo );
    rect.setXMaximum( std::ceil( rect.xMaximum() / roundTo ) * roundTo );
    rect.setYMaximum( std::ceil( rect.yMaximum() / roundTo ) * roundTo );
  }

  const QgsGeometry geom = QgsGeometry::fromRect( rect );

  const double minX = rect.xMinimum();
  const double maxX = rect.xMaximum();
  const double minY = rect.yMinimum();
  const double maxY = rect.yMaximum();
  const double height = rect.height();
  const double width = rect.width();
  const double cntX = minX + width / 2.0;
  const double cntY = minY + height / 2.0;
  const double area = width * height;
  const double perim = 2 * width + 2 * height;

  QgsFeature feat;
  feat.setGeometry( geom );
  feat.setAttributes( QgsAttributes() << minX << minY << maxX << maxY << cntX << cntY << area << perim << height << width );
  if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
    throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
