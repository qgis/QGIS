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
#include "qgsogrutils.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"

#include <QLocale>

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
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::Vector ) );
  QgsProcessingParameterFileDestination *outputParameter = new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Destination GeoPackage" ), QObject::tr( "GeoPackage files (*.gpkg)" ) );
  outputParameter->setMetadata( QVariantMap( {{QStringLiteral( "widget_wrapper" ), QVariantMap( {{QStringLiteral( "dontconfirmoverwrite" ), true }} ) }} ) );
  addParameter( outputParameter );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "OVERWRITE" ), QObject::tr( "Overwrite existing GeoPackage" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "SAVE_STYLES" ), QObject::tr( "Save layer styles into GeoPackage" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "SAVE_METADATA" ), QObject::tr( "Save layer metadata into GeoPackage" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "SELECTED_FEATURES_ONLY" ), QObject::tr( "Save only selected features" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "EXPORT_RELATED_LAYERS" ), QObject::tr( "Export related layers following relations defined in the project" ), false ) );
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

  for ( const QgsMapLayer *layer : std::as_const( layers ) )
  {
    QgsMapLayer *clonedLayer { layer->clone() };
    mClonedLayerIds.insert( clonedLayer->id(), layer->id( ) );
    mLayers.emplace_back( clonedLayer );
  }

  if ( mLayers.empty() )
  {
    feedback->reportError( QObject::tr( "No layers selected, geopackage will be empty" ), false );
  }

  return true;
}

