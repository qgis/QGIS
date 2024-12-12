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
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testBasic();
    void testNoBlock();
    void testSubRegion();
    void testPixelOverlap();
    void testSnapToPixelFactor();

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
  const QString band1byteRaster = mTestDataDir + "/big_raster.tif";

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

  QCOMPARE( it.blockCount(), 9 );
  QCOMPARE( it.blockCountWidth(), 3 );
  QCOMPARE( it.blockCountHeight(), 3 );
  QCOMPARE( it.progress( 1 ), 0 );

  int nCols;
  int nRows;
  int topLeftCol;
  int topLeftRow;
  QgsRectangle blockExtent;
  std::unique_ptr<QgsRasterBlock> block;

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QGSCOMPARENEAR( it.progress( 1 ), 0.111, 0.001 );
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
  QGSCOMPARENEAR( it.progress( 1 ), 0.222, 0.001 );
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
  QGSCOMPARENEAR( it.progress( 1 ), 0.333, 0.001 );
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
  QGSCOMPARENEAR( it.progress( 1 ), 0.444, 0.001 );
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
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QGSCOMPARENEAR( it.progress( 1 ), 0.555, 0.001 );
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
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QGSCOMPARENEAR( it.progress( 1 ), 0.666, 0.001 );
  QCOMPARE( nCols, 1200 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 6000 );
  QCOMPARE( topLeftRow, 2500 );
  QVERIFY( block.get() );
  QCOMPARE( block->width(), 1200 );
  QCOMPARE( block->height(), 2500 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QGSCOMPARENEAR( it.progress( 1 ), 0.777, 0.001 );
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
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QGSCOMPARENEAR( it.progress( 1 ), 0.888, 0.001 );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 450 );
  QCOMPARE( topLeftCol, 3000 );
  QCOMPARE( topLeftRow, 5000 );
  QVERIFY( block.get() );
  QCOMPARE( block->width(), 3000 );
  QCOMPARE( block->height(), 450 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QGSCOMPARENEAR( it.progress( 1 ), 1.0, 0.01 );
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
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
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
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 3000 );
  QCOMPARE( topLeftRow, 2500 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
  QCOMPARE( nCols, 1200 );
  QCOMPARE( nRows, 2500 );
  QCOMPARE( topLeftCol, 6000 );
  QCOMPARE( topLeftRow, 2500 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 450 );
  QCOMPARE( topLeftCol, 0 );
  QCOMPARE( topLeftRow, 5000 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 450 );
  QCOMPARE( topLeftCol, 3000 );
  QCOMPARE( topLeftRow, 5000 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
  QCOMPARE( nCols, 1200 );
  QCOMPARE( nRows, 450 );
  QCOMPARE( topLeftCol, 6000 );
  QCOMPARE( topLeftRow, 5000 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( !it.next( 1, nCols, nRows, topLeftCol, topLeftRow, blockExtent ) );
}

void TestQgsRasterIterator::testSubRegion()
{
  QgsRasterDataProvider *provider = mpRasterLayer->dataProvider();

  QGSCOMPARENEAR( provider->extent().xMinimum(), 497470, 0.001 );
  QGSCOMPARENEAR( provider->extent().yMinimum(), 7050585, 0.001 );
  QGSCOMPARENEAR( provider->extent().xMaximum(), 498190, 0.001 );
  QGSCOMPARENEAR( provider->extent().yMaximum(), 7051130, 0.001 );
  QCOMPARE( provider->xSize(), 7200 );
  QCOMPARE( provider->ySize(), 5450 );

  int subRectWidth = 0;
  int subRectHeight = 0;
  int subRectTop = 0;
  int subRectLeft = 0;
  // sub region is whole of raster extent
  QgsRectangle subRect = QgsRasterIterator::subRegion( provider->extent(), provider->xSize(), provider->ySize(), QgsRectangle( 497470, 7050585, 498190, 7051130 ), subRectWidth, subRectHeight, subRectLeft, subRectTop );
  QCOMPARE( subRect.xMinimum(), 497470 );
  QCOMPARE( subRect.yMinimum(), 7050585 );
  QCOMPARE( subRect.xMaximum(), 498190 );
  QCOMPARE( subRect.yMaximum(), 7051130 );
  QCOMPARE( subRectWidth, 7200 );
  QCOMPARE( subRectHeight, 5450 );
  QCOMPARE( subRectLeft, 0 );
  QCOMPARE( subRectTop, 0 );

  // sub region extends outside of raster extent, should be clipped back to raster extent
  subRect = QgsRasterIterator::subRegion( provider->extent(), provider->xSize(), provider->ySize(), QgsRectangle( 497370, 7050385, 498390, 7051330 ), subRectWidth, subRectHeight, subRectLeft, subRectTop );
  QCOMPARE( subRect.xMinimum(), 497470 );
  QCOMPARE( subRect.yMinimum(), 7050585 );
  QCOMPARE( subRect.xMaximum(), 498190 );
  QCOMPARE( subRect.yMaximum(), 7051130 );
  QCOMPARE( subRectWidth, 7200 );
  QCOMPARE( subRectHeight, 5450 );
  QCOMPARE( subRectLeft, 0 );
  QCOMPARE( subRectTop, 0 );

  // sub rect inside raster extent
  subRect = QgsRasterIterator::subRegion( provider->extent(), provider->xSize(), provider->ySize(), QgsRectangle( 497970.01, 7050985.05, 498030.95, 7051030.75 ), subRectWidth, subRectHeight, subRectLeft, subRectTop );
  QCOMPARE( subRect.xMinimum(), 497970 );
  QCOMPARE( subRect.yMinimum(), 7050985.0 );
  QCOMPARE( subRect.xMaximum(), 498031 );
  QCOMPARE( subRect.yMaximum(), 7051030.8 );

  QCOMPARE( subRectWidth, 610 );
  QCOMPARE( subRectHeight, 458 );
  QCOMPARE( subRectLeft, 5000 );
  QCOMPARE( subRectTop, 992 );

  // sub rect JUST inside raster extent
  subRect = QgsRasterIterator::subRegion( provider->extent(), provider->xSize(), provider->ySize(), QgsRectangle( 497370.001, 7050385.001, 498389.99999, 7051329.9999 ), subRectWidth, subRectHeight, subRectLeft, subRectTop );
  QCOMPARE( subRect.xMinimum(), 497470 );
  QCOMPARE( subRect.yMinimum(), 7050585 );
  QCOMPARE( subRect.xMaximum(), 498190 );
  QCOMPARE( subRect.yMaximum(), 7051130 );
  QCOMPARE( subRectWidth, 7200 );
  QCOMPARE( subRectHeight, 5450 );
  QCOMPARE( subRectLeft, 0 );
  QCOMPARE( subRectTop, 0 );
}

void TestQgsRasterIterator::testPixelOverlap()
{
  QgsRasterDataProvider *provider = mpRasterLayer->dataProvider();
  QVERIFY( provider );
  QgsRasterIterator it( provider, 20 );

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
  int tileCols;
  int tileRows;
  int tileTopLeftCol;
  int tileTopLeftRow;

  QgsRectangle blockExtent;
  std::unique_ptr<QgsRasterBlock> block;

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent, &tileCols, &tileRows, &tileTopLeftCol, &tileTopLeftRow ) );
  QCOMPARE( nCols, 3020 );
  QCOMPARE( nRows, 2520 );
  QCOMPARE( topLeftCol, 0 );
  QCOMPARE( topLeftRow, 0 );
  QCOMPARE( tileCols, 3000 );
  QCOMPARE( tileRows, 2500 );
  QCOMPARE( tileTopLeftCol, 0 );
  QCOMPARE( tileTopLeftRow, 0 );
  QVERIFY( block.get() );
  QVERIFY( block->isValid() );
  QCOMPARE( block->value( 1, 1 ), 1.0 );
  QCOMPARE( block->width(), 3020 );
  QCOMPARE( block->height(), 2520 );
  QCOMPARE( blockExtent.xMinimum(), 497470.0 );
  QCOMPARE( blockExtent.xMaximum(), 497772.0 );
  QCOMPARE( blockExtent.yMinimum(), 7050878.0 );
  QCOMPARE( blockExtent.yMaximum(), 7051130.0 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - nRows * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent, &tileCols, &tileRows, &tileTopLeftCol, &tileTopLeftRow ) );
  QCOMPARE( nCols, 3040 );
  QCOMPARE( nRows, 2520 );
  QCOMPARE( topLeftCol, 2980 );
  QCOMPARE( topLeftRow, 0 );
  QCOMPARE( tileCols, 3000 );
  QCOMPARE( tileRows, 2500 );
  QCOMPARE( tileTopLeftCol, 3000 );
  QCOMPARE( tileTopLeftRow, 0 );
  QVERIFY( block.get() );
  QVERIFY( block->isValid() );
  QCOMPARE( block->value( 1, 1 ), 1.0 );
  QCOMPARE( block->width(), 3040 );
  QCOMPARE( block->height(), 2520 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - nRows * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent, &tileCols, &tileRows, &tileTopLeftCol, &tileTopLeftRow ) );
  QCOMPARE( nCols, 1220 );
  QCOMPARE( nRows, 2520 );
  QCOMPARE( topLeftCol, 5980 );
  QCOMPARE( topLeftRow, 0 );
  QCOMPARE( tileCols, 1200 );
  QCOMPARE( tileRows, 2500 );
  QCOMPARE( tileTopLeftCol, 6000 );
  QCOMPARE( tileTopLeftRow, 0 );
  QVERIFY( block.get() );
  QVERIFY( block->isValid() );
  QCOMPARE( block->value( 1, 1 ), 1.0 );
  QCOMPARE( block->width(), 1220 );
  QCOMPARE( block->height(), 2520 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - nRows * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent, &tileCols, &tileRows, &tileTopLeftCol, &tileTopLeftRow ) );
  QCOMPARE( nCols, 3020 );
  QCOMPARE( nRows, 2540 );
  QCOMPARE( topLeftCol, 0 );
  QCOMPARE( topLeftRow, 2480 );
  QCOMPARE( tileCols, 3000 );
  QCOMPARE( tileRows, 2500 );
  QCOMPARE( tileTopLeftCol, 0 );
  QCOMPARE( tileTopLeftRow, 2500 );
  QVERIFY( block.get() );
  QVERIFY( block->isValid() );
  QCOMPARE( block->value( 1, 1 ), 1.0 );
  QCOMPARE( block->width(), 3020 );
  QCOMPARE( block->height(), 2540 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent, &tileCols, &tileRows, &tileTopLeftCol, &tileTopLeftRow ) );
  QCOMPARE( nCols, 3040 );
  QCOMPARE( nRows, 2540 );
  QCOMPARE( topLeftCol, 2980 );
  QCOMPARE( topLeftRow, 2480 );
  QCOMPARE( tileCols, 3000 );
  QCOMPARE( tileRows, 2500 );
  QCOMPARE( tileTopLeftCol, 3000 );
  QCOMPARE( tileTopLeftRow, 2500 );
  QVERIFY( block.get() );
  QVERIFY( block->isValid() );
  QCOMPARE( block->value( 1, 1 ), 1.0 );
  QCOMPARE( block->width(), 3040 );
  QCOMPARE( block->height(), 2540 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent, &tileCols, &tileRows, &tileTopLeftCol, &tileTopLeftRow ) );
  QCOMPARE( nCols, 1220 );
  QCOMPARE( nRows, 2540 );
  QCOMPARE( topLeftCol, 5980 );
  QCOMPARE( topLeftRow, 2480 );
  QCOMPARE( tileCols, 1200 );
  QCOMPARE( tileRows, 2500 );
  QCOMPARE( tileTopLeftCol, 6000 );
  QCOMPARE( tileTopLeftRow, 2500 );
  QVERIFY( block.get() );
  QCOMPARE( block->width(), 1220 );
  QCOMPARE( block->height(), 2540 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent, &tileCols, &tileRows, &tileTopLeftCol, &tileTopLeftRow ) );
  QCOMPARE( nCols, 3020 );
  QCOMPARE( nRows, 470 );
  QCOMPARE( topLeftCol, 0 );
  QCOMPARE( topLeftRow, 4980 );
  QCOMPARE( tileCols, 3000 );
  QCOMPARE( tileRows, 450 );
  QCOMPARE( tileTopLeftCol, 0 );
  QCOMPARE( tileTopLeftRow, 5000 );
  QVERIFY( block.get() );
  QVERIFY( block->isValid() );
  QCOMPARE( block->value( 1, 1 ), 1.0 );
  QCOMPARE( block->width(), 3020 );
  QCOMPARE( block->height(), 470 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent, &tileCols, &tileRows, &tileTopLeftCol, &tileTopLeftRow ) );
  QCOMPARE( nCols, 3040 );
  QCOMPARE( nRows, 470 );
  QCOMPARE( topLeftCol, 2980 );
  QCOMPARE( topLeftRow, 4980 );
  QCOMPARE( tileCols, 3000 );
  QCOMPARE( tileRows, 450 );
  QCOMPARE( tileTopLeftCol, 3000 );
  QCOMPARE( tileTopLeftRow, 5000 );
  QVERIFY( block.get() );
  QCOMPARE( block->width(), 3040 );
  QCOMPARE( block->height(), 470 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent, &tileCols, &tileRows, &tileTopLeftCol, &tileTopLeftRow ) );
  QCOMPARE( nCols, 1220 );
  QCOMPARE( nRows, 470 );
  QCOMPARE( topLeftCol, 5980 );
  QCOMPARE( topLeftRow, 4980 );
  QCOMPARE( tileCols, 1200 );
  QCOMPARE( tileRows, 450 );
  QCOMPARE( tileTopLeftCol, 6000 );
  QCOMPARE( tileTopLeftRow, 5000 );
  QVERIFY( block.get() );
  QVERIFY( block->isValid() );
  QCOMPARE( block->value( 1, 1 ), 1.0 );
  QCOMPARE( block->width(), 1220 );
  QCOMPARE( block->height(), 470 );
  QCOMPARE( blockExtent.xMinimum(), mpRasterLayer->extent().xMinimum() + topLeftCol * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.yMinimum(), mpRasterLayer->extent().yMaximum() - ( nRows + topLeftRow ) * mpRasterLayer->rasterUnitsPerPixelY() );
  QCOMPARE( blockExtent.width(), nCols * mpRasterLayer->rasterUnitsPerPixelX() );
  QCOMPARE( blockExtent.height(), nRows * mpRasterLayer->rasterUnitsPerPixelY() );

  QVERIFY( !it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent, &tileCols, &tileRows, &tileTopLeftCol, &tileTopLeftRow ) );
  QVERIFY( !block.get() );
}

