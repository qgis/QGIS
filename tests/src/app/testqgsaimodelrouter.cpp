/***************************************************************************
  testqgsaimodelrouter.cpp
  ------------------------
  begin                : April 2026
***************************************************************************/

#include <memory>

#include "ai/qgsaiagentsessionmanager.h"
#include "ai/qgsaiclaudeoauthclient.h"
#include "ai/qgsaicodexoauthclient.h"
#include "ai/qgsaimodelrouter.h"
#include "ai/tools/qgsaiechotool.h"
#include "ai/tools/qgsaitoolregistry.h"
#include "qgsaitestloopbackserver.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QHostAddress>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QString>
#include <QTemporaryDir>
#include <QTimer>
#include <QUrlQuery>

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

  void loadEnvFileIfPresent()
  {
    QDir dir( QDir::currentPath() );
    while ( true )
    {
      const QString candidate = dir.filePath( u".env.test"_s );
      if ( QFile::exists( candidate ) )
      {
        QFile file( candidate );
        if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
          while ( !file.atEnd() )
          {
            const QString line = QString::fromUtf8( file.readLine() ).trimmed();
            if ( line.isEmpty() || line.startsWith( '#' ) || !line.contains( '=' ) )
              continue;
            const int sep = line.indexOf( '=' );
            const QString key = line.left( sep ).trimmed();
            const QString value = line.mid( sep + 1 ).trimmed();
            if ( !key.isEmpty() && !value.isEmpty() && qEnvironmentVariableIsEmpty( key.toUtf8().constData() ) )
              qputenv( key.toUtf8(), value.toUtf8() );
          }
        }
        return;
      }

      if ( !dir.cdUp() )
        return;
    }
  }

  bool waitForReply( QNetworkReply *reply, int timeoutMs = 30000 )
  {
    if ( !reply )
      return false;

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot( true );
    QObject::connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit );
    QObject::connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit );
    timer.start( timeoutMs );
    loop.exec();
    if ( timer.isActive() )
      return true;
    reply->abort();
    return false;
  }

  /**
   * Wipes the persisted OpenRouter provider settings, stores a fake API key and
   * points the router at the loopback server. Restore by removing the
   * `ai/provider/openrouter` group (the returned scope guard's caller does it).
   */
  void configureOpenRouterForLoopback( QgsAiModelRouter &router, quint16 port, int maxRetries = 0 )
  {
    QgsSettings settings;
    settings.setValue( u"ai/provider/openrouter/apiKey"_s, u"sk-or-loopback-test"_s );
    settings.setValue( u"ai/network/maxRetries"_s, maxRetries );

    QgsAiModelRouter::ProviderSettings providerSettings = router.providerSettings( QgsAiModelRouter::Provider::OpenRouter );
    providerSettings.endpoint = u"http://127.0.0.1:%1/api/v1/chat/completions"_s.arg( port );
    providerSettings.model = u"test/model"_s;
    providerSettings.enabled = true;
    providerSettings.autoRouting = true;
    router.setProviderSettings( QgsAiModelRouter::Provider::OpenRouter, providerSettings );
  }

  void removeOpenRouterTestSettings()
  {
    QgsSettings settings;
    settings.remove( u"ai/provider/openrouter"_s );
    settings.remove( u"ai/network/maxRetries"_s );
  }

  QgsAiChatMessage userMessage( const QString &text )
  {
    QgsAiChatMessage message;
    message.role = QgsAiChatRole::User;
    message.content = text;
    return message;
  }

  // Neutralizes env API-key fallbacks, wipes per-provider settings and the active
  // selection so a resolve/availability test starts from a known-empty state; the
  // returned guard restores everything (incl. the active provider) when destroyed.
  [[nodiscard]] auto isolateProviderState()
  {
    const QList<QByteArray> envNames = { "OPENAI_API_KEY", "CLAUDE_API_KEY", "ANTHROPIC_API_KEY", "OPENROUTER_API_KEY" };
    QList<QPair<QByteArray, QByteArray>> savedEnv;
    for ( const QByteArray &name : envNames )
    {
      if ( qEnvironmentVariableIsSet( name.constData() ) )
        savedEnv.append( { name, qgetenv( name.constData() ) } );
      qunsetenv( name.constData() );
    }
    QgsSettings settings;
    const QStringList groups = { u"ai/provider/openai"_s, u"ai/provider/claude"_s, u"ai/provider/openrouter"_s, u"ai/provider/codex"_s, u"ai/provider/plan"_s };
    for ( const QString &group : groups )
      settings.remove( group );
    settings.remove( u"ai/activeProvider"_s );
    return qScopeGuard( [groups, savedEnv]() {
      QgsSettings restoreSettings;
      for ( const QString &group : groups )
        restoreSettings.remove( group );
      restoreSettings.remove( u"ai/activeProvider"_s );
      restoreSettings.remove( u"ai/security/secretsMigrated_v1"_s );
      for ( const QPair<QByteArray, QByteArray> &entry : savedEnv )
        qputenv( entry.first.constData(), entry.second );
    } );
  }
} //namespace

class TestQgsAiModelRouter : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void buildPayloadForOpenAi();
    void buildPayloadForOpenRouterCostOptimized();
    void buildPayloadForOpenRouterToolUseOptimized();
    void buildPayloadForCodexUsesGpt54();
    void codexModelFallback();
    void extractChatGptAccountIdFromIdToken();
    void claudeAuthorizationUrlUsesCurrentPlatformEndpoints();
    void claudeAuthorizationCodeParsing();
    void claudeAuthorizationStateParsing();
    void claudeTokenExchangeIncludesState();
    void sanitizeSecrets();
    void storeApiKeyPersistsInSettings();
    void storeOpenRouterApiKeyPersistsInSettings();
    void openRouterApiKeyFromEnvironment();
    void fallbackOrderContainsOnlyUsableProviders();
    void isProviderAvailableIgnoresEnabledFlag();
    void setActiveProviderPersistsAndResolves();
    void resolveProviderFallsBackWhenActiveUnavailable();
    void selectingProviderDoesNotDisableOthers();
    void toolUseDisabledOmitsToolsFromOpenAiPayload();
    void toolUseEnabledIncludesToolsForOpenAi();
    void toolUseDisabledOmitsToolsFromClaudePayload();
    void toolUseEnabledIncludesToolsForClaude();
    void allowedToolFilterCanAdvertiseNoTools();
    void unavailableToolsAreOmittedFromPayload();
    void visualContextImageIsAddedToOpenAiPayload();
    void visualContextImageIsAddedToClaudePayload();
    void visualContextImageRequiresConsent();
    void visualContextImageIsAddedToCodexPayload();
    void preDispatchFailureIsQueued();
    void dispatchLogDoesNotExposeEndpointQuery();

    // OpenRouter chat/completions migration
    void openRouterPayloadUsesChatCompletionsMessages();
    void openRouterRequireParametersWheneverToolsPresent();
    void openRouterPayloadIncludesModelsFallbackArray();
    void openRouterToolResultRoundTripInChatFormat();
    void openRouterVisualContextUsesImageUrlObject();
    void openRouterVisualContextDeferredAfterParallelToolResults();
    void openRouterCustomResponsesEndpointKeepsLegacyFormat();
    void openRouterDefaultEndpointMigrated();
    void openRouterAutoModelMigratedOncePinnedDefault();
    void openRouterCustomEndpointKeepsAutoModelOnMigration();

    // SSE transport & error hardening (loopback server)
    void openRouterStreamTextDeltas();
    void openRouterStreamToolCallDeltasAccumulateByIndex();
    void openRouterStreamFinishReasonErrorFails();
    void openRouterStreamFinishReasonLengthWithToolCallFails();
    void openRouterStreamWithoutFinishChunkHandlesToolArguments();
    void openRouterNonStreamingToolCalls();
    void keepAliveCommentLinesAreIgnored();
    void malformedSseDataLineIsLoggedAndSkipped();
    void midStreamErrorEventFailsRequest();
    void retryHonorsRetryAfterHeader();
    void http408IsRetriable();
    void http402MapsToCreditsMessage();
    void http403ModerationIncludesReasons();
    void http503MapsToNoProviderMessage();

    // Usage/cost accounting
    void openRouterUsageParsedFromFinalStreamChunk();
    void openRouterUsageParsedFromNonStreamingBody();

    void liveOpenAiRequest();
    void liveClaudeRequest();
    void liveOpenRouterRequest();
};

void TestQgsAiModelRouter::initTestCase()
{
  loadEnvFileIfPresent();
}

void TestQgsAiModelRouter::buildPayloadForOpenAi()
{
  QgsAiModelRouter router;
  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;

  const QByteArray payload = router.buildRequestPayload( QgsAiModelRouter::Provider::OpenAi, { message }, true );
  const QJsonDocument doc = QJsonDocument::fromJson( payload );
  QVERIFY( doc.isObject() );
  const QJsonObject object = doc.object();
  QCOMPARE( object.value( u"stream"_s ).toBool(), true );
  QCOMPARE( object.value( u"model"_s ).toString(), u"gpt-4.1-mini"_s );
  QVERIFY( object.contains( u"input"_s ) );
}

void TestQgsAiModelRouter::buildPayloadForOpenRouterCostOptimized()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( [&settings]() { settings.remove( u"ai/provider/openrouter"_s ); } );

  QgsAiModelRouter router;
  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;

  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenRouter, { message }, true ) ).object();
  QCOMPARE( object.value( u"stream"_s ).toBool(), true );
  // Default model is pinned to a strong tool-calling model, not openrouter/auto.
  QCOMPARE( object.value( u"model"_s ).toString(), u"anthropic/claude-sonnet-4.6"_s );
  // Chat Completions wire format: messages, not the beta Responses input array.
  QVERIFY( object.contains( u"messages"_s ) );
  QVERIFY( !object.contains( u"input"_s ) );
  QVERIFY( object.value( u"max_tokens"_s ).toInt() > 0 );

  const QJsonObject provider = object.value( u"provider"_s ).toObject();
  QCOMPARE( provider.value( u"sort"_s ).toString(), u"price"_s );
  QCOMPARE( provider.value( u"data_collection"_s ).toString(), u"deny"_s );
  QCOMPARE( provider.value( u"allow_fallbacks"_s ).toBool(), true );
  // No tools advertised: require_parameters is not needed.
  QVERIFY( !provider.contains( u"require_parameters"_s ) );
}

