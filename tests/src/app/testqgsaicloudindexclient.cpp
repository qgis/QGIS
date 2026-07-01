/***************************************************************************
  testqgsaicloudindexclient.cpp
  -----------------------------
  begin                : July 2026
***************************************************************************/

#include "ai/index/qgsaicloudindexclient.h"
#include "qgsaitestloopbackserver.h"
#include "qgstest.h"

#include <QCryptographicHash>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QString>
#include <QTemporaryDir>
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
} //namespace

class TestQgsAiCloudIndexClient : public QObject
{
    Q_OBJECT

  private slots:
    void fingerprintUsesFullWorkspaceSha1();
    void layerChunksBecomeMetadataOnlyContext();
    void workspaceFoldersReadRulesAndSkills();
    void validationRejectsSpatialPayloads();
    void syncContextUpsertsWorkspaceAndPostsBearerContext();
    void syncContextBatchesLargePayloads();
};

void TestQgsAiCloudIndexClient::fingerprintUsesFullWorkspaceSha1()
{
  QTemporaryDir dir;
  QVERIFY( dir.isValid() );
  const QString root = QDir( dir.path() ).absolutePath();
  const QString expected = QString::fromLatin1( QCryptographicHash::hash( root.toUtf8(), QCryptographicHash::Sha1 ).toHex() );
  QCOMPARE( QgsAiCloudIndexClient::workspaceFingerprint( root ), expected );
  QCOMPARE( expected.size(), 40 );
}

void TestQgsAiCloudIndexClient::layerChunksBecomeMetadataOnlyContext()
{
  QgsAiWorkspaceIndex::Chunk chunk;
  chunk.sourceType = QString::fromLatin1( QgsAiWorkspaceIndex::SOURCE_TYPE_LAYER );
  chunk.relativePath = u"Roads"_s;
  chunk.layerId = u"roads-1"_s;
  chunk.firstFeatureId = 10;
  chunk.lastFeatureId = 12;
  chunk.chunkIndex = 2;
  chunk.text = u"Vector layer 'Roads' (id=roads-1, crs=EPSG:3857, geometry=Line)\n"
               "feature_count=unknown; sampled_feature_limit=200; chunk_limit=20\n"
               "extent=(1,2,3,4)\n"
               "fields=name:String, speed:Integer\n"
               "[10] name=A; speed=30; bbox=(1,2,3,4); type=Line\n"_s;
  chunk.wktBlob = qCompress( QByteArrayLiteral( "LINESTRING(1 2,3 4)" ) );

  const QList<QgsAiCloudIndexClient::ContextItem> items = QgsAiCloudIndexClient::contextItemsFromChunks( { chunk } );
  QCOMPARE( items.size(), 1 );
  QCOMPARE( items.first().sourceType, u"layer"_s );
  QCOMPARE( items.first().layerId, u"roads-1"_s );
  QCOMPARE( items.first().chunkIndex, 2 );
  QVERIFY( items.first().text.contains( u"fields=name:String"_s ) );
  QVERIFY( items.first().text.contains( u"feature_id_range=10..12"_s ) );
  QVERIFY( !items.first().text.contains( u"bbox"_s, Qt::CaseInsensitive ) );
  QVERIFY( !items.first().text.contains( u"extent"_s, Qt::CaseInsensitive ) );
  QVERIFY( !items.first().text.contains( u"LINESTRING"_s, Qt::CaseInsensitive ) );

  QString error;
  QVERIFY2( QgsAiCloudIndexClient::validateContextItems( items, &error ), qPrintable( error ) );
}

