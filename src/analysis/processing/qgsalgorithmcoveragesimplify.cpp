/***************************************************************************
                         qgsalgorithmcoveragesimplify.cpp
                         ---------------------
    begin                : October 2023
    copyright            : (C) 2023 by Nyall Dawson
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


#include "qgsalgorithmcoveragesimplify.h"
#include "qgsgeometrycollection.h"
#include "qgsgeos.h"


///@cond PRIVATE

QString QgsCoverageSimplifyAlgorithm::name() const
{
  return QStringLiteral( "coveragesimplify" );
}

QString QgsCoverageSimplifyAlgorithm::displayName() const
{
  return QObject::tr( "Simplify coverage" );
}

QStringList QgsCoverageSimplifyAlgorithm::tags() const
{
  return QObject::tr( "topological,boundary" ).split( ',' );
}

QString QgsCoverageSimplifyAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsCoverageSimplifyAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

void QgsCoverageSimplifyAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorPolygon ) );
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "TOLERANCE" ),
                QObject::tr( "Tolerance" ), 1.0, QStringLiteral( "INPUT" ), false, 0, 10000000.0 ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "PRESERVE_BOUNDARY" ), QObject::tr( "Preserve boundary" ), false ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Simplified" ), QgsProcessing::TypeVectorPolygon ) );
}

QString QgsCoverageSimplifyAlgorithm::shortHelpString() const
{
  return QObject::tr( "" );
}

QgsCoverageSimplifyAlgorithm *QgsCoverageSimplifyAlgorithm::createInstance() const
{
  return new QgsCoverageSimplifyAlgorithm();
}

QVariantMap QgsCoverageSimplifyAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const bool preserveBoundary = parameterAsBoolean( parameters, QStringLiteral( "PRESERVE_BOUNDARY" ), context );
  const double tolerance = parameterAsDouble( parameters, QStringLiteral( "TOLERANCE" ), context );

  QString sinkId;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, sinkId, source->fields(), source->wkbType(), source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  // we have no choice but to build up a list of features in advance
  QVector< QgsFeature > featuresWithGeom;
  QVector< QgsFeature > featuresWithoutGeom;
  QgsGeometryCollection collection;

  const long count = source->featureCount();
  if ( count >  0 )
  {
    featuresWithGeom.reserve( count );
    collection.reserve( count );
  }

  const double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  feedback->pushInfo( QObject::tr( "Collecting features" ) );

  QgsFeature inFeature;
  QgsFeatureIterator features = source->getFeatures();
  while ( features.nextFeature( inFeature ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( inFeature.hasGeometry() )
    {
      featuresWithGeom.append( inFeature );
      collection.addGeometry( inFeature.geometry().constGet()->clone() );
    }
    else
    {
      featuresWithoutGeom.append( inFeature );
    }


    feedback->setProgress( current * step * 0.2 );
    current++;
  }

  feedback->pushInfo( QObject::tr( "Simplifying coverage" ) );

  QgsGeos geos( &collection );
  QString error;
  std::unique_ptr< QgsAbstractGeometry > simplified;
  try
  {
    simplified.reset( geos.simplifyCoverageVW( tolerance, preserveBoundary, &error ) );
  }
  catch ( QgsNotSupportedException &e )
  {
    throw QgsProcessingException( e.what() );
  }

  if ( !simplified )
  {
    if ( !error.isEmpty() )
      throw QgsProcessingException( error );
    else
      throw QgsProcessingException( QObject::tr( "No geometry was returned for simplified coverage" ) );
  }

  feedback->setProgress( 80 );

  feedback->pushInfo( QObject::tr( "Storing features" ) );
  long long featureIndex = 0;
  for ( auto partsIt = simplified->const_parts_begin(); partsIt != simplified->const_parts_end(); ++partsIt )
  {
    QgsFeature outFeature = featuresWithGeom.value( featureIndex );
    outFeature.setGeometry( QgsGeometry( *partsIt ? ( *partsIt )->clone() : nullptr ) );
    if ( !sink->addFeature( outFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );


    feedback->setProgress( featureIndex * step * 0.2 + 80 );
    featureIndex++;
  }
  for ( const QgsFeature &feature : featuresWithoutGeom )
  {
    QgsFeature outFeature = feature;
    if ( !sink->addFeature( outFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), sinkId );
  return outputs;
}

///@endcond
