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
#include "moc_qgsaiworkspaceindex.cpp"

#include <algorithm>
#include <cmath>

#include "qgsaiembeddingclient.h"
#include "qgsaifilecontextprovider.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QStringList>
#include <QUuid>
#include <QVariant>

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
} //namespace

QgsAiWorkspaceIndex::QgsAiWorkspaceIndex( QgsAiFileContextProvider *contextProvider, QgsAiEmbeddingClient *embeddingClient, QObject *parent )
  : QObject( parent )
  , mContextProvider( contextProvider )
  , mEmbeddingClient( embeddingClient )
{}

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
  return u"qgsai_index_%1"_s.arg( reinterpret_cast<quintptr>( this ) );
}

QString QgsAiWorkspaceIndex::dbPath() const
{
  if ( !mContextProvider )
    return QString();

  const QString root = mContextProvider->workspaceRoot();
  const QByteArray hash = QCryptographicHash::hash( root.toUtf8(), QCryptographicHash::Sha1 ).toHex().left( 16 );
  const QString dir = QgsApplication::qgisSettingsDirPath() + u"ai_index"_s;
  QDir().mkpath( dir );
  return QDir( dir ).filePath( u"ws_%1.sqlite"_s.arg( QString::fromLatin1( hash ) ) );
}

bool QgsAiWorkspaceIndex::isTextFile( const QString &relativePath )
{
  const QString ext = QFileInfo( relativePath ).suffix().toLower();
  if ( ext.isEmpty() )
    return false;
  return TEXT_EXTENSIONS.contains( ext );
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
  Status s;
  s.workspaceRoot = mContextProvider ? mContextProvider->workspaceRoot() : QString();
  s.chunkCount = mCache.size();
  QStringList uniqueFiles;
  for ( const CachedChunk &c : mCache )
  {
    if ( !uniqueFiles.contains( c.chunk.relativePath ) )
      uniqueFiles.append( c.chunk.relativePath );
  }
  s.fileCount = uniqueFiles.size();
  s.lastSync = mLastSync;
  s.indexed = !mCache.isEmpty();
  return s;
}

bool QgsAiWorkspaceIndex::loadAll( QString *errorMessage )
{
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

  QSqlQuery q( db );
  if ( !q.exec( u"SELECT relative_path, chunk_index, text, embedding, last_sync FROM chunks ORDER BY id"_s ) )
  {
    if ( errorMessage )
      *errorMessage = q.lastError().text();
    return false;
  }
  qint64 maxSync = 0;
  while ( q.next() )
  {
    CachedChunk c;
    c.chunk.relativePath = q.value( 0 ).toString();
    c.chunk.chunkIndex = q.value( 1 ).toInt();
    c.chunk.text = q.value( 2 ).toString();
    c.embedding = bytesToVector( q.value( 3 ).toByteArray() );
    const qint64 syncMs = q.value( 4 ).toLongLong();
    if ( syncMs > maxSync )
      maxSync = syncMs;
    if ( !c.embedding.isEmpty() )
      mCache.append( c );
  }
  if ( maxSync > 0 )
    mLastSync = QDateTime::fromMSecsSinceEpoch( maxSync );

  return true;
}

