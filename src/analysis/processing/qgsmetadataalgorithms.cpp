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
  return QObject::tr( "Metadata tools" );
}

QString QgsCopyLayerMetadataAlgorithm::groupId() const
{
  return QStringLiteral( "metadatatools" );
}

QString QgsCopyLayerMetadataAlgorithm::shortHelpString() const
{
  return QObject::tr( "Copies metadata from an input layer to a target layer.\n\nAny existing metadata in the target layer will be replaced." );
}

QgsCopyLayerMetadataAlgorithm *QgsCopyLayerMetadataAlgorithm::createInstance() const
{
  return new QgsCopyLayerMetadataAlgorithm();
}

void QgsCopyLayerMetadataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "INPUT" ), QObject::tr( "Source layer" ) ) );
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "TARGET" ), QObject::tr( "Target layer" ) ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DEFAULT" ), QObject::tr( "Save metadata as default" ), false ) );
  addOutput( new QgsProcessingOutputMapLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Updated layer" ) ) );
}

bool QgsCopyLayerMetadataAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *inputLayer = parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context );
  QgsMapLayer *targetLayer = parameterAsLayer( parameters, QStringLiteral( "TARGET" ), context );
  const bool saveAsDefault = parameterAsBool( parameters, QStringLiteral( "DEFAULT" ), context );

  if ( !inputLayer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  if ( !targetLayer )
    throw QgsProcessingException( QObject::tr( "Invalid target layer" ) );

  mLayerId = targetLayer->id();

  targetLayer->setMetadata( inputLayer->metadata() );
  if ( saveAsDefault )
  {
    bool ok;
    targetLayer->saveDefaultMetadata( ok );
    if ( !ok )
    {
      throw QgsProcessingException( QObject::tr( "Failed to save metadata as default metadata." ) );
    }
  }
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

///

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
  return QObject::tr( "Metadata tools" );
}

QString QgsApplyLayerMetadataAlgorithm::groupId() const
{
  return QStringLiteral( "metadatatools" );
}

QString QgsApplyLayerMetadataAlgorithm::shortHelpString() const
{
  return QObject::tr( "Applies the metadata to a layer. The metadata must be defined as QMD file.\n\nAny existing metadata in the layer will be replaced." );
}

QgsApplyLayerMetadataAlgorithm *QgsApplyLayerMetadataAlgorithm::createInstance() const
{
  return new QgsApplyLayerMetadataAlgorithm();
}

void QgsApplyLayerMetadataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "INPUT" ), QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterFile( QStringLiteral( "METADATA" ), QObject::tr( "Metadata file" ), Qgis::ProcessingFileParameterBehavior::File, QStringLiteral( "qmd" ) ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DEFAULT" ), QObject::tr( "Save metadata as default" ), false ) );
  addOutput( new QgsProcessingOutputMapLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Updated" ) ) );
}

bool QgsApplyLayerMetadataAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context );
  const QString metadata = parameterAsFile( parameters, QStringLiteral( "METADATA" ), context );
  const bool saveAsDefault = parameterAsBool( parameters, QStringLiteral( "DEFAULT" ), context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  mLayerId = layer->id();

  bool ok = false;
  const QString msg = layer->loadNamedMetadata( metadata, ok );
  if ( !ok )
  {
    throw QgsProcessingException( QObject::tr( "Failed to apply metadata. Error: %1" ).arg( msg ) );
  }

  if ( saveAsDefault )
  {
    bool ok;
    layer->saveDefaultMetadata( ok );
    if ( !ok )
    {
      throw QgsProcessingException( QObject::tr( "Failed to save metadata as default metadata." ) );
    }
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

///

QString QgsExportLayerMetadataAlgorithm::name() const
{
  return QStringLiteral( "exportlayermetadata" );
}

QString QgsExportLayerMetadataAlgorithm::displayName() const
{
  return QObject::tr( "Export layer metadata" );
}

QStringList QgsExportLayerMetadataAlgorithm::tags() const
{
  return QObject::tr( "export,layer,metadata,qmd" ).split( ',' );
}

QString QgsExportLayerMetadataAlgorithm::group() const
{
  return QObject::tr( "Metadata tools" );
}

QString QgsExportLayerMetadataAlgorithm::groupId() const
{
  return QStringLiteral( "metadatatools" );
}

QString QgsExportLayerMetadataAlgorithm::shortHelpString() const
{
  return QObject::tr( "Exports layer's metadata to a QMD file." );
}

QgsExportLayerMetadataAlgorithm *QgsExportLayerMetadataAlgorithm::createInstance() const
{
  return new QgsExportLayerMetadataAlgorithm();
}

void QgsExportLayerMetadataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "INPUT" ), QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output" ), QObject::tr( "QGIS Metadata File" ) + QStringLiteral( " (*.qmd *.QMD)" ) ) );
}

QVariantMap QgsExportLayerMetadataAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context );
  const QString outputFile = parameterAsString( parameters, QStringLiteral( "OUTPUT" ), context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  bool ok = false;
  const QString message = layer->saveNamedMetadata( outputFile, ok );
  if ( !ok )
  {
    throw QgsProcessingException( QObject::tr( "Failed to save metadata. Error: %1" ).arg( message ) );
  }

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return results;
}

///@endcond
