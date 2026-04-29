#include "qgsaireadtools.h"

#include "qgsaifilecontextprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerfactory.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrectangle.h"
#include "qgsvectorlayer.h"
#include "qgswkbtypes.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

using namespace Qt::StringLiterals;

namespace
{
  QJsonObject schemaObject( const QJsonObject &properties, const QJsonArray &required = QJsonArray() )
  {
    QJsonObject schema;
    schema.insert( u"type"_s, u"object"_s );
    schema.insert( u"properties"_s, properties );
    if ( !required.isEmpty() )
      schema.insert( u"required"_s, required );
    return schema;
  }

  QJsonObject prop( const QString &type, const QString &description )
  {
    QJsonObject p;
    p.insert( u"type"_s, type );
    p.insert( u"description"_s, description );
    return p;
  }
} //namespace

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
    const int to = endLine > 0 ? std::min( lines.size(), endLine ) : lines.size();
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

    QJsonObject entry;
    entry.insert( u"id"_s, layer->id() );
    entry.insert( u"name"_s, layer->name() );
    entry.insert( u"type"_s, QgsMapLayerFactory::typeToString( layer->type() ) );
    entry.insert( u"crs"_s, layer->crs().authid() );
    entry.insert( u"source"_s, layer->publicSource() );

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
  return QgsAiToolResult::ok( output );
}
