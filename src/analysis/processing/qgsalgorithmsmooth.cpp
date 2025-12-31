/***************************************************************************
                         qgsalgorithmsmooth.cpp
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

#include "qgsalgorithmsmooth.h"

///@cond PRIVATE

QString QgsSmoothAlgorithm::name() const
{
  return u"smoothgeometry"_s;
}

QString QgsSmoothAlgorithm::displayName() const
{
  return QObject::tr( "Smooth" );
}

QStringList QgsSmoothAlgorithm::tags() const
{
  return QObject::tr( "smooth,curve,generalize,round,bend,corners" ).split( ',' );
}

QString QgsSmoothAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSmoothAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsSmoothAlgorithm::outputName() const
{
  return QObject::tr( "Smoothed" );
}

QString QgsSmoothAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm smooths the geometries in a line or polygon layer. It creates a new layer with the "
                      "same features as the ones in the input layer, but with geometries containing a higher number of vertices "
                      "and corners in the geometries smoothed out.\n\n"
                      "The iterations parameter dictates how many smoothing iterations will be applied to each "
                      "geometry. A higher number of iterations results in smoother geometries with the cost of "
                      "greater number of nodes in the geometries.\n\n"
                      "The offset parameter controls how \"tightly\" the smoothed geometries follow the original geometries. "
                      "Smaller values results in a tighter fit, and larger values will create a looser fit.\n\n"
                      "The maximum angle parameter can be used to prevent smoothing of "
                      "nodes with large angles. Any node where the angle of the segments to either "
                      "side is larger than this will not be smoothed. For example, setting the maximum "
                      "angle to 90 degrees or lower would preserve right angles in the geometry.\n\n"
                      "If input geometries contain Z or M values, these will also be smoothed and the output "
                      "geometry will retain the same dimensionality as the input geometry." );
}

QString QgsSmoothAlgorithm::shortDescription() const
{
  return QObject::tr( "Smooths the geometries in a line or polygon layer by adding vertices and rounding corners." );
}

QgsSmoothAlgorithm *QgsSmoothAlgorithm::createInstance() const
{
  return new QgsSmoothAlgorithm();
}

QList<int> QgsSmoothAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

void QgsSmoothAlgorithm::initParameters( const QVariantMap & )
{
  auto iterations = std::make_unique<QgsProcessingParameterNumber>( u"ITERATIONS"_s, QObject::tr( "Iterations" ), Qgis::ProcessingNumberParameterType::Integer, 1, false, 1, 10 );
  iterations->setIsDynamic( true );
  iterations->setDynamicPropertyDefinition( QgsPropertyDefinition( u"ITERATIONS"_s, QObject::tr( "Iterations" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) );
  iterations->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( iterations.release() );

  auto offset = std::make_unique<QgsProcessingParameterNumber>( u"OFFSET"_s, QObject::tr( "Offset" ), Qgis::ProcessingNumberParameterType::Double, 0.25, false, 0.0, 0.5 );
  offset->setIsDynamic( true );
  offset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"OFFSET"_s, QObject::tr( "Offset" ), QgsPropertyDefinition::Double0To1 ) );
  offset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( offset.release() );

  auto maxAngle = std::make_unique<QgsProcessingParameterNumber>( u"MAX_ANGLE"_s, QObject::tr( "Maximum node angle to smooth" ), Qgis::ProcessingNumberParameterType::Double, 180.0, false, 0.0, 180.0 );
  maxAngle->setIsDynamic( true );
  maxAngle->setDynamicPropertyDefinition( QgsPropertyDefinition( u"MAX_ANGLE"_s, QObject::tr( "Maximum node angle to smooth" ), QgsPropertyDefinition::Rotation ) );
  maxAngle->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( maxAngle.release() );
}

bool QgsSmoothAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mIterations = parameterAsInt( parameters, u"ITERATIONS"_s, context );
  mDynamicIterations = QgsProcessingParameters::isDynamic( parameters, u"ITERATIONS"_s );
  if ( mDynamicIterations )
    mIterationsProperty = parameters.value( u"ITERATIONS"_s ).value<QgsProperty>();

  mOffset = parameterAsDouble( parameters, u"OFFSET"_s, context );
  mDynamicOffset = QgsProcessingParameters::isDynamic( parameters, u"OFFSET"_s );
  if ( mDynamicOffset )
    mOffsetProperty = parameters.value( u"OFFSET"_s ).value<QgsProperty>();

  mMaxAngle = parameterAsDouble( parameters, u"MAX_ANGLE"_s, context );
  mDynamicMaxAngle = QgsProcessingParameters::isDynamic( parameters, u"MAX_ANGLE"_s );
  if ( mDynamicMaxAngle )
    mMaxAngleProperty = parameters.value( u"MAX_ANGLE"_s ).value<QgsProperty>();

  return true;
}

QgsFeatureList QgsSmoothAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    int iterations = mIterations;
    if ( mDynamicIterations )
      iterations = mIterationsProperty.valueAsInt( context.expressionContext(), iterations );

    double offset = mOffset;
    if ( mDynamicOffset )
      offset = mOffsetProperty.valueAsDouble( context.expressionContext(), offset );

    double maxAngle = mMaxAngle;
    if ( mDynamicMaxAngle )
      maxAngle = mMaxAngleProperty.valueAsDouble( context.expressionContext(), maxAngle );

    const QgsGeometry outputGeometry = f.geometry().smooth( iterations, offset, -1, maxAngle );
    if ( outputGeometry.isNull() )
    {
      feedback->reportError( QObject::tr( "Error smoothing geometry %1" ).arg( feature.id() ) );
    }
    f.setGeometry( outputGeometry );
  }
  return QgsFeatureList() << f;
}

Qgis::ProcessingFeatureSourceFlags QgsSmoothAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

///@endcond
