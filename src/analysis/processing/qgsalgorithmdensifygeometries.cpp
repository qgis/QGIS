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


#include "qgsalgorithmdensifygeometries.h"

QString QgsDensifyGeometriesAlgorithm::name() const
{
  return QStringLiteral( "densifygeometriesgivenaninterval" );
}

QString QgsDensifyGeometriesAlgorithm::displayName() const
{
  return QObject::tr( "Densify by interval" );
}

QStringList QgsDensifyGeometriesAlgorithm::tags() const
{
  return QObject::tr( "add,vertex,vertices,points,nodes" ).split( ',' );
}

QString QgsDensifyGeometriesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsDensifyGeometriesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsDensifyGeometriesAlgorithm::shortHelpString() const
{
  return QObject::tr( "Geometries are densified by adding additional vertices on "
                      "edges that have a maximum distance of the interval parameter "
                      "in map units." );
}

QString QgsDensifyGeometriesAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a densified version of geometries." );
}

QgsDensifyGeometriesAlgorithm *QgsDensifyGeometriesAlgorithm::createInstance() const
{
  return new QgsDensifyGeometriesAlgorithm;

}

QList<int> QgsDensifyGeometriesAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPolygon;
}

void QgsDensifyGeometriesAlgorithm::initParameters( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "INTERVAL" ),
                QObject::tr( "Interval between vertices to add" ),
                1, QStringLiteral( "INPUT" ), false, 0, 10000000 ) );
}

QString QgsDensifyGeometriesAlgorithm::outputName() const
{
  return QObject::tr( "Densified" );
}

QgsFeatureList QgsDensifyGeometriesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( context );
  Q_UNUSED( feedback );
  QgsFeature modifiedFeature = feature;
  if ( feature.hasGeometry() )
    modifiedFeature.setGeometry( feature.geometry().densifyByDistance( mInterval ) );

  return QgsFeatureList() << modifiedFeature;
}

bool QgsDensifyGeometriesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );
  mInterval = parameterAsDouble( parameters, QStringLiteral( "INTERVAL" ), context );
  return true;
}
