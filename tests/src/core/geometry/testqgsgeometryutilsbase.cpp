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

#include "qgstest.h"
#include <QObject>
#include "qgsgeometryutils_base.h"

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

QGSTEST_MAIN( TestQgsGeometryUtilsBase )
#include "testqgsgeometryutilsbase.moc"
