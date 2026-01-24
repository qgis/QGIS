/***************************************************************************
  testqgscogtilereader.cpp - Test QgsCOGTileReader performance
  --------------------------------------
  Date                 : January 2026
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgscogtilereader.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsapplication.h"

#include <QElapsedTimer>
#include <gdal.h>

class TestQgsCOGTileReader : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsCOGTileReader() : QgsTest( QStringLiteral( "COG Tile Reader Tests" ) ) {}

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testInitialization();
    void testTileReading();
    void testOverviewSelection();
    void benchmarkTileReading();
    void benchmarkVsRasterBlock();

  private:
    QString mTestDataDir;
    QString mCogFile;
};

void TestQgsCOGTileReader::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR );

  // Use a test COG file - you'll need to provide this
  // For now, let's try to use any available raster
  mCogFile = mTestDataDir + "/raster/test_cog.tif";

  // If test COG doesn't exist, try to find any GeoTIFF
  if ( !QFile::exists( mCogFile ) )
  {
    QDir testDir( mTestDataDir + "/raster" );
    const QStringList tiffs = testDir.entryList( QStringList() << "*.tif" << "*.tiff", QDir::Files );
    if ( !tiffs.isEmpty() )
    {
      mCogFile = testDir.absoluteFilePath( tiffs.first() );
      qDebug() << "Using test file:" << mCogFile;
    }
  }
}

void TestQgsCOGTileReader::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsCOGTileReader::testInitialization()
{
  if ( !QFile::exists( mCogFile ) )
  {
    QSKIP( "No test COG file available" );
  }

  GDALDatasetH dataset = GDALOpen( mCogFile.toUtf8().constData(), GA_ReadOnly );
  QVERIFY( dataset != nullptr );

  QgsCOGTileReader reader( dataset );
  QVERIFY( reader.isValid() );

  QVERIFY( reader.width() > 0 );
  QVERIFY( reader.height() > 0 );
  QVERIFY( !reader.extent().isEmpty() );

  const auto tileInfo = reader.tileInfo( 0 );
  QVERIFY( tileInfo.width > 0 );
  QVERIFY( tileInfo.height > 0 );
  QVERIFY( tileInfo.bytesPerPixel > 0 );

  qDebug() << "Dataset dimensions:" << reader.width() << "x" << reader.height();
  qDebug() << "Tile size:" << tileInfo.width << "x" << tileInfo.height;
  qDebug() << "Overview count:" << reader.overviewCount();
  qDebug() << "Is tiled:" << tileInfo.isTiled;

  GDALClose( dataset );
}

void TestQgsCOGTileReader::testTileReading()
{
  if ( !QFile::exists( mCogFile ) )
  {
    QSKIP( "No test COG file available" );
  }

  GDALDatasetH dataset = GDALOpen( mCogFile.toUtf8().constData(), GA_ReadOnly );
  QVERIFY( dataset != nullptr );

  QgsCOGTileReader reader( dataset );
  QVERIFY( reader.isValid() );

  const auto tileInfo = reader.tileInfo( 0 );

  // Try to read the first tile
  QByteArray tileData;
  const bool success = reader.readTile( 0, 0, 0, 1, tileData );

  QVERIFY( success );
  QVERIFY( !tileData.isEmpty() );

  const int expectedSize = tileInfo.width * tileInfo.height * tileInfo.bytesPerPixel;
  QCOMPARE( tileData.size(), expectedSize );

  qDebug() << "Successfully read tile of size:" << tileData.size() << "bytes";

  GDALClose( dataset );
}

void TestQgsCOGTileReader::testOverviewSelection()
{
  if ( !QFile::exists( mCogFile ) )
  {
    QSKIP( "No test COG file available" );
  }

  GDALDatasetH dataset = GDALOpen( mCogFile.toUtf8().constData(), GA_ReadOnly );
  QVERIFY( dataset != nullptr );

  QgsCOGTileReader reader( dataset );
  QVERIFY( reader.isValid() );

  if ( reader.overviewCount() == 0 )
  {
    qDebug() << "No overviews available, skipping overview test";
    GDALClose( dataset );
    QSKIP( "No overviews in test file" );
  }

  // Test overview selection logic
  const double baseRes = 1.0;  // meters per pixel at base

  // Should select base level for high detail
  int level = reader.selectBestOverview( baseRes * 0.5 );
  QCOMPARE( level, 0 );

  // Should select coarser overview for lower detail
  level = reader.selectBestOverview( baseRes * 4.0 );
  QVERIFY( level >= 0 );

  qDebug() << "Overview selection test passed";

  GDALClose( dataset );
}

void TestQgsCOGTileReader::benchmarkTileReading()
{
  if ( !QFile::exists( mCogFile ) )
  {
    QSKIP( "No test COG file available" );
  }

  GDALDatasetH dataset = GDALOpen( mCogFile.toUtf8().constData(), GA_ReadOnly );
  QVERIFY( dataset != nullptr );

  QgsCOGTileReader reader( dataset );
  QVERIFY( reader.isValid() );

  const auto tileInfo = reader.tileInfo( 0 );
  const int tilesToRead = std::min( 100, tileInfo.tilesX * tileInfo.tilesY );

  QElapsedTimer timer;
  timer.start();

  int successCount = 0;
  for ( int i = 0; i < tilesToRead; ++i )
  {
    const int tileX = i % tileInfo.tilesX;
    const int tileY = i / tileInfo.tilesX;

    QByteArray tileData;
    if ( reader.readTile( 0, tileX, tileY, 1, tileData ) )
    {
      ++successCount;
    }
  }

  const qint64 elapsed = timer.elapsed();
  const double avgTime = elapsed / static_cast<double>( tilesToRead );

  qDebug() << "QgsCOGTileReader benchmark:";
  qDebug() << "  Tiles read:" << successCount << "/" << tilesToRead;
  qDebug() << "  Total time:" << elapsed << "ms";
  qDebug() << "  Average per tile:" << avgTime << "ms";
  qDebug() << "  Tiles/second:" << ( 1000.0 * successCount / elapsed );

  QVERIFY( successCount > 0 );

  GDALClose( dataset );
}

void TestQgsCOGTileReader::benchmarkVsRasterBlock()
{
  if ( !QFile::exists( mCogFile ) )
  {
    QSKIP( "No test COG file available" );
  }

  // Open with QGIS raster layer
  QgsRasterLayer layer( mCogFile, "test", "gdal" );
  QVERIFY( layer.isValid() );

  QgsRasterDataProvider *provider = layer.dataProvider();
  QVERIFY( provider != nullptr );

  // Open with COG reader
  GDALDatasetH dataset = GDALOpen( mCogFile.toUtf8().constData(), GA_ReadOnly );
  QVERIFY( dataset != nullptr );

  QgsCOGTileReader reader( dataset );
  QVERIFY( reader.isValid() );

  const auto tileInfo = reader.tileInfo( 0 );
  const int tilesToRead = std::min( 50, tileInfo.tilesX * tileInfo.tilesY );

  // Benchmark QgsCOGTileReader
  QElapsedTimer timerCOG;
  timerCOG.start();

  for ( int i = 0; i < tilesToRead; ++i )
  {
    const int tileX = i % tileInfo.tilesX;
    const int tileY = i / tileInfo.tilesX;

    QByteArray tileData;
    reader.readTile( 0, tileX, tileY, 1, tileData );
  }

  const qint64 cogTime = timerCOG.elapsed();

  // Benchmark standard QgsRasterBlock approach
  QElapsedTimer timerBlock;
  timerBlock.start();

  const QgsRectangle extent = reader.extent();
  const double tileWidthMu = extent.width() / tileInfo.tilesX;
  const double tileHeightMu = extent.height() / tileInfo.tilesY;

  for ( int i = 0; i < tilesToRead; ++i )
  {
    const int tileX = i % tileInfo.tilesX;
    const int tileY = i / tileInfo.tilesX;

    const double xMin = extent.xMinimum() + tileX * tileWidthMu;
    const double yMax = extent.yMaximum() - tileY * tileHeightMu;
    const QgsRectangle tileExtent(
      xMin, yMax - tileHeightMu,
      xMin + tileWidthMu, yMax
    );

    std::unique_ptr<QgsRasterBlock> block(
      provider->block( 1, tileExtent, tileInfo.width, tileInfo.height )
    );
  }

  const qint64 blockTime = timerBlock.elapsed();

  const double speedup = static_cast<double>( blockTime ) / cogTime;

  qDebug() << "\n=== Benchmark Results ===";
  qDebug() << "Tiles read:" << tilesToRead;
  qDebug() << "QgsCOGTileReader:  " << cogTime << "ms (" << (cogTime / static_cast<double>(tilesToRead)) << "ms/tile)";
  qDebug() << "QgsRasterBlock:    " << blockTime << "ms (" << (blockTime / static_cast<double>(tilesToRead)) << "ms/tile)";
  qDebug() << "Speedup:           " << speedup << "x";
  qDebug() << "========================\n";

  GDALClose( dataset );
}

QGSTEST_MAIN( TestQgsCOGTileReader )
#include "testqgscogtilereader.moc"
