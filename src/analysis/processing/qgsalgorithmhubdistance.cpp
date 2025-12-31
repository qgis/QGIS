/***************************************************************************
                         qgsalgorithmhubdistance.cpp
                         ---------------------
    begin                : April 2025
    copyright            : (C) 2025 by Alexander Bruy
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

#include "qgsalgorithmhubdistance.h"

#include "qgsfeaturerequest.h"
#include "qgsspatialindex.h"

///@cond PRIVATE

QString QgsHubDistanceAlgorithm::name() const
{
  return u"distancetonearesthub"_s;
}

QString QgsHubDistanceAlgorithm::displayName() const
{
  return QObject::tr( "Distance to nearest hub" );
}

QStringList QgsHubDistanceAlgorithm::tags() const
{
  return QObject::tr( "lines,points,hub,spoke,distance" ).split( ',' );
}

QString QgsHubDistanceAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsHubDistanceAlgorithm::groupId() const
{
  return u"vectoranalysis"_s;
}

QString QgsHubDistanceAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes the distance between features from the source layer to the closest feature "
                      "from the destination layer.\n\n"
                      "Distance calculations are based on the feature's bounding box center.\n\n"
                      "The resulting line layer contains lines linking each origin point with its nearest destination feature.\n\n"
                      "The resulting point layer contains each origin feature's center point with additional fields indicating the identifier "
                      "of the nearest destination feature and the distance to it."
  );
}

QString QgsHubDistanceAlgorithm::shortDescription() const
{
  return QObject::tr( "Computes the distance between features from the source layer to the closest feature from the destination layer." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsHubDistanceAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RespectsEllipsoid;
}

QgsHubDistanceAlgorithm *QgsHubDistanceAlgorithm::createInstance() const
{
  return new QgsHubDistanceAlgorithm();
}

void QgsHubDistanceAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Source layer (spokes)" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( u"HUBS"_s, QObject::tr( "Destination layer (hubs)" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );
  addParameter( new QgsProcessingParameterField( u"FIELD"_s, QObject::tr( "Hub layer name attribute" ), QVariant(), u"HUBS"_s ) );

  const QStringList options = QStringList()
                              << QObject::tr( "Meters" )
                              << QObject::tr( "Feet" )
                              << QObject::tr( "Miles" )
                              << QObject::tr( "Kilometers" )
                              << QObject::tr( "Layer Units" );
  addParameter( new QgsProcessingParameterEnum( u"UNIT"_s, QObject::tr( "Measurement unit" ), options, false, 0 ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT_LINES"_s, QObject::tr( "Hub lines" ), Qgis::ProcessingSourceType::VectorLine, QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT_POINTS"_s, QObject::tr( "Hub points" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true, false ) );
}

QVariantMap QgsHubDistanceAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( parameters.value( u"INPUT"_s ) == parameters.value( u"HUBS"_s ) )
    throw QgsProcessingException( QObject::tr( "The same layer was specified for both the hubs and spokes. The hubs and spoke layers must be different layers." ) );

  std::unique_ptr<QgsProcessingFeatureSource> hubSource( parameterAsSource( parameters, u"HUBS"_s, context ) );
  if ( !hubSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"HUBS"_s ) );

  std::unique_ptr<QgsProcessingFeatureSource> spokeSource( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !spokeSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const QString fieldHubName = parameterAsString( parameters, u"FIELD"_s, context );
  const int hubNameIndex = hubSource->fields().lookupField( fieldHubName );

  const int unitIndex = parameterAsEnum( parameters, u"UNIT"_s, context );
  Qgis::DistanceUnit unit = Qgis::DistanceUnit::Unknown;
  switch ( unitIndex )
  {
    case 0:
      unit = Qgis::DistanceUnit::Meters;
      break;
    case 1:
      unit = Qgis::DistanceUnit::Feet;
      break;
    case 2:
      unit = Qgis::DistanceUnit::Miles;
      break;
    case 3:
      unit = Qgis::DistanceUnit::Kilometers;
      break;
  }

  QgsFields fields;
  fields.append( QgsField( u"HubName"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"HubDist"_s, QMetaType::Type::Double ) );
  fields = QgsProcessingUtils::combineFields( spokeSource->fields(), fields );

  QString linesDest;
  std::unique_ptr<QgsFeatureSink> linesSink( parameterAsSink( parameters, u"OUTPUT_LINES"_s, context, linesDest, fields, Qgis::WkbType::LineString, hubSource->sourceCrs() ) );
  if ( !linesSink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT_LINES"_s ) );

  QString pointsDest;
  std::unique_ptr<QgsFeatureSink> pointsSink( parameterAsSink( parameters, u"OUTPUT_POINTS"_s, context, pointsDest, fields, Qgis::WkbType::Point, hubSource->sourceCrs() ) );
  if ( !pointsSink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT_POINTS"_s ) );

  QgsFeatureRequest request;
  request.setSubsetOfAttributes( QgsAttributeList() << hubNameIndex );
  request.setDestinationCrs( spokeSource->sourceCrs(), context.transformContext() );
  QHash<QgsFeatureId, QVariant> hubsAttributeCache;
  double step = hubSource->featureCount() > 0 ? 50.0 / hubSource->featureCount() : 1;
  long long i = 0;
  const QgsSpatialIndex hubsIndex( hubSource->getFeatures( request ), [&]( const QgsFeature &f ) -> bool {
    if ( feedback-> isCanceled() )
    {
      return false;
    }

    hubsAttributeCache.insert( f.id(), f.attributes().at( hubNameIndex ) );

    i++;
    feedback->setProgress( i * step );
    return true; }, QgsSpatialIndex::FlagStoreFeatureGeometries );

  QgsDistanceArea da;
  da.setSourceCrs( spokeSource->sourceCrs(), context.transformContext() );
  da.setEllipsoid( context.ellipsoid() );

  // Scan source points, find nearest hub, and write to output file
  i = 0;
  QgsFeature spokeFeature, outputFeature;
  step = spokeSource->featureCount() > 0 ? 50.0 / spokeSource->featureCount() : 1;
  QgsFeatureIterator features = spokeSource->getFeatures();
  while ( features.nextFeature( spokeFeature ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    i++;
    feedback->setProgress( i * step );

    if ( !spokeFeature.hasGeometry() )
    {
      spokeFeature.setAttributes( spokeFeature.attributes() << QVariant() << QVariant() );
      if ( linesSink && !linesSink->addFeature( spokeFeature, QgsFeatureSink::Flag::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( linesSink.get(), parameters, u"OUTPUT_LINES"_s ) );
      }
      if ( pointsSink && !pointsSink->addFeature( spokeFeature, QgsFeatureSink::Flag::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, u"OUTPUT_POINTS"_s ) );
      }
      continue;
    }

    QgsPointXY point = spokeFeature.geometry().boundingBox().center();

    const QList<QgsFeatureId> neighbors = hubsIndex.nearestNeighbor( point, 1 );
    if ( neighbors.isEmpty() )
    {
      feedback->pushWarning( QObject::tr( "Feature %1 does not have any neighbour hubs." ) );
      continue;
    }

    const QgsPointXY hub = hubsIndex.geometry( neighbors.at( 0 ) ).boundingBox().center();
    double hubDistance = da.measureLine( point, hub );

    if ( unit != Qgis::DistanceUnit::Unknown )
    {
      hubDistance = da.convertLengthMeasurement( hubDistance, unit );
    }

    QgsAttributes attrs = spokeFeature.attributes();
    attrs << hubsAttributeCache.value( neighbors.at( 0 ) ) << hubDistance;
    outputFeature = QgsFeature();
    outputFeature.setAttributes( attrs );

    if ( linesSink )
    {
      outputFeature.setGeometry( QgsGeometry::fromPolylineXY( QgsPolylineXY() << point << hub ) );
      if ( !linesSink->addFeature( outputFeature, QgsFeatureSink::Flag::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( linesSink.get(), parameters, u"OUTPUT_LINES"_s ) );
      }
    }

    if ( pointsSink )
    {
      outputFeature.setGeometry( QgsGeometry::fromPointXY( point ) );
      if ( !pointsSink->addFeature( outputFeature, QgsFeatureSink::Flag::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, u"OUTPUT_POINTS"_s ) );
      }
    }
  }

  if ( linesSink )
  {
    linesSink->finalize();
  }
  if ( pointsSink )
  {
    pointsSink->finalize();
  }

  QVariantMap results;
  if ( linesSink )
  {
    results.insert( u"OUTPUT_LINES"_s, linesDest );
  }
  if ( pointsSink )
  {
    results.insert( u"OUTPUT_POINTS"_s, pointsDest );
  }
  return results;
}

///@endcond
