/***************************************************************************
                         qgsalgorithmextendlines.cpp
                         ---------------------
    begin                : July 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsalgorithmextendlines.h"

///@cond PRIVATE

QString QgsExtendLinesAlgorithm::name() const
{
  return QStringLiteral( "extendlines" );
}

QString QgsExtendLinesAlgorithm::displayName() const
{
  return QObject::tr( "Extend lines" );
}

QStringList QgsExtendLinesAlgorithm::tags() const
{
  return QObject::tr( "linestring,continue,grow,extrapolate" ).split( ',' );
}

QString QgsExtendLinesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsExtendLinesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsExtendLinesAlgorithm::outputName() const
{
  return QObject::tr( "Extended" );
}

QString QgsExtendLinesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extends line geometries by a specified amount at the start and end "
                      "of the line. Lines are extended using the bearing of the first and last segment "
                      "in the line." );
}

QString QgsExtendLinesAlgorithm::shortDescription() const
{
  return QObject::tr( "Extends LineString geometries by extrapolating the start and end segments." );
}

QList<int> QgsExtendLinesAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine;
}

QgsProcessing::SourceType QgsExtendLinesAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorLine;
}

QgsExtendLinesAlgorithm *QgsExtendLinesAlgorithm::createInstance() const
{
  return new QgsExtendLinesAlgorithm();
}

void QgsExtendLinesAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterDistance> startDistance = qgis::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "START_DISTANCE" ),
      QObject::tr( "Start distance" ), 0.0, QStringLiteral( "INPUT" ), false, 0 );
  startDistance->setIsDynamic( true );
  startDistance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Start Distance" ), QObject::tr( "Start distance" ), QgsPropertyDefinition::DoublePositive ) );
  startDistance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( startDistance.release() );

  std::unique_ptr< QgsProcessingParameterDistance> endDistance = qgis::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "END_DISTANCE" ),
      QObject::tr( "End distance" ), 0.0, QStringLiteral( "INPUT" ), false, 0 );
  endDistance->setIsDynamic( true );
  endDistance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "End Distance" ), QObject::tr( "End distance" ), QgsPropertyDefinition::DoublePositive ) );
  endDistance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( endDistance.release() );
}

QgsProcessingFeatureSource::Flag QgsExtendLinesAlgorithm::sourceFlags() const
{
  // skip geometry checks - this algorithm doesn't care about invalid geometries
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

bool QgsExtendLinesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mStartDistance = parameterAsDouble( parameters, QStringLiteral( "START_DISTANCE" ), context );
  mDynamicStartDistance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "START_DISTANCE" ) );
  if ( mDynamicStartDistance )
    mStartDistanceProperty = parameters.value( QStringLiteral( "START_DISTANCE" ) ).value< QgsProperty >();

  mEndDistance = parameterAsDouble( parameters, QStringLiteral( "END_DISTANCE" ), context );
  mDynamicEndDistance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "END_DISTANCE" ) );
  if ( mDynamicEndDistance )
    mEndDistanceProperty = parameters.value( QStringLiteral( "END_DISTANCE" ) ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsExtendLinesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    const QgsGeometry geometry = f.geometry();
    double startDistance = mStartDistance;
    if ( mDynamicStartDistance )
      startDistance = mStartDistanceProperty.valueAsDouble( context.expressionContext(), startDistance );

    double endDistance = mEndDistance;
    if ( mDynamicEndDistance )
      endDistance = mEndDistanceProperty.valueAsDouble( context.expressionContext(), endDistance );

    const QgsGeometry outGeometry = geometry.extendLine( startDistance, endDistance );
    if ( outGeometry.isNull() )
      throw QgsProcessingException( QObject::tr( "Error calculating extended line" ) ); // don't think this can actually happen!

    f.setGeometry( outGeometry );
  }
  return QgsFeatureList() << f;
}

///@endcond


