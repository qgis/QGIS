/***************************************************************************
    qgsaireadtools.cpp
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

#include "qgsaireadtools.h"

#include <algorithm>
#include <cmath>

#include "qgsaifilecontextprovider.h"
#include "qgsaitoolschemautil.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslayertree.h"
#include "qgslayertreelayer.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerfactory.h"
#include "qgsmaprendererparalleljob.h"
#include "qgsmapsettings.h"
#include "qgsmeshlayer.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsrectangle.h"
#include "qgsrenderer.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortilerenderer.h"
#include "qgswkbtypes.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QUuid>

using namespace Qt::StringLiterals;

namespace
{
  QString visualConsentSettingKey()
  {
    return u"strata/visual_context/image_send_consent"_s;
  }

  bool hasVisualConsent( QgsSettings &settings )
  {
    if ( settings.contains( visualConsentSettingKey() ) )
      return settings.value( visualConsentSettingKey(), false ).toBool();
    return settings.value( u"geoai/visual_context/image_send_consent"_s, false ).toBool();
  }

  QJsonObject readToolsExtentJson( const QgsRectangle &extent )
  {
    QJsonObject e;
    e.insert( u"xmin"_s, extent.xMinimum() );
    e.insert( u"ymin"_s, extent.yMinimum() );
    e.insert( u"xmax"_s, extent.xMaximum() );
    e.insert( u"ymax"_s, extent.yMaximum() );
    return e;
  }

  QString readToolsLayerTreePath( QgsLayerTreeLayer *node )
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

  QJsonObject layerVisualSummary( QgsMapLayer *layer, QgsLayerTreeLayer *node = nullptr, int renderOrder = -1, double scale = 0 )
  {
    QJsonObject summary;
    if ( !layer )
      return summary;

    summary.insert( u"visible"_s, node ? node->isVisible() : true );
    summary.insert( u"checked"_s, node ? node->itemVisibilityChecked() : true );
    if ( node )
      summary.insert( u"tree_path"_s, readToolsLayerTreePath( node ) );
    if ( renderOrder >= 0 )
      summary.insert( u"render_order"_s, renderOrder );

    summary.insert( u"opacity"_s, layer->opacity() );
    summary.insert( u"scale_based_visibility"_s, layer->hasScaleBasedVisibility() );
    summary.insert( u"minimum_scale"_s, layer->minimumScale() );
    summary.insert( u"maximum_scale"_s, layer->maximumScale() );
    if ( scale > 0 )
      summary.insert( u"in_scale_range"_s, layer->isInScaleRange( scale ) );

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
    else if ( QgsVectorTileLayer *vectorTile = qobject_cast<QgsVectorTileLayer *>( layer ) )
    {
      summary.insert( u"renderer_type"_s, vectorTile->renderer() ? vectorTile->renderer()->type() : QString() );
      summary.insert( u"labels_enabled"_s, vectorTile->labelsEnabled() && vectorTile->labeling() );
    }
    else if ( QgsMeshLayer *mesh = qobject_cast<QgsMeshLayer *>( layer ) )
    {
      summary.insert( u"labels_enabled"_s, mesh->labelsEnabled() && mesh->labeling() );
    }
    else
    {
      summary.insert( u"labels_enabled"_s, false );
    }

    return summary;
  }

  QJsonArray layerRefsJson( const QList<QgsMapLayer *> &layers )
  {
    QJsonArray array;
    for ( int i = 0; i < layers.size(); ++i )
    {
      QgsMapLayer *layer = layers.at( i );
      if ( !layer )
        continue;
      QJsonObject entry;
      entry.insert( u"id"_s, layer->id() );
      entry.insert( u"name"_s, layer->name() );
      entry.insert( u"type"_s, QgsMapLayerFactory::typeToString( layer->type() ) );
      entry.insert( u"render_order"_s, i );
      array.push_back( entry );
    }
    return array;
  }

  QString visualContextDirectory()
  {
    const QString dir = QgsApplication::qgisSettingsDirPath() + u"ai_visual_context"_s;
    QDir().mkpath( dir );
    return dir;
  }

  void cleanupOldVisualContextFiles( const QString &dirPath )
  {
    QDir dir( dirPath );
    const QFileInfoList files = dir.entryInfoList( QStringList() << u"*.png"_s, QDir::Files, QDir::Time );
    const QDateTime now = QDateTime::currentDateTime();
    for ( const QFileInfo &info : files )
    {
      if ( info.lastModified().secsTo( now ) > 24 * 60 * 60 )
        QFile::remove( info.absoluteFilePath() );
    }
  }

  bool ensureVisualContextConsent( QWidget *parent )
  {
    QgsSettings settings;
    if ( hasVisualConsent( settings ) )
      return true;

    if ( !parent )
      return false;

    const QMessageBox::StandardButton answer = QMessageBox::question(
      parent,
      QObject::tr( "Share map screenshot with AI" ),
      QObject::tr(
        "Strata can capture the current map canvas as an image and send it to vision-capable AI providers for this and future visual-context requests. The image may include visible map data, labels, "
        "and styles. Do you want to allow this?"
      ),
      QMessageBox::Yes | QMessageBox::No,
      QMessageBox::No
    );

    if ( answer != QMessageBox::Yes )
      return false;

    settings.setValue( visualConsentSettingKey(), true );
    settings.remove( u"geoai/visual_context/image_send_consent"_s );
    return true;
  }

  QSize cappedRenderSize( QSize original, int requestedLongestSide )
  {
    if ( original.width() <= 0 || original.height() <= 0 )
      original = QSize( 1280, 720 );

    const int longestCap = std::clamp( requestedLongestSide <= 0 ? 1280 : requestedLongestSide, 1, 1600 );
    static constexpr double maxPixels = 2000000.0;
    const double longest = std::max( original.width(), original.height() );
    const double area = static_cast<double>( original.width() ) * original.height();
    double scale = std::min( 1.0, longestCap / longest );
    if ( area * scale * scale > maxPixels )
      scale = std::sqrt( maxPixels / area );

    return QSize( std::max( 1, static_cast<int>( std::round( original.width() * scale ) ) ), std::max( 1, static_cast<int>( std::round( original.height() * scale ) ) ) );
  }
} // namespace

// ---------------------------------------------------------------------------
// read_file
// ---------------------------------------------------------------------------

QgsAiReadFileTool::QgsAiReadFileTool( QgsAiFileContextProvider *contextProvider )
  : mContextProvider( contextProvider )
{}

QString QgsAiReadFileTool::description() const
{
  return QStringLiteral(
    "Reads a UTF-8 text file from the user's workspace and returns its content. "
    "Path may be relative to the workspace root or absolute (only paths inside the workspace are allowed). "
    "Optional start_line and end_line (1-based, inclusive) restrict the returned slice. "
    "Always read a file before proposing edits to it."
  );
}

QJsonObject QgsAiReadFileTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"path"_s, prop( u"string"_s, u"Workspace-relative or absolute path to read."_s ) );
  properties.insert( u"start_line"_s, prop( u"integer"_s, u"Optional 1-based first line to include."_s ) );
  properties.insert( u"end_line"_s, prop( u"integer"_s, u"Optional 1-based last line to include (inclusive)."_s ) );
  return schemaObject( properties, QJsonArray { u"path"_s } );
}

QgsAiToolResult QgsAiReadFileTool::execute( const QJsonObject &args )
{
  if ( !mContextProvider )
    return QgsAiToolResult::error( u"File context provider not available."_s );

  const QString path = args.value( u"path"_s ).toString().trimmed();
  if ( path.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'path' is required."_s );

  // 128 KB read cap: enough to cover most source files, prevents accidentally feeding huge data.
  const int maxBytes = 131072;
  const QgsAiFileContext context = mContextProvider->buildContext( path, QString(), maxBytes, /*allowExternal=*/false );
  if ( context.filePath.isEmpty() )
    return QgsAiToolResult::error( u"Path is outside the workspace, missing, or unreadable: %1"_s.arg( path ) );
  if ( context.binary )
    return QgsAiToolResult::error( u"Refusing to return content of binary file: %1"_s.arg( path ) );

  // Compose response
  QString content = context.fileSnippet;

  // Optional line range. start_line=1 means line index 0.
  const int startLine = args.value( u"start_line"_s ).toInt( 0 );
  const int endLine = args.value( u"end_line"_s ).toInt( 0 );
  if ( startLine > 0 || endLine > 0 )
  {
    const QStringList lines = content.split( '\n' );
    const int from = std::max( 1, startLine ) - 1;
    const int to = endLine > 0 ? std::min( static_cast<int>( lines.size() ), endLine ) : static_cast<int>( lines.size() );
    if ( from >= lines.size() )
      content = QString();
    else
      content = lines.mid( from, to - from ).join( '\n' );
  }

  const QString root = mContextProvider->workspaceRoot();
  const QString relative = root.isEmpty() ? context.filePath : QDir( root ).relativeFilePath( context.filePath );

  QJsonObject output;
  output.insert( u"path"_s, relative );
  output.insert( u"absolute_path"_s, context.filePath );
  output.insert( u"size_bytes"_s, static_cast<qint64>( context.fileSize ) );
  output.insert( u"truncated"_s, context.truncated );
  output.insert( u"content"_s, content );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// search_files
