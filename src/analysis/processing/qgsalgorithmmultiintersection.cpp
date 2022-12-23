/***************************************************************************
                         qgsalgorithmmultiintersection.cpp
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

#include "qgsalgorithmmultiintersection.h"

#include "qgsgeometrycollection.h"
#include "qgsgeometryengine.h"
#include "qgsoverlayutils.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE


QString QgsMultiIntersectionAlgorithm::name() const
{
  return QStringLiteral( "multiintersection" );
}

QString QgsMultiIntersectionAlgorithm::displayName() const
{
  return QObject::tr( "Intersection (multiple)" );
}

QStringList QgsMultiIntersectionAlgorithm::tags() const
{
  return QObject::tr( "intersection,extract,overlap" ).split( ',' );
}

QString QgsMultiIntersectionAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QString QgsMultiIntersectionAlgorithm::groupId() const
{
  return QStringLiteral( "vectoroverlay" );
}

QString QgsMultiIntersectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts the overlapping portions of features in the Input and all Overlay layers. "
                      "Features in the output layer are assigned the attributes of the overlapping features "
                      "from both the Input and Overlay layers." );
}

QgsProcessingAlgorithm *QgsMultiIntersectionAlgorithm::createInstance() const
{
  return new QgsMultiIntersectionAlgorithm();
}

void QgsMultiIntersectionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "OVERLAYS" ), QObject::tr( "Overlay layers" ), QgsProcessing::TypeVectorAnyGeometry ) );

  std::unique_ptr< QgsProcessingParameterString > prefix = std::make_unique< QgsProcessingParameterString >( QStringLiteral( "OVERLAY_FIELDS_PREFIX" ), QObject::tr( "Overlay fields prefix" ), QString(), false, true );
  prefix->setFlags( prefix->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( prefix.release() );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Intersection" ) ) );
}

QVariantMap QgsMultiIntersectionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  const QString overlayFieldsPrefix = parameterAsString( parameters, QStringLiteral( "OVERLAY_FIELDS_PREFIX" ), context );

  const QgsWkbTypes::Type geometryType = QgsWkbTypes::multiType( sourceA->wkbType() );
  const QgsCoordinateReferenceSystem crs = sourceA->sourceCrs();
  std::unique_ptr< QgsFeatureSink > sink;
  long count = 0;
  QVariantMap outputs;

  QList<int> fieldIndicesA, fieldIndicesB;
  QgsFields outputFields;

  if ( totalLayerCount == 1 )
  {
    QgsVectorLayer *overlayLayer = qobject_cast< QgsVectorLayer * >( layers.at( 0 ) );

    fieldIndicesA = QgsProcessingUtils::fieldNamesToIndices( QStringList(), sourceA->fields() );
    fieldIndicesB = QgsProcessingUtils::fieldNamesToIndices( QStringList(), overlayLayer->fields() );

    outputFields = QgsProcessingUtils::combineFields(
                     QgsProcessingUtils::indicesToFields( fieldIndicesA, sourceA->fields() ),
                     QgsProcessingUtils::indicesToFields( fieldIndicesB, overlayLayer->fields() ),
                     overlayFieldsPrefix );

    QString dest;
    sink.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outputFields, geometryType, crs, QgsFeatureSink::RegeneratePrimaryKey ) );
    if ( !sink )
      throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

    outputs.insert( QStringLiteral( "OUTPUT" ), dest );

    const long total = sourceA->featureCount();
    QgsOverlayUtils::intersection( *sourceA, *overlayLayer, *sink, context, feedback, count, total, fieldIndicesA, fieldIndicesB );
  }
  else
  {
    QgsProcessingMultiStepFeedback multiStepFeedback( totalLayerCount, feedback );
    QgsVectorLayer *intersectionLayer = nullptr;

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

      count = 0;
      if ( i == 0 )
      {
        fieldIndicesA = QgsProcessingUtils::fieldNamesToIndices( QStringList(), sourceA->fields() );
        fieldIndicesB = QgsProcessingUtils::fieldNamesToIndices( QStringList(), overlayLayer->fields() );

        outputFields = QgsProcessingUtils::combineFields(
                         QgsProcessingUtils::indicesToFields( fieldIndicesA, sourceA->fields() ),
                         QgsProcessingUtils::indicesToFields( fieldIndicesB, overlayLayer->fields() ),
                         overlayFieldsPrefix );

        QString id = QStringLiteral( "memory:" );
        sink.reset( QgsProcessingUtils::createFeatureSink( id, context, outputFields, geometryType, crs ) );
        QgsOverlayUtils::intersection( *sourceA, *overlayLayer, *sink, context, &multiStepFeedback, count, sourceA->featureCount(), fieldIndicesA, fieldIndicesB );

        intersectionLayer = qobject_cast< QgsVectorLayer * >( QgsProcessingUtils::mapLayerFromString( id, context ) );
      }
      else if ( i == totalLayerCount - 1 )
      {
        fieldIndicesA = QgsProcessingUtils::fieldNamesToIndices( QStringList(), intersectionLayer->fields() );
        fieldIndicesB = QgsProcessingUtils::fieldNamesToIndices( QStringList(), overlayLayer->fields() );

        outputFields = QgsProcessingUtils::combineFields(
                         QgsProcessingUtils::indicesToFields( fieldIndicesA, intersectionLayer->fields() ),
                         QgsProcessingUtils::indicesToFields( fieldIndicesB, overlayLayer->fields() ),
                         overlayFieldsPrefix );

        QString dest;
        std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outputFields, geometryType, crs ) );
        if ( !sink )
          throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

        outputs.insert( QStringLiteral( "OUTPUT" ), dest );

        QgsOverlayUtils::intersection( *intersectionLayer, *overlayLayer, *sink, context, &multiStepFeedback, count, intersectionLayer->featureCount(), fieldIndicesA, fieldIndicesB );
      }
      else
      {
        fieldIndicesA = QgsProcessingUtils::fieldNamesToIndices( QStringList(), intersectionLayer->fields() );
        fieldIndicesB = QgsProcessingUtils::fieldNamesToIndices( QStringList(), overlayLayer->fields() );

        outputFields = QgsProcessingUtils::combineFields(
                         QgsProcessingUtils::indicesToFields( fieldIndicesA, intersectionLayer->fields() ),
                         QgsProcessingUtils::indicesToFields( fieldIndicesB, overlayLayer->fields() ),
                         overlayFieldsPrefix );

        QString id = QStringLiteral( "memory:" );
        sink.reset( QgsProcessingUtils::createFeatureSink( id, context, outputFields, geometryType, crs ) );
        QgsOverlayUtils::intersection( *intersectionLayer, *overlayLayer, *sink, context, &multiStepFeedback, count, intersectionLayer->featureCount(), fieldIndicesA, fieldIndicesB );

        intersectionLayer = qobject_cast< QgsVectorLayer * >( QgsProcessingUtils::mapLayerFromString( id, context ) );
      }

      i++;
    }
  }

  return outputs;
}

///@endcond
