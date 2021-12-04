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
#include "qgsprocessing.h"
#include "qgsgeometryengine.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsfeaturesource.h"

///@cond PRIVATE


void QgsJoinByLocationAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Join to features in" ), QList< int > () << QgsProcessing::QgsProcessing::TypeVectorAnyGeometry ) );

  QStringList predicates;
  predicates << QObject::tr( "intersect" )
             << QObject::tr( "contain" )
             << QObject::tr( "equal" )
             << QObject::tr( "touch" )
             << QObject::tr( "overlap" )
             << QObject::tr( "are within" )
             << QObject::tr( "cross" );

  std::unique_ptr< QgsProcessingParameterEnum > predicateParam = std::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "PREDICATE" ), QObject::tr( "Features they (geometric predicate)" ), predicates, true, 0 );
  QVariantMap predicateMetadata;
  QVariantMap widgetMetadata;
  widgetMetadata.insert( QStringLiteral( "useCheckBoxes" ), true );
  widgetMetadata.insert( QStringLiteral( "columns" ), 2 );
  predicateMetadata.insert( QStringLiteral( "widget_wrapper" ), widgetMetadata );
  predicateParam->setMetadata( predicateMetadata );
  addParameter( predicateParam.release() );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "JOIN" ),
                QObject::tr( "By comparing to" ), QList< int > () << QgsProcessing::QgsProcessing::TypeVectorAnyGeometry ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "JOIN_FIELDS" ),
                QObject::tr( "Fields to add (leave empty to use all fields)" ),
                QVariant(), QStringLiteral( "JOIN" ), QgsProcessingParameterField::Any, true, true ) );

  QStringList joinMethods;
  joinMethods << QObject::tr( "Create separate feature for each matching feature (one-to-many)" )
              << QObject::tr( "Take attributes of the first matching feature only (one-to-one)" )
              << QObject::tr( "Take attributes of the feature with largest overlap only (one-to-one)" );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "METHOD" ),
                QObject::tr( "Join type" ),
                joinMethods, false, static_cast< int >( OneToMany ) ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DISCARD_NONMATCHING" ),
                QObject::tr( "Discard records which could not be joined" ),
                false ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "PREFIX" ),
                QObject::tr( "Joined field prefix" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Joined layer" ), QgsProcessing::TypeVectorAnyGeometry, QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "NON_MATCHING" ), QObject::tr( "Unjoinable features from first layer" ), QgsProcessing::TypeVectorAnyGeometry, QVariant(), true, false ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "JOINED_COUNT" ), QObject::tr( "Number of joined features from input table" ) ) );
}

QString QgsJoinByLocationAlgorithm::name() const
{
  return QStringLiteral( "joinattributesbylocation" );
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
  return QStringLiteral( "vectorgeneral" );
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
  return QObject::tr( "Join attributes from one vector layer to another by location." );
}

QgsJoinByLocationAlgorithm *QgsJoinByLocationAlgorithm::createInstance() const
{
  return new QgsJoinByLocationAlgorithm();
}


QVariantMap QgsJoinByLocationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mBaseSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mBaseSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  mJoinSource.reset( parameterAsSource( parameters, QStringLiteral( "JOIN" ), context ) );
  if ( !mJoinSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "JOIN" ) ) );

  mJoinMethod = static_cast< JoinMethod >( parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context ) );

  const QStringList joinedFieldNames = parameterAsFields( parameters, QStringLiteral( "JOIN_FIELDS" ), context );

  mPredicates = parameterAsEnums( parameters, QStringLiteral( "PREDICATE" ), context );
  sortPredicates( mPredicates );

  QString prefix = parameterAsString( parameters, QStringLiteral( "PREFIX" ), context );

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
      joinFields.rename( i, prefix + joinFields[ i ].name() );
    }
  }

  const QgsFields outputFields = QgsProcessingUtils::combineFields( mBaseSource->fields(), joinFields );

  QString joinedSinkId;
  mJoinedFeatures.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, joinedSinkId, outputFields,
                                          mBaseSource->wkbType(), mBaseSource->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );

  if ( parameters.value( QStringLiteral( "OUTPUT" ) ).isValid() && !mJoinedFeatures )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  mDiscardNonMatching = parameterAsBoolean( parameters, QStringLiteral( "DISCARD_NONMATCHING" ), context );

  QString nonMatchingSinkId;
  mUnjoinedFeatures.reset( parameterAsSink( parameters, QStringLiteral( "NON_MATCHING" ), context, nonMatchingSinkId, mBaseSource->fields(),
                           mBaseSource->wkbType(), mBaseSource->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( parameters.value( QStringLiteral( "NON_MATCHING" ) ).isValid() && !mUnjoinedFeatures )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "NON_MATCHING" ) ) );

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
    outputs.insert( QStringLiteral( "OUTPUT" ), joinedSinkId );
  }
  if ( mUnjoinedFeatures )
  {
    outputs.insert( QStringLiteral( "NON_MATCHING" ), nonMatchingSinkId );
  }

  // need to release sinks to finalize writing
  mJoinedFeatures.reset();
  mUnjoinedFeatures.reset();

  outputs.insert( QStringLiteral( "JOINED_COUNT" ), static_cast< long long >( mJoinedCount ) );
  return outputs;
}

