/***************************************************************************
                         qgsalgorithmdetectdatasetchanges.cpp
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

#include "qgsalgorithmdetectdatasetchanges.h"

#include "qgsspatialindex.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsDetectVectorChangesAlgorithm::name() const
{
  return u"detectvectorchanges"_s;
}

QString QgsDetectVectorChangesAlgorithm::displayName() const
{
  return QObject::tr( "Detect dataset changes" );
}

QStringList QgsDetectVectorChangesAlgorithm::tags() const
{
  return QObject::tr( "added,dropped,new,deleted,features,geometries,difference,delta,revised,original,version" ).split( ',' );
}

QString QgsDetectVectorChangesAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsDetectVectorChangesAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

void QgsDetectVectorChangesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"ORIGINAL"_s, QObject::tr( "Original layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( u"REVISED"_s, QObject::tr( "Revised layer" ) ) );

  auto compareAttributesParam = std::make_unique<QgsProcessingParameterField>( u"COMPARE_ATTRIBUTES"_s, QObject::tr( "Attributes to consider for match (or none to compare geometry only)" ), QVariant(), u"ORIGINAL"_s, Qgis::ProcessingFieldParameterDataType::Any, true, true );
  compareAttributesParam->setDefaultToAllFields( true );
  addParameter( compareAttributesParam.release() );

  std::unique_ptr<QgsProcessingParameterDefinition> matchTypeParam = std::make_unique<QgsProcessingParameterEnum>( u"MATCH_TYPE"_s, QObject::tr( "Geometry comparison behavior" ), QStringList() << QObject::tr( "Exact Match" ) << QObject::tr( "Tolerant Match (Topological Equality)" ), false, 1 );
  matchTypeParam->setFlags( matchTypeParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( matchTypeParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"UNCHANGED"_s, QObject::tr( "Unchanged features" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"ADDED"_s, QObject::tr( "Added features" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"DELETED"_s, QObject::tr( "Deleted features" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true, true ) );

  addOutput( new QgsProcessingOutputNumber( u"UNCHANGED_COUNT"_s, QObject::tr( "Count of unchanged features" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"ADDED_COUNT"_s, QObject::tr( "Count of features added in revised layer" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"DELETED_COUNT"_s, QObject::tr( "Count of features deleted from original layer" ) ) );
}

QString QgsDetectVectorChangesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm compares two vector layers, and determines which features are unchanged, added or deleted between "
                      "the two. It is designed for comparing two different versions of the same dataset.\n\n"
                      "When comparing features, the original and revised feature geometries will be compared against each other. Depending "
                      "on the Geometry Comparison Behavior setting, the comparison will either be made using an exact comparison (where "
                      "geometries must be an exact match for each other, including the order and count of vertices) or a topological "
                      "comparison only (where geometries are considered equal if all of their component edges overlap. E.g. "
                      "lines with the same vertex locations but opposite direction will be considered equal by this method). If the topological "
                      "comparison is selected then any z or m values present in the geometries will not be compared.\n\n"
                      "By default, the algorithm compares all attributes from the original and revised features. If the Attributes to Consider for Match "
                      "parameter is changed, then only the selected attributes will be compared (e.g. allowing users to ignore a timestamp or ID field "
                      "which is expected to change between the revisions).\n\n"
                      "If any features in the original or revised layers do not have an associated geometry, then care must be taken to ensure "
                      "that these features have a unique set of attributes selected for comparison. If this condition is not met, warnings will be "
                      "raised and the resultant outputs may be misleading.\n\n"
                      "The algorithm outputs three layers, one containing all features which are considered to be unchanged between the revisions, "
                      "one containing features deleted from the original layer which are not present in the revised layer, and one containing features "
                      "added to the revised layer which are not present in the original layer." );
}

QString QgsDetectVectorChangesAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates features which are unchanged, added or deleted between two dataset versions." );
}

QgsDetectVectorChangesAlgorithm *QgsDetectVectorChangesAlgorithm::createInstance() const
{
  return new QgsDetectVectorChangesAlgorithm();
}

bool QgsDetectVectorChangesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mOriginal.reset( parameterAsSource( parameters, u"ORIGINAL"_s, context ) );
  if ( !mOriginal )
    throw QgsProcessingException( invalidSourceError( parameters, u"ORIGINAL"_s ) );

  mRevised.reset( parameterAsSource( parameters, u"REVISED"_s, context ) );
  if ( !mRevised )
    throw QgsProcessingException( invalidSourceError( parameters, u"REVISED"_s ) );

  mMatchType = static_cast<GeometryMatchType>( parameterAsEnum( parameters, u"MATCH_TYPE"_s, context ) );

  switch ( mMatchType )
  {
    case Exact:
      if ( mOriginal->wkbType() != mRevised->wkbType() )
        throw QgsProcessingException( QObject::tr( "Geometry type of revised layer (%1) does not match the original layer (%2). Consider using the \"Tolerant Match\" option instead." ).arg( QgsWkbTypes::displayString( mRevised->wkbType() ), QgsWkbTypes::displayString( mOriginal->wkbType() ) ) );
      break;

    case Topological:
      if ( QgsWkbTypes::geometryType( mOriginal->wkbType() ) != QgsWkbTypes::geometryType( mRevised->wkbType() ) )
        throw QgsProcessingException( QObject::tr( "Geometry type of revised layer (%1) does not match the original layer (%2)" ).arg( QgsWkbTypes::geometryDisplayString( QgsWkbTypes::geometryType( mRevised->wkbType() ) ), QgsWkbTypes::geometryDisplayString( QgsWkbTypes::geometryType( mOriginal->wkbType() ) ) ) );
      break;
  }

  if ( mOriginal->sourceCrs() != mRevised->sourceCrs() )
    feedback->reportError( QObject::tr( "CRS for revised layer (%1) does not match the original layer (%2) - reprojection accuracy may affect geometry matching" ).arg( mOriginal->sourceCrs().userFriendlyIdentifier(), mRevised->sourceCrs().userFriendlyIdentifier() ), false );

  mFieldsToCompare = parameterAsStrings( parameters, u"COMPARE_ATTRIBUTES"_s, context );
  mOriginalFieldsToCompareIndices.reserve( mFieldsToCompare.size() );
  mRevisedFieldsToCompareIndices.reserve( mFieldsToCompare.size() );
  QStringList missingOriginalFields;
  QStringList missingRevisedFields;
  for ( const QString &field : mFieldsToCompare )
  {
    const int originalIndex = mOriginal->fields().lookupField( field );
    mOriginalFieldsToCompareIndices.append( originalIndex );
    if ( originalIndex < 0 )
      missingOriginalFields << field;

    const int revisedIndex = mRevised->fields().lookupField( field );
    if ( revisedIndex < 0 )
      missingRevisedFields << field;
    mRevisedFieldsToCompareIndices.append( revisedIndex );
  }

  if ( !missingOriginalFields.empty() )
    throw QgsProcessingException( QObject::tr( "Original layer missing selected comparison attributes: %1" ).arg( missingOriginalFields.join( ',' ) ) );
  if ( !missingRevisedFields.empty() )
    throw QgsProcessingException( QObject::tr( "Revised layer missing selected comparison attributes: %1" ).arg( missingRevisedFields.join( ',' ) ) );

  return true;
}

QVariantMap QgsDetectVectorChangesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString unchangedDestId;
  std::unique_ptr<QgsFeatureSink> unchangedSink( parameterAsSink( parameters, u"UNCHANGED"_s, context, unchangedDestId, mOriginal->fields(), mOriginal->wkbType(), mOriginal->sourceCrs() ) );
  if ( !unchangedSink && parameters.value( u"UNCHANGED"_s ).isValid() )
    throw QgsProcessingException( invalidSinkError( parameters, u"UNCHANGED"_s ) );

  QString addedDestId;
  std::unique_ptr<QgsFeatureSink> addedSink( parameterAsSink( parameters, u"ADDED"_s, context, addedDestId, mRevised->fields(), mRevised->wkbType(), mRevised->sourceCrs() ) );
  if ( !addedSink && parameters.value( u"ADDED"_s ).isValid() )
    throw QgsProcessingException( invalidSinkError( parameters, u"ADDED"_s ) );

  QString deletedDestId;
  std::unique_ptr<QgsFeatureSink> deletedSink( parameterAsSink( parameters, u"DELETED"_s, context, deletedDestId, mOriginal->fields(), mOriginal->wkbType(), mOriginal->sourceCrs() ) );
  if ( !deletedSink && parameters.value( u"DELETED"_s ).isValid() )
    throw QgsProcessingException( invalidSinkError( parameters, u"DELETED"_s ) );

  // first iteration: we loop through the entire original layer, building up a spatial index of ALL original geometries
  // and collecting the original geometries themselves along with the attributes to compare
  QgsFeatureRequest request;
  request.setSubsetOfAttributes( mOriginalFieldsToCompareIndices );

  QgsFeatureIterator it = mOriginal->getFeatures( request );

  double step = mOriginal->featureCount() > 0 ? 100.0 / mOriginal->featureCount() : 0;
  QHash<QgsFeatureId, QgsGeometry> originalGeometries;
  QHash<QgsFeatureId, QgsAttributes> originalAttributes;
  QHash<QgsAttributes, QgsFeatureId> originalNullGeometryAttributes;
  long current = 0;

  QgsAttributes attrs;
  attrs.resize( mFieldsToCompare.size() );

  const QgsSpatialIndex index( it, [&]( const QgsFeature &f ) -> bool {
    if ( feedback->isCanceled() )
      return false;

    if ( f.hasGeometry() )
    {
      originalGeometries.insert( f.id(), f.geometry() );
    }

    if ( !mFieldsToCompare.empty() )
    {
      int idx = 0;
      for ( const int field : mOriginalFieldsToCompareIndices )
      {
        attrs[idx++] = f.attributes().at( field );
      }
      originalAttributes.insert( f.id(), attrs );
    }

    if ( !f.hasGeometry() )
    {
      if ( originalNullGeometryAttributes.contains( attrs ) )
      {
        feedback->reportError( QObject::tr( "A non-unique set of comparison attributes was found for "
                                            "one or more features without geometries - results may be misleading (features %1 and %2)" )
                                 .arg( f.id() )
                                 .arg( originalNullGeometryAttributes.value( attrs ) ) );
      }
      else
      {
        originalNullGeometryAttributes.insert( attrs, f.id() );
      }
    }

    // overall this loop takes about 10% of time
    current++;
    feedback->setProgress( 0.10 * current * step );
    return true;
  } );

  QSet<QgsFeatureId> unchangedOriginalIds;
  QSet<QgsFeatureId> addedRevisedIds;
  current = 0;

  // second iteration: we loop through ALL revised features, checking whether each is a match for a geometry from the
  // original set. If so, check if the feature is unchanged. If there's no match with the original features, we mark it as an "added" feature
  step = mRevised->featureCount() > 0 ? 100.0 / mRevised->featureCount() : 0;
  QgsFeatureRequest revisedRequest = QgsFeatureRequest().setDestinationCrs( mOriginal->sourceCrs(), context.transformContext() );
  revisedRequest.setSubsetOfAttributes( mRevisedFieldsToCompareIndices );
  it = mRevised->getFeatures( revisedRequest );
  QgsFeature revisedFeature;
  while ( it.nextFeature( revisedFeature ) )
  {
    if ( feedback->isCanceled() )
      break;

    int idx = 0;
    for ( const int field : mRevisedFieldsToCompareIndices )
    {
      attrs[idx++] = revisedFeature.attributes().at( field );
    }

    bool matched = false;

    if ( !revisedFeature.hasGeometry() )
    {
      if ( originalNullGeometryAttributes.contains( attrs ) )
      {
        // found a match for feature
        unchangedOriginalIds.insert( originalNullGeometryAttributes.value( attrs ) );
        matched = true;
      }
    }
    else
    {
      // can we match this feature?
      const QList<QgsFeatureId> candidates = index.intersects( revisedFeature.geometry().boundingBox() );

      // lazy evaluate -- there may be NO candidates!
      QgsGeometry revised;

      for ( const QgsFeatureId candidateId : candidates )
      {
        if ( unchangedOriginalIds.contains( candidateId ) )
        {
          // already matched this original feature
          continue;
        }

        // attribute comparison is faster to do first, if desired
        if ( !mFieldsToCompare.empty() )
        {
          if ( attrs != originalAttributes[candidateId] )
          {
            // attributes don't match, so candidates is not a match
            continue;
          }
        }

        QgsGeometry original = originalGeometries.value( candidateId );
        // lazy evaluation
        if ( revised.isNull() )
        {
          revised = revisedFeature.geometry();
          // drop z/m if not wanted for match
          switch ( mMatchType )
          {
            case Topological:
            {
              revised.get()->dropMValue();
              revised.get()->dropZValue();
              original.get()->dropMValue();
              original.get()->dropZValue();
              break;
            }

            case Exact:
              break;
          }
        }

        bool geometryMatch = false;
        switch ( mMatchType )
        {
          case Topological:
          {
            geometryMatch = revised.isGeosEqual( original );
            break;
          }

          case Exact:
            geometryMatch = revised.equals( original );
            break;
        }

        if ( geometryMatch )
        {
          // candidate is a match for feature
          unchangedOriginalIds.insert( candidateId );
          matched = true;
          break;
        }
      }
    }

    if ( !matched )
    {
      // new feature
      addedRevisedIds.insert( revisedFeature.id() );
    }

    current++;
    feedback->setProgress( 0.70 * current * step + 10 ); // takes about 70% of time
  }

  // third iteration: iterate back over the original features, and direct them to the appropriate sink.
  // If they were marked as unchanged during the second iteration, we put them in the unchanged sink. Otherwise
  // they are placed into the deleted sink.
  step = mOriginal->featureCount() > 0 ? 100.0 / mOriginal->featureCount() : 0;

  request = QgsFeatureRequest().setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  it = mOriginal->getFeatures( request );
  current = 0;
  long deleted = 0;
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    // use already fetched geometry
    f.setGeometry( originalGeometries.value( f.id(), QgsGeometry() ) );

    if ( unchangedOriginalIds.contains( f.id() ) )
    {
      // unchanged
      if ( unchangedSink )
      {
        if ( !unchangedSink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( unchangedSink.get(), parameters, u"UNCHANGED"_s ) );
      }
    }
    else
    {
      // deleted feature
      if ( deletedSink )
      {
        if ( !deletedSink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( deletedSink.get(), parameters, u"DELETED"_s ) );
      }
      deleted++;
    }

    current++;
    feedback->setProgress( 0.10 * current * step + 80 ); // takes about 10% of time
  }

  // forth iteration: collect all added features and add them to the added sink
  // NOTE: while we could potentially do this as part of the second iteration and save some time, we instead
  // do this here using a brand new request because the second iteration
  // is fetching reprojected features and we ideally want geometries from the revised layer's actual CRS only here!
  // also, the second iteration is only fetching the actual attributes used in the comparison, whereas we want
  // to include all attributes in the "added" output
  if ( addedSink )
  {
    step = addedRevisedIds.size() > 0 ? 100.0 / addedRevisedIds.size() : 0;
    it = mRevised->getFeatures( QgsFeatureRequest().setFilterFids( addedRevisedIds ) );
    current = 0;
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
        break;

      // added feature
      if ( !addedSink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( addedSink.get(), parameters, u"ADDED"_s ) );

      current++;
      feedback->setProgress( 0.10 * current * step + 90 ); // takes about 10% of time
    }
  }
  feedback->setProgress( 100 );

  feedback->pushInfo( QObject::tr( "%n feature(s) unchanged", nullptr, unchangedOriginalIds.size() ) );
  feedback->pushInfo( QObject::tr( "%n feature(s) added", nullptr, addedRevisedIds.size() ) );
  feedback->pushInfo( QObject::tr( "%n feature(s) deleted", nullptr, deleted ) );

  if ( unchangedSink )
    unchangedSink->finalize();
  if ( addedSink )
    addedSink->finalize();
  if ( deletedSink )
    deletedSink->finalize();

  QVariantMap outputs;
  outputs.insert( u"UNCHANGED"_s, unchangedDestId );
  outputs.insert( u"ADDED"_s, addedDestId );
  outputs.insert( u"DELETED"_s, deletedDestId );
  outputs.insert( u"UNCHANGED_COUNT"_s, static_cast<long long>( unchangedOriginalIds.size() ) );
  outputs.insert( u"ADDED_COUNT"_s, static_cast<long long>( addedRevisedIds.size() ) );
  outputs.insert( u"DELETED_COUNT"_s, static_cast<long long>( deleted ) );

  return outputs;
}

///@endcond
