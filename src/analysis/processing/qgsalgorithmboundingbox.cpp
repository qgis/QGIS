/***************************************************************************
                         qgsalgorithmboundingbox.cpp
                         --------------------------
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

#include "qgsalgorithmboundingbox.h"

///@cond PRIVATE

QString QgsBoundingBoxAlgorithm::name() const
{
  return QStringLiteral( "boundingboxes" );
}

QString QgsBoundingBoxAlgorithm::displayName() const
{
  return QObject::tr( "Bounding boxes" );
}

QStringList QgsBoundingBoxAlgorithm::tags() const
{
  return QObject::tr( "bounding,boxes,envelope,rectangle,extent" ).split( ',' );
}

QString QgsBoundingBoxAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsBoundingBoxAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsBoundingBoxAlgorithm::outputName() const
{
  return QObject::tr( "Bounds" );
}

QString QgsBoundingBoxAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the bounding box (envelope) for each feature in an input layer." ) +
         QStringLiteral( "\n\n" )  +
         QObject::tr( "See the 'Minimum bounding geometry' algorithm for a bounding box calculation which covers the whole layer or grouped subsets of features." );
}

QgsBoundingBoxAlgorithm *QgsBoundingBoxAlgorithm::createInstance() const
{
  return new QgsBoundingBoxAlgorithm();
}

QgsFields QgsBoundingBoxAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields fields = inputFields;
  fields.append( QgsField( QStringLiteral( "width" ), QMetaType::Type::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "height" ), QMetaType::Type::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "area" ), QMetaType::Type::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "perimeter" ), QMetaType::Type::Double, QString(), 20, 6 ) );
  return fields;
}

QgsFeatureList QgsBoundingBoxAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    const QgsRectangle bounds = f.geometry().boundingBox();
    const QgsGeometry outputGeometry = QgsGeometry::fromRect( bounds );
    f.setGeometry( outputGeometry );
    QgsAttributes attrs = f.attributes();
    attrs << bounds.width()
          << bounds.height()
          << bounds.area()
          << bounds.perimeter();
    f.setAttributes( attrs );
  }
  else
  {
    QgsAttributes attrs = f.attributes();
    attrs << QVariant()
          << QVariant()
          << QVariant()
          << QVariant();
    f.setAttributes( attrs );
  }
  return QgsFeatureList() << f;
}

///@endcond
