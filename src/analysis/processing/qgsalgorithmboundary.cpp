/***************************************************************************
                         qgsalgorithmboundary.cpp
                         ------------------------
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

#include "qgsalgorithmboundary.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsBoundaryAlgorithm::name() const
{
  return QStringLiteral( "boundary" );
}

QString QgsBoundaryAlgorithm::displayName() const
{
  return QObject::tr( "Boundary" );
}

QStringList QgsBoundaryAlgorithm::tags() const
{
  return QObject::tr( "boundary,ring,border,exterior" ).split( ',' );
}

QString QgsBoundaryAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsBoundaryAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsBoundaryAlgorithm::outputName() const
{
  return QObject::tr( "Boundary" );
}

QString QgsBoundaryAlgorithm::shortHelpString() const
{
  return QObject::tr( "Returns the closure of the combinatorial boundary of the input geometries (ie the "
                      "topological boundary of the geometry). For instance, a polygon geometry will have a "
                      "boundary consisting of the linestrings for each ring in the polygon. Only valid for "
                      "polygon or line layers." );
}

QList<int> QgsBoundaryAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

bool QgsBoundaryAlgorithm::supportInPlaceEdit( const QgsMapLayer * ) const
{
  return false;
}

QgsBoundaryAlgorithm *QgsBoundaryAlgorithm::createInstance() const
{
  return new QgsBoundaryAlgorithm();
}

Qgis::ProcessingFeatureSourceFlags QgsBoundaryAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

Qgis::WkbType QgsBoundaryAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  Qgis::WkbType outputWkb = Qgis::WkbType::Unknown;
  switch ( QgsWkbTypes::geometryType( inputWkbType ) )
  {
    case Qgis::GeometryType::Line:
      outputWkb = Qgis::WkbType::MultiPoint;
      break;

    case Qgis::GeometryType::Polygon:
      outputWkb = Qgis::WkbType::MultiLineString;
      break;

    case Qgis::GeometryType::Point:
    case Qgis::GeometryType::Unknown:
    case Qgis::GeometryType::Null:
      outputWkb = Qgis::WkbType::NoGeometry;
      break;
  }

  if ( QgsWkbTypes::hasZ( inputWkbType ) )
    outputWkb = QgsWkbTypes::addZ( outputWkb );
  if ( QgsWkbTypes::hasM( inputWkbType ) )
    outputWkb = QgsWkbTypes::addM( outputWkb );

  return outputWkb;
}

QgsFeatureList QgsBoundaryAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  QgsFeature outFeature = feature;

  if ( feature.hasGeometry() )
  {
    const QgsGeometry inputGeometry = feature.geometry();
    const QgsGeometry outputGeometry = QgsGeometry( inputGeometry.constGet()->boundary() );
    if ( outputGeometry.isNull() )
    {
      feedback->reportError( QObject::tr( "No boundary for feature %1 (possibly a closed linestring?)" ).arg( feature.id() ) );
      outFeature.clearGeometry();
    }
    else
    {
      outFeature.setGeometry( outputGeometry );
    }
  }
  return QgsFeatureList() << outFeature;
}

///@endcond