void TestQgsAiModelRouter::buildPayloadForOpenRouterToolUseOptimized()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( [&settings]() { settings.remove( u"ai/provider/openrouter"_s ); } );

  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<QgsAiEchoTool>() );

  QgsAiModelRouter router;
  router.setToolRegistry( &registry );
  router.setToolUseEnabled( true );
  router.setOpenRouterRoutingProfile( QgsAiModelRouter::OpenRouterRoutingProfile::ToolUseOptimized );

  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;

  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenRouter, { message }, false ) ).object();
  QVERIFY( object.contains( u"tools"_s ) );
  QCOMPARE( object.value( u"tool_choice"_s ).toString(), u"auto"_s );

  // Chat Completions tools are nested under a function object.
  const QJsonArray tools = object.value( u"tools"_s ).toArray();
  QVERIFY( !tools.isEmpty() );
  const QJsonObject firstTool = tools.at( 0 ).toObject();
  QCOMPARE( firstTool.value( u"type"_s ).toString(), u"function"_s );
  QCOMPARE( firstTool.value( u"function"_s ).toObject().value( u"name"_s ).toString(), u"echo"_s );
  QVERIFY( firstTool.value( u"function"_s ).toObject().contains( u"parameters"_s ) );

  const QJsonObject provider = object.value( u"provider"_s ).toObject();
  QCOMPARE( provider.value( u"require_parameters"_s ).toBool(), true );
  QCOMPARE( provider.value( u"data_collection"_s ).toString(), u"deny"_s );
  QCOMPARE( provider.value( u"allow_fallbacks"_s ).toBool(), true );
  QVERIFY( !provider.contains( u"sort"_s ) );
}

void TestQgsAiModelRouter::buildPayloadForCodexUsesGpt54()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/codex"_s );

  QgsAiModelRouter router;
  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;

  const QByteArray payload = router.buildRequestPayload( QgsAiModelRouter::Provider::Codex, { message }, true );
  const QJsonDocument doc = QJsonDocument::fromJson( payload );
  QVERIFY( doc.isObject() );
  const QJsonObject object = doc.object();
  QCOMPARE( object.value( u"stream"_s ).toBool(), true );
  QCOMPARE( object.value( u"model"_s ).toString(), u"gpt-5.4"_s );
  QVERIFY( object.contains( u"input"_s ) );
}

void TestQgsAiModelRouter::codexModelFallback()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/codex"_s );

  QgsAiModelRouter router;
  QgsAiModelRouter::ProviderSettings codexSettings = router.providerSettings( QgsAiModelRouter::Provider::Codex );
  codexSettings.model = u"gpt-5"_s;
  router.setProviderSettings( QgsAiModelRouter::Provider::Codex, codexSettings );

  QCOMPARE( router.providerSettings( QgsAiModelRouter::Provider::Codex ).model, u"gpt-5.4"_s );

  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;
  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::Codex, { message }, false ) ).object();
  QCOMPARE( object.value( u"model"_s ).toString(), u"gpt-5.4"_s );

  settings.remove( u"ai/provider/codex"_s );
}

void TestQgsAiModelRouter::extractChatGptAccountIdFromIdToken()
{
  const auto encode = []( const QJsonObject &object ) { return QJsonDocument( object ).toJson( QJsonDocument::Compact ).toBase64( QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals ); };

  QJsonObject authClaims;
  authClaims.insert( u"chatgpt_account_id"_s, u"account-test-123"_s );
  QJsonObject payload;
  payload.insert( u"https://api.openai.com/auth"_s, authClaims );

  const QString idToken = QString::fromLatin1( encode( QJsonObject( { { u"alg"_s, u"none"_s } } ) ) ) + '.'_L1 + QString::fromLatin1( encode( payload ) ) + u".signature"_s;

  QCOMPARE( QgsAiCodexOAuthClient::extractChatGptAccountId( idToken ), u"account-test-123"_s );
}

void TestQgsAiModelRouter::claudeAuthorizationUrlUsesCurrentPlatformEndpoints()
{
  const QgsAiClaudeOAuthClient::AuthorizationRequest request = QgsAiClaudeOAuthClient::buildAuthorizationRequest();
  QCOMPARE( request.redirectUri, u"https://platform.claude.com/oauth/code/callback"_s );
  QCOMPARE( request.authorizationUrl.scheme(), u"https"_s );
  QCOMPARE( request.authorizationUrl.host(), u"platform.claude.com"_s );
  QCOMPARE( request.authorizationUrl.path(), u"/oauth/authorize"_s );

  const QUrlQuery query( request.authorizationUrl );
  QCOMPARE( query.queryItemValue( u"client_id"_s ), u"9d1c250a-e61b-44d9-88ed-5944d1962f5e"_s );
  QCOMPARE( query.queryItemValue( u"redirect_uri"_s ), request.redirectUri );
  QCOMPARE( query.queryItemValue( u"code_challenge_method"_s ), u"S256"_s );
  const QString scope = query.queryItemValue( u"scope"_s );
  QVERIFY( scope.contains( u"user:inference"_s ) );
  QVERIFY( scope.contains( u"user:sessions:claude_code"_s ) );
  QVERIFY( scope.contains( u"user:mcp_servers"_s ) );
  QVERIFY( scope.contains( u"user:file_upload"_s ) );
}

void TestQgsAiModelRouter::claudeAuthorizationCodeParsing()
{
  QCOMPARE( QgsAiClaudeOAuthClient::authorizationCodeFromInput( u"manual-code"_s ), u"manual-code"_s );
  QCOMPARE( QgsAiClaudeOAuthClient::authorizationCodeFromInput( u"manual-code#returned-state"_s ), u"manual-code"_s );
  QCOMPARE( QgsAiClaudeOAuthClient::authorizationCodeFromInput( u"https://platform.claude.com/oauth/code/callback?code=query-code&state=returned-state"_s ), u"query-code"_s );
  QCOMPARE( QgsAiClaudeOAuthClient::authorizationCodeFromInput( u"https://platform.claude.com/oauth/code/callback#code=fragment-code&state=returned-state"_s ), u"fragment-code"_s );
}

void TestQgsAiModelRouter::claudeAuthorizationStateParsing()
{
  QCOMPARE( QgsAiClaudeOAuthClient::authorizationStateFromInput( u"manual-code#returned-state"_s ), u"returned-state"_s );
  QCOMPARE( QgsAiClaudeOAuthClient::authorizationStateFromInput( u"https://platform.claude.com/oauth/code/callback?code=query-code&state=returned-state"_s ), u"returned-state"_s );
  QCOMPARE( QgsAiClaudeOAuthClient::authorizationStateFromInput( u"https://platform.claude.com/oauth/code/callback#code=fragment-code&state=returned-state"_s ), u"returned-state"_s );
  QVERIFY( QgsAiClaudeOAuthClient::authorizationStateFromInput( u"manual-code"_s ).isEmpty() );
}

void TestQgsAiModelRouter::claudeTokenExchangeIncludesState()
{
  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", QByteArrayLiteral( "{\"access_token\":\"sk-ant-oat01-test\",\"refresh_token\":\"sk-ant-ort01-test\",\"expires_in\":3600}" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  const QString loopbackUrl = u"http://127.0.0.1:%1/token"_s.arg( server.serverPort() );
  QgsAiClaudeOAuthClient::setTokenUrlForTesting( loopbackUrl );
  const auto clearTokenUrl = qScopeGuard( [] { QgsAiClaudeOAuthClient::clearTokenUrlForTesting(); } );

  const QgsAiClaudeOAuthClient::AuthorizationRequest authRequest = QgsAiClaudeOAuthClient::buildAuthorizationRequest();
  const QString callbackInput = u"https://platform.claude.com/oauth/code/callback?code=exchange-code&state=callback-state"_s;

  QString error;
  const bool exchanged = QgsAiClaudeOAuthClient::exchangeAuthorizationCode( callbackInput, authRequest.codeVerifier, authRequest.redirectUri, authRequest.state, &error );

  QCOMPARE( server.requestCount, 1 );
  const QJsonObject requestObject = QJsonDocument::fromJson( server.lastRequestBody() ).object();
  QCOMPARE( requestObject.value( u"grant_type"_s ).toString(), u"authorization_code"_s );
  QCOMPARE( requestObject.value( u"code"_s ).toString(), u"exchange-code"_s );
  QCOMPARE( requestObject.value( u"state"_s ).toString(), u"callback-state"_s );
  QCOMPARE( requestObject.value( u"code_verifier"_s ).toString(), authRequest.codeVerifier );
  QCOMPARE( requestObject.value( u"redirect_uri"_s ).toString(), authRequest.redirectUri );
  QVERIFY( server.lastRawRequest().toLower().contains( "accept: application/json" ) );

  QgsAuthManager *authManager = QgsApplication::authManager();
  if ( authManager && !authManager->isDisabled() )
  {
    QVERIFY2( exchanged, qPrintable( error ) );
    QVERIFY( QgsAiClaudeOAuthClient::hasRefreshToken() );
    QgsAiClaudeOAuthClient::clearRefreshToken();
  }
}

void TestQgsAiModelRouter::sanitizeSecrets()
{
  QgsAiModelRouter router;
  const QString raw = u"Authorization: Bearer sk-verysecrettoken and x-api-key: abc123 and sk-or-openroutersecret"_s;
  const QString sanitized = router.sanitizeErrorText( raw );
  QVERIFY( !sanitized.contains( u"verysecrettoken"_s ) );
  QVERIFY( !sanitized.contains( u"abc123"_s ) );
  QVERIFY( !sanitized.contains( u"openroutersecret"_s ) );
  QVERIFY( sanitized.contains( u"REDACTED"_s ) );
}

void TestQgsAiModelRouter::storeApiKeyPersistsInSettings()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openai"_s );

  QgsAiModelRouter router;
  QString error;
  QVERIFY2( router.storeApiKey( QgsAiModelRouter::Provider::OpenAi, u"sk-test-local-storage"_s, &error ), qPrintable( error ) );

  QgsAiModelRouter reloadedRouter;
  QNetworkRequest request( QUrl( u"https://api.openai.com/v1/responses"_s ) );
  QVERIFY2( reloadedRouter.applyAuthentication( QgsAiModelRouter::Provider::OpenAi, request, &error ), qPrintable( error ) );
  QCOMPARE( request.rawHeader( "Authorization" ), QByteArray( "Bearer sk-test-local-storage" ) );
  QVERIFY( reloadedRouter.providerSettings( QgsAiModelRouter::Provider::OpenAi ).enabled );

  settings.remove( u"ai/provider/openai"_s );
}

void TestQgsAiModelRouter::storeOpenRouterApiKeyPersistsInSettings()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );

  QgsAiModelRouter router;
  QString error;
  QVERIFY2( router.storeApiKey( QgsAiModelRouter::Provider::OpenRouter, u"sk-or-test-local-storage"_s, &error ), qPrintable( error ) );

  QgsAiModelRouter reloadedRouter;
  QNetworkRequest request( QUrl( u"https://openrouter.ai/api/v1/responses"_s ) );
  QVERIFY2( reloadedRouter.applyAuthentication( QgsAiModelRouter::Provider::OpenRouter, request, &error ), qPrintable( error ) );
  QCOMPARE( request.rawHeader( "Authorization" ), QByteArray( "Bearer sk-or-test-local-storage" ) );
  QVERIFY( reloadedRouter.providerSettings( QgsAiModelRouter::Provider::OpenRouter ).enabled );

  settings.remove( u"ai/provider/openrouter"_s );
}

