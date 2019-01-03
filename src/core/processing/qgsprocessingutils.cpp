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
#include "qgssettings.h"
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

QList<QgsRasterLayer *> QgsProcessingUtils::compatibleRasterLayers( QgsProject *project, bool sort )
{
  if ( !project )
    return QList<QgsRasterLayer *>();

  QList<QgsRasterLayer *> layers;

  const auto rasterLayers = project->layers<QgsRasterLayer *>();
  for ( QgsRasterLayer *l : rasterLayers )
  {
    if ( canUseLayer( l ) )
      layers << l;
  }

  if ( sort )
  {
    std::sort( layers.begin(), layers.end(), []( const QgsRasterLayer * a, const QgsRasterLayer * b ) -> bool
    {
      return QString::localeAwareCompare( a->name(), b->name() ) < 0;
    } );
  }
  return layers;
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
  if ( !project )
    return QList<QgsMeshLayer *>();

  QList<QgsMeshLayer *> layers;
  const auto meshLayers = project->layers<QgsMeshLayer *>();
  for ( QgsMeshLayer *l : meshLayers )
  {
    if ( canUseLayer( l ) )
      layers << l;
  }

  if ( sort )
  {
    std::sort( layers.begin(), layers.end(), []( const QgsMeshLayer * a, const QgsMeshLayer * b ) -> bool
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

  const auto rasterLayers = compatibleRasterLayers( project, false );
  for ( QgsRasterLayer *rl : rasterLayers )
    layers << rl;

  const auto vectorLayers = compatibleVectorLayers( project, QList< int >(), false );
  for ( QgsVectorLayer *vl : vectorLayers )
    layers << vl;

  const auto meshLayers = compatibleMeshLayers( project, false );
  for ( QgsMeshLayer *vl : meshLayers )
    layers << vl;

  if ( sort )
  {
    std::sort( layers.begin(), layers.end(), []( const QgsMapLayer * a, const QgsMapLayer * b ) -> bool
    {
      return QString::localeAwareCompare( a->name(), b->name() ) < 0;
    } );
  }
  return layers;
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
      case QgsMapLayer::VectorLayer:
        return !canUseLayer( qobject_cast< QgsVectorLayer * >( layer ) );
      case QgsMapLayer::RasterLayer:
        return !canUseLayer( qobject_cast< QgsRasterLayer * >( layer ) );
      case QgsMapLayer::PluginLayer:
        return true;
      case QgsMapLayer::MeshLayer:
        return !canUseLayer( qobject_cast< QgsMeshLayer * >( layer ) );
    }
    return true;
  } ), layers.end() );

  auto isCompatibleType = [typeHint]( QgsMapLayer * l ) -> bool
  {
    switch ( typeHint )
    {
      case UnknownType:
        return true;

      case Vector:
        return l->type() == QgsMapLayer::VectorLayer;

      case Raster:
        return l->type() == QgsMapLayer::RasterLayer;

      case Mesh:
        return l->type() == QgsMapLayer::MeshLayer;
    }
    return true;
  };

  for ( QgsMapLayer *l : qgis::as_const( layers ) )
  {
    if ( isCompatibleType( l ) && l->id() == string )
      return l;
  }
  for ( QgsMapLayer *l : qgis::as_const( layers ) )
  {
    if ( isCompatibleType( l ) && l->name() == string )
      return l;
  }
  for ( QgsMapLayer *l : qgis::as_const( layers ) )
  {
    if ( isCompatibleType( l ) && normalizeLayerSource( l->source() ) == normalizeLayerSource( string ) )
      return l;
  }
  return nullptr;
}

///@cond PRIVATE
class ProjectionSettingRestorer
{
  public:

    ProjectionSettingRestorer()
    {
      QgsSettings settings;
      previousSetting = settings.value( QStringLiteral( "/Projections/defaultBehavior" ) ).toString();
      settings.setValue( QStringLiteral( "/Projections/defaultBehavior" ), QStringLiteral( "useProject" ) );
    }

    ~ProjectionSettingRestorer()
    {
      QgsSettings settings;
      settings.setValue( QStringLiteral( "/Projections/defaultBehavior" ), previousSetting );
    }

    QString previousSetting;
};
///@endcond PRIVATE

