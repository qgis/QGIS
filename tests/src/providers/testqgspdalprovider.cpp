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

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QStringList>
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
      : QgsTest( u"PDAL Provider Tests"_s ) {}

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
    void testCopcGenerationLasFile();
    void testCopcGenerationTextFile();

  private:
    QString mTestDataDir;
};

//runs before all tests
void TestQgsPdalProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
}

//runs after all tests
void TestQgsPdalProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsPdalProvider::filters()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"pdal"_s );
  QVERIFY( metadata );

  const QString metadataFilters = metadata->filters( Qgis::FileFilterType::PointCloud );
  QVERIFY( metadataFilters.contains( "*.laz" ) );
  QVERIFY( metadataFilters.contains( "*.las" ) );
  QVERIFY( metadataFilters.contains( "*.LAZ" ) );
  QVERIFY( metadataFilters.contains( "*.LAS" ) );

  QCOMPARE( metadata->filters( Qgis::FileFilterType::Vector ), QString() );

  const QString registryPointCloudFilters = QgsProviderRegistry::instance()->filePointCloudFilters();
  QVERIFY( registryPointCloudFilters.contains( "*.laz" ) );
  QVERIFY( registryPointCloudFilters.contains( "*.las" ) );
  QVERIFY( registryPointCloudFilters.contains( "*.LAZ" ) );
  QVERIFY( registryPointCloudFilters.contains( "*.LAS" ) );
}

void TestQgsPdalProvider::encodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"pdal"_s );
  QVERIFY( metadata );

  QVariantMap parts;
  parts.insert( u"path"_s, u"/home/point_clouds/cloud.las"_s );
  QCOMPARE( metadata->encodeUri( parts ), u"/home/point_clouds/cloud.las"_s );
}

void TestQgsPdalProvider::decodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"pdal"_s );
  QVERIFY( metadata );

  const QVariantMap parts = metadata->decodeUri( u"/home/point_clouds/cloud.las"_s );
  QCOMPARE( parts.value( u"path"_s ).toString(), u"/home/point_clouds/cloud.las"_s );
}

void TestQgsPdalProvider::layerTypesForUri()
{
  QgsProviderMetadata *pdalMetadata = QgsProviderRegistry::instance()->providerMetadata( u"pdal"_s );
  QVERIFY( pdalMetadata->capabilities() & QgsProviderMetadata::LayerTypesForUri );

  QCOMPARE( pdalMetadata->validLayerTypesForUri( u"/home/test/cloud.las"_s ), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );
  QCOMPARE( pdalMetadata->validLayerTypesForUri( u"/home/test/cloud.shp"_s ), QList<Qgis::LayerType>() );
}

void TestQgsPdalProvider::preferredUri()
{
  QgsProviderMetadata *pdalMetadata = QgsProviderRegistry::instance()->providerMetadata( u"pdal"_s );
  QVERIFY( pdalMetadata->capabilities() & QgsProviderMetadata::PriorityForUri );

  // test that pdal is the preferred provider for las/laz uris
  QList<QgsProviderRegistry::ProviderCandidateDetails> candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( u"/home/test/cloud.las"_s );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), u"pdal"_s );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( u"/home/test/CLOUD.LAS"_s );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), u"pdal"_s );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( u"/home/test/cloud.laz"_s );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), u"pdal"_s );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( u"/home/test/CLOUD.LAZ"_s );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), u"pdal"_s );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  QVERIFY( !QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( u"/home/test/cloud.las"_s, u"pdal"_s ) );
  QVERIFY( QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( u"/home/test/cloud.las"_s, u"ept"_s ) );
}

