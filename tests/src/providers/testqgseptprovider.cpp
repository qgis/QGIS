/***************************************************************************
     testqgseptprovider.cpp
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
#include <fstream>
#include <QVector>
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
#include "qgsprovidermetadata.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the EPT provider
 */
class TestQgsEptProvider : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsEptProvider()
      : QgsTest( QStringLiteral( "EPT Provider Tests" ) ) {}

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
    void validLayerWithEptHierarchy();
    void attributes();
    void calculateZRange();
    void testIdentify_data();
    void testIdentify();
    void testExtraBytesAttributesExtraction();
    void testExtraBytesAttributesValues();
    void testPointCloudIndex();
    void testPointCloudRequest();
    void testStatsCalculator();
};

//runs before all tests
void TestQgsEptProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
}

//runs after all tests
void TestQgsEptProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsEptProvider::filters()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) );
  QVERIFY( metadata );

  QCOMPARE( metadata->filters( Qgis::FileFilterType::PointCloud ), QStringLiteral( "Entwine Point Clouds (ept.json EPT.JSON)" ) );
  QCOMPARE( metadata->filters( Qgis::FileFilterType::Vector ), QString() );

  const QString registryPointCloudFilters = QgsProviderRegistry::instance()->filePointCloudFilters();
  QVERIFY( registryPointCloudFilters.contains( "(ept.json EPT.JSON)" ) );
}

void TestQgsEptProvider::encodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) );
  QVERIFY( metadata );

  QVariantMap parts;
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "/home/point_clouds/ept.json" ) );
  QCOMPARE( metadata->encodeUri( parts ), QStringLiteral( "/home/point_clouds/ept.json" ) );
}

void TestQgsEptProvider::decodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) );
  QVERIFY( metadata );

  const QVariantMap parts = metadata->decodeUri( QStringLiteral( "/home/point_clouds/ept.json" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/point_clouds/ept.json" ) );
}

void TestQgsEptProvider::absoluteRelativeUri()
{
  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/project.qgs" ) ) );

  QgsProviderMetadata *eptMetadata = QgsProviderRegistry::instance()->providerMetadata( "ept" );
  QVERIFY( eptMetadata );

  QString absoluteUri = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/point_clouds/ept/rgb.json" );
  QString relativeUri = QStringLiteral( "./point_clouds/ept/rgb.json" );
  QCOMPARE( eptMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( eptMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );
}

void TestQgsEptProvider::preferredUri()
{
  QgsProviderMetadata *eptMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) );
  QVERIFY( eptMetadata->capabilities() & QgsProviderMetadata::PriorityForUri );

  // test that EPT is the preferred provider for ept.json uris
  QList<QgsProviderRegistry::ProviderCandidateDetails> candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/ept.json" ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), QStringLiteral( "ept" ) );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/EPT.JSON" ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), QStringLiteral( "ept" ) );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );

  QVERIFY( !QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( QStringLiteral( "/home/test/ept.json" ), QStringLiteral( "ept" ) ) );
  QVERIFY( QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( QStringLiteral( "/home/test/ept.json" ), QStringLiteral( "ogr" ) ) );
}

void TestQgsEptProvider::layerTypesForUri()
{
  QgsProviderMetadata *eptMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) );
  QVERIFY( eptMetadata->capabilities() & QgsProviderMetadata::LayerTypesForUri );

  QCOMPARE( eptMetadata->validLayerTypesForUri( QStringLiteral( "/home/test/ept.json" ) ), QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud );
  QCOMPARE( eptMetadata->validLayerTypesForUri( QStringLiteral( "/home/test/cloud.las" ) ), QList<Qgis::LayerType>() );
}

void TestQgsEptProvider::uriIsBlocklisted()
{
  QVERIFY( !QgsProviderRegistry::instance()->uriIsBlocklisted( QStringLiteral( "/home/nyall/ept.json" ) ) );
  QVERIFY( QgsProviderRegistry::instance()->uriIsBlocklisted( QStringLiteral( "/home/nyall/ept-build.json" ) ) );
}

