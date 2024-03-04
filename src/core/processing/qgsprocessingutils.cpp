/***************************************************************************
                         qgsprocessingutils.cpp
                         ------------------------
    begin                : April 2017
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

#include "qgsprocessingutils.h"
#include "qgsproject.h"
#include "qgsexception.h"
#include "qgsprocessingcontext.h"
#include "qgsvectorlayerexporter.h"
#include "qgsvectorfilewriter.h"
#include "qgsmemoryproviderutils.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingalgorithm.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsexpressioncontextscopegenerator.h"
#include "qgsfileutils.h"
#include "qgsvectorlayer.h"
#include "qgsproviderregistry.h"
#include "qgsmeshlayer.h"
#include "qgspluginlayer.h"
#include "qgsreferencedgeometry.h"
#include "qgsrasterfilewriter.h"
#include "qgsvectortilelayer.h"
#include "qgspointcloudlayer.h"
#include "qgsannotationlayer.h"
#include "qgstiledscenelayer.h"
#include <QRegularExpression>
#include <QTextCodec>
#include <QUuid>

QList<QgsRasterLayer *> QgsProcessingUtils::compatibleRasterLayers( QgsProject *project, bool sort )
{
  return compatibleMapLayers< QgsRasterLayer >( project, sort );
}

QList<QgsVectorLayer *> QgsProcessingUtils::compatibleVectorLayers( QgsProject *project, const QList<int> &geometryTypes, bool sort )
{
  if ( !project )
    return QList<QgsVectorLayer *>();

  QList<QgsVectorLayer *> layers;
  const auto vectorLayers = project->layers<QgsVectorLayer *>();
  for ( QgsVectorLayer *l :  vectorLayers )
  {
    if ( canUseLayer( l, geometryTypes ) )
      layers << l;
  }

  if ( sort )
  {
    std::sort( layers.begin(), layers.end(), []( const QgsVectorLayer * a, const QgsVectorLayer * b ) -> bool
    {
      return QString::localeAwareCompare( a->name(), b->name() ) < 0;
    } );
  }
  return layers;
}

QList<QgsMeshLayer *> QgsProcessingUtils::compatibleMeshLayers( QgsProject *project, bool sort )
{
  return compatibleMapLayers< QgsMeshLayer >( project, sort );
}

QList<QgsPluginLayer *> QgsProcessingUtils::compatiblePluginLayers( QgsProject *project, bool sort )
{
  return compatibleMapLayers< QgsPluginLayer >( project, sort );
}

QList<QgsPointCloudLayer *> QgsProcessingUtils::compatiblePointCloudLayers( QgsProject *project, bool sort )
{
  return compatibleMapLayers< QgsPointCloudLayer >( project, sort );
}

QList<QgsAnnotationLayer *> QgsProcessingUtils::compatibleAnnotationLayers( QgsProject *project, bool sort )
{
  // we have to defer sorting until we've added the main annotation layer too
  QList<QgsAnnotationLayer *> res = compatibleMapLayers< QgsAnnotationLayer >( project, false );
  if ( project )
    res.append( project->mainAnnotationLayer() );

  if ( sort )
  {
    std::sort( res.begin(), res.end(), []( const QgsAnnotationLayer * a, const QgsAnnotationLayer * b ) -> bool
    {
      return QString::localeAwareCompare( a->name(), b->name() ) < 0;
    } );
  }

  return res;
}

QList<QgsVectorTileLayer *> QgsProcessingUtils::compatibleVectorTileLayers( QgsProject *project, bool sort )
{
  return compatibleMapLayers< QgsVectorTileLayer >( project, sort );
}

QList<QgsTiledSceneLayer *> QgsProcessingUtils::compatibleTiledSceneLayers( QgsProject *project, bool sort )
{
  return compatibleMapLayers< QgsTiledSceneLayer >( project, sort );
}

template<typename T> QList<T *> QgsProcessingUtils::compatibleMapLayers( QgsProject *project, bool sort )
{
  if ( !project )
    return QList<T *>();

  QList<T *> layers;
  const auto projectLayers = project->layers<T *>();
  for ( T *l : projectLayers )
  {
    if ( canUseLayer( l ) )
      layers << l;
  }

  if ( sort )
  {
    std::sort( layers.begin(), layers.end(), []( const T * a, const T * b ) -> bool
    {
      return QString::localeAwareCompare( a->name(), b->name() ) < 0;
    } );
  }
  return layers;
}

QList<QgsMapLayer *> QgsProcessingUtils::compatibleLayers( QgsProject *project, bool sort )
{
  if ( !project )
    return QList<QgsMapLayer *>();

  QList<QgsMapLayer *> layers;

  const auto rasterLayers = compatibleMapLayers< QgsRasterLayer >( project, false );
  for ( QgsRasterLayer *rl : rasterLayers )
    layers << rl;

  const auto vectorLayers = compatibleVectorLayers( project, QList< int >(), false );
  for ( QgsVectorLayer *vl : vectorLayers )
    layers << vl;

  const auto meshLayers = compatibleMapLayers< QgsMeshLayer >( project, false );
  for ( QgsMeshLayer *ml : meshLayers )
    layers << ml;

  const auto pointCloudLayers = compatibleMapLayers< QgsPointCloudLayer >( project, false );
  for ( QgsPointCloudLayer *pcl : pointCloudLayers )
    layers << pcl;

  const auto annotationLayers = compatibleMapLayers< QgsAnnotationLayer >( project, false );
  for ( QgsAnnotationLayer *al : annotationLayers )
    layers << al;
  layers << project->mainAnnotationLayer();

  const auto vectorTileLayers = compatibleMapLayers< QgsVectorTileLayer >( project, false );
  for ( QgsVectorTileLayer *vtl : vectorTileLayers )
    layers << vtl;

  const auto tiledSceneLayers = compatibleMapLayers< QgsTiledSceneLayer >( project, false );
  for ( QgsTiledSceneLayer *tml : tiledSceneLayers )
    layers << tml;

  const auto pluginLayers = compatibleMapLayers< QgsPluginLayer >( project, false );
  for ( QgsPluginLayer *pl : pluginLayers )
    layers << pl;

  if ( sort )
  {
    std::sort( layers.begin(), layers.end(), []( const QgsMapLayer * a, const QgsMapLayer * b ) -> bool
    {
      return QString::localeAwareCompare( a->name(), b->name() ) < 0;
    } );
  }
  return layers;
}

QString QgsProcessingUtils::encodeProviderKeyAndUri( const QString &providerKey, const QString &uri )
{
  return QStringLiteral( "%1://%2" ).arg( providerKey, uri );
}

bool QgsProcessingUtils::decodeProviderKeyAndUri( const QString &string, QString &providerKey, QString &uri )
{
  const thread_local QRegularExpression re( QStringLiteral( "^(\\w+?):\\/\\/(.+)$" ) );
  const QRegularExpressionMatch match = re.match( string );
  if ( !match.hasMatch() )
    return false;

  providerKey = match.captured( 1 );
  uri = match.captured( 2 );

  // double check that provider is valid
  return QgsProviderRegistry::instance()->providerMetadata( providerKey );
}

QgsMapLayer *QgsProcessingUtils::mapLayerFromStore( const QString &string, QgsMapLayerStore *store, QgsProcessingUtils::LayerHint typeHint )
{
  if ( !store || string.isEmpty() )
    return nullptr;

  QList< QgsMapLayer * > layers = store->mapLayers().values();

  layers.erase( std::remove_if( layers.begin(), layers.end(), []( QgsMapLayer * layer )
  {
    switch ( layer->type() )
    {
      case Qgis::LayerType::Vector:
        return !canUseLayer( qobject_cast< QgsVectorLayer * >( layer ) );
      case Qgis::LayerType::Raster:
        return !canUseLayer( qobject_cast< QgsRasterLayer * >( layer ) );
      case Qgis::LayerType::Plugin:
      case Qgis::LayerType::Group:
        return true;
      case Qgis::LayerType::Mesh:
        return !canUseLayer( qobject_cast< QgsMeshLayer * >( layer ) );
      case Qgis::LayerType::VectorTile:
        return !canUseLayer( qobject_cast< QgsVectorTileLayer * >( layer ) );
      case Qgis::LayerType::TiledScene:
        return !canUseLayer( qobject_cast< QgsTiledSceneLayer * >( layer ) );
      case Qgis::LayerType::PointCloud:
        return !canUseLayer( qobject_cast< QgsPointCloudLayer * >( layer ) );
      case Qgis::LayerType::Annotation:
        return !canUseLayer( qobject_cast< QgsAnnotationLayer * >( layer ) );
    }
    return true;
  } ), layers.end() );

  auto isCompatibleType = [typeHint]( QgsMapLayer * l ) -> bool
  {
    switch ( typeHint )
    {
      case LayerHint::UnknownType:
        return true;

      case LayerHint::Vector:
        return l->type() == Qgis::LayerType::Vector;

      case LayerHint::Raster:
        return l->type() == Qgis::LayerType::Raster;

      case LayerHint::Mesh:
        return l->type() == Qgis::LayerType::Mesh;

      case LayerHint::PointCloud:
        return l->type() == Qgis::LayerType::PointCloud;

      case LayerHint::Annotation:
        return l->type() == Qgis::LayerType::Annotation;

      case LayerHint::VectorTile:
        return l->type() == Qgis::LayerType::VectorTile;

      case LayerHint::TiledScene:
        return l->type() == Qgis::LayerType::TiledScene;
    }
    return true;
  };

  for ( QgsMapLayer *l : std::as_const( layers ) )
  {
    if ( isCompatibleType( l ) && l->id() == string )
      return l;
  }
  for ( QgsMapLayer *l : std::as_const( layers ) )
  {
    if ( isCompatibleType( l ) && l->name() == string )
      return l;
  }
  for ( QgsMapLayer *l : std::as_const( layers ) )
  {
    if ( isCompatibleType( l ) && normalizeLayerSource( l->source() ) == normalizeLayerSource( string ) )
      return l;
  }
  return nullptr;
}

QgsMapLayer *QgsProcessingUtils::loadMapLayerFromString( const QString &string, const QgsCoordinateTransformContext &transformContext, LayerHint typeHint, QgsProcessing::LayerOptionsFlags flags )
{
  QString provider;
  QString uri;
  const bool useProvider = decodeProviderKeyAndUri( string, provider, uri );
  if ( !useProvider )
    uri = string;

  QString name;

  const QgsProviderMetadata *providerMetadata = useProvider ? QgsProviderRegistry::instance()->providerMetadata( provider ) : nullptr;
  if ( providerMetadata )
  {
    // use the uri parts to determine a suitable layer name
    const QVariantMap parts = providerMetadata->decodeUri( uri );
    const QString layerName = parts.value( QStringLiteral( "layerName" ) ).toString();

    if ( !layerName.isEmpty() )
    {
      name = layerName;
    }
    else if ( const QString path = parts.value( QStringLiteral( "path" ) ).toString(); !path.isEmpty() )
    {
      name = QFileInfo( path ).baseName();
    }
  }
  else
  {
    const QStringList components = uri.split( '|' );
    if ( components.isEmpty() )
      return nullptr;

    if ( QFileInfo fi( components.at( 0 ) ); fi.isFile() )
      name = fi.baseName();
    else
      name = QFileInfo( uri ).baseName();
  }

  if ( name.isEmpty() )
  {
    name = QgsDataSourceUri( uri ).table();
  }
  if ( name.isEmpty() )
  {
    name = uri;
  }

  QList< Qgis::LayerType > candidateTypes;
  switch ( typeHint )
  {
    case LayerHint::UnknownType:
    {
      if ( providerMetadata )
      {
        // refine the type hint based on what the provider supports
        candidateTypes = providerMetadata->supportedLayerTypes();
      }
      break;
    }
    case LayerHint::Vector:
      candidateTypes.append( Qgis::LayerType::Vector );
      break;
    case LayerHint::Raster:
      candidateTypes.append( Qgis::LayerType::Raster );
      break;
    case LayerHint::Mesh:
      candidateTypes.append( Qgis::LayerType::Mesh );
      break;
    case LayerHint::PointCloud:
      candidateTypes.append( Qgis::LayerType::PointCloud );
      break;
    case LayerHint::Annotation:
      candidateTypes.append( Qgis::LayerType::Annotation );
      break;
    case LayerHint::VectorTile:
      candidateTypes.append( Qgis::LayerType::VectorTile );
      break;
    case LayerHint::TiledScene:
      candidateTypes.append( Qgis::LayerType::TiledScene );
      break;
  }

  // brute force attempt to load a matching layer
  if ( candidateTypes.empty() || candidateTypes.contains( Qgis::LayerType::Vector ) )
  {
    QgsVectorLayer::LayerOptions options { transformContext };
    options.loadDefaultStyle = false;
    options.skipCrsValidation = true;

    std::unique_ptr< QgsVectorLayer > layer;
    if ( providerMetadata )
    {
      layer = std::make_unique<QgsVectorLayer>( uri, name, providerMetadata->key(), options );
    }
    else
    {
      // fallback to ogr
      layer = std::make_unique<QgsVectorLayer>( uri, name, QStringLiteral( "ogr" ), options );
    }
    if ( layer->isValid() )
    {
      return layer.release();
    }
  }
  if ( candidateTypes.empty() || candidateTypes.contains( Qgis::LayerType::Raster ) )
  {
    QgsRasterLayer::LayerOptions rasterOptions;
    rasterOptions.loadDefaultStyle = false;
    rasterOptions.skipCrsValidation = true;

    std::unique_ptr< QgsRasterLayer > rasterLayer;
    if ( providerMetadata )
    {
      rasterLayer = std::make_unique< QgsRasterLayer >( uri, name, providerMetadata->key(), rasterOptions );
    }
    else
    {
      // fallback to gdal
      rasterLayer = std::make_unique< QgsRasterLayer >( uri, name, QStringLiteral( "gdal" ), rasterOptions );
    }

    if ( rasterLayer->isValid() )
    {
      return rasterLayer.release();
    }
  }
  if ( candidateTypes.empty() || candidateTypes.contains( Qgis::LayerType::Mesh ) )
  {
    QgsMeshLayer::LayerOptions meshOptions;
    meshOptions.skipCrsValidation = true;

    std::unique_ptr< QgsMeshLayer > meshLayer;
    if ( providerMetadata )
    {
      meshLayer = std::make_unique< QgsMeshLayer >( uri, name, providerMetadata->key(), meshOptions );
    }
    else
    {
      meshLayer = std::make_unique< QgsMeshLayer >( uri, name, QStringLiteral( "mdal" ), meshOptions );
    }
    if ( meshLayer->isValid() )
    {
      return meshLayer.release();
    }
  }
  if ( candidateTypes.empty() || candidateTypes.contains( Qgis::LayerType::PointCloud ) )
  {
    QgsPointCloudLayer::LayerOptions pointCloudOptions;
    pointCloudOptions.skipCrsValidation = true;

    if ( flags & QgsProcessing::LayerOptionsFlag::SkipIndexGeneration )
    {
      pointCloudOptions.skipIndexGeneration = true;
    }

    std::unique_ptr< QgsPointCloudLayer > pointCloudLayer;
    if ( providerMetadata )
    {
      pointCloudLayer = std::make_unique< QgsPointCloudLayer >( uri, name, providerMetadata->key(), pointCloudOptions );
    }
    else
    {
      const QList< QgsProviderRegistry::ProviderCandidateDetails > preferredProviders = QgsProviderRegistry::instance()->preferredProvidersForUri( uri );
      if ( !preferredProviders.empty() )
      {
        pointCloudLayer = std::make_unique< QgsPointCloudLayer >( uri, name, preferredProviders.at( 0 ).metadata()->key(), pointCloudOptions );
      }
      else
      {
        // pdal provider can read ascii files but it is not exposed by the provider to
        // prevent automatic loading of tabular ascii files.
        // Try to open the file with pdal provider.
        QgsProviderMetadata *pdalProvider = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
        if ( pdalProvider )
        {
          pointCloudLayer = std::make_unique< QgsPointCloudLayer >( uri, name, QStringLiteral( "pdal" ), pointCloudOptions );
        }
      }
    }
    if ( pointCloudLayer && pointCloudLayer->isValid() )
    {
      return pointCloudLayer.release();
    }
  }
  if ( candidateTypes.empty() || candidateTypes.contains( Qgis::LayerType::VectorTile ) )
  {
    QgsDataSourceUri dsUri;
    dsUri.setParam( "type", "mbtiles" );
    dsUri.setParam( "url", uri );

    std::unique_ptr< QgsVectorTileLayer > tileLayer;
    tileLayer = std::make_unique< QgsVectorTileLayer >( dsUri.encodedUri(), name );

    if ( tileLayer->isValid() )
    {
      return tileLayer.release();
    }
  }
  if ( candidateTypes.empty() || candidateTypes.contains( Qgis::LayerType::TiledScene ) )
  {
    QgsTiledSceneLayer::LayerOptions tiledSceneOptions;
    tiledSceneOptions.skipCrsValidation = true;

    std::unique_ptr< QgsTiledSceneLayer > tiledSceneLayer;
    if ( providerMetadata )
    {
      tiledSceneLayer = std::make_unique< QgsTiledSceneLayer >( uri, name, providerMetadata->key(), tiledSceneOptions );
    }
    else
    {
      const QList< QgsProviderRegistry::ProviderCandidateDetails > preferredProviders = QgsProviderRegistry::instance()->preferredProvidersForUri( uri );
      if ( !preferredProviders.empty() )
      {
        tiledSceneLayer = std::make_unique< QgsTiledSceneLayer >( uri, name, preferredProviders.at( 0 ).metadata()->key(), tiledSceneOptions );
      }
    }
    if ( tiledSceneLayer && tiledSceneLayer->isValid() )
    {
      return tiledSceneLayer.release();
    }
  }
  return nullptr;
}

QgsMapLayer *QgsProcessingUtils::mapLayerFromString( const QString &string, QgsProcessingContext &context, bool allowLoadingNewLayers, LayerHint typeHint, QgsProcessing::LayerOptionsFlags flags )
{
  if ( string.isEmpty() )
    return nullptr;

  // prefer project layers
  if ( context.project() && typeHint == LayerHint::Annotation && string.compare( QLatin1String( "main" ), Qt::CaseInsensitive ) == 0 )
    return context.project()->mainAnnotationLayer();

  QgsMapLayer *layer = nullptr;
  if ( auto *lProject = context.project() )
  {
    QgsMapLayer *layer = mapLayerFromStore( string, lProject->layerStore(), typeHint );
    if ( layer )
      return layer;
  }

  layer = mapLayerFromStore( string, context.temporaryLayerStore(), typeHint );
  if ( layer )
    return layer;

  if ( !allowLoadingNewLayers )
    return nullptr;

  layer = loadMapLayerFromString( string, context.transformContext(), typeHint, flags );
  if ( layer )
  {
    context.temporaryLayerStore()->addMapLayer( layer );
    return layer;
  }
  else
  {
    return nullptr;
  }
}

QgsProcessingFeatureSource *QgsProcessingUtils::variantToSource( const QVariant &value, QgsProcessingContext &context, const QVariant &fallbackValue )
{
  QVariant val = value;
  bool selectedFeaturesOnly = false;
  long long featureLimit = -1;
  QString filterExpression;
  bool overrideGeometryCheck = false;
  Qgis::InvalidGeometryCheck geometryCheck = Qgis::InvalidGeometryCheck::AbortOnInvalid;
  if ( val.userType() == QMetaType::type( "QgsProcessingFeatureSourceDefinition" ) )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    selectedFeaturesOnly = fromVar.selectedFeaturesOnly;
    featureLimit = fromVar.featureLimit;
    filterExpression = fromVar.filterExpression;
    val = fromVar.source;
    overrideGeometryCheck = fromVar.flags & Qgis::ProcessingFeatureSourceDefinitionFlag::OverrideDefaultGeometryCheck;
    geometryCheck = fromVar.geometryCheck;
  }
  else if ( val.userType() == QMetaType::type( "QgsProcessingOutputLayerDefinition" ) )
  {
    // input is a QgsProcessingOutputLayerDefinition (e.g. an output from earlier in a model) - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( val ) ) )
  {
    std::unique_ptr< QgsProcessingFeatureSource> source = std::make_unique< QgsProcessingFeatureSource >( layer, context, false, featureLimit, filterExpression );
    if ( overrideGeometryCheck )
      source->setInvalidGeometryCheck( geometryCheck );
    return source.release();
  }

  QString layerRef;
  if ( val.userType() == QMetaType::type( "QgsProperty" ) )
  {
    layerRef = val.value< QgsProperty >().valueAsString( context.expressionContext(), fallbackValue.toString() );
  }
  else if ( !val.isValid() || val.toString().isEmpty() )
  {
    // fall back to default
    if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( fallbackValue ) ) )
    {
      std::unique_ptr< QgsProcessingFeatureSource> source = std::make_unique< QgsProcessingFeatureSource >( layer, context, false, featureLimit, filterExpression );
      if ( overrideGeometryCheck )
        source->setInvalidGeometryCheck( geometryCheck );
      return source.release();
    }

    layerRef = fallbackValue.toString();
  }
  else
  {
    layerRef = val.toString();
  }

  if ( layerRef.isEmpty() )
    return nullptr;

  QgsVectorLayer *vl = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( layerRef, context, true, LayerHint::Vector ) );
  if ( !vl )
    return nullptr;

  std::unique_ptr< QgsProcessingFeatureSource> source;
  if ( selectedFeaturesOnly )
  {
    source = std::make_unique< QgsProcessingFeatureSource>( new QgsVectorLayerSelectedFeatureSource( vl ), context, true, featureLimit, filterExpression );
  }
  else
  {
    source = std::make_unique< QgsProcessingFeatureSource >( vl, context, false, featureLimit, filterExpression );
  }

  if ( overrideGeometryCheck )
    source->setInvalidGeometryCheck( geometryCheck );
  return source.release();
}

QgsCoordinateReferenceSystem QgsProcessingUtils::variantToCrs( const QVariant &value, QgsProcessingContext &context, const QVariant &fallbackValue )
{
  QVariant val = value;

  if ( val.userType() == QMetaType::type( "QgsCoordinateReferenceSystem" ) )
  {
    // input is a QgsCoordinateReferenceSystem - done!
    return val.value< QgsCoordinateReferenceSystem >();
  }
  else if ( val.userType() == QMetaType::type( "QgsProcessingFeatureSourceDefinition" ) )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }
  else if ( val.userType() == QMetaType::type( "QgsProcessingOutputLayerDefinition" ) )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( val.userType() == QMetaType::type( "QgsProperty" ) && val.value< QgsProperty >().propertyType() == Qgis::PropertyType::Static )
  {
    val = val.value< QgsProperty >().staticValue();
  }

  // maybe a map layer
  if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( val ) ) )
    return layer->crs();

  if ( val.userType() == QMetaType::type( "QgsProperty" ) )
    val = val.value< QgsProperty >().valueAsString( context.expressionContext(), fallbackValue.toString() );

  if ( !val.isValid() )
  {
    // fall back to default
    val = fallbackValue;
  }

  QString crsText = val.toString();
  if ( crsText.isEmpty() )
    crsText = fallbackValue.toString();

  if ( crsText.isEmpty() )
    return QgsCoordinateReferenceSystem();

  // maybe special string
  if ( context.project() && crsText.compare( QLatin1String( "ProjectCrs" ), Qt::CaseInsensitive ) == 0 )
    return context.project()->crs();

  // maybe a map layer reference
  if ( QgsMapLayer *layer = QgsProcessingUtils::mapLayerFromString( crsText, context ) )
    return layer->crs();

  // else CRS from string
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( crsText );
  return crs;
}

bool QgsProcessingUtils::canUseLayer( const QgsMeshLayer *layer )
{
  return layer && layer->dataProvider();
}

bool QgsProcessingUtils::canUseLayer( const QgsPluginLayer *layer )
{
  return layer && layer->isValid();
}

bool QgsProcessingUtils::canUseLayer( const QgsVectorTileLayer *layer )
{
  return layer && layer->isValid();
}

bool QgsProcessingUtils::canUseLayer( const QgsRasterLayer *layer )
{
  return layer && layer->isValid();
}

bool QgsProcessingUtils::canUseLayer( const QgsPointCloudLayer *layer )
{
  return layer && layer->isValid();
}

bool QgsProcessingUtils::canUseLayer( const QgsAnnotationLayer *layer )
{
  return layer && layer->isValid();
}

bool QgsProcessingUtils::canUseLayer( const QgsTiledSceneLayer *layer )
{
  return layer && layer->isValid();
}

bool QgsProcessingUtils::canUseLayer( const QgsVectorLayer *layer, const QList<int> &sourceTypes )
{
  return layer && layer->isValid() &&
         ( sourceTypes.isEmpty()
           || ( sourceTypes.contains( static_cast< int >( Qgis::ProcessingSourceType::VectorPoint ) ) && layer->geometryType() == Qgis::GeometryType::Point )
           || ( sourceTypes.contains( static_cast< int >( Qgis::ProcessingSourceType::VectorLine ) ) && layer->geometryType() == Qgis::GeometryType::Line )
           || ( sourceTypes.contains( static_cast< int >( Qgis::ProcessingSourceType::VectorPolygon ) ) && layer->geometryType() == Qgis::GeometryType::Polygon )
           || ( sourceTypes.contains( static_cast< int >( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) && layer->isSpatial() )
           || sourceTypes.contains( static_cast< int >( Qgis::ProcessingSourceType::Vector ) )
         );
}

QString QgsProcessingUtils::normalizeLayerSource( const QString &source )
{
  QString normalized = source;
  normalized.replace( '\\', '/' );
  return normalized.trimmed();
}

QString QgsProcessingUtils::layerToStringIdentifier( const QgsMapLayer *layer )
{
  if ( !layer )
    return QString();

  const QString source = QgsProcessingUtils::normalizeLayerSource( layer->source() );
  if ( !source.isEmpty() )
  {
    const QString provider = layer->providerType();
    // don't prepend provider type for these exceptional providers -- we assume them
    // by default if the provider type is excluded. See logic in QgsProcessingUtils::loadMapLayerFromString
    if ( provider.compare( QLatin1String( "gdal" ), Qt::CaseInsensitive ) == 0
         || provider.compare( QLatin1String( "ogr" ), Qt::CaseInsensitive ) == 0
         || provider.compare( QLatin1String( "mdal" ), Qt::CaseInsensitive ) == 0 )
      return source;

    return QStringLiteral( "%1://%2" ).arg( provider, source );
  }
  return layer->id();
}

QString QgsProcessingUtils::variantToPythonLiteral( const QVariant &value )
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.userType() == QMetaType::type( "QgsProperty" ) )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );
  else if ( value.userType() == QMetaType::type( "QgsCoordinateReferenceSystem" ) )
  {
    if ( !value.value< QgsCoordinateReferenceSystem >().isValid() )
      return QStringLiteral( "QgsCoordinateReferenceSystem()" );
    else
      return QStringLiteral( "QgsCoordinateReferenceSystem('%1')" ).arg( value.value< QgsCoordinateReferenceSystem >().authid() );
  }
  else if ( value.userType() == QMetaType::type( "QgsRectangle" ) )
  {
    QgsRectangle r = value.value<QgsRectangle>();
    return QStringLiteral( "'%1, %3, %2, %4'" ).arg( qgsDoubleToString( r.xMinimum() ),
           qgsDoubleToString( r.yMinimum() ),
           qgsDoubleToString( r.xMaximum() ),
           qgsDoubleToString( r.yMaximum() ) );
  }
  else if ( value.userType() == QMetaType::type( "QgsReferencedRectangle" ) )
  {
    QgsReferencedRectangle r = value.value<QgsReferencedRectangle>();
    return QStringLiteral( "'%1, %3, %2, %4 [%5]'" ).arg( qgsDoubleToString( r.xMinimum() ),
           qgsDoubleToString( r.yMinimum() ),
           qgsDoubleToString( r.xMaximum() ),
           qgsDoubleToString( r.yMaximum() ),                                                                                                                             r.crs().authid() );
  }
  else if ( value.userType() == QMetaType::type( "QgsPointXY" ) )
  {
    QgsPointXY r = value.value<QgsPointXY>();
    return QStringLiteral( "'%1,%2'" ).arg( qgsDoubleToString( r.x() ),
                                            qgsDoubleToString( r.y() ) );
  }
  else if ( value.userType() == QMetaType::type( "QgsReferencedPointXY" ) )
  {
    QgsReferencedPointXY r = value.value<QgsReferencedPointXY>();
    return QStringLiteral( "'%1,%2 [%3]'" ).arg( qgsDoubleToString( r.x() ),
           qgsDoubleToString( r.y() ),
           r.crs().authid() );
  }

  switch ( value.type() )
  {
    case QVariant::Bool:
      return value.toBool() ? QStringLiteral( "True" ) : QStringLiteral( "False" );

    case QVariant::Double:
      return QString::number( value.toDouble() );

    case QVariant::Int:
    case QVariant::UInt:
      return QString::number( value.toInt() );

    case QVariant::LongLong:
    case QVariant::ULongLong:
      return QString::number( value.toLongLong() );

    case QVariant::List:
    {
      QStringList parts;
      const QVariantList vl = value.toList();
      for ( const QVariant &v : vl )
      {
        parts << variantToPythonLiteral( v );
      }
      return parts.join( ',' ).prepend( '[' ).append( ']' );
    }

    case QVariant::Map:
    {
      const QVariantMap map = value.toMap();
      QStringList parts;
      parts.reserve( map.size() );
      for ( auto it = map.constBegin(); it != map.constEnd(); ++it )
      {
        parts << QStringLiteral( "%1: %2" ).arg( stringToPythonLiteral( it.key() ), variantToPythonLiteral( it.value() ) );
      }
      return parts.join( ',' ).prepend( '{' ).append( '}' );
    }

    case QVariant::DateTime:
    {
      const QDateTime dateTime = value.toDateTime();
      return QStringLiteral( "QDateTime(QDate(%1, %2, %3), QTime(%4, %5, %6))" )
             .arg( dateTime.date().year() )
             .arg( dateTime.date().month() )
             .arg( dateTime.date().day() )
             .arg( dateTime.time().hour() )
             .arg( dateTime.time().minute() )
             .arg( dateTime.time().second() );
    }

    default:
      break;
  }

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingUtils::stringToPythonLiteral( const QString &string )
{
  QString s = string;
  s.replace( '\\', QLatin1String( "\\\\" ) );
  s.replace( '\n', QLatin1String( "\\n" ) );
  s.replace( '\r', QLatin1String( "\\r" ) );
  s.replace( '\t', QLatin1String( "\\t" ) );

  if ( s.contains( '\'' ) && !s.contains( '\"' ) )
  {
    s = s.prepend( '"' ).append( '"' );
  }
  else
  {
    s.replace( '\'', QLatin1String( "\\\'" ) );
    s = s.prepend( '\'' ).append( '\'' );
  }
  return s;
}

void QgsProcessingUtils::parseDestinationString( QString &destination, QString &providerKey, QString &uri, QString &layerName, QString &format, QMap<QString, QVariant> &options, bool &useWriter, QString &extension )
{
  extension.clear();
  bool matched = decodeProviderKeyAndUri( destination, providerKey, uri );

  if ( !matched )
  {
    const thread_local QRegularExpression splitRx( QStringLiteral( "^(.{3,}?):(.*)$" ) );
    QRegularExpressionMatch match = splitRx.match( destination );
    if ( match.hasMatch() )
    {
      providerKey = match.captured( 1 );
      uri = match.captured( 2 );
      matched = true;
    }
  }

  if ( matched )
  {
    if ( providerKey == QLatin1String( "postgis" ) ) // older processing used "postgis" instead of "postgres"
    {
      providerKey = QStringLiteral( "postgres" );
    }
    if ( providerKey == QLatin1String( "ogr" ) )
    {
      QgsDataSourceUri dsUri( uri );
      if ( !dsUri.database().isEmpty() )
      {
        if ( !dsUri.table().isEmpty() )
        {
          layerName = dsUri.table();
          options.insert( QStringLiteral( "layerName" ), layerName );
        }
        uri = dsUri.database();
        extension = QFileInfo( uri ).completeSuffix();
        format = QgsVectorFileWriter::driverForExtension( extension );
        options.insert( QStringLiteral( "driverName" ), format );
      }
      else
      {
        extension = QFileInfo( uri ).completeSuffix();
        options.insert( QStringLiteral( "driverName" ), QgsVectorFileWriter::driverForExtension( extension ) );
      }
      options.insert( QStringLiteral( "update" ), true );
    }
    useWriter = false;
  }
  else
  {
    useWriter = true;
    providerKey = QStringLiteral( "ogr" );

    const thread_local QRegularExpression splitRx( QStringLiteral( "^(.*)\\.(.*?)$" ) );
    QRegularExpressionMatch match = splitRx.match( destination );
    if ( match.hasMatch() )
    {
      extension = match.captured( 2 );
      format = QgsVectorFileWriter::driverForExtension( extension );
    }

    if ( format.isEmpty() )
    {
      format = QStringLiteral( "GPKG" );
      destination = destination + QStringLiteral( ".gpkg" );
    }

    options.insert( QStringLiteral( "driverName" ), format );
    uri = destination;
  }
}

QgsFeatureSink *QgsProcessingUtils::createFeatureSink( QString &destination, QgsProcessingContext &context, const QgsFields &fields, Qgis::WkbType geometryType, const QgsCoordinateReferenceSystem &crs, const QVariantMap &createOptions, const QStringList &datasourceOptions, const QStringList &layerOptions, QgsFeatureSink::SinkFlags sinkFlags, QgsRemappingSinkDefinition *remappingDefinition )
{
  QVariantMap options = createOptions;
  if ( !options.contains( QStringLiteral( "fileEncoding" ) ) )
  {
    // no destination encoding specified, use default
    options.insert( QStringLiteral( "fileEncoding" ), context.defaultEncoding().isEmpty() ? QStringLiteral( "system" ) : context.defaultEncoding() );
  }

  if ( destination.isEmpty() || destination.startsWith( QLatin1String( "memory:" ) ) )
  {
    // strip "memory:" from start of destination
    if ( destination.startsWith( QLatin1String( "memory:" ) ) )
      destination = destination.mid( 7 );

    if ( destination.isEmpty() )
      destination = QStringLiteral( "output" );

    // memory provider cannot be used with QgsVectorLayerImport - so create layer manually
    std::unique_ptr< QgsVectorLayer > layer( QgsMemoryProviderUtils::createMemoryLayer( destination, fields, geometryType, crs, false ) );
    if ( !layer || !layer->isValid() )
    {
      throw QgsProcessingException( QObject::tr( "Could not create memory layer" ) );
    }

    if ( QgsProcessingFeedback *feedback = context.feedback() )
    {
      for ( const QgsField &field : fields )
      {
        // TODO -- support these!
        if ( !field.alias().isEmpty() )
          feedback->pushWarning( QObject::tr( "%1: Aliases are not compatible with scratch layers" ).arg( field.name() ) );
        if ( !field.alias().isEmpty() )
          feedback->pushWarning( QObject::tr( "%1: Comments are not compatible with scratch layers" ).arg( field.name() ) );
      }
    }

    layer->setCustomProperty( QStringLiteral( "OnConvertFormatRegeneratePrimaryKey" ), static_cast< bool >( sinkFlags & QgsFeatureSink::RegeneratePrimaryKey ) );

    // update destination to layer ID
    destination = layer->id();

    // this is a factory, so we need to return a proxy
    std::unique_ptr< QgsProcessingFeatureSink > sink( new QgsProcessingFeatureSink( layer->dataProvider(), destination, context ) );
    context.temporaryLayerStore()->addMapLayer( layer.release() );

    return sink.release();
  }
  else
  {
    QString providerKey;
    QString uri;
    QString layerName;
    QString format;
    QString extension;
    bool useWriter = false;
    parseDestinationString( destination, providerKey, uri, layerName, format, options, useWriter, extension );

    QgsFields newFields = fields;
    if ( useWriter && providerKey == QLatin1String( "ogr" ) )
    {
      // use QgsVectorFileWriter for OGR destinations instead of QgsVectorLayerImport, as that allows
      // us to use any OGR format which supports feature addition
      QString finalFileName;
      QString finalLayerName;
      QgsVectorFileWriter::SaveVectorOptions saveOptions;
      saveOptions.fileEncoding = options.value( QStringLiteral( "fileEncoding" ) ).toString();
      saveOptions.layerName = !layerName.isEmpty() ? layerName : options.value( QStringLiteral( "layerName" ) ).toString();
      saveOptions.driverName = format;
      saveOptions.datasourceOptions = !datasourceOptions.isEmpty() ? datasourceOptions : QgsVectorFileWriter::defaultDatasetOptions( format );
      saveOptions.layerOptions = !layerOptions.isEmpty() ? layerOptions : QgsVectorFileWriter::defaultLayerOptions( format );
      saveOptions.symbologyExport = Qgis::FeatureSymbologyExport::NoSymbology;
      if ( remappingDefinition )
      {
        saveOptions.actionOnExistingFile = QgsVectorFileWriter::AppendToLayerNoNewFields;
        // sniff destination file to get correct wkb type and crs
        std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( destination );
        if ( vl->isValid() )
        {
          remappingDefinition->setDestinationWkbType( vl->wkbType() );
          remappingDefinition->setDestinationCrs( vl->crs() );
          newFields = vl->fields();
          remappingDefinition->setDestinationFields( newFields );
        }
        context.expressionContext().setFields( fields );
      }
      else
      {
        saveOptions.actionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteFile;
      }
      std::unique_ptr< QgsVectorFileWriter > writer( QgsVectorFileWriter::create( destination, newFields, geometryType, crs, context.transformContext(), saveOptions, sinkFlags, &finalFileName, &finalLayerName ) );
      if ( writer->hasError() )
      {
        throw QgsProcessingException( QObject::tr( "Could not create layer %1: %2" ).arg( destination, writer->errorMessage() ) );
      }

      if ( QgsProcessingFeedback *feedback = context.feedback() )
      {
        for ( const QgsField &field : fields )
        {
          if ( !field.alias().isEmpty() && !( writer->capabilities() & Qgis::VectorFileWriterCapability::FieldAliases ) )
            feedback->pushWarning( QObject::tr( "%1: Aliases are not supported by %2" ).arg( field.name(), writer->driverLongName() ) );
          if ( !field.alias().isEmpty()  && !( writer->capabilities() & Qgis::VectorFileWriterCapability::FieldComments ) )
            feedback->pushWarning( QObject::tr( "%1: Comments are not supported by %2" ).arg( field.name(), writer->driverLongName() ) );
        }
      }

      destination = finalFileName;
      if ( !saveOptions.layerName.isEmpty() && !finalLayerName.isEmpty() )
        destination += QStringLiteral( "|layername=%1" ).arg( finalLayerName );

      if ( remappingDefinition )
      {
        std::unique_ptr< QgsRemappingProxyFeatureSink > remapSink = std::make_unique< QgsRemappingProxyFeatureSink >( *remappingDefinition, writer.release(), true );
        remapSink->setExpressionContext( context.expressionContext() );
        remapSink->setTransformContext( context.transformContext() );
        return new QgsProcessingFeatureSink( remapSink.release(), destination, context, true );
      }
      else
        return new QgsProcessingFeatureSink( writer.release(), destination, context, true );
    }
    else
    {
      const QgsVectorLayer::LayerOptions layerOptions { context.transformContext() };
      if ( remappingDefinition )
      {
        //write to existing layer

        // use destination string as layer name (eg "postgis:..." )
        if ( !layerName.isEmpty() )
        {
          QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( providerKey, uri );
          parts.insert( QStringLiteral( "layerName" ), layerName );
          uri = QgsProviderRegistry::instance()->encodeUri( providerKey, parts );
        }

        std::unique_ptr< QgsVectorLayer > layer = std::make_unique<QgsVectorLayer>( uri, destination, providerKey, layerOptions );
        // update destination to layer ID
        destination = layer->id();
        if ( layer->isValid() )
        {
          remappingDefinition->setDestinationWkbType( layer->wkbType() );
          remappingDefinition->setDestinationCrs( layer->crs() );
          remappingDefinition->setDestinationFields( layer->fields() );
        }

        if ( QgsProcessingFeedback *feedback = context.feedback() )
        {
          const Qgis::VectorDataProviderAttributeEditCapabilities capabilities = layer->dataProvider() ? layer->dataProvider()->attributeEditCapabilities() : Qgis::VectorDataProviderAttributeEditCapabilities();
          for ( const QgsField &field : fields )
          {
            if ( !field.alias().isEmpty() && !( capabilities & Qgis::VectorDataProviderAttributeEditCapability::EditAlias ) )
              feedback->pushWarning( QObject::tr( "%1: Aliases are not supported by the %2 provider" ).arg( field.name(), providerKey ) );
            if ( !field.alias().isEmpty()  && !( capabilities & Qgis::VectorDataProviderAttributeEditCapability::EditComment ) )
              feedback->pushWarning( QObject::tr( "%1: Comments are not supported by the %2 provider" ).arg( field.name(), providerKey ) );
          }
        }

        std::unique_ptr< QgsRemappingProxyFeatureSink > remapSink = std::make_unique< QgsRemappingProxyFeatureSink >( *remappingDefinition, layer->dataProvider(), false );
        context.temporaryLayerStore()->addMapLayer( layer.release() );
        remapSink->setExpressionContext( context.expressionContext() );
        remapSink->setTransformContext( context.transformContext() );
        context.expressionContext().setFields( fields );
        return new QgsProcessingFeatureSink( remapSink.release(), destination, context, true );
      }
      else
      {
        //create empty layer
        std::unique_ptr< QgsVectorLayerExporter > exporter = std::make_unique<QgsVectorLayerExporter>( uri, providerKey, newFields, geometryType, crs, true, options, sinkFlags );
        if ( exporter->errorCode() != Qgis::VectorExportResult::Success )
        {
          throw QgsProcessingException( QObject::tr( "Could not create layer %1: %2" ).arg( destination, exporter->errorMessage() ) );
        }

        // use destination string as layer name (eg "postgis:..." )
        if ( !layerName.isEmpty() )
        {
          uri += QStringLiteral( "|layername=%1" ).arg( layerName );
          // update destination to generated URI
          destination = uri;
        }

        if ( QgsProcessingFeedback *feedback = context.feedback() )
        {
          for ( const QgsField &field : fields )
          {
            if ( !field.alias().isEmpty() && !( exporter->attributeEditCapabilities() & Qgis::VectorDataProviderAttributeEditCapability::EditAlias ) )
              feedback->pushWarning( QObject::tr( "%1: Aliases are not supported by the %2 provider" ).arg( field.name(), providerKey ) );
            if ( !field.alias().isEmpty()  && !( exporter->attributeEditCapabilities() & Qgis::VectorDataProviderAttributeEditCapability::EditComment ) )
              feedback->pushWarning( QObject::tr( "%1: Comments are not supported by the %2 provider" ).arg( field.name(), providerKey ) );
          }
        }

        return new QgsProcessingFeatureSink( exporter.release(), destination, context, true );
      }
    }
  }
}

void QgsProcessingUtils::createFeatureSinkPython( QgsFeatureSink **sink, QString &destination, QgsProcessingContext &context, const QgsFields &fields, Qgis::WkbType geometryType, const QgsCoordinateReferenceSystem &crs, const QVariantMap &options )
{
  *sink = createFeatureSink( destination, context, fields, geometryType, crs, options );
}


QgsRectangle QgsProcessingUtils::combineLayerExtents( const QList<QgsMapLayer *> &layers, const QgsCoordinateReferenceSystem &crs, QgsProcessingContext &context )
{
  QgsRectangle extent;
  for ( const QgsMapLayer *layer : layers )
  {
    if ( !layer )
      continue;

    if ( crs.isValid() )
    {
      //transform layer extent to target CRS
      QgsCoordinateTransform ct( layer->crs(), crs, context.transformContext() );
      ct.setBallparkTransformsAreAppropriate( true );
      try
      {
        QgsRectangle reprojExtent = ct.transformBoundingBox( layer->extent() );
        extent.combineExtentWith( reprojExtent );
      }
      catch ( QgsCsException & )
      {
        // can't reproject... what to do here? hmmm?
        // let's ignore this layer for now, but maybe we should just use the original extent?
      }
    }
    else
    {
      extent.combineExtentWith( layer->extent() );
    }

  }
  return extent;
}

// Deprecated
QgsRectangle QgsProcessingUtils::combineLayerExtents( const QList<QgsMapLayer *> &layers, const QgsCoordinateReferenceSystem &crs )
{
  QgsProcessingContext context;
  return QgsProcessingUtils::combineLayerExtents( layers, crs, context );
}

QVariant QgsProcessingUtils::generateIteratingDestination( const QVariant &input, const QVariant &id, QgsProcessingContext &context )
{
  if ( !input.isValid() )
    return QStringLiteral( "memory:%1" ).arg( id.toString() );

  if ( input.userType() == QMetaType::type( "QgsProcessingOutputLayerDefinition" ) )
  {
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( input );
    QVariant newSink = generateIteratingDestination( fromVar.sink, id, context );
    fromVar.sink = QgsProperty::fromValue( newSink );
    return fromVar;
  }
  else if ( input.userType() == QMetaType::type( "QgsProperty" ) )
  {
    QString res = input.value< QgsProperty>().valueAsString( context.expressionContext() );
    return generateIteratingDestination( res, id, context );
  }
  else
  {
    QString res = input.toString();
    if ( res == QgsProcessing::TEMPORARY_OUTPUT )
    {
      // temporary outputs map to temporary outputs!
      return QgsProcessing::TEMPORARY_OUTPUT;
    }
    else if ( res.startsWith( QLatin1String( "memory:" ) ) )
    {
      return QString( res + '_' + id.toString() );
    }
    else
    {
      // assume a filename type output for now
      // TODO - uris?
      int lastIndex = res.lastIndexOf( '.' );
      return lastIndex >= 0 ? QString( res.left( lastIndex ) + '_' + id.toString() + res.mid( lastIndex ) ) : QString( res + '_' + id.toString() );
    }
  }
}

QString QgsProcessingUtils::tempFolder( const QgsProcessingContext *context )
{
  // we maintain a list of temporary folders -- this allows us to append additional
  // folders when a setting change causes the base temp folder to change, while deferring
  // cleanup of ALL these temp folders until session end (we can't cleanup older folders immediately,
  // because we don't know whether they have data in them which is still wanted)
  static std::vector< std::unique_ptr< QTemporaryDir > > sTempFolders;
  static QString sFolder;
  static QMutex sMutex;
  QMutexLocker locker( &sMutex );
  QString basePath;

  if ( context )
    basePath = context->temporaryFolder();
  if ( basePath.isEmpty() )
    basePath = QgsProcessing::settingsTempPath->value();

  if ( basePath.isEmpty() )
  {
    // default setting -- automatically create a temp folder
    if ( sTempFolders.empty() )
    {
      const QString templatePath = QStringLiteral( "%1/processing_XXXXXX" ).arg( QDir::tempPath() );
      std::unique_ptr< QTemporaryDir > tempFolder = std::make_unique< QTemporaryDir >( templatePath );
      sFolder = tempFolder->path();
      sTempFolders.emplace_back( std::move( tempFolder ) );
    }
  }
  else if ( sFolder.isEmpty() || !sFolder.startsWith( basePath ) || sTempFolders.empty() )
  {
    if ( !QDir().exists( basePath ) )
      QDir().mkpath( basePath );

    const QString templatePath = QStringLiteral( "%1/processing_XXXXXX" ).arg( basePath );
    std::unique_ptr< QTemporaryDir > tempFolder = std::make_unique< QTemporaryDir >( templatePath );
    sFolder = tempFolder->path();
    sTempFolders.emplace_back( std::move( tempFolder ) );
  }
  return sFolder;
}

QString QgsProcessingUtils::generateTempFilename( const QString &basename, const QgsProcessingContext *context )
{
  QString subPath = QUuid::createUuid().toString().remove( '-' ).remove( '{' ).remove( '}' );
  QString path = tempFolder( context ) + '/' + subPath;
  if ( !QDir( path ).exists() ) //make sure the directory exists - it shouldn't, but lets be safe...
  {
    QDir tmpDir;
    tmpDir.mkdir( path );
  }
  return path + '/' + QgsFileUtils::stringToSafeFilename( basename );
}

QString QgsProcessingUtils::formatHelpMapAsHtml( const QVariantMap &map, const QgsProcessingAlgorithm *algorithm )
{
  auto getText = [map]( const QString & key )->QString
  {
    if ( map.contains( key ) )
      return map.value( key ).toString();
    return QString();
  };

  QString s;
  s += QStringLiteral( "<html><body><p>" ) + getText( QStringLiteral( "ALG_DESC" ) ) + QStringLiteral( "</p>\n" );

  QString inputs;
  const auto parameterDefinitions = algorithm->parameterDefinitions();
  for ( const QgsProcessingParameterDefinition *def : parameterDefinitions )
  {
    if ( def->flags() & Qgis::ProcessingParameterFlag::Hidden || def->isDestination() )
      continue;

    if ( !getText( def->name() ).isEmpty() )
    {
      inputs += QStringLiteral( "<h3>" ) + def->description() + QStringLiteral( "</h3>\n" );
      inputs += QStringLiteral( "<p>" ) + getText( def->name() ) + QStringLiteral( "</p>\n" );
    }
  }
  if ( !inputs.isEmpty() )
    s += QStringLiteral( "<h2>" ) + QObject::tr( "Input parameters" ) + QStringLiteral( "</h2>\n" ) + inputs;

  QString outputs;
  const auto outputDefinitions = algorithm->outputDefinitions();
  for ( const QgsProcessingOutputDefinition *def : outputDefinitions )
  {
    if ( !getText( def->name() ).isEmpty() )
    {
      outputs += QStringLiteral( "<h3>" ) + def->description() + QStringLiteral( "</h3>\n" );
      outputs += QStringLiteral( "<p>" ) + getText( def->name() ) + QStringLiteral( "</p>\n" );
    }
  }
  if ( !outputs.isEmpty() )
    s += QStringLiteral( "<h2>" ) + QObject::tr( "Outputs" ) + QStringLiteral( "</h2>\n" ) + outputs;

  if ( !map.value( QStringLiteral( "EXAMPLES" ) ).toString().isEmpty() )
    s += QStringLiteral( "<h2>%1</h2>\n<p>%2</p>" ).arg( QObject::tr( "Examples" ), getText( QStringLiteral( "EXAMPLES" ) ) );

  s += QLatin1String( "<br>" );
  if ( !map.value( QStringLiteral( "ALG_CREATOR" ) ).toString().isEmpty() )
    s += QStringLiteral( "<p align=\"right\">" ) + QObject::tr( "Algorithm author:" ) + QStringLiteral( " " ) + getText( QStringLiteral( "ALG_CREATOR" ) ) + QStringLiteral( "</p>" );
  if ( !map.value( QStringLiteral( "ALG_HELP_CREATOR" ) ).toString().isEmpty() )
    s += QStringLiteral( "<p align=\"right\">" ) + QObject::tr( "Help author:" ) + QStringLiteral( " " ) + getText( QStringLiteral( "ALG_HELP_CREATOR" ) ) + QStringLiteral( "</p>" );
  if ( !map.value( QStringLiteral( "ALG_VERSION" ) ).toString().isEmpty() )
    s += QStringLiteral( "<p align=\"right\">" ) + QObject::tr( "Algorithm version:" ) + QStringLiteral( " " ) + getText( QStringLiteral( "ALG_VERSION" ) ) + QStringLiteral( "</p>" );

  s += QLatin1String( "</body></html>" );
  return s;
}

QString convertToCompatibleFormatInternal( const QgsVectorLayer *vl, bool selectedFeaturesOnly, const QString &baseName, const QStringList &compatibleFormats, const QString &preferredFormat, QgsProcessingContext &context, QgsProcessingFeedback *feedback, QString *layerName,
    long long featureLimit, const QString &filterExpression )
{
  bool requiresTranslation = false;

  // if we are only looking for selected features then we have to export back to disk,
  // as we need to subset only selected features, a concept which doesn't exist outside QGIS!
  requiresTranslation = requiresTranslation || selectedFeaturesOnly;

  // if we are limiting the feature count, we better export
  requiresTranslation = requiresTranslation || featureLimit != -1 || !filterExpression.isEmpty();

  // if the data provider is NOT ogr, then we HAVE to convert. Otherwise we run into
  // issues with data providers like spatialite, delimited text where the format can be
  // opened outside of QGIS, but with potentially very different behavior!
  requiresTranslation = requiresTranslation || vl->providerType() != QLatin1String( "ogr" );

  // if the layer has a feature filter set, then we HAVE to convert. Feature filters are
  // a purely QGIS concept.
  requiresTranslation = requiresTranslation || !vl->subsetString().isEmpty();

  // if the layer opened using GDAL's virtual I/O mechanism (/vsizip/, etc.), then
  // we HAVE to convert as other tools may not work with it
  requiresTranslation = requiresTranslation || vl->source().startsWith( QLatin1String( "/vsi" ) );

  // Check if layer is a disk based format and if so if the layer's path has a compatible filename suffix
  QString diskPath;
  if ( !requiresTranslation )
  {
    const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( vl->providerType(), vl->source() );
    if ( parts.contains( QStringLiteral( "path" ) ) )
    {
      diskPath = parts.value( QStringLiteral( "path" ) ).toString();
      QFileInfo fi( diskPath );
      requiresTranslation = !compatibleFormats.contains( fi.suffix(), Qt::CaseInsensitive );

      // if the layer name doesn't match the filename, we need to convert the layer. This method can only return
      // a filename, and cannot handle layernames as well as file paths
      const QString srcLayerName = parts.value( QStringLiteral( "layerName" ) ).toString();
      if ( layerName )
      {
        // differing layer names are acceptable
        *layerName = srcLayerName;
      }
      else
      {
        // differing layer names are NOT acceptable
        requiresTranslation = requiresTranslation || ( !srcLayerName.isEmpty() && srcLayerName != fi.baseName() );
      }
    }
    else
    {
      requiresTranslation = true; // not a disk-based format
    }
  }

  if ( requiresTranslation )
  {
    QString temp = QgsProcessingUtils::generateTempFilename( baseName + '.' + preferredFormat, &context );

    QgsVectorFileWriter::SaveVectorOptions saveOptions;
    saveOptions.fileEncoding = context.defaultEncoding();
    saveOptions.driverName = QgsVectorFileWriter::driverForExtension( preferredFormat );
    std::unique_ptr< QgsVectorFileWriter > writer( QgsVectorFileWriter::create( temp, vl->fields(), vl->wkbType(), vl->crs(), context.transformContext(), saveOptions ) );
    QgsFeature f;
    QgsFeatureIterator it;
    QgsFeatureRequest request;
    if ( featureLimit != -1 )
    {
      request.setLimit( featureLimit );
    }
    if ( !filterExpression.isEmpty() )
    {
      request.setFilterExpression( filterExpression );
    }

    if ( selectedFeaturesOnly )
      it = vl->getSelectedFeatures( request );
    else
      it = vl->getFeatures( request );

    while ( it.nextFeature( f ) )
    {
      if ( feedback && feedback->isCanceled() )
        return QString();
      writer->addFeature( f, QgsFeatureSink::FastInsert );
    }
    return temp;
  }
  else
  {
    return diskPath;
  }
}

QString QgsProcessingUtils::convertToCompatibleFormat( const QgsVectorLayer *vl, bool selectedFeaturesOnly, const QString &baseName, const QStringList &compatibleFormats, const QString &preferredFormat, QgsProcessingContext &context, QgsProcessingFeedback *feedback, long long featureLimit, const QString &filterExpression )
{
  return convertToCompatibleFormatInternal( vl, selectedFeaturesOnly, baseName, compatibleFormats, preferredFormat, context, feedback, nullptr, featureLimit, filterExpression );
}

QString QgsProcessingUtils::convertToCompatibleFormatAndLayerName( const QgsVectorLayer *layer, bool selectedFeaturesOnly, const QString &baseName, const QStringList &compatibleFormats, const QString &preferredFormat, QgsProcessingContext &context, QgsProcessingFeedback *feedback, QString &layerName, long long featureLimit, const QString &filterExpression )
{
  layerName.clear();
  return convertToCompatibleFormatInternal( layer, selectedFeaturesOnly, baseName, compatibleFormats, preferredFormat, context, feedback, &layerName, featureLimit, filterExpression );
}

QgsFields QgsProcessingUtils::combineFields( const QgsFields &fieldsA, const QgsFields &fieldsB, const QString &fieldsBPrefix )
{
  QgsFields outFields = fieldsA;
  QSet< QString > usedNames;
  for ( const QgsField &f : fieldsA )
  {
    usedNames.insert( f.name().toLower() );
  }

  for ( const QgsField &f : fieldsB )
  {
    QgsField newField = f;
    newField.setName( fieldsBPrefix + f.name() );
    if ( usedNames.contains( newField.name().toLower() ) )
    {
      int idx = 2;
      QString newName = newField.name() + '_' + QString::number( idx );
      while ( usedNames.contains( newName.toLower() ) || fieldsB.indexOf( newName ) != -1 )
      {
        idx++;
        newName = newField.name() + '_' + QString::number( idx );
      }
      newField.setName( newName );
      outFields.append( newField );
    }
    else
    {
      outFields.append( newField );
    }
    usedNames.insert( newField.name() );
  }

  return outFields;
}


QList<int> QgsProcessingUtils::fieldNamesToIndices( const QStringList &fieldNames, const QgsFields &fields )
{
  QList<int> indices;
  if ( !fieldNames.isEmpty() )
  {
    indices.reserve( fieldNames.count() );
    for ( const QString &f : fieldNames )
    {
      int idx = fields.lookupField( f );
      if ( idx >= 0 )
        indices.append( idx );
    }
  }
  else
  {
    indices.reserve( fields.count() );
    for ( int i = 0; i < fields.count(); ++i )
      indices.append( i );
  }
  return indices;
}


QgsFields QgsProcessingUtils::indicesToFields( const QList<int> &indices, const QgsFields &fields )
{
  QgsFields fieldsSubset;
  for ( int i : indices )
    fieldsSubset.append( fields.at( i ) );
  return fieldsSubset;
}

QString QgsProcessingUtils::defaultVectorExtension()
{
  const int setting = QgsProcessing::settingsDefaultOutputVectorLayerExt->value();
  if ( setting == -1 )
    return QStringLiteral( "gpkg" );
  return QgsVectorFileWriter::supportedFormatExtensions().value( setting, QStringLiteral( "gpkg" ) );
}

QString QgsProcessingUtils::defaultRasterExtension()
{
  const int setting = QgsProcessing::settingsDefaultOutputRasterLayerExt->value();
  if ( setting == -1 )
    return QStringLiteral( "tif" );
  return QgsRasterFileWriter::supportedFormatExtensions().value( setting, QStringLiteral( "tif" ) );
}

QString QgsProcessingUtils::defaultPointCloudExtension()
{
  return QStringLiteral( "las" );
}

QString QgsProcessingUtils::defaultVectorTileExtension()
{
  return QStringLiteral( "mbtiles" );
}

QVariantMap QgsProcessingUtils::removePointerValuesFromMap( const QVariantMap &map )
{
  auto layerPointerToString = []( QgsMapLayer * layer ) -> QString
  {
    if ( layer && layer->providerType() == QLatin1String( "memory" ) )
      return layer->id();
    else if ( layer )
      return layer->source();
    else
      return QString();
  };

  auto cleanPointerValues = [&layerPointerToString]( const QVariant & value ) -> QVariant
  {
    if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( value.value< QObject * >() ) )
    {
      // don't store pointers in maps for long-term storage
      return layerPointerToString( layer );
    }
    else if ( value.userType() == QMetaType::type( "QPointer< QgsMapLayer >" ) )
    {
      // don't store pointers in maps for long-term storage
      return layerPointerToString( value.value< QPointer< QgsMapLayer > >().data() );
    }
    else
    {
      return value;
    }
  };

  QVariantMap res;
  for ( auto it = map.constBegin(); it != map.constEnd(); ++it )
  {
    if ( it->type() == QVariant::Map )
    {
      res.insert( it.key(), removePointerValuesFromMap( it.value().toMap() ) );
    }
    else if ( it->type() == QVariant::List )
    {
      QVariantList dest;
      const QVariantList source = it.value().toList();
      dest.reserve( source.size() );
      for ( const QVariant &v : source )
      {
        dest.append( cleanPointerValues( v ) );
      }
      res.insert( it.key(), dest );
    }
    else
    {
      res.insert( it.key(), cleanPointerValues( it.value() ) );
    }
  }
  return res;
}

QVariantMap QgsProcessingUtils::preprocessQgisProcessParameters( const QVariantMap &parameters, bool &ok, QString &error )
{
  QVariantMap output;
  ok = true;
  for ( auto it = parameters.constBegin(); it != parameters.constEnd(); ++it )
  {
    if ( it.value().type() == QVariant::Map )
    {
      const QVariantMap value = it.value().toMap();
      if ( value.value( QStringLiteral( "type" ) ).toString() == QLatin1String( "data_defined" ) )
      {
        const QString expression = value.value( QStringLiteral( "expression" ) ).toString();
        const QString field = value.value( QStringLiteral( "field" ) ).toString();
        if ( !expression.isEmpty() )
        {
          output.insert( it.key(), QgsProperty::fromExpression( expression ) );
        }
        else if ( !field.isEmpty() )
        {
          output.insert( it.key(), QgsProperty::fromField( field ) );
        }
        else
        {
          ok = false;
          error = QObject::tr( "Invalid data defined parameter for %1, requires 'expression' or 'field' values." ).arg( it.key() );
        }
      }
      else
      {
        output.insert( it.key(), it.value() );
      }
    }
    else if ( it.value().type() == QVariant::String )
    {
      const QString stringValue = it.value().toString();

      if ( stringValue.startsWith( QLatin1String( "field:" ) ) )
      {
        output.insert( it.key(), QgsProperty::fromField( stringValue.mid( 6 ) ) );
      }
      else if ( stringValue.startsWith( QLatin1String( "expression:" ) ) )
      {
        output.insert( it.key(), QgsProperty::fromExpression( stringValue.mid( 11 ) ) );
      }
      else
      {
        output.insert( it.key(), it.value() );
      }
    }
    else
    {
      output.insert( it.key(), it.value() );
    }
  }
  return output;
}

QString QgsProcessingUtils::resolveDefaultEncoding( const QString &defaultEncoding )
{
  if ( ! QTextCodec::availableCodecs().contains( defaultEncoding.toLatin1() ) )
  {
    const QString systemCodec = QTextCodec::codecForLocale()->name();
    if ( ! systemCodec.isEmpty() )
    {
      return systemCodec;
    }
    return QString( "UTF-8" );
  }

  return defaultEncoding;
}

//
// QgsProcessingFeatureSource
//

QgsProcessingFeatureSource::QgsProcessingFeatureSource( QgsFeatureSource *originalSource, const QgsProcessingContext &context, bool ownsOriginalSource, long long featureLimit, const QString &filterExpression )
  : mSource( originalSource )
  , mOwnsSource( ownsOriginalSource )
  , mSourceCrs( mSource->sourceCrs() )
  , mSourceFields( mSource->fields() )
  , mSourceWkbType( mSource->wkbType() )
  , mSourceName( mSource->sourceName() )
  , mSourceExtent( mSource->sourceExtent() )
  , mSourceSpatialIndexPresence( mSource->hasSpatialIndex() )
  , mInvalidGeometryCheck( QgsWkbTypes::geometryType( mSource->wkbType() ) == Qgis::GeometryType::Point
                           ? Qgis::InvalidGeometryCheck::NoCheck // never run geometry validity checks for point layers!
                           : context.invalidGeometryCheck() )
  , mInvalidGeometryCallback( context.invalidGeometryCallback( originalSource ) )
  , mTransformErrorCallback( context.transformErrorCallback() )
  , mInvalidGeometryCallbackSkip( context.defaultInvalidGeometryCallbackForCheck( Qgis::InvalidGeometryCheck::SkipInvalid, originalSource ) )
  , mInvalidGeometryCallbackAbort( context.defaultInvalidGeometryCallbackForCheck( Qgis::InvalidGeometryCheck::AbortOnInvalid, originalSource ) )
  , mFeatureLimit( featureLimit )
  , mFilterExpression( filterExpression )
{}

QgsProcessingFeatureSource::~QgsProcessingFeatureSource()
{
  if ( mOwnsSource )
    delete mSource;
}

QgsFeatureIterator QgsProcessingFeatureSource::getFeatures( const QgsFeatureRequest &request, Qgis::ProcessingFeatureSourceFlags flags ) const
{
  QgsFeatureRequest req( request );
  req.setTransformErrorCallback( mTransformErrorCallback );

  if ( flags & Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks )
    req.setInvalidGeometryCheck( Qgis::InvalidGeometryCheck::NoCheck );
  else
  {
    req.setInvalidGeometryCheck( mInvalidGeometryCheck );
    req.setInvalidGeometryCallback( mInvalidGeometryCallback );
  }

  if ( mFeatureLimit != -1 && req.limit() != -1 )
    req.setLimit( std::min( static_cast< long long >( req.limit() ), mFeatureLimit ) );
  else if ( mFeatureLimit != -1 )
    req.setLimit( mFeatureLimit );

  if ( !mFilterExpression.isEmpty() )
    req.combineFilterExpression( mFilterExpression );

  return mSource->getFeatures( req );
}

Qgis::FeatureAvailability QgsProcessingFeatureSource::hasFeatures() const
{
  Qgis::FeatureAvailability sourceAvailability = mSource->hasFeatures();
  if ( sourceAvailability == Qgis::FeatureAvailability::NoFeaturesAvailable )
    return Qgis::FeatureAvailability::NoFeaturesAvailable; // never going to be features if underlying source has no features
  else if ( mInvalidGeometryCheck == Qgis::InvalidGeometryCheck::NoCheck && mFilterExpression.isEmpty() )
    return sourceAvailability;
  else
    // we don't know... source has features, but these may be filtered out by invalid geometry check or filter expression
    return Qgis::FeatureAvailability::FeaturesMaybeAvailable;
}

QgsFeatureIterator QgsProcessingFeatureSource::getFeatures( const QgsFeatureRequest &request ) const
{
  QgsFeatureRequest req( request );
  req.setInvalidGeometryCheck( mInvalidGeometryCheck );
  req.setInvalidGeometryCallback( mInvalidGeometryCallback );
  req.setTransformErrorCallback( mTransformErrorCallback );

  if ( mFeatureLimit != -1 && req.limit() != -1 )
    req.setLimit( std::min( static_cast< long long >( req.limit() ), mFeatureLimit ) );
  else if ( mFeatureLimit != -1 )
    req.setLimit( mFeatureLimit );

  if ( !mFilterExpression.isEmpty() )
    req.combineFilterExpression( mFilterExpression );

  return mSource->getFeatures( req );
}

QgsCoordinateReferenceSystem QgsProcessingFeatureSource::sourceCrs() const
{
  return mSourceCrs;
}

QgsFields QgsProcessingFeatureSource::fields() const
{
  return mSourceFields;
}

Qgis::WkbType QgsProcessingFeatureSource::wkbType() const
{
  return mSourceWkbType;
}

long long QgsProcessingFeatureSource::featureCount() const
{
  if ( !mFilterExpression.isEmpty() )
    return static_cast< int >( Qgis::FeatureCountState::UnknownCount );

  if ( mFeatureLimit == -1 )
    return mSource->featureCount();
  else
    return std::min( mFeatureLimit, mSource->featureCount() );
}

QString QgsProcessingFeatureSource::sourceName() const
{
  return mSourceName;
}

QSet<QVariant> QgsProcessingFeatureSource::uniqueValues( int fieldIndex, int limit ) const
{
  if ( mFilterExpression.isEmpty() )
    return mSource->uniqueValues( fieldIndex, limit );

  // inefficient method when filter expression in use
  // TODO QGIS 4.0 -- add filter expression to virtual ::uniqueValues function
  if ( fieldIndex < 0 || fieldIndex >= fields().count() )
    return QSet<QVariant>();

  QgsFeatureRequest req;
  req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  req.setSubsetOfAttributes( QgsAttributeList() << fieldIndex );
  req.setFilterExpression( mFilterExpression );

  QSet<QVariant> values;
  QgsFeatureIterator it = getFeatures( req );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    values.insert( f.attribute( fieldIndex ) );
    if ( limit > 0 && values.size() >= limit )
      return values;
  }
  return values;
}

QVariant QgsProcessingFeatureSource::minimumValue( int fieldIndex ) const
{
  if ( mFilterExpression.isEmpty() )
    return mSource->minimumValue( fieldIndex );

  // inefficient method when filter expression in use
  // TODO QGIS 4.0 -- add filter expression to virtual ::minimumValue function
  if ( fieldIndex < 0 || fieldIndex >= fields().count() )
    return QVariant();

  QgsFeatureRequest req;
  req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  req.setSubsetOfAttributes( QgsAttributeList() << fieldIndex );

  QVariant min;
  QgsFeatureIterator it = getFeatures( req );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    const QVariant v = f.attribute( fieldIndex );
    if ( !QgsVariantUtils::isNull( v ) && ( qgsVariantLessThan( v, min ) || QgsVariantUtils::isNull( min ) ) )
    {
      min = v;
    }
  }
  return min;
}

QVariant QgsProcessingFeatureSource::maximumValue( int fieldIndex ) const
{
  if ( mFilterExpression.isEmpty() )
    return mSource->maximumValue( fieldIndex );

  // inefficient method when filter expression in use
  // TODO QGIS 4.0 -- add filter expression to virtual ::maximumValue function
  if ( fieldIndex < 0 || fieldIndex >= fields().count() )
    return QVariant();

  QgsFeatureRequest req;
  req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  req.setSubsetOfAttributes( QgsAttributeList() << fieldIndex );

  QVariant max;
  QgsFeatureIterator it = getFeatures( req );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    const QVariant v = f.attribute( fieldIndex );
    if ( !QgsVariantUtils::isNull( v ) && ( qgsVariantGreaterThan( v, max ) || QgsVariantUtils::isNull( max ) ) )
    {
      max = v;
    }
  }
  return max;
}

QgsRectangle QgsProcessingFeatureSource::sourceExtent() const
{
  return mSourceExtent;
}

QgsFeatureIds QgsProcessingFeatureSource::allFeatureIds() const
{
  if ( mFilterExpression.isEmpty() )
    return mSource->allFeatureIds();

  QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                        .setFlags( Qgis::FeatureRequestFlag::NoGeometry )
                                        .setNoAttributes()
                                        .setFilterExpression( mFilterExpression ) );

  QgsFeatureIds ids;

  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    ids << fet.id();
  }

  return ids;
}

Qgis::SpatialIndexPresence QgsProcessingFeatureSource::hasSpatialIndex() const
{
  return mSourceSpatialIndexPresence;
}

QgsExpressionContextScope *QgsProcessingFeatureSource::createExpressionContextScope() const
{
  QgsExpressionContextScope *expressionContextScope = nullptr;
  QgsExpressionContextScopeGenerator *generator = dynamic_cast<QgsExpressionContextScopeGenerator *>( mSource );
  if ( generator )
  {
    expressionContextScope = generator->createExpressionContextScope();
  }
  return expressionContextScope;
}

void QgsProcessingFeatureSource::setInvalidGeometryCheck( Qgis::InvalidGeometryCheck method )
{
  mInvalidGeometryCheck = method;
  switch ( mInvalidGeometryCheck )
  {
    case Qgis::InvalidGeometryCheck::NoCheck:
      mInvalidGeometryCallback = nullptr;
      break;

    case Qgis::InvalidGeometryCheck::SkipInvalid:
      mInvalidGeometryCallback = mInvalidGeometryCallbackSkip;
      break;

    case Qgis::InvalidGeometryCheck::AbortOnInvalid:
      mInvalidGeometryCallback = mInvalidGeometryCallbackAbort;
      break;

  }
}

Qgis::InvalidGeometryCheck QgsProcessingFeatureSource::invalidGeometryCheck() const
{
  return mInvalidGeometryCheck;
}


//
// QgsProcessingFeatureSink
//
QgsProcessingFeatureSink::QgsProcessingFeatureSink( QgsFeatureSink *originalSink, const QString &sinkName, QgsProcessingContext &context, bool ownsOriginalSink )
  : QgsProxyFeatureSink( originalSink )
  , mContext( context )
  , mSinkName( sinkName )
  , mOwnsSink( ownsOriginalSink )
{}

QgsProcessingFeatureSink::~QgsProcessingFeatureSink()
{
  if ( mOwnsSink )
    delete destinationSink();
}

bool QgsProcessingFeatureSink::addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags )
{
  bool result = QgsProxyFeatureSink::addFeature( feature, flags );
  if ( !result && mContext.feedback() )
  {
    const QString error = lastError();
    if ( !error.isEmpty() )
      mContext.feedback()->reportError( QObject::tr( "Feature could not be written to %1: %2" ).arg( mSinkName, error ) );
    else
      mContext.feedback()->reportError( QObject::tr( "Feature could not be written to %1" ).arg( mSinkName ) );
  }
  return result;
}

bool QgsProcessingFeatureSink::addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags )
{
  bool result = QgsProxyFeatureSink::addFeatures( features, flags );
  if ( !result && mContext.feedback() )
  {
    const QString error = lastError();
    if ( !error.isEmpty() )
      mContext.feedback()->reportError( QObject::tr( "%n feature(s) could not be written to %1: %2", nullptr, features.count() ).arg( mSinkName, error ) );
    else
      mContext.feedback()->reportError( QObject::tr( "%n feature(s) could not be written to %1", nullptr, features.count() ).arg( mSinkName ) );
  }
  return result;
}

bool QgsProcessingFeatureSink::addFeatures( QgsFeatureIterator &iterator, QgsFeatureSink::Flags flags )
{
  bool result = QgsProxyFeatureSink::addFeatures( iterator, flags );
  if ( !result && mContext.feedback() )
  {
    const QString error = lastError();
    if ( !error.isEmpty() )
      mContext.feedback()->reportError( QObject::tr( "Features could not be written to %1: %2" ).arg( mSinkName, error ) );
    else
      mContext.feedback()->reportError( QObject::tr( "Features could not be written to %1" ).arg( mSinkName ) );
  }
  return result;
}
