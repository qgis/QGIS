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
  return QStringLiteral( "densifygeometriesgivenaninterval" );
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
  return QStringLiteral( "vectorgeometry" );
}

QString QgsDensifyGeometriesByIntervalAlgorithm::shortHelpString() const
{
  return QObject::tr( "Geometries are densified by adding additional vertices on "
                      "edges that have a maximum distance of the interval parameter "
                      "in map units." );
}

QString QgsDensifyGeometriesByIntervalAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a densified version of geometries." );
}

QgsDensifyGeometriesByIntervalAlgorithm *QgsDensifyGeometriesByIntervalAlgorithm::createInstance() const
{
  return new QgsDensifyGeometriesByIntervalAlgorithm;

}

QList<int> QgsDensifyGeometriesByIntervalAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPolygon;
}

void QgsDensifyGeometriesByIntervalAlgorithm::initParameters( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )
  std::unique_ptr<QgsProcessingParameterDistance> interval = qgis::make_unique<QgsProcessingParameterDistance>( QStringLiteral( "INTERVAL" ),
      QObject::tr( "Interval between vertices to add" ),
      1, QStringLiteral( "INPUT" ), false, 0, 10000000 );
  interval->setIsDynamic( true );
  interval->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Interval" ), QObject::tr( "Interval" ), QgsPropertyDefinition::DoublePositive ) );
  interval->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( interval.release() );
}

QString QgsDensifyGeometriesByIntervalAlgorithm::outputName() const
{
  return QObject::tr( "Densified" );
}

QgsFeatureList QgsDensifyGeometriesByIntervalAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( context );
  Q_UNUSED( feedback );
  QgsFeature modifiedFeature = feature;

  double interval = mInterval;
  if ( mDynamicInterval )
    interval = mIntervalProperty.valueAsDouble( context.expressionContext(), interval );

  if ( feature.hasGeometry() )
    modifiedFeature.setGeometry( feature.geometry().densifyByDistance( mInterval ) );

  return QgsFeatureList() << modifiedFeature;
}

bool QgsDensifyGeometriesByIntervalAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );
  mInterval = parameterAsDouble( parameters, QStringLiteral( "INTERVAL" ), context );

  mDynamicInterval = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "INTERVAL" ) );
  if ( mDynamicInterval )
    mIntervalProperty = parameters.value( QStringLiteral( "INTERVAL" ) ).value< QgsProperty >();

  return true;
}

///@endcond PRIVATE
