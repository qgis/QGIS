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

/**
* \ingroup UnitTests
* This is a unit test for the virtualraster provider
*/


class TestQgsVirtualRasterProvider : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.

    void validLayer();
    //void testv();
    //void testRaster();
    //void testUrlDecoding();
    //void testUrlDecodingMinimal();
    void testUriProviderDecoding();
    void testUriEncoding();
    void testConstructorWrong();
    void testConstructor();

  private:
    QString mTestDataDir;
    QString mReport;
    QgsRasterLayer *mdemRasterLayer = nullptr;

};

//runs before all tests
void TestQgsVirtualRasterProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  mReport = QStringLiteral( "<h1>Virtual Raster Provider Tests</h1>\n" );

  QString demFileName = mTestDataDir + "raster/dem.tif";
  QFileInfo demRasterFileInfo( demFileName );

  mdemRasterLayer = new QgsRasterLayer( demRasterFileInfo.filePath(),
                                        demRasterFileInfo.completeBaseName() );
  /*
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mdemRasterLayer );
    */
}

void TestQgsVirtualRasterProvider::validLayer()
{
  QgsRasterLayer::LayerOptions options;

  std::unique_ptr< QgsRasterLayer > layer = std::make_unique< QgsRasterLayer >(
        mTestDataDir + QStringLiteral( "raster/dem.tif" ),
        QStringLiteral( "layer" ),
        QStringLiteral( "virtualrasterprovider" ),
        options
      );

  QVERIFY( ! layer->isValid() );

}

//runs after all tests
void TestQgsVirtualRasterProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

//void TestQgsVirtualRasterProvider::testv()
//{
/*
if ( mdemRasterLayer->extent().xMaximum()== 45.8117014376000000 )
{
    QgsDebugMsg("testv is starting");
}

*/

/* ------------------------------
    QgsRasterCalculatorEntry entry1;
    entry1.bandNumber = 1;
    entry1.raster = mdemRasterLayer;
    entry1.ref = QStringLiteral( "dem@1" );

    QVector<QgsRasterCalculatorEntry> entries;
    entries << entry1;

    //QgsCoordinateReferenceSystem crs( QStringLiteral( "EPSG:32633" ) );
    QgsCoordinateReferenceSystem mOutputCrs( QStringLiteral( "EPSG:4326" ) );
    QgsRectangle extent(18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000);
*/ //----------------------------------------

/*
QgsRectangle extent(18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000);
QString demFileName = "/home/franc/dev/cpp/QGIS/tests/testdata/raster/dem.tif";
QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "virtualrasterprovider" ), demFileName, QgsDataProvider::ProviderOptions() );
QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
QVERIFY( rp );
QVERIFY( rp->isValid() );
if ( rp )
  {
    std::unique_ptr<QgsRasterBlock> block( rp->block( 1, extent, 373, 350 ) );

    qDebug() << "VALUE BLOCK at row 0, col 0: " << block->value( 0, 0 );
    qDebug() << "VALUE BLOCK at  row 350, col 373: " << block->value(349,372);
    qDebug() << "bandCount result: " << rp->bandCount();
    QVERIFY( block );
    QCOMPARE( block->width(),  373 );
    QCOMPARE( block->height(), 350 );

    QCOMPARE( block->value( 0, 0 ), 292.86041259765625 );

    QCOMPARE( rp->bandCount(), 1 );
  }
delete provider;
*/

//}

//void TestQgsVirtualRasterProvider::testRaster()
//{
/*
QgsRasterLayer *r = new QgsRasterLayer("/home/franc/dev/cpp/QGIS/tests/testdata/raster/dem.tif","dem","virtualrasterprovider");
QgsDebugMsg("NAME of LAYER: "+r->name());

double sampledValue= r->dataProvider()->sample(QgsPointXY(18.67714, 45.79202),1);

qDebug() <<"VALUE IS "<< sampledValue;

delete r;
*/

//}

