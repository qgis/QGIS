/***************************************************************************
                         qgsalgorithmdeleteduplicategeometries.cpp
                         -----------------------------------------
    begin                : December 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgsalgorithmdeleteduplicategeometries.h"

#include "qgsgeometryengine.h"
#include "qgsspatialindex.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsDeleteDuplicateGeometriesAlgorithm::name() const
{
  return u"deleteduplicategeometries"_s;
}

QString QgsDeleteDuplicateGeometriesAlgorithm::displayName() const
{
  return QObject::tr( "Delete duplicate geometries" );
}

QStringList QgsDeleteDuplicateGeometriesAlgorithm::tags() const
{
  return QObject::tr( "drop,remove,same,points,coincident,overlapping,filter" ).split( ',' );
}

QString QgsDeleteDuplicateGeometriesAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsDeleteDuplicateGeometriesAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

void QgsDeleteDuplicateGeometriesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Cleaned" ) ) );

  QgsProcessingParameterFeatureSink *duplicatesOutput = new QgsProcessingParameterFeatureSink( u"DUPLICATES"_s, QObject::tr( "Duplicates" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true );
  duplicatesOutput->setCreateByDefault( false );
  addParameter( duplicatesOutput );

  addOutput( new QgsProcessingOutputNumber( u"RETAINED_COUNT"_s, QObject::tr( "Count of retained records" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"DUPLICATE_COUNT"_s, QObject::tr( "Count of discarded duplicate records" ) ) );
}

QString QgsDeleteDuplicateGeometriesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm finds duplicated geometries and removes them.\n\nAttributes are not checked, "
                      "so in case two features have identical geometries but different attributes, only one of "
                      "them will be added to the result layer.\n\n"
                      "Optionally, these duplicate features can be saved to a separate output for analysis." );
}

QString QgsDeleteDuplicateGeometriesAlgorithm::shortDescription() const
{
  return QObject::tr( "Finds duplicated geometries in a layer and removes them." );
}

QgsDeleteDuplicateGeometriesAlgorithm *QgsDeleteDuplicateGeometriesAlgorithm::createInstance() const
{
  return new QgsDeleteDuplicateGeometriesAlgorithm();
}

bool QgsDeleteDuplicateGeometriesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  return true;
}

QVariantMap QgsDeleteDuplicateGeometriesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString destId;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, destId, mSource->fields(), mSource->wkbType(), mSource->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QString dupesSinkId;
  std::unique_ptr<QgsFeatureSink> dupesSink( parameterAsSink( parameters, u"DUPLICATES"_s, context, dupesSinkId, mSource->fields(), mSource->wkbType(), mSource->sourceCrs() ) );

  QgsFeatureIterator it = mSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );

  double step = mSource->featureCount() > 0 ? 100.0 / mSource->featureCount() : 0;
  QHash<QgsFeatureId, QgsGeometry> geometries;
  QSet<QgsFeatureId> nullGeometryFeatures;
  long current = 0;
  const QgsSpatialIndex index( it, [&]( const QgsFeature &f ) -> bool {
    if ( feedback->isCanceled() )
      return false;

    if ( !f.hasGeometry() )
    {
      nullGeometryFeatures.insert( f.id() );
    }
    else
    {
      geometries.insert( f.id(), f.geometry() );
    }

    // overall this loop takes about 10% of time
    current++;
    feedback->setProgress( 0.10 * static_cast<double>( current ) * step );
    return true;
  } );

  QgsFeature f;

  // start by assuming everything is unique, and chop away at this list
  QHash<QgsFeatureId, QgsGeometry> uniqueFeatures = geometries;
  QHash<QgsFeatureId, QgsGeometry> duplicateFeatures;
  current = 0;
  long removed = 0;

  for ( auto it = geometries.constBegin(); it != geometries.constEnd(); ++it )
  {
    const QgsFeatureId featureId = it.key();
    const QgsGeometry geometry = it.value();

    if ( feedback->isCanceled() )
      break;

    if ( !uniqueFeatures.contains( featureId ) )
    {
      // feature was already marked as a duplicate
    }
    else
    {
      const QList<QgsFeatureId> candidates = index.intersects( geometry.boundingBox() );

      for ( const QgsFeatureId candidateId : candidates )
      {
        if ( candidateId == featureId )
          continue;

        if ( !uniqueFeatures.contains( candidateId ) )
        {
          // candidate already marked as a duplicate (not sure if this is possible,
          // since it would mean the current feature would also have to be a duplicate!
          // but let's be safe!)
          continue;
        }

        const QgsGeometry candidateGeom = geometries.value( candidateId );
        if ( geometry.isGeosEqual( candidateGeom ) )
        {
          // candidate is a duplicate of feature
          uniqueFeatures.remove( candidateId );
          if ( dupesSink )
          {
            duplicateFeatures.insert( candidateId, candidateGeom );
          }
          removed++;
        }
      }
    }

    current++;
    feedback->setProgress( 0.80 * static_cast<double>( current ) * step + 10 ); // takes about 80% of time
  }

  // now, fetch all the feature attributes for the unique features only
  // be super-smart and don't re-fetch geometries
  QSet<QgsFeatureId> outputFeatureIds = qgis::listToSet( uniqueFeatures.keys() );
  outputFeatureIds.unite( nullGeometryFeatures );
  step = outputFeatureIds.empty() ? 1 : 100.0 / outputFeatureIds.size();
  const double stepTime = dupesSink ? 0.05 : 0.10;

  const QgsFeatureRequest request = QgsFeatureRequest().setFilterFids( outputFeatureIds ).setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  it = mSource->getFeatures( request, Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  current = 0;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    // use already fetched geometry
    if ( !nullGeometryFeatures.contains( f.id() ) )
    {
      f.setGeometry( uniqueFeatures.value( f.id() ) );
    }
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );

    current++;
    feedback->setProgress( stepTime * static_cast<double>( current ) * step + 90 ); // takes about 5%-10% of time
  }

  feedback->pushInfo( QObject::tr( "%n duplicate feature(s) removed", nullptr, removed ) );

  sink->finalize();

  if ( dupesSink )
  {
    // now, fetch all the feature attributes for the duplicate features
    QSet<QgsFeatureId> duplicateFeatureIds = qgis::listToSet( duplicateFeatures.keys() );
    step = duplicateFeatureIds.empty() ? 1 : 100.0 / duplicateFeatureIds.size();

    const QgsFeatureRequest request = QgsFeatureRequest().setFilterFids( duplicateFeatureIds ).setFlags( Qgis::FeatureRequestFlag::NoGeometry );
    it = mSource->getFeatures( request, Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
    current = 0;
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
        break;

      // use already fetched geometry
      f.setGeometry( duplicateFeatures.value( f.id() ) );
      if ( !dupesSink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( dupesSink.get(), parameters, u"DUPLICATES"_s ) );

      current++;
      feedback->setProgress( 0.05 * static_cast<double>( current ) * step + 95 ); // takes about 5% of time
    }

    dupesSink->finalize();
  }

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, destId );
  outputs.insert( u"DUPLICATE_COUNT"_s, static_cast<long long>( removed ) );
  outputs.insert( u"RETAINED_COUNT"_s, outputFeatureIds.size() );
  if ( dupesSink )
  {
    outputs.insert( u"DUPLICATES"_s, dupesSinkId );
  }
  return outputs;
}

///@endcond
