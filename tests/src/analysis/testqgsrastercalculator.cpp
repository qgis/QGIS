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
#include "qgstest.h"

#ifdef HAVE_OPENCL
#include "qgsopenclutils.h"
#endif

#include "qgsrastercalculator.h"
#include "qgsrastercalcnode.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsrastermatrix.h"
#include "qgsapplication.h"
#include "qgsproject.h"

#include <QDebug>

Q_DECLARE_METATYPE( QgsRasterCalcNode::Operator )

class TestQgsRasterCalculator : public QObject
{
    Q_OBJECT

  public:
    TestQgsRasterCalculator() = default;

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

    void errors();
    void toString();
    void findNodes();

    void testRasterEntries();
    void calcFormulasWithReprojectedLayers();

    void testStatistics();

    void parseFunctionTypeString(); //test the parsing of the formule for the tFunction type
    void testFunctionTypeWithLayer(); //test of conditional statement

  private:

    QgsRasterLayer *mpLandsatRasterLayer = nullptr;
    QgsRasterLayer *mpLandsatRasterLayer4326 = nullptr;
};


void  TestQgsRasterCalculator::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::init();
  QgsApplication::initQgis();


  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt

  QString landsatFileName = testDataDir + "landsat.tif";
  QFileInfo landsatRasterFileInfo( landsatFileName );
  mpLandsatRasterLayer = new QgsRasterLayer( landsatRasterFileInfo.filePath(),
      landsatRasterFileInfo.completeBaseName() );


  QString landsat4326FileName = testDataDir + "landsat_4326.tif";
  QFileInfo landsat4326RasterFileInfo( landsat4326FileName );
  mpLandsatRasterLayer4326 = new QgsRasterLayer( landsat4326RasterFileInfo.filePath(),
      landsat4326RasterFileInfo.completeBaseName() );

  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpLandsatRasterLayer << mpLandsatRasterLayer4326 );
}

void  TestQgsRasterCalculator::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void  TestQgsRasterCalculator::init()
{
#ifdef HAVE_OPENCL
  QgsOpenClUtils::setEnabled( false );
  // Reset to default in case some tests mess it up
  QgsOpenClUtils::setSourcePath( QDir( QgsApplication::pkgDataPath() ).absoluteFilePath( QStringLiteral( "resources/opencl_programs" ) ) );
#endif
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

  QgsRasterMatrix result( 1, 1, nullptr, -999 );
  result.setNodataValue( -9999 );
  QMap<QString, QgsRasterBlock *> rasterData;

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

  QgsRasterCalcNode node( op, new QgsRasterCalcNode( value ), nullptr );

  QgsRasterMatrix result( 1, 1, nullptr, -9999 );
  QMap<QString, QgsRasterBlock *> rasterData;

  QVERIFY( node.calculate( rasterData, result ) );

  qDebug() << "Result: " << result.number() << " expected: " << expected;
  QGSCOMPARENEAR( result.number(), expected, 0.0000000001 );

}

