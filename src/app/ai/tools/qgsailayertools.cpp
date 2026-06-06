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

#include "qgsaifilecontextprovider.h"
#include "qgsaitoolschemautil.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsfields.h"
#include "qgslayertree.h"
#include "qgslayertreelayer.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerfactory.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsrectangle.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"
#include "qgswkbtypes.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
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
