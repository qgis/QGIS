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
  return QObject::tr( "Copies metadata from an source layer to a target layer.\n\nAny existing metadata in the target layer will be replaced." );
}

QgsCopyLayerMetadataAlgorithm *QgsCopyLayerMetadataAlgorithm::createInstance() const
{
  return new QgsCopyLayerMetadataAlgorithm();
}

void QgsCopyLayerMetadataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "SOURCE" ), QObject::tr( "Source layer" ) ) );
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "TARGET" ), QObject::tr( "Target layer" ) ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DEFAULT" ), QObject::tr( "Save metadata as default" ), false ) );
  addOutput( new QgsProcessingOutputMapLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Updated layer" ) ) );
}

bool QgsCopyLayerMetadataAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *sourceLayer = parameterAsLayer( parameters, QStringLiteral( "SOURCE" ), context );
  QgsMapLayer *targetLayer = parameterAsLayer( parameters, QStringLiteral( "TARGET" ), context );
  const bool saveAsDefault = parameterAsBool( parameters, QStringLiteral( "DEFAULT" ), context );

  if ( !sourceLayer )
    throw QgsProcessingException( QObject::tr( "Invalid source layer" ) );

  if ( !targetLayer )
    throw QgsProcessingException( QObject::tr( "Invalid target layer" ) );

  mLayerId = targetLayer->id();

  targetLayer->setMetadata( sourceLayer->metadata() );
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

///

QString QgsAddHistoryMetadataAlgorithm::name() const
{
  return QStringLiteral( "addhistorymetadata" );
}

QString QgsAddHistoryMetadataAlgorithm::displayName() const
{
  return QObject::tr( "Add history metadata" );
}

QStringList QgsAddHistoryMetadataAlgorithm::tags() const
{
  return QObject::tr( "add,history,metadata" ).split( ',' );
}

QString QgsAddHistoryMetadataAlgorithm::group() const
{
  return QObject::tr( "Metadata tools" );
}

QString QgsAddHistoryMetadataAlgorithm::groupId() const
{
  return QStringLiteral( "metadatatools" );
}

QString QgsAddHistoryMetadataAlgorithm::shortHelpString() const
{
  return QObject::tr( "Adds a new history entry to the layer's metadata." );
}

QgsAddHistoryMetadataAlgorithm *QgsAddHistoryMetadataAlgorithm::createInstance() const
{
  return new QgsAddHistoryMetadataAlgorithm();
}

void QgsAddHistoryMetadataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "INPUT" ), QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "HISTORY" ), QObject::tr( "History entry" ) ) );
  addOutput( new QgsProcessingOutputMapLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Updated" ) ) );
}

bool QgsAddHistoryMetadataAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context );
  const QString history = parameterAsString( parameters, QStringLiteral( "HISTORY" ), context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  mLayerId = layer->id();

  std::unique_ptr<QgsLayerMetadata> md( layer->metadata().clone() );
  md->addHistoryItem( history );
  layer->setMetadata( *md.get() );

  return true;
}

QVariantMap QgsAddHistoryMetadataAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  Q_UNUSED( parameters );
  Q_UNUSED( context );

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), mLayerId );
  return results;
}

///

QString QgsUpdateLayerMetadataAlgorithm::name() const
{
  return QStringLiteral( "updatelayermetadata" );
}

QString QgsUpdateLayerMetadataAlgorithm::displayName() const
{
  return QObject::tr( "Update layer metadata" );
}

QStringList QgsUpdateLayerMetadataAlgorithm::tags() const
{
  return QObject::tr( "change,update,layer,metadata,qmd" ).split( ',' );
}

QString QgsUpdateLayerMetadataAlgorithm::group() const
{
  return QObject::tr( "Metadata tools" );
}

QString QgsUpdateLayerMetadataAlgorithm::groupId() const
{
  return QStringLiteral( "metadatatools" );
}

QString QgsUpdateLayerMetadataAlgorithm::shortHelpString() const
{
  return QObject::tr( "Copies all non-empty metadata fields from an source layer to a target layer.\n\nLeaves empty input fields unchanged in the target." );
}

QgsUpdateLayerMetadataAlgorithm *QgsUpdateLayerMetadataAlgorithm::createInstance() const
{
  return new QgsUpdateLayerMetadataAlgorithm();
}

void QgsUpdateLayerMetadataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "SOURCE" ), QObject::tr( "Source layer" ) ) );
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "TARGET" ), QObject::tr( "Target layer" ) ) );
  addOutput( new QgsProcessingOutputMapLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Updated layer" ) ) );
}

bool QgsUpdateLayerMetadataAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *sourceLayer = parameterAsLayer( parameters, QStringLiteral( "SOURCE" ), context );
  QgsMapLayer *targetLayer = parameterAsLayer( parameters, QStringLiteral( "TARGET" ), context );

  if ( !sourceLayer )
    throw QgsProcessingException( QObject::tr( "Invalid source layer" ) );

  if ( !targetLayer )
    throw QgsProcessingException( QObject::tr( "Invalid target layer" ) );

  mLayerId = targetLayer->id();

  std::unique_ptr<QgsLayerMetadata> md( targetLayer->metadata().clone() );
  md->combine( &sourceLayer->metadata() );
  targetLayer->setMetadata( *md.get() );

  return true;
}

QVariantMap QgsUpdateLayerMetadataAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  Q_UNUSED( parameters );
  Q_UNUSED( context );

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), mLayerId );
  return results;
}

///

QString QgsSetMetadataFieldsAlgorithm::name() const
{
  return QStringLiteral( "setmetadatafields" );
}

QString QgsSetMetadataFieldsAlgorithm::displayName() const
{
  return QObject::tr( "Set metadata fields" );
}

QStringList QgsSetMetadataFieldsAlgorithm::tags() const
{
  return QObject::tr( "set,metadata,title,abstract,identifier" ).split( ',' );
}

QString QgsSetMetadataFieldsAlgorithm::group() const
{
  return QObject::tr( "Metadata tools" );
}

QString QgsSetMetadataFieldsAlgorithm::groupId() const
{
  return QStringLiteral( "metadatatools" );
}

QString QgsSetMetadataFieldsAlgorithm::shortHelpString() const
{
  return QObject::tr( "Sets various metadata fields for a layer." );
}

QgsSetMetadataFieldsAlgorithm *QgsSetMetadataFieldsAlgorithm::createInstance() const
{
  return new QgsSetMetadataFieldsAlgorithm();
}

void QgsSetMetadataFieldsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "INPUT" ), QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "IDENTIFIER" ), QObject::tr( "Identifier" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "PARENT_IDENTIFIER" ), QObject::tr( "Parent identifier" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "TITLE" ), QObject::tr( "Title" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "TYPE" ), QObject::tr( "Type" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "LANGUAGE" ), QObject::tr( "Language" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "ENCODING" ), QObject::tr( "Encoding" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "ABSTRACT" ), QObject::tr( "Abstract" ), QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS" ), QObject::tr( "Coordinate reference system" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "FEES" ), QObject::tr( "Fees" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "IGNORE_EMPTY" ), QObject::tr( "Ignore empty fields" ), false ) );
  addOutput( new QgsProcessingOutputMapLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Updated" ) ) );
}

bool QgsSetMetadataFieldsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  mLayerId = layer->id();

  const bool ignoreEmpty = parameterAsBool( parameters, QStringLiteral( "IGNORE_EMPTY" ), context );

  std::unique_ptr<QgsLayerMetadata> md( layer->metadata().clone() );

  if ( parameters.value( QStringLiteral( "IDENTIFIER" ) ).isValid() )
  {
    const QString identifier = parameterAsString( parameters, QStringLiteral( "IDENTIFIER" ), context );
    if ( !identifier.isEmpty() || !ignoreEmpty )
    {
      md->setIdentifier( identifier );
    }
  }

  if ( parameters.value( QStringLiteral( "PARENT_IDENTIFIER" ) ).isValid() )
  {
    const QString parentIdentifier = parameterAsString( parameters, QStringLiteral( "PARENT_IDENTIFIER" ), context );
    if ( !parentIdentifier.isEmpty() || !ignoreEmpty )
    {
      md->setParentIdentifier( parentIdentifier );
    }
  }

  if ( parameters.value( QStringLiteral( "TITLE" ) ).isValid() )
  {
    const QString title = parameterAsString( parameters, QStringLiteral( "TITLE" ), context );
    if ( !title.isEmpty() || !ignoreEmpty )
    {
      md->setTitle( title );
    }
  }

  if ( parameters.value( QStringLiteral( "TYPE" ) ).isValid() )
  {
    const QString type = parameterAsString( parameters, QStringLiteral( "TYPE" ), context );
    if ( !type.isEmpty() || !ignoreEmpty )
    {
      md->setType( type );
    }
  }

  if ( parameters.value( QStringLiteral( "LANGUAGE" ) ).isValid() )
  {
    const QString language = parameterAsString( parameters, QStringLiteral( "LANGUAGE" ), context );
    if ( !language.isEmpty() || !ignoreEmpty )
    {
      md->setLanguage( language );
    }
  }

  if ( parameters.value( QStringLiteral( "ENCODING" ) ).isValid() )
  {
    const QString encoding = parameterAsString( parameters, QStringLiteral( "ENCODING" ), context );
    if ( !encoding.isEmpty() || !ignoreEmpty )
    {
      md->setEncoding( encoding );
    }
  }

  if ( parameters.value( QStringLiteral( "ABSTRACT" ) ).isValid() )
  {
    const QString abstract = parameterAsString( parameters, QStringLiteral( "ABSTRACT" ), context );
    if ( !abstract.isEmpty() || !ignoreEmpty )
    {
      md->setAbstract( abstract );
    }
  }

  if ( parameters.value( QStringLiteral( "CRS" ) ).isValid() )
  {
    const QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );
    if ( crs.isValid() || !ignoreEmpty )
    {
      md->setCrs( crs );
    }
  }

  if ( parameters.value( QStringLiteral( "FEES" ) ).isValid() )
  {
    const QString fees = parameterAsString( parameters, QStringLiteral( "FEES" ), context );
    if ( !fees.isEmpty() || !ignoreEmpty )
    {
      md->setFees( fees );
    }
  }

  layer->setMetadata( *md.get() );

  return true;
}

QVariantMap QgsSetMetadataFieldsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  Q_UNUSED( parameters );
  Q_UNUSED( context );

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), mLayerId );
  return results;
}

///@endcond
