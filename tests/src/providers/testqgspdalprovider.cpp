/***************************************************************************
     testqgspdalprovider.cpp
     --------------------------------------
    Date                 : November 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>

#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryDir>

//qgis includes...
#include "qgis.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgsmaplayer.h"
#include "qgspointcloudlayer.h"
#include "qgspdalindexingtask.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsgeometry.h"
#include "qgsprovidermetadata.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the PDAL provider
 */
class TestQgsPdalProvider : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsPdalProvider()
      : QgsTest( QStringLiteral( "PDAL Provider Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void filters();
    void encodeUri();
    void decodeUri();
    void layerTypesForUri();
    void preferredUri();
    void querySublayers();
    void brokenPath();
    void validLayer();
    void testTextReader();
    void testTextReaderWithOptions();
    void testCopcGenerationLasFile();
    void testCopcGenerationTextFile();

  private:
    QString mTestDataDir;
    QString mTempDir;
};

//runs before all tests
void TestQgsPdalProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt

  mTempDir = QDir::tempPath() + QDir::separator() + "pdal" + QDir::separator();
  QVERIFY( QDir().mkdir( mTempDir ) );
}

//runs after all tests
void TestQgsPdalProvider::cleanupTestCase()
{
  QDir( mTempDir ).removeRecursively();

  QgsApplication::exitQgis();
}

void TestQgsPdalProvider::filters()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  QVERIFY( metadata );

  const QString metadataFilters = metadata->filters( Qgis::FileFilterType::PointCloud );
  QVERIFY( metadataFilters.contains( "*.laz" ) );
  QVERIFY( metadataFilters.contains( "*.las" ) );
  QVERIFY( metadataFilters.contains( "*.LAZ" ) );
  QVERIFY( metadataFilters.contains( "*.LAS" ) );
  QVERIFY( metadataFilters.contains( "*.txt" ) );
  QVERIFY( metadataFilters.contains( "*.TXT" ) );

  QCOMPARE( metadata->filters( Qgis::FileFilterType::Vector ), QString() );

  const QString registryPointCloudFilters = QgsProviderRegistry::instance()->filePointCloudFilters();
  QVERIFY( registryPointCloudFilters.contains( "*.laz" ) );
  QVERIFY( registryPointCloudFilters.contains( "*.las" ) );
  QVERIFY( registryPointCloudFilters.contains( "*.LAZ" ) );
  QVERIFY( registryPointCloudFilters.contains( "*.LAS" ) );
  QVERIFY( metadataFilters.contains( "*.txt" ) );
  QVERIFY( metadataFilters.contains( "*.TXT" ) );
}

void TestQgsPdalProvider::encodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  QVERIFY( metadata );

  // only path
  QVariantMap parts;
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "/home/point_clouds/cloud.las" ) );
  QCOMPARE( metadata->encodeUri( parts ), QStringLiteral( "/home/point_clouds/cloud.las" ) );

  // uri with empty options
  QVariantMap partsWithEmptyOptions;
  partsWithEmptyOptions.insert( QStringLiteral( "path" ), QStringLiteral( "/home/point_clouds/cloud.txt" ) );
  partsWithEmptyOptions.insert( QStringLiteral( "openOptions" ), QStringList() );
  QCOMPARE( metadata->encodeUri( partsWithEmptyOptions ), QStringLiteral( "/home/point_clouds/cloud.txt" ) );

  // uri with options
  QVariantMap partsWithOptions;
  const QStringList options = { "separator=59", "skip=2" };
  partsWithOptions.insert( QStringLiteral( "path" ), QStringLiteral( "/home/point_clouds/cloud.txt" ) );
  partsWithOptions.insert( QStringLiteral( "openOptions" ), options );
  QCOMPARE( metadata->encodeUri( partsWithOptions ), QStringLiteral( "/home/point_clouds/cloud.txt|option:separator=59|option:skip=2" ) );
}

