/***************************************************************************
    qgsaiworkspaceindex.cpp
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

#include "qgsaiworkspaceindex.h"

#include "ai/qgsaisecretstore.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "qgsaiembeddingprovider.h"
#include "qgsaifilecontextprovider.h"
#include "qgsailayerchunker.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QMutex>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QUuid>
#include <QVariant>

#include "moc_qgsaiworkspaceindex.cpp"

using namespace Qt::StringLiterals;

namespace
{
  const QStringList TEXT_EXTENSIONS = {
    u"h"_s,  u"hpp"_s,  u"cpp"_s, u"cc"_s,   u"c"_s,   u"py"_s,  u"md"_s,   u"txt"_s, u"json"_s, u"geojson"_s, u"csv"_s, u"tsv"_s,   u"qgs"_s,  u"qml"_s, u"xml"_s, u"ts"_s,
    u"js"_s, u"html"_s, u"css"_s, u"yaml"_s, u"yml"_s, u"ini"_s, u"toml"_s, u"cfg"_s, u"sh"_s,   u"bat"_s,     u"ps1"_s, u"cmake"_s, u"make"_s, u"mk"_s,  u"sql"_s, u"rst"_s,
  };

  QByteArray vectorToBytes( const QVector<float> &v )
  {
    QByteArray bytes;
    bytes.resize( static_cast<int>( v.size() * sizeof( float ) ) );
    if ( !v.isEmpty() )
      std::memcpy( bytes.data(), v.constData(), bytes.size() );
    return bytes;
  }

  QVector<float> bytesToVector( const QByteArray &bytes )
  {
    QVector<float> v;
    if ( bytes.size() % static_cast<int>( sizeof( float ) ) != 0 )
      return v;
    const int count = bytes.size() / static_cast<int>( sizeof( float ) );
    v.resize( count );
    if ( count > 0 )
      std::memcpy( v.data(), bytes.constData(), bytes.size() );
    return v;
  }

  bool ensureEmbeddingProviderAvailable( QgsAiEmbeddingProvider *provider, QString *errorMessage )
  {
    QString providerError;
    if ( provider && provider->isAvailable( &providerError ) )
      return true;

    if ( errorMessage )
      *errorMessage = providerError.isEmpty() ? u"Local embedding model is not installed."_s : providerError;
    return false;
  }

  QString storageProviderId( QgsAiEmbeddingProvider *provider )
  {
    return provider ? provider->providerId() : u"test"_s;
  }

  QString storageModelId( QgsAiEmbeddingProvider *provider )
  {
    return provider ? provider->modelId() : u"test"_s;
  }

  QString storageModelRevision( QgsAiEmbeddingProvider *provider )
  {
    if ( !provider )
      return u"test"_s;
    // A provider may report a null/empty revision (the base default returns a
    // null QString). The model_revision column is NOT NULL, so coalesce to a
    // non-null empty string to bind '' rather than SQL NULL.
    const QString revision = provider->modelRevision();
    return revision.isNull() ? u""_s : revision;
  }

  int storageDimension( QgsAiEmbeddingProvider *provider, const QVector<float> &embedding )
  {
    const int providerDimension = provider ? provider->embeddingDimension() : 0;
    return providerDimension > 0 ? providerDimension : embedding.size();
  }

  QString textHash( const QString &text, const QByteArray &extra = QByteArray() )
  {
    QCryptographicHash hash( QCryptographicHash::Sha1 );
    hash.addData( text.toUtf8() );
    if ( !extra.isEmpty() )
      hash.addData( extra );
    return QString::fromLatin1( hash.result().toHex() );
  }

  bool metadataMatchesProvider( const QString &providerId, const QString &modelId, const QString &modelRevision, int dimension, QgsAiEmbeddingProvider *provider )
  {
    if ( !provider )
      return true;
    if ( providerId != provider->providerId() )
      return false;
    if ( modelId != provider->modelId() )
      return false;
    if ( modelRevision != provider->modelRevision() )
      return false;
    const int providerDimension = provider->embeddingDimension();
    return providerDimension <= 0 || dimension == providerDimension;
  }

  QString chunkReuseKey( const QString &sourceType, const QString &relativePath, const QString &layerId, int chunkIndex )
  {
    return sourceType + QChar( 0x1f ) + relativePath + QChar( 0x1f ) + layerId + QChar( 0x1f ) + QString::number( chunkIndex );
  }
} //namespace

QgsAiWorkspaceIndex::QgsAiWorkspaceIndex( QgsAiFileContextProvider *contextProvider, QgsAiEmbeddingProvider *embeddingProvider, QObject *parent )
  : QObject( parent )
  , mContextProvider( contextProvider )
  , mEmbeddingProvider( embeddingProvider )
{
  if ( mContextProvider )
    connect( mContextProvider, &QgsAiFileContextProvider::workspaceRootChanged, this, &QgsAiWorkspaceIndex::onWorkspaceRootChanged );
}

QgsAiWorkspaceIndex::~QgsAiWorkspaceIndex()
{
  // Make sure we close any database connection we opened.
  const QString name = connectionName();
  if ( QSqlDatabase::contains( name ) )
    QSqlDatabase::removeDatabase( name );
}

QString QgsAiWorkspaceIndex::connectionName() const
{
  // One named SQL connection per index instance — avoid clashing with other
  // QGIS components that already use unnamed default connections.
  return u"qgsai_index_%1_%2"_s.arg( reinterpret_cast<quintptr>( this ) ).arg( reinterpret_cast<quintptr>( QThread::currentThreadId() ) );
}

QString QgsAiWorkspaceIndex::dbPath() const
{
  if ( !mContextProvider )
    return QString();

  const QString root = mContextProvider->workspaceRoot();
  if ( root.isEmpty() )
    return QString();

  const QByteArray hash = QCryptographicHash::hash( root.toUtf8(), QCryptographicHash::Sha1 ).toHex().left( 16 );

  // Separate index files per embedding provider so switching providers (for example
  // local <-> remote) keeps each index intact instead of rebuilding every time.
  // local and remote embeddings can never share a file.
  QString providerSlug;
  const QString rawId = mEmbeddingProvider ? mEmbeddingProvider->providerId().toLower() : QString();
  for ( const QChar ch : rawId )
    providerSlug.append( ch.isLetterOrNumber() ? ch : QChar( u'_' ) );
  if ( providerSlug.isEmpty() )
    providerSlug = u"none"_s;

  const QString dir = QgsApplication::qgisSettingsDirPath() + u"ai_index"_s;
  QDir().mkpath( dir );
  return QDir( dir ).filePath( u"ws_%1_%2.sqlite"_s.arg( QString::fromLatin1( hash ), providerSlug ) );
}

bool QgsAiWorkspaceIndex::isTextFile( const QString &relativePath )
{
  const QString ext = QFileInfo( relativePath ).suffix().toLower();
  if ( ext.isEmpty() )
    return false;
  return TEXT_EXTENSIONS.contains( ext );
}

bool QgsAiWorkspaceIndex::hasEmbeddingConfiguration() const
{
  return embeddingProviderAvailable();
}

void QgsAiWorkspaceIndex::setEmbeddingProvider( QgsAiEmbeddingProvider *embeddingProvider )
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  if ( mEmbeddingProvider == embeddingProvider )
    return;

  mEmbeddingProvider = embeddingProvider;
  mCache.clear();
  mLastSync = QDateTime();
  mLoaded = false;
  ensureLoaded();
}

bool QgsAiWorkspaceIndex::embeddingProviderAvailable() const
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  QString ignored;
  return mEmbeddingProvider && mEmbeddingProvider->isAvailable( &ignored );
}

void QgsAiWorkspaceIndex::onWorkspaceRootChanged()
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  const QString name = connectionName();
  if ( QSqlDatabase::contains( name ) )
  {
    {
      QSqlDatabase db = QSqlDatabase::database( name );
      db.close();
    }
    QSqlDatabase::removeDatabase( name );
  }

  mCache.clear();
  mLastSync = QDateTime();
  mLoaded = false;
  ensureLoaded();
}

QStringList QgsAiWorkspaceIndex::chunkText( const QString &content )
{
  QStringList chunks;
  if ( content.isEmpty() )
    return chunks;

  // Greedy chunker: walk forward CHUNK_TARGET_CHARS at a time, snap the
  // boundary back to the previous newline if there is one within the last 30%
  // of the chunk so we don't split mid-line.
  int pos = 0;
  while ( pos < content.size() )
  {
    int end = std::min( pos + CHUNK_TARGET_CHARS, static_cast<int>( content.size() ) );
    if ( end < content.size() )
    {
      const int searchFrom = end - CHUNK_TARGET_CHARS / 3;
      const int newline = content.lastIndexOf( '\n', end );
      if ( newline > pos && newline >= searchFrom )
        end = newline + 1;
    }
    QString slice = content.mid( pos, end - pos ).trimmed();
    if ( !slice.isEmpty() )
      chunks.append( slice );
    pos = end;
  }
  return chunks;
}

float QgsAiWorkspaceIndex::cosineSimilarity( const QVector<float> &a, const QVector<float> &b )
{
  if ( a.size() != b.size() || a.isEmpty() )
    return 0.0f;

  double dot = 0.0;
  double na = 0.0;
  double nb = 0.0;
  for ( int i = 0; i < a.size(); ++i )
  {
    dot += static_cast<double>( a[i] ) * b[i];
    na += static_cast<double>( a[i] ) * a[i];
    nb += static_cast<double>( b[i] ) * b[i];
  }
  if ( na == 0.0 || nb == 0.0 )
    return 0.0f;
  return static_cast<float>( dot / ( std::sqrt( na ) * std::sqrt( nb ) ) );
}

bool QgsAiWorkspaceIndex::ensureLoaded()
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  if ( mLoaded )
    return true;
  QString err;
  if ( !loadAll( &err ) )
  {
    QgsMessageLog::logMessage( u"Workspace index load failed: %1"_s.arg( err ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );
    // Treat missing/corrupt DB as empty rather than fatal.
    mCache.clear();
    mLastSync = QDateTime();
  }
  mLoaded = true;
  return true;
}

QgsAiWorkspaceIndex::Status QgsAiWorkspaceIndex::status() const
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  Status s;
  s.workspaceRoot = mContextProvider ? mContextProvider->workspaceRoot() : QString();
  s.embeddingProviderId = mEmbeddingProvider ? mEmbeddingProvider->providerId() : QString();
  s.embeddingModelId = mEmbeddingProvider ? mEmbeddingProvider->modelId() : QString();
  s.chunkCount = mCache.size();
  QStringList uniqueFiles;
  for ( const CachedChunk &c : mCache )
  {
    if ( c.chunk.sourceType == QString::fromLatin1( SOURCE_TYPE_LAYER ) )
      ++s.layerChunkCount;
    else
    {
      ++s.fileChunkCount;
      if ( !uniqueFiles.contains( c.chunk.relativePath ) )
        uniqueFiles.append( c.chunk.relativePath );
    }
  }
  s.fileCount = uniqueFiles.size();
  s.lastSync = mLastSync;
  s.indexed = !mCache.isEmpty();
  return s;
}

QList<QgsAiWorkspaceIndex::Chunk> QgsAiWorkspaceIndex::chunks( ReplaceScope scope, const QString &layerId ) const
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  QList<Chunk> out;
  for ( const CachedChunk &c : mCache )
  {
    const bool isLayer = c.chunk.sourceType == QString::fromLatin1( SOURCE_TYPE_LAYER );
    switch ( scope )
    {
      case ReplaceScope::All:
        out.append( c.chunk );
        break;
      case ReplaceScope::AllFiles:
        if ( !isLayer )
          out.append( c.chunk );
        break;
      case ReplaceScope::AllLayers:
        if ( isLayer )
          out.append( c.chunk );
        break;
      case ReplaceScope::SingleLayer:
        if ( isLayer && c.chunk.layerId == layerId )
          out.append( c.chunk );
        break;
    }
  }
  return out;
}

bool QgsAiWorkspaceIndex::loadAll( QString *errorMessage )
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  mCache.clear();
  const QString path = dbPath();
  if ( path.isEmpty() || !QFileInfo::exists( path ) )
    return true; // Nothing to load yet — that's not an error.

  QSqlDatabase db = QSqlDatabase::contains( connectionName() ) ? QSqlDatabase::database( connectionName() ) : QSqlDatabase::addDatabase( u"QSQLITE"_s, connectionName() );
  db.setDatabaseName( path );
  if ( !db.open() )
  {
    if ( errorMessage )
      *errorMessage = db.lastError().text();
    return false;
  }

  // Schema migration: read PRAGMA user_version. If older than SCHEMA_VERSION,
  // drop the chunks table — callers will rebuild it on the next reindex.
  {
    QSqlQuery v( db );
    int currentVersion = 0;
    if ( v.exec( u"PRAGMA user_version"_s ) && v.next() )
      currentVersion = v.value( 0 ).toInt();
    if ( currentVersion < SCHEMA_VERSION )
    {
      QgsMessageLog::logMessage( u"Workspace index schema upgrade: %1 → %2 (dropping old chunks)"_s.arg( currentVersion ).arg( SCHEMA_VERSION ), u"AI/Index"_s, Qgis::MessageLevel::Info, false );
      QSqlQuery drop( db );
      drop.exec( u"DROP TABLE IF EXISTS chunks"_s );
      drop.exec( u"PRAGMA user_version = %1"_s.arg( SCHEMA_VERSION ) );
      return true; // Empty — caller will reindex.
    }
  }

  {
    QSqlQuery tableCheck( db );
    if ( tableCheck.exec( u"SELECT name FROM sqlite_master WHERE type='table' AND name='chunks'"_s ) && !tableCheck.next() )
      return true;
  }

  {
    QSqlQuery metadata( db );
    if ( metadata.exec( u"SELECT provider_id, model_id, model_revision, embedding_dimension FROM chunks LIMIT 1"_s ) && metadata.next() )
    {
      const QString providerId = metadata.value( 0 ).toString();
      const QString modelId = metadata.value( 1 ).toString();
      const QString modelRevision = metadata.value( 2 ).toString();
      const int dimension = metadata.value( 3 ).toInt();
      if ( !metadataMatchesProvider( providerId, modelId, modelRevision, dimension, mEmbeddingProvider ) )
      {
        QgsMessageLog::logMessage( u"Workspace index provider changed (%1/%2 -> %3/%4); dropping old chunks"_s.arg( providerId, modelId, storageProviderId( mEmbeddingProvider ), storageModelId( mEmbeddingProvider ) ), u"AI/Index"_s, Qgis::MessageLevel::Info, false );
        QSqlQuery drop( db );
        drop.exec( u"DROP TABLE IF EXISTS chunks"_s );
        drop.exec( u"PRAGMA user_version = %1"_s.arg( SCHEMA_VERSION ) );
        return true;
      }
    }
  }

  QSqlQuery q( db );
  if ( !q.exec( u"SELECT source_type, relative_path, layer_id, feature_id_min, feature_id_max, chunk_index, text, wkt_blob, embedding, last_sync, provider_id, model_id, model_revision, embedding_dimension, content_hash, source_mtime FROM chunks ORDER BY id"_s ) )
  {
    if ( errorMessage )
      *errorMessage = q.lastError().text();
    return false;
  }
  qint64 maxSync = 0;
  while ( q.next() )
  {
    CachedChunk c;
    c.chunk.sourceType = q.value( 0 ).toString();
    if ( c.chunk.sourceType.isEmpty() )
      c.chunk.sourceType = QString::fromLatin1( SOURCE_TYPE_FILE );
    c.chunk.relativePath = q.value( 1 ).toString();
    c.chunk.layerId = q.value( 2 ).toString();
    c.chunk.firstFeatureId = q.value( 3 ).isNull() ? -1 : q.value( 3 ).toLongLong();
    c.chunk.lastFeatureId = q.value( 4 ).isNull() ? -1 : q.value( 4 ).toLongLong();
    c.chunk.chunkIndex = q.value( 5 ).toInt();
    // Per-value `enc1:` detection keeps legacy plaintext rows readable (mixed mode).
    c.chunk.text = QgsAiSecretStore::decryptValue( q.value( 6 ).toString() );
    c.chunk.wktBlob = QgsAiSecretStore::decryptBlob( q.value( 7 ).toByteArray() );
    c.embedding = bytesToVector( QgsAiSecretStore::decryptBlob( q.value( 8 ).toByteArray() ) );
    const qint64 syncMs = q.value( 9 ).toLongLong();
    c.providerId = q.value( 10 ).toString();
    c.modelId = q.value( 11 ).toString();
    c.modelRevision = q.value( 12 ).toString();
    c.embeddingDimension = q.value( 13 ).toInt();
    c.contentHash = q.value( 14 ).toString();
    c.sourceMTime = q.value( 15 ).toLongLong();
    if ( syncMs > maxSync )
      maxSync = syncMs;
    if ( !c.embedding.isEmpty() )
      mCache.append( c );
  }
  if ( maxSync > 0 )
    mLastSync = QDateTime::fromMSecsSinceEpoch( maxSync );

  return true;
}

bool QgsAiWorkspaceIndex::persistAll( const QList<CachedChunk> &chunks, ReplaceScope scope, const QString &scopedLayerId, QString *errorMessage )
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  const QString path = dbPath();
  if ( path.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Workspace root is unset; cannot persist index."_s;
    return false;
  }

  QSqlDatabase db = QSqlDatabase::contains( connectionName() ) ? QSqlDatabase::database( connectionName() ) : QSqlDatabase::addDatabase( u"QSQLITE"_s, connectionName() );
  db.setDatabaseName( path );
  if ( !db.open() )
  {
    if ( errorMessage )
      *errorMessage = db.lastError().text();
    return false;
  }

  QSqlQuery q( db );
  q.exec( QStringLiteral(
    "CREATE TABLE IF NOT EXISTS chunks ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "source_type TEXT NOT NULL DEFAULT 'file', "
    "relative_path TEXT NOT NULL, "
    "layer_id TEXT, "
    "feature_id_min INTEGER, "
    "feature_id_max INTEGER, "
    "chunk_index INTEGER NOT NULL, "
    "text TEXT NOT NULL, "
    "wkt_blob BLOB, "
    "embedding BLOB NOT NULL, "
    "last_sync INTEGER NOT NULL, "
    "provider_id TEXT NOT NULL DEFAULT 'unknown', "
    "model_id TEXT NOT NULL DEFAULT 'unknown', "
    "model_revision TEXT NOT NULL DEFAULT '', "
    "embedding_dimension INTEGER NOT NULL DEFAULT 0, "
    "content_hash TEXT NOT NULL DEFAULT '', "
    "source_mtime INTEGER NOT NULL DEFAULT 0"
    ")"
  ) );
  q.exec( u"CREATE INDEX IF NOT EXISTS idx_chunks_layer ON chunks(layer_id) WHERE layer_id IS NOT NULL"_s );
  q.exec( u"CREATE INDEX IF NOT EXISTS idx_chunks_source_hash ON chunks(source_type, relative_path, chunk_index, content_hash)"_s );
  q.exec( u"PRAGMA user_version = %1"_s.arg( SCHEMA_VERSION ) );

  // Replace only the rows in scope.
  switch ( scope )
  {
    case ReplaceScope::All:
      q.exec( u"DELETE FROM chunks"_s );
      break;
    case ReplaceScope::AllFiles:
      q.exec( u"DELETE FROM chunks WHERE source_type = 'file'"_s );
      break;
    case ReplaceScope::AllLayers:
      q.exec( u"DELETE FROM chunks WHERE source_type = 'layer'"_s );
      break;
    case ReplaceScope::SingleLayer:
    {
      QSqlQuery del( db );
      del.prepare( u"DELETE FROM chunks WHERE source_type = 'layer' AND layer_id = ?"_s );
      del.addBindValue( scopedLayerId );
      del.exec();
      break;
    }
  }

  if ( !db.transaction() )
  {
    if ( errorMessage )
      *errorMessage = u"Cannot start SQLite transaction: %1"_s.arg( db.lastError().text() );
    return false;
  }

  // Encrypt-at-rest when the data key is available; otherwise warn once and
  // persist plaintext (or refuse when the user requires encryption).
  const bool encryptionAvailable = QgsAiSecretStore::storageEncryptionAvailable();
  if ( !encryptionAvailable )
  {
    QgsSettings appSettings;
    if ( appSettings.value( u"ai/storage/requireEncryption"_s, false ).toBool() )
    {
      if ( errorMessage )
        *errorMessage = u"Encryption is required (ai/storage/requireEncryption) but the authentication vault is unavailable."_s;
      db.rollback();
      return false;
    }
    QgsAiSecretStore::warnPlaintextStorageOnce();
  }

  const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
  q.prepare( QStringLiteral(
    "INSERT INTO chunks (source_type, relative_path, layer_id, feature_id_min, feature_id_max, chunk_index, text, wkt_blob, embedding, last_sync, provider_id, model_id, model_revision, embedding_dimension, content_hash, source_mtime) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
  ) );
  for ( const CachedChunk &c : chunks )
  {
    const QString providerId = c.providerId.isEmpty() ? storageProviderId( mEmbeddingProvider ) : c.providerId;
    const QString modelId = c.modelId.isEmpty() ? storageModelId( mEmbeddingProvider ) : c.modelId;
    const QString modelRevision = c.modelRevision.isEmpty() ? storageModelRevision( mEmbeddingProvider ) : c.modelRevision;
    const int dimension = c.embeddingDimension > 0 ? c.embeddingDimension : storageDimension( mEmbeddingProvider, c.embedding );
    // The content hash stays computed over the PLAINTEXT so chunk-reuse keys
    // remain stable across encrypted and plaintext sessions.
    const QString hash = c.contentHash.isEmpty() ? textHash( c.chunk.text, c.chunk.wktBlob ) : c.contentHash;
    q.addBindValue( c.chunk.sourceType.isEmpty() ? QString::fromLatin1( SOURCE_TYPE_FILE ) : c.chunk.sourceType );
    q.addBindValue( c.chunk.relativePath );
    q.addBindValue( c.chunk.layerId.isEmpty() ? QVariant( QMetaType::fromType<QString>() ) : QVariant( c.chunk.layerId ) );
    q.addBindValue( c.chunk.firstFeatureId < 0 ? QVariant( QMetaType::fromType<qint64>() ) : QVariant( c.chunk.firstFeatureId ) );
    q.addBindValue( c.chunk.lastFeatureId < 0 ? QVariant( QMetaType::fromType<qint64>() ) : QVariant( c.chunk.lastFeatureId ) );
    q.addBindValue( c.chunk.chunkIndex );
    q.addBindValue( encryptionAvailable ? QgsAiSecretStore::encryptValue( c.chunk.text ) : c.chunk.text );
    q.addBindValue( c.chunk.wktBlob.isEmpty() ? QVariant( QMetaType::fromType<QByteArray>() ) : QVariant( encryptionAvailable ? QgsAiSecretStore::encryptBlob( c.chunk.wktBlob ) : c.chunk.wktBlob ) );
    q.addBindValue( encryptionAvailable ? QgsAiSecretStore::encryptBlob( vectorToBytes( c.embedding ) ) : vectorToBytes( c.embedding ) );
    q.addBindValue( nowMs );
    q.addBindValue( providerId );
    q.addBindValue( modelId );
    q.addBindValue( modelRevision );
    q.addBindValue( dimension );
    q.addBindValue( hash );
    q.addBindValue( c.sourceMTime );
    if ( !q.exec() )
    {
      if ( errorMessage )
        *errorMessage = q.lastError().text();
      db.rollback();
      return false;
    }
  }
  if ( !db.commit() )
  {
    if ( errorMessage )
      *errorMessage = db.lastError().text();
    return false;
  }
  return true;
}

bool QgsAiWorkspaceIndex::persistChunks( const QList<Chunk> &chunks, const QList<QVector<float>> &embeddings, ReplaceScope scope, const QString &scopedLayerId, QString *errorMessage )
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  if ( chunks.size() != embeddings.size() )
  {
    if ( errorMessage )
      *errorMessage = u"persistChunks: chunks/embeddings size mismatch (%1 vs %2)"_s.arg( chunks.size() ).arg( embeddings.size() );
    return false;
  }

  ensureLoaded();

  QList<CachedChunk> built;
  built.reserve( chunks.size() );
  for ( int i = 0; i < chunks.size(); ++i )
  {
    CachedChunk cc;
    cc.chunk = chunks.at( i );
    if ( cc.chunk.sourceType.isEmpty() )
      cc.chunk.sourceType = QString::fromLatin1( SOURCE_TYPE_FILE );
    cc.embedding = embeddings.at( i );
    cc.providerId = storageProviderId( mEmbeddingProvider );
    cc.modelId = storageModelId( mEmbeddingProvider );
    cc.modelRevision = storageModelRevision( mEmbeddingProvider );
    cc.embeddingDimension = storageDimension( mEmbeddingProvider, cc.embedding );
    cc.contentHash = textHash( cc.chunk.text, cc.chunk.wktBlob );
    built.append( cc );
  }

  if ( !persistAll( built, scope, scopedLayerId, errorMessage ) )
    return false;

  // Reflect the change in the in-memory cache without re-reading the DB.
  switch ( scope )
  {
    case ReplaceScope::All:
      mCache.clear();
      break;
    case ReplaceScope::AllFiles:
      mCache.erase( std::remove_if( mCache.begin(), mCache.end(), []( const CachedChunk &c ) { return c.chunk.sourceType == QString::fromLatin1( SOURCE_TYPE_FILE ); } ), mCache.end() );
      break;
    case ReplaceScope::AllLayers:
      mCache.erase( std::remove_if( mCache.begin(), mCache.end(), []( const CachedChunk &c ) { return c.chunk.sourceType == QString::fromLatin1( SOURCE_TYPE_LAYER ); } ), mCache.end() );
      break;
    case ReplaceScope::SingleLayer:
      mCache.erase(
        std::remove_if(
          mCache.begin(), mCache.end(), [&scopedLayerId]( const CachedChunk &c ) { return c.chunk.sourceType == QString::fromLatin1( SOURCE_TYPE_LAYER ) && c.chunk.layerId == scopedLayerId; }
        ),
        mCache.end()
      );
      break;
  }
  mCache.append( built );
  mLastSync = QDateTime::currentDateTimeUtc();
  mLoaded = true;
  return true;
}

bool QgsAiWorkspaceIndex::removeLayer( const QString &layerId, QString *errorMessage )
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  if ( layerId.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"removeLayer: empty layerId."_s;
    return false;
  }

  ensureLoaded();

  const QString path = dbPath();
  if ( !path.isEmpty() && QFileInfo::exists( path ) )
  {
    QSqlDatabase db = QSqlDatabase::contains( connectionName() ) ? QSqlDatabase::database( connectionName() ) : QSqlDatabase::addDatabase( u"QSQLITE"_s, connectionName() );
    db.setDatabaseName( path );
    if ( db.open() )
    {
      QSqlQuery del( db );
      del.prepare( u"DELETE FROM chunks WHERE source_type = 'layer' AND layer_id = ?"_s );
      del.addBindValue( layerId );
      if ( !del.exec() )
      {
        if ( errorMessage )
          *errorMessage = del.lastError().text();
        return false;
      }
    }
  }

  mCache.erase(
    std::remove_if( mCache.begin(), mCache.end(), [&layerId]( const CachedChunk &c ) { return c.chunk.sourceType == QString::fromLatin1( SOURCE_TYPE_LAYER ) && c.chunk.layerId == layerId; } ),
    mCache.end()
  );
  return true;
}

bool QgsAiWorkspaceIndex::reindex( int maxFiles, QString *errorMessage )
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  if ( !mContextProvider )
  {
    if ( errorMessage )
      *errorMessage = u"Workspace context provider is unavailable."_s;
    return false;
  }
  if ( mContextProvider->workspaceRoot().isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"AI workspace root is unset. Save the QGIS project or configure the AI workspace root in provider settings."_s;
    return false;
  }
  if ( !ensureEmbeddingProviderAvailable( mEmbeddingProvider, errorMessage ) )
    return false;

  ensureLoaded();

  QHash<QString, CachedChunk> reusableFileChunks;
  for ( const CachedChunk &cached : std::as_const( mCache ) )
  {
    if ( cached.chunk.sourceType == QString::fromLatin1( SOURCE_TYPE_FILE ) || cached.chunk.sourceType.isEmpty() )
    {
      if ( metadataMatchesProvider( cached.providerId, cached.modelId, cached.modelRevision, cached.embeddingDimension, mEmbeddingProvider ) )
      {
        reusableFileChunks.insert(
          chunkReuseKey( QString::fromLatin1( SOURCE_TYPE_FILE ), cached.chunk.relativePath, QString(), cached.chunk.chunkIndex ),
          cached
        );
      }
    }
  }

  const int cap = maxFiles > 0 ? maxFiles : DEFAULT_MAX_FILES;
  const QStringList all = mContextProvider->workspaceFileCandidates( QString(), 50000 );

  // Restrict to text-ish files inside the size cap.
  QStringList eligible;
  for ( const QString &rel : all )
  {
    if ( !isTextFile( rel ) )
      continue;
    const QString abs = mContextProvider->resolveWorkspaceFile( rel );
    if ( abs.isEmpty() )
      continue;
    if ( QFileInfo( abs ).size() > MAX_FILE_BYTES )
      continue;
    eligible.append( rel );
    if ( eligible.size() >= cap )
      break;
  }

  if ( eligible.isEmpty() )
  {
    // Clear file chunks from cache; preserve any existing layer chunks.
    mCache.erase(
      std::remove_if( mCache.begin(), mCache.end(), []( const CachedChunk &c ) { return c.chunk.sourceType == QString::fromLatin1( SOURCE_TYPE_FILE ) || c.chunk.sourceType.isEmpty(); } ), mCache.end()
    );
    mLastSync = QDateTime::currentDateTimeUtc();
    QString err;
    persistAll( {}, ReplaceScope::AllFiles, QString(), &err );
    return true;
  }

  // Build chunks for every file. We keep a parallel list of texts to embed so
  // the provider can process them in batches.
  QList<CachedChunk> built;
  QStringList textsToEmbed;
  QList<int> backRefIndex; // for each text in textsToEmbed, the position in `built`
  for ( int fileIdx = 0; fileIdx < eligible.size(); ++fileIdx )
  {
    const QString rel = eligible.at( fileIdx );
    emit progress( fileIdx + 1, eligible.size(), rel );

    const QString abs = mContextProvider->resolveWorkspaceFile( rel );
    const QFileInfo fileInfo( abs );
    const qint64 sourceMTime = fileInfo.exists() ? fileInfo.lastModified().toMSecsSinceEpoch() : 0;
    const QgsAiFileContext fileContext = mContextProvider->buildContext( rel, QString(), MAX_FILE_BYTES );
    if ( fileContext.fileSnippet.isEmpty() || fileContext.binary )
      continue;

    const QStringList chunks = chunkText( fileContext.fileSnippet );
    for ( int ci = 0; ci < chunks.size(); ++ci )
    {
      CachedChunk c;
      c.chunk.relativePath = rel;
      c.chunk.chunkIndex = ci;
      c.chunk.text = chunks.at( ci );
      c.chunk.sourceType = QString::fromLatin1( SOURCE_TYPE_FILE );
      c.providerId = storageProviderId( mEmbeddingProvider );
      c.modelId = storageModelId( mEmbeddingProvider );
      c.modelRevision = storageModelRevision( mEmbeddingProvider );
      c.contentHash = textHash( c.chunk.text );
      c.sourceMTime = sourceMTime;

      const QString reuseKey = chunkReuseKey( QString::fromLatin1( SOURCE_TYPE_FILE ), rel, QString(), ci );
      const auto reusableIt = reusableFileChunks.constFind( reuseKey );
      if ( reusableIt != reusableFileChunks.constEnd() && reusableIt->contentHash == c.contentHash && reusableIt->sourceMTime == c.sourceMTime && !reusableIt->embedding.isEmpty() )
      {
        c.embedding = reusableIt->embedding;
        c.embeddingDimension = reusableIt->embeddingDimension;
        built.append( c );
        continue;
      }

      built.append( c );
      backRefIndex.append( built.size() - 1 );
      textsToEmbed.append( c.chunk.text );
    }
  }

  if ( built.isEmpty() )
  {
    mCache.erase(
      std::remove_if( mCache.begin(), mCache.end(), []( const CachedChunk &c ) { return c.chunk.sourceType == QString::fromLatin1( SOURCE_TYPE_FILE ) || c.chunk.sourceType.isEmpty(); } ), mCache.end()
    );
    mLastSync = QDateTime::currentDateTimeUtc();
    QString err;
    persistAll( {}, ReplaceScope::AllFiles, QString(), &err );
    return true;
  }

  if ( !textsToEmbed.isEmpty() )
  {
    QList<QVector<float>> vectors;
    QgsAiEmbeddingOptions options;
    options.maxBatch = EMBEDDING_BATCH;
    if ( !mEmbeddingProvider->embed( textsToEmbed, QgsAiEmbeddingRole::Passage, vectors, errorMessage, options ) )
      return false;
    if ( vectors.size() != textsToEmbed.size() )
    {
      if ( errorMessage )
        *errorMessage = u"Embedding count mismatch: expected %1 got %2"_s.arg( textsToEmbed.size() ).arg( vectors.size() );
      return false;
    }

    for ( int i = 0; i < vectors.size(); ++i )
    {
      CachedChunk &chunk = built[backRefIndex.at( i )];
      chunk.embedding = vectors.at( i );
      chunk.embeddingDimension = storageDimension( mEmbeddingProvider, chunk.embedding );
    }
  }

  if ( !persistAll( built, ReplaceScope::AllFiles, QString(), errorMessage ) )
    return false;

  // Replace only file chunks in the cache; preserve any existing layer chunks.
  mCache
    .erase( std::remove_if( mCache.begin(), mCache.end(), []( const CachedChunk &c ) { return c.chunk.sourceType == QString::fromLatin1( SOURCE_TYPE_FILE ) || c.chunk.sourceType.isEmpty(); } ), mCache.end() );
  mCache.append( built );
  mLastSync = QDateTime::currentDateTimeUtc();
  mLoaded = true;
  QgsMessageLog::logMessage( u"Workspace index built: files=%1 chunks=%2"_s.arg( eligible.size() ).arg( built.size() ), u"AI/Index"_s, Qgis::MessageLevel::Info, false );
  return true;
}

QList<QgsAiWorkspaceIndex::Chunk> QgsAiWorkspaceIndex::search( const QString &query, int k, QString *errorMessage )
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  ensureLoaded();
  QList<Chunk> results;
  if ( query.trimmed().isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Empty query."_s;
    return results;
  }
  if ( !ensureEmbeddingProviderAvailable( mEmbeddingProvider, errorMessage ) )
    return results;
  if ( mCache.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Workspace index is empty. Run reindex_workspace first."_s;
    return results;
  }

  QStringList qList { query };
  QList<QVector<float>> qEmb;
  QgsAiEmbeddingOptions options;
  options.maxBatch = 1;
  if ( !mEmbeddingProvider->embed( qList, QgsAiEmbeddingRole::Query, qEmb, errorMessage, options ) || qEmb.isEmpty() )
    return results;

  const QVector<float> &qVec = qEmb.first();

  // Linear cosine scan; small enough for the MVP.
  QList<QPair<float, int>> scored;
  scored.reserve( mCache.size() );
  for ( int i = 0; i < mCache.size(); ++i )
    scored.append( { cosineSimilarity( qVec, mCache.at( i ).embedding ), i } );

  std::sort( scored.begin(), scored.end(), []( const auto &a, const auto &b ) { return a.first > b.first; } );

  const int topK = std::clamp( k, 1, std::min( 20, static_cast<int>( mCache.size() ) ) );
  for ( int i = 0; i < topK; ++i )
  {
    Chunk c = mCache.at( scored.at( i ).second ).chunk;
    c.score = scored.at( i ).first;
    results.append( c );
  }
  return results;
}

namespace
{
  QList<QgsAiWorkspaceIndex::Chunk> chunksForLayer( QgsMapLayer *layer )
  {
    if ( !layer )
      return {};
    if ( QgsVectorLayer *v = qobject_cast<QgsVectorLayer *>( layer ) )
      return QgsAiLayerChunker::chunkVector( v );
    if ( QgsRasterLayer *r = qobject_cast<QgsRasterLayer *>( layer ) )
      return QgsAiLayerChunker::chunkRaster( r );
    return {};
  }
} // namespace

bool QgsAiWorkspaceIndex::reindexLayers( QString *errorMessage )
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  if ( !ensureEmbeddingProviderAvailable( mEmbeddingProvider, errorMessage ) )
    return false;

  ensureLoaded();

  QgsProject *project = QgsProject::instance();
  if ( !project )
  {
    if ( errorMessage )
      *errorMessage = u"No active QgsProject available."_s;
    return false;
  }

  QList<Chunk> allChunks;
  const QMap<QString, QgsMapLayer *> layers = project->mapLayers();
  int processed = 0;
  for ( auto it = layers.constBegin(); it != layers.constEnd(); ++it )
  {
    emit progress( ++processed, layers.size(), it.value() ? it.value()->name() : QString() );
    allChunks.append( chunksForLayer( it.value() ) );
  }

  if ( allChunks.isEmpty() )
  {
    // Nothing to embed — drop existing layer chunks and we're done.
    return persistChunks( {}, {}, ReplaceScope::AllLayers, QString(), errorMessage );
  }

  QStringList textsToEmbed;
  textsToEmbed.reserve( allChunks.size() );
  for ( const Chunk &c : allChunks )
    textsToEmbed.append( c.text );

  QList<QVector<float>> vectors;
  QgsAiEmbeddingOptions options;
  options.maxBatch = EMBEDDING_BATCH;
  if ( !mEmbeddingProvider->embed( textsToEmbed, QgsAiEmbeddingRole::Passage, vectors, errorMessage, options ) )
    return false;
  if ( vectors.size() != allChunks.size() )
  {
    if ( errorMessage )
      *errorMessage = u"Embedding count mismatch: expected %1 got %2"_s.arg( allChunks.size() ).arg( vectors.size() );
    return false;
  }

  if ( !persistChunks( allChunks, vectors, ReplaceScope::AllLayers, QString(), errorMessage ) )
    return false;

  QgsMessageLog::logMessage( u"Workspace index: layer reindex done — layers=%1 chunks=%2"_s.arg( layers.size() ).arg( allChunks.size() ), u"AI/Index"_s, Qgis::MessageLevel::Info, false );
  return true;
}

bool QgsAiWorkspaceIndex::reindexLayer( const QString &layerId, QString *errorMessage )
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  if ( layerId.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"reindexLayer: empty layerId."_s;
    return false;
  }
  if ( !ensureEmbeddingProviderAvailable( mEmbeddingProvider, errorMessage ) )
    return false;

  ensureLoaded();

  QgsProject *project = QgsProject::instance();
  QgsMapLayer *layer = project ? project->mapLayer( layerId ) : nullptr;
  if ( !layer )
  {
    // Layer no longer exists — treat as removal so the index stays consistent.
    return removeLayer( layerId, errorMessage );
  }

  const QList<Chunk> chunks = chunksForLayer( layer );
  if ( chunks.isEmpty() )
    return persistChunks( {}, {}, ReplaceScope::SingleLayer, layerId, errorMessage );

  QStringList textsToEmbed;
  textsToEmbed.reserve( chunks.size() );
  for ( const Chunk &c : chunks )
    textsToEmbed.append( c.text );

  QList<QVector<float>> vectors;
  QgsAiEmbeddingOptions options;
  options.maxBatch = EMBEDDING_BATCH;
  if ( !mEmbeddingProvider->embed( textsToEmbed, QgsAiEmbeddingRole::Passage, vectors, errorMessage, options ) )
    return false;
  if ( vectors.size() != chunks.size() )
  {
    if ( errorMessage )
      *errorMessage = u"Embedding count mismatch: expected %1 got %2"_s.arg( chunks.size() ).arg( vectors.size() );
    return false;
  }

  return persistChunks( chunks, vectors, ReplaceScope::SingleLayer, layerId, errorMessage );
}

void QgsAiWorkspaceIndex::clear()
{
  QMutexLocker<QRecursiveMutex> locker( &mMutex );
  mCache.clear();
  mLastSync = QDateTime();
  const QString path = dbPath();
  if ( !path.isEmpty() && QFileInfo::exists( path ) )
    QFile::remove( path );
}
