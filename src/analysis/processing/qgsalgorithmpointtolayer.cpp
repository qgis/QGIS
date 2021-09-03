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
  return QStringLiteral( "pointtolayer" );
}

void QgsPointToLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPoint( QStringLiteral( "INPUT" ), QObject::tr( "Point" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Point" ), QgsProcessing::TypeVectorPoint ) );
}

QString QgsPointToLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that contains a single feature with geometry matching a point parameter.\n\n"
                      "It can be used in models to convert a point into a layer which can be used for other algorithms which require "
                      "a layer based input." );
}

QgsPointToLayerAlgorithm *QgsPointToLayerAlgorithm::createInstance() const
{
  return new QgsPointToLayerAlgorithm();
}

QVariantMap QgsPointToLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QgsCoordinateReferenceSystem crs = parameterAsPointCrs( parameters, QStringLiteral( "INPUT" ), context );
  const QgsGeometry geom = QgsGeometry::fromPointXY( parameterAsPoint( parameters, QStringLiteral( "INPUT" ), context ) );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "id" ), QVariant::Int ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::Point, crs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( geom );
  if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
    throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

  feedback->setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond

