/***************************************************************************
  testqgsaichathistorystore.cpp
  --------------------------------
  begin                : June 2026
***************************************************************************/

#include "ai/qgsaichathistorystore.h"
#include "ai/qgsaifilecontextprovider.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QTemporaryDir>
#include <QUuid>

using namespace Qt::StringLiterals;

class TestQgsAiChatHistoryStore : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void persistsSessionsWhenWorkspaceConfigured();
    void updatesMessageMetadata();
    void workspaceRootChangeLoadsSeparateHistory();
    void projectScopesWithSameWorkspaceUseSeparateHistory();
    void emptyProjectScopeDoesNotUseWorkspaceHistory();
    void workspaceRootSettingRoundTripsViaQgsSettings();
    void autoWorkspaceRootUsesProfileDirectoryWhenUnset();
    void workspaceRootMigratesGeoAiLegacySetting();
    void chatV1MigratesInPlacePreservingHistory();
    void requireEncryptionBlocksPlaintextPersistence();

  private:
    static QString expectedDbPath( const QString &workspaceRoot );
    static QString expectedProjectDbPath( const QString &scopeKey );
};

void TestQgsAiChatHistoryStore::initTestCase()
{
  // Other suites sharing the same settings dir (e.g. the agent session manager
  // tests) leave history DBs behind; wipe them so the scope assertions below
  // always start from a clean slate.
  QDir( QgsApplication::qgisSettingsDirPath() + u"ai_chat"_s ).removeRecursively();
}

QString TestQgsAiChatHistoryStore::expectedDbPath( const QString &workspaceRoot )
{
  const QByteArray hash = QCryptographicHash::hash( workspaceRoot.toUtf8(), QCryptographicHash::Sha1 ).toHex().left( 16 );
  const QString dir = QgsApplication::qgisSettingsDirPath() + u"ai_chat"_s;
  return QDir( dir ).filePath( u"ws_%1.sqlite"_s.arg( QString::fromLatin1( hash ) ) );
}

QString TestQgsAiChatHistoryStore::expectedProjectDbPath( const QString &scopeKey )
{
  const QByteArray hash = QCryptographicHash::hash( scopeKey.toUtf8(), QCryptographicHash::Sha1 ).toHex().left( 16 );
  const QString dir = QgsApplication::qgisSettingsDirPath() + u"ai_chat"_s;
  return QDir( dir ).filePath( u"project_%1.sqlite"_s.arg( QString::fromLatin1( hash ) ) );
}

void TestQgsAiChatHistoryStore::persistsSessionsWhenWorkspaceConfigured()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiChatHistoryStore store( &contextProvider );

  const QString sessionId = QUuid::createUuid().toString( QUuid::WithoutBraces );
  QVERIFY( store.createSession( sessionId, u"First chat"_s, u"plan"_s ) );

  QgsAiChatMessage userMessage;
  userMessage.id = QUuid::createUuid().toString( QUuid::WithoutBraces );
  userMessage.role = QgsAiChatRole::User;
  userMessage.content = u"Hello history"_s;
  QVERIFY( store.appendMessage( sessionId, userMessage, 0 ) );

  const QList<QgsAiChatHistoryStore::SessionInfo> sessions = store.listSessions();
  QCOMPARE( sessions.size(), 1 );
  QCOMPARE( sessions.first().id, sessionId );
  QCOMPARE( sessions.first().title, u"First chat"_s );
  QCOMPARE( sessions.first().messageCount, 1 );

  const QList<QgsAiChatMessage> messages = store.loadMessages( sessionId );
  QCOMPARE( messages.size(), 1 );
  QCOMPARE( messages.first().content, u"Hello history"_s );

  const QString dbPath = expectedDbPath( QDir( tempDir.path() ).absolutePath() );
  QVERIFY( QFileInfo::exists( dbPath ) );
}

