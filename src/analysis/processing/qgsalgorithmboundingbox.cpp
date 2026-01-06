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
  return u"boundingboxes"_s;
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
  return u"vectorgeometry"_s;
}

QString QgsBoundingBoxAlgorithm::outputName() const
{
  return QObject::tr( "Bounds" );
}

QString QgsBoundingBoxAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the bounding box (envelope) for each feature in an input layer." ) + u"\n\n"_s + QObject::tr( "See the 'Minimum bounding geometry' algorithm for a bounding box calculation which covers the whole layer or grouped subsets of features." );
}

QString QgsBoundingBoxAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the bounding box (envelope) for each feature in an input layer." );
}

QgsBoundingBoxAlgorithm *QgsBoundingBoxAlgorithm::createInstance() const
{
  return new QgsBoundingBoxAlgorithm();
}

QgsFields QgsBoundingBoxAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields newFields;
  newFields.append( QgsField( u"width"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  newFields.append( QgsField( u"height"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  newFields.append( QgsField( u"area"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  newFields.append( QgsField( u"perimeter"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  return QgsProcessingUtils::combineFields( inputFields, newFields );
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
