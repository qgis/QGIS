/***************************************************************************
  testqgsaiagentsessionmanager.cpp
  --------------------------------
  begin                : April 2026
***************************************************************************/

#include "ai/qgsaiagentsessionmanager.h"
#include "ai/qgsaifilecontextprovider.h"
#include "ai/qgsaimodelrouter.h"
#include "ai/qgsaireviewpatchengine.h"
#include "ai/tools/qgsaiechotool.h"
#include "ai/tools/qgsaitoolregistry.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

#include <memory>

class TestQgsAiAgentSessionManager : public QObject
{
    Q_OBJECT

  private slots:
    void createsPatchProposalFromCommand();
    void blocksContextOutsideWorkspace();
    void findsWorkspaceFilesForMentions();
    void allowsExplicitExternalAttachmentContext();
    void agentBehaviorSettingsRoundTrip();
    void agentBehaviorTogglePropagatesToRouter();
    void collectsInlineRulesAndSkills();
    void collectsWorkspaceRulesFiles();
    void rejectsRulesFolderOutsideWorkspace();
};

void TestQgsAiAgentSessionManager::createsPatchProposalFromCommand()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  QSignalSpy proposalSpy( &manager, &QgsAiAgentSessionManager::proposalCreated );
  QSignalSpy messageSpy( &manager, &QgsAiAgentSessionManager::messageAdded );

  manager.sendUserMessage( QStringLiteral( "/patch path=%1\n<<<<\nold\n====\nnew\n>>>>" ).arg( tempDir.filePath( QStringLiteral( "a.txt" ) ) ) );
  QCOMPARE( proposalSpy.count(), 1 );
  QVERIFY( messageSpy.count() >= 2 );
}

void TestQgsAiAgentSessionManager::blocksContextOutsideWorkspace()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  QSignalSpy stateSpy( &manager, &QgsAiAgentSessionManager::requestStateChanged );
  manager.sendUserMessage( QStringLiteral( "hello" ), QStringLiteral( "/etc/passwd" ) );
  QVERIFY( stateSpy.count() >= 1 );
  const QList<QVariant> args = stateSpy.takeLast();
  QCOMPARE( args.at( 0 ).toString(), QStringLiteral( "error" ) );
  QVERIFY( args.at( 1 ).toString().contains( QStringLiteral( "blocked" ), Qt::CaseInsensitive ) );
}

void TestQgsAiAgentSessionManager::findsWorkspaceFilesForMentions()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QVERIFY( QDir( tempDir.path() ).mkpath( QStringLiteral( "src/app" ) ) );
  QFile file( tempDir.filePath( QStringLiteral( "src/app/main.cpp" ) ) );
  QVERIFY( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
  file.write( "int main() { return 0; }\n" );
  file.close();

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  const QStringList candidates = contextProvider.workspaceFileCandidates( QStringLiteral( "main" ), 5 );
  QVERIFY( candidates.contains( QStringLiteral( "src/app/main.cpp" ) ) );
  QCOMPARE( contextProvider.resolveWorkspaceFile( QStringLiteral( "src/app/main.cpp" ) ), QDir::cleanPath( file.fileName() ) );
}