void TestQgsPdalProvider::decodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  QVERIFY( metadata );

  const QVariantMap parts = metadata->decodeUri( QStringLiteral( "/home/point_clouds/cloud.las" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/point_clouds/cloud.las" ) );
  QCOMPARE( parts.value( QStringLiteral( "openOptions" ) ).toStringList(), QStringList() );

  // uri with options
  const QVariantMap partsWithOptions = metadata->decodeUri( QStringLiteral( "/home/point_clouds/cloud.txt|option:separator=59|option:skip=2" ) );
  QCOMPARE( partsWithOptions.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/point_clouds/cloud.txt" ) );
  const QStringList expectedOptions = { "separator=59", "skip=2" };
  QCOMPARE( partsWithOptions.value( QStringLiteral( "openOptions" ) ).toStringList(), expectedOptions );
}

void TestQgsPdalProvider::layerTypesForUri()
{
  QgsProviderMetadata *pdalMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  QVERIFY( pdalMetadata->capabilities() & QgsProviderMetadata::LayerTypesForUri );

  QCOMPARE( pdalMetadata->validLayerTypesForUri( QStringLiteral( "/home/test/cloud.las" ) ), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );
  QCOMPARE( pdalMetadata->validLayerTypesForUri( QStringLiteral( "/home/test/cloud.shp" ) ), QList<Qgis::LayerType>() );
}

void TestQgsPdalProvider::preferredUri()
{
  QgsProviderMetadata *pdalMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  QVERIFY( pdalMetadata->capabilities() & QgsProviderMetadata::PriorityForUri );

  // test that pdal is the preferred provider for las/laz uris
  QList<QgsProviderRegistry::ProviderCandidateDetails> candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/cloud.las" ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), QStringLiteral( "pdal" ) );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/CLOUD.LAS" ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), QStringLiteral( "pdal" ) );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/cloud.laz" ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), QStringLiteral( "pdal" ) );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/CLOUD.LAZ" ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), QStringLiteral( "pdal" ) );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  QVERIFY( !QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( QStringLiteral( "/home/test/cloud.las" ), QStringLiteral( "pdal" ) ) );
  QVERIFY( QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( QStringLiteral( "/home/test/cloud.las" ), QStringLiteral( "ept" ) ) );
}

