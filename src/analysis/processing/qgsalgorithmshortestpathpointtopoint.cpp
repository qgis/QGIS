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

#include "qgsgraph.h"
#include "qgsgraphbuilder.h"
#include "qgsgraphanalyzer.h"
#include "qgsvectorlayerdirector.h"
#include "qgsnetworkspeedstrategy.h"
#include "qgsnetworkdistancestrategy.h"

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

QString QgsShortestPathPointToPointAlgorithm::group() const
{
  return QObject::tr( "Network analysis" );
}

QString QgsShortestPathPointToPointAlgorithm::groupId() const
{
  return QStringLiteral( "networkanalysis" );
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
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Vector layer representing network" ), QList< int >() << QgsProcessing::TypeVectorLine ) );
  addParameter( new QgsProcessingParameterPoint( QStringLiteral( "START_POINT" ), QObject::tr( "Start point" ) ) );
  addParameter( new QgsProcessingParameterPoint( QStringLiteral( "END_POINT" ), QObject::tr( "End point" ) ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "STRATEGY" ), QObject::tr( "Path type to calculate" ), QStringList() << QObject::tr( "Shortest" ) << QObject::tr( "Fastest" ), false, 0 ) );

  std::unique_ptr< QgsProcessingParameterField > directionField = qgis::make_unique< QgsProcessingParameterField >( QStringLiteral( "DIRECTION_FIELD" ),
      QObject::tr( "Direction field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, false, true );
  directionField->setFlags( directionField->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( directionField.release() );

  std::unique_ptr< QgsProcessingParameterString > forwardValue = qgis::make_unique< QgsProcessingParameterString >( QStringLiteral( "VALUE_FORWARD" ),
      QObject::tr( "Value for forward direction" ), QVariant(), false, true );
  forwardValue->setFlags( forwardValue->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( forwardValue.release() );

  std::unique_ptr< QgsProcessingParameterString > backwardValue = qgis::make_unique< QgsProcessingParameterString >( QStringLiteral( "VALUE_BACKWARD" ),
      QObject::tr( "Value for backward direction" ), QVariant(), false, true );
  backwardValue->setFlags( backwardValue->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( backwardValue.release() );

  std::unique_ptr< QgsProcessingParameterString > bothValue = qgis::make_unique< QgsProcessingParameterString >( QStringLiteral( "VALUE_BOTH" ),
      QObject::tr( "Value for both directions" ), QVariant(), false, true );
  bothValue->setFlags( bothValue->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( bothValue.release() );

  std::unique_ptr< QgsProcessingParameterEnum > directionValue = qgis::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "DEFAULT_DIRECTION" ),
      QObject::tr( "Default direction" ), QStringList() << QObject::tr( "Forward direction" ) << QObject::tr( "Backward direction" ) << QObject::tr( "Both directions" ), false, 2 );
  directionValue->setFlags( directionValue->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( directionValue.release() );

  std::unique_ptr< QgsProcessingParameterField > speedField = qgis::make_unique< QgsProcessingParameterField >( QStringLiteral( "SPEED_FIELD" ),
      QObject::tr( "Speed field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Numeric, false, true );
  speedField->setFlags( speedField->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( speedField.release() );

  std::unique_ptr< QgsProcessingParameterNumber > speed = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "DEFAULT_SPEED" ), QObject::tr( "Default speed (km/h)" ), QgsProcessingParameterNumber::Double, 50, false, 0 );
  speed->setFlags( speed->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( speed.release() );

  std::unique_ptr< QgsProcessingParameterNumber > tolerance = qgis::make_unique < QgsProcessingParameterDistance >( QStringLiteral( "TOLERANCE" ), QObject::tr( "Topology tolerance" ), 0, QStringLiteral( "INPUT" ), false, 0 );
  tolerance->setFlags( tolerance->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( tolerance.release() );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Shortest path" ), QgsProcessing::TypeVectorLine ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TRAVEL_COST" ), QObject::tr( "Travel cost" ) ) );
}

QVariantMap QgsShortestPathPointToPointAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "start" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "end" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "cost" ), QVariant::Double ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::LineString, source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QgsPointXY startPoint = parameterAsPoint( parameters, QStringLiteral( "START_POINT" ), context, source->sourceCrs() );
  QgsPointXY endPoint = parameterAsPoint( parameters, QStringLiteral( "END_POINT" ), context, source->sourceCrs() );
  int strategy = parameterAsInt( parameters, QStringLiteral( "STRATEGY" ), context );
  QString directionFieldName = parameterAsString( parameters, QStringLiteral( "DIRECTION_FIELD" ), context );
  QString forwardValue = parameterAsString( parameters, QStringLiteral( "VALUE_FORWARD" ), context );
  QString backwardValue = parameterAsString( parameters, QStringLiteral( "VALUE_BACKWARD" ), context );
  QString bothValue = parameterAsString( parameters, QStringLiteral( "VALUE_BOTH" ), context );
  QgsVectorLayerDirector::Direction defaultDirection = static_cast< QgsVectorLayerDirector::Direction>( parameterAsInt( parameters, QStringLiteral( "DEFAULT_DIRECTION" ), context ) );
  QString speedFieldName = parameterAsString( parameters, QStringLiteral( "SPEED_FIELD" ), context );
  double defaultSpeed = parameterAsDouble( parameters, QStringLiteral( "DEFAULT_SPEED" ), context );
  double tolerance = parameterAsDouble( parameters, QStringLiteral( "TOLERANCE" ), context );

  int directionIndex = -1;
  if ( !directionFieldName.isEmpty() )
  {
    directionIndex = source->fields().lookupField( directionFieldName );
  }

  int speedIndex = -1;
  if ( !speedFieldName.isEmpty() )
  {
    speedIndex = source->fields().lookupField( speedFieldName );
  }

  QgsVectorLayerDirector *director = new QgsVectorLayerDirector( source.get(), directionIndex, forwardValue, backwardValue, bothValue, defaultDirection );

  QgsUnitTypes::DistanceUnit distanceUnits = context.project()->crs().mapUnits();
  double multiplier = QgsUnitTypes::fromUnitToUnitFactor( distanceUnits, QgsUnitTypes::DistanceMeters );

  if ( strategy )
  {
    director->addStrategy( new QgsNetworkSpeedStrategy( speedIndex, defaultSpeed, multiplier * 1000.0 / 3600.0 ) );
    multiplier = 3600;
  }
  else
  {
    director->addStrategy( new QgsNetworkDistanceStrategy() );
  }

  QgsGraphBuilder builder( source->sourceCrs(), true, tolerance );

  feedback->pushInfo( QObject::tr( "Building graph…" ) );
  QVector< QgsPointXY > points;
  points << startPoint << endPoint;
  QVector< QgsPointXY > snappedPoints;
  director->makeGraph( &builder, points, snappedPoints, feedback );

  feedback->pushInfo( QObject::tr( "Calculating shortest path…" ) );
  QgsGraph *graph = builder.graph();
  int idxStart = graph->findVertex( snappedPoints[0] );
  int idxEnd = graph->findVertex( snappedPoints[1] );

  QVector< int > tree;
  QVector< double > costs;
  QgsGraphAnalyzer::dijkstra( graph, idxStart, 0, &tree, &costs );

  if ( tree.at( idxEnd ) == -1 )
  {
    throw QgsProcessingException( QObject::tr( "There is no route from start point to end point." ) );
  }

  QVector<QgsPointXY> route;
  route.push_front( graph->vertex( idxEnd ).point() );
  double cost = costs.at( idxEnd );
  while ( idxEnd != idxStart )
  {
    idxEnd = graph->edge( tree.at( idxEnd ) ).fromVertex();
    route.push_front( graph->vertex( idxEnd ).point() );
  }

  feedback->pushInfo( QObject::tr( "Writing results…" ) );
  QgsGeometry geom = QgsGeometry::fromPolylineXY( route );
  QgsFeature feat;
  feat.setFields( fields );
  QgsAttributes attributes;
  attributes << startPoint.toString() << endPoint.toString() << cost / multiplier;
  feat.setGeometry( geom );
  feat.setAttributes( attributes );
  sink->addFeature( feat, QgsFeatureSink::FastInsert );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  outputs.insert( QStringLiteral( "TRAVEL_COST" ), cost / multiplier );
  return outputs;
}

///@endcond
