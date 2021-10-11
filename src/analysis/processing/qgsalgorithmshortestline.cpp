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

QString QgsShortestLineAlgorithm::name() const
{
  return QStringLiteral( "shortestline" );
}

QString QgsShortestLineAlgorithm::displayName() const
{
  return QObject::tr( "Shortest line between layers" );
}

QStringList QgsShortestLineAlgorithm::tags() const
{
  return QObject::tr( "distance,shortest,minimum,nearest" ).split( ',' );
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
                      "The n-nearest neighboring features number can be specified. "
                      "If a maximum distance is specified, then only "
                      "features which are closer than this distance will "
                      "be considered. The output features will contain all the "
                      "source layer attributes, all the attributes from the n-nearest "
                      "feature and the additional field of the distance. "
                      "This algorithm uses purely Cartesian calculations for distance, "
                      "and does not consider geodetic or ellipsoid properties when "
                      "determining feature proximity."
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
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "NEIGHBORS" ), QObject::tr( "Maximum number of neighbors" ), QgsProcessingParameterNumber::Integer, 1, false, 1 ) );
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "DISTANCE" ), QObject::tr( "Maximum distance" ), QVariant(), QString( "SOURCE" ), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Shortest lines" ) ) );
}

bool QgsShortestLineAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "SOURCE" ), context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "SOURCE" ) ) );

  mDestination.reset( parameterAsSource( parameters, QStringLiteral( "DESTINATION" ), context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "DESTINATION" ) ) );

  mKNeighbors = parameterAsInt( parameters, QStringLiteral( "NEIGHBORS" ), context );

  double paramMaxDist = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context ); //defaults to zero if not set
  if ( paramMaxDist )
    mMaxDistance = paramMaxDist;

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

  QgsFeatureRequest request = QgsFeatureRequest();
  request.setDestinationCrs( mSource->sourceCrs(), context.transformContext() );

  QgsSpatialIndex idx = QgsSpatialIndex( mDestination->getFeatures( request ), nullptr, QgsSpatialIndex::FlagStoreFeatureGeometries );

  QgsFeatureIterator sourceIterator = mSource->getFeatures();
  double step = mSource->featureCount() > 0 ? 100.0 / mSource->featureCount() : 1;

  QgsDistanceArea da = QgsDistanceArea();
  da.setSourceCrs( mSource->sourceCrs(), context.transformContext() );
  da.setEllipsoid( context.ellipsoid() );

  int i = 0;
  QgsFeature sourceFeature;
  while ( sourceIterator.nextFeature( sourceFeature ) )
  {
    if ( feedback->isCanceled() )
      break;


    QgsGeometry sourceGeom = sourceFeature.geometry();
    QgsFeatureIds nearestIds = idx.nearestNeighbor( sourceGeom, mKNeighbors, mMaxDistance ).toSet();

    QgsFeatureRequest targetRequest = QgsFeatureRequest();
    targetRequest.setFilterFids( nearestIds );

    QgsFeatureIterator destinationIterator = mDestination->getFeatures( targetRequest );
    QgsFeature destinationFeature;
    while ( destinationIterator.nextFeature( destinationFeature ) )
    {
      QgsGeometry destinationGeom = destinationFeature.geometry();

      QgsGeometry shortestLine = sourceGeom.shortestLine( destinationGeom );
      double dist = da.measureLength( shortestLine );

      QgsFeature f;

      QgsAttributes attrs = sourceFeature.attributes();
      attrs << destinationFeature.attributes() << dist;

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
