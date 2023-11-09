/***************************************************************************
                         qgsalgorithmsegmentize.cpp
                         ---------------------
    begin                : March 2018
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

#include "qgsalgorithmsegmentize.h"

///@cond PRIVATE

QString QgsSegmentizeByMaximumDistanceAlgorithm::name() const
{
  return QStringLiteral( "segmentizebymaxdistance" );
}

QString QgsSegmentizeByMaximumDistanceAlgorithm::displayName() const
{
  return QObject::tr( "Segmentize by maximum distance" );
}

QStringList QgsSegmentizeByMaximumDistanceAlgorithm::tags() const
{
  return QObject::tr( "straighten,linearize,densify,curves,curved,circular" ).split( ',' );
}

QString QgsSegmentizeByMaximumDistanceAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSegmentizeByMaximumDistanceAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsSegmentizeByMaximumDistanceAlgorithm::outputName() const
{
  return QObject::tr( "Segmentized" );
}

QString QgsSegmentizeByMaximumDistanceAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm segmentizes a geometry by converting curved sections to linear sections.\n\n"
                      "The segmentization is performed by specifying the maximum allowed offset distance between the original "
                      "curve and the segmentized representation.\n\n"
                      "Non-curved geometries will be retained without change." );
}

QgsSegmentizeByMaximumDistanceAlgorithm *QgsSegmentizeByMaximumDistanceAlgorithm::createInstance() const
{
  return new QgsSegmentizeByMaximumDistanceAlgorithm();
}

QList<int> QgsSegmentizeByMaximumDistanceAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPolygon;
}

void QgsSegmentizeByMaximumDistanceAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterDistance > tolerance = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "DISTANCE" ),
      QObject::tr( "Maximum offset distance" ),
      1.0, QStringLiteral( "INPUT" ), false, 0, 10000000.0 );
  tolerance->setIsDynamic( true );
  tolerance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DISTANCE" ), QObject::tr( "Maximum offset distance" ), QgsPropertyDefinition::DoublePositive ) );
  tolerance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( tolerance.release() );
}

bool QgsSegmentizeByMaximumDistanceAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

bool QgsSegmentizeByMaximumDistanceAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  mDynamicTolerance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  if ( mDynamicTolerance )
    mToleranceProperty = parameters.value( QStringLiteral( "DISTANCE" ) ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsSegmentizeByMaximumDistanceAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry geometry = f.geometry();
    double tolerance = mTolerance;
    if ( mDynamicTolerance )
      tolerance = mToleranceProperty.valueAsDouble( context.expressionContext(), tolerance );
    geometry.convertToStraightSegment( tolerance, QgsAbstractGeometry::MaximumDifference );
    f.setGeometry( geometry );
  }
  return QgsFeatureList() << f;
}





QString QgsSegmentizeByMaximumAngleAlgorithm::name() const
{
  return QStringLiteral( "segmentizebymaxangle" );
}

QString QgsSegmentizeByMaximumAngleAlgorithm::displayName() const
{
  return QObject::tr( "Segmentize by maximum angle" );
}

QStringList QgsSegmentizeByMaximumAngleAlgorithm::tags() const
{
  return QObject::tr( "straighten,linearize,densify,curves,curved,circular,angle" ).split( ',' );
}

QString QgsSegmentizeByMaximumAngleAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSegmentizeByMaximumAngleAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsSegmentizeByMaximumAngleAlgorithm::outputName() const
{
  return QObject::tr( "Segmentized" );
}

QString QgsSegmentizeByMaximumAngleAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm segmentizes a geometry by converting curved sections to linear sections.\n\n"
                      "The segmentization is performed by specifying the maximum allowed radius angle between vertices "
                      "on the straightened geometry (e.g the angle of the arc created from the original arc center to consecutive "
                      "output vertices on the linearized geometry).\n\n"
                      "Non-curved geometries will be retained without change." );
}

QgsSegmentizeByMaximumAngleAlgorithm *QgsSegmentizeByMaximumAngleAlgorithm::createInstance() const
{
  return new QgsSegmentizeByMaximumAngleAlgorithm();
}

QList<int> QgsSegmentizeByMaximumAngleAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPolygon;
}

void QgsSegmentizeByMaximumAngleAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterNumber > tolerance = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "ANGLE" ),
      QObject::tr( "Maximum angle between vertices (degrees)" ), QgsProcessingParameterNumber::Double,
      5.0, false, 0, 360.0 );
  tolerance->setIsDynamic( true );
  tolerance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "ANGLE" ), QObject::tr( "Maximum angle between vertices (degrees)" ), QgsPropertyDefinition::DoublePositive ) );
  tolerance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( tolerance.release() );
}

bool QgsSegmentizeByMaximumAngleAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

bool QgsSegmentizeByMaximumAngleAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsDouble( parameters, QStringLiteral( "ANGLE" ), context );
  mDynamicTolerance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "ANGLE" ) );
  if ( mDynamicTolerance )
    mToleranceProperty = parameters.value( QStringLiteral( "ANGLE" ) ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsSegmentizeByMaximumAngleAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry geometry = f.geometry();
    double tolerance = mTolerance;
    if ( mDynamicTolerance )
      tolerance = mToleranceProperty.valueAsDouble( context.expressionContext(), tolerance );
    geometry.convertToStraightSegment( M_PI * tolerance / 180.0, QgsAbstractGeometry::MaximumAngle );
    f.setGeometry( geometry );
  }
  return QgsFeatureList() << f;
}

///@endcond


