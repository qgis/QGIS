/***************************************************************************
                         qgsalgorithmshortestpathlayertopoint.cpp
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

#include "qgsalgorithmshortestpathlayertopoint.h"

#include "qgsgraphanalyzer.h"

#include "qgsmessagelog.h"

///@cond PRIVATE

QString QgsShortestPathLayerToPointAlgorithm::name() const
{
  return QStringLiteral( "shortestpathlayertopoint" );
}

QString QgsShortestPathLayerToPointAlgorithm::displayName() const
{
  return QObject::tr( "Shortest path (layer to point)" );
}

QStringList QgsShortestPathLayerToPointAlgorithm::tags() const
{
  return QObject::tr( "network,path,shortest,fastest" ).split( ',' );
}

QString QgsShortestPathLayerToPointAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes optimal (shortest or fastest) route from multiple start points defined by vector layer and given end point." );
}

QgsShortestPathLayerToPointAlgorithm *QgsShortestPathLayerToPointAlgorithm::createInstance() const
{
  return new QgsShortestPathLayerToPointAlgorithm();
}

void QgsShortestPathLayerToPointAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParams();
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "START_POINTS" ), QObject::tr( "Vector layer with start points" ), QList< int >() << QgsProcessing::TypeVectorPoint ) );
  addParameter( new QgsProcessingParameterPoint( QStringLiteral( "END_POINT" ), QObject::tr( "End point" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Shortest path" ), QgsProcessing::TypeVectorLine ) );
}

QVariantMap QgsShortestPathLayerToPointAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  loadCommonParams( parameters, context, feedback );

  const QgsPointXY endPoint = parameterAsPoint( parameters, QStringLiteral( "END_POINT" ), context, mNetwork->sourceCrs() );

  std::unique_ptr< QgsFeatureSource > startPoints( parameterAsSource( parameters, QStringLiteral( "START_POINTS" ), context ) );
  if ( !startPoints )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "START_POINTS" ) ) );

  QgsFields fields = startPoints->fields();
  fields.append( QgsField( QStringLiteral( "start" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "end" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "cost" ), QVariant::Double ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::LineString, mNetwork->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QVector< QgsPointXY > points;
  points.push_front( endPoint );
  QHash< int, QgsAttributes > sourceAttributes;
  loadPoints( startPoints.get(), points, sourceAttributes, context, feedback );

  feedback->pushInfo( QObject::tr( "Building graph…" ) );
  QVector< QgsPointXY > snappedPoints;
  mDirector->makeGraph( mBuilder.get(), points, snappedPoints, feedback );

  feedback->pushInfo( QObject::tr( "Calculating shortest paths…" ) );
  std::unique_ptr< QgsGraph > graph( mBuilder->takeGraph() );
  const int idxEnd = graph->findVertex( snappedPoints[0] );
  int idxStart;
  int currentIdx;

  QVector< int > tree;
  QVector< double > costs;

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

    idxStart = graph->findVertex( snappedPoints[i] );
    QgsGraphAnalyzer::dijkstra( graph.get(), idxStart, 0, &tree, &costs );

    if ( tree.at( idxEnd ) == -1 )
    {
      feedback->reportError( QObject::tr( "There is no route from start point (%1) to end point (%2)." )
                             .arg( points[i].toString(),
                                   endPoint.toString() ) );
      feat.clearGeometry();
      attributes = sourceAttributes.value( i );
      attributes.append( points[i].toString() );
      feat.setAttributes( attributes );
      if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      continue;
    }

    route.clear();
    route.push_front( graph->vertex( idxEnd ).point() );
    cost = costs.at( idxEnd );
    currentIdx = idxEnd;
    while ( currentIdx != idxStart )
    {
      currentIdx = graph->edge( tree.at( currentIdx ) ).fromVertex();
      route.push_front( graph->vertex( currentIdx ).point() );
    }

    const QgsGeometry geom = QgsGeometry::fromPolylineXY( route );
    QgsFeature feat;
    feat.setFields( fields );
    attributes = sourceAttributes.value( i );
    attributes.append( points[i].toString() );
    attributes.append( endPoint.toString() );
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
