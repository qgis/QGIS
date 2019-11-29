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

QgsExtractSpecificVerticesAlgorithm *QgsExtractSpecificVerticesAlgorithm::createInstance() const
{
  return new QgsExtractSpecificVerticesAlgorithm();
}

void QgsExtractSpecificVerticesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "VERTICES" ), QObject::tr( "Vertex indices" ), QStringLiteral( "0" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Vertices" ), QgsProcessing::TypeVectorPoint ) );
}

QVariantMap QgsExtractSpecificVerticesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  QgsFields outputFields = source->fields();
  outputFields.append( QgsField( QStringLiteral( "vertex_pos" ), QVariant::Int ) );
  outputFields.append( QgsField( QStringLiteral( "vertex_index" ), QVariant::Int ) );
  outputFields.append( QgsField( QStringLiteral( "vertex_part" ), QVariant::Int ) );
  if ( QgsWkbTypes::geometryType( source->wkbType() ) == QgsWkbTypes::PolygonGeometry )
  {
    outputFields.append( QgsField( QStringLiteral( "vertex_part_ring" ), QVariant::Int ) );
  }
  outputFields.append( QgsField( QStringLiteral( "vertex_part_index" ), QVariant::Int ) );
  outputFields.append( QgsField( QStringLiteral( "distance" ), QVariant::Double ) );
  outputFields.append( QgsField( QStringLiteral( "angle" ), QVariant::Double ) );

  QgsWkbTypes::Type outputWkbType = QgsWkbTypes::Point;
  if ( QgsWkbTypes::hasM( source->wkbType() ) )
  {
    outputWkbType = QgsWkbTypes::addM( outputWkbType );
  }
  if ( QgsWkbTypes::hasZ( source->wkbType() ) )
  {
    outputWkbType = QgsWkbTypes::addZ( outputWkbType );
  }

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outputFields, outputWkbType, source->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QString verticesString = parameterAsString( parameters, QStringLiteral( "VERTICES" ), context );
  QStringList verticesList = verticesString.split( ',', QString::SkipEmptyParts );
  QList< int > indices;
  for ( QString &vertex : verticesList )
  {
    bool ok = false;
    int i = vertex.toInt( &ok );
    if ( ok )
    {
      indices << i;
    }
    else
    {
      throw QgsProcessingException( QObject::tr( "'%1' is not a valid vertex index" ).arg( vertex ) );
    }
  }

  double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 1;
  QgsFeatureIterator fi = source->getFeatures( QgsFeatureRequest(), QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
  QgsFeature f;
  int i = 0;
  while ( fi.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    QgsGeometry inputGeom = f.geometry();
    if ( inputGeom.isNull() )
    {
      sink->addFeature( f, QgsFeatureSink::FastInsert );
    }
    else
    {
      int vertexIndex;
      int totalVertices = inputGeom.constGet()->nCoordinates();
      for ( int vertex : indices )
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

        double distance = inputGeom.distanceToVertex( vertexIndex );
        double angle = inputGeom.angleAtVertex( vertexIndex ) * 180 / M_PI;

        QgsFeature outputFeature = QgsFeature();
        QgsAttributes attrs = f.attributes();
        attrs << vertex
              << vertexIndex
              << vertexId.part;
        if ( QgsWkbTypes::geometryType( source->wkbType() ) == QgsWkbTypes::PolygonGeometry )
        {
          attrs << vertexId.ring;
        }
        attrs << vertexId.vertex
              << distance
              << angle;

        outputFeature.setAttributes( attrs );
        QgsPoint point = inputGeom.vertexAt( vertexIndex );
        outputFeature.setGeometry( QgsGeometry( point.clone() ) );
        sink->addFeature( outputFeature, QgsFeatureSink::FastInsert );
      }
    }
    feedback->setProgress( i * step );
    i++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
