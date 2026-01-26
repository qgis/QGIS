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
  return u"shortestpathlayertopoint"_s;
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
  return QObject::tr( "This algorithm computes optimal (shortest or fastest) routes "
                      "from multiple start points defined by a vector layer and a given end point." );
}

QString QgsShortestPathLayerToPointAlgorithm::shortDescription() const
{
  return QObject::tr( "Computes optimal (shortest or fastest) routes "
                      "from multiple start points defined by a vector layer and a given end point." );
}

QgsShortestPathLayerToPointAlgorithm *QgsShortestPathLayerToPointAlgorithm::createInstance() const
{
  return new QgsShortestPathLayerToPointAlgorithm();
}

void QgsShortestPathLayerToPointAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParams();
  addParameter( new QgsProcessingParameterFeatureSource( u"START_POINTS"_s, QObject::tr( "Vector layer with start points" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) ) );
  addParameter( new QgsProcessingParameterPoint( u"END_POINT"_s, QObject::tr( "End point" ) ) );

  std::unique_ptr<QgsProcessingParameterNumber> maxEndPointDistanceFromNetwork = std::make_unique<QgsProcessingParameterDistance>( u"POINT_TOLERANCE"_s, QObject::tr( "Maximum point distance from network" ), QVariant(), u"INPUT"_s, true, 0 );
  maxEndPointDistanceFromNetwork->setFlags( maxEndPointDistanceFromNetwork->flags() | Qgis::ProcessingParameterFlag::Advanced );
  maxEndPointDistanceFromNetwork->setHelp( QObject::tr( "Specifies an optional limit on the distance from the start and end points to the network layer. If the start feature is further from the network than this distance it will be treated as non-routable. If the end point is further from the network than this distance an error will be raised." ) );
  addParameter( maxEndPointDistanceFromNetwork.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Shortest path" ), Qgis::ProcessingSourceType::VectorLine ) );

  auto outputNonRoutable = std::make_unique<QgsProcessingParameterFeatureSink>( u"OUTPUT_NON_ROUTABLE"_s, QObject::tr( "Non-routable features" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true );
  outputNonRoutable->setHelp( QObject::tr( "An optional output which will be used to store any input features which could not be routed (e.g. those which are too far from the network layer)." ) );
  outputNonRoutable->setCreateByDefault( false );
  addParameter( outputNonRoutable.release() );
}

QVariantMap QgsShortestPathLayerToPointAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  loadCommonParams( parameters, context, feedback );

  const QgsPointXY endPoint = parameterAsPoint( parameters, u"END_POINT"_s, context, mNetwork->sourceCrs() );

  std::unique_ptr<QgsFeatureSource> startPoints( parameterAsSource( parameters, u"START_POINTS"_s, context ) );
  if ( !startPoints )
    throw QgsProcessingException( invalidSourceError( parameters, u"START_POINTS"_s ) );

  QgsFields newFields;
  newFields.append( QgsField( u"start"_s, QMetaType::Type::QString ) );
  newFields.append( QgsField( u"end"_s, QMetaType::Type::QString ) );
  newFields.append( QgsField( u"cost"_s, QMetaType::Type::Double ) );
  QgsFields fields = QgsProcessingUtils::combineFields( startPoints->fields(), newFields );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::LineString, mNetwork->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QString nonRoutableSinkId;
  std::unique_ptr<QgsFeatureSink> nonRoutableSink( parameterAsSink( parameters, u"OUTPUT_NON_ROUTABLE"_s, context, nonRoutableSinkId, startPoints->fields(), Qgis::WkbType::Point, mNetwork->sourceCrs() ) );

  const double pointDistanceThreshold = parameters.value( u"POINT_TOLERANCE"_s ).isValid() ? parameterAsDouble( parameters, u"POINT_TOLERANCE"_s, context ) : -1;

  QVector<QgsPointXY> points;
  points.push_front( endPoint );
  QHash<int, QgsAttributes> sourceAttributes;
  loadPoints( startPoints.get(), &points, &sourceAttributes, context, feedback, nullptr );

  feedback->pushInfo( QObject::tr( "Building graph…" ) );
  QVector<QgsPointXY> snappedPoints;
  mDirector->makeGraph( mBuilder.get(), points, snappedPoints, feedback );

  const QgsPointXY snappedEndPoint = snappedPoints[0];

  if ( pointDistanceThreshold >= 0 )
  {
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

  feedback->pushInfo( QObject::tr( "Calculating shortest paths…" ) );
  std::unique_ptr<QgsGraph> graph( mBuilder->takeGraph() );
  const int idxEnd = graph->findVertex( snappedEndPoint );
  int idxStart;
  int currentIdx;

  QVector<int> tree;
  QVector<double> costs;

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
            throw QgsProcessingException( writeFeatureError( nonRoutableSink.get(), parameters, u"OUTPUT_NON_ROUTABLE"_s ) );
        }

        feedback->setProgress( i * step );
        continue;
      }
    }

    idxStart = graph->findVertex( snappedPoint );
    QgsGraphAnalyzer::dijkstra( graph.get(), idxStart, 0, &tree, &costs );

    if ( tree.at( idxEnd ) == -1 )
    {
      feedback->reportError( QObject::tr( "There is no route from start point (%1) to end point (%2)." )
                               .arg( originalPoint.toString(), endPoint.toString() ) );
      feat.clearGeometry();
      attributes = sourceAttributes.value( i );
      attributes.append( originalPoint.toString() );
      feat.setAttributes( attributes );
      if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
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
    attributes.append( originalPoint.toString() );
    attributes.append( endPoint.toString() );
    attributes.append( cost / mMultiplier );
    feat.setAttributes( attributes );
    feat.setGeometry( geom );
    if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );

    feedback->setProgress( i * step );
  }

  if ( sink )
    sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  if ( nonRoutableSink )
  {
    nonRoutableSink->finalize();
    outputs.insert( u"OUTPUT_NON_ROUTABLE"_s, nonRoutableSinkId );
  }
  return outputs;
}

///@endcond