void TestQgsRasterCalculator::singleOpMatrices()
{
  // test single op run on matrix
  double *d = new double[6];
  d[0] = 1.0;
  d[1] = 2.0;
  d[2] = 3.0;
  d[3] = 4.0;
  d[4] = 5.0;
  d[5] = -1.0;

  QgsRasterMatrix m( 2, 3, d, -1.0 );

  QgsRasterCalcNode node( QgsRasterCalcNode::opSIGN, new QgsRasterCalcNode( &m ), nullptr );

  QgsRasterMatrix result( 1, 1, nullptr, -9999 );
  QMap<QString, QgsRasterBlock *> rasterData;

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
  double *d = new double[6];
  d[0] = 1.0;
  d[1] = 2.0;
  d[2] = 3.0;
  d[3] = 4.0;
  d[4] = 5.0;
  d[5] = -1.0;

  QgsRasterMatrix m( 2, 3, d, -1.0 );

  QgsRasterCalcNode node( QgsRasterCalcNode::opPLUS, new QgsRasterCalcNode( 5.0 ), new QgsRasterCalcNode( &m ) );

  QgsRasterMatrix result( 1, 1, nullptr, -9999 );
  QMap<QString, QgsRasterBlock *> rasterData;

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
  double *d = new double[6];
  d[0] = 1.0;
  d[1] = 2.0;
  d[2] = 3.0;
  d[3] = 4.0;
  d[4] = 5.0;
  d[5] = -1.0;

  QgsRasterMatrix m( 2, 3, d, -1.0 );

  QgsRasterCalcNode node( QgsRasterCalcNode::opPLUS, new QgsRasterCalcNode( &m ), new QgsRasterCalcNode( 5.0 ) );

  QgsRasterMatrix result( 1, 1, nullptr, -9999 );
  QMap<QString, QgsRasterBlock *> rasterData;

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
  double *d = new double[6];
  d[0] = 1.0;
  d[1] = 2.0;
  d[2] = -2.0;
  d[3] = -1.0; //nodata
  d[4] = 5.0;
  d[5] = -1.0; //nodata
  QgsRasterMatrix m1( 2, 3, d, -1.0 );

  double *d2 = new double[6];
  d2[0] = -1.0;
  d2[1] = -2.0; //nodata
  d2[2] = 13.0;
  d2[3] = -2.0; //nodata
  d2[4] = 15.0;
  d2[5] = -1.0;
  QgsRasterMatrix m2( 2, 3, d2, -2.0 ); //different no data value

  QgsRasterCalcNode node( QgsRasterCalcNode::opPLUS, new QgsRasterCalcNode( &m1 ), new QgsRasterCalcNode( &m2 ) );

  QgsRasterMatrix result( 1, 1, nullptr, -9999 );
  QMap<QString, QgsRasterBlock *> rasterData;

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
  QgsRasterCalcNode node( QgsRasterCalcNode::opSIGN, new QgsRasterCalcNode( QStringLiteral( "raster" ) ), nullptr );

  QgsRasterMatrix result( 1, 1, nullptr, -9999 );
  QMap<QString, QgsRasterBlock *> rasterData;

  //first test invalid raster ref
  QVERIFY( !node.calculate( rasterData, result ) );

  //now create raster ref
  QgsRasterBlock m( Qgis::DataType::Float32, 2, 3 );
  m.setNoDataValue( -1.0 );
  m.setValue( 0, 0, 1.0 );
  m.setValue( 0, 1, 2.0 );
  m.setValue( 1, 0, 3.0 );
  m.setValue( 1, 1, 4.0 );
  m.setValue( 2, 0, 5.0 );
  m.setValue( 2, 1, -1.0 );
  rasterData.insert( QStringLiteral( "raster" ), &m );

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

  QgsRasterBlock m1( Qgis::DataType::Float32, 2, 3 );
  m1.setNoDataValue( -1.0 );
  m1.setValue( 0, 0, 1.0 );
  m1.setValue( 0, 1, 2.0 );
  m1.setValue( 1, 0, -2.0 );
  m1.setValue( 1, 1, -1.0 ); //nodata
  m1.setValue( 2, 0, 5.0 );
  m1.setValue( 2, 1, -1.0 ); //nodata
  QMap<QString, QgsRasterBlock *> rasterData;
  rasterData.insert( QStringLiteral( "raster1" ), &m1 );

  QgsRasterBlock m2( Qgis::DataType::Float32, 2, 3 );
  m2.setNoDataValue( -2.0 ); //different no data value
  m2.setValue( 0, 0, -1.0 );
  m2.setValue( 0, 1, -2.0 ); //nodata
  m2.setValue( 1, 0, 13.0 );
  m2.setValue( 1, 1, -2.0 ); //nodata
  m2.setValue( 2, 0, 15.0 );
  m2.setValue( 2, 1, -1.0 );
  rasterData.insert( QStringLiteral( "raster2" ), &m2 );

  QgsRasterCalcNode node( QgsRasterCalcNode::opPLUS, new QgsRasterCalcNode( QStringLiteral( "raster1" ) ), new QgsRasterCalcNode( QStringLiteral( "raster2" ) ) );

  QgsRasterMatrix result( 1, 1, nullptr, -9999 );

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
  entry1.ref = QStringLiteral( "landsat@1" );

  QgsRasterCalculatorEntry entry2;
  entry2.bandNumber = 2;
  entry2.raster = mpLandsatRasterLayer;
  entry2.ref = QStringLiteral( "landsat@2" );

  QVector<QgsRasterCalculatorEntry> entries;
  entries << entry1 << entry2;

  QgsCoordinateReferenceSystem crs( QStringLiteral( "EPSG:32633" ) );
  QgsRectangle extent( 783235, 3348110, 783350, 3347960 );

  QTemporaryFile tmpFile;
  tmpFile.open(); // fileName is not available until open
  QString tmpName = tmpFile.fileName();
  tmpFile.close();

  QgsRasterCalculator rc( QStringLiteral( "\"landsat@1\" + 2" ),
                          tmpName,
                          QStringLiteral( "GTiff" ),
                          extent, crs, 2, 3, entries,
                          QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc.processCalculation() ), 0 );

  //open output file and check results
  QgsRasterLayer *result = new QgsRasterLayer( tmpName, QStringLiteral( "result" ) );
  QCOMPARE( result->width(), 2 );
  QCOMPARE( result->height(), 3 );
  QgsRasterBlock *block = result->dataProvider()->block( 1, extent, 2, 3 );
  QCOMPARE( block->value( 0, 0 ), 127.0 );
  QCOMPARE( block->value( 0, 1 ), 127.0 );
  QCOMPARE( block->value( 1, 0 ), 126.0 );
  QCOMPARE( block->value( 1, 1 ), 127.0 );
  QCOMPARE( block->value( 2, 0 ), 127.0 );
  QCOMPARE( block->value( 2, 1 ), 126.0 );
  delete result;
  delete block;

  //now try with 2 raster bands
  QgsRasterCalculator rc2( QStringLiteral( "\"landsat@1\" + \"landsat@2\"" ),
                           tmpName,
                           QStringLiteral( "GTiff" ),
                           extent, crs, 2, 3, entries,
                           QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc2.processCalculation() ), 0 );

  //open output file and check results
  result = new QgsRasterLayer( tmpName, QStringLiteral( "result" ) );
  QCOMPARE( result->width(), 2 );
  QCOMPARE( result->height(), 3 );
  block = result->dataProvider()->block( 1, extent, 2, 3 );
  QCOMPARE( block->value( 0, 0 ), 265.0 );
  QCOMPARE( block->value( 0, 1 ), 263.0 );
  QCOMPARE( block->value( 1, 0 ), 263.0 );
  QCOMPARE( block->value( 1, 1 ), 264.0 );
  QCOMPARE( block->value( 2, 0 ), 266.0 );
  QCOMPARE( block->value( 2, 1 ), 261.0 );
  delete result;
  delete block;
}

