/***************************************************************************
     testqgsvirtualpointcloudprovider.cpp
     ------------------------------------
    Date                 : April 2023
    Copyright            : (C) 2023 by Stefanos Natsis
    Email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <fstream>
#include <limits>

#include "qgstest.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QQueue>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QVector>

//qgis includes...
#include "qgis.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsgeometry.h"
#include "qgspointcloudstatscalculator.h"
#include "qgspointcloudstatistics.h"
#include "qgsfeedback.h"
#include "qgspointcloudsubindex.h"
#include "qgsprovidermetadata.h"
#include "qgsrendercontext.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the VirtualPointCloud provider
 */
class TestQgsVirtualPointCloudProvider : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsVirtualPointCloudProvider()
      : QgsTest( u"Virtual Point Cloud Provider Tests"_s ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void filters();
    void encodeUri();
    void decodeUri();
    void absoluteRelativeUri();
    void preferredUri();
    void layerTypesForUri();
    void uriIsBlocklisted();
    void querySublayers();
    void brokenPath();
    void validLayer();
    void attributes();
    void testLazyLoading();

  private:
    QString mTestDataDir;
};

//runs before all tests
void TestQgsVirtualPointCloudProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
}

//runs after all tests
void TestQgsVirtualPointCloudProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsVirtualPointCloudProvider::filters()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"vpc"_s );
  QVERIFY( metadata );

  QCOMPARE( metadata->filters( Qgis::FileFilterType::PointCloud ), u"Virtual Point Clouds (*.vpc *.VPC)"_s );
  QCOMPARE( metadata->filters( Qgis::FileFilterType::Vector ), QString() );

  const QString registryPointCloudFilters = QgsProviderRegistry::instance()->filePointCloudFilters();
  QVERIFY( registryPointCloudFilters.contains( "(*.vpc *.VPC)" ) );
}

void TestQgsVirtualPointCloudProvider::encodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"vpc"_s );
  QVERIFY( metadata );

  QVariantMap parts;
  parts.insert( u"path"_s, u"/home/point_clouds/dataset.vpc"_s );
  QCOMPARE( metadata->encodeUri( parts ), u"/home/point_clouds/dataset.vpc"_s );
}

void TestQgsVirtualPointCloudProvider::decodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"vpc"_s );
  QVERIFY( metadata );

  const QVariantMap parts = metadata->decodeUri( u"/home/point_clouds/dataset.vpc"_s );
  QCOMPARE( parts.value( u"path"_s ).toString(), u"/home/point_clouds/dataset.vpc"_s );
}

void TestQgsVirtualPointCloudProvider::absoluteRelativeUri()
{
  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( QStringLiteral( TEST_DATA_DIR ) + u"/project.qgs"_s ) );

  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( "vpc" );
  QVERIFY( metadata );

  QString absoluteUri = QStringLiteral( TEST_DATA_DIR ) + u"/point_clouds/virtual/tiles.vpc"_s;
  QString relativeUri = u"./point_clouds/virtual/tiles.vpc"_s;
  QCOMPARE( metadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( metadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );
}

void TestQgsVirtualPointCloudProvider::preferredUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"vpc"_s );
  QVERIFY( metadata->capabilities() & QgsProviderMetadata::PriorityForUri );

  // test that Virtual is the preferred provider for .vpc uris
  QList<QgsProviderRegistry::ProviderCandidateDetails> candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( u"/home/test/dataset.vpc"_s );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), u"vpc"_s );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( u"/home/test/dataset.VPC"_s );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), u"vpc"_s );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  QVERIFY( !QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( u"/home/test/dataset.vpc"_s, u"vpc"_s ) );
  QVERIFY( QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( u"/home/test/dataset.vpc"_s, u"ogr"_s ) );
}

void TestQgsVirtualPointCloudProvider::layerTypesForUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"vpc"_s );
  QVERIFY( metadata->capabilities() & QgsProviderMetadata::LayerTypesForUri );

  QCOMPARE( metadata->validLayerTypesForUri( u"/home/test/cloud.vpc"_s ), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );
  QCOMPARE( metadata->validLayerTypesForUri( u"/home/test/cloud.copc.laz"_s ), QList<Qgis::LayerType>() );
  QCOMPARE( metadata->validLayerTypesForUri( u"/home/test/ept.json"_s ), QList<Qgis::LayerType>() );
}

void TestQgsVirtualPointCloudProvider::uriIsBlocklisted()
{
  QVERIFY( !QgsProviderRegistry::instance()->uriIsBlocklisted( u"/home/test/dataset.vpc"_s ) );
}

void TestQgsVirtualPointCloudProvider::querySublayers()
{
  // test querying sub layers for a ept layer
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"vpc"_s );

  // invalid uri
  QList<QgsProviderSublayerDetails> res = metadata->querySublayers( QString() );
  QVERIFY( res.empty() );

  // not a VPC layer
  res = metadata->querySublayers( QString( TEST_DATA_DIR ) + "/lines.shp" );
  QVERIFY( res.empty() );

  // valid VPC layer
  res = metadata->querySublayers( mTestDataDir + "/point_clouds/virtual/tiles.vpc" );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), u"tiles"_s );
  QCOMPARE( res.at( 0 ).uri(), mTestDataDir + "/point_clouds/virtual/tiles.vpc" );
  QCOMPARE( res.at( 0 ).providerKey(), u"vpc"_s );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::PointCloud );

  // make sure result is valid to load layer from
  QgsProviderSublayerDetails::LayerOptions options { QgsCoordinateTransformContext() };
  std::unique_ptr<QgsPointCloudLayer> ml( qgis::down_cast<QgsPointCloudLayer *>( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( ml->isValid() );
}

void TestQgsVirtualPointCloudProvider::brokenPath()
{
  // test loading a bad layer URI
  auto layer = std::make_unique<QgsPointCloudLayer>( u"not valid"_s, u"layer"_s, u"vpc"_s );
  QVERIFY( !layer->isValid() );
}

void TestQgsVirtualPointCloudProvider::validLayer()
{
  auto layer = std::make_unique<QgsPointCloudLayer>( mTestDataDir + u"point_clouds/virtual/tiles.vpc"_s, u"layer"_s, u"vpc"_s );
  QVERIFY( layer->isValid() );

  QCOMPARE( layer->crs().authid(), u"EPSG:5514"_s );
  QGSCOMPARENEAR( layer->extent().xMinimum(), -498328.32, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMinimum(), -1205552.89, 0.1 );
  QGSCOMPARENEAR( layer->extent().xMaximum(), -497853.64, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMaximum(), -1205189.02, 0.1 );
  QCOMPARE( layer->dataProvider()->polygonBounds().asMultiPolygon().size(), 18 );
  QCOMPARE( layer->dataProvider()->pointCount(), 3365334 );
  QCOMPARE( layer->pointCount(), 3365334 );

  QVERIFY( !layer->dataProvider()->index() );
}

void TestQgsVirtualPointCloudProvider::attributes()
{
  auto layer = std::make_unique<QgsPointCloudLayer>( mTestDataDir + u"point_clouds/virtual/tiles.vpc"_s, u"layer"_s, u"vpc"_s );
  QVERIFY( layer->isValid() );

  const QgsPointCloudAttributeCollection attributes = layer->attributes();
  QCOMPARE( attributes.count(), 16 );
  QCOMPARE( attributes.at( 0 ).name(), u"X"_s );
  QCOMPARE( attributes.at( 0 ).type(), QgsPointCloudAttribute::Int32 );
  QCOMPARE( attributes.at( 1 ).name(), u"Y"_s );
  QCOMPARE( attributes.at( 1 ).type(), QgsPointCloudAttribute::Int32 );
  QCOMPARE( attributes.at( 2 ).name(), u"Z"_s );
  QCOMPARE( attributes.at( 2 ).type(), QgsPointCloudAttribute::Int32 );
  QCOMPARE( attributes.at( 3 ).name(), u"Intensity"_s );
  QCOMPARE( attributes.at( 3 ).type(), QgsPointCloudAttribute::UShort );
  QCOMPARE( attributes.at( 4 ).name(), u"ReturnNumber"_s );
  QCOMPARE( attributes.at( 4 ).type(), QgsPointCloudAttribute::Char );
  QCOMPARE( attributes.at( 5 ).name(), u"NumberOfReturns"_s );
  QCOMPARE( attributes.at( 5 ).type(), QgsPointCloudAttribute::Char );
  QCOMPARE( attributes.at( 6 ).name(), u"ScanDirectionFlag"_s );
  QCOMPARE( attributes.at( 6 ).type(), QgsPointCloudAttribute::Char );
  QCOMPARE( attributes.at( 7 ).name(), u"EdgeOfFlightLine"_s );
  QCOMPARE( attributes.at( 7 ).type(), QgsPointCloudAttribute::Char );
  QCOMPARE( attributes.at( 8 ).name(), u"Classification"_s );
  QCOMPARE( attributes.at( 8 ).type(), QgsPointCloudAttribute::UChar );
  QCOMPARE( attributes.at( 9 ).name(), u"ScanAngleRank"_s );
  QCOMPARE( attributes.at( 9 ).type(), QgsPointCloudAttribute::Float );
  QCOMPARE( attributes.at( 10 ).name(), u"UserData"_s );
  QCOMPARE( attributes.at( 10 ).type(), QgsPointCloudAttribute::Char );
  QCOMPARE( attributes.at( 11 ).name(), u"PointSourceId"_s );
  QCOMPARE( attributes.at( 11 ).type(), QgsPointCloudAttribute::UShort );
  QCOMPARE( attributes.at( 12 ).name(), u"GpsTime"_s );
  QCOMPARE( attributes.at( 12 ).type(), QgsPointCloudAttribute::Double );
  QCOMPARE( attributes.at( 13 ).name(), u"Red"_s );
  QCOMPARE( attributes.at( 13 ).type(), QgsPointCloudAttribute::UShort );
  QCOMPARE( attributes.at( 14 ).name(), u"Green"_s );
  QCOMPARE( attributes.at( 14 ).type(), QgsPointCloudAttribute::UShort );
  QCOMPARE( attributes.at( 15 ).name(), u"Blue"_s );
  QCOMPARE( attributes.at( 15 ).type(), QgsPointCloudAttribute::UShort );
}

void TestQgsVirtualPointCloudProvider::testLazyLoading()
{
  auto layer = std::make_unique<QgsPointCloudLayer>( mTestDataDir + u"point_clouds/virtual/tiles.vpc"_s, u"layer"_s, u"vpc"_s );
  QVERIFY( layer->isValid() );

  QgsPointCloudIndex index = layer->dataProvider()->index();
  QVERIFY( !index );

  QVector<QgsPointCloudSubIndex> subIndexes = layer->dataProvider()->subIndexes();

  QCOMPARE( subIndexes.size(), 18 );
  int loadedIndexes = 0;
  for ( const auto &si : subIndexes )
    if ( si.index() )
      ++loadedIndexes;

  QCOMPARE( loadedIndexes, 0 );


  QgsRenderContext ctx;
  ctx.setMapExtent( QgsRectangle( -498160, -1205380, -498090, -1205330 ) );
  layer->loadIndexesForRenderContext( ctx );
  subIndexes = layer->dataProvider()->subIndexes();
  for ( const auto &si : subIndexes )
    if ( si.index() )
      ++loadedIndexes;

  QCOMPARE( loadedIndexes, 4 );
}

QGSTEST_MAIN( TestQgsVirtualPointCloudProvider )
#include "testqgsvirtualpointcloudprovider.moc"
