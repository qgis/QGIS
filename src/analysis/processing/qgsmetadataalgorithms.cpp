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
  return u"copylayermetadata"_s;
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
  return u"metadatatools"_s;
}

QString QgsCopyLayerMetadataAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm copies metadata from a source layer to a target layer.\n\nAny existing metadata in the target layer will be replaced." );
}

QString QgsCopyLayerMetadataAlgorithm::shortDescription() const
{
  return QObject::tr( "Copies the metadata from one layer to another." );
}

QgsCopyLayerMetadataAlgorithm *QgsCopyLayerMetadataAlgorithm::createInstance() const
{
  return new QgsCopyLayerMetadataAlgorithm();
}

void QgsCopyLayerMetadataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( u"SOURCE"_s, QObject::tr( "Source layer" ) ) );
  addParameter( new QgsProcessingParameterMapLayer( u"TARGET"_s, QObject::tr( "Target layer" ) ) );
  addParameter( new QgsProcessingParameterBoolean( u"DEFAULT"_s, QObject::tr( "Save metadata as default" ), false ) );
  addOutput( new QgsProcessingOutputMapLayer( u"OUTPUT"_s, QObject::tr( "Updated layer" ) ) );
}

bool QgsCopyLayerMetadataAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *sourceLayer = parameterAsLayer( parameters, u"SOURCE"_s, context );
  QgsMapLayer *targetLayer = parameterAsLayer( parameters, u"TARGET"_s, context );
  const bool saveAsDefault = parameterAsBool( parameters, u"DEFAULT"_s, context );

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
  results.insert( u"OUTPUT"_s, mLayerId );
  return results;
}

///

QString QgsApplyLayerMetadataAlgorithm::name() const
{
  return u"setlayermetadata"_s;
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
  return u"metadatatools"_s;
}

QString QgsApplyLayerMetadataAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm applies the metadata to a layer. The metadata must be defined as QMD file.\n\nAny existing metadata in the layer will be replaced." );
}

QString QgsApplyLayerMetadataAlgorithm::shortDescription() const
{
  return QObject::tr( "Applies the metadata from a QMD file to a layer." );
}

QgsApplyLayerMetadataAlgorithm *QgsApplyLayerMetadataAlgorithm::createInstance() const
{
  return new QgsApplyLayerMetadataAlgorithm();
}

void QgsApplyLayerMetadataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( u"INPUT"_s, QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterFile( u"METADATA"_s, QObject::tr( "Metadata file" ), Qgis::ProcessingFileParameterBehavior::File, u"qmd"_s ) );
  addParameter( new QgsProcessingParameterBoolean( u"DEFAULT"_s, QObject::tr( "Save metadata as default" ), false ) );
  addOutput( new QgsProcessingOutputMapLayer( u"OUTPUT"_s, QObject::tr( "Updated" ) ) );
}

bool QgsApplyLayerMetadataAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, u"INPUT"_s, context );
  const QString metadata = parameterAsFile( parameters, u"METADATA"_s, context );
  const bool saveAsDefault = parameterAsBool( parameters, u"DEFAULT"_s, context );

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
  results.insert( u"OUTPUT"_s, mLayerId );
  return results;
}

///

QString QgsExportLayerMetadataAlgorithm::name() const
{
  return u"exportlayermetadata"_s;
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
  return u"metadatatools"_s;
}

QString QgsExportLayerMetadataAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports layer's metadata to a QMD file." );
}

QString QgsExportLayerMetadataAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports layer's metadata to a QMD file." );
}

QgsExportLayerMetadataAlgorithm *QgsExportLayerMetadataAlgorithm::createInstance() const
{
  return new QgsExportLayerMetadataAlgorithm();
}

void QgsExportLayerMetadataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( u"INPUT"_s, QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterFileDestination( u"OUTPUT"_s, QObject::tr( "Output" ), QObject::tr( "QGIS Metadata File" ) + u" (*.qmd *.QMD)"_s ) );
}

QVariantMap QgsExportLayerMetadataAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, u"INPUT"_s, context );
  const QString outputFile = parameterAsString( parameters, u"OUTPUT"_s, context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  bool ok = false;
  const QString message = layer->saveNamedMetadata( outputFile, ok );
  if ( !ok )
  {
    throw QgsProcessingException( QObject::tr( "Failed to save metadata. Error: %1" ).arg( message ) );
  }

  QVariantMap results;
  results.insert( u"OUTPUT"_s, outputFile );
  return results;
}

///

QString QgsAddHistoryMetadataAlgorithm::name() const
{
  return u"addhistorymetadata"_s;
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
  return u"metadatatools"_s;
}

QString QgsAddHistoryMetadataAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm adds a new history entry to the layer's metadata." );
}

QString QgsAddHistoryMetadataAlgorithm::shortDescription() const
{
  return QObject::tr( "Adds a new history entry to the layer's metadata." );
}

QgsAddHistoryMetadataAlgorithm *QgsAddHistoryMetadataAlgorithm::createInstance() const
{
  return new QgsAddHistoryMetadataAlgorithm();
}

void QgsAddHistoryMetadataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( u"INPUT"_s, QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterString( u"HISTORY"_s, QObject::tr( "History entry" ) ) );
  addOutput( new QgsProcessingOutputMapLayer( u"OUTPUT"_s, QObject::tr( "Updated" ) ) );
}

bool QgsAddHistoryMetadataAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, u"INPUT"_s, context );
  const QString history = parameterAsString( parameters, u"HISTORY"_s, context );

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
  results.insert( u"OUTPUT"_s, mLayerId );
  return results;
}

///

QString QgsUpdateLayerMetadataAlgorithm::name() const
{
  return u"updatelayermetadata"_s;
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
  return u"metadatatools"_s;
}

QString QgsUpdateLayerMetadataAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm copies all non-empty metadata fields from a source layer to a target layer.\n\nLeaves empty input fields unchanged in the target." );
}

QString QgsUpdateLayerMetadataAlgorithm::shortDescription() const
{
  return QObject::tr( "Copies all non-empty metadata fields from one layer to another." );
}

QgsUpdateLayerMetadataAlgorithm *QgsUpdateLayerMetadataAlgorithm::createInstance() const
{
  return new QgsUpdateLayerMetadataAlgorithm();
}

void QgsUpdateLayerMetadataAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( u"SOURCE"_s, QObject::tr( "Source layer" ) ) );
  addParameter( new QgsProcessingParameterMapLayer( u"TARGET"_s, QObject::tr( "Target layer" ) ) );
  addOutput( new QgsProcessingOutputMapLayer( u"OUTPUT"_s, QObject::tr( "Updated layer" ) ) );
}

bool QgsUpdateLayerMetadataAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *sourceLayer = parameterAsLayer( parameters, u"SOURCE"_s, context );
  QgsMapLayer *targetLayer = parameterAsLayer( parameters, u"TARGET"_s, context );

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
  results.insert( u"OUTPUT"_s, mLayerId );
  return results;
}

///

QString QgsSetMetadataFieldsAlgorithm::name() const
{
  return u"setmetadatafields"_s;
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
  return u"metadatatools"_s;
}

QString QgsSetMetadataFieldsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm sets various metadata fields for a layer." );
}

QString QgsSetMetadataFieldsAlgorithm::shortDescription() const
{
  return QObject::tr( "Sets various metadata fields for a layer." );
}

QgsSetMetadataFieldsAlgorithm *QgsSetMetadataFieldsAlgorithm::createInstance() const
{
  return new QgsSetMetadataFieldsAlgorithm();
}

void QgsSetMetadataFieldsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( u"INPUT"_s, QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterString( u"IDENTIFIER"_s, QObject::tr( "Identifier" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( u"PARENT_IDENTIFIER"_s, QObject::tr( "Parent identifier" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( u"TITLE"_s, QObject::tr( "Title" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( u"TYPE"_s, QObject::tr( "Type" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( u"LANGUAGE"_s, QObject::tr( "Language" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( u"ENCODING"_s, QObject::tr( "Encoding" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( u"ABSTRACT"_s, QObject::tr( "Abstract" ), QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterCrs( u"CRS"_s, QObject::tr( "Coordinate reference system" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterString( u"FEES"_s, QObject::tr( "Fees" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterBoolean( u"IGNORE_EMPTY"_s, QObject::tr( "Ignore empty fields" ), false ) );
  addOutput( new QgsProcessingOutputMapLayer( u"OUTPUT"_s, QObject::tr( "Updated" ) ) );
}

bool QgsSetMetadataFieldsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, u"INPUT"_s, context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  mLayerId = layer->id();

  const bool ignoreEmpty = parameterAsBool( parameters, u"IGNORE_EMPTY"_s, context );

  std::unique_ptr<QgsLayerMetadata> md( layer->metadata().clone() );

  if ( parameters.value( u"IDENTIFIER"_s ).isValid() )
  {
    const QString identifier = parameterAsString( parameters, u"IDENTIFIER"_s, context );
    if ( !identifier.isEmpty() || !ignoreEmpty )
    {
      md->setIdentifier( identifier );
    }
  }

  if ( parameters.value( u"PARENT_IDENTIFIER"_s ).isValid() )
  {
    const QString parentIdentifier = parameterAsString( parameters, u"PARENT_IDENTIFIER"_s, context );
    if ( !parentIdentifier.isEmpty() || !ignoreEmpty )
    {
      md->setParentIdentifier( parentIdentifier );
    }
  }

  if ( parameters.value( u"TITLE"_s ).isValid() )
  {
    const QString title = parameterAsString( parameters, u"TITLE"_s, context );
    if ( !title.isEmpty() || !ignoreEmpty )
    {
      md->setTitle( title );
    }
  }

  if ( parameters.value( u"TYPE"_s ).isValid() )
  {
    const QString type = parameterAsString( parameters, u"TYPE"_s, context );
    if ( !type.isEmpty() || !ignoreEmpty )
    {
      md->setType( type );
    }
  }

  if ( parameters.value( u"LANGUAGE"_s ).isValid() )
  {
    const QString language = parameterAsString( parameters, u"LANGUAGE"_s, context );
    if ( !language.isEmpty() || !ignoreEmpty )
    {
      md->setLanguage( language );
    }
  }

  if ( parameters.value( u"ENCODING"_s ).isValid() )
  {
    const QString encoding = parameterAsString( parameters, u"ENCODING"_s, context );
    if ( !encoding.isEmpty() || !ignoreEmpty )
    {
      md->setEncoding( encoding );
    }
  }

  if ( parameters.value( u"ABSTRACT"_s ).isValid() )
  {
    const QString abstract = parameterAsString( parameters, u"ABSTRACT"_s, context );
    if ( !abstract.isEmpty() || !ignoreEmpty )
    {
      md->setAbstract( abstract );
    }
  }

  if ( parameters.value( u"CRS"_s ).isValid() )
  {
    const QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, u"CRS"_s, context );
    if ( crs.isValid() || !ignoreEmpty )
    {
      md->setCrs( crs );
    }
  }

  if ( parameters.value( u"FEES"_s ).isValid() )
  {
    const QString fees = parameterAsString( parameters, u"FEES"_s, context );
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
  results.insert( u"OUTPUT"_s, mLayerId );
  return results;
}

///@endcond
