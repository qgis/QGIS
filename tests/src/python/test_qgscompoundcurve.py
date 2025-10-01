"""QGIS Unit tests for QgsCompoundCurve.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Loïc Bartoletti"
__date__ = "19/12/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import qgis  # NOQA

from qgis.core import QgsCompoundCurve, QgsCircularString, QgsLineString, QgsPoint, QgsVertexId
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsCompoundCurve(QgisTestCase):

    def testFuzzyComparisons(self):

        ######
        # 2D #
        ######
        # Error on the LineString
        epsilon = 0.001
        geom1 = QgsCompoundCurve()
        geom2 = QgsCompoundCurve()

        line1 = QgsLineString(QgsPoint(0.0, 0.0), QgsPoint(0.001, 0.001))
        circularString1 = QgsCircularString(
            QgsPoint(0.0, 0.0), QgsPoint(0.001, 0.001), QgsPoint(0.5, 0.5)
        )
        geom1.addCurve(line1)
        geom1.addCurve(circularString1)

        line2 = QgsLineString(QgsPoint(0.0, 0.0), QgsPoint(0.002, 0.002))
        circularString2 = QgsCircularString(
            QgsPoint(0.0, 0.0), QgsPoint(0.001, 0.001), QgsPoint(0.5, 0.5)
        )
        geom2.addCurve(line2)
        geom2.addCurve(circularString2)

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # Error on the CircularString
        epsilon = 0.001
        geom1 = QgsCompoundCurve()
        geom2 = QgsCompoundCurve()

        line1 = QgsLineString(QgsPoint(0.0, 0.0), QgsPoint(0.001, 0.001))
        circularString1 = QgsCircularString(
            QgsPoint(0.0, 0.0), QgsPoint(0.001, 0.001), QgsPoint(0.5, 0.5)
        )
        geom1.addCurve(line1)
        geom1.addCurve(circularString1)

        line2 = QgsLineString(QgsPoint(0.0, 0.0), QgsPoint(0.001, 0.001))
        circularString2 = QgsCircularString(
            QgsPoint(0.0, 0.0), QgsPoint(0.002, 0.002), QgsPoint(0.5, 0.5)
        )
        geom2.addCurve(line2)
        geom2.addCurve(circularString2)

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

        #######
        # 3DZ #
        #######
        # Error on the LineString
        epsilon = 0.001
        geom1 = QgsCompoundCurve()
        geom2 = QgsCompoundCurve()

        line1 = QgsLineString(QgsPoint(0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001))
        circularString1 = QgsCircularString(
            QgsPoint(0.0, 0.0, 0.0),
            QgsPoint(0.001, 0.001, 0.001),
            QgsPoint(0.5, 0.5, 0.5),
        )
        geom1.addCurve(line1)
        geom1.addCurve(circularString1)

        line2 = QgsLineString(QgsPoint(0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.002))
        circularString2 = QgsCircularString(
            QgsPoint(0.0, 0.0, 0.0),
            QgsPoint(0.001, 0.001, 0.001),
            QgsPoint(0.5, 0.5, 0.5),
        )
        geom2.addCurve(line2)
        geom2.addCurve(circularString2)

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # Error on the CircularString
        epsilon = 0.001
        geom1 = QgsCompoundCurve()
        geom2 = QgsCompoundCurve()

        line1 = QgsLineString(
            QgsPoint(0.0, 0.0, m=0.0), QgsPoint(0.001, 0.001, m=0.001)
        )
        circularString1 = QgsCircularString(
            QgsPoint(0.0, 0.0, m=0.0),
            QgsPoint(0.001, 0.001, m=0.001),
            QgsPoint(0.5, 0.5, m=0.5),
        )
        geom1.addCurve(line1)
        geom1.addCurve(circularString1)

        line2 = QgsLineString(
            QgsPoint(0.0, 0.0, m=0.0), QgsPoint(0.001, 0.001, m=0.001)
        )
        circularString2 = QgsCircularString(
            QgsPoint(0.0, 0.0, m=0.0),
            QgsPoint(0.001, 0.001, m=0.002),
            QgsPoint(0.5, 0.5, m=0.5),
        )
        geom2.addCurve(line2)
        geom2.addCurve(circularString2)

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

        #######
        # 3DM #
        #######
        # Error on the LineString
        epsilon = 0.001
        geom1 = QgsCompoundCurve()
        geom2 = QgsCompoundCurve()

        line1 = QgsLineString(
            QgsPoint(0.0, 0.0, m=0.0), QgsPoint(0.001, 0.001, m=0.001)
        )
        circularString1 = QgsCircularString(
            QgsPoint(0.0, 0.0, m=0.0),
            QgsPoint(0.001, 0.001, m=0.001),
            QgsPoint(0.5, 0.5, m=0.5),
        )
        geom1.addCurve(line1)
        geom1.addCurve(circularString1)

        line2 = QgsLineString(
            QgsPoint(0.0, 0.0, m=0.0), QgsPoint(0.001, 0.001, m=0.002)
        )
        circularString2 = QgsCircularString(
            QgsPoint(0.0, 0.0, m=0.0),
            QgsPoint(0.001, 0.001, m=0.001),
            QgsPoint(0.5, 0.5, m=0.5),
        )
        geom2.addCurve(line2)
        geom2.addCurve(circularString2)

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # Error on the CircularString
        epsilon = 0.001
        geom1 = QgsCompoundCurve()
        geom2 = QgsCompoundCurve()

        line1 = QgsLineString(QgsPoint(0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001))
        circularString1 = QgsCircularString(
            QgsPoint(0.0, 0.0, 0.0),
            QgsPoint(0.001, 0.001, 0.001),
            QgsPoint(0.5, 0.5, 0.5),
        )
        geom1.addCurve(line1)
        geom1.addCurve(circularString1)

        line2 = QgsLineString(QgsPoint(0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001))
        circularString2 = QgsCircularString(
            QgsPoint(0.0, 0.0, 0.0),
            QgsPoint(0.001, 0.001, 0.002),
            QgsPoint(0.5, 0.5, 0.5),
        )
        geom2.addCurve(line2)
        geom2.addCurve(circularString2)

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

        ######
        # 4D #
        ######
        # Error on the LineString
        epsilon = 0.001
        geom1 = QgsCompoundCurve()
        geom2 = QgsCompoundCurve()

        line1 = QgsLineString(
            QgsPoint(0.0, 0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001, 0.001)
        )
        circularString1 = QgsCircularString(
            QgsPoint(0.0, 0.0, 0.0, 0.0),
            QgsPoint(0.001, 0.001, 0.001, 0.001),
            QgsPoint(0.5, 0.5, 0.5, 0.5),
        )
        geom1.addCurve(line1)
        geom1.addCurve(circularString1)

        line2 = QgsLineString(
            QgsPoint(0.0, 0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.002, 0.002)
        )
        circularString2 = QgsCircularString(
            QgsPoint(0.0, 0.0, 0.0, 0.0),
            QgsPoint(0.001, 0.001, 0.001, 0.001),
            QgsPoint(0.5, 0.5, 0.5, 0.5),
        )
        geom2.addCurve(line2)
        geom2.addCurve(circularString2)

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # Error on the CircularString
        epsilon = 0.001
        geom1 = QgsCompoundCurve()
        geom2 = QgsCompoundCurve()

        line1 = QgsLineString(
            QgsPoint(0.0, 0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001, 0.001)
        )
        circularString1 = QgsCircularString(
            QgsPoint(0.0, 0.0, 0.0, 0.0),
            QgsPoint(0.001, 0.001, 0.001, 0.001),
            QgsPoint(0.5, 0.5, 0.5, 0.5),
        )
        geom1.addCurve(line1)
        geom1.addCurve(circularString1)

        line2 = QgsLineString(
            QgsPoint(0.0, 0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001, 0.001)
        )
        circularString2 = QgsCircularString(
            QgsPoint(0.0, 0.0, 0.0, 0.0),
            QgsPoint(0.001, 0.001, 0.002, 0.002),
            QgsPoint(0.5, 0.5, 0.5, 0.5),
        )
        geom2.addCurve(line2)
        geom2.addCurve(circularString2)

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

    def test_simplify_by_distance(self):
        """
        test simplifyByDistance
        """
        p = QgsCompoundCurve()
        p.fromWkt(
            "CompoundCurve (CircularString (4.40660981021897413 0.93610259854013833, 11.01953454014598321 23.6382050218978037, 34.67607970802919226 28.41041874452553984),(34.67607970802919226 28.41041874452553984, 46.06121816058393392 30.38747871532845934, 61.74134896350363988 29.02398908029196178))"
        )
        self.assertEqual(
            p.simplifyByDistance(0.5).asWkt(3),
            "LineString (4.407 0.936, 3.765 8.905, 5.88 16.615, 10.217 22.855, 16.706 27.525, 21.235 29.154, 26.003 29.808, 34.676 28.41, 46.061 30.387, 61.741 29.024)",
        )
        self.assertEqual(
            p.simplifyByDistance(0.75).asWkt(3),
            "LineString (4.407 0.936, 3.765 8.905, 5.88 16.615, 10.217 22.855, 16.706 27.525, 26.003 29.808, 34.676 28.41, 46.061 30.387, 61.741 29.024)",
        )
        self.assertEqual(
            p.simplifyByDistance(1).asWkt(3),
            "LineString (4.407 0.936, 3.765 8.905, 5.88 16.615, 10.217 22.855, 16.706 27.525, 26.003 29.808, 34.676 28.41, 46.061 30.387, 61.741 29.024)",
        )

    def test_distance_between_vertices(self):
        """
        Test distanceBetweenVertices method for QgsCompoundCurve
        """
        import math

        # Test case: CompoundCurve((-10 0, -3 -7, 0 0), CircularString(0 0, 3 7, 10 10), (10 10, 12 10, 12 12))
        compound = QgsCompoundCurve()
        compound.fromWkt("CompoundCurve((-10 0, -3 -7, 0 0), CircularString(0 0, 3 7, 10 10), (10 10, 12 10, 12 12))")

        # Verify the compound curve was created correctly
        self.assertTrue(compound.isValid())
        self.assertEqual(compound.nCurves(), 3)  # Should have 3 component curves

        # Get total number of vertices
        total_vertices = compound.numPoints()
        self.assertGreater(total_vertices, 0)

        # Test basic functionality - distance from first to last vertex should equal total length
        total_length = compound.length()
        distance_first_to_last = compound.distanceBetweenVertices(QgsVertexId(0, 0, 0), QgsVertexId(0, 0, total_vertices - 1))
        self.assertAlmostEqual(distance_first_to_last, total_length, places=10)

        # Test edge cases
        self.assertEqual(compound.distanceBetweenVertices(QgsVertexId(0, 0, 0), QgsVertexId(0, 0, 0)), 0.0)  # Same vertex
        self.assertEqual(compound.distanceBetweenVertices(QgsVertexId(0, 0, 0), QgsVertexId(0, 0, 999)), -1.0)  # Invalid vertex index

        # Test reverse direction (should give same result)
        distance_reverse = compound.distanceBetweenVertices(QgsVertexId(0, 0, total_vertices - 1), QgsVertexId(0, 0, 0))
        self.assertAlmostEqual(distance_reverse, distance_first_to_last, places=10)

        # Test intermediate distances
        if total_vertices >= 3:
            mid_vertex = total_vertices // 2
            distance_first_to_mid = compound.distanceBetweenVertices(QgsVertexId(0, 0, 0), QgsVertexId(0, 0, mid_vertex))
            distance_mid_to_last = compound.distanceBetweenVertices(QgsVertexId(0, 0, mid_vertex), QgsVertexId(0, 0, total_vertices - 1))

            # Sum should equal total
            sum_distances = distance_first_to_mid + distance_mid_to_last
            self.assertAlmostEqual(sum_distances, distance_first_to_last, places=8)

        # Test a simpler compound curve for more predictable results
        simple_compound = QgsCompoundCurve()

        # Create a compound with two line segments: (0,0)-(2,0) and (2,0)-(4,0)
        line1 = QgsLineString()
        line1.fromWkt("LineString(0 0, 2 0)")
        simple_compound.addCurve(line1)

        line2 = QgsLineString()
        line2.fromWkt("LineString(2 0, 4 0)")
        simple_compound.addCurve(line2)

        # Test distances on the simple compound curve
        self.assertEqual(simple_compound.numPoints(), 3)  # (0,0), (2,0), (4,0)

        # Distance from point 0 to point 1 should be 2.0
        dist_0_to_1 = simple_compound.distanceBetweenVertices(QgsVertexId(0, 0, 0), QgsVertexId(0, 0, 1))
        self.assertAlmostEqual(dist_0_to_1, 2.0, places=10)

        # Distance from point 1 to point 2 should be 2.0
        dist_1_to_2 = simple_compound.distanceBetweenVertices(QgsVertexId(0, 0, 1), QgsVertexId(0, 0, 2))
        self.assertAlmostEqual(dist_1_to_2, 2.0, places=10)

        # Total distance from point 0 to point 2 should be 4.0
        dist_0_to_2 = simple_compound.distanceBetweenVertices(QgsVertexId(0, 0, 0), QgsVertexId(0, 0, 2))
        self.assertAlmostEqual(dist_0_to_2, 4.0, places=10)

        # Verify that the sum equals the total
        self.assertAlmostEqual(dist_0_to_1 + dist_1_to_2, dist_0_to_2, places=10)


if __name__ == "__main__":
    unittest.main()
