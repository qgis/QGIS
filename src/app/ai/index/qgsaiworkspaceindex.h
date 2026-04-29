#ifndef QGSAIWORKSPACEINDEX_H
#define QGSAIWORKSPACEINDEX_H

#include "qgis_app.h"

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <QVector>

class QgsAiEmbeddingClient;
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
 * - Embeddings come from the OpenAI embeddings endpoint via QgsAiEmbeddingClient.
 * - Retrieval is a linear cosine-similarity scan in C++ (fast enough for
 *   tens of thousands of chunks; we cap reindex at 500 files for the MVP).
 *
 * Privacy: indexing sends the file chunks to OpenAI. Callers are expected to
 * surface a disclaimer before triggering reindex.
 */
class APP_EXPORT QgsAiWorkspaceIndex : public QObject
{
    Q_OBJECT

  public:
    static constexpr int CHUNK_TARGET_CHARS = 1200;
    static constexpr int MAX_FILE_BYTES = 256 * 1024;
    static constexpr int DEFAULT_MAX_FILES = 500;
    static constexpr int EMBEDDING_BATCH = 64;

    struct Chunk
    {
        QString relativePath;
        int chunkIndex = 0;
        QString text;
        float score = 0.0f; // populated by search()
    };

    struct Status
    {
        bool indexed = false;
        int fileCount = 0;
        int chunkCount = 0;
        QDateTime lastSync;
        QString workspaceRoot;
    };

    QgsAiWorkspaceIndex( QgsAiFileContextProvider *contextProvider, QgsAiEmbeddingClient *embeddingClient, QObject *parent = nullptr );
    ~QgsAiWorkspaceIndex() override;

    Status status() const;

    /**
     * Walks the workspace, chunks every eligible text file, embeds the chunks
     * via the OpenAI embeddings endpoint, and stores them in the local SQLite
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
     * Drops the index file from disk and clears the in-memory cache. Useful
     * when the user wants to revoke the OpenAI embedding cache.
     */
    void clear();

  signals:
    void progress( int current, int total, const QString &filePath );

  private:
    struct CachedChunk
    {
        Chunk chunk;
        QVector<float> embedding;
    };

    bool ensureLoaded();
    bool persistAll( const QList<CachedChunk> &chunks, QString *errorMessage );
    bool loadAll( QString *errorMessage );
    QString dbPath() const;
    QString connectionName() const;
    static QStringList chunkText( const QString &content );
    static bool isTextFile( const QString &relativePath );
    static float cosineSimilarity( const QVector<float> &a, const QVector<float> &b );

    QgsAiFileContextProvider *mContextProvider = nullptr;
    QgsAiEmbeddingClient *mEmbeddingClient = nullptr;
    QList<CachedChunk> mCache;
    QDateTime mLastSync;
    bool mLoaded = false;
};

#endif // QGSAIWORKSPACEINDEX_H
