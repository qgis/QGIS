/***************************************************************************
                         qgsalgorithmsavefeatures.cpp
                         ---------------------
    begin                : July 2020
    copyright            : (C) 2020 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmsavefeatures.h"

#include "qgsvectorfilewriter.h"

#include <QRegularExpression>

///@cond PRIVATE

QString QgsSaveFeaturesAlgorithm::name() const
{
  return u"savefeatures"_s;
}

QString QgsSaveFeaturesAlgorithm::displayName() const
{
  return QObject::tr( "Save vector features to file" );
}

QStringList QgsSaveFeaturesAlgorithm::tags() const
{
  return QObject::tr( "save,write,export" ).split( ',' );
}

QString QgsSaveFeaturesAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsSaveFeaturesAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

QString QgsSaveFeaturesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm saves vector features to a specified file dataset.\n\n"
                      "For dataset formats supporting layers, an optional layer name parameter can be used to specify a custom string.\n\n"
                      "Optional GDAL-defined dataset and layer options can be specified. For more information on this, "
                      "read the online GDAL documentation." );
}

QString QgsSaveFeaturesAlgorithm::shortDescription() const
{
  return QObject::tr( "Saves vector features to a specified file dataset." );
}

QgsSaveFeaturesAlgorithm *QgsSaveFeaturesAlgorithm::createInstance() const
{
  return new QgsSaveFeaturesAlgorithm();
}

void QgsSaveFeaturesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Vector features" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterFileDestination( u"OUTPUT"_s, QObject::tr( "Saved features" ), QgsVectorFileWriter::fileFilterString(), QVariant(), false ) );

  auto param = std::make_unique<QgsProcessingParameterString>( u"LAYER_NAME"_s, QObject::tr( "Layer name" ), QVariant(), false, true );
  param->setFlags( param->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( param.release() );
  param = std::make_unique<QgsProcessingParameterString>( u"DATASOURCE_OPTIONS"_s, QObject::tr( "GDAL dataset options (separate individual options with semicolons)" ), QVariant(), false, true );
  param->setFlags( param->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( param.release() );
  param = std::make_unique<QgsProcessingParameterString>( u"LAYER_OPTIONS"_s, QObject::tr( "GDAL layer options (separate individual options with semicolons)" ), QVariant(), false, true );
  param->setFlags( param->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( param.release() );

  auto paramEnum = std::make_unique<QgsProcessingParameterEnum>( u"ACTION_ON_EXISTING_FILE"_s, QObject::tr( "Action to take on pre-existing file" ), QStringList() << QObject::tr( "Create or overwrite file" ) << QObject::tr( "Create or overwrite layer" ) << QObject::tr( "Append features to existing layer, but do not create new fields" ) << QObject::tr( "Append features to existing layer, and create new fields if needed" ), false, 0 );
  paramEnum->setFlags( paramEnum->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( paramEnum.release() );

  addOutput( new QgsProcessingOutputString( u"FILE_PATH"_s, QObject::tr( "File name and path" ) ) );
  addOutput( new QgsProcessingOutputString( u"LAYER_NAME"_s, QObject::tr( "Layer name" ) ) );
}

QVariantMap QgsSaveFeaturesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  QString layerName = parameterAsString( parameters, u"LAYER_NAME"_s, context ).trimmed();
  QVariantMap createOptions;
  if ( !layerName.isEmpty() )
  {
    createOptions[u"layerName"_s] = layerName;
  }

  const QStringList datasourceOptions = parameterAsString( parameters, u"DATASOURCE_OPTIONS"_s, context ).trimmed().split( ';', Qt::SkipEmptyParts );
  const QStringList layerOptions = parameterAsString( parameters, u"LAYER_OPTIONS"_s, context ).trimmed().split( ';', Qt::SkipEmptyParts );

  QString destination = parameterAsString( parameters, u"OUTPUT"_s, context );
  const QString format = QgsVectorFileWriter::driverForExtension( QFileInfo( destination ).completeSuffix() );

  const QgsVectorFileWriter::ActionOnExistingFile actionOnExistingFile = static_cast<QgsVectorFileWriter::ActionOnExistingFile>( parameterAsInt( parameters, u"ACTION_ON_EXISTING_FILE"_s, context ) );

  QString finalFileName;
  QString finalLayerName;
  QgsVectorFileWriter::SaveVectorOptions saveOptions;
  saveOptions.fileEncoding = context.defaultEncoding().isEmpty() ? u"system"_s : context.defaultEncoding();
  saveOptions.layerName = layerName;
  saveOptions.driverName = format;
  saveOptions.datasourceOptions = datasourceOptions;
  saveOptions.layerOptions = layerOptions;
  saveOptions.symbologyExport = Qgis::FeatureSymbologyExport::NoSymbology;
  saveOptions.actionOnExistingFile = actionOnExistingFile;

  std::unique_ptr<QgsVectorFileWriter> writer( QgsVectorFileWriter::create( destination, source->fields(), source->wkbType(), source->sourceCrs(), context.transformContext(), saveOptions, QgsFeatureSink::SinkFlags(), &finalFileName, &finalLayerName ) );
  if ( writer->hasError() )
  {
    throw QgsProcessingException( QObject::tr( "Could not create layer %1: %2" ).arg( destination, writer->errorMessage() ) );
  }

  if ( QgsProcessingFeedback *feedback = context.feedback() )
  {
    for ( const QgsField &field : source->fields() )
    {
      if ( !field.alias().isEmpty() && !( writer->capabilities() & Qgis::VectorFileWriterCapability::FieldAliases ) )
        feedback->pushWarning( QObject::tr( "%1: Aliases are not supported by %2" ).arg( field.name(), writer->driverLongName() ) );
      if ( !field.alias().isEmpty() && !( writer->capabilities() & Qgis::VectorFileWriterCapability::FieldComments ) )
        feedback->pushWarning( QObject::tr( "%1: Comments are not supported by %2" ).arg( field.name(), writer->driverLongName() ) );
    }
  }

  destination = finalFileName;
  if ( !saveOptions.layerName.isEmpty() && !finalLayerName.isEmpty() )
    destination += u"|layername=%1"_s.arg( finalLayerName );

  std::unique_ptr<QgsFeatureSink> sink( new QgsProcessingFeatureSink( writer.release(), destination, context, true ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  const double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 1;
  long long i = 0;

  QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest(), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  QgsFeature feat;
  while ( features.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( i * step );

    if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
  }

  finalFileName = destination;
  finalLayerName.clear(); // value of final layer name will be extracted from the destination string
  const int separatorIndex = destination.indexOf( '|' );
  if ( separatorIndex > -1 )
  {
    const thread_local QRegularExpression layerNameRx( u"\\|layername=([^\\|]*)"_s );
    const QRegularExpressionMatch match = layerNameRx.match( destination );
    if ( match.hasMatch() )
    {
      finalLayerName = match.captured( 1 );
    }
    finalFileName = destination.mid( 0, separatorIndex );
  }

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, destination );
  outputs.insert( u"FILE_PATH"_s, finalFileName );
  outputs.insert( u"LAYER_NAME"_s, finalLayerName );
  return outputs;
}

///@endcond
