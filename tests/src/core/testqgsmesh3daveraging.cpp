/***************************************************************************
     testqgsmesh3daveraging.cpp
     --------------------------------
    Date                 : December 2019
    Copyright            : (C) 2019 by Peter Petrik
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
#include <QObject>
#include <QString>
#include <QApplication>

//qgis includes...
#include <qgsrasterblock.h>
#include <qgsmeshlayer.h>
#include <qgsmeshlayerinterpolator.h>
#include <qgsapplication.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsproject.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsMesh3dAveragingMethod derived classes
 */
class TestQgsMesh3dAveraging: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testMeshSingleLevelFromTopAveragingMethod_data();
    void testMeshSingleLevelFromTopAveragingMethod();

    void testMeshSingleLevelFromBottomAveragingMethod_data();
    void testMeshSingleLevelFromBottomAveragingMethod();

    void testMeshMultiLevelsFromTopAveragingMethod_data();
    void testMeshMultiLevelsFromTopAveragingMethod();

    void testMeshMultiLevelsFromBottomAveragingMethod_data();
    void testMeshMultiLevelsFromBottomAveragingMethod();

    void testMeshSigmaAveragingMethod_data();
    void testMeshSigmaAveragingMethod();

    void testMeshDepthAveragingMethod_data();
    void testMeshDepthAveragingMethod();

    void testMeshHeightAveragingMethod_data();
    void testMeshHeightAveragingMethod();

    void testMeshElevationAveragingMethod_data();
    void testMeshElevationAveragingMethod();
    void testMeshElevationAveragingMethodVariableMesh();

  private:
    void compare( const QgsMesh3dAveragingMethod *method, double expected, bool valid );

    QString mTestDataDir;
    QgsMesh3dDataBlock scalarBlock;
    QgsMesh3dDataBlock vectorBlock;
};

void TestQgsMesh3dAveraging::compare( const QgsMesh3dAveragingMethod *method, double expected, bool valid )
{
  const QgsMeshDataBlock block = method->calculate( scalarBlock );
  if ( !valid )
  {
    QVERIFY( !block.isValid() );
    return;
  }
  QVERIFY( block.isValid() );
  QVERIFY( block.count() == 2 );

  const QgsMeshDataBlock blockVec = method->calculate( vectorBlock );
  QVERIFY( blockVec.isValid() );
  QVERIFY( blockVec.count() == 2 );

  for ( int i = 0; i < 2; ++i )
  {
    if ( std::isnan( expected ) )
    {
      QVERIFY( std::isnan( block.value( i ).scalar() ) );
      QVERIFY( std::isnan( blockVec.value( i ).x() ) );
      QVERIFY( std::isnan( blockVec.value( i ).y() ) );
    }
    else
    {
      QCOMPARE( block.value( i ).scalar(), expected );
      QCOMPARE( blockVec.value( i ).x(), expected );
      QCOMPARE( blockVec.value( i ).y(), expected );
    }
  }
}

//runs before all tests
void TestQgsMesh3dAveraging::initTestCase()
{
  mTestDataDir = QStringLiteral( TEST_DATA_DIR ); //defined in CmakeLists.txt

  const QVector<int> faceToVolumeIndex = { 0, 4 };

  // so intervals are { 1, 0.5, 1.5, 1 }
  const QVector<double> verticalLevels = { -1.0, -2.0, -2.5, -4.0, -5.0,
                                           -1.0, -2.0, -2.5, -4.0, -5.0
                                         };

  const QVector<int> verticalLevelsCount = { 4, 4 };

  scalarBlock = QgsMesh3dDataBlock( 2, false );
  const QVector<double> values = { 1, 2, std::numeric_limits<double>::quiet_NaN(), 4,
                                   1, 2, std::numeric_limits<double>::quiet_NaN(), 4
                                 };

  scalarBlock.setFaceToVolumeIndex( faceToVolumeIndex );
  scalarBlock.setVerticalLevelsCount( verticalLevelsCount );
  scalarBlock.setVerticalLevels( verticalLevels );
  scalarBlock.setValues( values );
  scalarBlock.setValid( true );

  vectorBlock = QgsMesh3dDataBlock( 2, true );
  const QVector<double> valuesVec = { 1, 1, 2, 2, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), 4, 4,
                                      1, 1, 2, 2, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), 4, 4
                                    };
  vectorBlock.setFaceToVolumeIndex( faceToVolumeIndex );
  vectorBlock.setVerticalLevelsCount( verticalLevelsCount );
  vectorBlock.setVerticalLevels( verticalLevels );
  vectorBlock.setValues( valuesVec );
  vectorBlock.setValid( true );
}
//runs after all tests
void TestQgsMesh3dAveraging::cleanupTestCase()
{
}