void TestQgsAiModelRouter::openRouterApiKeyFromEnvironment()
{
  QgsSettings settings;
  const QString apiKeyKey = u"ai/provider/openrouter/apiKey"_s;
  const bool hadApiKeySetting = settings.contains( apiKeyKey );
  const QVariant savedApiKey = settings.value( apiKeyKey );
  const bool hadEnv = qEnvironmentVariableIsSet( "OPENROUTER_API_KEY" );
  const QByteArray savedEnv = qgetenv( "OPENROUTER_API_KEY" );
  const auto restore = qScopeGuard( [&settings, apiKeyKey, hadApiKeySetting, savedApiKey, hadEnv, savedEnv]() {
    if ( hadApiKeySetting )
      settings.setValue( apiKeyKey, savedApiKey );
    else
      settings.remove( apiKeyKey );

    qunsetenv( "OPENROUTER_API_KEY" );
    if ( hadEnv )
      qputenv( "OPENROUTER_API_KEY", savedEnv );
  } );

  settings.remove( apiKeyKey );
  qputenv( "OPENROUTER_API_KEY", "sk-or-test-env" );

  QgsAiModelRouter router;
  QString error;
  QNetworkRequest request( QUrl( u"https://openrouter.ai/api/v1/responses"_s ) );
  QVERIFY2( router.applyAuthentication( QgsAiModelRouter::Provider::OpenRouter, request, &error ), qPrintable( error ) );
  QCOMPARE( request.rawHeader( "Authorization" ), QByteArray( "Bearer sk-or-test-env" ) );
}

void TestQgsAiModelRouter::fallbackOrderContainsOnlyUsableProviders()
{
  // initTestCase loads .env.test: neutralize the env fallbacks so this test
  // controls exactly which providers count as configured.
  const QList<QByteArray> envNames = { "OPENAI_API_KEY", "CLAUDE_API_KEY", "ANTHROPIC_API_KEY", "OPENROUTER_API_KEY" };
  QList<QPair<QByteArray, QByteArray>> savedEnv;
  for ( const QByteArray &name : envNames )
  {
    if ( qEnvironmentVariableIsSet( name.constData() ) )
      savedEnv.append( { name, qgetenv( name.constData() ) } );
    qunsetenv( name.constData() );
  }
  QgsSettings settings;
  const QStringList groups = { u"ai/provider/openai"_s, u"ai/provider/claude"_s, u"ai/provider/openrouter"_s, u"ai/provider/codex"_s, u"ai/provider/plan"_s };
  for ( const QString &group : groups )
    settings.remove( group );
  settings.remove( u"ai/activeProvider"_s );
  const auto restore = qScopeGuard( [&settings, groups, savedEnv]() {
    for ( const QString &group : groups )
      settings.remove( group );
    settings.remove( u"ai/activeProvider"_s );
    settings.remove( u"ai/security/secretsMigrated_v1"_s );
    for ( const QPair<QByteArray, QByteArray> &entry : savedEnv )
      qputenv( entry.first.constData(), entry.second );
  } );

  // Nothing configured: the chain is EMPTY (no blind attempts).
  {
    QgsAiModelRouter router;
    QgsAiAgentSessionManager manager( &router, nullptr, nullptr );
    QVERIFY( manager.providerFallbackOrder().isEmpty() );
    QVERIFY( !router.isProviderUsable( QgsAiModelRouter::Provider::OpenAi ) );
  }

  // Configure OpenRouter + Claude only: the chain contains exactly those two,
  // preferred (OpenRouter, higher priority) first, no duplicates.
  {
    QgsAiModelRouter router;
    QVERIFY( router.storeApiKey( QgsAiModelRouter::Provider::OpenRouter, u"sk-or-fallback-test"_s ) );
    QVERIFY( router.storeApiKey( QgsAiModelRouter::Provider::Claude, u"sk-ant-fallback-test"_s ) );

    QVERIFY( router.isProviderUsable( QgsAiModelRouter::Provider::OpenRouter ) );
    QVERIFY( router.isProviderUsable( QgsAiModelRouter::Provider::Claude ) );
    QVERIFY( !router.isProviderUsable( QgsAiModelRouter::Provider::OpenAi ) );
    QVERIFY( !router.isProviderUsable( QgsAiModelRouter::Provider::Codex ) );
    QVERIFY( !router.isProviderUsable( QgsAiModelRouter::Provider::Plan ) );

    QgsAiAgentSessionManager manager( &router, nullptr, nullptr );
    const QList<QgsAiModelRouter::Provider> order = manager.providerFallbackOrder();
    QCOMPARE( order.size(), 2 );
    QCOMPARE( order.at( 0 ), QgsAiModelRouter::Provider::OpenRouter );
    QCOMPARE( order.at( 1 ), QgsAiModelRouter::Provider::Claude );
  }
}

void TestQgsAiModelRouter::isProviderAvailableIgnoresEnabledFlag()
{
  const auto guard = isolateProviderState();

  QgsAiModelRouter router;
  QVERIFY( router.storeApiKey( QgsAiModelRouter::Provider::OpenAi, u"sk-available-test"_s ) );

  // A synced provider can be explicitly disabled (i.e. not the active choice) yet
  // must still count as "available" so the picker keeps offering its models.
  QgsAiModelRouter::ProviderSettings s = router.providerSettings( QgsAiModelRouter::Provider::OpenAi );
  s.enabled = false;
  router.setProviderSettings( QgsAiModelRouter::Provider::OpenAi, s );

  QVERIFY( router.isProviderAvailable( QgsAiModelRouter::Provider::OpenAi ) );
  QVERIFY( !router.isProviderUsable( QgsAiModelRouter::Provider::OpenAi ) );
  // OpenRouter has no credential at all: not available, not usable.
  QVERIFY( !router.isProviderAvailable( QgsAiModelRouter::Provider::OpenRouter ) );
}

void TestQgsAiModelRouter::setActiveProviderPersistsAndResolves()
{
  const auto guard = isolateProviderState();

  QgsAiModelRouter router;
  QVERIFY( router.storeApiKey( QgsAiModelRouter::Provider::OpenRouter, u"sk-or-active-test"_s ) );
  QVERIFY( router.storeApiKey( QgsAiModelRouter::Provider::Claude, u"sk-ant-active-test"_s ) );

  // Claude is lower priority than OpenRouter in the fallback chain, but an explicit
  // active selection must win.
  router.setActiveProvider( QgsAiModelRouter::Provider::Claude );
  QCOMPARE( router.resolveProvider(), QgsAiModelRouter::Provider::Claude );

  // The choice survives a reload (a fresh router instance reads ai/activeProvider).
  QgsAiModelRouter reloaded;
  QCOMPARE( reloaded.resolveProvider(), QgsAiModelRouter::Provider::Claude );
}

void TestQgsAiModelRouter::resolveProviderFallsBackWhenActiveUnavailable()
{
  const auto guard = isolateProviderState();

  QgsAiModelRouter router;
  QVERIFY( router.storeApiKey( QgsAiModelRouter::Provider::OpenRouter, u"sk-or-fallback-active-test"_s ) );

  // Codex has no OAuth token, so it is not usable; resolveProvider must fall back
  // to the synced provider instead of stranding on the stale active choice.
  router.setActiveProvider( QgsAiModelRouter::Provider::Codex );
  QVERIFY( !router.isProviderUsable( QgsAiModelRouter::Provider::Codex ) );
  QCOMPARE( router.resolveProvider(), QgsAiModelRouter::Provider::OpenRouter );
}

void TestQgsAiModelRouter::selectingProviderDoesNotDisableOthers()
{
  const auto guard = isolateProviderState();

  QgsAiModelRouter router;
  QVERIFY( router.storeApiKey( QgsAiModelRouter::Provider::OpenRouter, u"sk-or-switch-test"_s ) );
  QVERIFY( router.storeApiKey( QgsAiModelRouter::Provider::Claude, u"sk-ant-switch-test"_s ) );

  // Mimic the dock's onModelSelected: set the chosen model + mark it active, WITHOUT
  // touching the other providers.
  QgsAiModelRouter::ProviderSettings s = router.providerSettings( QgsAiModelRouter::Provider::OpenRouter );
  s.model = u"openrouter/auto"_s;
  s.enabled = true;
  router.setProviderSettings( QgsAiModelRouter::Provider::OpenRouter, s );
  router.setActiveProvider( QgsAiModelRouter::Provider::OpenRouter );

  QCOMPARE( router.resolveProvider(), QgsAiModelRouter::Provider::OpenRouter );
  // Claude was not disabled, so it remains synced and ready for an instant switch back.
  QVERIFY( router.isProviderUsable( QgsAiModelRouter::Provider::Claude ) );
  QVERIFY( router.isProviderAvailable( QgsAiModelRouter::Provider::Claude ) );
}

void TestQgsAiModelRouter::toolUseDisabledOmitsToolsFromOpenAiPayload()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<QgsAiEchoTool>() );

  QgsAiModelRouter router;
  router.setToolRegistry( &registry );
  router.setToolUseEnabled( false );

  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;

  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenAi, { message }, false ) ).object();
  QVERIFY( !object.contains( u"tools"_s ) );
  QVERIFY( !object.contains( u"tool_choice"_s ) );
}

void TestQgsAiModelRouter::toolUseEnabledIncludesToolsForOpenAi()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<QgsAiEchoTool>() );

  QgsAiModelRouter router;
  router.setToolRegistry( &registry );
  router.setToolUseEnabled( true );

  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;

  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenAi, { message }, false ) ).object();
  QVERIFY( object.contains( u"tools"_s ) );
  QCOMPARE( object.value( u"tool_choice"_s ).toString(), u"auto"_s );
  const QJsonArray tools = object.value( u"tools"_s ).toArray();
  QVERIFY( !tools.isEmpty() );
  QCOMPARE( tools.at( 0 ).toObject().value( u"name"_s ).toString(), u"echo"_s );
}

void TestQgsAiModelRouter::toolUseDisabledOmitsToolsFromClaudePayload()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<QgsAiEchoTool>() );

  QgsAiModelRouter router;
  router.setToolRegistry( &registry );
  router.setToolUseEnabled( false );

  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;

  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::Claude, { message }, false ) ).object();
  QVERIFY( !object.contains( u"tools"_s ) );
}

void TestQgsAiModelRouter::toolUseEnabledIncludesToolsForClaude()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<QgsAiEchoTool>() );

  QgsAiModelRouter router;
  router.setToolRegistry( &registry );
  router.setToolUseEnabled( true );

  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;

  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::Claude, { message }, false ) ).object();
  QVERIFY( object.contains( u"tools"_s ) );
  const QJsonArray tools = object.value( u"tools"_s ).toArray();
  QVERIFY( !tools.isEmpty() );
  QCOMPARE( tools.at( 0 ).toObject().value( u"name"_s ).toString(), u"echo"_s );
}

