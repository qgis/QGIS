/***************************************************************************
                         qgsalgorithmdistancewithin.cpp
                         ---------------------
    begin                : August 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#include "qgsalgorithmdistancewithin.h"
#include "qgsgeometryengine.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

void QgsDistanceWithinAlgorithm::addDistanceParameter()
{
  std::unique_ptr< QgsProcessingParameterDistance > distanceParam( new QgsProcessingParameterDistance( QStringLiteral( "DISTANCE" ),
      QObject::tr( "Where the features are within" ), 100, QStringLiteral( "INPUT" ), false, 0 ) );
  distanceParam->setIsDynamic( true );
  distanceParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Distance" ), QObject::tr( "Distance within" ), QgsPropertyDefinition::DoublePositive ) );
  distanceParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );

  addParameter( distanceParam.release() );
}

void QgsDistanceWithinAlgorithm::process( const QgsProcessingContext &context, QgsFeatureSource *targetSource,
    QgsFeatureSource *referenceSource,
    double distance, const QgsProperty &distanceProperty,
    const std::function < void( const QgsFeature & ) > &handleFeatureFunction,
    bool onlyRequireTargetIds,
    QgsProcessingFeedback *feedback, QgsExpressionContext &expressionContext )
{
  // By default we will iterate over the reference source and match back
  // to the target source. We do this on the assumption that the most common
  // use case is joining a points layer to a polygon layer (e.g. findings
  // points near a polygon), so by iterating
  // over the polygons we can take advantage of prepared geometries for
  // the spatial relationship test.
  bool iterateOverTarget = false;

  //
  // Possible reasons to iterate over target are considered here
  //
  do
  {
    // If distance is dynamic, we MUST iterate over target
    if ( distanceProperty.isActive() )
    {
      iterateOverTarget = true;
      break;
    }

    // If reference needs reprojection, we MUST iterate over target
    if ( targetSource->sourceCrs() != referenceSource->sourceCrs() )
    {
      iterateOverTarget = true;
      break;
    }

    // if reference is POINTs and target is not, we prefer iterating
    // over target, to benefit from preparation
    if ( referenceSource->wkbType() == QgsWkbTypes::Point &&
         targetSource->wkbType() != QgsWkbTypes::Point )
    {
      iterateOverTarget = true;
      break;
    }

    // neither source nor target or both of them are POINTs, we will
    // iterate over the source with FEWER features to prepare less
    if ( targetSource->featureCount() < referenceSource->featureCount() )
    {
      iterateOverTarget = true;
      break;
    }
  }
  while ( 0 );

  if ( iterateOverTarget )
  {
    processByIteratingOverTargetSource( context, targetSource, referenceSource,
                                        distance, distanceProperty, handleFeatureFunction,
                                        onlyRequireTargetIds, feedback, expressionContext );
  }
  else
  {
    processByIteratingOverReferenceSource( context, targetSource, referenceSource,
                                           distance, handleFeatureFunction,
                                           onlyRequireTargetIds, feedback );
  }
}

void QgsDistanceWithinAlgorithm::processByIteratingOverTargetSource( const QgsProcessingContext &context, QgsFeatureSource *targetSource,
    QgsFeatureSource *referenceSource,
    const double distance, const QgsProperty &distanceProperty,
    const std::function < void( const QgsFeature & ) > &handleFeatureFunction,
    bool onlyRequireTargetIds,
    QgsProcessingFeedback *feedback, QgsExpressionContext &expressionContext )
{
  if ( referenceSource->hasSpatialIndex() == QgsFeatureSource::SpatialIndexNotPresent )
    feedback->pushWarning( QObject::tr( "No spatial index exists for intersect layer, performance will be severely degraded" ) );

  QgsFeatureIds foundSet;
  QgsFeatureRequest request = QgsFeatureRequest();
  if ( onlyRequireTargetIds )
    request.setNoAttributes();

  const bool dynamicDistance = distanceProperty.isActive();

  QgsFeatureIterator fIt = targetSource->getFeatures( request );
  const double step = targetSource->featureCount() > 0 ? 100.0 / targetSource->featureCount() : 1;
  int current = 0;
  QgsFeature f;
  while ( fIt.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    if ( !f.hasGeometry() )
      continue;

    double currentDistance = distance;
    if ( dynamicDistance )
    {
      expressionContext.setFeature( f );
      currentDistance = distanceProperty.valueAsDouble( expressionContext, currentDistance );
    }

    request = QgsFeatureRequest().setDistanceWithin( f.geometry(), currentDistance ).setNoAttributes().setDestinationCrs( targetSource->sourceCrs(), context.transformContext() );
    // we only care IF there's ANY features within the target distance here, so fetch at most 1 feature
    request.setLimit( 1 );

    QgsFeatureIterator testFeatureIt = referenceSource->getFeatures( request );
    QgsFeature testFeature;
    if ( testFeatureIt.nextFeature( testFeature ) )
    {
      foundSet.insert( f.id() );
      handleFeatureFunction( f );
    }

    current += 1;
    feedback->setProgress( current * step );
  }
}

void QgsDistanceWithinAlgorithm::processByIteratingOverReferenceSource( const QgsProcessingContext &context, QgsFeatureSource *targetSource,
    QgsFeatureSource *referenceSource,
    const double distance,
    const std::function < void( const QgsFeature & ) > &handleFeatureFunction,
    bool onlyRequireTargetIds,
    QgsProcessingFeedback *feedback )
{
  if ( targetSource->hasSpatialIndex() == QgsFeatureSource::SpatialIndexNotPresent )
    feedback->pushWarning( QObject::tr( "No spatial index exists for input layer, performance will be severely degraded" ) );

  QgsFeatureIds foundSet;

  QgsFeatureRequest request = QgsFeatureRequest().setNoAttributes().setDestinationCrs( targetSource->sourceCrs(), context.transformContext() );
  QgsFeatureIterator fIt = referenceSource->getFeatures( request );
  const double step = referenceSource->featureCount() > 0 ? 100.0 / referenceSource->featureCount() : 1;
  int current = 0;
  QgsFeature f;
  while ( fIt.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    if ( !f.hasGeometry() )
      continue;

    request = QgsFeatureRequest().setDistanceWithin( f.geometry(), distance );
    if ( onlyRequireTargetIds )
      request.setNoAttributes();

    QgsFeatureIterator testFeatureIt = targetSource->getFeatures( request );
    QgsFeature testFeature;
    while ( testFeatureIt.nextFeature( testFeature ) )
    {
      if ( feedback->isCanceled() )
        break;

      if ( foundSet.contains( testFeature.id() ) )
      {
        // already added this one, no need for further tests
        continue;
      }

      foundSet.insert( testFeature.id() );
      handleFeatureFunction( testFeature );
    }

    current += 1;
    feedback->setProgress( current * step );
  }
}


//
// QgsSelectWithinDistanceAlgorithm
//

void QgsSelectWithinDistanceAlgorithm::initAlgorithm( const QVariantMap & )
{
  const QStringList methods = QStringList() << QObject::tr( "creating new selection" )
                              << QObject::tr( "adding to current selection" )
                              << QObject::tr( "selecting within current selection" )
                              << QObject::tr( "removing from current selection" );

  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "INPUT" ), QObject::tr( "Select features from" ),
                QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );

  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "REFERENCE" ),
                QObject::tr( "By comparing to the features from" ),
                QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );
  addDistanceParameter();

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "METHOD" ),
                QObject::tr( "Modify current selection by" ),
                methods, false, 0 ) );
}

QString QgsSelectWithinDistanceAlgorithm::name() const
{
  return QStringLiteral( "selectwithindistance" );
}

QgsProcessingAlgorithm::Flags QgsSelectWithinDistanceAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagNoThreading | QgsProcessingAlgorithm::FlagNotAvailableInStandaloneTool;
}

QString QgsSelectWithinDistanceAlgorithm::displayName() const
{
  return QObject::tr( "Select within distance" );
}

QStringList QgsSelectWithinDistanceAlgorithm::tags() const
{
  return QObject::tr( "select,by,maximum,buffer" ).split( ',' );
}

QString QgsSelectWithinDistanceAlgorithm::group() const
{
  return QObject::tr( "Vector selection" );
}

QString QgsSelectWithinDistanceAlgorithm::groupId() const
{
  return QStringLiteral( "vectorselection" );
}

QString QgsSelectWithinDistanceAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a selection in a vector layer. Features are selected wherever they are within "
                      "the specified maximum distance from the features in an additional reference layer." );
}

QgsSelectWithinDistanceAlgorithm *QgsSelectWithinDistanceAlgorithm::createInstance() const
{
  return new QgsSelectWithinDistanceAlgorithm();
}

QVariantMap QgsSelectWithinDistanceAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsVectorLayer *selectLayer = parameterAsVectorLayer( parameters, QStringLiteral( "INPUT" ), context );
  if ( !selectLayer )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for INPUT" ) );

  const Qgis::SelectBehavior method = static_cast< Qgis::SelectBehavior >( parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context ) );
  const std::unique_ptr< QgsFeatureSource > referenceSource( parameterAsSource( parameters, QStringLiteral( "REFERENCE" ), context ) );
  if ( !referenceSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "REFERENCE" ) ) );

  const double distance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  const bool dynamicDistance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  QgsProperty distanceProperty;
  if ( dynamicDistance )
    distanceProperty = parameters.value( QStringLiteral( "DISTANCE" ) ).value< QgsProperty >();
  QgsExpressionContext expressionContext = createExpressionContext( parameters, context );
  expressionContext.appendScope( selectLayer->createExpressionContextScope() );

  QgsFeatureIds selectedIds;
  auto addToSelection = [&]( const QgsFeature & feature )
  {
    selectedIds.insert( feature.id() );
  };
  process( context, selectLayer, referenceSource.get(), distance, distanceProperty, addToSelection, true, feedback, expressionContext );

  selectLayer->selectByIds( selectedIds, method );
  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), parameters.value( QStringLiteral( "INPUT" ) ) );
  return results;
}


//
// QgsExtractWithinDistanceAlgorithm
//

void QgsExtractWithinDistanceAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Extract features from" ),
                QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "REFERENCE" ),
                QObject::tr( "By comparing to the features from" ),
                QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );
  addDistanceParameter();

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Extracted (location)" ) ) );
}

QString QgsExtractWithinDistanceAlgorithm::name() const
{
  return QStringLiteral( "extractwithindistance" );
}

QString QgsExtractWithinDistanceAlgorithm::displayName() const
{
  return QObject::tr( "Extract within distance" );
}

QStringList QgsExtractWithinDistanceAlgorithm::tags() const
{
  return QObject::tr( "extract,by,filter,select,maximum,buffer" ).split( ',' );
}

QString QgsExtractWithinDistanceAlgorithm::group() const
{
  return QObject::tr( "Vector selection" );
}

QString QgsExtractWithinDistanceAlgorithm::groupId() const
{
  return QStringLiteral( "vectorselection" );
}

QString QgsExtractWithinDistanceAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that only contains matching features from an "
                      "input layer. Features are copied wherever they are within "
                      "the specified maximum distance from the features in an additional reference layer." );
}

QgsExtractWithinDistanceAlgorithm *QgsExtractWithinDistanceAlgorithm::createInstance() const
{
  return new QgsExtractWithinDistanceAlgorithm();
}

QVariantMap QgsExtractWithinDistanceAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > input( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );
  const std::unique_ptr< QgsFeatureSource > referenceSource( parameterAsSource( parameters, QStringLiteral( "REFERENCE" ), context ) );
  if ( !referenceSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "REFERENCE" ) ) );

  const double distance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  const bool dynamicDistance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  QgsProperty distanceProperty;
  if ( dynamicDistance )
    distanceProperty = parameters.value( QStringLiteral( "DISTANCE" ) ).value< QgsProperty >();
  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, input.get() );

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
  process( context, input.get(), referenceSource.get(), distance, distanceProperty, addToSink, false, feedback, expressionContext );

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), dest );
  return results;
}

///@endcond