void TestQgsMesh3dAveraging::testMeshSingleLevelFromTopAveragingMethod_data()
{
  QTest::addColumn<int>( "level" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "invalid" ) << -111 << std::numeric_limits<double>::quiet_NaN() ;
  QTest::newRow( "top" ) << 1 << 1.0;
  QTest::newRow( "2" ) << 2 << 2.0;
  QTest::newRow( "3" ) << 3 << std::numeric_limits<double>::quiet_NaN();
  QTest::newRow( "bottom" ) << 4 << 4.0;
  QTest::newRow( "outside" ) << 111 << std::numeric_limits<double>::quiet_NaN();
}

void TestQgsMesh3dAveraging::testMeshSingleLevelFromTopAveragingMethod()
{
  QFETCH( int, level );
  QFETCH( double, expected );

  QgsMeshMultiLevelsAveragingMethod method( level, true );
  compare( &method, expected, level >= 1 );
}

void TestQgsMesh3dAveraging::testMeshSingleLevelFromBottomAveragingMethod_data()
{
  QTest::addColumn<int>( "level" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "invalid" ) << -111 << std::numeric_limits<double>::quiet_NaN() ;
  QTest::newRow( "bottom" ) << 1 << 4.0;
  QTest::newRow( "2" ) << 2 << std::numeric_limits<double>::quiet_NaN();
  QTest::newRow( "3" ) << 3 << 2.0;
  QTest::newRow( "top" ) << 4 << 1.0;
  QTest::newRow( "outside" ) << 111 << std::numeric_limits<double>::quiet_NaN();
}

void TestQgsMesh3dAveraging::testMeshSingleLevelFromBottomAveragingMethod()
{
  QFETCH( int, level );
  QFETCH( double, expected );

  QgsMeshMultiLevelsAveragingMethod method( level, false );
  compare( &method, expected, level >= 1 );
}

void TestQgsMesh3dAveraging::testMeshMultiLevelsFromTopAveragingMethod_data()
{
  QTest::addColumn<int>( "startLevel" );
  QTest::addColumn<int>( "endLevel" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "invalid" ) << -111 << -111 << std::numeric_limits<double>::quiet_NaN() ;
  QTest::newRow( "invalid2" ) << 1 << -111 << std::numeric_limits<double>::quiet_NaN() ;
  QTest::newRow( "single level" ) << 1 << 1 << 1.0;
  QTest::newRow( "1to2" ) << 1 << 2 << ( 1.0 * 1.0 + 2.0 * 0.5 ) / 1.5 ;
  QTest::newRow( "1to3" ) << 1 << 3 << ( 1.0 * 1.0 + 2.0 * 0.5 ) / 1.5 ;
  QTest::newRow( "2to4" ) << 2 << 4 << ( 2.0 * 0.5 + 4.0 * 1.0 ) / 1.5 ;
  QTest::newRow( "outside" ) << 100 << 111 << std::numeric_limits<double>::quiet_NaN();
  QTest::newRow( "outside2" ) << 1 << 111 << ( 1.0 * 1.0 + 2.0 * 0.5 + 4.0 * 1.0 ) / 2.5 ;
  QTest::newRow( "reverted" ) << 111 << 1 << ( 1.0 * 1.0 + 2.0 * 0.5 + 4.0 * 1.0 ) / 2.5 ;
}

void TestQgsMesh3dAveraging::testMeshMultiLevelsFromTopAveragingMethod()
{
  QFETCH( int, startLevel );
  QFETCH( int, endLevel );
  QFETCH( double, expected );

  QgsMeshMultiLevelsAveragingMethod method( startLevel, endLevel, true );
  compare( &method, expected, startLevel >= 1 && endLevel >= 1 );
}

