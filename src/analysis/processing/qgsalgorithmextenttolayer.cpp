/***************************************************************************
                         qgsalgorithmextenttolayer.cpp
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmextenttolayer.h"

///@cond PRIVATE

QString QgsExtentToLayerAlgorithm::name() const
{
  return u"extenttolayer"_s;
}

void QgsExtentToLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterExtent( u"INPUT"_s, QObject::tr( "Extent" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Extent" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

QString QgsExtentToLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that contains a single feature with geometry matching an extent parameter.\n\n"
                      "It can be used in models to convert an extent into a layer which can be used for other algorithms which require "
                      "a layer based input." );
}

QString QgsExtentToLayerAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a vector layer that contains a single feature with geometry matching an extent parameter." );
}

QgsExtentToLayerAlgorithm *QgsExtentToLayerAlgorithm::createInstance() const
{
  return new QgsExtentToLayerAlgorithm();
}

QVariantMap QgsExtentToLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QgsCoordinateReferenceSystem crs = parameterAsExtentCrs( parameters, u"INPUT"_s, context );
  const QgsGeometry geom = parameterAsExtentGeometry( parameters, u"INPUT"_s, context );

  QgsFields fields;
  fields.append( QgsField( u"id"_s, QMetaType::Type::Int ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::Polygon, crs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( geom );
  if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
    throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );

  sink->finalize();
  feedback->setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