QgsMapLayer *QgsProcessingUtils::loadMapLayerFromString( const QString &string, LayerHint typeHint )
{
  QStringList components = string.split( '|' );
  if ( components.isEmpty() )
    return nullptr;

  QFileInfo fi;
  if ( QFileInfo::exists( string ) )
    fi = QFileInfo( string );
  else if ( QFileInfo::exists( components.at( 0 ) ) )
    fi = QFileInfo( components.at( 0 ) );
  else
    return nullptr;

  // TODO - remove when there is a cleaner way to block the unknown projection dialog!
  ProjectionSettingRestorer restorer;
  ( void )restorer; // no warnings

  QString name = fi.baseName();

  // brute force attempt to load a matching layer
  if ( typeHint == UnknownType || typeHint == Vector )
  {
    QgsVectorLayer::LayerOptions options;
    options.loadDefaultStyle = false;
    std::unique_ptr< QgsVectorLayer > layer( new QgsVectorLayer( string, name, QStringLiteral( "ogr" ), options ) );
    if ( layer->isValid() )
    {
      return layer.release();
    }
  }
  if ( typeHint == UnknownType || typeHint == Raster )
  {
    QgsRasterLayer::LayerOptions rasterOptions;
    rasterOptions.loadDefaultStyle = false;
    std::unique_ptr< QgsRasterLayer > rasterLayer( new QgsRasterLayer( string, name, QStringLiteral( "gdal" ), rasterOptions ) );
    if ( rasterLayer->isValid() )
    {
      return rasterLayer.release();
    }
  }
  if ( typeHint == UnknownType || typeHint == Mesh )
  {
    QgsMeshLayer::LayerOptions meshOptions;
    std::unique_ptr< QgsMeshLayer > meshLayer( new QgsMeshLayer( string, name, QStringLiteral( "mdal" ), meshOptions ) );
    if ( meshLayer->isValid() )
    {
      return meshLayer.release();
    }
  }
  return nullptr;
}

QgsMapLayer *QgsProcessingUtils::mapLayerFromString( const QString &string, QgsProcessingContext &context, bool allowLoadingNewLayers, LayerHint typeHint )
{
  if ( string.isEmpty() )
    return nullptr;

  // prefer project layers
  QgsMapLayer *layer = nullptr;
  if ( context.project() )
  {
    QgsMapLayer *layer = mapLayerFromStore( string, context.project()->layerStore(), typeHint );
    if ( layer )
      return layer;
  }

  layer = mapLayerFromStore( string, context.temporaryLayerStore(), typeHint );
  if ( layer )
    return layer;

  if ( !allowLoadingNewLayers )
    return nullptr;

  layer = loadMapLayerFromString( string, typeHint );
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
  if ( val.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    selectedFeaturesOnly = fromVar.selectedFeaturesOnly;
    val = fromVar.source;
  }
  else if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition (e.g. an output from earlier in a model) - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( val ) ) )
  {
    return new QgsProcessingFeatureSource( layer, context );
  }

  QString layerRef;
  if ( val.canConvert<QgsProperty>() )
  {
    layerRef = val.value< QgsProperty >().valueAsString( context.expressionContext(), fallbackValue.toString() );
  }
  else if ( !val.isValid() || val.toString().isEmpty() )
  {
    // fall back to default
    if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( fallbackValue ) ) )
    {
      return new QgsProcessingFeatureSource( layer, context );
    }

    layerRef = fallbackValue.toString();
  }
  else
  {
    layerRef = val.toString();
  }

  if ( layerRef.isEmpty() )
    return nullptr;

  QgsVectorLayer *vl = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( layerRef, context, true, Vector ) );
  if ( !vl )
    return nullptr;

  if ( selectedFeaturesOnly )
  {
    return new QgsProcessingFeatureSource( new QgsVectorLayerSelectedFeatureSource( vl ), context, true );
  }
  else
  {
    return new QgsProcessingFeatureSource( vl, context );
  }
}

bool QgsProcessingUtils::canUseLayer( const QgsMeshLayer *layer )
{
  return layer && layer->dataProvider();
}

bool QgsProcessingUtils::canUseLayer( const QgsRasterLayer *layer )
{
  // only gdal file-based layers
  return layer && layer->providerType() == QStringLiteral( "gdal" );
}

