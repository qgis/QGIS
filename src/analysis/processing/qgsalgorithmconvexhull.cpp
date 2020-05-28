/***************************************************************************
                         qgsalgorithmconvexhull.cpp
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

#include "qgsalgorithmconvexhull.h"

///@cond PRIVATE

QString QgsConvexHullAlgorithm::name() const
{
  return QStringLiteral( "convexhull" );
}

QString QgsConvexHullAlgorithm::displayName() const
{
  return QObject::tr( "Convex hull" );
}

QStringList QgsConvexHullAlgorithm::tags() const
{
  return QObject::tr( "convex,hull,bounds,bounding" ).split( ',' );
}

QString QgsConvexHullAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsConvexHullAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsConvexHullAlgorithm::outputName() const
{
  return QObject::tr( "Convex hulls" );
}

QString QgsConvexHullAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the convex hull for each feature in an input layer." ) +
         QStringLiteral( "\n\n" ) +
         QObject::tr( "See the 'Minimum bounding geometry' algorithm for a convex hull calculation which covers the whole layer or grouped subsets of features." );
}

QgsConvexHullAlgorithm *QgsConvexHullAlgorithm::createInstance() const
{
  return new QgsConvexHullAlgorithm();
}

QgsFields QgsConvexHullAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields fields = inputFields;
  fields.append( QgsField( QStringLiteral( "area" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "perimeter" ), QVariant::Double, QString(), 20, 6 ) );
  return fields;
}

QgsFeatureList QgsConvexHullAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry outputGeometry;
    if ( QgsWkbTypes::flatType( f.geometry().wkbType() ) == QgsWkbTypes::Point )
    {
      feedback->reportError( QObject::tr( "Cannot calculate convex hull for a single Point feature (try 'Minimum bounding geometry' algorithm instead)." ) );
      f.clearGeometry();
    }
    else
    {
      outputGeometry = f.geometry().convexHull();
      if ( outputGeometry.isNull() )
        feedback->reportError( outputGeometry.lastError() );
      f.setGeometry( outputGeometry );
    }
    if ( !outputGeometry.isNull() )
    {
      QgsAttributes attrs = f.attributes();
      attrs << outputGeometry.constGet()->area()
            << outputGeometry.constGet()->perimeter();
      f.setAttributes( attrs );
    }
    else
    {
      QgsAttributes attrs = f.attributes();
      attrs << QVariant()
            << QVariant();
      f.setAttributes( attrs );
    }
  }
  return QgsFeatureList() << f;
}

///@endcond

