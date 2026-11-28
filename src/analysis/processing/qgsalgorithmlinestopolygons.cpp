/***************************************************************************
                         qgsalgorithmlinestopolygons.cpp
                         ------------------------------
    begin                : November 2026
    copyright            : (C) 2026 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmlinestopolygons.h"

#include "qgsmultipolygon.h"
#include "qgsmultisurface.h"
#include "qgspolygon.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsLinesToPolygonsAlgorithm::name() const
{
  return u"linestopolygons"_s;
}

QString QgsLinesToPolygonsAlgorithm::displayName() const
{
  return QObject::tr( "Lines to polygons" );
}

QStringList QgsLinesToPolygonsAlgorithm::tags() const
{
  return QObject::tr( "line,polygon,convert" ).split( ',' );
}

QString QgsLinesToPolygonsAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsLinesToPolygonsAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsLinesToPolygonsAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm generates a polygon layer using as polygon rings the lines from an input line layer.\n"
    "All attributes from the input line layer will be copied to the output layer."
  );
}

QString QgsLinesToPolygonsAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates polygons using the lines from an input line layer." );
}

QString QgsLinesToPolygonsAlgorithm::outputName() const
{
  return QObject::tr( "Polygons" );
}

Qgis::ProcessingSourceType QgsLinesToPolygonsAlgorithm::outputType() const
{
  return Qgis::ProcessingSourceType::VectorPolygon;
}

QList<int> QgsLinesToPolygonsAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine );
}

Qgis::WkbType QgsLinesToPolygonsAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  return convertWkbToPolygons( inputWkbType );
}

bool QgsLinesToPolygonsAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

QgsLinesToPolygonsAlgorithm *QgsLinesToPolygonsAlgorithm::createInstance() const
{
  return new QgsLinesToPolygonsAlgorithm();
}

QgsFeatureList QgsLinesToPolygonsAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  Q_UNUSED( context )

  QgsFeature outFeature( feature );
  if ( outFeature.hasGeometry() )
  {
    const QgsGeometry converted = convertToPolygons( outFeature.geometry() );
    if ( converted.isEmpty() )
    {
      feedback->reportError( QObject::tr( "One or more line ignored due to geometry not having a minimum of three vertices." ) );
    }
    outFeature.setGeometry( converted );
  }
  return QgsFeatureList() << outFeature;
}

Qgis::WkbType QgsLinesToPolygonsAlgorithm::convertWkbToPolygons( Qgis::WkbType wkb )
{
  Qgis::WkbType multiWkb = Qgis::WkbType::NoGeometry;
  const Qgis::WkbType singleFlat = QgsWkbTypes::singleType( QgsWkbTypes::flatType( wkb ) );

  if ( singleFlat == Qgis::WkbType::LineString )
  {
    multiWkb = Qgis::WkbType::MultiPolygon;
  }
  else if ( singleFlat == Qgis::WkbType::CompoundCurve )
  {
    multiWkb = Qgis::WkbType::MultiSurface;
  }

  if ( QgsWkbTypes::hasM( wkb ) )
  {
    multiWkb = QgsWkbTypes::addM( multiWkb );
  }
  if ( QgsWkbTypes::hasZ( wkb ) )
  {
    multiWkb = QgsWkbTypes::addZ( multiWkb );
  }

  return multiWkb;
}

QgsGeometry QgsLinesToPolygonsAlgorithm::convertToPolygons( const QgsGeometry &geometry )
{
  QList<QgsPolygon *> surfaces;
  collectSurfaces( geometry.constGet(), surfaces );

  const Qgis::WkbType outWkb = convertWkbToPolygons( geometry.wkbType() );
  std::unique_ptr<QgsGeometryCollection> outGeom;

  if ( QgsWkbTypes::flatType( outWkb ) == Qgis::WkbType::MultiPolygon )
  {
    outGeom = std::make_unique<QgsMultiPolygon>();
  }
  else
  {
    outGeom = std::make_unique<QgsMultiSurface>();
  }

  for ( QgsPolygon *surface : std::as_const( surfaces ) )
  {
    outGeom->addGeometry( surface );
  }

  return QgsGeometry( outGeom.release() );
}

void QgsLinesToPolygonsAlgorithm::collectSurfaces( const QgsAbstractGeometry *geometry, QList<QgsPolygon *> &surfaces )
{
  if ( !geometry )
  {
    return;
  }

  if ( const auto *collection = dynamic_cast<const QgsGeometryCollection *>( geometry ) )
  {
    for ( int i = 0; i < collection->numGeometries(); ++i )
    {
      collectSurfaces( collection->geometryN( i ), surfaces );
    }
  }
  else if ( geometry->vertexCount() > 2 )
  {
    if ( const auto *curve = dynamic_cast<const QgsCurve *>( geometry ) )
    {
      auto polygon = std::make_unique<QgsPolygon>();
      polygon->setExteriorRing( curve->clone() );
      surfaces.append( polygon.release() );
    }
  }
}

///@endcond
