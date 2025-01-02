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

from qgis.core import QgsCircularString, QgsPoint
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


if __name__ == "__main__":
    unittest.main()
