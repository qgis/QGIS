/***************************************************************************
    testqgsvirtualrasterprovider.cpp
    --------------------------------------
   Date                 : June 2021
   Copyright            : (C) 2021 by Francesco Bursi
   Email                : francesco.bursi@hotmail.it
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
#include <qgis.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsrasterdataprovider.h>
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"
#include <qgsrectangle.h>
#include "qgsproject.h"
#include "qgsrastercalculator.h"
#include "qgsrastercalcnode.h"

#include "qgsvirtualrasterprovider.h"

#include <QUrl>
#include <QUrlQuery>
#include <QTemporaryDir>

/**
* \ingroup UnitTests
* This is a unit test for the virtualraster provider
*/


class TestQgsVirtualRasterProvider : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsVirtualRasterProvider()
      : QgsTest( QStringLiteral( "Virtual Raster Provider Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void validLayer();
    void testUriProviderDecoding();
    void testUriEncoding();
    void absoluteRelativeUri();
    void testConstructorWrong();
    void testConstructor();
    void testNewCalcNodeMethods();
    void testSecondGenerationVirtualRaster();
    void testNoData();

  private:
    QString mTestDataDir;
    QgsRasterLayer *mDemRasterLayer = nullptr;
    QgsRasterLayer *mLandsatRasterLayer = nullptr;
    QgsRasterLayer *mNoDataRasterLayer = nullptr;
};

//runs before all tests
void TestQgsVirtualRasterProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt

  QString demFileName = mTestDataDir + "raster/dem.tif";
  QFileInfo demRasterFileInfo( demFileName );
  mDemRasterLayer = new QgsRasterLayer( demRasterFileInfo.filePath(), demRasterFileInfo.completeBaseName() );

  QString landsatFileName = mTestDataDir + "landsat.tif";
  QFileInfo landsatRasterFileInfo( landsatFileName );
  mLandsatRasterLayer = new QgsRasterLayer( landsatRasterFileInfo.filePath(), landsatRasterFileInfo.completeBaseName() );

  QString nodataFileName = mTestDataDir + "raster/no_data.tif";
  QFileInfo nodataRasterFileInfo( nodataFileName );
  mNoDataRasterLayer = new QgsRasterLayer( nodataRasterFileInfo.filePath(), nodataRasterFileInfo.completeBaseName() );
}

void TestQgsVirtualRasterProvider::validLayer()
{
  QgsRasterLayer::LayerOptions options;

  std::unique_ptr<QgsRasterLayer> layer = std::make_unique<QgsRasterLayer>(
    mTestDataDir + QStringLiteral( "raster/dem.tif" ),
    QStringLiteral( "layer" ),
    QStringLiteral( "virtualraster" ),
    options
  );

  QVERIFY( !layer->isValid() );
}

