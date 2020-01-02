/***************************************************************************
                         qgsalgorithmjoinbylocation.cpp
                         ---------------------
    begin                : January 2020
    copyright            : (C) 2020 by
    email                :
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
                QObject::tr( "Base Layer" ), QList< int > () << QgsProcessing::QgsProcessing::TypeVectorAnyGeometry ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "JOIN" ),
                QObject::tr( "Join Layer" ), QList< int > () << QgsProcessing::QgsProcessing::TypeVectorAnyGeometry ) );

  QStringList predicates;
  predicates << QObject::tr( "intersects" )
             << QObject::tr( "contains" )
             << QObject::tr( "equals" )
             << QObject::tr( "touches" )
             << QObject::tr( "overlaps" )
             << QObject::tr( "within" )
             << QObject::tr( "crosses" );

  std::unique_ptr< QgsProcessingParameterEnum > predicateParam = qgis::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "PREDICATE" ), QObject::tr( "Geometric predicate" ), predicates, true, 0 );
  QVariantMap predicateMetadata;
  QVariantMap widgetMetadata;
  widgetMetadata.insert( QStringLiteral( "useCheckBoxes" ), true );
  widgetMetadata.insert( QStringLiteral( "columns" ), 2 );
  predicateMetadata.insert( QStringLiteral( "widget_wrapper" ), widgetMetadata );
  predicateParam->setMetadata( predicateMetadata );
  addParameter( predicateParam.release() );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "JOIN_FIELDS" ),
                QObject::tr( "Fields to add (leave empty to use all fields)" ),
                QVariant(), QStringLiteral( "JOIN" ), QgsProcessingParameterField::Any, true, true ) );

  QStringList joinMethods;
  joinMethods << QObject::tr( "Create separate feature for each matching feature (one-to-many)" )
              << QObject::tr( "Take attributes of the first matching feature only (one-to-one)" );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "METHOD" ),
                QObject::tr( "Join type" ),
                joinMethods, false, 0 ) );
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

  if ( mBaseSource->hasSpatialIndex() == QgsFeatureSource::SpatialIndexNotPresent )
    feedback->reportError( QObject::tr( "No spatial index exists for input layer, performance will be severely degraded" ) );

  std::unique_ptr< QgsFeatureSource > joinSource( parameterAsSource( parameters, QStringLiteral( "JOIN" ), context ) );
  if ( !joinSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "JOIN" ) ) );

  mJoinMethod = parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context );

  const QStringList joinedFieldNames = parameterAsFields( parameters, QStringLiteral( "JOIN_FIELDS" ), context );

  mPredicates = parameterAsEnums( parameters, QStringLiteral( "PREDICATE" ), context );

  QString prefix = parameterAsString( parameters, QStringLiteral( "PREFIX" ), context );

  QgsFields joinFields;
  if ( joinedFieldNames.empty() )
  {
    joinFields = joinSource->fields();
    mFields2Indices.reserve( joinFields.count() );
    for ( int i = 0; i < joinFields.count(); ++i )
    {
      mFields2Indices << i;
    }
  }
  else
  {
    mFields2Indices.reserve( joinedFieldNames.count() );
    for ( const QString &field : joinedFieldNames )
    {
      int index = joinSource->fields().lookupField( field );
      if ( index >= 0 )
      {
        mFields2Indices << index;
        joinFields.append( joinSource->fields().at( index ) );
      }
    }
  }

  if ( !prefix.isEmpty() )
  {
    for ( int i = 0; i < joinFields.count(); ++i )
    {
      joinFields[ i ].setName( prefix + joinFields[ i ].name() );
    }
  }

  const QgsFields mOutFields = QgsProcessingUtils::combineFields( mBaseSource->fields(), joinFields );

  QString joinedSinkId;
  mJoinedFeatures.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, joinedSinkId, mOutFields,
                                          mBaseSource->wkbType(), mBaseSource->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );

  if ( parameters.value( QStringLiteral( "OUTPUT" ) ).isValid() && !mJoinedFeatures )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  mDiscardNonMatching = parameterAsBoolean( parameters, QStringLiteral( "DISCARD_NONMATCHING" ), context );

  QString nonMatchingSinkId;
  mUnjoinedFeatures.reset( parameterAsSink( parameters, QStringLiteral( "NON_MATCHING" ), context, nonMatchingSinkId, mBaseSource->fields(),
                           mBaseSource->wkbType(), mBaseSource->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( parameters.value( QStringLiteral( "NON_MATCHING" ) ).isValid() && !mUnjoinedFeatures )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "NON_MATCHING" ) ) );

  if ( !mDiscardNonMatching || mUnjoinedFeatures )
    mUnjoinedIds = mBaseSource->allFeatureIds();

  qlonglong joinedCount = 0;
  QgsFeatureIterator joinIter = joinSource->getFeatures( QgsFeatureRequest().setDestinationCrs( mBaseSource->sourceCrs(), context.transformContext() ).setSubsetOfAttributes( mFields2Indices ) );
  QgsFeature f;

  // Create output vector layer with additional attributes
  const double step = mBaseSource->featureCount() > 0 ? 100.0 / mBaseSource->featureCount() : 1;
  long i = 0;
  while ( joinIter.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;
    feedback->setProgress( i * step );
    if ( processFeatures( f, feedback ) )
      joinedCount++;
    i++;
  }

  if ( !mDiscardNonMatching ||  mUnjoinedFeatures )
  {
    QgsFeature f2;
    QgsFeatureRequest remainings = QgsFeatureRequest().setFilterFids( mUnjoinedIds );
    QgsFeatureIterator remainIter = mBaseSource->getFeatures( remainings );

    QgsAttributes emptyAttributes;
    emptyAttributes.reserve( mFields2Indices.count() );
    for ( int i = 0; i < mFields2Indices.count(); ++i )
      emptyAttributes << QVariant();

    while ( remainIter.nextFeature( f2 ) )
    {
      if ( feedback->isCanceled() )
        break;
      if ( mJoinedFeatures )
      {
        QgsAttributes attributes = f2.attributes();
        attributes.append( emptyAttributes );
        QgsFeature outputFeature( f2 );
        outputFeature.setAttributes( attributes );
        mJoinedFeatures->addFeature( outputFeature, QgsFeatureSink::FastInsert );
      }
      if ( mUnjoinedFeatures )
        mUnjoinedFeatures->addFeature( f2, QgsFeatureSink::FastInsert );
    }
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
  outputs.insert( QStringLiteral( "JOINED_COUNT" ), joinedCount );
  return outputs;
}

bool QgsJoinByLocationAlgorithm::featureFilter( const QgsFeature &feature, QgsGeometryEngine *engine ) const
{
  const QgsAbstractGeometry *geom = feature.geometry().constGet();
  bool ok = false;
  for ( const auto  predicate : mPredicates )
  {
    switch ( predicate )
    {
      case 0:
        if ( engine->intersects( geom ) )
        {
          ok = true;
        }
        break;
      case 1:
        if ( engine->within( geom ) )
        {
          ok = true;
        }
        break;
      case 2:
        if ( engine->isEqual( geom ) )
        {
          ok = true;
        }
        break;
      case 3:
        if ( engine->touches( geom ) )
        {
          ok = true;
        }
        break;
      case 4:
        if ( engine->overlaps( geom ) )
        {
          ok = true;
        }
        break;
      case 5:
        if ( engine->contains( geom ) )
        {
          ok = true;
        }
        break;
      case 6:
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

bool QgsJoinByLocationAlgorithm::processFeatures( QgsFeature &joinFeature, QgsProcessingFeedback *feedback )
{

  if ( !joinFeature.hasGeometry() )
    return false;
  const QgsGeometry featGeom = joinFeature.geometry();
  std::unique_ptr< QgsGeometryEngine > engine;
  QgsFeatureRequest req = QgsFeatureRequest().setFilterRect( featGeom.boundingBox() );
  QgsFeatureIterator it = mBaseSource->getFeatures( req );
  QList<QgsFeature> filtered;
  QgsFeature baseFeature;
  bool ok = false;
  QgsAttributes joinAttributes;

  while ( it.nextFeature( baseFeature ) )
  {
    if ( feedback->isCanceled() )
      break;
    if ( mJoinMethod == 1 && !mUnjoinedIds.contains( baseFeature.id() ) )
      continue;

    if ( !engine )
    {
      engine.reset( QgsGeometry::createGeometryEngine( featGeom.constGet() ) );
      engine->prepareGeometry();
      for ( int ix : qgis::as_const( mFields2Indices ) )
      {
        joinAttributes.append( joinFeature.attribute( ix ) );
      }
    }
    if ( featureFilter( baseFeature, engine.get() ) )
    {
      if ( mJoinedFeatures )
      {
        QgsFeature outputFeature( baseFeature );
        outputFeature.setAttributes( baseFeature.attributes() + joinAttributes );
        mJoinedFeatures->addFeature( outputFeature, QgsFeatureSink::FastInsert );
      }
      if ( !ok )
        ok = true;
      if ( !mDiscardNonMatching || mUnjoinedFeatures )
        mUnjoinedIds.remove( baseFeature.id() );
    }
  }
  return ok;
}


///@endcond



