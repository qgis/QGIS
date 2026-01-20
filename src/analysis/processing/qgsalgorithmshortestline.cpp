/***************************************************************************
                         qgsalgorithmshortestline.cpp
                         ---------------------
    begin                : September 2021
    copyright            : (C) 2020 by Matteo Ghetta, Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//Disclaimer:This feature was originally developed in Python by: Matteo Ghetta, August 2021

#include "qgsalgorithmshortestline.h"

#include "qgsdistancearea.h"
#include "qgsspatialindex.h"

///@cond PRIVATE

QString QgsShortestLineAlgorithm::name() const
{
  return u"shortestline"_s;
}

QString QgsShortestLineAlgorithm::displayName() const
{
  return QObject::tr( "Shortest line between features" );
}

QStringList QgsShortestLineAlgorithm::tags() const
{
  return QObject::tr( "distance,shortest,minimum,nearest,closest,proximity" ).split( ',' );
}

QString QgsShortestLineAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsShortestLineAlgorithm::groupId() const
{
  return u"vectoranalysis"_s;
}

QString QgsShortestLineAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the shortest lines between features in source and destination layers." );
}

QString QgsShortestLineAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a line layer as the "
                      "shortest line between the source and the destination layer. "
                      "By default only the first nearest feature of the "
                      "destination layer is taken into account. "
                      "The n-nearest neighboring features number can be specified.\n\n"
                      "If a maximum distance is specified, then only "
                      "features which are closer than this distance will "
                      "be considered.\n\nThe output features will contain all the "
                      "source layer attributes, all the attributes from the n-nearest "
                      "feature and the additional field of the distance.\n\n"
                      "This algorithm uses purely Cartesian calculations for distance, "
                      "and does not consider geodetic or ellipsoid properties when "
                      "determining feature proximity. The measurement and output coordinate "
                      "system is based on the coordinate system of the source layer."
  );
}

QgsShortestLineAlgorithm *QgsShortestLineAlgorithm::createInstance() const
{
  return new QgsShortestLineAlgorithm();
}

void QgsShortestLineAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"SOURCE"_s, QObject::tr( "Source layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( u"DESTINATION"_s, QObject::tr( "Destination layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );
  addParameter( new QgsProcessingParameterEnum( u"METHOD"_s, QObject::tr( "Method" ), QStringList() << "Distance to Nearest Point on feature" << "Distance to Feature Centroid", false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( u"NEIGHBORS"_s, QObject::tr( "Maximum number of neighbors" ), Qgis::ProcessingNumberParameterType::Integer, 1, false, 1 ) );
  addParameter( new QgsProcessingParameterDistance( u"DISTANCE"_s, QObject::tr( "Maximum distance" ), QVariant(), QString( "SOURCE" ), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Shortest lines" ), Qgis::ProcessingSourceType::VectorLine ) );
}

bool QgsShortestLineAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, u"SOURCE"_s, context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"SOURCE"_s ) );

  mDestination.reset( parameterAsSource( parameters, u"DESTINATION"_s, context ) );
  if ( !mDestination )
    throw QgsProcessingException( invalidSourceError( parameters, u"DESTINATION"_s ) );

  mMethod = parameterAsInt( parameters, u"METHOD"_s, context );

  mKNeighbors = parameterAsInt( parameters, u"NEIGHBORS"_s, context );

  mMaxDistance = parameterAsDouble( parameters, u"DISTANCE"_s, context ); //defaults to zero if not set

  return true;
}

QVariantMap QgsShortestLineAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( mKNeighbors > mDestination->featureCount() )
    mKNeighbors = mDestination->featureCount();

  QgsFields fields = QgsProcessingUtils::combineFields( mSource->fields(), mDestination->fields() );

  QgsFields newFields;
  newFields.append( QgsField( u"distance"_s, QMetaType::Type::Double ) );
  fields = QgsProcessingUtils::combineFields( fields, newFields );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::MultiLineString, mSource->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  const QgsFeatureIterator destinationIterator = mDestination->getFeatures( QgsFeatureRequest().setDestinationCrs( mSource->sourceCrs(), context.transformContext() ) );
  QHash<QgsFeatureId, QgsAttributes> destinationAttributeCache;
  double step = mDestination->featureCount() > 0 ? 50.0 / mDestination->featureCount() : 1;
  int i = 0;
  const QgsSpatialIndex idx( destinationIterator, [&]( const QgsFeature &f ) -> bool {
    i++;
    if ( feedback-> isCanceled() )
      return false;

    feedback->setProgress( i * step );

    destinationAttributeCache.insert( f.id(), f.attributes() );

    return true; }, QgsSpatialIndex::FlagStoreFeatureGeometries );

  step = mSource->featureCount() > 0 ? 50.0 / mSource->featureCount() : 1;
  QgsFeatureIterator sourceIterator = mSource->getFeatures();

  QgsDistanceArea da = QgsDistanceArea();
  da.setSourceCrs( mSource->sourceCrs(), context.transformContext() );

  QgsFeature sourceFeature;
  while ( sourceIterator.nextFeature( sourceFeature ) )
  {
    if ( feedback->isCanceled() )
      break;

    const QgsGeometry sourceGeom = sourceFeature.geometry();
    QgsFeatureIds nearestIds = qgis::listToSet( idx.nearestNeighbor( sourceGeom, mKNeighbors, mMaxDistance ) );

    for ( const QgsFeatureId id : nearestIds )
    {
      QgsGeometry destinationGeom = idx.geometry( id );
      if ( mMethod == 1 )
      {
        destinationGeom = idx.geometry( id ).centroid();
      }

      const QgsGeometry shortestLine = sourceGeom.shortestLine( destinationGeom );
      double dist = 0;
      try
      {
        dist = da.measureLength( shortestLine );
      }
      catch ( QgsCsException & )
      {
        throw QgsProcessingException( QObject::tr( "An error occurred while calculating shortest line length" ) );
      }

      QgsFeature f;
      QgsAttributes attrs = sourceFeature.attributes();
      attrs << destinationAttributeCache.value( id ) << dist;

      f.setAttributes( attrs );
      f.setGeometry( shortestLine );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    }

    i++;
    feedback->setProgress( i * step );
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
