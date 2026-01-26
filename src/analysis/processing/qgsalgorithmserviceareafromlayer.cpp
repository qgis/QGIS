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
  return u"serviceareafromlayer"_s;
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
  return QObject::tr( "This algorithm creates a new vector layer with all the edges or parts of "
                      "edges of a network line layer that can be reached within a distance "
                      "or a time, starting from features of a point layer. The distance and "
                      "the time (both referred to as \"travel cost\") must be specified "
                      "respectively in the network layer units or in hours." );
}

QString QgsServiceAreaFromLayerAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a vector layer with all the edges or parts of "
                      "edges of a network line layer that can be reached within a distance "
                      "or a time, starting from features of a point layer." );
}

QgsServiceAreaFromLayerAlgorithm *QgsServiceAreaFromLayerAlgorithm::createInstance() const
{
  return new QgsServiceAreaFromLayerAlgorithm();
}

void QgsServiceAreaFromLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParams();
  addParameter( new QgsProcessingParameterFeatureSource( u"START_POINTS"_s, QObject::tr( "Vector layer with start points" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) ) );

  auto travelCost = std::make_unique<QgsProcessingParameterNumber>( u"TRAVEL_COST"_s, QObject::tr( "Travel cost (distance for 'Shortest', time for 'Fastest')" ), Qgis::ProcessingNumberParameterType::Double, 0, true, 0 );
  travelCost->setFlags( travelCost->flags() | Qgis::ProcessingParameterFlag::Hidden );
  addParameter( std::move( travelCost ) );

  auto travelCost2 = std::make_unique<QgsProcessingParameterNumber>( u"TRAVEL_COST2"_s, QObject::tr( "Travel cost (distance for 'Shortest', time for 'Fastest')" ), Qgis::ProcessingNumberParameterType::Double, 0, false, 0 );
  travelCost2->setIsDynamic( true );
  travelCost2->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Travel Cost"_s, QObject::tr( "Travel cost (distance for 'Shortest', time for 'Fastest')" ), QgsPropertyDefinition::DoublePositive ) );
  travelCost2->setDynamicLayerParameterName( u"START_POINTS"_s );
  addParameter( std::move( travelCost2 ) );

  auto includeBounds = std::make_unique<QgsProcessingParameterBoolean>( u"INCLUDE_BOUNDS"_s, QObject::tr( "Include upper/lower bound points" ), false, true );
  includeBounds->setFlags( includeBounds->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( includeBounds.release() );

  std::unique_ptr<QgsProcessingParameterNumber> maxPointDistanceFromNetwork = std::make_unique<QgsProcessingParameterDistance>( u"POINT_TOLERANCE"_s, QObject::tr( "Maximum point distance from network" ), QVariant(), u"INPUT"_s, true, 0 );
  maxPointDistanceFromNetwork->setFlags( maxPointDistanceFromNetwork->flags() | Qgis::ProcessingParameterFlag::Advanced );
  maxPointDistanceFromNetwork->setHelp( QObject::tr( "Specifies an optional limit on the distance from the points to the network layer. If a point is further from the network than this distance it will be treated as non-routable." ) );
  addParameter( maxPointDistanceFromNetwork.release() );

  auto outputLines = std::make_unique<QgsProcessingParameterFeatureSink>( u"OUTPUT_LINES"_s, QObject::tr( "Service area (lines)" ), Qgis::ProcessingSourceType::VectorLine, QVariant(), true );
  outputLines->setCreateByDefault( true );
  addParameter( outputLines.release() );

  auto outputPoints = std::make_unique<QgsProcessingParameterFeatureSink>( u"OUTPUT"_s, QObject::tr( "Service area (boundary nodes)" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true );
  outputPoints->setCreateByDefault( false );
  addParameter( outputPoints.release() );

  auto outputNonRoutable = std::make_unique<QgsProcessingParameterFeatureSink>( u"OUTPUT_NON_ROUTABLE"_s, QObject::tr( "Non-routable features" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true );
  outputNonRoutable->setHelp( QObject::tr( "An optional output which will be used to store any input features which could not be routed (e.g. those which are too far from the network layer)." ) );
  outputNonRoutable->setCreateByDefault( false );
  addParameter( outputNonRoutable.release() );
}

QVariantMap QgsServiceAreaFromLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  loadCommonParams( parameters, context, feedback );

  std::unique_ptr<QgsProcessingFeatureSource> startPoints( parameterAsSource( parameters, u"START_POINTS"_s, context ) );
  if ( !startPoints )
    throw QgsProcessingException( invalidSourceError( parameters, u"START_POINTS"_s ) );

  // use older deprecated travel cost style if specified, to maintain old api
  const bool useOldTravelCost = parameters.value( u"TRAVEL_COST"_s ).isValid();
  const double defaultTravelCost = parameterAsDouble( parameters, useOldTravelCost ? u"TRAVEL_COST"_s : u"TRAVEL_COST2"_s, context );

  const bool dynamicTravelCost = QgsProcessingParameters::isDynamic( parameters, u"TRAVEL_COST2"_s );
  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, startPoints.get() );
  QgsProperty travelCostProperty;
  if ( dynamicTravelCost )
  {
    travelCostProperty = parameters.value( u"TRAVEL_COST2"_s ).value<QgsProperty>();
  }

  const int strategy = parameterAsInt( parameters, u"STRATEGY"_s, context );
  const double multiplier = ( strategy && !useOldTravelCost ) ? mMultiplier : 1;

  bool includeBounds = true; // default to true to maintain 3.0 API
  if ( parameters.contains( u"INCLUDE_BOUNDS"_s ) )
  {
    includeBounds = parameterAsBool( parameters, u"INCLUDE_BOUNDS"_s, context );
  }

  QVector<QgsPointXY> points;
  QHash<int, QgsFeature> sourceFeatures;
  loadPoints( startPoints.get(), &points, nullptr, context, feedback, &sourceFeatures );

  feedback->pushInfo( QObject::tr( "Building graph…" ) );
  QVector<QgsPointXY> snappedPoints;
  mDirector->makeGraph( mBuilder.get(), points, snappedPoints, feedback );

  feedback->pushInfo( QObject::tr( "Calculating service areas…" ) );
  std::unique_ptr<QgsGraph> graph( mBuilder->takeGraph() );

  QgsFields newFields;
  newFields.append( QgsField( u"type"_s, QMetaType::Type::QString ) );
  newFields.append( QgsField( u"start"_s, QMetaType::Type::QString ) );
  QgsFields fields = QgsProcessingUtils::combineFields( startPoints->fields(), newFields );

  QString pointsSinkId;
  std::unique_ptr<QgsFeatureSink> pointsSink( parameterAsSink( parameters, u"OUTPUT"_s, context, pointsSinkId, fields, Qgis::WkbType::MultiPoint, mNetwork->sourceCrs() ) );

  QString linesSinkId;
  std::unique_ptr<QgsFeatureSink> linesSink( parameterAsSink( parameters, u"OUTPUT_LINES"_s, context, linesSinkId, fields, Qgis::WkbType::MultiLineString, mNetwork->sourceCrs() ) );

  QString nonRoutableSinkId;
  std::unique_ptr<QgsFeatureSink> nonRoutableSink( parameterAsSink( parameters, u"OUTPUT_NON_ROUTABLE"_s, context, nonRoutableSinkId, startPoints->fields(), Qgis::WkbType::Point, mNetwork->sourceCrs() ) );

  const double pointDistanceThreshold = parameters.value( u"POINT_TOLERANCE"_s ).isValid() ? parameterAsDouble( parameters, u"POINT_TOLERANCE"_s, context ) : -1;

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

    double travelCost = defaultTravelCost;
    if ( dynamicTravelCost )
    {
      expressionContext.setFeature( sourceFeatures.value( i + 1 ) );
      travelCost = travelCostProperty.valueAsDouble( expressionContext, travelCost );
    }
    travelCost *= multiplier;

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
          attributes = sourceFeatures.value( i + 1 ).attributes();
          feat.setAttributes( attributes );
          if ( !nonRoutableSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( nonRoutableSink.get(), parameters, u"OUTPUT_NON_ROUTABLE"_s ) );
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
      attributes = sourceFeatures.value( i + 1 ).attributes();
      attributes << u"within"_s << originalPointString;
      feat.setAttributes( attributes );
      if ( !pointsSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, u"OUTPUT"_s ) );

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
        attributes = sourceFeatures.value( i + 1 ).attributes();
        attributes << u"upper"_s << originalPointString;
        feat.setAttributes( attributes );
        if ( !pointsSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, u"OUTPUT"_s ) );

        feat.setGeometry( geomLower );
        attributes = sourceFeatures.value( i + 1 ).attributes();
        attributes << u"lower"_s << originalPointString;
        feat.setAttributes( attributes );
        if ( !pointsSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, u"OUTPUT"_s ) );
      } // includeBounds
    }

    if ( linesSink )
    {
      QgsGeometry geomLines = QgsGeometry::fromMultiPolylineXY( lines );
      feat.setGeometry( geomLines );
      attributes = sourceFeatures.value( i + 1 ).attributes();
      attributes << u"lines"_s << originalPointString;
      feat.setAttributes( attributes );
      if ( !linesSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( linesSink.get(), parameters, u"OUTPUT_LINES"_s ) );
    }

    feedback->setProgress( i * step );
  } // snappedPoints

  QVariantMap outputs;
  if ( pointsSink )
  {
    pointsSink->finalize();
    outputs.insert( u"OUTPUT"_s, pointsSinkId );
  }
  if ( linesSink )
  {
    linesSink->finalize();
    outputs.insert( u"OUTPUT_LINES"_s, linesSinkId );
  }
  if ( nonRoutableSink )
  {
    nonRoutableSink->finalize();
    outputs.insert( u"OUTPUT_NON_ROUTABLE"_s, nonRoutableSinkId );
  }

  return outputs;
}

///@endcond
