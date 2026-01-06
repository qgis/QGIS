/***************************************************************************
                         qgsalgorithmjoinbylocation.cpp
                         ---------------------
    begin                : January 2020
    copyright            : (C) 2020 by Alexis Roy-Lizotte
    email                : roya2 at premiertech dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmjoinbylocation.h"

#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsfeaturesource.h"
#include "qgsgeometryengine.h"
#include "qgsprocessing.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE


void QgsJoinByLocationAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Join to features in" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );

  auto predicateParam = std::make_unique<QgsProcessingParameterEnum>( u"PREDICATE"_s, QObject::tr( "Features they (geometric predicate)" ), translatedPredicates(), true, 0 );
  QVariantMap predicateMetadata;
  QVariantMap widgetMetadata;
  widgetMetadata.insert( u"useCheckBoxes"_s, true );
  widgetMetadata.insert( u"columns"_s, 2 );
  predicateMetadata.insert( u"widget_wrapper"_s, widgetMetadata );
  predicateParam->setMetadata( predicateMetadata );
  addParameter( predicateParam.release() );
  addParameter( new QgsProcessingParameterFeatureSource( u"JOIN"_s, QObject::tr( "By comparing to" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );
  addParameter( new QgsProcessingParameterField( u"JOIN_FIELDS"_s, QObject::tr( "Fields to add (leave empty to use all fields)" ), QVariant(), u"JOIN"_s, Qgis::ProcessingFieldParameterDataType::Any, true, true ) );

  QStringList joinMethods;
  joinMethods << QObject::tr( "Create separate feature for each matching feature (one-to-many)" )
              << QObject::tr( "Take attributes of the first matching feature only (one-to-one)" )
              << QObject::tr( "Take attributes of the feature with largest overlap only (one-to-one)" );
  addParameter( new QgsProcessingParameterEnum( u"METHOD"_s, QObject::tr( "Join type" ), joinMethods, false, static_cast<int>( OneToMany ) ) );
  addParameter( new QgsProcessingParameterBoolean( u"DISCARD_NONMATCHING"_s, QObject::tr( "Discard records which could not be joined" ), false ) );
  addParameter( new QgsProcessingParameterString( u"PREFIX"_s, QObject::tr( "Joined field prefix" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Joined layer" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"NON_MATCHING"_s, QObject::tr( "Unjoinable features from first layer" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true, false ) );
  addOutput( new QgsProcessingOutputNumber( u"JOINED_COUNT"_s, QObject::tr( "Number of joined features from input table" ) ) );
}

QString QgsJoinByLocationAlgorithm::name() const
{
  return u"joinattributesbylocation"_s;
}

QString QgsJoinByLocationAlgorithm::displayName() const
{
  return QObject::tr( "Join attributes by location" );
}

QStringList QgsJoinByLocationAlgorithm::tags() const
{
  return QObject::tr( "join,intersects,intersecting,touching,within,contains,overlaps,relation,spatial" ).split( ',' );
}

QString QgsJoinByLocationAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsJoinByLocationAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

QString QgsJoinByLocationAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes an input vector layer and creates a new vector layer "
                      "that is an extended version of the input one, with additional attributes in its attribute table.\n\n"
                      "The additional attributes and their values are taken from a second vector layer. "
                      "A spatial criteria is applied to select the values from the second layer that are added "
                      "to each feature from the first layer in the resulting one." );
}

QString QgsJoinByLocationAlgorithm::shortDescription() const
{
  return QObject::tr( "Joins attributes from one vector layer to another by location." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsJoinByLocationAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsJoinByLocationAlgorithm *QgsJoinByLocationAlgorithm::createInstance() const
{
  return new QgsJoinByLocationAlgorithm();
}

QStringList QgsJoinByLocationAlgorithm::translatedPredicates()
{
  return { QObject::tr( "intersect" ), QObject::tr( "contain" ), QObject::tr( "equal" ), QObject::tr( "touch" ), QObject::tr( "overlap" ), QObject::tr( "are within" ), QObject::tr( "cross" ) };
}

QVariantMap QgsJoinByLocationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mBaseSource.reset( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !mBaseSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  mJoinSource.reset( parameterAsSource( parameters, u"JOIN"_s, context ) );
  if ( !mJoinSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"JOIN"_s ) );

  mJoinMethod = static_cast<JoinMethod>( parameterAsEnum( parameters, u"METHOD"_s, context ) );

  const QStringList joinedFieldNames = parameterAsStrings( parameters, u"JOIN_FIELDS"_s, context );

  mPredicates = parameterAsEnums( parameters, u"PREDICATE"_s, context );
  sortPredicates( mPredicates );

  QString prefix = parameterAsString( parameters, u"PREFIX"_s, context );

  QgsFields joinFields;
  if ( joinedFieldNames.empty() )
  {
    joinFields = mJoinSource->fields();
    mJoinedFieldIndices = joinFields.allAttributesList();
  }
  else
  {
    mJoinedFieldIndices.reserve( joinedFieldNames.count() );
    for ( const QString &field : joinedFieldNames )
    {
      int index = mJoinSource->fields().lookupField( field );
      if ( index >= 0 )
      {
        mJoinedFieldIndices << index;
        joinFields.append( mJoinSource->fields().at( index ) );
      }
    }
  }

  if ( !prefix.isEmpty() )
  {
    for ( int i = 0; i < joinFields.count(); ++i )
    {
      joinFields.rename( i, prefix + joinFields[i].name() );
    }
  }

  const QgsFields outputFields = QgsProcessingUtils::combineFields( mBaseSource->fields(), joinFields );

  QString joinedSinkId;
  mJoinedFeatures.reset( parameterAsSink( parameters, u"OUTPUT"_s, context, joinedSinkId, outputFields, mBaseSource->wkbType(), mBaseSource->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );

  if ( parameters.value( u"OUTPUT"_s ).isValid() && !mJoinedFeatures )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  mDiscardNonMatching = parameterAsBoolean( parameters, u"DISCARD_NONMATCHING"_s, context );

  QString nonMatchingSinkId;
  mUnjoinedFeatures.reset( parameterAsSink( parameters, u"NON_MATCHING"_s, context, nonMatchingSinkId, mBaseSource->fields(), mBaseSource->wkbType(), mBaseSource->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( parameters.value( u"NON_MATCHING"_s ).isValid() && !mUnjoinedFeatures )
    throw QgsProcessingException( invalidSinkError( parameters, u"NON_MATCHING"_s ) );

  switch ( mJoinMethod )
  {
    case OneToMany:
    case JoinToFirst:
    {
      if ( mBaseSource->featureCount() > 0 && mJoinSource->featureCount() > 0 && mBaseSource->featureCount() < mJoinSource->featureCount() )
      {
        // joining FEWER features to a layer with MORE features. So we iterate over the FEW features and find matches from the MANY
        processAlgorithmByIteratingOverInputSource( context, feedback );
      }
      else
      {
        // default -- iterate over the join source and match back to the base source. We do this on the assumption that the most common
        // use case is joining a points layer to a polygon layer (taking polygon attributes and adding them to the points), so by iterating
        // over the polygons we can take advantage of prepared geometries for the spatial relationship test.

        // TODO - consider using more heuristics to determine whether it's always best to iterate over the join
        // source.
        processAlgorithmByIteratingOverJoinedSource( context, feedback );
      }
      break;
    }

    case JoinToLargestOverlap:
      processAlgorithmByIteratingOverInputSource( context, feedback );
      break;
  }

  QVariantMap outputs;
  if ( mJoinedFeatures )
  {
    mJoinedFeatures->finalize();
    outputs.insert( u"OUTPUT"_s, joinedSinkId );
  }
  if ( mUnjoinedFeatures )
  {
    mUnjoinedFeatures->finalize();
    outputs.insert( u"NON_MATCHING"_s, nonMatchingSinkId );
  }

  // need to release sinks to finalize writing
  mJoinedFeatures.reset();
  mUnjoinedFeatures.reset();

  outputs.insert( u"JOINED_COUNT"_s, static_cast<long long>( mJoinedCount ) );
  return outputs;
}

bool QgsJoinByLocationAlgorithm::featureFilter( const QgsFeature &feature, QgsGeometryEngine *engine, bool comparingToJoinedFeature, const QList<int> &predicates )
{
  const QgsAbstractGeometry *geom = feature.geometry().constGet();
  bool ok = false;
  for ( const int predicate : predicates )
  {
    switch ( predicate )
    {
      case 0:
        // intersects
        if ( engine->intersects( geom ) )
        {
          ok = true;
        }
        break;
      case 1:
        // contains
        if ( comparingToJoinedFeature )
        {
          if ( engine->contains( geom ) )
          {
            ok = true;
          }
        }
        else
        {
          if ( engine->within( geom ) )
          {
            ok = true;
          }
        }
        break;
      case 2:
        // equals
        if ( engine->isEqual( geom ) )
        {
          ok = true;
        }
        break;
      case 3:
        // touches
        if ( engine->touches( geom ) )
        {
          ok = true;
        }
        break;
      case 4:
        // overlaps
        if ( engine->overlaps( geom ) )
        {
          ok = true;
        }
        break;
      case 5:
        // within
        if ( comparingToJoinedFeature )
        {
          if ( engine->within( geom ) )
          {
            ok = true;
          }
        }
        else
        {
          if ( engine->contains( geom ) )
          {
            ok = true;
          }
        }
        break;
      case 6:
        // crosses
        if ( engine->crosses( geom ) )
        {
          ok = true;
        }
        break;
    }
    if ( ok )
      return ok;
  }
  return ok;
}

void QgsJoinByLocationAlgorithm::processAlgorithmByIteratingOverJoinedSource( QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( mBaseSource->hasSpatialIndex() == Qgis::SpatialIndexPresence::NotPresent )
    feedback->pushWarning( QObject::tr( "No spatial index exists for input layer, performance will be severely degraded" ) );

  QgsFeatureIterator joinIter = mJoinSource->getFeatures( QgsFeatureRequest().setDestinationCrs( mBaseSource->sourceCrs(), context.transformContext() ).setSubsetOfAttributes( mJoinedFieldIndices ) );
  QgsFeature f;

  // Create output vector layer with additional attributes
  const double step = mJoinSource->featureCount() > 0 ? 100.0 / mJoinSource->featureCount() : 1;
  long i = 0;
  while ( joinIter.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    processFeatureFromJoinSource( f, feedback );

    i++;
    feedback->setProgress( i * step );
  }

  if ( !mDiscardNonMatching || mUnjoinedFeatures )
  {
    QgsFeatureIds unjoinedIds = mBaseSource->allFeatureIds();
    unjoinedIds.subtract( mAddedIds );

    QgsFeature f2;
    QgsFeatureRequest remainings = QgsFeatureRequest().setFilterFids( unjoinedIds );
    QgsFeatureIterator remainIter = mBaseSource->getFeatures( remainings );

    QgsAttributes emptyAttributes;
    emptyAttributes.reserve( mJoinedFieldIndices.count() );
    for ( int i = 0; i < mJoinedFieldIndices.count(); ++i )
      emptyAttributes << QVariant();

    while ( remainIter.nextFeature( f2 ) )
    {
      if ( feedback->isCanceled() )
        break;

      if ( mJoinedFeatures && !mDiscardNonMatching )
      {
        QgsAttributes attributes = f2.attributes();
        attributes.append( emptyAttributes );
        QgsFeature outputFeature( f2 );
        outputFeature.setAttributes( attributes );
        if ( !mJoinedFeatures->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( mJoinedFeatures.get(), QVariantMap(), u"OUTPUT"_s ) );
      }

      if ( mUnjoinedFeatures )
      {
        if ( !mUnjoinedFeatures->addFeature( f2, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( mUnjoinedFeatures.get(), QVariantMap(), u"NON_MATCHING"_s ) );
      }
    }
  }
}

void QgsJoinByLocationAlgorithm::processAlgorithmByIteratingOverInputSource( QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( mJoinSource->hasSpatialIndex() == Qgis::SpatialIndexPresence::NotPresent )
    feedback->pushWarning( QObject::tr( "No spatial index exists for join layer, performance will be severely degraded" ) );

  QgsFeatureIterator it = mBaseSource->getFeatures();
  QgsFeature f;

  const double step = mBaseSource->featureCount() > 0 ? 100.0 / mBaseSource->featureCount() : 1;
  long i = 0;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    processFeatureFromInputSource( f, context, feedback );

    i++;
    feedback->setProgress( i * step );
  }
}

void QgsJoinByLocationAlgorithm::sortPredicates( QList<int> &predicates )
{
  // Sort predicate list so that faster predicates are earlier in the list
  // Some predicates in GEOS do not have prepared geometry implementations, and are slow to calculate. So if users
  // are testing multiple predicates, make sure the optimised ones are always tested first just in case we can shortcut
  // these slower ones

  std::sort( predicates.begin(), predicates.end(), []( int a, int b ) -> bool {
    // return true if predicate a is faster than b

    if ( a == 0 ) // intersects is fastest
      return true;
    else if ( b == 0 )
      return false;

    else if ( a == 5 ) // contains is fast for polygons
      return true;
    else if ( b == 5 )
      return false;

    // that's it, the rest don't have optimised prepared methods (as of GEOS 3.8)
    return a < b;
  } );
}

bool QgsJoinByLocationAlgorithm::processFeatureFromJoinSource( QgsFeature &joinFeature, QgsProcessingFeedback *feedback )
{
  if ( !joinFeature.hasGeometry() )
    return false;

  const QgsGeometry featGeom = joinFeature.geometry();
  std::unique_ptr<QgsGeometryEngine> engine;
  QgsFeatureRequest req = QgsFeatureRequest().setFilterRect( featGeom.boundingBox() );
  QgsFeatureIterator it = mBaseSource->getFeatures( req );
  QgsFeature baseFeature;
  bool ok = false;
  QgsAttributes joinAttributes;

  while ( it.nextFeature( baseFeature ) )
  {
    if ( feedback->isCanceled() )
      break;

    switch ( mJoinMethod )
    {
      case JoinToFirst:
        if ( mAddedIds.contains( baseFeature.id() ) )
        {
          //  already added this feature, and user has opted to only output first match
          continue;
        }
        break;

      case OneToMany:
        break;

      case JoinToLargestOverlap:
        Q_ASSERT_X( false, "QgsJoinByLocationAlgorithm::processFeatureFromJoinSource", "processFeatureFromJoinSource should not be used with join to largest overlap method" );
    }

    if ( !engine )
    {
      engine.reset( QgsGeometry::createGeometryEngine( featGeom.constGet() ) );
      engine->prepareGeometry();
      for ( int ix : std::as_const( mJoinedFieldIndices ) )
      {
        joinAttributes.append( joinFeature.attribute( ix ) );
      }
    }
    if ( featureFilter( baseFeature, engine.get(), false, mPredicates ) )
    {
      if ( mJoinedFeatures )
      {
        QgsFeature outputFeature( baseFeature );
        outputFeature.setAttributes( baseFeature.attributes() + joinAttributes );
        if ( !mJoinedFeatures->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( mJoinedFeatures.get(), QVariantMap(), u"OUTPUT"_s ) );
      }
      if ( !ok )
        ok = true;

      mAddedIds.insert( baseFeature.id() );
      mJoinedCount++;
    }
  }
  return ok;
}

bool QgsJoinByLocationAlgorithm::processFeatureFromInputSource( QgsFeature &baseFeature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( !baseFeature.hasGeometry() )
  {
    // no geometry, treat as if we didn't find a match...
    if ( mJoinedFeatures && !mDiscardNonMatching )
    {
      QgsAttributes emptyAttributes;
      emptyAttributes.reserve( mJoinedFieldIndices.count() );
      for ( int i = 0; i < mJoinedFieldIndices.count(); ++i )
        emptyAttributes << QVariant();

      QgsAttributes attributes = baseFeature.attributes();
      attributes.append( emptyAttributes );
      QgsFeature outputFeature( baseFeature );
      outputFeature.setAttributes( attributes );
      if ( !mJoinedFeatures->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( mJoinedFeatures.get(), QVariantMap(), u"OUTPUT"_s ) );
    }

    if ( mUnjoinedFeatures )
    {
      if ( !mUnjoinedFeatures->addFeature( baseFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( mUnjoinedFeatures.get(), QVariantMap(), u"NON_MATCHING"_s ) );
    }

    return false;
  }

  const QgsGeometry featGeom = baseFeature.geometry();
  std::unique_ptr<QgsGeometryEngine> engine;
  QgsFeatureRequest req = QgsFeatureRequest().setDestinationCrs( mBaseSource->sourceCrs(), context.transformContext() ).setFilterRect( featGeom.boundingBox() ).setSubsetOfAttributes( mJoinedFieldIndices );

  QgsFeatureIterator it = mJoinSource->getFeatures( req );
  QgsFeature joinFeature;
  bool ok = false;

  double largestOverlap = std::numeric_limits<double>::lowest();
  QgsFeature bestMatch;

  while ( it.nextFeature( joinFeature ) )
  {
    if ( feedback->isCanceled() )
      break;

    if ( !engine )
    {
      engine.reset( QgsGeometry::createGeometryEngine( featGeom.constGet() ) );
      engine->prepareGeometry();
    }

    if ( featureFilter( joinFeature, engine.get(), true, mPredicates ) )
    {
      switch ( mJoinMethod )
      {
        case JoinToFirst:
        case OneToMany:
          if ( mJoinedFeatures )
          {
            QgsAttributes joinAttributes = baseFeature.attributes();
            joinAttributes.reserve( joinAttributes.size() + mJoinedFieldIndices.size() );
            for ( int ix : std::as_const( mJoinedFieldIndices ) )
            {
              joinAttributes.append( joinFeature.attribute( ix ) );
            }

            QgsFeature outputFeature( baseFeature );
            outputFeature.setAttributes( joinAttributes );
            if ( !mJoinedFeatures->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
              throw QgsProcessingException( writeFeatureError( mJoinedFeatures.get(), QVariantMap(), u"OUTPUT"_s ) );
          }
          break;

        case JoinToLargestOverlap:
        {
          // calculate area of overlap
          std::unique_ptr<QgsAbstractGeometry> intersection( engine->intersection( joinFeature.geometry().constGet() ) );
          double overlap = 0;
          switch ( QgsWkbTypes::geometryType( intersection->wkbType() ) )
          {
            case Qgis::GeometryType::Line:
              overlap = intersection->length();
              break;

            case Qgis::GeometryType::Polygon:
              overlap = intersection->area();
              break;

            case Qgis::GeometryType::Unknown:
            case Qgis::GeometryType::Point:
            case Qgis::GeometryType::Null:
              break;
          }

          if ( overlap > largestOverlap )
          {
            largestOverlap = overlap;
            bestMatch = joinFeature;
          }
          break;
        }
      }

      ok = true;

      if ( mJoinMethod == JoinToFirst )
        break;
    }
  }

  switch ( mJoinMethod )
  {
    case OneToMany:
    case JoinToFirst:
      break;

    case JoinToLargestOverlap:
    {
      if ( bestMatch.isValid() )
      {
        // grab attributes from feature with best match
        if ( mJoinedFeatures )
        {
          QgsAttributes joinAttributes = baseFeature.attributes();
          joinAttributes.reserve( joinAttributes.size() + mJoinedFieldIndices.size() );
          for ( int ix : std::as_const( mJoinedFieldIndices ) )
          {
            joinAttributes.append( bestMatch.attribute( ix ) );
          }

          QgsFeature outputFeature( baseFeature );
          outputFeature.setAttributes( joinAttributes );
          if ( !mJoinedFeatures->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( mJoinedFeatures.get(), QVariantMap(), u"OUTPUT"_s ) );
        }
      }
      else
      {
        ok = false; // shouldn't happen...
      }
      break;
    }
  }

  if ( !ok )
  {
    // didn't find a match...
    if ( mJoinedFeatures && !mDiscardNonMatching )
    {
      QgsAttributes emptyAttributes;
      emptyAttributes.reserve( mJoinedFieldIndices.count() );
      for ( int i = 0; i < mJoinedFieldIndices.count(); ++i )
        emptyAttributes << QVariant();

      QgsAttributes attributes = baseFeature.attributes();
      attributes.append( emptyAttributes );
      QgsFeature outputFeature( baseFeature );
      outputFeature.setAttributes( attributes );
      if ( !mJoinedFeatures->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( mJoinedFeatures.get(), QVariantMap(), u"OUTPUT"_s ) );
    }

    if ( mUnjoinedFeatures )
    {
      if ( !mUnjoinedFeatures->addFeature( baseFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( mUnjoinedFeatures.get(), QVariantMap(), u"NON_MATCHING"_s ) );
    }
  }
  else
    mJoinedCount++;

  return ok;
}


///@endcond