void TestQgsAiModelRouter::allowedToolFilterCanAdvertiseNoTools()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<QgsAiEchoTool>() );

  QgsAiModelRouter router;
  router.setToolRegistry( &registry );
  router.setToolUseEnabled( true );
  router.setAllowedTools( QStringList() );

  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;

  const QJsonObject openAiObject = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenAi, { message }, false ) ).object();
  QVERIFY( !openAiObject.contains( u"tools"_s ) );
  QVERIFY( !openAiObject.contains( u"tool_choice"_s ) );

  const QJsonObject codexObject = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::Codex, { message }, false ) ).object();
  QVERIFY( !codexObject.contains( u"tools"_s ) );
  QVERIFY( !codexObject.contains( u"tool_choice"_s ) );
}

void TestQgsAiModelRouter::unavailableToolsAreOmittedFromPayload()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<AvailabilityTool>( u"run_python"_s, false ) );

  QgsAiModelRouter router;
  router.setToolRegistry( &registry );
  router.setToolUseEnabled( true );

  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;

  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenAi, { message }, false ) ).object();
  QVERIFY( !object.contains( u"tools"_s ) );
  QVERIFY( !object.contains( u"tool_choice"_s ) );
}

void TestQgsAiModelRouter::visualContextImageIsAddedToOpenAiPayload()
{
  QgsSettings settings;
  settings.setValue( u"strata/visual_context/image_send_consent"_s, true );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  const QString imagePath = tempDir.filePath( u"canvas.png"_s );
  QImage image( 2, 2, QImage::Format_ARGB32 );
  image.fill( Qt::red );
  QVERIFY( image.save( imagePath, "PNG" ) );

  QgsAiChatMessage toolMessage;
  toolMessage.role = QgsAiChatRole::Tool;
  toolMessage.content = u"{\"image\":{\"path\":\"canvas.png\"}}"_s;
  toolMessage.metadata.insert( u"tool_call_id"_s, u"call_1"_s );
  toolMessage.metadata.insert( u"visual_context_image_path"_s, imagePath );
  toolMessage.metadata.insert( u"visual_context_mime_type"_s, u"image/png"_s );

  QgsAiModelRouter router;
  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenAi, { toolMessage }, false ) ).object();
  const QJsonArray input = object.value( u"input"_s ).toArray();
  QCOMPARE( input.size(), 2 );
  QCOMPARE( input.at( 0 ).toObject().value( u"type"_s ).toString(), u"function_call_output"_s );

  const QJsonArray content = input.at( 1 ).toObject().value( u"content"_s ).toArray();
  bool hasImage = false;
  for ( const QJsonValue &value : content )
  {
    const QJsonObject block = value.toObject();
    if ( block.value( u"type"_s ).toString() == "input_image"_L1 )
    {
      hasImage = true;
      QVERIFY( block.value( u"image_url"_s ).toString().startsWith( "data:image/png;base64,"_L1 ) );
    }
  }
  QVERIFY( hasImage );

  settings.remove( u"strata/visual_context/image_send_consent"_s );
  settings.remove( u"geoai/visual_context/image_send_consent"_s );
}

void TestQgsAiModelRouter::visualContextImageIsAddedToClaudePayload()
{
  QgsSettings settings;
  settings.setValue( u"strata/visual_context/image_send_consent"_s, true );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  const QString imagePath = tempDir.filePath( u"canvas.png"_s );
  QImage image( 2, 2, QImage::Format_ARGB32 );
  image.fill( Qt::blue );
  QVERIFY( image.save( imagePath, "PNG" ) );

  QgsAiChatMessage toolMessage;
  toolMessage.role = QgsAiChatRole::Tool;
  toolMessage.content = u"{\"image\":{\"path\":\"canvas.png\"}}"_s;
  toolMessage.metadata.insert( u"tool_call_id"_s, u"toolu_1"_s );
  toolMessage.metadata.insert( u"visual_context_image_path"_s, imagePath );
  toolMessage.metadata.insert( u"visual_context_mime_type"_s, u"image/png"_s );

  QgsAiModelRouter router;
  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::Claude, { toolMessage }, false ) ).object();
  const QJsonArray messages = object.value( u"messages"_s ).toArray();
  QCOMPARE( messages.size(), 1 );
  const QJsonArray content = messages.at( 0 ).toObject().value( u"content"_s ).toArray();
  QCOMPARE( content.size(), 1 );
  const QJsonObject toolResult = content.at( 0 ).toObject();
  QCOMPARE( toolResult.value( u"type"_s ).toString(), u"tool_result"_s );
  const QJsonArray resultContent = toolResult.value( u"content"_s ).toArray();

  bool hasImage = false;
  for ( const QJsonValue &value : resultContent )
  {
    const QJsonObject block = value.toObject();
    if ( block.value( u"type"_s ).toString() == "image"_L1 )
    {
      hasImage = true;
      const QJsonObject source = block.value( u"source"_s ).toObject();
      QCOMPARE( source.value( u"media_type"_s ).toString(), u"image/png"_s );
      QVERIFY( !source.value( u"data"_s ).toString().isEmpty() );
    }
  }
  QVERIFY( hasImage );

  settings.remove( u"strata/visual_context/image_send_consent"_s );
  settings.remove( u"geoai/visual_context/image_send_consent"_s );
}

void TestQgsAiModelRouter::visualContextImageRequiresConsent()
{
  QgsSettings settings;
  settings.remove( u"strata/visual_context/image_send_consent"_s );
  settings.remove( u"geoai/visual_context/image_send_consent"_s );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  const QString imagePath = tempDir.filePath( u"canvas.png"_s );
  QImage image( 2, 2, QImage::Format_ARGB32 );
  image.fill( Qt::green );
  QVERIFY( image.save( imagePath, "PNG" ) );

  QgsAiChatMessage toolMessage;
  toolMessage.role = QgsAiChatRole::Tool;
  toolMessage.content = u"{\"image\":{\"path\":\"canvas.png\"}}"_s;
  toolMessage.metadata.insert( u"tool_call_id"_s, u"call_1"_s );
  toolMessage.metadata.insert( u"visual_context_image_path"_s, imagePath );
  toolMessage.metadata.insert( u"visual_context_mime_type"_s, u"image/png"_s );

  QgsAiModelRouter router;
  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenAi, { toolMessage }, false ) ).object();
  const QString payloadText = QString::fromUtf8( QJsonDocument( object ).toJson( QJsonDocument::Compact ) );
  QVERIFY( !payloadText.contains( u"input_image"_s ) );
  QVERIFY( !payloadText.contains( u"data:image/png;base64"_s ) );
}

void TestQgsAiModelRouter::visualContextImageIsAddedToCodexPayload()
{
  QgsSettings settings;
  settings.setValue( u"strata/visual_context/image_send_consent"_s, true );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  const QString imagePath = tempDir.filePath( u"canvas.png"_s );
  QImage image( 2, 2, QImage::Format_ARGB32 );
  image.fill( Qt::green );
  QVERIFY( image.save( imagePath, "PNG" ) );

  QgsAiChatMessage toolMessage;
  toolMessage.role = QgsAiChatRole::Tool;
  toolMessage.content = u"{\"image\":{\"path\":\"canvas.png\"}}"_s;
  toolMessage.metadata.insert( u"tool_call_id"_s, u"call_1"_s );
  toolMessage.metadata.insert( u"visual_context_image_path"_s, imagePath );
  toolMessage.metadata.insert( u"visual_context_mime_type"_s, u"image/png"_s );

  QgsAiModelRouter router;
  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::Codex, { toolMessage }, false ) ).object();
  const QJsonArray input = object.value( u"input"_s ).toArray();
  QCOMPARE( input.size(), 2 );
  QCOMPARE( input.at( 0 ).toObject().value( u"type"_s ).toString(), u"function_call_output"_s );

  const QJsonArray content = input.at( 1 ).toObject().value( u"content"_s ).toArray();
  bool hasImage = false;
  for ( const QJsonValue &value : content )
  {
    const QJsonObject block = value.toObject();
    if ( block.value( u"type"_s ).toString() == "input_image"_L1 )
    {
      hasImage = true;
      QVERIFY( block.value( u"image_url"_s ).toString().startsWith( "data:image/png;base64,"_L1 ) );
    }
  }
  QVERIFY( hasImage );

  settings.remove( u"strata/visual_context/image_send_consent"_s );
  settings.remove( u"geoai/visual_context/image_send_consent"_s );
}

void TestQgsAiModelRouter::preDispatchFailureIsQueued()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/plan"_s );

  QgsAiModelRouter router;
  QgsAiModelRouter::ProviderSettings planSettings = router.providerSettings( QgsAiModelRouter::Provider::Plan );
  planSettings.endpoint = u"https://example.invalid/ai/messages"_s;
  planSettings.enabled = true;
  router.setProviderSettings( QgsAiModelRouter::Provider::Plan, planSettings );

  QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );

  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;
  const QString requestId = router.startChatRequest( QgsAiModelRouter::Provider::Plan, { message }, false );

  QVERIFY( !requestId.isEmpty() );
  QCOMPARE( finishedSpy.count(), 0 );
  QVERIFY( router.hasActiveRequest( requestId ) );

  QTRY_COMPARE( finishedSpy.count(), 1 );
  QVERIFY( !router.hasActiveRequest( requestId ) );

  const QList<QVariant> args = finishedSpy.takeFirst();
  QCOMPARE( args.at( 0 ).toString(), requestId );
  QCOMPARE( args.at( 1 ).toBool(), false );
  QCOMPARE( args.at( 2 ).toString(), u"Plan Account"_s );
  QVERIFY( args.at( 4 ).toString().contains( u"endpoint is not configured"_s, Qt::CaseInsensitive ) );

  settings.remove( u"ai/provider/plan"_s );
}

void TestQgsAiModelRouter::dispatchLogDoesNotExposeEndpointQuery()
{
  QgsSettings settings;
  const QString apiKeyKey = u"ai/provider/openai/apiKey"_s;
  const QString endpointKey = u"ai/provider/openai/endpoint"_s;
  const QString modelKey = u"ai/provider/openai/model"_s;
  const QString enabledKey = u"ai/provider/openai/enabled"_s;
  const QVariant savedApiKey = settings.value( apiKeyKey );
  const QVariant savedEndpoint = settings.value( endpointKey );
  const QVariant savedModel = settings.value( modelKey );
  const QVariant savedEnabled = settings.value( enabledKey );

  auto restoreSetting = [&settings]( const QString &key, const QVariant &value ) {
    if ( value.isValid() )
      settings.setValue( key, value );
    else
      settings.remove( key );
  };
  const auto restoreSettings = qScopeGuard( [&]() {
    restoreSetting( apiKeyKey, savedApiKey );
    restoreSetting( endpointKey, savedEndpoint );
    restoreSetting( modelKey, savedModel );
    restoreSetting( enabledKey, savedEnabled );
  } );

  QgsAiModelRouter router;
  router.storeApiKey( QgsAiModelRouter::Provider::OpenAi, u"test-key"_s );
  QgsAiModelRouter::ProviderSettings providerSettings = router.providerSettings( QgsAiModelRouter::Provider::OpenAi );
  providerSettings.endpoint = u"http://127.0.0.1:9/v1/responses?token=secret-query&path=/Users/francesco/private"_s;
  providerSettings.model = u"gpt-4.1-mini"_s;
  providerSettings.enabled = true;
  router.setProviderSettings( QgsAiModelRouter::Provider::OpenAi, providerSettings );

  QSignalSpy logSpy( QgsApplication::messageLog(), &QgsMessageLog::messageReceivedWithFormat );

  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;
  const QString requestId = router.startChatRequest( QgsAiModelRouter::Provider::OpenAi, { message }, false );
  router.cancelRequest( requestId );

  bool foundDispatchLog = false;
  for ( const QList<QVariant> &args : logSpy )
  {
    const QString logMessage = args.at( 0 ).toString();
    if ( !logMessage.contains( u"Dispatching request"_s ) )
      continue;
    foundDispatchLog = true;
    QVERIFY( logMessage.contains( u"endpointHost=127.0.0.1"_s ) );
    QVERIFY( !logMessage.contains( u"secret-query"_s ) );
    QVERIFY( !logMessage.contains( u"/Users/francesco/private"_s ) );
  }
  QVERIFY( foundDispatchLog );
}