// ---------------------------------------------------------------------------

QgsAiSearchFilesTool::QgsAiSearchFilesTool( QgsAiFileContextProvider *contextProvider )
  : mContextProvider( contextProvider )
{}

QString QgsAiSearchFilesTool::description() const
{
  return QStringLiteral(
    "Case-insensitive substring search across the workspace. "
    "Returns up to max_results matches as objects with {path, line, text}. "
    "Optional 'glob' is a substring filter on the relative path (e.g., '.cpp', 'src/app/ai'). "
    "This is plain substring matching, not regex."
  );
}

QJsonObject QgsAiSearchFilesTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"query"_s, prop( u"string"_s, u"Substring to look for in file content."_s ) );
  properties.insert( u"glob"_s, prop( u"string"_s, u"Optional substring filter on the relative path (case-insensitive)."_s ) );
  properties.insert( u"max_results"_s, prop( u"integer"_s, u"Maximum number of matches to return (default 50, max 200)."_s ) );
  return schemaObject( properties, QJsonArray { u"query"_s } );
}

QgsAiToolResult QgsAiSearchFilesTool::execute( const QJsonObject &args )
{
  if ( !mContextProvider )
    return QgsAiToolResult::error( u"File context provider not available."_s );

  const QString query = args.value( u"query"_s ).toString();
  if ( query.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'query' is required and must be non-empty."_s );

  const QString glob = args.value( u"glob"_s ).toString().trimmed();
  const int requestedMax = args.value( u"max_results"_s ).toInt( 50 );
  const int maxResults = std::clamp( requestedMax, 1, 200 );

  // Walk all workspace candidates, filter by glob substring on relative path, then grep each file.
  // workspaceFileCandidates already excludes noise dirs (.git, build, external, …).
  const QStringList allFiles = mContextProvider->workspaceFileCandidates( glob, /*maxResults=*/5000 );

  QJsonArray matches;
  int collected = 0;
  for ( const QString &relative : allFiles )
  {
    if ( collected >= maxResults )
      break;
    const QStringList hits = mContextProvider->searchInFile( relative, query, maxResults - collected );
    for ( const QString &hit : hits )
    {
      // searchInFile returns "<line>:<text>" — split once.
      const int sep = hit.indexOf( ':' );
      if ( sep <= 0 )
        continue;
      const int lineNo = hit.left( sep ).toInt();
      const QString text = hit.mid( sep + 1 );
      QJsonObject match;
      match.insert( u"path"_s, relative );
      match.insert( u"line"_s, lineNo );
      match.insert( u"text"_s, text );
      matches.push_back( match );
      ++collected;
      if ( collected >= maxResults )
        break;
    }
  }

  QJsonObject output;
  output.insert( u"query"_s, query );
  output.insert( u"matches"_s, matches );
  output.insert( u"count"_s, collected );
  output.insert( u"truncated"_s, collected >= maxResults );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// list_files
// ---------------------------------------------------------------------------

QgsAiListFilesTool::QgsAiListFilesTool( QgsAiFileContextProvider *contextProvider )
  : mContextProvider( contextProvider )
{}

QString QgsAiListFilesTool::description() const
{
  return QStringLiteral(
    "Lists files in the workspace, optionally filtered by a substring on the relative path. "
    "Skips noisy folders (.git, build, external, vcpkg, i18n, tests/testdata)."
  );
}

QJsonObject QgsAiListFilesTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"glob"_s, prop( u"string"_s, u"Optional substring filter on the relative path (e.g. '.cpp')."_s ) );
  properties.insert( u"max"_s, prop( u"integer"_s, u"Maximum number of paths to return (default 200, hard cap 2000)."_s ) );
  return schemaObject( properties );
}

