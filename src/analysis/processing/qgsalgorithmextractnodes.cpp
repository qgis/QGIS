/***************************************************************************
                         qgsalgorithmextractnodes.cpp
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

#include "qgsalgorithmextractnodes.h"

#include "qgsabstractgeometry.h"
#include "qgsgeometryutils.h"

///@cond PRIVATE

QString QgsExtractNodesAlgorithm::name() const
{
  return QStringLiteral( "extractnodes" );
}

QString QgsExtractNodesAlgorithm::displayName() const
{
  return QObject::tr( "Extract nodes" );
}

QStringList QgsExtractNodesAlgorithm::tags() const
{
  return QObject::tr( "points,vertex,vertices" ).split( ',' );
}

QString QgsExtractNodesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsExtractNodesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsExtractNodesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a line or polygon layer and generates a point layer with points representing the nodes in the input lines or polygons. The attributes associated to each point are the same ones associated to the line or polygon that the point belongs to." ) +
         QStringLiteral( "\n\n" )  +
         QObject::tr( "Additional fields are added to the nodes indicating the node index (beginning at 0), the node’s part and its index within the part (as well as its ring for polygons), distance along original geometry and bisector angle of node for original geometry." );
}

QgsExtractNodesAlgorithm *QgsExtractNodesAlgorithm::createInstance() const
{
  return new QgsExtractNodesAlgorithm();
}

QgsProcessingAlgorithm::Flags QgsExtractNodesAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsExtractNodesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Nodes" ) ) );
}

QVariantMap QgsExtractNodesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > featureSource( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !featureSource )
    return QVariantMap();

  QgsWkbTypes::Type outputWkbType = QgsWkbTypes::Point;
  if ( QgsWkbTypes::hasM( featureSource->wkbType() ) )
  {
    outputWkbType = QgsWkbTypes::addM( outputWkbType );
  }
  if ( QgsWkbTypes::hasZ( featureSource->wkbType() ) )
  {
    outputWkbType = QgsWkbTypes::addZ( outputWkbType );
  }

  QgsFields outputFields = featureSource->fields();
  outputFields.append( QgsField( QStringLiteral( "node_index" ), QVariant::Int, QString(), 10, 0 ) );
  outputFields.append( QgsField( QStringLiteral( "node_part" ), QVariant::Int, QString(), 10, 0 ) );
  if ( QgsWkbTypes::geometryType( featureSource->wkbType() ) == QgsWkbTypes::PolygonGeometry )
  {
    outputFields.append( QgsField( QStringLiteral( "node_part_ring" ), QVariant::Int, QString(), 10, 0 ) );
  }
  outputFields.append( QgsField( QStringLiteral( "node_part_index" ), QVariant::Int, QString(), 10, 0 ) );
  outputFields.append( QgsField( QStringLiteral( "distance" ), QVariant::Double, QString(), 20, 14 ) );
  outputFields.append( QgsField( QStringLiteral( "angle" ), QVariant::Double, QString(), 20, 14 ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outputFields, outputWkbType, featureSource->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();

  double step = featureSource->featureCount() > 0 ? 100.0 / featureSource->featureCount() : 1;
  QgsFeatureIterator fi = featureSource->getFeatures( QgsFeatureRequest() );
  QgsFeature f;
  int i = -1;
  while ( fi.nextFeature( f ) )
  {
    i++;
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
      QgsAbstractGeometry::vertex_iterator vi = inputGeom.constGet()->vertices_begin();
      double cumulativeDistance = 0.0;
      int vertexPos = 0;
      while ( vi != inputGeom.constGet()->vertices_end() )
      {
        QgsVertexId vertexId = vi.vertexId();
        double angle = inputGeom.constGet()->vertexAngle( vertexId ) * 180 / M_PI;
        QgsAttributes attrs = f.attributes();
        attrs << vertexPos
              << vertexId.part;
        if ( QgsWkbTypes::geometryType( featureSource->wkbType() ) == QgsWkbTypes::PolygonGeometry )
        {
          attrs << vertexId.ring;
        }
        attrs << vertexId.vertex
              << cumulativeDistance
              << angle;
        QgsFeature outputFeature = QgsFeature();
        outputFeature.setAttributes( attrs );
        outputFeature.setGeometry( QgsGeometry( ( *vi ).clone() ) );
        sink->addFeature( outputFeature, QgsFeatureSink::FastInsert );
        vi++;
        vertexPos++;

        // calculate distance to next vertex
        double distanceToNext = inputGeom.constGet()->segmentLength( vertexId );
        cumulativeDistance += distanceToNext;
      }
    }
    feedback->setProgress( i * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
