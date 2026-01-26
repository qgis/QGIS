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
#include "qgssettings.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"

#include <QLocale>

///@cond PRIVATE

QString QgsPackageAlgorithm::name() const
{
  return u"package"_s;
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
  return u"database"_s;
}

void QgsPackageAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( u"LAYERS"_s, QObject::tr( "Input layers" ), Qgis::ProcessingSourceType::Vector ) );
  QgsProcessingParameterFileDestination *outputParameter = new QgsProcessingParameterFileDestination( u"OUTPUT"_s, QObject::tr( "Destination GeoPackage" ), QObject::tr( "GeoPackage files (*.gpkg)" ) );
  outputParameter->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"dontconfirmoverwrite"_s, true } } ) } } ) );
  addParameter( outputParameter );
  addParameter( new QgsProcessingParameterBoolean( u"OVERWRITE"_s, QObject::tr( "Overwrite existing GeoPackage" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( u"SAVE_STYLES"_s, QObject::tr( "Save layer styles into GeoPackage" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"SAVE_METADATA"_s, QObject::tr( "Save layer metadata into GeoPackage" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"SELECTED_FEATURES_ONLY"_s, QObject::tr( "Save only selected features" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( u"EXPORT_RELATED_LAYERS"_s, QObject::tr( "Export related layers following relations defined in the project" ), false ) );
  auto extentParam = std::make_unique<QgsProcessingParameterExtent>( u"EXTENT"_s, QObject::tr( "Extent" ), QVariant(), true );
  extentParam->setHelp( QObject::tr( "Limit exported features to those with geometries intersecting the provided extent" ) );
  addParameter( extentParam.release() );

  auto crsParam = std::make_unique< QgsProcessingParameterCrs >( u"CRS"_s, QObject::tr( "Destination CRS" ), QVariant(), true );
  crsParam->setFlags( crsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  crsParam->setHelp( QObject::tr( "If set, all layers will be transformed to the destination CRS during packaging." ) );
  addParameter( std::move( crsParam ) );

  addOutput( new QgsProcessingOutputMultipleLayers( u"OUTPUT_LAYERS"_s, QObject::tr( "Layers within new package" ) ) );
}

QString QgsPackageAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm collects a number of existing layers and packages them together into a single GeoPackage database." );
}

QString QgsPackageAlgorithm::shortDescription() const
{
  return QObject::tr( "Packages a number of existing layers together into a single GeoPackage database." );
}

QgsPackageAlgorithm *QgsPackageAlgorithm::createInstance() const
{
  return new QgsPackageAlgorithm();
}

bool QgsPackageAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"LAYERS"_s, context );

  for ( const QgsMapLayer *layer : std::as_const( layers ) )
  {
    QgsMapLayer *clonedLayer { layer->clone() };
    mClonedLayerIds.insert( clonedLayer->id(), layer->id() );
    mLayers.emplace_back( clonedLayer );
  }

  if ( mLayers.empty() )
  {
    feedback->reportError( QObject::tr( "No layers selected, geopackage will be empty" ), false );
  }

  mDestinationCrs = parameterAsCrs( parameters, u"CRS"_s, context );

  return true;
}

QVariantMap QgsPackageAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const bool overwrite = parameterAsBoolean( parameters, u"OVERWRITE"_s, context );
  const bool saveStyles = parameterAsBoolean( parameters, u"SAVE_STYLES"_s, context );
  const bool saveMetadata = parameterAsBoolean( parameters, u"SAVE_METADATA"_s, context );
  const bool selectedFeaturesOnly = parameterAsBoolean( parameters, u"SELECTED_FEATURES_ONLY"_s, context );
  const bool exportRelatedLayers = parameterAsBoolean( parameters, u"EXPORT_RELATED_LAYERS"_s, context );
  const QString packagePath = parameterAsString( parameters, u"OUTPUT"_s, context );
  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"LAYERS"_s, context );

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
    if ( project && !project->relationManager()->relations().isEmpty() )
    {
      // Infinite recursion should not happen in the real world but better safe than sorry
      const int maxRecursion { 10 };
      int recursionGuard { 0 };

      // This function recursively finds referenced layers
      const auto findReferenced = [this, &project, &feedback, &recursionGuard, &layers]( const QgsVectorLayer *vLayer, bool onlySaveSelected, auto &&findReferenced ) -> void {
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
            if ( !layers.contains( qobject_cast<QgsMapLayer *>( referencedLayer ) ) || referencedLayer->selectedFeatureCount() > 0 )
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
      const auto findReferencing = [this, &project, &feedback, &recursionGuard, &layers]( const QgsVectorLayer *vLayer, bool onlySaveSelected, auto &&findReferencing ) -> void {
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

          if ( onlySaveSelected && ( !layerWasExplicitlyAdded || referencingLayer->selectedFeatureCount() > 0 ) )
          {
            if ( !layers.contains( qobject_cast<QgsMapLayer *>( referencingLayer ) ) || referencingLayer->selectedFeatureCount() > 0 )
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

          if ( !alreadyAdded && ( !onlySaveSelected || !selected.isEmpty() ) )
          {
            feedback->pushInfo( QObject::tr( "Adding referencing layer '%1'" ).arg( referencingLayer->name() ) );
            relatedLayer = referencingLayer->clone();
            mLayers.emplace_back( relatedLayer );
            mClonedLayerIds.insert( relatedLayer->id(), referencingLayer->id() );
          }

          if ( relatedLayer && !selected.isEmpty() )
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
            findReferencing( relatedLayer, onlySaveSelected, findReferencing );
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

  const bool validExtent = parameters.value( u"EXTENT"_s ).isValid();

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

    QgsRectangle extent;

    switch ( layer->type() )
    {
      case Qgis::LayerType::Vector:
      {
        QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer.get() );

        if ( validExtent )
        {
          if ( vectorLayer->hasSpatialIndex() == Qgis::SpatialIndexPresence::NotPresent )
          {
            feedback->pushWarning( QObject::tr( "No spatial index exists for layer %1, performance will be severely degraded" ).arg( vectorLayer->name() ) );
          }

          extent = parameterAsExtent( parameters, u"EXTENT"_s, context, mDestinationCrs.isValid() ? mDestinationCrs : layer->crs() );
        }

        if ( !packageVectorLayer( vectorLayer, packagePath, context, &multiStepFeedback, saveStyles, saveMetadata, selectedFeaturesOnly, extent ) )
          errored = true;
        else
          outputLayers.append( u"%1|layername=%2"_s.arg( packagePath, layer->name() ) );
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
  outputs.insert( u"OUTPUT"_s, packagePath );
  outputs.insert( u"OUTPUT_LAYERS"_s, outputLayers );
  return outputs;
}

bool QgsPackageAlgorithm::packageVectorLayer( QgsVectorLayer *layer, const QString &path, QgsProcessingContext &context, QgsProcessingFeedback *feedback, bool saveStyles, bool saveMetadata, bool selectedFeaturesOnly, const QgsRectangle &extent )
{
  QgsVectorFileWriter::SaveVectorOptions options;
  options.driverName = u"GPKG"_s;
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

  if ( mDestinationCrs.isValid() )
  {
    options.ct = QgsCoordinateTransform( layer->crs(), mDestinationCrs, context.transformContext() );
  }

  // Check FID compatibility with GPKG and remove any existing FID field if not compatible,
  // let this be completely recreated since many layer sources have fid fields which are
  // not compatible with gpkg requirements
  const QgsFields fields = layer->fields();
  const int fidIndex = fields.lookupField( u"fid"_s );

  options.attributes = fields.allAttributesList();
  if ( fidIndex >= 0 )
  {
    const QMetaType::Type fidType { layer->fields().field( fidIndex ).type() };
    if ( !layer->fieldConstraints( fidIndex ).testFlag( QgsFieldConstraints::Constraint::ConstraintUnique )
         && !layer->fieldConstraints( fidIndex ).testFlag( QgsFieldConstraints::Constraint::ConstraintNotNull )
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

  if ( !extent.isNull() )
  {
    options.filterExtent = extent;
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
      auto res = std::make_unique<QgsVectorLayer>( u"%1|layername=%2"_s.arg( newFilename, newLayer ) );
      if ( res->isValid() )
      {
        QString errorMsg;
        QDomDocument doc( u"qgis"_s );
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
            const QVariant prevOverwriteStyle = settings.value( u"qgis/overwriteStyle"_s );
            settings.setValue( u"qgis/overwriteStyle"_s, true );
            QgsMapLayer::SaveStyleResults saveStyleResults = res->saveStyleToDatabaseV2( newLayer, QString(), true, QString(), errorMsg );
            settings.setValue( u"qgis/overwriteStyle"_s, prevOverwriteStyle );
            if ( saveStyleResults.testFlag( QgsMapLayer::SaveStyleResult::QmlGenerationFailed )
                 || saveStyleResults.testFlag( QgsMapLayer::SaveStyleResult::DatabaseWriteFailed ) )
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
