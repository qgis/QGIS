/***************************************************************************
                         qgsalgorithmpointtolayer.cpp
                         ---------------------
    begin                : May 2019
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

#include "qgsalgorithmpointtolayer.h"

///@cond PRIVATE

QString QgsPointToLayerAlgorithm::name() const
{
  return u"pointtolayer"_s;
}

void QgsPointToLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPoint( u"INPUT"_s, QObject::tr( "Point" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Point" ), Qgis::ProcessingSourceType::VectorPoint ) );
}

QString QgsPointToLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that contains a single feature with geometry matching a point parameter.\n\n"
                      "It can be used in models to convert a point into a layer which can be used for other algorithms which require "
                      "a layer based input." );
}

QString QgsPointToLayerAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a vector layer that contains a single feature with geometry matching a point parameter." );
}

QgsPointToLayerAlgorithm *QgsPointToLayerAlgorithm::createInstance() const
{
  return new QgsPointToLayerAlgorithm();
}

QVariantMap QgsPointToLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QgsCoordinateReferenceSystem crs = parameterAsPointCrs( parameters, u"INPUT"_s, context );
  const QgsGeometry geom = QgsGeometry::fromPointXY( parameterAsPoint( parameters, u"INPUT"_s, context ) );

  QgsFields fields;
  fields.append( QgsField( u"id"_s, QMetaType::Type::Int ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::Point, crs ) );
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
