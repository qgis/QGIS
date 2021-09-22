/***************************************************************************
                         qgsalgorithmsimplify.cpp
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

#include "qgsalgorithmsimplify.h"

///@cond PRIVATE

QString QgsSimplifyAlgorithm::name() const
{
  return QStringLiteral( "simplifygeometries" );
}

QString QgsSimplifyAlgorithm::displayName() const
{
  return QObject::tr( "Simplify" );
}

QStringList QgsSimplifyAlgorithm::tags() const
{
  return QObject::tr( "simplify,generalize,douglas,peucker,visvalingam" ).split( ',' );
}

QString QgsSimplifyAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSimplifyAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsSimplifyAlgorithm::outputName() const
{
  return QObject::tr( "Simplified" );
}

QString QgsSimplifyAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm simplifies the geometries in a line or polygon layer. It creates a new layer "
                      "with the same features as the ones in the input layer, but with geometries containing a lower number of vertices.\n\n"
                      "The algorithm gives a choice of simplification methods, including distance based "
                      "(the \"Douglas-Peucker\" algorithm), area based (\"Visvalingam\" algorithm) and snapping geometries to a grid." );
}

QgsSimplifyAlgorithm *QgsSimplifyAlgorithm::createInstance() const
{
  return new QgsSimplifyAlgorithm();
}

QList<int> QgsSimplifyAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPolygon;
}

void QgsSimplifyAlgorithm::initParameters( const QVariantMap & )
{
  QStringList methods;
  methods << QObject::tr( "Distance (Douglas-Peucker)" )
          << QObject::tr( "Snap to grid" )
          << QObject::tr( "Area (Visvalingam)" );

  addParameter( new QgsProcessingParameterEnum(
                  QStringLiteral( "METHOD" ),
                  QObject::tr( "Simplification method" ),
                  methods, false, 0 ) );
  std::unique_ptr< QgsProcessingParameterDistance > tolerance = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "TOLERANCE" ),
      QObject::tr( "Tolerance" ), 1.0, QStringLiteral( "INPUT" ), false, 0, 10000000.0 );
  tolerance->setIsDynamic( true );
  tolerance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Tolerance" ), QObject::tr( "Tolerance distance" ), QgsPropertyDefinition::DoublePositive ) );
  tolerance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( tolerance.release() );
}

bool QgsSimplifyAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsDouble( parameters, QStringLiteral( "TOLERANCE" ), context );
  mDynamicTolerance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "TOLERANCE" ) );
  if ( mDynamicTolerance )
    mToleranceProperty = parameters.value( QStringLiteral( "TOLERANCE" ) ).value< QgsProperty >();

  mMethod = static_cast< QgsMapToPixelSimplifier::SimplifyAlgorithm >( parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context ) );
  if ( mMethod != QgsMapToPixelSimplifier::Distance )
    mSimplifier.reset( new QgsMapToPixelSimplifier( QgsMapToPixelSimplifier::SimplifyGeometry, mTolerance, mMethod ) );

  return true;
}

QgsFeatureList QgsSimplifyAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    const QgsGeometry inputGeometry = f.geometry();
    QgsGeometry outputGeometry;
    if ( mMethod == QgsMapToPixelSimplifier::Distance )
    {
      double tolerance = mTolerance;
      if ( mDynamicTolerance )
        tolerance = mToleranceProperty.valueAsDouble( context.expressionContext(), tolerance );
      outputGeometry = inputGeometry.simplify( tolerance );
    }
    else
    {
      if ( !mDynamicTolerance )
      {
        outputGeometry = mSimplifier->simplify( inputGeometry );
      }
      else
      {
        const double tolerance = mToleranceProperty.valueAsDouble( context.expressionContext(), mTolerance );
        const QgsMapToPixelSimplifier simplifier( QgsMapToPixelSimplifier::SimplifyGeometry, tolerance, mMethod );
        outputGeometry = simplifier.simplify( inputGeometry );
      }
    }
    f.setGeometry( outputGeometry );
  }
  return QgsFeatureList() << f;
}

QgsProcessingFeatureSource::Flag QgsSimplifyAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

///@endcond


