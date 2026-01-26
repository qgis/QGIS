/***************************************************************************
     testqgscopcprovider.cpp
     --------------------------------------
    Date                 : March 2022
    Copyright            : (C) 2022 by Belgacem Nedjima
    Email                : belgacem dot nedjima at gmail dot com
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
#include "qgslazinfo.h"
#include "qgspointcloudstatscalculator.h"
#include "qgspointcloudstatistics.h"
#include "qgsfeedback.h"
#include "qgsrangerequestcache.h"
#include "qgscopcpointcloudindex.h"
#include "qgsprovidermetadata.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the COPC provider
 */
class TestQgsCopcProvider : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsCopcProvider()
      : QgsTest( u"COPC Provider Tests"_s ) {}

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
    void testLazInfo();
    void validLayer();
    void validLayerWithCopcHierarchy();
    void attributes();
    void calculateZRange();
    void testIdentify_data();
    void testIdentify();
    void testExtraBytesAttributesExtraction();
    void testExtraBytesAttributesValues();
    void testClassFlagsValues();
    void testPointCloudIndex();
    void testStatsCalculator();
    void testSaveLoadStats();
    void testPointCloudRequest();
    void testPointCloudRequestIgnoreFilter();
    void testQgsRangeRequestCache();
};

//runs before all tests
void TestQgsCopcProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
}

//runs after all tests
void TestQgsCopcProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsCopcProvider::filters()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"copc"_s );
  QVERIFY( metadata );

  QCOMPARE( metadata->filters( Qgis::FileFilterType::PointCloud ), u"COPC Point Clouds (*.copc.laz *.COPC.LAZ)"_s );
  QCOMPARE( metadata->filters( Qgis::FileFilterType::Vector ), QString() );

  const QString registryPointCloudFilters = QgsProviderRegistry::instance()->filePointCloudFilters();
  QVERIFY( registryPointCloudFilters.contains( "(*.copc.laz *.COPC.LAZ)" ) );
}

void TestQgsCopcProvider::encodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"copc"_s );
  QVERIFY( metadata );

  QVariantMap parts;
  parts.insert( u"path"_s, u"/home/point_clouds/dataset.copc.laz"_s );
  QCOMPARE( metadata->encodeUri( parts ), u"/home/point_clouds/dataset.copc.laz"_s );

  parts.insert( u"authcfg"_s, u"abc1234"_s );
  QCOMPARE( metadata->encodeUri( parts ), u"/home/point_clouds/dataset.copc.laz authcfg='abc1234'"_s );
}

void TestQgsCopcProvider::decodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"copc"_s );
  QVERIFY( metadata );

  QVariantMap parts = metadata->decodeUri( u"/home/point_clouds/dataset.copc.laz"_s );
  QCOMPARE( parts.value( u"path"_s ).toString(), u"/home/point_clouds/dataset.copc.laz"_s );
  QCOMPARE( parts.value( u"file-name"_s ).toString(), u"dataset.copc.laz"_s );
  QVERIFY( parts.value( u"authcfg"_s ).toString().isEmpty() );

  parts = metadata->decodeUri( u"/home/point_clouds/dataset.copc.laz authcfg='abc1234'"_s );
  QCOMPARE( parts.value( u"path"_s ).toString(), u"/home/point_clouds/dataset.copc.laz"_s );
  QCOMPARE( parts.value( u"authcfg"_s ).toString(), u"abc1234"_s );
}

void TestQgsCopcProvider::absoluteRelativeUri()
{
  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( testDataPath( u"/project.qgs"_s ) ) );

  QgsProviderMetadata *copcMetadata = QgsProviderRegistry::instance()->providerMetadata( "copc" );
  QVERIFY( copcMetadata );

  QString absoluteUri = testDataPath( u"/point_clouds/copc/rgb.copc.laz"_s );
  QString relativeUri = u"./point_clouds/copc/rgb.copc.laz"_s;
  QCOMPARE( copcMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( copcMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );
}

void TestQgsCopcProvider::preferredUri()
{
  QgsProviderMetadata *copcMetadata = QgsProviderRegistry::instance()->providerMetadata( u"copc"_s );
  QVERIFY( copcMetadata->capabilities() & QgsProviderMetadata::PriorityForUri );

  // test that COPC is the preferred provider for .copc.laz uris
  QList<QgsProviderRegistry::ProviderCandidateDetails> candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( u"/home/test/dataset.copc.laz"_s );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), u"copc"_s );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( u"/home/test/dataset.COPC.LAZ"_s );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), u"copc"_s );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  QVERIFY( !QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( u"/home/test/dataset.copc.laz"_s, u"copc"_s ) );
  QVERIFY( QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( u"/home/test/dataset.copc.laz"_s, u"ogr"_s ) );
}

void TestQgsCopcProvider::layerTypesForUri()
{
  QgsProviderMetadata *copcMetadata = QgsProviderRegistry::instance()->providerMetadata( u"copc"_s );
  QVERIFY( copcMetadata->capabilities() & QgsProviderMetadata::LayerTypesForUri );

  QCOMPARE( copcMetadata->validLayerTypesForUri( u"/home/test/cloud.copc.laz"_s ), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );
  QCOMPARE( copcMetadata->validLayerTypesForUri( u"/home/test/cloud.copc.laz authcfg='abc1234'"_s ), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );
  QCOMPARE( copcMetadata->validLayerTypesForUri( u"/home/test/ept.json"_s ), QList<Qgis::LayerType>() );
}

void TestQgsCopcProvider::uriIsBlocklisted()
{
  QVERIFY( !QgsProviderRegistry::instance()->uriIsBlocklisted( u"/home/test/ept.json"_s ) );
  QVERIFY( !QgsProviderRegistry::instance()->uriIsBlocklisted( u"/home/test/dataset.copc.laz"_s ) );
}

