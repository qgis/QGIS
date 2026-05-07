/***************************************************************************
  testqgsaimodelrouter.cpp
  ------------------------
  begin                : April 2026
***************************************************************************/

#include "ai/qgsaicodexoauthclient.h"
#include "ai/qgsaimodelrouter.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QString>
#include <QTimer>

using namespace Qt::StringLiterals;

namespace
{
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
    void buildPayloadForCodexUsesGpt55();
    void codexModelFallback();
    void extractChatGptAccountIdFromIdToken();
    void sanitizeSecrets();
    void storeApiKeyPersistsInSettings();
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

void TestQgsAiModelRouter::buildPayloadForCodexUsesGpt55()
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
  QCOMPARE( object.value( u"model"_s ).toString(), u"gpt-5.5"_s );
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

  QCOMPARE( router.providerSettings( QgsAiModelRouter::Provider::Codex ).model, u"gpt-5.5"_s );

  QgsAiChatMessage message;
  message.role = QgsAiChatRole::User;
  message.content = u"hello"_s;
  const QJsonObject object = QJsonDocument::fromJson( router.buildRequestPayload( QgsAiModelRouter::Provider::Codex, { message }, false ) ).object();
  QCOMPARE( object.value( u"model"_s ).toString(), u"gpt-5.5"_s );

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
