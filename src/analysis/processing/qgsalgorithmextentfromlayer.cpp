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
  return u"polygonfromlayerextent"_s;
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
  return u"layertools"_s;
}

QString QgsExtentFromLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a map layer and generates a new vector "
                      "layer with the minimum bounding box (rectangle polygon with "
                      "N-S orientation) that covers the input layer. Optionally, the "
                      "extent can be enlarged to a rounded value." );
}

QString QgsExtentFromLayerAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a vector layer with the minimum bounding box that covers the input layer." );
}

QString QgsExtentFromLayerAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( u"/algorithms/mAlgorithmExtractLayerExtent.svg"_s );
}

QIcon QgsExtentFromLayerAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmExtractLayerExtent.svg"_s );
}

QgsExtentFromLayerAlgorithm *QgsExtentFromLayerAlgorithm::createInstance() const
{
  return new QgsExtentFromLayerAlgorithm();
}

void QgsExtentFromLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );

  auto roundParam = std::make_unique<QgsProcessingParameterDistance>( u"ROUND_TO"_s, QObject::tr( "Round values to" ), 0, u"INPUT"_s, 0 );
  roundParam->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( roundParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Extent" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

QVariantMap QgsExtentFromLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, u"INPUT"_s, context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  const double roundTo = parameterAsDouble( parameters, u"ROUND_TO"_s, context );

  QgsFields fields;
  fields.append( QgsField( u"MINX"_s, QMetaType::Type::Double ) );
  fields.append( QgsField( u"MINY"_s, QMetaType::Type::Double ) );
  fields.append( QgsField( u"MAXX"_s, QMetaType::Type::Double ) );
  fields.append( QgsField( u"MAXY"_s, QMetaType::Type::Double ) );
  fields.append( QgsField( u"CNTX"_s, QMetaType::Type::Double ) );
  fields.append( QgsField( u"CNTY"_s, QMetaType::Type::Double ) );
  fields.append( QgsField( u"AREA"_s, QMetaType::Type::Double ) );
  fields.append( QgsField( u"PERIM"_s, QMetaType::Type::Double ) );
  fields.append( QgsField( u"HEIGHT"_s, QMetaType::Type::Double ) );
  fields.append( QgsField( u"WIDTH"_s, QMetaType::Type::Double ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::Polygon, layer->crs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer ) )
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
    throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
