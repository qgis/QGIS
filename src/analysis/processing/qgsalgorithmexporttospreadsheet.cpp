/***************************************************************************
                         qgsalgorithmexporttospreadsheet.cpp
                         ------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsalgorithmexporttospreadsheet.h"
#include "qgsogrutils.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsfieldformatterregistry.h"
#include "qgsfieldformatter.h"

///@cond PRIVATE

class FieldValueConverter : public QgsVectorFileWriter::FieldValueConverter
{
  public:
    FieldValueConverter( QgsVectorLayer *vl )
      : mLayer( vl )
    {
      const QStringList formattersAllowList{ QStringLiteral( "KeyValue" ),
                                             QStringLiteral( "List" ),
                                             QStringLiteral( "ValueRelation" ),
                                             QStringLiteral( "ValueMap" ) };

      for ( int i = 0; i < mLayer->fields().count(); ++i )
      {
        const QgsEditorWidgetSetup setup = mLayer->fields().at( i ).editorWidgetSetup();
        const QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
        if ( formattersAllowList.contains( fieldFormatter->id() ) )
        {
          mFormatters[i] = fieldFormatter;
          mConfig[i] = setup.config();
        }
      }
    }

    QgsField fieldDefinition( const QgsField &field ) override
    {
      if ( !mLayer )
        return field;

      const int idx = mLayer->fields().indexFromName( field.name() );
      if ( mFormatters.contains( idx ) )
      {
        return QgsField( field.name(), QVariant::String );
      }
      return field;
    }

    QVariant convert( int i, const QVariant &value ) override
    {
      const QgsFieldFormatter *formatter = mFormatters.value( i );
      if ( !formatter )
        return value;

      QVariant cache;
      if ( mCaches.contains( i ) )
      {
        cache = mCaches.value( i );
      }
      else
      {
        cache = formatter->createCache( mLayer.data(), i, mConfig.value( i ) );
        mCaches[ i ] = cache;
      }

      return formatter->representValue( mLayer.data(), i, mConfig.value( i ), cache, value );
    }

    FieldValueConverter *clone() const override
    {
      return new FieldValueConverter( *this );
    }

  private:
    QPointer< QgsVectorLayer > mLayer;
    QMap< int, const QgsFieldFormatter * > mFormatters;
    QMap< int, QVariantMap > mConfig;
    QMap< int, QVariant > mCaches;
};

QString QgsExportToSpreadsheetAlgorithm::name() const
{
  return QStringLiteral( "exporttospreadsheet" );
}

QString QgsExportToSpreadsheetAlgorithm::displayName() const
{
  return QObject::tr( "Export to spreadsheet" );
}

QStringList QgsExportToSpreadsheetAlgorithm::tags() const
{
  return QObject::tr( "microsoft,excel,xls,xlsx,calc,open,office,libre,ods" ).split( ',' );
}

QString QgsExportToSpreadsheetAlgorithm::group() const
{
  return QObject::tr( "Layer tools" );
}

QString QgsExportToSpreadsheetAlgorithm::groupId() const
{
  return QStringLiteral( "layertools" );
}

void QgsExportToSpreadsheetAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ), QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "USE_ALIAS" ), QObject::tr( "Use field aliases as column headings" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "FORMATTED_VALUES" ), QObject::tr( "Export formatted values instead of raw values" ), false ) );
  QgsProcessingParameterFileDestination *outputParameter = new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Destination spreadsheet" ), QObject::tr( "Microsoft Excel (*.xlsx);;Open Document Spreadsheet (*.ods)" ) );
  outputParameter->setMetadata( QVariantMap( {{QStringLiteral( "widget_wrapper" ), QVariantMap( {{QStringLiteral( "dontconfirmoverwrite" ), true }} ) }} ) );
  addParameter( outputParameter );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "OVERWRITE" ), QObject::tr( "Overwrite existing spreadsheet" ), true ) );
  addOutput( new QgsProcessingOutputMultipleLayers( QStringLiteral( "OUTPUT_LAYERS" ), QObject::tr( "Layers within spreadsheet" ) ) );
}

QString QgsExportToSpreadsheetAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm collects a number of existing layers and exports them into a spreadsheet document.\n\n"
                      "Optionally the layers can be appended to an existing spreadsheet as additional sheets.\n\n" );
}

QgsExportToSpreadsheetAlgorithm *QgsExportToSpreadsheetAlgorithm::createInstance() const
{
  return new QgsExportToSpreadsheetAlgorithm();
}

bool QgsExportToSpreadsheetAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context );
  for ( QgsMapLayer *layer : layers )
  {
    mLayers.emplace_back( layer->clone() );
  }

  if ( mLayers.empty() )
    feedback->reportError( QObject::tr( "No layers selected, spreadsheet will be empty" ), false );

  return true;
}

QVariantMap QgsExportToSpreadsheetAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const bool overwrite = parameterAsBoolean( parameters, QStringLiteral( "OVERWRITE" ), context );
  const QString outputPath = parameterAsString( parameters, QStringLiteral( "OUTPUT" ), context );
  if ( outputPath.isEmpty() )
    throw QgsProcessingException( QObject::tr( "No output file specified." ) );

  const bool useAlias = parameterAsBoolean( parameters, QStringLiteral( "USE_ALIAS" ), context );
  const bool formattedValues = parameterAsBoolean( parameters, QStringLiteral( "FORMATTED_VALUES" ), context );
  bool createNew = true;
  // delete existing spreadsheet if it exists
  if ( overwrite && QFile::exists( outputPath ) )
  {
    feedback->pushInfo( QObject::tr( "Removing existing file '%1'" ).arg( outputPath ) );
    if ( !QFile( outputPath ).remove() )
    {
      throw QgsProcessingException( QObject::tr( "Could not remove existing file '%1'" ).arg( outputPath ) );
    }
  }
  else if ( QFile::exists( outputPath ) )
  {
    createNew = false;
  }

  const QFileInfo fi( outputPath );
  const QString driverName = QgsVectorFileWriter::driverForExtension( fi.suffix() );

  OGRSFDriverH hDriver = OGRGetDriverByName( driverName.toLocal8Bit().constData() );
  if ( !hDriver )
  {
    if ( driverName == QLatin1String( "ods" ) )
      throw QgsProcessingException( QObject::tr( "Open Document Spreadsheet driver not found." ) );
    else
      throw QgsProcessingException( QObject::tr( "Microsoft Excel driver not found." ) );
  }

  const gdal::ogr_datasource_unique_ptr hDS;
#if 0
  if ( !QFile::exists( outputPath ) )
  {
    hDS = gdal::ogr_datasource_unique_ptr( OGR_Dr_CreateDataSource( hDriver, outputPath.toUtf8().constData(), nullptr ) );
    if ( !hDS )
      throw QgsProcessingException( QObject::tr( "Creation of spreadsheet %1 failed (OGR error: %2)" ).arg( outputPath, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
  }
#endif
  bool errored = false;

  QgsProcessingMultiStepFeedback multiStepFeedback( mLayers.size(), feedback );

  QStringList outputLayers;
  int i = 0;
  for ( const auto &layer : mLayers )
  {
    if ( feedback->isCanceled() )
      break;

    multiStepFeedback.setCurrentStep( i );
    i++;

    if ( !layer )
    {
      // don't throw immediately - instead do what we can and error out later
      feedback->pushDebugInfo( QObject::tr( "Error retrieving map layer." ) );
      errored = true;
      continue;
    }

    feedback->pushInfo( QObject::tr( "Exporting layer %1/%2: %3" ).arg( i ).arg( mLayers.size() ).arg( layer ? layer->name() : QString() ) );

    FieldValueConverter converter( qobject_cast< QgsVectorLayer * >( layer.get() ) );

    if ( !exportVectorLayer( qobject_cast< QgsVectorLayer * >( layer.get() ), outputPath,
                             context, &multiStepFeedback, driverName, createNew, useAlias, formattedValues ? &converter : nullptr ) )
      errored = true;
    else
    {
      outputLayers.append( QStringLiteral( "%1|layername=%2" ).arg( outputPath, layer->name() ) );
      createNew = false;
    }
  }

  if ( errored )
    throw QgsProcessingException( QObject::tr( "Error obtained while exporting one or more layers." ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputPath );
  outputs.insert( QStringLiteral( "OUTPUT_LAYERS" ), outputLayers );
  return outputs;
}

bool QgsExportToSpreadsheetAlgorithm::exportVectorLayer( QgsVectorLayer *layer, const QString &path, QgsProcessingContext &context,
    QgsProcessingFeedback *feedback, const QString &driverName, bool createNew, bool preferAlias, QgsVectorFileWriter::FieldValueConverter *converter )
{
  QgsVectorFileWriter::SaveVectorOptions options;
  options.driverName = driverName;
  options.layerName = layer->name();
  options.actionOnExistingFile = createNew ? QgsVectorFileWriter::CreateOrOverwriteFile : QgsVectorFileWriter::CreateOrOverwriteLayer;
  options.fileEncoding = context.defaultEncoding();
  options.feedback = feedback;
  options.fieldNameSource = preferAlias ? QgsVectorFileWriter::PreferAlias : QgsVectorFileWriter::Original;
  options.fieldValueConverter = converter;


  QString error;
  QString newFilename;
  QString newLayer;
  if ( QgsVectorFileWriter::writeAsVectorFormatV3( layer, path, context.transformContext(), options, &error, &newFilename, &newLayer ) != QgsVectorFileWriter::NoError )
  {
    feedback->reportError( QObject::tr( "Exporting layer failed: %1" ).arg( error ) );
    return false;
  }
  else
  {
    return true;
  }
}

///@endcond
