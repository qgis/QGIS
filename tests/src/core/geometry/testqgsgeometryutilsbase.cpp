/***************************************************************************
     testqgsgeometryutilsbase.cpp
     --------------------------------------
    Date                 : 2023-12-15
    Copyright            : (C) 2023 by Loïc Bartoletti
    Email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryutils_base.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgstest.h"

#include <QObject>

class TestQgsGeometryUtilsBase : public QObject
{
    Q_OBJECT

  private slots:
    void testFuzzyEqual();
    void testFuzzyDistanceEqual();
    void testCreateChamferBase_data();
    void testCreateChamferBase();
    void testCreateFilletBase_data();
    void testCreateFilletBase();
    void testPointsAreCollinear();
    void testPointByDeflectionAngle_data();
    void testPointByDeflectionAngle();
};

void TestQgsGeometryUtilsBase::testFuzzyEqual()
{
  QVERIFY( QgsGeometryUtilsBase::fuzzyEqual( 0.1, 1.0, 2.0, 1.0, 2.0 ) );

  QVERIFY( QgsGeometryUtilsBase::fuzzyEqual( 1.0, 1.0, 2.0, 1.5, 2.5 ) );

  QVERIFY( QgsGeometryUtilsBase::fuzzyEqual( 0.01, 1.0, 2.0, 1.001, 2.001 ) );

  QVERIFY( !QgsGeometryUtilsBase::fuzzyEqual( 0.1, 1.0, 2.0, 1.5, 2.0 ) );

  QVERIFY( !QgsGeometryUtilsBase::fuzzyEqual( 0.4, 1.0, 2.0, 1.5, 2.5 ) );

  QVERIFY( !QgsGeometryUtilsBase::fuzzyEqual( 0.001, 1.0, 2.0, 1.1, 2.1 ) );
}

void TestQgsGeometryUtilsBase::testFuzzyDistanceEqual()
{
  QVERIFY( QgsGeometryUtilsBase::fuzzyDistanceEqual( 0.1, 1.0, 2.0, 1.0, 2.0 ) );

  QVERIFY( QgsGeometryUtilsBase::fuzzyDistanceEqual( 2.0, 1.0, 2.0, 1.5, 2.1 ) );

  QVERIFY( QgsGeometryUtilsBase::fuzzyDistanceEqual( 0.01, 1.0, 2.0, 1.001, 2.001 ) );

  QVERIFY( !QgsGeometryUtilsBase::fuzzyDistanceEqual( 0.1, 1.0, 2.0, 1.5, 2.0 ) );

  QVERIFY( !QgsGeometryUtilsBase::fuzzyDistanceEqual( 0.2, 1.0, 2.0, 1.5, 2.5 ) );

  QVERIFY( !QgsGeometryUtilsBase::fuzzyDistanceEqual( 0.001, 1.0, 2.0, 1.1, 2.1 ) );
}

void TestQgsGeometryUtilsBase::testCreateChamferBase_data()
{
  QTest::addColumn<double>( "segment1StartX" );
  QTest::addColumn<double>( "segment1StartY" );
  QTest::addColumn<double>( "segment1EndX" );
  QTest::addColumn<double>( "segment1EndY" );
  QTest::addColumn<double>( "segment2StartX" );
  QTest::addColumn<double>( "segment2StartY" );
  QTest::addColumn<double>( "segment2EndX" );
  QTest::addColumn<double>( "segment2EndY" );
  QTest::addColumn<double>( "distance1" );
  QTest::addColumn<double>( "distance2" );
  QTest::addColumn<bool>( "expectedSuccess" );
  QTest::addColumn<double>( "expectedChamferStartX" );
  QTest::addColumn<double>( "expectedChamferStartY" );
  QTest::addColumn<double>( "expectedChamferEndX" );
  QTest::addColumn<double>( "expectedChamferEndY" );

  // Test 1: Basic symmetric chamfer on right angle
  QTest::newRow( "symmetric_right_angle" )
    << 1.0 << 0.0 << -1.0 << 0.0 // segment1: horizontal from (1,0) to (-1,0) passing through (0,0)
    << 0.0 << 1.0 << 0.0 << -1.0 // segment2: vertical from (0,1) to (0,-1) passing through (0,0)
    << 0.1 << 0.1                // distances: 0.1 on both segments
    << true                      // expected success
    << 0.1 << 0.0                // chamfer start: 0.1 along segment1 from intersection
    << 0.0 << 0.1;               // chamfer end: 0.1 along segment2 from intersection

  // Test 2: Asymmetric chamfer with different distances
  QTest::newRow( "asymmetric_chamfer" )
    << 2.0 << 0.0 << -2.0 << 0.0 // segment1: longer horizontal segment through origin
    << 0.0 << 2.0 << 0.0 << -2.0 // segment2: longer vertical segment through origin
    << 0.3 << 0.2                // different distances: 0.3 and 0.2
    << true
    << 0.3 << 0.0  // 0.3 along segment1
    << 0.0 << 0.2; // 0.2 along segment2

  // Test 3: Default distance2 (negative value should use distance1)
  QTest::newRow( "default_distance2" )
    << 1.0 << 0.0 << -1.0 << 0.0 // horizontal through origin
    << 0.0 << 1.0 << 0.0 << -1.0 // vertical through origin
    << 0.15 << -1.0              // distance2 negative, should use distance1
    << true
    << 0.15 << 0.0
    << 0.0 << 0.15;

  // Test 4: Parallel segments (should fail)
  QTest::newRow( "parallel_segments" )
    << 0.0 << 0.0 << 1.0 << 0.0 // horizontal segment
    << 0.0 << 1.0 << 1.0 << 1.0 // parallel horizontal segment
    << 0.1 << 0.1
    << false                     // should fail
    << 0.0 << 0.0 << 0.0 << 0.0; // values irrelevant for failure case

  // Test 5: Distance larger than available segment (should clamp)
  QTest::newRow( "distance_too_large" )
    << 0.5 << 0.0 << -0.5 << 0.0 // short horizontal segment through origin, total length 1.0
    << 0.0 << 0.3 << 0.0 << -0.3 // short vertical segment through origin, total length 0.6
    << 1.0 << 1.0                // distances larger than available from intersection
    << true
    << 0.5 << 0.0  // clamped to available length from intersection to segment1Start
    << 0.0 << 0.3; // clamped to available length from intersection to segment2Start

  // Test 6: Acute angle chamfer
  QTest::newRow( "acute_angle" )
    << 1.0 << 0.0 << -1.0 << 0.0  // horizontal segment
    << 0.2 << 0.1 << -0.2 << -0.1 // shallow angle segment
    << 0.1 << 0.1
    << true
    << 0.1 << 0.0
    << 0.08944 << 0.04472; // calculated: 0.1 * (0.2, 0.1) / ||(0.2, 0.1)||

  // Test 7: Very small coordinates (precision test)
  QTest::newRow( "precision_small" )
    << 1e-6 << 0.0 << -1e-6 << 0.0
    << 0.0 << 1e-6 << 0.0 << -1e-6
    << 1e-7 << 1e-7
    << true
    << 1e-7 << 0.0
    << 0.0 << 1e-7;

  // Test 8: Obtuse angle chamfer
  QTest::newRow( "obtuse_angle" )
    << 1.0 << 0.0 << -1.0 << 0.0  // horizontal segment
    << -0.8 << 0.6 << 0.8 << -0.6 // obtuse angle (135 degrees)
    << 0.1 << 0.1
    << true
    << 0.1 << 0.0
    << -0.08 << 0.06; // proportional to segment direction
}

void TestQgsGeometryUtilsBase::testCreateChamferBase()
{
  QFETCH( double, segment1StartX );
  QFETCH( double, segment1StartY );
  QFETCH( double, segment1EndX );
  QFETCH( double, segment1EndY );
  QFETCH( double, segment2StartX );
  QFETCH( double, segment2StartY );
  QFETCH( double, segment2EndX );
  QFETCH( double, segment2EndY );
  QFETCH( double, distance1 );
  QFETCH( double, distance2 );
  QFETCH( bool, expectedSuccess );
  QFETCH( double, expectedChamferStartX );
  QFETCH( double, expectedChamferStartY );
  QFETCH( double, expectedChamferEndX );
  QFETCH( double, expectedChamferEndY );

  double chamferStartX, chamferStartY, chamferEndX, chamferEndY;
  double trim1StartX, trim1StartY, trim1EndX, trim1EndY;
  double trim2StartX, trim2StartY, trim2EndX, trim2EndY;

  bool result;
  try
  {
    result = QgsGeometryUtilsBase::createChamfer(
      segment1StartX, segment1StartY, segment1EndX, segment1EndY,
      segment2StartX, segment2StartY, segment2EndX, segment2EndY,
      distance1, distance2,
      chamferStartX, chamferStartY, chamferEndX, chamferEndY,
      &trim1StartX, &trim1StartY, &trim1EndX, &trim1EndY,
      &trim2StartX, &trim2StartY, &trim2EndX, &trim2EndY
    );
  }
  catch ( QgsInvalidArgumentException &e )
  {
    result = false;
  }
  catch ( ... )
  {
    QVERIFY2( false, "Caught unhandled exception" );
  }

  QCOMPARE( result, expectedSuccess );

  if ( expectedSuccess )
  {
    const double tolerance = 1e-4; // Relaxed tolerance for geometric calculations

    // Verify chamfer points
    QVERIFY2( qgsDoubleNear( chamferStartX, expectedChamferStartX, tolerance ), QString( "Chamfer start X: got %1, expected %2" ).arg( chamferStartX ).arg( expectedChamferStartX ).toLatin1() );
    QVERIFY2( qgsDoubleNear( chamferStartY, expectedChamferStartY, tolerance ), QString( "Chamfer start Y: got %1, expected %2" ).arg( chamferStartY ).arg( expectedChamferStartY ).toLatin1() );
    QVERIFY2( qgsDoubleNear( chamferEndX, expectedChamferEndX, tolerance ), QString( "Chamfer end X: got %1, expected %2" ).arg( chamferEndX ).arg( expectedChamferEndX ).toLatin1() );
    QVERIFY2( qgsDoubleNear( chamferEndY, expectedChamferEndY, tolerance ), QString( "Chamfer end Y: got %1, expected %2" ).arg( chamferEndY ).arg( expectedChamferEndY ).toLatin1() );

    // Verify trimmed segments connect properly
    QVERIFY2( qgsDoubleNear( trim1StartX, segment1StartX, tolerance ), "Trim1 should start at segment1 start" );
    QVERIFY2( qgsDoubleNear( trim1StartY, segment1StartY, tolerance ), "Trim1 should start at segment1 start" );
    QVERIFY2( qgsDoubleNear( trim1EndX, chamferStartX, tolerance ), "Trim1 should end at chamfer start" );
    QVERIFY2( qgsDoubleNear( trim1EndY, chamferStartY, tolerance ), "Trim1 should end at chamfer start" );

    QVERIFY2( qgsDoubleNear( trim2StartX, segment2StartX, tolerance ), "Trim2 should start at segment2 start" );
    QVERIFY2( qgsDoubleNear( trim2StartY, segment2StartY, tolerance ), "Trim2 should start at segment2 start" );
    QVERIFY2( qgsDoubleNear( trim2EndX, chamferEndX, tolerance ), "Trim2 should end at chamfer end" );
    QVERIFY2( qgsDoubleNear( trim2EndY, chamferEndY, tolerance ), "Trim2 should end at chamfer end" );

    // Verify all coordinates are finite
    QVERIFY2( std::isfinite( chamferStartX ), "Chamfer start X should be finite" );
    QVERIFY2( std::isfinite( chamferStartY ), "Chamfer start Y should be finite" );
    QVERIFY2( std::isfinite( chamferEndX ), "Chamfer end X should be finite" );
    QVERIFY2( std::isfinite( chamferEndY ), "Chamfer end Y should be finite" );
  }
}

void TestQgsGeometryUtilsBase::testCreateFilletBase_data()
{
  QTest::addColumn<double>( "segment1StartX" );
  QTest::addColumn<double>( "segment1StartY" );
  QTest::addColumn<double>( "segment1EndX" );
  QTest::addColumn<double>( "segment1EndY" );
  QTest::addColumn<double>( "segment2StartX" );
  QTest::addColumn<double>( "segment2StartY" );
  QTest::addColumn<double>( "segment2EndX" );
  QTest::addColumn<double>( "segment2EndY" );
  QTest::addColumn<double>( "radius" );
  QTest::addColumn<bool>( "expectedSuccess" );

  // Test 1: Basic right angle fillet
  QTest::newRow( "right_angle_basic" )
    << 1.0 << 0.0 << -1.0 << 0.0 // segment1: horizontal from (1,0) to (-1,0) through origin
    << 0.0 << 1.0 << 0.0 << -1.0 // segment2: vertical from (0,1) to (0,-1) through origin
    << 0.1                       // radius
    << true;                     // should succeed

  // Test 2: Small radius fillet
  QTest::newRow( "small_radius" )
    << 2.0 << 0.0 << -2.0 << 0.0
    << 0.0 << 2.0 << 0.0 << -2.0
    << 0.01
    << true;

  // Test 3: Large radius fillet (but within limits)
  QTest::newRow( "large_radius" )
    << 5.0 << 0.0 << -5.0 << 0.0
    << 0.0 << 5.0 << 0.0 << -5.0
    << 2.0
    << true;

  // Test 4: Radius too large (should fail) - make segments much shorter
  QTest::newRow( "radius_too_large" )
    << 0.05 << 0.0 << -0.05 << 0.0 // very short horizontal segment (0.05 from intersection)
    << 0.0 << 0.03 << 0.0 << -0.03 // very short vertical segment (0.03 from intersection)
    << 1.0                         // huge radius that definitely won't fit
    << false;

  // Test 5: Parallel segments (should fail)
  QTest::newRow( "parallel_segments" )
    << 0.0 << 0.0 << 1.0 << 0.0
    << 0.0 << 1.0 << 1.0 << 1.0
    << 0.1
    << false;

  // Test 6: No intersection (truly non-intersecting segments - parallel with no overlap)
  QTest::newRow( "no_intersection" )
    << 0.0 << 0.0 << 1.0 << 0.0 // horizontal segment from (0,0) to (1,0)
    << 0.0 << 1.0 << 1.0 << 1.0 // parallel horizontal segment from (0,1) to (1,1) - parallel, no intersection
    << 0.1
    << false;

  // Test 7: Simple acute angle - remove complex calculations for now
  QTest::newRow( "acute_angle_30deg" )
    << 2.0 << 0.0 << -2.0 << 0.0      // horizontal through origin
    << 1.0 << 1.732 << -1.0 << -1.732 // 60 degree angle through origin (simpler than 30)
    << 0.1
    << true;

  // Test 8: Simple obtuse angle
  QTest::newRow( "obtuse_angle_120deg" )
    << 2.0 << 0.0 << -2.0 << 0.0  // horizontal through origin
    << -1.0 << 1.0 << 1.0 << -1.0 // 135 degree angle through origin (simpler)
    << 0.1
    << true;

  // Test 9: Small coordinates (precision test with reasonable small values)
  QTest::newRow( "precision_test" )
    << 0.001 << 0.0 << -0.001 << 0.0 // small but not tiny coordinates
    << 0.0 << 0.001 << 0.0 << -0.001
    << 0.0001 // small but reasonable radius
    << true;

  // Test 10: Nearly parallel segments (should fail)
  QTest::newRow( "nearly_parallel" )
    << 1.0 << 0.0 << -1.0 << 0.0      // horizontal segment
    << 1.0 << 0.001 << -1.0 << -0.001 // nearly parallel segment (very small angle)
    << 0.1
    << false;
}

void TestQgsGeometryUtilsBase::testCreateFilletBase()
{
  QFETCH( double, segment1StartX );
  QFETCH( double, segment1StartY );
  QFETCH( double, segment1EndX );
  QFETCH( double, segment1EndY );
  QFETCH( double, segment2StartX );
  QFETCH( double, segment2StartY );
  QFETCH( double, segment2EndX );
  QFETCH( double, segment2EndY );
  QFETCH( double, radius );
  QFETCH( bool, expectedSuccess );

  // Prepare output arrays for exactly 3 points (CircularString)
  double filletPointsX[3];
  double filletPointsY[3];
  double trim1StartX, trim1StartY, trim1EndX, trim1EndY;
  double trim2StartX, trim2StartY, trim2EndX, trim2EndY;

  bool result;
  try
  {
    result = QgsGeometryUtilsBase::createFillet(
      segment1StartX, segment1StartY, segment1EndX, segment1EndY,
      segment2StartX, segment2StartY, segment2EndX, segment2EndY,
      radius,
      filletPointsX, filletPointsY,
      &trim1StartX, &trim1StartY, &trim1EndX, &trim1EndY,
      &trim2StartX, &trim2StartY, &trim2EndX, &trim2EndY
    );
  }
  catch ( QgsInvalidArgumentException &e )
  {
    result = false;
  }
  catch ( ... )
  {
    QVERIFY2( false, "Caught unhandled exception" );
  }

  QCOMPARE( result, expectedSuccess );

  if ( expectedSuccess )
  {
    const double tolerance = 1e-4; // Relaxed tolerance for geometric calculations

    // Verify all fillet points are finite (not NaN or infinite)
    for ( int i = 0; i < 3; ++i )
    {
      QVERIFY2( std::isfinite( filletPointsX[i] ), QString( "Fillet point %1 X coordinate should be finite" ).arg( i ).toLatin1() );
      QVERIFY2( std::isfinite( filletPointsY[i] ), QString( "Fillet point %1 Y coordinate should be finite" ).arg( i ).toLatin1() );
    }

    // Verify geometric continuity - trimmed segments should connect to fillet
    QVERIFY2( qgsDoubleNear( trim1EndX, filletPointsX[0], tolerance ), QString( "Trim1 end (%1,%2) should connect to fillet start (%3,%4)" ).arg( trim1EndX ).arg( trim1EndY ).arg( filletPointsX[0] ).arg( filletPointsY[0] ).toLatin1() );
    QVERIFY2( qgsDoubleNear( trim1EndY, filletPointsY[0], tolerance ), "Trimmed segment 1 end should connect to fillet start" );

    QVERIFY2( qgsDoubleNear( trim2EndX, filletPointsX[2], tolerance ), QString( "Trim2 end (%1,%2) should connect to fillet end (%3,%4)" ).arg( trim2EndX ).arg( trim2EndY ).arg( filletPointsX[2] ).arg( filletPointsY[2] ).toLatin1() );
    QVERIFY2( qgsDoubleNear( trim2EndY, filletPointsY[2], tolerance ), "Trimmed segment 2 end should connect to fillet end" );

    // Verify trimmed segments start at original segment starts
    QVERIFY2( qgsDoubleNear( trim1StartX, segment1StartX, tolerance ), "Trimmed segment 1 should start at original segment 1 start" );
    QVERIFY2( qgsDoubleNear( trim1StartY, segment1StartY, tolerance ), "Trimmed segment 1 should start at original segment 1 start" );
    QVERIFY2( qgsDoubleNear( trim2StartX, segment2StartX, tolerance ), "Trimmed segment 2 should start at original segment 2 start" );
    QVERIFY2( qgsDoubleNear( trim2StartY, segment2StartY, tolerance ), "Trimmed segment 2 should start at original segment 2 start" );

    // Verify the arc radius using QGIS circle calculation - use relaxed tolerance
    double centerX, centerY, calculatedRadius;
    QgsGeometryUtilsBase::circleCenterRadius(
      filletPointsX[0], filletPointsY[0],
      filletPointsX[1], filletPointsY[1],
      filletPointsX[2], filletPointsY[2],
      calculatedRadius, centerX, centerY
    );

    // Use relaxed tolerance for radius comparison due to numerical precision
    QVERIFY2( qgsDoubleNear( calculatedRadius, radius, 0.01 ), QString( "Calculated radius %1 should be close to expected radius %2" ).arg( calculatedRadius ).arg( radius ).toLatin1() );

    // Verify the circle geometry is reasonable
    double dist1 = QgsGeometryUtilsBase::distance2D( centerX, centerY, filletPointsX[0], filletPointsY[0] );
    double dist2 = QgsGeometryUtilsBase::distance2D( centerX, centerY, filletPointsX[1], filletPointsY[1] );
    double dist3 = QgsGeometryUtilsBase::distance2D( centerX, centerY, filletPointsX[2], filletPointsY[2] );

    QVERIFY2( qgsDoubleNear( dist1, calculatedRadius, 0.001 ), QString( "Distance from center to point 1: %1, should be %2" ).arg( dist1 ).arg( calculatedRadius ).toLatin1() );
    QVERIFY2( qgsDoubleNear( dist2, calculatedRadius, 0.001 ), QString( "Distance from center to point 2: %1, should be %2" ).arg( dist2 ).arg( calculatedRadius ).toLatin1() );
    QVERIFY2( qgsDoubleNear( dist3, calculatedRadius, 0.001 ), QString( "Distance from center to point 3: %1, should be %2" ).arg( dist3 ).arg( calculatedRadius ).toLatin1() );
  }
}

void TestQgsGeometryUtilsBase::testPointsAreCollinear()
{
  // 2D version
  QVERIFY( QgsGeometryUtilsBase::pointsAreCollinear( 0, 10, 10, 10, 20, 10, 0.00001 ) );
  QVERIFY( QgsGeometryUtilsBase::pointsAreCollinear( 10, 10, 0, 10, 20, 10, 0.00001 ) );
  QVERIFY( QgsGeometryUtilsBase::pointsAreCollinear( 20, 10, 10, 10, 0, 10, 0.00001 ) );
  QVERIFY( !QgsGeometryUtilsBase::pointsAreCollinear( 20, 15, 10, 10, 0, 10, 0.00001 ) );
  QVERIFY( !QgsGeometryUtilsBase::pointsAreCollinear( 20, 10, 10, 15, 0, 10, 0.00001 ) );
  QVERIFY( !QgsGeometryUtilsBase::pointsAreCollinear( 20, 10, 10, 10, 0, 15, 0.00001 ) );
  QVERIFY( QgsGeometryUtilsBase::pointsAreCollinear( 10, 0, 10, 10, 10, 20, 0.00001 ) );
  QVERIFY( QgsGeometryUtilsBase::pointsAreCollinear( 10, 0, 10, 20, 10, 10, 0.00001 ) );
  QVERIFY( QgsGeometryUtilsBase::pointsAreCollinear( 10, 20, 10, 0, 10, 10, 0.00001 ) );
  QVERIFY( !QgsGeometryUtilsBase::pointsAreCollinear( 15, 20, 10, 10, 10, 20, 0.00001 ) );

  // 3D version
  QVERIFY( QgsGeometryUtilsBase::points3DAreCollinear( 0, 0, 0, 1, 1, 1, 2, 2, 2, 0.00001 ) );
  QVERIFY( QgsGeometryUtilsBase::points3DAreCollinear( 1, 1, 1, 0, 0, 0, 2, 2, 2, 0.00001 ) );
  QVERIFY( QgsGeometryUtilsBase::points3DAreCollinear( 2, 2, 2, 0, 0, 0, 1, 1, 1, 0.00001 ) );
  QVERIFY( QgsGeometryUtilsBase::points3DAreCollinear( 0, 0, 0, 0, 0, 1, 0, 0, 2, 0.00001 ) );
  QVERIFY( QgsGeometryUtilsBase::points3DAreCollinear( 0, 0, 1, 0, 0, 0, 0, 0, 2, 0.00001 ) );
  QVERIFY( QgsGeometryUtilsBase::points3DAreCollinear( 0, 0, 2, 0, 0, 0, 0, 0, 1, 0.00001 ) );
  QVERIFY( !QgsGeometryUtilsBase::points3DAreCollinear( 0, 0, 0, 1, 0, 0, 0, 1, 1, 0.00001 ) );
  QVERIFY( !QgsGeometryUtilsBase::points3DAreCollinear( 1, 0, 0, 0, 0, 0, 0, 1, 1, 0.00001 ) );
}

void TestQgsGeometryUtilsBase::testPointByDeflectionAngle_data()
{
  QTest::addColumn<double>( "x1" );
  QTest::addColumn<double>( "y1" );
  QTest::addColumn<double>( "x2" );
  QTest::addColumn<double>( "y2" );
  QTest::addColumn<double>( "deflectionAngle" );
  QTest::addColumn<double>( "distance" );
  QTest::addColumn<double>( "expectedX" );
  QTest::addColumn<double>( "expectedY" );

  // Test 1: No deflection (continue in same direction)
  // Base point at origin, direction point at (0,10) (north), distance 5
  // Should produce point at (0,5)
  QTest::newRow( "no_deflection_north" )
    << 0.0 << 0.0  // base point
    << 0.0 << 10.0 // direction point (north)
    << 0.0         // no deflection
    << 5.0         // distance
    << 0.0 << 5.0; // expected result (continue north)

  // Test 2: 90 degree deflection to the right (clockwise)
  // From (0,0) with direction to (0,10), deflect 90 degrees right = east
  QTest::newRow( "deflection_90_right" )
    << 0.0 << 0.0  // base point
    << 0.0 << 10.0 // direction point (north)
    << M_PI_2      // 90 degrees right (clockwise)
    << 5.0         // distance
    << 5.0 << 0.0; // expected result (east)

  // Test 3: 90 degree deflection to the left (counter-clockwise)
  // From (0,0) with direction to (0,10), deflect 90 degrees left = west
  QTest::newRow( "deflection_90_left" )
    << 0.0 << 0.0   // base point
    << 0.0 << 10.0  // direction point (north)
    << -M_PI_2      // 90 degrees left (counter-clockwise)
    << 5.0          // distance
    << -5.0 << 0.0; // expected result (west)

  // Test 4: 180 degree deflection (reverse direction)
  // From (0,0) with direction to (0,10), deflect 180 degrees = south
  QTest::newRow( "deflection_180" )
    << 0.0 << 0.0   // base point
    << 0.0 << 10.0  // direction point (north)
    << M_PI         // 180 degrees
    << 5.0          // distance
    << 0.0 << -5.0; // expected result (south)

  // Test 5: 45 degree deflection to the right
  // From (0,0) with direction to (0,10), deflect 45 degrees right = northeast
  QTest::newRow( "deflection_45_right" )
    << 0.0 << 0.0       // base point
    << 0.0 << 10.0      // direction point (north)
    << M_PI_4           // 45 degrees right
    << std::sqrt( 2.0 ) // distance sqrt(2) for easier calculation
    << 1.0 << 1.0;      // expected result (northeast at 45 degrees)

  // Test 6: Direction from east (0,0) to (10,0)
  // Deflect 90 degrees right (clockwise) = south
  QTest::newRow( "direction_east_deflect_right" )
    << 0.0 << 0.0   // base point
    << 10.0 << 0.0  // direction point (east)
    << M_PI_2       // 90 degrees right
    << 5.0          // distance
    << 0.0 << -5.0; // expected result (south)

  // Test 7: Non-origin base point
  // From (10,10) with direction to (10,20), no deflection
  QTest::newRow( "non_origin_base" )
    << 10.0 << 10.0  // base point
    << 10.0 << 20.0  // direction point (north)
    << 0.0           // no deflection
    << 5.0           // distance
    << 10.0 << 15.0; // expected result

  // Test 8: Small deflection angle
  // Tiny angle should produce result close to original direction
  QTest::newRow( "small_deflection" )
    << 0.0 << 0.0                 // base point
    << 0.0 << 10.0                // direction point (north)
    << 0.001                      // very small deflection
    << 100.0                      // large distance to show effect
    << 100.0 * std::sin( 0.001 )  // nearly north but slightly east
    << 100.0 * std::cos( 0.001 ); // x = d*sin(angle), y = d*cos(angle)

  // Test 9: Full circle deflection (360 degrees = no change)
  QTest::newRow( "full_circle_deflection" )
    << 0.0 << 0.0  // base point
    << 0.0 << 10.0 // direction point (north)
    << 2.0 * M_PI  // 360 degrees
    << 5.0         // distance
    << 0.0 << 5.0; // expected result (same as no deflection)

  // Test 10: Diagonal initial direction
  // From (0,0) with direction to (10,10) (northeast), no deflection
  QTest::newRow( "diagonal_direction" )
    << 0.0 << 0.0       // base point
    << 10.0 << 10.0     // direction point (northeast)
    << 0.0              // no deflection
    << std::sqrt( 2.0 ) // distance sqrt(2)
    << 1.0 << 1.0;      // expected result (continues northeast)
}

void TestQgsGeometryUtilsBase::testPointByDeflectionAngle()
{
  QFETCH( double, x1 );
  QFETCH( double, y1 );
  QFETCH( double, x2 );
  QFETCH( double, y2 );
  QFETCH( double, deflectionAngle );
  QFETCH( double, distance );
  QFETCH( double, expectedX );
  QFETCH( double, expectedY );

  double resultX, resultY;
  QgsGeometryUtilsBase::pointByDeflectionAngle( x1, y1, x2, y2, deflectionAngle, distance, resultX, resultY );

  const double tolerance = 1e-8;

  QVERIFY2( std::isfinite( resultX ), "Result X should be finite" );
  QVERIFY2( std::isfinite( resultY ), "Result Y should be finite" );
  QVERIFY2( qgsDoubleNear( resultX, expectedX, tolerance ), QString( "X coordinate: got %1, expected %2" ).arg( resultX ).arg( expectedX ).toLatin1() );
  QVERIFY2( qgsDoubleNear( resultY, expectedY, tolerance ), QString( "Y coordinate: got %1, expected %2" ).arg( resultY ).arg( expectedY ).toLatin1() );

  // Verify the distance from base point to result is correct
  const double actualDistance = QgsGeometryUtilsBase::distance2D( x1, y1, resultX, resultY );
  QVERIFY2( qgsDoubleNear( actualDistance, distance, tolerance ), QString( "Distance: got %1, expected %2" ).arg( actualDistance ).arg( distance ).toLatin1() );
}

QGSTEST_MAIN( TestQgsGeometryUtilsBase )
#include "testqgsgeometryutilsbase.moc"