//runs after all tests
void TestQgsVirtualRasterProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsVirtualRasterProvider::testUriProviderDecoding()
{
  QgsRasterDataProvider::VirtualRasterParameters decodedParams = QgsVirtualRasterProvider::decodeVirtualRasterProviderUri( QStringLiteral( "?crs=EPSG:4326&extent=18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000&width=373&height=350&formula=\"dem@1\" + 200&dem:uri=path/to/file&dem:provider=gdal&landsat:uri=path/to/landsat&landsat:provider=gdal" ) );

  QCOMPARE( decodedParams.width, 373 );
  QCOMPARE( decodedParams.height, 350 );
  QCOMPARE( decodedParams.extent, QgsRectangle( 18.6662979442000001, 45.7767014376000034, 18.7035979441999984, 45.8117014376000000 ) );
  QCOMPARE( decodedParams.crs, QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  QCOMPARE( decodedParams.formula, QStringLiteral( "\"dem@1\" + 200" ) );

  QCOMPARE( decodedParams.rInputLayers.at( 0 ).name, QStringLiteral( "dem" ) );
  QCOMPARE( decodedParams.rInputLayers.at( 1 ).name, QStringLiteral( "landsat" ) );
  QCOMPARE( decodedParams.rInputLayers.at( 0 ).uri, QStringLiteral( "path/to/file" ) );
  QCOMPARE( decodedParams.rInputLayers.at( 1 ).uri, QStringLiteral( "path/to/landsat" ) );
  QCOMPARE( decodedParams.rInputLayers.at( 0 ).provider, QStringLiteral( "gdal" ) );
  QCOMPARE( decodedParams.rInputLayers.at( 1 ).provider, QStringLiteral( "gdal" ) );
}

void TestQgsVirtualRasterProvider::testUriEncoding()
{
  QgsRasterDataProvider::VirtualRasterParameters params;

  params.crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
  params.extent = QgsRectangle( 18.5, 45.5, 19.5, 45.5 );
  params.width = 1000;
  params.height = 1500;
  params.formula = QString( "\"test_raster@1\"" );

  QgsRasterDataProvider::VirtualRasterInputLayers rasterParams;
  rasterParams.name = QString( "test_raster" );
  rasterParams.uri = QString( "path/to/file" );
  rasterParams.provider = QString( "test_provider" );
  params.rInputLayers.append( rasterParams );

  QString expecetedEncodedUri( QStringLiteral( "?crs=EPSG:4326&extent=18.5,45.5,19.5,45.5&width=1000&height=1500&formula=%22test_raster@1%22&test_raster:uri=path/to/file&test_raster:provider=test_provider" ) );
  QCOMPARE( QUrl::fromPercentEncoding( QgsVirtualRasterProvider::encodeVirtualRasterProviderUri( params ).toUtf8() ), expecetedEncodedUri );
}

void TestQgsVirtualRasterProvider::absoluteRelativeUri()
{
  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/project.qgs" ) ) );

  QString uriAbs = "?crs=EPSG:32633&"
                   "extent=781662.375,3339523.125,793062.375,3350923.125&"
                   "width=200&"
                   "height=200&"
                   "formula=%22landsat@1%22+1&"
                   "landsat:uri="
                   + QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif&"
                                                       "landsat:provider=gdal";

  QString uriRel = "?crs=EPSG:32633&"
                   "extent=781662.375,3339523.125,793062.375,3350923.125&"
                   "width=200&"
                   "height=200&"
                   "formula=%22landsat@1%22+1&"
                   "landsat:uri=./landsat.tif&"
                   "landsat:provider=gdal";

  QgsProviderMetadata *vrMetadata = QgsProviderRegistry::instance()->providerMetadata( "virtualraster" );
  QVERIFY( vrMetadata );

  QString absoluteUri = QUrl::toPercentEncoding( uriAbs );
  QString relativeUri = QUrl::toPercentEncoding( uriRel );
  QCOMPARE( vrMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( vrMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );
}

void TestQgsVirtualRasterProvider::testConstructorWrong()
{
  //Giving an invalid uri, with more raster referencies compared to the raster.ref that are present in the formula
  QString str1 = QStringLiteral( "?crs=EPSG:4326&extent=18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000&width=373&height=350&formula=\"dem@1\" + 200&dem:provider=gdal&landsat:provider=gdal" );
  QString uri = QString( "%1&%2&%3" ).arg( str1, QStringLiteral( "dem:uri=" ) % mTestDataDir % QStringLiteral( "raster/dem.tif" ), QStringLiteral( "landsat:uri=" ) % mTestDataDir % QStringLiteral( "landsat.tif" ) );
  std::unique_ptr<QgsRasterLayer> layer = std::make_unique<QgsRasterLayer>( uri, QStringLiteral( "layer" ), QStringLiteral( "virtualraster" ) );

  QVERIFY( !layer->isValid() );
}

void TestQgsVirtualRasterProvider::testConstructor()
{
  QString str1 = QStringLiteral( "?crs=EPSG:4326&extent=18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000&width=373&height=350&formula=\"dem@1\" + 200&dem:provider=gdal" );
  QString uri1 = QString( "%1&%2" ).arg( str1, QStringLiteral( "dem:uri=" ) % mTestDataDir % QStringLiteral( "raster/dem.tif" ) );
  std::unique_ptr<QgsRasterLayer> layer_1 = std::make_unique<QgsRasterLayer>( uri1, QStringLiteral( "layer_1" ), QStringLiteral( "virtualraster" ) );

  QVERIFY( layer_1->dataProvider()->isValid() );
  QVERIFY( layer_1->isValid() );

  double sampledValueCalc_1 = layer_1->dataProvider()->sample( QgsPointXY( 18.67714, 45.79202 ), 1 );
  double sampledValue = mDemRasterLayer->dataProvider()->sample( QgsPointXY( 18.67714, 45.79202 ), 1 );

  QCOMPARE( sampledValueCalc_1, sampledValue + 200. );
  QCOMPARE( layer_1->dataProvider()->crs(), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  QTemporaryDir dir;
  const QString landsatPath = dir.filePath( QStringLiteral( "landsat.tif" ) );
  QVERIFY( QFile::copy( mTestDataDir + "landsat.tif", landsatPath ) );
  // remove nodata values from layer for consistent test results
  std::unique_ptr<QgsRasterLayer> landsat = std::make_unique<QgsRasterLayer>( landsatPath, QString(), QStringLiteral( "gdal" ) );
  QVERIFY( landsat->isValid() );
  landsat->dataProvider()->setNoDataValue( 1, -999999 );
  landsat->dataProvider()->setNoDataValue( 2, -999999 );
  landsat.reset();

  QString str2 = QStringLiteral( "?crs=EPSG:32633&extent=781662.375,3339523.125,793062.375,3350923.125&width=200&height=200&formula=\"landsat@1\" + \"landsat@2\"&landsat:provider=gdal" );
  QString uri2 = QString( "%1&%2" ).arg( str2, QStringLiteral( "landsat:uri=" ) % landsatPath );
  std::unique_ptr<QgsRasterLayer> layer_2 = std::make_unique<QgsRasterLayer>( uri2, QStringLiteral( "layer_2" ), QStringLiteral( "virtualraster" ) );

  QVERIFY( layer_2->isValid() );
  QVERIFY( layer_2->dataProvider()->isValid() );
  QCOMPARE( layer_2->dataProvider()->crs(), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) ) );
  double sampledValueCalc_2 = layer_2->dataProvider()->sample( QgsPointXY( 790688, 3349113 ), 1 );
  QCOMPARE( sampledValueCalc_2, 267. );

  //use wrong formula
  QString str3 = QStringLiteral( "?crs=EPSG:32633&extent=781662.375,3339523.125,793062.375,3350923.125&width=200&height=200&formula=\"landsat@1\" xxxxxx+ \"landsat@2\"&landsat:provider=gdal" );
  QString uri3 = QString( "%1&%2" ).arg( str3, QStringLiteral( "landsat:uri=" ) % landsatPath );
  std::unique_ptr<QgsRasterLayer> layer_3 = std::make_unique<QgsRasterLayer>( uri3, QStringLiteral( "layer_3" ), QStringLiteral( "virtualraster" ) );
  QVERIFY( !layer_3->isValid() );
}

void TestQgsVirtualRasterProvider::testNewCalcNodeMethods()
{
  QString formula( "\"landsat@1\" + \"landsat@2\"-\"landsat@3\"" );
  QString errorString;
  std::unique_ptr<QgsRasterCalcNode> calcNodeApp( QgsRasterCalcNode::parseRasterCalcString( formula, errorString ) );

  QStringList rLayers = calcNodeApp->referencedLayerNames();
  QStringList rasterRef = calcNodeApp->cleanRasterReferences();

  QCOMPARE( rLayers, QStringList( "landsat" ) );

  QString bandOne = QString( "landsat@1" );
  QString bandTwo = QString( "landsat@2" );
  QString bandThree = QString( "landsat@3" );
  QStringList rasterRefExpected;
  rasterRefExpected << bandOne << bandTwo << bandThree;

  QCOMPARE( rasterRef, rasterRefExpected );
}

void TestQgsVirtualRasterProvider::testSecondGenerationVirtualRaster()
{
  // creation of the "first generation" virtual raster, meaning a virtual raster that comes directly from a file
  QString str = QStringLiteral( "?crs=EPSG:4326&extent=18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000&width=373&height=350&formula=\"dem@1\" + 200&dem:provider=gdal" );
  QString uri = QString( "%1&%2" ).arg( str, QStringLiteral( "dem:uri=" ) % mTestDataDir % QStringLiteral( "raster/dem.tif" ) );
  std::unique_ptr<QgsRasterLayer> layerFirst = std::make_unique<QgsRasterLayer>( uri, QStringLiteral( "firstGenerationLayer" ), QStringLiteral( "virtualraster" ) );
  QVERIFY( layerFirst->dataProvider()->isValid() );
  QVERIFY( layerFirst->isValid() );

  // creation of the "second generation" virtual raster uri, this raster is derived form the virtual raster called firstGenerationLayer
  QgsRasterDataProvider::VirtualRasterParameters params;

  params.crs = layerFirst->crs();
  params.extent = layerFirst->extent();
  params.width = 373;
  params.height = 350;
  params.formula = QString( "\"firstGenerationLayer@1\"" );

  QgsRasterDataProvider::VirtualRasterInputLayers rasterParams;
  rasterParams.name = layerFirst->name();
  rasterParams.uri = layerFirst->source();
  rasterParams.provider = layerFirst->dataProvider()->name();
  params.rInputLayers.append( rasterParams );

  QString uriSecond = QgsVirtualRasterProvider::encodeVirtualRasterProviderUri( params );
  std::unique_ptr<QgsRasterLayer> layerSecond = std::make_unique<QgsRasterLayer>( uriSecond, QStringLiteral( "SecondGenerationLayer" ), QStringLiteral( "virtualraster" ) );
  QVERIFY( layerSecond->dataProvider()->isValid() );
  QVERIFY( layerSecond->isValid() );

  double sampledValueCalc_1 = layerSecond->dataProvider()->sample( QgsPointXY( 18.67714, 45.79202 ), 1 );
  double sampledValue = mDemRasterLayer->dataProvider()->sample( QgsPointXY( 18.67714, 45.79202 ), 1 );

  QCOMPARE( sampledValueCalc_1, sampledValue + 200. );
  QCOMPARE( layerSecond->dataProvider()->crs(), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
}

void TestQgsVirtualRasterProvider::testNoData()
{
  double tlx = 415780.969;
  double tly = 759360.133;
  double cellSize = 0.1;

  // nodata
  //  0  NaN
  // NaN  1
  //  2  NaN
  QString str = QStringLiteral( "?crs=EPSG:2105&extent=415780.96899999998277053,759359.8330999999307096,415781.16899999999441206,759360.13309999997727573&width=2&height=3&formula=\"nodata@1\" ^2 / \"nodata@1\"&nodata:provider=gdal" );

  QString uri = QString( "%1&%2" ).arg( str, QStringLiteral( "nodata:uri=" ) % mTestDataDir % QStringLiteral( "raster/no_data.tif" ) );

  std::unique_ptr<QgsRasterLayer> layerNoData = std::make_unique<QgsRasterLayer>( uri, QStringLiteral( "no-data" ), QStringLiteral( "virtualraster" ) );
  QVERIFY( layerNoData->dataProvider()->isValid() );
  QVERIFY( layerNoData->isValid() );

  QgsPointXY p11( tlx + .5 * cellSize, tly - .5 * cellSize );
  QgsPointXY p12( tlx + 1.5 * cellSize, tly - .5 * cellSize );
  QgsPointXY p31( tlx + .5 * cellSize, tly - 2.5 * cellSize );

  Q_ASSERT( std::isnan( layerNoData->dataProvider()->sample( p11, 1 ) ) ); // 0^2/0
  Q_ASSERT( std::isnan( layerNoData->dataProvider()->sample( p12, 1 ) ) ); // NaN^2/NaN
  QCOMPARE( layerNoData->dataProvider()->sample( p31, 1 ), 2 );            // 2^2/2
}


QGSTEST_MAIN( TestQgsVirtualRasterProvider )
#include "testqgsvirtualrasterprovider.moc"