void TestQgsAiAgentSessionManager::allowsExplicitExternalAttachmentContext()
{
  QTemporaryDir workspaceDir;
  QVERIFY( workspaceDir.isValid() );
  QTemporaryDir externalDir;
  QVERIFY( externalDir.isValid() );

  QFile externalFile( externalDir.filePath( QStringLiteral( "data.txt" ) ) );
  QVERIFY( externalFile.open( QIODevice::WriteOnly | QIODevice::Text ) );
  externalFile.write( "external data\n" );
  externalFile.close();

  QgsAiFileContextProvider contextProvider( workspaceDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  QgsAiChatContextFile contextFile;
  contextFile.filePath = externalFile.fileName();
  contextFile.allowExternal = true;

  QSignalSpy stateSpy( &manager, &QgsAiAgentSessionManager::requestStateChanged );
  QSignalSpy messageSpy( &manager, &QgsAiAgentSessionManager::messageAdded );
  manager.sendUserMessage( QStringLiteral( "hello" ), QList<QgsAiChatContextFile>() << contextFile );

  QVERIFY( stateSpy.isEmpty() );
  QVERIFY( messageSpy.count() >= 2 );
  QVERIFY( manager.history().last().content.contains( QStringLiteral( "No provider" ), Qt::CaseInsensitive ) );
}

void TestQgsAiAgentSessionManager::agentBehaviorSettingsRoundTrip()
{
  QgsSettings settings;
  settings.remove( QStringLiteral( "qgis_ai/agent" ) );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  {
    QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
    const QgsAiAgentBehaviorSettings defaults = manager.agentBehaviorSettings();
    QCOMPARE( defaults.allowCustomActions, false );
    QVERIFY( defaults.rulesText.isEmpty() );
    QVERIFY( defaults.skillsText.isEmpty() );
    QCOMPARE( defaults.rulesPath, QStringLiteral( ".qgis_ai/rules" ) );

    QgsAiAgentBehaviorSettings updated = defaults;
    updated.allowCustomActions = true;
    updated.rulesText = QStringLiteral( "Always answer in English." );
    updated.skillsText = QStringLiteral( "Prefer GeoPandas." );
    updated.rulesPath = QStringLiteral( "ai/rules" );
    updated.skillsPath = QString();
    manager.setAgentBehaviorSettings( updated );

    const QgsAiAgentBehaviorSettings reread = manager.agentBehaviorSettings();
    QCOMPARE( reread.allowCustomActions, true );
    QCOMPARE( reread.rulesText, QStringLiteral( "Always answer in English." ) );
    QCOMPARE( reread.skillsText, QStringLiteral( "Prefer GeoPandas." ) );
    QCOMPARE( reread.rulesPath, QStringLiteral( "ai/rules" ) );
    // Empty skill path must fall back to the default folder so the file loader stays predictable.
    QCOMPARE( reread.skillsPath, QStringLiteral( ".qgis_ai/skills" ) );
  }

  QgsAiAgentSessionManager reloaded( nullptr, &contextProvider, &reviewEngine );
  const QgsAiAgentBehaviorSettings restored = reloaded.agentBehaviorSettings();
  QCOMPARE( restored.allowCustomActions, true );
  QCOMPARE( restored.rulesText, QStringLiteral( "Always answer in English." ) );
  QCOMPARE( restored.skillsText, QStringLiteral( "Prefer GeoPandas." ) );

  settings.remove( QStringLiteral( "qgis_ai/agent" ) );
}

void TestQgsAiAgentSessionManager::agentBehaviorTogglePropagatesToRouter()
{
  QgsSettings settings;
  settings.remove( QStringLiteral( "qgis_ai/agent" ) );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<QgsAiEchoTool>() );

  QgsAiModelRouter router;
  router.setToolRegistry( &registry );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( &router, &contextProvider, &reviewEngine );

  // Default: tool use stays off until the user opts in.
  QCOMPARE( router.toolUseEnabled(), false );

  QgsAiAgentBehaviorSettings updated = manager.agentBehaviorSettings();
  updated.allowCustomActions = true;
  manager.setAgentBehaviorSettings( updated );
  QCOMPARE( router.toolUseEnabled(), true );

  updated.allowCustomActions = false;
  manager.setAgentBehaviorSettings( updated );
  QCOMPARE( router.toolUseEnabled(), false );

  settings.remove( QStringLiteral( "qgis_ai/agent" ) );
}

void TestQgsAiAgentSessionManager::collectsInlineRulesAndSkills()
{
  QgsSettings settings;
  settings.remove( QStringLiteral( "qgis_ai/agent" ) );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  QgsAiAgentBehaviorSettings updated = manager.agentBehaviorSettings();
  updated.rulesText = QStringLiteral( "  Be concise.  " );
  updated.skillsText = QStringLiteral( "Use OSMnx for graphs." );
  updated.loadWorkspaceRules = false;
  updated.loadWorkspaceSkills = false;
  manager.setAgentBehaviorSettings( updated );

  QCOMPARE( manager.collectRulesContent(), QStringLiteral( "Be concise." ) );
  QCOMPARE( manager.collectSkillsContent(), QStringLiteral( "Use OSMnx for graphs." ) );

  settings.remove( QStringLiteral( "qgis_ai/agent" ) );
}

void TestQgsAiAgentSessionManager::collectsWorkspaceRulesFiles()
{
  QgsSettings settings;
  settings.remove( QStringLiteral( "qgis_ai/agent" ) );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QVERIFY( QDir( tempDir.path() ).mkpath( QStringLiteral( ".qgis_ai/rules" ) ) );
  QFile rulesFile( tempDir.filePath( QStringLiteral( ".qgis_ai/rules/coding.md" ) ) );
  QVERIFY( rulesFile.open( QIODevice::WriteOnly | QIODevice::Text ) );
  rulesFile.write( "- Always run linters.\n" );
  rulesFile.close();

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  QgsAiAgentBehaviorSettings updated = manager.agentBehaviorSettings();
  updated.rulesText.clear();
  updated.loadWorkspaceRules = true;
  manager.setAgentBehaviorSettings( updated );

  const QString rules = manager.collectRulesContent();
  QVERIFY( rules.contains( QStringLiteral( "Always run linters." ) ) );
  QVERIFY( rules.contains( QStringLiteral( ".qgis_ai/rules/coding.md" ) ) );

  settings.remove( QStringLiteral( "qgis_ai/agent" ) );
}

void TestQgsAiAgentSessionManager::rejectsRulesFolderOutsideWorkspace()
{
  QgsSettings settings;
  settings.remove( QStringLiteral( "qgis_ai/agent" ) );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  QgsAiAgentBehaviorSettings updated = manager.agentBehaviorSettings();
  updated.rulesText.clear();
  updated.loadWorkspaceRules = true;
  // Path that escapes the workspace must be rejected silently rather than reading anything.
  updated.rulesPath = QStringLiteral( "../../etc" );
  manager.setAgentBehaviorSettings( updated );

  QVERIFY( manager.collectRulesContent().isEmpty() );

  settings.remove( QStringLiteral( "qgis_ai/agent" ) );
}

QGSTEST_MAIN( TestQgsAiAgentSessionManager )
#include "testqgsaiagentsessionmanager.moc"