void TestQgsMesh3dAveraging::testMeshMultiLevelsFromBottomAveragingMethod_data()
{
  QTest::addColumn<int>( "startLevel" );
  QTest::addColumn<int>( "endLevel" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "invalid" ) << -111 << -111 << std::numeric_limits<double>::quiet_NaN() ;
  QTest::newRow( "invalid2" ) << 1 << -111 << std::numeric_limits<double>::quiet_NaN() ;
  QTest::newRow( "single level" ) << 1 << 1 << 4.0;
  QTest::newRow( "1to2" ) << 1 << 2 << 4.0;
  QTest::newRow( "1to3" ) << 1 << 3 << ( 4.0 * 1.0 + 2.0 * 0.5 ) / 1.5 ;
  QTest::newRow( "2to4" ) << 2 << 4 << ( 2.0 * 0.5 + 1.0 * 1.0 ) / 1.5 ;
  QTest::newRow( "outside" ) << 100 << 111 << std::numeric_limits<double>::quiet_NaN();
  QTest::newRow( "outside2" ) << 1 << 111 << ( 1.0 * 1.0 + 2.0 * 0.5 + 4.0 * 1.0 ) / 2.5 ;
  QTest::newRow( "reverted" ) << 111 << 1 << ( 1.0 * 1.0 + 2.0 * 0.5 + 4.0 * 1.0 ) / 2.5 ;
}

void TestQgsMesh3dAveraging::testMeshMultiLevelsFromBottomAveragingMethod()
{
  QFETCH( int, startLevel );
  QFETCH( int, endLevel );
  QFETCH( double, expected );

  QgsMeshMultiLevelsAveragingMethod method( startLevel, endLevel, false );
  compare( &method, expected, startLevel >= 1 && endLevel >= 1 );
}

void TestQgsMesh3dAveraging::testMeshSigmaAveragingMethod_data()
{
  QTest::addColumn<double>( "startParam" );
  QTest::addColumn<double>( "endParam" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "invalid" ) << -1.0 << -111.0 << std::numeric_limits<double>::quiet_NaN() ;
  QTest::newRow( "invalid2" ) << 1.1 << 111.0 << std::numeric_limits<double>::quiet_NaN() ;
  QTest::newRow( "invalid3" ) << -1.0 << 111.0 << std::numeric_limits<double>::quiet_NaN() ;
  QTest::newRow( "full" ) << 0.0 << 1.0 << ( 1.0 * 1.0 + 2.0 * 0.5 + 4.0 * 1.0 ) / 2.5;
}

void TestQgsMesh3dAveraging::testMeshSigmaAveragingMethod()
{
  QFETCH( double, startParam );
  QFETCH( double, endParam );
  QFETCH( double, expected );

  QgsMeshSigmaAveragingMethod method( startParam, endParam );
  compare( &method, expected, startParam >= 0 && endParam <= 1 );
}

void TestQgsMesh3dAveraging::testMeshDepthAveragingMethod_data()
{
  QTest::addColumn<double>( "startParam" );
  QTest::addColumn<double>( "endParam" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "invalid" ) << -1.0 << -111.0 << std::numeric_limits<double>::quiet_NaN() ;
  QTest::newRow( "1to2" ) << 0.0 << 1.5 << ( 1.0 * 1.0 + 2.0 * 0.5 ) / 1.5;
  QTest::newRow( "25to45" ) << 1.25 << 3.5 << ( 2.0 * 0.25 + 4.0 * 0.5 ) / 0.75;
  QTest::newRow( "full" ) << 0.0 << 10.0 << ( 1.0 * 1.0 + 2.0 * 0.5 + 4.0 * 1.0 ) / 2.5;
}

void TestQgsMesh3dAveraging::testMeshDepthAveragingMethod()
{
  QFETCH( double, startParam );
  QFETCH( double, endParam );
  QFETCH( double, expected );

  QgsMeshRelativeHeightAveragingMethod method( startParam, endParam, true );
  compare( &method, expected, startParam >= 0 );
}

