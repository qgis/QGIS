/***************************************************************************
                         qgsalgorithmorientedminimumboundingbox.cpp
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

#include "qgsalgorithmorientedminimumboundingbox.h"

///@cond PRIVATE

QgsProcessingAlgorithm::Flags QgsOrientedMinimumBoundingBoxAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QString QgsOrientedMinimumBoundingBoxAlgorithm::name() const
{
  return QStringLiteral( "orientedminimumboundingbox" );
}

QString QgsOrientedMinimumBoundingBoxAlgorithm::displayName() const
{
  return QObject::tr( "Oriented minimum bounding box" );
}

QStringList QgsOrientedMinimumBoundingBoxAlgorithm::tags() const
{
  return QObject::tr( "bounding,boxes,envelope,rectangle,extent,oriented,angle" ).split( ',' );
}

QString QgsOrientedMinimumBoundingBoxAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsOrientedMinimumBoundingBoxAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsOrientedMinimumBoundingBoxAlgorithm::outputName() const
{
  return QObject::tr( "Bounding boxes" );
}

QgsWkbTypes::Type QgsOrientedMinimumBoundingBoxAlgorithm::outputWkbType( QgsWkbTypes::Type ) const
{
  return QgsWkbTypes::Polygon;
}

QString QgsOrientedMinimumBoundingBoxAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the minimum area rotated rectangle which covers each feature in an input layer." ) +
         QStringLiteral( "\n\n" ) +
         QObject::tr( "See the 'Minimum bounding geometry' algorithm for a oriented bounding box calculation which covers the whole layer or grouped subsets of features." );
}

QgsOrientedMinimumBoundingBoxAlgorithm *QgsOrientedMinimumBoundingBoxAlgorithm::createInstance() const
{
  return new QgsOrientedMinimumBoundingBoxAlgorithm();
}

QgsFields QgsOrientedMinimumBoundingBoxAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields fields = inputFields;
  fields.append( QgsField( QStringLiteral( "width" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "height" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "angle" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "area" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "perimeter" ), QVariant::Double, QString(), 20, 6 ) );
  return fields;
}

QgsFeature QgsOrientedMinimumBoundingBoxAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    double area = 0;
    double angle = 0;
    double width = 0;
    double height = 0;
    QgsGeometry outputGeometry = f.geometry().orientedMinimumBoundingBox( area, angle, width, height );
    f.setGeometry( outputGeometry );
    QgsAttributes attrs = f.attributes();
    attrs << width
          << height
          << angle
          << area
          << 2 * width + 2 * height;
    f.setAttributes( attrs );
  }
  return f;
}

///@endcond


