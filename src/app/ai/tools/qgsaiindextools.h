#ifndef QGSAIINDEXTOOLS_H
#define QGSAIINDEXTOOLS_H

#include "qgis_app.h"
#include "qgsaitool.h"

class QgsAiWorkspaceIndex;

/**
 * index_status: read-only diagnostic tool. Returns whether the workspace index
 * exists, how many files/chunks it covers, and when it was last refreshed.
 */
class APP_EXPORT QgsAiIndexStatusTool : public QgsAiTool
{
  public:
    explicit QgsAiIndexStatusTool( QgsAiWorkspaceIndex *index );

    QString name() const override { return QStringLiteral( "index_status" ); }
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

    QString name() const override { return QStringLiteral( "search_workspace" ); }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsAiWorkspaceIndex *mIndex = nullptr;
};

/**
 * reindex_workspace: triggers a full rebuild of the workspace index.
 * Sends file chunks to the OpenAI embeddings endpoint, so the user must opt
 * in by setting `confirm: true` on the call. Bounded by `max_files` (default 500).
 *
 * The tool blocks the agent while indexing runs; the chat panel surfaces
 * progress updates via the index's `progress(...)` signal (wired in qgisapp.cpp).
 */
class APP_EXPORT QgsAiReindexWorkspaceTool : public QgsAiTool
{
  public:
    explicit QgsAiReindexWorkspaceTool( QgsAiWorkspaceIndex *index );

    QString name() const override { return QStringLiteral( "reindex_workspace" ); }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsAiWorkspaceIndex *mIndex = nullptr;
};

#endif // QGSAIINDEXTOOLS_H
