/***************************************************************************
  testqgsaiopenroutermodelcatalog.cpp
  -----------------------------------
  begin                : June 2026
***************************************************************************/

#include "ai/qgsaiopenroutermodelcatalog.h"
#include "qgsaitestloopbackserver.h"
#include "qgsapplication.h"
#include "qgstest.h"

#include <QDateTime>
#include <QFile>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>

using namespace Qt::StringLiterals;

namespace
{
  const QByteArray MODELS_FIXTURE = QByteArrayLiteral(
    "{\"data\":["
    "{\"id\":\"anthropic/claude-sonnet-4.6\",\"name\":\"Claude Sonnet 4.6\",\"context_length\":200000,"
    "\"pricing\":{\"prompt\":\"0.000003\",\"completion\":\"0.000015\"},"
    "\"supported_parameters\":[\"tools\",\"tool_choice\",\"max_tokens\"]},"
    "{\"id\":\"some/no-tools-model\",\"name\":\"No Tools\",\"context_length\":8192,"
    "\"pricing\":{\"prompt\":\"0.0000001\",\"completion\":\"0.0000002\"},"
    "\"supported_parameters\":[\"max_tokens\"]},"
    "{\"id\":\"openai/gpt-5.1\",\"name\":\"GPT-5.1\",\"context_length\":400000,"
    "\"pricing\":{\"prompt\":\"0.00000125\",\"completion\":\"0.00001\"},"
    "\"supported_parameters\":[\"tools\",\"tool_choice\"]}"
    "]}"
  );
} //namespace

class TestQgsAiOpenRouterModelCatalog : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void parsesModelsFixture();
    void filtersOutNonToolModels();
    void cacheRoundTripRespectsTtl();
    void fetchAgainstLoopbackServer();
    void offlineFallsBackToCuratedList();
    void keyInfoParsesCreditsAndMapsErrors();
};

void TestQgsAiOpenRouterModelCatalog::initTestCase()
{
  QgsApplication::initQgis();
}

void TestQgsAiOpenRouterModelCatalog::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAiOpenRouterModelCatalog::parsesModelsFixture()
{
  const QList<QgsAiOpenRouterModelCatalog::ModelInfo> models = QgsAiOpenRouterModelCatalog::parseModelsJson( MODELS_FIXTURE );
  QCOMPARE( models.size(), 2 );

  const QgsAiOpenRouterModelCatalog::ModelInfo &first = models.at( 0 );
  QCOMPARE( first.id, u"anthropic/claude-sonnet-4.6"_s );
  QCOMPARE( first.name, u"Claude Sonnet 4.6"_s );
  QCOMPARE( first.contextLength, 200000 );
  QVERIFY( qAbs( first.promptUsdPerMTok - 3.0 ) < 0.001 );
  QVERIFY( qAbs( first.completionUsdPerMTok - 15.0 ) < 0.001 );
  QVERIFY( first.displayLabel().contains( u"200k ctx"_s ) );
  QVERIFY( first.displayLabel().contains( u"$3/M in"_s ) );
}

void TestQgsAiOpenRouterModelCatalog::filtersOutNonToolModels()
{
  const QList<QgsAiOpenRouterModelCatalog::ModelInfo> models = QgsAiOpenRouterModelCatalog::parseModelsJson( MODELS_FIXTURE );
  for ( const QgsAiOpenRouterModelCatalog::ModelInfo &model : models )
    QVERIFY( model.id != u"some/no-tools-model"_s );
}

void TestQgsAiOpenRouterModelCatalog::cacheRoundTripRespectsTtl()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  const QString cachePath = tempDir.filePath( u"cache.json"_s );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", MODELS_FIXTURE );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiOpenRouterModelCatalog catalog;
  catalog.setCacheFilePathOverride( cachePath );
  catalog.setApiBaseOverride( u"http://127.0.0.1:%1"_s.arg( server.serverPort() ) );

  // First refresh: cache miss → network fetch → cache written.
  {
    QSignalSpy readySpy( &catalog, &QgsAiOpenRouterModelCatalog::modelsReady );
    catalog.refresh();
    QTRY_COMPARE_WITH_TIMEOUT( readySpy.count(), 1, 10000 );
    QCOMPARE( readySpy.takeFirst().at( 1 ).toBool(), false ); // not from cache
    QCOMPARE( server.requestCount, 1 );
    QVERIFY( QFile::exists( cachePath ) );
  }

  // Second refresh: fresh cache → no network request.
  {
    QSignalSpy readySpy( &catalog, &QgsAiOpenRouterModelCatalog::modelsReady );
    catalog.refresh();
    QTRY_COMPARE_WITH_TIMEOUT( readySpy.count(), 1, 10000 );
    const QList<QVariant> args = readySpy.takeFirst();
    QCOMPARE( args.at( 1 ).toBool(), true ); // from cache
    QCOMPARE( server.requestCount, 1 );      // unchanged
  }

  // Expire the cache by rewriting fetchedAt in the past: refresh refetches.
  {
    QFile file( cachePath );
    QVERIFY( file.open( QIODevice::ReadOnly ) );
    QJsonObject root = QJsonDocument::fromJson( file.readAll() ).object();
    file.close();
    root.insert( u"fetchedAt"_s, QDateTime::currentSecsSinceEpoch() - QgsAiOpenRouterModelCatalog::CACHE_TTL_SECONDS - 60 );
    QVERIFY( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) );
    file.write( QJsonDocument( root ).toJson( QJsonDocument::Compact ) );
    file.close();

    QSignalSpy readySpy( &catalog, &QgsAiOpenRouterModelCatalog::modelsReady );
    catalog.refresh();
    QTRY_COMPARE_WITH_TIMEOUT( readySpy.count(), 1, 10000 );
    QCOMPARE( server.requestCount, 2 );
  }
}