void TestQgsVirtualRasterProvider::testUriProviderDecoding()
{

  QgsRasterDataProvider::DecodedUriParameters decodedParams = QgsVirtualRasterProvider::decodeVirtualRasterProviderUri( QStringLiteral( "?crs=EPSG:4326&extent=18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000&width=373&height=350&formula=\"dem@1\" + 200&dem:uri=path/to/file&dem:provider=gdal&landsat:uri=path/to/landsat&landsat:provider=gdal" ) );


  QCOMPARE( decodedParams.width, 373 );
  QCOMPARE( decodedParams.height, 350 );
  QCOMPARE( decodedParams.extent, QgsRectangle( 18.6662979442000001, 45.7767014376000034, 18.7035979441999984, 45.8117014376000000 ) );
  QCOMPARE( decodedParams.crs, QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  QCOMPARE( decodedParams.formula, QStringLiteral( "\"dem@1\" + 200" ) );


  QCOMPARE( decodedParams.rInputLayers.at( 1 ).provider, QStringLiteral( "gdal" ) );
  QCOMPARE( decodedParams.rInputLayers.at( 0 ).provider, QStringLiteral( "gdal" ) );

  qDebug() << endl;
  qDebug() << QStringLiteral( "Raster layer: name, uri, provider" );
  for ( int i = 0; i < decodedParams.rInputLayers.size() ; ++i )
  {
    qDebug() << decodedParams.rInputLayers.at( i ).name << ", " <<
             decodedParams.rInputLayers.at( i ).uri  << ", " <<
             decodedParams.rInputLayers.at( i ).provider;
  }
  qDebug() << endl;

}

void TestQgsVirtualRasterProvider::testUriEncoding()
{

  QgsRasterDataProvider::DecodedUriParameters decodedParams = QgsVirtualRasterProvider::decodeVirtualRasterProviderUri( QStringLiteral( "?crs=EPSG:4326&extent=18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000&width=373&height=350&formula=\"dem@1\" + 200&dem:uri=/home/franc/dev/cpp/QGIS/tests/testdata/raster/dem.tif&dem:provider=gdal&rband:uri=/home/franc/dev/cpp/QGIS/tests/testdata/raster/band1_byte_ct_epsg4326.tif&rband:provider=gdal" ) );
  qDebug() << QgsVirtualRasterProvider::encodeVirtualRasterProviderUri( decodedParams ) << endl;

}

void TestQgsVirtualRasterProvider::testConstructorWrong()
{
  //Giving an invalid uri, with more raster referencies compared to the raster.ref that are present in the formula
  QString uri = QStringLiteral( "?crs=EPSG:4326&extent=18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000&width=373&height=350&formula=\"dem@1\" + 200&dem:uri=/home/franc/dev/cpp/QGIS/tests/testdata/raster/dem.tif&dem:provider=gdal&landsat:uri=/home/franc/dev/cpp/QGIS/tests/testdata/landsat.tif&landsat:provider=gdal" );
  std::unique_ptr< QgsRasterLayer > layer = std::make_unique< QgsRasterLayer >( uri,
      QStringLiteral( "layer" ),
      QStringLiteral( "virtualrasterprovider" ) );

  if ( layer->dataProvider()->isValid() )
  {
    QVERIFY( layer->dataProvider()->isValid() );
    QVERIFY( layer->isValid() );

  }
  else
  {
    QVERIFY( ! layer->dataProvider()->isValid() );
    QVERIFY( ! layer->isValid() );
    qDebug() << QStringLiteral( "The dataprovider is not valid" );
  }

}

void TestQgsVirtualRasterProvider::testConstructor()
{
  QString uri = QStringLiteral( "?crs=EPSG:4326&extent=18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000&width=373&height=350&formula=\"dem@1\" + 200&dem:uri=/home/franc/dev/cpp/QGIS/tests/testdata/raster/dem.tif&dem:provider=gdal" );

  std::unique_ptr< QgsRasterLayer > layer = std::make_unique< QgsRasterLayer >( uri,
      QStringLiteral( "layer" ),
      QStringLiteral( "virtualrasterprovider" ) );

  if ( layer->dataProvider()->isValid() )
  {
    QVERIFY( layer->dataProvider()->isValid() );
    QVERIFY( layer->isValid() );

    double sampledValueCalc = layer->dataProvider()->sample( QgsPointXY( 18.67714, 45.79202 ), 1 );
    double sampledValue = mdemRasterLayer->dataProvider()->sample( QgsPointXY( 18.67714, 45.79202 ), 1 );

    QCOMPARE( sampledValueCalc, sampledValue + 200. );
    QCOMPARE( layer->dataProvider()->crs(), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

    qDebug() << QStringLiteral( "The computed value at random point X Y is: " ) << sampledValueCalc;
  }


}

QGSTEST_MAIN( TestQgsVirtualRasterProvider )
#include "testqgsvirtualrasterprovider.moc"
