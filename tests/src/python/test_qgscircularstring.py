"""QGIS Unit tests for QgsCircularString.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Loïc Bartoletti"
__date__ = "19/12/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import qgis  # NOQA

from qgis.core import QgsCircularString, QgsPoint, QgsVertexId
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsCircularString(QgisTestCase):

    def testFuzzyComparisons(self):
        ######
        # 2D #
        ######
        epsilon = 0.001
        geom1 = QgsCircularString(
            QgsPoint(0.0, 0.0), QgsPoint(0.001, 0.001), QgsPoint(0.5, 0.5)
        )
        geom2 = QgsCircularString(
            QgsPoint(0.0, 0.0), QgsPoint(0.002, 0.002), QgsPoint(0.5, 0.5)
        )

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
        epsilon = 0.001
        geom1 = QgsCircularString(
            QgsPoint(0.0, 0.0, 0.0),
            QgsPoint(0.001, 0.001, 0.001),
            QgsPoint(0.5, 0.5, 0.5),
        )
        geom2 = QgsCircularString(
            QgsPoint(0.0, 0.0, 0.0),
            QgsPoint(0.001, 0.001, 0.002),
            QgsPoint(0.5, 0.5, 0.5),
        )

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
        epsilon = 0.001
        geom1 = QgsCircularString(
            QgsPoint(0.0, 0.0, m=0.0),
            QgsPoint(0.001, 0.001, m=0.001),
            QgsPoint(0.5, 0.5, m=0.5),
        )
        geom2 = QgsCircularString(
            QgsPoint(0.0, 0.0, m=0.0),
            QgsPoint(0.001, 0.001, m=0.002),
            QgsPoint(0.5, 0.5, m=0.5),
        )

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
        epsilon = 0.001
        geom1 = QgsCircularString(
            QgsPoint(0.0, 0.0, 0.0, 0.0),
            QgsPoint(0.001, 0.001, 0.001, 0.001),
            QgsPoint(0.5, 0.5, 0.5, 0.5),
        )
        geom2 = QgsCircularString(
            QgsPoint(0.0, 0.0, 0.0, 0.0),
            QgsPoint(0.001, 0.001, 0.002, 0.002),
            QgsPoint(0.5, 0.5, 0.5, 0.5),
        )

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
        p = QgsCircularString()
        p.fromWkt(
            "CircularString (0.58883883211678167 0.93610259854013833, 8.76977664233575993 27.04692910948904228, 31.60822802919707541 31.61461938686130324)"
        )
        self.assertEqual(
            p.simplifyByDistance(0.5).asWkt(3),
            "LineString (0.589 0.936, -0.368 7.336, 0.467 14.185, 2.932 20.168, 6.857 25.312, 11.976 29.27, 18.362 31.883, 24.787 32.651, 31.608 31.615)",
        )
        self.assertEqual(
            p.simplifyByDistance(1).asWkt(3),
            "LineString (0.589 0.936, 0.467 14.185, 6.857 25.312, 18.362 31.883, 31.608 31.615)",
        )

    def test_distance_between_vertices(self):
        """
        Test distanceBetweenVertices method for QgsCircularString
        """
        import math

        # Test case 1: CircularString(0 0, 3 7, 10 10)
        arc1 = QgsCircularString()
        arc1.fromWkt("CircularString(0 0, 3 7, 10 10)")

        # For circular strings, the distance is calculated along the arc, not straight line
        # We'll test the basic functionality and compare with total length
        self.assertEqual(arc1.numPoints(), 3)

        # Distance from vertex 0 to vertex 2 should equal the total arc length
        total_length = arc1.length()
        distance_0_to_2 = arc1.distanceBetweenVertices(
            QgsVertexId(0, 0, 0), QgsVertexId(0, 0, 2)
        )
        self.assertAlmostEqual(distance_0_to_2, total_length, places=10)

        # Distance from vertex 0 to vertex 1 (middle point) should be half the arc
        # Note: this is approximate since vertex 1 is the curve control point
        distance_0_to_1 = arc1.distanceBetweenVertices(
            QgsVertexId(0, 0, 0), QgsVertexId(0, 0, 1)
        )
        self.assertAlmostEqual(distance_0_to_1, 7.802700546685484, places=10)
        distance_1_to_2 = arc1.distanceBetweenVertices(
            QgsVertexId(0, 0, 1), QgsVertexId(0, 0, 2)
        )
        self.assertAlmostEqual(distance_1_to_2, 7.802700546685484, places=10)

        # The sum of partial distances should equal total distance
        self.assertAlmostEqual(
            distance_0_to_1 + distance_1_to_2, distance_0_to_2, places=10
        )

        # Test case 2: CircularString(-10 0, -3 -7, 0 0, 3 7, 10 10)
        arc2 = QgsCircularString()
        arc2.fromWkt("CircularString(-10 0, -3 -7, 0 0, 3 7, 10 10)")

        self.assertEqual(arc2.numPoints(), 5)

        # Test total distance equals total arc length
        total_length2 = arc2.length()
        distance_0_to_4 = arc2.distanceBetweenVertices(
            QgsVertexId(0, 0, 0), QgsVertexId(0, 0, 4)
        )
        self.assertAlmostEqual(distance_0_to_4, total_length2, places=10)

        # Test intermediate distances
        distance_0_to_2 = arc2.distanceBetweenVertices(
            QgsVertexId(0, 0, 0), QgsVertexId(0, 0, 2)
        )
        self.assertAlmostEqual(distance_0_to_2, 21.016173298295566, places=10)
        distance_2_to_4 = arc2.distanceBetweenVertices(
            QgsVertexId(0, 0, 2), QgsVertexId(0, 0, 4)
        )
        self.assertAlmostEqual(distance_2_to_4, 15.60540109337097, places=10)

        # Sum should equal total
        self.assertAlmostEqual(
            distance_0_to_2 + distance_2_to_4, distance_0_to_4, places=10
        )

        # Test edge cases
        self.assertEqual(
            arc2.distanceBetweenVertices(QgsVertexId(0, 0, 2), QgsVertexId(0, 0, 2)),
            0.0,
        )  # Same vertex
        self.assertEqual(
            arc2.distanceBetweenVertices(QgsVertexId(0, 0, 0), QgsVertexId(0, 0, 10)),
            -1.0,
        )  # Invalid vertex index

        # Test reverse direction (should give same result)
        self.assertAlmostEqual(
            arc2.distanceBetweenVertices(QgsVertexId(0, 0, 4), QgsVertexId(0, 0, 0)),
            distance_0_to_4,
            places=10,
        )

        # Test that arc distance is different from straight line distance
        # Create a simple arc where we can verify the arc is longer than straight line
        simple_arc = QgsCircularString()
        simple_arc.fromWkt("CircularString(0 0, 1 1, 2 0)")

        arc_distance = simple_arc.distanceBetweenVertices(
            QgsVertexId(0, 0, 0), QgsVertexId(0, 0, 2)
        )
        # Straight line distance from (0,0) to (2,0) would be 2.0
        straight_distance = 2.0

        # Arc distance should be longer than straight line
        self.assertGreater(arc_distance, straight_distance)


if __name__ == "__main__":
    unittest.main()
