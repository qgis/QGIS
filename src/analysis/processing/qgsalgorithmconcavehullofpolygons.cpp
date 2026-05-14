/***************************************************************************
                         qgsalgorithmconcavehullofpolygons.cpp
                         ---------------------
    begin                : March 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#include "qgsalgorithmconcavehullofpolygons.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsConcaveHullOfPolygonsAlgorithm::name() const
{
  return u"tightconcavehullofpolygons"_s;
}

QString QgsConcaveHullOfPolygonsAlgorithm::displayName() const
{
  return QObject::tr( "Fill gaps between polygons" );
}

QStringList QgsConcaveHullOfPolygonsAlgorithm::tags() const
{
  return QObject::tr( "concave,hull,bounds,bounding,convex,multipolygons,boundary,fill,holes,between,space,tight,strict" ).split( ',' );
}

QString QgsConcaveHullOfPolygonsAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsConcaveHullOfPolygonsAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsConcaveHullOfPolygonsAlgorithm::outputName() const
{
  return QObject::tr( "Tight concave hulls" );
}

QString QgsConcaveHullOfPolygonsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates a concave hull for each multi-polygon feature in an input layer." )
         + u"\n\n"_s
         + QObject::tr(
           "Unlike the standard Concave Hull algorithm, a tight concave hull is a (possibly) non-convex polygon containing all the input polygons. The computed hull fills the gaps between the "
           "polygons without intersecting their interiors. It strictly follows the outer boundaries of the input polygons, allowing you to fill gaps between them without distorting their original "
           "shapes."
         )
         + u"\n\n"_s
         + QObject::tr(
           "It is particularly useful for cases such as generalizing groups of building outlines, creating 'district' polygons around blocks, or removing gaps and joining disjoint sets of polygons."
         )
         + u"\n\n"_s
         + QObject::tr( "The algorithm works by creating a constrained Delaunay Triangulation of the space between the polygons and removing the longest outer edges until a target criterion is reached." )
         + u"\n\n"_s
         + QObject::tr( "The input geometry must be a valid Polygon or MultiPolygon (i.e., the individual polygons must not overlap)." );
}

QString QgsConcaveHullOfPolygonsAlgorithm::shortDescription() const
{
  return QObject::tr( "Constructs a tight concave hull for a set of polygons, filling gaps between them while strictly preserving their original outer boundaries." );
}

QgsConcaveHullOfPolygonsAlgorithm *QgsConcaveHullOfPolygonsAlgorithm::createInstance() const
{
  return new QgsConcaveHullOfPolygonsAlgorithm();
}

void QgsConcaveHullOfPolygonsAlgorithm::initParameters( const QVariantMap & )
{
  auto ratioParam
    = std::make_unique<QgsProcessingParameterNumber>( u"RATIO"_s, QObject::tr( "Ratio (0-1, where 1 is equivalent with Convex Hull)" ), Qgis::ProcessingNumberParameterType::Double, 0.3, false, 0, 1 );
  ratioParam->setHelp(
    QObject::tr(
      "Specifies the maximum edge length as a fraction of the difference between the longest and shortest edge lengths between the polygons. This normalizes the maximum edge length to be scale-free. "
      "A value of 1 produces the convex hull; a value of 0 produces the original polygons."
    )
  );
  addParameter( ratioParam.release() );

  auto holesParam = std::make_unique<QgsProcessingParameterBoolean>( u"HOLES"_s, QObject::tr( "Allow holes" ), true );
  holesParam->setHelp( QObject::tr( "Controls whether the computed concave hull is allowed to contain holes." ) );
  addParameter( holesParam.release() );
}

QList<int> QgsConcaveHullOfPolygonsAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

QgsFields QgsConcaveHullOfPolygonsAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields newFields;
  newFields.append( QgsField( u"area"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  newFields.append( QgsField( u"perimeter"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  return QgsProcessingUtils::combineFields( inputFields, newFields );
}

bool QgsConcaveHullOfPolygonsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
#if GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR < 11
  throw QgsProcessingException( QObject::tr( "This algorithm requires a QGIS build based on GEOS 3.11 or later" ) );
#endif
  mPercentage = parameterAsDouble( parameters, u"RATIO"_s, context );
  mAllowHoles = parameterAsBool( parameters, u"HOLES"_s, context );
  return true;
}

QgsFeatureList QgsConcaveHullOfPolygonsAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry outputGeometry;
    outputGeometry = f.geometry().concaveHullOfPolygons( mPercentage, mAllowHoles, true );
    if ( outputGeometry.isNull() && !outputGeometry.lastError().isEmpty() )
      feedback->reportError( outputGeometry.lastError() );
    f.setGeometry( outputGeometry );

    if ( outputGeometry.type() == Qgis::GeometryType::Polygon )
    {
      QgsAttributes attrs = f.attributes();
      attrs << outputGeometry.constGet()->area() << outputGeometry.constGet()->perimeter();
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
      attrs << QVariant() << QVariant();
      f.setAttributes( attrs );
    }
  }
  return QgsFeatureList() << f;
}

///@endcond
