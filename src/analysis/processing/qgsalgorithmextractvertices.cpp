/***************************************************************************
                         qgsalgorithmextractvertices.cpp
                         --------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmextractvertices.h"

#include "qgsabstractgeometry.h"
#include "qgsgeometryutils.h"

///@cond PRIVATE

QString QgsExtractVerticesAlgorithm::name() const
{
  return QStringLiteral( "extractvertices" );
}

QString QgsExtractVerticesAlgorithm::displayName() const
{
  return QObject::tr( "Extract vertices" );
}

QStringList QgsExtractVerticesAlgorithm::tags() const
{
  return QObject::tr( "points,vertex,nodes" ).split( ',' );
}

QString QgsExtractVerticesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsExtractVerticesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsExtractVerticesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer and generates a point layer with points representing the vertices in the input geometries. The attributes associated to each point are the same ones associated to the feature that the point belongs to." ) +
         QStringLiteral( "\n\n" )  +
         QObject::tr( "Additional fields are added to the point indicating the vertex index (beginning at 0), the vertex’s part and its index within the part (as well as its ring for polygons), distance along original geometry and bisector angle of vertex for original geometry." );
}

QString QgsExtractVerticesAlgorithm::outputName() const
{
  return QObject::tr( "Vertices" );
}

QgsExtractVerticesAlgorithm *QgsExtractVerticesAlgorithm::createInstance() const
{
  return new QgsExtractVerticesAlgorithm();
}

Qgis::ProcessingSourceType QgsExtractVerticesAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPoint;
}

QgsFields QgsExtractVerticesAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields outputFields = inputFields;
  outputFields.append( QgsField( QStringLiteral( "vertex_index" ), QVariant::Int, QString(), 10, 0 ) );
  outputFields.append( QgsField( QStringLiteral( "vertex_part" ), QVariant::Int, QString(), 10, 0 ) );
  if ( mGeometryType == Qgis::GeometryType::Polygon )
  {
    outputFields.append( QgsField( QStringLiteral( "vertex_part_ring" ), QVariant::Int, QString(), 10, 0 ) );
  }
  outputFields.append( QgsField( QStringLiteral( "vertex_part_index" ), QVariant::Int, QString(), 10, 0 ) );
  outputFields.append( QgsField( QStringLiteral( "distance" ), QVariant::Double, QString(), 20, 14 ) );
  outputFields.append( QgsField( QStringLiteral( "angle" ), QVariant::Double, QString(), 20, 14 ) );

  return outputFields;
}

Qgis::WkbType QgsExtractVerticesAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  Qgis::WkbType outputWkbType = Qgis::WkbType::Point;
  if ( QgsWkbTypes::hasM( inputWkbType ) )
  {
    outputWkbType = QgsWkbTypes::addM( outputWkbType );
  }
  if ( QgsWkbTypes::hasZ( inputWkbType ) )
  {
    outputWkbType = QgsWkbTypes::addZ( outputWkbType );
  }

  return outputWkbType;
}

Qgis::ProcessingFeatureSourceFlags QgsExtractVerticesAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

QgsFeatureSink::SinkFlags QgsExtractVerticesAlgorithm::sinkFlags() const
{
  return QgsFeatureSink::RegeneratePrimaryKey;
}

bool QgsExtractVerticesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  mGeometryType = QgsWkbTypes::geometryType( source->wkbType() );
  return true;
}

QgsFeatureList QgsExtractVerticesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeatureList outputFeatures;

  QgsFeature f = feature;
  const QgsGeometry inputGeom = f.geometry();
  if ( inputGeom.isEmpty() )
  {
    QgsAttributes attrs = f.attributes();
    attrs << QVariant()
          << QVariant();
    if ( mGeometryType == Qgis::GeometryType::Polygon )
    {
      attrs << QVariant();
    }
    attrs << QVariant()
          << QVariant()
          << QVariant();

    f.clearGeometry();
    f.setAttributes( attrs );
    outputFeatures << f;
  }
  else
  {
    QgsAbstractGeometry::vertex_iterator vi = inputGeom.constGet()->vertices_begin();
    double cumulativeDistance = 0.0;
    int vertexPos = 0;
    while ( vi != inputGeom.constGet()->vertices_end() )
    {
      const QgsVertexId vertexId = vi.vertexId();
      const double angle = inputGeom.constGet()->vertexAngle( vertexId ) * 180 / M_PI;
      QgsAttributes attrs = f.attributes();
      attrs << vertexPos
            << vertexId.part;
      if ( mGeometryType == Qgis::GeometryType::Polygon )
      {
        attrs << vertexId.ring;
      }
      attrs << vertexId.vertex
            << cumulativeDistance
            << angle;

      QgsFeature outputFeature = QgsFeature();
      outputFeature.setAttributes( attrs );
      outputFeature.setGeometry( QgsGeometry( ( *vi ).clone() ) );
      outputFeatures << outputFeature;
      vi++;
      vertexPos++;

      // calculate distance to next vertex
      const double distanceToNext = inputGeom.constGet()->segmentLength( vertexId );
      cumulativeDistance += distanceToNext;
    }
  }

  return outputFeatures;
}

///@endcond