void TestQgsAiChatHistoryStore::updatesMessageMetadata()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiChatHistoryStore store( &contextProvider );

  const QString sessionId = QUuid::createUuid().toString( QUuid::WithoutBraces );
  QVERIFY( store.createSession( sessionId, u"Plan chat"_s, u"planner"_s ) );

  QgsAiChatMessage assistantMessage;
  assistantMessage.id = QUuid::createUuid().toString( QUuid::WithoutBraces );
  assistantMessage.role = QgsAiChatRole::Assistant;
  assistantMessage.content = u"<proposed_plan>Do it</proposed_plan>"_s;
  assistantMessage.metadata.insert( u"ui_kind"_s, u"plan"_s );
  assistantMessage.metadata.insert( u"plan_status"_s, u"pending"_s );
  QVERIFY( store.appendMessage( sessionId, assistantMessage, 0 ) );

  QVariantMap updated = assistantMessage.metadata;
  updated.insert( u"plan_status"_s, u"accepted"_s );
  QVERIFY( store.updateMessageMetadata( sessionId, assistantMessage.id, updated ) );

  const QList<QgsAiChatMessage> messages = store.loadMessages( sessionId );
  QCOMPARE( messages.size(), 1 );
  QCOMPARE( messages.first().metadata.value( u"ui_kind"_s ).toString(), u"plan"_s );
  QCOMPARE( messages.first().metadata.value( u"plan_status"_s ).toString(), u"accepted"_s );
}

void TestQgsAiChatHistoryStore::workspaceRootChangeLoadsSeparateHistory()
{
  QTemporaryDir root1;
  QTemporaryDir root2;
  QVERIFY( root1.isValid() );
  QVERIFY( root2.isValid() );

  QgsAiFileContextProvider contextProvider( root1.path() );
  QgsAiChatHistoryStore store( &contextProvider );
  QSignalSpy listSpy( &store, &QgsAiChatHistoryStore::sessionListChanged );

  const QString session1 = QUuid::createUuid().toString( QUuid::WithoutBraces );
  QVERIFY( store.createSession( session1, u"Workspace one"_s, u"plan"_s ) );
  QCOMPARE( store.listSessions().size(), 1 );

  contextProvider.setWorkspaceRoot( root2.path() );
  QCOMPARE( listSpy.count(), 1 );
  QCOMPARE( store.listSessions().size(), 0 );

  const QString session2 = QUuid::createUuid().toString( QUuid::WithoutBraces );
  QVERIFY( store.createSession( session2, u"Workspace two"_s, u"plan"_s ) );
  QCOMPARE( store.listSessions().size(), 1 );
  QCOMPARE( store.listSessions().first().title, u"Workspace two"_s );

  contextProvider.setWorkspaceRoot( root1.path() );
  QCOMPARE( listSpy.count(), 2 );
  QCOMPARE( store.listSessions().size(), 1 );
  QCOMPARE( store.listSessions().first().title, u"Workspace one"_s );
}

void TestQgsAiChatHistoryStore::projectScopesWithSameWorkspaceUseSeparateHistory()
{
  QTemporaryDir workspace;
  QVERIFY( workspace.isValid() );

  QgsAiFileContextProvider contextProvider( workspace.path() );
  QgsAiChatHistoryStore store( &contextProvider );

  const QString projectScope1 = u"project:file:/tmp/project-one.qgz"_s;
  const QString projectScope2 = u"project:file:/tmp/project-two.qgz"_s;

  store.setHistoryScopeKey( projectScope1 );
  QVERIFY( store.hasExplicitHistoryScopeKey() );
  QVERIFY( store.hasPersistentHistoryScope() );
  const QString session1 = QUuid::createUuid().toString( QUuid::WithoutBraces );
  QVERIFY( store.createSession( session1, u"Project one"_s, u"planner"_s ) );
  QCOMPARE( store.listSessions().size(), 1 );
  QCOMPARE( store.listSessions().first().title, u"Project one"_s );
  QVERIFY( QFileInfo::exists( expectedProjectDbPath( projectScope1 ) ) );

  store.setHistoryScopeKey( projectScope2 );
  QCOMPARE( store.listSessions().size(), 0 );
  const QString session2 = QUuid::createUuid().toString( QUuid::WithoutBraces );
  QVERIFY( store.createSession( session2, u"Project two"_s, u"planner"_s ) );
  QCOMPARE( store.listSessions().size(), 1 );
  QCOMPARE( store.listSessions().first().title, u"Project two"_s );

  store.setHistoryScopeKey( projectScope1 );
  QCOMPARE( store.listSessions().size(), 1 );
  QCOMPARE( store.listSessions().first().title, u"Project one"_s );
}

