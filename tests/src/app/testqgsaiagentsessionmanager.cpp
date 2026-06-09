/***************************************************************************
  testqgsaiagentsessionmanager.cpp
  --------------------------------
  begin                : April 2026
***************************************************************************/

#include <memory>

#include "ai/index/qgsaiembeddingprovider.h"
#include "ai/index/qgsaiworkspaceindex.h"
#include "ai/qgsaiagentsessionmanager.h"
#include "ai/qgsaichathistorystore.h"
#include "ai/qgsaifilecontextprovider.h"
#include "ai/qgsaimodelrouter.h"
#include "ai/qgsaireviewpatchengine.h"
#include "ai/tools/qgsaiechotool.h"
#include "ai/tools/qgsaitoolregistry.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QString>
#include <QTemporaryDir>
#include <QVector>

using namespace Qt::StringLiterals;

namespace
{
  class AvailabilityTool : public QgsAiTool
  {
    public:
      AvailabilityTool( const QString &name, bool available )
        : mName( name )
        , mAvailable( available )
      {}

      QString name() const override { return mName; }
      QString description() const override { return u"test tool"_s; }
      QJsonObject schema() const override
      {
        QJsonObject schema;
        schema.insert( u"type"_s, u"object"_s );
        schema.insert( u"properties"_s, QJsonObject() );
        return schema;
      }
      QgsAiToolResult execute( const QJsonObject & ) override { return QgsAiToolResult::ok( QJsonObject() ); }
      bool isAvailable() const override { return mAvailable; }
      QString availabilityReason() const override { return u"not available"_s; }

    private:
      QString mName;
      bool mAvailable = true;
  };

  void clearProviderSettings()
  {
    QgsSettings settings;
    settings.remove( u"ai/provider/plan"_s );
    settings.remove( u"ai/provider/codex"_s );
    settings.remove( u"ai/provider/openai"_s );
    settings.remove( u"ai/provider/claude"_s );
  }

  void forceProviderPreDispatchFailures( QgsAiModelRouter &router )
  {
    const QList<QgsAiModelRouter::Provider> providers = {
      QgsAiModelRouter::Provider::Plan,
      QgsAiModelRouter::Provider::Codex,
      QgsAiModelRouter::Provider::OpenAi,
      QgsAiModelRouter::Provider::Claude,
    };
    for ( QgsAiModelRouter::Provider provider : providers )
    {
      QgsAiModelRouter::ProviderSettings settings = router.providerSettings( provider );
      settings.endpoint.clear();
      settings.enabled = true;
      router.setProviderSettings( provider, settings );
    }
  }
} // namespace

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
    void planModeDoesNotAdvertiseTools();
    void askAndAgentAdvertiseCaptureMapCanvasTool();
    void agentModeOmitsUnavailableTools();
    void collectsInlineRulesAndSkills();
    void collectsWorkspaceRulesFiles();
    void collectsGeoAiWorkspaceRulesFiles();
    void collectsLegacyWorkspaceRulesFiles();
    void readsGeoAiAgentBehaviorSettings();
    void readsLegacyAgentBehaviorSettings();
    void rejectsRulesFolderOutsideWorkspace();
    void projectHistoryScopeChangeClearsActiveSessionAndTranscript();
    void unsavedProjectResetClearsEvenWhenScopeEmpty();
    void unsavedProjectFirstSavePromotesCurrentChat();
    void formatRetrievedContextRendersFileAndLayerHeaders();
    void formatRetrievedContextTruncatesOverBudget();
    void retrievalSkippedWithoutWorkspaceIndex();
    void preDispatchFailureUnlocksRunningState();
    void fallbackPreDispatchFailuresAreDrained();
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

  manager.sendUserMessage( u"/patch path=%1\n<<<<\nold\n====\nnew\n>>>>"_s.arg( tempDir.filePath( u"a.txt"_s ) ) );
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
  manager.sendUserMessage( u"hello"_s, u"/etc/passwd"_s );
  QVERIFY( stateSpy.count() >= 1 );
  const QList<QVariant> args = stateSpy.takeLast();
  QCOMPARE( args.at( 0 ).toString(), u"error"_s );
  QVERIFY( args.at( 1 ).toString().contains( u"blocked"_s, Qt::CaseInsensitive ) );
}

