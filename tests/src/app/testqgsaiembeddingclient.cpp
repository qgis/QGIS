/***************************************************************************
  testqgsaiembeddingclient.cpp
  --------------------------------
  begin                : June 2026
***************************************************************************/

#include "ai/index/qgsaiembeddingclient.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QHostAddress>
#include <QList>
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
  class Loopback401Server : public QTcpServer
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
      if ( args.at( 1 ).toString() == u"AI/Index"_s && args.at( 0 ).toString().contains( u"authentication failed"_s, Qt::CaseInsensitive ) )
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

QGSTEST_MAIN( TestQgsAiEmbeddingClient )
#include "testqgsaiembeddingclient.moc"
