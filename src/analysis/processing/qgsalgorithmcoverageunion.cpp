/***************************************************************************
                         qgsalgorithmcoverageunion.cpp
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


#include "qgsalgorithmcoverageunion.h"
#include "qgsgeometrycollection.h"
#include "qgsgeos.h"


///@cond PRIVATE

QString QgsCoverageUnionAlgorithm::name() const
{
  return QStringLiteral( "coverageunion" );
}

QString QgsCoverageUnionAlgorithm::displayName() const
{
  return QObject::tr( "Dissolve coverage" );
}

QStringList QgsCoverageUnionAlgorithm::tags() const
{
  return QObject::tr( "union,merge,topological,boundary" ).split( ',' );
}

QString QgsCoverageUnionAlgorithm::group() const
{
  return QObject::tr( "Vector coverage" );
}

QString QgsCoverageUnionAlgorithm::groupId() const
{
  return QStringLiteral( "vectorcoverage" );
}

void QgsCoverageUnionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Dissolved" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

QString QgsCoverageUnionAlgorithm::shortDescription() const
{
  return QObject::tr( "Dissolves a coverage of polygon features" );
}

QString QgsCoverageUnionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm operates on a coverage (represented as a set of polygon features "
                      "with exactly matching edge geometry) to dissolve (union) the geometries.\n\n"
                      "It provides a heavily optimized approach for unioning these features compared with "
                      "the standard Dissolve tools." );
}

QgsCoverageUnionAlgorithm *QgsCoverageUnionAlgorithm::createInstance() const
{
  return new QgsCoverageUnionAlgorithm();
}

QVariantMap QgsCoverageUnionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  QString sinkId;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, sinkId, source->fields(), Qgis::WkbType::MultiPolygon, source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  // we have to build up a list of geometries in advance
  QgsGeometryCollection collection;

  const long count = source->featureCount();
  if ( count > 0 )
  {
    collection.reserve( count );
  }

  const double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  feedback->pushInfo( QObject::tr( "Collecting features" ) );

  QgsFeature inFeature;
  QgsFeatureRequest req;
  req.setNoAttributes();
  QgsFeatureIterator features = source->getFeatures( req );
  while ( features.nextFeature( inFeature ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( inFeature.hasGeometry() )
    {
      collection.addGeometry( inFeature.geometry().constGet()->clone() );
    }

    feedback->setProgress( current * step * 0.2 );
    current++;
  }

  feedback->pushInfo( QObject::tr( "Dissolving coverage" ) );

  QgsGeos geos( &collection );
  QString error;

  switch ( source->invalidGeometryCheck() )
  {
    case Qgis::InvalidGeometryCheck::NoCheck:
      break;

    case Qgis::InvalidGeometryCheck::SkipInvalid:
    case Qgis::InvalidGeometryCheck::AbortOnInvalid:
    {
      if ( geos.validateCoverage( 0, nullptr, &error ) != Qgis::CoverageValidityResult::Valid )
      {
        throw QgsProcessingException( QObject::tr( "Coverage is not valid" ) );
      }
      break;
    }
  }

  std::unique_ptr<QgsAbstractGeometry> dissolved = geos.unionCoverage( &error );

  if ( !dissolved )
  {
    if ( !error.isEmpty() )
      throw QgsProcessingException( error );
    else
      throw QgsProcessingException( QObject::tr( "No geometry was returned for dissolved coverage" ) );
  }

  feedback->setProgress( 95 );

  feedback->pushInfo( QObject::tr( "Storing output" ) );
  QgsFeature outFeature( source->fields() );
  outFeature.setGeometry( std::move( dissolved ) );
  if ( !sink->addFeature( outFeature, QgsFeatureSink::FastInsert ) )
    throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), sinkId );
  return outputs;
}

///@endcond
