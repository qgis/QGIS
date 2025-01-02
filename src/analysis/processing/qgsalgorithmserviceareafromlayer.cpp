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
  return QObject::tr( "This algorithm creates a new vector with all the edges or parts of "
                      "edges of a network line layer that can be reached within a distance "
                      "or a time, starting from features of a point layer. The distance and "
                      "the time (both referred to as \"travel cost\") must be specified "
                      "respectively in the network layer units or in hours." );
}

QgsServiceAreaFromLayerAlgorithm *QgsServiceAreaFromLayerAlgorithm::createInstance() const
{
  return new QgsServiceAreaFromLayerAlgorithm();
}

void QgsServiceAreaFromLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParams();
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "START_POINTS" ), QObject::tr( "Vector layer with start points" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) ) );

  std::unique_ptr<QgsProcessingParameterNumber> travelCost = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "TRAVEL_COST" ), QObject::tr( "Travel cost (distance for 'Shortest', time for 'Fastest')" ), Qgis::ProcessingNumberParameterType::Double, 0, true, 0 );
  travelCost->setFlags( travelCost->flags() | Qgis::ProcessingParameterFlag::Hidden );
  addParameter( travelCost.release() );

  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TRAVEL_COST2" ), QObject::tr( "Travel cost (distance for 'Shortest', time for 'Fastest')" ), Qgis::ProcessingNumberParameterType::Double, 0, false, 0 ) );

  std::unique_ptr<QgsProcessingParameterBoolean> includeBounds = std::make_unique<QgsProcessingParameterBoolean>( QStringLiteral( "INCLUDE_BOUNDS" ), QObject::tr( "Include upper/lower bound points" ), false, true );
  includeBounds->setFlags( includeBounds->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( includeBounds.release() );

  std::unique_ptr<QgsProcessingParameterNumber> maxPointDistanceFromNetwork = std::make_unique<QgsProcessingParameterDistance>( QStringLiteral( "POINT_TOLERANCE" ), QObject::tr( "Maximum point distance from network" ), QVariant(), QStringLiteral( "INPUT" ), true, 0 );
  maxPointDistanceFromNetwork->setFlags( maxPointDistanceFromNetwork->flags() | Qgis::ProcessingParameterFlag::Advanced );
  maxPointDistanceFromNetwork->setHelp( QObject::tr( "Specifies an optional limit on the distance from the points to the network layer. If a point is further from the network than this distance it will be treated as non-routable." ) );
  addParameter( maxPointDistanceFromNetwork.release() );

  std::unique_ptr<QgsProcessingParameterFeatureSink> outputLines = std::make_unique<QgsProcessingParameterFeatureSink>( QStringLiteral( "OUTPUT_LINES" ), QObject::tr( "Service area (lines)" ), Qgis::ProcessingSourceType::VectorLine, QVariant(), true );
  outputLines->setCreateByDefault( true );
  addParameter( outputLines.release() );

  std::unique_ptr<QgsProcessingParameterFeatureSink> outputPoints = std::make_unique<QgsProcessingParameterFeatureSink>( QStringLiteral( "OUTPUT" ), QObject::tr( "Service area (boundary nodes)" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true );
  outputPoints->setCreateByDefault( false );
  addParameter( outputPoints.release() );

  std::unique_ptr<QgsProcessingParameterFeatureSink> outputNonRoutable = std::make_unique<QgsProcessingParameterFeatureSink>( QStringLiteral( "OUTPUT_NON_ROUTABLE" ), QObject::tr( "Non-routable features" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true );
  outputNonRoutable->setHelp( QObject::tr( "An optional output which will be used to store any input features which could not be routed (e.g. those which are too far from the network layer)." ) );
  outputNonRoutable->setCreateByDefault( false );
  addParameter( outputNonRoutable.release() );
}

QVariantMap QgsServiceAreaFromLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  loadCommonParams( parameters, context, feedback );

  std::unique_ptr<QgsFeatureSource> startPoints( parameterAsSource( parameters, QStringLiteral( "START_POINTS" ), context ) );
  if ( !startPoints )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "START_POINTS" ) ) );

  // use older deprecated travel cost style if specified, to maintain old api
  const bool useOldTravelCost = parameters.value( QStringLiteral( "TRAVEL_COST" ) ).isValid();
  double travelCost = parameterAsDouble( parameters, useOldTravelCost ? QStringLiteral( "TRAVEL_COST" ) : QStringLiteral( "TRAVEL_COST2" ), context );

  int strategy = parameterAsInt( parameters, QStringLiteral( "STRATEGY" ), context );
  if ( strategy && !useOldTravelCost )
    travelCost *= mMultiplier;

  bool includeBounds = true; // default to true to maintain 3.0 API
  if ( parameters.contains( QStringLiteral( "INCLUDE_BOUNDS" ) ) )
  {
    includeBounds = parameterAsBool( parameters, QStringLiteral( "INCLUDE_BOUNDS" ), context );
  }

  QVector<QgsPointXY> points;
  QHash<int, QgsAttributes> sourceAttributes;
  loadPoints( startPoints.get(), points, sourceAttributes, context, feedback );

  feedback->pushInfo( QObject::tr( "Building graph…" ) );
  QVector<QgsPointXY> snappedPoints;
  mDirector->makeGraph( mBuilder.get(), points, snappedPoints, feedback );

  feedback->pushInfo( QObject::tr( "Calculating service areas…" ) );
  std::unique_ptr<QgsGraph> graph( mBuilder->takeGraph() );

  QgsFields fields = startPoints->fields();
  fields.append( QgsField( QStringLiteral( "type" ), QMetaType::Type::QString ) );
  fields.append( QgsField( QStringLiteral( "start" ), QMetaType::Type::QString ) );

  QString pointsSinkId;
  std::unique_ptr<QgsFeatureSink> pointsSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, pointsSinkId, fields, Qgis::WkbType::MultiPoint, mNetwork->sourceCrs() ) );

  QString linesSinkId;
  std::unique_ptr<QgsFeatureSink> linesSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT_LINES" ), context, linesSinkId, fields, Qgis::WkbType::MultiLineString, mNetwork->sourceCrs() ) );

  QString nonRoutableSinkId;
  std::unique_ptr<QgsFeatureSink> nonRoutableSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT_NON_ROUTABLE" ), context, nonRoutableSinkId, startPoints->fields(), Qgis::WkbType::Point, mNetwork->sourceCrs() ) );

  const double pointDistanceThreshold = parameters.value( QStringLiteral( "POINT_TOLERANCE" ) ).isValid() ? parameterAsDouble( parameters, QStringLiteral( "POINT_TOLERANCE" ), context ) : -1;

  int idxStart;
  QVector<int> tree;
  QVector<double> costs;

  int inboundEdgeIndex;
  double startVertexCost, endVertexCost;
  QgsPointXY startPoint, endPoint;
  QgsGraphEdge edge;

  QgsFeature feat;
  QgsAttributes attributes;

  const double step = snappedPoints.size() > 0 ? 100.0 / snappedPoints.size() : 1;
  for ( int i = 0; i < snappedPoints.size(); i++ )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    const QgsPointXY snappedPoint = snappedPoints.at( i );
    const QgsPointXY originalPoint = points.at( i );

    if ( pointDistanceThreshold >= 0 )
    {
      double distancePointToNetwork = 0;
      try
      {
        distancePointToNetwork = mBuilder->distanceArea()->measureLine( originalPoint, snappedPoint );
      }
      catch ( QgsCsException & )
      {
        throw QgsProcessingException( QObject::tr( "An error occurred while calculating length" ) );
      }

      if ( distancePointToNetwork > pointDistanceThreshold )
      {
        feedback->pushWarning( QObject::tr( "Point is too far from the network layer (%1, maximum permitted is %2)" ).arg( distancePointToNetwork ).arg( pointDistanceThreshold ) );
        if ( nonRoutableSink )
        {
          feat.setGeometry( QgsGeometry::fromPointXY( originalPoint ) );
          attributes = sourceAttributes.value( i + 1 );
          feat.setAttributes( attributes );
          if ( !nonRoutableSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( nonRoutableSink.get(), parameters, QStringLiteral( "OUTPUT_NON_ROUTABLE" ) ) );
        }

        feedback->setProgress( i * step );
        continue;
      }
    }

    const QString originalPointString = originalPoint.toString();

    idxStart = graph->findVertex( snappedPoint );

    QgsGraphAnalyzer::dijkstra( graph.get(), idxStart, 0, &tree, &costs );

    QgsMultiPointXY areaPoints;
    QgsMultiPolylineXY lines;
    QSet<int> vertices;

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
      const QList<int> outgoingEdges = graph->vertex( j ).outgoingEdges();
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
          QgsPointXY interpolatedEndPoint = QgsGeometryUtils::interpolatePointOnLineByValue( startPoint.x(), startPoint.y(), startVertexCost, endPoint.x(), endPoint.y(), endVertexCost, travelCost );

          areaPoints.push_back( interpolatedEndPoint );
          lines.push_back( QgsPolylineXY() << startPoint << interpolatedEndPoint );
        }
      } // edges
    } // costs

    // convert to list and sort to maintain same order of points between algorithm runs
    QList<int> verticesList = qgis::setToList( vertices );
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
      attributes << QStringLiteral( "within" ) << originalPointString;
      feat.setAttributes( attributes );
      if ( !pointsSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

      if ( includeBounds )
      {
        QgsMultiPointXY upperBoundary, lowerBoundary;
        QVector<int> nodes;
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

        upperBoundary.reserve( nodes.size() );
        lowerBoundary.reserve( nodes.size() );
        for ( int n : std::as_const( nodes ) )
        {
          upperBoundary.push_back( graph->vertex( graph->edge( tree.at( n ) ).toVertex() ).point() );
          lowerBoundary.push_back( graph->vertex( graph->edge( tree.at( n ) ).fromVertex() ).point() );
        } // nodes

        QgsGeometry geomUpper = QgsGeometry::fromMultiPointXY( upperBoundary );
        QgsGeometry geomLower = QgsGeometry::fromMultiPointXY( lowerBoundary );

        feat.setGeometry( geomUpper );
        attributes = sourceAttributes.value( i + 1 );
        attributes << QStringLiteral( "upper" ) << originalPointString;
        feat.setAttributes( attributes );
        if ( !pointsSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

        feat.setGeometry( geomLower );
        attributes = sourceAttributes.value( i + 1 );
        attributes << QStringLiteral( "lower" ) << originalPointString;
        feat.setAttributes( attributes );
        if ( !pointsSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      } // includeBounds
    }

    if ( linesSink )
    {
      QgsGeometry geomLines = QgsGeometry::fromMultiPolylineXY( lines );
      feat.setGeometry( geomLines );
      attributes = sourceAttributes.value( i + 1 );
      attributes << QStringLiteral( "lines" ) << originalPointString;
      feat.setAttributes( attributes );
      if ( !linesSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( linesSink.get(), parameters, QStringLiteral( "OUTPUT_LINES" ) ) );
    }

    feedback->setProgress( i * step );
  } // snappedPoints

  QVariantMap outputs;
  if ( pointsSink )
  {
    pointsSink->finalize();
    outputs.insert( QStringLiteral( "OUTPUT" ), pointsSinkId );
  }
  if ( linesSink )
  {
    linesSink->finalize();
    outputs.insert( QStringLiteral( "OUTPUT_LINES" ), linesSinkId );
  }
  if ( nonRoutableSink )
  {
    nonRoutableSink->finalize();
    outputs.insert( QStringLiteral( "OUTPUT_NON_ROUTABLE" ), nonRoutableSinkId );
  }

  return outputs;
}

///@endcond
