/***************************************************************************
                         qgsalgorithmdropgeometry.cpp
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

#include "qgsalgorithmdropgeometry.h"

///@cond PRIVATE

QgsProcessingAlgorithm::Flags QgsDropGeometryAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QString QgsDropGeometryAlgorithm::name() const
{
  return QStringLiteral( "dropgeometries" );
}

QString QgsDropGeometryAlgorithm::displayName() const
{
  return QObject::tr( "Drop geometries" );
}

QStringList QgsDropGeometryAlgorithm::tags() const
{
  return QObject::tr( "remove,drop,delete,geometry,objects" ).split( ',' );
}

QString QgsDropGeometryAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsDropGeometryAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QString QgsDropGeometryAlgorithm::outputName() const
{
  return QObject::tr( "Dropped geometries" );
}

QString QgsDropGeometryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm removes any geometries from an input layer and returns a layer containing only the feature attributes." );
}

QgsDropGeometryAlgorithm *QgsDropGeometryAlgorithm::createInstance() const
{
  return new QgsDropGeometryAlgorithm();
}

QgsCoordinateReferenceSystem QgsDropGeometryAlgorithm::outputCrs( const QgsCoordinateReferenceSystem & ) const
{
  return QgsCoordinateReferenceSystem();
}

QgsWkbTypes::Type QgsDropGeometryAlgorithm::outputWkbType( QgsWkbTypes::Type ) const
{
  return QgsWkbTypes::NoGeometry;
}

QgsFeatureRequest QgsDropGeometryAlgorithm::request() const
{
  return QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry );
}

QgsFeature QgsDropGeometryAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  f.clearGeometry();
  return f;
}

///@endcond
