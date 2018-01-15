/***************************************************************************
                         qgsalgorithmfixgeometries.cpp
                         -----------------------------
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

#include "qgsalgorithmfixgeometries.h"

///@cond PRIVATE

QgsProcessingAlgorithm::Flags QgsFixGeometriesAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QString QgsFixGeometriesAlgorithm::name() const
{
  return QStringLiteral( "fixgeometries" );
}

QString QgsFixGeometriesAlgorithm::displayName() const
{
  return QObject::tr( "Fix geometries" );
}

QStringList QgsFixGeometriesAlgorithm::tags() const
{
  return QObject::tr( "repair,invalid,geometry,make,valid" ).split( ',' );
}

QString QgsFixGeometriesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsFixGeometriesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QgsProcessingFeatureSource::Flag QgsFixGeometriesAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

QString QgsFixGeometriesAlgorithm::outputName() const
{
  return QObject::tr( "Fixed geometries" );
}

QgsWkbTypes::Type QgsFixGeometriesAlgorithm::outputWkbType( QgsWkbTypes::Type type ) const
{
  return QgsWkbTypes::multiType( type );
}

QString QgsFixGeometriesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm attempts to create a valid representation of a given invalid geometry without "
                      "losing any of the input vertices. Already-valid geometries are returned without further intervention. "
                      "Always outputs multi-geometry layer.\n\n"
                      "NOTE: M values will be dropped from the output." );
}

QgsFixGeometriesAlgorithm *QgsFixGeometriesAlgorithm::createInstance() const
{
  return new QgsFixGeometriesAlgorithm();
}

QgsFeature QgsFixGeometriesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  if ( !feature.hasGeometry() )
    return feature;

  QgsFeature outputFeature = feature;

  QgsGeometry outputGeometry = outputFeature.geometry().makeValid();
  if ( !outputGeometry )
  {
    feedback->pushInfo( QObject::tr( "makeValid failed for feature %1 " ).arg( feature.id() ) );
    outputFeature.clearGeometry();
    return outputFeature;
  }

  if ( outputGeometry.wkbType() == QgsWkbTypes::Unknown ||
       QgsWkbTypes::flatType( outputGeometry.wkbType() ) == QgsWkbTypes::GeometryCollection )
  {
    // keep only the parts of the geometry collection with correct type
    const QVector< QgsGeometry > tmpGeometries = outputGeometry.asGeometryCollection();
    QVector< QgsGeometry > matchingParts;
    for ( const QgsGeometry &g : tmpGeometries )
    {
      if ( g.type() == feature.geometry().type() )
        matchingParts << g;
    }
    if ( !matchingParts.empty() )
      outputGeometry = QgsGeometry::collectGeometry( matchingParts );
    else
      outputGeometry = QgsGeometry();
  }

  outputGeometry.convertToMultiType();
  if ( QgsWkbTypes::geometryType( outputGeometry.wkbType() ) != QgsWkbTypes::geometryType( feature.geometry().wkbType() ) )
  {
    // don't keep geometries which have different types - e.g. lines converted to points
    feedback->pushInfo( QObject::tr( "Fixing geometry for feature %1 resulted in %2, geometry has been dropped." ).arg( feature.id() ).arg( QgsWkbTypes::displayString( outputGeometry.wkbType() ) ) );
    outputFeature.clearGeometry();
  }
  else
  {
    outputFeature.setGeometry( outputGeometry );
  }
  return outputFeature;
}

///@endcond
