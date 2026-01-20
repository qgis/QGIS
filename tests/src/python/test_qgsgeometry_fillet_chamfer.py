"""Qgis Unit tests for QgsGeometry.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Loïc Bartoletti"
__date__ = "2025-07-09"
__copyright__ = "Copyright 2025, The QGIS Project"

from qgis.core import (
    Qgis,
    QgsInvalidArgumentException,
    QgsGeometryUtils,
    QgsCircularString,
    QgsCompoundCurve,
    QgsGeometry,
    QgsLineString,
    QgsMultiLineString,
    QgsPoint,
    QgsVertexId,
    QgsWkbTypes,
)
import unittest

from qgis.testing import start_app, QgisTestCase

from utilities import compareWkt, unitTestDataPath, writeShape

# Convenience instances in case you may need them not used in this test

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsGeometry(QgisTestCase):

    # CHAMFER TESTS - SEGMENT-BASED OVERLOAD

    def test_chamfer_segments_basic_right_angle_quadrant1(self):
        """Test basic chamfer creation between perpendicular segments - Quadrant 1 (E→N)"""
        # Create LineString with touching segments: East to North
        original_geom = QgsGeometry.fromWkt("LineString (1 0, 0 0, 0 1)")

        # Extract segment points
        segment1_start = QgsPoint(1.0, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, 1.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().chamfer(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 0.1
        )

        expected_wkt = "LineString (1 0, 0.1 0, 0 0.1, 0 1)"
        self.assertEqual(result.asWkt(1), expected_wkt)

    def test_chamfer_segments_basic_right_angle_quadrant2(self):
        """Test basic chamfer creation between perpendicular segments - Quadrant 2 (N→W)"""
        # Create LineString with touching segments: North to West
        original_geom = QgsGeometry.fromWkt("LineString (0 1, 0 0, -1 0)")

        # Extract segment points
        segment1_start = QgsPoint(0.0, 1.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(-1.0, 0.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().chamfer(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 0.1
        )

        expected_wkt = "LineString (0 1, 0 0.1, -0.1 0, -1 0)"
        self.assertEqual(result.asWkt(1), expected_wkt)

    def test_chamfer_segments_basic_right_angle_quadrant3(self):
        """Test basic chamfer creation between perpendicular segments - Quadrant 3 (W→S)"""
        # Create LineString with touching segments: West to South
        original_geom = QgsGeometry.fromWkt("LineString (-1 0, 0 0, 0 -1)")

        # Extract segment points
        segment1_start = QgsPoint(-1.0, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, -1.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().chamfer(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 0.1
        )

        expected_wkt = "LineString (-1 0, -0.1 0, 0 -0.1, 0 -1)"
        self.assertEqual(result.asWkt(1), expected_wkt)

    def test_chamfer_segments_basic_right_angle_quadrant4(self):
        """Test basic chamfer creation between perpendicular segments - Quadrant 4 (S→E)"""
        # Create LineString with touching segments: South to East
        original_geom = QgsGeometry.fromWkt("LineString (0 -1, 0 0, 1 0)")

        # Extract segment points
        segment1_start = QgsPoint(0.0, -1.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(1.0, 0.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().chamfer(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 0.1
        )

        expected_wkt = "LineString (0 -1, 0 -0.1, 0.1 0, 1 0)"
        self.assertEqual(result.asWkt(1), expected_wkt)

    def test_chamfer_segments_basic_right_angle_non_touching(self):
        """Test basic chamfer creation between perpendicular segments - non touching"""
        original_geom = QgsGeometry.fromWkt("MultiLineString ((0 0, 10 0), (5 1, 5 5))")

        # Extract segment points
        segment1_start = QgsPoint(0.0, 0.0)
        segment1_end = QgsPoint(10.0, 0.0)
        segment2_start = QgsPoint(5.0, 1.0)
        segment2_end = QgsPoint(5.0, 5.0)

        result = QgsGeometry().chamfer(
            segment1_start, segment1_end, segment2_start, segment2_end, 1, 1
        )

        expected_wkt = "LineString (0 0, 4 0, 5 1, 5 1)"
        self.assertEqual(result.asWkt(1), expected_wkt)

    def test_chamfer_segments_basic_right_angle_non_touching2(self):
        """Test basic chamfer creation between perpendicular segments - non touching2"""
        original_geom = QgsGeometry.fromWkt("MultiLineString ((0 0, 2 0), (5 1, 5 5))")

        # Extract segment points
        segment1_start = QgsPoint(0.0, 0.0)
        segment1_end = QgsPoint(2.0, 0.0)
        segment2_start = QgsPoint(5.0, 1.0)
        segment2_end = QgsPoint(5.0, 5.0)

        result = QgsGeometry().chamfer(
            segment1_start, segment1_end, segment2_start, segment2_end, 1, 1
        )

        expected_wkt = "LineString (0 0, 4 0, 5 1, 5 1)"
        self.assertEqual(result.asWkt(1), expected_wkt)

    def test_chamfer_segments_asymmetric_distances(self):
        """Test chamfer with different distances on each segment"""
        # Create LineString with touching segments
        original_geom = QgsGeometry.fromWkt("LineString (2 0, 0 0, 0 3)")

        # Extract segment points
        segment1_start = QgsPoint(2.0, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, 3.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().chamfer(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.5, 0.3
        )

        expected_wkt = "LineString (2 0, 0.5 0, 0 0.3, 0 3)"
        self.assertEqual(result.asWkt(1), expected_wkt)

    def test_chamfer_segments_default_distance2(self):
        """Test chamfer with default distance2 parameter (should use distance1)"""
        # Create LineString with touching segments
        original_geom = QgsGeometry.fromWkt("LineString (1 0, 0 0, 0 1)")

        # Extract segment points
        segment1_start = QgsPoint(1.0, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, 1.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().chamfer(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.15
        )

        expected_wkt = "LineString (1 0, 0.15 0, 0 0.15, 0 1)"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_chamfer_segments_with_z_coordinates(self):
        """Test chamfer creation with Z coordinate interpolation"""
        # Create LineStringZ with touching segments
        original_geom = QgsGeometry.fromWkt("LineStringZ (1 0 10, 0 0 0, 0 1 5)")

        # Extract segment points
        segment1_start = QgsPoint(1.0, 0.0, 10.0)
        segment1_end = QgsPoint(0.0, 0.0, 0.0)
        segment2_start = QgsPoint(0.0, 1.0, 5.0)
        segment2_end = QgsPoint(0.0, 0.0, 0.0)

        result = QgsGeometry().chamfer(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.2, 0.2
        )

        expected_wkt = "LineString Z (1 0 10, 0.2 0 2, 0 0.2 1, 0 1 5)"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_chamfer_segments_with_m_coordinates(self):
        """Test chamfer creation with M coordinate interpolation"""
        # Create LineStringM with touching segments
        original_geom = QgsGeometry.fromWkt("LineStringM (1 0 100, 0 0 0, 0 1 50)")

        # Extract segment points
        segment1_start = QgsPoint(1.0, 0.0, m=100.0)
        segment1_end = QgsPoint(0.0, 0.0, m=0.0)
        segment2_start = QgsPoint(0.0, 1.0, m=50.0)
        segment2_end = QgsPoint(0.0, 0.0, m=0.0)

        result = QgsGeometry().chamfer(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 0.1
        )

        expected_wkt = "LineString M (1 0 100, 0.1 0 10, 0 0.1 5, 0 1 50)"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_chamfer_segments_parallel_failure(self):
        """Test that parallel segments return empty geometry"""
        # Create MultiLineString with parallel segments (not touching)
        original_geom = QgsGeometry.fromWkt("MultiLineString ((0 0, 1 0), (0 1, 1 1))")

        # Extract segment points
        segment1_start = QgsPoint(0.0, 0.0)
        segment1_end = QgsPoint(1.0, 0.0)
        segment2_start = QgsPoint(0.0, 1.0)
        segment2_end = QgsPoint(1.0, 1.0)

        try:
            QgsGeometry.chamfer(
                segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 0.1
            )
            self.fail("Should have failed!")
        except QgsInvalidArgumentException as e:
            self.assertEqual(e.__str__(), "Segments do not intersect.")

    def test_chamfer_segments_negative_distances(self):
        """Test that negative distances return empty geometry"""
        # Create LineString with touching segments
        original_geom = QgsGeometry.fromWkt("LineString (1 0, 0 0, 0 1)")

        # Extract segment points
        segment1_start = QgsPoint(1.0, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, 1.0)
        segment2_end = QgsPoint(0.0, 0.0)

        try:
            QgsGeometry.chamfer(
                segment1_start, segment1_end, segment2_start, segment2_end, -0.1, 0.1
            )
            self.fail("Should have failed!")
        except QgsInvalidArgumentException as e:
            self.assertEqual(e.__str__(), "Negative distances.")

    # CHAMFER TESTS - VERTEX-BASED OVERLOAD

    def test_chamfer_vertex_middle_vertex(self):
        """Test applying chamfer to middle vertex of a linestring"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 0, 1 1)")

        result = linestring.chamfer(1, 0.1, 0.1)

        expected_wkt = "LineString (0 0, 0.9 0, 1 0.1, 1 1)"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_chamfer_vertex_symmetric_default(self):
        """Test chamfer at vertex with symmetric distances using default parameter"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 0, 1 1)")

        result = linestring.chamfer(1, 0.15)

        expected_wkt = "LineString (0 0, 0.85 0, 1 0.15, 1 1)"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_chamfer_vertex_with_z_values(self):
        """Test chamfer at vertex with Z coordinate preservation"""
        linestring = QgsGeometry.fromWkt("LineStringZ (0 0 0, 1 0 5, 1 1 10)")

        result = linestring.chamfer(1, 0.2, 0.2)

        expected_wkt = "LineString Z (0 0 0, 0.8 0 4, 1 0.2 6, 1 1 10)"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_chamfer_vertex_invalid_index_first(self):
        """Test chamfer with invalid vertex index (first vertex)"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 0, 1 1)")

        result = linestring.chamfer(0, 0.1, 0.1)
        self.assertTrue(result.isEmpty())
        self.assertEqual(
            linestring.lastError(),
            "Vertex index out of range. 0 must be in (0, 2). Requested vertex: 0 was resolved as: [part: -1, ring: -1, vertex: 0]",
        )

    def test_chamfer_vertex_invalid_index_last(self):
        """Test chamfer with invalid vertex index (last vertex)"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 0, 1 1)")

        result = linestring.chamfer(2, 0.1, 0.1)

        self.assertTrue(result.isEmpty())
        self.assertEqual(
            linestring.lastError(),
            "Vertex index out of range. 2 must be in (0, 2). Requested vertex: 2 was resolved as: [part: -1, ring: -1, vertex: 2]",
        )

    def test_chamfer_vertex_acute_angle(self):
        """Test chamfer at vertex with acute angle"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 0, 0.5 0.5)")

        result = linestring.chamfer(1, 0.1, 0.1)

        expected_wkt = "LineString (0 0, 0.9 0, 0.93 0.07, 0.5 0.5)"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_chamfer_vertex_complex_linestring(self):
        """Test chamfer on complex linestring with multiple vertices"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 2 0, 4 0, 4 2, 4 4)")

        result = linestring.chamfer(2, 0.3)

        expected_wkt = "LineString (0 0, 2 0, 3.7 0, 4 0.3, 4 2, 4 4)"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_geometryutils_chamfer_vertex_complex_linestring(self):
        """Test chamfer on complex linestring with multiple vertices from QgsGeometryUtils."""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 2 0, 4 0, 4 2, 4 4)")

        result = QgsGeometryUtils.chamferVertex(linestring.get(), 2, 0.3, 0.3)

        expected_wkt = "LineString (0 0, 2 0, 3.7 0, 4 0.3, 4 2, 4 4)"
        self.assertEqual(result.asWkt(2), expected_wkt)

    # FILLET TESTS - SEGMENT-BASED OVERLOAD

    def test_fillet_segments_basic_right_angle_non_touching(self):
        """Test basic chamfer creation between perpendicular segments - non touching"""
        original_geom = QgsGeometry.fromWkt("MultiLineString ((0 0, 10 0), (5 1, 5 5))")

        # Extract segment points
        segment1_start = QgsPoint(0.0, 0.0)
        segment1_end = QgsPoint(10.0, 0.0)
        segment2_start = QgsPoint(5.0, 1.0)
        segment2_end = QgsPoint(5.0, 5.0)

        result = QgsGeometry().fillet(
            segment1_start, segment1_end, segment2_start, segment2_end, 1, 0
        )

        expected_wkt = (
            "CompoundCurve ((0 0, 4 0),CircularString (4 0, 4.7 0.3, 5 1),(5 1, 5 5))"
        )
        self.assertEqual(result.asWkt(1), expected_wkt)

    def test_fillet_segments_basic_right_angle_non_touching2(self):
        """Test basic chamfer creation between perpendicular segments - non touching2"""
        original_geom = QgsGeometry.fromWkt("MultiLineString ((0 0, 2 0), (5 1, 5 5))")

        # Extract segment points
        segment1_start = QgsPoint(0.0, 0.0)
        segment1_end = QgsPoint(2.0, 0.0)
        segment2_start = QgsPoint(5.0, 1.0)
        segment2_end = QgsPoint(5.0, 5.0)

        result = QgsGeometry().fillet(
            segment1_start, segment1_end, segment2_start, segment2_end, 1, 0
        )

        expected_wkt = (
            "CompoundCurve ((0 0, 4 0),CircularString (4 0, 4.7 0.3, 5 1),(5 1, 5 5))"
        )
        self.assertEqual(result.asWkt(1), expected_wkt)

    def test_fillet_segments_basic_right_angle_circular_quadrant1(self):
        """Test basic fillet creation between perpendicular segments (CircularString) - Quadrant 1 (E→N)"""
        # Create LineString with touching segments: East to North
        original_geom = QgsGeometry.fromWkt("LineString (1 0, 0 0, 0 1)")

        # Extract segment points
        segment1_start = QgsPoint(1.0, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, 1.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().fillet(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 0
        )

        expected_wkt = "CompoundCurve ((1 0, 0.1 0),CircularString (0.1 0, 0.03 0.03, 0 0.1),(0 0.1, 0 1))"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_fillet_segments_basic_right_angle_circular_quadrant2(self):
        """Test basic fillet creation between perpendicular segments (CircularString) - Quadrant 2 (N→W)"""
        # Create LineString with touching segments: North to West
        original_geom = QgsGeometry.fromWkt("LineString (0 1, 0 0, -1 0)")

        # Extract segment points
        segment1_start = QgsPoint(0.0, 1.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(-1.0, 0.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().fillet(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 0
        )

        self.assertEqual(result.wkbType(), QgsWkbTypes.CompoundCurve)
        wkt = result.asWkt(3)
        self.assertIn("CompoundCurve", wkt)
        self.assertIn("CircularString", wkt)

    def test_fillet_segments_basic_right_angle_circular_quadrant3(self):
        """Test basic fillet creation between perpendicular segments (CircularString) - Quadrant 3 (W→S)"""
        # Create LineString with touching segments: West to South
        original_geom = QgsGeometry.fromWkt("LineString (-1 0, 0 0, 0 -1)")

        # Extract segment points
        segment1_start = QgsPoint(-1.0, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, -1.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().fillet(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 0
        )

        self.assertEqual(result.wkbType(), QgsWkbTypes.CompoundCurve)
        wkt = result.asWkt(3)
        self.assertIn("CompoundCurve", wkt)
        self.assertIn("CircularString", wkt)

    def test_fillet_segments_basic_right_angle_circular_quadrant4(self):
        """Test basic fillet creation between perpendicular segments (CircularString) - Quadrant 4 (S→E)"""
        # Create LineString with touching segments: South to East
        original_geom = QgsGeometry.fromWkt("LineString (0 -1, 0 0, 1 0)")

        # Extract segment points
        segment1_start = QgsPoint(0.0, -1.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(1.0, 0.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().fillet(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 0
        )

        self.assertEqual(result.wkbType(), QgsWkbTypes.CompoundCurve)
        wkt = result.asWkt(3)
        self.assertIn("CompoundCurve", wkt)
        self.assertIn("CircularString", wkt)

    def test_fillet_segments_basic_right_angle_segmented_quadrant1(self):
        """Test basic fillet creation between perpendicular segments (segmented) - Quadrant 1 (E→N)"""
        # Create LineString with touching segments: East to North
        original_geom = QgsGeometry.fromWkt("LineString (1 0, 0 0, 0 1)")

        # Extract segment points
        segment1_start = QgsPoint(1.0, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, 1.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().fillet(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 8
        )

        self.assertEqual(result.wkbType(), QgsWkbTypes.LineString)
        points = result.asPolyline()

        start_point = points[0]
        self.assertAlmostEqual(start_point.x(), 1.0, places=1)
        self.assertAlmostEqual(start_point.y(), 0.0, places=1)

        end_point = points[-1]
        self.assertAlmostEqual(end_point.x(), 0.0, places=1)
        self.assertAlmostEqual(end_point.y(), 1.0, places=1)

        self.assertGreater(len(points), 10)

    def test_fillet_segments_basic_right_angle_segmented_quadrant2(self):
        """Test basic fillet creation between perpendicular segments (segmented) - Quadrant 2 (N→W)"""
        # Create LineString with touching segments: North to West
        original_geom = QgsGeometry.fromWkt("LineString (0 1, 0 0, -1 0)")

        # Extract segment points
        segment1_start = QgsPoint(0.0, 1.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(-1.0, 0.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().fillet(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 8
        )

        self.assertEqual(result.wkbType(), QgsWkbTypes.LineString)
        points = result.asPolyline()

        start_point = points[0]
        self.assertAlmostEqual(start_point.x(), 0.0, places=1)
        self.assertAlmostEqual(start_point.y(), 1.0, places=1)

        end_point = points[-1]
        self.assertAlmostEqual(end_point.x(), -1.0, places=1)
        self.assertAlmostEqual(end_point.y(), 0.0, places=1)

        self.assertGreater(len(points), 10)

    def test_fillet_segments_basic_right_angle_segmented_quadrant3(self):
        """Test basic fillet creation between perpendicular segments (segmented) - Quadrant 3 (W→S)"""
        # Create LineString with touching segments: West to South
        original_geom = QgsGeometry.fromWkt("LineString (-1 0, 0 0, 0 -1)")

        # Extract segment points
        segment1_start = QgsPoint(-1.0, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, -1.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().fillet(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 8
        )

        self.assertEqual(result.wkbType(), QgsWkbTypes.LineString)
        points = result.asPolyline()

        start_point = points[0]
        self.assertAlmostEqual(start_point.x(), -1.0, places=1)
        self.assertAlmostEqual(start_point.y(), 0.0, places=1)

        end_point = points[-1]
        self.assertAlmostEqual(end_point.x(), 0.0, places=1)
        self.assertAlmostEqual(end_point.y(), -1.0, places=1)

        self.assertGreater(len(points), 10)

    def test_fillet_segments_basic_right_angle_segmented_quadrant4(self):
        """Test basic fillet creation between perpendicular segments (segmented) - Quadrant 4 (S→E)"""
        # Create LineString with touching segments: South to East
        original_geom = QgsGeometry.fromWkt("LineString (0 -1, 0 0, 1 0)")

        # Extract segment points
        segment1_start = QgsPoint(0.0, -1.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(1.0, 0.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().fillet(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 8
        )

        self.assertEqual(result.wkbType(), QgsWkbTypes.LineString)
        points = result.asPolyline()

        start_point = points[0]
        self.assertAlmostEqual(start_point.x(), 0.0, places=1)
        self.assertAlmostEqual(start_point.y(), -1.0, places=1)

        end_point = points[-1]
        self.assertAlmostEqual(end_point.x(), 1.0, places=1)
        self.assertAlmostEqual(end_point.y(), 0.0, places=1)

        self.assertGreater(len(points), 10)

    def test_fillet_segments_with_z_coordinates(self):
        """Test fillet creation with Z coordinate interpolation"""
        # Create LineStringZ with touching segments
        original_geom = QgsGeometry.fromWkt("LineStringZ (1 0 10, 0 0 0, 0 1 8)")

        # Extract segment points
        segment1_start = QgsPoint(1.0, 0.0, 10.0)
        segment1_end = QgsPoint(0.0, 0.0, 0.0)
        segment2_start = QgsPoint(0.0, 1.0, 8.0)
        segment2_end = QgsPoint(0.0, 0.0, 0.0)

        result = QgsGeometry().fillet(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 0
        )

        expected_wkt = "CompoundCurve Z ((1 0 10, 0.1 0 1),CircularString Z (0.1 0 1, 0.03 0.03 0.9, 0 0.1 0.8),(0 0.1 0.8, 0 1 8))"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_fillet_segments_large_radius_failure(self):
        """Test that oversized radius returns empty geometry"""
        # Create LineString with very short segments
        original_geom = QgsGeometry.fromWkt("LineString (0.1 0, 0 0, 0 0.1)")

        # Extract segment points
        segment1_start = QgsPoint(0.1, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, 0.1)
        segment2_end = QgsPoint(0.0, 0.0)

        try:
            QgsGeometry.fillet(
                segment1_start, segment1_end, segment2_start, segment2_end, 1.0
            )
            self.fail("Should have failed!")
        except QgsInvalidArgumentException as e:
            self.assertEqual(e.__str__(), "Intersection 1 on segment but too far.")

    def test_fillet_segments_zero_radius_failure(self):
        """Test that zero radius returns empty geometry"""
        # Create LineString with touching segments
        original_geom = QgsGeometry.fromWkt("LineString (1 0, 0 0, 0 1)")

        # Extract segment points
        segment1_start = QgsPoint(1.0, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, 1.0)
        segment2_end = QgsPoint(0.0, 0.0)

        try:
            QgsGeometry.fillet(
                segment1_start, segment1_end, segment2_start, segment2_end, 0.0
            )
            self.fail("Should have failed!")
        except QgsInvalidArgumentException as e:
            self.assertEqual(e.__str__(), "Radius must be greater than 0.")

    def test_fillet_segments_acute_angle(self):
        """Test fillet creation with acute angle"""
        # Create LineString with acute angle
        original_geom = QgsGeometry.fromWkt("LineString (2 0, 0 0, 1 1.732)")

        # Extract segment points
        segment1_start = QgsPoint(2.0, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(1.0, 1.732)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().fillet(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.1, 0
        )

        expected_wkt = "CompoundCurve ((2 0, 0.17 0),CircularString (0.17 0, 0.09 0.05, 0.09 0.15),(0.09 0.15, 1 1.73))"
        self.assertEqual(result.asWkt(2), expected_wkt)

    # FILLET TESTS - VERTEX-BASED OVERLOAD

    def test_fillet_vertex_linestring_default_segments(self):
        """Test applying fillet to vertex in a linestring with default segments"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 0, 1 1)")

        result = linestring.fillet(1, 0.1)

        self.assertFalse(result.isEmpty())
        if not result.isEmpty():
            self.assertEqual(result.wkbType(), QgsWkbTypes.LineString)
            points = result.asPolyline()
            self.assertEqual(points[0].x(), 0)
            self.assertEqual(points[0].y(), 0)
            self.assertEqual(points[-1].x(), 1)
            self.assertEqual(points[-1].y(), 1)
            self.assertGreater(len(points), 3)

    def test_fillet_vertex_linestring_custom_segments(self):
        """Test applying fillet with custom segment count"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 0, 1 1)")

        result = linestring.fillet(1, 0.1, 12)

        self.assertFalse(result.isEmpty())
        if not result.isEmpty():
            self.assertEqual(result.wkbType(), QgsWkbTypes.LineString)
            self.assertGreater(len(result.asPolyline()), 10)

    def test_fillet_vertex_with_z_values(self):
        """Test fillet at vertex with Z coordinate preservation"""
        linestring = QgsGeometry.fromWkt("LineStringZ (0 0 0, 1 0 5, 1 1 10)")

        result = linestring.fillet(1, 0.1)

        self.assertFalse(result.isEmpty())
        if not result.isEmpty():
            wkt = result.asWkt()
            self.assertTrue("Z" in wkt or "z" in wkt.lower())

            points = result.asPolyline()
            for point in points:
                if hasattr(point, "z") and point.z() is not None:
                    self.assertTrue(0 <= point.z() <= 10)

    def test_fillet_vertex_invalid_index_first(self):
        """Test fillet with invalid vertex index (first vertex)"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 0, 1 1)")

        result = linestring.fillet(0, 0.1)
        self.assertTrue(result.isEmpty())
        self.assertEqual(
            linestring.lastError(),
            "Vertex index out of range. 0 must be in (0, 2). Requested vertex: 0 was resolved as: [part: -1, ring: -1, vertex: 0]",
        )

    def test_fillet_vertex_invalid_index_last(self):
        """Test fillet with invalid vertex index (last vertex)"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 0, 1 1)")

        result = linestring.fillet(2, 0.1)
        self.assertTrue(result.isEmpty())
        self.assertEqual(
            linestring.lastError(),
            "Vertex index out of range. 2 must be in (0, 2). Requested vertex: 2 was resolved as: [part: -1, ring: -1, vertex: 2]",
        )

    def test_fillet_vertex_large_radius_failure(self):
        """Test fillet with radius too large for available geometry"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 0.1 0, 0.1 0.1)")

        result = linestring.fillet(1, 1.0)
        self.assertFalse(result.isEmpty())

        # After fix, the exact tangent points are now included
        expected_wkt = "LineString (0 0, 0 0, 0.02 0, 0.04 0.01, 0.06 0.02, 0.07 0.03, 0.08 0.04, 0.09 0.06, 0.1 0.08, 0.1 0.1, 0.1 0.1)"
        self.assertEqual(result.asWkt(2), expected_wkt)

    # EDGE CASES AND ERROR HANDLING

    def test_empty_geometry_handling_chamfer(self):
        """Test that chamfer handles empty geometries gracefully"""
        empty_geom = QgsGeometry()

        result = empty_geom.chamfer(1, 0.1, 0.1)

        self.assertTrue(result.isEmpty())

    def test_empty_geometry_handling_fillet(self):
        """Test that fillet handles empty geometries gracefully"""
        empty_geom = QgsGeometry()

        result = empty_geom.fillet(1, 0.1)

        self.assertTrue(result.isEmpty())

    def test_single_point_geometry_chamfer(self):
        """Test chamfer behavior with point geometries"""
        point = QgsGeometry.fromWkt("Point (0 0)")

        result = point.chamfer(0, 0.1, 0.1)

        self.assertTrue(result.isEmpty())

    def test_single_point_geometry_fillet(self):
        """Test fillet behavior with point geometries"""
        point = QgsGeometry.fromWkt("Point (0 0)")

        result = point.fillet(0, 0.1)
        self.assertTrue(result.isEmpty())

    def test_two_point_linestring_chamfer(self):
        """Test chamfer behavior with minimum linestring"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 1)")

        result = linestring.chamfer(1, 0.1, 0.1)
        self.assertTrue(result.isEmpty())
        self.assertEqual(
            linestring.lastError(),
            "Opened curve must have at least 3 points. Requested vertex: 1 was resolved as: [part: -1, ring: -1, vertex: 1]",
        )

    def test_two_point_linestring_fillet(self):
        """Test fillet behavior with minimum linestring"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 1)")

        result = linestring.fillet(1, 0.1)
        self.assertTrue(result.isEmpty())
        self.assertEqual(
            linestring.lastError(),
            "Opened curve must have at least 3 points. Requested vertex: 1 was resolved as: [part: -1, ring: -1, vertex: 1]",
        )

    def test_geometryutils_two_point_linestring_fillet(self):
        """Test fillet behavior with minimum linestring from QgsGeometryUtils. Should throw exception"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 1)")

        try:
            result = QgsGeometryUtils.filletVertex(linestring.get(), 1, 0.1, 8)
            self.fail("Should have failed!")
        except QgsInvalidArgumentException as e:
            self.assertEqual(e.__str__(), "Opened curve must have at least 3 points.")

    def test_geometryutils_two_point_linestring_chamfer(self):
        """Test chamfer behavior with minimum linestring from QgsGeometryUtils. Should throw exception"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 1)")

        try:
            result = QgsGeometryUtils.chamferVertex(linestring.get(), 1, 0.1, 0.1)
            self.fail("Should have failed!")
        except QgsInvalidArgumentException as e:
            self.assertEqual(e.__str__(), "Opened curve must have at least 3 points.")

    def test_geometryutils_bad_distance_linestring_chamfer(self):
        """Test chamfer behavior with minimum linestring from QgsGeometryUtils. Should throw exception"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 1, 1 2, 2 3)")

        try:
            result = QgsGeometryUtils.chamferVertex(linestring.get(), 1, -0.1, -0.1)
            self.fail("Should have failed!")
        except QgsInvalidArgumentException as e:
            self.assertEqual(e.__str__(), "Negative distances.")

    def test_polygon_geometry_handling_chamfer(self):
        """Test chamfer behavior with polygon geometries"""
        polygon = QgsGeometry.fromWkt("Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))")

        result = polygon.chamfer(1, 0.1)
        self.assertFalse(result.isEmpty())

        expected_wkt = "Polygon ((0 0, 0.9 0, 1 0.1, 1 1, 0 1, 0 0))"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_polygon_geometry_handling_fillet(self):
        """Test fillet behavior with polygon geometries"""
        polygon = QgsGeometry.fromWkt("Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))")

        result = polygon.fillet(1, 0.1)
        self.assertFalse(result.isEmpty())

        # After fix, the exact tangent points are now included
        expected_wkt = "Polygon ((0 0, 0.9 0, 0.92 0, 0.94 0.01, 0.96 0.02, 0.97 0.03, 0.98 0.04, 0.99 0.06, 1 0.08, 1 0.1, 1 1, 0 1, 0 0))"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_polygon_geometry_handling_fillet_first_vertex(self):
        """Test fillet behavior with polygon geometries"""
        polygon = QgsGeometry.fromWkt("Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))")

        result = polygon.fillet(0, 0.1)
        self.assertFalse(result.isEmpty())

        # After fix, the exact tangent points are now included
        expected_wkt = "Polygon ((0 0.1, 0 0.08, 0.01 0.06, 0.02 0.04, 0.03 0.03, 0.04 0.02, 0.06 0.01, 0.08 0, 0.1 0, 1 0, 1 1, 0 1, 0 0.1))"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_polygon_geometry_handling_fillet_last_vertex(self):
        """Test fillet behavior with polygon geometries"""
        polygon = QgsGeometry.fromWkt("Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))")

        result = polygon.fillet(4, 0.1)
        self.assertFalse(result.isEmpty())

        # After fix, the exact tangent points are now included
        expected_wkt = "Polygon ((1 0, 1 1, 0 1, 0 0.1, 0 0.08, 0.01 0.06, 0.02 0.04, 0.03 0.03, 0.04 0.02, 0.06 0.01, 0.08 0, 0.1 0, 1 0))"
        self.assertEqual(result.asWkt(2), expected_wkt)

    # PRECISION AND PERFORMANCE TESTS

    def test_precision_with_small_values_chamfer(self):
        """Test chamfer precision handling with small coordinate values"""
        # Create LineString with very small segments
        original_geom = QgsGeometry.fromWkt("LineString (0.001 0, 0 0, 0 0.001)")

        # Extract segment points
        segment1_start = QgsPoint(0.001, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, 0.001)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().chamfer(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.0001, 0.0001
        )

        expected_wkt = "LineString (0 0, 0 0, 0 0, 0 0)"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_precision_with_small_values_fillet(self):
        """Test fillet precision handling with small coordinate values"""
        # Create LineString with very small segments
        original_geom = QgsGeometry.fromWkt("LineString (0.001 0, 0 0, 0 0.001)")

        # Extract segment points
        segment1_start = QgsPoint(0.001, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, 0.001)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().fillet(
            segment1_start, segment1_end, segment2_start, segment2_end, 0.0001, 0
        )

        expected_wkt = (
            "CompoundCurve ((0 0, 0 0),CircularString (0 0, 0 0, 0 0),(0 0, 0 0))"
        )
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_precision_with_large_values_chamfer(self):
        """Test chamfer precision handling with large coordinate values"""
        # Create LineString with large segments
        original_geom = QgsGeometry.fromWkt("LineString (1000 0, 0 0, 0 1000)")

        # Extract segment points
        segment1_start = QgsPoint(1000.0, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, 1000.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().chamfer(
            segment1_start, segment1_end, segment2_start, segment2_end, 10.0, 10.0
        )

        expected_wkt = "LineString (1000 0, 10 0, 0 10, 0 1000)"
        self.assertEqual(result.asWkt(0), expected_wkt)

    def test_precision_with_large_values_fillet(self):
        """Test fillet precision handling with large coordinate values"""
        # Create LineString with large segments
        original_geom = QgsGeometry.fromWkt("LineString (1000 0, 0 0, 0 1000)")

        # Extract segment points
        segment1_start = QgsPoint(1000.0, 0.0)
        segment1_end = QgsPoint(0.0, 0.0)
        segment2_start = QgsPoint(0.0, 1000.0)
        segment2_end = QgsPoint(0.0, 0.0)

        result = QgsGeometry().fillet(
            segment1_start, segment1_end, segment2_start, segment2_end, 10.0, 0
        )

        expected_wkt = "CompoundCurve ((1000 0, 10 0),CircularString (10 0, 2.93 2.93, 0 10),(0 10, 0 1000))"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_method_overload_disambiguation_result1(self):
        """Test that chamfer method overloads work correctly (symmetric)"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 0, 1 1)")

        result1 = linestring.chamfer(1, 0.1)

        expected_wkt = "LineString (0 0, 0.9 0, 1 0.1, 1 1)"
        self.assertEqual(result1.asWkt(2), expected_wkt)

    def test_method_overload_disambiguation_result2(self):
        """Test that chamfer method overloads work correctly (asymmetric)"""
        linestring = QgsGeometry.fromWkt("LineString (0 0, 1 0, 1 1)")

        result2 = linestring.chamfer(1, 0.1, 0.2)

        expected_wkt = "LineString (0 0, 0.9 0, 1 0.2, 1 1)"
        self.assertEqual(result2.asWkt(2), expected_wkt)

    def test_building_outline_corners(self):
        """Test chamfering corners of a building outline"""
        building = QgsGeometry.fromWkt("LineString (0 0, 10 0, 10 5, 0 5, 0 0)")

        result = building.chamfer(1, 0.5)

        expected_wkt = "LineString (0 0, 9.5 0, 10 0.5, 10 5, 0 5, 0 0)"
        self.assertEqual(result.asWkt(2), expected_wkt)

    def test_road_intersection_rounding(self):
        """Test filleting road intersections"""
        road = QgsGeometry.fromWkt("LineString (0 5, 5 5, 5 0)")

        result = road.fillet(1, 0.5)

        self.assertFalse(result.isEmpty())
        self.assertEqual(result.wkbType(), QgsWkbTypes.LineString)

        if not result.isEmpty():
            points = result.asPolyline()
            # Verify start and end points are preserved
            self.assertAlmostEqual(points[0].x(), 0, places=1)
            self.assertAlmostEqual(points[0].y(), 5, places=1)
            self.assertAlmostEqual(points[-1].x(), 5, places=1)
            self.assertAlmostEqual(points[-1].y(), 0, places=1)
            # Verify that there are many points (segmented fillet)
            self.assertGreaterEqual(len(points), 10)

    def test_fillet_vertex_compound_curve_preserve_circular(self):
        """Test fillet on CompoundCurve vertex preserving CircularString nature"""
        # CompoundCurve with linear segment, circular arc, and linear segment
        compound = QgsGeometry.fromWkt(
            "CompoundCurve((0 0, 10 0), CircularString(10 0, 11.414213562373096 0.5857864376269049, 12 2), (12 2, 12 4, 10 4))"
        )

        # Apply fillet at vertex index 4 in last LineString
        result = compound.fillet(4, 1, -1)
        self.assertEqual(compound.lastError(), "")

        expected_wkt = "CompoundCurve ((0 0, 10 0),CircularString (10 0, 11.41 0.59, 12 2),(12 2, 12 3),CircularString (12 3, 11.71 3.71, 11 4),(11 4, 10 4))"
        self.assertEqual(result.asWkt(2), expected_wkt)

        # Verify the result is still a CompoundCurve
        self.assertEqual(result.wkbType(), QgsWkbTypes.CompoundCurve)

        # Verify it contains both linear and circular segments
        compound_result = result.constGet()
        if compound_result:
            wkt = result.asWkt()
            self.assertIn("CircularString", wkt)
            self.assertIn("CompoundCurve", wkt)

    def test_fillet_vertex_compound_curve_preserve_circular_closed_first_vertex(self):
        """Test fillet on CompoundCurve vertex preserving CircularString nature"""
        # CompoundCurve with linear segment, circular arc, and linear segment
        compound = QgsGeometry.fromWkt(
            "CompoundCurve((0 0, 10 0), CircularString(10 0, 11.414213562373096 0.5857864376269049, 12 2), (12 2, 12 4, 0 0))"
        )

        # Apply fillet at vertex index 4 in last LineString
        result = compound.fillet(0, 1, -1)
        self.assertEqual(compound.lastError(), "")

        expected_wkt = "CompoundCurve (CircularString (5.85 1.95, 5.18 0.84, 6.16 0),(6.16 0, 10 0),CircularString (10 0, 10.74 0.22, 11.41 0.59),(12 2, 12 4, 5.85 1.95))"
        self.assertEqual(result.asWkt(2), expected_wkt)

        # Verify the result is still a CompoundCurve
        self.assertEqual(result.wkbType(), QgsWkbTypes.CompoundCurve)

        # Verify it contains both linear and circular segments
        compound_result = result.constGet()
        if compound_result:
            wkt = result.asWkt()
            self.assertIn("CircularString", wkt)
            self.assertIn("CompoundCurve", wkt)

    def test_fillet_vertex_compound_curve_preserve_circular_closed_last_vertex(self):
        """Test fillet on CompoundCurve vertex preserving CircularString nature"""
        # CompoundCurve with linear segment, circular arc, and linear segment
        compound = QgsGeometry.fromWkt(
            "CompoundCurve((0 0, 10 0), CircularString(10 0, 11.414213562373096 0.5857864376269049, 12 2), (12 2, 12 4, 0 0))"
        )

        # Apply fillet at vertex index 4 in last LineString
        result = compound.fillet(5, 1, -1)
        self.assertEqual(compound.lastError(), "")

        expected_wkt = "CompoundCurve ((6.16 0, 10 0),CircularString (8.85 -0.23, 11.41 0.59, 12 2),(12 2, 12 4, 5.85 1.95),CircularString (5.85 1.95, 5.18 0.84, 6.16 0))"
        self.assertEqual(result.asWkt(2), expected_wkt)

        # Verify the result is still a CompoundCurve
        self.assertEqual(result.wkbType(), QgsWkbTypes.CompoundCurve)

        # Verify it contains both linear and circular segments
        compound_result = result.constGet()
        if compound_result:
            wkt = result.asWkt()
            self.assertIn("CircularString", wkt)
            self.assertIn("CompoundCurve", wkt)

    def test_chamfer_vertex_compound_curve_preserve_circular(self):
        """Test chamfer on CompoundCurve vertex preserving CircularString nature"""
        # CompoundCurve with linear segment, circular arc, and linear segment
        compound = QgsGeometry.fromWkt(
            "CompoundCurve((0 0, 10 0), CircularString(10 0, 11.414213562373096 0.5857864376269049, 12 2), (12 2, 12 4, 10 4))"
        )

        # Apply chamfer at vertex index 4 in last LineString
        result = compound.chamfer(4, 1)
        self.assertEqual(compound.lastError(), "")

        expected_wkt = "CompoundCurve ((0 0, 10 0),CircularString (10 0, 11.41 0.59, 12 2),(12 2, 12 3),(12 3, 11 4),(11 4, 10 4))"
        self.assertEqual(result.asWkt(2), expected_wkt)

        # Verify the result is still a CompoundCurve
        self.assertEqual(result.wkbType(), QgsWkbTypes.CompoundCurve)

        # Verify it contains the original circular segment
        compound_result = result.constGet()
        if compound_result:
            wkt = result.asWkt()
            self.assertIn("CircularString", wkt)
            self.assertIn("CompoundCurve", wkt)

    def test_chamfer_vertex_compound_curve_break_circular(self):
        """Test chamfer on CompoundCurve vertex breaking CircularString nature"""
        # CompoundCurve with linear segment, circular arc, and linear segment
        compound = QgsGeometry.fromWkt(
            "CompoundCurve((0 0, 10 0), CircularString(10 0, 11.414213562373096 0.5857864376269049, 12 2), (12 2, 12 4, 10 4))"
        )

        # Apply chamfer at vertex index 3 within CircularString
        result = compound.chamfer(3, 1)
        self.assertEqual(compound.lastError(), "")

        expected_wkt = "CompoundCurve ((0 0, 10 0),(10 0, 11.41 0.59, 11.62 1.08),(11.62 1.08, 12 3),(12 2, 12 4, 10 4))"
        self.assertEqual(result.asWkt(2), expected_wkt)

        # Verify the result is still a CompoundCurve
        self.assertEqual(result.wkbType(), QgsWkbTypes.CompoundCurve)

        # Verify it does not contain the original circular segment
        compound_result = result.constGet()
        if compound_result:
            wkt = result.asWkt()
            self.assertIn("CompoundCurve", wkt)

    def test_chamfer_vertex_compound_curve_preserve_circular_opened_first_vertex(self):
        """Test chamfer on CompoundCurve vertex preserving CircularString nature"""
        # CompoundCurve with linear segment, circular arc, and linear segment
        compound = QgsGeometry.fromWkt(
            "CompoundCurve((0 0, 10 0), CircularString(10 0, 11.414213562373096 0.5857864376269049, 12 2), (12 2, 12 4, 10 4))"
        )

        # Apply chamfer at vertex index 0 ==> out of range
        result = compound.chamfer(0, 1)
        self.assertEqual(
            compound.lastError(),
            "Vertex index out of range. 0 must be in (0, 5). Requested vertex: 0 was resolved as: [part: -1, ring: -1, vertex: 0]",
        )

    def test_chamfer_vertex_compound_curve_preserve_circular_closed_first_vertex(self):
        """Test chamfer on CompoundCurve vertex preserving CircularString nature"""
        # CompoundCurve with linear segment, circular arc, and linear segment
        compound = QgsGeometry.fromWkt(
            "CompoundCurve((0 0, 10 0), CircularString(10 0, 11.414213562373096 0.5857864376269049, 12 2), (12 2, 12 4, 0 0))"
        )

        # Apply chamfer at vertex index 0
        result = compound.chamfer(0, 1)
        self.assertEqual(compound.lastError(), "")
        expected_wkt = "CompoundCurve ((0.95 0.32, 1 0),(1 0, 10 0),CircularString (10 0, 10.72 0.27, 11.41 0.59),(12 2, 12 4, 0.95 0.32))"
        self.assertEqual(result.asWkt(2), expected_wkt)

        # Verify the result is still a CompoundCurve
        self.assertEqual(result.wkbType(), QgsWkbTypes.CompoundCurve)

        # Verify it contains the original circular segment
        compound_result = result.constGet()
        if compound_result:
            wkt = result.asWkt()
            self.assertIn("CircularString", wkt)
            self.assertIn("CompoundCurve", wkt)

    def test_chamfer_vertex_compound_curve_preserve_circular_closed_last_vertex(self):
        """Test chamfer on CompoundCurve vertex preserving CircularString nature"""
        # CompoundCurve with linear segment, circular arc, and linear segment
        compound = QgsGeometry.fromWkt(
            "CompoundCurve((0 0, 10 0), CircularString(10 0, 11.414213562373096 0.5857864376269049, 12 2), (12 2, 12 4, 0 0))"
        )

        # Apply chamfer at vertex index 0
        result = compound.chamfer(5, 1)
        self.assertEqual(compound.lastError(), "")
        expected_wkt = "CompoundCurve ((1 0, 10 0),CircularString (6.27 -0.74, 11.41 0.59, 12 2),(12 2, 12 4, 0.95 0.32),(0.95 0.32, 1 0))"
        self.assertEqual(result.asWkt(2), expected_wkt)

        # Verify the result is still a CompoundCurve
        self.assertEqual(result.wkbType(), QgsWkbTypes.CompoundCurve)

        # Verify it contains the original circular segment
        compound_result = result.constGet()
        if compound_result:
            wkt = result.asWkt()
            self.assertIn("CircularString", wkt)
            self.assertIn("CompoundCurve", wkt)

    def test_max_fillet_radius(self):
        """Test maxFilletRadius utility function"""

        # Test 90-degree angle (right angle)
        segment1_start = QgsPoint(2, 0)
        segment1_end = QgsPoint(0, 0)
        segment2_start = QgsPoint(0, 0)
        segment2_end = QgsPoint(0, 2)

        max_radius = QgsGeometryUtils.maxFilletRadius(
            segment1_start, segment1_end, segment2_start, segment2_end
        )
        # For a right angle with segments of length 2, max radius should be 2
        self.assertAlmostEqual(max_radius, 2.0, places=5)

        # Test 60-degree angle
        import math

        segment1_start = QgsPoint(2, 0)
        segment1_end = QgsPoint(0, 0)
        segment2_start = QgsPoint(0, 0)
        segment2_end = QgsPoint(math.cos(math.pi / 3) * 2, math.sin(math.pi / 3) * 2)

        max_radius = QgsGeometryUtils.maxFilletRadius(
            segment1_start, segment1_end, segment2_start, segment2_end
        )
        # For 60-degree angle, tan(30°) = 0.577
        expected = 2.0 * math.tan(math.pi / 6)
        self.assertAlmostEqual(max_radius, expected, places=5)

        # Test parallel segments (should return -1)
        segment1_start = QgsPoint(0, 0)
        segment1_end = QgsPoint(1, 0)
        segment2_start = QgsPoint(0, 1)
        segment2_end = QgsPoint(1, 1)

        max_radius = QgsGeometryUtils.maxFilletRadius(
            segment1_start, segment1_end, segment2_start, segment2_end
        )
        self.assertEqual(max_radius, -1.0)

        # Test non-intersecting segments that converge when extended
        segment1_start = QgsPoint(0, 0)
        segment1_end = QgsPoint(1, 0)
        segment2_start = QgsPoint(3, 0)
        segment2_end = QgsPoint(3, 1)

        max_radius = QgsGeometryUtils.maxFilletRadius(
            segment1_start, segment1_end, segment2_start, segment2_end
        )
        # Intersection at (3, 0), distance to (3,1) is 1, so max radius is 1
        self.assertAlmostEqual(max_radius, 1.0, places=5)

        # Test with very small segments
        segment1_start = QgsPoint(0.1, 0)
        segment1_end = QgsPoint(0, 0)
        segment2_start = QgsPoint(0, 0)
        segment2_end = QgsPoint(0, 0.1)

        max_radius = QgsGeometryUtils.maxFilletRadius(
            segment1_start, segment1_end, segment2_start, segment2_end
        )
        # For small segments, max radius should be 0.1
        self.assertAlmostEqual(max_radius, 0.1, places=5)


if __name__ == "__main__":
    unittest.main()
