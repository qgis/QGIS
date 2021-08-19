/***************************************************************************
                         qgsalgorithmserviceareafrompoint.cpp
                         ---------------------
    begin                : July 2018
    copyright            : (C) 2018 by Alexander Bruy
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

#include "qgsalgorithmserviceareafrompoint.h"

#include "qgsgeometryutils.h"
#include "qgsgraphanalyzer.h"

///@cond PRIVATE

QString QgsServiceAreaFromPointAlgorithm::name() const
{
  return QStringLiteral( "serviceareafrompoint" );
}

QString QgsServiceAreaFromPointAlgorithm::displayName() const
{
  return QObject::tr( "Service area (from point)" );
}

QStringList QgsServiceAreaFromPointAlgorithm::tags() const
{
  return QObject::tr( "network,service,area,shortest,fastest" ).split( ',' );
}

QString QgsServiceAreaFromPointAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector with all the edges or parts of edges "
                      "of a network line layer that can be reached within a distance or a time, "
                      "starting from a point feature. The distance and the time (both referred to "
                      "as \"travel cost\") must be specified respectively in the network layer "
                      "units or in hours." );
}

QgsServiceAreaFromPointAlgorithm *QgsServiceAreaFromPointAlgorithm::createInstance() const
{
  return new QgsServiceAreaFromPointAlgorithm();
}

void QgsServiceAreaFromPointAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParams();
  addParameter( new QgsProcessingParameterPoint( QStringLiteral( "START_POINT" ), QObject::tr( "Start point" ) ) );

  std::unique_ptr< QgsProcessingParameterNumber > travelCost = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "TRAVEL_COST" ), QObject::tr( "Travel cost (distance for 'Shortest', time for 'Fastest')" ), QgsProcessingParameterNumber::Double, 0, true, 0 );
  travelCost->setFlags( travelCost->flags() | QgsProcessingParameterDefinition::FlagHidden );
  addParameter( travelCost.release() );

  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TRAVEL_COST2" ), QObject::tr( "Travel cost (distance for 'Shortest', time for 'Fastest')" ),
                QgsProcessingParameterNumber::Double, 0, false, 0 ) );

  std::unique_ptr< QgsProcessingParameterBoolean > includeBounds = std::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "INCLUDE_BOUNDS" ), QObject::tr( "Include upper/lower bound points" ), false, true );
  includeBounds->setFlags( includeBounds->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( includeBounds.release() );

  std::unique_ptr< QgsProcessingParameterFeatureSink > outputLines = std::make_unique< QgsProcessingParameterFeatureSink >( QStringLiteral( "OUTPUT_LINES" ),  QObject::tr( "Service area (lines)" ),
      QgsProcessing::TypeVectorLine, QVariant(), true );
  outputLines->setCreateByDefault( true );
  addParameter( outputLines.release() );

  std::unique_ptr< QgsProcessingParameterFeatureSink > outputPoints = std::make_unique< QgsProcessingParameterFeatureSink >( QStringLiteral( "OUTPUT" ),  QObject::tr( "Service area (boundary nodes)" ),
      QgsProcessing::TypeVectorPoint, QVariant(), true );
  outputPoints->setCreateByDefault( false );
  addParameter( outputPoints.release() );
}

QVariantMap QgsServiceAreaFromPointAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  loadCommonParams( parameters, context, feedback );

  const QgsPointXY startPoint = parameterAsPoint( parameters, QStringLiteral( "START_POINT" ), context, mNetwork->sourceCrs() );

  // use older deprecated travel cost style if specified, to maintain old api
  const bool useOldTravelCost = parameters.value( QStringLiteral( "TRAVEL_COST" ) ).isValid();
  double travelCost = parameterAsDouble( parameters, useOldTravelCost ? QStringLiteral( "TRAVEL_COST" ) : QStringLiteral( "TRAVEL_COST2" ), context );

  const int strategy = parameterAsInt( parameters, QStringLiteral( "STRATEGY" ), context );
  if ( strategy && !useOldTravelCost )
    travelCost *= mMultiplier;

  bool includeBounds = true;  // default to true to maintain 3.0 API
  if ( parameters.contains( QStringLiteral( "INCLUDE_BOUNDS" ) ) )
  {
    includeBounds = parameterAsBool( parameters, QStringLiteral( "INCLUDE_BOUNDS" ), context );
  }

  feedback->pushInfo( QObject::tr( "Building graph…" ) );
  QVector< QgsPointXY > snappedPoints;
  mDirector->makeGraph( mBuilder.get(), QVector< QgsPointXY >() << startPoint, snappedPoints, feedback );

  feedback->pushInfo( QObject::tr( "Calculating service area…" ) );
  std::unique_ptr< QgsGraph> graph( mBuilder->takeGraph() );
  const int idxStart = graph->findVertex( snappedPoints[0] );

  QVector< int > tree;
  QVector< double > costs;
  QgsGraphAnalyzer::dijkstra( graph.get(), idxStart, 0, &tree, &costs );

  QgsMultiPointXY points;
  QgsMultiPolylineXY lines;
  QSet< int > vertices;

  int inboundEdgeIndex;
  double startVertexCost, endVertexCost;
  QgsPointXY edgeStart, edgeEnd;
  QgsGraphEdge edge;

  for ( int i = 0; i < costs.size(); i++ )
  {
    inboundEdgeIndex = tree.at( i );
    if ( inboundEdgeIndex == -1 && i != idxStart )
    {
      // unreachable vertex
      continue;
    }

    startVertexCost = costs.at( i );
    if ( startVertexCost > travelCost )
    {
      // vertex is too expensive, discard
      continue;
    }

    vertices.insert( i );
    edgeStart = graph->vertex( i ).point();

    // find all edges coming from this vertex
    const QList< int > outgoingEdges = graph->vertex( i ).outgoingEdges() ;
    for ( const int edgeId : outgoingEdges )
    {
      edge = graph->edge( edgeId );
      endVertexCost = startVertexCost + edge.cost( 0 ).toDouble();
      edgeEnd = graph->vertex( edge.toVertex() ).point();
      if ( endVertexCost <= travelCost )
      {
        // end vertex is cheap enough to include
        vertices.insert( edge.toVertex() );
        lines.push_back( QgsPolylineXY() << edgeStart << edgeEnd );
      }
      else
      {
        // travelCost sits somewhere on this edge, interpolate position
        const QgsPointXY interpolatedEndPoint = QgsGeometryUtils::interpolatePointOnLineByValue( edgeStart.x(), edgeStart.y(), startVertexCost,
                                                edgeEnd.x(), edgeEnd.y(), endVertexCost, travelCost );

        points.push_back( interpolatedEndPoint );
        lines.push_back( QgsPolylineXY() << edgeStart << interpolatedEndPoint );
      }
    } // edges
  } // costs

  // convert to list and sort to maintain same order of points between algorithm runs
  QList< int > verticesList = qgis::setToList( vertices );
  points.reserve( verticesList.size() );
  std::sort( verticesList.begin(), verticesList.end() );
  for ( const int v : verticesList )
  {
    points.push_back( graph->vertex( v ).point() );
  }

  feedback->pushInfo( QObject::tr( "Writing results…" ) );

  QVariantMap outputs;

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "type" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "start" ), QVariant::String ) );

  QgsFeature feat;
  feat.setFields( fields );

  QString pointsSinkId;
  std::unique_ptr< QgsFeatureSink > pointsSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, pointsSinkId, fields,
      QgsWkbTypes::MultiPoint, mNetwork->sourceCrs() ) );

  if ( pointsSink )
  {
    outputs.insert( QStringLiteral( "OUTPUT" ), pointsSinkId );

    const QgsGeometry geomPoints = QgsGeometry::fromMultiPointXY( points );
    feat.setGeometry( geomPoints );
    feat.setAttributes( QgsAttributes() << QStringLiteral( "within" ) << startPoint.toString() );
    if ( !pointsSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

    if ( includeBounds )
    {
      QgsMultiPointXY upperBoundary, lowerBoundary;
      QVector< int > nodes;

      int vertexId;
      for ( int i = 0; i < costs.size(); i++ )
      {
        if ( costs.at( i ) > travelCost && tree.at( i ) != -1 )
        {
          vertexId = graph->edge( tree.at( i ) ).fromVertex();
          if ( costs.at( vertexId ) <= travelCost )
          {
            nodes.push_back( i );
          }
        }
      } // costs

      upperBoundary.reserve( nodes.size() );
      lowerBoundary.reserve( nodes.size() );
      for ( const int i : nodes )
      {
        upperBoundary.push_back( graph->vertex( graph->edge( tree.at( i ) ).toVertex() ).point() );
        lowerBoundary.push_back( graph->vertex( graph->edge( tree.at( i ) ).fromVertex() ).point() );
      } // nodes

      const QgsGeometry geomUpper = QgsGeometry::fromMultiPointXY( upperBoundary );
      const QgsGeometry geomLower = QgsGeometry::fromMultiPointXY( lowerBoundary );

      feat.setGeometry( geomUpper );
      feat.setAttributes( QgsAttributes() << QStringLiteral( "upper" ) << startPoint.toString() );
      if ( !pointsSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

      feat.setGeometry( geomLower );
      feat.setAttributes( QgsAttributes() << QStringLiteral( "lower" ) << startPoint.toString() );
      if ( !pointsSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    } // includeBounds
  }

  QString linesSinkId;
  std::unique_ptr< QgsFeatureSink > linesSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT_LINES" ), context, linesSinkId, fields,
      QgsWkbTypes::MultiLineString, mNetwork->sourceCrs() ) );

  if ( linesSink )
  {
    outputs.insert( QStringLiteral( "OUTPUT_LINES" ), linesSinkId );
    const QgsGeometry geomLines = QgsGeometry::fromMultiPolylineXY( lines );
    feat.setGeometry( geomLines );
    feat.setAttributes( QgsAttributes() << QStringLiteral( "lines" ) << startPoint.toString() );
    if ( !linesSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( linesSink.get(), parameters, QStringLiteral( "OUTPUT_LINES" ) ) );
  }

  return outputs;
}

///@endcond