void TestQgsAiModelRouter::openRouterPayloadUsesChatCompletionsMessages()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiModelRouter router;

  QgsAiChatMessage systemMessage;
  systemMessage.role = QgsAiChatRole::System;
  systemMessage.content = u"you are a GIS assistant"_s;

  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenRouter, { systemMessage, userMessage( u"hello"_s ) }, true ) ).object();
  QVERIFY( !object.contains( u"input"_s ) );

  const QJsonArray messages = object.value( u"messages"_s ).toArray();
  QCOMPARE( messages.size(), 2 );
  QCOMPARE( messages.at( 0 ).toObject().value( u"role"_s ).toString(), u"system"_s );
  QCOMPARE( messages.at( 0 ).toObject().value( u"content"_s ).toString(), u"you are a GIS assistant"_s );
  QCOMPARE( messages.at( 1 ).toObject().value( u"role"_s ).toString(), u"user"_s );
  QCOMPARE( messages.at( 1 ).toObject().value( u"content"_s ).toString(), u"hello"_s );
}

void TestQgsAiModelRouter::openRouterRequireParametersWheneverToolsPresent()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<QgsAiEchoTool>() );

  QgsAiModelRouter router;
  router.setToolRegistry( &registry );
  router.setToolUseEnabled( true );
  // Cost-optimized profile: tools still force require_parameters so OpenRouter
  // never routes to a provider that drops the tools parameter.
  router.setOpenRouterRoutingProfile( QgsAiModelRouter::OpenRouterRoutingProfile::CostOptimized );

  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, false ) ).object();
  const QJsonObject provider = object.value( u"provider"_s ).toObject();
  QCOMPARE( provider.value( u"require_parameters"_s ).toBool(), true );
  QCOMPARE( provider.value( u"sort"_s ).toString(), u"price"_s );
}

void TestQgsAiModelRouter::openRouterPayloadIncludesModelsFallbackArray()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiModelRouter router;

  // Pinned model + autoRouting: models fallback array ending in openrouter/auto.
  const QJsonObject pinned = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, false ) ).object();
  const QJsonArray fallbackModels = pinned.value( u"models"_s ).toArray();
  QCOMPARE( fallbackModels.size(), 2 );
  QCOMPARE( fallbackModels.at( 0 ).toString(), u"anthropic/claude-sonnet-4.6"_s );
  QCOMPARE( fallbackModels.at( 1 ).toString(), u"openrouter/auto"_s );

  // Explicit openrouter/auto: no fallback array needed.
  QgsAiModelRouter::ProviderSettings autoSettings = router.providerSettings( QgsAiModelRouter::Provider::OpenRouter );
  autoSettings.model = u"openrouter/auto"_s;
  router.setProviderSettings( QgsAiModelRouter::Provider::OpenRouter, autoSettings );
  const QJsonObject autoRouted = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, false ) ).object();
  QVERIFY( !autoRouted.contains( u"models"_s ) );

  // Auto-routing disabled: no provider preferences and no fallback array.
  QgsAiModelRouter::ProviderSettings manualSettings = router.providerSettings( QgsAiModelRouter::Provider::OpenRouter );
  manualSettings.model = u"anthropic/claude-sonnet-4.6"_s;
  manualSettings.autoRouting = false;
  router.setProviderSettings( QgsAiModelRouter::Provider::OpenRouter, manualSettings );
  const QJsonObject manual = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, false ) ).object();
  QVERIFY( !manual.contains( u"models"_s ) );
  QVERIFY( !manual.contains( u"provider"_s ) );
}

void TestQgsAiModelRouter::openRouterToolResultRoundTripInChatFormat()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiModelRouter router;

  QgsAiChatMessage assistantMessage;
  assistantMessage.role = QgsAiChatRole::Assistant;
  assistantMessage.content = QString();
  QVariantMap call;
  call.insert( u"id"_s, u"call_1"_s );
  call.insert( u"name"_s, u"echo"_s );
  QVariantMap args;
  args.insert( u"text"_s, u"hi"_s );
  call.insert( u"args"_s, args );
  assistantMessage.metadata.insert( u"tool_calls"_s, QVariantList() << call );

  QgsAiChatMessage toolMessage;
  toolMessage.role = QgsAiChatRole::Tool;
  toolMessage.content = u"{\"echoed\":\"hi\"}"_s;
  toolMessage.metadata.insert( u"tool_call_id"_s, u"call_1"_s );

  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"echo hi"_s ), assistantMessage, toolMessage }, false ) ).object();
  const QJsonArray messages = object.value( u"messages"_s ).toArray();
  QCOMPARE( messages.size(), 3 );

  const QJsonObject assistant = messages.at( 1 ).toObject();
  QCOMPARE( assistant.value( u"role"_s ).toString(), u"assistant"_s );
  // Tool-call-only turns carry null content.
  QVERIFY( assistant.value( u"content"_s ).isNull() );
  const QJsonArray toolCalls = assistant.value( u"tool_calls"_s ).toArray();
  QCOMPARE( toolCalls.size(), 1 );
  const QJsonObject toolCall = toolCalls.at( 0 ).toObject();
  QCOMPARE( toolCall.value( u"id"_s ).toString(), u"call_1"_s );
  QCOMPARE( toolCall.value( u"type"_s ).toString(), u"function"_s );
  const QJsonObject function = toolCall.value( u"function"_s ).toObject();
  QCOMPARE( function.value( u"name"_s ).toString(), u"echo"_s );
  const QJsonObject parsedArgs = QJsonDocument::fromJson( function.value( u"arguments"_s ).toString().toUtf8() ).object();
  QCOMPARE( parsedArgs.value( u"text"_s ).toString(), u"hi"_s );

  const QJsonObject toolResult = messages.at( 2 ).toObject();
  QCOMPARE( toolResult.value( u"role"_s ).toString(), u"tool"_s );
  QCOMPARE( toolResult.value( u"tool_call_id"_s ).toString(), u"call_1"_s );
  QCOMPARE( toolResult.value( u"content"_s ).toString(), u"{\"echoed\":\"hi\"}"_s );
}

void TestQgsAiModelRouter::openRouterVisualContextUsesImageUrlObject()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  settings.setValue( u"strata/visual_context/image_send_consent"_s, true );
  const auto cleanup = qScopeGuard( [&settings]() {
    removeOpenRouterTestSettings();
    settings.remove( u"strata/visual_context/image_send_consent"_s );
    settings.remove( u"geoai/visual_context/image_send_consent"_s );
  } );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  const QString imagePath = tempDir.filePath( u"canvas.png"_s );
  QImage image( 2, 2, QImage::Format_ARGB32 );
  image.fill( Qt::red );
  QVERIFY( image.save( imagePath, "PNG" ) );

  QgsAiChatMessage toolMessage;
  toolMessage.role = QgsAiChatRole::Tool;
  toolMessage.content = u"{\"image\":{\"path\":\"canvas.png\"}}"_s;
  toolMessage.metadata.insert( u"tool_call_id"_s, u"call_1"_s );
  toolMessage.metadata.insert( u"visual_context_image_path"_s, imagePath );
  toolMessage.metadata.insert( u"visual_context_mime_type"_s, u"image/png"_s );

  QgsAiModelRouter router;
  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenRouter, { toolMessage }, false ) ).object();
  const QJsonArray messages = object.value( u"messages"_s ).toArray();
  QCOMPARE( messages.size(), 2 );
  QCOMPARE( messages.at( 0 ).toObject().value( u"role"_s ).toString(), u"tool"_s );

  // Chat Completions image blocks use the nested image_url object form.
  const QJsonArray content = messages.at( 1 ).toObject().value( u"content"_s ).toArray();
  bool hasImage = false;
  for ( const QJsonValue &value : content )
  {
    const QJsonObject block = value.toObject();
    if ( block.value( u"type"_s ).toString() == "image_url"_L1 )
    {
      hasImage = true;
      QVERIFY( block.value( u"image_url"_s ).toObject().value( u"url"_s ).toString().startsWith( "data:image/png;base64,"_L1 ) );
    }
  }
  QVERIFY( hasImage );
}

void TestQgsAiModelRouter::openRouterVisualContextDeferredAfterParallelToolResults()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  settings.setValue( u"strata/visual_context/image_send_consent"_s, true );
  const auto cleanup = qScopeGuard( [&settings]() {
    removeOpenRouterTestSettings();
    settings.remove( u"strata/visual_context/image_send_consent"_s );
    settings.remove( u"geoai/visual_context/image_send_consent"_s );
  } );

  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  const QString imagePath = tempDir.filePath( u"canvas.png"_s );
  QImage image( 2, 2, QImage::Format_ARGB32 );
  image.fill( Qt::red );
  QVERIFY( image.save( imagePath, "PNG" ) );

  // Parallel tool-call turn: the visual-context image belongs to the FIRST call,
  // but every tool message must stay adjacent to the assistant tool_calls turn.
  QgsAiChatMessage assistantMessage;
  assistantMessage.role = QgsAiChatRole::Assistant;
  QVariantMap callA;
  callA.insert( u"id"_s, u"call_a"_s );
  callA.insert( u"name"_s, u"capture_map_canvas"_s );
  callA.insert( u"args"_s, QVariantMap() );
  QVariantMap callB;
  callB.insert( u"id"_s, u"call_b"_s );
  callB.insert( u"name"_s, u"echo"_s );
  callB.insert( u"args"_s, QVariantMap() );
  assistantMessage.metadata.insert( u"tool_calls"_s, QVariantList() << callA << callB );

  QgsAiChatMessage toolMessageA;
  toolMessageA.role = QgsAiChatRole::Tool;
  toolMessageA.content = u"{\"image\":{\"path\":\"canvas.png\"}}"_s;
  toolMessageA.metadata.insert( u"tool_call_id"_s, u"call_a"_s );
  toolMessageA.metadata.insert( u"visual_context_image_path"_s, imagePath );
  toolMessageA.metadata.insert( u"visual_context_mime_type"_s, u"image/png"_s );

  QgsAiChatMessage toolMessageB;
  toolMessageB.role = QgsAiChatRole::Tool;
  toolMessageB.content = u"{\"echoed\":true}"_s;
  toolMessageB.metadata.insert( u"tool_call_id"_s, u"call_b"_s );

  QgsAiModelRouter router;
  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenRouter, { assistantMessage, toolMessageA, toolMessageB }, false ) ).object();
  const QJsonArray messages = object.value( u"messages"_s ).toArray();
  QCOMPARE( messages.size(), 4 );
  QCOMPARE( messages.at( 0 ).toObject().value( u"role"_s ).toString(), u"assistant"_s );
  QCOMPARE( messages.at( 1 ).toObject().value( u"role"_s ).toString(), u"tool"_s );
  // The second tool message must directly follow the first — the image user
  // message is deferred to after the tool block.
  QCOMPARE( messages.at( 2 ).toObject().value( u"role"_s ).toString(), u"tool"_s );
  QCOMPARE( messages.at( 2 ).toObject().value( u"tool_call_id"_s ).toString(), u"call_b"_s );
  QCOMPARE( messages.at( 3 ).toObject().value( u"role"_s ).toString(), u"user"_s );
  const QString lastMessageText = QString::fromUtf8( QJsonDocument( messages.at( 3 ).toObject() ).toJson( QJsonDocument::Compact ) );
  QVERIFY( lastMessageText.contains( u"image_url"_s ) );
}