bool QgsProcessingUtils::canUseLayer( const QgsVectorLayer *layer, const QList<int> &sourceTypes )
{
  return layer &&
         ( sourceTypes.isEmpty()
           || ( sourceTypes.contains( QgsProcessing::TypeVectorPoint ) && layer->geometryType() == QgsWkbTypes::PointGeometry )
           || ( sourceTypes.contains( QgsProcessing::TypeVectorLine ) && layer->geometryType() == QgsWkbTypes::LineGeometry )
           || ( sourceTypes.contains( QgsProcessing::TypeVectorPolygon ) && layer->geometryType() == QgsWkbTypes::PolygonGeometry )
           || ( sourceTypes.contains( QgsProcessing::TypeVectorAnyGeometry ) && layer->isSpatial() )
           || sourceTypes.contains( QgsProcessing::TypeVector )
         );
}

QString QgsProcessingUtils::normalizeLayerSource( const QString &source )
{
  QString normalized = source;
  normalized.replace( '\\', '/' );
  return normalized.trimmed();
}

QString QgsProcessingUtils::stringToPythonLiteral( const QString &string )
{
  QString s = string;
  s.replace( '\\', QStringLiteral( "\\\\" ) );
  s.replace( '\n', QStringLiteral( "\\n" ) );
  s.replace( '\r', QStringLiteral( "\\r" ) );
  s.replace( '\t', QStringLiteral( "\\t" ) );
  s.replace( '"', QStringLiteral( "\\\"" ) );
  s.replace( '\'', QStringLiteral( "\\\'" ) );
  s = s.prepend( '\'' ).append( '\'' );
  return s;
}

void QgsProcessingUtils::parseDestinationString( QString &destination, QString &providerKey, QString &uri, QString &layerName, QString &format, QMap<QString, QVariant> &options, bool &useWriter )
{
  QRegularExpression splitRx( QStringLiteral( "^(.{3,}?):(.*)$" ) );
  QRegularExpressionMatch match = splitRx.match( destination );
  if ( match.hasMatch() )
  {
    providerKey = match.captured( 1 );
    if ( providerKey == QStringLiteral( "postgis" ) ) // older processing used "postgis" instead of "postgres"
    {
      providerKey = QStringLiteral( "postgres" );
    }
    uri = match.captured( 2 );
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
        format = QgsVectorFileWriter::driverForExtension( QFileInfo( uri ).completeSuffix() );
      }
      options.insert( QStringLiteral( "update" ), true );
    }
    useWriter = false;
  }
  else
  {
    useWriter = true;
    providerKey = QStringLiteral( "ogr" );
    QRegularExpression splitRx( QStringLiteral( "^(.*)\\.(.*?)$" ) );
    QRegularExpressionMatch match = splitRx.match( destination );
    QString extension;
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

QgsFeatureSink *QgsProcessingUtils::createFeatureSink( QString &destination, QgsProcessingContext &context, const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs, const QVariantMap &createOptions, QgsFeatureSink::SinkFlags sinkFlags )
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
    std::unique_ptr< QgsVectorLayer > layer( QgsMemoryProviderUtils::createMemoryLayer( destination, fields, geometryType, crs ) );
    if ( !layer || !layer->isValid() )
    {
      throw QgsProcessingException( QObject::tr( "Could not create memory layer" ) );
    }

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
    bool useWriter = false;
    parseDestinationString( destination, providerKey, uri, layerName, format, options, useWriter );

    QgsFields newFields = fields;
    if ( useWriter && providerKey == QLatin1String( "ogr" ) )
    {
      // use QgsVectorFileWriter for OGR destinations instead of QgsVectorLayerImport, as that allows
      // us to use any OGR format which supports feature addition
      QString finalFileName;
      std::unique_ptr< QgsVectorFileWriter > writer = qgis::make_unique< QgsVectorFileWriter >( destination, options.value( QStringLiteral( "fileEncoding" ) ).toString(), newFields, geometryType, crs, format, QgsVectorFileWriter::defaultDatasetOptions( format ),
          QgsVectorFileWriter::defaultLayerOptions( format ), &finalFileName, QgsVectorFileWriter::NoSymbology, sinkFlags );

      if ( writer->hasError() )
      {
        throw QgsProcessingException( QObject::tr( "Could not create layer %1: %2" ).arg( destination, writer->errorMessage() ) );
      }
      destination = finalFileName;
      return new QgsProcessingFeatureSink( writer.release(), destination, context, true );
    }
    else
    {
      //create empty layer
      std::unique_ptr< QgsVectorLayerExporter > exporter( new QgsVectorLayerExporter( uri, providerKey, newFields, geometryType, crs, true, options, sinkFlags ) );
      if ( exporter->errorCode() )
      {
        throw QgsProcessingException( QObject::tr( "Could not create layer %1: %2" ).arg( destination, exporter->errorMessage() ) );
      }

      // use destination string as layer name (eg "postgis:..." )
      if ( !layerName.isEmpty() )
        uri += QStringLiteral( "|layername=%1" ).arg( layerName );
      std::unique_ptr< QgsVectorLayer > layer( new QgsVectorLayer( uri, destination, providerKey ) );
      // update destination to layer ID
      destination = layer->id();

      context.temporaryLayerStore()->addMapLayer( layer.release() );
      return new QgsProcessingFeatureSink( exporter.release(), destination, context, true );
    }
  }
  return nullptr;
}

