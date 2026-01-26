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
  return u"extendlines"_s;
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
  return u"vectorgeometry"_s;
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
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine );
}

Qgis::ProcessingSourceType QgsExtendLinesAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorLine;
}

QgsExtendLinesAlgorithm *QgsExtendLinesAlgorithm::createInstance() const
{
  return new QgsExtendLinesAlgorithm();
}

void QgsExtendLinesAlgorithm::initParameters( const QVariantMap & )
{
  auto startDistance = std::make_unique<QgsProcessingParameterDistance>( u"START_DISTANCE"_s, QObject::tr( "Start distance" ), 0.0, u"INPUT"_s, false, 0 );
  startDistance->setIsDynamic( true );
  startDistance->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Start Distance"_s, QObject::tr( "Start distance" ), QgsPropertyDefinition::DoublePositive ) );
  startDistance->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( startDistance.release() );

  auto endDistance = std::make_unique<QgsProcessingParameterDistance>( u"END_DISTANCE"_s, QObject::tr( "End distance" ), 0.0, u"INPUT"_s, false, 0 );
  endDistance->setIsDynamic( true );
  endDistance->setDynamicPropertyDefinition( QgsPropertyDefinition( u"End Distance"_s, QObject::tr( "End distance" ), QgsPropertyDefinition::DoublePositive ) );
  endDistance->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( endDistance.release() );
}

Qgis::ProcessingFeatureSourceFlags QgsExtendLinesAlgorithm::sourceFlags() const
{
  // skip geometry checks - this algorithm doesn't care about invalid geometries
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

bool QgsExtendLinesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mStartDistance = parameterAsDouble( parameters, u"START_DISTANCE"_s, context );
  mDynamicStartDistance = QgsProcessingParameters::isDynamic( parameters, u"START_DISTANCE"_s );
  if ( mDynamicStartDistance )
    mStartDistanceProperty = parameters.value( u"START_DISTANCE"_s ).value<QgsProperty>();

  mEndDistance = parameterAsDouble( parameters, u"END_DISTANCE"_s, context );
  mDynamicEndDistance = QgsProcessingParameters::isDynamic( parameters, u"END_DISTANCE"_s );
  if ( mDynamicEndDistance )
    mEndDistanceProperty = parameters.value( u"END_DISTANCE"_s ).value<QgsProperty>();

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