void TestQgsAiOpenRouterModelCatalog::fetchAgainstLoopbackServer()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiTestLoopbackServer server;
  server.responses << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", MODELS_FIXTURE );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiOpenRouterModelCatalog catalog;
  catalog.setCacheFilePathOverride( tempDir.filePath( u"cache.json"_s ) );
  catalog.setApiBaseOverride( u"http://127.0.0.1:%1"_s.arg( server.serverPort() ) );

  QSignalSpy readySpy( &catalog, &QgsAiOpenRouterModelCatalog::modelsReady );
  catalog.refresh( true );
  QTRY_COMPARE_WITH_TIMEOUT( readySpy.count(), 1, 10000 );

  const QList<QgsAiOpenRouterModelCatalog::ModelInfo> models = readySpy.takeFirst().at( 0 ).value<QList<QgsAiOpenRouterModelCatalog::ModelInfo>>();
  QCOMPARE( models.size(), 2 );
  // The query string carries the tools filter.
  QVERIFY( server.lastRawRequest().contains( "supported_parameters=tools" ) );
}

void TestQgsAiOpenRouterModelCatalog::offlineFallsBackToCuratedList()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiOpenRouterModelCatalog catalog;
  catalog.setCacheFilePathOverride( tempDir.filePath( u"cache.json"_s ) );
  // Unreachable endpoint: nothing is listening on this port.
  catalog.setApiBaseOverride( u"http://127.0.0.1:9"_s );

  QSignalSpy readySpy( &catalog, &QgsAiOpenRouterModelCatalog::modelsReady );
  catalog.refresh( true );
  QTRY_COMPARE_WITH_TIMEOUT( readySpy.count(), 1, 30000 );

  const QList<QVariant> args = readySpy.takeFirst();
  const QList<QgsAiOpenRouterModelCatalog::ModelInfo> models = args.at( 0 ).value<QList<QgsAiOpenRouterModelCatalog::ModelInfo>>();
  QVERIFY( !models.isEmpty() );
  bool hasPinnedDefault = false;
  for ( const QgsAiOpenRouterModelCatalog::ModelInfo &model : models )
  {
    if ( model.id == u"anthropic/claude-sonnet-4.6"_s )
      hasPinnedDefault = true;
  }
  QVERIFY( hasPinnedDefault );
}

void TestQgsAiOpenRouterModelCatalog::keyInfoParsesCreditsAndMapsErrors()
{
  // Success: credits summary.
  {
    QgsAiTestLoopbackServer server;
    server.responses << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", QByteArrayLiteral( "{\"data\":{\"label\":\"strata-key\",\"usage\":1.25,\"limit\":10.0,\"limit_remaining\":8.75}}" ) );
    QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

    QgsAiOpenRouterModelCatalog catalog;
    catalog.setApiBaseOverride( u"http://127.0.0.1:%1"_s.arg( server.serverPort() ) );

    QSignalSpy readySpy( &catalog, &QgsAiOpenRouterModelCatalog::keyInfoReady );
    catalog.fetchKeyInfo( u"sk-or-test"_s );
    QTRY_COMPARE_WITH_TIMEOUT( readySpy.count(), 1, 10000 );
    const QString summary = readySpy.takeFirst().at( 0 ).toString();
    QVERIFY2( summary.contains( u"strata-key"_s ), qPrintable( summary ) );
    QVERIFY2( summary.contains( u"8.75"_s ), qPrintable( summary ) );
  }

  // Invalid key: distinct error message.
  {
    QgsAiTestLoopbackServer server;
    server.responses << QgsAiTestLoopbackServer::jsonResponse( 401, "Unauthorized", QByteArrayLiteral( "{\"error\":{\"code\":401,\"message\":\"invalid key\"}}" ) );
    QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

    QgsAiOpenRouterModelCatalog catalog;
    catalog.setApiBaseOverride( u"http://127.0.0.1:%1"_s.arg( server.serverPort() ) );

    QSignalSpy failedSpy( &catalog, &QgsAiOpenRouterModelCatalog::keyInfoFailed );
    catalog.fetchKeyInfo( u"sk-or-bad"_s );
    QTRY_COMPARE_WITH_TIMEOUT( failedSpy.count(), 1, 10000 );
    QVERIFY( failedSpy.takeFirst().at( 0 ).toString().contains( u"invalid or disabled"_s ) );
  }

  // Empty key: fails fast without network.
  {
    QgsAiOpenRouterModelCatalog catalog;
    QSignalSpy failedSpy( &catalog, &QgsAiOpenRouterModelCatalog::keyInfoFailed );
    catalog.fetchKeyInfo( QString() );
    QTRY_COMPARE_WITH_TIMEOUT( failedSpy.count(), 1, 5000 );
  }
}

QGSTEST_MAIN( TestQgsAiOpenRouterModelCatalog )
#include "testqgsaiopenroutermodelcatalog.moc"
