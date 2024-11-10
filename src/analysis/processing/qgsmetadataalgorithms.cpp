/***************************************************************************
                         qgsmetadataalgorithms.cpp
                         ---------------------
    begin                : November 2024
    copyright            : (C) 2024 by Alexander Bruy
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

#include "qgsmetadataalgorithms.h"

///@cond PRIVATE

QString QgsCopyLayerMetadataAlgorithm::name() const
{
  return QStringLiteral( "copylayermetadata" );
}

QString QgsCopyLayerMetadataAlgorithm::displayName() const
{
  return QObject::tr( "Copy layer metadata" );
}

QStringList QgsCopyLayerMetadataAlgorithm::tags() const
{
  return QObject::tr( "change,layer,metadata,qmd" ).split( ',' );
}

QString QgsCopyLayerMetadataAlgorithm::group() const
{
  return QObject::tr( "Layer tools" );
}

QString QgsCopyLayerMetadataAlgorithm::groupId() const
{
  return QStringLiteral( "layertools" );
}

QString QgsCopyLayerMetadataAlgorithm::shortHelpString() const
{
  return QObject::tr( "Copies metadata from an input layer to a target layer." );
}

QgsCopyLayerMetadataAlgorithm *QgsCopyLayerMetadataAlgorithm::createInstance() const
{
  return new QgsCopyLayerMetadataAlgorithm();
}

void QgsCopyLayerMetadataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "INPUT" ), QObject::tr( "Source layer" ) ) );
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "TARGET" ), QObject::tr( "Target layer" ) ) );
  addOutput( new QgsProcessingOutputMapLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Updated layer" ) ) );
}

bool QgsCopyLayerMetadataAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *inputLayer = parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context );
  QgsMapLayer *targetLayer = parameterAsLayer( parameters, QStringLiteral( "TARGET" ), context );

  if ( !inputLayer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  if ( !targetLayer )
    throw QgsProcessingException( QObject::tr( "Invalid target layer" ) );

  mLayerId = targetLayer->id();

  QgsLayerMetadata *metadata = inputLayer->metadata().clone();
  targetLayer->setMetadata( *metadata );
  return true;
}

QVariantMap QgsCopyLayerMetadataAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  Q_UNUSED( parameters );
  Q_UNUSED( context );

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), mLayerId );
  return results;
}



QString QgsApplyLayerMetadataAlgorithm::name() const
{
  return QStringLiteral( "setlayermetadata" );
}

QString QgsApplyLayerMetadataAlgorithm::displayName() const
{
  return QObject::tr( "Set layer metadata" );
}

QStringList QgsApplyLayerMetadataAlgorithm::tags() const
{
  return QObject::tr( "change,layer,metadata,qmd" ).split( ',' );
}

QString QgsApplyLayerMetadataAlgorithm::group() const
{
  return QObject::tr( "Layer tools" );
}

QString QgsApplyLayerMetadataAlgorithm::groupId() const
{
  return QStringLiteral( "layertools" );
}

QString QgsApplyLayerMetadataAlgorithm::shortHelpString() const
{
  return QObject::tr( "Applies the metadata to a layer. The metadata must be defined as QMD file." );
}

QgsApplyLayerMetadataAlgorithm *QgsApplyLayerMetadataAlgorithm::createInstance() const
{
  return new QgsApplyLayerMetadataAlgorithm();
}

void QgsApplyLayerMetadataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "INPUT" ), QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterFile( QStringLiteral( "METADATA" ), QObject::tr( "Metadata file" ), Qgis::ProcessingFileParameterBehavior::File, QStringLiteral( "qmd" ) ) );
  addOutput( new QgsProcessingOutputMapLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Updated" ) ) );
}

bool QgsApplyLayerMetadataAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context );
  const QString metadata = parameterAsFile( parameters, QStringLiteral( "METADATA" ), context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  mLayerId = layer->id();

  bool ok = false;
  const QString msg = layer->loadNamedMetadata( metadata, ok );
  if ( !ok )
  {
    throw QgsProcessingException( QObject::tr( "Failed to apply metadata. Error: %1" ).arg( msg ) );
  }

  return true;
}

QVariantMap QgsApplyLayerMetadataAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  Q_UNUSED( parameters );
  Q_UNUSED( context );

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), mLayerId );
  return results;
}

///@endcond
