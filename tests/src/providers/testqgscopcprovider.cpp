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

#include <limits>

#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <fstream>
#include <QVector>
#include <QTest>
#include <QStandardPaths>
#include <QQueue>

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
      : QgsTest( QStringLiteral( "COPC Provider Tests" ) ) {}

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
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "copc" ) );
  QVERIFY( metadata );

  QCOMPARE( metadata->filters( Qgis::FileFilterType::PointCloud ), QStringLiteral( "COPC Point Clouds (*.copc.laz *.COPC.LAZ)" ) );
  QCOMPARE( metadata->filters( Qgis::FileFilterType::Vector ), QString() );

  const QString registryPointCloudFilters = QgsProviderRegistry::instance()->filePointCloudFilters();
  QVERIFY( registryPointCloudFilters.contains( "(*.copc.laz *.COPC.LAZ)" ) );
}

void TestQgsCopcProvider::encodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "copc" ) );
  QVERIFY( metadata );

  QVariantMap parts;
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "/home/point_clouds/dataset.copc.laz" ) );
  QCOMPARE( metadata->encodeUri( parts ), QStringLiteral( "/home/point_clouds/dataset.copc.laz" ) );
}

void TestQgsCopcProvider::decodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "copc" ) );
  QVERIFY( metadata );

  const QVariantMap parts = metadata->decodeUri( QStringLiteral( "/home/point_clouds/dataset.copc.laz" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/point_clouds/dataset.copc.laz" ) );
}

void TestQgsCopcProvider::absoluteRelativeUri()
{
  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( testDataPath( QStringLiteral( "/project.qgs" ) ) ) );

  QgsProviderMetadata *copcMetadata = QgsProviderRegistry::instance()->providerMetadata( "copc" );
  QVERIFY( copcMetadata );

  QString absoluteUri = testDataPath( QStringLiteral( "/point_clouds/copc/rgb.copc.laz" ) );
  QString relativeUri = QStringLiteral( "./point_clouds/copc/rgb.copc.laz" );
  QCOMPARE( copcMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( copcMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );
}

void TestQgsCopcProvider::preferredUri()
{
  QgsProviderMetadata *copcMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "copc" ) );
  QVERIFY( copcMetadata->capabilities() & QgsProviderMetadata::PriorityForUri );

  // test that COPC is the preferred provider for .copc.laz uris
  QList<QgsProviderRegistry::ProviderCandidateDetails> candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/dataset.copc.laz" ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), QStringLiteral( "copc" ) );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/dataset.COPC.LAZ" ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), QStringLiteral( "copc" ) );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  QVERIFY( !QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( QStringLiteral( "/home/test/dataset.copc.laz" ), QStringLiteral( "copc" ) ) );
  QVERIFY( QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( QStringLiteral( "/home/test/dataset.copc.laz" ), QStringLiteral( "ogr" ) ) );
}

void TestQgsCopcProvider::layerTypesForUri()
{
  QgsProviderMetadata *copcMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "copc" ) );
  QVERIFY( copcMetadata->capabilities() & QgsProviderMetadata::LayerTypesForUri );

  QCOMPARE( copcMetadata->validLayerTypesForUri( QStringLiteral( "/home/test/cloud.copc.laz" ) ), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );
  QCOMPARE( copcMetadata->validLayerTypesForUri( QStringLiteral( "/home/test/ept.json" ) ), QList<Qgis::LayerType>() );
}

void TestQgsCopcProvider::uriIsBlocklisted()
{
  QVERIFY( !QgsProviderRegistry::instance()->uriIsBlocklisted( QStringLiteral( "/home/test/ept.json" ) ) );
  QVERIFY( !QgsProviderRegistry::instance()->uriIsBlocklisted( QStringLiteral( "/home/test/dataset.copc.laz" ) ) );
}

