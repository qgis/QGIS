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

QgsProcessingAlgorithm::Flags QgsSimplifyAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

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
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TOLERANCE" ),
                QObject::tr( "Tolerance" ), QgsProcessingParameterNumber::Double,
                1.0, false, 0, 10000000.0 ) );
}

bool QgsSimplifyAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsDouble( parameters, QStringLiteral( "TOLERANCE" ), context );
  mMethod = static_cast< QgsMapToPixelSimplifier::SimplifyAlgorithm >( parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context ) );
  if ( mMethod != QgsMapToPixelSimplifier::Distance )
    mSimplifier.reset( new QgsMapToPixelSimplifier( QgsMapToPixelSimplifier::SimplifyGeometry, mTolerance, mMethod ) );

  return true;
}

QgsFeature QgsSimplifyAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry inputGeometry = f.geometry();
    QgsGeometry outputGeometry;
    if ( mMethod == QgsMapToPixelSimplifier::Distance )
    {
      outputGeometry = inputGeometry.simplify( mTolerance );
    }
    else
    {
      outputGeometry = mSimplifier->simplify( inputGeometry );
    }
    f.setGeometry( outputGeometry );
  }
  return f;
}

///@endcond