bool QgsAiWorkspaceIndex::persistAll( const QList<CachedChunk> &chunks, QString *errorMessage )
{
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
    "relative_path TEXT NOT NULL, "
    "chunk_index INTEGER NOT NULL, "
    "text TEXT NOT NULL, "
    "embedding BLOB NOT NULL, "
    "last_sync INTEGER NOT NULL"
    ")"
  ) );
  q.exec( u"DELETE FROM chunks"_s );

  if ( !db.transaction() )
  {
    if ( errorMessage )
      *errorMessage = u"Cannot start SQLite transaction: %1"_s.arg( db.lastError().text() );
    return false;
  }

  const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
  q.prepare( u"INSERT INTO chunks (relative_path, chunk_index, text, embedding, last_sync) VALUES (?, ?, ?, ?, ?)"_s );
  for ( const CachedChunk &c : chunks )
  {
    q.addBindValue( c.chunk.relativePath );
    q.addBindValue( c.chunk.chunkIndex );
    q.addBindValue( c.chunk.text );
    q.addBindValue( vectorToBytes( c.embedding ) );
    q.addBindValue( nowMs );
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

bool QgsAiWorkspaceIndex::reindex( int maxFiles, QString *errorMessage )
{
  if ( !mContextProvider )
  {
    if ( errorMessage )
      *errorMessage = u"Workspace context provider is unavailable."_s;
    return false;
  }
  if ( !mEmbeddingClient || !mEmbeddingClient->hasApiKey() )
  {
    if ( errorMessage )
      *errorMessage = u"OpenAI API key is required for embeddings; configure it in AI Provider Settings."_s;
    return false;
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
    mCache.clear();
    mLastSync = QDateTime::currentDateTimeUtc();
    QString err;
    persistAll( {}, &err );
    return true;
  }

  // Build chunks for every file. We keep a parallel list of texts to embed so
  // we can call the OpenAI endpoint in larger batches than per-file.
  QList<CachedChunk> built;
  QStringList textsToEmbed;
  QList<int> backRefIndex; // for each text in textsToEmbed, the position in `built`
  for ( int fileIdx = 0; fileIdx < eligible.size(); ++fileIdx )
  {
    const QString rel = eligible.at( fileIdx );
    emit progress( fileIdx + 1, eligible.size(), rel );

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
      built.append( c );
      backRefIndex.append( built.size() - 1 );
      textsToEmbed.append( c.chunk.text );
    }
  }

  if ( textsToEmbed.isEmpty() )
  {
    mCache.clear();
    mLastSync = QDateTime::currentDateTimeUtc();
    QString err;
    persistAll( {}, &err );
    return true;
  }

  QList<QVector<float>> vectors;
  if ( !mEmbeddingClient->embed( textsToEmbed, vectors, errorMessage, EMBEDDING_BATCH ) )
    return false;
  if ( vectors.size() != textsToEmbed.size() )
  {
    if ( errorMessage )
      *errorMessage = u"Embedding count mismatch: expected %1 got %2"_s.arg( textsToEmbed.size() ).arg( vectors.size() );
    return false;
  }

  for ( int i = 0; i < vectors.size(); ++i )
    built[backRefIndex.at( i )].embedding = vectors.at( i );

  if ( !persistAll( built, errorMessage ) )
    return false;

  mCache = built;
  mLastSync = QDateTime::currentDateTimeUtc();
  mLoaded = true;
  QgsMessageLog::logMessage( u"Workspace index built: files=%1 chunks=%2"_s.arg( eligible.size() ).arg( built.size() ), u"AI/Index"_s, Qgis::MessageLevel::Info, false );
  return true;
}

QList<QgsAiWorkspaceIndex::Chunk> QgsAiWorkspaceIndex::search( const QString &query, int k, QString *errorMessage )
{
  ensureLoaded();
  QList<Chunk> results;
  if ( query.trimmed().isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Empty query."_s;
    return results;
  }
  if ( mCache.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Workspace index is empty. Run reindex_workspace first."_s;
    return results;
  }
  if ( !mEmbeddingClient || !mEmbeddingClient->hasApiKey() )
  {
    if ( errorMessage )
      *errorMessage = u"OpenAI API key is required for query embedding."_s;
    return results;
  }

  QStringList qList { query };
  QList<QVector<float>> qEmb;
  if ( !mEmbeddingClient->embed( qList, qEmb, errorMessage, 1 ) || qEmb.isEmpty() )
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

void QgsAiWorkspaceIndex::clear()
{
  mCache.clear();
  mLastSync = QDateTime();
  const QString path = dbPath();
  if ( !path.isEmpty() && QFileInfo::exists( path ) )
    QFile::remove( path );
}
