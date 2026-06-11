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
#include "qgsaisecretstore.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"

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

namespace
{
  bool requireStorageEncryption()
  {
    const QgsSettings settings;
    return settings.value( u"ai/storage/requireEncryption"_s, false ).toBool();
  }

  QString encryptedChatValueForPersistence( const QString &plain, bool *ok )
  {
    if ( ok )
      *ok = true;

    if ( !QgsAiSecretStore::storageEncryptionAvailable() )
      return plain;

    const QgsAiSecretStore::EncryptionResult encrypted = QgsAiSecretStore::tryEncryptValue( plain );
    if ( encrypted.ok )
      return encrypted.value;

    if ( requireStorageEncryption() )
    {
      if ( ok )
        *ok = false;
      QgsMessageLog::
        logMessage( encrypted.errorMessage.isEmpty() ? u"Chat history encryption failed; refusing plaintext persistence because encryption is required."_s : encrypted.errorMessage, u"AI/ChatHistory"_s, Qgis::MessageLevel::Warning, false );
      return QString();
    }

    QgsAiSecretStore::warnPlaintextStorageOnce();
    return plain;
  }
} // namespace

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

void QgsAiChatHistoryStore::migratePlaintextRows( QSqlDatabase &db )
{
  if ( !QgsAiSecretStore::storageEncryptionAvailable() )
  {
    QgsAiSecretStore::warnPlaintextStorageOnce();
    return;
  }

  if ( !db.transaction() )
    return;

  bool ok = true;

  // Collect first, update after: stepping a SELECT while UPDATE-ing the same
  // table on the same SQLite connection has undefined row-visiting behavior.

  // Messages: content + metadata_json.
  {
    struct PendingRow
    {
        QString id;
        QString content;
        QString metaJson;
    };
    QList<PendingRow> rows;
    QSqlQuery select( db );
    if ( select.exec(
           u"SELECT message_id, content, metadata_json FROM messages WHERE content NOT LIKE 'enc1:%' OR (metadata_json IS NOT NULL AND metadata_json != '' AND metadata_json NOT LIKE 'enc1:%')"_s
         ) )
    {
      while ( select.next() )
        rows.append( { select.value( 0 ).toString(), select.value( 1 ).toString(), select.value( 2 ).toString() } );
    }
    else
      ok = false;

    if ( ok )
    {
      QSqlQuery update( db );
      update.prepare( u"UPDATE messages SET content = ?, metadata_json = ? WHERE message_id = ?"_s );
      for ( const PendingRow &row : std::as_const( rows ) )
      {
        bool encrypted = true;
        const QString content = row.content.startsWith( "enc1:"_L1 ) ? row.content : encryptedChatValueForPersistence( row.content, &encrypted );
        if ( !encrypted )
        {
          ok = false;
          break;
        }
        const QString metaJson = row.metaJson.isEmpty() || row.metaJson.startsWith( "enc1:"_L1 ) ? row.metaJson : encryptedChatValueForPersistence( row.metaJson, &encrypted );
        if ( !encrypted )
        {
          ok = false;
          break;
        }
        update.addBindValue( content );
        update.addBindValue( metaJson );
        update.addBindValue( row.id );
        if ( !update.exec() )
        {
          ok = false;
          break;
        }
      }
    }
  }

  // Session titles.
  if ( ok )
  {
    QList<QPair<QString, QString>> rows; // id, title
    QSqlQuery select( db );
    if ( select.exec( u"SELECT id, title FROM sessions WHERE title NOT LIKE 'enc1:%'"_s ) )
    {
      while ( select.next() )
        rows.append( { select.value( 0 ).toString(), select.value( 1 ).toString() } );
    }
    else
      ok = false;

    if ( ok )
    {
      QSqlQuery update( db );
      update.prepare( u"UPDATE sessions SET title = ? WHERE id = ?"_s );
      for ( const QPair<QString, QString> &row : std::as_const( rows ) )
      {
        bool encrypted = true;
        const QString title = encryptedChatValueForPersistence( row.second, &encrypted );
        if ( !encrypted )
        {
          ok = false;
          break;
        }
        update.addBindValue( title );
        update.addBindValue( row.first );
        if ( !update.exec() )
        {
          ok = false;
          break;
        }
      }
    }
  }

  if ( ok )
  {
    db.commit();
    QgsMessageLog::logMessage( u"Chat history encrypted in place."_s, u"AI/ChatHistory"_s, Qgis::MessageLevel::Info, false );
  }
  else
  {
    // Rollback keeps the plaintext rows readable (per-value prefix detection).
    db.rollback();
    QgsMessageLog::logMessage( u"Chat history encryption migration failed; keeping plaintext rows."_s, u"AI/ChatHistory"_s, Qgis::MessageLevel::Warning, false );
  }
}

