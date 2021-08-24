/***************************************************************************
                         qgsalgorithmorthogonalize.cpp
                         ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmorthogonalize.h"
#include "qgsprocessing.h"

///@cond PRIVATE

QString QgsOrthogonalizeAlgorithm::name() const
{
  return QStringLiteral( "orthogonalize" );
}

QString QgsOrthogonalizeAlgorithm::displayName() const
{
  return QObject::tr( "Orthogonalize" );
}

QStringList QgsOrthogonalizeAlgorithm::tags() const
{
  return QObject::tr( "rectangle,perpendicular,right,angles,square,quadrilateralise" ).split( ',' );
}

QString QgsOrthogonalizeAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsOrthogonalizeAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsOrthogonalizeAlgorithm::shortHelpString() const
{
  return QObject::tr( "Takes a line or polygon layer and attempts to orthogonalize "
                      "all the geometries in the layer. This process shifts the nodes "
                      "in the geometries to try to make every angle in the geometry "
                      "either a right angle or a straight line.\n\n"
                      "The angle tolerance parameter is used to specify the maximum "
                      "deviation from a right angle or straight line a node can have "
                      "for it to be adjusted. Smaller tolerances mean that only nodes "
                      "which are already closer to right angles will be adjusted, and "
                      "larger tolerances mean that nodes which deviate further from "
                      "right angles will also be adjusted.\n\n"
                      "The algorithm is iterative. Setting a larger number for the maximum "
                      "iterations will result in a more orthogonal geometry at the cost of "
                      "extra processing time." );
}

QString QgsOrthogonalizeAlgorithm::outputName() const
{
  return QObject::tr( "Orthogonalized" );
}

QList<int> QgsOrthogonalizeAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorPolygon << QgsProcessing::TypeVectorLine;
}

QgsOrthogonalizeAlgorithm *QgsOrthogonalizeAlgorithm::createInstance() const
{
  return new QgsOrthogonalizeAlgorithm();
}

void QgsOrthogonalizeAlgorithm::initParameters( const QVariantMap & )
{
  auto angleToleranceParam = std::make_unique < QgsProcessingParameterNumber >( QStringLiteral( "ANGLE_TOLERANCE" ), QObject::tr( "Maximum angle tolerance (degrees)" ),
                             QgsProcessingParameterNumber::Double, 15.0, false, 0.0, 45.0 );
  angleToleranceParam->setIsDynamic( true );
  angleToleranceParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Angle tolerance" ), QObject::tr( "Maximum angle tolerance (degrees)" ), QgsPropertyDefinition::Double ) );
  angleToleranceParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( angleToleranceParam.release() );

  std::unique_ptr< QgsProcessingParameterNumber> maxIterations = std::make_unique< QgsProcessingParameterNumber >(
        QStringLiteral( "MAX_ITERATIONS" ),
        QObject::tr( "Maximum algorithm iterations" ),
        QgsProcessingParameterNumber::Integer,
        1000, false, 1, 10000 );
  maxIterations->setFlags( maxIterations->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( maxIterations.release() );
}

bool QgsOrthogonalizeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mAngleTolerance = parameterAsDouble( parameters, QStringLiteral( "ANGLE_TOLERANCE" ), context );
  mDynamicAngleTolerance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "ANGLE_TOLERANCE" ) );
  if ( mDynamicAngleTolerance )
    mAngleToleranceProperty = parameters.value( QStringLiteral( "ANGLE_TOLERANCE" ) ).value< QgsProperty >();

  mMaxIterations = parameterAsDouble( parameters, QStringLiteral( "MAX_ITERATIONS" ), context );

  return true;
}

QgsFeatureList QgsOrthogonalizeAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;

  if ( f.hasGeometry() )
  {
    double angleTolerance = mAngleTolerance;
    if ( mDynamicAngleTolerance )
      angleTolerance = mAngleToleranceProperty.valueAsDouble( context.expressionContext(), angleTolerance );

    const QgsGeometry outputGeometry = f.geometry().orthogonalize( 1.0e-8, mMaxIterations, angleTolerance );
    if ( outputGeometry.isNull() )
      throw QgsProcessingException( QObject::tr( "Error orthogonalizing geometry" ) );

    f.setGeometry( outputGeometry );
  }

  return QgsFeatureList() << f;
}

///@endcond
