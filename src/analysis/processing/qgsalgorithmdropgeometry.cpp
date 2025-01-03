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
#include "qgsvectorlayer.h"

///@cond PRIVATE

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

bool QgsDropGeometryAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  return qobject_cast<const QgsVectorLayer *>( l );
}

Qgis::WkbType QgsDropGeometryAlgorithm::outputWkbType( Qgis::WkbType ) const
{
  return Qgis::WkbType::NoGeometry;
}

Qgis::ProcessingFeatureSourceFlags QgsDropGeometryAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

QgsFeatureRequest QgsDropGeometryAlgorithm::request() const
{
  return QgsFeatureRequest().setFlags( Qgis::FeatureRequestFlag::NoGeometry );
}

QgsFeatureList QgsDropGeometryAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  f.clearGeometry();
  return QgsFeatureList() << f;
}

///@endcond
