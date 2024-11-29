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

from qgis.core import QgsCompoundCurve, QgsCircularString, QgsLineString, QgsPoint
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


if __name__ == "__main__":
    unittest.main()
