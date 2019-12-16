/***************************************************************************
     testqgsmeshlayerinterpolator.cpp
     --------------------------------
    Date                 : January 2019
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

    void testMeshSingleLevelAveragingMethod_data();
    void testMeshSingleLevelAveragingMethod();

  private:
    QString mTestDataDir;
    QgsMesh3dDataBlock scalarBlock;
    QgsMesh3dDataBlock vectorBlock;
};

//runs before all tests
void TestQgsMesh3dAveraging::initTestCase()
{
  mTestDataDir = QStringLiteral( TEST_DATA_DIR ); //defined in CmakeLists.txt

  QVector<int> faceToVolumeIndex = { 0 };
  QVector<double> verticalLevels = { -1, -2, -3, -4, -5 };
  QVector<int> verticalLevelsCount = { 4 };

  scalarBlock = QgsMesh3dDataBlock( 1, false );
  QVector<double> values = { 1, 2, std::numeric_limits<double>::quiet_NaN(), 4 };
  scalarBlock.setFaceToVolumeIndex( faceToVolumeIndex );
  scalarBlock.setVerticalLevelsCount( verticalLevelsCount );
  scalarBlock.setVerticalLevels( verticalLevels );
  scalarBlock.setValues( values );
  scalarBlock.setValid( true );

  vectorBlock = QgsMesh3dDataBlock( 1, true );
  QVector<double> valuesVec = { 1, 1, 2, 2, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), 4, 4 };
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

void TestQgsMesh3dAveraging::testMeshSingleLevelAveragingMethod_data()
{
  QTest::addColumn<int>( "level" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "invalid" ) << -111 << std::numeric_limits<double>::quiet_NaN() ;
  QTest::newRow( "top" ) << 0 << 1.0;
  QTest::newRow( "1" ) << 1 << 2.0;
  QTest::newRow( "2" ) << 2 << std::numeric_limits<double>::quiet_NaN();
  QTest::newRow( "bottom" ) << 3 << 4.0;
  QTest::newRow( "outside" ) << 111 << std::numeric_limits<double>::quiet_NaN();
}

void TestQgsMesh3dAveraging::testMeshSingleLevelAveragingMethod()
{
  QFETCH( int, level );
  QFETCH( double, expected );

  QgsMeshSingleLevelAveragingMethod method( level );
  QgsMeshDataBlock block = method.calculate( scalarBlock );
  if ( level < 0 )
  {
    QVERIFY( !block.isValid() );
    return;
  }
  QVERIFY( block.isValid() );

  QgsMeshDataBlock blockVec = method.calculate( vectorBlock );
  QVERIFY( blockVec.isValid() );

  if ( std::isnan( expected ) )
  {
    QVERIFY( std::isnan( block.value( 0 ).scalar() ) );
    QVERIFY( std::isnan( blockVec.value( 0 ).x() ) );
    QVERIFY( std::isnan( blockVec.value( 0 ).y() ) );
  }
  else
  {
    QCOMPARE( block.value( 0 ).scalar(), expected );
    QCOMPARE( blockVec.value( 0 ).x(), expected );
    QCOMPARE( blockVec.value( 0 ).y(), expected );
  }
}

QGSTEST_MAIN( TestQgsMesh3dAveraging )
#include "testqgsmesh3daveraging.moc"