void TestQgsRasterIterator::testSnapToPixelFactor()
{
  // Test setting/getting snap to pixel factor
  QgsRasterIterator it( mpRasterLayer->dataProvider() );
  QCOMPARE( it.snapToPixelFactor(), 1 );

  it.setSnapToPixelFactor( 2 );
  QCOMPARE( it.snapToPixelFactor(), 2 );

  // Test invalid values are sanitized
  it.setSnapToPixelFactor( 0 );
  QCOMPARE( it.snapToPixelFactor(), 1 );
  it.setSnapToPixelFactor( -1 );
  QCOMPARE( it.snapToPixelFactor(), 1 );

  // Test subregion calculation with snapping
  int subRectWidth = 0;
  int subRectHeight = 0;
  int subRectTop = 0;
  int subRectLeft = 0;

  const QgsRectangle rasterExtent( 497470, 7050585, 498190, 7051130 );
  constexpr int rasterWidth = 7200;
  constexpr int rasterHeight = 5450;

  // Request a region that will need to be snapped to 2-pixel boundaries
  const QgsRectangle originalRegion( 497970.01, 7050985.05, 498030.95, 7051030.75 );
  QgsRectangle subRect = QgsRasterIterator::subRegion( rasterExtent, rasterWidth, rasterHeight, originalRegion, subRectWidth, subRectHeight, subRectLeft, subRectTop, 2 );

  // Dimensions should be multiples of snapping factor
  QCOMPARE( subRectWidth, 608 );
  QCOMPARE( subRectHeight, 456 );
  QCOMPARE( subRectLeft, 5000 );
  QCOMPARE( subRectTop, 992 );
  // ..this is actually what we want to check, but the above explicit checks make debugging easier! Feel free to change the values above as required.
  QCOMPARE( subRectWidth % 2, 0 );
  QCOMPARE( subRectHeight % 2, 0 );
  QCOMPARE( subRectLeft % 2, 0 );
  QCOMPARE( subRectTop % 2, 0 );

  QGSCOMPARENEAR( subRect.xMinimum(), 497970, 0.000001 );
  QGSCOMPARENEAR( subRect.xMaximum(), 498030.8, 0.000001 );
  QGSCOMPARENEAR( subRect.yMinimum(), 7050985.2, 0.000001 );
  QGSCOMPARENEAR( subRect.yMaximum(), 7051030.8, 0.000001 );

  // Test block iteration with snapping
  it.setMaximumTileWidth( 3000 );
  it.setMaximumTileHeight( 3000 );
  it.startRasterRead( 1, rasterWidth, rasterHeight, rasterExtent );

  int nCols, nRows, topLeftCol, topLeftRow;
  QgsRectangle blockExtent;
  std::unique_ptr<QgsRasterBlock> block;

  // Test with snapping factor of 2
  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );

  // Block dimensions should be multiples of snapping factor
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 3000 );
  QCOMPARE( topLeftCol, 0 );
  QCOMPARE( topLeftRow, 0 );

  QGSCOMPARENEAR( blockExtent.xMinimum(), 497470, 0.000001 );
  QGSCOMPARENEAR( blockExtent.xMaximum(), 497770.0, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMinimum(), 7050830.0, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMaximum(), 7051130.0, 0.000001 );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 3000 );
  QCOMPARE( topLeftCol, 3000 );
  QCOMPARE( topLeftRow, 0 );

  QGSCOMPARENEAR( blockExtent.xMinimum(), 497770, 0.000001 );
  QGSCOMPARENEAR( blockExtent.xMaximum(), 498070.0, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMinimum(), 7050830.0, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMaximum(), 7051130.0, 0.000001 );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 1200 );
  QCOMPARE( nRows, 3000 );
  QCOMPARE( topLeftCol, 6000 );
  QCOMPARE( topLeftRow, 0 );

  QGSCOMPARENEAR( blockExtent.xMinimum(), 498070, 0.000001 );
  QGSCOMPARENEAR( blockExtent.xMaximum(), 498190.0, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMinimum(), 7050830.0, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMaximum(), 7051130.0, 0.000001 );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 2450 );
  QCOMPARE( topLeftCol, 0 );
  QCOMPARE( topLeftRow, 3000 );

  QGSCOMPARENEAR( blockExtent.xMinimum(), 497470, 0.000001 );
  QGSCOMPARENEAR( blockExtent.xMaximum(), 497770.0, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMinimum(), 7050585.0, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMaximum(), 7050830.0, 0.000001 );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 3000 );
  QCOMPARE( nRows, 2450 );
  QCOMPARE( topLeftCol, 3000 );
  QCOMPARE( topLeftRow, 3000 );

  QGSCOMPARENEAR( blockExtent.xMinimum(), 497770, 0.000001 );
  QGSCOMPARENEAR( blockExtent.xMaximum(), 498070.0, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMinimum(), 7050585.0, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMaximum(), 7050830.0, 0.000001 );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 1200 );
  QCOMPARE( nRows, 2450 );
  QCOMPARE( topLeftCol, 6000 );
  QCOMPARE( topLeftRow, 3000 );

  QGSCOMPARENEAR( blockExtent.xMinimum(), 498070, 0.000001 );
  QGSCOMPARENEAR( blockExtent.xMaximum(), 498190.0, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMinimum(), 7050585.0, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMaximum(), 7050830.0, 0.000001 );

  QVERIFY( !it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );

  // Test with snapping factor of 4
  subRectWidth = 0;
  subRectHeight = 0;
  subRectTop = 0;
  subRectLeft = 0;

  QgsRectangle subRect4 = QgsRasterIterator::subRegion( rasterExtent, rasterWidth, rasterHeight, originalRegion, subRectWidth, subRectHeight, subRectLeft, subRectTop, 4 );

  // Test snapping factor 4 results
  // Dimensions should be multiples of snapping factor
  QCOMPARE( subRectWidth, 608 );
  QCOMPARE( subRectHeight, 456 );
  QCOMPARE( subRectLeft, 5000 );
  QCOMPARE( subRectTop, 992 );
  // ..this is actually what we want to check, but the above explicit checks make debugging easier! Feel free to change the values above as required.
  QCOMPARE( subRectWidth % 4, 0 );
  QCOMPARE( subRectHeight % 4, 0 );
  QCOMPARE( subRectLeft % 4, 0 );
  QCOMPARE( subRectTop % 4, 0 );

  QGSCOMPARENEAR( subRect4.xMinimum(), 497970, 0.000001 );
  QGSCOMPARENEAR( subRect4.xMaximum(), 498030.8, 0.000001 );
  QGSCOMPARENEAR( subRect4.yMinimum(), 7050985.2, 0.000001 );
  QGSCOMPARENEAR( subRect4.yMaximum(), 7051030.8, 0.000001 );

  const QgsRectangle subRect128 = QgsRasterIterator::subRegion( rasterExtent, rasterWidth, rasterHeight, originalRegion, subRectWidth, subRectHeight, subRectLeft, subRectTop, 128 );

  // Test snapping factor 128 results
  // Dimensions should be multiples of snapping factor
  QCOMPARE( subRectWidth, 384 );
  QCOMPARE( subRectHeight, 384 );
  QCOMPARE( subRectLeft, 5120 );
  QCOMPARE( subRectTop, 1024 );
  // ..this is actually what we want to check, but the above explicit checks make debugging easier! Feel free to change the values above as required.
  QCOMPARE( subRectWidth % 128, 0 );
  QCOMPARE( subRectHeight % 128, 0 );
  QCOMPARE( subRectLeft % 128, 0 );
  QCOMPARE( subRectTop % 128, 0 );

  QGSCOMPARENEAR( subRect128.xMinimum(), 497982.0, 0.000001 );
  QGSCOMPARENEAR( subRect128.xMaximum(), 498020.4, 0.000001 );
  QGSCOMPARENEAR( subRect128.yMinimum(), 7050989.2, 0.000001 );
  QGSCOMPARENEAR( subRect128.yMaximum(), 7051027.6, 0.000001 );

  it.setSnapToPixelFactor( 128 );
  it.startRasterRead( 1, rasterWidth, rasterHeight, rasterExtent );
  // Test with snapping factor of 128
  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );

  // Block dimensions should be multiples of snapping factor
  QCOMPARE( nCols, 2944 );
  QCOMPARE( nRows, 2944 );
  QCOMPARE( topLeftCol, 0 );
  QCOMPARE( topLeftRow, 0 );

  QGSCOMPARENEAR( blockExtent.xMinimum(), 497470, 0.000001 );
  QGSCOMPARENEAR( blockExtent.xMaximum(), 497764.4, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMinimum(), 7050835.6, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMaximum(), 7051130.0, 0.000001 );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 2944 );
  QCOMPARE( nRows, 2944 );
  QCOMPARE( topLeftCol, 2944 );
  QCOMPARE( topLeftRow, 0 );

  QGSCOMPARENEAR( blockExtent.xMinimum(), 497764.4, 0.000001 );
  QGSCOMPARENEAR( blockExtent.xMaximum(), 498058.8, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMinimum(), 7050835.6, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMaximum(), 7051130.0, 0.000001 );

  QVERIFY( it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
  QCOMPARE( nCols, 1280 );
  QCOMPARE( nRows, 2944 );
  QCOMPARE( topLeftCol, 5888 );
  QCOMPARE( topLeftRow, 0 );

  QGSCOMPARENEAR( blockExtent.xMinimum(), 498058.8, 0.000001 );
  QGSCOMPARENEAR( blockExtent.xMaximum(), 498186.8, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMinimum(), 7050835.6, 0.000001 );
  QGSCOMPARENEAR( blockExtent.yMaximum(), 7051130.0, 0.000001 );

  QVERIFY( !it.readNextRasterPart( 1, nCols, nRows, block, topLeftCol, topLeftRow, &blockExtent ) );
}


QGSTEST_MAIN( TestQgsRasterIterator )

#include "testqgsrasteriterator.moc"
