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
  return QStringLiteral( "distancetonearesthub" );
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
  return QStringLiteral( "vectoranalysis" );
}

QString QgsHubDistanceAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes the distance between features from the source layer to the closest feature "
                      "from the destination layer.\n\n"
                      "Distance calculations are based on the features center.\n\n"
                      "The resulting line layer contains lines linking each origin point with its nearest destination feature.\n\n"
                      "The resulting point layer contains origin features center point with an additional field indicating the identifier "
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
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Source layer (spokes)" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "HUBS" ), QObject::tr( "Destination layer (hubs)" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Hub layer name attribute" ), QVariant(), QStringLiteral( "HUBS" ) ) );

  const QStringList options = QStringList()
                              << QObject::tr( "Meters" )
                              << QObject::tr( "Feets" )
                              << QObject::tr( "Miles" )
                              << QObject::tr( "Kilometers" )
                              << QObject::tr( "Layer units" );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "UNIT" ), QObject::tr( "Measurement unit" ), options, false, 0 ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT_LINES" ), QObject::tr( "Hub lines" ), Qgis::ProcessingSourceType::VectorLine, QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT_POINTS" ), QObject::tr( "Hub points" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true, false ) );
}

QVariantMap QgsHubDistanceAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( parameters.value( QStringLiteral( "INPUT" ) ) == parameters.value( QStringLiteral( "HUBS" ) ) )
    throw QgsProcessingException( QObject::tr( "Same layer given for both hubs and spokes" ) );

  std::unique_ptr<QgsProcessingFeatureSource> hubSource( parameterAsSource( parameters, QStringLiteral( "HUBS" ), context ) );
  if ( !hubSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "HUBS" ) ) );

  std::unique_ptr<QgsProcessingFeatureSource> spokeSource( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !spokeSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString fieldHubName = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  const int hubNameIndex = hubSource->fields().lookupField( fieldHubName );

  const int unitIndex = parameterAsEnum( parameters, QStringLiteral( "UNIT" ), context );
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

  QgsFields fields = spokeSource->fields();
  fields.append( QgsField( QStringLiteral( "HubName" ), QMetaType::Type::QString ) );
  fields.append( QgsField( QStringLiteral( "HubDist" ), QMetaType::Type::Double ) );

  QString linesDest;
  std::unique_ptr<QgsFeatureSink> linesSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT_LINES" ), context, linesDest, fields, Qgis::WkbType::LineString, hubSource->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !linesSink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT_LINES" ) ) );

  QString pointsDest;
  std::unique_ptr<QgsFeatureSink> pointsSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT_POINTS" ), context, pointsDest, fields, Qgis::WkbType::Point, hubSource->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !pointsSink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT_POINTS" ) ) );

  QgsFeatureRequest request;
  request.setSubsetOfAttributes( QgsAttributeList() << hubNameIndex );
  request.setDestinationCrs( spokeSource->sourceCrs(), context.transformContext() );

  const QgsSpatialIndex hubsIndex( hubSource->getFeatures( request ), feedback, QgsSpatialIndex::FlagStoreFeatureGeometries );
  QHash<QgsFeatureId, QVariant> hubsAttributeCache;
  double step = hubSource->featureCount() > 0 ? 50.0 / hubSource->featureCount() : 1;
  long long i = 0;
  const QgsSpatialIndex idx( hubSource->getFeatures( request ), [&]( const QgsFeature &f ) -> bool {
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
      if ( linesSink && !linesSink->addFeature( spokeFeature, QgsFeatureSink::Flag::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( linesSink.get(), parameters, QStringLiteral( "OUTPUT_LINES" ) ) );
      }
      if ( pointsSink && !pointsSink->addFeature( spokeFeature, QgsFeatureSink::Flag::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, QStringLiteral( "OUTPUT_POINTS" ) ) );
      }
      continue;
    }

    QgsPointXY point = spokeFeature.geometry().boundingBox().center();

    const QList<QgsFeatureId> neighbors = hubsIndex.nearestNeighbor( point, 1 );
    if ( neighbors.isEmpty() )
    {
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
        throw QgsProcessingException( writeFeatureError( linesSink.get(), parameters, QStringLiteral( "OUTPUT_LINES" ) ) );
      }
    }

    if ( pointsSink )
    {
      outputFeature.setGeometry( QgsGeometry::fromPointXY( point ) );
      if ( !pointsSink->addFeature( outputFeature, QgsFeatureSink::Flag::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( pointsSink.get(), parameters, QStringLiteral( "OUTPUT_POINTS" ) ) );
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
    results.insert( QStringLiteral( "OUTPUT_LINES" ), linesDest );
  }
  if ( pointsSink )
  {
    results.insert( QStringLiteral( "OUTPUT_POINTS" ), pointsDest );
  }
  return results;
}

///@endcond