void TestQgsAiAgentSessionManager::findsWorkspaceFilesForMentions()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QVERIFY( QDir( tempDir.path() ).mkpath( u"src/app"_s ) );
  QFile file( tempDir.filePath( u"src/app/main.cpp"_s ) );
  QVERIFY( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
  file.write( "int main() { return 0; }\n" );
  file.close();

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  const QStringList candidates = contextProvider.workspaceFileCandidates( u"main"_s, 5 );
  QVERIFY( candidates.contains( u"src/app/main.cpp"_s ) );
  QCOMPARE( contextProvider.resolveWorkspaceFile( u"src/app/main.cpp"_s ), QDir::cleanPath( file.fileName() ) );
}

void TestQgsAiAgentSessionManager::allowsExplicitExternalAttachmentContext()
{
  QTemporaryDir workspaceDir;
  QVERIFY( workspaceDir.isValid() );
  QTemporaryDir externalDir;
  QVERIFY( externalDir.isValid() );

  QFile externalFile( externalDir.filePath( u"data.txt"_s ) );
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
  manager.sendUserMessage( u"hello"_s, QList<QgsAiChatContextFile>() << contextFile );

  QVERIFY( stateSpy.isEmpty() );
  QVERIFY( messageSpy.count() >= 2 );
  QVERIFY( manager.history().last().content.contains( u"No provider"_s, Qt::CaseInsensitive ) );
}

void TestQgsAiAgentSessionManager::agentBehaviorSettingsRoundTrip()
{
  QgsSettings settings;
  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );

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
    QCOMPARE( defaults.rulesPath, u".strata/rules"_s );

    QgsAiAgentBehaviorSettings updated = defaults;
    updated.allowCustomActions = true;
    updated.rulesText = u"Always answer in English."_s;
    updated.skillsText = u"Prefer GeoPandas."_s;
    updated.rulesPath = u"ai/rules"_s;
    updated.skillsPath = QString();
    manager.setAgentBehaviorSettings( updated );

    const QgsAiAgentBehaviorSettings reread = manager.agentBehaviorSettings();
    QCOMPARE( reread.allowCustomActions, true );
    QCOMPARE( reread.rulesText, u"Always answer in English."_s );
    QCOMPARE( reread.skillsText, u"Prefer GeoPandas."_s );
    QCOMPARE( reread.rulesPath, u"ai/rules"_s );
    // Empty skill path must fall back to the default folder so the file loader stays predictable.
    QCOMPARE( reread.skillsPath, u".strata/skills"_s );
  }

  QgsAiAgentSessionManager reloaded( nullptr, &contextProvider, &reviewEngine );
  const QgsAiAgentBehaviorSettings restored = reloaded.agentBehaviorSettings();
  QCOMPARE( restored.allowCustomActions, true );
  QCOMPARE( restored.rulesText, u"Always answer in English."_s );
  QCOMPARE( restored.skillsText, u"Prefer GeoPandas."_s );

  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );
}

void TestQgsAiAgentSessionManager::agentBehaviorTogglePropagatesToRouter()
{
  QgsSettings settings;
  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<QgsAiEchoTool>() );
  registry.registerTool( std::make_unique<AvailabilityTool>( u"capture_map_canvas"_s, true ) );

  QgsAiModelRouter router;
  router.setToolRegistry( &registry );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( &router, &contextProvider, &reviewEngine );
  manager.setToolRegistry( &registry );

  // Default: tool use stays off until the user opts in.
  QCOMPARE( router.toolUseEnabled(), false );

  QgsAiAgentBehaviorSettings updated = manager.agentBehaviorSettings();
  updated.allowCustomActions = true;
  manager.setAgentBehaviorSettings( updated );
  QCOMPARE( router.toolUseEnabled(), false );

  manager.setActiveAgent( u"editor"_s );
  QCOMPARE( router.toolUseEnabled(), true );

  updated.allowCustomActions = false;
  manager.setAgentBehaviorSettings( updated );
  QCOMPARE( router.toolUseEnabled(), false );

  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );
}

void TestQgsAiAgentSessionManager::planModeDoesNotAdvertiseTools()
{
  QgsSettings settings;
  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<QgsAiEchoTool>() );

  QgsAiModelRouter router;
  router.setToolRegistry( &registry );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( &router, &contextProvider, &reviewEngine );
  manager.setToolRegistry( &registry );

  QgsAiAgentBehaviorSettings updated = manager.agentBehaviorSettings();
  updated.allowCustomActions = true;
  manager.setAgentBehaviorSettings( updated );
  manager.setActiveAgent( u"planner"_s );

  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;
  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenAi, { message }, false ) ).object();
  QVERIFY( !object.contains( u"tools"_s ) );
  QVERIFY( !object.contains( u"tool_choice"_s ) );

  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );
}

void TestQgsAiAgentSessionManager::askAndAgentAdvertiseCaptureMapCanvasTool()
{
  QgsSettings settings;
  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<AvailabilityTool>( u"capture_map_canvas"_s, true ) );
  registry.registerTool( std::make_unique<AvailabilityTool>( u"run_python"_s, true ) );

  QgsAiModelRouter router;
  router.setToolRegistry( &registry );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( &router, &contextProvider, &reviewEngine );
  manager.setToolRegistry( &registry );

  QgsAiAgentBehaviorSettings updated = manager.agentBehaviorSettings();
  updated.allowCustomActions = true;
  manager.setAgentBehaviorSettings( updated );

  manager.setActiveAgent( u"reviewer"_s );
  QVERIFY( router.allowedTools().contains( u"capture_map_canvas"_s ) );
  QVERIFY( !router.allowedTools().contains( u"run_python"_s ) );

  manager.setActiveAgent( u"editor"_s );
  QVERIFY( router.allowedTools().contains( u"capture_map_canvas"_s ) );
  QVERIFY( router.allowedTools().contains( u"run_python"_s ) );

  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );
}

void TestQgsAiAgentSessionManager::agentModeOmitsUnavailableTools()
{
  QgsSettings settings;
  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<AvailabilityTool>( u"run_python"_s, false ) );

  QgsAiModelRouter router;
  router.setToolRegistry( &registry );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( &router, &contextProvider, &reviewEngine );
  manager.setToolRegistry( &registry );

  QgsAiAgentBehaviorSettings updated = manager.agentBehaviorSettings();
  updated.allowCustomActions = true;
  manager.setAgentBehaviorSettings( updated );
  manager.setActiveAgent( u"editor"_s );

  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;
  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenAi, { message }, false ) ).object();
  QVERIFY( !object.contains( u"tools"_s ) );
  QVERIFY( !object.contains( u"tool_choice"_s ) );

  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );
}

void TestQgsAiAgentSessionManager::collectsInlineRulesAndSkills()
{
  QgsSettings settings;
  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  QgsAiAgentBehaviorSettings updated = manager.agentBehaviorSettings();
  updated.rulesText = u"  Be concise.  "_s;
  updated.skillsText = u"Use OSMnx for graphs."_s;
  updated.loadWorkspaceRules = false;
  updated.loadWorkspaceSkills = false;
  manager.setAgentBehaviorSettings( updated );

  QCOMPARE( manager.collectRulesContent(), u"Be concise."_s );
  QCOMPARE( manager.collectSkillsContent(), u"Use OSMnx for graphs."_s );

  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );
}

void TestQgsAiAgentSessionManager::collectsWorkspaceRulesFiles()
{
  QgsSettings settings;
  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QVERIFY( QDir( tempDir.path() ).mkpath( u".strata/rules"_s ) );
  QFile rulesFile( tempDir.filePath( u".strata/rules/coding.md"_s ) );
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
  QVERIFY( rules.contains( u"Always run linters."_s ) );
  QVERIFY( rules.contains( u".strata/rules/coding.md"_s ) );

  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );
}

void TestQgsAiAgentSessionManager::collectsLegacyWorkspaceRulesFiles()
{
  QgsSettings settings;
  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QVERIFY( QDir( tempDir.path() ).mkpath( u".qgis_ai/rules"_s ) );
  QFile rulesFile( tempDir.filePath( u".qgis_ai/rules/legacy.md"_s ) );
  QVERIFY( rulesFile.open( QIODevice::WriteOnly | QIODevice::Text ) );
  rulesFile.write( "- Keep legacy folders readable.\n" );
  rulesFile.close();

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  QgsAiAgentBehaviorSettings updated = manager.agentBehaviorSettings();
  updated.rulesText.clear();
  updated.loadWorkspaceRules = true;
  manager.setAgentBehaviorSettings( updated );

  const QString rules = manager.collectRulesContent();
  QVERIFY( rules.contains( u"Keep legacy folders readable."_s ) );
  QVERIFY( rules.contains( u".qgis_ai/rules/legacy.md"_s ) );

  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );
}

