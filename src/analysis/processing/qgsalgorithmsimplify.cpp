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

#include <memory>

#include "qgsmaptopixelgeometrysimplifier.h"

///@cond PRIVATE

QgsSimplifyAlgorithm::~QgsSimplifyAlgorithm() = default;

QString QgsSimplifyAlgorithm::name() const
{
  return u"simplifygeometries"_s;
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
  return u"vectorgeometry"_s;
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

QString QgsSimplifyAlgorithm::shortDescription() const
{
  return QObject::tr( "Simplifies the geometries in a line or polygon layer by removing a number of vertices." );
}

QgsSimplifyAlgorithm *QgsSimplifyAlgorithm::createInstance() const
{
  return new QgsSimplifyAlgorithm();
}

QList<int> QgsSimplifyAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

void QgsSimplifyAlgorithm::initParameters( const QVariantMap & )
{
  QStringList methods;
  methods << QObject::tr( "Distance (Douglas-Peucker)" )
          << QObject::tr( "Snap to grid" )
          << QObject::tr( "Area (Visvalingam)" );

  addParameter( new QgsProcessingParameterEnum(
    u"METHOD"_s,
    QObject::tr( "Simplification method" ),
    methods, false, 0
  ) );
  auto tolerance = std::make_unique<QgsProcessingParameterDistance>( u"TOLERANCE"_s, QObject::tr( "Tolerance" ), 1.0, u"INPUT"_s, false, 0, 10000000.0 );
  tolerance->setIsDynamic( true );
  tolerance->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Tolerance"_s, QObject::tr( "Tolerance distance" ), QgsPropertyDefinition::DoublePositive ) );
  tolerance->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( tolerance.release() );
}

bool QgsSimplifyAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsDouble( parameters, u"TOLERANCE"_s, context );
  mDynamicTolerance = QgsProcessingParameters::isDynamic( parameters, u"TOLERANCE"_s );
  if ( mDynamicTolerance )
    mToleranceProperty = parameters.value( u"TOLERANCE"_s ).value<QgsProperty>();

  mMethod = static_cast<Qgis::VectorSimplificationAlgorithm>( parameterAsEnum( parameters, u"METHOD"_s, context ) );
  if ( mMethod != Qgis::VectorSimplificationAlgorithm::Distance )
    mSimplifier = std::make_unique<QgsMapToPixelSimplifier>( QgsMapToPixelSimplifier::SimplifyGeometry, mTolerance, mMethod );

  return true;
}

QgsFeatureList QgsSimplifyAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    const QgsGeometry inputGeometry = f.geometry();
    QgsGeometry outputGeometry;
    if ( mMethod == Qgis::VectorSimplificationAlgorithm::Distance )
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

Qgis::ProcessingFeatureSourceFlags QgsSimplifyAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

///@endcond
