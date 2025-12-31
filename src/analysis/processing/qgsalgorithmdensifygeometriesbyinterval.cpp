/***************************************************************************
                         qgsalgorithmdensifygeometries.cpp
                         ---------------------
    begin                : January 2019
    copyright            : (C) 2019 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsalgorithmdensifygeometriesbyinterval.h"

///@cond PRIVATE

QString QgsDensifyGeometriesByIntervalAlgorithm::name() const
{
  return u"densifygeometriesgivenaninterval"_s;
}

QString QgsDensifyGeometriesByIntervalAlgorithm::displayName() const
{
  return QObject::tr( "Densify by interval" );
}

QStringList QgsDensifyGeometriesByIntervalAlgorithm::tags() const
{
  return QObject::tr( "add,vertex,vertices,points,nodes" ).split( ',' );
}

QString QgsDensifyGeometriesByIntervalAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsDensifyGeometriesByIntervalAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsDensifyGeometriesByIntervalAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a polygon or line layer and generates a new one "
                      "in which the geometries have a larger number of vertices than the original one.\n\n"
                      "Geometries are densified by adding additional vertices on "
                      "edges that have a maximum distance of the interval parameter "
                      "in map units." );
}

QString QgsDensifyGeometriesByIntervalAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a densified version of geometries by setting a maximum distance for segments." );
}

QgsDensifyGeometriesByIntervalAlgorithm *QgsDensifyGeometriesByIntervalAlgorithm::createInstance() const
{
  return new QgsDensifyGeometriesByIntervalAlgorithm;
}

QList<int> QgsDensifyGeometriesByIntervalAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

void QgsDensifyGeometriesByIntervalAlgorithm::initParameters( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )
  auto interval = std::make_unique<QgsProcessingParameterDistance>( u"INTERVAL"_s, QObject::tr( "Interval between vertices to add" ), 1, u"INPUT"_s, false, 0, 10000000 );
  interval->setIsDynamic( true );
  interval->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Interval"_s, QObject::tr( "Interval" ), QgsPropertyDefinition::DoublePositive ) );
  interval->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( interval.release() );
}

QString QgsDensifyGeometriesByIntervalAlgorithm::outputName() const
{
  return QObject::tr( "Densified" );
}

QgsFeatureList QgsDensifyGeometriesByIntervalAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( context )
  Q_UNUSED( feedback )
  QgsFeature modifiedFeature = feature;

  double interval = mInterval;
  if ( mDynamicInterval )
    interval = mIntervalProperty.valueAsDouble( context.expressionContext(), interval );

  if ( feature.hasGeometry() )
    modifiedFeature.setGeometry( feature.geometry().densifyByDistance( interval ) );

  return QgsFeatureList() << modifiedFeature;
}

bool QgsDensifyGeometriesByIntervalAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )
  mInterval = parameterAsDouble( parameters, u"INTERVAL"_s, context );

  mDynamicInterval = QgsProcessingParameters::isDynamic( parameters, u"INTERVAL"_s );
  if ( mDynamicInterval )
    mIntervalProperty = parameters.value( u"INTERVAL"_s ).value<QgsProperty>();

  return true;
}

///@endcond PRIVATE
