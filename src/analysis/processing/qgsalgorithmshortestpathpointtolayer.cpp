/***************************************************************************
                         qgsalgorithmshortestpathpointtolayer.cpp
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

#include "qgsalgorithmshortestpathpointtolayer.h"

#include "qgsgraphanalyzer.h"

#include "qgsmessagelog.h"

///@cond PRIVATE

QString QgsShortestPathPointToLayerAlgorithm::name() const
{
  return QStringLiteral( "shortestpathpointtolayer" );
}

QString QgsShortestPathPointToLayerAlgorithm::displayName() const
{
  return QObject::tr( "Shortest path (point to layer)" );
}

QStringList QgsShortestPathPointToLayerAlgorithm::tags() const
{
  return QObject::tr( "network,path,shortest,fastest" ).split( ',' );
}

QString QgsShortestPathPointToLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes optimal (shortest or fastest) route between given start point and multiple end points defined by point vector layer." );
}

QgsShortestPathPointToLayerAlgorithm *QgsShortestPathPointToLayerAlgorithm::createInstance() const
{
  return new QgsShortestPathPointToLayerAlgorithm();
}

void QgsShortestPathPointToLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParams();
  addParameter( new QgsProcessingParameterPoint( QStringLiteral( "START_POINT" ), QObject::tr( "Start point" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "END_POINTS" ), QObject::tr( "Vector layer with end points" ), QList< int >() << QgsProcessing::TypeVectorPoint ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Shortest path" ), QgsProcessing::TypeVectorLine ) );
}

QVariantMap QgsShortestPathPointToLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  loadCommonParams( parameters, context, feedback );

  const QgsPointXY startPoint = parameterAsPoint( parameters, QStringLiteral( "START_POINT" ), context, mNetwork->sourceCrs() );

  std::unique_ptr< QgsFeatureSource > endPoints( parameterAsSource( parameters, QStringLiteral( "END_POINTS" ), context ) );
  if ( !endPoints )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "END_POINTS" ) ) );

  QgsFields fields = endPoints->fields();
  fields.append( QgsField( QStringLiteral( "start" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "end" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "cost" ), QVariant::Double ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::LineString, mNetwork->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QVector< QgsPointXY > points;
  points.push_front( startPoint );
  QHash< int, QgsAttributes > sourceAttributes;
  loadPoints( endPoints.get(), points, sourceAttributes, context, feedback );

  feedback->pushInfo( QObject::tr( "Building graph…" ) );
  QVector< QgsPointXY > snappedPoints;
  mDirector->makeGraph( mBuilder.get(), points, snappedPoints, feedback );

  feedback->pushInfo( QObject::tr( "Calculating shortest paths…" ) );
  std::unique_ptr< QgsGraph > graph( mBuilder->takeGraph() );
  const int idxStart = graph->findVertex( snappedPoints[0] );
  int idxEnd;

  QVector< int > tree;
  QVector< double > costs;
  QgsGraphAnalyzer::dijkstra( graph.get(), idxStart, 0, &tree, &costs );

  QVector<QgsPointXY> route;
  double cost;

  QgsFeature feat;
  feat.setFields( fields );
  QgsAttributes attributes;

  const double step = points.size() > 0 ? 100.0 / points.size() : 1;
  for ( int i = 1; i < points.size(); i++ )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    idxEnd = graph->findVertex( snappedPoints[i] );
    if ( tree.at( idxEnd ) == -1 )
    {
      feedback->reportError( QObject::tr( "There is no route from start point (%1) to end point (%2)." )
                             .arg( startPoint.toString(),
                                   points[i].toString() ) );
      feat.clearGeometry();
      attributes = sourceAttributes.value( i );
      attributes.append( QVariant() );
      attributes.append( points[i].toString() );
      feat.setAttributes( attributes );
      if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      continue;
    }

    route.clear();
    route.push_front( graph->vertex( idxEnd ).point() );
    cost = costs.at( idxEnd );
    while ( idxEnd != idxStart )
    {
      idxEnd = graph->edge( tree.at( idxEnd ) ).fromVertex();
      route.push_front( graph->vertex( idxEnd ).point() );
    }

    const QgsGeometry geom = QgsGeometry::fromPolylineXY( route );
    QgsFeature feat;
    feat.setFields( fields );
    attributes = sourceAttributes.value( i );
    attributes.append( startPoint.toString() );
    attributes.append( points[i].toString() );
    attributes.append( cost / mMultiplier );
    feat.setAttributes( attributes );
    feat.setGeometry( geom );
    if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

    feedback->setProgress( i * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