void TestQgsAiAgentSessionManager::collectsGeoAiWorkspaceRulesFiles()
{
  QgsSettings settings;
  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QVERIFY( QDir( tempDir.path() ).mkpath( u".geoai/rules"_s ) );
  QFile rulesFile( tempDir.filePath( u".geoai/rules/legacy.md"_s ) );
  QVERIFY( rulesFile.open( QIODevice::WriteOnly | QIODevice::Text ) );
  rulesFile.write( "- Keep GeoAI folders readable.\n" );
  rulesFile.close();

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  QgsAiAgentBehaviorSettings updated = manager.agentBehaviorSettings();
  updated.rulesText.clear();
  updated.loadWorkspaceRules = true;
  manager.setAgentBehaviorSettings( updated );

  const QString rules = manager.collectRulesContent();
  QVERIFY( rules.contains( u"Keep GeoAI folders readable."_s ) );
  QVERIFY( rules.contains( u".geoai/rules/legacy.md"_s ) );

  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );
}

void TestQgsAiAgentSessionManager::readsLegacyAgentBehaviorSettings()
{
  QgsSettings settings;
  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );

  settings.setValue( u"qgis_ai/agent/allow_custom_actions"_s, true );
  settings.setValue( u"qgis_ai/agent/rules_text"_s, u"Legacy rule"_s );
  settings.setValue( u"qgis_ai/agent/skills_path"_s, u".qgis_ai/skills"_s );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  const QgsAiAgentBehaviorSettings restored = manager.agentBehaviorSettings();
  QCOMPARE( restored.allowCustomActions, true );
  QCOMPARE( restored.rulesText, u"Legacy rule"_s );
  QCOMPARE( restored.skillsPath, u".qgis_ai/skills"_s );

  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );
}

void TestQgsAiAgentSessionManager::readsGeoAiAgentBehaviorSettings()
{
  QgsSettings settings;
  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );

  settings.setValue( u"geoai/agent/allow_custom_actions"_s, true );
  settings.setValue( u"geoai/agent/rules_text"_s, u"GeoAI legacy rule"_s );
  settings.setValue( u"geoai/agent/skills_path"_s, u".geoai/skills"_s );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  const QgsAiAgentBehaviorSettings restored = manager.agentBehaviorSettings();
  QCOMPARE( restored.allowCustomActions, true );
  QCOMPARE( restored.rulesText, u"GeoAI legacy rule"_s );
  QCOMPARE( restored.skillsPath, u".geoai/skills"_s );

  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );
}

void TestQgsAiAgentSessionManager::rejectsRulesFolderOutsideWorkspace()
{
  QgsSettings settings;
  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  QgsAiAgentBehaviorSettings updated = manager.agentBehaviorSettings();
  updated.rulesText.clear();
  updated.loadWorkspaceRules = true;
  // Path that escapes the workspace must be rejected silently rather than reading anything.
  updated.rulesPath = u"../../etc"_s;
  manager.setAgentBehaviorSettings( updated );

  QVERIFY( manager.collectRulesContent().isEmpty() );

  settings.remove( u"strata/agent"_s );
  settings.remove( u"geoai/agent"_s );
  settings.remove( u"qgis_ai/agent"_s );
}

void TestQgsAiAgentSessionManager::projectHistoryScopeChangeClearsActiveSessionAndTranscript()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiChatHistoryStore store( &contextProvider );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
  manager.setHistoryStore( &store );

  const QString projectScope1 = QgsAiAgentSessionManager::chatHistoryScopeKeyForProjectFile( tempDir.filePath( u"one.qgz"_s ) );
  const QString projectScope2 = QgsAiAgentSessionManager::chatHistoryScopeKeyForProjectFile( tempDir.filePath( u"two.qgz"_s ) );

  manager.setProjectChatHistoryScopeKey( projectScope1 );
  QVERIFY( manager.hasPersistentChatHistoryScope() );
  manager.sendUserMessage( u"project one chat"_s );
  QVERIFY( !manager.activeSessionId().isEmpty() );
  QVERIFY( !manager.history().isEmpty() );
  QCOMPARE( manager.listSessions().size(), 1 );

  manager.setProjectChatHistoryScopeKey( projectScope2 );
  QVERIFY( manager.history().isEmpty() );
  QVERIFY( manager.activeSessionId().isEmpty() );
  QCOMPARE( manager.listSessions().size(), 0 );

  manager.setProjectChatHistoryScopeKey( projectScope1 );
  QCOMPARE( manager.listSessions().size(), 1 );
  QCOMPARE( manager.listSessions().first().title, u"project one chat"_s );
}