QVariantMap QgsPackageAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const bool overwrite = parameterAsBoolean( parameters, QStringLiteral( "OVERWRITE" ), context );
  const bool saveStyles = parameterAsBoolean( parameters, QStringLiteral( "SAVE_STYLES" ), context );
  const bool saveMetadata = parameterAsBoolean( parameters, QStringLiteral( "SAVE_METADATA" ), context );
  const bool selectedFeaturesOnly = parameterAsBoolean( parameters, QStringLiteral( "SELECTED_FEATURES_ONLY" ), context );
  const bool exportRelatedLayers = parameterAsBoolean( parameters, QStringLiteral( "EXPORT_RELATED_LAYERS" ), context );
  const QString packagePath = parameterAsString( parameters, QStringLiteral( "OUTPUT" ), context );
  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context );

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


  // Collect related layers from project relations
  if ( exportRelatedLayers )
  {
    const QgsProject *project { context.project() };
    if ( project && ! project->relationManager()->relations().isEmpty() )
    {

      // Infinite recursion should not happen in the real world but better safe than sorry
      const int maxRecursion { 10 };
      int recursionGuard { 0 };

      // This function recursively finds referenced layers
      const auto findReferenced = [ =, &project, &feedback, &recursionGuard, &layers ]( const QgsVectorLayer * vLayer, bool onlySaveSelected, auto &&findReferenced ) -> void
      {
        const QgsVectorLayer *originalLayer { qobject_cast<QgsVectorLayer *>( project->mapLayer( mClonedLayerIds.value( vLayer->id(), vLayer->id() ) ) ) };
        Q_ASSERT( originalLayer );
        const QList<QgsRelation> relations { project->relationManager()->referencingRelations( originalLayer ) };
        for ( const QgsRelation &relation : std::as_const( relations ) )
        {

          QgsVectorLayer *referencedLayer { relation.referencedLayer() };
          QgsVectorLayer *relatedLayer { nullptr };

          // Check if the layer was already in the export list
          bool alreadyAdded { false };
          for ( const auto &layerToExport : std::as_const( mLayers ) )
          {
            const QString originalId { mClonedLayerIds.value( layerToExport->id() ) };
            if ( originalId == referencedLayer->id() )
            {
              relatedLayer = qobject_cast<QgsVectorLayer *>( layerToExport.get() );
              alreadyAdded = true;
              break;
            }
          }

          if ( !alreadyAdded )
          {
            feedback->pushInfo( QObject::tr( "Adding referenced layer '%1'" ).arg( referencedLayer->name() ) );
            relatedLayer = referencedLayer->clone();
            mLayers.emplace_back( relatedLayer );
            mClonedLayerIds.insert( relatedLayer->id(), referencedLayer->id() );
          }

          // Export only relevant features unless the layer was in the original export list and no selection was made on that layer
          // in that case the user explicitly marked the layer for export and it is supposed to be exported fully.
          if ( onlySaveSelected )
          {
            if ( ! layers.contains( qobject_cast<QgsMapLayer *>( referencedLayer ) ) || referencedLayer->selectedFeatureCount() > 0 )
            {
              Q_ASSERT( relatedLayer );
              QgsFeatureIds selected;
              QgsFeatureIterator it { vLayer->getSelectedFeatures() };
              QgsFeature selectedFeature;
              while ( it.nextFeature( selectedFeature ) )
              {
                QgsFeature referencedFeature { relation.getReferencedFeature( selectedFeature ) };
                if ( referencedFeature.isValid() )
                {
                  selected.insert( referencedFeature.id() );
                }
              }
              relatedLayer->selectByIds( selected, Qgis::SelectBehavior::AddToSelection );
            }
          }

          // Recursive finding
          if ( recursionGuard > maxRecursion )
          {
            feedback->pushWarning( QObject::tr( "Max recursion (%1) adding referenced layer '%2', layer was not added" ).arg( QLocale().toString( recursionGuard ), referencedLayer->name() ) );
          }
          else
          {
            recursionGuard++;
            findReferenced( relatedLayer, onlySaveSelected, findReferenced );
          }
        }

      };

      // This function recursively finds referencing layers
      const auto findReferencing = [ =, &project, &feedback, &recursionGuard, &layers ]( const QgsVectorLayer * vLayer, bool onlySaveSelected, auto &&findReferencing ) -> void
      {
        const QgsVectorLayer *originalLayer { qobject_cast<QgsVectorLayer *>( project->mapLayer( mClonedLayerIds.value( vLayer->id(), vLayer->id() ) ) ) };
        Q_ASSERT( originalLayer );
        const QList<QgsRelation> relations { project->relationManager()->referencedRelations( originalLayer ) };
        for ( const QgsRelation &relation : std::as_const( relations ) )
        {

          QgsVectorLayer *referencingLayer { relation.referencingLayer() };
          QgsVectorLayer *relatedLayer { nullptr };
          const bool layerWasExplicitlyAdded { layers.contains( qobject_cast<QgsMapLayer *>( referencingLayer ) ) };

          // Check if the layer was already in the export list
          bool alreadyAdded { false };
          for ( const auto &layerToExport : std::as_const( mLayers ) )
          {
            const QString originalId { mClonedLayerIds.value( layerToExport->id() ) };
            if ( originalId == referencingLayer->id() )
            {
              relatedLayer = qobject_cast<QgsVectorLayer *>( layerToExport.get() );
              alreadyAdded = true;
              break;
            }
          }

          // Export only relevant features unless the layer was in the original export list and no selection was made on that layer
          // in that case the user explicitly marked the layer for export and it is supposed to be exported fully.
          QgsFeatureIds selected;

          if ( onlySaveSelected && ( ! layerWasExplicitlyAdded || referencingLayer->selectedFeatureCount() > 0 ) )
          {
            if ( ! layers.contains( qobject_cast<QgsMapLayer *>( referencingLayer ) ) || referencingLayer->selectedFeatureCount() > 0 )
            {
              QgsFeatureIterator it { vLayer->getSelectedFeatures() };
              QgsFeature selectedFeature;
              while ( it.nextFeature( selectedFeature ) )
              {
                QgsFeatureIterator referencingFeaturesIterator { relation.getRelatedFeatures( selectedFeature ) };
                QgsFeature referencingFeature;
                while ( referencingFeaturesIterator.nextFeature( referencingFeature ) )
                {
                  if ( referencingFeature.isValid() )
                  {
                    selected.insert( referencingFeature.id() );
                  }
                }
              }
            }
          }

          if ( ! alreadyAdded && ( ! onlySaveSelected || ! selected.isEmpty() ) )
          {
            feedback->pushInfo( QObject::tr( "Adding referencing layer '%1'" ).arg( referencingLayer->name() ) );
            relatedLayer = referencingLayer->clone();
            mLayers.emplace_back( relatedLayer );
            mClonedLayerIds.insert( relatedLayer->id(), referencingLayer->id() );
          }

          if ( relatedLayer && ! selected.isEmpty() )
          {
            relatedLayer->selectByIds( selected, Qgis::SelectBehavior::AddToSelection );
          }

          // Recursive finding
          if ( recursionGuard > maxRecursion )
          {
            feedback->pushWarning( QObject::tr( "Max recursion (%1) adding referencing layer '%2', layer was not added" ).arg( QLocale().toString( recursionGuard ), referencingLayer->name() ) );
          }
          else if ( relatedLayer )
          {
            recursionGuard++;
            findReferencing( relatedLayer, onlySaveSelected, findReferencing ) ;
          }
        }

      };

      for ( const QgsMapLayer *layer : std::as_const( layers ) )
      {
        const QgsVectorLayer *vLayer { qobject_cast<const QgsVectorLayer *>( layer ) };
        if ( vLayer )
        {
          const bool onlySaveSelected = vLayer->selectedFeatureCount() > 0 && selectedFeaturesOnly;
          recursionGuard = 0;
          findReferenced( vLayer, onlySaveSelected, findReferenced );
          recursionGuard = 0;
          findReferencing( vLayer, onlySaveSelected, findReferencing );
        }
      }

    }
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
      case Qgis::LayerType::Vector:
      {
        QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer.get() );
        if ( !packageVectorLayer( vectorLayer, packagePath, context, &multiStepFeedback, saveStyles, saveMetadata, selectedFeaturesOnly ) )
          errored = true;
        else
          outputLayers.append( QStringLiteral( "%1|layername=%2" ).arg( packagePath, layer->name() ) );
        break;
      }

      case Qgis::LayerType::Raster:
      {
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging raster layers is not supported." ) );
        errored = true;
        break;
      }

      case Qgis::LayerType::Plugin:
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging plugin layers is not supported." ) );
        errored = true;
        break;

      case Qgis::LayerType::Mesh:
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging mesh layers is not supported." ) );
        errored = true;
        break;

      case Qgis::LayerType::PointCloud:
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging point cloud layers is not supported." ) );
        errored = true;
        break;

      case Qgis::LayerType::VectorTile:
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging vector tile layers is not supported." ) );
        errored = true;
        break;

      case Qgis::LayerType::Annotation:
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging annotation layers is not supported." ) );
        errored = true;
        break;

      case Qgis::LayerType::Group:
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging group layers is not supported." ) );
        errored = true;
        break;

      case Qgis::LayerType::TiledScene:
        //not supported
        feedback->pushDebugInfo( QObject::tr( "Packaging tiled scene layers is not supported." ) );
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

  // Check FID compatibility with GPKG and remove any existing FID field if not compatible,
  // let this be completely recreated since many layer sources have fid fields which are
  // not compatible with gpkg requirements
  const QgsFields fields = layer->fields();
  const int fidIndex = fields.lookupField( QStringLiteral( "fid" ) );

  options.attributes = fields.allAttributesList();
  if ( fidIndex >= 0 )
  {
    const QMetaType::Type fidType { layer->fields().field( fidIndex ).type() };
    if ( ! layer->fieldConstraints( fidIndex ).testFlag( QgsFieldConstraints::Constraint::ConstraintUnique )
         && ! layer->fieldConstraints( fidIndex ).testFlag( QgsFieldConstraints::Constraint::ConstraintNotNull )
         && fidType != QMetaType::Type::Int
         && fidType != QMetaType::Type::UInt
         && fidType != QMetaType::Type::LongLong
         && fidType != QMetaType::Type::ULongLong )
    {
      options.attributes.removeAll( fidIndex );
    }
  }

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
