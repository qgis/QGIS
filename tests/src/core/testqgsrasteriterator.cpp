/***************************************************************************
     testqgsrasteriterator.cpp
     --------------------------------------
    Date                 : June 2018
    Copyright            : (C) 2018 by Nyall Dawson
    Email                : nyall dot dawson at gmail.com
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
#include <QTemporaryFile>

#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasteriterator.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsRasterIterator class.
 */
class TestQgsRasterIterator : public QObject
{
    Q_OBJECT
  public:
    TestQgsRasterIterator() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testBasic();
    void testNoBlock();

  private:

    QString mTestDataDir;
    QgsRasterLayer *mpRasterLayer = nullptr;
};


//runs before all tests
void TestQgsRasterIterator::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString band1byteRaster = mTestDataDir + "/big_raster.tif";

  mpRasterLayer = new QgsRasterLayer( band1byteRaster, QStringLiteral( "big_raster" ) );

  QVERIFY( mpRasterLayer && mpRasterLayer->isValid() );
}

//runs after all tests
void TestQgsRasterIterator::cleanupTestCase()
{
  delete mpRasterLayer;

  QgsApplication::exitQgis();
}

void TestQgsRasterIterator::testBasic()
{
  QgsRasterDataProvider *provider = mpRasterLayer->dataProvider();
  QVERIFY( provider );
  QgsRasterIterator it( provider );

  QCOMPARE( it.input(), provider );

  it.setMaximumTileHeight( 2500 );
  QCOMPARE( it.maximumTileHeight(), 2500 );

  it.setMaximumTileWidth( 3000 );
  QCOMPARE( it.maximumTileWidth(), 3000 );

  it.startRasterRead( 1, mpRasterLayer->width(), mpRasterLayer->height(), mpRasterLayer->extent() );

  int nCols;
  int nRows;
  int topLeftCol;
  int topLeftRow;
  QgsRectangle blockExtent;
  std::unique_ptr< QgsRasterBlock > block;

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 0 );
  QCOMPARE( topLeftRow, 0 );
  QVERIFY( block.get() );
  QVERIFY( block->isValid() );
  QCOMPARE( block->value( 1, 1 ), 1.0 );
  QCOMPARE( block->width(), 3000 );
  QCOMPARE( block->height(), 2500 );
  QCOMPARE( blockExtent.xMinimum(), 497470.0 );
  QCOMPARE( blockExtent.xMaximum(), 497770.0 );
  QCOMPARE( blockExtent.yMinimum(), 7050880.0 );
  QCOMPARE( blockExtent.yMaximum(), 7051130.0 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - nRows * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 3000 );
  QCOMPARE( topLeftRow, 0 );
  QVERIFY( block.get() );
  QVERIFY( block->isValid() );
  QCOMPARE( block->value( 1, 1 ), 1.0 );
  QCOMPARE( block->width(), 3000 );
  QCOMPARE( block->height(), 2500 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - nRows * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 1200 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 6000 );
  QCOMPARE( topLeftRow, 0 );
  QVERIFY( block.get() );
  QVERIFY( block->isValid() );
  QCOMPARE( block->value( 1, 1 ), 1.0 );
  QCOMPARE( block->width(), 1200 );
  QCOMPARE( block->height(), 2500 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - nRows * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 0 );
  QCOMPARE( topLeftRow, 2500 );
  QVERIFY( block.get() );
  QVERIFY( block->isValid() );
  QCOMPARE( block->value( 1, 1 ), 1.0 );
  QCOMPARE( block->width(), 3000 );
  QCOMPARE( block->height(), 2500 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow )* mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 3000 );
  QCOMPARE( topLeftRow, 2500 );
  QVERIFY( block.get() );
  QVERIFY( block->isValid() );
  QCOMPARE( block->value( 1, 1 ), 1.0 );
  QCOMPARE( block->width(), 3000 );
  QCOMPARE( block->height(), 2500 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow )* mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 1200 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 6000 );
  QCOMPARE( topLeftRow, 2500 );
  QVERIFY( block.get() );
  QCOMPARE( block->width(), 1200 );
  QCOMPARE( block->height(), 2500 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow )* mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 450 );
  QCOMPARE( topLeftCol, 0 );
  QCOMPARE( topLeftRow, 5000 );
  QVERIFY( block.get() );
  QVERIFY( block->isValid() );
  QCOMPARE( block->value( 1, 1 ), 1.0 );
  QCOMPARE( block->width(), 3000 );
  QCOMPARE( block->height(), 450 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow )* mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 450 );
  QCOMPARE( topLeftCol, 3000 );
  QCOMPARE( topLeftRow, 5000 );
  QVERIFY( block.get() );
  QCOMPARE( block->width(), 3000 );
  QCOMPARE( block->height(), 450 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow )* mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 1200 );
  QCOMPARE( nRows, 450 );
  QCOMPARE( topLeftCol, 6000 );
  QCOMPARE( topLeftRow, 5000 );
  QVERIFY( block.get() );
  QVERIFY( block->isValid() );
  QCOMPARE( block->value( 1, 1 ), 1.0 );
  QCOMPARE( block->width(), 1200 );
  QCOMPARE( block->height(), 450 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow )* mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( !it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QVERIFY( !block.get() );
}

void TestQgsRasterIterator::testNoBlock()
{
  // test iterating with no block
  QgsRasterDataProvider *provider = mpRasterLayer->dataProvider();
  QVERIFY( provider );
  QgsRasterIterator it( provider );

  QCOMPARE( it.input(), provider );

  it.setMaximumTileHeight( 2500 );
  QCOMPARE( it.maximumTileHeight(), 2500 );

  it.setMaximumTileWidth( 3000 );
  QCOMPARE( it.maximumTileWidth(), 3000 );

  it.startRasterRead( 1, mpRasterLayer->width(), mpRasterLayer->height(), mpRasterLayer->extent() );

  int nCols;
  int nRows;
  int topLeftCol;
  int topLeftRow;
  QgsRectangle blockExtent;

  QVERIFY( it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 0 );
  QCOMPARE( topLeftRow, 0 );
  QCOMPARE( blockExtent.xMinimum(), 497470.0 );
  QCOMPARE( blockExtent.xMaximum(), 497770.0 );
  QCOMPARE( blockExtent.yMinimum(), 7050880.0 );
  QCOMPARE( blockExtent.yMaximum(), 7051130.0 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - nRows * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 3000 );
  QCOMPARE( topLeftRow, 0 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - nRows * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
  QCOMPARE( nCols, 1200 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 6000 );
  QCOMPARE( topLeftRow, 0 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - nRows * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 0 );
  QCOMPARE( topLeftRow, 2500 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow )* mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 3000 );
  QCOMPARE( topLeftRow, 2500 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow )* mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
  QCOMPARE( nCols, 1200 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 6000 );
  QCOMPARE( topLeftRow, 2500 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow )* mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 450 );
  QCOMPARE( topLeftCol, 0 );
  QCOMPARE( topLeftRow, 5000 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow )* mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 450 );
  QCOMPARE( topLeftCol, 3000 );
  QCOMPARE( topLeftRow, 5000 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow )* mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
  QCOMPARE( nCols, 1200 );
  QCOMPARE( nRows, 450 );
  QCOMPARE( topLeftCol, 6000 );
  QCOMPARE( topLeftRow, 5000 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow )* mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( !it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
}


QGSTEST_MAIN( TestQgsRasterIterator )

#include "testqgsrasteriterator.moc"