void TestQgsAiChatHistoryStore::emptyProjectScopeDoesNotUseWorkspaceHistory()
{
  QTemporaryDir workspace;
  QVERIFY( workspace.isValid() );

  QgsAiFileContextProvider contextProvider( workspace.path() );
  QgsAiChatHistoryStore store( &contextProvider );

  const QString legacySession = QUuid::createUuid().toString( QUuid::WithoutBraces );
  QVERIFY( store.createSession( legacySession, u"Legacy workspace chat"_s, u"planner"_s ) );
  QCOMPARE( store.listSessions().size(), 1 );

  store.setHistoryScopeKey( QString() );
  QVERIFY( store.hasExplicitHistoryScopeKey() );
  QVERIFY( !store.hasPersistentHistoryScope() );
  QCOMPARE( store.listSessions().size(), 0 );
  QVERIFY( !store.createSession( QUuid::createUuid().toString( QUuid::WithoutBraces ), u"Unsaved project"_s, u"planner"_s ) );

  store.clearHistoryScopeKey();
  QVERIFY( !store.hasExplicitHistoryScopeKey() );
  QVERIFY( store.hasPersistentHistoryScope() );
  QCOMPARE( store.listSessions().size(), 1 );
  QCOMPARE( store.listSessions().first().title, u"Legacy workspace chat"_s );
}

void TestQgsAiChatHistoryStore::workspaceRootSettingRoundTripsViaQgsSettings()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  const QString workspacePath = QDir( tempDir.path() ).absolutePath();
  const QString settingsKey = u"strata/workspace/root"_s;
  const QString geoAiLegacyKey = u"geoai/workspace/root"_s;
  const QString qgisAiLegacyKey = u"qgis_ai/workspace/root"_s;

  QgsSettings settings;
  settings.remove( settingsKey );
  settings.remove( geoAiLegacyKey );
  settings.remove( qgisAiLegacyKey );
  settings.setValue( settingsKey, workspacePath );

  QCOMPARE( settings.value( settingsKey ).toString(), workspacePath );
  QVERIFY( !settings.contains( geoAiLegacyKey ) );
  QVERIFY( !settings.contains( qgisAiLegacyKey ) );

  settings.remove( settingsKey );
}

void TestQgsAiChatHistoryStore::autoWorkspaceRootUsesProfileDirectoryWhenUnset()
{
  const QString settingsKey = u"strata/workspace/root"_s;
  const QString geoAiLegacyKey = u"geoai/workspace/root"_s;
  const QString qgisAiLegacyKey = u"qgis_ai/workspace/root"_s;

  QgsSettings settings;
  settings.remove( settingsKey );
  settings.remove( geoAiLegacyKey );
  settings.remove( qgisAiLegacyKey );

  QgsProject::instance()->setFileName( QString() );
  QgsProject::instance()->setPresetHomePath( QString() );

  const QString expectedRoot = QDir( QgsApplication::qgisSettingsDirPath() ).filePath( u"ai_workspace"_s );
  const QString resolvedRoot = QgsAiFileContextProvider::resolveWorkspaceRoot();

  QCOMPARE( resolvedRoot, QDir( expectedRoot ).absolutePath() );
  QVERIFY( QDir( resolvedRoot ).exists() );
  QCOMPARE( settings.value( settingsKey ).toString(), resolvedRoot );
  QVERIFY( !settings.contains( geoAiLegacyKey ) );
  QVERIFY( !settings.contains( qgisAiLegacyKey ) );

  settings.remove( settingsKey );
}

void TestQgsAiChatHistoryStore::workspaceRootMigratesGeoAiLegacySetting()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  const QString workspacePath = QDir( tempDir.path() ).absolutePath();
  const QString settingsKey = u"strata/workspace/root"_s;
  const QString geoAiLegacyKey = u"geoai/workspace/root"_s;
  const QString qgisAiLegacyKey = u"qgis_ai/workspace/root"_s;

  QgsSettings settings;
  settings.remove( settingsKey );
  settings.remove( geoAiLegacyKey );
  settings.remove( qgisAiLegacyKey );
  settings.setValue( geoAiLegacyKey, workspacePath );

  QgsProject::instance()->setFileName( QString() );
  QgsProject::instance()->setPresetHomePath( QString() );

  const QString resolvedRoot = QgsAiFileContextProvider::resolveWorkspaceRoot();
  QCOMPARE( resolvedRoot, workspacePath );
  QCOMPARE( settings.value( settingsKey ).toString(), workspacePath );
  QVERIFY( !settings.contains( geoAiLegacyKey ) );
  QVERIFY( !settings.contains( qgisAiLegacyKey ) );

  settings.remove( settingsKey );
}