bool QgsJoinByLocationAlgorithm::featureFilter( const QgsFeature &feature, QgsGeometryEngine *engine, bool comparingToJoinedFeature ) const
{
  const QgsAbstractGeometry *geom = feature.geometry().constGet();
  bool ok = false;
  for ( const int predicate : mPredicates )
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
  if ( mBaseSource->hasSpatialIndex() == QgsFeatureSource::SpatialIndexNotPresent )
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
          throw QgsProcessingException( writeFeatureError( mJoinedFeatures.get(), QVariantMap(), QStringLiteral( "OUTPUT" ) ) );
      }

      if ( mUnjoinedFeatures )
      {
        if ( !mUnjoinedFeatures->addFeature( f2, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( mUnjoinedFeatures.get(), QVariantMap(), QStringLiteral( "NON_MATCHING" ) ) );
      }
    }
  }
}

void QgsJoinByLocationAlgorithm::processAlgorithmByIteratingOverInputSource( QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( mJoinSource->hasSpatialIndex() == QgsFeatureSource::SpatialIndexNotPresent )
    feedback->pushWarning( QObject::tr( "No spatial index exists for join layer, performance will be severely degraded" ) );

  QgsFeatureIterator it = mBaseSource->getFeatures();
  QgsFeature f;

  const double step = mBaseSource->featureCount() > 0 ? 100.0 / mBaseSource->featureCount() : 1;
  long i = 0;
  while ( it .nextFeature( f ) )
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

  std::sort( predicates.begin(), predicates.end(), []( int a, int b ) -> bool
  {
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
  std::unique_ptr< QgsGeometryEngine > engine;
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
    if ( featureFilter( baseFeature, engine.get(), false ) )
    {
      if ( mJoinedFeatures )
      {
        QgsFeature outputFeature( baseFeature );
        outputFeature.setAttributes( baseFeature.attributes() + joinAttributes );
        if ( !mJoinedFeatures->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( mJoinedFeatures.get(), QVariantMap(), QStringLiteral( "OUTPUT" ) ) );
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
        throw QgsProcessingException( writeFeatureError( mJoinedFeatures.get(), QVariantMap(), QStringLiteral( "OUTPUT" ) ) );
    }

    if ( mUnjoinedFeatures )
    {
      if ( !mUnjoinedFeatures->addFeature( baseFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( mUnjoinedFeatures.get(), QVariantMap(), QStringLiteral( "NON_MATCHING" ) ) );
    }

    return false;
  }

  const QgsGeometry featGeom = baseFeature.geometry();
  std::unique_ptr< QgsGeometryEngine > engine;
  QgsFeatureRequest req = QgsFeatureRequest().setDestinationCrs( mBaseSource->sourceCrs(), context.transformContext() ).setFilterRect( featGeom.boundingBox() ).setSubsetOfAttributes( mJoinedFieldIndices );

  QgsFeatureIterator it = mJoinSource->getFeatures( req );
  QgsFeature joinFeature;
  bool ok = false;

  double largestOverlap  = std::numeric_limits< double >::lowest();
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

    if ( featureFilter( joinFeature, engine.get(), true ) )
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
              throw QgsProcessingException( writeFeatureError( mJoinedFeatures.get(), QVariantMap(), QStringLiteral( "OUTPUT" ) ) );
          }
          break;

        case JoinToLargestOverlap:
        {
          // calculate area of overlap
          std::unique_ptr< QgsAbstractGeometry > intersection( engine->intersection( joinFeature.geometry().constGet() ) );
          double overlap = 0;
          switch ( QgsWkbTypes::geometryType( intersection->wkbType() ) )
          {
            case QgsWkbTypes::LineGeometry:
              overlap = intersection->length();
              break;

            case QgsWkbTypes::PolygonGeometry:
              overlap = intersection->area();
              break;

            case QgsWkbTypes::UnknownGeometry:
            case QgsWkbTypes::PointGeometry:
            case QgsWkbTypes::NullGeometry:
              break;
          }

          if ( overlap > largestOverlap )
          {
            largestOverlap  = overlap;
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
            throw QgsProcessingException( writeFeatureError( mJoinedFeatures.get(), QVariantMap(), QStringLiteral( "OUTPUT" ) ) );
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
        throw QgsProcessingException( writeFeatureError( mJoinedFeatures.get(), QVariantMap(), QStringLiteral( "OUTPUT" ) ) );
    }

    if ( mUnjoinedFeatures )
    {
      if ( !mUnjoinedFeatures->addFeature( baseFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( mUnjoinedFeatures.get(), QVariantMap(), QStringLiteral( "NON_MATCHING" ) ) );
    }
  }
  else
    mJoinedCount++;

  return ok;
}


///@endcond



