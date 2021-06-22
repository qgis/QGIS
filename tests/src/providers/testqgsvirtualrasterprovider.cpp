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
    void testv();
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

    QgsProject::instance()->addMapLayers(
      QList<QgsMapLayer *>() << mdemRasterLayer );
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

  QVERIFY( layer->isValid() );

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

void TestQgsVirtualRasterProvider::testv()
{
    if ( mdemRasterLayer->extent().xMaximum()== 45.8117014376000000 )
    {
        QgsDebugMsg("testv is starting");
    }
/*
    QgsRasterCalculatorEntry entry1;
    entry1.bandNumber = 1;
    entry1.raster = mdemRasterLayer;
    entry1.ref = QStringLiteral( "dem@1" );

    QVector<QgsRasterCalculatorEntry> entries;
    entries << entry1;

    //QgsCoordinateReferenceSystem crs( QStringLiteral( "EPSG:32633" ) );
    QgsCoordinateReferenceSystem mOutputCrs( QStringLiteral( "EPSG:4326" ) );
    QgsRectangle extent(18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000);
*/


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

        QVERIFY( block );
        QCOMPARE( block->width(),  373 );
        QCOMPARE( block->height(), 350 );

        QCOMPARE( block->value( 0, 0 ), 292.86041259765625 );
      }
    delete provider;


}
QGSTEST_MAIN( TestQgsVirtualRasterProvider )
#include "testqgsvirtualrasterprovider.moc"
