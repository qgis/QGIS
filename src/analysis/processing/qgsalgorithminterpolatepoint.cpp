/***************************************************************************
                         qgsalgorithminterpolatepoint.cpp
                         ---------------------
    begin                : August 2018
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

#include "qgsalgorithminterpolatepoint.h"
#include "qgsgeometrycollection.h"
#include "qgscurve.h"

///@cond PRIVATE

QString QgsInterpolatePointAlgorithm::name() const
{
  return QStringLiteral( "interpolatepoint" );
}

QString QgsInterpolatePointAlgorithm::displayName() const
{
  return QObject::tr( "Interpolate point on line" );
}

QStringList QgsInterpolatePointAlgorithm::tags() const
{
  return QObject::tr( "linestring,reference,referencing,distance,interpolate" ).split( ',' );
}

QString QgsInterpolatePointAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsInterpolatePointAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsInterpolatePointAlgorithm::outputName() const
{
  return QObject::tr( "Interpolated points" );
}

QString QgsInterpolatePointAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a point geometry interpolated at a set distance along line or curve geometries.\n\n"
                      "Z and M values are linearly interpolated from existing values.\n\n"
                      "If a multipart geometry is encountered, only the first part is considered when "
                      "interpolating the point.\n\n"
                      "If the specified distance is greater than the curve's length, the resultant feature will have a null geometry." );
}

QString QgsInterpolatePointAlgorithm::shortDescription() const
{
  return QObject::tr( "Interpolates a point along lines at a set distance." );
}

QList<int> QgsInterpolatePointAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

Qgis::ProcessingSourceType QgsInterpolatePointAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPoint;
}

Qgis::WkbType QgsInterpolatePointAlgorithm::outputWkbType( Qgis::WkbType inputType ) const
{
  Qgis::WkbType out = Qgis::WkbType::Point;
  if ( QgsWkbTypes::hasZ( inputType ) )
    out = QgsWkbTypes::addZ( out );
  if ( QgsWkbTypes::hasM( inputType ) )
    out = QgsWkbTypes::addM( out );
  return out;
}

QgsInterpolatePointAlgorithm *QgsInterpolatePointAlgorithm::createInstance() const
{
  return new QgsInterpolatePointAlgorithm();
}

void QgsInterpolatePointAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr<QgsProcessingParameterDistance> distance = std::make_unique<QgsProcessingParameterDistance>( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance" ), 0.0, QStringLiteral( "INPUT" ), false, 0 );
  distance->setIsDynamic( true );
  distance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance" ), QgsPropertyDefinition::DoublePositive ) );
  distance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( distance.release() );
}

Qgis::ProcessingFeatureSourceFlags QgsInterpolatePointAlgorithm::sourceFlags() const
{
  // skip geometry checks - this algorithm doesn't care about invalid geometries
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

bool QgsInterpolatePointAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDistance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  mDynamicDistance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  if ( mDynamicDistance )
    mDistanceProperty = parameters.value( QStringLiteral( "DISTANCE" ) ).value<QgsProperty>();

  return true;
}

QgsFeatureList QgsInterpolatePointAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    const QgsGeometry geometry = f.geometry();
    double distance = mDistance;
    if ( mDynamicDistance )
      distance = mDistanceProperty.valueAsDouble( context.expressionContext(), distance );

    f.setGeometry( geometry.interpolate( distance ) );
  }
  return QgsFeatureList() << f;
}

///@endcond
