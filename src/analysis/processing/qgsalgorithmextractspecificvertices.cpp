/***************************************************************************
                         qgsalgorithmextractspecificvertices.cpp
                         --------------------------
    begin                : November 2019
    copyright            : (C) 2019 by Alexander Bruy
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

#include "qgsalgorithmextractspecificvertices.h"

#include "qgsabstractgeometry.h"
#include "qgsgeometryutils.h"

///@cond PRIVATE

QString QgsExtractSpecificVerticesAlgorithm::name() const
{
  return QStringLiteral( "extractspecificvertices" );
}

QString QgsExtractSpecificVerticesAlgorithm::displayName() const
{
  return QObject::tr( "Extract specific vertices" );
}

QStringList QgsExtractSpecificVerticesAlgorithm::tags() const
{
  return QObject::tr( "points,vertex,nodes" ).split( ',' );
}

QString QgsExtractSpecificVerticesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsExtractSpecificVerticesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsExtractSpecificVerticesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a line or polygon layer and generates a point layer with points "
                      "representing specific vertices in the input lines or polygons. For instance, this algorithm "
                      "can be used to extract the first or last vertices in the geometry. The attributes associated "
                      "to each point are the same ones associated to the line or polygon that the point belongs to." ) +
         QStringLiteral( "\n\n" )  +
         QObject::tr( "The vertex indices parameter accepts a comma separated string specifying the indices of the "
                      "vertices to extract. The first vertex corresponds to an index of 0, the second vertex has an "
                      "index of 1, etc. Negative indices can be used to find vertices at the end of the geometry, "
                      "e.g., an index of -1 corresponds to the last vertex, -2 corresponds to the second last vertex, etc." ) +
         QStringLiteral( "\n\n" )  +
         QObject::tr( "Additional fields are added to the points indicating the specific vertex position (e.g., 0, -1, etc), "
                      "the original vertex index, the vertex’s part and its index within the part (as well as its ring for "
                      "polygons), distance along the original geometry and bisector angle of vertex for the original geometry." );
}

QString QgsExtractSpecificVerticesAlgorithm::outputName() const
{
  return QObject::tr( "Vertices" );
}

QgsExtractSpecificVerticesAlgorithm *QgsExtractSpecificVerticesAlgorithm::createInstance() const
{
  return new QgsExtractSpecificVerticesAlgorithm();
}

QgsProcessing::SourceType QgsExtractSpecificVerticesAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPoint;
}

QgsFields QgsExtractSpecificVerticesAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields outputFields = inputFields;
  outputFields.append( QgsField( QStringLiteral( "vertex_pos" ), QVariant::Int ) );
  outputFields.append( QgsField( QStringLiteral( "vertex_index" ), QVariant::Int ) );
  outputFields.append( QgsField( QStringLiteral( "vertex_part" ), QVariant::Int ) );
  if ( mGeometryType == QgsWkbTypes::PolygonGeometry )
  {
    outputFields.append( QgsField( QStringLiteral( "vertex_part_ring" ), QVariant::Int ) );
  }
  outputFields.append( QgsField( QStringLiteral( "vertex_part_index" ), QVariant::Int ) );
  outputFields.append( QgsField( QStringLiteral( "distance" ), QVariant::Double ) );
  outputFields.append( QgsField( QStringLiteral( "angle" ), QVariant::Double ) );

  return outputFields;
}

QgsWkbTypes::Type QgsExtractSpecificVerticesAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
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

QgsProcessingFeatureSource::Flag QgsExtractSpecificVerticesAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

QgsFeatureSink::SinkFlags QgsExtractSpecificVerticesAlgorithm::sinkFlags() const
{
  return QgsFeatureSink::RegeneratePrimaryKey;
}

void QgsExtractSpecificVerticesAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( QStringLiteral( "VERTICES" ), QObject::tr( "Vertex indices" ), QStringLiteral( "0" ) ) );
}

bool QgsExtractSpecificVerticesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  mGeometryType = QgsWkbTypes::geometryType( source->wkbType() );

  const QString verticesString = parameterAsString( parameters, QStringLiteral( "VERTICES" ), context );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  const QStringList verticesList = verticesString.split( ',', QString::SkipEmptyParts );
#else
  const QStringList verticesList = verticesString.split( ',', Qt::SkipEmptyParts );
#endif
  for ( const QString &vertex : verticesList )
  {
    bool ok = false;
    const int i = vertex.toInt( &ok );
    if ( ok )
    {
      mIndices << i;
    }
    else
    {
      throw QgsProcessingException( QObject::tr( "'%1' is not a valid vertex index" ).arg( vertex ) );
    }
  }

  return true;
}

QgsFeatureList QgsExtractSpecificVerticesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeatureList outputFeatures;

  QgsFeature f = feature;
  const QgsGeometry inputGeom = f.geometry();
  if ( inputGeom.isNull() )
  {
    QgsAttributes attrs = f.attributes();
    attrs << QVariant()
          << QVariant()
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
    int vertexIndex;
    const int totalVertices = inputGeom.constGet()->nCoordinates();
    for ( const int vertex : mIndices )
    {
      if ( vertex < 0 )
      {
        vertexIndex = totalVertices + vertex;
      }
      else
      {
        vertexIndex = vertex;
      }

      if ( vertexIndex < 0 || vertexIndex >= totalVertices )
        continue;

      QgsVertexId vertexId;
      inputGeom.vertexIdFromVertexNr( vertexIndex, vertexId );

      const double distance = inputGeom.distanceToVertex( vertexIndex );
      const double angle = inputGeom.angleAtVertex( vertexIndex ) * 180 / M_PI;

      QgsFeature outFeature = QgsFeature();
      QgsAttributes attrs = f.attributes();
      attrs << vertex
            << vertexIndex
            << vertexId.part;
      if ( mGeometryType == QgsWkbTypes::PolygonGeometry )
      {
        attrs << vertexId.ring;
      }
      attrs << vertexId.vertex
            << distance
            << angle;

      outFeature.setAttributes( attrs );
      const QgsPoint point = inputGeom.vertexAt( vertexIndex );
      outFeature.setGeometry( QgsGeometry( point.clone() ) );
      outputFeatures << outFeature;
    }
  }

  return outputFeatures;
}

///@endcond
