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

#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsOrientedMinimumBoundingBoxAlgorithm::name() const
{
  return u"orientedminimumboundingbox"_s;
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
  return u"vectorgeometry"_s;
}

QString QgsOrientedMinimumBoundingBoxAlgorithm::outputName() const
{
  return QObject::tr( "Bounding boxes" );
}

Qgis::WkbType QgsOrientedMinimumBoundingBoxAlgorithm::outputWkbType( Qgis::WkbType ) const
{
  return Qgis::WkbType::Polygon;
}

QString QgsOrientedMinimumBoundingBoxAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the minimum area rotated rectangle which covers each feature in an input layer." ) + u"\n\n"_s + QObject::tr( "For singlepart point features, the output corresponds to the bounding box of the geometry." ) + u"\n\n"_s + QObject::tr( "See the 'Minimum bounding geometry' algorithm for an oriented bounding box calculation which covers the whole layer or grouped subsets of features." );
}

QString QgsOrientedMinimumBoundingBoxAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the minimum area rotated rectangle which covers each feature in an input layer." );
}

QgsOrientedMinimumBoundingBoxAlgorithm *QgsOrientedMinimumBoundingBoxAlgorithm::createInstance() const
{
  return new QgsOrientedMinimumBoundingBoxAlgorithm();
}

bool QgsOrientedMinimumBoundingBoxAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast<const QgsVectorLayer *>( l );
  if ( !layer )
    return false;

  if ( !QgsProcessingFeatureBasedAlgorithm::supportInPlaceEdit( layer ) )
    return false;
  // Polygons only
  return layer->wkbType() == Qgis::WkbType::Polygon || layer->wkbType() == Qgis::WkbType::MultiPolygon;
}

QgsFields QgsOrientedMinimumBoundingBoxAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields newFields;
  newFields.append( QgsField( u"width"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  newFields.append( QgsField( u"height"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  newFields.append( QgsField( u"angle"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  newFields.append( QgsField( u"area"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  newFields.append( QgsField( u"perimeter"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  return QgsProcessingUtils::combineFields( inputFields, newFields );
}

QgsFeatureList QgsOrientedMinimumBoundingBoxAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    double area = 0;
    double angle = 0;
    double width = 0;
    double height = 0;
    const QgsGeometry outputGeometry = f.geometry().orientedMinimumBoundingBox( area, angle, width, height );
    f.setGeometry( outputGeometry );
    QgsAttributes attrs = f.attributes();
    attrs << width
          << height
          << angle
          << area
          << 2 * width + 2 * height;
    f.setAttributes( attrs );
  }
  else
  {
    QgsAttributes attrs = f.attributes();
    attrs << QVariant()
          << QVariant()
          << QVariant()
          << QVariant()
          << QVariant();
    f.setAttributes( attrs );
  }
  return QgsFeatureList() << f;
}

///@endcond
