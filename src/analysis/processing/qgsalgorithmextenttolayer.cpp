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
  return QStringLiteral( "extenttolayer" );
}

QgsProcessingAlgorithm::Flags QgsExtentToLayerAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsExtentToLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "INPUT" ), QObject::tr( "Extent" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Extent" ), QgsProcessing::TypeVectorPolygon ) );
}

QString QgsExtentToLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that contains a single feature with geometry matching an extent parameter.\n\n"
                      "It can be used in models to convert an extent into a layer which can be used for other algorithms which require "
                      "a layer based input." );
}

QgsExtentToLayerAlgorithm *QgsExtentToLayerAlgorithm::createInstance() const
{
  return new QgsExtentToLayerAlgorithm();
}

QVariantMap QgsExtentToLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsCoordinateReferenceSystem crs = parameterAsExtentCrs( parameters, QStringLiteral( "INPUT" ), context );
  QgsGeometry geom = parameterAsExtentGeometry( parameters, QStringLiteral( "INPUT" ), context );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "id" ), QVariant::Int ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::Polygon, crs ) );
  if ( !sink )
    return QVariantMap();

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( geom );
  sink->addFeature( f, QgsFeatureSink::FastInsert );

  feedback->setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond

