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
#include "qgspdalprovider.h"
#include "qgsmaplayer.h"
#include "qgspointcloudlayer.h"
#include "qgspdalindexingtask.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsgeometry.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the PDAL provider
 */
class TestQgsPdalProvider : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.

    void filters();
    void encodeUri();
    void decodeUri();
    void layerTypesForUri();
    void preferredUri();
    void querySublayers();
    void brokenPath();
    void validLayer();
    void testEptGeneration();
    void testCopcGeneration();

  private:
    QString mTestDataDir;
    QString mReport;
};

//runs before all tests
void TestQgsPdalProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  mReport = QStringLiteral( "<h1>PDAL Provider Tests</h1>\n" );
}

//runs after all tests
void TestQgsPdalProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsPdalProvider::filters()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  QVERIFY( metadata );

  const QString metadataFilters = metadata->filters( QgsProviderMetadata::FilterType::FilterPointCloud );
  QVERIFY( metadataFilters.contains( "*.laz" ) );
  QVERIFY( metadataFilters.contains( "*.las" ) );
  QVERIFY( metadataFilters.contains( "*.LAZ" ) );
  QVERIFY( metadataFilters.contains( "*.LAS" ) );

  QCOMPARE( metadata->filters( QgsProviderMetadata::FilterType::FilterVector ), QString() );

  const QString registryPointCloudFilters = QgsProviderRegistry::instance()->filePointCloudFilters();
  QVERIFY( registryPointCloudFilters.contains( "*.laz" ) );
  QVERIFY( registryPointCloudFilters.contains( "*.las" ) );
  QVERIFY( registryPointCloudFilters.contains( "*.LAZ" ) );
  QVERIFY( registryPointCloudFilters.contains( "*.LAS" ) );
}

void TestQgsPdalProvider::encodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  QVERIFY( metadata );

  QVariantMap parts;
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "/home/point_clouds/cloud.las" ) );
  QCOMPARE( metadata->encodeUri( parts ), QStringLiteral( "/home/point_clouds/cloud.las" ) );
}

void TestQgsPdalProvider::decodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  QVERIFY( metadata );

  const QVariantMap parts = metadata->decodeUri( QStringLiteral( "/home/point_clouds/cloud.las" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/point_clouds/cloud.las" ) );
}

void TestQgsPdalProvider::layerTypesForUri()
{
  QgsProviderMetadata *pdalMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  QVERIFY( pdalMetadata->capabilities() & QgsProviderMetadata::LayerTypesForUri );

  QCOMPARE( pdalMetadata->validLayerTypesForUri( QStringLiteral( "/home/test/cloud.las" ) ), QList< QgsMapLayerType >() << QgsMapLayerType::PointCloudLayer );
  QCOMPARE( pdalMetadata->validLayerTypesForUri( QStringLiteral( "/home/test/cloud.shp" ) ), QList< QgsMapLayerType >() );
}

void TestQgsPdalProvider::preferredUri()
{
  QgsProviderMetadata *pdalMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  QVERIFY( pdalMetadata->capabilities() & QgsProviderMetadata::PriorityForUri );

  // test that pdal is the preferred provider for las/laz uris
  QList<QgsProviderRegistry::ProviderCandidateDetails> candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/cloud.las" ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), QStringLiteral( "pdal" ) );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList< QgsMapLayerType >() << QgsMapLayerType::PointCloudLayer );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/CLOUD.LAS" ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), QStringLiteral( "pdal" ) );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList< QgsMapLayerType >() << QgsMapLayerType::PointCloudLayer );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/cloud.laz" ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), QStringLiteral( "pdal" ) );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList< QgsMapLayerType >() << QgsMapLayerType::PointCloudLayer );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/CLOUD.LAZ" ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), QStringLiteral( "pdal" ) );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList< QgsMapLayerType >() << QgsMapLayerType::PointCloudLayer );

  QVERIFY( !QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( QStringLiteral( "/home/test/cloud.las" ), QStringLiteral( "pdal" ) ) );
  QVERIFY( QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( QStringLiteral( "/home/test/cloud.las" ), QStringLiteral( "ept" ) ) );
}

void TestQgsPdalProvider::querySublayers()
{
  // test querying sub layers for a pdal layer
  QgsProviderMetadata *pdalMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );

  // invalid uri
  QList< QgsProviderSublayerDetails >res = pdalMetadata->querySublayers( QString() );
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
  QCOMPARE( res.at( 0 ).type(), QgsMapLayerType::PointCloudLayer );

  // make sure result is valid to load layer from
  const QgsProviderSublayerDetails::LayerOptions options{ QgsCoordinateTransformContext() };
  std::unique_ptr< QgsPointCloudLayer > ml( qgis::down_cast< QgsPointCloudLayer * >( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( ml->isValid() );
}

void TestQgsPdalProvider::brokenPath()
{
  // test loading a bad layer URI
  std::unique_ptr< QgsPointCloudLayer > layer = std::make_unique< QgsPointCloudLayer >(
        QStringLiteral( "not valid" ),
        QStringLiteral( "layer" ),
        QStringLiteral( "pdal" ) );
  QVERIFY( !layer->isValid() );
}

void TestQgsPdalProvider::validLayer()
{
  QgsPointCloudLayer::LayerOptions options;
  options.skipIndexGeneration = true;

  std::unique_ptr< QgsPointCloudLayer > layer = std::make_unique< QgsPointCloudLayer >(
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

void TestQgsPdalProvider::testEptGeneration()
{
  const QTemporaryDir dir;
  QVERIFY( dir.isValid() );
  QgsPdalIndexingTask task( mTestDataDir + QStringLiteral( "point_clouds/las/cloud.las" ), dir.path(), QgsPdalIndexingTask::OutputFormat::Ept );
  QVERIFY( task.run() );
  const QFileInfo fi( dir.path() + "/ept.json" );
  QVERIFY( fi.exists() );
}

void TestQgsPdalProvider::testCopcGeneration()
{
  const QTemporaryDir dir;
  QString outputPath = dir.path() + QDir::separator() + "cloud.copc.laz";
  QVERIFY( dir.isValid() );
  QgsPdalIndexingTask task( mTestDataDir + QStringLiteral( "point_clouds/las/cloud.las" ), outputPath,  QgsPdalIndexingTask::OutputFormat::Copc );
  QVERIFY( task.run() );
  const QFileInfo fi( outputPath );
  QVERIFY( fi.exists() );
}

QGSTEST_MAIN( TestQgsPdalProvider )
#include "testqgspdalprovider.moc"
