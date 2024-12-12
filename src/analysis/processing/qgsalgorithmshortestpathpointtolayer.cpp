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

Qgis::ProcessingAlgorithmDocumentationFlags QgsShortestPathPointToLayerAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsShortestPathPointToLayerAlgorithm *QgsShortestPathPointToLayerAlgorithm::createInstance() const
{
  return new QgsShortestPathPointToLayerAlgorithm();
}

void QgsShortestPathPointToLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParams();
  addParameter( new QgsProcessingParameterPoint( QStringLiteral( "START_POINT" ), QObject::tr( "Start point" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "END_POINTS" ), QObject::tr( "Vector layer with end points" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) ) );

  std::unique_ptr<QgsProcessingParameterNumber> maxEndPointDistanceFromNetwork = std::make_unique<QgsProcessingParameterDistance>( QStringLiteral( "POINT_TOLERANCE" ), QObject::tr( "Maximum point distance from network" ), QVariant(), QStringLiteral( "INPUT" ), true, 0 );
  maxEndPointDistanceFromNetwork->setFlags( maxEndPointDistanceFromNetwork->flags() | Qgis::ProcessingParameterFlag::Advanced );
  maxEndPointDistanceFromNetwork->setHelp( QObject::tr( "Specifies an optional limit on the distance from the start and end points to the network layer.If the start point is further from the network than this distance an error will be raised. If the end feature is further from the network than this distance it will be treated as non-routable." ) );
  addParameter( maxEndPointDistanceFromNetwork.release() );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Shortest path" ), Qgis::ProcessingSourceType::VectorLine ) );

  std::unique_ptr<QgsProcessingParameterFeatureSink> outputNonRoutable = std::make_unique<QgsProcessingParameterFeatureSink>( QStringLiteral( "OUTPUT_NON_ROUTABLE" ), QObject::tr( "Non-routable features" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true );
  outputNonRoutable->setHelp( QObject::tr( "An optional output which will be used to store any input features which could not be routed (e.g. those which are too far from the network layer)." ) );
  outputNonRoutable->setCreateByDefault( false );
  addParameter( outputNonRoutable.release() );
}

QVariantMap QgsShortestPathPointToLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  loadCommonParams( parameters, context, feedback );

  const QgsPointXY startPoint = parameterAsPoint( parameters, QStringLiteral( "START_POINT" ), context, mNetwork->sourceCrs() );

  std::unique_ptr<QgsFeatureSource> endPoints( parameterAsSource( parameters, QStringLiteral( "END_POINTS" ), context ) );
  if ( !endPoints )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "END_POINTS" ) ) );

  QgsFields fields = endPoints->fields();
  fields.append( QgsField( QStringLiteral( "start" ), QMetaType::Type::QString ) );
  fields.append( QgsField( QStringLiteral( "end" ), QMetaType::Type::QString ) );
  fields.append( QgsField( QStringLiteral( "cost" ), QMetaType::Type::Double ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, Qgis::WkbType::LineString, mNetwork->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QString nonRoutableSinkId;
  std::unique_ptr<QgsFeatureSink> nonRoutableSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT_NON_ROUTABLE" ), context, nonRoutableSinkId, endPoints->fields(), Qgis::WkbType::Point, mNetwork->sourceCrs() ) );

  const double pointDistanceThreshold = parameters.value( QStringLiteral( "POINT_TOLERANCE" ) ).isValid() ? parameterAsDouble( parameters, QStringLiteral( "POINT_TOLERANCE" ), context ) : -1;

  QVector<QgsPointXY> points;
  points.push_front( startPoint );
  QHash<int, QgsAttributes> sourceAttributes;
  loadPoints( endPoints.get(), points, sourceAttributes, context, feedback );

  feedback->pushInfo( QObject::tr( "Building graph…" ) );
  QVector<QgsPointXY> snappedPoints;
  mDirector->makeGraph( mBuilder.get(), points, snappedPoints, feedback );

  const QgsPointXY snappedStartPoint = snappedPoints[0];

  if ( pointDistanceThreshold >= 0 )
  {
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
  }

  feedback->pushInfo( QObject::tr( "Calculating shortest paths…" ) );
  std::unique_ptr<QgsGraph> graph( mBuilder->takeGraph() );
  const int idxStart = graph->findVertex( snappedStartPoint );
  int idxEnd;

  QVector<int> tree;
  QVector<double> costs;
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
          attributes = sourceAttributes.value( i );
          feat.setAttributes( attributes );
          if ( !nonRoutableSink->addFeature( feat, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( nonRoutableSink.get(), parameters, QStringLiteral( "OUTPUT_NON_ROUTABLE" ) ) );
        }

        feedback->setProgress( i * step );
        continue;
      }
    }

    idxEnd = graph->findVertex( snappedPoint );
    if ( tree.at( idxEnd ) == -1 )
    {
      feedback->reportError( QObject::tr( "There is no route from start point (%1) to end point (%2)." )
                               .arg( startPoint.toString(), originalPoint.toString() ) );
      feat.clearGeometry();
      attributes = sourceAttributes.value( i );
      attributes.append( QVariant() );
      attributes.append( originalPoint.toString() );
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
    attributes.append( originalPoint.toString() );
    attributes.append( cost / mMultiplier );
    feat.setAttributes( attributes );
    feat.setGeometry( geom );
    if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

    feedback->setProgress( i * step );
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  if ( nonRoutableSink )
  {
    nonRoutableSink->finalize();
    outputs.insert( QStringLiteral( "OUTPUT_NON_ROUTABLE" ), nonRoutableSinkId );
  }
  return outputs;
}

///@endcond