void TestQgsRasterCalculator::calcWithReprojectedLayers()
{
  QgsRasterCalculatorEntry entry1;
  entry1.bandNumber = 1;
  entry1.raster = mpLandsatRasterLayer;
  entry1.ref = QStringLiteral( "landsat@1" );

  QgsRasterCalculatorEntry entry2;
  entry2.bandNumber = 2;
  entry2.raster = mpLandsatRasterLayer4326;
  entry2.ref = QStringLiteral( "landsat_4326@2" );

  QVector<QgsRasterCalculatorEntry> entries;
  entries << entry1 << entry2;

  QgsCoordinateReferenceSystem crs( QStringLiteral( "EPSG:32633" ) );
  QgsRectangle extent( 783235, 3348110, 783350, 3347960 );

  QTemporaryFile tmpFile;
  tmpFile.open(); // fileName is not available until open
  QString tmpName = tmpFile.fileName();
  tmpFile.close();

  QgsRasterCalculator rc( QStringLiteral( "\"landsat@1\" + \"landsat_4326@2\"" ),
                          tmpName,
                          QStringLiteral( "GTiff" ),
                          extent, crs, 2, 3, entries,
                          QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc.processCalculation() ), 0 );

  //open output file and check results
  QgsRasterLayer *result = new QgsRasterLayer( tmpName, QStringLiteral( "result" ) );
  QCOMPARE( result->width(), 2 );
  QCOMPARE( result->height(), 3 );
  QgsRasterBlock *block = result->dataProvider()->block( 1, extent, 2, 3 );
  QCOMPARE( block->value( 0, 0 ), 264.0 );
  QCOMPARE( block->value( 0, 1 ), 263.0 );
  QCOMPARE( block->value( 1, 0 ), 264.0 );
  QCOMPARE( block->value( 1, 1 ), 264.0 );
  QCOMPARE( block->value( 2, 0 ), 266.0 );
  QCOMPARE( block->value( 2, 1 ), 261.0 );
  delete result;
  delete block;
}

void TestQgsRasterCalculator::findNodes()
{

  std::unique_ptr< QgsRasterCalcNode > calcNode;

  auto _test =
    [ & ]( QString exp, const QgsRasterCalcNode::Type type ) -> QList<const QgsRasterCalcNode *>
  {
    QString error;
    calcNode.reset( QgsRasterCalcNode::parseRasterCalcString( exp, error ) );
    return calcNode->findNodes( type );
  };

  QCOMPARE( _test( QStringLiteral( "atan(\"raster@1\") * cos( 3  +  2 )" ), QgsRasterCalcNode::Type::tOperator ).length(), 4 );
  QCOMPARE( _test( QStringLiteral( "\"raster@1\"" ), QgsRasterCalcNode::Type::tOperator ).length(), 0 );
  QCOMPARE( _test( QStringLiteral( "\"raster@1\"" ), QgsRasterCalcNode::Type::tRasterRef ).length(), 1 );
  QCOMPARE( _test( QStringLiteral( "\"raster@1\"" ), QgsRasterCalcNode::Type::tMatrix ).length(), 0 );
  QCOMPARE( _test( QStringLiteral( "2 + 3" ), QgsRasterCalcNode::Type::tNumber ).length(), 2 );
  QCOMPARE( _test( QStringLiteral( "2 + 3" ), QgsRasterCalcNode::Type::tOperator ).length(), 1 );

  // Test parser with valid and invalid expressions
  QString errorString;
  const QgsRasterCalcNode *node { QgsRasterCalcNode::parseRasterCalcString( QString( ), errorString ) };
  QVERIFY( ! node );
  QVERIFY( ! errorString.isEmpty() );
  errorString = QString();
  node = QgsRasterCalcNode::parseRasterCalcString( QStringLiteral( "log10(2)" ), errorString );
  QVERIFY( node );
  QVERIFY( errorString.isEmpty() );
  errorString = QString();
  node = QgsRasterCalcNode::parseRasterCalcString( QStringLiteral( "not_a_function(2)" ), errorString );
  QVERIFY( ! node );
  QVERIFY( ! errorString.isEmpty() );

  // Test new abs, min, max
  errorString.clear();
  node = QgsRasterCalcNode::parseRasterCalcString( QStringLiteral( "abs(2)" ), errorString );
  QVERIFY( node );
  QVERIFY( errorString.isEmpty() );
  node = QgsRasterCalcNode::parseRasterCalcString( QStringLiteral( "min(-1,1)" ), errorString );
  QVERIFY( node );
  QVERIFY( errorString.isEmpty() );
  node = QgsRasterCalcNode::parseRasterCalcString( QStringLiteral( "max(-1,1)" ), errorString );
  QVERIFY( node );
  QVERIFY( errorString.isEmpty() );
}

