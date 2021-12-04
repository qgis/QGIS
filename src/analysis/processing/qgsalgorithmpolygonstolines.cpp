/***************************************************************************
                         qgsalgorithmpolygonstolines.cpp
                         ---------------------
    begin                : January 2019
    copyright            : (C) 2019 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmpolygonstolines.h"
#include "qgsgeometrycollection.h"
#include "qgscurvepolygon.h"
#include "qgscurve.h"
#include "qgsmultilinestring.h"

///@cond PRIVATE

QString QgsPolygonsToLinesAlgorithm::name() const
{
  return QStringLiteral( "polygonstolines" );
}

QString QgsPolygonsToLinesAlgorithm::displayName() const
{
  return QObject::tr( "Polygons to lines" );
}

QStringList QgsPolygonsToLinesAlgorithm::tags() const
{
  return QObject::tr( "line,polygon,convert" ).split( ',' );
}

QString QgsPolygonsToLinesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsPolygonsToLinesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsPolygonsToLinesAlgorithm::outputName() const
{
  return QObject::tr( "Lines" );
}

QgsProcessing::SourceType QgsPolygonsToLinesAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorLine;
}

QgsWkbTypes::Type QgsPolygonsToLinesAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  QgsWkbTypes::Type wkbType = QgsWkbTypes::Unknown;

  if ( QgsWkbTypes::singleType( QgsWkbTypes::flatType( inputWkbType ) ) == QgsWkbTypes::Polygon )
    wkbType = QgsWkbTypes::MultiLineString;
  else if ( QgsWkbTypes::singleType( QgsWkbTypes::flatType( inputWkbType ) ) == QgsWkbTypes::CurvePolygon )
    wkbType = QgsWkbTypes::MultiCurve;

  if ( QgsWkbTypes::hasM( inputWkbType ) )
    wkbType = QgsWkbTypes::addM( wkbType );
  if ( QgsWkbTypes::hasZ( inputWkbType ) )
    wkbType = QgsWkbTypes::addZ( wkbType );

  return wkbType;
}

QString QgsPolygonsToLinesAlgorithm::shortHelpString() const
{
  return QObject::tr( "Converts polygons to lines" );
}

QString QgsPolygonsToLinesAlgorithm::shortDescription() const
{
  return QObject::tr( "Converts polygons to lines." );
}

QgsPolygonsToLinesAlgorithm *QgsPolygonsToLinesAlgorithm::createInstance() const
{
  return new QgsPolygonsToLinesAlgorithm();
}

QList<int> QgsPolygonsToLinesAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorPolygon;
}

QgsFeatureList QgsPolygonsToLinesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  Q_UNUSED( context )

  QgsFeatureList result;
  QgsFeature feat = feature;
  if ( feat.hasGeometry() )
    feat.setGeometry( convertToLines( feat.geometry() ) );

  result << feat;
  return result;
}

QgsGeometry QgsPolygonsToLinesAlgorithm::convertToLines( const QgsGeometry &geometry ) const
{
  auto rings = extractRings( geometry.constGet() );

  QgsWkbTypes::Type resultType = outputWkbType( geometry.wkbType() );

  std::unique_ptr<QgsMultiCurve> lineGeometry;

  if ( QgsWkbTypes::flatType( resultType ) == QgsWkbTypes::MultiLineString )
    lineGeometry = std::make_unique<QgsMultiLineString>();
  else
    lineGeometry = std::make_unique<QgsMultiCurve>();

  lineGeometry->reserve( rings.size() );
  for ( auto ring : std::as_const( rings ) )
    lineGeometry->addGeometry( ring );

  return QgsGeometry( lineGeometry.release() );
}

QList<QgsCurve *> QgsPolygonsToLinesAlgorithm::extractRings( const QgsAbstractGeometry *geom ) const
{
  QList<QgsCurve *> rings;

  if ( QgsGeometryCollection *collection = qgsgeometry_cast<QgsGeometryCollection *>( geom ) )
  {
    QgsGeometryPartIterator parts = collection->parts();
    while ( parts.hasNext() )
      rings.append( extractRings( parts.next() ) );
  }
  else if ( QgsCurvePolygon *polygon = qgsgeometry_cast<QgsCurvePolygon *>( geom ) )
  {
    if ( auto exteriorRing = polygon->exteriorRing() )
      rings.append( exteriorRing->clone() );
    for ( int i = 0; i < polygon->numInteriorRings(); ++i )
    {
      rings.append( polygon->interiorRing( i )->clone() );
    }
  }

  return rings;
}



///@endcond


