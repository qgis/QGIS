#include "qgsaiindextools.h"

#include "qgsaiworkspaceindex.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>

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
// index_status
// ---------------------------------------------------------------------------

QgsAiIndexStatusTool::QgsAiIndexStatusTool( QgsAiWorkspaceIndex *index )
  : mIndex( index )
{}

QString QgsAiIndexStatusTool::description() const
{
  return QStringLiteral(
    "Returns whether the AI workspace index exists, how many files/chunks it "
    "covers, and when it was last refreshed. Call this before search_workspace "
    "to know whether retrieval is available."
  );
}

QJsonObject QgsAiIndexStatusTool::schema() const
{
  return schemaObject( QJsonObject() );
}

QgsAiToolResult QgsAiIndexStatusTool::execute( const QJsonObject &args )
{
  Q_UNUSED( args )
  if ( !mIndex )
    return QgsAiToolResult::error( QStringLiteral( "Workspace index is not available." ) );

  const QgsAiWorkspaceIndex::Status status = mIndex->status();
  QJsonObject output;
  output.insert( QStringLiteral( "indexed" ), status.indexed );
  output.insert( QStringLiteral( "file_count" ), status.fileCount );
  output.insert( QStringLiteral( "chunk_count" ), status.chunkCount );
  output.insert( QStringLiteral( "workspace_root" ), status.workspaceRoot );
  output.insert( QStringLiteral( "last_sync" ), status.lastSync.isValid() ? status.lastSync.toString( Qt::ISODate ) : QString() );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// search_workspace
// ---------------------------------------------------------------------------

QgsAiSearchWorkspaceTool::QgsAiSearchWorkspaceTool( QgsAiWorkspaceIndex *index )
  : mIndex( index )
{}

QString QgsAiSearchWorkspaceTool::description() const
{
  return QStringLiteral(
    "Semantic top-k retrieval over the indexed workspace. Embeds 'query' with "
    "OpenAI embeddings and returns the best-matching chunks as "
    "[{path, chunk_index, score, text}]. The model should typically use this "
    "to locate the right file before reading it with read_file. "
    "If the index is empty, the result will say so — call reindex_workspace first."
  );
}

QJsonObject QgsAiSearchWorkspaceTool::schema() const
{
  QJsonObject properties;
  properties.insert( QStringLiteral( "query" ), prop( QStringLiteral( "string" ), QStringLiteral( "Natural-language query to search for." ) ) );
  properties.insert( QStringLiteral( "k" ), prop( QStringLiteral( "integer" ), QStringLiteral( "Number of results to return (default 5, max 20)." ) ) );
  return schemaObject( properties, QJsonArray { QStringLiteral( "query" ) } );
}

QgsAiToolResult QgsAiSearchWorkspaceTool::execute( const QJsonObject &args )
{
  if ( !mIndex )
    return QgsAiToolResult::error( QStringLiteral( "Workspace index is not available." ) );

  const QString query = args.value( QStringLiteral( "query" ) ).toString();
  if ( query.trimmed().isEmpty() )
    return QgsAiToolResult::error( QStringLiteral( "Argument 'query' is required and must be non-empty." ) );

  const int k = args.value( QStringLiteral( "k" ) ).toInt( 5 );
  QString errorMessage;
  const QList<QgsAiWorkspaceIndex::Chunk> hits = mIndex->search( query, k, &errorMessage );
  if ( hits.isEmpty() && !errorMessage.isEmpty() )
    return QgsAiToolResult::error( errorMessage );

  QJsonArray array;
  for ( const QgsAiWorkspaceIndex::Chunk &c : hits )
  {
    QJsonObject entry;
    entry.insert( QStringLiteral( "path" ), c.relativePath );
    entry.insert( QStringLiteral( "chunk_index" ), c.chunkIndex );
    entry.insert( QStringLiteral( "score" ), static_cast<double>( c.score ) );
    // Cap text length per result so we don't blow the model's context window
    // when returning many hits.
    entry.insert( QStringLiteral( "text" ), c.text.left( 1500 ) );
    array.append( entry );
  }
  QJsonObject output;
  output.insert( QStringLiteral( "matches" ), array );
  output.insert( QStringLiteral( "count" ), array.size() );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// reindex_workspace
// ---------------------------------------------------------------------------

QgsAiReindexWorkspaceTool::QgsAiReindexWorkspaceTool( QgsAiWorkspaceIndex *index )
  : mIndex( index )
{}

QString QgsAiReindexWorkspaceTool::description() const
{
  return QStringLiteral(
    "Rebuilds the AI workspace retrieval index. Walks every text-like file in "
    "the workspace, chunks it, and sends each chunk to the OpenAI embeddings "
    "endpoint. The user's API key is used for billing; chunks are uploaded to "
    "OpenAI. The caller MUST set 'confirm: true' to acknowledge this. "
    "Optional 'max_files' caps how many files are processed (default 500)."
  );
}

QJsonObject QgsAiReindexWorkspaceTool::schema() const
{
  QJsonObject properties;
  properties.insert( QStringLiteral( "confirm" ), prop( QStringLiteral( "boolean" ), QStringLiteral( "Must be true to acknowledge that file chunks will be sent to OpenAI." ) ) );
  properties.insert( QStringLiteral( "max_files" ), prop( QStringLiteral( "integer" ), QStringLiteral( "Cap on indexed files (default 500, hard cap 5000)." ) ) );
  return schemaObject( properties, QJsonArray { QStringLiteral( "confirm" ) } );
}

QgsAiToolResult QgsAiReindexWorkspaceTool::execute( const QJsonObject &args )
{
  if ( !mIndex )
    return QgsAiToolResult::error( QStringLiteral( "Workspace index is not available." ) );

  if ( !args.value( QStringLiteral( "confirm" ) ).toBool( false ) )
    return QgsAiToolResult::error( QStringLiteral( "Refusing to reindex without explicit 'confirm: true' (chunks would be sent to OpenAI)." ) );

  const int requestedMax = args.value( QStringLiteral( "max_files" ) ).toInt( QgsAiWorkspaceIndex::DEFAULT_MAX_FILES );
  const int maxFiles = std::clamp( requestedMax, 1, 5000 );

  const qint64 startedAt = QDateTime::currentMSecsSinceEpoch();
  QString errorMessage;
  const bool ok = mIndex->reindex( maxFiles, &errorMessage );
  const qint64 durationMs = QDateTime::currentMSecsSinceEpoch() - startedAt;

  if ( !ok )
    return QgsAiToolResult::error( errorMessage.isEmpty() ? QStringLiteral( "Reindex failed." ) : errorMessage );

  const QgsAiWorkspaceIndex::Status status = mIndex->status();
  QJsonObject output;
  output.insert( QStringLiteral( "status" ), QStringLiteral( "ok" ) );
  output.insert( QStringLiteral( "file_count" ), status.fileCount );
  output.insert( QStringLiteral( "chunk_count" ), status.chunkCount );
  output.insert( QStringLiteral( "duration_ms" ), durationMs );
  return QgsAiToolResult::ok( output );
}
