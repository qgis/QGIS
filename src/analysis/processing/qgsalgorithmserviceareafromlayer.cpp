/***************************************************************************
                         qgsalgorithmserviceareafromlayer.cpp
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

#include "qgsalgorithmserviceareafromlayer.h"

#include "qgsgeometryutils.h"
#include "qgsgraphanalyzer.h"

///@cond PRIVATE

QString QgsServiceAreaFromLayerAlgorithm::name() const
{
  return QStringLiteral( "serviceareafromlayer" );
}

QString QgsServiceAreaFromLayerAlgorithm::displayName() const
{
  return QObject::tr( "Service area (from layer)" );
}

QStringList QgsServiceAreaFromLayerAlgorithm::tags() const
{
  return QObject::tr( "network,service,area,shortest,fastest" ).split( ',' );
}

QString QgsServiceAreaFromLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector with all the edges or parts of edges of a network line layer that can be reached within a distance or a time, starting from features of a point layer. The distance and the time (both referred to as \"travel cost\") must be specified respectively in the network layer units or in seconds." );
}

QgsServiceAreaFromLayerAlgorithm *QgsServiceAreaFromLayerAlgorithm::createInstance() const
{
  return new QgsServiceAreaFromLayerAlgorithm();
}

void QgsServiceAreaFromLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParams();
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "START_POINTS" ), QObject::tr( "Vector layer with start points" ), QList< int >() << QgsProcessing::TypeVectorPoint ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TRAVEL_COST" ), QObject::tr( "Travel cost (distance for 'Shortest', time for 'Fastest')" ),
                QgsProcessingParameterNumber::Double, 0, false, 0 ) );

  std::unique_ptr< QgsProcessingParameterBoolean > includeBounds = qgis::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "INCLUDE_BOUNDS" ), QObject::tr( "Include upper/lower bound points" ), false, true );
  includeBounds->setFlags( includeBounds->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( includeBounds.release() );

  std::unique_ptr< QgsProcessingParameterFeatureSink > outputLines = qgis::make_unique< QgsProcessingParameterFeatureSink >( QStringLiteral( "OUTPUT_LINES" ),  QObject::tr( "Service area (lines)" ),
      QgsProcessing::TypeVectorLine, QVariant(), true );
  outputLines->setCreateByDefault( true );
  addParameter( outputLines.release() );

  std::unique_ptr< QgsProcessingParameterFeatureSink > outputPoints = qgis::make_unique< QgsProcessingParameterFeatureSink >( QStringLiteral( "OUTPUT" ),  QObject::tr( "Service area (boundary nodes)" ),
      QgsProcessing::TypeVectorPoint, QVariant(), true );
  outputPoints->setCreateByDefault( false );
  addParameter( outputPoints.release() );
}

QVariantMap QgsServiceAreaFromLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  loadCommonParams( parameters, context, feedback );

  std::unique_ptr< QgsFeatureSource > startPoints( parameterAsSource( parameters, QStringLiteral( "START_POINTS" ), context ) );
  if ( !startPoints )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "START_POINTS" ) ) );

  double travelCost = parameterAsDouble( parameters, QStringLiteral( "TRAVEL_COST" ), context );

  bool includeBounds = true;  // default to true to maintain 3.0 API
  if ( parameters.contains( QStringLiteral( "INCLUDE_BOUNDS" ) ) )
  {
    includeBounds = parameterAsBool( parameters, QStringLiteral( "INCLUDE_BOUNDS" ), context );
  }

  QVector< QgsPointXY > points;
  QHash< int, QgsAttributes > sourceAttributes;
  loadPoints( startPoints.get(), points, sourceAttributes, context, feedback );

  feedback->pushInfo( QObject::tr( "Building graph…" ) );
  QVector< QgsPointXY > snappedPoints;
  mDirector->makeGraph( mBuilder.get(), points, snappedPoints, feedback );

  feedback->pushInfo( QObject::tr( "Calculating service areas…" ) );
  QgsGraph *graph = mBuilder->graph();

  QgsFields fields = startPoints->fields();
  fields.append( QgsField( QStringLiteral( "type" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "start" ), QVariant::String ) );

  QString pointsSinkId;
  std::unique_ptr< QgsFeatureSink > pointsSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, pointsSinkId, fields,
      QgsWkbTypes::MultiPoint, mNetwork->sourceCrs() ) );

  QString linesSinkId;
  std::unique_ptr< QgsFeatureSink > linesSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT_LINES" ), context, linesSinkId, fields,
      QgsWkbTypes::MultiLineString, mNetwork->sourceCrs() ) );

  int idxStart;
  QString origPoint;
  QVector< int > tree;
  QVector< double > costs;

  int inboundEdgeIndex;
  double startVertexCost, endVertexCost;
  QgsPointXY startPoint, endPoint;
  QgsGraphEdge edge;

  QgsFeature feat;
  QgsAttributes attributes;

  int step =  snappedPoints.size() > 0 ? 100.0 / snappedPoints.size() : 1;
  for ( int i = 0; i < snappedPoints.size(); i++ )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    idxStart = graph->findVertex( snappedPoints.at( i ) );
    origPoint = points.at( i ).toString();

    QgsGraphAnalyzer::dijkstra( graph, idxStart, 0, &tree, &costs );

    QgsMultiPointXY areaPoints;
    QgsMultiPolylineXY lines;
    QSet< int > vertices;

    for ( int j = 0; j < costs.size(); j++ )
    {
      inboundEdgeIndex = tree.at( j );

      if ( inboundEdgeIndex == -1 && j != idxStart )
      {
        // unreachable vertex
        continue;
      }

      startVertexCost = costs.at( j );
      if ( startVertexCost > travelCost )
      {
        // vertex is too expensive, discard
        continue;
      }

      vertices.insert( j );
      startPoint = graph->vertex( j ).point();

      // find all edges coming from this vertex
      const QList< int > outgoingEdges = graph->vertex( j ).outgoingEdges() ;
      for ( int edgeId : outgoingEdges )
      {
        edge = graph->edge( edgeId );
        endVertexCost = startVertexCost + edge.cost( 0 ).toDouble();
        endPoint = graph->vertex( edge.toVertex() ).point();
        if ( endVertexCost <= travelCost )
        {
          // end vertex is cheap enough to include
          vertices.insert( edge.toVertex() );
          lines.push_back( QgsPolylineXY() << startPoint << endPoint );
        }
        else
        {
          // travelCost sits somewhere on this edge, interpolate position
          QgsPointXY interpolatedEndPoint = QgsGeometryUtils::interpolatePointOnLineByValue( startPoint.x(), startPoint.y(), startVertexCost,
                                            endPoint.x(), endPoint.y(), endVertexCost, travelCost );

          areaPoints.push_back( interpolatedEndPoint );
          lines.push_back( QgsPolylineXY() << startPoint << interpolatedEndPoint );
        }
      } // edges
    } // costs

    // convert to list and sort to maintain same order of points between algorithm runs
    QList< int > verticesList = vertices.toList();
    areaPoints.reserve( verticesList.size() );
    std::sort( verticesList.begin(), verticesList.end() );
    for ( int v : verticesList )
    {
      areaPoints.push_back( graph->vertex( v ).point() );
    }

    if ( pointsSink )
    {
      QgsGeometry geomPoints = QgsGeometry::fromMultiPointXY( areaPoints );
      feat.setGeometry( geomPoints );
      attributes = sourceAttributes.value( i + 1 );
      attributes << QStringLiteral( "within" ) << origPoint;
      feat.setAttributes( attributes );
      pointsSink->addFeature( feat, QgsFeatureSink::FastInsert );

      if ( includeBounds )
      {
        QgsMultiPointXY upperBoundary, lowerBoundary;
        QVector< int > nodes;
        nodes.reserve( costs.size() );

        int vertexId;
        for ( int v = 0; v < costs.size(); v++ )
        {
          if ( costs.at( v ) > travelCost && tree.at( v ) != -1 )
          {
            vertexId = graph->edge( tree.at( v ) ).fromVertex();
            if ( costs.at( vertexId ) <= travelCost )
            {
              nodes.push_back( v );
            }
          }
        } // costs

        for ( int n : qgis::as_const( nodes ) )
        {
          upperBoundary.push_back( graph->vertex( graph->edge( tree.at( n ) ).toVertex() ).point() );
          lowerBoundary.push_back( graph->vertex( graph->edge( tree.at( n ) ).fromVertex() ).point() );
        } // nodes

        QgsGeometry geomUpper = QgsGeometry::fromMultiPointXY( upperBoundary );
        QgsGeometry geomLower = QgsGeometry::fromMultiPointXY( lowerBoundary );

        feat.setGeometry( geomUpper );
        attributes = sourceAttributes.value( i );
        attributes << QStringLiteral( "upper" ) << origPoint;
        feat.setAttributes( attributes );
        pointsSink->addFeature( feat, QgsFeatureSink::FastInsert );

        feat.setGeometry( geomLower );
        attributes = sourceAttributes.value( i );
        attributes << QStringLiteral( "lower" ) << origPoint;
        feat.setAttributes( attributes );
        pointsSink->addFeature( feat, QgsFeatureSink::FastInsert );
      } // includeBounds
    }

    if ( linesSink )
    {
      QgsGeometry geomLines = QgsGeometry::fromMultiPolylineXY( lines );
      feat.setGeometry( geomLines );
      attributes = sourceAttributes.value( i );
      attributes << QStringLiteral( "lines" ) << origPoint;
      feat.setAttributes( attributes );
      linesSink->addFeature( feat, QgsFeatureSink::FastInsert );
    }

    feedback->setProgress( i * step );
  } // snappedPoints

  QVariantMap outputs;
  if ( pointsSink )
  {
    outputs.insert( QStringLiteral( "OUTPUT" ), pointsSinkId );
  }
  if ( linesSink )
  {
    outputs.insert( QStringLiteral( "OUTPUT_LINES" ), linesSinkId );
  }

  return outputs;
}

///@endcond
