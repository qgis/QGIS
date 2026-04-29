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
#include <QStringList>

namespace
{
  QJsonObject schemaObject( const QJsonObject &properties, const QJsonArray &required = QJsonArray() )
  {
    QJsonObject schema;
    schema.insert( QStringLiteral( "type" ), QStringLiteral( "object" ) );
    schema.insert( QStringLiteral( "properties" ), properties );
    if ( !required.isEmpty() )
      schema.insert( QStringLiteral( "required" ), required );
    return schema;
  }

  QJsonObject prop( const QString &type, const QString &description )
  {
    QJsonObject p;
    p.insert( QStringLiteral( "type" ), type );
    p.insert( QStringLiteral( "description" ), description );
    return p;
  }
}

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
  properties.insert( QStringLiteral( "path" ), prop( QStringLiteral( "string" ), QStringLiteral( "Workspace-relative or absolute path to read." ) ) );
  properties.insert( QStringLiteral( "start_line" ), prop( QStringLiteral( "integer" ), QStringLiteral( "Optional 1-based first line to include." ) ) );
  properties.insert( QStringLiteral( "end_line" ), prop( QStringLiteral( "integer" ), QStringLiteral( "Optional 1-based last line to include (inclusive)." ) ) );
  return schemaObject( properties, QJsonArray { QStringLiteral( "path" ) } );
}

QgsAiToolResult QgsAiReadFileTool::execute( const QJsonObject &args )
{
  if ( !mContextProvider )
    return QgsAiToolResult::error( QStringLiteral( "File context provider not available." ) );

  const QString path = args.value( QStringLiteral( "path" ) ).toString().trimmed();
  if ( path.isEmpty() )
    return QgsAiToolResult::error( QStringLiteral( "Argument 'path' is required." ) );

  // 128 KB read cap: enough to cover most source files, prevents accidentally feeding huge data.
  const int maxBytes = 131072;
  const QgsAiFileContext context = mContextProvider->buildContext( path, QString(), maxBytes, /*allowExternal=*/false );
  if ( context.filePath.isEmpty() )
    return QgsAiToolResult::error( QStringLiteral( "Path is outside the workspace, missing, or unreadable: %1" ).arg( path ) );
  if ( context.binary )
    return QgsAiToolResult::error( QStringLiteral( "Refusing to return content of binary file: %1" ).arg( path ) );

  // Compose response
  QString content = context.fileSnippet;

  // Optional line range. start_line=1 means line index 0.
  const int startLine = args.value( QStringLiteral( "start_line" ) ).toInt( 0 );
  const int endLine = args.value( QStringLiteral( "end_line" ) ).toInt( 0 );
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
  output.insert( QStringLiteral( "path" ), relative );
  output.insert( QStringLiteral( "absolute_path" ), context.filePath );
  output.insert( QStringLiteral( "size_bytes" ), static_cast<qint64>( context.fileSize ) );
  output.insert( QStringLiteral( "truncated" ), context.truncated );
  output.insert( QStringLiteral( "content" ), content );
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
  properties.insert( QStringLiteral( "query" ), prop( QStringLiteral( "string" ), QStringLiteral( "Substring to look for in file content." ) ) );
  properties.insert( QStringLiteral( "glob" ), prop( QStringLiteral( "string" ), QStringLiteral( "Optional substring filter on the relative path (case-insensitive)." ) ) );
  properties.insert( QStringLiteral( "max_results" ), prop( QStringLiteral( "integer" ), QStringLiteral( "Maximum number of matches to return (default 50, max 200)." ) ) );
  return schemaObject( properties, QJsonArray { QStringLiteral( "query" ) } );
}

