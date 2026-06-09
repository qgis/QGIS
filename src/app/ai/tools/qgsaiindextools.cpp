/***************************************************************************
    qgsaiindextools.cpp
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

#include "qgsaiindextools.h"

#include "qgsaitoolschemautil.h"
#include "qgsaiworkspaceindex.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

using namespace Qt::StringLiterals;

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
    return QgsAiToolResult::error( u"Workspace index is not available."_s );

  mIndex->ensureLoaded();
  const QgsAiWorkspaceIndex::Status status = mIndex->status();
  QJsonObject output;
  output.insert( u"indexed"_s, status.indexed );
  output.insert( u"file_count"_s, status.fileCount );
  output.insert( u"chunk_count"_s, status.chunkCount );
  output.insert( u"file_chunk_count"_s, status.fileChunkCount );
  output.insert( u"layer_chunk_count"_s, status.layerChunkCount );
  output.insert( u"workspace_root"_s, status.workspaceRoot );
  output.insert( u"last_sync"_s, status.lastSync.isValid() ? status.lastSync.toString( Qt::ISODate ) : QString() );
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
    "the configured embeddings provider and returns the best-matching chunks as "
    "[{path, chunk_index, score, text}]. The model should typically use this "
    "to locate the right file before reading it with read_file. "
    "If the index is empty, the result will say so — call reindex_workspace first."
  );
}

QJsonObject QgsAiSearchWorkspaceTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"query"_s, prop( u"string"_s, u"Natural-language query to search for."_s ) );
  properties.insert( u"k"_s, prop( u"integer"_s, u"Number of results to return (default 5, max 20)."_s ) );
  return schemaObject( properties, QJsonArray { u"query"_s } );
}

QgsAiToolResult QgsAiSearchWorkspaceTool::execute( const QJsonObject &args )
{
  if ( !mIndex )
    return QgsAiToolResult::error( u"Workspace index is not available."_s );

  const QString query = args.value( u"query"_s ).toString();
  if ( query.trimmed().isEmpty() )
    return QgsAiToolResult::error( u"Argument 'query' is required and must be non-empty."_s );

  const int k = args.value( u"k"_s ).toInt( 5 );
  QString errorMessage;
  const QList<QgsAiWorkspaceIndex::Chunk> hits = mIndex->search( query, k, &errorMessage );
  if ( hits.isEmpty() && !errorMessage.isEmpty() )
    return QgsAiToolResult::error( errorMessage );

  QJsonArray array;
  for ( const QgsAiWorkspaceIndex::Chunk &c : hits )
  {
    QJsonObject entry;
    entry.insert( u"source_type"_s, c.sourceType );
    entry.insert( u"path"_s, c.relativePath );
    entry.insert( u"chunk_index"_s, c.chunkIndex );
    entry.insert( u"score"_s, static_cast<double>( c.score ) );
    if ( c.sourceType == QString::fromLatin1( QgsAiWorkspaceIndex::SOURCE_TYPE_LAYER ) )
    {
      entry.insert( u"layer_id"_s, c.layerId );
      entry.insert( u"feature_id_min"_s, c.firstFeatureId );
      entry.insert( u"feature_id_max"_s, c.lastFeatureId );
    }
    // Cap text length per result so we don't blow the model's context window
    // when returning many hits.
    entry.insert( u"text"_s, c.text.left( 1500 ) );
    array.append( entry );
  }
  QJsonObject output;
  output.insert( u"matches"_s, array );
  output.insert( u"count"_s, array.size() );
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
    "the workspace, chunks it, and sends each chunk to the configured embeddings "
    "endpoint. The user's API key is used for billing; chunks are uploaded to "
    "the configured provider. The caller MUST set 'confirm: true' to acknowledge this. "
    "Optional 'max_files' caps how many files are processed (default 500)."
  );
}

QJsonObject QgsAiReindexWorkspaceTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"confirm"_s, prop( u"boolean"_s, u"Must be true to acknowledge that file chunks will be sent to the configured embeddings provider."_s ) );
  properties.insert( u"max_files"_s, prop( u"integer"_s, u"Cap on indexed files (default 500, hard cap 5000)."_s ) );
  return schemaObject( properties, QJsonArray { u"confirm"_s } );
}

QgsAiToolResult QgsAiReindexWorkspaceTool::execute( const QJsonObject &args )
{
  if ( !mIndex )
    return QgsAiToolResult::error( u"Workspace index is not available."_s );

  if ( !args.value( u"confirm"_s ).toBool( false ) )
    return QgsAiToolResult::error( u"Refusing to reindex without explicit 'confirm: true' (chunks would be sent to the configured embeddings provider)."_s );

  const int requestedMax = args.value( u"max_files"_s ).toInt( QgsAiWorkspaceIndex::DEFAULT_MAX_FILES );
  const int maxFiles = std::clamp( requestedMax, 1, 5000 );

  const qint64 startedAt = QDateTime::currentMSecsSinceEpoch();
  QString errorMessage;
  const bool ok = mIndex->reindex( maxFiles, &errorMessage );
  const qint64 durationMs = QDateTime::currentMSecsSinceEpoch() - startedAt;

  if ( !ok )
    return QgsAiToolResult::error( errorMessage.isEmpty() ? u"Reindex failed."_s : errorMessage );

  const QgsAiWorkspaceIndex::Status status = mIndex->status();
  QJsonObject output;
  output.insert( u"status"_s, u"ok"_s );
  output.insert( u"file_count"_s, status.fileCount );
  output.insert( u"chunk_count"_s, status.chunkCount );
  output.insert( u"duration_ms"_s, durationMs );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// reindex_layers
// ---------------------------------------------------------------------------

QgsAiReindexLayersTool::QgsAiReindexLayersTool( QgsAiWorkspaceIndex *index )
  : mIndex( index )
{}

QString QgsAiReindexLayersTool::description() const
{
  return QStringLiteral(
    "Rebuilds the layer portion of the AI retrieval index. Walks every vector "
    "and raster layer in the active QgsProject, packs features into auto-sized "
    "chunks (attributes + bounding boxes), and embeds them via the configured "
    "embeddings endpoint. File chunks are preserved. The user's API key is used "
    "for billing; the data is uploaded to the configured provider. The caller MUST set "
    "'confirm: true' to acknowledge this."
  );
}

QJsonObject QgsAiReindexLayersTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"confirm"_s, prop( u"boolean"_s, u"Must be true to acknowledge that layer data will be sent to the configured embeddings provider."_s ) );
  return schemaObject( properties, QJsonArray { u"confirm"_s } );
}

QgsAiToolResult QgsAiReindexLayersTool::execute( const QJsonObject &args )
{
  if ( !mIndex )
    return QgsAiToolResult::error( u"Workspace index is not available."_s );

  if ( !args.value( u"confirm"_s ).toBool( false ) )
    return QgsAiToolResult::error( u"Refusing to reindex layers without explicit 'confirm: true' (data would be sent to the configured embeddings provider)."_s );

  const qint64 startedAt = QDateTime::currentMSecsSinceEpoch();
  QString errorMessage;
  const bool ok = mIndex->reindexLayers( &errorMessage );
  const qint64 durationMs = QDateTime::currentMSecsSinceEpoch() - startedAt;

  if ( !ok )
    return QgsAiToolResult::error( errorMessage.isEmpty() ? u"Layer reindex failed."_s : errorMessage );

  const QgsAiWorkspaceIndex::Status status = mIndex->status();
  QJsonObject output;
  output.insert( u"status"_s, u"ok"_s );
  output.insert( u"layer_chunk_count"_s, status.layerChunkCount );
  output.insert( u"file_chunk_count"_s, status.fileChunkCount );
  output.insert( u"duration_ms"_s, durationMs );
  return QgsAiToolResult::ok( output );
}
