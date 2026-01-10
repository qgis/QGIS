/***************************************************************************
                         qgsalgorithmnetworkextractendpoints.cpp
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

#include "qgsalgorithmnetworkextractendpoints.h"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsgraph.h"
#include "qgsgraphbuilder.h"
#include "qgslinestring.h"
#include "qgsprocessingoutputs.h"
#include "qgsspatialindexkdbush.h"
#include "qgsvectorlayerdirector.h"

///@cond PRIVATE

QString QgsExtractNetworkEndpointsAlgorithm::name() const
{
  return u"extractnetworkendpoints"_s;
}

QString QgsExtractNetworkEndpointsAlgorithm::displayName() const
{
  return QObject::tr( "Extract network end points" );
}

QStringList QgsExtractNetworkEndpointsAlgorithm::tags() const
{
  return QObject::tr( "network,dead,end,node,degree,terminal,dangle" ).split( ',' );
}

QIcon QgsExtractNetworkEndpointsAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmNetworkAnalysis.svg"_s );
}

QString QgsExtractNetworkEndpointsAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( u"/algorithms/mAlgorithmNetworkAnalysis.svg"_s );
}

QString QgsExtractNetworkEndpointsAlgorithm::shortDescription() const
{
  return QObject::tr( "Extracts the end points (nodes) from a network line layer." );
}

QString QgsExtractNetworkEndpointsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts the end points (nodes) from a network line layer.\n\n"
                      "Two definitions are available for identifying end points:\n\n"
                      "1. Nodes with only all incoming or all outgoing edges: Identifies 'Source' or 'Sink' nodes "
                      "based on the direction of flow. These are nodes where flow can start (only outgoing) or stop "
                      "(only incoming).\n"
                      "2. Nodes connected to a single edge: Identifies topological 'dead-ends' or 'dangles', regardless "
                      "of directionality. This checks if the node is connected to only one other distinct node." );
}

QgsExtractNetworkEndpointsAlgorithm *QgsExtractNetworkEndpointsAlgorithm::createInstance() const
{
  return new QgsExtractNetworkEndpointsAlgorithm();
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsExtractNetworkEndpointsAlgorithm::documentationFlags() const
{
  // override QgsNetworkAnalysisAlgorithmBase flag for ellipsoid use -- we don't use distances
  // here
  return Qgis::ProcessingAlgorithmDocumentationFlags();
}

void QgsExtractNetworkEndpointsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addCommonParams();

  // remove unused common parameters
  removeParameter( u"STRATEGY"_s );
  removeParameter( u"SPEED_FIELD"_s );
  removeParameter( u"DEFAULT_SPEED"_s );

  QStringList definitions;
  definitions << QObject::tr( "Extract Nodes with only All Incoming or All Outgoing Edges" )
              << QObject::tr( "Extract Nodes Connected to a Single Edge" );

  auto strategyParam = std::make_unique<QgsProcessingParameterEnum>( u"ENDPOINT_DEFINITION"_s, QObject::tr( "End point definition" ), definitions, false, 1 );
  addParameter( strategyParam.release() );

  auto outputPoints = std::make_unique<QgsProcessingParameterFeatureSink>( u"OUTPUT"_s, QObject::tr( "Network endpoints" ), Qgis::ProcessingSourceType::VectorPoint );
  addParameter( outputPoints.release() );
}

QVariantMap QgsExtractNetworkEndpointsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsProcessingMultiStepFeedback multiFeedback( 2, feedback );
  multiFeedback.setStepWeights( { 80, 20 } );
  multiFeedback.setCurrentStep( 0 );

  loadCommonParams( parameters, context, &multiFeedback );

  const int definition = parameterAsEnum( parameters, u"ENDPOINT_DEFINITION"_s, context );

  QgsFields outFields;
  outFields.append( QgsField( u"node_id"_s, QMetaType::Type::Int ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, outFields, Qgis::WkbType::Point, mNetwork->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  multiFeedback.pushInfo( QObject::tr( "Building graph…" ) );
  QVector<QgsPointXY> snappedPoints;
  mDirector->makeGraph( mBuilder.get(), {}, snappedPoints, &multiFeedback );
  if ( multiFeedback.isCanceled() )
    return {};

  multiFeedback.setCurrentStep( 1 );
  multiFeedback.pushInfo( QObject::tr( "Calculating endpoints…" ) );
  std::unique_ptr<QgsGraph> graph( mBuilder->takeGraph() );

  const int vertexCount = graph->vertexCount();
  double step = vertexCount > 0 ? 20.0 / vertexCount : 1;
  long endpointsFound = 0;

  for ( int i = 0; i < vertexCount; ++i )
  {
    if ( multiFeedback.isCanceled() )
      break;

    const QgsGraphVertex &v = graph->vertex( i );
    bool isEndPoint = false;

    if ( definition == 0 )
    {
      // definition = only all incoming or only all outgoing edges
      const bool hasIncoming = !v.incomingEdges().empty();
      const int hasOutgoing = !v.outgoingEdges().empty();
      if ( ( hasIncoming && !hasOutgoing ) || ( hasOutgoing && !hasIncoming ) )
      {
        isEndPoint = true;
      }
    }
    else
    {
      // definition = single connected edge
      // count unique neighbors to handle bidirectional segments (A->B and B->A) counting as one connection
      QSet<int> adjacentNodeIndices;
      for ( int edgeId : v.outgoingEdges() )
      {
        adjacentNodeIndices.insert( graph->edge( edgeId ).toVertex() );
      }
      for ( int edgeId : v.incomingEdges() )
      {
        adjacentNodeIndices.insert( graph->edge( edgeId ).fromVertex() );
      }
      if ( adjacentNodeIndices.count() == 1 )
      {
        isEndPoint = true;
      }
    }

    if ( isEndPoint )
    {
      QgsFeature f;
      f.setGeometry( QgsGeometry::fromPointXY( v.point() ) );
      f.setAttributes( QgsAttributes() << i );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      }
      endpointsFound++;
    }

    multiFeedback.setProgress( i * step );
  }
  sink->finalize();

  multiFeedback.pushInfo( QObject::tr( "Found %1 end points." ).arg( endpointsFound ) );
  feedback->setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