void TestQgsEptProvider::querySublayers()
{
  // test querying sub layers for a ept layer
  QgsProviderMetadata *eptMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) );

  // invalid uri
  QList<QgsProviderSublayerDetails> res = eptMetadata->querySublayers( QString() );
  QVERIFY( res.empty() );

  // not a ept layer
  res = eptMetadata->querySublayers( QString( TEST_DATA_DIR ) + "/lines.shp" );
  QVERIFY( res.empty() );

  // valid ept layer
  const QString path = copyTestDataDirectory( QStringLiteral( "/point_clouds/ept/sunshine-coast" ) );
  res = eptMetadata->querySublayers( path + "/ept.json" );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "sunshine-coast" ) );
  QCOMPARE( res.at( 0 ).uri(), path + "/ept.json" );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "ept" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::PointCloud );

  // make sure result is valid to load layer from
  const QgsProviderSublayerDetails::LayerOptions options { QgsCoordinateTransformContext() };
  std::unique_ptr<QgsPointCloudLayer> ml( qgis::down_cast<QgsPointCloudLayer *>( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( ml->isValid() );
}

void TestQgsEptProvider::brokenPath()
{
  // test loading a bad layer URI
  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( QStringLiteral( "not valid" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
  QVERIFY( !layer->isValid() );
}

void TestQgsEptProvider::testLazInfo()
{
  const QString path = copyTestDataDirectory( QStringLiteral( "/point_clouds/ept/lone-star-laszip" ) );
  {
    QString dataPath = path + QStringLiteral( "/ept.json" );
    std::ifstream file( dataPath.toStdString(), std::ios::binary );
    QgsLazInfo lazInfo = QgsLazInfo::fromFile( file );
    QVERIFY( !lazInfo.isValid() );
  }
  {
    QString dataPath = path + QStringLiteral( "/ept-data/0-0-0-0.laz" );
    std::ifstream file( dataPath.toStdString(), std::ios::binary );
    QgsLazInfo lazInfo = QgsLazInfo::fromFile( file );
    QVERIFY( lazInfo.isValid() );
    QCOMPARE( lazInfo.pointCount(), 41998 );
    QCOMPARE( lazInfo.scale().toVector3D(), QVector3D( 0.00025f, 0.00025f, 0.00025f ) );
    QCOMPARE( lazInfo.offset().toVector3D(), QVector3D( 515385, 4918360, 2331 ) );
    QPair<uint16_t, uint16_t> creationYearDay = lazInfo.creationYearDay();
    QCOMPARE( creationYearDay.first, 2019 );
    QCOMPARE( creationYearDay.second, 172 );
    QPair<uint8_t, uint8_t> version = lazInfo.version();
    QCOMPARE( version.first, 1 );
    QCOMPARE( version.second, 2 );
    QCOMPARE( lazInfo.pointFormat(), 1 );
    QCOMPARE( lazInfo.systemId(), "PDAL" );
    QCOMPARE( lazInfo.softwareId(), QStringLiteral( "Entwine" ) );
    QCOMPARE( lazInfo.minCoords().toVector3D(), QVector3D( 515368.63225000002421439f, 4918340.36400000005960464f, 2322.90050000000019281f ) );
    QCOMPARE( lazInfo.maxCoords().toVector3D(), QVector3D( 515401.03749999997671694f, 4918381.10350000020116568f, 2338.56550000000015643f ) );
    QCOMPARE( lazInfo.firstPointRecordOffset(), 865 );
    QCOMPARE( lazInfo.firstVariableLengthRecord(), 227 );
    QCOMPARE( lazInfo.pointRecordLength(), 32 );
    QCOMPARE( lazInfo.extrabytesCount(), 4 );
  }
}


void TestQgsEptProvider::validLayer()
{
  const QString path = copyTestDataDirectory( QStringLiteral( "point_clouds/ept/sunshine-coast" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( path + QStringLiteral( "/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
  QVERIFY( layer->isValid() );

  QCOMPARE( layer->crs().authid(), QStringLiteral( "EPSG:28356" ) );
  QGSCOMPARENEAR( layer->extent().xMinimum(), 498061.0, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMinimum(), 7050992.0, 0.1 );
  QGSCOMPARENEAR( layer->extent().xMaximum(), 498068.0, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMaximum(), 7050998.0, 0.1 );
  QCOMPARE( layer->dataProvider()->polygonBounds().asWkt( 0 ), QStringLiteral( "Polygon ((498061 7050992, 498068 7050992, 498068 7050998, 498061 7050998, 498061 7050992))" ) );
  QCOMPARE( layer->dataProvider()->pointCount(), 253 );
  QCOMPARE( layer->pointCount(), 253 );

  QVERIFY( layer->dataProvider()->index() );
  // all hierarchy is stored in a single node
  QVERIFY( layer->dataProvider()->index().hasNode( QgsPointCloudNodeId::fromString( "0-0-0-0" ) ) );
  QVERIFY( !layer->dataProvider()->index().hasNode( QgsPointCloudNodeId::fromString( "1-0-0-0" ) ) );
}

void TestQgsEptProvider::validLayerWithEptHierarchy()
{
  const QString path = copyTestDataDirectory( QStringLiteral( "point_clouds/ept/lone-star-laszip" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( path + QStringLiteral( "/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
  QVERIFY( layer->isValid() );

  QGSCOMPARENEAR( layer->extent().xMinimum(), 515368.000000, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMinimum(), 4918340.000000, 0.1 );
  QGSCOMPARENEAR( layer->extent().xMaximum(), 515402.000000, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMaximum(), 4918382.000000, 0.1 );

  QVERIFY( layer->dataProvider()->index() );
  // all hierarchy is stored in multiple nodes
  QVERIFY( layer->dataProvider()->index().hasNode( QgsPointCloudNodeId::fromString( "1-1-1-1" ) ) );
  QVERIFY( layer->dataProvider()->index().hasNode( QgsPointCloudNodeId::fromString( "2-3-3-1" ) ) );
}

void TestQgsEptProvider::attributes()
{
  const QString path = copyTestDataDirectory( QStringLiteral( "point_clouds/ept/sunshine-coast" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( path + QStringLiteral( "/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
  QVERIFY( layer->isValid() );

  const QgsPointCloudAttributeCollection attributes = layer->attributes();
  QCOMPARE( attributes.count(), 16 );
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
  QCOMPARE( attributes.at( 8 ).type(), QgsPointCloudAttribute::Char );
  QCOMPARE( attributes.at( 9 ).name(), QStringLiteral( "ScanAngleRank" ) );
  QCOMPARE( attributes.at( 9 ).type(), QgsPointCloudAttribute::Float );
  QCOMPARE( attributes.at( 10 ).name(), QStringLiteral( "UserData" ) );
  QCOMPARE( attributes.at( 10 ).type(), QgsPointCloudAttribute::Char );
  QCOMPARE( attributes.at( 11 ).name(), QStringLiteral( "PointSourceId" ) );
  QCOMPARE( attributes.at( 11 ).type(), QgsPointCloudAttribute::UShort );
  QCOMPARE( attributes.at( 12 ).name(), QStringLiteral( "GpsTime" ) );
  QCOMPARE( attributes.at( 12 ).type(), QgsPointCloudAttribute::Double );
  QCOMPARE( attributes.at( 13 ).name(), QStringLiteral( "Red" ) );
  QCOMPARE( attributes.at( 13 ).type(), QgsPointCloudAttribute::UShort );
  QCOMPARE( attributes.at( 14 ).name(), QStringLiteral( "Green" ) );
  QCOMPARE( attributes.at( 14 ).type(), QgsPointCloudAttribute::UShort );
  QCOMPARE( attributes.at( 15 ).name(), QStringLiteral( "Blue" ) );
  QCOMPARE( attributes.at( 15 ).type(), QgsPointCloudAttribute::UShort );
}

void TestQgsEptProvider::calculateZRange()
{
  const QString path = copyTestDataDirectory( QStringLiteral( "point_clouds/ept/sunshine-coast" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( path + QStringLiteral( "/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
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

void TestQgsEptProvider::testIdentify_data()
{
  QTest::addColumn<QString>( "datasetPath" );

  QTest::newRow( "ept with bin" ) << QStringLiteral( "point_clouds/ept/sunshine-coast/" );
  QTest::newRow( "ept with laz" ) << QStringLiteral( "point_clouds/ept/sunshine-coast-laz/" );
}

void TestQgsEptProvider::testIdentify()
{
  QFETCH( QString, datasetPath );

  const QString path = copyTestDataDirectory( datasetPath );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( path + QStringLiteral( "/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );

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
    expected[QStringLiteral( "GpsTime" )] = 268793.37257748609409;
    expected[QStringLiteral( "Green" )] = 0;
    expected[QStringLiteral( "Intensity" )] = 1765;
    expected[QStringLiteral( "NumberOfReturns" )] = 1;
    expected[QStringLiteral( "PointSourceId" )] = 7041;
    expected[QStringLiteral( "Red" )] = 0;
    expected[QStringLiteral( "ReturnNumber" )] = 1;
    expected[QStringLiteral( "ScanAngleRank" )] = -28;
    expected[QStringLiteral( "ScanDirectionFlag" )] = 1;
    expected[QStringLiteral( "UserData" )] = 17;
    expected[QStringLiteral( "X" )] = 498062.52;
    expected[QStringLiteral( "Y" )] = 7050996.61;
    expected[QStringLiteral( "Z" )] = 75.0;
    QVERIFY( identifiedPoint == expected );
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
      point[QStringLiteral( "GpsTime" )] = "268793.3373408913";
      point[QStringLiteral( "Green" )] = "0";
      point[QStringLiteral( "Intensity" )] = "278";
      point[QStringLiteral( "NumberOfReturns" )] = "1";
      point[QStringLiteral( "PointSourceId" )] = "7041";
      point[QStringLiteral( "Red" )] = "0";
      point[QStringLiteral( "ReturnNumber" )] = "1";
      point[QStringLiteral( "ScanAngleRank" )] = "-28";
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
      QCOMPARE( identifiedPoints[0][k].toDouble(), expected[0][k].toDouble() );
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
      point[QStringLiteral( "GpsTime" )] = "268793.3813974548";
      point[QStringLiteral( "Green" )] = "0";
      point[QStringLiteral( "Intensity" )] = "1142";
      point[QStringLiteral( "NumberOfReturns" )] = "1";
      point[QStringLiteral( "PointSourceId" )] = "7041";
      point[QStringLiteral( "Red" )] = "0";
      point[QStringLiteral( "ReturnNumber" )] = "1";
      point[QStringLiteral( "ScanAngleRank" )] = "-28";
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
      point[QStringLiteral( "GpsTime" )] = "269160.5176644815";
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

    QVERIFY( identifiedPoints == expected );
  }
}

void TestQgsEptProvider::testExtraBytesAttributesExtraction()
{
  {
    const QString path = copyTestDataDirectory( QStringLiteral( "point_clouds/ept/extrabytes-dataset" ) );

    QString dataPath = path + QStringLiteral( "/ept-data/0-0-0-0.laz" );
    std::ifstream file( dataPath.toStdString(), std::ios::binary );
    QgsLazInfo lazInfo = QgsLazInfo::fromFile( file );
    QVector<QgsLazInfo::ExtraBytesAttributeDetails> attributes = lazInfo.extrabytes();
    QCOMPARE( attributes.size(), 4 );

    QCOMPARE( attributes[0].attribute, QStringLiteral( "Amplitude" ) );
    QCOMPARE( attributes[1].attribute, QStringLiteral( "Reflectance" ) );
    QCOMPARE( attributes[2].attribute, QStringLiteral( "ClassFlags" ) );
    QCOMPARE( attributes[3].attribute, QStringLiteral( "Deviation" ) );

    QCOMPARE( attributes[0].type, QgsPointCloudAttribute::Float );
    QCOMPARE( attributes[1].type, QgsPointCloudAttribute::Float );
    QCOMPARE( attributes[2].type, QgsPointCloudAttribute::UChar );
    QCOMPARE( attributes[3].type, QgsPointCloudAttribute::Float );

    QCOMPARE( attributes[0].size, 4 );
    QCOMPARE( attributes[1].size, 4 );
    QCOMPARE( attributes[2].size, 1 );
    QCOMPARE( attributes[3].size, 4 );

    QCOMPARE( attributes[0].offset, 43 );
    QCOMPARE( attributes[1].offset, 39 );
    QCOMPARE( attributes[2].offset, 38 );
    QCOMPARE( attributes[3].offset, 34 );
  }

  {
    const QString path = copyTestDataDirectory( QStringLiteral( "point_clouds/ept/no-extrabytes-dataset" ) );

    QString dataPath = path + QStringLiteral( "/ept-data/0-0-0-0.laz" );
    std::ifstream file( dataPath.toStdString(), std::ios::binary );
    QgsLazInfo lazInfo = QgsLazInfo::fromFile( file );
    QVector<QgsLazInfo::ExtraBytesAttributeDetails> attributes = lazInfo.extrabytes();
    QCOMPARE( attributes.size(), 0 );
  }
}

void TestQgsEptProvider::testExtraBytesAttributesValues()
{
  const QString path = copyTestDataDirectory( QStringLiteral( "point_clouds/ept/extrabytes-dataset" ) );

  QString dataPath = path + QStringLiteral( "/ept.json" );
  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
  QVERIFY( layer->isValid() );
  {
    const float maxErrorInMapCoords = 0.0015207174f;
    QPolygonF polygon;
    polygon.push_back( QPointF( 527919.2459517354, 6210983.5918774214 ) );
    polygon.push_back( QPointF( 527919.0742796324, 6210983.5918774214 ) );
    polygon.push_back( QPointF( 527919.0742796324, 6210983.4383113598 ) );
    polygon.push_back( QPointF( 527919.2459517354, 6210983.4383113598 ) );
    polygon.push_back( QPointF( 527919.2459517354, 6210983.5918774214 ) );

    const QVector<QMap<QString, QVariant>> identifiedPoints = layer->dataProvider()->identify( maxErrorInMapCoords, QgsGeometry::fromQPolygonF( polygon ) );

    QVector<QMap<QString, QVariant>> expectedPoints;
    {
      QMap<QString, QVariant> point;
      point[QStringLiteral( "Amplitude" )] = "4.409999847412109";
      point[QStringLiteral( "Blue" )] = "0";
      point[QStringLiteral( "Synthetic" )] = "0";
      point[QStringLiteral( "KeyPoint" )] = "0";
      point[QStringLiteral( "Withheld" )] = "0";
      point[QStringLiteral( "Overlap" )] = "0";
      point[QStringLiteral( "Classification" )] = "5";
      point[QStringLiteral( "Deviation" )] = "2";
      point[QStringLiteral( "EdgeOfFlightLine" )] = "0";
      point[QStringLiteral( "GpsTime" )] = "302522582.235838";
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
      point[QStringLiteral( "X" )] = "527919.18";
      point[QStringLiteral( "Y" )] = "6210983.47";
      point[QStringLiteral( "Z" )] = "149.341";
      expectedPoints.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[QStringLiteral( "Amplitude" )] = "14.170000076293945";
      point[QStringLiteral( "Blue" )] = "0";
      point[QStringLiteral( "Synthetic" )] = "0";
      point[QStringLiteral( "KeyPoint" )] = "0";
      point[QStringLiteral( "Withheld" )] = "0";
      point[QStringLiteral( "Overlap" )] = "0";
      point[QStringLiteral( "Classification" )] = "2";
      point[QStringLiteral( "Deviation" )] = "0";
      point[QStringLiteral( "EdgeOfFlightLine" )] = "0";
      point[QStringLiteral( "GpsTime" )] = "302522582.235839";
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

    QCOMPARE( identifiedPoints, expectedPoints );
  }
}

void TestQgsEptProvider::testPointCloudIndex()
{
  const QString path = copyTestDataDirectory( QStringLiteral( "point_clouds/ept/lone-star-laszip" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( path + QStringLiteral( "/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
  QVERIFY( layer->isValid() );

  QgsPointCloudIndex index = layer->dataProvider()->index();
  QVERIFY( index.isValid() );

  QCOMPARE( index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "0-0-0-0" ) ) ).pointCount(), 41998 );
  QCOMPARE( index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "1-1-1-1" ) ) ).pointCount(), 48879 );
  QCOMPARE( index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "2-3-3-1" ) ) ).pointCount(), 41734 );
  QVERIFY( !index.hasNode( QgsPointCloudNodeId::fromString( QStringLiteral( "9-9-9-9" ) ) ) );

  QCOMPARE( index.pointCount(), 518862 );
  QCOMPARE( index.zMin(), 2322 );
  QCOMPARE( index.zMax(), 2339 );
  QCOMPARE( index.scale().toVector3D(), QVector3D( 0.00025f, 0.00025f, 0.00025f ) );
  QCOMPARE( index.offset().toVector3D(), QVector3D( 515385, 4918361, 2331 ) );
  QCOMPARE( index.span(), 128 );

  QCOMPARE( index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "0-0-0-0" ) ) ).error(), 0.34375 );
  QCOMPARE( index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "1-1-1-1" ) ) ).error(), 0.171875 );
  QCOMPARE( index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "2-3-3-1" ) ) ).error(), 0.0859375 );

  {
    QgsBox3D bounds = index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "0-0-0-0" ) ) ).bounds();
    QCOMPARE( bounds.xMinimum(), 515363 );
    QCOMPARE( bounds.yMinimum(), 4918339 );
    QCOMPARE( bounds.zMinimum(), 2309 );
    QCOMPARE( bounds.xMaximum(), 515407 );
    QCOMPARE( bounds.yMaximum(), 4918383 );
    QCOMPARE( bounds.zMaximum(), 2353 );
  }

  {
    QgsBox3D bounds = index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "1-1-1-1" ) ) ).bounds();
    QCOMPARE( bounds.xMinimum(), 515385 );
    QCOMPARE( bounds.yMinimum(), 4918361 );
    QCOMPARE( bounds.zMinimum(), 2331 );
    QCOMPARE( bounds.xMaximum(), 515407 );
    QCOMPARE( bounds.yMaximum(), 4918383 );
    QCOMPARE( bounds.zMaximum(), 2353 );
  }

  {
    QgsBox3D bounds = index.getNode( QgsPointCloudNodeId::fromString( QStringLiteral( "2-3-3-1" ) ) ).bounds();
    QCOMPARE( bounds.xMinimum(), 515396 );
    QCOMPARE( bounds.yMinimum(), 4918372 );
    QCOMPARE( bounds.zMinimum(), 2320 );
    QCOMPARE( bounds.xMaximum(), 515407 );
    QCOMPARE( bounds.yMaximum(), 4918383 );
    QCOMPARE( bounds.zMaximum(), 2331 );
  }
}

void TestQgsEptProvider::testPointCloudRequest()
{
  const QString path = copyTestDataDirectory( QStringLiteral( "point_clouds/ept/lone-star-laszip" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( path + QStringLiteral( "/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
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
    std::unique_ptr<QgsPointCloudBlock> block( index.nodeData( node, request ) );
    count += block->pointCount();
  }
  QCOMPARE( count, layer->pointCount() );

  // Now let's repeat the counting with an extent
  QgsRectangle extent( 515390, 4918360, 515400, 4918370 );
  request.setFilterRect( extent );
  count = 0;
  for ( QgsPointCloudNodeId node : nodes )
  {
    std::unique_ptr<QgsPointCloudBlock> block( index.nodeData( node, request ) );
    count += block->pointCount();
  }
  QCOMPARE( count, 217600 );

  // Now let's repeat the counting with an extent away from the pointcloud
  extent = QgsRectangle( 0, 0, 1, 1 );
  request.setFilterRect( extent );
  count = 0;
  for ( QgsPointCloudNodeId node : nodes )
  {
    std::unique_ptr<QgsPointCloudBlock> block( index.nodeData( node, request ) );
    count += block->pointCount();
  }
  QCOMPARE( count, 0 );

  // An empty extent should fetch all points again
  count = 0;
  extent = QgsRectangle();
  request.setFilterRect( extent );
  for ( QgsPointCloudNodeId node : nodes )
  {
    std::unique_ptr<QgsPointCloudBlock> block( index.nodeData( node, request ) );
    count += block->pointCount();
  }
  QCOMPARE( count, layer->pointCount() );
}

void TestQgsEptProvider::testStatsCalculator()
{
  const QString path = copyTestDataDirectory( QStringLiteral( "point_clouds/ept/extrabytes-dataset" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( path + QStringLiteral( "/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
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
  attributes.append( QgsPointCloudAttribute( QStringLiteral( "UserData" ), QgsPointCloudAttribute::Char ) );
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
    QCOMPARE( ( float ) s.maximum, 0 );
    QMap<int, int> classCount = s.classCount;
    QCOMPARE( classCount.size(), 1 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "KeyPoint" ) );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
    QMap<int, int> classCount = s.classCount;
    QCOMPARE( classCount.size(), 1 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "Withheld" ) );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
    QMap<int, int> classCount = s.classCount;
    QCOMPARE( classCount.size(), 1 );
  }

  {
    QgsPointCloudAttributeStatistics s = stats.statisticsOf( QStringLiteral( "Overlap" ) );
    QCOMPARE( ( float ) s.minimum, 0 );
    QCOMPARE( ( float ) s.maximum, 0 );
    QMap<int, int> classCount = s.classCount;
    QCOMPARE( classCount.size(), 1 );
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
    QCOMPARE( ( float ) s.minimum, -11 );
    QCOMPARE( ( float ) s.maximum, -4 );
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

QGSTEST_MAIN( TestQgsEptProvider )
#include "testqgseptprovider.moc"
