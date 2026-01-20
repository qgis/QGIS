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

    def test_point_by_deflection_angle(self):
        """Test pointByDeflectionAngle function"""
        # Test 1: No deflection, continue north
        base = QgsPoint(0, 0)
        direction = QgsPoint(0, 10)
        result = QgsGeometryUtils.pointByDeflectionAngle(base, direction, 0.0, 5.0)
        self.assertAlmostEqual(result.x(), 0.0, places=8)
        self.assertAlmostEqual(result.y(), 5.0, places=8)

        # Test 2: 90 degree right deflection (pi/2 radians)
        result = QgsGeometryUtils.pointByDeflectionAngle(
            base, direction, math.pi / 2, 5.0
        )
        self.assertAlmostEqual(result.x(), 5.0, places=8)
        self.assertAlmostEqual(result.y(), 0.0, places=8)

        # Test 3: 90 degree left deflection (-pi/2 radians)
        result = QgsGeometryUtils.pointByDeflectionAngle(
            base, direction, -math.pi / 2, 5.0
        )
        self.assertAlmostEqual(result.x(), -5.0, places=8)
        self.assertAlmostEqual(result.y(), 0.0, places=8)

        # Test 4: 180 degree deflection (pi radians)
        result = QgsGeometryUtils.pointByDeflectionAngle(base, direction, math.pi, 5.0)
        self.assertAlmostEqual(result.x(), 0.0, places=8)
        self.assertAlmostEqual(result.y(), -5.0, places=8)

        # Test 5: Preserve Z value from basePoint
        base_z = QgsPoint(0, 0, 100)  # PointZ
        result = QgsGeometryUtils.pointByDeflectionAngle(base_z, direction, 0.0, 5.0)
        self.assertTrue(result.is3D())
        self.assertAlmostEqual(result.z(), 100.0, places=8)

        # Test 6: Non-origin base point
        base_offset = QgsPoint(10, 10)
        direction_offset = QgsPoint(10, 20)
        result = QgsGeometryUtils.pointByDeflectionAngle(
            base_offset, direction_offset, 0.0, 5.0
        )
        self.assertAlmostEqual(result.x(), 10.0, places=8)
        self.assertAlmostEqual(result.y(), 15.0, places=8)

        # Test 7: Diagonal direction with 45 degree deflection
        direction_ne = QgsPoint(10, 10)  # NE direction (45 deg from north)
        result = QgsGeometryUtils.pointByDeflectionAngle(
            base, direction_ne, math.pi / 4, math.sqrt(2.0)
        )
        # 45 deg (NE) + 45 deg right = 90 deg = East
        # East direction with distance sqrt(2) gives (sqrt(2), 0)
        self.assertAlmostEqual(result.x(), math.sqrt(2.0), places=8)
        self.assertAlmostEqual(result.y(), 0.0, places=8)


if __name__ == "__main__":
    unittest.main()
