/***************************************************************************
  testqgsaiembeddingclient.cpp
  --------------------------------
  begin                : June 2026
***************************************************************************/

#include "ai/index/qgsaiembeddingclient.h"
#include "qgsaitestloopbackserver.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QElapsedTimer>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVariant>
#include <QVector>

using namespace Qt::StringLiterals;

namespace
{
  /**
   * Minimal loopback HTTP server that answers every request with 401 Unauthorized
   * and counts how many requests it received. Used to drive the embedding client's
   * authentication circuit breaker without touching a live API.
   */
  class Loopback401Server : public QTcpServer // clazy:exclude=missing-qobject-macro
  {
    public:
      int requestCount = 0;

    protected:
      void incomingConnection( qintptr handle ) override
      {
        ++requestCount;
        QTcpSocket *socket = new QTcpSocket( this );
        socket->setSocketDescriptor( handle );
        connect( socket, &QTcpSocket::readyRead, socket, [socket]() {
          if ( socket->property( "answered" ).toBool() )
          {
            socket->readAll();
            return;
          }
          socket->setProperty( "answered", true );
          socket->readAll();
          const QByteArray body = QByteArrayLiteral( "{\"error\":{\"message\":\"invalid api key\"}}" );
          QByteArray response = "HTTP/1.1 401 Unauthorized\r\n";
          response += "Content-Type: application/json\r\n";
          response += "Content-Length: " + QByteArray::number( body.size() ) + "\r\n";
          response += "Connection: close\r\n\r\n";
          response += body;
          socket->write( response );
          socket->flush();
          socket->disconnectFromHost();
        } );
        connect( socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater );
      }
  };

  int countAuthFailureLogs( const QSignalSpy &spy )
  {
    int count = 0;
    for ( const QList<QVariant> &args : spy )
    {
      if ( args.size() < 2 )
        continue;
      if ( args.at( 1 ).toString() == "AI/Index"_L1 && args.at( 0 ).toString().contains( u"authentication failed"_s, Qt::CaseInsensitive ) )
        ++count;
    }
    return count;
  }
} // namespace

class TestQgsAiEmbeddingClient : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void authFailureTripsBreakerAndLogsOnce();
    void retryAfterHonoredForEmbeddings429();
    void http402TripsBreakerWithCreditsMessage();
    void successfulEmbedParsesVectorsInIndexOrder();
    void openRouterEmbedPayloadCarriesModelAndProviderPrefs();
};

void TestQgsAiEmbeddingClient::initTestCase()
{
  QgsApplication::initQgis();
}

void TestQgsAiEmbeddingClient::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAiEmbeddingClient::authFailureTripsBreakerAndLogsOnce()
{
  Loopback401Server server;
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  // Provide an API key so the client proceeds to the network request rather than
  // failing fast on a missing key.
  QgsSettings settings;
  const bool hadKey = settings.contains( u"ai/provider/openai/apiKey"_s );
  const QVariant savedKey = settings.value( u"ai/provider/openai/apiKey"_s );
  settings.setValue( u"ai/provider/openai/apiKey"_s, u"sk-loopback-test"_s );

  QSignalSpy logSpy( QgsApplication::messageLog(), &QgsMessageLog::messageReceivedWithFormat );

  QgsAiEmbeddingClient client;
  client.setProvider( QgsAiEmbeddingClient::Provider::OpenAi );
  client.setEndpointOverride( u"http://127.0.0.1:%1/v1/embeddings"_s.arg( server.serverPort() ) );

  // First call: hits the server, gets 401, trips the breaker and logs once.
  QList<QVector<float>> out;
  QString err;
  QVERIFY( !client.embed( { u"hello"_s }, out, &err ) );
  QVERIFY( client.authenticationFailed() );
  QCOMPARE( server.requestCount, 1 );
  QCOMPARE( countAuthFailureLogs( logSpy ), 1 );

  // Second call: must short-circuit without any new network request, and without
  // logging the authentication failure again.
  QVERIFY( !client.embed( { u"world"_s }, out, &err ) );
  QCOMPARE( server.requestCount, 1 );
  QCOMPARE( countAuthFailureLogs( logSpy ), 1 );

  // Resetting the breaker re-enables requests.
  client.resetCircuitBreaker();
  QVERIFY( !client.authenticationFailed() );

  if ( hadKey )
    settings.setValue( u"ai/provider/openai/apiKey"_s, savedKey );
  else
    settings.remove( u"ai/provider/openai/apiKey"_s );
}