void TestQgsRasterCalculator::testRasterEntries()
{
  // Create some test layers
  QList<QgsMapLayer *> layers;
  QgsRasterLayer *rlayer = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/analysis/dem.tif",  QStringLiteral( "dem" ) );
  layers << rlayer;
  // Duplicate name, same source
  rlayer = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/analysis/dem.tif",  QStringLiteral( "dem" ) );
  layers << rlayer;
  // Duplicated name different source
  rlayer = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/analysis/dem_int16.tif",  QStringLiteral( "dem" ) );
  layers << rlayer;
  // Different name and different source
  rlayer = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/analysis/slope.tif",  QStringLiteral( "slope" ) );
  layers << rlayer ;
  // Different name and same source
  rlayer = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/analysis/slope.tif",  QStringLiteral( "slope2" ) );
  layers << rlayer ;
  QgsProject::instance()->addMapLayers( layers );
  QVector<QgsRasterCalculatorEntry> availableRasterBands = QgsRasterCalculatorEntry::rasterEntries();
  QMap<QString, QgsRasterCalculatorEntry> entryMap;
  for ( const auto &rb : std::as_const( availableRasterBands ) )
  {
    entryMap[rb.ref] = rb;
  }
  QStringList keys( entryMap.keys() );
  keys.sort();
  QCOMPARE( keys.join( ',' ), QStringLiteral( "dem@1,dem_1@1,landsat@1,landsat@2,landsat@3,landsat@4,"
            "landsat@5,landsat@6,landsat@7,landsat@8,landsat@9,"
            "landsat_4326@1,landsat_4326@2,landsat_4326@3,landsat_4326@4,"
            "landsat_4326@5,landsat_4326@6,landsat_4326@7,landsat_4326@8,landsat_4326@9,slope2@1" ) );
}

void TestQgsRasterCalculator::errors( )
{
  QgsRasterCalculatorEntry entry1;
  entry1.bandNumber = 0; // bad band
  entry1.raster = mpLandsatRasterLayer;
  entry1.ref = QStringLiteral( "landsat@0" );

  QVector<QgsRasterCalculatorEntry> entries;
  entries << entry1;

  QgsCoordinateReferenceSystem crs( QStringLiteral( "EPSG:32633" ) );
  QgsRectangle extent( 783235, 3348110, 783350, 3347960 );

  QTemporaryFile tmpFile;
  tmpFile.open(); // fileName is not available until open
  QString tmpName = tmpFile.fileName();
  tmpFile.close();

  QgsRasterCalculator rc( QStringLiteral( "\"landsat@0\"" ),
                          tmpName,
                          QStringLiteral( "GTiff" ),
                          extent, crs, 2, 3, entries,
                          QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc.processCalculation() ), 6 );
  QCOMPARE( rc.lastError(), QStringLiteral( "Band number 0 is not valid for entry landsat@0" ) );

  entry1.bandNumber = 10; // bad band
  entries.clear();
  entries << entry1;
  rc = QgsRasterCalculator( QStringLiteral( "\"landsat@0\"" ),
                            tmpName,
                            QStringLiteral( "GTiff" ),
                            extent, crs, 2, 3, entries,
                            QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc.processCalculation() ), 6 );
  QCOMPARE( rc.lastError(), QStringLiteral( "Band number 10 is not valid for entry landsat@0" ) );


  // no raster
  entry1.raster = nullptr;
  entry1.bandNumber = 1;
  entries.clear();
  entries << entry1;
  rc = QgsRasterCalculator( QStringLiteral( "\"landsat@0\"" ),
                            tmpName,
                            QStringLiteral( "GTiff" ),
                            extent, crs, 2, 3, entries,
                            QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc.processCalculation() ), 2 );
  QCOMPARE( rc.lastError(), QStringLiteral( "No raster layer for entry landsat@0" ) );

  // bad driver
  entry1.raster = mpLandsatRasterLayer;
  entry1.bandNumber = 1;
  entries.clear();
  entries << entry1;
  rc = QgsRasterCalculator( QStringLiteral( "\"landsat@0\"" ),
                            tmpName,
                            QStringLiteral( "xxxxx" ),
                            extent, crs, 2, 3, entries,
                            QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc.processCalculation() ), 1 );
  QCOMPARE( rc.lastError(), QStringLiteral( "Could not obtain driver for xxxxx" ) );

  // bad filename
  rc = QgsRasterCalculator( QStringLiteral( "\"landsat@0\"" ),
                            QStringLiteral( "/goodluckwritinghere/blah/blah.tif" ),
                            QStringLiteral( "GTiff" ),
                            extent, crs, 2, 3, entries,
                            QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc.processCalculation() ), 1 );
  QCOMPARE( rc.lastError(), QStringLiteral( "Could not create output /goodluckwritinghere/blah/blah.tif" ) );

  // canceled
  QgsFeedback feedback;
  feedback.cancel();
  rc = QgsRasterCalculator( QStringLiteral( "\"landsat@0\"" ),
                            tmpName,
                            QStringLiteral( "GTiff" ),
                            extent, crs, 2, 3, entries,
                            QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc.processCalculation( &feedback ) ), 3 );
  QVERIFY( rc.lastError().isEmpty() );
}

