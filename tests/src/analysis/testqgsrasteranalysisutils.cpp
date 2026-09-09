/***************************************************************************
                         testqgsrasteranalysisutils.cpp
                         ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproviderregistry.h"
#include "qgsrasteranalysisutils.h"
#include "qgsrasterblock.h"
#include "qgstest.h"

#include <QString>
#include <QTemporaryDir>

using namespace Qt::StringLiterals;

class TestQgsRasterAnalysisUtils : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsRasterAnalysisUtils()
      : QgsTest( u"Raster Analysis Utils Test"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.
    void testNeighborCellDistance();
    void testNeighborCellCoordinates();
    void testSteepestGradientDirection();
};

void TestQgsRasterAnalysisUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsRasterAnalysisUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRasterAnalysisUtils::testNeighborCellDistance()
{
  const double dx = 10.0;
  const double dy = 10.0;
  const double diag = std::hypot( dx, dy );

  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 0, dx, dy ), dy );   // North
  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 1, dx, dy ), diag ); // North-East
  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 2, dx, dy ), dx );   // East
  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 3, dx, dy ), diag ); // South-East
  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 4, dx, dy ), dy );   // South
  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 5, dx, dy ), diag ); // South-West
  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 6, dx, dy ), dx );   // West
  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 7, dx, dy ), diag ); // North-West

  const double adx = 5.0;
  const double ady = 20.0;
  const double adiag = std::hypot( adx, ady );

  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 0, adx, ady ), 20.0 );
  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 1, adx, ady ), adiag );
  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 2, adx, ady ), 5.0 );
  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 3, adx, ady ), adiag );
  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 4, adx, ady ), 20.0 );
  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 5, adx, ady ), adiag );
  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 6, adx, ady ), 5.0 );
  QCOMPARE( QgsRasterAnalysisUtils::neighborCellDistance( 7, adx, ady ), adiag );
}

void TestQgsRasterAnalysisUtils::testNeighborCellCoordinates()
{
  int nCol = -1;
  int nRow = -1;
  const int cols = 5;
  const int rows = 5;

  // from center cell (2, 2) - all 8 neighbors are within grid bounds
  QVERIFY( QgsRasterAnalysisUtils::neighborCellCoordinates( 0, 2, 2, nRow, nCol, rows, cols ) ); // North
  QCOMPARE( nCol, 2 );
  QCOMPARE( nRow, 1 );

  QVERIFY( QgsRasterAnalysisUtils::neighborCellCoordinates( 1, 2, 2, nRow, nCol, rows, cols ) ); // North-East
  QCOMPARE( nCol, 3 );
  QCOMPARE( nRow, 1 );

  QVERIFY( QgsRasterAnalysisUtils::neighborCellCoordinates( 2, 2, 2, nRow, nCol, rows, cols ) ); // East
  QCOMPARE( nCol, 3 );
  QCOMPARE( nRow, 2 );

  QVERIFY( QgsRasterAnalysisUtils::neighborCellCoordinates( 3, 2, 2, nRow, nCol, rows, cols ) ); // South-East
  QCOMPARE( nCol, 3 );
  QCOMPARE( nRow, 3 );

  QVERIFY( QgsRasterAnalysisUtils::neighborCellCoordinates( 4, 2, 2, nRow, nCol, rows, cols ) ); // South
  QCOMPARE( nCol, 2 );
  QCOMPARE( nRow, 3 );

  QVERIFY( QgsRasterAnalysisUtils::neighborCellCoordinates( 5, 2, 2, nRow, nCol, rows, cols ) ); // South-West
  QCOMPARE( nCol, 1 );
  QCOMPARE( nRow, 3 );

  QVERIFY( QgsRasterAnalysisUtils::neighborCellCoordinates( 6, 2, 2, nRow, nCol, rows, cols ) ); // West
  QCOMPARE( nCol, 1 );
  QCOMPARE( nRow, 2 );

  QVERIFY( QgsRasterAnalysisUtils::neighborCellCoordinates( 7, 2, 2, nRow, nCol, rows, cols ) ); // North-West
  QCOMPARE( nCol, 1 );
  QCOMPARE( nRow, 1 );

  // top left corner (0, 0)
  QVERIFY( !QgsRasterAnalysisUtils::neighborCellCoordinates( 0, 0, 0, nRow, nCol, rows, cols ) ); // N out of bounds
  QVERIFY( !QgsRasterAnalysisUtils::neighborCellCoordinates( 1, 0, 0, nRow, nCol, rows, cols ) ); // NE out of bounds
  QVERIFY( QgsRasterAnalysisUtils::neighborCellCoordinates( 2, 0, 0, nRow, nCol, rows, cols ) );  // E in bounds (1, 0)
  QCOMPARE( nCol, 1 );
  QCOMPARE( nRow, 0 );
  QVERIFY( QgsRasterAnalysisUtils::neighborCellCoordinates( 3, 0, 0, nRow, nCol, rows, cols ) );  // SE in bounds (1, 1)
  QVERIFY( QgsRasterAnalysisUtils::neighborCellCoordinates( 4, 0, 0, nRow, nCol, rows, cols ) );  // S in bounds (0, 1)
  QVERIFY( !QgsRasterAnalysisUtils::neighborCellCoordinates( 5, 0, 0, nRow, nCol, rows, cols ) ); // SW out of bounds
  QVERIFY( !QgsRasterAnalysisUtils::neighborCellCoordinates( 6, 0, 0, nRow, nCol, rows, cols ) ); // W out of bounds
  QVERIFY( !QgsRasterAnalysisUtils::neighborCellCoordinates( 7, 0, 0, nRow, nCol, rows, cols ) ); // NW out of bounds

  // bottom right corner (4, 4)
  QVERIFY( !QgsRasterAnalysisUtils::neighborCellCoordinates( 2, 4, 4, nRow, nCol, rows, cols ) ); // E out of bounds
  QVERIFY( !QgsRasterAnalysisUtils::neighborCellCoordinates( 3, 4, 4, nRow, nCol, rows, cols ) ); // SE out of bounds
  QVERIFY( !QgsRasterAnalysisUtils::neighborCellCoordinates( 4, 4, 4, nRow, nCol, rows, cols ) ); // S out of bounds
  QVERIFY( QgsRasterAnalysisUtils::neighborCellCoordinates( 0, 4, 4, nRow, nCol, rows, cols ) );  // N in bounds (4, 3)
  QVERIFY( QgsRasterAnalysisUtils::neighborCellCoordinates( 6, 4, 4, nRow, nCol, rows, cols ) );  // W in bounds (3, 4)
  QVERIFY( QgsRasterAnalysisUtils::neighborCellCoordinates( 7, 4, 4, nRow, nCol, rows, cols ) );  // NW in bounds (3, 3)
}

void TestQgsRasterAnalysisUtils::testSteepestGradientDirection()
{
  const double nodata = -9999.0;
  QgsRasterBlock block( Qgis::DataType::Float32, 3, 3 );
  block.setNoDataValue( nodata );

  // fill initial 3x3 elevation block with uniform value 10.0
  for ( int r = 0; r < 3; ++r )
  {
    for ( int c = 0; c < 3; ++c )
    {
      block.setValue( r, c, 10.0 );
    }
  }

  // steepest slope to East (2)
  block.setValue( 1, 2, 2.0 );
  QCOMPARE( QgsRasterAnalysisUtils::steepestGradientDirection( &block, 1, 1, 10.0, 10.0, true, false ), 2 );
  QCOMPARE( QgsRasterAnalysisUtils::steepestGradientDirection( &block, 1, 1, 10.0, 10.0, true, true ), 2 );

  // center cell is NoData -> returns -1
  block.setValue( 1, 1, nodata );
  QCOMPARE( QgsRasterAnalysisUtils::steepestGradientDirection( &block, 1, 1, 10.0, 10.0, true, false ), -1 );
  block.setValue( 1, 1, 10.0 );

  // Neighbor cell is NoData with noEdges = true vs false
  block.setValue( 0, 1, nodata );                                                                            // Set North neighbor to NoData
  QCOMPARE( QgsRasterAnalysisUtils::steepestGradientDirection( &block, 1, 1, 10.0, 10.0, true, true ), -1 ); // noEdges = true fails
  QCOMPARE( QgsRasterAnalysisUtils::steepestGradientDirection( &block, 1, 1, 10.0, 10.0, true, false ), 2 ); // noEdges = false ignores North, finds East
  block.setValue( 0, 1, 10.0 );                                                                              // Restore North neighbor

  // Edge cell (0, 0) with noEdges = true vs false
  QCOMPARE( QgsRasterAnalysisUtils::steepestGradientDirection( &block, 0, 0, 10.0, 10.0, true, true ), -1 ); // Edge cell fails when noEdges = true
  block.setValue( 0, 1, 5.0 );                                                                               // East of (0, 0)
  QCOMPARE( QgsRasterAnalysisUtils::steepestGradientDirection( &block, 0, 0, 10.0, 10.0, true, false ), 2 ); // Finds East on edge when noEdges = false

  // Pit cell (all neighbors higher) and testing down = false
  // Set all neighbors to 20.0, Center = 10.0
  for ( int r = 0; r < 3; ++r )
  {
    for ( int c = 0; c < 3; ++c )
    {
      block.setValue( r, c, 20.0 );
    }
  }
  block.setValue( 1, 1, 10.0 ); // Pit cell
  block.setValue( 0, 1, 12.0 ); // North neighbor (smallest elevation rise = +2.0)

  // With down = true -> no downhill neighbors exist -> returns -1
  QCOMPARE( QgsRasterAnalysisUtils::steepestGradientDirection( &block, 1, 1, 10.0, 10.0, true, false ), -1 );

  // With down = false -> finds neighbor with maximum gradient (smallest uphill rise = North, Dir 0)
  QCOMPARE( QgsRasterAnalysisUtils::steepestGradientDirection( &block, 1, 1, 10.0, 10.0, false, false ), 0 );
}

QGSTEST_MAIN( TestQgsRasterAnalysisUtils )
#include "testqgsrasteranalysisutils.moc"
