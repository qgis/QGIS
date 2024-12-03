/***************************************************************************
                         qgsalgorithmshortestpathpointtopoint.cpp
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

#include "qgsalgorithmshortestpathpointtopoint.h"

#include "qgsgraphanalyzer.h"

///@cond PRIVATE

QString QgsShortestPathPointToPointAlgorithm::name() const
{
  return QStringLiteral( "shortestpathpointtopoint" );
}

QString QgsShortestPathPointToPointAlgorithm::displayName() const
{
  return QObject::tr( "Shortest path (point to point)" );
}

QStringList QgsShortestPathPointToPointAlgorithm::tags() const
{
  return QObject::tr( "network,path,shortest,fastest" ).split( ',' );
}

QString QgsShortestPathPointToPointAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes optimal (shortest or fastest) route between given start and end points." );
}

QgsShortestPathPointToPointAlgorithm *QgsShortestPathPointToPointAlgorithm::createInstance() const
{
  return new QgsShortestPathPointToPointAlgorithm();
}

void QgsShortestPathPointToPointAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParams();
  addParameter( new QgsProcessingParameterPoint( QStringLiteral( "START_POINT" ), QObject::tr( "Start point" ) ) );
  addParameter( new QgsProcessingParameterPoint( QStringLiteral( "END_POINT" ), QObject::tr( "End point" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Shortest path" ), Qgis::ProcessingSourceType::VectorLine ) );

  std::unique_ptr<QgsProcessingParameterNumber> maxEndPointDistanceFromNetwork = std::make_unique<QgsProcessingParameterDistance>( QStringLiteral( "POINT_TOLERANCE" ), QObject::tr( "Maximum point distance from network" ), QVariant(), QStringLiteral( "INPUT" ), true, 0 );
  maxEndPointDistanceFromNetwork->setFlags( maxEndPointDistanceFromNetwork->flags() | Qgis::ProcessingParameterFlag::Advanced );
  maxEndPointDistanceFromNetwork->setHelp( QObject::tr( "Specifies an optional limit on the distance from the start and end points to the network layer. If either point is further from the network than this distance an error will be raised." ) );
  addParameter( maxEndPointDistanceFromNetwork.release() );

  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TRAVEL_COST" ), QObject::tr( "Travel cost" ) ) );
}

QVariantMap QgsShortestPathPointToPointAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  loadCommonParams( parameters, context, feedback );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "start" ), QMetaType::Type::QString ) );
  fields.append( QgsField( QStringLiteral( "end" ), QMetaType::Type::QString ) );
  fields.append( QgsField( QStringLiteral( "cost" ), QMetaType::Type::Double ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, Qgis::WkbType::LineString, mNetwork->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  const QgsPointXY startPoint = parameterAsPoint( parameters, QStringLiteral( "START_POINT" ), context, mNetwork->sourceCrs() );
  const QgsPointXY endPoint = parameterAsPoint( parameters, QStringLiteral( "END_POINT" ), context, mNetwork->sourceCrs() );

  feedback->pushInfo( QObject::tr( "Building graph…" ) );
  QVector<QgsPointXY> snappedPoints;
  mDirector->makeGraph( mBuilder.get(), { startPoint, endPoint }, snappedPoints, feedback );
  const QgsPointXY snappedStartPoint = snappedPoints[0];
  const QgsPointXY snappedEndPoint = snappedPoints[1];

  // check distance for the snapped start/end points
  if ( parameters.value( QStringLiteral( "POINT_TOLERANCE" ) ).isValid() )
  {
    const double pointDistanceThreshold = parameterAsDouble( parameters, QStringLiteral( "POINT_TOLERANCE" ), context );

    double distanceStartPointToNetwork = 0;
    try
    {
      distanceStartPointToNetwork = mBuilder->distanceArea()->measureLine( startPoint, snappedStartPoint );
    }
    catch ( QgsCsException & )
    {
      throw QgsProcessingException( QObject::tr( "An error occurred while calculating length" ) );
    }

    if ( distanceStartPointToNetwork > pointDistanceThreshold )
    {
      throw QgsProcessingException( QObject::tr( "Start point is too far from the network layer (%1, maximum permitted is %2)" ).arg( distanceStartPointToNetwork ).arg( pointDistanceThreshold ) );
    }

    double distanceEndPointToNetwork = 0;
    try
    {
      distanceEndPointToNetwork = mBuilder->distanceArea()->measureLine( endPoint, snappedEndPoint );
    }
    catch ( QgsCsException & )
    {
      throw QgsProcessingException( QObject::tr( "An error occurred while calculating length" ) );
    }

    if ( distanceEndPointToNetwork > pointDistanceThreshold )
    {
      throw QgsProcessingException( QObject::tr( "End point is too far from the network layer (%1, maximum permitted is %2)" ).arg( distanceEndPointToNetwork ).arg( pointDistanceThreshold ) );
    }
  }

  feedback->pushInfo( QObject::tr( "Calculating shortest path…" ) );
  std::unique_ptr<QgsGraph> graph( mBuilder->takeGraph() );

  const int idxStart = graph->findVertex( snappedStartPoint );
  int idxEnd = graph->findVertex( snappedEndPoint );

  QVector<int> tree;
  QVector<double> costs;
  QgsGraphAnalyzer::dijkstra( graph.get(), idxStart, 0, &tree, &costs );

  if ( tree.at( idxEnd ) == -1 )
  {
    throw QgsProcessingException( QObject::tr( "There is no route from start point to end point." ) );
  }

  QVector<QgsPointXY> route;
  route.push_front( graph->vertex( idxEnd ).point() );
  const double cost = costs.at( idxEnd );
  while ( idxEnd != idxStart )
  {
    idxEnd = graph->edge( tree.at( idxEnd ) ).fromVertex();
    route.push_front( graph->vertex( idxEnd ).point() );
  }

  feedback->pushInfo( QObject::tr( "Writing results…" ) );
  const QgsGeometry geom = QgsGeometry::fromPolylineXY( route );
  QgsFeature feat;
  feat.setFields( fields );
  QgsAttributes attributes;
  attributes << startPoint.toString() << endPoint.toString() << cost / mMultiplier;
  feat.setGeometry( geom );
  feat.setAttributes( attributes );
  if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
    throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  outputs.insert( QStringLiteral( "TRAVEL_COST" ), cost / mMultiplier );
  return outputs;
}

///@endcond