void TestQgsRasterCalculator::toString()
{
  auto _test = [ ]( QString exp, bool cStyle ) -> QString
  {
    QString error;
    std::unique_ptr< QgsRasterCalcNode > calcNode( QgsRasterCalcNode::parseRasterCalcString( exp, error ) );
    if ( ! error.isEmpty() )
      return error;
    return calcNode->toString( cStyle );
  };
  QCOMPARE( _test( QStringLiteral( "\"raster@1\"  + 2" ), false ), QString( "( \"raster@1\" + 2 )" ) );
  QCOMPARE( _test( QStringLiteral( "\"raster@1\"  +  2" ), true ), QString( "( ( float ) \"raster@1\" + ( float ) 2 )" ) );
  QCOMPARE( _test( QStringLiteral( "\"raster@1\" ^ 3  +  2" ), false ), QString( "( \"raster@1\"^3 + 2 )" ) );
  QCOMPARE( _test( QStringLiteral( "\"raster@1\" ^ 3  +  2" ), true ), QString( "( pow( ( float ) \"raster@1\", ( float ) 3 ) + ( float ) 2 )" ) );
  QCOMPARE( _test( QStringLiteral( "atan(\"raster@1\") * cos( 3  +  2 )" ), false ), QString( "atan( \"raster@1\" ) * cos( ( 3 + 2 ) )" ) );
  QCOMPARE( _test( QStringLiteral( "atan(\"raster@1\") * cos( 3  +  2 )" ), true ), QString( "atan( ( float ) \"raster@1\" ) * cos( ( ( float ) 3 + ( float ) 2 ) )" ) );
  QCOMPARE( _test( QStringLiteral( "0.5 * ( 1.4 * (\"raster@1\" + 2) )" ), false ), QString( "0.5 * 1.4 * ( \"raster@1\" + 2 )" ) );
  QCOMPARE( _test( QStringLiteral( "0.5 * ( 1.4 * (\"raster@1\" + 2) )" ), true ), QString( "( float ) 0.5 * ( float ) 1.4 * ( ( float ) \"raster@1\" + ( float ) 2 )" ) );
  QCOMPARE( _test( QStringLiteral( "0.5 * ( 1 > 0 )" ), false ), QString( "0.5 * 1 > 0" ) );
  QCOMPARE( _test( QStringLiteral( "0.5 * ( 1 > 0 )" ), true ), QString( "( float ) 0.5 * ( float ) ( ( float ) 1 > ( float ) 0 )" ) );
  // Test negative numbers
  QCOMPARE( _test( QStringLiteral( "0.5 * ( -1 > 0 )" ), false ), QString( "0.5 * -1 > 0" ) );
  QCOMPARE( _test( QStringLiteral( "0.5 * ( -1 > 0 )" ), true ), QString( "( float ) 0.5 * ( float ) ( -( float ) 1 > ( float ) 0 )" ) );
  // Test new functions
  QCOMPARE( _test( QStringLiteral( "0.5 * abs( -1 )" ), false ), QString( "0.5 * abs( -1 )" ) );
  QCOMPARE( _test( QStringLiteral( "0.5 * abs( -1 )" ), true ), QString( "( float ) 0.5 * fabs( -( float ) 1 )" ) );
  QCOMPARE( _test( QStringLiteral( "0.5 * min( -1, 1 )" ), false ), QString( "0.5 * min( -1, 1 )" ) );
  QCOMPARE( _test( QStringLiteral( "0.5 * min( -1, 1 )" ), true ), QString( "( float ) 0.5 * min( ( float ) ( -( float ) 1 ), ( float ) ( ( float ) 1 ) )" ) );
  QCOMPARE( _test( QStringLiteral( "0.5 * max( -1, 1 )" ), false ), QString( "0.5 * max( -1, 1 )" ) );
  QCOMPARE( _test( QStringLiteral( "0.5 * max( -1, 1 )" ), true ), QString( "( float ) 0.5 * max( ( float ) ( -( float ) 1 ), ( float ) ( ( float ) 1 ) )" ) );
  // Test regression #32477
  QCOMPARE( _test( QStringLiteral( R"raw(("r@1"<100.09)*0.1)raw" ), true ),
            QString( R"raw(( float ) ( ( float ) "r@1" < ( float ) 100.09 ) * ( float ) 0.1)raw" ) );
  //test the conditional statement
  QCOMPARE( _test( QStringLiteral( "if( \"raster@1\" > 5 , 100 , 5)" ), false ), QString( "if( \"raster@1\" > 5 , 100 , 5 )" ) );
  QCOMPARE( _test( QStringLiteral( "if( \"raster@1\" > 5 , 100 , 5)" ), true ), QString( " ( ( float ) ( ( float ) \"raster@1\" > ( float ) 5 ) ) ? ( ( float ) 100 ) : ( ( float ) 5 ) " ) );

  QString error;
  std::unique_ptr< QgsRasterCalcNode > calcNode( QgsRasterCalcNode::parseRasterCalcString( QStringLiteral( "min( \"raster@1\" )" ), error ) );
  QVERIFY( calcNode == nullptr );
  calcNode.reset( QgsRasterCalcNode::parseRasterCalcString( QStringLiteral( "max( \"raster@1\" )" ), error ) );
  QVERIFY( calcNode == nullptr );
}

