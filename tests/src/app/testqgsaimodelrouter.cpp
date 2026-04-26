/***************************************************************************
  testqgsaimodelrouter.cpp
  ------------------------
  begin                : April 2026
***************************************************************************/

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
#include <QTimer>

namespace
{
  void loadEnvFileIfPresent()
  {
    QDir dir( QDir::currentPath() );
    while ( true )
    {
      const QString candidate = dir.filePath( QStringLiteral( ".env.test" ) );
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
}

class TestQgsAiModelRouter : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void buildPayloadForOpenAi();
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
  message.content = QStringLiteral( "hello" );

  const QByteArray payload = router.buildRequestPayload( QgsAiModelRouter::Provider::OpenAi, {message}, true );
  const QJsonDocument doc = QJsonDocument::fromJson( payload );
  QVERIFY( doc.isObject() );
  const QJsonObject object = doc.object();
  QCOMPARE( object.value( QStringLiteral( "stream" ) ).toBool(), true );
  QCOMPARE( object.value( QStringLiteral( "model" ) ).toString(), QStringLiteral( "gpt-4.1-mini" ) );
  QVERIFY( object.contains( QStringLiteral( "input" ) ) );
}

void TestQgsAiModelRouter::sanitizeSecrets()
{
  QgsAiModelRouter router;
  const QString raw = QStringLiteral( "Authorization: Bearer sk-verysecrettoken and x-api-key: abc123" );
  const QString sanitized = router.sanitizeErrorText( raw );
  QVERIFY( !sanitized.contains( QStringLiteral( "verysecrettoken" ) ) );
  QVERIFY( !sanitized.contains( QStringLiteral( "abc123" ) ) );
  QVERIFY( sanitized.contains( QStringLiteral( "REDACTED" ) ) );
}

void TestQgsAiModelRouter::storeApiKeyPersistsInSettings()
{
  QgsSettings settings;
  settings.remove( QStringLiteral( "ai/provider/openai" ) );

  QgsAiModelRouter router;
  QString error;
  QVERIFY2( router.storeApiKey( QgsAiModelRouter::Provider::OpenAi, QStringLiteral( "sk-test-local-storage" ), &error ), qPrintable( error ) );

  QgsAiModelRouter reloadedRouter;
  QNetworkRequest request( QUrl( QStringLiteral( "https://api.openai.com/v1/responses" ) ) );
  QVERIFY2( reloadedRouter.applyAuthentication( QgsAiModelRouter::Provider::OpenAi, request, &error ), qPrintable( error ) );
  QCOMPARE( request.rawHeader( "Authorization" ), QByteArray( "Bearer sk-test-local-storage" ) );
  QVERIFY( reloadedRouter.providerSettings( QgsAiModelRouter::Provider::OpenAi ).enabled );

  settings.remove( QStringLiteral( "ai/provider/openai" ) );
}

void TestQgsAiModelRouter::liveOpenAiRequest()
{
  const QString apiKey = qEnvironmentVariable( "OPENAI_API_KEY" );
  if ( apiKey.trimmed().isEmpty() )
    QSKIP( "OPENAI_API_KEY non disponibile: test live OpenAI skippato." );

  QNetworkRequest request( QUrl( QStringLiteral( "https://api.openai.com/v1/responses" ) ) );
  request.setHeader( QNetworkRequest::ContentTypeHeader, QStringLiteral( "application/json" ) );
  request.setRawHeader( "Authorization", QByteArray( "Bearer " ) + apiKey.toUtf8() );
  request.setTransferTimeout( 30000 );

  const QByteArray payload = R"({"model":"gpt-4.1-mini","input":[{"role":"user","content":[{"type":"input_text","text":"reply with OK"}]}],"max_output_tokens":16})";
  QNetworkAccessManager nam;
  QNetworkReply *reply = nam.post( request, payload );
  QVERIFY2( waitForReply( reply, 35000 ), "Timeout durante test live OpenAI" );

  const int status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  const QByteArray body = reply->readAll();
  reply->deleteLater();

  QVERIFY2( status >= 200 && status < 300, qPrintable( QStringLiteral( "OpenAI HTTP status: %1" ).arg( status ) ) );
  const QJsonDocument doc = QJsonDocument::fromJson( body );
  QVERIFY( doc.isObject() );
}

void TestQgsAiModelRouter::liveClaudeRequest()
{
  const QString apiKey = !qEnvironmentVariable( "CLAUDE_API_KEY" ).trimmed().isEmpty()
                           ? qEnvironmentVariable( "CLAUDE_API_KEY" )
                           : qEnvironmentVariable( "ANTHROPIC_API_KEY" );
  if ( apiKey.trimmed().isEmpty() )
    QSKIP( "CLAUDE_API_KEY non disponibile: test live Claude skippato." );

  QgsAiModelRouter router;
  const QString model = !qEnvironmentVariable( "CLAUDE_MODEL" ).trimmed().isEmpty()
                          ? qEnvironmentVariable( "CLAUDE_MODEL" )
                          : router.providerSettings( QgsAiModelRouter::Provider::Claude ).model;

  QNetworkRequest request( QUrl( QStringLiteral( "https://api.anthropic.com/v1/messages" ) ) );
  request.setHeader( QNetworkRequest::ContentTypeHeader, QStringLiteral( "application/json" ) );
  request.setRawHeader( "x-api-key", apiKey.toUtf8() );
  request.setRawHeader( "anthropic-version", "2023-06-01" );
  request.setTransferTimeout( 30000 );

  const QByteArray payload = QStringLiteral( R"({"model":"%1","max_tokens":16,"messages":[{"role":"user","content":[{"type":"text","text":"reply with OK"}]}]})" )
                               .arg( model )
                               .toUtf8();
  QNetworkAccessManager nam;
  QNetworkReply *reply = nam.post( request, payload );
  QVERIFY2( waitForReply( reply, 35000 ), "Timeout durante test live Claude" );

  const int status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  const QByteArray body = reply->readAll();
  reply->deleteLater();

  QVERIFY2( status >= 200 && status < 300, qPrintable( QStringLiteral( "Claude HTTP status: %1" ).arg( status ) ) );
  const QJsonDocument doc = QJsonDocument::fromJson( body );
  QVERIFY( doc.isObject() );
}

QGSTEST_MAIN( TestQgsAiModelRouter )
#include "testqgsaimodelrouter.moc"
