/***************************************************************************
                         qgsalgorithmpackage.cpp
                         ---------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmpackage.h"
#include "qgsgeometryengine.h"
#include "qgsogrutils.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"

///@cond PRIVATE

QString QgsPackageAlgorithm::name() const
{
  return QStringLiteral( "package" );
}

QString QgsPackageAlgorithm::displayName() const
{
  return QObject::tr( "Package layers" );
}

QStringList QgsPackageAlgorithm::tags() const
{
  return QObject::tr( "geopackage,collect,merge,combine,styles" ).split( ',' );
}

QString QgsPackageAlgorithm::group() const
{
  return QObject::tr( "Database" );
}

QString QgsPackageAlgorithm::groupId() const
{
  return QStringLiteral( "database" );
}

void QgsPackageAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ), QgsProcessing::TypeVector ) );
  QgsProcessingParameterFileDestination *outputParameter = new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Destination GeoPackage" ), QObject::tr( "GeoPackage files (*.gpkg)" ) );
  outputParameter->setMetadata( QVariantMap( {{QStringLiteral( "widget_wrapper" ), QVariantMap( {{QStringLiteral( "dontconfirmoverwrite" ), true }} ) }} ) );
  addParameter( outputParameter );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "OVERWRITE" ), QObject::tr( "Overwrite existing GeoPackage" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "SAVE_STYLES" ), QObject::tr( "Save layer styles into GeoPackage" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "SAVE_METADATA" ), QObject::tr( "Save layer metadata into GeoPackage" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "SELECTED_FEATURES_ONLY" ), QObject::tr( "Save only selected features" ), false ) );
  addOutput( new QgsProcessingOutputMultipleLayers( QStringLiteral( "OUTPUT_LAYERS" ), QObject::tr( "Layers within new package" ) ) );
}

QString QgsPackageAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm collects a number of existing layers and packages them together into a single GeoPackage database." );
}

QgsPackageAlgorithm *QgsPackageAlgorithm::createInstance() const
{
  return new QgsPackageAlgorithm();
}

bool QgsPackageAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context );
  for ( QgsMapLayer *layer : layers )
  {
    mLayers.emplace_back( layer->clone() );
  }

  if ( mLayers.empty() )
    feedback->reportError( QObject::tr( "No layers selected, geopackage will be empty" ), false );

  return true;
}

QVariantMap QgsPackageAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const bool overwrite = parameterAsBoolean( parameters, QStringLiteral( "OVERWRITE" ), context );
  const bool saveStyles = parameterAsBoolean( parameters, QStringLiteral( "SAVE_STYLES" ), context );
  const bool saveMetadata = parameterAsBoolean( parameters, QStringLiteral( "SAVE_METADATA" ), context );
  const bool selectedFeaturesOnly = parameterAsBoolean( parameters, QStringLiteral( "SELECTED_FEATURES_ONLY" ), context );
  const QString packagePath = parameterAsString( parameters, QStringLiteral( "OUTPUT" ), context );
  if ( packagePath.isEmpty() )
    throw QgsProcessingException( QObject::tr( "No output file specified." ) );

  // delete existing geopackage if it exists
  if ( overwrite && QFile::exists( packagePath ) )
  {
    feedback->pushInfo( QObject::tr( "Removing existing file '%1'" ).arg( packagePath ) );
    if ( !QFile( packagePath ).remove() )
    {
      throw QgsProcessingException( QObject::tr( "Could not remove existing file '%1'" ).arg( packagePath ) );
    }
  }

  OGRSFDriverH hGpkgDriver = OGRGetDriverByName( "GPKG" );
  if ( !hGpkgDriver )
  {
    throw QgsProcessingException( QObject::tr( "GeoPackage driver not found." ) );
  }

  gdal::ogr_datasource_unique_ptr hDS;

  if ( !QFile::exists( packagePath ) )
  {
    hDS = gdal::ogr_datasource_unique_ptr( OGR_Dr_CreateDataSource( hGpkgDriver, packagePath.toUtf8().constData(), nullptr ) );
    if ( !hDS )
      throw QgsProcessingException( QObject::tr( "Creation of database %1 failed (OGR error: %2)" ).arg( packagePath, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
  }
  else
  {
    hDS = gdal::ogr_datasource_unique_ptr( OGROpen( packagePath.toUtf8().constData(), true, nullptr ) );
    if ( !hDS )
      throw QgsProcessingException( QObject::tr( "Opening database %1 failed (OGR error: %2)" ).arg( packagePath, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
  }


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

    feedback->pushInfo( QObject::tr( "Packaging layer %1/%2: %3" ).arg( i ).arg( mLayers.size() ).arg( layer ? layer->name() : QString() ) );

    switch ( layer->type() )
    {
      case QgsMapLayerType::VectorLayer:
      {
        QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer.get() );
        const bool onlySaveSelected = vectorLayer->selectedFeatureCount() > 0 && selectedFeaturesOnly;
        if ( !packageVectorLayer( vectorLayer, packagePath, context, &multiStepFeedback, saveStyles, saveMetadata, onlySaveSelected ) )
          errored = true;
        else
          outputLayers.append( QStringLiteral( "%1|layername=%2" ).arg( packagePath, layer->name() ) );
        break;
      }

      case QgsMapLayerType::RasterLayer:
      {
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging raster layers is not supported." ) );
        errored = true;
        break;
      }

      case QgsMapLayerType::PluginLayer:
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging plugin layers is not supported." ) );
        errored = true;
        break;

      case QgsMapLayerType::MeshLayer:
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging mesh layers is not supported." ) );
        errored = true;
        break;

      case QgsMapLayerType::PointCloudLayer:
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging point cloud layers is not supported." ) );
        errored = true;
        break;

      case QgsMapLayerType::VectorTileLayer:
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging vector tile layers is not supported." ) );
        errored = true;
        break;

      case QgsMapLayerType::AnnotationLayer:
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging annotation layers is not supported." ) );
        errored = true;
        break;

      case QgsMapLayerType::GroupLayer:
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging group layers is not supported." ) );
        errored = true;
        break;
    }
  }

  if ( errored )
    throw QgsProcessingException( QObject::tr( "Error obtained while packaging one or more layers." ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), packagePath );
  outputs.insert( QStringLiteral( "OUTPUT_LAYERS" ), outputLayers );
  return outputs;
}

bool QgsPackageAlgorithm::packageVectorLayer( QgsVectorLayer *layer, const QString &path, QgsProcessingContext &context,
    QgsProcessingFeedback *feedback, bool saveStyles, bool saveMetadata, bool selectedFeaturesOnly )
{
  QgsVectorFileWriter::SaveVectorOptions options;
  options.driverName = QStringLiteral( "GPKG" );
  options.layerName = layer->name();
  options.actionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteLayer;
  options.fileEncoding = context.defaultEncoding();
  options.onlySelectedFeatures = selectedFeaturesOnly;
  options.feedback = feedback;
  if ( saveMetadata )
  {
    options.layerMetadata = layer->metadata();
    options.saveMetadata = true;
  }

  // remove any existing FID field, let this be completely recreated
  // since many layer sources have fid fields which are not compatible with gpkg requirements
  const QgsFields fields = layer->fields();
  const int fidIndex = fields.lookupField( QStringLiteral( "fid" ) );

  options.attributes = fields.allAttributesList();
  if ( fidIndex >= 0 )
    options.attributes.removeAll( fidIndex );
  if ( options.attributes.isEmpty() )
  {
    // fid was the only field
    options.skipAttributeCreation = true;
  }

  QString error;
  QString newFilename;
  QString newLayer;
  if ( QgsVectorFileWriter::writeAsVectorFormatV3( layer, path, context.transformContext(), options, &error, &newFilename, &newLayer ) != QgsVectorFileWriter::NoError )
  {
    feedback->reportError( QObject::tr( "Packaging layer failed: %1" ).arg( error ) );
    return false;
  }
  else
  {
    if ( saveStyles )
    {
      std::unique_ptr< QgsVectorLayer > res = std::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layername=%2" ).arg( newFilename, newLayer ) );
      if ( res )
      {
        QString errorMsg;
        QDomDocument doc( QStringLiteral( "qgis" ) );
        const QgsReadWriteContext context;
        layer->exportNamedStyle( doc, errorMsg, context );
        if ( !errorMsg.isEmpty() )
        {
          feedback->reportError( QObject::tr( "Could not retrieve existing layer style: %1 " ).arg( errorMsg ) );
        }
        else
        {
          if ( !res->importNamedStyle( doc, errorMsg ) )
          {
            feedback->reportError( QObject::tr( "Could not set existing layer style: %1 " ).arg( errorMsg ) );
          }
          else
          {
            QgsSettings settings;
            // this is not nice -- but needed to avoid an "overwrite" prompt messagebox from the provider! This api needs a rework to avoid this.
            const QVariant prevOverwriteStyle = settings.value( QStringLiteral( "qgis/overwriteStyle" ) );
            settings.setValue( QStringLiteral( "qgis/overwriteStyle" ), true );
            res->saveStyleToDatabase( newLayer, QString(), true, QString(), errorMsg );
            settings.setValue( QStringLiteral( "qgis/overwriteStyle" ), prevOverwriteStyle );
            if ( !errorMsg.isEmpty() )
            {
              feedback->reportError( QObject::tr( "Could not save layer style: %1 " ).arg( errorMsg ) );
            }
          }
        }
      }
      else
      {
        feedback->reportError( QObject::tr( "Could not save layer style -- error loading: %1 %2" ).arg( newFilename, newLayer ) );
      }
    }
    return true;
  }
}

///@endcond