void TestQgsRasterCalculator::calcFormulasWithReprojectedLayers()
{
  QgsRasterCalculatorEntry entry1;
  entry1.bandNumber = 1;
  entry1.raster = mpLandsatRasterLayer;
  entry1.ref = QStringLiteral( "landsat@1" );

  QgsRasterCalculatorEntry entry2;
  entry2.bandNumber = 2;
  entry2.raster = mpLandsatRasterLayer4326;
  entry2.ref = QStringLiteral( "landsat_4326@2" );

  QVector<QgsRasterCalculatorEntry> entries;
  entries << entry1 << entry2;

  QgsCoordinateReferenceSystem crs( QStringLiteral( "EPSG:32633" ) );
  QgsRectangle extent( 783235, 3348110, 783350, 3347960 );


  auto _chk = [ = ]( const QString & formula, const std::vector<float> &values, bool useOpenCL )
  {

    qDebug() << formula;

#ifdef HAVE_OPENCL
    if ( ! QgsOpenClUtils::available() )
      return ;
    QgsOpenClUtils::setEnabled( useOpenCL );
#else
    Q_UNUSED( useOpenCL )
#endif

    QTemporaryFile tmpFile;
    tmpFile.open(); // fileName is not available until open
    QString tmpName = tmpFile.fileName();
    tmpFile.close();
    QgsRasterCalculator rc( formula,
                            tmpName,
                            QStringLiteral( "GTiff" ),
                            extent, crs, 2, 3, entries,
                            QgsProject::instance()->transformContext() );
    QCOMPARE( static_cast< int >( rc.processCalculation() ), 0 );
    //open output file and check results
    QgsRasterLayer *result = new QgsRasterLayer( tmpName, QStringLiteral( "result" ) );
    QCOMPARE( result->width(), 2 );
    QCOMPARE( result->height(), 3 );
    QgsRasterBlock *block = result->dataProvider()->block( 1, extent, 2, 3 );
    qDebug() << "Actual:" << block->value( 0, 0 ) << block->value( 0, 1 ) <<  block->value( 1, 0 ) <<  block->value( 1, 1 ) <<  block->value( 2, 0 ) <<  block->value( 2, 1 );
    qDebug() << "Expected:" << values[0] << values[1] <<  values[2] << values[3] << values[4] << values[5];
    const double epsilon { 0.001 };
    QVERIFY2( qgsDoubleNear( block->value( 0, 0 ), static_cast<double>( values[0] ), epsilon ), formula.toUtf8().constData() );
    QVERIFY2( qgsDoubleNear( block->value( 0, 1 ), static_cast<double>( values[1] ), epsilon ), formula.toUtf8().constData() );
    QVERIFY2( qgsDoubleNear( block->value( 1, 0 ), static_cast<double>( values[2] ), epsilon ), formula.toUtf8().constData() );
    QVERIFY2( qgsDoubleNear( block->value( 1, 1 ), static_cast<double>( values[3] ), epsilon ), formula.toUtf8().constData() );
    QVERIFY2( qgsDoubleNear( block->value( 2, 0 ), static_cast<double>( values[4] ), epsilon ), formula.toUtf8().constData() );
    QVERIFY2( qgsDoubleNear( block->value( 2, 1 ), static_cast<double>( values[5] ), epsilon ), formula.toUtf8().constData() );
    delete result;
    delete block;
  };

  _chk( QStringLiteral( "\"landsat@1\" + \"landsat_4326@2\"" ), {264.0, 263.0, 264.0, 264.0, 266.0, 261.0}, false );
  _chk( QStringLiteral( "\"landsat@1\" + \"landsat_4326@2\"" ), {264.0, 263.0, 264.0, 264.0, 266.0, 261.0}, true );
  _chk( QStringLiteral( "\"landsat@1\"^2 + 3 + \"landsat_4326@2\"" ), {15767, 15766, 15519, 15767, 15769, 15516}, false );
  _chk( QStringLiteral( "\"landsat@1\"^2 + 3 + \"landsat_4326@2\"" ), {15767, 15766, 15519, 15767, 15769, 15516}, true );
  _chk( QStringLiteral( "0.5*((2*\"landsat@1\"+1)-sqrt((2*\"landsat@1\"+1)^2-8*(\"landsat@1\"-\"landsat_4326@2\")))" ), {-0.111504f, -0.103543f, -0.128448f, -0.111504f, -0.127425f, -0.104374f}, false );
  _chk( QStringLiteral( "0.5*((2*\"landsat@1\"+1)-sqrt((2*\"landsat@1\"+1)^2-8*(\"landsat@1\"-\"landsat_4326@2\")))" ), {-0.111504f, -0.103543f, -0.128448f, -0.111504f, -0.127425f, -0.104374f}, true );
  _chk( QStringLiteral( "\"landsat@1\" * ( \"landsat@1\" > 124 )" ), {125.0, 125.0, 0.0, 125.0, 125.0, 0.0}, false );
  _chk( QStringLiteral( "\"landsat@1\" * ( \"landsat@1\" > 124 )" ), {125.0, 125.0, 0.0, 125.0, 125.0, 0.0}, true );

  // Test negative numbers
  _chk( QStringLiteral( "-2.5" ), { -2.5, -2.5, -2.5, -2.5, -2.5, -2.5 }, false );
  _chk( QStringLiteral( "- 2.5" ), { -2.5, -2.5, -2.5, -2.5, -2.5, -2.5 }, false );
  _chk( QStringLiteral( "-2.5" ), { -2.5, -2.5, -2.5, -2.5, -2.5, -2.5 }, true );
  _chk( QStringLiteral( "- 2.5" ), { -2.5, -2.5, -2.5, -2.5, -2.5, -2.5 }, true );
  _chk( QStringLiteral( "-\"landsat@1\"" ), {-125, -125, -124, -125, -125, -124}, false );
  _chk( QStringLiteral( "-\"landsat@1\"" ), {-125, -125, -124, -125, -125, -124}, true );

  // Test abs, min and max
  // landsat values: 125 125 124 125 125 124
  // landsat_4326 values: 139 138 140 139 141 137
  _chk( QStringLiteral( "abs(-123)" ), {123, 123, 123, 123, 123, 123}, false );
  _chk( QStringLiteral( "abs(-\"landsat@1\")" ), {125, 125, 124, 125, 125, 124}, true );
  _chk( QStringLiteral( "abs(-123)" ), {123, 123, 123, 123, 123, 123}, false );
  _chk( QStringLiteral( "abs(-\"landsat@1\")" ), {125, 125, 124, 125, 125, 124}, true );
  _chk( QStringLiteral( "-\"landsat_4326@2\" + 15" ), {-124, -123, -125, -124, -126, -122}, false );
  _chk( QStringLiteral( "min(-\"landsat@1\", -\"landsat_4326@2\" + 15 )" ), {-125, -125, -125, -125, -126, -124}, false );
  _chk( QStringLiteral( "min(-\"landsat@1\", -\"landsat_4326@2\" + 15 )" ), {-125, -125, -125, -125, -126, -124}, true );
  _chk( QStringLiteral( "max(-\"landsat@1\", -\"landsat_4326@2\" + 15 )" ), {-124, -123, -124, -124, -125, -122}, false );
  _chk( QStringLiteral( "max(-\"landsat@1\", -\"landsat_4326@2\" + 15 )" ), {-124, -123, -124, -124, -125, -122}, true );

}

