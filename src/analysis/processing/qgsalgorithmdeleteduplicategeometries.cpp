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
#include "qgsvectorlayer.h"
#include "qgsgeometryengine.h"

///@cond PRIVATE

QString QgsDeleteDuplicateGeometriesAlgorithm::name() const
{
  return QStringLiteral( "deleteduplicategeometries" );
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
  return QStringLiteral( "vectorgeneral" );
}

void QgsDeleteDuplicateGeometriesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Cleaned" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "RETAINED_COUNT" ), QObject::tr( "Count of retained records" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "DUPLICATE_COUNT" ), QObject::tr( "Count of discarded duplicate records" ) ) );
}

QString QgsDeleteDuplicateGeometriesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm finds duplicated geometries and removes them.\n\nAttributes are not checked, "
                      "so in case two features have identical geometries but different attributes, only one of "
                      "them will be added to the result layer." );
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
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  return true;
}

QVariantMap QgsDeleteDuplicateGeometriesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString destId;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, destId, mSource->fields(),
                                          mSource->wkbType(), mSource->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QgsFeatureIterator it = mSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) );

  double step = mSource->featureCount() > 0 ? 100.0 / mSource->featureCount() : 0;
  QHash< QgsFeatureId, QgsGeometry > geometries;
  QSet< QgsFeatureId > nullGeometryFeatures;
  long current = 0;
  const QgsSpatialIndex index( it, [&]( const QgsFeature & f ) ->bool
  {
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
    feedback->setProgress( 0.10 * current * step );
    return true;
  } );

  QgsFeature f;

  // start by assuming everything is unique, and chop away at this list
  QHash< QgsFeatureId, QgsGeometry > uniqueFeatures = geometries;
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
        else if ( geometry.isGeosEqual( geometries.value( candidateId ) ) )
        {
          // candidate is a duplicate of feature
          uniqueFeatures.remove( candidateId );
          removed++;
        }
      }
    }

    current++;
    feedback->setProgress( 0.80 * current * step + 10 ); // takes about 80% of time
  }

  // now, fetch all the feature attributes for the unique features only
  // be super-smart and don't re-fetch geometries
  QSet< QgsFeatureId > outputFeatureIds = qgis::listToSet( uniqueFeatures.keys() );
  outputFeatureIds.unite( nullGeometryFeatures );
  step = outputFeatureIds.empty() ? 1 : 100.0 / outputFeatureIds.size();

  const QgsFeatureRequest request = QgsFeatureRequest().setFilterFids( outputFeatureIds ).setFlags( QgsFeatureRequest::NoGeometry );
  it = mSource->getFeatures( request );
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
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

    current++;
    feedback->setProgress( 0.10 * current * step + 90 ); // takes about 10% of time
  }

  feedback->pushInfo( QObject::tr( "%n duplicate feature(s) removed", nullptr, removed ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), destId );
  outputs.insert( QStringLiteral( "DUPLICATE_COUNT" ), static_cast< long long >( removed ) );
  outputs.insert( QStringLiteral( "RETAINED_COUNT" ), outputFeatureIds.size() );
  return outputs;
}

///@endcond