void TestQgsAiEmbeddingClient::retryAfterHonoredForEmbeddings429()
{
  QgsAiTestLoopbackServer server;
  server.responses
    << QgsAiTestLoopbackServer::
         jsonResponse( 429, "Too Many Requests", QByteArrayLiteral( "{\"error\":{\"code\":429,\"message\":\"slow down\"}}" ), { { QByteArrayLiteral( "Retry-After" ), QByteArrayLiteral( "1" ) } } )
    << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", QByteArrayLiteral( "{\"data\":[{\"embedding\":[0.1,0.2],\"index\":0}]}" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsSettings settings;
  const bool hadKey = settings.contains( u"ai/provider/openai/apiKey"_s );
  const QVariant savedKey = settings.value( u"ai/provider/openai/apiKey"_s );
  settings.setValue( u"ai/provider/openai/apiKey"_s, u"sk-loopback-test"_s );
  const auto restore = qScopeGuard( [&settings, hadKey, savedKey]() {
    if ( hadKey )
      settings.setValue( u"ai/provider/openai/apiKey"_s, savedKey );
    else
      settings.remove( u"ai/provider/openai/apiKey"_s );
  } );

  QgsAiEmbeddingClient client;
  client.setProvider( QgsAiEmbeddingClient::Provider::OpenAi );
  client.setEndpointOverride( u"http://127.0.0.1:%1/v1/embeddings"_s.arg( server.serverPort() ) );

  QElapsedTimer elapsed;
  elapsed.start();
  QList<QVector<float>> out;
  QString err;
  QVERIFY2( client.embed( { u"hello"_s }, out, &err ), qPrintable( err ) );
  QCOMPARE( server.requestCount, 2 );
  QCOMPARE( out.size(), 1 );
  QVERIFY2( elapsed.elapsed() >= 1000, qPrintable( u"elapsed=%1ms"_s.arg( elapsed.elapsed() ) ) );
}

void TestQgsAiEmbeddingClient::http402TripsBreakerWithCreditsMessage()
{
  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::jsonResponse( 402, "Payment Required", QByteArrayLiteral( "{\"error\":{\"code\":402,\"message\":\"Insufficient credits\"}}" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsSettings settings;
  const bool hadKey = settings.contains( u"ai/provider/openrouter/apiKey"_s );
  const QVariant savedKey = settings.value( u"ai/provider/openrouter/apiKey"_s );
  settings.setValue( u"ai/provider/openrouter/apiKey"_s, u"sk-or-loopback-test"_s );
  const auto restore = qScopeGuard( [&settings, hadKey, savedKey]() {
    if ( hadKey )
      settings.setValue( u"ai/provider/openrouter/apiKey"_s, savedKey );
    else
      settings.remove( u"ai/provider/openrouter/apiKey"_s );
  } );

  QgsAiEmbeddingClient client;
  client.setProvider( QgsAiEmbeddingClient::Provider::OpenRouter );
  client.setEndpointOverride( u"http://127.0.0.1:%1/api/v1/embeddings"_s.arg( server.serverPort() ) );

  QList<QVector<float>> out;
  QString err;
  QVERIFY( !client.embed( { u"hello"_s }, out, &err ) );
  QVERIFY2( err.contains( u"openrouter.ai/credits"_s ), qPrintable( err ) );
  QVERIFY( client.creditsExhausted() );
  QVERIFY( client.authenticationFailed() );

  // Breaker is tripped: no further network requests.
  QVERIFY( !client.embed( { u"world"_s }, out, &err ) );
  QCOMPARE( server.requestCount, 1 );
  QVERIFY2( err.contains( u"credits"_s, Qt::CaseInsensitive ), qPrintable( err ) );
}

void TestQgsAiEmbeddingClient::successfulEmbedParsesVectorsInIndexOrder()
{
  QgsAiTestLoopbackServer server;
  // Vectors deliberately out of order: the client must reorder them by index.
  server.responses << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", QByteArrayLiteral( "{\"data\":[{\"embedding\":[0.3,0.4],\"index\":1},{\"embedding\":[0.1,0.2],\"index\":0}]}" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsSettings settings;
  const bool hadKey = settings.contains( u"ai/provider/openai/apiKey"_s );
  const QVariant savedKey = settings.value( u"ai/provider/openai/apiKey"_s );
  settings.setValue( u"ai/provider/openai/apiKey"_s, u"sk-loopback-test"_s );
  const auto restore = qScopeGuard( [&settings, hadKey, savedKey]() {
    if ( hadKey )
      settings.setValue( u"ai/provider/openai/apiKey"_s, savedKey );
    else
      settings.remove( u"ai/provider/openai/apiKey"_s );
  } );

  QgsAiEmbeddingClient client;
  client.setProvider( QgsAiEmbeddingClient::Provider::OpenAi );
  client.setEndpointOverride( u"http://127.0.0.1:%1/v1/embeddings"_s.arg( server.serverPort() ) );

  QList<QVector<float>> out;
  QString err;
  QVERIFY2( client.embed( { u"first"_s, u"second"_s }, out, &err ), qPrintable( err ) );
  QCOMPARE( out.size(), 2 );
  QCOMPARE( out.at( 0 ).at( 0 ), 0.1f );
  QCOMPARE( out.at( 1 ).at( 0 ), 0.3f );
}

void TestQgsAiEmbeddingClient::openRouterEmbedPayloadCarriesModelAndProviderPrefs()
{
  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", QByteArrayLiteral( "{\"data\":[{\"embedding\":[0.1],\"index\":0}]}" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsSettings settings;
  const bool hadKey = settings.contains( u"ai/provider/openrouter/apiKey"_s );
  const QVariant savedKey = settings.value( u"ai/provider/openrouter/apiKey"_s );
  settings.setValue( u"ai/provider/openrouter/apiKey"_s, u"sk-or-loopback-test"_s );
  const auto restore = qScopeGuard( [&settings, hadKey, savedKey]() {
    if ( hadKey )
      settings.setValue( u"ai/provider/openrouter/apiKey"_s, savedKey );
    else
      settings.remove( u"ai/provider/openrouter/apiKey"_s );
  } );

  QgsAiEmbeddingClient client;
  client.setProvider( QgsAiEmbeddingClient::Provider::OpenRouter );
  client.setEndpointOverride( u"http://127.0.0.1:%1/api/v1/embeddings"_s.arg( server.serverPort() ) );

  QList<QVector<float>> out;
  QString err;
  QVERIFY2( client.embed( { u"hello"_s }, out, &err ), qPrintable( err ) );

  const QJsonObject payload = QJsonDocument::fromJson( server.lastRequestBody() ).object();
  QCOMPARE( payload.value( u"model"_s ).toString(), u"openai/text-embedding-3-small"_s );
  const QJsonObject provider = payload.value( u"provider"_s ).toObject();
  QCOMPARE( provider.value( u"data_collection"_s ).toString(), u"deny"_s );
  QCOMPARE( provider.value( u"sort"_s ).toString(), u"price"_s );

  // App attribution headers are sent for OpenRouter (case-insensitive: Qt
  // normalizes raw header casing, e.g. Http-Referer).
  QVERIFY( server.lastRawRequest().toLower().contains( "x-title: strata" ) );
  QVERIFY( server.lastRawRequest().toLower().contains( "http-referer:" ) );
}

QGSTEST_MAIN( TestQgsAiEmbeddingClient )
#include "testqgsaiembeddingclient.moc"
