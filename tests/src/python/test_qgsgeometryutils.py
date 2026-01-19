"""QGIS Unit tests for QgsGeometryUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Loïc Bartoletti"
__date__ = "January 2026"
__copyright__ = "Copyright 2026, The QGIS Project"

import math
from qgis.core import (
    QgsGeometryUtils,
    QgsPoint,
)
from qgis.testing import unittest


class TestQgsGeometryUtils(unittest.TestCase):

    def test_line_by_two_angles(self):
        """Test lineByTwoAngles function"""
        # Test 1: Simple right angle intersection
        pt1 = QgsPoint(0, 0)
        pt2 = QgsPoint(10, 0)
        # Point 1 bearing north, Point 2 bearing west -> intersection at origin
        ok, result = QgsGeometryUtils.lineByTwoAngles(
            pt1, 0.0, pt2, 3.0 * math.pi / 2.0
        )
        self.assertTrue(ok)
        self.assertAlmostEqual(result.x(), 0.0, places=8)
        self.assertAlmostEqual(result.y(), 0.0, places=8)

        # Test 2: Lines meeting at center
        # Point 1 at (0,0) bearing NE (45 deg), Point 2 at (10,0) bearing NW (315 deg)
        ok, result = QgsGeometryUtils.lineByTwoAngles(
            pt1, math.pi / 4.0, pt2, 7.0 * math.pi / 4.0
        )
        self.assertTrue(ok)
        self.assertAlmostEqual(result.x(), 5.0, places=8)
        self.assertAlmostEqual(result.y(), 5.0, places=8)

        # Test 3: Parallel lines - no intersection
        pt2_parallel = QgsPoint(0, 5)
        # Both bearing east -> parallel, result should be empty
        ok, result = QgsGeometryUtils.lineByTwoAngles(
            pt1, math.pi / 2.0, pt2_parallel, math.pi / 2.0
        )
        self.assertFalse(ok)

        # Test 4: Z value preservation
        pt1_z = QgsPoint(0, 0, 100)  # PointZ
        ok, result = QgsGeometryUtils.lineByTwoAngles(
            pt1_z, math.pi / 4.0, pt2, 7.0 * math.pi / 4.0
        )
        self.assertTrue(ok)
        self.assertTrue(result.is3D())
        self.assertAlmostEqual(result.z(), 100.0, places=8)

        # Test 5: Negative coordinates intersection
        pt1_neg = QgsPoint(0, 0)
        pt2_neg = QgsPoint(-10, 0)
        # Point 1 bearing SW (225 deg), Point 2 bearing NW (315 deg) -> intersection at (-5,-5)
        ok, result = QgsGeometryUtils.lineByTwoAngles(
            pt1_neg, 5.0 * math.pi / 4.0, pt2_neg, 7.0 * math.pi / 4.0
        )
        self.assertTrue(ok)
        self.assertAlmostEqual(result.x(), -5.0, places=8)
        self.assertAlmostEqual(result.y(), -5.0, places=8)


if __name__ == "__main__":
    unittest.main()