void TestQgsCopcProvider::querySublayers()
{
  // test querying sub layers for a ept layer
  QgsProviderMetadata *copcMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "copc" ) );

  // invalid uri
  QList<QgsProviderSublayerDetails> res = copcMetadata->querySublayers( QString() );
  QVERIFY( res.empty() );

  // not a copc layer
  res = copcMetadata->querySublayers( testDataPath( "/lines.shp" ) );
  QVERIFY( res.empty() );

  // valid copc layer
  const QString dataPath = copyTestData( QStringLiteral( "/point_clouds/copc/sunshine-coast.copc.laz" ) );

  res = copcMetadata->querySublayers( dataPath );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "sunshine-coast.copc" ) );
  QCOMPARE( res.at( 0 ).uri(), dataPath );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "copc" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::PointCloud );

  // make sure result is valid to load layer from
  QgsProviderSublayerDetails::LayerOptions options { QgsCoordinateTransformContext() };
  std::unique_ptr<QgsPointCloudLayer> ml( qgis::down_cast<QgsPointCloudLayer *>( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( ml->isValid() );
}

void TestQgsCopcProvider::brokenPath()
{
  // test loading a bad layer URI
  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( QStringLiteral( "not valid" ), QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
  QVERIFY( !layer->isValid() );
}

void TestQgsCopcProvider::testLazInfo()
{
  const QString dataPath = copyTestData( QStringLiteral( "point_clouds/copc/lone-star.copc.laz" ) );

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
  const QString dataPath = copyTestData( QStringLiteral( "point_clouds/copc/sunshine-coast.copc.laz" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
  QVERIFY( layer->isValid() );

  QCOMPARE( layer->crs().authid(), QStringLiteral( "EPSG:28356" ) );
  QGSCOMPARENEAR( layer->extent().xMinimum(), 498062.0, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMinimum(), 7050992.84, 0.1 );
  QGSCOMPARENEAR( layer->extent().xMaximum(), 498067.39, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMaximum(), 7050997.04, 0.1 );
  QCOMPARE( layer->dataProvider()->polygonBounds().asWkt( 0 ), QStringLiteral( "Polygon ((498062 7050993, 498067 7050993, 498067 7050997, 498062 7050997, 498062 7050993))" ) );
  QCOMPARE( layer->dataProvider()->pointCount(), 253 );
  QCOMPARE( layer->pointCount(), 253 );

  QVERIFY( layer->dataProvider()->index() );
  // all hierarchy is stored in a single node
  QVERIFY( layer->dataProvider()->index().hasNode( QgsPointCloudNodeId::fromString( "0-0-0-0" ) ) );
  QVERIFY( !layer->dataProvider()->index().hasNode( QgsPointCloudNodeId::fromString( "1-0-0-0" ) ) );
}

void TestQgsCopcProvider::validLayerWithCopcHierarchy()
{
  const QString dataPath = copyTestData( QStringLiteral( "point_clouds/copc/lone-star.copc.laz" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
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
  const QString dataPath = copyTestData( QStringLiteral( "/point_clouds/copc/sunshine-coast.copc.laz" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
  QVERIFY( layer->isValid() );

  const QgsPointCloudAttributeCollection attributes = layer->attributes();
  QCOMPARE( attributes.count(), 21 );
  QCOMPARE( attributes.at( 0 ).name(), QStringLiteral( "X" ) );
  QCOMPARE( attributes.at( 0 ).type(), QgsPointCloudAttribute::Int32 );
  QCOMPARE( attributes.at( 1 ).name(), QStringLiteral( "Y" ) );
  QCOMPARE( attributes.at( 1 ).type(), QgsPointCloudAttribute::Int32 );
  QCOMPARE( attributes.at( 2 ).name(), QStringLiteral( "Z" ) );
  QCOMPARE( attributes.at( 2 ).type(), QgsPointCloudAttribute::Int32 );
  QCOMPARE( attributes.at( 3 ).name(), QStringLiteral( "Intensity" ) );
  QCOMPARE( attributes.at( 3 ).type(), QgsPointCloudAttribute::UShort );
  QCOMPARE( attributes.at( 4 ).name(), QStringLiteral( "ReturnNumber" ) );
  QCOMPARE( attributes.at( 4 ).type(), QgsPointCloudAttribute::Char );
  QCOMPARE( attributes.at( 5 ).name(), QStringLiteral( "NumberOfReturns" ) );
  QCOMPARE( attributes.at( 5 ).type(), QgsPointCloudAttribute::Char );
  QCOMPARE( attributes.at( 6 ).name(), QStringLiteral( "ScanDirectionFlag" ) );
  QCOMPARE( attributes.at( 6 ).type(), QgsPointCloudAttribute::Char );
  QCOMPARE( attributes.at( 7 ).name(), QStringLiteral( "EdgeOfFlightLine" ) );
  QCOMPARE( attributes.at( 7 ).type(), QgsPointCloudAttribute::Char );
  QCOMPARE( attributes.at( 8 ).name(), QStringLiteral( "Classification" ) );
  QCOMPARE( attributes.at( 8 ).type(), QgsPointCloudAttribute::UChar );
  QCOMPARE( attributes.at( 9 ).name(), QStringLiteral( "ScanAngleRank" ) );
  QCOMPARE( attributes.at( 9 ).type(), QgsPointCloudAttribute::Float );
  QCOMPARE( attributes.at( 10 ).name(), QStringLiteral( "UserData" ) );
  QCOMPARE( attributes.at( 10 ).type(), QgsPointCloudAttribute::UChar );
  QCOMPARE( attributes.at( 11 ).name(), QStringLiteral( "PointSourceId" ) );
  QCOMPARE( attributes.at( 11 ).type(), QgsPointCloudAttribute::UShort );
  QCOMPARE( attributes.at( 12 ).name(), QStringLiteral( "Synthetic" ) );
  QCOMPARE( attributes.at( 12 ).type(), QgsPointCloudAttribute::UChar );
  QCOMPARE( attributes.at( 13 ).name(), QStringLiteral( "KeyPoint" ) );
  QCOMPARE( attributes.at( 13 ).type(), QgsPointCloudAttribute::UChar );
  QCOMPARE( attributes.at( 14 ).name(), QStringLiteral( "Withheld" ) );
  QCOMPARE( attributes.at( 14 ).type(), QgsPointCloudAttribute::UChar );
  QCOMPARE( attributes.at( 15 ).name(), QStringLiteral( "Overlap" ) );
  QCOMPARE( attributes.at( 15 ).type(), QgsPointCloudAttribute::UChar );
  QCOMPARE( attributes.at( 16 ).name(), QStringLiteral( "ScannerChannel" ) );
  QCOMPARE( attributes.at( 16 ).type(), QgsPointCloudAttribute::Char );
  QCOMPARE( attributes.at( 17 ).name(), QStringLiteral( "GpsTime" ) );
  QCOMPARE( attributes.at( 17 ).type(), QgsPointCloudAttribute::Double );
  QCOMPARE( attributes.at( 18 ).name(), QStringLiteral( "Red" ) );
  QCOMPARE( attributes.at( 18 ).type(), QgsPointCloudAttribute::UShort );
  QCOMPARE( attributes.at( 19 ).name(), QStringLiteral( "Green" ) );
  QCOMPARE( attributes.at( 19 ).type(), QgsPointCloudAttribute::UShort );
  QCOMPARE( attributes.at( 20 ).name(), QStringLiteral( "Blue" ) );
  QCOMPARE( attributes.at( 20 ).type(), QgsPointCloudAttribute::UShort );
}

void TestQgsCopcProvider::calculateZRange()
{
  const QString dataPath = copyTestData( QStringLiteral( "/point_clouds/copc/sunshine-coast.copc.laz" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
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

  QTest::newRow( "copc" ) << QStringLiteral( "point_clouds/copc/sunshine-coast.copc.laz" );
}

void TestQgsCopcProvider::testIdentify()
{
  QFETCH( QString, srcDatasetPath );

  const QString datasetPath = copyTestData( srcDatasetPath );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( datasetPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );

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

    expected[QStringLiteral( "Blue" )] = 0;
    expected[QStringLiteral( "Classification" )] = 2;
    expected[QStringLiteral( "EdgeOfFlightLine" )] = 0;
    expected[QStringLiteral( "GpsTime (raw)" )] = 268793.37257748609409;
    expected[QStringLiteral( "Green" )] = 0;
    expected[QStringLiteral( "Intensity" )] = 1765;
    expected[QStringLiteral( "NumberOfReturns" )] = 1;
    expected[QStringLiteral( "PointSourceId" )] = 7041;
    expected[QStringLiteral( "Red" )] = 0;
    expected[QStringLiteral( "ReturnNumber" )] = 1;
    expected[QStringLiteral( "ScanAngleRank" )] = -28.0020008087;
    expected[QStringLiteral( "ScanDirectionFlag" )] = 1;
    expected[QStringLiteral( "UserData" )] = 17;
    expected[QStringLiteral( "X" )] = 498062.52;
    expected[QStringLiteral( "Y" )] = 7050996.61;
    expected[QStringLiteral( "Z" )] = 75.0;
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
      point[QStringLiteral( "Blue" )] = "0";
      point[QStringLiteral( "Classification" )] = "2";
      point[QStringLiteral( "EdgeOfFlightLine" )] = "0";
      point[QStringLiteral( "GpsTime (raw)" )] = "268793.3373408913";
      point[QStringLiteral( "Green" )] = "0";
      point[QStringLiteral( "Intensity" )] = "278";
      point[QStringLiteral( "NumberOfReturns" )] = "1";
      point[QStringLiteral( "PointSourceId" )] = "7041";
      point[QStringLiteral( "Red" )] = "0";
      point[QStringLiteral( "ReturnNumber" )] = "1";
      point[QStringLiteral( "ScanAngleRank" )] = "-28.0020008087";
      point[QStringLiteral( "ScanDirectionFlag" )] = "1";
      point[QStringLiteral( "UserData" )] = "17";
      point[QStringLiteral( "X" )] = "498066.27";
      point[QStringLiteral( "Y" )] = "7050995.06";
      point[QStringLiteral( "Z" )] = "74.60";
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
      point[QStringLiteral( "Blue" )] = "0";
      point[QStringLiteral( "Classification" )] = "2";
      point[QStringLiteral( "EdgeOfFlightLine" )] = "0";
      point[QStringLiteral( "GpsTime (raw)" )] = "268793.3813974548";
      point[QStringLiteral( "Green" )] = "0";
      point[QStringLiteral( "Intensity" )] = "1142";
      point[QStringLiteral( "NumberOfReturns" )] = "1";
      point[QStringLiteral( "PointSourceId" )] = "7041";
      point[QStringLiteral( "Red" )] = "0";
      point[QStringLiteral( "ReturnNumber" )] = "1";
      point[QStringLiteral( "ScanAngleRank" )] = "-28.0020008087";
      point[QStringLiteral( "ScanDirectionFlag" )] = "1";
      point[QStringLiteral( "UserData" )] = "17";
      point[QStringLiteral( "X" )] = "498063.14";
      point[QStringLiteral( "Y" )] = "7050996.79";
      point[QStringLiteral( "Z" )] = "74.89";
      expected.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[QStringLiteral( "Blue" )] = "0";
      point[QStringLiteral( "Classification" )] = "3";
      point[QStringLiteral( "EdgeOfFlightLine" )] = "0";
      point[QStringLiteral( "GpsTime (raw)" )] = "269160.5176644815";
      point[QStringLiteral( "Green" )] = "0";
      point[QStringLiteral( "Intensity" )] = "1631";
      point[QStringLiteral( "NumberOfReturns" )] = "1";
      point[QStringLiteral( "PointSourceId" )] = "7042";
      point[QStringLiteral( "Red" )] = "0";
      point[QStringLiteral( "ReturnNumber" )] = "1";
      point[QStringLiteral( "ScanAngleRank" )] = "-12";
      point[QStringLiteral( "ScanDirectionFlag" )] = "1";
      point[QStringLiteral( "UserData" )] = "17";
      point[QStringLiteral( "X" )] = "498063.11";
      point[QStringLiteral( "Y" )] = "7050996.75";
      point[QStringLiteral( "Z" )] = "74.90";
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
    const QString dataPath = copyTestData( QStringLiteral( "/point_clouds/copc/extrabytes-dataset.copc.laz" ) );

    std::ifstream file( dataPath.toStdString(), std::ios::binary );
    QgsLazInfo lazInfo = QgsLazInfo::fromFile( file );
    QVector<QgsLazInfo::ExtraBytesAttributeDetails> attributes = lazInfo.extrabytes();
    QCOMPARE( attributes.size(), 3 );

    QCOMPARE( attributes[0].attribute, QStringLiteral( "Amplitude" ) );
    QCOMPARE( attributes[1].attribute, QStringLiteral( "Reflectance" ) );
    QCOMPARE( attributes[2].attribute, QStringLiteral( "Deviation" ) );

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
    const QString dataPath = copyTestData( QStringLiteral( "/point_clouds/copc/no-extrabytes-dataset.copc.laz" ) );

    std::ifstream file( dataPath.toStdString(), std::ios::binary );
    QgsLazInfo lazInfo = QgsLazInfo::fromFile( file );
    QVector<QgsLazInfo::ExtraBytesAttributeDetails> attributes = lazInfo.extrabytes();
    QCOMPARE( attributes.size(), 0 );
  }
}

void TestQgsCopcProvider::testExtraBytesAttributesValues()
{
  const QString dataPath = copyTestData( QStringLiteral( "/point_clouds/copc/extrabytes-dataset.copc.laz" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
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
      point[QStringLiteral( "Amplitude" )] = "14.170000076293945";
      point[QStringLiteral( "Blue" )] = "0";
      point[QStringLiteral( "Classification" )] = "2";
      point[QStringLiteral( "Deviation" )] = "0";
      point[QStringLiteral( "EdgeOfFlightLine" )] = "0";
      point[QStringLiteral( "GpsTime (raw)" )] = "302522582.235839";
      point[QStringLiteral( "Green" )] = "0";
      point[QStringLiteral( "Intensity" )] = "1417";
      point[QStringLiteral( "NumberOfReturns" )] = "3";
      point[QStringLiteral( "PointSourceId" )] = "15017";
      point[QStringLiteral( "Red" )] = "0";
      point[QStringLiteral( "Reflectance" )] = "-8.050000190734863";
      point[QStringLiteral( "ReturnNumber" )] = "3";
      point[QStringLiteral( "ScanAngleRank" )] = "-6";
      point[QStringLiteral( "ScanDirectionFlag" )] = "0";
      point[QStringLiteral( "UserData" )] = "0";
      point[QStringLiteral( "X" )] = "527919.11";
      point[QStringLiteral( "Y" )] = "6210983.55";
      point[QStringLiteral( "Z" )] = "147.111";
      expectedPoints.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[QStringLiteral( "Amplitude" )] = "4.409999847412109";
      point[QStringLiteral( "Blue" )] = "0";
      point[QStringLiteral( "Classification" )] = "5";
      point[QStringLiteral( "Deviation" )] = "2";
      point[QStringLiteral( "EdgeOfFlightLine" )] = "0";
      point[QStringLiteral( "GpsTime (raw)" )] = "302522582.235838";
      point[QStringLiteral( "Green" )] = "0";
      point[QStringLiteral( "Intensity" )] = "441";
      point[QStringLiteral( "NumberOfReturns" )] = "3";
      point[QStringLiteral( "PointSourceId" )] = "15017";
      point[QStringLiteral( "Red" )] = "0";
      point[QStringLiteral( "Reflectance" )] = "-17.829999923706055";
      point[QStringLiteral( "ReturnNumber" )] = "2";
      point[QStringLiteral( "ScanAngleRank" )] = "-6";
      point[QStringLiteral( "ScanDirectionFlag" )] = "0";
      point[QStringLiteral( "UserData" )] = "0";
      point[QStringLiteral( "X" )] = "527919.1799999999";
      point[QStringLiteral( "Y" )] = "6210983.47";
      point[QStringLiteral( "Z" )] = "149.341";
      expectedPoints.push_back( point );
    }

    auto cmp = []( const QMap<QString, QVariant> &p1, const QMap<QString, QVariant> &p2 ) {
      return qgsVariantLessThan( p1.value( QStringLiteral( "X" ), 0 ), p2.value( QStringLiteral( "X" ), 0 ) );
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
  const QString dataPath = copyTestData( QStringLiteral( "/point_clouds/copc/extrabytes-dataset.copc.laz" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
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
      point[QStringLiteral( "Amplitude" )] = "14.170000076293945";
      point[QStringLiteral( "Blue" )] = "0";
      point[QStringLiteral( "Classification" )] = "2";
      point[QStringLiteral( "Deviation" )] = "0";
      point[QStringLiteral( "EdgeOfFlightLine" )] = "0";
      point[QStringLiteral( "GpsTime (raw)" )] = "302522582.235839";
      point[QStringLiteral( "Green" )] = "0";
      point[QStringLiteral( "Intensity" )] = "1417";
      point[QStringLiteral( "NumberOfReturns" )] = "3";
      point[QStringLiteral( "PointSourceId" )] = "15017";
      point[QStringLiteral( "Red" )] = "0";
      point[QStringLiteral( "Reflectance" )] = "-8.050000190734863";
      point[QStringLiteral( "ReturnNumber" )] = "3";
      point[QStringLiteral( "ScanAngleRank" )] = "-6";
      point[QStringLiteral( "ScanDirectionFlag" )] = "0";
      point[QStringLiteral( "UserData" )] = "0";
      point[QStringLiteral( "Synthetic" )] = "1";
      point[QStringLiteral( "KeyPoint" )] = "0";
      point[QStringLiteral( "Withheld" )] = "0";
      point[QStringLiteral( "Overlap" )] = "0";
      point[QStringLiteral( "X" )] = "527919.11";
      point[QStringLiteral( "Y" )] = "6210983.55";
      point[QStringLiteral( "Z" )] = "147.111";
      expectedPoints.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[QStringLiteral( "Amplitude" )] = "4.409999847412109";
      point[QStringLiteral( "Blue" )] = "0";
      point[QStringLiteral( "Classification" )] = "5";
      point[QStringLiteral( "Deviation" )] = "2";
      point[QStringLiteral( "EdgeOfFlightLine" )] = "0";
      point[QStringLiteral( "GpsTime (raw)" )] = "302522582.235838";
      point[QStringLiteral( "Green" )] = "0";
      point[QStringLiteral( "Intensity" )] = "441";
      point[QStringLiteral( "NumberOfReturns" )] = "3";
      point[QStringLiteral( "PointSourceId" )] = "15017";
      point[QStringLiteral( "Red" )] = "0";
      point[QStringLiteral( "Reflectance" )] = "-17.829999923706055";
      point[QStringLiteral( "ReturnNumber" )] = "2";
      point[QStringLiteral( "ScanAngleRank" )] = "-6";
      point[QStringLiteral( "ScanDirectionFlag" )] = "0";
      point[QStringLiteral( "UserData" )] = "0";
      point[QStringLiteral( "Synthetic" )] = "1";
      point[QStringLiteral( "KeyPoint" )] = "1";
      point[QStringLiteral( "Withheld" )] = "0";
      point[QStringLiteral( "Overlap" )] = "0";
      point[QStringLiteral( "X" )] = "527919.1799999999";
      point[QStringLiteral( "Y" )] = "6210983.47";
      point[QStringLiteral( "Z" )] = "149.341";
      expectedPoints.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[QStringLiteral( "Amplitude" )] = "7.539999961853027";
      point[QStringLiteral( "Blue" )] = "0";
      point[QStringLiteral( "Classification" )] = "5";
      point[QStringLiteral( "Deviation" )] = "8";
      point[QStringLiteral( "EdgeOfFlightLine" )] = "0";
      point[QStringLiteral( "GpsTime (raw)" )] = "302522582.235837";
      point[QStringLiteral( "Green" )] = "0";
      point[QStringLiteral( "Intensity" )] = "754";
      point[QStringLiteral( "NumberOfReturns" )] = "3";
      point[QStringLiteral( "PointSourceId" )] = "15017";
      point[QStringLiteral( "Red" )] = "0";
      point[QStringLiteral( "Reflectance" )] = "-14.720000267028809";
      point[QStringLiteral( "ReturnNumber" )] = "2";
      point[QStringLiteral( "ScanAngleRank" )] = "-6";
      point[QStringLiteral( "ScanDirectionFlag" )] = "0";
      point[QStringLiteral( "UserData" )] = "0";
      point[QStringLiteral( "Synthetic" )] = "1";
      point[QStringLiteral( "KeyPoint" )] = "1";
      point[QStringLiteral( "Withheld" )] = "1";
      point[QStringLiteral( "Overlap" )] = "1";
      point[QStringLiteral( "X" )] = "527919.31";
      point[QStringLiteral( "Y" )] = "6210983.42";
      point[QStringLiteral( "Z" )] = "150.99099999999999";
      expectedPoints.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[QStringLiteral( "Amplitude" )] = "15.390000343322754";
      point[QStringLiteral( "Blue" )] = "0";
      point[QStringLiteral( "Classification" )] = "2";
      point[QStringLiteral( "Deviation" )] = "6";
      point[QStringLiteral( "EdgeOfFlightLine" )] = "0";
      point[QStringLiteral( "GpsTime (raw)" )] = "302522582.235838";
      point[QStringLiteral( "Green" )] = "0";
      point[QStringLiteral( "Intensity" )] = "1539";
      point[QStringLiteral( "NumberOfReturns" )] = "3";
      point[QStringLiteral( "PointSourceId" )] = "15017";
      point[QStringLiteral( "Red" )] = "0";
      point[QStringLiteral( "Reflectance" )] = "-6.829999923706055";
      point[QStringLiteral( "ReturnNumber" )] = "3";
      point[QStringLiteral( "ScanAngleRank" )] = "-6";
      point[QStringLiteral( "ScanDirectionFlag" )] = "0";
      point[QStringLiteral( "UserData" )] = "0";
      point[QStringLiteral( "Synthetic" )] = "1";
      point[QStringLiteral( "KeyPoint" )] = "1";
      point[QStringLiteral( "Withheld" )] = "1";
      point[QStringLiteral( "Overlap" )] = "0";
      point[QStringLiteral( "X" )] = "527919.39";
      point[QStringLiteral( "Y" )] = "6210983.56";
      point[QStringLiteral( "Z" )] = "147.101";
      expectedPoints.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[QStringLiteral( "Amplitude" )] = "11.710000038146973";
      point[QStringLiteral( "Blue" )] = "0";
      point[QStringLiteral( "Classification" )] = "5";
      point[QStringLiteral( "Deviation" )] = "43";
      point[QStringLiteral( "EdgeOfFlightLine" )] = "0";
      point[QStringLiteral( "GpsTime (raw)" )] = "302522582.23583597";
      point[QStringLiteral( "Green" )] = "0";
      point[QStringLiteral( "Intensity" )] = "1171";
      point[QStringLiteral( "NumberOfReturns" )] = "3";
      point[QStringLiteral( "PointSourceId" )] = "15017";
      point[QStringLiteral( "Red" )] = "0";
      point[QStringLiteral( "Reflectance" )] = "-10.550000190734863";
      point[QStringLiteral( "ReturnNumber" )] = "1";
      point[QStringLiteral( "ScanAngleRank" )] = "-6";
      point[QStringLiteral( "ScanDirectionFlag" )] = "0";
      point[QStringLiteral( "UserData" )] = "0";
      point[QStringLiteral( "Synthetic" )] = "0";
      point[QStringLiteral( "KeyPoint" )] = "0";
      point[QStringLiteral( "Withheld" )] = "0";
      point[QStringLiteral( "Overlap" )] = "0";
      point[QStringLiteral( "X" )] = "527919.58";
      point[QStringLiteral( "Y" )] = "6210983.42";
      point[QStringLiteral( "Z" )] = "151.131";
      expectedPoints.push_back( point );
    }

    auto cmp = []( const QMap<QString, QVariant> &p1, const QMap<QString, QVariant> &p2 ) {
      return qgsVariantLessThan( p1.value( QStringLiteral( "X" ), 0 ), p2.value( QStringLiteral( "X" ), 0 ) );
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
  const QString dataPath = copyTestData( QStringLiteral( "/point_clouds/copc/lone-star.copc.laz" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
  QVERIFY( layer->isValid() );

  QgsPointCloudIndex index = layer->dataProvider()->index();
  QVERIFY( index.isValid() );

  QCOMPARE( index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "0-0-0-0" ) ) ).pointCount(), 56721 );
  QVERIFY( !index.hasNode( QgsPointCloudNodeId::fromString( QStringLiteral( "1-1-1-1" ) ) ) );
  QCOMPARE( index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "2-3-3-1" ) ) ).pointCount(), 446 );
  QVERIFY( !index.hasNode( QgsPointCloudNodeId::fromString( QStringLiteral( "9-9-9-9" ) ) ) );

  QCOMPARE( index.pointCount(), 518862 );
  QCOMPARE( index.zMin(), 2322.89625 );
  QCOMPARE( index.zMax(), 2338.5755 );
  QCOMPARE( index.scale().toVector3D(), QVector3D( 0.0001f, 0.0001f, 0.0001f ) );
  QCOMPARE( index.offset().toVector3D(), QVector3D( 515385, 4918361, 2330.5 ) );
  QCOMPARE( index.span(), 128 );

  QCOMPARE( index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "0-0-0-0" ) ) ).error(), 0.328125 );
  QCOMPARE( index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "2-3-3-1" ) ) ).error(), 0.08203125 );

  {
    QgsBox3D bounds = index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "0-0-0-0" ) ) ).bounds();
    QCOMPARE( bounds.xMinimum(), 515368 );
    QCOMPARE( bounds.yMinimum(), 4918340 );
    QCOMPARE( bounds.zMinimum(), 2322 );
    QCOMPARE( bounds.xMaximum(), 515410 );
    QCOMPARE( bounds.yMaximum(), 4918382 );
    QCOMPARE( bounds.zMaximum(), 2364 );
  }

  {
    QgsBox3D bounds = QgsPointCloudNode::bounds( index.rootNodeBounds(), QgsPointCloudNodeId::fromString( QStringLiteral( "1-1-1-1" ) ) );
    QCOMPARE( bounds.xMinimum(), 515389 );
    QCOMPARE( bounds.yMinimum(), 4918361 );
    QCOMPARE( bounds.zMinimum(), 2343 );
    QCOMPARE( bounds.xMaximum(), 515410 );
    QCOMPARE( bounds.yMaximum(), 4918382 );
    QCOMPARE( bounds.zMaximum(), 2364 );
  }

  {
    QgsBox3D bounds = index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "2-3-3-1" ) ) ).bounds();
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
  const QString dataPath = copyTestData( QStringLiteral( "/point_clouds/copc/extrabytes-dataset.copc.laz" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
  QgsPointCloudIndex index = layer->dataProvider()->index();
  QgsPointCloudStatsCalculator calculator( index );

  QVector<QgsPointCloudAttribute> attributes;
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "Deviation" ), QgsPointCloudAttribute::Float ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "Synthetic" ), QgsPointCloudAttribute::UChar ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "KeyPoint" ), QgsPointCloudAttribute::UChar ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "Withheld" ), QgsPointCloudAttribute::UChar ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "Overlap" ), QgsPointCloudAttribute::UChar ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "Red" ), QgsPointCloudAttribute::UShort ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "EdgeOfFlightLine" ), QgsPointCloudAttribute::Char ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "Blue" ), QgsPointCloudAttribute::UShort ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "GpsTime" ), QgsPointCloudAttribute::Double ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "Green" ), QgsPointCloudAttribute::UShort ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "ReturnNumber" ), QgsPointCloudAttribute::Char ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "NumberOfReturns" ), QgsPointCloudAttribute::Char ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "Intensity" ), QgsPointCloudAttribute::UShort ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "PointSourceId" ), QgsPointCloudAttribute::UShort ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "ScanDirectionFlag" ), QgsPointCloudAttribute::Char ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "UserData" ), QgsPointCloudAttribute::UChar ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "ScanAngleRank" ), QgsPointCloudAttribute::Float ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "Reflectance" ), QgsPointCloudAttribute::Float ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "Amplitude" ), QgsPointCloudAttribute::Float ) );
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "Classification" ), QgsPointCloudAttribute::Char ) );

  QgsFeedback feedback;

  calculator.calculateStats( &feedback, attributes );

  QgsPointCloudStatistics stats = calculator.statistics();

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "Amplitude" ) );
    QCOMPARE( ( float ) s.minimum, 1.1599999666214 );
    QCOMPARE( ( float ) s.maximum, 19.6000003814697 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "Blue" ) );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "Synthetic" ) );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 1 );
    QMap<int, int> classCount = s.classCount;
    QCOMPARE( classCount.size(), 2 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "KeyPoint" ) );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 1 );
    QMap<int, int> classCount = s.classCount;
    QCOMPARE( classCount.size(), 2 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "Withheld" ) );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 1 );
    QMap<int, int> classCount = s.classCount;
    QCOMPARE( classCount.size(), 2 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "Overlap" ) );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 1 );
    QMap<int, int> classCount = s.classCount;
    QCOMPARE( classCount.size(), 2 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "Classification" ) );
    QCOMPARE( ( float ) s.minimum, 2 );
    QCOMPARE( ( float ) s.maximum, 18 );
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
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "Deviation" ) );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 120 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "EdgeOfFlightLine" ) );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "GpsTime" ) );
    QCOMPARE( ( float ) s.minimum, ( float ) 302522581.972046196460723876953 );
    QCOMPARE( ( float ) s.maximum, ( float ) 302522583.437068104743957519531 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "Green" ) );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "Intensity" ) );
    QCOMPARE( ( float ) s.minimum, 116 );
    QCOMPARE( ( float ) s.maximum, 1960 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "NumberOfReturns" ) );
    QCOMPARE( ( float ) s.minimum, 1 );
    QCOMPARE( ( float ) s.maximum, 5 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "PointSourceId" ) );
    QCOMPARE( ( float ) s.minimum, 15017 );
    QCOMPARE( ( float ) s.maximum, 15017 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "Red" ) );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "Reflectance" ) );
    QCOMPARE( ( float ) s.minimum, -21.1100006103515625 );
    QCOMPARE( ( float ) s.maximum, -2.6099998950958251953125 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "ReturnNumber" ) );
    QCOMPARE( ( float ) s.minimum, 1 );
    QCOMPARE( ( float ) s.maximum, 5 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "ScanAngleRank" ) );
    QCOMPARE( ( float ) s.minimum, -10.998000145f );
    QCOMPARE( ( float ) s.maximum, -4.001999855f );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "ScanDirectionFlag" ) );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "UserData" ) );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
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

  QUrl url( QStringLiteral( "0.0.0.0/laz.copc.laz" ) );
  QgsRangeRequestCache cache;
  cache.setCacheDirectory( cacheDir.path() );
  cache.clear();
  cache.setCacheSize( 2 );

  cache.registerEntry( request( url, QStringLiteral( "bytes=1-2" ) ), QByteArray( 1, '0' ) );
  QTest::qSleep( 10 );

  cache.registerEntry( request( url, QStringLiteral( "bytes=3-4" ) ), QByteArray( 1, '1' ) );
  QTest::qSleep( 10 );

  cache.registerEntry( request( url, QStringLiteral( "bytes=5-6" ) ), QByteArray( 1, '2' ) );
  QTest::qSleep( 10 );

  // (5, 6) -> (3, 4)
  {
    QFileInfoList files = cache.cacheEntries();
    QCOMPARE( files.size(), 2 );
    QVERIFY( files[0].baseName().endsWith( QLatin1String( "bytes=5-6" ) ) );
    QVERIFY( files[1].baseName().endsWith( QLatin1String( "bytes=3-4" ) ) );
  }

  cache.entry( request( url, QStringLiteral( "bytes=3-4" ) ) );
  QTest::qSleep( 10 );

  // -> (3, 4) -> (5, 6)
  {
    QFileInfoList files = cache.cacheEntries();
    QCOMPARE( files.size(), 2 );
    QVERIFY( files[0].baseName().endsWith( QLatin1String( "bytes=3-4" ) ) );
    QVERIFY( files[1].baseName().endsWith( QLatin1String( "bytes=5-6" ) ) );
  }

  cache.registerEntry( request( url, QStringLiteral( "bytes=7-8" ) ), QByteArray( 1, '3' ) );
  QTest::qSleep( 10 );

  // (7, 8) -> (3, 4)
  {
    QFileInfoList files = cache.cacheEntries();
    QCOMPARE( files.size(), 2 );
    QVERIFY( files[0].baseName().endsWith( QLatin1String( "bytes=7-8" ) ) );
    QVERIFY( files[1].baseName().endsWith( QLatin1String( "bytes=3-4" ) ) );
  }

  cache.registerEntry( request( url, QStringLiteral( "bytes=9-10" ) ), QByteArray( 1, '4' ) );
  // (9, 10) -> (7, 8)
  {
    QFileInfoList files = cache.cacheEntries();
    QCOMPARE( files.size(), 2 );
    QVERIFY( files[0].baseName().endsWith( QLatin1String( "bytes=9-10" ) ) );
    QVERIFY( files[1].baseName().endsWith( QLatin1String( "bytes=7-8" ) ) );
  }
}

void TestQgsCopcProvider::testSaveLoadStats()
{
  QgsPointCloudStatistics calculatedStats;
  QgsPointCloudStatistics readStats;
  const QString dataPath = copyTestData( QStringLiteral( "/point_clouds/copc/lone-star.copc.laz" ) );

  {
    std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
    QVERIFY( layer->isValid() );

    QVERIFY( layer->dataProvider() && layer->dataProvider()->isValid() && layer->dataProvider()->index() );
    QgsPointCloudIndex index = layer->dataProvider()->index();

    calculatedStats = layer->statistics();
    index.writeStatistics( calculatedStats );
  }

  {
    std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
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
  const QString dataPath = copyTestData( QStringLiteral( "/point_clouds/copc/lone-star.copc.laz" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
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
QGSTEST_MAIN( TestQgsCopcProvider )
#include "testqgscopcprovider.moc"
