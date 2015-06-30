/***************************************************************************
  testqgsrastercalculator.cpp
  --------------------------------------
Date                 : Jun-2015
Copyright            : (C) 2015 by Nyall Dawson
Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>

#include "qgsrastercalculator.h"
#include "qgsrastercalcnode.h"
#include "qgsrasterlayer.h"
#include "qgsrastermatrix.h"
#include "qgsapplication.h"


Q_DECLARE_METATYPE( QgsRasterCalcNode::Operator );


class TestQgsRasterCalculator : public QObject
{
    Q_OBJECT

  public:
    TestQgsRasterCalculator()
        : mpLandsatRasterLayer( 0 )
        , mpLandsatRasterLayer4326( 0 )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() ;// will be called before each testfunction is executed.
    void cleanup() ;// will be called after every testfunction.

    void dualOp_data();
    void dualOp(); //test operators which operate on a left&right node

    void singleOp_data();
    void singleOp(); //test operators which operate on a single value

    void singleOpMatrices(); // test single op using matrix
    void dualOpNumberMatrix(); // test dual op run on number and matrix
    void dualOpMatrixNumber(); // test dual op run on matrix and number
    void dualOpMatrixMatrix(); // test dual op run on matrix and matrix

    void rasterRefOp();
    void dualOpRasterRaster(); //test dual op on raster ref and raster ref

    void calcWithLayers();
    void calcWithReprojectedLayers();

  private:

    QgsRasterLayer * mpLandsatRasterLayer;
    QgsRasterLayer * mpLandsatRasterLayer4326;
};

void  TestQgsRasterCalculator::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  QString testDataDir = QString( TEST_DATA_DIR ) + QDir::separator(); //defined in CmakeLists.txt

  QString landsatFileName = testDataDir + "landsat.tif";
  QFileInfo landsatRasterFileInfo( landsatFileName );
  mpLandsatRasterLayer = new QgsRasterLayer( landsatRasterFileInfo.filePath(),
      landsatRasterFileInfo.completeBaseName() );


  QString landsat4326FileName = testDataDir + "landsat_4326.tif";
  QFileInfo landsat4326RasterFileInfo( landsat4326FileName );
  mpLandsatRasterLayer4326 = new QgsRasterLayer( landsat4326RasterFileInfo.filePath(),
      landsat4326RasterFileInfo.completeBaseName() );
}

void  TestQgsRasterCalculator::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void  TestQgsRasterCalculator::init()
{

}
void  TestQgsRasterCalculator::cleanup()
{

}

void TestQgsRasterCalculator::dualOp_data()
{
  QTest::addColumn< QgsRasterCalcNode::Operator >( "op" );
  QTest::addColumn<double>( "left" );
  QTest::addColumn<double>( "right" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "opPlus" ) << QgsRasterCalcNode::opPLUS << 5.5 << 2.2 << 7.7;
  QTest::newRow( "opMINUS" ) << QgsRasterCalcNode::opMINUS << 5.0 << 2.5 << 2.5;
  QTest::newRow( "opMUL" ) << QgsRasterCalcNode::opMUL << 2.5 << 4.0 << 10.0;
  QTest::newRow( "opDIV" ) << QgsRasterCalcNode::opDIV << 2.5 << 2.0 << 1.25;
  QTest::newRow( "opDIV by 0" ) << QgsRasterCalcNode::opDIV << 2.5 << 0.0 << -9999.0;
  QTest::newRow( "opPOW" ) << QgsRasterCalcNode::opPOW << 3.0 << 2.0 << 9.0;
  QTest::newRow( "opPOW negative" ) << QgsRasterCalcNode::opPOW << 4.0 << -2.0 << 0.0625;
  QTest::newRow( "opPOW sqrt" ) << QgsRasterCalcNode::opPOW << 4.0 << 0.5 << 2.0;
  QTest::newRow( "opPOW complex" ) << QgsRasterCalcNode::opPOW << -2.0 << 0.5 << -9999.0;
  QTest::newRow( "opEQ true" ) << QgsRasterCalcNode::opEQ << 1.0 << 1.0 << 1.0;
  QTest::newRow( "opEQ false" ) << QgsRasterCalcNode::opEQ << 0.5 << 1.0 << 0.0;
  QTest::newRow( "opNE equal" ) << QgsRasterCalcNode::opNE << 1.0 << 1.0 << 0.0;
  QTest::newRow( "opNE not equal" ) << QgsRasterCalcNode::opNE << 0.5 << 1.0 << 1.0;
  QTest::newRow( "opGT >" ) << QgsRasterCalcNode::opGT << 1.0 << 0.5 << 1.0;
  QTest::newRow( "opGT =" ) << QgsRasterCalcNode::opGT << 0.5 << 0.5 << 0.0;
  QTest::newRow( "opGT <" ) << QgsRasterCalcNode::opGT << 0.5 << 1.0 << 0.0;
  QTest::newRow( "opLT >" ) << QgsRasterCalcNode::opLT << 1.0 << 0.5 << 0.0;
  QTest::newRow( "opLT =" ) << QgsRasterCalcNode::opLT << 0.5 << 0.5 << 0.0;
  QTest::newRow( "opLT <" ) << QgsRasterCalcNode::opLT << 0.5 << 1.0 << 1.0;
  QTest::newRow( "opGE >" ) << QgsRasterCalcNode::opGE << 1.0 << 0.5 << 1.0;
  QTest::newRow( "opGE =" ) << QgsRasterCalcNode::opGE << 0.5 << 0.5 << 1.0;
  QTest::newRow( "opGE <" ) << QgsRasterCalcNode::opGE << 0.5 << 1.0 << 0.0;
  QTest::newRow( "opLE >" ) << QgsRasterCalcNode::opLE << 1.0 << 0.5 << 0.0;
  QTest::newRow( "opLE =" ) << QgsRasterCalcNode::opLE << 0.5 << 0.5 << 1.0;
  QTest::newRow( "opLE <" ) << QgsRasterCalcNode::opLE << 0.5 << 1.0 << 1.0;
  QTest::newRow( "opAND 0/0" ) << QgsRasterCalcNode::opAND << 0.0 << 0.0 << 0.0;
  QTest::newRow( "opAND 0/1" ) << QgsRasterCalcNode::opAND << 0.0 << 1.0 << 0.0;
  QTest::newRow( "opAND 1/0" ) << QgsRasterCalcNode::opAND << 1.0 << 0.0 << 0.0;
  QTest::newRow( "opAND 1/1" ) << QgsRasterCalcNode::opAND << 1.0 << 1.0 << 1.0;
  QTest::newRow( "opOR 0/0" ) << QgsRasterCalcNode::opOR << 0.0 << 0.0 << 0.0;
  QTest::newRow( "opOR 0/1" ) << QgsRasterCalcNode::opOR << 0.0 << 1.0 << 1.0;
  QTest::newRow( "opOR 1/0" ) << QgsRasterCalcNode::opOR << 1.0 << 0.0 << 1.0;
  QTest::newRow( "opOR 1/1" ) << QgsRasterCalcNode::opOR << 1.0 << 1.0 << 1.0;
}

void TestQgsRasterCalculator::dualOp()
{
  QFETCH( QgsRasterCalcNode::Operator, op );
  QFETCH( double, left );
  QFETCH( double, right );
  QFETCH( double, expected );

  QgsRasterCalcNode node( op, new QgsRasterCalcNode( left ), new QgsRasterCalcNode( right ) );

  QgsRasterMatrix result;
  result.setNodataValue( -9999 );
  QMap<QString, QgsRasterBlock*> rasterData;

  QVERIFY( node.calculate( rasterData, result ) );

  qDebug() << "Result: " << result.number() << " expected: " << expected;
  QCOMPARE( result.number(), expected );

}

void TestQgsRasterCalculator::singleOp_data()
{
  QTest::addColumn< QgsRasterCalcNode::Operator >( "op" );
  QTest::addColumn<double>( "value" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "opSQRT" ) << QgsRasterCalcNode::opSQRT << 16.0 << 4.0;
  QTest::newRow( "opSQRT negative" ) << QgsRasterCalcNode::opSQRT << -16.0 << -9999.0;
  QTest::newRow( "opSIN 0" ) << QgsRasterCalcNode::opSIN << 0.0 << 0.0;
  QTest::newRow( "opSIN pi/2" ) << QgsRasterCalcNode::opSIN << M_PI / 2.0 << 1.0;
  QTest::newRow( "opCOS 0" ) << QgsRasterCalcNode::opCOS << 0.0 << 1.0;
  QTest::newRow( "opCOS pi" ) << QgsRasterCalcNode::opCOS << M_PI << -1.0;
  QTest::newRow( "opTAN 0" ) << QgsRasterCalcNode::opTAN << 0.0 << 0.0;
  QTest::newRow( "opTAN pi" ) << QgsRasterCalcNode::opTAN << M_PI << 0.0;
  QTest::newRow( "opASIN 0" ) << QgsRasterCalcNode::opASIN << 0.0 << 0.0;
  QTest::newRow( "opASIN pi/2" ) << QgsRasterCalcNode::opASIN << 1.0 << M_PI / 2.0;
  QTest::newRow( "opACOS 0" ) << QgsRasterCalcNode::opACOS << 1.0 << 0.0;
  QTest::newRow( "opACOS pi/2" ) << QgsRasterCalcNode::opACOS << -1.0 << M_PI;
  QTest::newRow( "opATAN 0" ) << QgsRasterCalcNode::opATAN << 0.0 << 0.0;
  QTest::newRow( "opATAN 1.0" ) << QgsRasterCalcNode::opATAN << 1.0 << 0.7853981634;
  QTest::newRow( "opSIGN +" ) << QgsRasterCalcNode::opSIGN << 1.0 << -1.0;
  QTest::newRow( "opSIGN -" ) << QgsRasterCalcNode::opSIGN << -1.0 << 1.0;
  QTest::newRow( "opLOG -1" ) << QgsRasterCalcNode::opLOG << -1.0 << -9999.0;
  QTest::newRow( "opLOG 0" ) << QgsRasterCalcNode::opLOG << 0.0 << -9999.0;
  QTest::newRow( "opLOG 1" ) << QgsRasterCalcNode::opLOG << 1.0 << 0.0;
  QTest::newRow( "opLOG10 -1" ) << QgsRasterCalcNode::opLOG10 << -1.0 << -9999.0;
  QTest::newRow( "opLOG10 0" ) << QgsRasterCalcNode::opLOG10 << 0.0 << -9999.0;
  QTest::newRow( "opLOG10 1" ) << QgsRasterCalcNode::opLOG10 << 1.0 << 0.0;
  QTest::newRow( "opLOG10 10" ) << QgsRasterCalcNode::opLOG10 << 10.0 << 1.0;
}

void TestQgsRasterCalculator::singleOp()
{
  QFETCH( QgsRasterCalcNode::Operator, op );
  QFETCH( double, value );
  QFETCH( double, expected );

  QgsRasterCalcNode node( op, new QgsRasterCalcNode( value ), 0 );

  QgsRasterMatrix result;
  result.setNodataValue( -9999 );
  QMap<QString, QgsRasterBlock*> rasterData;

  QVERIFY( node.calculate( rasterData, result ) );

  qDebug() << "Result: " << result.number() << " expected: " << expected;
  QVERIFY( qgsDoubleNear( result.number(), expected, 0.0000000001 ) );

}

void TestQgsRasterCalculator::singleOpMatrices()
{
  // test single op run on matrix
  double* d = new double[6];
  d[0] = 1.0;
  d[1] = 2.0;
  d[2] = 3.0;
  d[3] = 4.0;
  d[4] = 5.0;
  d[5] = -1.0;

  QgsRasterMatrix m( 2, 3, d, -1.0 );

  QgsRasterCalcNode node( QgsRasterCalcNode::opSIGN, new QgsRasterCalcNode( &m ) , 0 );

  QgsRasterMatrix result;
  result.setNodataValue( -9999 );
  QMap<QString, QgsRasterBlock*> rasterData;

  QVERIFY( node.calculate( rasterData, result ) );

  QCOMPARE( result.data()[0], -d[0] );
  QCOMPARE( result.data()[1], -d[1] );
  QCOMPARE( result.data()[2], -d[2] );
  QCOMPARE( result.data()[3], -d[3] );
  QCOMPARE( result.data()[4], -d[4] );
  QCOMPARE( result.data()[5], -9999.0 );
}

void TestQgsRasterCalculator::dualOpNumberMatrix()
{
  // test dual op run on number and matrix
  double* d = new double[6];
  d[0] = 1.0;
  d[1] = 2.0;
  d[2] = 3.0;
  d[3] = 4.0;
  d[4] = 5.0;
  d[5] = -1.0;

  QgsRasterMatrix m( 2, 3, d, -1.0 );

  QgsRasterCalcNode node( QgsRasterCalcNode::opPLUS, new QgsRasterCalcNode( 5.0 ), new QgsRasterCalcNode( &m ) );

  QgsRasterMatrix result;
  result.setNodataValue( -9999 );
  QMap<QString, QgsRasterBlock*> rasterData;

  QVERIFY( node.calculate( rasterData, result ) );

  QCOMPARE( result.data()[0], 6.0 );
  QCOMPARE( result.data()[1], 7.0 );
  QCOMPARE( result.data()[2], 8.0 );
  QCOMPARE( result.data()[3], 9.0 );
  QCOMPARE( result.data()[4], 10.0 );
  QCOMPARE( result.data()[5], -9999.0 );

  //also check adding no data number
  QgsRasterCalcNode nodeNoData( QgsRasterCalcNode::opPLUS, new QgsRasterCalcNode( -9999 ), new QgsRasterCalcNode( &m ) );
  QVERIFY( nodeNoData.calculate( rasterData, result ) );
  QCOMPARE( result.data()[0], -9999.0 );
}

void TestQgsRasterCalculator::dualOpMatrixNumber()
{
  // test dual op run on matrix and number
  double* d = new double[6];
  d[0] = 1.0;
  d[1] = 2.0;
  d[2] = 3.0;
  d[3] = 4.0;
  d[4] = 5.0;
  d[5] = -1.0;

  QgsRasterMatrix m( 2, 3, d, -1.0 );

  QgsRasterCalcNode node( QgsRasterCalcNode::opPLUS, new QgsRasterCalcNode( &m ), new QgsRasterCalcNode( 5.0 ) );

  QgsRasterMatrix result;
  result.setNodataValue( -9999 );
  QMap<QString, QgsRasterBlock*> rasterData;

  QVERIFY( node.calculate( rasterData, result ) );

  QCOMPARE( result.data()[0], 6.0 );
  QCOMPARE( result.data()[1], 7.0 );
  QCOMPARE( result.data()[2], 8.0 );
  QCOMPARE( result.data()[3], 9.0 );
  QCOMPARE( result.data()[4], 10.0 );
  QCOMPARE( result.data()[5], -9999.0 );

  //also check adding no data number
  QgsRasterCalcNode nodeNoData( QgsRasterCalcNode::opPLUS, new QgsRasterCalcNode( &m ), new QgsRasterCalcNode( -9999 ) );
  QVERIFY( nodeNoData.calculate( rasterData, result ) );
  QCOMPARE( result.data()[0], -9999.0 );
}

void TestQgsRasterCalculator::dualOpMatrixMatrix()
{
  // test dual op run on matrix and matrix
  double* d = new double[6];
  d[0] = 1.0;
  d[1] = 2.0;
  d[2] = -2.0;
  d[3] = -1.0; //nodata
  d[4] = 5.0;
  d[5] = -1.0; //nodata
  QgsRasterMatrix m1( 2, 3, d, -1.0 );

  double* d2 = new double[6];
  d2[0] = -1.0;
  d2[1] = -2.0; //nodata
  d2[2] = 13.0;
  d2[3] = -2.0; //nodata
  d2[4] = 15.0;
  d2[5] = -1.0;
  QgsRasterMatrix m2( 2, 3, d2, -2.0 ); //different no data value

  QgsRasterCalcNode node( QgsRasterCalcNode::opPLUS, new QgsRasterCalcNode( &m1 ), new QgsRasterCalcNode( &m2 ) );

  QgsRasterMatrix result;
  result.setNodataValue( -9999 );
  QMap<QString, QgsRasterBlock*> rasterData;

  QVERIFY( node.calculate( rasterData, result ) );

  QCOMPARE( result.data()[0], 0.0 );
  QCOMPARE( result.data()[1], -9999.0 );
  QCOMPARE( result.data()[2], 11.0 );
  QCOMPARE( result.data()[3], -9999.0 );
  QCOMPARE( result.data()[4], 20.0 );
  QCOMPARE( result.data()[5], -9999.0 );
}

void TestQgsRasterCalculator::rasterRefOp()
{
  // test single op run on raster ref
  QgsRasterCalcNode node( QgsRasterCalcNode::opSIGN, new QgsRasterCalcNode( "raster" ), 0 );

  QgsRasterMatrix result;
  result.setNodataValue( -9999 );
  QMap<QString, QgsRasterBlock*> rasterData;

  //first test invalid raster ref
  QVERIFY( !node.calculate( rasterData, result ) );

  //now create raster ref
  QgsRasterBlock m( QGis::Float32, 2, 3, -1.0 );
  m.setValue( 0, 0, 1.0 );
  m.setValue( 0, 1, 2.0 );
  m.setValue( 1, 0, 3.0 );
  m.setValue( 1, 1, 4.0 );
  m.setValue( 2, 0, 5.0 );
  m.setValue( 2, 1, -1.0 );
  rasterData.insert( "raster", &m );

  QVERIFY( node.calculate( rasterData, result ) );
  QCOMPARE( result.data()[0], -1.0 );
  QCOMPARE( result.data()[1], -2.0 );
  QCOMPARE( result.data()[2], -3.0 );
  QCOMPARE( result.data()[3], -4.0 );
  QCOMPARE( result.data()[4], -5.0 );
  QCOMPARE( result.data()[5], -9999.0 );
}

void TestQgsRasterCalculator::dualOpRasterRaster()
{
  // test dual op run on matrix and matrix

  QgsRasterBlock m1( QGis::Float32, 2, 3, -1.0 );
  m1.setValue( 0, 0, 1.0 );
  m1.setValue( 0, 1, 2.0 );
  m1.setValue( 1, 0, -2.0 );
  m1.setValue( 1, 1, -1.0 ); //nodata
  m1.setValue( 2, 0, 5.0 );
  m1.setValue( 2, 1, -1.0 ); //nodata
  QMap<QString, QgsRasterBlock*> rasterData;
  rasterData.insert( "raster1", &m1 );

  QgsRasterBlock m2( QGis::Float32, 2, 3, -2.0 ); //different no data value
  m2.setValue( 0, 0, -1.0 );
  m2.setValue( 0, 1, -2.0 ); //nodata
  m2.setValue( 1, 0, 13.0 );
  m2.setValue( 1, 1, -2.0 ); //nodata
  m2.setValue( 2, 0, 15.0 );
  m2.setValue( 2, 1, -1.0 );
  rasterData.insert( "raster2", &m2 );

  QgsRasterCalcNode node( QgsRasterCalcNode::opPLUS, new QgsRasterCalcNode( "raster1" ), new QgsRasterCalcNode( "raster2" ) );

  QgsRasterMatrix result;
  result.setNodataValue( -9999 );

  QVERIFY( node.calculate( rasterData, result ) );
  QCOMPARE( result.data()[0], 0.0 );
  QCOMPARE( result.data()[1], -9999.0 );
  QCOMPARE( result.data()[2], 11.0 );
  QCOMPARE( result.data()[3], -9999.0 );
  QCOMPARE( result.data()[4], 20.0 );
  QCOMPARE( result.data()[5], -9999.0 );
}

void TestQgsRasterCalculator::calcWithLayers()
{
  QgsRasterCalculatorEntry entry1;
  entry1.bandNumber = 1;
  entry1.raster = mpLandsatRasterLayer;
  entry1.ref = "landsat@1";

  QgsRasterCalculatorEntry entry2;
  entry2.bandNumber = 2;
  entry2.raster = mpLandsatRasterLayer;
  entry2.ref = "landsat@2";

  QVector<QgsRasterCalculatorEntry> entries;
  entries << entry1 << entry2;

  QgsCoordinateReferenceSystem crs;
  crs.createFromId( 32633, QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsRectangle extent( 783270, 3348110, 783370, 3347910 );

  QTemporaryFile tmpFile;
  tmpFile.open(); // fileName is no avialable until open
  QString tmpName = tmpFile.fileName();
  tmpFile.close();

  QgsRasterCalculator rc( QString( "\"landsat@1\" + 2" ),
                          tmpName,
                          "GTiff",
                          extent, crs, 2, 3, entries );
  QCOMPARE( rc.processCalculation(), 0 );

  //open output file and check results
  QgsRasterLayer* result = new QgsRasterLayer( tmpName, "result" );
  QCOMPARE( result->width(), 2 );
  QCOMPARE( result->height(), 3 );
  QgsRasterBlock* block = result->dataProvider()->block( 1, extent, 2, 3 );
  QCOMPARE( block->value( 0, 0 ), 127.0 );
  QCOMPARE( block->value( 0, 1 ), 127.0 );
  QCOMPARE( block->value( 1, 0 ), 127.0 );
  QCOMPARE( block->value( 1, 1 ), 126.0 );
  QCOMPARE( block->value( 2, 0 ), 128.0 );
  QCOMPARE( block->value( 2, 1 ), 129.0 );
  delete result;
  delete block;

  //now try with 2 raster bands
  QgsRasterCalculator rc2( QString( "\"landsat@1\" + \"landsat@2\"" ),
                           tmpName,
                           "GTiff",
                           extent, crs, 2, 3, entries );
  QCOMPARE( rc2.processCalculation(), 0 );

  //open output file and check results
  result = new QgsRasterLayer( tmpName, "result" );
  QCOMPARE( result->width(), 2 );
  QCOMPARE( result->height(), 3 );
  block = result->dataProvider()->block( 1, extent, 2, 3 );
  QCOMPARE( block->value( 0, 0 ), 265.0 );
  QCOMPARE( block->value( 0, 1 ), 263.0 );
  QCOMPARE( block->value( 1, 0 ), 266.0 );
  QCOMPARE( block->value( 1, 1 ), 261.0 );
  QCOMPARE( block->value( 2, 0 ), 266.0 );
  QCOMPARE( block->value( 2, 1 ), 268.0 );
  delete result;
  delete block;
}

void TestQgsRasterCalculator::calcWithReprojectedLayers()
{
  QgsRasterCalculatorEntry entry1;
  entry1.bandNumber = 1;
  entry1.raster = mpLandsatRasterLayer;
  entry1.ref = "landsat@1";

  QgsRasterCalculatorEntry entry2;
  entry2.bandNumber = 2;
  entry2.raster = mpLandsatRasterLayer4326;
  entry2.ref = "landsat_4326@2";

  QVector<QgsRasterCalculatorEntry> entries;
  entries << entry1 << entry2;

  QgsCoordinateReferenceSystem crs;
  crs.createFromId( 32633, QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsRectangle extent( 783235, 3348110, 783350, 3347960 );

  QTemporaryFile tmpFile;
  tmpFile.open(); // fileName is no avialable until open
  QString tmpName = tmpFile.fileName();
  tmpFile.close();

  QgsRasterCalculator rc( QString( "\"landsat@1\" + \"landsat_4326@2\"" ),
                          tmpName,
                          "GTiff",
                          extent, crs, 2, 3, entries );
  QCOMPARE( rc.processCalculation(), 0 );

  //open output file and check results
  QgsRasterLayer* result = new QgsRasterLayer( tmpName, "result" );
  QCOMPARE( result->width(), 2 );
  QCOMPARE( result->height(), 3 );
  QgsRasterBlock* block = result->dataProvider()->block( 1, extent, 2, 3 );
  QCOMPARE( block->value( 0, 0 ), 264.0 );
  QCOMPARE( block->value( 0, 1 ), 263.0 );
  QCOMPARE( block->value( 1, 0 ), 264.0 );
  QCOMPARE( block->value( 1, 1 ), 264.0 );
  QCOMPARE( block->value( 2, 0 ), 266.0 );
  QCOMPARE( block->value( 2, 1 ), 261.0 );
  delete result;
  delete block;
}

QTEST_MAIN( TestQgsRasterCalculator )
#include "testqgsrastercalculator.moc"
