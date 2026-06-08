/***************************************************************************
  testqgsaimodelrouter.cpp
  ------------------------
  begin                : April 2026
***************************************************************************/

#include <memory>

#include "ai/qgsaiclaudeoauthclient.h"
#include "ai/qgsaicodexoauthclient.h"
#include "ai/qgsaimodelrouter.h"
#include "ai/tools/qgsaiechotool.h"
#include "ai/tools/qgsaitoolregistry.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
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
} //namespace

class TestQgsAiModelRouter : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void buildPayloadForOpenAi();
    void buildPayloadForCodexUsesGpt54();
    void codexModelFallback();
    void extractChatGptAccountIdFromIdToken();
    void claudeAuthorizationUrlUsesCurrentPlatformEndpoints();
    void claudeAuthorizationCodeParsing();
    void sanitizeSecrets();
    void storeApiKeyPersistsInSettings();
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
    void liveOpenAiRequest();
    void liveClaudeRequest();
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
  QVERIFY( query.queryItemValue( u"scope"_s ).contains( u"user:inference"_s ) );
}

void TestQgsAiModelRouter::claudeAuthorizationCodeParsing()
{
  QCOMPARE( QgsAiClaudeOAuthClient::authorizationCodeFromInput( u"manual-code"_s ), u"manual-code"_s );
  QCOMPARE( QgsAiClaudeOAuthClient::authorizationCodeFromInput( u"manual-code#returned-state"_s ), u"manual-code"_s );
  QCOMPARE( QgsAiClaudeOAuthClient::authorizationCodeFromInput( u"https://platform.claude.com/oauth/code/callback?code=query-code&state=returned-state"_s ), u"query-code"_s );
  QCOMPARE( QgsAiClaudeOAuthClient::authorizationCodeFromInput( u"https://platform.claude.com/oauth/code/callback#code=fragment-code&state=returned-state"_s ), u"fragment-code"_s );
}

void TestQgsAiModelRouter::sanitizeSecrets()
{
  QgsAiModelRouter router;
  const QString raw = u"Authorization: Bearer sk-verysecrettoken and x-api-key: abc123"_s;
  const QString sanitized = router.sanitizeErrorText( raw );
  QVERIFY( !sanitized.contains( u"verysecrettoken"_s ) );
  QVERIFY( !sanitized.contains( u"abc123"_s ) );
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

QGSTEST_MAIN( TestQgsAiModelRouter )
#include "testqgsaimodelrouter.moc"