QgsAiToolResult QgsAiListFilesTool::execute( const QJsonObject &args )
{
  if ( !mContextProvider )
    return QgsAiToolResult::error( u"File context provider not available."_s );

  const QString glob = args.value( u"glob"_s ).toString().trimmed();
  const int requestedMax = args.value( u"max"_s ).toInt( 200 );
  const int maxResults = std::clamp( requestedMax, 1, 2000 );

  const QStringList files = mContextProvider->workspaceFileCandidates( glob, maxResults );

  QJsonArray array;
  for ( const QString &f : files )
    array.push_back( f );

  QJsonObject output;
  output.insert( u"files"_s, array );
  output.insert( u"count"_s, files.size() );
  output.insert( u"truncated"_s, files.size() >= maxResults );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// list_project_layers
// ---------------------------------------------------------------------------

QgsAiListProjectLayersTool::QgsAiListProjectLayersTool( QgsProject *project )
  : mProject( project )
{}

QString QgsAiListProjectLayersTool::description() const
{
  return QStringLiteral(
    "Returns the list of layers currently loaded in the active QGIS project: "
    "id, name, type (vector/raster/mesh/…), geometry type for vectors, CRS authid, "
    "source path or URI, and feature count for vectors. Use this when the user references "
    "a layer by name, or when an action needs to know the project state."
  );
}

QJsonObject QgsAiListProjectLayersTool::schema() const
{
  return schemaObject( QJsonObject() );
}

QgsAiToolResult QgsAiListProjectLayersTool::execute( const QJsonObject &args )
{
  Q_UNUSED( args )
  QgsProject *project = mProject ? mProject : QgsProject::instance();
  if ( !project )
    return QgsAiToolResult::error( u"No active QgsProject available."_s );

  QJsonArray layers;
  const QMap<QString, QgsMapLayer *> projectLayers = project->mapLayers();
  for ( auto it = projectLayers.constBegin(); it != projectLayers.constEnd(); ++it )
  {
    QgsMapLayer *layer = it.value();
    if ( !layer )
      continue;

    QgsLayerTree *root = project->layerTreeRoot();
    QgsLayerTreeLayer *treeNode = root ? root->findLayer( layer->id() ) : nullptr;
    int renderOrder = -1;
    if ( root )
    {
      const QList<QgsMapLayer *> order = root->layerOrder();
      renderOrder = order.indexOf( layer );
    }

    QJsonObject entry;
    entry.insert( u"id"_s, layer->id() );
    entry.insert( u"name"_s, layer->name() );
    entry.insert( u"type"_s, QgsMapLayerFactory::typeToString( layer->type() ) );
    entry.insert( u"crs"_s, layer->crs().authid() );
    entry.insert( u"source"_s, layer->publicSource() );
    entry.insert( u"visual"_s, layerVisualSummary( layer, treeNode, renderOrder ) );

    if ( QgsVectorLayer *vector = qobject_cast<QgsVectorLayer *>( layer ) )
    {
      entry.insert( u"geometry_type"_s, QgsWkbTypes::geometryDisplayString( vector->geometryType() ) );
      entry.insert( u"feature_count"_s, static_cast<qint64>( vector->featureCount() ) );
    }
    else if ( QgsRasterLayer *raster = qobject_cast<QgsRasterLayer *>( layer ) )
    {
      entry.insert( u"width"_s, raster->width() );
      entry.insert( u"height"_s, raster->height() );
      entry.insert( u"bands"_s, raster->bandCount() );
    }

    layers.push_back( entry );
  }

  QJsonObject output;
  output.insert( u"project_file"_s, project->fileName() );
  output.insert( u"layer_count"_s, layers.size() );
  output.insert( u"layers"_s, layers );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// get_active_canvas_extent
// ---------------------------------------------------------------------------

QgsAiGetCanvasExtentTool::QgsAiGetCanvasExtentTool( QgsMapCanvas *canvas )
  : mCanvas( canvas )
{}

QString QgsAiGetCanvasExtentTool::description() const
{
  return QStringLiteral(
    "Returns the visible extent of the main map canvas (xmin/ymin/xmax/ymax) and its "
    "destination CRS authid. Use this when you need to bound spatial queries to what "
    "the user is currently looking at."
  );
}

QJsonObject QgsAiGetCanvasExtentTool::schema() const
{
  return schemaObject( QJsonObject() );
}

QgsAiToolResult QgsAiGetCanvasExtentTool::execute( const QJsonObject &args )
{
  Q_UNUSED( args )
  if ( !mCanvas )
    return QgsAiToolResult::error( u"No map canvas available."_s );

  const QgsRectangle extent = mCanvas->extent();
  const QgsCoordinateReferenceSystem crs = mCanvas->mapSettings().destinationCrs();

  QJsonObject extentJson;
  extentJson.insert( u"xmin"_s, extent.xMinimum() );
  extentJson.insert( u"ymin"_s, extent.yMinimum() );
  extentJson.insert( u"xmax"_s, extent.xMaximum() );
  extentJson.insert( u"ymax"_s, extent.yMaximum() );

  QJsonObject output;
  output.insert( u"extent"_s, extentJson );
  output.insert( u"crs"_s, crs.authid() );
  output.insert( u"scale"_s, mCanvas->scale() );
  output.insert( u"rotation"_s, mCanvas->rotation() );
  output.insert( u"width"_s, mCanvas->mapSettings().outputSize().width() );
  output.insert( u"height"_s, mCanvas->mapSettings().outputSize().height() );
  output.insert( u"device_pixel_ratio"_s, mCanvas->mapSettings().devicePixelRatio() );
  output.insert( u"rendered_layers"_s, layerRefsJson( mCanvas->mapSettings().layers( true ) ) );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// capture_map_canvas
// ---------------------------------------------------------------------------

QgsAiCaptureMapCanvasTool::QgsAiCaptureMapCanvasTool( QgsMapCanvas *canvas, QWidget *consentParent )
  : mCanvas( canvas )
  , mConsentParent( consentParent )
{}

QString QgsAiCaptureMapCanvasTool::description() const
{
  return QStringLiteral(
    "Renders the current 2D QGIS map canvas offscreen to a temporary PNG and returns "
    "the image path plus canvas metadata. Use this only when the user asks you to inspect "
    "what is visually happening on the map. The first use requires explicit user consent "
    "because vision-capable providers may receive the screenshot."
  );
}

QJsonObject QgsAiCaptureMapCanvasTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"max_longest_side"_s, prop( u"integer"_s, u"Optional longest-side pixel cap for the rendered image (default 1280, hard cap 1600)."_s ) );
  return schemaObject( properties );
}

QgsAiToolResult QgsAiCaptureMapCanvasTool::execute( const QJsonObject &args )
{
  if ( !mCanvas )
    return QgsAiToolResult::error( u"No map canvas available."_s );

  if ( !ensureVisualContextConsent( mConsentParent ) )
    return QgsAiToolResult::error( u"Visual context screenshot was not sent because the user has not consented to sharing map images with AI providers."_s );

  QgsMapSettings settings = mCanvas->mapSettings();
  settings.setDevicePixelRatio( 1.0 );
  const QSize outputSize = cappedRenderSize( settings.outputSize().isValid() ? settings.outputSize() : mCanvas->size(), args.value( u"max_longest_side"_s ).toInt( 1280 ) );
  settings.setOutputSize( outputSize );
  settings.setExtent( mCanvas->extent() );

  QgsMapRendererParallelJob job( settings );
  job.start();
  job.waitForFinished();
  QImage image = job.renderedImage();
  if ( image.isNull() )
    return QgsAiToolResult::error( u"Map canvas render produced an empty image."_s );

  const QString dir = visualContextDirectory();
  cleanupOldVisualContextFiles( dir );
  const QString path = QDir( dir ).filePath( u"canvas_%1.png"_s.arg( QUuid::createUuid().toString( QUuid::WithoutBraces ) ) );
  if ( !image.save( path, "PNG" ) )
    return QgsAiToolResult::error( u"Failed to save visual context image: %1"_s.arg( path ) );

  QJsonObject imageJson;
  imageJson.insert( u"path"_s, path );
  imageJson.insert( u"mime_type"_s, u"image/png"_s );
  imageJson.insert( u"width"_s, image.width() );
  imageJson.insert( u"height"_s, image.height() );

  QJsonObject canvasJson;
  canvasJson.insert( u"extent"_s, readToolsExtentJson( settings.extent() ) );
  canvasJson.insert( u"visible_extent"_s, readToolsExtentJson( settings.visibleExtent() ) );
  canvasJson.insert( u"crs"_s, settings.destinationCrs().authid() );
  canvasJson.insert( u"scale"_s, settings.scale() );
  canvasJson.insert( u"rotation"_s, settings.rotation() );
  canvasJson.insert( u"width"_s, outputSize.width() );
  canvasJson.insert( u"height"_s, outputSize.height() );

  QJsonObject output;
  output.insert( u"image"_s, imageJson );
  output.insert( u"canvas"_s, canvasJson );
  output.insert( u"layers_rendered"_s, layerRefsJson( settings.layers( true ) ) );
  return QgsAiToolResult::ok( output );
}
