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

///@cond PRIVATE

QString QgsShortestLineAlgorithm::name() const
{
  return QStringLiteral( "shortestline" );
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
  return QStringLiteral( "vectoranalysis" );
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
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "SOURCE" ), QObject::tr( "Source layer" ), QList<int>() << QgsProcessing::TypeVectorAnyGeometry ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "DESTINATION" ), QObject::tr( "Destination layer" ), QList<int>() << QgsProcessing::TypeVectorAnyGeometry ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "METHOD" ), QObject::tr( "Method" ), QStringList() << "Distance to Nearest Point on feature" << "Distance to Feature Centroid", false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "NEIGHBORS" ), QObject::tr( "Maximum number of neighbors" ), QgsProcessingParameterNumber::Integer, 1, false, 1 ) );
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "DISTANCE" ), QObject::tr( "Maximum distance" ), QVariant(), QString( "SOURCE" ), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Shortest lines" ), QgsProcessing::TypeVectorLine ) );
}

bool QgsShortestLineAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "SOURCE" ), context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "SOURCE" ) ) );

  mDestination.reset( parameterAsSource( parameters, QStringLiteral( "DESTINATION" ), context ) );
  if ( !mDestination )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "DESTINATION" ) ) );

  mMethod = parameterAsInt( parameters, QStringLiteral( "METHOD" ), context );

  mKNeighbors = parameterAsInt( parameters, QStringLiteral( "NEIGHBORS" ), context );

  mMaxDistance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context ); //defaults to zero if not set

  return true;
}

QVariantMap QgsShortestLineAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( mKNeighbors > mDestination->featureCount() )
    mKNeighbors = mDestination->featureCount();

  QgsFields fields = QgsProcessingUtils::combineFields( mSource->fields(), mDestination->fields() );
  fields.append( QgsField( QStringLiteral( "distance" ), QVariant::Double ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::MultiLineString, mSource->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  const QgsFeatureIterator destinationIterator = mDestination->getFeatures( QgsFeatureRequest().setDestinationCrs( mSource->sourceCrs(), context.transformContext() ) );
  QHash< QgsFeatureId, QgsAttributes > destinationAttributeCache;
  double step = mDestination->featureCount() > 0 ? 50.0 / mDestination->featureCount() : 1;
  int i = 0;
  const QgsSpatialIndex idx( destinationIterator, [&]( const QgsFeature & f )->bool
  {
    i++;
    if ( feedback-> isCanceled() )
      return false;

    feedback->setProgress( i * step );

    destinationAttributeCache.insert( f.id(), f.attributes() );

    return true;
  }, QgsSpatialIndex::FlagStoreFeatureGeometries );

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
      double dist = da.measureLength( shortestLine );
      QgsFeature f;
      QgsAttributes attrs = sourceFeature.attributes();
      attrs << destinationAttributeCache.value( id ) << dist;

      f.setAttributes( attrs );
      f.setGeometry( shortestLine );
      sink->addFeature( f, QgsFeatureSink::FastInsert );
    }

    i++;
    feedback->setProgress( i * step );
  }


  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