void TestQgsAiAgentSessionManager::unsavedProjectResetClearsEvenWhenScopeEmpty()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiChatHistoryStore store( &contextProvider );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
  manager.setHistoryStore( &store );

  manager.resetProjectChatHistoryScope();
  QVERIFY( !manager.hasPersistentChatHistoryScope() );
  manager.sendUserMessage( u"memory-only chat"_s );
  QVERIFY( !manager.history().isEmpty() );
  QVERIFY( manager.activeSessionId().isEmpty() );
  QCOMPARE( manager.listSessions().size(), 0 );

  manager.resetProjectChatHistoryScope();
  QVERIFY( manager.history().isEmpty() );
  QVERIFY( manager.activeSessionId().isEmpty() );
  QCOMPARE( manager.listSessions().size(), 0 );
}

void TestQgsAiAgentSessionManager::unsavedProjectFirstSavePromotesCurrentChat()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiChatHistoryStore store( &contextProvider );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
  manager.setHistoryStore( &store );

  manager.resetProjectChatHistoryScope();
  manager.sendUserMessage( u"promote this chat"_s );
  const QList<QgsAiChatMessage> memoryHistory = manager.history();
  QVERIFY( memoryHistory.size() >= 2 );
  QVERIFY( manager.activeSessionId().isEmpty() );

  const QString savedProjectScope = QgsAiAgentSessionManager::chatHistoryScopeKeyForProjectFile( tempDir.filePath( u"saved.qgz"_s ) );
  manager.setProjectChatHistoryScopeKey( savedProjectScope );

  QVERIFY( manager.hasPersistentChatHistoryScope() );
  QCOMPARE( manager.history().size(), memoryHistory.size() );
  QVERIFY( !manager.activeSessionId().isEmpty() );

  const QList<QgsAiChatHistoryStore::SessionInfo> sessions = manager.listSessions();
  QCOMPARE( sessions.size(), 1 );
  QCOMPARE( sessions.first().title, u"promote this chat"_s );

  const QList<QgsAiChatMessage> persistedMessages = store.loadMessages( manager.activeSessionId() );
  QCOMPARE( persistedMessages.size(), memoryHistory.size() );
  QCOMPARE( persistedMessages.first().content, memoryHistory.first().content );
}

void TestQgsAiAgentSessionManager::formatRetrievedContextRendersFileAndLayerHeaders()
{
  QList<QgsAiWorkspaceIndex::Chunk> chunks;

  QgsAiWorkspaceIndex::Chunk fileChunk;
  fileChunk.sourceType = QString::fromLatin1( QgsAiWorkspaceIndex::SOURCE_TYPE_FILE );
  fileChunk.relativePath = u"src/foo.cpp"_s;
  fileChunk.chunkIndex = 2;
  fileChunk.text = u"some file body"_s;
  fileChunk.score = 0.91f;
  chunks << fileChunk;

  QgsAiWorkspaceIndex::Chunk layerChunk;
  layerChunk.sourceType = QString::fromLatin1( QgsAiWorkspaceIndex::SOURCE_TYPE_LAYER );
  layerChunk.relativePath = u"Comuni"_s;
  layerChunk.layerId = u"layer-xyz"_s;
  layerChunk.firstFeatureId = 12;
  layerChunk.lastFeatureId = 50;
  layerChunk.text = u"comune attribute body"_s;
  layerChunk.wktBlob = qCompress( QByteArray( "POINT(1 2)" ) );
  layerChunk.score = 0.83f;
  chunks << layerChunk;

  const QString out = QgsAiAgentSessionManager::formatRetrievedContext( chunks );
  QVERIFY( out.contains( u"== Retrieved context =="_s ) );
  QVERIFY( out.contains( u"[file:src/foo.cpp chunk=2 score=0.910]"_s ) );
  QVERIFY( out.contains( u"some file body"_s ) );
  QVERIFY( out.contains( u"[layer:Comuni id=layer-xyz fid=12-50 score=0.830]"_s ) );
  QVERIFY( out.contains( u"comune attribute body"_s ) );
  QVERIFY( out.contains( u"WKT:"_s ) );
  QVERIFY( out.contains( u"POINT(1 2)"_s ) );
}