void TestQgsPdalProvider::querySublayers()
{
  // test querying sub layers for a pdal layer
  QgsProviderMetadata *pdalMetadata = QgsProviderRegistry::instance()->providerMetadata( u"pdal"_s );

  // invalid uri
  QList<QgsProviderSublayerDetails> res = pdalMetadata->querySublayers( QString() );
  QVERIFY( res.empty() );

  // not a pdal layer
  res = pdalMetadata->querySublayers( QString( TEST_DATA_DIR ) + "/lines.shp" );
  QVERIFY( res.empty() );

  // valid pdal layer
  res = pdalMetadata->querySublayers( mTestDataDir + "/point_clouds/las/cloud.las" );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), u"cloud"_s );
  QCOMPARE( res.at( 0 ).uri(), mTestDataDir + "/point_clouds/las/cloud.las" );
  QCOMPARE( res.at( 0 ).providerKey(), u"pdal"_s );
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
    u"not valid"_s,
    u"layer"_s,
    u"pdal"_s
  );
  QVERIFY( !layer->isValid() );
}

void TestQgsPdalProvider::validLayer()
{
  QgsPointCloudLayer::LayerOptions options;
  options.skipIndexGeneration = true;

  auto layer = std::make_unique<QgsPointCloudLayer>(
    mTestDataDir + u"point_clouds/las/cloud.las"_s,
    u"layer"_s,
    u"pdal"_s,
    options
  );
  QVERIFY( layer->isValid() );

  QCOMPARE( layer->crs().authid(), u"EPSG:28356"_s );
  QGSCOMPARENEAR( layer->extent().xMinimum(), 498062.0, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMinimum(), 7050992.84, 0.1 );
  QGSCOMPARENEAR( layer->extent().xMaximum(), 498067.39, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMaximum(), 7050997.04, 0.1 );
  QCOMPARE( layer->dataProvider()->polygonBounds().asWkt( 0 ), u"Polygon ((498062 7050993, 498067 7050993, 498067 7050997, 498062 7050997, 498062 7050993))"_s );

  QCOMPARE( layer->dataProvider()->pointCount(), 253 );
  QCOMPARE( layer->pointCount(), 253 );
}

void TestQgsPdalProvider::testTextReader()
{
  QgsPointCloudLayer::LayerOptions options;
  options.skipIndexGeneration = true;

  auto layer = std::make_unique<QgsPointCloudLayer>(
    mTestDataDir + u"point_clouds/text/cloud.txt"_s,
    u"layer"_s,
    u"pdal"_s,
    options
  );
  QVERIFY( layer->isValid() );

  QCOMPARE( layer->crs().authid(), "" );
  QGSCOMPARENEAR( layer->extent().xMinimum(), 473850.0, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMinimum(), 6374925.0, 0.1 );
  QGSCOMPARENEAR( layer->extent().xMaximum(), 488625.0, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMaximum(), 6375000.0, 0.1 );
  QCOMPARE( layer->dataProvider()->polygonBounds().asWkt( 0 ), u"Polygon ((473850 6374925, 488625 6374925, 488625 6375000, 473850 6375000, 473850 6374925))"_s );

  QCOMPARE( layer->dataProvider()->pointCount(), 320 );
  QCOMPARE( layer->pointCount(), 320 );
}

void TestQgsPdalProvider::testCopcGenerationLasFile()
{
  const QTemporaryDir dir;
  QString outputPath = dir.path() + QDir::separator() + "cloud.copc.laz";
  QVERIFY( dir.isValid() );
  QgsPdalIndexingTask task( mTestDataDir + u"point_clouds/las/cloud.las"_s, outputPath );
  QVERIFY( task.run() );
  const QFileInfo fi( outputPath );
  QVERIFY( fi.exists() );
}

void TestQgsPdalProvider::testCopcGenerationTextFile()
{
  const QTemporaryDir dir;
  QString outputPath = dir.path() + QDir::separator() + "cloud.copc.txt";
  QVERIFY( dir.isValid() );
  QgsPdalIndexingTask task( mTestDataDir + u"point_clouds/text/cloud.txt"_s, outputPath );
  QVERIFY( task.run() );
  const QFileInfo fi( outputPath );
  QVERIFY( fi.exists() );
}

QGSTEST_MAIN( TestQgsPdalProvider )
#include "testqgspdalprovider.moc"
