/***************************************************************************
                         qgsalgorithmvalidatenetwork.cpp
                         -------------------------------
    begin                : January 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmvalidatenetwork.h"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsgraph.h"
#include "qgsgraphbuilder.h"
#include "qgslinestring.h"
#include "qgsprocessingoutputs.h"
#include "qgsspatialindexkdbush.h"
#include "qgsvectorlayerdirector.h"

///@cond PRIVATE

QString QgsValidateNetworkAlgorithm::name() const
{
  return u"validatenetwork"_s;
}

QString QgsValidateNetworkAlgorithm::displayName() const
{
  return QObject::tr( "Validate network" );
}

QStringList QgsValidateNetworkAlgorithm::tags() const
{
  return QObject::tr( "topological,topology,check,graph,shortest,path" ).split( ',' );
}

QString QgsValidateNetworkAlgorithm::group() const
{
  return QObject::tr( "Network analysis" );
}

QString QgsValidateNetworkAlgorithm::groupId() const
{
  return u"networkanalysis"_s;
}

QIcon QgsValidateNetworkAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmNetworkAnalysis.svg"_s );
}

QString QgsValidateNetworkAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( u"/algorithms/mAlgorithmNetworkAnalysis.svg"_s );
}

QString QgsValidateNetworkAlgorithm::shortDescription() const
{
  return QObject::tr( "Validates a network line layer, identifying data and topology errors that may affect network analysis tools." );
}

QString QgsValidateNetworkAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm analyzes a network vector layer to identify data and topology errors "
                      "that may affect network analysis tools (like shortest path).\n\n"
                      "Optional checks include:\n\n"
                      "1. Validating the 'Direction' field to ensure all direction field values in the input layer "
                      "match the configured forward/backward/both values. Errors will be reported if the direction field "
                      "value is non-null and does not match one of the configured values.\n"
                      "2. Checking node-to-node separation. This check identifies nodes from the network graph that "
                      "are closer to other nodes than the specified tolerance distance. This often indicates missed "
                      "snaps or short segments in the input layer. In the case that a node violates this condition with multiple other "
                      "nodes, only the closest violation will be reported.\n"
                      "3. Checking node-to-segment separation: This check identifies nodes that are closer to a line "
                      "segment (e.g. a graph edge) than the specified tolerance distance, without being connected to it. In the case "
                      "that a node violates this condition with multiple other edges, only the closest violation will be reported.\n\n"
                      "Two layers are output by this algorithm:\n"
                      "1. An output containing features from the original network layer which failed the direction validation checks.\n"
                      "2. An output representing the problematic node locations with a 'error' field explaining the error. This is "
                      "a line layer, where the output features join the problematic node to the node or "
                      "segment which failed the tolerance checks." );
}

QgsValidateNetworkAlgorithm *QgsValidateNetworkAlgorithm::createInstance() const
{
  return new QgsValidateNetworkAlgorithm();
}

void QgsValidateNetworkAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Vector layer representing network" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) ) );

  auto separationNodeNodeParam = std::make_unique<QgsProcessingParameterDistance>( u"TOLERANCE_NODE_NODE"_s, QObject::tr( "Minimum separation between nodes" ), QVariant(), u"INPUT"_s, true );
  separationNodeNodeParam->setFlags( separationNodeNodeParam->flags() | Qgis::ProcessingParameterFlag::Optional );
  separationNodeNodeParam->setHelp( QObject::tr( "The minimum allowed distance between two distinct graph nodes.\n\n"
                                                 "Nodes closer than this distance (but not identical) will be flagged as errors.\n\n"
                                                 "Leave empty to disable this check." ) );
  addParameter( separationNodeNodeParam.release() );

  auto separationNodeSegmentParam = std::make_unique<QgsProcessingParameterDistance>( u"TOLERANCE_NODE_SEGMENT"_s, QObject::tr( "Minimum separation between nodes and non-noded segments" ), QVariant(), u"INPUT"_s, true );
  separationNodeSegmentParam->setFlags( separationNodeSegmentParam->flags() | Qgis::ProcessingParameterFlag::Optional );
  separationNodeSegmentParam->setHelp( QObject::tr( "The minimum allowed distance between a graph node and a graph edge (segment) "
                                                    "that is not connected to the node.\n\n"
                                                    "Nodes closer to a segment than this distance "
                                                    "will be flagged. Leave empty to disable this check." ) );
  addParameter( separationNodeSegmentParam.release() );

  auto directionField = std::make_unique<QgsProcessingParameterField>( u"DIRECTION_FIELD"_s, QObject::tr( "Direction field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, false, true );
  directionField->setHelp( QObject::tr( "The attribute field specifying the direction of traffic flow for each segment." ) );
  addParameter( directionField.release() );

  auto forwardValue = std::make_unique<QgsProcessingParameterString>( u"VALUE_FORWARD"_s, QObject::tr( "Value for forward direction" ), QVariant(), false, true );
  forwardValue->setHelp( QObject::tr( "The string value in the direction field that indicates one-way traffic in the digitized direction." ) );
  addParameter( forwardValue.release() );

  auto backwardValue = std::make_unique<QgsProcessingParameterString>( u"VALUE_BACKWARD"_s, QObject::tr( "Value for backward direction" ), QVariant(), false, true );
  backwardValue->setHelp( QObject::tr( "The string value in the direction field that indicates one-way traffic opposite to the digitized direction." ) );
  addParameter( backwardValue.release() );

  auto bothValue = std::make_unique<QgsProcessingParameterString>( u"VALUE_BOTH"_s, QObject::tr( "Value for both directions" ), QVariant(), false, true );
  bothValue->setHelp( QObject::tr( "The string value in the direction field that indicates two-way traffic." ) );
  addParameter( bothValue.release() );

  std::unique_ptr<QgsProcessingParameterNumber> tolerance = std::make_unique<QgsProcessingParameterDistance>( u"TOLERANCE"_s, QObject::tr( "Topology tolerance" ), 0, u"INPUT"_s, false, 0 );
  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( tolerance.release() );

  auto invalidNetworkOutput = std::make_unique< QgsProcessingParameterFeatureSink >( u"OUTPUT_INVALID_NETWORK"_s, QObject::tr( "Invalid network features" ), Qgis::ProcessingSourceType::VectorLine, QVariant(), true, true );
  invalidNetworkOutput->setHelp( QObject::tr( "Output line layer containing geometries representing features from the network layer with validity errors.\n\n"
                                              "This output includes an attribute explaining why each feature is invalid." ) );
  addParameter( invalidNetworkOutput.release() );

  addOutput( new QgsProcessingOutputNumber( u"COUNT_INVALID_NETWORK_FEATURES"_s, QObject::tr( "Count of invalid network features" ) ) );

  auto invalidNodeOutput = std::make_unique< QgsProcessingParameterFeatureSink >( u"OUTPUT_INVALID_NODES"_s, QObject::tr( "Invalid network nodes" ), Qgis::ProcessingSourceType::VectorLine, QVariant(), true, true );
  invalidNodeOutput->setHelp( QObject::tr( "Output line layer containing geometries representing nodes from the network layer with validity errors.\n\n"
                                           "This output includes an attribute explaining why each node is invalid." ) );
  addParameter( invalidNodeOutput.release() );

  addOutput( new QgsProcessingOutputNumber( u"COUNT_INVALID_NODES"_s, QObject::tr( "Count of invalid network nodes" ) ) );
}

QVariantMap QgsValidateNetworkAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsFeatureSource> networkSource( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !networkSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const QString directionFieldName = parameterAsString( parameters, u"DIRECTION_FIELD"_s, context );
  const QString forwardValue = parameterAsString( parameters, u"VALUE_FORWARD"_s, context );
  const QString backwardValue = parameterAsString( parameters, u"VALUE_BACKWARD"_s, context );
  const QString bothValue = parameterAsString( parameters, u"VALUE_BOTH"_s, context );
  const double tolerance = parameterAsDouble( parameters, u"TOLERANCE"_s, context );

  double toleranceNodeToNode = 0;
  bool checkNodeToNodeDistance = false;
  if ( parameters.value( u"TOLERANCE_NODE_NODE"_s ).isValid() )
  {
    toleranceNodeToNode = parameterAsDouble( parameters, u"TOLERANCE_NODE_NODE"_s, context );
    checkNodeToNodeDistance = ( toleranceNodeToNode > 0 );
  }

  double toleranceNodeToSegment = 0;
  bool checkNodeToSegmentDistance = false;
  if ( parameters.value( u"TOLERANCE_NODE_SEGMENT"_s ).isValid() )
  {
    toleranceNodeToSegment = parameterAsDouble( parameters, u"TOLERANCE_NODE_SEGMENT"_s, context );
    checkNodeToSegmentDistance = ( toleranceNodeToSegment > 0 );
  }

  QgsFields newNetworkErrorFields;
  newNetworkErrorFields.append( QgsField( u"error"_s, QMetaType::Type::QString ) );
  const QgsFields networkErrorFields = QgsProcessingUtils::combineFields( networkSource->fields(), newNetworkErrorFields );

  QString networkErrorDest;
  std::unique_ptr<QgsFeatureSink> networkErrorSink( parameterAsSink( parameters, u"OUTPUT_INVALID_NETWORK"_s, context, networkErrorDest, networkErrorFields, networkSource->wkbType(), networkSource->sourceCrs() ) );

  QgsFields nodeErrorFields;
  nodeErrorFields.append( QgsField( u"error"_s, QMetaType::Type::QString ) );

  QString nodeErrorDest;
  std::unique_ptr<QgsFeatureSink> nodeErrorSink( parameterAsSink( parameters, u"OUTPUT_INVALID_NODES"_s, context, nodeErrorDest, nodeErrorFields, Qgis::WkbType::LineString, networkSource->sourceCrs() ) );

  QgsProcessingMultiStepFeedback multiFeedback( 4, feedback );
  multiFeedback.setStepWeights( { 10, 40, 10, 40 } );
  multiFeedback.setCurrentStep( 0 );

  QVariantMap outputs;
  if ( networkErrorSink )
    outputs.insert( u"OUTPUT_INVALID_NETWORK"_s, networkErrorDest );
  if ( nodeErrorSink )
    outputs.insert( u"OUTPUT_INVALID_NODES"_s, nodeErrorDest );

  // attribute validation
  int directionFieldIdx = -1;
  long long countInvalidFeatures = 0;
  if ( !directionFieldName.isEmpty() )
  {
    directionFieldIdx = networkSource->fields().lookupField( directionFieldName );
    if ( directionFieldIdx < 0 )
    {
      throw QgsProcessingException( QObject::tr( "Missing field %1 in input layer" ).arg( directionFieldName ) );
    }

    multiFeedback.pushInfo( QObject::tr( "Validating direction attributes…" ) );
    const long long count = networkSource->featureCount();
    long long current = 0;
    const double step = count > 0 ? 100.0 / static_cast< double >( count ) : 1;

    QgsFeatureIterator fit = networkSource->getFeatures();
    QgsFeature feature;
    while ( fit.nextFeature( feature ) )
    {
      if ( multiFeedback.isCanceled() )
        break;

      const QVariant val = feature.attribute( directionFieldIdx );
      if ( !QgsVariantUtils::isNull( val ) )
      {
        const QString directionValueString = val.toString();
        if ( directionValueString != forwardValue && directionValueString != backwardValue && directionValueString != bothValue )
        {
          if ( networkErrorSink )
          {
            QgsFeature outputFeature = feature;
            QgsAttributes outputFeatureAttrs = outputFeature.attributes();
            outputFeatureAttrs.append( QObject::tr( "Invalid direction value: '%1'" ).arg( directionValueString ) );
            outputFeature.setAttributes( outputFeatureAttrs );
            if ( !networkErrorSink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
            {
              throw QgsProcessingException( writeFeatureError( networkErrorSink.get(), parameters, u"OUTPUT_INVALID_NETWORK"_s ) );
            }
          }
          countInvalidFeatures++;
        }
      }

      current++;
      multiFeedback.setProgress( static_cast< double >( current ) * step );
    }

    if ( networkErrorSink )
    {
      networkErrorSink->finalize();
    }
  }

  outputs.insert( u"COUNT_INVALID_NETWORK_FEATURES"_s, countInvalidFeatures );
  if ( countInvalidFeatures > 0 )
  {
    multiFeedback.reportError( QObject::tr( "Found %1 invalid network features" ).arg( countInvalidFeatures ) );
  }

  if ( !checkNodeToNodeDistance && !checkNodeToSegmentDistance )
  {
    // nothing more to do
    return outputs;
  }

  multiFeedback.pushInfo( QObject::tr( "Building graph for topology validation…" ) );
  multiFeedback.setCurrentStep( 1 );

  QgsVectorLayerDirector director( networkSource.get(), directionFieldIdx, forwardValue, backwardValue, bothValue, QgsVectorLayerDirector::DirectionBoth );
  QgsGraphBuilder builder( networkSource->sourceCrs(), true, tolerance, context.ellipsoid() );

  QVector<QgsPointXY> snappedPoints;
  director.makeGraph( &builder, {}, snappedPoints, &multiFeedback );

  std::unique_ptr<QgsGraph> graph( builder.takeGraph() );

  if ( multiFeedback.isCanceled() )
    return outputs;

  multiFeedback.pushInfo( QObject::tr( "Indexing graph nodes and edges…" ) );
  multiFeedback.setCurrentStep( 2 );

  // better index choice for point node index -- we satisfy the requirements
  // of point geometries only, finalized once before reading
  QgsSpatialIndexKDBush nodeIndex;
  // standard QgsSpatialIndex for edges -- we can't use the faster KDBush index for these, as that is point only
  QgsSpatialIndex edgeIndex( QgsSpatialIndex::FlagStoreFeatureGeometries );

  const int vertexCount = graph->vertexCount();

  const long long totalGraphElements = ( checkNodeToNodeDistance ? vertexCount : 0 ) + ( checkNodeToSegmentDistance ? graph->edgeCount() : 0 );
  const double indexStep = totalGraphElements > 0 ? 100.0 / static_cast< double >( totalGraphElements ) : 1;
  long long elementsProcessed = 0;

  if ( checkNodeToNodeDistance )
  {
    for ( int i = 0; i < vertexCount; ++i )
    {
      if ( multiFeedback.isCanceled() )
        break;
      nodeIndex.addFeature( i, graph->vertex( i ).point() );
      elementsProcessed++;
      multiFeedback.setProgress( static_cast< double >( elementsProcessed ) * indexStep );
    }
    nodeIndex.finalize();
  }

  if ( checkNodeToSegmentDistance )
  {
    for ( int i = 0; i < graph->edgeCount(); ++i )
    {
      if ( multiFeedback.isCanceled() )
        break;

      const QgsGraphEdge &edge = graph->edge( i );
      const QgsPointXY p1 = graph->vertex( edge.fromVertex() ).point();
      const QgsPointXY p2 = graph->vertex( edge.toVertex() ).point();

      edgeIndex.addFeature( i, QgsRectangle( p1, p2 ) );
      elementsProcessed++;
      multiFeedback.setProgress( static_cast< double >( elementsProcessed ) * indexStep );
    }
  }

  // perform topology checks
  multiFeedback.pushInfo( QObject::tr( "Validating graph topology…" ) );
  multiFeedback.setCurrentStep( 2 );

  const double topoStep = vertexCount > 0 ? 100.0 / vertexCount : 1;

  struct NodeError
  {
      long long id = 0;
      QgsPointXY pt;
      double distance = std::numeric_limits<double>::max();
  };

  QSet< QPair< long long, long long > > alreadyReportedNodes;
  long long countInvalidNodes = 0;

  for ( long long i = 0; i < vertexCount; ++i )
  {
    if ( multiFeedback.isCanceled() )
      break;

    const QgsGraphVertex &v = graph->vertex( i );
    const QgsPointXY &pt = v.point();

    if ( checkNodeToNodeDistance )
    {
      const QList<QgsSpatialIndexKDBushData> candidates = nodeIndex.intersects(
        QgsRectangle::fromCenterAndSize( pt, toleranceNodeToNode * 2, toleranceNodeToNode * 2 )
      );

      // only keep the closest violation
      NodeError closestError;
      for ( const QgsSpatialIndexKDBushData &data : candidates )
      {
        // skip self
        if ( data.id == i )
          continue;

        // ignore nodes which are directly connected to each other
        bool skip = false;
        for ( const int edge : v.incomingEdges() )
        {
          if ( graph->edge( edge ).fromVertex() == i )
          {
            skip = true;
            break;
          }
        }
        if ( skip )
          continue;
        for ( const int edge : v.outgoingEdges() )
        {
          if ( graph->edge( edge ).toVertex() == data.id )
          {
            skip = true;
            break;
          }
        }
        if ( skip )
          continue;

        const double distanceNodeToNode = pt.distance( data.point() );
        if ( distanceNodeToNode < toleranceNodeToNode && distanceNodeToNode < closestError.distance )
        {
          closestError.distance = distanceNodeToNode;
          closestError.id = data.id;
          closestError.pt = data.point();
        }
      }

      if ( !closestError.pt.isEmpty() )
      {
        const QPair< long long, long long > nodeId = qMakePair( std::min( closestError.id, i ), std::max( closestError.id, i ) );
        if ( alreadyReportedNodes.contains( nodeId ) )
        {
          // already reported this -- eg when checking the other node in the pair
          continue;
        }
        alreadyReportedNodes.insert( nodeId );

        if ( nodeErrorSink )
        {
          QgsFeature nodeErrorFeature( nodeErrorFields );
          nodeErrorFeature.setGeometry( std::make_unique< QgsLineString >( QVector<QgsPointXY>() << pt << closestError.pt ) );
          nodeErrorFeature.setAttributes( QgsAttributes() << QObject::tr( "Node too close to adjacent node (%1 < %2)" ).arg( closestError.distance ).arg( toleranceNodeToNode ) );
          if ( !nodeErrorSink->addFeature( nodeErrorFeature, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( nodeErrorSink.get(), parameters, u"OUTPUT_INVALID_NODES"_s ) );
        }
        countInvalidNodes++;
      }
    }

    if ( checkNodeToSegmentDistance )
    {
      // only keep the closest violation
      NodeError closestError;

      const QList<QgsFeatureId> edgeIds = edgeIndex.intersects( QgsRectangle::fromCenterAndSize( pt, toleranceNodeToSegment * 2, toleranceNodeToSegment * 2 ) );
      for ( QgsFeatureId edgeIdx : edgeIds )
      {
        const QgsGraphEdge &edge = graph->edge( static_cast< int >( edgeIdx ) );
        // skip edges connected to this node
        if ( edge.fromVertex() == i || edge.toVertex() == i )
          continue;

        const QgsPointXY p1 = graph->vertex( edge.fromVertex() ).point();
        const QgsPointXY p2 = graph->vertex( edge.toVertex() ).point();

        QgsPointXY closestPt;
        const double distanceToSegment = std::sqrt( pt.sqrDistToSegment( p1.x(), p1.y(), p2.x(), p2.y(), closestPt ) );
        if ( distanceToSegment >= toleranceNodeToSegment )
          continue;

        // we don't consider this a node-to-segment error if the closest point is actually one of the segment endpoints.
        // in that case it's a node-to-NODE error.
        if ( closestPt.compare( p1 ) || closestPt.compare( p2 ) )
        {
          continue;
        }

        if ( distanceToSegment > closestError.distance )
        {
          continue;
        }
        closestError.distance = distanceToSegment;
        closestError.pt = closestPt;
      }

      if ( !closestError.pt.isEmpty() )
      {
        if ( nodeErrorSink )
        {
          QgsFeature nodeErrorFeature( nodeErrorFields );
          nodeErrorFeature.setGeometry( std::make_unique< QgsLineString >( QVector<QgsPointXY>() << pt << closestError.pt ) );
          nodeErrorFeature.setAttributes( QgsAttributes() << QObject::tr( "Node too close to non-noded segment (%1 < %2)" ).arg( closestError.distance ).arg( toleranceNodeToSegment ) );
          if ( !nodeErrorSink->addFeature( nodeErrorFeature, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( nodeErrorSink.get(), parameters, u"OUTPUT_INVALID_NODES"_s ) );
        }
        countInvalidNodes++;
      }
    }

    multiFeedback.setProgress( i * topoStep );
    if ( nodeErrorSink )
    {
      nodeErrorSink->finalize();
    }
  }

  feedback->setProgress( 100 );
  if ( countInvalidNodes > 0 )
  {
    multiFeedback.reportError( QObject::tr( "Found %1 invalid network nodes" ).arg( countInvalidNodes ) );
  }

  outputs.insert( u"COUNT_INVALID_NODES"_s, countInvalidNodes );

  return outputs;
}

///@endcond

///@endcond