void QgsProcessingUtils::createFeatureSinkPython( QgsFeatureSink **sink, QString &destination, QgsProcessingContext &context, const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs, const QVariantMap &options )
{
  *sink = createFeatureSink( destination, context, fields, geometryType, crs, options );
}


QgsRectangle QgsProcessingUtils::combineLayerExtents( const QList<QgsMapLayer *> &layers, const QgsCoordinateReferenceSystem &crs )
{
  QgsRectangle extent;
  for ( const QgsMapLayer *layer : layers )
  {
    if ( !layer )
      continue;

    if ( crs.isValid() )
    {
      //transform layer extent to target CRS
      Q_NOWARN_DEPRECATED_PUSH
      QgsCoordinateTransform ct( layer->crs(), crs );
      Q_NOWARN_DEPRECATED_POP
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

QVariant QgsProcessingUtils::generateIteratingDestination( const QVariant &input, const QVariant &id, QgsProcessingContext &context )
{
  if ( !input.isValid() )
    return QStringLiteral( "memory:%1" ).arg( id.toString() );

  if ( input.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( input );
    QVariant newSink = generateIteratingDestination( fromVar.sink, id, context );
    fromVar.sink = QgsProperty::fromValue( newSink );
    return fromVar;
  }
  else if ( input.canConvert<QgsProperty>() )
  {
    QString res = input.value< QgsProperty>().valueAsString( context.expressionContext() );
    return generateIteratingDestination( res, id, context );
  }
  else
  {
    QString res = input.toString();
    if ( res.startsWith( QLatin1String( "memory:" ) ) )
    {
      return res + '_' + id.toString();
    }
    else
    {
      // assume a filename type output for now
      // TODO - uris?
      int lastIndex = res.lastIndexOf( '.' );
      return res.left( lastIndex ) + '_' + id.toString() + res.mid( lastIndex );
    }
  }
}

QString QgsProcessingUtils::tempFolder()
{
  static QString sFolder;
  static QMutex sMutex;
  sMutex.lock();
  if ( sFolder.isEmpty() )
  {
    QString subPath = QUuid::createUuid().toString().remove( '-' ).remove( '{' ).remove( '}' );
    sFolder = QDir::tempPath() + QStringLiteral( "/processing_" ) + subPath;
    if ( !QDir( sFolder ).exists() )
      QDir().mkpath( sFolder );
  }
  sMutex.unlock();
  return sFolder;
}

QString QgsProcessingUtils::generateTempFilename( const QString &basename )
{
  QString subPath = QUuid::createUuid().toString().remove( '-' ).remove( '{' ).remove( '}' );
  QString path = tempFolder() + '/' + subPath;
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

  QString s = QObject::tr( "<html><body><h2>Algorithm description</h2>\n" );
  s += QStringLiteral( "<p>" ) + getText( QStringLiteral( "ALG_DESC" ) ) + QStringLiteral( "</p>\n" );

  QString inputs;

  const auto parameterDefinitions = algorithm->parameterDefinitions();
  for ( const QgsProcessingParameterDefinition *def : parameterDefinitions )
  {
    inputs += QStringLiteral( "<h3>" ) + def->description() + QStringLiteral( "</h3>\n" );
    inputs += QStringLiteral( "<p>" ) + getText( def->name() ) + QStringLiteral( "</p>\n" );
  }
  if ( !inputs.isEmpty() )
    s += QObject::tr( "<h2>Input parameters</h2>\n" ) + inputs;

  QString outputs;
  const auto outputDefinitions = algorithm->outputDefinitions();
  for ( const QgsProcessingOutputDefinition *def : outputDefinitions )
  {
    outputs += QStringLiteral( "<h3>" ) + def->description() + QStringLiteral( "</h3>\n" );
    outputs += QStringLiteral( "<p>" ) + getText( def->name() ) + QStringLiteral( "</p>\n" );
  }
  if ( !outputs.isEmpty() )
    s += QObject::tr( "<h2>Outputs</h2>\n" ) + outputs;

  s += QLatin1String( "<br>" );
  if ( !map.value( QStringLiteral( "ALG_CREATOR" ) ).toString().isEmpty() )
    s += QObject::tr( "<p align=\"right\">Algorithm author: %1</p>" ).arg( getText( QStringLiteral( "ALG_CREATOR" ) ) );
  if ( !map.value( QStringLiteral( "ALG_HELP_CREATOR" ) ).toString().isEmpty() )
    s += QObject::tr( "<p align=\"right\">Help author: %1</p>" ).arg( getText( QStringLiteral( "ALG_HELP_CREATOR" ) ) );
  if ( !map.value( QStringLiteral( "ALG_VERSION" ) ).toString().isEmpty() )
    s += QObject::tr( "<p align=\"right\">Algorithm version: %1</p>" ).arg( getText( QStringLiteral( "ALG_VERSION" ) ) );

  s += QStringLiteral( "</body></html>" );
  return s;
}

QString QgsProcessingUtils::convertToCompatibleFormat( const QgsVectorLayer *vl, bool selectedFeaturesOnly, const QString &baseName, const QStringList &compatibleFormats, const QString &preferredFormat, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  bool requiresTranslation = false;

  // if we are only looking for selected features then we have to export back to disk,
  // as we need to subset only selected features, a concept which doesn't exist outside QGIS!
  requiresTranslation = requiresTranslation || selectedFeaturesOnly;

  // if the data provider is NOT ogr, then we HAVE to convert. Otherwise we run into
  // issues with data providers like spatialite, delimited text where the format can be
  // opened outside of QGIS, but with potentially very different behavior!
  requiresTranslation = requiresTranslation || vl->dataProvider()->name() != QLatin1String( "ogr" );

  // if the layer has a feature filter set, then we HAVE to convert. Feature filters are
  // a purely QGIS concept.
  requiresTranslation = requiresTranslation || !vl->subsetString().isEmpty();

  // Check if layer is a disk based format and if so if the layer's path has a compatible filename suffix
  if ( !requiresTranslation )
  {
    const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( vl->dataProvider()->name(), vl->source() );
    if ( parts.contains( QLatin1String( "path" ) ) )
    {
      QFileInfo fi( parts.value( QLatin1String( "path" ) ).toString() );
      requiresTranslation = !compatibleFormats.contains( fi.suffix(), Qt::CaseInsensitive );
    }
    else
    {
      requiresTranslation = true; // not a disk-based format
    }
  }

  if ( requiresTranslation )
  {
    QString temp = QgsProcessingUtils::generateTempFilename( baseName + '.' + preferredFormat );

    QgsVectorFileWriter writer( temp, context.defaultEncoding(),
                                vl->fields(), vl->wkbType(), vl->crs(), QgsVectorFileWriter::driverForExtension( preferredFormat ) );
    QgsFeature f;
    QgsFeatureIterator it;
    if ( selectedFeaturesOnly )
      it = vl->getSelectedFeatures();
    else
      it = vl->getFeatures();

    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
        return QString();
      writer.addFeature( f, QgsFeatureSink::FastInsert );
    }
    return temp;
  }
  else
  {
    return vl->source();
  }
}

QgsFields QgsProcessingUtils::combineFields( const QgsFields &fieldsA, const QgsFields &fieldsB )
{
  QgsFields outFields = fieldsA;
  QSet< QString > usedNames;
  for ( const QgsField &f : fieldsA )
  {
    usedNames.insert( f.name().toLower() );
  }

  for ( const QgsField &f : fieldsB )
  {
    if ( usedNames.contains( f.name().toLower() ) )
    {
      int idx = 2;
      QString newName = f.name() + '_' + QString::number( idx );
      while ( usedNames.contains( newName.toLower() ) )
      {
        idx++;
        newName = f.name() + '_' + QString::number( idx );
      }
      QgsField newField = f;
      newField.setName( newName );
      outFields.append( newField );
      usedNames.insert( newName.toLower() );
    }
    else
    {
      usedNames.insert( f.name().toLower() );
      outFields.append( f );
    }
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

//
// QgsProcessingFeatureSource
//

QgsProcessingFeatureSource::QgsProcessingFeatureSource( QgsFeatureSource *originalSource, const QgsProcessingContext &context, bool ownsOriginalSource )
  : mSource( originalSource )
  , mOwnsSource( ownsOriginalSource )
  , mInvalidGeometryCheck( QgsWkbTypes::geometryType( mSource->wkbType() ) == QgsWkbTypes::PointGeometry
                           ? QgsFeatureRequest::GeometryNoCheck // never run geometry validity checks for point layers!
                           : context.invalidGeometryCheck() )
  , mInvalidGeometryCallback( context.invalidGeometryCallback() )
  , mTransformErrorCallback( context.transformErrorCallback() )
{}

QgsProcessingFeatureSource::~QgsProcessingFeatureSource()
{
  if ( mOwnsSource )
    delete mSource;
}

QgsFeatureIterator QgsProcessingFeatureSource::getFeatures( const QgsFeatureRequest &request, Flags flags ) const
{
  QgsFeatureRequest req( request );
  req.setTransformErrorCallback( mTransformErrorCallback );

  if ( flags & FlagSkipGeometryValidityChecks )
    req.setInvalidGeometryCheck( QgsFeatureRequest::GeometryNoCheck );
  else
  {
    req.setInvalidGeometryCheck( mInvalidGeometryCheck );
    req.setInvalidGeometryCallback( mInvalidGeometryCallback );
  }

  return mSource->getFeatures( req );
}

QgsFeatureSource::FeatureAvailability QgsProcessingFeatureSource::hasFeatures() const
{
  FeatureAvailability sourceAvailability = mSource->hasFeatures();
  if ( sourceAvailability == NoFeaturesAvailable )
    return NoFeaturesAvailable; // never going to be features if underlying source has no features
  else if ( mInvalidGeometryCheck == QgsFeatureRequest::GeometryNoCheck )
    return sourceAvailability;
  else
    // we don't know... source has features, but these may be filtered out by invalid geometry check
    return FeaturesMaybeAvailable;
}

QgsFeatureIterator QgsProcessingFeatureSource::getFeatures( const QgsFeatureRequest &request ) const
{
  QgsFeatureRequest req( request );
  req.setInvalidGeometryCheck( mInvalidGeometryCheck );
  req.setInvalidGeometryCallback( mInvalidGeometryCallback );
  req.setTransformErrorCallback( mTransformErrorCallback );
  return mSource->getFeatures( req );
}

QgsCoordinateReferenceSystem QgsProcessingFeatureSource::sourceCrs() const
{
  return mSource->sourceCrs();
}

QgsFields QgsProcessingFeatureSource::fields() const
{
  return mSource->fields();
}

QgsWkbTypes::Type QgsProcessingFeatureSource::wkbType() const
{
  return mSource->wkbType();
}

long QgsProcessingFeatureSource::featureCount() const
{
  return mSource->featureCount();
}

QString QgsProcessingFeatureSource::sourceName() const
{
  return mSource->sourceName();

}

QSet<QVariant> QgsProcessingFeatureSource::uniqueValues( int fieldIndex, int limit ) const
{
  return mSource->uniqueValues( fieldIndex, limit );
}

QVariant QgsProcessingFeatureSource::minimumValue( int fieldIndex ) const
{
  return mSource->minimumValue( fieldIndex );
}

QVariant QgsProcessingFeatureSource::maximumValue( int fieldIndex ) const
{
  return mSource->maximumValue( fieldIndex );
}

QgsRectangle QgsProcessingFeatureSource::sourceExtent() const
{
  return mSource->sourceExtent();
}

QgsFeatureIds QgsProcessingFeatureSource::allFeatureIds() const
{
  return mSource->allFeatureIds();
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
  if ( !result )
    mContext.feedback()->reportError( QObject::tr( "Feature could not be written to %1" ).arg( mSinkName ) );
  return result;
}

bool QgsProcessingFeatureSink::addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags )
{
  bool result = QgsProxyFeatureSink::addFeatures( features, flags );
  if ( !result )
    mContext.feedback()->reportError( QObject::tr( "%1 feature(s) could not be written to %2" ).arg( features.count() ).arg( mSinkName ) );
  return result;
}

bool QgsProcessingFeatureSink::addFeatures( QgsFeatureIterator &iterator, QgsFeatureSink::Flags flags )
{
  bool result = !QgsProxyFeatureSink::addFeatures( iterator, flags );
  if ( !result )
    mContext.feedback()->reportError( QObject::tr( "Features could not be written to %1" ).arg( mSinkName ) );
  return result;
}
