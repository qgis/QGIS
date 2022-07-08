/***************************************************************************
                         qgsalgorithmmultidifference.cpp
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

#include "qgsalgorithmmultidifference.h"

#include "qgsoverlayutils.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE


QString QgsMultiDifferenceAlgorithm::name() const
{
  return QStringLiteral( "multidifference" );
}

QString QgsMultiDifferenceAlgorithm::displayName() const
{
  return QObject::tr( "Difference (multiple)" );
}

QStringList QgsMultiDifferenceAlgorithm::tags() const
{
  return QObject::tr( "difference,erase,not overlap" ).split( ',' );
}

QString QgsMultiDifferenceAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QString QgsMultiDifferenceAlgorithm::groupId() const
{
  return QStringLiteral( "vectoroverlay" );
}

QString QgsMultiDifferenceAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts features from the Input layer that fall completely outside or only partially overlap the features from any of the Overlay layer(s). "
                      "For each overlay layer the difference is calculated between the result of all previous difference operations and this overlay layer. "
                      "Input layer features that partially overlap feature(s) in the Overlay layers are split along those features' boundary "
                      "and only the portions outside the Overlay layer features are retained." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Attributes are not modified, although properties such as area or length of the features will "
                        "be modified by the difference operation. If such properties are stored as attributes, those attributes will have to "
                        "be manually updated." );
}

QgsProcessingAlgorithm *QgsMultiDifferenceAlgorithm::createInstance() const
{
  return new QgsMultiDifferenceAlgorithm();
}

void QgsMultiDifferenceAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "OVERLAYS" ), QObject::tr( "Overlay layers" ), QgsProcessing::TypeVectorAnyGeometry ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Difference" ) ) );
}


QVariantMap QgsMultiDifferenceAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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
  std::unique_ptr< QgsFeatureSink > sink;
  long count = 0;
  QVariantMap outputs;

  if ( totalLayerCount == 1 )
  {
    QString dest;
    sink.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, sourceA->fields(), geometryType, crs ) );
    if ( !sink )
      throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

    outputs.insert( QStringLiteral( "OUTPUT" ), dest );

    QgsVectorLayer *overlayLayer = qobject_cast< QgsVectorLayer * >( layers.at( 0 ) );

    const long total = sourceA->featureCount();
    QgsOverlayUtils::difference( *sourceA, *overlayLayer, *sink, context, feedback, count, total, QgsOverlayUtils::OutputA );
  }
  else
  {
    QgsProcessingMultiStepFeedback multiStepFeedback( totalLayerCount, feedback );
    QgsVectorLayer *differenceLayer = nullptr;

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
        QString id = QStringLiteral( "memory:" );
        sink.reset( QgsProcessingUtils::createFeatureSink( id, context, sourceA->fields(), geometryType, crs ) );
        QgsOverlayUtils::difference( *sourceA, *overlayLayer, *sink, context, &multiStepFeedback, count, sourceA->featureCount(), QgsOverlayUtils::OutputA );

        differenceLayer = qobject_cast< QgsVectorLayer * >( QgsProcessingUtils::mapLayerFromString( id, context ) );
      }
      else if ( i == totalLayerCount - 1 )
      {
        QString dest;
        std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, differenceLayer->fields(), geometryType, crs ) );
        if ( !sink )
          throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

        outputs.insert( QStringLiteral( "OUTPUT" ), dest );

        QgsOverlayUtils::difference( *differenceLayer, *overlayLayer, *sink, context, &multiStepFeedback, count, differenceLayer->featureCount(), QgsOverlayUtils::OutputA );
      }
      else
      {
        QString id = QStringLiteral( "memory:" );
        sink.reset( QgsProcessingUtils::createFeatureSink( id, context, differenceLayer->fields(), geometryType, crs ) );
        QgsOverlayUtils::difference( *differenceLayer, *overlayLayer, *sink, context, &multiStepFeedback, count, differenceLayer->featureCount(), QgsOverlayUtils::OutputA );

        differenceLayer = qobject_cast< QgsVectorLayer * >( QgsProcessingUtils::mapLayerFromString( id, context ) );
      }

      i++;
    }
  }

  return outputs;
}

///@endcond PRIVATE
