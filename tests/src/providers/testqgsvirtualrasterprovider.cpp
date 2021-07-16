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
    void testUriProviderDecoding();
    void testUriEncoding();
    void testConstructorWrong();
    void testConstructor();
    void testProviderProperties();

  private:
    QString mTestDataDir;
    QString mReport;
    QgsRasterLayer *mDemRasterLayer = nullptr;
    QgsRasterLayer *mLandsatRasterLayer = nullptr;

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
  mDemRasterLayer = new QgsRasterLayer( demRasterFileInfo.filePath(),
                                        demRasterFileInfo.completeBaseName() );

  QString landsatFileName = mTestDataDir + "landsat.tif";
  QFileInfo landsatRasterFileInfo( landsatFileName );
  mLandsatRasterLayer = new QgsRasterLayer( landsatRasterFileInfo.filePath(),
      landsatRasterFileInfo.completeBaseName() );
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
  QString str1 = QStringLiteral( "?crs=EPSG:4326&extent=18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000&width=373&height=350&formula=\"dem@1\" + 200&dem:provider=gdal&landsat:provider=gdal" );
  QString uri = QString( "%1&%2&%3" ).arg( str1, QStringLiteral( "dem:uri=" ) % mTestDataDir % QStringLiteral( "raster/dem.tif" ),
                QStringLiteral( "landsat:uri=" ) % mTestDataDir % QStringLiteral( "landsat.tif" ) );
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
  QString str1 = QStringLiteral( "?crs=EPSG:4326&extent=18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000&width=373&height=350&formula=\"dem@1\" + 200&dem:provider=gdal" );
  QString uri1 = QString( "%1&%2" ).arg( str1, QStringLiteral( "dem:uri=" ) % mTestDataDir % QStringLiteral( "raster/dem.tif" ) );
  std::unique_ptr< QgsRasterLayer > layer_1 = std::make_unique< QgsRasterLayer >( uri1,
      QStringLiteral( "layer_1" ),
      QStringLiteral( "virtualrasterprovider" ) );

  QVERIFY( layer_1->dataProvider()->isValid() );
  QVERIFY( layer_1->isValid() );

  double sampledValueCalc_1 = layer_1->dataProvider()->sample( QgsPointXY( 18.67714, 45.79202 ), 1 );
  double sampledValue = mDemRasterLayer->dataProvider()->sample( QgsPointXY( 18.67714, 45.79202 ), 1 );

  QCOMPARE( sampledValueCalc_1, sampledValue + 200. );
  QCOMPARE( layer_1->dataProvider()->crs(), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );


  QString str2 = QStringLiteral( "?crs=EPSG:32633&extent=781662.375,3339523.125,793062.375,3350923.125&width=200&height=200&formula=\"landsat@1\" + \"landsat@2\"&landsat:provider=gdal" );
  QString uri2 = QString( "%1&%2" ).arg( str2, QStringLiteral( "landsat:uri=" ) % mTestDataDir % QStringLiteral( "landsat.tif" ) );
  std::unique_ptr< QgsRasterLayer > layer_2 = std::make_unique< QgsRasterLayer >( uri2,
      QStringLiteral( "layer_2" ),
      QStringLiteral( "virtualrasterprovider" ) );

  QVERIFY( layer_2->isValid() );
  QVERIFY( layer_2->dataProvider()->isValid() );
  QCOMPARE( layer_2->dataProvider()->crs(), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) ) );
  double sampledValueCalc_2 = layer_2->dataProvider()->sample( QgsPointXY( 790688, 3349113 ), 1 );
  //qDebug() << layer_2->dataProvider()->sample( QgsPointXY( 790688, 3349113 ), 1 );
  QCOMPARE( sampledValueCalc_2, 267. );
}

void TestQgsVirtualRasterProvider::testProviderProperties()
{


}
QGSTEST_MAIN( TestQgsVirtualRasterProvider )
#include "testqgsvirtualrasterprovider.moc"
