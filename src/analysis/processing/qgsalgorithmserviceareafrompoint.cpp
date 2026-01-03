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
  return u"serviceareafrompoint"_s;
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
  return QObject::tr( "This algorithm creates a new vector layer with all the edges or parts of edges "
                      "of a network line layer that can be reached within a distance or a time, "
                      "starting from a point feature. The distance and the time (both referred to "
                      "as \"travel cost\") must be specified respectively in the network layer "
                      "units or in hours." );
}

QString QgsServiceAreaFromPointAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a vector layer with all the edges or parts of edges "
                      "of a network line layer that can be reached within a distance or a time, "
                      "starting from a point feature." );
}

QgsServiceAreaFromPointAlgorithm *QgsServiceAreaFromPointAlgorithm::createInstance() const
{
  return new QgsServiceAreaFromPointAlgorithm();
}

void QgsServiceAreaFromPointAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParams();
  addParameter( new QgsProcessingParameterPoint( u"START_POINT"_s, QObject::tr( "Start point" ) ) );

  auto travelCost = std::make_unique<QgsProcessingParameterNumber>( u"TRAVEL_COST"_s, QObject::tr( "Travel cost (distance for 'Shortest', time for 'Fastest')" ), Qgis::ProcessingNumberParameterType::Double, 0, true, 0 );
  travelCost->setFlags( travelCost->flags() | Qgis::ProcessingParameterFlag::Hidden );
  addParameter( travelCost.release() );

  addParameter( new QgsProcessingParameterNumber( u"TRAVEL_COST2"_s, QObject::tr( "Travel cost (distance for 'Shortest', time for 'Fastest')" ), Qgis::ProcessingNumberParameterType::Double, 0, false, 0 ) );

  std::unique_ptr<QgsProcessingParameterNumber> maxPointDistanceFromNetwork = std::make_unique<QgsProcessingParameterDistance>( u"POINT_TOLERANCE"_s, QObject::tr( "Maximum point distance from network" ), QVariant(), u"INPUT"_s, true, 0 );
  maxPointDistanceFromNetwork->setFlags( maxPointDistanceFromNetwork->flags() | Qgis::ProcessingParameterFlag::Advanced );
  maxPointDistanceFromNetwork->setHelp( QObject::tr( "Specifies an optional limit on the distance from the point to the network layer. If the point is further from the network than this distance an error will be raised." ) );
  addParameter( maxPointDistanceFromNetwork.release() );

  auto includeBounds = std::make_unique<QgsProcessingParameterBoolean>( u"INCLUDE_BOUNDS"_s, QObject::tr( "Include upper/lower bound points" ), false, true );
  includeBounds->setFlags( includeBounds->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( includeBounds.release() );

  auto outputLines = std::make_unique<QgsProcessingParameterFeatureSink>( u"OUTPUT_LINES"_s, QObject::tr( "Service area (lines)" ), Qgis::ProcessingSourceType::VectorLine, QVariant(), true );
  outputLines->setCreateByDefault( true );
  addParameter( outputLines.release() );

  auto outputPoints = std::make_unique<QgsProcessingParameterFeatureSink>( u"OUTPUT"_s, QObject::tr( "Service area (boundary nodes)" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true );
  outputPoints->setCreateByDefault( false );
  addParameter( outputPoints.release() );
}

QVariantMap QgsServiceAreaFromPointAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  loadCommonParams( parameters, context, feedback );

  const QgsPointXY startPoint = parameterAsPoint( parameters, u"START_POINT"_s, context, mNetwork->sourceCrs() );

  // use older deprecated travel cost style if specified, to maintain old api
  const bool useOldTravelCost = parameters.value( u"TRAVEL_COST"_s ).isValid();
  double travelCost = parameterAsDouble( parameters, useOldTravelCost ? u"TRAVEL_COST"_s : u"TRAVEL_COST2"_s, context );

  const int strategy = parameterAsInt( parameters, u"STRATEGY"_s, context );
  if ( strategy && !useOldTravelCost )
    travelCost *= mMultiplier;

  bool includeBounds = true; // default to true to maintain 3.0 API
  if ( parameters.contains( u"INCLUDE_BOUNDS"_s ) )
  {
    includeBounds = parameterAsBool( parameters, u"INCLUDE_BOUNDS"_s, context );
  }

  feedback->pushInfo( QObject::tr( "Building graph…" ) );
  QVector<QgsPointXY> snappedPoints;
  mDirector->makeGraph( mBuilder.get(), { startPoint }, snappedPoints, feedback );
  const QgsPointXY snappedStartPoint = snappedPoints[0];

  // check distance for the snapped point
  if ( parameters.value( u"POINT_TOLERANCE"_s ).isValid() )
  {
    const double pointDistanceThreshold = parameterAsDouble( parameters, u"POINT_TOLERANCE"_s, context );

    double distancePointToNetwork = 0;
    try
    {
      distancePointToNetwork = mBuilder->distanceArea()->measureLine( startPoint, snappedStartPoint );
    }
    catch ( QgsCsException & )
    {
      throw QgsProcessingException( QObject::tr( "An error occurred while calculating length" ) );
    }


    if ( distancePointToNetwork > pointDistanceThreshold )
    {
      throw QgsProcessingException( QObject::tr( "Point is too far from the network layer (%1, maximum permitted is %2)" ).arg( distancePointToNetwork ).arg( pointDistanceThreshold ) );
    }
  }

  feedback->pushInfo( QObject::tr( "Calculating service area…" ) );
  std::unique_ptr<QgsGraph> graph( mBuilder->takeGraph() );
  const int idxStart = graph->findVertex( snappedStartPoint );

  QVector<int> tree;
  QVector<double> costs;
  QgsGraphAnalyzer::dijkstra( graph.get(), idxStart, 0, &tree, &costs );

  QgsMultiPointXY points;
  QgsMultiPolylineXY lines;
  QSet<int> vertices;

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
    const QList<int> outgoingEdges = graph->vertex( i ).outgoingEdges();
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
        const QgsPointXY interpolatedEndPoint = QgsGeometryUtils::interpolatePointOnLineByValue( edgeStart.x(), edgeStart.y(), startVertexCost, edgeEnd.x(), edgeEnd.y(), endVertexCost, travelCost );

        points.push_back( interpolatedEndPoint );
        lines.push_back( QgsPolylineXY() << edgeStart << interpolatedEndPoint );
      }
    } // edges
  } // costs

  // convert to list and sort to maintain same order of points between algorithm runs
  QList<int> verticesList = qgis::setToList( vertices );
  points.reserve( verticesList.size() );
  std::sort( verticesList.begin(), verticesList.end() );
  for ( const int v : verticesList )
  {
    points.push_back( graph->vertex( v ).point() );
  }

  feedback->pushInfo( QObject::tr( "Writing results…" ) );

  QVariantMap outputs;

  QgsFields fields;
  fields.append( QgsField( u"type"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"start"_s, QMetaType::Type::QString ) );

  QgsFeature feat;
  feat.setFields( fields );

  QString pointsSinkId;
  std::unique_ptr<QgsFeatureSink> pointsSink( parameterAsSink( parameters, u"OUTPUT"_s, context, pointsSinkId, fields, Qgis::WkbType::MultiPoint, mNetwork->sourceCrs() ) );

  if ( pointsSink )
  {
    outputs.insert( u"OUTPUT"_s, pointsSinkId );

    const QgsGeometry geomPoints = QgsGeometry::fromMultiPointXY( points );
    feat.setGeometry( geomPoints );
    feat.setAttributes( QgsAttributes() << u"within"_s << startPoint.toString() );
    if ( !pointsSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, u"OUTPUT"_s ) );

    if ( includeBounds )
    {
      QgsMultiPointXY upperBoundary, lowerBoundary;
      QVector<int> nodes;

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
      feat.setAttributes( QgsAttributes() << u"upper"_s << startPoint.toString() );
      if ( !pointsSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, u"OUTPUT"_s ) );

      feat.setGeometry( geomLower );
      feat.setAttributes( QgsAttributes() << u"lower"_s << startPoint.toString() );
      if ( !pointsSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, u"OUTPUT"_s ) );
    } // includeBounds

    pointsSink->finalize();
  }

  QString linesSinkId;
  std::unique_ptr<QgsFeatureSink> linesSink( parameterAsSink( parameters, u"OUTPUT_LINES"_s, context, linesSinkId, fields, Qgis::WkbType::MultiLineString, mNetwork->sourceCrs() ) );

  if ( linesSink )
  {
    outputs.insert( u"OUTPUT_LINES"_s, linesSinkId );
    const QgsGeometry geomLines = QgsGeometry::fromMultiPolylineXY( lines );
    feat.setGeometry( geomLines );
    feat.setAttributes( QgsAttributes() << u"lines"_s << startPoint.toString() );
    if ( !linesSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( linesSink.get(), parameters, u"OUTPUT_LINES"_s ) );
    linesSink->finalize();
  }

  return outputs;
}

///@endcond
