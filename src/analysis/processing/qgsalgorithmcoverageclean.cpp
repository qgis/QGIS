/***************************************************************************
                         qgsalgorithmcoverageclean.h
                         ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Juho Ervasti
    email                : juho dot ervasti at gispo dot fi
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsalgorithmcoverageclean.h"

#include "qgsgeometrycollection.h"
#include "qgsgeos.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsCoverageCleanAlgorithm::name() const
{
  return u"coverageclean"_s;
}

QString QgsCoverageCleanAlgorithm::displayName() const
{
  return QObject::tr( "Clean coverage" );
}

QStringList QgsCoverageCleanAlgorithm::tags() const
{
  return QObject::tr( "topological,boundary" ).split( ',' );
}

QString QgsCoverageCleanAlgorithm::group() const
{
  return QObject::tr( "Vector coverage" );
}

QString QgsCoverageCleanAlgorithm::groupId() const
{
  return u"vectorcoverage"_s;
}

void QgsCoverageCleanAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) ) );
  auto gapWidthParam = std::make_unique<QgsProcessingParameterDistance>( u"GAP_WIDTH"_s, QObject::tr( "Gap width" ), 0.0, u"INPUT"_s, false, 0, 10000000.0 );
  gapWidthParam->setHelp( QObject::tr( "The maximum width of gaps to detect" ) );
  auto snappingDistanceParam = std::make_unique<QgsProcessingParameterDistance>( u"SNAPPING_DISTANCE"_s, QObject::tr( "Snapping distance" ), -1.0, u"INPUT"_s, false, -1, 10000000.0 );
  snappingDistanceParam->setHelp( QObject::tr( "The snapping distance at which nearby vertices are snapped together to. When -1 a snapping distance is calculated based on input." ) );
  auto mergeStrategyParam = std::make_unique<QgsProcessingParameterEnum>(
    u"OVERLAP_MERGE_STRATEGY"_s,
    QObject::tr( "Overlap merge strategy" ),
    QStringList() << QObject::tr( "Longest border" ) << QObject::tr( "Maximum area" ) << QObject::tr( "Minimum area" ) << QObject::tr( "Minimum index" ),
    false,
    0
  );
  mergeStrategyParam->setHelp( QObject::tr( "Determines which neighboring polygons to merge overlapping areas into." ) );

  addParameter( gapWidthParam.release() );
  addParameter( snappingDistanceParam.release() );
  addParameter( mergeStrategyParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Cleaned" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

QString QgsCoverageCleanAlgorithm::shortDescription() const
{
  return QObject::tr( "Cleans a coverage of polygon features which may have gaps and overlaps by removing both." );
}

QString QgsCoverageCleanAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm operates on a coverage (represented as a list of polygonal geometry) "
    "and cleans the coverage by removing overlapping areas and closing small gaps."
  );
}

QgsCoverageCleanAlgorithm *QgsCoverageCleanAlgorithm::createInstance() const
{
  return new QgsCoverageCleanAlgorithm();
}

QVariantMap QgsCoverageCleanAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const double gapWidth = parameterAsDouble( parameters, u"GAP_WIDTH"_s, context );
  const double snappingDistance = parameterAsDouble( parameters, u"SNAPPING_DISTANCE"_s, context );
  const auto mergeStrategy = static_cast<Qgis::CoverageCleanOverlapMergeStrategy>( parameterAsEnum( parameters, u"OVERLAP_MERGE_STRATEGY"_s, context ) );

  QString sinkId;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, sinkId, source->fields(), source->wkbType(), source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  // build list of features in advance
  QVector<QgsFeature> featuresWithGeom;
  QVector<QgsFeature> featuresWithoutGeom;
  QgsGeometryCollection collection;

  const long count = source->featureCount();
  if ( count > 0 )
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

  QString error;
  QgsGeos geos( &collection );

  feedback->pushInfo( QObject::tr( "Simplifying coverage" ) );
  std::unique_ptr<QgsAbstractGeometry> cleaned;
  try
  {
    cleaned = geos.cleanCoverage( gapWidth, snappingDistance, mergeStrategy, &error );
  }
  catch ( QgsNotSupportedException &e )
  {
    throw QgsProcessingException( e.what() );
  }

  if ( !cleaned )
  {
    if ( !error.isEmpty() )
      throw QgsProcessingException( error );
    else
      throw QgsProcessingException( QObject::tr( "No geometry was returned for cleaned coverage" ) );
  }

  feedback->setProgress( 80 );

  feedback->pushInfo( QObject::tr( "Storing features" ) );
  long long featureIndex = 0;
  for ( auto partsIt = cleaned->const_parts_begin(); partsIt != cleaned->const_parts_end(); ++partsIt )
  {
    QgsFeature outFeature = featuresWithGeom.value( featureIndex );
    outFeature.setGeometry( QgsGeometry( *partsIt ? ( *partsIt )->clone() : nullptr ) );
    if ( !sink->addFeature( outFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    else
      feedback->featureAddedToSink( u"OUTPUT"_s );

    feedback->setProgress( featureIndex * step * 0.2 + 80 );
    featureIndex++;
  }

  for ( const QgsFeature &feature : featuresWithoutGeom )
  {
    QgsFeature outFeature = feature;
    if ( !sink->addFeature( outFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    else
      feedback->featureAddedToSink( u"OUTPUT"_s );
  }

  sink->finalize();
  feedback->featureSinkFinalized( u"OUTPUT"_s );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, sinkId );
  return outputs;
}

///@endcond