void TestQgsPdalProvider::querySublayers()
{
  // test querying sub layers for a pdal layer
  QgsProviderMetadata *pdalMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );

  // invalid uri
  QList<QgsProviderSublayerDetails> res = pdalMetadata->querySublayers( QString() );
  QVERIFY( res.empty() );

  // not a pdal layer
  res = pdalMetadata->querySublayers( QString( TEST_DATA_DIR ) + "/lines.shp" );
  QVERIFY( res.empty() );

  // valid pdal layer
  res = pdalMetadata->querySublayers( mTestDataDir + "/point_clouds/las/cloud.las" );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "cloud" ) );
  QCOMPARE( res.at( 0 ).uri(), mTestDataDir + "/point_clouds/las/cloud.las" );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "pdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::PointCloud );

  // make sure result is valid to load layer from
  const QgsProviderSublayerDetails::LayerOptions options { QgsCoordinateTransformContext() };
  std::unique_ptr<QgsPointCloudLayer> ml( qgis::down_cast<QgsPointCloudLayer *>( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( ml->isValid() );
}

void TestQgsPdalProvider::brokenPath()
{
  // test loading a bad layer URI
  auto layer = std::make_unique<QgsPointCloudLayer>(
    QStringLiteral( "not valid" ),
    QStringLiteral( "layer" ),
    QStringLiteral( "pdal" )
  );
  QVERIFY( !layer->isValid() );
}

void TestQgsPdalProvider::validLayer()
{
  QgsPointCloudLayer::LayerOptions options;
  options.skipIndexGeneration = true;

  auto layer = std::make_unique<QgsPointCloudLayer>(
    mTestDataDir + QStringLiteral( "point_clouds/las/cloud.las" ),
    QStringLiteral( "layer" ),
    QStringLiteral( "pdal" ),
    options
  );
  QVERIFY( layer->isValid() );

  QCOMPARE( layer->crs().authid(), QStringLiteral( "EPSG:28356" ) );
  QGSCOMPARENEAR( layer->extent().xMinimum(), 498062.0, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMinimum(), 7050992.84, 0.1 );
  QGSCOMPARENEAR( layer->extent().xMaximum(), 498067.39, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMaximum(), 7050997.04, 0.1 );
  QCOMPARE( layer->dataProvider()->polygonBounds().asWkt( 0 ), QStringLiteral( "Polygon ((498062 7050993, 498067 7050993, 498067 7050997, 498062 7050997, 498062 7050993))" ) );

  QCOMPARE( layer->dataProvider()->pointCount(), 253 );
  QCOMPARE( layer->pointCount(), 253 );
}

void TestQgsPdalProvider::testTextReader()
{
  QgsPointCloudLayer::LayerOptions options;
  options.skipIndexGeneration = true;

  auto layer = std::make_unique<QgsPointCloudLayer>(
    mTestDataDir + QStringLiteral( "point_clouds/text/cloud.txt" ),
    QStringLiteral( "layer" ),
    QStringLiteral( "pdal" ),
    options
  );
  QVERIFY( layer->isValid() );

  QCOMPARE( layer->crs().authid(), "" );
  QGSCOMPARENEAR( layer->extent().xMinimum(), 473850.0, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMinimum(), 6374925.0, 0.1 );
  QGSCOMPARENEAR( layer->extent().xMaximum(), 488625.0, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMaximum(), 6375000.0, 0.1 );
  QCOMPARE( layer->dataProvider()->polygonBounds().asWkt( 0 ), QStringLiteral( "Polygon ((473850 6374925, 488625 6374925, 488625 6375000, 473850 6375000, 473850 6374925))" ) );

  QCOMPARE( layer->dataProvider()->pointCount(), 320 );
  QCOMPARE( layer->pointCount(), 320 );
}

void TestQgsPdalProvider::testTextReaderWithOptions()
{
  QgsPointCloudLayer::LayerOptions layerOptions;
  layerOptions.skipIndexGeneration = true;

  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  QVERIFY( metadata );

  // Test option skip
  QVariantMap uriPartsSkip;
  const QStringList openOptionsSkip = { "skip=2" };
  const QString pathSkip = mTestDataDir + QStringLiteral( "point_clouds/text/cloud_skip.txt" );
  const QString pathSkipCopy = mTempDir + "cloud_skip.txt";
  QVERIFY( QFile::copy( pathSkip, pathSkipCopy ) );
  uriPartsSkip.insert( QStringLiteral( "path" ), pathSkipCopy );
  uriPartsSkip.insert( QStringLiteral( "openOptions" ), openOptionsSkip );
  const QString uriSkip = metadata->encodeUri( uriPartsSkip );

  auto layerSkip = std::make_unique<QgsPointCloudLayer>(
    uriSkip,
    QStringLiteral( "layer" ),
    QStringLiteral( "pdal" ),
    layerOptions
  );
  QVERIFY( layerSkip->isValid() );
  QCOMPARE( layerSkip->crs().authid(), "" );
  QGSCOMPARENEAR( layerSkip->extent().xMinimum(), 473850.0, 0.1 );
  QGSCOMPARENEAR( layerSkip->extent().yMinimum(), 6375000.0, 0.1 );
  QGSCOMPARENEAR( layerSkip->extent().xMaximum(), 476550.0, 0.1 );
  QGSCOMPARENEAR( layerSkip->extent().yMaximum(), 6375000.0, 0.1 );
  QCOMPARE( layerSkip->dataProvider()->polygonBounds().asWkt( 0 ), QStringLiteral( "Polygon ((473850 6375000, 476550 6375000, 476550 6375000, 473850 6375000, 473850 6375000))" ) );

  QCOMPARE( layerSkip->dataProvider()->pointCount(), 37 );
  QCOMPARE( layerSkip->pointCount(), 37 );
}

void TestQgsPdalProvider::testCopcGenerationLasFile()
{
  const QTemporaryDir dir;
  QString outputPath = dir.path() + QDir::separator() + "cloud.copc.laz";
  QVERIFY( dir.isValid() );
  QgsPdalIndexingTask task( mTestDataDir + QStringLiteral( "point_clouds/las/cloud.las" ), outputPath );
  QVERIFY( task.run() );
  const QFileInfo fi( outputPath );
  QVERIFY( fi.exists() );
}

void TestQgsPdalProvider::testCopcGenerationTextFile()
{
  const QTemporaryDir dir;
  QString outputPath = dir.path() + QDir::separator() + "cloud.copc.txt";
  QVERIFY( dir.isValid() );
  QgsPdalIndexingTask task( mTestDataDir + QStringLiteral( "point_clouds/text/cloud.txt" ), outputPath );
  QVERIFY( task.run() );
  const QFileInfo fi( outputPath );
  QVERIFY( fi.exists() );
}

QGSTEST_MAIN( TestQgsPdalProvider )
#include "testqgspdalprovider.moc"
