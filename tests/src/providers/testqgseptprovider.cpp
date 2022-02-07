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

//qgis includes...
#include "qgis.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgseptprovider.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsgeometry.h"
#include "qgseptdecoder.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the EPT provider
 */
class TestQgsEptProvider : public QObject
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
    void preferredUri();
    void layerTypesForUri();
    void uriIsBlocklisted();
    void querySublayers();
    void brokenPath();
    void validLayer();
    void validLayerWithEptHierarchy();
    void attributes();
    void calculateZRange();
    void testIdentify_data();
    void testIdentify();
    void testExtraBytesAttributesExtraction();
    void testExtraBytesAttributesValues();
    void testPointCloudIndex();

  private:
    QString mTestDataDir;
    QString mReport;
};

//runs before all tests
void TestQgsEptProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  mReport = QStringLiteral( "<h1>EPT Provider Tests</h1>\n" );
}

//runs after all tests
void TestQgsEptProvider::cleanupTestCase()
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

void TestQgsEptProvider::filters()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) );
  QVERIFY( metadata );

  QCOMPARE( metadata->filters( QgsProviderMetadata::FilterType::FilterPointCloud ), QStringLiteral( "Entwine Point Clouds (ept.json EPT.JSON)" ) );
  QCOMPARE( metadata->filters( QgsProviderMetadata::FilterType::FilterVector ), QString() );

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

void TestQgsEptProvider::preferredUri()
{
  QgsProviderMetadata *eptMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) );
  QVERIFY( eptMetadata->capabilities() & QgsProviderMetadata::PriorityForUri );

  // test that EPT is the preferred provider for ept.json uris
  QList<QgsProviderRegistry::ProviderCandidateDetails> candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/ept.json" ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), QStringLiteral( "ept" ) );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList< QgsMapLayerType >() << QgsMapLayerType::PointCloudLayer );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/EPT.JSON" ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), QStringLiteral( "ept" ) );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList< QgsMapLayerType >() << QgsMapLayerType::PointCloudLayer );

  QVERIFY( !QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( QStringLiteral( "/home/test/ept.json" ), QStringLiteral( "ept" ) ) );
  QVERIFY( QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( QStringLiteral( "/home/test/ept.json" ), QStringLiteral( "ogr" ) ) );
}

