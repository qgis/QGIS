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
  return QObject::tr( "topological,boundary,repair" ).split( ',' );
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
  auto gapWidthParam = std::make_unique<QgsProcessingParameterDistance>( u"GAP_WIDTH"_s, QObject::tr( "Maximum gap width" ), 0.0, u"INPUT"_s, false, 0, 10000000.0 );
  gapWidthParam->setHelp(
    QObject::tr(
      "Gaps which are narrower than this distance are merged with an adjacent polygon. Polygon width is determined as twice the radius of the maximum inscribed circle of the gap polygon. Empty holes "
      "in input polygons are treated as gaps, and may be filled in. Gaps which are not fully enclosed ('inlets') are not removed."
    )
  );
  auto snappingDistanceParam = std::make_unique<QgsProcessingParameterDistance>( u"SNAPPING_DISTANCE"_s, QObject::tr( "Snapping distance" ), QVariant(), u"INPUT"_s, true, 0.0, 10000000.0 );
  snappingDistanceParam->setHelp(
    QObject::tr(
      "Snapping to nearby vertices and line segment snapping is used to improve noding robustness and eliminate small errors in an efficient way. By default the snapping distance is not set, which "
      "means that the clean operation uses a very small snapping distance based on the extent of the input data. A distance of zero prevents snapping from being used."
    )
  );
  auto mergeStrategyParam = std::make_unique<QgsProcessingParameterEnum>(
    u"OVERLAP_MERGE_STRATEGY"_s,
    QObject::tr( "Overlap merge strategy" ),
    QStringList() << QObject::tr( "Longest border" ) << QObject::tr( "Maximum area" ) << QObject::tr( "Minimum area" ) << QObject::tr( "Minimum index" ),
    false,
    0
  );
  mergeStrategyParam->setHelp(
    QObject::tr(
      "Determines the strategy used to merge gaps with adjacent polygons. Strategies include merging with the polygon sharing the longest border, the polygon with the maximum or minimum area, or the "
      "first encountered polygon (Minimum Index)."
    )
  );

  addParameter( gapWidthParam.release() );
  addParameter( snappingDistanceParam.release() );
  addParameter( mergeStrategyParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Cleaned" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

QString QgsCoverageCleanAlgorithm::shortDescription() const
{
  return QObject::tr( "Cleans a coverage of polygon features to fix cases where the geometries do not exactly align." );
}

QString QgsCoverageCleanAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm operates on a coverage (represented as a list of polygon features) "
    "to fix cases where the geometry does not in fact exactly match, repairing small "
    "overlaps and gaps to restore a valid topological coverage.\n\n"
    "It uses a combination of snapping and gap/overlap merging. Snapping to nearby vertices "
    "and line segments improves noding robustness and eliminates small errors. If the snapping "
    "distance is not specified, an automatic distance based on the data extent is used. A distance "
    "of 0 disables snapping.\n\n"
    "Gaps and holes that are narrower than the Maximum Gap Width are merged with an adjacent polygon. "
    "Gaps which are not fully enclosed ('inlets') are not removed.\n\n"
    "When repairing overlaps and gaps, the Overlap Merge Strategy determines which adjacent "
    "polygon the problematic area is merged into (e.g., merging with the polygon that shares "
    "the longest border, or the one with the maximum/minimum area).\n\n"
    "Polygons that have collapsed during cleaning will be returned as empty polygons."
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
  QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest(), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
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

  QgsCoverageCleanParameters cleanParameters;
  if ( parameters.value( u"SNAPPING_DISTANCE"_s ).isValid() )
  {
    cleanParameters.setSnappingDistance( parameterAsDouble( parameters, u"SNAPPING_DISTANCE"_s, context ) );
  }
  cleanParameters.setMaximumGapWidth( parameterAsDouble( parameters, u"MAXIMUM_GAP_WIDTH"_s, context ) );
  switch ( parameterAsEnum( parameters, u"OVERLAP_MERGE_STRATEGY"_s, context ) )
  {
    case 0:
      cleanParameters.setOverlapMergeStrategy( Qgis::CoverageCleanOverlapMergeStrategy::LongestBorder );
      break;
    case 1:
      cleanParameters.setOverlapMergeStrategy( Qgis::CoverageCleanOverlapMergeStrategy::MaximumArea );
      break;
    case 2:
      cleanParameters.setOverlapMergeStrategy( Qgis::CoverageCleanOverlapMergeStrategy::MinimumArea );
      break;
    case 3:
      cleanParameters.setOverlapMergeStrategy( Qgis::CoverageCleanOverlapMergeStrategy::MinimumIndex );
      break;
    default:
      break;
  }

  feedback->pushInfo( QObject::tr( "Cleaning coverage" ) );
  std::unique_ptr<QgsAbstractGeometry> cleaned;
  try
  {
    cleaned = geos.cleanCoverage( cleanParameters, &error, feedback );
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