void TestQgsAiModelRouter::openRouterCustomResponsesEndpointKeepsLegacyFormat()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiModelRouter router;
  QgsAiModelRouter::ProviderSettings providerSettings = router.providerSettings( QgsAiModelRouter::Provider::OpenRouter );
  providerSettings.endpoint = u"https://my-gateway.example.com/api/v1/responses"_s;
  router.setProviderSettings( QgsAiModelRouter::Provider::OpenRouter, providerSettings );

  // Escape hatch: custom /responses endpoints keep the legacy Responses wire format.
  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, true ) ).object();
  QVERIFY( object.contains( u"input"_s ) );
  QVERIFY( !object.contains( u"messages"_s ) );
}

void TestQgsAiModelRouter::openRouterDefaultEndpointMigrated()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  // Simulate a pre-migration profile pointing at the beta Responses endpoint.
  settings.setValue( u"ai/provider/openrouter/endpoint"_s, u"https://openrouter.ai/api/v1/responses"_s );

  QgsAiModelRouter router;
  QCOMPARE( router.providerSettings( QgsAiModelRouter::Provider::OpenRouter ).endpoint, u"https://openrouter.ai/api/v1/chat/completions"_s );
  QVERIFY( settings.value( u"ai/provider/openrouter/defaultModelMigrated_v1"_s ).toBool() );
  QCOMPARE( settings.value( u"ai/provider/openrouter/endpoint"_s ).toString(), u"https://openrouter.ai/api/v1/chat/completions"_s );
}

void TestQgsAiModelRouter::openRouterAutoModelMigratedOncePinnedDefault()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  // Pre-migration profile on the unpinned auto router model.
  settings.setValue( u"ai/provider/openrouter/model"_s, u"openrouter/auto"_s );

  QgsAiModelRouter router;
  QCOMPARE( router.providerSettings( QgsAiModelRouter::Provider::OpenRouter ).model, u"anthropic/claude-sonnet-4.6"_s );
  QVERIFY( settings.value( u"ai/provider/openrouter/defaultModelMigrated_v1"_s ).toBool() );

  // An explicit re-selection of openrouter/auto after the migration is respected.
  QgsAiModelRouter::ProviderSettings providerSettings = router.providerSettings( QgsAiModelRouter::Provider::OpenRouter );
  providerSettings.model = u"openrouter/auto"_s;
  router.setProviderSettings( QgsAiModelRouter::Provider::OpenRouter, providerSettings );

  QgsAiModelRouter reloadedRouter;
  QCOMPARE( reloadedRouter.providerSettings( QgsAiModelRouter::Provider::OpenRouter ).model, u"openrouter/auto"_s );
}

void TestQgsAiModelRouter::openRouterCustomEndpointKeepsAutoModelOnMigration()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  // On a custom gateway, an explicit openrouter/auto choice may be deliberate:
  // the one-time model migration must not touch it.
  settings.setValue( u"ai/provider/openrouter/endpoint"_s, u"https://my-gateway.example.com/api/v1/chat/completions"_s );
  settings.setValue( u"ai/provider/openrouter/model"_s, u"openrouter/auto"_s );

  QgsAiModelRouter router;
  QCOMPARE( router.providerSettings( QgsAiModelRouter::Provider::OpenRouter ).model, u"openrouter/auto"_s );
  QVERIFY( settings.value( u"ai/provider/openrouter/defaultModelMigrated_v1"_s ).toBool() );
}

void TestQgsAiModelRouter::openRouterStreamTextDeltas()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::sseResponse( {
    QByteArrayLiteral( "data: {\"choices\":[{\"index\":0,\"delta\":{\"role\":\"assistant\",\"content\":\"Hel\"}}]}\n\n" ),
    QByteArrayLiteral( "data: {\"choices\":[{\"index\":0,\"delta\":{\"content\":\"lo\"}}]}\n\n" ),
    QByteArrayLiteral( "data: {\"choices\":[{\"index\":0,\"delta\":{},\"finish_reason\":\"stop\"}]}\n\ndata: [DONE]\n\n" ),
  } );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort() );

  QSignalSpy progressSpy( &router, &QgsAiModelRouter::requestProgress );
  QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, true );

  QTRY_COMPARE_WITH_TIMEOUT( finishedSpy.count(), 1, 10000 );
  const QList<QVariant> args = finishedSpy.takeFirst();
  QVERIFY2( args.at( 1 ).toBool(), qPrintable( args.at( 4 ).toString() ) );
  QCOMPARE( args.at( 3 ).toString(), u"Hello"_s );
  QVERIFY( progressSpy.count() >= 2 );

  // The dispatched request used the Chat Completions wire format and attribution
  // headers (case-insensitive: Qt normalizes raw header casing, e.g. Http-Referer).
  QVERIFY( server.lastRequestBody().contains( "\"messages\"" ) );
  QVERIFY( server.lastRawRequest().toLower().contains( "x-title: strata" ) );
  QVERIFY( server.lastRawRequest().toLower().contains( "http-referer:" ) );
}

void TestQgsAiModelRouter::openRouterStreamToolCallDeltasAccumulateByIndex()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::sseResponse( {
    QByteArrayLiteral( "data: {\"choices\":[{\"delta\":{\"tool_calls\":[{\"index\":0,\"id\":\"call_a\",\"type\":\"function\",\"function\":{\"name\":\"echo\",\"arguments\":\"\"}}]}}]}\n\n" ),
    QByteArrayLiteral( "data: {\"choices\":[{\"delta\":{\"tool_calls\":[{\"index\":1,\"id\":\"call_b\",\"type\":\"function\",\"function\":{\"name\":\"list_files\",\"arguments\":\"{\\\"glob\"}}]}}]}\n\n" ),
    QByteArrayLiteral( "data: {\"choices\":[{\"delta\":{\"tool_calls\":[{\"index\":0,\"function\":{\"arguments\":\"{\\\"text\\\":\\\"hi\\\"}\"}}]}}]}\n\n" ),
    QByteArrayLiteral( "data: {\"choices\":[{\"delta\":{\"tool_calls\":[{\"index\":1,\"function\":{\"arguments\":\"\\\":\\\"*.txt\\\"}\"}}]}}]}\n\n" ),
    QByteArrayLiteral( "data: {\"choices\":[{\"delta\":{},\"finish_reason\":\"tool_calls\"}]}\n\ndata: [DONE]\n\n" ),
  } );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort() );

  QSignalSpy progressSpy( &router, &QgsAiModelRouter::requestProgress );
  QSignalSpy toolCallsSpy( &router, &QgsAiModelRouter::toolCallsRequested );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"run the tools"_s ) }, true );

  QTRY_COMPARE_WITH_TIMEOUT( toolCallsSpy.count(), 1, 10000 );
  const QList<QVariant> args = toolCallsSpy.takeFirst();
  const QList<QgsAiToolCall> calls = args.at( 3 ).value<QList<QgsAiToolCall>>();
  QCOMPARE( calls.size(), 2 );
  QCOMPARE( calls.at( 0 ).id, u"call_a"_s );
  QCOMPARE( calls.at( 0 ).name, u"echo"_s );
  QCOMPARE( calls.at( 0 ).args.value( u"text"_s ).toString(), u"hi"_s );
  QCOMPARE( calls.at( 1 ).id, u"call_b"_s );
  QCOMPARE( calls.at( 1 ).name, u"list_files"_s );
  QCOMPARE( calls.at( 1 ).args.value( u"glob"_s ).toString(), u"*.txt"_s );

  // Tool-call argument fragments must never leak into the chat text stream.
  for ( const QList<QVariant> &progressArgs : progressSpy )
    QVERIFY( !progressArgs.at( 1 ).toString().contains( u"glob"_s ) );
}

void TestQgsAiModelRouter::openRouterStreamFinishReasonErrorFails()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::sseResponse( {
    QByteArrayLiteral( "data: {\"choices\":[{\"index\":0,\"delta\":{\"content\":\"partial\"}}]}\n\n" ),
    QByteArrayLiteral( "data: {\"error\":{\"code\":502,\"message\":\"provider crashed\"},\"choices\":[{\"delta\":{},\"finish_reason\":\"error\"}]}\n\ndata: [DONE]\n\n" ),
  } );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort() );

  QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, true );

  QTRY_COMPARE_WITH_TIMEOUT( finishedSpy.count(), 1, 10000 );
  const QList<QVariant> args = finishedSpy.takeFirst();
  QCOMPARE( args.at( 1 ).toBool(), false );
  QVERIFY2( args.at( 4 ).toString().contains( u"provider crashed"_s ), qPrintable( args.at( 4 ).toString() ) );
}

void TestQgsAiModelRouter::openRouterStreamFinishReasonLengthWithToolCallFails()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  // max_tokens hit mid tool-call: the arguments JSON is truncated and the turn
  // must fail instead of executing the tool with empty args.
  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::sseResponse( {
    QByteArrayLiteral(
      "data: {\"choices\":[{\"delta\":{\"tool_calls\":[{\"index\":0,\"id\":\"call_a\",\"type\":\"function\",\"function\":{\"name\":\"run_python\",\"arguments\":\"{\\\"code\\\":\\\"print(\"}}]}}]}\n\n"
    ),
    QByteArrayLiteral( "data: {\"choices\":[{\"delta\":{},\"finish_reason\":\"length\"}]}\n\ndata: [DONE]\n\n" ),
  } );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort() );

  QSignalSpy toolCallsSpy( &router, &QgsAiModelRouter::toolCallsRequested );
  QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, true );

  QTRY_COMPARE_WITH_TIMEOUT( finishedSpy.count(), 1, 10000 );
  QCOMPARE( toolCallsSpy.count(), 0 );
  const QList<QVariant> args = finishedSpy.takeFirst();
  QCOMPARE( args.at( 1 ).toBool(), false );
  QVERIFY2( args.at( 4 ).toString().contains( u"max_tokens"_s ), qPrintable( args.at( 4 ).toString() ) );
}

