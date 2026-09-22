/***************************************************************************
                         qgsalgorithmcoverageedges.cpp
                         ---------------------
    begin                : September 2026
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


#include "qgsalgorithmcoverageedges.h"

#include "qgsgeometrycollection.h"
#include "qgsgeos.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsCoverageEdgesAlgorithm::name() const
{
  return u"coverageedges"_s;
}

QString QgsCoverageEdgesAlgorithm::displayName() const
{
  return QObject::tr( "Extract coverage edges" );
}

QStringList QgsCoverageEdgesAlgorithm::tags() const
{
  return QObject::tr( "topological,boundary,shared,boundaries,edges" ).split( ',' );
}

QString QgsCoverageEdgesAlgorithm::group() const
{
  return QObject::tr( "Vector coverage" );
}

QString QgsCoverageEdgesAlgorithm::groupId() const
{
  return u"vectorcoverage"_s;
}

void QgsCoverageEdgesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) ) );

  const QStringList types = { QObject::tr( "All Edges" ), QObject::tr( "Exterior Edges" ), QObject::tr( "Interior Edges" ) };
  auto typeParam = std::make_unique<QgsProcessingParameterEnum>( u"EDGE_TYPE"_s, QObject::tr( "Edge type" ), types, false, 0 );
  typeParam->setHelp( QObject::tr( "Types of edges to extract." ) );
  addParameter( typeParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Edges" ), Qgis::ProcessingSourceType::VectorLine ) );
}

QString QgsCoverageEdgesAlgorithm::shortDescription() const
{
  return QObject::tr( "Extracts unique edges from a coverage of polygon features." );
}

QString QgsCoverageEdgesAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm operates on a coverage (represented as a set of polygon features "
    "with exactly matching edge geometry) to extract all unique edges of the coverage.\n\n"
    "Extracted edges can be filtered to all edges, exterior edges (edges not shared "
    "between multiple features) or interior edges (edges with multiple features)."
  );
}

QgsCoverageEdgesAlgorithm *QgsCoverageEdgesAlgorithm::createInstance() const
{
  return new QgsCoverageEdgesAlgorithm();
}

QVariantMap QgsCoverageEdgesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const int edgeTypeEnum = parameterAsInt( parameters, u"EDGE_TYPE"_s, context );
  Qgis::CoverageEdgeType edgeType = Qgis::CoverageEdgeType::AllEdges;
  switch ( edgeTypeEnum )
  {
    case 0:
      edgeType = Qgis::CoverageEdgeType::AllEdges;
      break;
    case 1:
      edgeType = Qgis::CoverageEdgeType::Exterior;
      break;
    case 2:
      edgeType = Qgis::CoverageEdgeType::Interior;
      break;
  }

  QString sinkId;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, sinkId, QgsFields(), Qgis::WkbType::LineString, source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

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
  QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest().setNoAttributes() );
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

  QString error;
  QgsGeos geos( &collection );
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

  feedback->pushInfo( QObject::tr( "Extracting edges from coverage" ) );

  std::unique_ptr<QgsAbstractGeometry> edges;
  try
  {
    edges = geos.extractCoverageEdges( edgeType, &error );
  }
  catch ( QgsNotSupportedException &e )
  {
    throw QgsProcessingException( e.what() );
  }

  if ( !edges )
  {
    if ( !error.isEmpty() )
      throw QgsProcessingException( error );
    else
      throw QgsProcessingException( QObject::tr( "No geometry was returned for coverage edges" ) );
  }

  feedback->setProgress( 80 );

  feedback->pushInfo( QObject::tr( "Storing features" ) );
  long long featureIndex = 0;
  for ( auto partsIt = edges->const_parts_begin(); partsIt != edges->const_parts_end(); ++partsIt )
  {
    QgsFeature outFeature;
    outFeature.setGeometry( QgsGeometry( *partsIt ? ( *partsIt )->clone() : nullptr ) );
    if ( !sink->addFeature( outFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    else
      feedback->featureAddedToSink( u"OUTPUT"_s );

    feedback->setProgress( featureIndex * step * 0.2 + 80 );
    featureIndex++;
  }

  sink->finalize();
  feedback->featureSinkFinalized( u"OUTPUT"_s );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, sinkId );
  return outputs;
}

///@endcond