void TestQgsMesh3dAveraging::testMeshHeightAveragingMethod_data()
{
  QTest::addColumn<double>( "startParam" );
  QTest::addColumn<double>( "endParam" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "invalid" ) << -1.0 << -111.0 << std::numeric_limits<double>::quiet_NaN() ;
  QTest::newRow( "1to2" ) << 2.5 << 4.0 << ( 1.0 * 1.0 + 2.0 * 0.5 ) / 1.5;
  QTest::newRow( "25to45" ) << 2.75 << 0.5 << ( 2.0 * 0.25 + 4.0 * 0.5 ) / 0.75;
  QTest::newRow( "full" ) << 0.0 << 10.0 << ( 1.0 * 1.0 + 2.0 * 0.5 + 4.0 * 1.0 ) / 2.5;
}

void TestQgsMesh3dAveraging::testMeshHeightAveragingMethod()
{
  QFETCH( double, startParam );
  QFETCH( double, endParam );
  QFETCH( double, expected );

  QgsMeshRelativeHeightAveragingMethod method( startParam, endParam, false );
  compare( &method, expected, startParam >= 0 );
}

void TestQgsMesh3dAveraging::testMeshElevationAveragingMethod_data()
{
  QTest::addColumn<double>( "startParam" );
  QTest::addColumn<double>( "endParam" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "invalid" ) << 1.0 << 111.0 << std::numeric_limits<double>::quiet_NaN() ;
  QTest::newRow( "1to2" ) << -1.0 << -2.5 << ( 1.0 * 1.0 + 2.0 * 0.5 ) / 1.5;
  QTest::newRow( "25to45" ) << -2.25 << -4.5 << ( 2.0 * 0.25 + 4.0 * 0.5 ) / 0.75;
  QTest::newRow( "full" ) << 0.0 << -10.0 << ( 1.0 * 1.0 + 2.0 * 0.5 + 4.0 * 1.0 ) / 2.5;
}

void TestQgsMesh3dAveraging::testMeshElevationAveragingMethod()
{
  QFETCH( double, startParam );
  QFETCH( double, endParam );
  QFETCH( double, expected );

  QgsMeshElevationAveragingMethod method( startParam, endParam );
  compare( &method, expected, startParam <= 0 );
}

void TestQgsMesh3dAveraging::testMeshElevationAveragingMethodVariableMesh()
{
  // Test the situation when the number of vertical levels is different for
  // each face and also that for face 1 the vertical levels are outside of
  // requested elevation range
  const QVector<int> faceToVolumeIndex = { 0, 4, 7 };
  const QVector<double> verticalLevels = { -1.0, -2.0, -3.0, -4.0, -5.0,
                                           -1.0, -1.1, -1.3, -1.5,
                                           -1.0, -2.0, -3.0, -4.0, -5.0, -6.0
                                         };

  const QVector<int> verticalLevelsCount = { 4, 3, 5 };

  QgsMesh3dDataBlock scalarBlock2( 3, false );
  const QVector<double> values = { 1, 2, 3, 4,
                                   0, 0, 0,
                                   100, 200, 300, 400, 500
                                 };

  scalarBlock2.setFaceToVolumeIndex( faceToVolumeIndex );
  scalarBlock2.setVerticalLevelsCount( verticalLevelsCount );
  scalarBlock2.setVerticalLevels( verticalLevels );
  scalarBlock2.setValues( values );
  scalarBlock2.setValid( true );

  const QgsMeshElevationAveragingMethod method( -2.0, -6.0 );
  const QgsMeshDataBlock block = method.calculate( scalarBlock2 );
  QVERIFY( block.isValid() );
  QVERIFY( block.count() == 3 );

  QCOMPARE( block.value( 0 ).scalar(), ( 2 + 3 + 4 ) / 3.0 );
  QVERIFY( std::isnan( block.value( 1 ).scalar() ) );
  QCOMPARE( block.value( 2 ).scalar(), ( 200 + 300 + 400 + 500 ) / 4.0 );
}

QGSTEST_MAIN( TestQgsMesh3dAveraging )
#include "testqgsmesh3daveraging.moc"
