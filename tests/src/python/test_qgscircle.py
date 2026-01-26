"""QGIS Unit tests for QgsCircularString.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Loïc Bartoletti"
__date__ = "29/01/2025"
__copyright__ = "Copyright 2025, The QGIS Project"

import qgis  # NOQA

from qgis.core import Qgis, QgsCircle, QgsPoint
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsCircularString(QgisTestCase):

    def testSegmentCalculation(self):
        """Test different methods for calculating segments for circle approximation"""

        # Test point (EPSG:3946)
        center = QgsPoint(1981498.81943113403394818, 5199309.83504835329949856)
        radius = 0.1

        # Store results for comparison
        results = []

        # Expected values from the actual implementation
        expected_values = [
            # radius, standard, adaptive, area, constant
            [0.1, 8, 8, 257, 8],
            [1.0, 23, 13, 257, 8],
            [10.0, 71, 41, 257, 20],
            [100.0, 223, 100, 257, 200],
            [1000.0, 703, 217, 257, 2000],
            [10000.0, 2222, 445, 257, 20000],
            [100000.0, 7025, 884, 257, 200000],
            [1000000.0, 22215, 1721, 257, 2000000],
        ]

        while radius <= 1500000:
            circle = QgsCircle(center, radius)
            tolerance = 0.01
            constant = 2
            min_segments = 8

            # Test all methods
            standard_segments = circle.calculateSegments(
                radius,
                tolerance,
                min_segments,
                Qgis.SegmentCalculationMethod.Standard,
            )
            adaptive_segments = circle.calculateSegments(
                radius,
                tolerance,
                min_segments,
                Qgis.SegmentCalculationMethod.Adaptive,
            )
            area_segments = circle.calculateSegments(
                radius,
                tolerance,
                min_segments,
                Qgis.SegmentCalculationMethod.AreaError,
            )
            constant_segments = circle.calculateSegments(
                radius,
                constant,
                min_segments,
                Qgis.SegmentCalculationMethod.ConstantDensity,
            )

            # Store current results
            results.append(
                [
                    radius,
                    standard_segments,
                    adaptive_segments,
                    area_segments,
                    constant_segments,
                ]
            )

            # Find corresponding expected values
            expected = next(
                val for val in expected_values if abs(val[0] - radius) < 0.01
            )

            # Test against expected values
            self.assertEqual(
                standard_segments,
                expected[1],
                f"Standard method for radius {radius} should give {expected[1]} segments",
            )
            self.assertEqual(
                adaptive_segments,
                expected[2],
                f"Adaptive method for radius {radius} should give {expected[2]} segments",
            )
            self.assertEqual(
                area_segments,
                expected[3],
                f"Area method for radius {radius} should give {expected[3]} segments",
            )
            self.assertEqual(
                constant_segments,
                expected[4],
                f"Constant method for radius {radius} should give {expected[4]} segments",
            )

            radius *= 10


if __name__ == "__main__":
    unittest.main()
