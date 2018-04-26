/***************************************************************************
  qgsalgorithmunion.cpp
  ---------------------
  Date                 : April 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmunion.h"

#include "qgsoverlayutils.h"

///@cond PRIVATE


QString QgsUnionAlgorithm::name() const
{
  return QStringLiteral( "union" );
}

QString QgsUnionAlgorithm::displayName() const
{
  return QObject::tr( "Union" );
}

QString QgsUnionAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QString QgsUnionAlgorithm::groupId() const
{
  return QStringLiteral( "vectoroverlay" );
}

QString QgsUnionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a layer containing all the features from both input layers. In the case of polygon layers, separate features are created for overlapping and non-overlapping features. The attribute table of the union layer contains attribute values from the respective input layer for non-overlapping features, and attribute values from both input layers for overlapping features." );
}

QgsProcessingAlgorithm *QgsUnionAlgorithm::createInstance() const
{
  return new QgsUnionAlgorithm();
}

void QgsUnionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "OVERLAY" ), QObject::tr( "Union layer" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Union" ) ) );
}


QVariantMap QgsUnionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > sourceA( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !sourceA )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for INPUT" ) );

  std::unique_ptr< QgsFeatureSource > sourceB( parameterAsSource( parameters, QStringLiteral( "OVERLAY" ), context ) );
  if ( !sourceB )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for OVERLAY" ) );

  QgsWkbTypes::Type geomType = QgsWkbTypes::multiType( sourceA->wkbType() );

  QgsFields fields = QgsProcessingUtils::combineFields( sourceA->fields(), sourceB->fields() );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, geomType, sourceA->sourceCrs() ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );

  QList<int> fieldIndicesA = QgsProcessingUtils::fieldNamesToIndices( QStringList(), sourceA->fields() );
  QList<int> fieldIndicesB = QgsProcessingUtils::fieldNamesToIndices( QStringList(), sourceB->fields() );

  int count = 0;
  int total = sourceA->featureCount() * 2 + sourceB->featureCount();

  QgsOverlayUtils::intersection( *sourceA.get(), *sourceB.get(), *sink.get(), context, feedback, count, total, fieldIndicesA, fieldIndicesB );

  QgsOverlayUtils::difference( *sourceA.get(), *sourceB.get(), *sink.get(), context, feedback, count, total, QgsOverlayUtils::OutputAB );

  QgsOverlayUtils::difference( *sourceB.get(), *sourceA.get(), *sink.get(), context, feedback, count, total, QgsOverlayUtils::OutputBA );

  return outputs;
}

///@endcond
