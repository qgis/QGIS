/***************************************************************************
  testqgsaiplanclient.cpp
  -----------------------
  begin                : July 2026
***************************************************************************/

#include "ai/qgsaiplanclient.h"
#include "qgsaitestloopbackserver.h"
#include "qgstest.h"

#include <QEventLoop>
#include <QFile>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QString>
#include <QTimer>

using namespace Qt::StringLiterals;

namespace
{
  bool waitForSignal( QObject *sender, const char *signal, int timeoutMs = 10000 )
  {
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot( true );
    QObject::connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit );
    QObject::connect( sender, signal, &loop, SLOT( quit() ) );
    timer.start( timeoutMs );
    loop.exec();
    return timer.isActive();
  }

  QByteArray modelsBody()
  {
    return QByteArrayLiteral(
      R"({"items":[{"id":"managed-plan","label":"Managed Plan","provider":"openrouter","contextWindow":200000,"priceInCredits":{"input":1,"output":3},"capabilities":["tools","vision"],"tierAvailability":["FREE","PRO"]},{"id":"gpt-4o","label":"GPT-4o","provider":"openai","contextWindow":128000,"priceInCredits":{"input":5,"output":15},"capabilities":["tools","vision"],"tierAvailability":["PRO"]}]})"
    );
  }

  QByteArray agentsBody()
  {
    return QByteArrayLiteral(
      R"({"items":[{"mode":"reviewer","label":"Reviewer","allowedTools":["read_file","web_search"],"allowedModels":["managed-plan"]},{"mode":"editor","label":"Editor","allowedTools":["read_file","run_python"],"allowedModels":["managed-plan","gpt-4o"]}]})"
    );
  }

  QByteArray policyBody()
  {
    return QByteArrayLiteral(
      R"({"tier":"PRO","modes":["ask","plan","ask_before_edits","auto_edit"],"allowedTools":["read_file","web_search","run_python"],"allowedModels":["managed-plan","gpt-4o"],"presets":[{"mode":"reviewer","label":"Reviewer","allowedTools":["read_file","web_search"],"allowedModels":["managed-plan"]},{"mode":"editor","label":"Editor","allowedTools":["read_file","run_python"],"allowedModels":["managed-plan","gpt-4o"]}]})"
    );
  }
} //namespace

class TestQgsAiPlanClient : public QObject
{
    Q_OBJECT

  private slots:
    void parsesManagedModelCatalog();
    void parsesAgentPolicy();
    void loginMintsDesktopToken();
    void refreshModelsFetchesAndCaches();
    void refreshAgentsAndPolicyUseBearerToken();
    void setModelPreferenceEncodesModelIdPathSegment();
};

void TestQgsAiPlanClient::parsesManagedModelCatalog()
{
  const QList<QgsAiPlanClient::ModelInfo> models = QgsAiPlanClient::parseModelsJson( modelsBody() );
  QCOMPARE( models.size(), 2 );
  QCOMPARE( models.at( 0 ).id, u"managed-plan"_s );
  QCOMPARE( models.at( 0 ).label, u"Managed Plan"_s );
  QCOMPARE( models.at( 0 ).contextWindow, 200000 );
  QCOMPARE( models.at( 0 ).inputCredits, 1 );
  QCOMPARE( models.at( 0 ).outputCredits, 3 );
  QVERIFY( models.at( 0 ).capabilities.contains( u"vision"_s ) );
  QVERIFY( models.at( 0 ).tierAvailability.contains( u"FREE"_s ) );
  QVERIFY( models.at( 0 ).displayLabel().contains( u"200k ctx"_s ) );
  QVERIFY( models.at( 0 ).tooltip().contains( u"Capabilities"_s ) );

  QCOMPARE( QgsAiPlanClient::apiBaseForChatEndpoint( u"http://127.0.0.1:1234/ai/messages"_s ), u"http://127.0.0.1:1234"_s );
}

void TestQgsAiPlanClient::parsesAgentPolicy()
{
  const QList<QgsAiManagedAgentPreset> agents = QgsAiPlanClient::parseAgentsJson( agentsBody() );
  QCOMPARE( agents.size(), 2 );
  QCOMPARE( agents.first().mode, u"reviewer"_s );
  QVERIFY( agents.first().allowedTools.contains( u"web_search"_s ) );

  const QgsAiManagedAgentPolicy policy = QgsAiPlanClient::parseAgentPolicyJson( policyBody() );
  QCOMPARE( policy.tier, u"PRO"_s );
  QVERIFY( policy.modes.contains( u"ask_before_edits"_s ) );
  QVERIFY( policy.allowedModels.contains( u"gpt-4o"_s ) );
  QCOMPARE( policy.allowedToolsForPreset( u"reviewer"_s ), QStringList( { u"read_file"_s, u"web_search"_s } ) );
  QCOMPARE( policy.allowedModelsForPreset( u"editor"_s ), QStringList( { u"managed-plan"_s, u"gpt-4o"_s } ) );
}

void TestQgsAiPlanClient::loginMintsDesktopToken()
{
  QgsAiTestLoopbackServer server;
  server.responses
    << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", QByteArrayLiteral( "{\"accessToken\":\"access-123\",\"refreshToken\":\"refresh-123\",\"tokenType\":\"Bearer\",\"expiresIn\":900}" ) )
    << QgsAiTestLoopbackServer::jsonResponse( 201, "Created", QByteArrayLiteral( "{\"id\":\"tok_1\",\"token\":\"strata_dt_123\",\"tokenPrefix\":\"strata\",\"name\":\"Strata Desktop\"}" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiPlanClient client;
  QSignalSpy tokenSpy( &client, &QgsAiPlanClient::desktopTokenReady );
  QSignalSpy failedSpy( &client, &QgsAiPlanClient::requestFailed );

  client.login( u"http://127.0.0.1:%1/ai/messages"_s.arg( server.serverPort() ), u"user@example.com"_s, u"supersecret123"_s );
  QVERIFY( waitForSignal( &client, SIGNAL( desktopTokenReady( QString ) ) ) );

  QCOMPARE( failedSpy.count(), 0 );
  QCOMPARE( tokenSpy.count(), 1 );
  QCOMPARE( tokenSpy.at( 0 ).at( 0 ).toString(), u"strata_dt_123"_s );
  QCOMPARE( server.requestCount, 2 );
  QVERIFY( server.rawRequests.at( 0 ).startsWith( "POST /v1/auth/login " ) );
  QVERIFY( server.rawRequests.at( 1 ).toLower().contains( "authorization: bearer access-123" ) );

  const QJsonObject loginBody = QJsonDocument::fromJson( server.requestBodies.at( 0 ) ).object();
  QCOMPARE( loginBody.value( u"email"_s ).toString(), u"user@example.com"_s );
  QCOMPARE( loginBody.value( u"password"_s ).toString(), u"supersecret123"_s );
}

void TestQgsAiPlanClient::refreshModelsFetchesAndCaches()
{
  QFile::remove( QgsAiPlanClient::cacheFilePath() );
  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", modelsBody() );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiPlanClient client;
  QList<QgsAiPlanClient::ModelInfo> received;
  bool fromCache = true;
  connect( &client, &QgsAiPlanClient::modelsReady, this, [&received, &fromCache]( const QList<QgsAiPlanClient::ModelInfo> &models, bool cached ) {
    received = models;
    fromCache = cached;
  } );

  client.refreshModels( u"http://127.0.0.1:%1/ai/messages"_s.arg( server.serverPort() ) );
  QVERIFY( waitForSignal( &client, SIGNAL( modelsReady( QList<QgsAiPlanClient::ModelInfo>, bool ) ) ) );

  QCOMPARE( received.size(), 2 );
  QVERIFY( !fromCache );
  QCOMPARE( server.requestCount, 1 );
  QVERIFY( server.rawRequests.first().startsWith( "GET /v1/models " ) );

  const QList<QgsAiPlanClient::ModelInfo> cached = QgsAiPlanClient::cachedModels();
  QCOMPARE( cached.size(), 2 );
  QCOMPARE( cached.first().id, u"managed-plan"_s );
  QFile::remove( QgsAiPlanClient::cacheFilePath() );
}

void TestQgsAiPlanClient::refreshAgentsAndPolicyUseBearerToken()
{
  QFile::remove( QgsAiPlanClient::agentsCacheFilePath() );
  QFile::remove( QgsAiPlanClient::agentPolicyCacheFilePath() );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", agentsBody() ) << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", policyBody() );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiPlanClient client;
  QSignalSpy agentsSpy( &client, &QgsAiPlanClient::agentsReady );
  QSignalSpy policySpy( &client, &QgsAiPlanClient::agentPolicyReady );
  QSignalSpy failedSpy( &client, &QgsAiPlanClient::requestFailed );

  const QString endpoint = u"http://127.0.0.1:%1/ai/messages"_s.arg( server.serverPort() );
  client.refreshAgents( endpoint, u"strata_dt_123"_s );
  QVERIFY( waitForSignal( &client, SIGNAL( agentsReady( QList<QgsAiManagedAgentPreset>, bool ) ) ) );
  client.refreshAgentPolicy( endpoint, u"strata_dt_123"_s );
  QVERIFY( waitForSignal( &client, SIGNAL( agentPolicyReady( QgsAiManagedAgentPolicy, bool ) ) ) );

  QCOMPARE( failedSpy.count(), 0 );
  QCOMPARE( agentsSpy.count(), 1 );
  QCOMPARE( policySpy.count(), 1 );
  QCOMPARE( server.requestCount, 2 );
  QVERIFY( server.rawRequests.at( 0 ).startsWith( "GET /v1/agents " ) );
  QVERIFY( server.rawRequests.at( 1 ).startsWith( "GET /v1/agents/policy " ) );
  QVERIFY( server.rawRequests.at( 0 ).toLower().contains( "authorization: bearer strata_dt_123" ) );
  QVERIFY( server.rawRequests.at( 1 ).toLower().contains( "authorization: bearer strata_dt_123" ) );

  QCOMPARE( QgsAiPlanClient::cachedAgents().size(), 2 );
  QCOMPARE( QgsAiPlanClient::cachedAgentPolicy().tier, u"PRO"_s );
  QFile::remove( QgsAiPlanClient::agentsCacheFilePath() );
  QFile::remove( QgsAiPlanClient::agentPolicyCacheFilePath() );
}

