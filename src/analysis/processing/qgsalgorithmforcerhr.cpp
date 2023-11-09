/***************************************************************************
                         qgsalgorithmforcerhr.cpp
                         ---------------------
    begin                : November 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsalgorithmforcerhr.h"
#include "qgsvectorlayer.h"
#include "qgsgeometrycollection.h"
#include "qgscurvepolygon.h"

///@cond PRIVATE

QString QgsForceRHRAlgorithm::name() const
{
  return QStringLiteral( "forcerhr" );
}

QString QgsForceRHRAlgorithm::displayName() const
{
  return QObject::tr( "Force right-hand-rule" );
}

QStringList QgsForceRHRAlgorithm::tags() const
{
  return QObject::tr( "clockwise,counter,orientation,ring,repair,invalid,geometry,make,valid" ).split( ',' );
}

QString QgsForceRHRAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsForceRHRAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QgsProcessingFeatureSource::Flag QgsForceRHRAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

QString QgsForceRHRAlgorithm::outputName() const
{
  return QObject::tr( "Reoriented" );
}

QString QgsForceRHRAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm forces polygon geometries to respect the Right-Hand-Rule, in which the area that is bounded by a polygon "
                      "is to the right of the boundary. In particular, the exterior ring is oriented in a clockwise direction and the interior "
                      "rings in a counter-clockwise direction." );
}

QString QgsForceRHRAlgorithm::shortDescription() const
{
  return QObject::tr( "Forces polygon geometries to respect the Right-Hand-Rule." );
}

QList<int> QgsForceRHRAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorPolygon;
}

QgsForceRHRAlgorithm *QgsForceRHRAlgorithm::createInstance() const
{
  return new QgsForceRHRAlgorithm();
}

QgsFeatureList QgsForceRHRAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  if ( !feature.hasGeometry() )
    return QgsFeatureList() << feature;

  QgsFeature outputFeature = feature;
  outputFeature.setGeometry( feature.geometry().forceRHR() );

  return QgsFeatureList() << outputFeature;
}

///@endcond