void TestQgsEptProvider::layerTypesForUri()
{
  QgsProviderMetadata *eptMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) );
  QVERIFY( eptMetadata->capabilities() & QgsProviderMetadata::LayerTypesForUri );

  QCOMPARE( eptMetadata->validLayerTypesForUri( QStringLiteral( "/home/test/ept.json" ) ), QList< QgsMapLayerType >() << QgsMapLayerType::PointCloudLayer );
  QCOMPARE( eptMetadata->validLayerTypesForUri( QStringLiteral( "/home/test/cloud.las" ) ), QList< QgsMapLayerType >() );
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
  QList< QgsProviderSublayerDetails >res = eptMetadata->querySublayers( QString() );
  QVERIFY( res.empty() );

  // not a ept layer
  res = eptMetadata->querySublayers( QString( TEST_DATA_DIR ) + "/lines.shp" );
  QVERIFY( res.empty() );

  // valid ept layer
  res = eptMetadata->querySublayers( mTestDataDir + "/point_clouds/ept/sunshine-coast/ept.json" );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "sunshine-coast" ) );
  QCOMPARE( res.at( 0 ).uri(), mTestDataDir + "/point_clouds/ept/sunshine-coast/ept.json" );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "ept" ) );
  QCOMPARE( res.at( 0 ).type(), QgsMapLayerType::PointCloudLayer );

  // make sure result is valid to load layer from
  const QgsProviderSublayerDetails::LayerOptions options{ QgsCoordinateTransformContext() };
  std::unique_ptr< QgsPointCloudLayer > ml( qgis::down_cast< QgsPointCloudLayer * >( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( ml->isValid() );
}

void TestQgsEptProvider::brokenPath()
{
  // test loading a bad layer URI
  std::unique_ptr< QgsPointCloudLayer > layer = std::make_unique< QgsPointCloudLayer >( QStringLiteral( "not valid" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
  QVERIFY( !layer->isValid() );
}

void TestQgsEptProvider::validLayer()
{
  std::unique_ptr< QgsPointCloudLayer > layer = std::make_unique< QgsPointCloudLayer >( mTestDataDir + QStringLiteral( "point_clouds/ept/sunshine-coast/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
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
  QVERIFY( layer->dataProvider()->index()->hasNode( IndexedPointCloudNode::fromString( "0-0-0-0" ) ) );
  QVERIFY( !layer->dataProvider()->index()->hasNode( IndexedPointCloudNode::fromString( "1-0-0-0" ) ) );
}

void TestQgsEptProvider::validLayerWithEptHierarchy()
{
  std::unique_ptr< QgsPointCloudLayer > layer = std::make_unique< QgsPointCloudLayer >( mTestDataDir + QStringLiteral( "point_clouds/ept/lone-star-laszip/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
  QVERIFY( layer->isValid() );

  QGSCOMPARENEAR( layer->extent().xMinimum(), 515368.000000, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMinimum(), 4918340.000000, 0.1 );
  QGSCOMPARENEAR( layer->extent().xMaximum(), 515402.000000, 0.1 );
  QGSCOMPARENEAR( layer->extent().yMaximum(), 4918382.000000, 0.1 );

  QVERIFY( layer->dataProvider()->index() );
  // all hierarchy is stored in multiple nodes
  QVERIFY( layer->dataProvider()->index()->hasNode( IndexedPointCloudNode::fromString( "1-1-1-1" ) ) );
  QVERIFY( layer->dataProvider()->index()->hasNode( IndexedPointCloudNode::fromString( "2-3-3-1" ) ) );
}

void TestQgsEptProvider::attributes()
{
  std::unique_ptr< QgsPointCloudLayer > layer = std::make_unique< QgsPointCloudLayer >( mTestDataDir + QStringLiteral( "point_clouds/ept/sunshine-coast/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
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
  std::unique_ptr< QgsPointCloudLayer > layer = std::make_unique< QgsPointCloudLayer >( mTestDataDir + QStringLiteral( "point_clouds/ept/sunshine-coast/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
  QVERIFY( layer->isValid() );

  QgsDoubleRange range = layer->elevationProperties()->calculateZRange( layer.get() );
  QGSCOMPARENEAR( range.lower(), 74.34, 0.01 );
  QGSCOMPARENEAR( range.upper(), 80.02, 0.01 );

  static_cast< QgsPointCloudLayerElevationProperties * >( layer->elevationProperties() )->setZScale( 2 );
  static_cast< QgsPointCloudLayerElevationProperties * >( layer->elevationProperties() )->setZOffset( 0.5 );

  range = layer->elevationProperties()->calculateZRange( layer.get() );
  QGSCOMPARENEAR( range.lower(), 149.18, 0.01 );
  QGSCOMPARENEAR( range.upper(), 160.54, 0.01 );
}

void TestQgsEptProvider::testIdentify_data()
{
  QTest::addColumn<QString>( "datasetPath" );

  QTest::newRow( "ept with bin" ) << mTestDataDir + QStringLiteral( "point_clouds/ept/sunshine-coast/ept.json" );
  QTest::newRow( "ept with laz" ) << mTestDataDir + QStringLiteral( "point_clouds/ept/sunshine-coast-laz/ept.json" );
}

void TestQgsEptProvider::testIdentify()
{
  QFETCH( QString, datasetPath );

  std::unique_ptr< QgsPointCloudLayer > layer = std::make_unique< QgsPointCloudLayer >( datasetPath, QStringLiteral( "layer" ), QStringLiteral( "ept" ) );

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

    expected[ QStringLiteral( "Blue" ) ] = 0;
    expected[ QStringLiteral( "Classification" ) ] = 2;
    expected[ QStringLiteral( "EdgeOfFlightLine" ) ] = 0;
    expected[ QStringLiteral( "GpsTime" ) ] = 268793.37257748609409;
    expected[ QStringLiteral( "Green" ) ] = 0;
    expected[ QStringLiteral( "Intensity" ) ] = 1765;
    expected[ QStringLiteral( "NumberOfReturns" ) ] = 1;
    expected[ QStringLiteral( "PointSourceId" ) ] = 7041;
    expected[ QStringLiteral( "Red" ) ] = 0;
    expected[ QStringLiteral( "ReturnNumber" ) ] = 1;
    expected[ QStringLiteral( "ScanAngleRank" ) ] = -28;
    expected[ QStringLiteral( "ScanDirectionFlag" ) ] = 1;
    expected[ QStringLiteral( "UserData" ) ] = 17;
    expected[ QStringLiteral( "X" ) ] = 498062.52;
    expected[ QStringLiteral( "Y" ) ] = 7050996.61;
    expected[ QStringLiteral( "Z" ) ] = 75.0;
    QVERIFY( identifiedPoint == expected );
  }

  // identify 1 point (circular point shape)
  {
    QPolygonF polygon;
    polygon.push_back( QPointF( 498066.28873652569018,  7050994.9709538575262 ) );
    polygon.push_back( QPointF( 498066.21890226693358,  7050995.0112726856023 ) );
    polygon.push_back( QPointF( 498066.21890226693358,  7050995.0919103417546 ) );
    polygon.push_back( QPointF( 498066.28873652569018,  7050995.1322291698307 ) );
    polygon.push_back( QPointF( 498066.35857078444678,  7050995.0919103417546 ) );
    polygon.push_back( QPointF( 498066.35857078444678,  7050995.0112726856023 ) );
    polygon.push_back( QPointF( 498066.28873652569018,  7050994.9709538575262 ) );
    const float maxErrorInMapCoords =  0.0091431681066751480103;
    const QVector<QMap<QString, QVariant>> identifiedPoints = layer->dataProvider()->identify( maxErrorInMapCoords, QgsGeometry::fromQPolygonF( polygon ) );
    QVector<QMap<QString, QVariant>> expected;
    {
      QMap<QString, QVariant> point;
      point[ QStringLiteral( "Blue" ) ] =  "0" ;
      point[ QStringLiteral( "Classification" ) ] =  "2" ;
      point[ QStringLiteral( "EdgeOfFlightLine" ) ] =  "0" ;
      point[ QStringLiteral( "GpsTime" ) ] =  "268793.3373408913" ;
      point[ QStringLiteral( "Green" ) ] =  "0" ;
      point[ QStringLiteral( "Intensity" ) ] =  "278" ;
      point[ QStringLiteral( "NumberOfReturns" ) ] =  "1" ;
      point[ QStringLiteral( "PointSourceId" ) ] =  "7041" ;
      point[ QStringLiteral( "Red" ) ] =  "0" ;
      point[ QStringLiteral( "ReturnNumber" ) ] =  "1" ;
      point[ QStringLiteral( "ScanAngleRank" ) ] =  "-28" ;
      point[ QStringLiteral( "ScanDirectionFlag" ) ] =  "1" ;
      point[ QStringLiteral( "UserData" ) ] =  "17" ;
      point[ QStringLiteral( "X" ) ] =  "498066.27" ;
      point[ QStringLiteral( "Y" ) ] =  "7050995.06" ;
      point[ QStringLiteral( "Z" ) ] =  "74.60" ;
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
      point[ QStringLiteral( "Blue" ) ] =  "0" ;
      point[ QStringLiteral( "Classification" ) ] =  "2" ;
      point[ QStringLiteral( "EdgeOfFlightLine" ) ] =  "0" ;
      point[ QStringLiteral( "GpsTime" ) ] =  "268793.3813974548" ;
      point[ QStringLiteral( "Green" ) ] =  "0" ;
      point[ QStringLiteral( "Intensity" ) ] =  "1142" ;
      point[ QStringLiteral( "NumberOfReturns" ) ] =  "1" ;
      point[ QStringLiteral( "PointSourceId" ) ] =  "7041" ;
      point[ QStringLiteral( "Red" ) ] =  "0" ;
      point[ QStringLiteral( "ReturnNumber" ) ] =  "1" ;
      point[ QStringLiteral( "ScanAngleRank" ) ] =  "-28" ;
      point[ QStringLiteral( "ScanDirectionFlag" ) ] =  "1" ;
      point[ QStringLiteral( "UserData" ) ] =  "17" ;
      point[ QStringLiteral( "X" ) ] =  "498063.14" ;
      point[ QStringLiteral( "Y" ) ] =  "7050996.79" ;
      point[ QStringLiteral( "Z" ) ] =  "74.89" ;
      expected.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[ QStringLiteral( "Blue" ) ] =  "0" ;
      point[ QStringLiteral( "Classification" ) ] =  "3" ;
      point[ QStringLiteral( "EdgeOfFlightLine" ) ] =  "0" ;
      point[ QStringLiteral( "GpsTime" ) ] =  "269160.5176644815" ;
      point[ QStringLiteral( "Green" ) ] =  "0" ;
      point[ QStringLiteral( "Intensity" ) ] =  "1631" ;
      point[ QStringLiteral( "NumberOfReturns" ) ] =  "1" ;
      point[ QStringLiteral( "PointSourceId" ) ] =  "7042" ;
      point[ QStringLiteral( "Red" ) ] =  "0" ;
      point[ QStringLiteral( "ReturnNumber" ) ] =  "1" ;
      point[ QStringLiteral( "ScanAngleRank" ) ] =  "-12" ;
      point[ QStringLiteral( "ScanDirectionFlag" ) ] =  "1" ;
      point[ QStringLiteral( "UserData" ) ] =  "17" ;
      point[ QStringLiteral( "X" ) ] =  "498063.11" ;
      point[ QStringLiteral( "Y" ) ] =  "7050996.75" ;
      point[ QStringLiteral( "Z" ) ] =  "74.90" ;
      expected.push_back( point );
    }

    QVERIFY( identifiedPoints == expected );
  }
}

void TestQgsEptProvider::testExtraBytesAttributesExtraction()
{
  {
    QString dataPath = mTestDataDir + QStringLiteral( "point_clouds/ept/extrabytes-dataset/ept-data/0-0-0-0.laz" );
    std::ifstream file( dataPath.toStdString(), std::ios::binary );
    QVector<QgsEptDecoder::ExtraBytesAttributeDetails> attributes = QgsEptDecoder::readExtraByteAttributes<std::ifstream>( file );
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
    QString dataPath = mTestDataDir + QStringLiteral( "point_clouds/ept/no-extrabytes-dataset/ept-data/0-0-0-0.laz" );
    std::ifstream file( dataPath.toStdString(), std::ios::binary );
    QVector<QgsEptDecoder::ExtraBytesAttributeDetails> attributes = QgsEptDecoder::readExtraByteAttributes<std::ifstream>( file );
    QCOMPARE( attributes.size(), 0 );
  }
}

void TestQgsEptProvider::testExtraBytesAttributesValues()
{
  QString dataPath = mTestDataDir + QStringLiteral( "point_clouds/ept/extrabytes-dataset/ept.json" );
  std::unique_ptr< QgsPointCloudLayer > layer = std::make_unique< QgsPointCloudLayer >( dataPath, QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
  QVERIFY( layer->isValid() );
  {
    const float maxErrorInMapCoords = 0.0015207174f;
    QPolygonF polygon;
    polygon.push_back( QPointF( 527919.2459517354,   6210983.5918774214 ) );
    polygon.push_back( QPointF( 527919.0742796324,   6210983.5918774214 ) );
    polygon.push_back( QPointF( 527919.0742796324,   6210983.4383113598 ) );
    polygon.push_back( QPointF( 527919.2459517354,   6210983.4383113598 ) );
    polygon.push_back( QPointF( 527919.2459517354,   6210983.5918774214 ) );

    const QVector<QMap<QString, QVariant>> identifiedPoints = layer->dataProvider()->identify( maxErrorInMapCoords, QgsGeometry::fromQPolygonF( polygon ) );

    QVector<QMap<QString, QVariant>> expectedPoints;
    {
      QMap<QString, QVariant> point;
      point[ QStringLiteral( "Amplitude" ) ] =   "4.409999847412109"  ;
      point[ QStringLiteral( "Blue" ) ] =   "0"  ;
      point[ QStringLiteral( "ClassFlags" ) ] =   "0"  ;
      point[ QStringLiteral( "Classification" ) ] =   "5"  ;
      point[ QStringLiteral( "Deviation" ) ] =   "2"  ;
      point[ QStringLiteral( "EdgeOfFlightLine" ) ] =   "0"  ;
      point[ QStringLiteral( "GpsTime" ) ] =   "302522582.235838"  ;
      point[ QStringLiteral( "Green" ) ] =   "0"  ;
      point[ QStringLiteral( "Intensity" ) ] =   "441"  ;
      point[ QStringLiteral( "NumberOfReturns" ) ] =   "3"  ;
      point[ QStringLiteral( "PointSourceId" ) ] =   "15017"  ;
      point[ QStringLiteral( "Red" ) ] =   "0"  ;
      point[ QStringLiteral( "Reflectance" ) ] =   "-17.829999923706055"  ;
      point[ QStringLiteral( "ReturnNumber" ) ] =   "2"  ;
      point[ QStringLiteral( "ScanAngleRank" ) ] =   "-6"  ;
      point[ QStringLiteral( "ScanDirectionFlag" ) ] =   "0"  ;
      point[ QStringLiteral( "UserData" ) ] =   "0"  ;
      point[ QStringLiteral( "X" ) ] =   "527919.18"  ;
      point[ QStringLiteral( "Y" ) ] =   "6210983.47"  ;
      point[ QStringLiteral( "Z" ) ] =   "149.341"  ;
      expectedPoints.push_back( point );
    }
    {
      QMap<QString, QVariant> point;
      point[ QStringLiteral( "Amplitude" ) ] =   "14.170000076293945"  ;
      point[ QStringLiteral( "Blue" ) ] =   "0"  ;
      point[ QStringLiteral( "ClassFlags" ) ] =   "0"  ;
      point[ QStringLiteral( "Classification" ) ] =   "2"  ;
      point[ QStringLiteral( "Deviation" ) ] =   "0"  ;
      point[ QStringLiteral( "EdgeOfFlightLine" ) ] =   "0"  ;
      point[ QStringLiteral( "GpsTime" ) ] =   "302522582.235839"  ;
      point[ QStringLiteral( "Green" ) ] =   "0"  ;
      point[ QStringLiteral( "Intensity" ) ] =   "1417"  ;
      point[ QStringLiteral( "NumberOfReturns" ) ] =   "3"  ;
      point[ QStringLiteral( "PointSourceId" ) ] =   "15017"  ;
      point[ QStringLiteral( "Red" ) ] =   "0"  ;
      point[ QStringLiteral( "Reflectance" ) ] =   "-8.050000190734863"  ;
      point[ QStringLiteral( "ReturnNumber" ) ] =   "3"  ;
      point[ QStringLiteral( "ScanAngleRank" ) ] =   "-6"  ;
      point[ QStringLiteral( "ScanDirectionFlag" ) ] =   "0"  ;
      point[ QStringLiteral( "UserData" ) ] =   "0"  ;
      point[ QStringLiteral( "X" ) ] =   "527919.11"  ;
      point[ QStringLiteral( "Y" ) ] =   "6210983.55"  ;
      point[ QStringLiteral( "Z" ) ] =   "147.111"  ;
      expectedPoints.push_back( point );
    }

    QCOMPARE( identifiedPoints, expectedPoints );
  }
}

void TestQgsEptProvider::testPointCloudIndex()
{
  std::unique_ptr< QgsPointCloudLayer > layer = std::make_unique< QgsPointCloudLayer >( mTestDataDir + QStringLiteral( "point_clouds/ept/lone-star-laszip/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
  QVERIFY( layer->isValid() );

  QgsPointCloudIndex *index = layer->dataProvider()->index();
  QVERIFY( index->isValid() );

  QVERIFY( index->nodePointCount( IndexedPointCloudNode::fromString( QStringLiteral( "0-0-0-0" ) ) ) == 41998 );
  QVERIFY( index->nodePointCount( IndexedPointCloudNode::fromString( QStringLiteral( "1-1-1-1" ) ) ) == 48879 );
  QVERIFY( index->nodePointCount( IndexedPointCloudNode::fromString( QStringLiteral( "2-3-3-1" ) ) ) == 41734 );
  QVERIFY( index->nodePointCount( IndexedPointCloudNode::fromString( QStringLiteral( "9-9-9-9" ) ) ) == -1 );

  QVERIFY( index->pointCount() == 518862 );
  QVERIFY( index->zMin() == 2322 );
  QVERIFY( index->zMax() == 2339 );
  QVERIFY( index->scale().toVector3D() == QVector3D( 0.00025f, 0.00025f, 0.00025f ) );
  QVERIFY( index->offset().toVector3D() == QVector3D( 515385, 4918361, 2331 ) );
  QVERIFY( index->span() == 128 );

  QVERIFY( index->nodeError( IndexedPointCloudNode::fromString( QStringLiteral( "0-0-0-0" ) ) ) == 0.34375 );
  QVERIFY( index->nodeError( IndexedPointCloudNode::fromString( QStringLiteral( "1-1-1-1" ) ) ) == 0.171875 );
  QVERIFY( index->nodeError( IndexedPointCloudNode::fromString( QStringLiteral( "2-3-3-1" ) ) ) == 0.0859375 );

  {
    QgsPointCloudDataBounds bounds = index->nodeBounds( IndexedPointCloudNode::fromString( QStringLiteral( "0-0-0-0" ) ) );
    QVERIFY( bounds.xMin() == -88000 );
    QVERIFY( bounds.yMin() == -88000 );
    QVERIFY( bounds.zMin() == -88000 );
    QVERIFY( bounds.xMax() ==  88000 );
    QVERIFY( bounds.yMax() ==  88000 );
    QVERIFY( bounds.zMax() ==  88000 );
  }

  {
    QgsPointCloudDataBounds bounds = index->nodeBounds( IndexedPointCloudNode::fromString( QStringLiteral( "1-1-1-1" ) ) );
    QVERIFY( bounds.xMin() == 0 );
    QVERIFY( bounds.yMin() == 0 );
    QVERIFY( bounds.zMin() == 0 );
    QVERIFY( bounds.xMax() == 88000 );
    QVERIFY( bounds.yMax() == 88000 );
    QVERIFY( bounds.zMax() == 88000 );
  }

  {
    QgsPointCloudDataBounds bounds = index->nodeBounds( IndexedPointCloudNode::fromString( QStringLiteral( "2-3-3-1" ) ) );
    QVERIFY( bounds.xMin() == 44000 );
    QVERIFY( bounds.yMin() == 44000 );
    QVERIFY( bounds.zMin() == -44000 );
    QVERIFY( bounds.xMax() == 88000 );
    QVERIFY( bounds.yMax() == 88000 );
    QVERIFY( bounds.zMax() == 0 );
  }
}

QGSTEST_MAIN( TestQgsEptProvider )
#include "testqgseptprovider.moc"
