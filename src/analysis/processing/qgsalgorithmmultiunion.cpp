/***************************************************************************
                         qgsalgorithmmultiunion.cpp
                         ------------------
    begin                : December 2021
    copyright            : (C) 2021 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmmultiunion.h"

#include "qgsoverlayutils.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE


QString QgsMultiUnionAlgorithm::name() const
{
  return QStringLiteral( "multiunion" );
}

QString QgsMultiUnionAlgorithm::displayName() const
{
  return QObject::tr( "Union (multiple)" );
}

QStringList QgsMultiUnionAlgorithm::tags() const
{
  return QObject::tr( "union,overlap,not overlap" ).split( ',' );
}

QString QgsMultiUnionAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QString QgsMultiUnionAlgorithm::groupId() const
{
  return QStringLiteral( "vectoroverlay" );
}

QString QgsMultiUnionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm checks overlaps between features within the Input layer and creates separate features for overlapping "
                      "and non-overlapping parts. The area of overlap will create as many identical overlapping features as there are "
                      "features that participate in that overlap." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Multiple Overlay layers can also be used, in which case features from each layer are split at their overlap with features from "
                        "all other layers, creating a layer containing all the portions from both Input and Overlay layers. "
                        "The attribute table of the Union layer is filled with attribute values from the respective original layer "
                        "for non-overlapping features, and attribute values from both layers for overlapping features." );
}

QgsProcessingAlgorithm *QgsMultiUnionAlgorithm::createInstance() const
{
  return new QgsMultiUnionAlgorithm();
}

void QgsMultiUnionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "OVERLAYS" ), QObject::tr( "Overlay layers" ), QgsProcessing::TypeVectorAnyGeometry, QVariant(), true ) );

  std::unique_ptr< QgsProcessingParameterString > prefix = std::make_unique< QgsProcessingParameterString >( QStringLiteral( "OVERLAY_FIELDS_PREFIX" ), QObject::tr( "Overlay fields prefix" ), QString(), false, true );
  prefix->setFlags( prefix->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( prefix.release() );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Union" ) ) );
}

QVariantMap QgsMultiUnionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > sourceA( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !sourceA )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "OVERLAYS" ), context );

  // loop through overlay layers and check whether they are vectors
  long totalLayerCount = 0;
  for ( QgsMapLayer *layer : layers )
  {
    if ( feedback->isCanceled() )
      break;

    if ( !layer )
      throw QgsProcessingException( QObject::tr( "Error retrieving map layer." ) );

    if ( layer->type() != QgsMapLayerType::VectorLayer )
      throw QgsProcessingException( QObject::tr( "All layers must be vector layers!" ) );

    totalLayerCount++;
  }

  const QgsWkbTypes::Type geometryType = QgsWkbTypes::multiType( sourceA->wkbType() );
  const QgsCoordinateReferenceSystem crs = sourceA->sourceCrs();
  const QString overlayFieldsPrefix = parameterAsString( parameters, QStringLiteral( "OVERLAY_FIELDS_PREFIX" ), context );
  std::unique_ptr< QgsFeatureSink > sink;
  QVariantMap outputs;
  bool ok;

  if ( totalLayerCount == 0 )
  {
    // we are doing single layer union
    QString dest;
    std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, sourceA->fields(), geometryType, sourceA->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
    if ( !sink )
      throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

    outputs.insert( QStringLiteral( "OUTPUT" ), dest );

    QgsOverlayUtils::resolveOverlaps( *sourceA, *sink, feedback );
    return outputs;
  }
  else
  {
    QgsProcessingMultiStepFeedback multiStepFeedback( totalLayerCount, feedback );
    QgsVectorLayer *unionLayer = nullptr;
    QgsFields fields;

    long i = 0;
    for ( QgsMapLayer *layer : layers )
    {
      if ( feedback->isCanceled() )
        break;

      multiStepFeedback.setCurrentStep( i );

      if ( !layer )
        continue;

      QgsVectorLayer *overlayLayer = qobject_cast< QgsVectorLayer * >( layer );
      if ( !overlayLayer )
        continue;

      if ( i == 0 )
      {
        QString id = QStringLiteral( "memory:" );
        fields = QgsProcessingUtils::combineFields( sourceA->fields(), overlayLayer->fields(), overlayFieldsPrefix );
        sink.reset( QgsProcessingUtils::createFeatureSink( id, context, fields, geometryType, crs, QVariantMap(), QStringList(), QStringList(), QgsFeatureSink::RegeneratePrimaryKey ) );
        ok = makeUnion( *sourceA, *overlayLayer, *sink, context, &multiStepFeedback );

        if ( !ok )
          throw QgsProcessingException( QObject::tr( "Interrupted by user." ) );

        unionLayer = qobject_cast< QgsVectorLayer * >( QgsProcessingUtils::mapLayerFromString( id, context ) );
      }
      else if ( i == totalLayerCount - 1 )
      {
        fields = QgsProcessingUtils::combineFields( unionLayer->fields(), overlayLayer->fields(), overlayFieldsPrefix );


        QString dest;
        std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, geometryType, crs, QgsFeatureSink::RegeneratePrimaryKey ) );
        if ( !sink )
          throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

        outputs.insert( QStringLiteral( "OUTPUT" ), dest );
        ok = makeUnion( *unionLayer, *overlayLayer, *sink, context, &multiStepFeedback );
        if ( !ok )
          throw QgsProcessingException( QObject::tr( "Interrupted by user." ) );
      }
      else
      {
        QString id = QStringLiteral( "memory:" );
        fields = QgsProcessingUtils::combineFields( unionLayer->fields(), overlayLayer->fields(), overlayFieldsPrefix );
        sink.reset( QgsProcessingUtils::createFeatureSink( id, context, fields, geometryType, crs, QVariantMap(), QStringList(), QStringList(), QgsFeatureSink::RegeneratePrimaryKey ) );
        ok = makeUnion( *unionLayer, *overlayLayer, *sink, context, &multiStepFeedback );
        if ( !ok )
          throw QgsProcessingException( QObject::tr( "Interrupted by user." ) );

        unionLayer = qobject_cast< QgsVectorLayer * >( QgsProcessingUtils::mapLayerFromString( id, context ) );
      }

      i++;
    }
  }

  return outputs;
}

bool QgsMultiUnionAlgorithm::makeUnion( const QgsFeatureSource &sourceA, const QgsFeatureSource &sourceB, QgsFeatureSink &sink, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QList<int> fieldIndicesA = QgsProcessingUtils::fieldNamesToIndices( QStringList(), sourceA.fields() );
  const QList<int> fieldIndicesB = QgsProcessingUtils::fieldNamesToIndices( QStringList(), sourceB.fields() );

  long count = 0;
  const long total = sourceA.featureCount() * 2 + sourceB.featureCount();

  QgsOverlayUtils::intersection( sourceA, sourceB, sink, context, feedback, count, total, fieldIndicesA, fieldIndicesB );
  if ( feedback->isCanceled() )
    return false;

  QgsOverlayUtils::difference( sourceA, sourceB, sink, context, feedback, count, total, QgsOverlayUtils::OutputAB );
  if ( feedback->isCanceled() )
    return false;

  QgsOverlayUtils::difference( sourceB, sourceA, sink, context, feedback, count, total, QgsOverlayUtils::OutputBA );
  return true;
}

///@endcond
