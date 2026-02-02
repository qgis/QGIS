/***************************************************************************
                         qgsalgorithmforcecw.cpp
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

#include "qgsalgorithmforcecw.h"

#include "qgscurvepolygon.h"
#include "qgsgeometrycollection.h"
#include "qgsvectorlayer.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsForceCWAlgorithm::name() const
{
  return u"forcecw"_s;
}

QString QgsForceCWAlgorithm::displayName() const
{
  return QObject::tr( "Force polygons clockwise" );
}

QStringList QgsForceCWAlgorithm::tags() const
{
  return QObject::tr( "clockwise,cw,orientation,ring,repair,invalid,geometry,make,valid" ).split( ',' );
}

QString QgsForceCWAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsForceCWAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

Qgis::ProcessingFeatureSourceFlags QgsForceCWAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

QString QgsForceCWAlgorithm::outputName() const
{
  return QObject::tr( "Reoriented" );
}

QString QgsForceCWAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm forces polygon geometries to respect the convention where the exterior ring is oriented in a clockwise direction "
                      "and the interior rings in a counter-clockwise direction." );
}

QString QgsForceCWAlgorithm::shortDescription() const
{
  return QObject::tr( "Forces polygon geometries to have clockwise exterior rings and counter-clockwise interior rings." );
}

QList<int> QgsForceCWAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

QgsForceCWAlgorithm *QgsForceCWAlgorithm::createInstance() const
{
  return new QgsForceCWAlgorithm();
}

QgsFeatureList QgsForceCWAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  if ( !feature.hasGeometry() )
    return QgsFeatureList() << feature;

  QgsFeature outputFeature = feature;
  outputFeature.setGeometry( feature.geometry().forcePolygonClockwise() );

  return QgsFeatureList() << outputFeature;
}

///@endcond
