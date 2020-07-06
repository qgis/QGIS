/***************************************************************************
  testqgsmeshcalculator.cpp
  --------------------------------------
Date                 : December 2018
Copyright            : (C) 2018 by Peter Petrik
Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <limits>
#include <cmath>

#include "qgsmeshcalculator.h"
#include "qgsmeshcalcnode.h"
#include "qgsmeshdataprovider.h"
#include "qgsmeshlayer.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsmeshmemorydataprovider.h"

Q_DECLARE_METATYPE( QgsMeshCalcNode::Operator )

class TestQgsMeshCalculator : public QObject
{
    Q_OBJECT

  public:
    TestQgsMeshCalculator() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() ;// will be called before each testfunction is executed.
    void cleanup() ;// will be called after every testfunction.

    void dualOp_data();
    void dualOp(); //test operators which operate on a left&right node

    void singleOp_data();
    void singleOp(); //test operators which operate on a single value

    void calcWithVertexLayers();
    void calcWithFaceLayers();
    void calcWithMixedLayers();

    void calcAndSave();

  private:

    QgsMeshLayer *mpMeshLayer = nullptr;
};

void  TestQgsMeshCalculator::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/mesh/" );
  QString uri( testDataDir + "/quad_and_triangle.2dm" );
  mpMeshLayer = new QgsMeshLayer( uri, "Triangle and Quad MDAL", "mdal" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_vertex_scalar.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_vertex_scalar2.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_vertex_scalar_max.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_vertex_vector.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_vertex_vector2.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_vertex_vector_max.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_els_face_scalar.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_els_face_vector.dat" );

  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpMeshLayer );
}

void  TestQgsMeshCalculator::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void  TestQgsMeshCalculator::init()
{
}
void  TestQgsMeshCalculator::cleanup()
{
}

void TestQgsMeshCalculator::dualOp_data()
{
  QTest::addColumn< QgsMeshCalcNode::Operator >( "op" );
  QTest::addColumn<double>( "left" );
  QTest::addColumn<double>( "right" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "opPlus" ) << QgsMeshCalcNode::opPLUS << 5.5 << 2.2 << 7.7;
  QTest::newRow( "opMINUS" ) << QgsMeshCalcNode::opMINUS << 5.0 << 2.5 << 2.5;
  QTest::newRow( "opMUL" ) << QgsMeshCalcNode::opMUL << 2.5 << 4.0 << 10.0;
  QTest::newRow( "opDIV" ) << QgsMeshCalcNode::opDIV << 2.5 << 2.0 << 1.25;
  QTest::newRow( "opDIV by 0" ) << QgsMeshCalcNode::opDIV << 2.5 << 0.0 << std::numeric_limits<double>::quiet_NaN();
  QTest::newRow( "opPOW" ) << QgsMeshCalcNode::opPOW << 3.0 << 2.0 << 9.0;
  QTest::newRow( "opPOW negative" ) << QgsMeshCalcNode::opPOW << 4.0 << -2.0 << 0.0625;
  QTest::newRow( "opPOW sqrt" ) << QgsMeshCalcNode::opPOW << 4.0 << 0.5 << 2.0;
  QTest::newRow( "opPOW complex" ) << QgsMeshCalcNode::opPOW << -2.0 << 0.5 << std::numeric_limits<double>::quiet_NaN();
  QTest::newRow( "opEQ true" ) << QgsMeshCalcNode::opEQ << 1.0 << 1.0 << 1.0;
  QTest::newRow( "opEQ false" ) << QgsMeshCalcNode::opEQ << 0.5 << 1.0 << 0.0;
  QTest::newRow( "opNE equal" ) << QgsMeshCalcNode::opNE << 1.0 << 1.0 << 0.0;
  QTest::newRow( "opNE not equal" ) << QgsMeshCalcNode::opNE << 0.5 << 1.0 << 1.0;
  QTest::newRow( "opGT >" ) << QgsMeshCalcNode::opGT << 1.0 << 0.5 << 1.0;
  QTest::newRow( "opGT =" ) << QgsMeshCalcNode::opGT << 0.5 << 0.5 << 0.0;
  QTest::newRow( "opGT <" ) << QgsMeshCalcNode::opGT << 0.5 << 1.0 << 0.0;
  QTest::newRow( "opLT >" ) << QgsMeshCalcNode::opLT << 1.0 << 0.5 << 0.0;
  QTest::newRow( "opLT =" ) << QgsMeshCalcNode::opLT << 0.5 << 0.5 << 0.0;
  QTest::newRow( "opLT <" ) << QgsMeshCalcNode::opLT << 0.5 << 1.0 << 1.0;
  QTest::newRow( "opGE >" ) << QgsMeshCalcNode::opGE << 1.0 << 0.5 << 1.0;
  QTest::newRow( "opGE =" ) << QgsMeshCalcNode::opGE << 0.5 << 0.5 << 1.0;
  QTest::newRow( "opGE <" ) << QgsMeshCalcNode::opGE << 0.5 << 1.0 << 0.0;
  QTest::newRow( "opLE >" ) << QgsMeshCalcNode::opLE << 1.0 << 0.5 << 0.0;
  QTest::newRow( "opLE =" ) << QgsMeshCalcNode::opLE << 0.5 << 0.5 << 1.0;
  QTest::newRow( "opLE <" ) << QgsMeshCalcNode::opLE << 0.5 << 1.0 << 1.0;
  QTest::newRow( "opAND 0/0" ) << QgsMeshCalcNode::opAND << 0.0 << 0.0 << 0.0;
  QTest::newRow( "opAND 0/1" ) << QgsMeshCalcNode::opAND << 0.0 << 1.0 << 0.0;
  QTest::newRow( "opAND 1/0" ) << QgsMeshCalcNode::opAND << 1.0 << 0.0 << 0.0;
  QTest::newRow( "opAND 1/1" ) << QgsMeshCalcNode::opAND << 1.0 << 1.0 << 1.0;
  QTest::newRow( "opOR 0/0" ) << QgsMeshCalcNode::opOR << 0.0 << 0.0 << 0.0;
  QTest::newRow( "opOR 0/1" ) << QgsMeshCalcNode::opOR << 0.0 << 1.0 << 1.0;
  QTest::newRow( "opOR 1/0" ) << QgsMeshCalcNode::opOR << 1.0 << 0.0 << 1.0;
  QTest::newRow( "opOR 1/1" ) << QgsMeshCalcNode::opOR << 1.0 << 1.0 << 1.0;
}

void TestQgsMeshCalculator::dualOp()
{
  QFETCH( QgsMeshCalcNode::Operator, op );
  QFETCH( double, left );
  QFETCH( double, right );
  QFETCH( double, expected );

  QgsMeshCalcNode node( op, new QgsMeshCalcNode( left ), new QgsMeshCalcNode( right ) );

  QgsMeshMemoryDatasetGroup result( "result" );
  QStringList usedDatasetNames;
  usedDatasetNames << "VertexScalarDataset2" << "VertexScalarDataset" << "VertexScalarDatasetMax";

  QgsMeshCalcUtils utils( mpMeshLayer,
                          usedDatasetNames,
                          0,
                          3600 );

  QVERIFY( node.calculate( utils, result ) );
  QCOMPARE( result.datasetCount(), 1 );
  std::shared_ptr<QgsMeshMemoryDataset> ds = result.memoryDatasets[0];
  for ( int i = 0; i < ds->values.size(); ++i )
  {
    double val = ds->values.at( i ).scalar();
    if ( std::isnan( expected ) )
    {
      QVERIFY( std::isnan( val ) );
    }
    else
    {
      QCOMPARE( val, expected );
    }
  }
}

void TestQgsMeshCalculator::singleOp_data()
{
  QTest::addColumn< QgsMeshCalcNode::Operator >( "op" );
  QTest::addColumn<double>( "value" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "opSIGN +" ) << QgsMeshCalcNode::opSIGN << 1.0 << -1.0;
  QTest::newRow( "opSIGN -" ) << QgsMeshCalcNode::opSIGN << -1.0 << 1.0;
  QTest::newRow( "opABS +" ) << QgsMeshCalcNode::opABS << 1.0 << 1.0;
  QTest::newRow( "opABS -" ) << QgsMeshCalcNode::opABS << -1.0 << 1.0;
}

void TestQgsMeshCalculator::singleOp()
{
  QFETCH( QgsMeshCalcNode::Operator, op );
  QFETCH( double, value );
  QFETCH( double, expected );

  QgsMeshCalcNode node( op, new QgsMeshCalcNode( value ), nullptr );

  QgsMeshMemoryDatasetGroup result( "result" );
  QStringList usedDatasetNames;
  usedDatasetNames << "VertexScalarDataset";

  QgsMeshCalcUtils utils( mpMeshLayer,
                          usedDatasetNames,
                          0,
                          3600 );

  QVERIFY( node.calculate( utils, result ) );

  QCOMPARE( result.datasetCount(), 1 );
  std::shared_ptr<QgsMeshMemoryDataset> ds = result.memoryDatasets[0];
  for ( int i = 0; i < ds->values.size(); ++i )
  {
    double val = ds->values.at( i ).scalar();
    if ( std::isnan( expected ) )
    {
      QVERIFY( std::isnan( val ) );
    }
    else
    {
      QCOMPARE( val, expected );
    }
  }
}

void TestQgsMeshCalculator::calcWithVertexLayers()
{
  QgsRectangle extent( 1000.000, 1000.000, 3000.000, 3000.000 );

  QTemporaryFile tmpFile;
  tmpFile.open(); // fileName is not available until open
  QString tmpName = tmpFile.fileName();
  tmpFile.close();

  QgsMeshCalculator rc( QStringLiteral( "\"VertexScalarDataset\" + 2" ),
                        QStringLiteral( "BINARY_DAT" ),
                        "NewVertexScalarDataset",
                        tmpName,
                        extent,
                        0,
                        3600,
                        mpMeshLayer
                      );
  int groupCount = mpMeshLayer->dataProvider()->datasetGroupCount();
  QCOMPARE( static_cast< int >( rc.processCalculation() ), 0 );
  int newGroupCount = mpMeshLayer->dataProvider()->datasetGroupCount();
  QCOMPARE( newGroupCount, groupCount + 1 );
  const QgsMeshDatasetValue val = mpMeshLayer->dataProvider()->datasetValue( QgsMeshDatasetIndex( newGroupCount - 1, 0 ), 0 );
  QCOMPARE( val.scalar(), 3.0 );
}

void TestQgsMeshCalculator::calcWithFaceLayers()
{
  QgsRectangle extent( 1000.000, 1000.000, 3000.000, 3000.000 );

  QTemporaryFile tmpFile;
  tmpFile.open(); // fileName is not available until open
  QString tmpName = tmpFile.fileName();
  tmpFile.close();

  QgsMeshCalculator rc( QStringLiteral( "\"FaceScalarDataset\" + 2" ),
                        QStringLiteral( "ASCII_DAT" ),
                        "NewFaceScalarDataset",
                        tmpName,
                        extent,
                        0,
                        3600,
                        mpMeshLayer
                      );
  int groupCount = mpMeshLayer->dataProvider()->datasetGroupCount();
  QCOMPARE( static_cast< int >( rc.processCalculation() ), 0 );
  int newGroupCount = mpMeshLayer->dataProvider()->datasetGroupCount();
  QCOMPARE( newGroupCount, groupCount + 1 );
  const QgsMeshDatasetValue val = mpMeshLayer->dataProvider()->datasetValue( QgsMeshDatasetIndex( newGroupCount - 1, 0 ), 0 );
  QCOMPARE( val.scalar(), 3.0 );
}

void TestQgsMeshCalculator::calcWithMixedLayers()
{
  QgsRectangle extent( 1000.000, 1000.000, 3000.000, 3000.000 );

  QTemporaryFile tmpFile;
  tmpFile.open(); // fileName is not available until open
  QString tmpName = tmpFile.fileName();
  tmpFile.close();

  QgsMeshCalculator rc( QStringLiteral( "\"VertexScalarDataset\" + \"FaceScalarDataset\"" ),
                        QStringLiteral( "BINARY_DAT" ),
                        "NewMixScalarDataset",
                        tmpName,
                        extent,
                        0,
                        3600,
                        mpMeshLayer
                      );
  int groupCount = mpMeshLayer->dataProvider()->datasetGroupCount();
  QCOMPARE( static_cast< int >( rc.processCalculation() ), 0 );
  int newGroupCount = mpMeshLayer->dataProvider()->datasetGroupCount();
  QCOMPARE( newGroupCount, groupCount + 1 );
  const QgsMeshDatasetValue val = mpMeshLayer->dataProvider()->datasetValue( QgsMeshDatasetIndex( newGroupCount - 1, 0 ), 0 );
  QCOMPARE( val.scalar(), 2.0 );
}

void TestQgsMeshCalculator::calcAndSave()
{
  QgsRectangle extent( 1000.000, 1000.000, 3000.000, 3000.000 );

  QString tempFilePath = QDir::tempPath() + '/' + "meshCalculatorResult.out";
  QgsMeshCalculator rc( QStringLiteral( "\"VertexScalarDataset\" + \"FaceScalarDataset\"" ),
                        QStringLiteral( "BINARY_DAT" ),
                        "NewMixScalarDataset",
                        tempFilePath,
                        extent,
                        0,
                        3600,
                        mpMeshLayer
                      );

  rc.processCalculation();

  QFileInfo fileInfo( tempFilePath );
  QVERIFY( fileInfo.exists() );
}

QGSTEST_MAIN( TestQgsMeshCalculator )
#include "testqgsmeshcalculator.moc"