QgsAiToolResult QgsAiSearchFilesTool::execute( const QJsonObject &args )
{
  if ( !mContextProvider )
    return QgsAiToolResult::error( QStringLiteral( "File context provider not available." ) );

  const QString query = args.value( QStringLiteral( "query" ) ).toString();
  if ( query.isEmpty() )
    return QgsAiToolResult::error( QStringLiteral( "Argument 'query' is required and must be non-empty." ) );

  const QString glob = args.value( QStringLiteral( "glob" ) ).toString().trimmed();
  const int requestedMax = args.value( QStringLiteral( "max_results" ) ).toInt( 50 );
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
      match.insert( QStringLiteral( "path" ), relative );
      match.insert( QStringLiteral( "line" ), lineNo );
      match.insert( QStringLiteral( "text" ), text );
      matches.push_back( match );
      ++collected;
      if ( collected >= maxResults )
        break;
    }
  }

  QJsonObject output;
  output.insert( QStringLiteral( "query" ), query );
  output.insert( QStringLiteral( "matches" ), matches );
  output.insert( QStringLiteral( "count" ), collected );
  output.insert( QStringLiteral( "truncated" ), collected >= maxResults );
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
  properties.insert( QStringLiteral( "glob" ), prop( QStringLiteral( "string" ), QStringLiteral( "Optional substring filter on the relative path (e.g. '.cpp')." ) ) );
  properties.insert( QStringLiteral( "max" ), prop( QStringLiteral( "integer" ), QStringLiteral( "Maximum number of paths to return (default 200, hard cap 2000)." ) ) );
  return schemaObject( properties );
}

QgsAiToolResult QgsAiListFilesTool::execute( const QJsonObject &args )
{
  if ( !mContextProvider )
    return QgsAiToolResult::error( QStringLiteral( "File context provider not available." ) );

  const QString glob = args.value( QStringLiteral( "glob" ) ).toString().trimmed();
  const int requestedMax = args.value( QStringLiteral( "max" ) ).toInt( 200 );
  const int maxResults = std::clamp( requestedMax, 1, 2000 );

  const QStringList files = mContextProvider->workspaceFileCandidates( glob, maxResults );

  QJsonArray array;
  for ( const QString &f : files )
    array.push_back( f );

  QJsonObject output;
  output.insert( QStringLiteral( "files" ), array );
  output.insert( QStringLiteral( "count" ), files.size() );
  output.insert( QStringLiteral( "truncated" ), files.size() >= maxResults );
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
    return QgsAiToolResult::error( QStringLiteral( "No active QgsProject available." ) );

  QJsonArray layers;
  const QMap<QString, QgsMapLayer *> projectLayers = project->mapLayers();
  for ( auto it = projectLayers.constBegin(); it != projectLayers.constEnd(); ++it )
  {
    QgsMapLayer *layer = it.value();
    if ( !layer )
      continue;

    QJsonObject entry;
    entry.insert( QStringLiteral( "id" ), layer->id() );
    entry.insert( QStringLiteral( "name" ), layer->name() );
    entry.insert( QStringLiteral( "type" ), QgsMapLayerFactory::typeToString( layer->type() ) );
    entry.insert( QStringLiteral( "crs" ), layer->crs().authid() );
    entry.insert( QStringLiteral( "source" ), layer->publicSource() );

    if ( QgsVectorLayer *vector = qobject_cast<QgsVectorLayer *>( layer ) )
    {
      entry.insert( QStringLiteral( "geometry_type" ), QgsWkbTypes::geometryDisplayString( vector->geometryType() ) );
      entry.insert( QStringLiteral( "feature_count" ), static_cast<qint64>( vector->featureCount() ) );
    }
    else if ( QgsRasterLayer *raster = qobject_cast<QgsRasterLayer *>( layer ) )
    {
      entry.insert( QStringLiteral( "width" ), raster->width() );
      entry.insert( QStringLiteral( "height" ), raster->height() );
      entry.insert( QStringLiteral( "bands" ), raster->bandCount() );
    }

    layers.push_back( entry );
  }

  QJsonObject output;
  output.insert( QStringLiteral( "project_file" ), project->fileName() );
  output.insert( QStringLiteral( "layer_count" ), layers.size() );
  output.insert( QStringLiteral( "layers" ), layers );
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
    return QgsAiToolResult::error( QStringLiteral( "No map canvas available." ) );

  const QgsRectangle extent = mCanvas->extent();
  const QgsCoordinateReferenceSystem crs = mCanvas->mapSettings().destinationCrs();

  QJsonObject extentJson;
  extentJson.insert( QStringLiteral( "xmin" ), extent.xMinimum() );
  extentJson.insert( QStringLiteral( "ymin" ), extent.yMinimum() );
  extentJson.insert( QStringLiteral( "xmax" ), extent.xMaximum() );
  extentJson.insert( QStringLiteral( "ymax" ), extent.yMaximum() );

  QJsonObject output;
  output.insert( QStringLiteral( "extent" ), extentJson );
  output.insert( QStringLiteral( "crs" ), crs.authid() );
  output.insert( QStringLiteral( "scale" ), mCanvas->scale() );
  return QgsAiToolResult::ok( output );
}