void TestQgsAiPlanClient::setModelPreferenceEncodesModelIdPathSegment()
{
  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", QByteArrayLiteral( "{\"modelId\":\"deepseek/deepseek-v4-flash\",\"enabled\":false}" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiPlanClient client;
  QSignalSpy updatedSpy( &client, &QgsAiPlanClient::modelPreferenceUpdated );
  QSignalSpy failedSpy( &client, &QgsAiPlanClient::modelPreferenceUpdateFailed );

  const QString endpoint = u"http://127.0.0.1:%1/ai/messages"_s.arg( server.serverPort() );
  client.setModelPreference( endpoint, u"strata_dt_123"_s, u"deepseek/deepseek-v4-flash"_s, false );
  QVERIFY( waitForSignal( &client, SIGNAL( modelPreferenceUpdated( QString, bool ) ) ) );

  QCOMPARE( failedSpy.count(), 0 );
  QCOMPARE( updatedSpy.count(), 1 );
  QCOMPARE( updatedSpy.at( 0 ).at( 0 ).toString(), u"deepseek/deepseek-v4-flash"_s );
  QCOMPARE( updatedSpy.at( 0 ).at( 1 ).toBool(), false );
  QCOMPARE( server.requestCount, 1 );
  QVERIFY2( server.rawRequests.first().startsWith( "PUT /v1/models/preferences/deepseek%2Fdeepseek-v4-flash " ), qPrintable( QString::fromUtf8( server.rawRequests.first() ) ) );
  QVERIFY( server.rawRequests.first().toLower().contains( "authorization: bearer strata_dt_123" ) );

  const QJsonObject body = QJsonDocument::fromJson( server.requestBodies.first() ).object();
  QCOMPARE( body.value( u"enabled"_s ).toBool(), false );
}

QGSTEST_MAIN( TestQgsAiPlanClient )
#include "testqgsaiplanclient.moc"
