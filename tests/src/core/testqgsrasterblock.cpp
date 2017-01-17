/***************************************************************************
     testqgsrasterblock.cpp
     --------------------------------------
    Date                 : January 2017
    Copyright            : (C) 2017 by Martin Dobias
    Email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>
#include <QString>

#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"

/** \ingroup UnitTests
 * This is a unit test for the QgsRasterBlock class.
 */
class TestQgsRasterBlock : public QObject
{
    Q_OBJECT
  public:
    TestQgsRasterBlock()
        : mpRasterLayer( nullptr )
    {}
    ~TestQgsRasterBlock()
    {
    }

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testBasic();

  private:

    QString mTestDataDir;
    QgsRasterLayer* mpRasterLayer;
};


//runs before all tests
void TestQgsRasterBlock::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString band1byteRaster = mTestDataDir + "/raster/band1_byte_ct_epsg4326.tif";

  mpRasterLayer = new QgsRasterLayer( band1byteRaster, "band1_byte" );

  QVERIFY( mpRasterLayer && mpRasterLayer->isValid() );
}

//runs after all tests
void TestQgsRasterBlock::cleanupTestCase()
{
  delete mpRasterLayer;

  QgsApplication::exitQgis();
}

void TestQgsRasterBlock::testBasic()
{
  QgsRasterDataProvider* provider = mpRasterLayer->dataProvider();
  QVERIFY( provider );

  QgsRectangle fullExtent = mpRasterLayer->extent();
  int width = mpRasterLayer->width();
  int height = mpRasterLayer->height();

  QgsRasterBlock* block = provider->block( 1, fullExtent, width, height );

  QCOMPARE( block->width(), 10 );
  QCOMPARE( block->height(), 10 );

  QVERIFY( block->isValid() );
  QVERIFY( !block->isEmpty() );
  QCOMPARE( block->dataType(), Qgis::Byte );
  QCOMPARE( block->dataTypeSize(), 1 );
  QVERIFY( block->hasNoDataValue() );
  QVERIFY( block->hasNoData() );
  QCOMPARE( block->noDataValue(), 255. );

  // value() with row, col
  QCOMPARE( block->value( 0, 0 ), 2. );
  QCOMPARE( block->value( 0, 1 ), 5. );
  QCOMPARE( block->value( 1, 0 ), 27. );
  // value() with index
  QCOMPARE( block->value( 0 ), 2. );
  QCOMPARE( block->value( 1 ), 5. );
  QCOMPARE( block->value( 10 ), 27. );

  // isNoData()
  QCOMPARE( block->isNoData( 0, 1 ), false );
  QCOMPARE( block->isNoData( 0, 2 ), true );
  QCOMPARE( block->isNoData( 1 ), false );
  QCOMPARE( block->isNoData( 2 ), true );

  // data()
  QByteArray data = block->data();
  QCOMPARE( data.count(), 100 );  // 10x10 raster with 1 byte/pixel
  QCOMPARE( data.at( 0 ), ( char ) 2 );
  QCOMPARE( data.at( 1 ), ( char ) 5 );
  QCOMPARE( data.at( 10 ), ( char ) 27 );

  delete block;
}

QGSTEST_MAIN( TestQgsRasterBlock )

#include "testqgsrasterblock.moc"
