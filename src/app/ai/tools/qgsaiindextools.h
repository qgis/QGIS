/***************************************************************************
    qgsaiindextools.h
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

#ifndef QGSAIINDEXTOOLS_H
#define QGSAIINDEXTOOLS_H

#include "qgis_app.h"
#include "qgsaitool.h"

#include <QString>

using namespace Qt::StringLiterals;

class QgsAiWorkspaceIndex;

/**
 * index_status: read-only diagnostic tool. Returns whether the workspace index
 * exists, how many files/chunks it covers, and when it was last refreshed.
 */
class APP_EXPORT QgsAiIndexStatusTool : public QgsAiTool
{
  public:
    explicit QgsAiIndexStatusTool( QgsAiWorkspaceIndex *index );

    QString name() const override { return u"index_status"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsAiWorkspaceIndex *mIndex = nullptr;
};

/**
 * search_workspace: semantic top-k retrieval over the indexed workspace.
 * Returns chunks {path, chunk_index, score, text} for the best matches.
 */
class APP_EXPORT QgsAiSearchWorkspaceTool : public QgsAiTool
{
  public:
    explicit QgsAiSearchWorkspaceTool( QgsAiWorkspaceIndex *index );

    QString name() const override { return u"search_workspace"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsAiWorkspaceIndex *mIndex = nullptr;
};

/**
 * reindex_workspace: triggers a full rebuild of the workspace index.
 * Uses the local embedding model, so the user must opt in by setting
 * `confirm: true` on the call. Bounded by `max_files` (default 500).
 *
 * The tool blocks the agent while indexing runs; the chat panel surfaces
 * progress updates via the index's `progress(...)` signal (wired in qgisapp.cpp).
 */
class APP_EXPORT QgsAiReindexWorkspaceTool : public QgsAiTool
{
  public:
    explicit QgsAiReindexWorkspaceTool( QgsAiWorkspaceIndex *index );

    QString name() const override { return u"reindex_workspace"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsAiWorkspaceIndex *mIndex = nullptr;
};

/**
 * reindex_layers: triggers a full rebuild of the *layer* portion of the index
 * (every QgsVectorLayer + QgsRasterLayer in the active QgsProject). File chunks
 * are preserved. Uses layer attributes + bounding boxes with the local embedding
 * model, so the user must opt in by setting `confirm: true`.
 */
class APP_EXPORT QgsAiReindexLayersTool : public QgsAiTool
{
  public:
    explicit QgsAiReindexLayersTool( QgsAiWorkspaceIndex *index );

    QString name() const override { return u"reindex_layers"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsAiWorkspaceIndex *mIndex = nullptr;
};

#endif // QGSAIINDEXTOOLS_H
