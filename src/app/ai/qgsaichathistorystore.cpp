/***************************************************************************
    qgsaichathistorystore.cpp
    ---------------------
    begin                : May 2026
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

#include "qgsaichathistorystore.h"

#include "qgsaifilecontextprovider.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QUuid>
#include <QVariant>

#include "moc_qgsaichathistorystore.cpp"

using namespace Qt::StringLiterals;

QgsAiChatHistoryStore::QgsAiChatHistoryStore( QgsAiFileContextProvider *contextProvider, QObject *parent )
  : QObject( parent )
  , mContextProvider( contextProvider )
{
  if ( mContextProvider )
    connect( mContextProvider, &QgsAiFileContextProvider::workspaceRootChanged, this, &QgsAiChatHistoryStore::onWorkspaceRootChanged );
}

QgsAiChatHistoryStore::~QgsAiChatHistoryStore()
{
  resetDatabaseConnection();
}

void QgsAiChatHistoryStore::resetDatabaseConnection()
{
  const QString name = connectionName();
  if ( QSqlDatabase::contains( name ) )
  {
    {
      QSqlDatabase db = QSqlDatabase::database( name );
      db.close();
    }
    QSqlDatabase::removeDatabase( name );
  }
  mReady = false;
}

void QgsAiChatHistoryStore::onWorkspaceRootChanged()
{
  if ( mHasExplicitHistoryScopeKey )
    return;

  resetDatabaseConnection();
  emit sessionListChanged();
}

QString QgsAiChatHistoryStore::connectionName() const
{
  return u"qgsai_chat_%1"_s.arg( reinterpret_cast<quintptr>( this ) );
}

QString QgsAiChatHistoryStore::dbPath() const
{
  if ( mHasExplicitHistoryScopeKey )
  {
    const QString scope = mHistoryScopeKey.trimmed();
    if ( scope.isEmpty() )
      return QString();

    const QByteArray hash = QCryptographicHash::hash( scope.toUtf8(), QCryptographicHash::Sha1 ).toHex().left( 16 );
    const QString dir = QgsApplication::qgisSettingsDirPath() + u"ai_chat"_s;
    QDir().mkpath( dir );
    return QDir( dir ).filePath( u"project_%1.sqlite"_s.arg( QString::fromLatin1( hash ) ) );
  }

  if ( !mContextProvider )
    return QString();
  const QString root = mContextProvider->workspaceRoot();
  if ( root.isEmpty() )
    return QString();

  const QByteArray hash = QCryptographicHash::hash( root.toUtf8(), QCryptographicHash::Sha1 ).toHex().left( 16 );
  const QString dir = QgsApplication::qgisSettingsDirPath() + u"ai_chat"_s;
  QDir().mkpath( dir );
  return QDir( dir ).filePath( u"ws_%1.sqlite"_s.arg( QString::fromLatin1( hash ) ) );
}

void QgsAiChatHistoryStore::setHistoryScopeKey( const QString &scopeKey )
{
  const QString normalizedScope = scopeKey.trimmed();
  if ( mHasExplicitHistoryScopeKey && mHistoryScopeKey == normalizedScope )
    return;

  mHasExplicitHistoryScopeKey = true;
  mHistoryScopeKey = normalizedScope;
  resetDatabaseConnection();
  emit sessionListChanged();
}

void QgsAiChatHistoryStore::clearHistoryScopeKey()
{
  if ( !mHasExplicitHistoryScopeKey )
    return;

  mHasExplicitHistoryScopeKey = false;
  mHistoryScopeKey.clear();
  resetDatabaseConnection();
  emit sessionListChanged();
}

bool QgsAiChatHistoryStore::hasPersistentHistoryScope() const
{
  if ( mHasExplicitHistoryScopeKey )
    return !mHistoryScopeKey.trimmed().isEmpty();

  return mContextProvider && !mContextProvider->workspaceRoot().isEmpty();
}

bool QgsAiChatHistoryStore::openDatabase( QString *errorMessage ) const
{
  const QString path = dbPath();
  if ( path.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = mHasExplicitHistoryScopeKey ? u"QGIS project is unsaved; cannot open chat history."_s : u"Workspace root is unset; cannot open chat history."_s;
    return false;
  }

  QSqlDatabase db = QSqlDatabase::contains( connectionName() ) ? QSqlDatabase::database( connectionName() ) : QSqlDatabase::addDatabase( u"QSQLITE"_s, connectionName() );
  if ( db.databaseName() != path )
  {
    if ( db.isOpen() )
      db.close();
    db.setDatabaseName( path );
  }
  if ( !db.isOpen() && !db.open() )
  {
    if ( errorMessage )
      *errorMessage = db.lastError().text();
    return false;
  }

  QSqlQuery pragma( db );
  pragma.exec( u"PRAGMA foreign_keys = ON"_s );
  return true;
}

bool QgsAiChatHistoryStore::ensureReady( QString *errorMessage )
{
  if ( mReady )
    return true;

  if ( !openDatabase( errorMessage ) )
    return false;

  QSqlDatabase db = QSqlDatabase::database( connectionName() );

  // Schema migration: read PRAGMA user_version. If older than SCHEMA_VERSION
  // drop the tables — chat history is non-critical user data; an upgrade that
  // changes the schema will reset it.
  {
    QSqlQuery v( db );
    int currentVersion = 0;
    if ( v.exec( u"PRAGMA user_version"_s ) && v.next() )
      currentVersion = v.value( 0 ).toInt();
    if ( currentVersion > 0 && currentVersion < SCHEMA_VERSION )
    {
      QgsMessageLog::logMessage( u"Chat history schema upgrade: %1 → %2 (resetting)"_s.arg( currentVersion ).arg( SCHEMA_VERSION ), u"AI/ChatHistory"_s, Qgis::MessageLevel::Info, false );
      QSqlQuery drop( db );
      drop.exec( u"DROP TABLE IF EXISTS messages"_s );
      drop.exec( u"DROP TABLE IF EXISTS sessions"_s );
    }
  }

  QSqlQuery q( db );
  if ( !q.exec( QStringLiteral(
         "CREATE TABLE IF NOT EXISTS sessions ("
         "id TEXT PRIMARY KEY, "
         "title TEXT NOT NULL, "
         "agent TEXT, "
         "created_at INTEGER NOT NULL, "
         "updated_at INTEGER NOT NULL"
         ")"
       ) ) )
  {
    if ( errorMessage )
      *errorMessage = q.lastError().text();
    return false;
  }
  if ( !q.exec( QStringLiteral(
         "CREATE TABLE IF NOT EXISTS messages ("
         "message_id TEXT PRIMARY KEY, "
         "session_id TEXT NOT NULL, "
         "role TEXT NOT NULL, "
         "content TEXT NOT NULL, "
         "timestamp INTEGER NOT NULL, "
         "metadata_json TEXT, "
         "ordering INTEGER NOT NULL, "
         "FOREIGN KEY (session_id) REFERENCES sessions(id) ON DELETE CASCADE"
         ")"
       ) ) )
  {
    if ( errorMessage )
      *errorMessage = q.lastError().text();
    return false;
  }
  q.exec( u"CREATE INDEX IF NOT EXISTS idx_messages_session ON messages(session_id, ordering)"_s );
  q.exec( u"PRAGMA user_version = %1"_s.arg( SCHEMA_VERSION ) );

  mReady = true;
  return true;
}

QList<QgsAiChatHistoryStore::SessionInfo> QgsAiChatHistoryStore::listSessions() const
{
  QList<SessionInfo> sessions;
  if ( !const_cast<QgsAiChatHistoryStore *>( this )->ensureReady() )
    return sessions;

  QSqlDatabase db = QSqlDatabase::database( connectionName() );
  QSqlQuery q( db );
  if ( !q.exec( QStringLiteral(
         "SELECT s.id, s.title, s.agent, s.created_at, s.updated_at, "
         "(SELECT COUNT(*) FROM messages m WHERE m.session_id = s.id) AS msg_count "
         "FROM sessions s ORDER BY s.updated_at DESC"
       ) ) )
  {
    QgsMessageLog::logMessage( u"listSessions failed: %1"_s.arg( q.lastError().text() ), u"AI/ChatHistory"_s, Qgis::MessageLevel::Warning, false );
    return sessions;
  }

  while ( q.next() )
  {
    SessionInfo s;
    s.id = q.value( 0 ).toString();
    s.title = q.value( 1 ).toString();
    s.agent = q.value( 2 ).toString();
    s.createdAt = QDateTime::fromMSecsSinceEpoch( q.value( 3 ).toLongLong() );
    s.updatedAt = QDateTime::fromMSecsSinceEpoch( q.value( 4 ).toLongLong() );
    s.messageCount = q.value( 5 ).toInt();
    sessions.append( s );
  }
  return sessions;
}

QList<QgsAiChatMessage> QgsAiChatHistoryStore::loadMessages( const QString &sessionId ) const
{
  QList<QgsAiChatMessage> out;
  if ( sessionId.isEmpty() )
    return out;
  if ( !const_cast<QgsAiChatHistoryStore *>( this )->ensureReady() )
    return out;

  QSqlDatabase db = QSqlDatabase::database( connectionName() );
  QSqlQuery q( db );
  q.prepare( u"SELECT message_id, role, content, timestamp, metadata_json FROM messages WHERE session_id = ? ORDER BY ordering"_s );
  q.addBindValue( sessionId );
  if ( !q.exec() )
  {
    QgsMessageLog::logMessage( u"loadMessages failed: %1"_s.arg( q.lastError().text() ), u"AI/ChatHistory"_s, Qgis::MessageLevel::Warning, false );
    return out;
  }

  while ( q.next() )
  {
    QgsAiChatMessage m;
    m.id = q.value( 0 ).toString();
    m.role = qgsAiChatRoleFromString( q.value( 1 ).toString() );
    m.content = q.value( 2 ).toString();
    m.timestamp = QDateTime::fromMSecsSinceEpoch( q.value( 3 ).toLongLong() );
    const QString metaJson = q.value( 4 ).toString();
    if ( !metaJson.isEmpty() )
    {
      const QJsonDocument doc = QJsonDocument::fromJson( metaJson.toUtf8() );
      if ( doc.isObject() )
        m.metadata = doc.object().toVariantMap();
    }
    out.append( m );
  }
  return out;
}

bool QgsAiChatHistoryStore::createSession( const QString &id, const QString &title, const QString &agent )
{
  if ( !ensureReady() )
    return false;

  QSqlDatabase db = QSqlDatabase::database( connectionName() );
  QSqlQuery q( db );
  const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
  q.prepare( u"INSERT INTO sessions (id, title, agent, created_at, updated_at) VALUES (?, ?, ?, ?, ?)"_s );
  q.addBindValue( id );
  q.addBindValue( title );
  q.addBindValue( agent );
  q.addBindValue( nowMs );
  q.addBindValue( nowMs );
  if ( !q.exec() )
  {
    QgsMessageLog::logMessage( u"createSession failed: %1"_s.arg( q.lastError().text() ), u"AI/ChatHistory"_s, Qgis::MessageLevel::Warning, false );
    return false;
  }
  return true;
}

bool QgsAiChatHistoryStore::appendMessage( const QString &sessionId, const QgsAiChatMessage &msg, int ordering )
{
  if ( sessionId.isEmpty() )
    return false;
  if ( !ensureReady() )
    return false;

  QSqlDatabase db = QSqlDatabase::database( connectionName() );
  QSqlQuery q( db );
  q.prepare( QStringLiteral(
    "INSERT OR REPLACE INTO messages "
    "(message_id, session_id, role, content, timestamp, metadata_json, ordering) "
    "VALUES (?, ?, ?, ?, ?, ?, ?)"
  ) );
  q.addBindValue( msg.id.isEmpty() ? QUuid::createUuid().toString( QUuid::WithoutBraces ) : msg.id );
  q.addBindValue( sessionId );
  q.addBindValue( qgsAiChatRoleToString( msg.role ) );
  q.addBindValue( msg.content );
  q.addBindValue( msg.timestamp.isValid() ? msg.timestamp.toMSecsSinceEpoch() : QDateTime::currentMSecsSinceEpoch() );
  const QString metaJson = msg.metadata.isEmpty() ? QString() : QString::fromUtf8( QJsonDocument( QJsonObject::fromVariantMap( msg.metadata ) ).toJson( QJsonDocument::Compact ) );
  q.addBindValue( metaJson );
  q.addBindValue( ordering );
  if ( !q.exec() )
  {
    QgsMessageLog::logMessage( u"appendMessage failed: %1"_s.arg( q.lastError().text() ), u"AI/ChatHistory"_s, Qgis::MessageLevel::Warning, false );
    return false;
  }

  return touchSession( sessionId );
}

bool QgsAiChatHistoryStore::updateMessageMetadata( const QString &sessionId, const QString &messageId, const QVariantMap &metadata )
{
  if ( sessionId.isEmpty() || messageId.isEmpty() )
    return false;
  if ( !ensureReady() )
    return false;

  QSqlDatabase db = QSqlDatabase::database( connectionName() );
  QSqlQuery q( db );
  q.prepare( u"UPDATE messages SET metadata_json = ? WHERE session_id = ? AND message_id = ?"_s );
  q.addBindValue( QString::fromUtf8( QJsonDocument( QJsonObject::fromVariantMap( metadata ) ).toJson( QJsonDocument::Compact ) ) );
  q.addBindValue( sessionId );
  q.addBindValue( messageId );
  if ( !q.exec() )
  {
    QgsMessageLog::logMessage( u"updateMessageMetadata failed: %1"_s.arg( q.lastError().text() ), u"AI/ChatHistory"_s, Qgis::MessageLevel::Warning, false );
    return false;
  }
  if ( q.numRowsAffected() <= 0 )
    return false;

  return touchSession( sessionId );
}

bool QgsAiChatHistoryStore::renameSession( const QString &sessionId, const QString &newTitle )
{
  if ( !ensureReady() )
    return false;

  QSqlDatabase db = QSqlDatabase::database( connectionName() );
  QSqlQuery q( db );
  q.prepare( u"UPDATE sessions SET title = ?, updated_at = ? WHERE id = ?"_s );
  q.addBindValue( newTitle );
  q.addBindValue( QDateTime::currentMSecsSinceEpoch() );
  q.addBindValue( sessionId );
  if ( !q.exec() )
  {
    QgsMessageLog::logMessage( u"renameSession failed: %1"_s.arg( q.lastError().text() ), u"AI/ChatHistory"_s, Qgis::MessageLevel::Warning, false );
    return false;
  }
  return q.numRowsAffected() > 0;
}

bool QgsAiChatHistoryStore::deleteSession( const QString &sessionId )
{
  if ( !ensureReady() )
    return false;

  QSqlDatabase db = QSqlDatabase::database( connectionName() );
  // ON DELETE CASCADE on messages.session_id handles the children, but
  // foreign_keys may have been turned off; delete messages explicitly too.
  QSqlQuery delMsg( db );
  delMsg.prepare( u"DELETE FROM messages WHERE session_id = ?"_s );
  delMsg.addBindValue( sessionId );
  delMsg.exec();

  QSqlQuery q( db );
  q.prepare( u"DELETE FROM sessions WHERE id = ?"_s );
  q.addBindValue( sessionId );
  if ( !q.exec() )
  {
    QgsMessageLog::logMessage( u"deleteSession failed: %1"_s.arg( q.lastError().text() ), u"AI/ChatHistory"_s, Qgis::MessageLevel::Warning, false );
    return false;
  }
  return q.numRowsAffected() > 0;
}

bool QgsAiChatHistoryStore::touchSession( const QString &sessionId )
{
  if ( !ensureReady() )
    return false;

  QSqlDatabase db = QSqlDatabase::database( connectionName() );
  QSqlQuery q( db );
  q.prepare( u"UPDATE sessions SET updated_at = ? WHERE id = ?"_s );
  q.addBindValue( QDateTime::currentMSecsSinceEpoch() );
  q.addBindValue( sessionId );
  if ( !q.exec() )
  {
    QgsMessageLog::logMessage( u"touchSession failed: %1"_s.arg( q.lastError().text() ), u"AI/ChatHistory"_s, Qgis::MessageLevel::Warning, false );
    return false;
  }
  return true;
}

int QgsAiChatHistoryStore::lastOrdering( const QString &sessionId ) const
{
  if ( sessionId.isEmpty() )
    return -1;
  if ( !const_cast<QgsAiChatHistoryStore *>( this )->ensureReady() )
    return -1;

  QSqlDatabase db = QSqlDatabase::database( connectionName() );
  QSqlQuery q( db );
  q.prepare( u"SELECT COALESCE(MAX(ordering), -1) FROM messages WHERE session_id = ?"_s );
  q.addBindValue( sessionId );
  if ( !q.exec() || !q.next() )
    return -1;
  return q.value( 0 ).toInt();
}