void TestQgsAiModelRouter::openRouterStreamWithoutFinishChunkHandlesToolArguments()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  // Stream cut before the finish chunk, with TRUNCATED arguments: the safety net
  // must abort the turn instead of dispatching the tool with empty args.
  {
    QgsAiTestLoopbackServer server;
    server.responses << QgsAiTestLoopbackServer::sseResponse( {
      QByteArrayLiteral( "data: {\"choices\":[{\"delta\":{\"tool_calls\":[{\"index\":0,\"id\":\"call_a\",\"type\":\"function\",\"function\":{\"name\":\"echo\",\"arguments\":\"{\\\"text\"}}]}}]}\n\n" ),
    } );
    QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

    QgsAiModelRouter router;
    configureOpenRouterForLoopback( router, server.serverPort() );

    QSignalSpy toolCallsSpy( &router, &QgsAiModelRouter::toolCallsRequested );
    QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );
    router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, true );

    QTRY_COMPARE_WITH_TIMEOUT( finishedSpy.count(), 1, 10000 );
    QCOMPARE( toolCallsSpy.count(), 0 );
    const QList<QVariant> args = finishedSpy.takeFirst();
    QCOMPARE( args.at( 1 ).toBool(), false );
    QVERIFY2( args.at( 4 ).toString().contains( u"incomplete tool-call arguments"_s ), qPrintable( args.at( 4 ).toString() ) );
  }

  // Stream cut before the finish chunk, but with COMPLETE arguments: the safety
  // net parses them and the tool call is still dispatched.
  {
    QgsAiTestLoopbackServer server;
    server.responses << QgsAiTestLoopbackServer::sseResponse( {
      QByteArrayLiteral(
        "data: {\"choices\":[{\"delta\":{\"tool_calls\":[{\"index\":0,\"id\":\"call_a\",\"type\":\"function\",\"function\":{\"name\":\"echo\",\"arguments\":\"{\\\"text\\\":\\\"hi\\\"}\"}}]}}]}\n\n"
      ),
    } );
    QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

    QgsAiModelRouter router;
    configureOpenRouterForLoopback( router, server.serverPort() );

    QSignalSpy toolCallsSpy( &router, &QgsAiModelRouter::toolCallsRequested );
    router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, true );

    QTRY_COMPARE_WITH_TIMEOUT( toolCallsSpy.count(), 1, 10000 );
    const QList<QgsAiToolCall> calls = toolCallsSpy.takeFirst().at( 3 ).value<QList<QgsAiToolCall>>();
    QCOMPARE( calls.size(), 1 );
    QCOMPARE( calls.at( 0 ).args.value( u"text"_s ).toString(), u"hi"_s );
  }
}

void TestQgsAiModelRouter::openRouterNonStreamingToolCalls()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::
      jsonResponse( 200, "OK", QByteArrayLiteral( "{\"choices\":[{\"message\":{\"role\":\"assistant\",\"content\":null,\"tool_calls\":[{\"id\":\"call_1\",\"type\":\"function\",\"function\":{\"name\":\"echo\",\"arguments\":\"{\\\"text\\\":\\\"hi\\\"}\"}}]},\"finish_reason\":\"tool_calls\"}]}" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort() );

  QSignalSpy toolCallsSpy( &router, &QgsAiModelRouter::toolCallsRequested );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"echo hi"_s ) }, false );

  QTRY_COMPARE_WITH_TIMEOUT( toolCallsSpy.count(), 1, 10000 );
  const QList<QgsAiToolCall> calls = toolCallsSpy.takeFirst().at( 3 ).value<QList<QgsAiToolCall>>();
  QCOMPARE( calls.size(), 1 );
  QCOMPARE( calls.at( 0 ).id, u"call_1"_s );
  QCOMPARE( calls.at( 0 ).name, u"echo"_s );
  QCOMPARE( calls.at( 0 ).args.value( u"text"_s ).toString(), u"hi"_s );
}

void TestQgsAiModelRouter::keepAliveCommentLinesAreIgnored()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::sseResponse( {
    QByteArrayLiteral( ": OPENROUTER PROCESSING\n\n" ),
    QByteArrayLiteral( "data: {\"choices\":[{\"index\":0,\"delta\":{\"content\":\"OK\"}}]}\n\n" ),
    QByteArrayLiteral( ": OPENROUTER PROCESSING\n\n" ),
    QByteArrayLiteral( "data: {\"choices\":[{\"index\":0,\"delta\":{},\"finish_reason\":\"stop\"}]}\n\ndata: [DONE]\n\n" ),
  } );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort() );

  QSignalSpy logSpy( QgsApplication::messageLog(), &QgsMessageLog::messageReceivedWithFormat );
  QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, true );

  QTRY_COMPARE_WITH_TIMEOUT( finishedSpy.count(), 1, 10000 );
  const QList<QVariant> args = finishedSpy.takeFirst();
  QVERIFY2( args.at( 1 ).toBool(), qPrintable( args.at( 4 ).toString() ) );
  QCOMPARE( args.at( 3 ).toString(), u"OK"_s );

  // Keep-alive comments are not data frames: nothing must be logged as malformed.
  for ( const QList<QVariant> &logArgs : logSpy )
    QVERIFY( !logArgs.at( 0 ).toString().contains( u"malformed SSE"_s, Qt::CaseInsensitive ) );
}

void TestQgsAiModelRouter::malformedSseDataLineIsLoggedAndSkipped()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::sseResponse( {
    QByteArrayLiteral( "data: {\"choices\":[{\"index\":0,\"delta\":{\"content\":\"Va\"}}]}\n\n" ),
    QByteArrayLiteral( "data: {corrupted json\n\n" ),
    QByteArrayLiteral( "data: {\"choices\":[{\"index\":0,\"delta\":{\"content\":\"le\"}}]}\n\n" ),
    QByteArrayLiteral( "data: {\"choices\":[{\"index\":0,\"delta\":{},\"finish_reason\":\"stop\"}]}\n\ndata: [DONE]\n\n" ),
  } );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort() );

  QSignalSpy logSpy( QgsApplication::messageLog(), &QgsMessageLog::messageReceivedWithFormat );
  QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, true );

  QTRY_COMPARE_WITH_TIMEOUT( finishedSpy.count(), 1, 10000 );
  const QList<QVariant> args = finishedSpy.takeFirst();
  QVERIFY2( args.at( 1 ).toBool(), qPrintable( args.at( 4 ).toString() ) );
  QCOMPARE( args.at( 3 ).toString(), u"Vale"_s );

  int malformedLogs = 0;
  for ( const QList<QVariant> &logArgs : logSpy )
  {
    if ( logArgs.at( 0 ).toString().contains( u"malformed SSE"_s, Qt::CaseInsensitive ) )
      ++malformedLogs;
  }
  QCOMPARE( malformedLogs, 1 );
}

void TestQgsAiModelRouter::midStreamErrorEventFailsRequest()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::sseResponse( {
    QByteArrayLiteral( "data: {\"error\":{\"code\":429,\"message\":\"upstream rate limited mid-stream\"}}\n\ndata: [DONE]\n\n" ),
  } );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort() );

  QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, true );

  QTRY_COMPARE_WITH_TIMEOUT( finishedSpy.count(), 1, 10000 );
  const QList<QVariant> args = finishedSpy.takeFirst();
  QCOMPARE( args.at( 1 ).toBool(), false );
  QVERIFY2( args.at( 4 ).toString().contains( u"upstream rate limited mid-stream"_s ), qPrintable( args.at( 4 ).toString() ) );
}

void TestQgsAiModelRouter::retryHonorsRetryAfterHeader()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiTestLoopbackServer server;
  server.responses
    << QgsAiTestLoopbackServer::
         jsonResponse( 429, "Too Many Requests", QByteArrayLiteral( "{\"error\":{\"code\":429,\"message\":\"slow down\"}}" ), { { QByteArrayLiteral( "Retry-After" ), QByteArrayLiteral( "1" ) } } )
    << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", QByteArrayLiteral( "{\"choices\":[{\"message\":{\"role\":\"assistant\",\"content\":\"OK\"},\"finish_reason\":\"stop\"}]}" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort(), 2 );

  QElapsedTimer elapsed;
  elapsed.start();
  QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, false );

  QTRY_COMPARE_WITH_TIMEOUT( finishedSpy.count(), 1, 15000 );
  const QList<QVariant> args = finishedSpy.takeFirst();
  QVERIFY2( args.at( 1 ).toBool(), qPrintable( args.at( 4 ).toString() ) );
  QCOMPARE( args.at( 3 ).toString(), u"OK"_s );
  QCOMPARE( args.at( 6 ).toInt(), 1 ); // one retry happened
  QCOMPARE( server.requestCount, 2 );
  QVERIFY2( elapsed.elapsed() >= 1000, qPrintable( u"elapsed=%1ms"_s.arg( elapsed.elapsed() ) ) );
}

void TestQgsAiModelRouter::http408IsRetriable()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiTestLoopbackServer server;
  server.responses
    << QgsAiTestLoopbackServer::jsonResponse( 408, "Request Timeout", QByteArrayLiteral( "{\"error\":{\"code\":408,\"message\":\"timed out\"}}" ) )
    << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", QByteArrayLiteral( "{\"choices\":[{\"message\":{\"role\":\"assistant\",\"content\":\"OK\"},\"finish_reason\":\"stop\"}]}" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort(), 2 );

  QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, false );

  QTRY_COMPARE_WITH_TIMEOUT( finishedSpy.count(), 1, 15000 );
  const QList<QVariant> args = finishedSpy.takeFirst();
  QVERIFY2( args.at( 1 ).toBool(), qPrintable( args.at( 4 ).toString() ) );
  QCOMPARE( server.requestCount, 2 );
}

void TestQgsAiModelRouter::http402MapsToCreditsMessage()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::jsonResponse( 402, "Payment Required", QByteArrayLiteral( "{\"error\":{\"code\":402,\"message\":\"Insufficient credits\"}}" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort() );

  QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, false );

  QTRY_COMPARE_WITH_TIMEOUT( finishedSpy.count(), 1, 10000 );
  const QList<QVariant> args = finishedSpy.takeFirst();
  QCOMPARE( args.at( 1 ).toBool(), false );
  QVERIFY2( args.at( 4 ).toString().contains( u"openrouter.ai/credits"_s ), qPrintable( args.at( 4 ).toString() ) );
}

