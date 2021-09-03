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

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Shortest path" ), QgsProcessing::TypeVectorLine ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TRAVEL_COST" ), QObject::tr( "Travel cost" ) ) );
}

QVariantMap QgsShortestPathPointToPointAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  loadCommonParams( parameters, context, feedback );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "start" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "end" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "cost" ), QVariant::Double ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::LineString, mNetwork->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  const QgsPointXY startPoint = parameterAsPoint( parameters, QStringLiteral( "START_POINT" ), context, mNetwork->sourceCrs() );
  const QgsPointXY endPoint = parameterAsPoint( parameters, QStringLiteral( "END_POINT" ), context, mNetwork->sourceCrs() );

  feedback->pushInfo( QObject::tr( "Building graph…" ) );
  QVector< QgsPointXY > points;
  points << startPoint << endPoint;
  QVector< QgsPointXY > snappedPoints;
  mDirector->makeGraph( mBuilder.get(), points, snappedPoints, feedback );

  feedback->pushInfo( QObject::tr( "Calculating shortest path…" ) );
  std::unique_ptr< QgsGraph > graph( mBuilder->takeGraph() );
  const int idxStart = graph->findVertex( snappedPoints[0] );
  int idxEnd = graph->findVertex( snappedPoints[1] );

  QVector< int > tree;
  QVector< double > costs;
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

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  outputs.insert( QStringLiteral( "TRAVEL_COST" ), cost / mMultiplier );
  return outputs;
}

///@endcond
