/***************************************************************************
    qgsaiworkspaceindex.h
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

#ifndef QGSAIWORKSPACEINDEX_H
#define QGSAIWORKSPACEINDEX_H

#include "qgis_app.h"

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <QVector>

class QgsAiEmbeddingProvider;
class QgsAiFileContextProvider;

/**
 * Lightweight retrieval index over the user's workspace. The model uses this
 * via the search_workspace / index_status / reindex_workspace tools to ground
 * its answers on actual project content (RAG).
 *
 * MVP design (intentionally simple):
 *
 * - Plain SQLite database stored under qgisSettingsDirPath()/ai_index/, one
 *   file per workspace (hashed root path).
 * - One row per text chunk: { file_path, chunk_index, text, embedding BLOB }.
 * - Embeddings come from the configured local embeddings provider.
 * - Retrieval is a linear cosine-similarity scan in C++ (fast enough for
 *   tens of thousands of chunks; we cap reindex at 500 files for the MVP).
 *
 * Privacy: the default product path is local/on-device embeddings. Remote
 * embedding providers must be explicitly selected by future code before use.
 */
class APP_EXPORT QgsAiWorkspaceIndex : public QObject
{
    Q_OBJECT

  public:
    static constexpr int CHUNK_TARGET_CHARS = 1200;
    static constexpr int MAX_FILE_BYTES = 256 * 1024;
    static constexpr int DEFAULT_MAX_FILES = 500;
    static constexpr int EMBEDDING_BATCH = 64;
    //! Bumped when the on-disk SQLite schema changes; older DBs are dropped on first load.
    static constexpr int SCHEMA_VERSION = 2;

    //! Discriminates between workspace-file chunks and layer-data chunks.
    static constexpr const char *SOURCE_TYPE_FILE = "file";
    static constexpr const char *SOURCE_TYPE_LAYER = "layer";

    /**
     * Scope of a persistChunks() call. Determines which existing rows are replaced.
     *
     * - All: drop everything and write the new chunks.
     * - AllFiles: replace every chunk with source_type='file' (layer chunks preserved).
     * - AllLayers: replace every chunk with source_type='layer' (file chunks preserved).
     * - SingleLayer: replace only the chunks of the given layer id (file + other layer chunks preserved).
     */
    enum class ReplaceScope
    {
      All,
      AllFiles,
      AllLayers,
      SingleLayer,
    };

    struct Chunk
    {
        //! "file" or "layer" (see SOURCE_TYPE_*).
        QString sourceType = QString::fromLatin1( SOURCE_TYPE_FILE );
        //! For files: workspace-relative path. For layers: layer name (cosmetic).
        QString relativePath;
        //! Layer id (QgsMapLayer::id()). Empty for file chunks.
        QString layerId;
        //! Range of QgsFeature ids covered by this chunk. -1 when not applicable (file/raster).
        qint64 firstFeatureId = -1;
        qint64 lastFeatureId = -1;
        int chunkIndex = 0;
        QString text;
        //! Compressed (qCompress) join of the WKT of the features in this chunk. Empty for files/raster.
        QByteArray wktBlob;
        float score = 0.0f; // populated by search()
    };

    struct Status
    {
        bool indexed = false;
        int fileCount = 0;
        int chunkCount = 0;
        int fileChunkCount = 0;
        int layerChunkCount = 0;
        QDateTime lastSync;
        QString workspaceRoot;
    };

    QgsAiWorkspaceIndex( QgsAiFileContextProvider *contextProvider, QgsAiEmbeddingProvider *embeddingProvider, QObject *parent = nullptr );
    ~QgsAiWorkspaceIndex() override;

    virtual bool embeddingProviderAvailable() const;
    //! Temporary compatibility wrapper for older call sites.
    virtual bool hasEmbeddingConfiguration() const;
    Status status() const;

    /**
     * Walks the workspace, chunks every eligible text file, embeds the chunks
     * via the configured local embeddings provider, and stores them in the local SQLite
     * database, replacing any previous content. Returns false on failure with
     * \a errorMessage filled.
     *
     * \param maxFiles    Cap on the number of files indexed in one run (default 500).
     */
    bool reindex( int maxFiles, QString *errorMessage = nullptr );

    /**
     * Embeds \a query and returns the top-\a k chunks by cosine similarity.
     */
    QList<Chunk> search( const QString &query, int k, QString *errorMessage = nullptr );

    /**
     * Builds chunks for every layer in the active QgsProject (vectors + rasters),
     * embeds them and persists them with ReplaceScope::AllLayers (file chunks
     * are preserved). Returns false on failure with \a errorMessage filled.
     */
    bool reindexLayers( QString *errorMessage = nullptr );

    /**
     * Re-embeds and persists the chunks for a single layer identified by
     * \a layerId. Existing chunks for the same layer are replaced; other
     * layers and file chunks are preserved.
     */
    virtual bool reindexLayer( const QString &layerId, QString *errorMessage = nullptr );

    /**
     * Drops the index file from disk and clears the in-memory cache. Useful
     * when the user wants to revoke the embedding cache.
     */
    void clear();

    /**
     * Writes \a chunks (with their precomputed \a embeddings) to the SQLite store
     * and updates the in-memory cache. Existing rows are replaced according to
     * \a scope (and \a scopedLayerId for SingleLayer). Used by reindex(),
     * reindexLayers(), reindexLayer() and by tests.
     */
    bool persistChunks( const QList<Chunk> &chunks, const QList<QVector<float>> &embeddings, ReplaceScope scope, const QString &scopedLayerId, QString *errorMessage = nullptr );

    /**
     * Removes every chunk belonging to \a layerId from the cache and the SQLite store.
     */
    virtual bool removeLayer( const QString &layerId, QString *errorMessage = nullptr );

    /**
     * Returns the cached chunks (without embeddings), optionally filtered.
     *
     * - scope=All: every chunk regardless of source.
     * - scope=AllFiles: only source_type='file'.
     * - scope=AllLayers: only source_type='layer'.
     * - scope=SingleLayer: only source_type='layer' matching \a layerId.
     */
    QList<Chunk> chunks( ReplaceScope scope = ReplaceScope::All, const QString &layerId = QString() ) const;

    /**
     * Loads the on-disk SQLite store into the in-memory cache the first time
     * it is called. Subsequent calls are no-ops. Public so callers (e.g. tests
     * or UI code reading status before any reindex) can force the lazy load.
     */
    bool ensureLoaded();

  signals:
    void progress( int current, int total, const QString &filePath );

  private slots:
    void onWorkspaceRootChanged();

  private:
    struct CachedChunk
    {
        Chunk chunk;
        QVector<float> embedding;
    };

    bool persistAll( const QList<CachedChunk> &chunks, ReplaceScope scope, const QString &scopedLayerId, QString *errorMessage );
    bool loadAll( QString *errorMessage );
    QString dbPath() const;
    QString connectionName() const;
    static QStringList chunkText( const QString &content );
    static bool isTextFile( const QString &relativePath );
    static float cosineSimilarity( const QVector<float> &a, const QVector<float> &b );

    QgsAiFileContextProvider *mContextProvider = nullptr;
    QgsAiEmbeddingProvider *mEmbeddingProvider = nullptr;
    QList<CachedChunk> mCache;
    QDateTime mLastSync;
    bool mLoaded = false;
};

#endif // QGSAIWORKSPACEINDEX_H
