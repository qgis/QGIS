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
  return QObject::tr( "This algorithm takes a line or polygon layer and generates a point layer with points representing the vertices in the input lines or polygons. The attributes associated to each point are the same ones associated to the line or polygon that the point belongs to." ) +
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

QgsProcessing::SourceType QgsExtractVerticesAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPoint;
}

QgsFields QgsExtractVerticesAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields outputFields = inputFields;
  outputFields.append( QgsField( QStringLiteral( "vertex_index" ), QVariant::Int, QString(), 10, 0 ) );
  outputFields.append( QgsField( QStringLiteral( "vertex_part" ), QVariant::Int, QString(), 10, 0 ) );
  if ( mGeometryType == QgsWkbTypes::PolygonGeometry )
  {
    outputFields.append( QgsField( QStringLiteral( "vertex_part_ring" ), QVariant::Int, QString(), 10, 0 ) );
  }
  outputFields.append( QgsField( QStringLiteral( "vertex_part_index" ), QVariant::Int, QString(), 10, 0 ) );
  outputFields.append( QgsField( QStringLiteral( "distance" ), QVariant::Double, QString(), 20, 14 ) );
  outputFields.append( QgsField( QStringLiteral( "angle" ), QVariant::Double, QString(), 20, 14 ) );

  return outputFields;
}

QgsWkbTypes::Type QgsExtractVerticesAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  QgsWkbTypes::Type outputWkbType = QgsWkbTypes::Point;
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

QgsProcessingFeatureSource::Flag QgsExtractVerticesAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
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
  if ( inputGeom.isNull() )
  {
    QgsAttributes attrs = f.attributes();
    attrs << QVariant()
          << QVariant();
    if ( mGeometryType == QgsWkbTypes::PolygonGeometry )
    {
      attrs << QVariant();
    }
    attrs << QVariant()
          << QVariant()
          << QVariant();

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
      if ( mGeometryType == QgsWkbTypes::PolygonGeometry )
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