void TestQgsAiModelRouter::http403ModerationIncludesReasons()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::
      jsonResponse( 403, "Forbidden", QByteArrayLiteral( "{\"error\":{\"code\":403,\"message\":\"Forbidden\",\"metadata\":{\"reasons\":[\"unsafe content\"],\"flagged_input\":\"redacted text\",\"provider_name\":\"X\",\"model_slug\":\"y\"}}}" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort() );

  QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, false );

  QTRY_COMPARE_WITH_TIMEOUT( finishedSpy.count(), 1, 10000 );
  const QList<QVariant> args = finishedSpy.takeFirst();
  QCOMPARE( args.at( 1 ).toBool(), false );
  QVERIFY2( args.at( 4 ).toString().contains( u"unsafe content"_s ), qPrintable( args.at( 4 ).toString() ) );
  QVERIFY( args.at( 4 ).toString().contains( u"redacted text"_s ) );
}

void TestQgsAiModelRouter::http503MapsToNoProviderMessage()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::jsonResponse( 503, "Service Unavailable", QByteArrayLiteral( "{\"error\":{\"code\":503,\"message\":\"No provider available\"}}" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort() );

  QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, false );

  QTRY_COMPARE_WITH_TIMEOUT( finishedSpy.count(), 1, 10000 );
  const QList<QVariant> args = finishedSpy.takeFirst();
  QCOMPARE( args.at( 1 ).toBool(), false );
  QVERIFY2( args.at( 4 ).toString().contains( u"routing requirements"_s ), qPrintable( args.at( 4 ).toString() ) );
}

void TestQgsAiModelRouter::openRouterUsageParsedFromFinalStreamChunk()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::sseResponse( {
    QByteArrayLiteral( "data: {\"model\":\"anthropic/claude-sonnet-4.6\",\"choices\":[{\"index\":0,\"delta\":{\"content\":\"OK\"}}]}\n\n" ),
    QByteArrayLiteral( "data: {\"choices\":[{\"index\":0,\"delta\":{},\"finish_reason\":\"stop\"}]}\n\n" ),
    QByteArrayLiteral(
      "data: "
      "{\"choices\":[],\"usage\":{\"prompt_tokens\":12,\"completion_tokens\":4,\"total_tokens\":16,\"cost\":0.000123,\"prompt_tokens_details\":{\"cached_tokens\":6},\"completion_tokens_details\":{"
      "\"reasoning_tokens\":2}}}\n\ndata: [DONE]\n\n"
    ),
  } );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort() );

  QSignalSpy usageSpy( &router, &QgsAiModelRouter::usageReported );
  QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, true );

  QTRY_COMPARE_WITH_TIMEOUT( finishedSpy.count(), 1, 10000 );
  QCOMPARE( usageSpy.count(), 1 );
  const QList<QVariant> args = usageSpy.takeFirst();
  // The model that actually served the response is reported, not the requested one.
  QCOMPARE( args.at( 2 ).toString(), u"anthropic/claude-sonnet-4.6"_s );
  const QgsAiUsage usage = args.at( 3 ).value<QgsAiUsage>();
  QCOMPARE( usage.promptTokens, static_cast<qint64>( 12 ) );
  QCOMPARE( usage.completionTokens, static_cast<qint64>( 4 ) );
  QCOMPARE( usage.totalTokens, static_cast<qint64>( 16 ) );
  QCOMPARE( usage.cachedTokens, static_cast<qint64>( 6 ) );
  QCOMPARE( usage.reasoningTokens, static_cast<qint64>( 2 ) );
  QVERIFY( usage.costUsd > 0.0001 && usage.costUsd < 0.0002 );
}

void TestQgsAiModelRouter::openRouterUsageParsedFromNonStreamingBody()
{
  QgsSettings settings;
  settings.remove( u"ai/provider/openrouter"_s );
  const auto cleanup = qScopeGuard( []() { removeOpenRouterTestSettings(); } );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::
      jsonResponse( 200, "OK", QByteArrayLiteral( "{\"model\":\"served/model\",\"choices\":[{\"message\":{\"role\":\"assistant\",\"content\":\"OK\"},\"finish_reason\":\"stop\"}],\"usage\":{\"prompt_tokens\":7,\"completion_tokens\":3,\"total_tokens\":10,\"cost\":0.0005}}" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiModelRouter router;
  configureOpenRouterForLoopback( router, server.serverPort() );

  QSignalSpy usageSpy( &router, &QgsAiModelRouter::usageReported );
  QSignalSpy finishedSpy( &router, &QgsAiModelRouter::requestFinished );
  router.startChatRequest( QgsAiModelRouter::Provider::OpenRouter, { userMessage( u"hello"_s ) }, false );

  QTRY_COMPARE_WITH_TIMEOUT( finishedSpy.count(), 1, 10000 );
  QCOMPARE( usageSpy.count(), 1 );
  const QList<QVariant> args = usageSpy.takeFirst();
  QCOMPARE( args.at( 2 ).toString(), u"served/model"_s );
  const QgsAiUsage usage = args.at( 3 ).value<QgsAiUsage>();
  QCOMPARE( usage.promptTokens, static_cast<qint64>( 7 ) );
  QCOMPARE( usage.completionTokens, static_cast<qint64>( 3 ) );
  QCOMPARE( usage.totalTokens, static_cast<qint64>( 10 ) );
}

void TestQgsAiModelRouter::liveOpenAiRequest()
{
  const QString apiKey = qEnvironmentVariable( "OPENAI_API_KEY" );
  if ( apiKey.trimmed().isEmpty() )
    QSKIP( "OPENAI_API_KEY non disponibile: test live OpenAI skippato." );

  QNetworkRequest request( QUrl( u"https://api.openai.com/v1/responses"_s ) );
  request.setHeader( QNetworkRequest::ContentTypeHeader, u"application/json"_s );
  request.setRawHeader( "Authorization", QByteArray( "Bearer " ) + apiKey.toUtf8() );
  request.setTransferTimeout( 30000 );

  const QByteArray payload = R"({"model":"gpt-4.1-mini","input":[{"role":"user","content":[{"type":"input_text","text":"reply with OK"}]}],"max_output_tokens":16})";
  QNetworkAccessManager nam;
  QNetworkReply *reply = nam.post( request, payload );
  QVERIFY2( waitForReply( reply, 35000 ), "Timeout durante test live OpenAI" );

  const int status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  const QByteArray body = reply->readAll();
  reply->deleteLater();

  QVERIFY2( status >= 200 && status < 300, qPrintable( u"OpenAI HTTP status: %1"_s.arg( status ) ) );
  const QJsonDocument doc = QJsonDocument::fromJson( body );
  QVERIFY( doc.isObject() );
}

void TestQgsAiModelRouter::liveClaudeRequest()
{
  const QString apiKey = !qEnvironmentVariable( "CLAUDE_API_KEY" ).trimmed().isEmpty() ? qEnvironmentVariable( "CLAUDE_API_KEY" ) : qEnvironmentVariable( "ANTHROPIC_API_KEY" );
  if ( apiKey.trimmed().isEmpty() )
    QSKIP( "CLAUDE_API_KEY non disponibile: test live Claude skippato." );

  QgsAiModelRouter router;
  const QString model = !qEnvironmentVariable( "CLAUDE_MODEL" ).trimmed().isEmpty() ? qEnvironmentVariable( "CLAUDE_MODEL" ) : router.providerSettings( QgsAiModelRouter::Provider::Claude ).model;

  QNetworkRequest request( QUrl( u"https://api.anthropic.com/v1/messages"_s ) );
  request.setHeader( QNetworkRequest::ContentTypeHeader, u"application/json"_s );
  request.setRawHeader( "x-api-key", apiKey.toUtf8() );
  request.setRawHeader( "anthropic-version", "2023-06-01" );
  request.setTransferTimeout( 30000 );

  const QByteArray payload = QStringLiteral( R"({"model":"%1","max_tokens":16,"messages":[{"role":"user","content":[{"type":"text","text":"reply with OK"}]}]})" ).arg( model ).toUtf8();
  QNetworkAccessManager nam;
  QNetworkReply *reply = nam.post( request, payload );
  QVERIFY2( waitForReply( reply, 35000 ), "Timeout durante test live Claude" );

  const int status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  const QByteArray body = reply->readAll();
  reply->deleteLater();

  QVERIFY2( status >= 200 && status < 300, qPrintable( u"Claude HTTP status: %1"_s.arg( status ) ) );
  const QJsonDocument doc = QJsonDocument::fromJson( body );
  QVERIFY( doc.isObject() );
}

void TestQgsAiModelRouter::liveOpenRouterRequest()
{
  const QString apiKey = qEnvironmentVariable( "OPENROUTER_API_KEY" );
  if ( apiKey.trimmed().isEmpty() )
    QSKIP( "OPENROUTER_API_KEY non disponibile: test live OpenRouter skippato." );

  QgsAiModelRouter router;
  // Allow overriding the model under test; default to the pinned production model
  // so this test also verifies that the pinned slug is still valid on OpenRouter.
  const QString model = !qEnvironmentVariable( "OPENROUTER_MODEL" ).trimmed().isEmpty() ? qEnvironmentVariable( "OPENROUTER_MODEL" )
                                                                                        : router.providerSettings( QgsAiModelRouter::Provider::OpenRouter ).model;

  QNetworkRequest request( QUrl( u"https://openrouter.ai/api/v1/chat/completions"_s ) );
  request.setHeader( QNetworkRequest::ContentTypeHeader, u"application/json"_s );
  request.setRawHeader( "Authorization", QByteArray( "Bearer " ) + apiKey.toUtf8() );
  request.setRawHeader( "HTTP-Referer", "https://github.com/francemazzi/strata" );
  request.setRawHeader( "X-Title", "Strata" );
  request.setTransferTimeout( 30000 );

  const QByteArray payload = QStringLiteral( R"({"model":"%1","max_tokens":16,"messages":[{"role":"user","content":"reply with OK"}]})" ).arg( model ).toUtf8();
  QNetworkAccessManager nam;
  QNetworkReply *reply = nam.post( request, payload );
  QVERIFY2( waitForReply( reply, 35000 ), "Timeout durante test live OpenRouter" );

  const int status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  const QByteArray body = reply->readAll();
  reply->deleteLater();

  QVERIFY2( status >= 200 && status < 300, qPrintable( u"OpenRouter HTTP status: %1 body: %2"_s.arg( status ).arg( QString::fromUtf8( body.left( 300 ) ) ) ) );
  const QJsonDocument doc = QJsonDocument::fromJson( body );
  QVERIFY( doc.isObject() );
  QVERIFY( doc.object().contains( u"choices"_s ) );
  // Usage accounting is always included by OpenRouter.
  QVERIFY( doc.object().contains( u"usage"_s ) );
}

QGSTEST_MAIN( TestQgsAiModelRouter )
#include "testqgsaimodelrouter.moc"
