/***************************************************************************
                         qgsalgorithmminimumenclosingcircle.cpp
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

#include "qgsalgorithmminimumenclosingcircle.h"

#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsMinimumEnclosingCircleAlgorithm::name() const
{
  return u"minimumenclosingcircle"_s;
}

QString QgsMinimumEnclosingCircleAlgorithm::displayName() const
{
  return QObject::tr( "Minimum enclosing circles" );
}

QStringList QgsMinimumEnclosingCircleAlgorithm::tags() const
{
  return QObject::tr( "minimum,circle,ellipse,extent,bounds,bounding" ).split( ',' );
}

QString QgsMinimumEnclosingCircleAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsMinimumEnclosingCircleAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsMinimumEnclosingCircleAlgorithm::outputName() const
{
  return QObject::tr( "Minimum enclosing circles" );
}

Qgis::WkbType QgsMinimumEnclosingCircleAlgorithm::outputWkbType( Qgis::WkbType ) const
{
  return Qgis::WkbType::Polygon;
}

void QgsMinimumEnclosingCircleAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterNumber( u"SEGMENTS"_s, QObject::tr( "Number of segments in circles" ), Qgis::ProcessingNumberParameterType::Integer, 72, false, 8, 100000 ) );
}

QString QgsMinimumEnclosingCircleAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the minimum enclosing circle which covers each feature in an input layer." ) + u"\n\n"_s + QObject::tr( "See the 'Minimum bounding geometry' algorithm for a minimal enclosing circle calculation which covers the whole layer or grouped subsets of features." );
}

QString QgsMinimumEnclosingCircleAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the minimum enclosing circle which covers each feature in an input layer." );
}

QgsMinimumEnclosingCircleAlgorithm *QgsMinimumEnclosingCircleAlgorithm::createInstance() const
{
  return new QgsMinimumEnclosingCircleAlgorithm();
}

bool QgsMinimumEnclosingCircleAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast<const QgsVectorLayer *>( l );
  if ( !layer )
    return false;

  if ( !QgsProcessingFeatureBasedAlgorithm::supportInPlaceEdit( layer ) )
    return false;
  // (no Z no M)
  return !( QgsWkbTypes::hasM( layer->wkbType() ) || QgsWkbTypes::hasZ( layer->wkbType() ) );
}

QgsFields QgsMinimumEnclosingCircleAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields newFields;
  newFields.append( QgsField( u"radius"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  newFields.append( QgsField( u"area"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  return QgsProcessingUtils::combineFields( inputFields, newFields );
}

bool QgsMinimumEnclosingCircleAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSegments = parameterAsInt( parameters, u"SEGMENTS"_s, context );
  return true;
}

QgsFeatureList QgsMinimumEnclosingCircleAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    double radius = 0;
    QgsPointXY center;
    const QgsGeometry outputGeometry = f.geometry().minimalEnclosingCircle( center, radius, mSegments );
    f.setGeometry( outputGeometry );
    QgsAttributes attrs = f.attributes();
    attrs << radius
          << M_PI * radius * radius;
    f.setAttributes( attrs );
  }
  else
  {
    QgsAttributes attrs = f.attributes();
    attrs << QVariant()
          << QVariant();
    f.setAttributes( attrs );
  }
  return QgsFeatureList() << f;
}

///@endcond
