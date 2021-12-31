/***************************************************************************
                         qgsalgorithmextractbylocation.cpp
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmextractbylocation.h"
#include "qgsgeometryengine.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"

///@cond PRIVATE

void QgsLocationBasedAlgorithm::addPredicateParameter()
{
  std::unique_ptr< QgsProcessingParameterEnum > predicateParam( new QgsProcessingParameterEnum( QStringLiteral( "PREDICATE" ),
      QObject::tr( "Where the features (geometric predicate)" ),
      predicateOptionsList(), true, QVariant::fromValue( QList< int >() << 0 ) ) );

  QVariantMap predicateMetadata;
  QVariantMap widgetMetadata;
  widgetMetadata.insert( QStringLiteral( "useCheckBoxes" ), true );
  widgetMetadata.insert( QStringLiteral( "columns" ), 2 );
  predicateMetadata.insert( QStringLiteral( "widget_wrapper" ), widgetMetadata );
  predicateParam->setMetadata( predicateMetadata );

  addParameter( predicateParam.release() );
}

QgsLocationBasedAlgorithm::Predicate QgsLocationBasedAlgorithm::reversePredicate( QgsLocationBasedAlgorithm::Predicate predicate ) const
{
  switch ( predicate )
  {
    case Intersects:
      return Intersects;
    case Contains:
      return Within;
    case Disjoint:
      return Disjoint;
    case IsEqual:
      return IsEqual;
    case Touches:
      return Touches;
    case Overlaps:
      return Overlaps;
    case Within:
      return Contains;
    case Crosses:
      return Crosses;
  }
  // no warnings
  return Intersects;
}

QStringList QgsLocationBasedAlgorithm::predicateOptionsList() const
{
  return QStringList() << QObject::tr( "intersect" )
         << QObject::tr( "contain" )
         << QObject::tr( "disjoint" )
         << QObject::tr( "equal" )
         << QObject::tr( "touch" )
         << QObject::tr( "overlap" )
         << QObject::tr( "are within" )
         << QObject::tr( "cross" );
}

void QgsLocationBasedAlgorithm::process( const QgsProcessingContext &context, QgsFeatureSource *targetSource,
    QgsFeatureSource *intersectSource,
    const QList< int > &selectedPredicates,
    const std::function < void( const QgsFeature & ) > &handleFeatureFunction,
    bool onlyRequireTargetIds,
    QgsProcessingFeedback *feedback,
    const QgsFeatureIds &skipTargetFeatureIds )
{
  // skip if there are no features to select from!
  if ( targetSource->featureCount() == 0 )
    return;

  // skip if intersect layer is empty, unless we are looking for disjoints
  if ( intersectSource->featureCount() == 0 &&
       !selectedPredicates.contains( Disjoint ) )
    return;

  if ( targetSource->featureCount() > 0 && intersectSource->featureCount() > 0 &&
       targetSource->featureCount() < intersectSource->featureCount() )
  {
    // joining FEWER features to a layer with MORE features. So we iterate over the FEW features and find matches from the MANY
    processByIteratingOverTargetSource( context, targetSource, intersectSource,
                                        selectedPredicates, handleFeatureFunction,
                                        onlyRequireTargetIds, feedback, skipTargetFeatureIds );
  }
  else
  {
    // default -- iterate over the intersect source and match back to the target source. We do this on the assumption that the most common
    // use case is joining a points layer to a polygon layer (e.g. findings points within a polygon), so by iterating
    // over the polygons we can take advantage of prepared geometries for the spatial relationship test.

    // TODO - consider using more heuristics to determine whether it's always best to iterate over the intersect
    // source.
    processByIteratingOverIntersectSource( context, targetSource, intersectSource,
                                           selectedPredicates, handleFeatureFunction,
                                           onlyRequireTargetIds, feedback, skipTargetFeatureIds );
  }
}

void QgsLocationBasedAlgorithm::processByIteratingOverTargetSource( const QgsProcessingContext &context, QgsFeatureSource *targetSource,
    QgsFeatureSource *intersectSource,
    const QList< int > &selectedPredicates,
    const std::function < void( const QgsFeature & ) > &handleFeatureFunction,
    bool onlyRequireTargetIds,
    QgsProcessingFeedback *feedback,
    const QgsFeatureIds &skipTargetFeatureIds )
{
  if ( intersectSource->hasSpatialIndex() == QgsFeatureSource::SpatialIndexNotPresent )
    feedback->pushWarning( QObject::tr( "No spatial index exists for intersect layer, performance will be severely degraded" ) );

  QgsFeatureIds foundSet;
  QgsFeatureRequest request = QgsFeatureRequest();
  if ( onlyRequireTargetIds )
    request.setNoAttributes();

  QgsFeatureIterator fIt = targetSource->getFeatures( request );
  double step = targetSource->featureCount() > 0 ? 100.0 / targetSource->featureCount() : 1;
  int current = 0;
  QgsFeature f;
  std::unique_ptr< QgsGeometryEngine > engine;
  while ( fIt.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    // don't check features in skipTargetFeatureIds
    if ( skipTargetFeatureIds.contains( f.id() ) )
      continue;
    if ( !f.hasGeometry() )
      continue;

    engine.reset();

    QgsRectangle bbox = f.geometry().boundingBox();
    request = QgsFeatureRequest().setFilterRect( bbox ).setNoAttributes().setDestinationCrs( targetSource->sourceCrs(), context.transformContext() );

    QgsFeatureIterator testFeatureIt = intersectSource->getFeatures( request );
    QgsFeature testFeature;
    bool isMatch = false;
    bool isDisjoint = true;
    while ( testFeatureIt.nextFeature( testFeature ) )
    {
      if ( feedback->isCanceled() )
        break;

      if ( !engine )
      {
        engine.reset( QgsGeometry::createGeometryEngine( f.geometry().constGet() ) );
        engine->prepareGeometry();
      }

      for ( int predicate : selectedPredicates )
      {
        switch ( static_cast< Predicate>( predicate ) )
        {
          case Intersects:
            isMatch = engine->intersects( testFeature.geometry().constGet() );
            break;
          case Contains:
            isMatch = engine->contains( testFeature.geometry().constGet() );
            break;
          case Disjoint:
            if ( engine->intersects( testFeature.geometry().constGet() ) )
            {
              isDisjoint = false;
            }
            break;
          case IsEqual:
            isMatch = engine->isEqual( testFeature.geometry().constGet() );
            break;
          case Touches:
            isMatch = engine->touches( testFeature.geometry().constGet() );
            break;
          case Overlaps:
            isMatch = engine->overlaps( testFeature.geometry().constGet() );
            break;
          case Within:
            isMatch = engine->within( testFeature.geometry().constGet() );
            break;
          case Crosses:
            isMatch = engine->crosses( testFeature.geometry().constGet() );
            break;
        }

        if ( isMatch )
          break;
      }

      if ( isMatch )
      {
        foundSet.insert( f.id() );
        handleFeatureFunction( f );
        break;
      }
    }
    if ( isDisjoint && selectedPredicates.contains( Disjoint ) )
    {
      foundSet.insert( f.id() );
      handleFeatureFunction( f );
    }

    current += 1;
    feedback->setProgress( current * step );
  }
}

void QgsLocationBasedAlgorithm::processByIteratingOverIntersectSource( const QgsProcessingContext &context, QgsFeatureSource *targetSource,
    QgsFeatureSource *intersectSource,
    const QList< int > &selectedPredicates,
    const std::function < void( const QgsFeature & ) > &handleFeatureFunction,
    bool onlyRequireTargetIds,
    QgsProcessingFeedback *feedback,
    const QgsFeatureIds &skipTargetFeatureIds )
{
  if ( targetSource->hasSpatialIndex() == QgsFeatureSource::SpatialIndexNotPresent )
    feedback->pushWarning( QObject::tr( "No spatial index exists for input layer, performance will be severely degraded" ) );

  // build a list of 'reversed' predicates, because in this function
  // we actually test the reverse of what the user wants (allowing us
  // to prepare geometries and optimise the algorithm)
  QList< Predicate > predicates;
  predicates.reserve( selectedPredicates.count() );
  for ( int i : selectedPredicates )
  {
    predicates << reversePredicate( static_cast< Predicate >( i ) );
  }

  QgsFeatureIds disjointSet;
  if ( predicates.contains( Disjoint ) )
    disjointSet = targetSource->allFeatureIds();

  QgsFeatureIds foundSet;
  QgsFeatureRequest request = QgsFeatureRequest().setNoAttributes().setDestinationCrs( targetSource->sourceCrs(), context.transformContext() );
  QgsFeatureIterator fIt = intersectSource->getFeatures( request );
  double step = intersectSource->featureCount() > 0 ? 100.0 / intersectSource->featureCount() : 1;
  int current = 0;
  QgsFeature f;
  std::unique_ptr< QgsGeometryEngine > engine;
  while ( fIt.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    if ( !f.hasGeometry() )
      continue;

    engine.reset();

    QgsRectangle bbox = f.geometry().boundingBox();
    request = QgsFeatureRequest().setFilterRect( bbox );
    if ( onlyRequireTargetIds )
      request.setNoAttributes();

    QgsFeatureIterator testFeatureIt = targetSource->getFeatures( request );
    QgsFeature testFeature;
    while ( testFeatureIt.nextFeature( testFeature ) )
    {
      if ( feedback->isCanceled() )
        break;

      if ( skipTargetFeatureIds.contains( testFeature.id() ) )
      {
        // don't check features in skipTargetFeatureIds
        continue;
      }
      if ( foundSet.contains( testFeature.id() ) )
      {
        // already added this one, no need for further tests
        continue;
      }
      if ( predicates.count() == 1 && predicates.at( 0 ) == Disjoint && !disjointSet.contains( testFeature.id() ) )
      {
        // calculating only the disjoint set, and we've already eliminated this feature so no need for further tests
        continue;
      }

      if ( !engine )
      {
        engine.reset( QgsGeometry::createGeometryEngine( f.geometry().constGet() ) );
        engine->prepareGeometry();
      }

      bool isMatch = false;

      for ( Predicate predicate : std::as_const( predicates ) )
      {
        switch ( predicate )
        {
          case Intersects:
            isMatch = engine->intersects( testFeature.geometry().constGet() );
            break;
          case Contains:
            isMatch = engine->contains( testFeature.geometry().constGet() );
            break;
          case Disjoint:
            if ( engine->intersects( testFeature.geometry().constGet() ) )
            {
              disjointSet.remove( testFeature.id() );
            }
            break;
          case IsEqual:
            isMatch = engine->isEqual( testFeature.geometry().constGet() );
            break;
          case Touches:
            isMatch = engine->touches( testFeature.geometry().constGet() );
            break;
          case Overlaps:
            isMatch = engine->overlaps( testFeature.geometry().constGet() );
            break;
          case Within:
            isMatch = engine->within( testFeature.geometry().constGet() );
            break;
          case Crosses:
            isMatch = engine->crosses( testFeature.geometry().constGet() );
            break;
        }
        if ( isMatch )
          break;
      }

      if ( isMatch )
      {
        foundSet.insert( testFeature.id() );
        handleFeatureFunction( testFeature );
      }

    }

    current += 1;
    feedback->setProgress( current * step );
  }

  if ( predicates.contains( Disjoint ) )
  {
    disjointSet = disjointSet.subtract( foundSet );
    QgsFeatureRequest disjointReq = QgsFeatureRequest().setFilterFids( disjointSet );
    if ( onlyRequireTargetIds )
      disjointReq.setNoAttributes().setFlags( QgsFeatureRequest::NoGeometry );
    QgsFeatureIterator disjointIt = targetSource->getFeatures( disjointReq );
    QgsFeature f;
    while ( disjointIt.nextFeature( f ) )
    {
      handleFeatureFunction( f );
    }
  }
}


//
// QgsSelectByLocationAlgorithm
//

void QgsSelectByLocationAlgorithm::initAlgorithm( const QVariantMap & )
{
  QStringList methods = QStringList() << QObject::tr( "creating new selection" )
                        << QObject::tr( "adding to current selection" )
                        << QObject::tr( "selecting within current selection" )
                        << QObject::tr( "removing from current selection" );

  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "INPUT" ), QObject::tr( "Select features from" ),
                QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );
  addPredicateParameter();
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INTERSECT" ),
                QObject::tr( "By comparing to the features from" ),
                QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "METHOD" ),
                QObject::tr( "Modify current selection by" ),
                methods, false, 0 ) );
}

QString QgsSelectByLocationAlgorithm::name() const
{
  return QStringLiteral( "selectbylocation" );
}

QgsProcessingAlgorithm::Flags QgsSelectByLocationAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagNoThreading | QgsProcessingAlgorithm::FlagNotAvailableInStandaloneTool;
}

QString QgsSelectByLocationAlgorithm::displayName() const
{
  return QObject::tr( "Select by location" );
}

QStringList QgsSelectByLocationAlgorithm::tags() const
{
  return QObject::tr( "select,intersects,intersecting,disjoint,touching,within,contains,overlaps,relation" ).split( ',' );
}

QString QgsSelectByLocationAlgorithm::group() const
{
  return QObject::tr( "Vector selection" );
}

QString QgsSelectByLocationAlgorithm::groupId() const
{
  return QStringLiteral( "vectorselection" );
}

QString QgsSelectByLocationAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a selection in a vector layer. The criteria for selecting "
                      "features is based on the spatial relationship between each feature and the features in an additional layer." );
}

QgsSelectByLocationAlgorithm *QgsSelectByLocationAlgorithm::createInstance() const
{
  return new QgsSelectByLocationAlgorithm();
}

QVariantMap QgsSelectByLocationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsVectorLayer *selectLayer = parameterAsVectorLayer( parameters, QStringLiteral( "INPUT" ), context );
  if ( !selectLayer )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for INPUT" ) );

  Qgis::SelectBehavior method = static_cast< Qgis::SelectBehavior >( parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context ) );
  std::unique_ptr< QgsFeatureSource > intersectSource( parameterAsSource( parameters, QStringLiteral( "INTERSECT" ), context ) );
  if ( !intersectSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INTERSECT" ) ) );

  const QList< int > selectedPredicates = parameterAsEnums( parameters, QStringLiteral( "PREDICATE" ), context );

  QgsFeatureIds selectedIds;
  auto addToSelection = [&]( const QgsFeature & feature )
  {
    selectedIds.insert( feature.id() );
  };
  switch ( method )
  {
    case Qgis::SelectBehavior::IntersectSelection:
    case Qgis::SelectBehavior::RemoveFromSelection:
    {
      // When subsetting or removing we only need to check already selected features
      std::unique_ptr< QgsVectorLayerSelectedFeatureSource > selectLayerSelected( new QgsVectorLayerSelectedFeatureSource( selectLayer ) );
      process( context, selectLayerSelected.get(), intersectSource.get(), selectedPredicates, addToSelection, true, feedback );
      break;
    }
    case Qgis::SelectBehavior::AddToSelection:
      // When adding we can skip checking already selected features
      process( context, selectLayer, intersectSource.get(), selectedPredicates, addToSelection, true, feedback, selectLayer->selectedFeatureIds() );
      break;
    case Qgis::SelectBehavior::SetSelection:
      process( context, selectLayer, intersectSource.get(), selectedPredicates, addToSelection, true, feedback );
      break;
  }

  selectLayer->selectByIds( selectedIds, method );
  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), parameters.value( QStringLiteral( "INPUT" ) ) );
  return results;
}


//
// QgsExtractByLocationAlgorithm
//

void QgsExtractByLocationAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Extract features from" ),
                QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );
  addPredicateParameter();
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INTERSECT" ),
                QObject::tr( "By comparing to the features from" ),
                QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Extracted (location)" ) ) );
}

QString QgsExtractByLocationAlgorithm::name() const
{
  return QStringLiteral( "extractbylocation" );
}

QString QgsExtractByLocationAlgorithm::displayName() const
{
  return QObject::tr( "Extract by location" );
}

QStringList QgsExtractByLocationAlgorithm::tags() const
{
  return QObject::tr( "extract,filter,intersects,intersecting,disjoint,touching,within,contains,overlaps,relation" ).split( ',' );
}

QString QgsExtractByLocationAlgorithm::group() const
{
  return QObject::tr( "Vector selection" );
}

QString QgsExtractByLocationAlgorithm::groupId() const
{
  return QStringLiteral( "vectorselection" );
}

QString QgsExtractByLocationAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that only contains matching features from an "
                      "input layer. The criteria for adding features to the resulting layer is defined "
                      "based on the spatial relationship between each feature and the features in an additional layer." );
}

QgsExtractByLocationAlgorithm *QgsExtractByLocationAlgorithm::createInstance() const
{
  return new QgsExtractByLocationAlgorithm();
}

QVariantMap QgsExtractByLocationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > input( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );
  std::unique_ptr< QgsFeatureSource > intersectSource( parameterAsSource( parameters, QStringLiteral( "INTERSECT" ), context ) );
  if ( !intersectSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INTERSECT" ) ) );

  const QList< int > selectedPredicates = parameterAsEnums( parameters, QStringLiteral( "PREDICATE" ), context );
  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, input->fields(), input->wkbType(), input->sourceCrs() ) );

  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  auto addToSink = [&]( const QgsFeature & feature )
  {
    QgsFeature f = feature;
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
  };
  process( context, input.get(), intersectSource.get(), selectedPredicates, addToSink, false, feedback );

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), dest );
  return results;
}

///@endcond


