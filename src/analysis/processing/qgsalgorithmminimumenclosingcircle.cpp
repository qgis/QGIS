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

///@cond PRIVATE

QString QgsMinimumEnclosingCircleAlgorithm::name() const
{
  return QStringLiteral( "minimumenclosingcircle" );
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
  return QStringLiteral( "vectorgeometry" );
}

QString QgsMinimumEnclosingCircleAlgorithm::outputName() const
{
  return QObject::tr( "Minimum enclosing circles" );
}

QgsWkbTypes::Type QgsMinimumEnclosingCircleAlgorithm::outputWkbType( QgsWkbTypes::Type ) const
{
  return QgsWkbTypes::Polygon;
}

QgsProcessingAlgorithm::Flags QgsMinimumEnclosingCircleAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsMinimumEnclosingCircleAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SEGMENTS" ), QObject::tr( "Number of segments in circles" ), QgsProcessingParameterNumber::Integer,
                72, false, 8, 100000 ) );
}

QString QgsMinimumEnclosingCircleAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the minimum enclosing circle which covers each feature in an input layer." ) +
         QStringLiteral( "\n\n" ) +
         QObject::tr( "See the 'Minimum bounding geometry' algorithm for a minimal enclosing circle calculation which covers the whole layer or grouped subsets of features." );
}

QgsMinimumEnclosingCircleAlgorithm *QgsMinimumEnclosingCircleAlgorithm::createInstance() const
{
  return new QgsMinimumEnclosingCircleAlgorithm();
}

QgsFields QgsMinimumEnclosingCircleAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields fields = inputFields;
  fields.append( QgsField( QStringLiteral( "radius" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "area" ), QVariant::Double, QString(), 20, 6 ) );
  return fields;
}

bool QgsMinimumEnclosingCircleAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSegments = parameterAsInt( parameters, QStringLiteral( "SEGMENTS" ), context );
  return true;
}

QgsFeature QgsMinimumEnclosingCircleAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    double radius = 0;
    QgsPointXY center;
    QgsGeometry outputGeometry = f.geometry().minimalEnclosingCircle( center, radius, mSegments );
    f.setGeometry( outputGeometry );
    QgsAttributes attrs = f.attributes();
    attrs << radius
          << M_PI *radius *radius;
    f.setAttributes( attrs );
  }
  return f;
}

///@endcond