void TestQgsAiCloudIndexClient::workspaceFoldersReadRulesAndSkills()
{
  QTemporaryDir dir;
  QVERIFY( dir.isValid() );
  QVERIFY( QDir( dir.path() ).mkpath( u".strata/rules"_s ) );
  QVERIFY( QDir( dir.path() ).mkpath( u".strata/skills"_s ) );

  QFile rule( QDir( dir.path() ).filePath( u".strata/rules/base.md"_s ) );
  QVERIFY( rule.open( QIODevice::WriteOnly | QIODevice::Text ) );
  rule.write( "Always cite assumptions." );
  rule.close();

  QFile skill( QDir( dir.path() ).filePath( u".strata/skills/qgis.txt"_s ) );
  QVERIFY( skill.open( QIODevice::WriteOnly | QIODevice::Text ) );
  skill.write( "Inspect layer metadata before edits." );
  skill.close();

  const QList<QgsAiCloudIndexClient::ContextItem> items = QgsAiCloudIndexClient::contextItemsFromWorkspaceFolders( dir.path(), u".strata/rules"_s, u".strata/skills"_s );
  QCOMPARE( items.size(), 2 );
  QCOMPARE( items.at( 0 ).sourceType, u"rule"_s );
  QCOMPARE( items.at( 0 ).path, u".strata/rules/base.md"_s );
  QCOMPARE( items.at( 1 ).sourceType, u"skill"_s );
  QCOMPARE( items.at( 1 ).path, u".strata/skills/qgis.txt"_s );
}

void TestQgsAiCloudIndexClient::validationRejectsSpatialPayloads()
{
  QgsAiCloudIndexClient::ContextItem validPdf;
  validPdf.sourceType = u"pdf"_s;
  validPdf.path = u"docs/report.pdf"_s;
  validPdf.mimeType = u"application/pdf"_s;
  validPdf.text = u"Zoning report summary."_s;
  QVERIFY( !QgsAiCloudIndexClient::containsForbiddenPayload( validPdf ) );

  QgsAiCloudIndexClient::ContextItem badWkt = validPdf;
  badWkt.text = u"POINT(1 2)"_s;
  QVERIFY( QgsAiCloudIndexClient::containsForbiddenPayload( badWkt ) );

  QgsAiCloudIndexClient::ContextItem badUri = validPdf;
  badUri.path = u"postgres://example/table"_s;
  QVERIFY( QgsAiCloudIndexClient::containsForbiddenPayload( badUri ) );

  QString error;
  QVERIFY( !QgsAiCloudIndexClient::validateContextItems( { badWkt }, &error ) );
  QVERIFY( error.contains( u"geometry"_s, Qt::CaseInsensitive ) );
}

void TestQgsAiCloudIndexClient::syncContextUpsertsWorkspaceAndPostsBearerContext()
{
  QTemporaryDir dir;
  QVERIFY( dir.isValid() );

  QgsAiTestLoopbackServer server;
  server.responses
    << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", QByteArrayLiteral( R"({"id":"ws_123","fingerprint":"fp","name":"Demo","role":"owner","createdAt":"2026-01-01T00:00:00Z","updatedAt":"2026-01-01T00:00:00Z"})" ) )
    << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", QByteArrayLiteral( R"({"upserted":0,"queued":1})" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QgsAiCloudIndexClient::ContextItem item;
  item.sourceType = u"rule"_s;
  item.path = u".strata/rules/base.md"_s;
  item.text = u"Always cite assumptions."_s;

  QgsAiCloudIndexClient client;
  QSignalSpy syncedSpy( &client, &QgsAiCloudIndexClient::contextSynced );
  QSignalSpy failedSpy( &client, &QgsAiCloudIndexClient::requestFailed );
  client.syncWorkspaceContext( u"http://127.0.0.1:%1/ai/messages"_s.arg( server.serverPort() ), u"strata_dt_123"_s, dir.path(), u"Demo"_s, { item }, true );
  QVERIFY( waitForSignal( &client, SIGNAL( contextSynced( QgsAiCloudIndexClient::SyncResult ) ) ) );

  QCOMPARE( failedSpy.count(), 0 );
  QCOMPARE( syncedSpy.count(), 1 );
  const auto result = qvariant_cast<QgsAiCloudIndexClient::SyncResult>( syncedSpy.at( 0 ).at( 0 ) );
  QCOMPARE( result.workspaceId, u"ws_123"_s );
  QCOMPARE( result.queued, 1 );
  QCOMPARE( server.requestCount, 2 );
  QVERIFY( server.rawRequests.at( 0 ).startsWith( "POST /v1/workspaces " ) );
  QVERIFY( server.rawRequests.at( 1 ).startsWith( "POST /v1/index/ws_123/context " ) );
  QVERIFY( server.rawRequests.at( 0 ).toLower().contains( "authorization: bearer strata_dt_123" ) );
  QVERIFY( server.rawRequests.at( 1 ).toLower().contains( "authorization: bearer strata_dt_123" ) );

  const QJsonObject workspaceBody = QJsonDocument::fromJson( server.requestBodies.at( 0 ) ).object();
  QCOMPARE( workspaceBody.value( u"fingerprint"_s ).toString(), QgsAiCloudIndexClient::workspaceFingerprint( dir.path() ) );
  QCOMPARE( workspaceBody.value( u"name"_s ).toString(), u"Demo"_s );

  const QJsonObject contextBody = QJsonDocument::fromJson( server.requestBodies.at( 1 ) ).object();
  QVERIFY( contextBody.value( u"contentOptIn"_s ).toBool() );
  const QJsonArray items = contextBody.value( u"items"_s ).toArray();
  QCOMPARE( items.size(), 1 );
  QCOMPARE( items.first().toObject().value( u"sourceType"_s ).toString(), u"rule"_s );
  QVERIFY( !QJsonDocument( contextBody ).toJson( QJsonDocument::Compact ).contains( "geometry" ) );
  QVERIFY( !QJsonDocument( contextBody ).toJson( QJsonDocument::Compact ).contains( "coordinates" ) );
  QVERIFY( !QJsonDocument( contextBody ).toJson( QJsonDocument::Compact ).contains( "datasourceUri" ) );
}

