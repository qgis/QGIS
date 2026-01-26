/***************************************************************************
                         qgsalgorithmconcavehullbyfeature.cpp
                         ---------------------
    begin                : May 2025
    copyright            : (C) 2025 by Nyall Dawson
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

#include "qgsalgorithmconcavehullbyfeature.h"

#include "qgsgeometrycollection.h"

///@cond PRIVATE

QString QgsConcaveHullByFeatureAlgorithm::name() const
{
  return u"concavehullbyfeature"_s;
}

QString QgsConcaveHullByFeatureAlgorithm::displayName() const
{
  return QObject::tr( "Concave hull (by feature)" );
}

QStringList QgsConcaveHullByFeatureAlgorithm::tags() const
{
  return QObject::tr( "concave,hull,bounds,bounding,convex" ).split( ',' );
}

QString QgsConcaveHullByFeatureAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsConcaveHullByFeatureAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsConcaveHullByFeatureAlgorithm::outputName() const
{
  return QObject::tr( "Concave hulls" );
}

QString QgsConcaveHullByFeatureAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the concave hull for each feature in an input layer." ) + u"\n\n"_s
         + QObject::tr( "A concave hull is a polygon which contains all the points of the input geometries, but is a better approximation than the convex hull to the area occupied by the input." ) + u"\n\n"_s
         + QObject::tr( "It is frequently used to convert a multi-point into a polygonal area which contains all the points from the input geometry." ) + u"\n\n"_s
         + QObject::tr( "See the 'Concave hull (by layer)' algorithm for a concave hull calculation which covers the whole layer or grouped subsets of features." );
}

QString QgsConcaveHullByFeatureAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the concave hull for each feature in an input layer." );
}

QgsConcaveHullByFeatureAlgorithm *QgsConcaveHullByFeatureAlgorithm::createInstance() const
{
  return new QgsConcaveHullByFeatureAlgorithm();
}

void QgsConcaveHullByFeatureAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterNumber( u"ALPHA"_s, QObject::tr( "Threshold (0-1, where 1 is equivalent with Convex Hull)" ), Qgis::ProcessingNumberParameterType::Double, 0.3, false, 0, 1 ) );
  addParameter( new QgsProcessingParameterBoolean( u"HOLES"_s, QObject::tr( "Allow holes" ), true ) );
}

QList<int> QgsConcaveHullByFeatureAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint );
}

QgsFields QgsConcaveHullByFeatureAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields newFields;
  newFields.append( QgsField( u"area"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  newFields.append( QgsField( u"perimeter"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  return QgsProcessingUtils::combineFields( inputFields, newFields );
}

bool QgsConcaveHullByFeatureAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
#if GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR < 11
  throw QgsProcessingException( QObject::tr( "This algorithm requires a QGIS build based on GEOS 3.11 or later" ) );
#endif
  mPercentage = parameterAsDouble( parameters, u"ALPHA"_s, context );
  mAllowHoles = parameterAsBool( parameters, u"HOLES"_s, context );
  return true;
}

QgsFeatureList QgsConcaveHullByFeatureAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry outputGeometry;
    const QgsAbstractGeometry *inputGeometry = f.geometry().constGet();
    const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( inputGeometry );
    if ( !collection || collection->numGeometries() == 1 )
    {
      feedback->reportError( QObject::tr( "Cannot calculate convex hull for a single point feature (%1) (try 'Concave hull (by layer)' algorithm instead)." ).arg( f.id() ) );
      f.clearGeometry();
    }
    else
    {
      outputGeometry = f.geometry().concaveHull( mPercentage, mAllowHoles );
      if ( outputGeometry.isNull() )
        feedback->reportError( outputGeometry.lastError() );
      f.setGeometry( outputGeometry );
    }
    if ( outputGeometry.type() == Qgis::GeometryType::Polygon )
    {
      QgsAttributes attrs = f.attributes();
      attrs << outputGeometry.constGet()->area()
            << outputGeometry.constGet()->perimeter();
      f.setAttributes( attrs );
    }
    else
    {
      if ( outputGeometry.type() == Qgis::GeometryType::Line )
      {
        feedback->pushWarning( QObject::tr( "Concave hull for feature %1 resulted in a linestring, ignoring" ).arg( f.id() ) );
      }
      else if ( outputGeometry.type() == Qgis::GeometryType::Point )
      {
        feedback->pushWarning( QObject::tr( "Concave hull for feature %1 resulted in a point, ignoring" ).arg( f.id() ) );
      }
      QgsAttributes attrs = f.attributes();
      attrs << QVariant()
            << QVariant();
      f.setAttributes( attrs );
    }
  }
  return QgsFeatureList() << f;
}

///@endcond
