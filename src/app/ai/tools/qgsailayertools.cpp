/***************************************************************************
    qgsailayertools.cpp
    ---------------------
    begin                : April 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsailayertools.h"

#include <algorithm>
#include <memory>

#include "qgsaifilecontextprovider.h"
#include "qgsaitoolschemautil.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsfields.h"
#include "qgslayertree.h"
#include "qgslayertreelayer.h"
#include "qgslayoutexporter.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutmanager.h"
#include "qgslayoutpoint.h"
#include "qgslayoutsize.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerfactory.h"
#include "qgsmaprendererparalleljob.h"
#include "qgsmapsettings.h"
#include "qgsmasterlayoutinterface.h"
#include "qgsprintlayout.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingfeedback.h"
#include "qgsprocessingoutputs.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingregistry.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsrectangle.h"
#include "qgsrenderer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbol.h"
#include "qgsvectorlayer.h"
#include "qgswkbtypes.h"

#include <QColor>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QVariant>

using namespace Qt::StringLiterals;

namespace
{
  // Vector and raster file extensions we auto-detect. Lowercase, no leading dot.
  const QSet<QString> &vectorExts()
  {
    static const QSet<QString> set { u"shp"_s, u"geojson"_s, u"json"_s, u"gpkg"_s, u"kml"_s, u"kmz"_s, u"tab"_s, u"mif"_s, u"gml"_s, u"gpx"_s, u"csv"_s, u"fgb"_s, u"sqlite"_s, u"db"_s };
    return set;
  }
  const QSet<QString> &rasterExts()
  {
    static const QSet<QString> set { u"tif"_s, u"tiff"_s, u"asc"_s, u"img"_s, u"jp2"_s, u"vrt"_s, u"nc"_s, u"hdf"_s, u"hgt"_s, u"ecw"_s, u"mbtiles"_s, u"dem"_s, u"bil"_s, u"png"_s, u"jpg"_s, u"jpeg"_s };
    return set;
  }

  QString detectKind( const QString &path )
  {
    const QString ext = QFileInfo( path ).suffix().toLower();
    if ( vectorExts().contains( ext ) )
      return u"vector"_s;
    if ( rasterExts().contains( ext ) )
      return u"raster"_s;
    return QString();
  }

  // Resolve a user-supplied path: try workspace first, then accept absolute path
  // if the file exists on disk. Returns empty QString if unresolvable.
  QString resolvePath( QgsAiFileContextProvider *provider, const QString &path )
  {
    if ( provider )
    {
      const QString resolved = provider->resolveWorkspaceFile( path );
      if ( !resolved.isEmpty() && QFileInfo::exists( resolved ) )
        return resolved;
    }
    const QFileInfo info( path );
    if ( info.exists() && info.isFile() )
      return info.absoluteFilePath();
    return QString();
  }

  QJsonObject extentJson( const QgsRectangle &extent )
  {
    QJsonObject e;
    e.insert( u"xmin"_s, extent.xMinimum() );
    e.insert( u"ymin"_s, extent.yMinimum() );
    e.insert( u"xmax"_s, extent.xMaximum() );
    e.insert( u"ymax"_s, extent.yMaximum() );
    return e;
  }

  QString layerTreePath( QgsLayerTreeLayer *node )
  {
    if ( !node )
      return QString();

    QStringList parts;
    QgsLayerTreeNode *current = node;
    while ( current )
    {
      if ( current->parent() && !current->name().isEmpty() )
        parts.prepend( current->name() );
      current = current->parent();
    }
    return u"/%1"_s.arg( parts.join( "/"_L1 ) );
  }

  QJsonObject layerVisualSummary( QgsMapLayer *layer, QgsLayerTreeLayer *node )
  {
    QJsonObject summary;
    if ( !layer )
      return summary;

    summary.insert( u"visible"_s, node ? node->isVisible() : true );
    summary.insert( u"checked"_s, node ? node->itemVisibilityChecked() : true );
    if ( node )
      summary.insert( u"tree_path"_s, layerTreePath( node ) );
    summary.insert( u"opacity"_s, layer->opacity() );
    summary.insert( u"scale_based_visibility"_s, layer->hasScaleBasedVisibility() );
    summary.insert( u"minimum_scale"_s, layer->minimumScale() );
    summary.insert( u"maximum_scale"_s, layer->maximumScale() );

    if ( QgsVectorLayer *vector = qobject_cast<QgsVectorLayer *>( layer ) )
    {
      summary.insert( u"renderer_type"_s, vector->renderer() ? vector->renderer()->type() : QString() );
      summary.insert( u"labels_enabled"_s, vector->labelsEnabled() && vector->labeling() );
    }
    else if ( QgsRasterLayer *raster = qobject_cast<QgsRasterLayer *>( layer ) )
    {
      summary.insert( u"renderer_type"_s, raster->renderer() ? raster->renderer()->type() : QString() );
      summary.insert( u"labels_enabled"_s, raster->labelsEnabled() && raster->labeling() );
    }
    else
    {
      summary.insert( u"labels_enabled"_s, false );
    }
    return summary;
  }

  QString uniqueLayoutName( QgsProject *project, const QString &requestedName )
  {
    const QString base = requestedName.trimmed().isEmpty() ? u"Strata Layout"_s : requestedName.trimmed();
    QgsLayoutManager *manager = project ? project->layoutManager() : nullptr;
    if ( !manager || !manager->layoutByName( base ) )
      return base;

    for ( int i = 2; i < 1000; ++i )
    {
      const QString candidate = u"%1 %2"_s.arg( base ).arg( i );
      if ( !manager->layoutByName( candidate ) )
        return candidate;
    }
    return u"%1 %2"_s.arg( base, QString::number( QDateTime::currentMSecsSinceEpoch() ) );
  }

  QVariantMap jsonObjectToVariantMap( const QJsonObject &object )
  {
    QVariantMap map;
    for ( auto it = object.constBegin(); it != object.constEnd(); ++it )
      map.insert( it.key(), it.value().toVariant() );
    return map;
  }

  QJsonArray processingParametersJson( const QgsProcessingAlgorithm *algorithm )
  {
    QJsonArray array;
    if ( !algorithm )
      return array;

    const QgsProcessingParameterDefinitions definitions = algorithm->parameterDefinitions();
    for ( const QgsProcessingParameterDefinition *definition : definitions )
    {
      if ( !definition )
        continue;

      QJsonObject entry;
      entry.insert( u"name"_s, definition->name() );
      entry.insert( u"description"_s, definition->description() );
      entry.insert( u"type"_s, definition->type() );
      entry.insert( u"optional"_s, definition->flags().testFlag( Qgis::ProcessingParameterFlag::Optional ) );
      entry.insert( u"destination"_s, definition->isDestination() );
      if ( definition->defaultValue().isValid() )
        entry.insert( u"default"_s, QJsonValue::fromVariant( definition->defaultValue() ) );
      array.push_back( entry );
    }
    return array;
  }

  QJsonArray processingOutputsJson( const QgsProcessingAlgorithm *algorithm )
  {
    QJsonArray array;
    if ( !algorithm )
      return array;

    const QgsProcessingOutputDefinitions definitions = algorithm->outputDefinitions();
    for ( const QgsProcessingOutputDefinition *definition : definitions )
    {
      if ( !definition )
        continue;
      QJsonObject entry;
      entry.insert( u"name"_s, definition->name() );
      entry.insert( u"description"_s, definition->description() );
      entry.insert( u"type"_s, definition->type() );
      array.push_back( entry );
    }
    return array;
  }

  QJsonObject processingAlgorithmMetadataJson( const QgsProcessingAlgorithm *algorithm )
  {
    QJsonObject output;
    if ( !algorithm )
      return output;
    output.insert( u"algorithm_id"_s, algorithm->id() );
    output.insert( u"name"_s, algorithm->name() );
    output.insert( u"display_name"_s, algorithm->displayName() );
    output.insert( u"group"_s, algorithm->group() );
    output.insert( u"group_id"_s, algorithm->groupId() );
    output.insert( u"parameters"_s, processingParametersJson( algorithm ) );
    output.insert( u"outputs"_s, processingOutputsJson( algorithm ) );
    return output;
  }

  QString exportResultMessage( QgsLayoutExporter::ExportResult result )
  {
    switch ( result )
    {
      case QgsLayoutExporter::Success:
        return u"success"_s;
      case QgsLayoutExporter::Canceled:
        return u"canceled"_s;
      case QgsLayoutExporter::MemoryError:
        return u"memory_error"_s;
      case QgsLayoutExporter::FileError:
        return u"file_error"_s;
      case QgsLayoutExporter::PrintError:
        return u"print_error"_s;
      case QgsLayoutExporter::SvgLayerError:
        return u"svg_layer_error"_s;
      case QgsLayoutExporter::IteratorError:
        return u"iterator_error"_s;
    }
    return u"unknown_error"_s;
  }

  QgsPrintLayout *printLayoutByName( QgsProject *project, const QString &name )
  {
    if ( !project || name.trimmed().isEmpty() )
      return nullptr;
    QgsMasterLayoutInterface *layout = project->layoutManager() ? project->layoutManager()->layoutByName( name.trimmed() ) : nullptr;
    return dynamic_cast<QgsPrintLayout *>( layout );
  }

  QSize exportRenderSize( const QSize &requested, const QSize &fallback )
  {
    QSize size = requested.isValid() ? requested : fallback;
    if ( !size.isValid() || size.width() <= 0 || size.height() <= 0 )
      size = QSize( 1280, 720 );
    return QSize( std::clamp( size.width(), 1, 4096 ), std::clamp( size.height(), 1, 4096 ) );
  }
} //namespace

// ---------------------------------------------------------------------------
// add_layer_from_file
// ---------------------------------------------------------------------------

QgsAiAddLayerFromFileTool::QgsAiAddLayerFromFileTool( QgsAiFileContextProvider *contextProvider, QgsProject *project )
  : mContextProvider( contextProvider )
  , mProject( project )
{}

QString QgsAiAddLayerFromFileTool::description() const
{
  return QStringLiteral(
    "Loads a vector or raster file as a new map layer in the active QGIS project. "
    "Path may be workspace-relative or an absolute path to a file on disk. "
    "The layer kind is auto-detected from the file extension unless 'kind' is set explicitly. "
    "Returns the created layer's id, feature_count (vector) or band info (raster), CRS authid and extent. "
    "Use this instead of writing run_python with QgsVectorLayer/QgsRasterLayer when the user wants to "
    "load a single file — it's a single-shot, typed call with no Python quoting concerns."
  );
}

QJsonObject QgsAiAddLayerFromFileTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"path"_s, prop( u"string"_s, u"Workspace-relative or absolute path to the source file."_s ) );
  properties.insert( u"name"_s, prop( u"string"_s, u"Optional display name. Defaults to the file stem."_s ) );
  properties.insert( u"kind"_s, prop( u"string"_s, u"Optional. One of: 'vector', 'raster', 'auto'. Default 'auto' (detect by extension)."_s ) );
  return schemaObject( properties, QJsonArray { u"path"_s } );
}

QgsAiToolResult QgsAiAddLayerFromFileTool::execute( const QJsonObject &args )
{
  const QString rawPath = args.value( u"path"_s ).toString().trimmed();
  if ( rawPath.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'path' is required."_s );

  const QString path = resolvePath( mContextProvider, rawPath );
  if ( path.isEmpty() )
    return QgsAiToolResult::error( u"Cannot resolve path to an existing file: %1"_s.arg( rawPath ) );

  QgsProject *project = mProject ? mProject : QgsProject::instance();
  if ( !project )
    return QgsAiToolResult::error( u"No active QgsProject available."_s );

  QString kind = args.value( u"kind"_s ).toString().toLower().trimmed();
  if ( kind.isEmpty() || kind == "auto"_L1 )
    kind = detectKind( path );
  if ( kind.isEmpty() )
    return QgsAiToolResult::error( u"Cannot auto-detect layer kind from extension. Set 'kind' to 'vector' or 'raster'. Path: %1"_s.arg( path ) );

  const QString name = args.value( u"name"_s ).toString().trimmed().isEmpty() ? QFileInfo( path ).completeBaseName() : args.value( u"name"_s ).toString().trimmed();

  QgsMapLayer *added = nullptr;
  QJsonObject output;
  output.insert( u"kind"_s, kind );

  if ( kind == "vector"_L1 )
  {
    auto layer = std::make_unique<QgsVectorLayer>( path, name, u"ogr"_s );
    if ( !layer->isValid() )
      return QgsAiToolResult::error( u"Vector layer is invalid: %1 (provider error: %2)"_s.arg( path, layer->error().summary() ) );

    output.insert( u"feature_count"_s, static_cast<qint64>( layer->featureCount() ) );
    output.insert( u"geometry_type"_s, QgsWkbTypes::geometryDisplayString( layer->geometryType() ) );
    added = layer.release();
  }
  else if ( kind == "raster"_L1 )
  {
    auto layer = std::make_unique<QgsRasterLayer>( path, name, u"gdal"_s );
    if ( !layer->isValid() )
      return QgsAiToolResult::error( u"Raster layer is invalid: %1 (provider error: %2)"_s.arg( path, layer->error().summary() ) );

    output.insert( u"width"_s, layer->width() );
    output.insert( u"height"_s, layer->height() );
    output.insert( u"bands"_s, layer->bandCount() );
    added = layer.release();
  }
  else
  {
    return QgsAiToolResult::error( u"Unknown 'kind': %1 (expected 'vector' or 'raster')."_s.arg( kind ) );
  }

  project->addMapLayer( added );

  output.insert( u"layer_id"_s, added->id() );
  output.insert( u"name"_s, added->name() );
  output.insert( u"crs"_s, added->crs().authid() );
  output.insert( u"source"_s, added->publicSource() );
  output.insert( u"extent"_s, extentJson( added->extent() ) );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// describe_layer
// ---------------------------------------------------------------------------

QgsAiDescribeLayerTool::QgsAiDescribeLayerTool( QgsProject *project )
  : mProject( project )
{}

QString QgsAiDescribeLayerTool::description() const
{
  return QStringLiteral(
    "Describes a layer already loaded in the project. For vectors: id, name, geometry_type, "
    "feature_count, CRS, extent, fields[{name,type}], and (optionally) a small sample of "
    "feature attributes. For rasters: dimensions and band count. "
    "Use this BEFORE writing run_python that filters or aggregates by attribute — it lets you "
    "discover field names and types in one read-only call instead of two run_python rounds."
  );
}

QJsonObject QgsAiDescribeLayerTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"layer_id"_s, prop( u"string"_s, u"Id of the layer (as returned by list_project_layers / add_layer_from_file)."_s ) );
  properties.insert( u"sample_features"_s, prop( u"integer"_s, u"Optional number of feature attribute samples to include (default 0, hard cap 10)."_s ) );
  return schemaObject( properties, QJsonArray { u"layer_id"_s } );
}

QgsAiToolResult QgsAiDescribeLayerTool::execute( const QJsonObject &args )
{
  const QString layerId = args.value( u"layer_id"_s ).toString().trimmed();
  if ( layerId.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'layer_id' is required."_s );

  QgsProject *project = mProject ? mProject : QgsProject::instance();
  if ( !project )
    return QgsAiToolResult::error( u"No active QgsProject available."_s );

  QgsMapLayer *layer = project->mapLayer( layerId );
  if ( !layer )
    return QgsAiToolResult::error( u"No layer with id: %1"_s.arg( layerId ) );

  QgsLayerTree *root = project->layerTreeRoot();
  QgsLayerTreeLayer *treeNode = root ? root->findLayer( layerId ) : nullptr;

  const int requestedSamples = args.value( u"sample_features"_s ).toInt( 0 );
  const int maxSamples = std::clamp( requestedSamples, 0, 10 );

  QJsonObject output;
  output.insert( u"id"_s, layer->id() );
  output.insert( u"name"_s, layer->name() );
  output.insert( u"type"_s, QgsMapLayerFactory::typeToString( layer->type() ) );
  output.insert( u"crs"_s, layer->crs().authid() );
  output.insert( u"source"_s, layer->publicSource() );
  output.insert( u"extent"_s, extentJson( layer->extent() ) );
  output.insert( u"visual"_s, layerVisualSummary( layer, treeNode ) );

  if ( QgsVectorLayer *vector = qobject_cast<QgsVectorLayer *>( layer ) )
  {
    output.insert( u"geometry_type"_s, QgsWkbTypes::geometryDisplayString( vector->geometryType() ) );
    output.insert( u"feature_count"_s, static_cast<qint64>( vector->featureCount() ) );

    QJsonArray fields;
    const QgsFields layerFields = vector->fields();
    for ( const QgsField &field : layerFields )
    {
      QJsonObject f;
      f.insert( u"name"_s, field.name() );
      f.insert( u"type"_s, field.typeName() );
      fields.push_back( f );
    }
    output.insert( u"fields"_s, fields );

    if ( maxSamples > 0 )
    {
      QJsonArray samples;
      QgsFeatureRequest request;
      request.setLimit( maxSamples );
      request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
      QgsFeatureIterator it = vector->getFeatures( request );
      QgsFeature feature;
      while ( it.nextFeature( feature ) && samples.size() < maxSamples )
      {
        QJsonObject row;
        for ( const QgsField &field : layerFields )
        {
          const QVariant value = feature.attribute( field.name() );
          row.insert( field.name(), QJsonValue::fromVariant( value ) );
        }
        samples.push_back( row );
      }
      output.insert( u"sample_features"_s, samples );
    }
  }
  else if ( QgsRasterLayer *raster = qobject_cast<QgsRasterLayer *>( layer ) )
  {
    output.insert( u"width"_s, raster->width() );
    output.insert( u"height"_s, raster->height() );
    output.insert( u"bands"_s, raster->bandCount() );
  }

  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// run_processing_algorithm
// ---------------------------------------------------------------------------

QgsAiRunProcessingAlgorithmTool::QgsAiRunProcessingAlgorithmTool( QgsProject *project )
  : mProject( project )
{}

QString QgsAiRunProcessingAlgorithmTool::description() const
{
  return QStringLiteral(
    "Runs a QGIS Processing algorithm by id with JSON parameters, or returns a dry-run schema for the algorithm. "
    "Use this native tool for common GIS transformations (buffers, dissolve, reprojection, statistics, raster tools) "
    "instead of generating run_python code. Mutating or output-producing runs require approval."
  );
}

QJsonObject QgsAiRunProcessingAlgorithmTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"algorithm_id"_s, prop( u"string"_s, u"QGIS Processing algorithm id, e.g. 'native:buffer'."_s ) );
  properties.insert( u"parameters"_s, prop( u"object"_s, u"Algorithm parameters as a JSON object. Use dry_run first if unsure."_s ) );
  properties.insert( u"configuration"_s, prop( u"object"_s, u"Optional Processing algorithm configuration map."_s ) );
  properties.insert( u"dry_run"_s, prop( u"boolean"_s, u"If true, returns algorithm parameter/output metadata without executing it."_s ) );
  return schemaObject( properties, QJsonArray { u"algorithm_id"_s } );
}

bool QgsAiRunProcessingAlgorithmTool::isAvailable() const
{
  return QgsApplication::processingRegistry();
}

QString QgsAiRunProcessingAlgorithmTool::availabilityReason() const
{
  return u"QGIS Processing registry is not available."_s;
}

QgsAiToolResult QgsAiRunProcessingAlgorithmTool::execute( const QJsonObject &args )
{
  QgsProcessingRegistry *registry = QgsApplication::processingRegistry();
  if ( !registry )
    return QgsAiToolResult::error( availabilityReason() );

  const QString algorithmId = args.value( u"algorithm_id"_s ).toString().trimmed();
  if ( algorithmId.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'algorithm_id' is required."_s );

  const QgsProcessingAlgorithm *algorithm = registry->algorithmById( algorithmId );
  if ( !algorithm )
    return QgsAiToolResult::error( u"Unknown Processing algorithm: %1"_s.arg( algorithmId ) );

  QString canExecuteMessage;
  if ( !algorithm->canExecute( &canExecuteMessage ) )
    return QgsAiToolResult::error( canExecuteMessage.isEmpty() ? u"Processing algorithm cannot execute."_s : canExecuteMessage );

  if ( args.value( u"dry_run"_s ).toBool( false ) )
  {
    QJsonObject output = processingAlgorithmMetadataJson( algorithm );
    output.insert( u"dry_run"_s, true );
    return QgsAiToolResult::ok( output );
  }

  if ( !args.value( u"parameters"_s ).isObject() )
    return QgsAiToolResult::error( u"Argument 'parameters' must be an object. Call with dry_run=true to inspect required parameters."_s );

  QgsProject *project = mProject ? mProject : QgsProject::instance();
  QgsProcessingContext context;
  if ( project )
    context.setProject( project );
  QgsProcessingFeedback feedback;
  context.setFeedback( &feedback );

  const QVariantMap parameters = jsonObjectToVariantMap( args.value( u"parameters"_s ).toObject() );
  const QVariantMap configuration = args.value( u"configuration"_s ).isObject() ? jsonObjectToVariantMap( args.value( u"configuration"_s ).toObject() ) : QVariantMap();

  QString checkMessage;
  if ( !algorithm->checkParameterValues( parameters, context, &checkMessage ) )
    return QgsAiToolResult::error( checkMessage.isEmpty() ? u"Processing parameter validation failed."_s : checkMessage );

  bool ok = false;
  const QVariantMap result = algorithm->run( parameters, context, &feedback, &ok, configuration, true );
  if ( !ok )
    return QgsAiToolResult::error( u"Processing algorithm failed: %1"_s.arg( feedback.textLog().trimmed() ) );

  QJsonObject output = processingAlgorithmMetadataJson( algorithm );
  output.insert( u"dry_run"_s, false );
  output.insert( u"result"_s, QJsonObject::fromVariantMap( result ) );
  output.insert( u"log"_s, feedback.textLog() );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// style_layer
// ---------------------------------------------------------------------------

QgsAiStyleLayerTool::QgsAiStyleLayerTool( QgsProject *project )
  : mProject( project )
{}

QString QgsAiStyleLayerTool::description() const
{
  return QStringLiteral(
    "Applies common native styling changes to a loaded layer: opacity, visibility and single-symbol vector color. "
    "Use this instead of run_python for simple layer styling. It requires approval because it mutates the project."
  );
}

QJsonObject QgsAiStyleLayerTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"layer_id"_s, prop( u"string"_s, u"Target layer id."_s ) );
  properties.insert( u"opacity"_s, prop( u"number"_s, u"Optional opacity between 0 and 1."_s ) );
  properties.insert( u"visible"_s, prop( u"boolean"_s, u"Optional layer tree visibility."_s ) );
  properties.insert( u"color"_s, prop( u"string"_s, u"Optional vector single-symbol color, e.g. '#2F80ED'."_s ) );
  return schemaObject( properties, QJsonArray { u"layer_id"_s } );
}

QgsAiToolResult QgsAiStyleLayerTool::execute( const QJsonObject &args )
{
  const QString layerId = args.value( u"layer_id"_s ).toString().trimmed();
  if ( layerId.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'layer_id' is required."_s );

  QgsProject *project = mProject ? mProject : QgsProject::instance();
  if ( !project )
    return QgsAiToolResult::error( u"No active QgsProject available."_s );

  QgsMapLayer *layer = project->mapLayer( layerId );
  if ( !layer )
    return QgsAiToolResult::error( u"No layer with id: %1"_s.arg( layerId ) );

  if ( args.contains( u"opacity"_s ) )
  {
    const double opacity = std::clamp( args.value( u"opacity"_s ).toDouble( layer->opacity() ), 0.0, 1.0 );
    layer->setOpacity( opacity );
  }

  QgsLayerTreeLayer *treeNode = project->layerTreeRoot() ? project->layerTreeRoot()->findLayer( layerId ) : nullptr;
  if ( treeNode && args.contains( u"visible"_s ) )
    treeNode->setItemVisibilityChecked( args.value( u"visible"_s ).toBool() );

  if ( args.contains( u"color"_s ) )
  {
    QgsVectorLayer *vector = qobject_cast<QgsVectorLayer *>( layer );
    if ( !vector )
      return QgsAiToolResult::error( u"Color styling is currently supported for vector layers only."_s );

    const QColor color( args.value( u"color"_s ).toString().trimmed() );
    if ( !color.isValid() )
      return QgsAiToolResult::error( u"Invalid color. Use a CSS-style color such as '#2F80ED'."_s );

    std::unique_ptr<QgsSymbol> symbol( QgsSymbol::defaultSymbol( vector->geometryType() ) );
    if ( !symbol )
      return QgsAiToolResult::error( u"Could not create a default symbol for the layer geometry."_s );

    symbol->setColor( color );
    vector->setRenderer( new QgsSingleSymbolRenderer( symbol.release() ) );
  }

  layer->triggerRepaint();
  if ( QgsVectorLayer *vector = qobject_cast<QgsVectorLayer *>( layer ) )
    vector->emitStyleChanged();

  QJsonObject output;
  output.insert( u"layer_id"_s, layer->id() );
  output.insert( u"name"_s, layer->name() );
  output.insert( u"visual"_s, layerVisualSummary( layer, treeNode ) );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// create_print_layout
// ---------------------------------------------------------------------------

QgsAiCreatePrintLayoutTool::QgsAiCreatePrintLayoutTool( QgsProject *project, QgsMapCanvas *canvas )
  : mProject( project )
  , mCanvas( canvas )
{}

QString QgsAiCreatePrintLayoutTool::description() const
{
  return QStringLiteral(
    "Creates a simple native QGIS print layout with a map frame and optional title. "
    "The map uses the current canvas extent and rendered layers when a canvas is available. "
    "Use this before export_map when the user asks for a report/map layout."
  );
}

QJsonObject QgsAiCreatePrintLayoutTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"name"_s, prop( u"string"_s, u"Optional layout name. Defaults to 'Strata Layout' and is made unique."_s ) );
  properties.insert( u"title"_s, prop( u"string"_s, u"Optional title label to place above the map."_s ) );
  return schemaObject( properties );
}

QgsAiToolResult QgsAiCreatePrintLayoutTool::execute( const QJsonObject &args )
{
  QgsProject *project = mProject ? mProject : QgsProject::instance();
  if ( !project )
    return QgsAiToolResult::error( u"No active QgsProject available."_s );
  if ( !project->layoutManager() )
    return QgsAiToolResult::error( u"No layout manager available."_s );

  const QString layoutName = uniqueLayoutName( project, args.value( u"name"_s ).toString() );
  const QString title = args.value( u"title"_s ).toString().trimmed();

  auto layout = std::make_unique<QgsPrintLayout>( project );
  layout->initializeDefaults();
  layout->setName( layoutName );

  const double titleHeight = title.isEmpty() ? 0.0 : 15.0;
  QgsLayoutItemMap *map = new QgsLayoutItemMap( layout.get() );
  map->attemptMove( QgsLayoutPoint( 10, 10 + titleHeight ) );
  map->attemptResize( QgsLayoutSize( 190, 277 - titleHeight ) );
  if ( mCanvas )
  {
    map->setExtent( mCanvas->extent() );
    map->setLayers( mCanvas->mapSettings().layers( true ) );
    map->setKeepLayerSet( true );
  }
  else if ( project->layerTreeRoot() )
  {
    map->setLayers( project->layerTreeRoot()->layerOrder() );
    map->setKeepLayerSet( true );
  }
  layout->addLayoutItem( map );
  layout->setReferenceMap( map );

  if ( !title.isEmpty() )
  {
    QgsLayoutItemLabel *label = new QgsLayoutItemLabel( layout.get() );
    label->setText( title );
    label->attemptMove( QgsLayoutPoint( 10, 8 ) );
    label->attemptResize( QgsLayoutSize( 190, 12 ) );
    label->adjustSizeToText();
    layout->addLayoutItem( label );
  }

  if ( !project->layoutManager()->addLayout( layout.release() ) )
    return QgsAiToolResult::error( u"Could not add print layout to the project."_s );

  QJsonObject output;
  output.insert( u"layout_name"_s, layoutName );
  output.insert( u"title"_s, title );
  output.insert( u"has_map"_s, true );
  output.insert( u"uses_canvas_extent"_s, mCanvas != nullptr );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// export_map
// ---------------------------------------------------------------------------

QgsAiExportMapTool::QgsAiExportMapTool( QgsAiFileContextProvider *contextProvider, QgsProject *project, QgsMapCanvas *canvas )
  : mContextProvider( contextProvider )
  , mProject( project )
  , mCanvas( canvas )
{}

QString QgsAiExportMapTool::description() const
{
  return QStringLiteral(
    "Exports a named print layout to PDF/PNG, or the current map canvas to PNG, writing inside the workspace. "
    "Use create_print_layout first when a layout does not already exist. This requires approval because it writes files."
  );
}

QJsonObject QgsAiExportMapTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"path"_s, prop( u"string"_s, u"Workspace-relative output path, e.g. 'exports/map.pdf' or 'exports/canvas.png'."_s ) );
  properties.insert( u"format"_s, prop( u"string"_s, u"Optional output format: 'pdf' or 'png'. Defaults from path extension, then png."_s ) );
  properties.insert( u"layout_name"_s, prop( u"string"_s, u"Optional print layout name. If omitted, exports the current canvas to PNG."_s ) );
  properties.insert( u"width"_s, prop( u"integer"_s, u"Optional canvas export width in pixels, max 4096."_s ) );
  properties.insert( u"height"_s, prop( u"integer"_s, u"Optional canvas export height in pixels, max 4096."_s ) );
  properties.insert( u"dpi"_s, prop( u"integer"_s, u"Optional layout export DPI. Default 300."_s ) );
  return schemaObject( properties, QJsonArray { u"path"_s } );
}

QgsAiToolResult QgsAiExportMapTool::execute( const QJsonObject &args )
{
  if ( !mContextProvider )
    return QgsAiToolResult::error( u"File context provider not available."_s );

  const QString rawPath = args.value( u"path"_s ).toString().trimmed();
  if ( rawPath.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'path' is required."_s );

  const QString outputPath = mContextProvider->normalizePath( rawPath, /*allowExternal=*/false );
  if ( outputPath.isEmpty() )
    return QgsAiToolResult::error( u"Output path must be inside the workspace: %1"_s.arg( rawPath ) );

  QString format = args.value( u"format"_s ).toString().trimmed().toLower();
  if ( format.isEmpty() )
    format = QFileInfo( outputPath ).suffix().toLower();
  if ( format.isEmpty() )
    format = u"png"_s;
  if ( format != "png"_L1 && format != "pdf"_L1 )
    return QgsAiToolResult::error( u"Unsupported export format '%1'. Use 'png' or 'pdf'."_s.arg( format ) );

  if ( !QDir().mkpath( QFileInfo( outputPath ).absolutePath() ) )
    return QgsAiToolResult::error( u"Could not create output directory: %1"_s.arg( QFileInfo( outputPath ).absolutePath() ) );

  QgsProject *project = mProject ? mProject : QgsProject::instance();
  const QString layoutName = args.value( u"layout_name"_s ).toString().trimmed();
  if ( !layoutName.isEmpty() )
  {
    QgsPrintLayout *layout = printLayoutByName( project, layoutName );
    if ( !layout )
      return QgsAiToolResult::error( u"No print layout named: %1"_s.arg( layoutName ) );

    QgsLayoutExporter exporter( layout );
    QgsLayoutExporter::ExportResult result = QgsLayoutExporter::Success;
    const int dpi = std::clamp( args.value( u"dpi"_s ).toInt( 300 ), 36, 1200 );
    if ( format == "pdf"_L1 )
    {
      QgsLayoutExporter::PdfExportSettings settings;
      settings.dpi = dpi;
      result = exporter.exportToPdf( outputPath, settings );
    }
    else
    {
      QgsLayoutExporter::ImageExportSettings settings;
      settings.dpi = dpi;
      result = exporter.exportToImage( outputPath, settings );
    }

    if ( result != QgsLayoutExporter::Success )
      return QgsAiToolResult::error( u"Layout export failed: %1"_s.arg( exportResultMessage( result ) ) );
  }
  else
  {
    if ( format == "pdf"_L1 )
      return QgsAiToolResult::error( u"Canvas export supports PNG only. Create a print layout first for PDF export."_s );
    if ( !mCanvas )
      return QgsAiToolResult::error( u"No map canvas available."_s );

    QgsMapSettings settings = mCanvas->mapSettings();
    settings.setDevicePixelRatio( 1.0 );
    settings.setExtent( mCanvas->extent() );
    const QSize requestedSize( args.value( u"width"_s ).toInt( 0 ), args.value( u"height"_s ).toInt( 0 ) );
    settings.setOutputSize( exportRenderSize( requestedSize, settings.outputSize().isValid() ? settings.outputSize() : mCanvas->size() ) );

    QgsMapRendererParallelJob job( settings );
    job.start();
    job.waitForFinished();
    const QImage image = job.renderedImage();
    if ( image.isNull() )
      return QgsAiToolResult::error( u"Map canvas render produced an empty image."_s );
    if ( !image.save( outputPath, "PNG" ) )
      return QgsAiToolResult::error( u"Failed to save map export: %1"_s.arg( outputPath ) );
  }

  QJsonObject output;
  output.insert( u"path"_s, mContextProvider->workspaceRoot().isEmpty() ? outputPath : QDir( mContextProvider->workspaceRoot() ).relativeFilePath( outputPath ) );
  output.insert( u"absolute_path"_s, outputPath );
  output.insert( u"format"_s, format );
  output.insert( u"layout_name"_s, layoutName );
  output.insert( u"size_bytes"_s, static_cast<qint64>( QFileInfo( outputPath ).size() ) );
  return QgsAiToolResult::ok( output );
}