void TestQgsAiAgentSessionManager::formatRetrievedContextTruncatesOverBudget()
{
  QList<QgsAiWorkspaceIndex::Chunk> chunks;
  for ( int i = 0; i < 100; ++i )
  {
    QgsAiWorkspaceIndex::Chunk c;
    c.sourceType = QString::fromLatin1( QgsAiWorkspaceIndex::SOURCE_TYPE_FILE );
    c.relativePath = u"f.txt"_s;
    c.chunkIndex = i;
    c.text = QString( 200, 'X'_L1 );
    c.score = 0.5f;
    chunks << c;
  }

  // Cap = ~600 bytes: only the first couple of chunks fit, the rest must be truncated.
  const int cap = 600;
  const QString out = QgsAiAgentSessionManager::formatRetrievedContext( chunks, cap );
  QVERIFY( out.contains( u"…[retrieved context truncated at %1 bytes]"_s.arg( cap ) ) );
  QVERIFY( out.size() < cap + 200 );
}

void TestQgsAiAgentSessionManager::retrievalSkippedWithoutWorkspaceIndex()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  // No workspace index attached: even after a user message arrives, no
  // retrieval helper string is produced (no embedding call is made).
  manager.sendUserMessage( u"hello"_s );
  QCOMPARE( manager.workspaceIndex(), static_cast<QgsAiWorkspaceIndex *>( nullptr ) );

  // Attach an empty index: same outcome — retrieval must short-circuit on
  // status().chunkCount == 0 instead of calling the embedding client.
  QgsAiUnavailableLocalEmbeddingProvider provider;
  QgsAiWorkspaceIndex emptyIndex( &contextProvider, &provider );
  manager.setWorkspaceIndex( &emptyIndex );
  QCOMPARE( emptyIndex.status().chunkCount, 0 );
  // Re-send: no crash, no error. Retrieval must not attempt embeddings when
  // the local embedding provider is unavailable.
  manager.sendUserMessage( u"second"_s );
}

void TestQgsAiAgentSessionManager::preDispatchFailureUnlocksRunningState()
{
  clearProviderSettings();

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiModelRouter router;
  forceProviderPreDispatchFailures( router );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( &router, &contextProvider, &reviewEngine );

  QSignalSpy runningSpy( &manager, &QgsAiAgentSessionManager::requestRunningChanged );
  QSignalSpy messageSpy( &manager, &QgsAiAgentSessionManager::messageAdded );

  manager.sendUserMessage( u"hello"_s );

  QVERIFY( runningSpy.count() >= 1 );
  QCOMPARE( runningSpy.first().at( 0 ).toBool(), true );
  QTRY_VERIFY( !manager.hasActiveRequest() );
  QVERIFY( runningSpy.count() >= 2 );
  QCOMPARE( runningSpy.last().at( 0 ).toBool(), false );
  QVERIFY( messageSpy.count() >= 2 );
  QVERIFY( manager.history().last().content.contains( u"not fully configured"_s, Qt::CaseInsensitive ) );

  clearProviderSettings();
}

void TestQgsAiAgentSessionManager::fallbackPreDispatchFailuresAreDrained()
{
  clearProviderSettings();

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiModelRouter router;
  forceProviderPreDispatchFailures( router );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( &router, &contextProvider, &reviewEngine );

  QSignalSpy runningSpy( &manager, &QgsAiAgentSessionManager::requestRunningChanged );
  QSignalSpy stateSpy( &manager, &QgsAiAgentSessionManager::requestStateChanged );

  manager.sendUserMessage( u"exercise provider fallback"_s );
  QTRY_VERIFY( !manager.hasActiveRequest() );

  int retryingCount = 0;
  bool sawFailed = false;
  for ( const QList<QVariant> &args : stateSpy )
  {
    const QString state = args.at( 0 ).toString();
    if ( state == "retrying"_L1 )
      ++retryingCount;
    if ( state == "failed"_L1 )
      sawFailed = true;
  }

  QCOMPARE( retryingCount, 3 );
  QVERIFY( sawFailed );
  QVERIFY( runningSpy.count() >= 2 );
  QCOMPARE( runningSpy.last().at( 0 ).toBool(), false );

  clearProviderSettings();
}

QGSTEST_MAIN( TestQgsAiAgentSessionManager )
#include "testqgsaiagentsessionmanager.moc"