bool QgsAiChatHistoryStore::ensureReady( QString *errorMessage )
{
  if ( mReady )
    return true;

  // Hard-block plaintext persistence when the user requires encryption: the
  // in-memory chat keeps working, only SQLite persistence is disabled.
  if ( !QgsAiSecretStore::storageEncryptionAvailable() )
  {
    const QgsSettings settings;
    if ( settings.value( u"ai/storage/requireEncryption"_s, false ).toBool() )
    {
      if ( errorMessage )
        *errorMessage = u"Encryption is required (ai/storage/requireEncryption) but the authentication vault is unavailable."_s;
      return false;
    }
  }

  if ( !openDatabase( errorMessage ) )
    return false;

  QSqlDatabase db = QSqlDatabase::database( connectionName() );

  // Schema migration: v1 → v2 only changed the VALUE encoding (encryption at
  // rest), so it migrates IN PLACE and preserves the history. Truly unknown
  // older versions still reset.
  {
    QSqlQuery v( db );
    int currentVersion = 0;
    if ( v.exec( u"PRAGMA user_version"_s ) && v.next() )
      currentVersion = v.value( 0 ).toInt();
    if ( currentVersion == 1 )
    {
      migratePlaintextRows( db );
      QSqlQuery setVersion( db );
      // Per-value `enc1:` prefix detection keeps mixed rows readable, so the
      // version can be bumped even when encryption was not available yet.
      setVersion.exec( u"PRAGMA user_version = %1"_s.arg( SCHEMA_VERSION ) );
      QgsMessageLog::logMessage( u"Chat history schema upgrade: 1 → %1 (in place, history preserved)"_s.arg( SCHEMA_VERSION ), u"AI/ChatHistory"_s, Qgis::MessageLevel::Info, false );
    }
    else if ( currentVersion > 0 && currentVersion < SCHEMA_VERSION )
    {
      QgsMessageLog::logMessage( u"Chat history schema upgrade: %1 → %2 (resetting)"_s.arg( currentVersion ).arg( SCHEMA_VERSION ), u"AI/ChatHistory"_s, Qgis::MessageLevel::Info, false );
      QSqlQuery drop( db );
      drop.exec( u"DROP TABLE IF EXISTS messages"_s );
      drop.exec( u"DROP TABLE IF EXISTS sessions"_s );
    }
    else if ( currentVersion == SCHEMA_VERSION )
    {
      // Opportunistic sweep: plaintext rows written during a vault-less session
      // converge to encrypted once the vault becomes available.
      QSqlQuery probe( db );
      if ( QgsAiSecretStore::storageEncryptionAvailable()
           && probe.exec( u"SELECT EXISTS(SELECT 1 FROM messages WHERE content NOT LIKE 'enc1:%' LIMIT 1)"_s )
           && probe.next()
           && probe.value( 0 ).toInt() == 1 )
        migratePlaintextRows( db );
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
    s.title = QgsAiSecretStore::decryptValue( q.value( 1 ).toString() );
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
    m.content = QgsAiSecretStore::decryptValue( q.value( 2 ).toString() );
    m.timestamp = QDateTime::fromMSecsSinceEpoch( q.value( 3 ).toLongLong() );
    const QString metaJson = QgsAiSecretStore::decryptValue( q.value( 4 ).toString() );
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
  bool encryptionOk = true;
  q.addBindValue( id );
  q.addBindValue( encryptedChatValueForPersistence( title, &encryptionOk ) );
  if ( !encryptionOk )
    return false;
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
  bool encryptionOk = true;
  q.addBindValue( encryptedChatValueForPersistence( msg.content, &encryptionOk ) );
  if ( !encryptionOk )
    return false;
  q.addBindValue( msg.timestamp.isValid() ? msg.timestamp.toMSecsSinceEpoch() : QDateTime::currentMSecsSinceEpoch() );
  const QString metaJson = msg.metadata.isEmpty() ? QString() : QString::fromUtf8( QJsonDocument( QJsonObject::fromVariantMap( msg.metadata ) ).toJson( QJsonDocument::Compact ) );
  q.addBindValue( encryptedChatValueForPersistence( metaJson, &encryptionOk ) );
  if ( !encryptionOk )
    return false;
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
  bool encryptionOk = true;
  q.addBindValue( encryptedChatValueForPersistence( QString::fromUtf8( QJsonDocument( QJsonObject::fromVariantMap( metadata ) ).toJson( QJsonDocument::Compact ) ), &encryptionOk ) );
  if ( !encryptionOk )
    return false;
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
  bool encryptionOk = true;
  q.addBindValue( encryptedChatValueForPersistence( newTitle, &encryptionOk ) );
  if ( !encryptionOk )
    return false;
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