void TestQgsAiChatHistoryStore::chatV1MigratesInPlacePreservingHistory()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  // Build a v1 database by hand (plaintext rows, user_version = 1).
  const QString dbPath = expectedDbPath( QDir( tempDir.path() ).absolutePath() );
  QDir().mkpath( QFileInfo( dbPath ).absolutePath() );
  {
    QSqlDatabase db = QSqlDatabase::addDatabase( u"QSQLITE"_s, u"chat_v1_fixture"_s );
    db.setDatabaseName( dbPath );
    QVERIFY( db.open() );
    QSqlQuery q( db );
    QVERIFY( q.exec( u"CREATE TABLE sessions (id TEXT PRIMARY KEY, title TEXT NOT NULL, agent TEXT, created_at INTEGER NOT NULL, updated_at INTEGER NOT NULL)"_s ) );
    QVERIFY( q.exec(
      u"CREATE TABLE messages (message_id TEXT PRIMARY KEY, session_id TEXT NOT NULL, role TEXT NOT NULL, content TEXT NOT NULL, timestamp INTEGER NOT NULL, metadata_json TEXT, ordering INTEGER NOT NULL, FOREIGN KEY (session_id) REFERENCES sessions(id) ON DELETE CASCADE)"_s
    ) );
    QVERIFY( q.exec( u"INSERT INTO sessions VALUES ('s1', 'Legacy chat', 'plan', 1, 1)"_s ) );
    QVERIFY( q.exec( u"INSERT INTO messages VALUES ('m1', 's1', 'User', 'old plaintext message', 1, NULL, 0)"_s ) );
    QVERIFY( q.exec( u"PRAGMA user_version = 1"_s ) );
    db.close();
  }
  QSqlDatabase::removeDatabase( u"chat_v1_fixture"_s );

  // Opening through the store migrates in place: the history is PRESERVED.
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiChatHistoryStore store( &contextProvider );

  const QList<QgsAiChatHistoryStore::SessionInfo> sessions = store.listSessions();
  QCOMPARE( sessions.size(), 1 );
  QCOMPARE( sessions.first().title, u"Legacy chat"_s );

  const QList<QgsAiChatMessage> messages = store.loadMessages( u"s1"_s );
  QCOMPARE( messages.size(), 1 );
  QCOMPARE( messages.first().content, u"old plaintext message"_s );

  // The version was bumped (no DROP happened).
  {
    QSqlDatabase db = QSqlDatabase::addDatabase( u"QSQLITE"_s, u"chat_v1_verify"_s );
    db.setDatabaseName( dbPath );
    QVERIFY( db.open() );
    QSqlQuery v( db );
    QVERIFY( v.exec( u"PRAGMA user_version"_s ) && v.next() );
    QCOMPARE( v.value( 0 ).toInt(), QgsAiChatHistoryStore::SCHEMA_VERSION );
    db.close();
  }
  QSqlDatabase::removeDatabase( u"chat_v1_verify"_s );
}

void TestQgsAiChatHistoryStore::requireEncryptionBlocksPlaintextPersistence()
{
  QgsAuthManager *authManager = QgsApplication::authManager();
  if ( authManager && authManager->masterPasswordIsSet() )
    QSKIP( "Master password set: encryption is available, the block path is not reachable." );

  QgsSettings settings;
  settings.setValue( u"ai/storage/requireEncryption"_s, true );
  const auto cleanup = qScopeGuard( [&settings]() { settings.remove( u"ai/storage/requireEncryption"_s ); } );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiChatHistoryStore store( &contextProvider );

  QString error;
  QVERIFY( !store.ensureReady( &error ) );
  QVERIFY2( error.contains( u"Encryption is required"_s ), qPrintable( error ) );
  QVERIFY( !store.createSession( u"s1"_s, u"blocked"_s, u"plan"_s ) );
}

QGSTEST_MAIN( TestQgsAiChatHistoryStore )
#include "testqgsaichathistorystore.moc"