void TestQgsRasterCalculator::testStatistics()
{
  QgsRasterCalculatorEntry entry1;
  entry1.bandNumber = 1;
  entry1.raster = mpLandsatRasterLayer;
  entry1.ref = QStringLiteral( "landsat@1" );

  QTemporaryFile tmpFile;
  tmpFile.open(); // fileName is not available until open
  QString tmpName = tmpFile.fileName();
  tmpFile.close();

  QgsCoordinateReferenceSystem crs( QStringLiteral( "EPSG:32633" ) );
  QgsRectangle extent( 783235, 3348110, 783350, 3347960 );

  QgsRasterCalculator rc( QStringLiteral( "\"landsat@1\" * 2" ),
                          tmpName,
                          QStringLiteral( "GTiff" ),
                          extent, crs, 2, 3, { entry1 },
                          QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc.processCalculation() ), 0 );

  //open output file and check stats are there
  auto ds = GDALOpenEx( tmpName.toUtf8().constData(), GDAL_OF_RASTER | GDAL_OF_READONLY, nullptr, nullptr, nullptr );
  auto band = GDALGetRasterBand( ds, 1 );
  double sMin, sMax, sMean, sStdDev;
  QCOMPARE( GDALGetRasterStatistics( band, true, false, &sMin, &sMax, &sMean, &sStdDev ), CE_None );
  QCOMPARE( sMin, 248.0 );
  QCOMPARE( sMax, 250.0 );

}

void TestQgsRasterCalculator::parseFunctionTypeString()
{
  QString errorString;
  const QgsRasterCalcNode *node { QgsRasterCalcNode::parseRasterCalcString( QString( ), errorString ) };
  QVERIFY( ! node );
  QVERIFY( ! errorString.isEmpty() );

  errorString = QString();
  node = QgsRasterCalcNode::parseRasterCalcString( QStringLiteral( "if(\"raster@1\">5,100,5)" ), errorString );
  QVERIFY( node );
  QVERIFY( errorString.isEmpty() );
  QVERIFY( node->findNodes( QgsRasterCalcNode::Type::tRasterRef ).length() == 1 );

  //test case sensitivity (instead of "if", use "IF")
  errorString = QString();
  node = QgsRasterCalcNode::parseRasterCalcString( QStringLiteral( "IF(\"raster@1\">5,100,5)" ), errorString );
  QVERIFY( node );
  QVERIFY( errorString.isEmpty() );
  QVERIFY( node->findNodes( QgsRasterCalcNode::Type::tRasterRef ).length() == 1 );
}

