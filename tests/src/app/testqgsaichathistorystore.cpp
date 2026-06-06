/***************************************************************************
  testqgsaichathistorystore.cpp
  --------------------------------
  begin                : June 2026
***************************************************************************/

#include "ai/qgsaichathistorystore.h"
#include "ai/qgsaifilecontextprovider.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QSignalSpy>
#include <QString>
#include <QTemporaryDir>
#include <QUuid>

using namespace Qt::StringLiterals;

class TestQgsAiChatHistoryStore : public QObject
{
    Q_OBJECT

  private slots:
    void persistsSessionsWhenWorkspaceConfigured();
    void updatesMessageMetadata();
    void workspaceRootChangeLoadsSeparateHistory();
    void workspaceRootSettingRoundTripsViaQgsSettings();
    void autoWorkspaceRootUsesProfileDirectoryWhenUnset();

  private:
    static QString expectedDbPath( const QString &workspaceRoot );
};

QString TestQgsAiChatHistoryStore::expectedDbPath( const QString &workspaceRoot )
{
  const QByteArray hash = QCryptographicHash::hash( workspaceRoot.toUtf8(), QCryptographicHash::Sha1 ).toHex().left( 16 );
  const QString dir = QgsApplication::qgisSettingsDirPath() + u"ai_chat"_s;
  return QDir( dir ).filePath( u"ws_%1.sqlite"_s.arg( QString::fromLatin1( hash ) ) );
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

void TestQgsAiChatHistoryStore::workspaceRootSettingRoundTripsViaQgsSettings()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  const QString workspacePath = QDir( tempDir.path() ).absolutePath();
  const QString settingsKey = u"geoai/workspace/root"_s;
  const QString legacyKey = u"qgis_ai/workspace/root"_s;

  QgsSettings settings;
  settings.remove( settingsKey );
  settings.remove( legacyKey );
  settings.setValue( settingsKey, workspacePath );

  QCOMPARE( settings.value( settingsKey ).toString(), workspacePath );
  QVERIFY( !settings.contains( legacyKey ) );

  settings.remove( settingsKey );
}

void TestQgsAiChatHistoryStore::autoWorkspaceRootUsesProfileDirectoryWhenUnset()
{
  const QString settingsKey = u"geoai/workspace/root"_s;
  const QString legacyKey = u"qgis_ai/workspace/root"_s;

  QgsSettings settings;
  settings.remove( settingsKey );
  settings.remove( legacyKey );

  QgsProject::instance()->setFileName( QString() );
  QgsProject::instance()->setPresetHomePath( QString() );

  const QString expectedRoot = QDir( QgsApplication::qgisSettingsDirPath() ).filePath( u"ai_workspace"_s );
  const QString resolvedRoot = QgsAiFileContextProvider::resolveWorkspaceRoot();

  QCOMPARE( resolvedRoot, QDir( expectedRoot ).absolutePath() );
  QVERIFY( QDir( resolvedRoot ).exists() );
  QCOMPARE( settings.value( settingsKey ).toString(), resolvedRoot );
  QVERIFY( !settings.contains( legacyKey ) );

  settings.remove( settingsKey );
}

QGSTEST_MAIN( TestQgsAiChatHistoryStore )
#include "testqgsaichathistorystore.moc"