void TestQgsAiCloudIndexClient::syncContextBatchesLargePayloads()
{
  QTemporaryDir dir;
  QVERIFY( dir.isValid() );

  QgsAiTestLoopbackServer server;
  server.responses
    << QgsAiTestLoopbackServer::
         jsonResponse( 200, "OK", QByteArrayLiteral( R"({"id":"ws_batch","fingerprint":"fp","name":"Batch","role":"owner","createdAt":"2026-01-01T00:00:00Z","updatedAt":"2026-01-01T00:00:00Z"})" ) )
    << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", QByteArrayLiteral( R"({"upserted":0,"queued":128})" ) )
    << QgsAiTestLoopbackServer::jsonResponse( 200, "OK", QByteArrayLiteral( R"({"upserted":0,"queued":1})" ) );
  QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

  QList<QgsAiCloudIndexClient::ContextItem> items;
  for ( int i = 0; i < 129; ++i )
  {
    QgsAiCloudIndexClient::ContextItem item;
    item.sourceType = u"rule"_s;
    item.path = u".strata/rules/%1.md"_s.arg( i );
    item.text = u"Rule %1"_s.arg( i );
    items << item;
  }

  QgsAiCloudIndexClient client;
  QSignalSpy syncedSpy( &client, &QgsAiCloudIndexClient::contextSynced );
  client.syncWorkspaceContext( u"http://127.0.0.1:%1/ai/messages"_s.arg( server.serverPort() ), u"strata_dt_123"_s, dir.path(), u"Batch"_s, items, true );
  QVERIFY( waitForSignal( &client, SIGNAL( contextSynced( QgsAiCloudIndexClient::SyncResult ) ) ) );

  QCOMPARE( server.requestCount, 3 );
  const auto result = qvariant_cast<QgsAiCloudIndexClient::SyncResult>( syncedSpy.at( 0 ).at( 0 ) );
  QCOMPARE( result.workspaceId, u"ws_batch"_s );
  QCOMPARE( result.queued, 129 );
  QCOMPARE( QJsonDocument::fromJson( server.requestBodies.at( 1 ) ).object().value( u"items"_s ).toArray().size(), 128 );
  QCOMPARE( QJsonDocument::fromJson( server.requestBodies.at( 2 ) ).object().value( u"items"_s ).toArray().size(), 1 );
}

QGSTEST_MAIN( TestQgsAiCloudIndexClient )
#include "testqgsaicloudindexclient.moc"
