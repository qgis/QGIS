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
  return QStringLiteral( "smoothgeometry" );
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
  return QStringLiteral( "vectorgeometry" );
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

QgsSmoothAlgorithm *QgsSmoothAlgorithm::createInstance() const
{
  return new QgsSmoothAlgorithm();
}

QList<int> QgsSmoothAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPolygon;
}

void QgsSmoothAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterNumber > iterations = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "ITERATIONS" ),
      QObject::tr( "Iterations" ), QgsProcessingParameterNumber::Integer,
      1, false, 1, 10 );
  iterations->setIsDynamic( true );
  iterations->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "ITERATIONS" ), QObject::tr( "Iterations" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) );
  iterations->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( iterations.release() );

  std::unique_ptr< QgsProcessingParameterNumber > offset = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "OFFSET" ),
      QObject::tr( "Offset" ), QgsProcessingParameterNumber::Double,
      0.25, false, 0.0, 0.5 );
  offset->setIsDynamic( true );
  offset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "OFFSET" ), QObject::tr( "Offset" ), QgsPropertyDefinition::Double0To1 ) );
  offset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( offset.release() );

  std::unique_ptr< QgsProcessingParameterNumber > maxAngle = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "MAX_ANGLE" ),
      QObject::tr( "Maximum node angle to smooth" ), QgsProcessingParameterNumber::Double,
      180.0, false, 0.0, 180.0 );
  maxAngle->setIsDynamic( true );
  maxAngle->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "MAX_ANGLE" ), QObject::tr( "Maximum node angle to smooth" ), QgsPropertyDefinition::Rotation ) );
  maxAngle->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( maxAngle.release() );
}

bool QgsSmoothAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mIterations = parameterAsInt( parameters, QStringLiteral( "ITERATIONS" ), context );
  mDynamicIterations = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "ITERATIONS" ) );
  if ( mDynamicIterations )
    mIterationsProperty = parameters.value( QStringLiteral( "ITERATIONS" ) ).value< QgsProperty >();

  mOffset = parameterAsDouble( parameters, QStringLiteral( "OFFSET" ), context );
  mDynamicOffset = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "OFFSET" ) );
  if ( mDynamicOffset )
    mOffsetProperty = parameters.value( QStringLiteral( "OFFSET" ) ).value< QgsProperty >();

  mMaxAngle = parameterAsDouble( parameters, QStringLiteral( "MAX_ANGLE" ), context );
  mDynamicMaxAngle = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "MAX_ANGLE" ) );
  if ( mDynamicMaxAngle )
    mMaxAngleProperty = parameters.value( QStringLiteral( "MAX_ANGLE" ) ).value< QgsProperty >();

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

QgsProcessingFeatureSource::Flag QgsSmoothAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

///@endcond


