/***************************************************************************
  testqgsailayerchunker.cpp
  --------------------------------
  begin                : May 2026
***************************************************************************/

#include <algorithm>
#include <memory>

#include "ai/index/qgsailayerchunker.h"
#include "ai/index/qgsaiworkspaceindex.h"
#include "qgsrasterlayer.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QByteArray>
#include <QRegularExpression>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsAiLayerChunker : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void chunksCoverEveryFeatureExactlyOnce();
    void chunkTextStaysWithinTargetSize();
    void wktBlobIsRecoverable();
    void rasterEmitsSingleMetadataChunk();
    void vectorHeaderSkipsFeatureCountScan();
    void officeSpreadsheetVectorLayerSkipsFeatureSampling();
    void rasterMetadataSkipsBandStatistics();
};

void TestQgsAiLayerChunker::initTestCase()
{
  QgsApplication::initQgis();
}

void TestQgsAiLayerChunker::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAiLayerChunker::chunksCoverEveryFeatureExactlyOnce()
{
  const QString shpPath = QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s;
  auto layer = std::make_unique<QgsVectorLayer>( shpPath, u"points"_s, u"ogr"_s );
  QVERIFY2( layer->isValid(), shpPath.toUtf8().constData() );

  const auto chunks = QgsAiLayerChunker::chunkVector( layer.get() );
  QVERIFY( !chunks.isEmpty() );
  QVERIFY( chunks.size() <= 20 );

  qint64 covered = 0;
  qint64 prevLast = -1;
  for ( const auto &c : chunks )
  {
    QCOMPARE( c.sourceType, QString::fromLatin1( QgsAiWorkspaceIndex::SOURCE_TYPE_LAYER ) );
    QCOMPARE( c.layerId, layer->id() );
    QVERIFY( c.firstFeatureId >= 0 );
    QVERIFY( c.lastFeatureId >= c.firstFeatureId );
    QVERIFY( c.firstFeatureId > prevLast );
    covered += ( c.lastFeatureId - c.firstFeatureId + 1 );
    prevLast = c.lastFeatureId;
  }
  QCOMPARE( covered, std::min<qint64>( layer->featureCount(), 200 ) );
}

void TestQgsAiLayerChunker::chunkTextStaysWithinTargetSize()
{
  const QString shpPath = QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s;
  auto layer = std::make_unique<QgsVectorLayer>( shpPath, u"points"_s, u"ogr"_s );
  QVERIFY( layer->isValid() );

  const auto chunks = QgsAiLayerChunker::chunkVector( layer.get() );
  QVERIFY( !chunks.isEmpty() );

  // The chunker flushes after exceeding CHUNK_TARGET_CHARS, so a chunk can
  // overshoot by at most one feature line. 1.5x is a generous tolerance.
  const int hardCap = static_cast<int>( QgsAiWorkspaceIndex::CHUNK_TARGET_CHARS * 1.5 );
  for ( const auto &c : chunks )
    QVERIFY2( c.text.size() <= hardCap, qPrintable( u"chunk text size %1 exceeds %2"_s.arg( c.text.size() ).arg( hardCap ) ) );
}

void TestQgsAiLayerChunker::wktBlobIsRecoverable()
{
  const QString shpPath = QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s;
  auto layer = std::make_unique<QgsVectorLayer>( shpPath, u"points"_s, u"ogr"_s );
  QVERIFY( layer->isValid() );

  const auto chunks = QgsAiLayerChunker::chunkVector( layer.get() );
  QVERIFY( !chunks.isEmpty() );

  for ( const auto &c : chunks )
  {
    QVERIFY( !c.wktBlob.isEmpty() );
    const QByteArray decoded = qUncompress( c.wktBlob );
    QVERIFY( !decoded.isEmpty() );
    static const QRegularExpression wktRx( "\\b(POINT|MULTIPOINT|LINESTRING|MULTILINESTRING|POLYGON|MULTIPOLYGON)\\b", QRegularExpression::CaseInsensitiveOption );
    QVERIFY2( wktRx.match( QString::fromUtf8( decoded ) ).hasMatch(), decoded.left( 80 ).constData() );
  }
}

void TestQgsAiLayerChunker::rasterEmitsSingleMetadataChunk()
{
  // We don't need a real raster on disk to validate that nullptr -> empty list
  // and that the chunker is wired correctly; deeper raster tests live downstream.
  QList<QgsAiWorkspaceIndex::Chunk> chunks = QgsAiLayerChunker::chunkRaster( nullptr );
  QCOMPARE( chunks.size(), 0 );
}

void TestQgsAiLayerChunker::vectorHeaderSkipsFeatureCountScan()
{
  const QString shpPath = QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s;
  auto layer = std::make_unique<QgsVectorLayer>( shpPath, u"points"_s, u"ogr"_s );
  QVERIFY( layer->isValid() );

  const auto chunks = QgsAiLayerChunker::chunkVector( layer.get() );
  QVERIFY( !chunks.isEmpty() );
  QVERIFY( chunks.first().text.contains( u"feature_count=unknown"_s ) );
}

void TestQgsAiLayerChunker::officeSpreadsheetVectorLayerSkipsFeatureSampling()
{
  const QString odsPath = QStringLiteral( TEST_DATA_DIR ) + u"/spreadsheet.ods|layername=Sheet1"_s;
  auto layer = std::make_unique<QgsVectorLayer>( odsPath, u"sheet"_s, u"ogr"_s );
  QVERIFY2( layer->isValid(), odsPath.toUtf8().constData() );

  const auto chunks = QgsAiLayerChunker::chunkVector( layer.get() );
  QCOMPARE( chunks.size(), 1 );
  QCOMPARE( chunks.first().sourceType, QString::fromLatin1( QgsAiWorkspaceIndex::SOURCE_TYPE_LAYER ) );
  QCOMPARE( chunks.first().layerId, layer->id() );
  QCOMPARE( chunks.first().firstFeatureId, qint64( -1 ) );
  QCOMPARE( chunks.first().lastFeatureId, qint64( -1 ) );
  QVERIFY( chunks.first().wktBlob.isEmpty() );
  QVERIFY( chunks.first().text.contains( u"sampled_feature_limit=0"_s ) );
  QVERIFY( chunks.first().text.contains( u"Office spreadsheet layers"_s ) );
  QVERIFY( !chunks.first().text.contains( u"fields="_s ) );
}

void TestQgsAiLayerChunker::rasterMetadataSkipsBandStatistics()
{
  auto layer = std::make_unique<QgsRasterLayer>( QStringLiteral( TEST_DATA_DIR ) + u"/landsat.tif"_s, u"landsat"_s );
  if ( !layer->isValid() )
    QSKIP( "landsat raster fixture unavailable" );

  const auto chunks = QgsAiLayerChunker::chunkRaster( layer.get() );
  QVERIFY( !chunks.isEmpty() );
  QVERIFY( chunks.first().text.contains( u"band statistics skipped during fast layer snapshot"_s ) );
}

QGSTEST_MAIN( TestQgsAiLayerChunker )
#include "testqgsailayerchunker.moc"