void TestQgsRasterCalculator::testFunctionTypeWithLayer()
{
  // first band
  QgsRasterCalculatorEntry entry1;
  entry1.bandNumber = 1;
  entry1.raster = mpLandsatRasterLayer;
  entry1.ref = QStringLiteral( "landsat@1" );

  // second band
  QgsRasterCalculatorEntry entry2;
  entry2.bandNumber = 2;
  entry2.raster = mpLandsatRasterLayer;
  entry2.ref = QStringLiteral( "landsat@2" );

  QVector<QgsRasterCalculatorEntry> entries;
  entries << entry1 << entry2;

  QgsCoordinateReferenceSystem crs( QStringLiteral( "EPSG:32633" ) );
  QgsRectangle extent( 783235, 3348110, 783350, 3347960 );

  QTemporaryFile tmpFile;
  tmpFile.open(); // fileName is not available until open
  QString tmpName = tmpFile.fileName();
  tmpFile.close();

  // Test with one raster as condition and numbers as first and second option
  // if ( landsat@1 > 124.5, 100.0 , 5.0 )
  QgsRasterCalculator rc( QStringLiteral( " if(\"landsat@1\">124.5, 100.0 , 5.0 ) " ),
                          tmpName,
                          QStringLiteral( "GTiff" ),
                          extent, crs, 2, 3, entries,
                          QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc.processCalculation() ), 0 );

  //open output file and check results
  QgsRasterLayer *result = new QgsRasterLayer( tmpName, QStringLiteral( "result" ) );
  QCOMPARE( result->width(), 2 );
  QCOMPARE( result->height(), 3 );
  QgsRasterBlock *block = result->dataProvider()->block( 1, extent, 2, 3 );

  QCOMPARE( block->value( 0, 0 ), 100.0 );
  QCOMPARE( block->value( 0, 1 ), 100.0 );
  QCOMPARE( block->value( 1, 0 ), 5.0 );
  QCOMPARE( block->value( 1, 1 ), 100.0 );
  QCOMPARE( block->value( 2, 0 ), 100.0 );
  QCOMPARE( block->value( 2, 1 ), 5.0 );

  // Test with one raster as condition, one raster first option and number as second option
  // if ( landsat@1 > 124.5, landsat@1 + landsat@2 , 5.0 )
  QgsRasterCalculator rc2( QStringLiteral( " if(\"landsat@1\">124.5, \"landsat@1\" + \"landsat@2\" , 5.0 ) " ),
                           tmpName,
                           QStringLiteral( "GTiff" ),
                           extent, crs, 2, 3, entries,
                           QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc2.processCalculation() ), 0 );

  //open output file and check results
  result = new QgsRasterLayer( tmpName, QStringLiteral( "result" ) );
  QCOMPARE( result->width(), 2 );
  QCOMPARE( result->height(), 3 );
  block = result->dataProvider()->block( 1, extent, 2, 3 );
  QCOMPARE( block->value( 0, 0 ), 265.0 );
  QCOMPARE( block->value( 0, 1 ), 263.0 );
  QCOMPARE( block->value( 1, 0 ), 5.0 );
  QCOMPARE( block->value( 1, 1 ), 264.0 );
  QCOMPARE( block->value( 2, 0 ), 266.0 );
  QCOMPARE( block->value( 2, 1 ), 5.0 );

  // Test with one raster as condition, one raster first option and number as second option
  // if ( landsat@1 > 124.5, landsat@1 + landsat@2 , landsat@3 )
  QgsRasterCalculator rc3( QStringLiteral( " if(\"landsat@1\">124.5, \"landsat@1\" + \"landsat@2\" , \"landsat@1\" - \"landsat@2\" ) " ),
                           tmpName,
                           QStringLiteral( "GTiff" ),
                           extent, crs, 2, 3, entries,
                           QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc3.processCalculation() ), 0 );

  //open output file and check results
  result = new QgsRasterLayer( tmpName, QStringLiteral( "result" ) );
  QCOMPARE( result->width(), 2 );
  QCOMPARE( result->height(), 3 );
  block = result->dataProvider()->block( 1, extent, 2, 3 );
  QCOMPARE( block->value( 0, 0 ), 265.0 );
  QCOMPARE( block->value( 0, 1 ), 263.0 );
  QCOMPARE( block->value( 1, 0 ), -15.0 );
  QCOMPARE( block->value( 1, 1 ), 264.0 );
  QCOMPARE( block->value( 2, 0 ), 266.0 );
  QCOMPARE( block->value( 2, 1 ), -13.0 );

  // Test with scalar (always true) as condition, one raster first option and number as second option
  // if ( 5 > 4, landsat@1 + landsat@2 , 0 )
  QgsRasterCalculator rc4( QStringLiteral( " if( 5>4 , \"landsat@1\" + \"landsat@2\" , 0 ) " ),
                           tmpName,
                           QStringLiteral( "GTiff" ),
                           extent, crs, 2, 3, entries,
                           QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc4.processCalculation() ), 0 );

  //open output file and check results
  result = new QgsRasterLayer( tmpName, QStringLiteral( "result" ) );
  QCOMPARE( result->width(), 2 );
  QCOMPARE( result->height(), 3 );
  block = result->dataProvider()->block( 1, extent, 2, 3 );
  QCOMPARE( block->value( 0, 0 ), 265.0 );
  QCOMPARE( block->value( 0, 1 ), 263.0 );
  QCOMPARE( block->value( 1, 0 ), 263.0 );
  QCOMPARE( block->value( 1, 1 ), 264.0 );
  QCOMPARE( block->value( 2, 0 ), 266.0 );
  QCOMPARE( block->value( 2, 1 ), 261.0 );

  // Test with scalar (always false) as condition, one raster first option and number as second option
  // if ( 4 > 5, landsat@1 + landsat@2 , 0 )
  QgsRasterCalculator rc5( QStringLiteral( " if( 4>5 , \"landsat@1\" + \"landsat@2\" , 0 ) " ),
                           tmpName,
                           QStringLiteral( "GTiff" ),
                           extent, crs, 2, 3, entries,
                           QgsProject::instance()->transformContext() );
  QCOMPARE( static_cast< int >( rc5.processCalculation() ), 0 );

  //open output file and check results
  result = new QgsRasterLayer( tmpName, QStringLiteral( "result" ) );
  QCOMPARE( result->width(), 2 );
  QCOMPARE( result->height(), 3 );
  block = result->dataProvider()->block( 1, extent, 2, 3 );
  QCOMPARE( block->value( 0, 0 ), 0.0 );
  QCOMPARE( block->value( 0, 1 ), 0.0 );
  QCOMPARE( block->value( 1, 0 ), 0.0 );
  QCOMPARE( block->value( 1, 1 ), 0.0 );
  QCOMPARE( block->value( 2, 0 ), 0.0 );
  QCOMPARE( block->value( 2, 1 ), 0.0 );
  delete result;
  delete block;
}

QGSTEST_MAIN( TestQgsRasterCalculator )
#include "testqgsrastercalculator.moc"
