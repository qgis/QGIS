/***************************************************************************
                         qgsalgorithmforceccw.cpp
                         ---------------------
    begin                : January 2026
    copyright            : (C) 2026 by Andrea Giudiceandrea
    email                : andreaerdna at libero dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmforceccw.h"

#include "qgscurvepolygon.h"
#include "qgsgeometrycollection.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsForceCCWAlgorithm::name() const
{
  return u"forceccw"_s;
}

QString QgsForceCCWAlgorithm::displayName() const
{
  return QObject::tr( "Force polygons counter-clockwise" );
}

QStringList QgsForceCCWAlgorithm::tags() const
{
  return QObject::tr( "clockwise,ccw,counter,counter-clockwise,orientation,ring,repair,invalid,geometry,make,valid" ).split( ',' );
}

QString QgsForceCCWAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsForceCCWAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

Qgis::ProcessingFeatureSourceFlags QgsForceCCWAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

QString QgsForceCCWAlgorithm::outputName() const
{
  return QObject::tr( "Reoriented" );
}

QString QgsForceCCWAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm forces polygon geometries to respect the convention where the exterior ring is oriented in a counter-clockwise "
                      "direction and the interior rings in a clockwise direction." );
}

QString QgsForceCCWAlgorithm::shortDescription() const
{
  return QObject::tr( "Forces polygon geometries to have counter-clockwise exterior rings and clockwise interior rings." );
}

QList<int> QgsForceCCWAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

QgsForceCCWAlgorithm *QgsForceCCWAlgorithm::createInstance() const
{
  return new QgsForceCCWAlgorithm();
}

QgsFeatureList QgsForceCCWAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  if ( !feature.hasGeometry() )
    return QgsFeatureList() << feature;

  QgsFeature outputFeature = feature;
  outputFeature.setGeometry( feature.geometry().forcePolygonCounterClockwise() );

  return QgsFeatureList() << outputFeature;
}

///@endcond