void TestQgsCopcProvider::querySublayers()
{
  // test querying sub layers for a ept layer
  QgsProviderMetadata *copcMetadata = QgsProviderRegistry::instance()->providerMetadata( u"copc"_s );

  // invalid uri
  QList<QgsProviderSublayerDetails> res = copcMetadata->querySublayers( QString() );
  QVERIFY( res.empty() );

  // not a copc layer
  res = copcMetadata->querySublayers( testDataPath( "/lines.shp" ) );
  QVERIFY( res.empty() );

  // valid copc layer
  const QString dataPath = copyTestData( u"/point_clouds/copc/sunshine-coast.copc.laz"_s );

  res = copcMetadata->querySublayers( dataPath );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), u"sunshine-coast.copc"_s );
  QCOMPARE( res.at( 0 ).uri(), dataPath );
  QCOMPARE( res.at( 0 ).providerKey(), u"copc"_s );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::PointCloud );

  // make sure result is valid to load layer from
  QgsProviderSublayerDetails::LayerOptions options { QgsCoordinateTransformContext() };
  std::unique_ptr<QgsPointCloudLayer> ml( qgis::down_cast<QgsPointCloudLayer *>( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( ml->isValid() );
}

void TestQgsCopcProvider::brokenPath()
{
  // test loading a bad layer URI
  auto layer = std::make_unique<QgsPointCloudLayer>( u"not valid"_s, u"layer"_s, u"copc"_s );
  QVERIFY( !layer->isValid() );
}

void TestQgsCopcProvider::testLazInfo()
{
  const QString dataPath = copyTestData( u"point_clouds/copc/lone-star.copc.laz"_s );

  std::ifstream file( dataPath.toStdString(), std::ios::binary );
  QgsLazInfo lazInfo = QgsLazInfo::fromFile( file );

  QVERIFY( lazInfo.isValid() );
  QCOMPARE( lazInfo.pointCount(), 518862 );
  QCOMPARE( lazInfo.scale().toVector3D(), QVector3D( 0.0001f, 0.0001f, 0.0001f ) );
  QCOMPARE( lazInfo.offset().toVector3D(), QVector3D( 515385, 4918361, 2330.5 ) );
  QPair<uint16_t, uint16_t> creationYearDay = lazInfo.creationYearDay();
  QCOMPARE( creationYearDay.first, 1 );
  QCOMPARE( creationYearDay.second, 1 );
  QPair<uint8_t, uint8_t> version = lazInfo.version();
  QCOMPARE( version.first, 1 );
  QCOMPARE( version.second, 4 );
  QCOMPARE( lazInfo.pointFormat(), 6 );
  QCOMPARE( lazInfo.systemId(), QString() );
  QCOMPARE( lazInfo.softwareId(), QString() );
  QCOMPARE( lazInfo.minCoords().toVector3D(), QVector3D( 515368.60224999999627471f, 4918340.36400000005960464f, 2322.89624999999978172f ) );
  QCOMPARE( lazInfo.maxCoords().toVector3D(), QVector3D( 515401.04300000000512227f, 4918381.12375000026077032f, 2338.57549999999991996f ) );
  QCOMPARE( lazInfo.firstPointRecordOffset(), 1628 );
  QCOMPARE( lazInfo.firstVariableLengthRecord(), 375 );
  QCOMPARE( lazInfo.pointRecordLength(), 34 );
  QCOMPARE( lazInfo.extrabytesCount(), 4 );
}

void TestQgsCopcProvider::validLayer()
{
  const QString dataPath = copyTestData( u"point_clouds/copc/sunshine-coast.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );

  QCOMPARE( layer->crs().authid(), u"EPSG:28356"_s );
  QGSCOMPARENEAR( layer->extent().xMinimum(), 498062.0, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMinimum(), 7050992.84, 0.1 );
  QGSCOMPARENEAR( layer->extent().xMaximum(), 498067.39, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMaximum(), 7050997.04, 0.1 );
  QCOMPARE( layer->dataProvider()->polygonBounds().asWkt( 0 ), u"Polygon ((498062 7050993, 498067 7050993, 498067 7050997, 498062 7050997, 498062 7050993))"_s );
  QCOMPARE( layer->dataProvider()->pointCount(), 253 );
  QCOMPARE( layer->pointCount(), 253 );

  QVERIFY( layer->dataProvider()->index() );
  // all hierarchy is stored in a single node
  QVERIFY( layer->dataProvider()->index().hasNode( QgsPointCloudNodeId::fromString( "0-0-0-0" ) ) );
  QVERIFY( !layer->dataProvider()->index().hasNode( QgsPointCloudNodeId::fromString( "1-0-0-0" ) ) );
}

void TestQgsCopcProvider::validLayerWithCopcHierarchy()
{
  const QString dataPath = copyTestData( u"point_clouds/copc/lone-star.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );

  QGSCOMPARENEAR( layer->extent().xMinimum(), 515368.6022, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMinimum(), 4918340.364, 0.1 );
  QGSCOMPARENEAR( layer->extent().xMaximum(), 515401.043, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMaximum(), 4918381.124, 0.1 );

  QVERIFY( layer->dataProvider()->index() );
  // all hierarchy is stored in multiple nodes
  QVERIFY( layer->dataProvider()->index().hasNode( QgsPointCloudNodeId::fromString( "1-1-1-0" ) ) );
  QVERIFY( layer->dataProvider()->index().hasNode( QgsPointCloudNodeId::fromString( "2-3-3-1" ) ) );
}

void TestQgsCopcProvider::attributes()
{
  const QString dataPath = copyTestData( u"/point_clouds/copc/sunshine-coast.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );

  const QgsPointCloudAttributeCollection attributes = layer->attributes();
  QCOMPARE( attributes.count(), 21 );
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
  QCOMPARE( attributes.at( 10 ).type(), QgsPointCloudAttribute::UChar );
  QCOMPARE( attributes.at( 11 ).name(), u"PointSourceId"_s );
  QCOMPARE( attributes.at( 11 ).type(), QgsPointCloudAttribute::UShort );
  QCOMPARE( attributes.at( 12 ).name(), u"Synthetic"_s );
  QCOMPARE( attributes.at( 12 ).type(), QgsPointCloudAttribute::UChar );
  QCOMPARE( attributes.at( 13 ).name(), u"KeyPoint"_s );
  QCOMPARE( attributes.at( 13 ).type(), QgsPointCloudAttribute::UChar );
  QCOMPARE( attributes.at( 14 ).name(), u"Withheld"_s );
  QCOMPARE( attributes.at( 14 ).type(), QgsPointCloudAttribute::UChar );
  QCOMPARE( attributes.at( 15 ).name(), u"Overlap"_s );
  QCOMPARE( attributes.at( 15 ).type(), QgsPointCloudAttribute::UChar );
  QCOMPARE( attributes.at( 16 ).name(), u"ScannerChannel"_s );
  QCOMPARE( attributes.at( 16 ).type(), QgsPointCloudAttribute::Char );
  QCOMPARE( attributes.at( 17 ).name(), u"GpsTime"_s );
  QCOMPARE( attributes.at( 17 ).type(), QgsPointCloudAttribute::Double );
  QCOMPARE( attributes.at( 18 ).name(), u"Red"_s );
  QCOMPARE( attributes.at( 18 ).type(), QgsPointCloudAttribute::UShort );
  QCOMPARE( attributes.at( 19 ).name(), u"Green"_s );
  QCOMPARE( attributes.at( 19 ).type(), QgsPointCloudAttribute::UShort );
  QCOMPARE( attributes.at( 20 ).name(), u"Blue"_s );
  QCOMPARE( attributes.at( 20 ).type(), QgsPointCloudAttribute::UShort );
}

void TestQgsCopcProvider::calculateZRange()
{
  const QString dataPath = copyTestData( u"/point_clouds/copc/sunshine-coast.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );

  QgsDoubleRange range = layer->elevationProperties()->calculateZRange( layer.get() );
  QGSCOMPARENEAR( range.lower(), 74.34, 0.01 );
  QGSCOMPARENEAR( range.upper(), 80.02, 0.01 );

  static_cast<QgsPointCloudLayerElevationProperties *>( layer->elevationProperties() )->setZScale( 2 );
  static_cast<QgsPointCloudLayerElevationProperties *>( layer->elevationProperties() )->setZOffset( 0.5 );

  range = layer->elevationProperties()->calculateZRange( layer.get() );
  QGSCOMPARENEAR( range.lower(), 149.18, 0.01 );
  QGSCOMPARENEAR( range.upper(), 160.54, 0.01 );
}

void TestQgsCopcProvider::testIdentify_data()
{
  QTest::addColumn<QString>( "srcDatasetPath" );

  QTest::newRow( "copc" ) << u"point_clouds/copc/sunshine-coast.copc.laz"_s;
}

void TestQgsCopcProvider::testIdentify()
{
  QFETCH( QString, srcDatasetPath );

  const QString datasetPath = copyTestData( srcDatasetPath );

  auto layer = std::make_unique<QgsPointCloudLayer>( datasetPath, u"layer"_s, u"copc"_s );

  QVERIFY( layer->isValid() );

  // identify 1 point click (rectangular point shape)
  {
    QgsPolygonXY polygon;
    QVector<QgsPointXY> ring;
    ring.push_back( QgsPointXY( 498062.50018404237926, 7050996.5845294082537 ) );
    ring.push_back( QgsPointXY( 498062.5405028705718, 7050996.5845294082537 ) );
    ring.push_back( QgsPointXY( 498062.5405028705718, 7050996.6248482363299 ) );
    ring.push_back( QgsPointXY( 498062.50018404237926, 7050996.6248482363299 ) );
    ring.push_back( QgsPointXY( 498062.50018404237926, 7050996.5845294082537 ) );
    polygon.push_back( ring );
    const float maxErrorInMapCoords = 0.0022857920266687870026;
    QVector<QMap<QString, QVariant>> points = layer->dataProvider()->identify( maxErrorInMapCoords, QgsGeometry::fromPolygonXY( polygon ) );
    QCOMPARE( points.size(), 1 );
    const QMap<QString, QVariant> identifiedPoint = points[0];
    QMap<QString, QVariant> expected;

    expected[u"Blue"_s] = 0;
    expected[u"Classification"_s] = 2;
    expected[u"EdgeOfFlightLine"_s] = 0;
    expected[u"GpsTime (raw)"_s] = 268793.37257748609409;
    expected[u"Green"_s] = 0;
    expected[u"Intensity"_s] = 1765;
    expected[u"NumberOfReturns"_s] = 1;
    expected[u"PointSourceId"_s] = 7041;
    expected[u"Red"_s] = 0;
    expected[u"ReturnNumber"_s] = 1;
    expected[u"ScanAngleRank"_s] = -28.0020008087;
    expected[u"ScanDirectionFlag"_s] = 1;
    expected[u"UserData"_s] = 17;
    expected[u"X"_s] = 498062.52;
    expected[u"Y"_s] = 7050996.61;
    expected[u"Z"_s] = 75.0;
    // compare values using toDouble() so that fuzzy comparison is used in case of
    // tiny rounding errors (e.g. 74.6 vs 74.60000000000001)
    for ( auto it = expected.constBegin(); it != expected.constEnd(); it++ )
    {
      QCOMPARE( identifiedPoint[it.key()].toDouble(), it.value().toDouble() );
    }
  }

  // identify 1 point (circular point shape)
  {
    QPolygonF polygon;
    polygon.push_back( QPointF( 498066.28873652569018, 7050994.9709538575262 ) );
    polygon.push_back( QPointF( 498066.21890226693358, 7050995.0112726856023 ) );
    polygon.push_back( QPointF( 498066.21890226693358, 7050995.0919103417546 ) );
    polygon.push_back( QPointF( 498066.28873652569018, 7050995.1322291698307 ) );
    polygon.push_back( QPointF( 498066.35857078444678, 7050995.0919103417546 ) );
    polygon.push_back( QPointF( 498066.35857078444678, 7050995.0112726856023 ) );
    polygon.push_back( QPointF( 498066.28873652569018, 7050994.9709538575262 ) );
    const float maxErrorInMapCoords = 0.0091431681066751480103;
    const QVector<QMap<QString, QVariant>> identifiedPoints = layer->dataProvider()->identify( maxErrorInMapCoords, QgsGeometry::fromQPolygonF( polygon ) );
    QVector<QMap<QString, QVariant>> expected;
    {
      QMap<QString, QVariant> point;
      point[u"Blue"_s] = "0";
      point[u"Classification"_s] = "2";
      point[u"EdgeOfFlightLine"_s] = "0";
      point[u"GpsTime (raw)"_s] = "268793.3373408913";
      point[u"Green"_s] = "0";
      point[u"Intensity"_s] = "278";
      point[u"NumberOfReturns"_s] = "1";
      point[u"PointSourceId"_s] = "7041";
      point[u"Red"_s] = "0";
      point[u"ReturnNumber"_s] = "1";
      point[u"ScanAngleRank"_s] = "-28.0020008087";
      point[u"ScanDirectionFlag"_s] = "1";
      point[u"UserData"_s] = "17";
      point[u"X"_s] = "498066.27";
      point[u"Y"_s] = "7050995.06";
      point[u"Z"_s] = "74.60";
      expected.push_back( point );
    }

    // compare values using toDouble() so that fuzzy comparison is used in case of
    // tiny rounding errors (e.g. 74.6 vs 74.60000000000001)
    QCOMPARE( identifiedPoints.count(), 1 );
    const QStringList keys = expected[0].keys();
    for ( const QString &k : keys )
    {
      QCOMPARE( identifiedPoints[0][k].toDouble(), expected[0][k].toDouble() );
    }
  }

  // test rectangle selection
  {
    QPolygonF polygon;
    polygon.push_back( QPointF( 498063.24382022250211, 7050996.8638040581718 ) );
    polygon.push_back( QPointF( 498063.02206666755956, 7050996.8638040581718 ) );
    polygon.push_back( QPointF( 498063.02206666755956, 7050996.6360026793554 ) );
    polygon.push_back( QPointF( 498063.24382022250211, 7050996.6360026793554 ) );
    polygon.push_back( QPointF( 498063.24382022250211, 7050996.8638040581718 ) );

    const float maxErrorInMapCoords = 0.0022857920266687870026;
    const QVector<QMap<QString, QVariant>> identifiedPoints = layer->dataProvider()->identify( maxErrorInMapCoords, QgsGeometry::fromQPolygonF( polygon ) );
    QVector<QMap<QString, QVariant>> expected;
    {
      QMap<QString, QVariant> point;
      point[u"Blue"_s] = "0";
      point[u"Classification"_s] = "2";
      point[u"EdgeOfFlightLine"_s] = "0";
      point[u"GpsTime (raw)"_s] = "268793.3813974548";
      point[u"Green"_s] = "0";
      point[u"Intensity"_s] = "1142";
      point[u"NumberOfReturns"_s] = "1";
      point[u"PointSourceId"_s] = "7041";
      point[u"Red"_s] = "0";
      point[u"ReturnNumber"_s] = "1";
      point[u"ScanAngleRank"_s] = "-28.0020008087";
      point[u"ScanDirectionFlag"_s] = "1";
      point[u"UserData"_s] = "17";
      point[u"X"_s] = "498063.14";
      point[u"Y"_s] = "7050996.79";
      point[u"Z"_s] = "74.89";
      expected.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[u"Blue"_s] = "0";
      point[u"Classification"_s] = "3";
      point[u"EdgeOfFlightLine"_s] = "0";
      point[u"GpsTime (raw)"_s] = "269160.5176644815";
      point[u"Green"_s] = "0";
      point[u"Intensity"_s] = "1631";
      point[u"NumberOfReturns"_s] = "1";
      point[u"PointSourceId"_s] = "7042";
      point[u"Red"_s] = "0";
      point[u"ReturnNumber"_s] = "1";
      point[u"ScanAngleRank"_s] = "-12";
      point[u"ScanDirectionFlag"_s] = "1";
      point[u"UserData"_s] = "17";
      point[u"X"_s] = "498063.11";
      point[u"Y"_s] = "7050996.75";
      point[u"Z"_s] = "74.90";
      expected.push_back( point );
    }

    QVERIFY( expected.size() == identifiedPoints.size() );
    const QStringList keys = expected[0].keys();
    for ( int i = 0; i < expected.size(); ++i )
    {
      for ( const QString &k : keys )
      {
        QCOMPARE( identifiedPoints[i][k].toDouble(), expected[i][k].toDouble() );
      }
    }
  }
}

void TestQgsCopcProvider::testExtraBytesAttributesExtraction()
{
  {
    const QString dataPath = copyTestData( u"/point_clouds/copc/extrabytes-dataset.copc.laz"_s );

    std::ifstream file( dataPath.toStdString(), std::ios::binary );
    QgsLazInfo lazInfo = QgsLazInfo::fromFile( file );
    QVector<QgsLazInfo::ExtraBytesAttributeDetails> attributes = lazInfo.extrabytes();
    QCOMPARE( attributes.size(), 3 );

    QCOMPARE( attributes[0].attribute, u"Amplitude"_s );
    QCOMPARE( attributes[1].attribute, u"Reflectance"_s );
    QCOMPARE( attributes[2].attribute, u"Deviation"_s );

    QCOMPARE( attributes[0].type, QgsPointCloudAttribute::Float );
    QCOMPARE( attributes[1].type, QgsPointCloudAttribute::Float );
    QCOMPARE( attributes[2].type, QgsPointCloudAttribute::Float );

    QCOMPARE( attributes[0].size, 4 );
    QCOMPARE( attributes[1].size, 4 );
    QCOMPARE( attributes[2].size, 4 );

    QCOMPARE( attributes[0].offset, 44 );
    QCOMPARE( attributes[1].offset, 40 );
    QCOMPARE( attributes[2].offset, 36 );
  }

  {
    const QString dataPath = copyTestData( u"/point_clouds/copc/no-extrabytes-dataset.copc.laz"_s );

    std::ifstream file( dataPath.toStdString(), std::ios::binary );
    QgsLazInfo lazInfo = QgsLazInfo::fromFile( file );
    QVector<QgsLazInfo::ExtraBytesAttributeDetails> attributes = lazInfo.extrabytes();
    QCOMPARE( attributes.size(), 0 );
  }
}

void TestQgsCopcProvider::testExtraBytesAttributesValues()
{
  const QString dataPath = copyTestData( u"/point_clouds/copc/extrabytes-dataset.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );
  {
    const float maxErrorInMapCoords = 0.0015207174f;
    QPolygonF polygon;
    polygon.push_back( QPointF( 527919.2459517354, 6210983.5918774214 ) );
    polygon.push_back( QPointF( 527919.0742796324, 6210983.5918774214 ) );
    polygon.push_back( QPointF( 527919.0742796324, 6210983.4383113598 ) );
    polygon.push_back( QPointF( 527919.2459517354, 6210983.4383113598 ) );
    polygon.push_back( QPointF( 527919.2459517354, 6210983.5918774214 ) );

    QVector<QMap<QString, QVariant>> identifiedPoints = layer->dataProvider()->identify( maxErrorInMapCoords, QgsGeometry::fromQPolygonF( polygon ) );

    QVector<QMap<QString, QVariant>> expectedPoints;
    {
      QMap<QString, QVariant> point;
      point[u"Amplitude"_s] = "14.170000076293945";
      point[u"Blue"_s] = "0";
      point[u"Classification"_s] = "2";
      point[u"Deviation"_s] = "0";
      point[u"EdgeOfFlightLine"_s] = "0";
      point[u"GpsTime (raw)"_s] = "302522582.235839";
      point[u"Green"_s] = "0";
      point[u"Intensity"_s] = "1417";
      point[u"NumberOfReturns"_s] = "3";
      point[u"PointSourceId"_s] = "15017";
      point[u"Red"_s] = "0";
      point[u"Reflectance"_s] = "-8.050000190734863";
      point[u"ReturnNumber"_s] = "3";
      point[u"ScanAngleRank"_s] = "-6";
      point[u"ScanDirectionFlag"_s] = "0";
      point[u"UserData"_s] = "0";
      point[u"X"_s] = "527919.11";
      point[u"Y"_s] = "6210983.55";
      point[u"Z"_s] = "147.111";
      expectedPoints.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[u"Amplitude"_s] = "4.409999847412109";
      point[u"Blue"_s] = "0";
      point[u"Classification"_s] = "5";
      point[u"Deviation"_s] = "2";
      point[u"EdgeOfFlightLine"_s] = "0";
      point[u"GpsTime (raw)"_s] = "302522582.235838";
      point[u"Green"_s] = "0";
      point[u"Intensity"_s] = "441";
      point[u"NumberOfReturns"_s] = "3";
      point[u"PointSourceId"_s] = "15017";
      point[u"Red"_s] = "0";
      point[u"Reflectance"_s] = "-17.829999923706055";
      point[u"ReturnNumber"_s] = "2";
      point[u"ScanAngleRank"_s] = "-6";
      point[u"ScanDirectionFlag"_s] = "0";
      point[u"UserData"_s] = "0";
      point[u"X"_s] = "527919.1799999999";
      point[u"Y"_s] = "6210983.47";
      point[u"Z"_s] = "149.341";
      expectedPoints.push_back( point );
    }

    auto cmp = []( const QMap<QString, QVariant> &p1, const QMap<QString, QVariant> &p2 ) {
      return qgsVariantLessThan( p1.value( u"X"_s, 0 ), p2.value( u"X"_s, 0 ) );
    };
    std::sort( expectedPoints.begin(), expectedPoints.end(), cmp );
    std::sort( identifiedPoints.begin(), identifiedPoints.end(), cmp );

    QVERIFY( identifiedPoints.size() == expectedPoints.size() );
    const QStringList keys = expectedPoints[0].keys();
    for ( int i = 0; i < identifiedPoints.size(); ++i )
    {
      for ( const QString &k : keys )
      {
        QCOMPARE( identifiedPoints[i][k].toDouble(), expectedPoints[i][k].toDouble() );
      }
    }
  }
}

void TestQgsCopcProvider::testClassFlagsValues()
{
  const QString dataPath = copyTestData( u"/point_clouds/copc/extrabytes-dataset.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );
  {
    const float maxErrorInMapCoords = 0.0015207174f;
    QPolygonF polygon;
    polygon.push_back( QPointF( 527919.6, 6210983.6 ) );
    polygon.push_back( QPointF( 527919.0, 6210983.6 ) );
    polygon.push_back( QPointF( 527919.0, 6210983.4 ) );
    polygon.push_back( QPointF( 527919.6, 6210983.4 ) );
    polygon.push_back( QPointF( 527919.6, 6210983.6 ) );

    QVector<QMap<QString, QVariant>> identifiedPoints = layer->dataProvider()->identify( maxErrorInMapCoords, QgsGeometry::fromQPolygonF( polygon ) );

    QVector<QMap<QString, QVariant>> expectedPoints;
    {
      QMap<QString, QVariant> point;
      point[u"Amplitude"_s] = "14.170000076293945";
      point[u"Blue"_s] = "0";
      point[u"Classification"_s] = "2";
      point[u"Deviation"_s] = "0";
      point[u"EdgeOfFlightLine"_s] = "0";
      point[u"GpsTime (raw)"_s] = "302522582.235839";
      point[u"Green"_s] = "0";
      point[u"Intensity"_s] = "1417";
      point[u"NumberOfReturns"_s] = "3";
      point[u"PointSourceId"_s] = "15017";
      point[u"Red"_s] = "0";
      point[u"Reflectance"_s] = "-8.050000190734863";
      point[u"ReturnNumber"_s] = "3";
      point[u"ScanAngleRank"_s] = "-6";
      point[u"ScanDirectionFlag"_s] = "0";
      point[u"UserData"_s] = "0";
      point[u"Synthetic"_s] = "1";
      point[u"KeyPoint"_s] = "0";
      point[u"Withheld"_s] = "0";
      point[u"Overlap"_s] = "0";
      point[u"X"_s] = "527919.11";
      point[u"Y"_s] = "6210983.55";
      point[u"Z"_s] = "147.111";
      expectedPoints.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[u"Amplitude"_s] = "4.409999847412109";
      point[u"Blue"_s] = "0";
      point[u"Classification"_s] = "5";
      point[u"Deviation"_s] = "2";
      point[u"EdgeOfFlightLine"_s] = "0";
      point[u"GpsTime (raw)"_s] = "302522582.235838";
      point[u"Green"_s] = "0";
      point[u"Intensity"_s] = "441";
      point[u"NumberOfReturns"_s] = "3";
      point[u"PointSourceId"_s] = "15017";
      point[u"Red"_s] = "0";
      point[u"Reflectance"_s] = "-17.829999923706055";
      point[u"ReturnNumber"_s] = "2";
      point[u"ScanAngleRank"_s] = "-6";
      point[u"ScanDirectionFlag"_s] = "0";
      point[u"UserData"_s] = "0";
      point[u"Synthetic"_s] = "1";
      point[u"KeyPoint"_s] = "1";
      point[u"Withheld"_s] = "0";
      point[u"Overlap"_s] = "0";
      point[u"X"_s] = "527919.1799999999";
      point[u"Y"_s] = "6210983.47";
      point[u"Z"_s] = "149.341";
      expectedPoints.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[u"Amplitude"_s] = "7.539999961853027";
      point[u"Blue"_s] = "0";
      point[u"Classification"_s] = "5";
      point[u"Deviation"_s] = "8";
      point[u"EdgeOfFlightLine"_s] = "0";
      point[u"GpsTime (raw)"_s] = "302522582.235837";
      point[u"Green"_s] = "0";
      point[u"Intensity"_s] = "754";
      point[u"NumberOfReturns"_s] = "3";
      point[u"PointSourceId"_s] = "15017";
      point[u"Red"_s] = "0";
      point[u"Reflectance"_s] = "-14.720000267028809";
      point[u"ReturnNumber"_s] = "2";
      point[u"ScanAngleRank"_s] = "-6";
      point[u"ScanDirectionFlag"_s] = "0";
      point[u"UserData"_s] = "0";
      point[u"Synthetic"_s] = "1";
      point[u"KeyPoint"_s] = "1";
      point[u"Withheld"_s] = "1";
      point[u"Overlap"_s] = "1";
      point[u"X"_s] = "527919.31";
      point[u"Y"_s] = "6210983.42";
      point[u"Z"_s] = "150.99099999999999";
      expectedPoints.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[u"Amplitude"_s] = "15.390000343322754";
      point[u"Blue"_s] = "0";
      point[u"Classification"_s] = "2";
      point[u"Deviation"_s] = "6";
      point[u"EdgeOfFlightLine"_s] = "0";
      point[u"GpsTime (raw)"_s] = "302522582.235838";
      point[u"Green"_s] = "0";
      point[u"Intensity"_s] = "1539";
      point[u"NumberOfReturns"_s] = "3";
      point[u"PointSourceId"_s] = "15017";
      point[u"Red"_s] = "0";
      point[u"Reflectance"_s] = "-6.829999923706055";
      point[u"ReturnNumber"_s] = "3";
      point[u"ScanAngleRank"_s] = "-6";
      point[u"ScanDirectionFlag"_s] = "0";
      point[u"UserData"_s] = "0";
      point[u"Synthetic"_s] = "1";
      point[u"KeyPoint"_s] = "1";
      point[u"Withheld"_s] = "1";
      point[u"Overlap"_s] = "0";
      point[u"X"_s] = "527919.39";
      point[u"Y"_s] = "6210983.56";
      point[u"Z"_s] = "147.101";
      expectedPoints.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[u"Amplitude"_s] = "11.710000038146973";
      point[u"Blue"_s] = "0";
      point[u"Classification"_s] = "5";
      point[u"Deviation"_s] = "43";
      point[u"EdgeOfFlightLine"_s] = "0";
      point[u"GpsTime (raw)"_s] = "302522582.23583597";
      point[u"Green"_s] = "0";
      point[u"Intensity"_s] = "1171";
      point[u"NumberOfReturns"_s] = "3";
      point[u"PointSourceId"_s] = "15017";
      point[u"Red"_s] = "0";
      point[u"Reflectance"_s] = "-10.550000190734863";
      point[u"ReturnNumber"_s] = "1";
      point[u"ScanAngleRank"_s] = "-6";
      point[u"ScanDirectionFlag"_s] = "0";
      point[u"UserData"_s] = "0";
      point[u"Synthetic"_s] = "0";
      point[u"KeyPoint"_s] = "0";
      point[u"Withheld"_s] = "0";
      point[u"Overlap"_s] = "0";
      point[u"X"_s] = "527919.58";
      point[u"Y"_s] = "6210983.42";
      point[u"Z"_s] = "151.131";
      expectedPoints.push_back( point );
    }

    auto cmp = []( const QMap<QString, QVariant> &p1, const QMap<QString, QVariant> &p2 ) {
      return qgsVariantLessThan( p1.value( u"X"_s, 0 ), p2.value( u"X"_s, 0 ) );
    };
    std::sort( expectedPoints.begin(), expectedPoints.end(), cmp );
    std::sort( identifiedPoints.begin(), identifiedPoints.end(), cmp );

    QVERIFY( identifiedPoints.size() == expectedPoints.size() );
    const QStringList keys = expectedPoints[0].keys();
    for ( int i = 0; i < identifiedPoints.size(); ++i )
    {
      for ( const QString &k : keys )
      {
        QCOMPARE( identifiedPoints[i][k].toDouble(), expectedPoints[i][k].toDouble() );
      }
    }
  }
}

void TestQgsCopcProvider::testPointCloudIndex()
{
  const QString dataPath = copyTestData( u"/point_clouds/copc/lone-star.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );

  QgsPointCloudIndex index = layer->dataProvider()->index();
  QVERIFY( index.isValid() );

  QCOMPARE( index.getNode( QgsPointCloudNodeId::fromString( u"0-0-0-0"_s ) ).pointCount(), 56721 );
  QVERIFY( !index.hasNode( QgsPointCloudNodeId::fromString( u"1-1-1-1"_s ) ) );
  QCOMPARE( index.getNode( QgsPointCloudNodeId::fromString( u"2-3-3-1"_s ) ).pointCount(), 446 );
  QVERIFY( !index.hasNode( QgsPointCloudNodeId::fromString( u"9-9-9-9"_s ) ) );

  QCOMPARE( index.pointCount(), 518862 );
  QCOMPARE( index.zMin(), 2322.89625 );
  QCOMPARE( index.zMax(), 2338.5755 );
  QCOMPARE( index.scale().toVector3D(), QVector3D( 0.0001f, 0.0001f, 0.0001f ) );
  QCOMPARE( index.offset().toVector3D(), QVector3D( 515385, 4918361, 2330.5 ) );
  QCOMPARE( index.span(), 128 );

  QCOMPARE( index.getNode( QgsPointCloudNodeId::fromString( u"0-0-0-0"_s ) ).error(), 0.328125 );
  QCOMPARE( index.getNode( QgsPointCloudNodeId::fromString( u"2-3-3-1"_s ) ).error(), 0.08203125 );

  {
    QgsBox3D bounds = index.getNode( QgsPointCloudNodeId::fromString( u"0-0-0-0"_s ) ).bounds();
    QCOMPARE( bounds.xMinimum(), 515368 );
    QCOMPARE( bounds.yMinimum(), 4918340 );
    QCOMPARE( bounds.zMinimum(), 2322 );
    QCOMPARE( bounds.xMaximum(), 515410 );
    QCOMPARE( bounds.yMaximum(), 4918382 );
    QCOMPARE( bounds.zMaximum(), 2364 );
  }

  {
    QgsBox3D bounds = QgsPointCloudNode::bounds( index.rootNodeBounds(), QgsPointCloudNodeId::fromString( u"1-1-1-1"_s ) );
    QCOMPARE( bounds.xMinimum(), 515389 );
    QCOMPARE( bounds.yMinimum(), 4918361 );
    QCOMPARE( bounds.zMinimum(), 2343 );
    QCOMPARE( bounds.xMaximum(), 515410 );
    QCOMPARE( bounds.yMaximum(), 4918382 );
    QCOMPARE( bounds.zMaximum(), 2364 );
  }

  {
    QgsBox3D bounds = index.getNode( QgsPointCloudNodeId::fromString( u"2-3-3-1"_s ) ).bounds();
    QCOMPARE( bounds.xMinimum(), 515399.5 );
    QCOMPARE( bounds.yMinimum(), 4918371.5 );
    QCOMPARE( bounds.zMinimum(), 2332.5 );
    QCOMPARE( bounds.xMaximum(), 515410 );
    QCOMPARE( bounds.yMaximum(), 4918382 );
    QCOMPARE( bounds.zMaximum(), 2343 );
  }
}

void TestQgsCopcProvider::testStatsCalculator()
{
  const QString dataPath = copyTestData( u"/point_clouds/copc/extrabytes-dataset.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QgsPointCloudIndex index = layer->dataProvider()->index();
  QgsPointCloudStatsCalculator calculator( index );

  QVector<QgsPointCloudAttribute> attributes;
  attributes.append( QgsPointCloudAttribute( u"Deviation"_s, QgsPointCloudAttribute::Float ) );
  attributes.append( QgsPointCloudAttribute( u"Synthetic"_s, QgsPointCloudAttribute::UChar ) );
  attributes.append( QgsPointCloudAttribute( u"KeyPoint"_s, QgsPointCloudAttribute::UChar ) );
  attributes.append( QgsPointCloudAttribute( u"Withheld"_s, QgsPointCloudAttribute::UChar ) );
  attributes.append( QgsPointCloudAttribute( u"Overlap"_s, QgsPointCloudAttribute::UChar ) );
  attributes.append( QgsPointCloudAttribute( u"Red"_s, QgsPointCloudAttribute::UShort ) );
  attributes.append( QgsPointCloudAttribute( u"EdgeOfFlightLine"_s, QgsPointCloudAttribute::Char ) );
  attributes.append( QgsPointCloudAttribute( u"Blue"_s, QgsPointCloudAttribute::UShort ) );
  attributes.append( QgsPointCloudAttribute( u"GpsTime"_s, QgsPointCloudAttribute::Double ) );
  attributes.append( QgsPointCloudAttribute( u"Green"_s, QgsPointCloudAttribute::UShort ) );
  attributes.append( QgsPointCloudAttribute( u"ReturnNumber"_s, QgsPointCloudAttribute::Char ) );
  attributes.append( QgsPointCloudAttribute( u"NumberOfReturns"_s, QgsPointCloudAttribute::Char ) );
  attributes.append( QgsPointCloudAttribute( u"Intensity"_s, QgsPointCloudAttribute::UShort ) );
  attributes.append( QgsPointCloudAttribute( u"PointSourceId"_s, QgsPointCloudAttribute::UShort ) );
  attributes.append( QgsPointCloudAttribute( u"ScanDirectionFlag"_s, QgsPointCloudAttribute::Char ) );
  attributes.append( QgsPointCloudAttribute( u"UserData"_s, QgsPointCloudAttribute::UChar ) );
  attributes.append( QgsPointCloudAttribute( u"ScanAngleRank"_s, QgsPointCloudAttribute::Float ) );
  attributes.append( QgsPointCloudAttribute( u"Reflectance"_s, QgsPointCloudAttribute::Float ) );
  attributes.append( QgsPointCloudAttribute( u"Amplitude"_s, QgsPointCloudAttribute::Float ) );
  attributes.append( QgsPointCloudAttribute( u"Classification"_s, QgsPointCloudAttribute::Char ) );

  QgsFeedback feedback;

  calculator.calculateStats( &feedback, attributes );

  QgsPointCloudStatistics stats = calculator.statistics();

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"Amplitude"_s );
    QCOMPARE( ( float ) s.minimum, 1.1599999666214 );
    QCOMPARE( ( float ) s.maximum, 19.6000003814697 );
    QGSCOMPARENEAR( static_cast<float>( s.stDev ), 1.1360613108, 0.0000001 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"Blue"_s );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
    QCOMPARE( static_cast<float>( s.stDev ), 0 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"Synthetic"_s );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 1 );
    QGSCOMPARENEAR( static_cast<float>( s.stDev ), 0.0061436155811, 0.0000001 );
    QMap<int, int> classCount = s.classCount;
    QCOMPARE( classCount.size(), 2 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"KeyPoint"_s );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 1 );
    QGSCOMPARENEAR( static_cast<float>( s.stDev ), 0.0053205453, 0.0000001 );
    QMap<int, int> classCount = s.classCount;
    QCOMPARE( classCount.size(), 2 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"Withheld"_s );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 1 );
    QGSCOMPARENEAR( static_cast<float>( s.stDev ), 0.0043442328, 0.0000001 );
    QMap<int, int> classCount = s.classCount;
    QCOMPARE( classCount.size(), 2 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"Overlap"_s );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 1 );
    QGSCOMPARENEAR( static_cast<float>( s.stDev ), 0.0030718162, 0.0000001 );
    QMap<int, int> classCount = s.classCount;
    QCOMPARE( classCount.size(), 2 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"Classification"_s );
    QCOMPARE( ( float ) s.minimum, 2 );
    QCOMPARE( ( float ) s.maximum, 18 );
    QGSCOMPARENEAR( static_cast<float>( s.stDev ), 0.5567283630, 0.0000001 );
    QMap<int, int> classCount = s.classCount;
    QCOMPARE( classCount.size(), 7 );
    QCOMPARE( classCount[2], 103782 );
    QCOMPARE( classCount[3], 484 );
    QCOMPARE( classCount[4], 79 );
    QCOMPARE( classCount[5], 966 );
    QCOMPARE( classCount[7], 12 );
    QCOMPARE( classCount[8], 648 );
    QCOMPARE( classCount[18], 1 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"Deviation"_s );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 120 );
    QGSCOMPARENEAR( static_cast<float>( s.stDev ), 3.8086988926, 0.0000001 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"EdgeOfFlightLine"_s );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
    QCOMPARE( static_cast<float>( s.stDev ), 0 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"GpsTime"_s );
    QCOMPARE( ( float ) s.minimum, ( float ) 302522581.972046196460723876953 );
    QCOMPARE( ( float ) s.maximum, ( float ) 302522583.437068104743957519531 );
    QGSCOMPARENEAR( static_cast<float>( s.stDev ), 0.4110022783, 0.0000001 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"Green"_s );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
    QCOMPARE( static_cast<float>( s.stDev ), 0 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"Intensity"_s );
    QCOMPARE( ( float ) s.minimum, 116 );
    QCOMPARE( ( float ) s.maximum, 1960 );
    QGSCOMPARENEAR( static_cast<float>( s.stDev ), 113.6061325073, 0.0000001 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"NumberOfReturns"_s );
    QCOMPARE( ( float ) s.minimum, 1 );
    QCOMPARE( ( float ) s.maximum, 5 );
    QGSCOMPARENEAR( static_cast<float>( s.stDev ), 0.2079153657, 0.0000001 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"PointSourceId"_s );
    QCOMPARE( ( float ) s.minimum, 15017 );
    QCOMPARE( ( float ) s.maximum, 15017 );
    QGSCOMPARENEAR( static_cast<float>( s.stDev ), 0, 0.0000001 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"Red"_s );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
    QCOMPARE( static_cast<float>( s.stDev ), 0 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"Reflectance"_s );
    QCOMPARE( ( float ) s.minimum, -21.1100006103515625 );
    QCOMPARE( ( float ) s.maximum, -2.6099998950958251953125 );
    QGSCOMPARENEAR( static_cast<float>( s.stDev ), 1.1387866735, 0.0000001 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"ReturnNumber"_s );
    QCOMPARE( ( float ) s.minimum, 1 );
    QCOMPARE( ( float ) s.maximum, 5 );
    QGSCOMPARENEAR( static_cast<float>( s.stDev ), 0.1361096203, 0.0000001 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"ScanAngleRank"_s );
    QCOMPARE( ( float ) s.minimum, -10.998000145f );
    QCOMPARE( ( float ) s.maximum, -4.001999855f );
    QGSCOMPARENEAR( static_cast<float>( s.stDev ), 1.9546363354, 0.0000001 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"ScanDirectionFlag"_s );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
    QCOMPARE( static_cast<float>( s.stDev ), 0 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( u"UserData"_s );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
    QCOMPARE( static_cast<float>( s.stDev ), 0 );
  }
}

void TestQgsCopcProvider::testQgsRangeRequestCache()
{
  // Note: the QTest::qSleep calls were added to prevent 2 files from being created at very close times

  auto request = []( const QUrl &url, const QString &range ) {
    QNetworkRequest req( url );
    req.setRawHeader( "Range", range.toUtf8() );
    return req;
  };

  QTemporaryDir cacheDir;
  if ( !cacheDir.isValid() )
  {
    QVERIFY( false );
    return;
  }

  QUrl url( u"0.0.0.0/laz.copc.laz"_s );
  QgsRangeRequestCache cache;
  cache.setCacheDirectory( cacheDir.path() );
  cache.clear();
  cache.setCacheSize( 2 );

  cache.registerEntry( request( url, u"bytes=1-2"_s ), QByteArray( 1, '0' ) );
  QTest::qSleep( 10 );

  cache.registerEntry( request( url, u"bytes=3-4"_s ), QByteArray( 1, '1' ) );
  QTest::qSleep( 10 );

  cache.registerEntry( request( url, u"bytes=5-6"_s ), QByteArray( 1, '2' ) );
  QTest::qSleep( 10 );

  // (5, 6) -> (3, 4)
  {
    QFileInfoList files = cache.cacheEntries();
    QCOMPARE( files.size(), 2 );
    QVERIFY( files[0].baseName().endsWith( "bytes=5-6"_L1 ) );
    QVERIFY( files[1].baseName().endsWith( "bytes=3-4"_L1 ) );
  }

  cache.entry( request( url, u"bytes=3-4"_s ) );
  QTest::qSleep( 10 );

  // -> (3, 4) -> (5, 6)
  {
    QFileInfoList files = cache.cacheEntries();
    QCOMPARE( files.size(), 2 );
    QVERIFY( files[0].baseName().endsWith( "bytes=3-4"_L1 ) );
    QVERIFY( files[1].baseName().endsWith( "bytes=5-6"_L1 ) );
  }

  cache.registerEntry( request( url, u"bytes=7-8"_s ), QByteArray( 1, '3' ) );
  QTest::qSleep( 10 );

  // (7, 8) -> (3, 4)
  {
    QFileInfoList files = cache.cacheEntries();
    QCOMPARE( files.size(), 2 );
    QVERIFY( files[0].baseName().endsWith( "bytes=7-8"_L1 ) );
    QVERIFY( files[1].baseName().endsWith( "bytes=3-4"_L1 ) );
  }

  cache.registerEntry( request( url, u"bytes=9-10"_s ), QByteArray( 1, '4' ) );
  // (9, 10) -> (7, 8)
  {
    QFileInfoList files = cache.cacheEntries();
    QCOMPARE( files.size(), 2 );
    QVERIFY( files[0].baseName().endsWith( "bytes=9-10"_L1 ) );
    QVERIFY( files[1].baseName().endsWith( "bytes=7-8"_L1 ) );
  }
}

void TestQgsCopcProvider::testSaveLoadStats()
{
  QgsPointCloudStatistics calculatedStats;
  QgsPointCloudStatistics readStats;
  const QString dataPath = copyTestData( u"/point_clouds/copc/lone-star.copc.laz"_s );

  {
    auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
    QVERIFY( layer->isValid() );

    QVERIFY( layer->dataProvider() && layer->dataProvider()->isValid() && layer->dataProvider()->index() );
    QgsPointCloudIndex index = layer->dataProvider()->index();

    calculatedStats = layer->statistics();
    index.writeStatistics( calculatedStats );
  }

  {
    auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
    QVERIFY( layer->isValid() );

    QVERIFY( layer->dataProvider() && layer->dataProvider()->isValid() && layer->dataProvider()->index() );

    QgsPointCloudIndex index = layer->dataProvider()->index();
    readStats = index.metadataStatistics();
  }

  QVERIFY( calculatedStats.sampledPointsCount() == readStats.sampledPointsCount() );
  QVERIFY( calculatedStats.toStatisticsJson() == readStats.toStatisticsJson() );
}

void TestQgsCopcProvider::testPointCloudRequest()
{
  const QString dataPath = copyTestData( u"/point_clouds/copc/lone-star.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );

  QgsPointCloudIndex index = layer->dataProvider()->index();
  QVERIFY( index.isValid() );

  QVector<QgsPointCloudNodeId> nodes;
  QQueue<QgsPointCloudNodeId> queue;
  queue.push_back( index.root() );
  while ( !queue.empty() )
  {
    QgsPointCloudNodeId node = queue.front();
    queue.pop_front();
    nodes.push_back( node );

    for ( const QgsPointCloudNodeId &child : index.getNode( node ).children() )
    {
      queue.push_back( child );
    }
  }

  QgsPointCloudRequest request;
  request.setAttributes( layer->attributes() );
  // If request.setFilterRect() is not called, no filter should be applied
  int count = 0;
  for ( QgsPointCloudNodeId node : nodes )
  {
    auto block = index.nodeData( node, request );
    count += block->pointCount();
  }
  QCOMPARE( count, layer->pointCount() );

  // Now let's repeat the counting with an extent
  QgsRectangle extent( 515390, 4918360, 515400, 4918370 );
  request.setFilterRect( extent );
  count = 0;
  for ( QgsPointCloudNodeId node : nodes )
  {
    auto block = index.nodeData( node, request );
    count += block->pointCount();
  }
  QCOMPARE( count, 217600 );

  // Now let's repeat the counting with an extent away from the pointcloud
  extent = QgsRectangle( 0, 0, 1, 1 );
  request.setFilterRect( extent );
  count = 0;
  for ( QgsPointCloudNodeId node : nodes )
  {
    auto block = index.nodeData( node, request );
    count += block->pointCount();
  }
  QCOMPARE( count, 0 );

  // An empty extent should fetch all points again
  count = 0;
  extent = QgsRectangle();
  request.setFilterRect( extent );
  for ( QgsPointCloudNodeId node : nodes )
  {
    auto block = index.nodeData( node, request );
    count += block->pointCount();
  }
  QCOMPARE( count, layer->pointCount() );
}

void TestQgsCopcProvider::testPointCloudRequestIgnoreFilter()
{
  const QString dataPath = copyTestData( u"/point_clouds/copc/lone-star.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );

  layer->setSubsetString( u"Intensity < 1000"_s );
  QgsPointCloudIndex index = layer->dataProvider()->index();
  QVERIFY( index.isValid() );

  QVector<QgsPointCloudNodeId> nodes;
  QQueue<QgsPointCloudNodeId> queue;
  queue.push_back( index.root() );
  while ( !queue.empty() )
  {
    QgsPointCloudNodeId node = queue.front();
    queue.pop_front();
    nodes.push_back( node );

    for ( const QgsPointCloudNodeId &child : index.getNode( node ).children() )
    {
      queue.push_back( child );
    }
  }

  QgsPointCloudRequest request;
  request.setAttributes( layer->attributes() );
  // layer has a filter, point count is reduced
  int count = 0;
  for ( QgsPointCloudNodeId node : nodes )
  {
    auto block = index.nodeData( node, request );
    count += block->pointCount();
  }
  QCOMPARE( count, 247636 );

  // Now let's repeat the counting but ignore the subset string filter
  request.setIgnoreIndexFilterEnabled( true );
  count = 0;
  for ( QgsPointCloudNodeId node : nodes )
  {
    auto block = index.nodeData( node, request );
    count += block->pointCount();
  }
  QCOMPARE( count, layer->pointCount() );
}

QGSTEST_MAIN( TestQgsCopcProvider )
#include "testqgscopcprovider.moc"
