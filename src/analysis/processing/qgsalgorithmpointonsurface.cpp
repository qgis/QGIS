/***************************************************************************
                         qgsalgorithmpointonsurface.cpp
                         ------------------------
    begin                : March 2018
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

#include "qgsalgorithmpointonsurface.h"

///@cond PRIVATE

QString QgsPointOnSurfaceAlgorithm::name() const
{
  return QStringLiteral( "pointonsurface" );
}

QString QgsPointOnSurfaceAlgorithm::displayName() const
{
  return QObject::tr( "Point on surface" );
}

QStringList QgsPointOnSurfaceAlgorithm::tags() const
{
  return QObject::tr( "centroid,inside,within" ).split( ',' );
}

QString QgsPointOnSurfaceAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsPointOnSurfaceAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsPointOnSurfaceAlgorithm::outputName() const
{
  return QObject::tr( "Point" );
}

QString QgsPointOnSurfaceAlgorithm::shortHelpString() const
{
  return QObject::tr( "Returns a point guaranteed to lie on the surface of a geometry." );
}

QgsPointOnSurfaceAlgorithm *QgsPointOnSurfaceAlgorithm::createInstance() const
{
  return new QgsPointOnSurfaceAlgorithm();
}

QgsFeatureList QgsPointOnSurfaceAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  QgsFeature feature = f;
  if ( feature.hasGeometry() )
  {
    QgsGeometry outputGeometry = feature.geometry().pointOnSurface();
    if ( !outputGeometry )
    {
      feedback->pushInfo( QObject::tr( "Error calculating point on surface for feature %1: %2" ).arg( feature.id() ).arg( outputGeometry.lastError() ) );
    }
    feature.setGeometry( outputGeometry );
  }
  return QgsFeatureList() << feature;
}

///@endcond
