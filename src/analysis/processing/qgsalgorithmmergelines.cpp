/***************************************************************************
                         qgsalgorithmmergelines.cpp
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

#include "qgsalgorithmmergelines.h"

///@cond PRIVATE

QString QgsMergeLinesAlgorithm::name() const
{
  return QStringLiteral( "mergelines" );
}

QString QgsMergeLinesAlgorithm::displayName() const
{
  return QObject::tr( "Merge lines" );
}

QStringList QgsMergeLinesAlgorithm::tags() const
{
  return QObject::tr( "line,merge,join,parts" ).split( ',' );
}

QString QgsMergeLinesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsMergeLinesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsMergeLinesAlgorithm::outputName() const
{
  return QObject::tr( "Merged" );
}

Qgis::ProcessingSourceType QgsMergeLinesAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorLine;
}

Qgis::WkbType QgsMergeLinesAlgorithm::outputWkbType( Qgis::WkbType ) const
{
  return Qgis::WkbType::MultiLineString;
}

QString QgsMergeLinesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm joins all connected parts of MultiLineString geometries into single LineString geometries.\n\n"
                      "If any parts of the input MultiLineString geometries are not connected, the resultant "
                      "geometry will be a MultiLineString containing any lines which could be merged and any non-connected line parts." );
}

QList<int> QgsMergeLinesAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine );
}

QgsMergeLinesAlgorithm *QgsMergeLinesAlgorithm::createInstance() const
{
  return new QgsMergeLinesAlgorithm();
}

QgsFeatureList QgsMergeLinesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  if ( !feature.hasGeometry() )
    return QgsFeatureList() << feature;

  QgsFeature out = feature;
  const QgsGeometry outputGeometry = feature.geometry().mergeLines();
  if ( outputGeometry.isNull() )
    feedback->reportError( QObject::tr( "Error merging lines for feature %1" ).arg( feature.id() ) );

  out.setGeometry( outputGeometry );
  return QgsFeatureList() << out;
}

///@endcond
