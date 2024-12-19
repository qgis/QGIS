"""QGIS Unit tests for QgsLineString.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Loïc Bartoletti"
__date__ = "12/09/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import unittest
import math

from qgis.core import Qgis, QgsLineString, QgsPoint
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLineString(QgisTestCase):

    def testConstruct(self):
        # With points
        line = QgsLineString(QgsPoint(1, 2), QgsPoint(3, 4))
        self.assertEqual(line.startPoint(), QgsPoint(1, 2))
        self.assertEqual(line.endPoint(), QgsPoint(3, 4))

        # With list
        line = QgsLineString([[1, 2], [3, 4]])
        self.assertEqual(line.startPoint(), QgsPoint(1, 2))
        self.assertEqual(line.endPoint(), QgsPoint(3, 4))

    def testMeasureLine(self):
        line = QgsLineString()
        m_line = line.measuredLine(10, 20)
        self.assertEqual(m_line.asWkt(0), "LineString M EMPTY")

        line = QgsLineString([[0, 0], [2, 0], [4, 0]])
        m_line = line.measuredLine(10, 20)
        self.assertEqual(
            m_line,
            QgsLineString(
                [QgsPoint(0, 0, m=10), QgsPoint(2, 0, m=15), QgsPoint(4, 0, m=20)]
            ),
        )

        line = QgsLineString([[0, 0], [9, 0], [10, 0]])
        m_line = line.measuredLine(10, 20)
        self.assertEqual(
            m_line,
            QgsLineString(
                [QgsPoint(0, 0, m=10), QgsPoint(9, 0, m=19), QgsPoint(10, 0, m=20)]
            ),
        )

        line = QgsLineString([[0, 0], [0, 0], [0, 0]])
        m_line = line.measuredLine(10, 20)
        self.assertEqual(
            m_line,
            QgsLineString(
                [QgsPoint(0, 0, m=10), QgsPoint(0, 0, m=15), QgsPoint(0, 0, m=20)]
            ),
        )

    def testFuzzyComparisons(self):
        ######
        # 2D #
        ######
        epsilon = 0.001
        geom1 = QgsLineString(QgsPoint(0.0, 0.0), QgsPoint(0.001, 0.001))
        geom2 = QgsLineString(QgsPoint(0.0, 0.0), QgsPoint(0.002, 0.002))

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
        geom1 = QgsLineString(QgsPoint(0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001))
        geom2 = QgsLineString(QgsPoint(0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.002))

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
        geom1 = QgsLineString(
            QgsPoint(0.0, 0.0, m=0.0), QgsPoint(0.001, 0.001, m=0.001)
        )
        geom2 = QgsLineString(
            QgsPoint(0.0, 0.0, m=0.0), QgsPoint(0.001, 0.001, m=0.002)
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
        geom1 = QgsLineString(
            QgsPoint(0.0, 0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001, 0.001)
        )
        geom2 = QgsLineString(
            QgsPoint(0.0, 0.0, 0.0, 0.0), QgsPoint(0.002, 0.002, 0.002, 0.002)
        )

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

    def testInterpolateM(self):
        line = QgsLineString()

        # empty line
        self.assertIsNone(line.interpolateM())

        # not m
        line.fromWkt("LineString (10 6, 20 6)")
        self.assertIsNone(line.interpolateM())

        # single point
        line.fromWkt("LineStringM (10 6 0)")
        self.assertEqual(line.interpolateM().asWkt(), "LineString M (10 6 0)")

        # valid cases
        line.fromWkt("LineStringM (10 6 0, 20 6 10)")
        self.assertEqual(line.interpolateM().asWkt(), "LineString M (10 6 0, 20 6 10)")

        line.fromWkt("LineStringM (10 6 1, 20 6 0, 10 10 1)")
        self.assertEqual(
            line.interpolateM().asWkt(), "LineString M (10 6 1, 20 6 0, 10 10 1)"
        )

        line.fromWkt("LineStringZM (10 6 1 5, 20 6 0 6, 10 10 1 7)")
        self.assertEqual(
            line.interpolateM().asWkt(), "LineString ZM (10 6 1 5, 20 6 0 6, 10 10 1 7)"
        )

        # no valid m values
        line = QgsLineString([[10, 6, 1, math.nan], [20, 6, 2, math.nan]])
        self.assertEqual(line.wkbType(), Qgis.WkbType.LineStringZM)
        self.assertIsNone(line.interpolateM())

        # missing m values at start of line
        line = QgsLineString([[10, 6, 1, math.nan], [20, 6, 2, 13], [20, 10, 5, 17]])
        self.assertEqual(
            line.interpolateM().asWkt(),
            "LineString ZM (10 6 1 13, 20 6 2 13, 20 10 5 17)",
        )

        line = QgsLineString(
            [[10, 6, 1, math.nan], [20, 6, 2, math.nan], [20, 10, 5, 17]]
        )
        self.assertEqual(
            line.interpolateM().asWkt(),
            "LineString ZM (10 6 1 17, 20 6 2 17, 20 10 5 17)",
        )

        line = QgsLineString(
            [
                [10, 6, 1, math.nan],
                [20, 6, 2, math.nan],
                [20, 10, 5, 17],
                [22, 10, 15, 19],
            ]
        )
        self.assertEqual(
            line.interpolateM().asWkt(),
            "LineString ZM (10 6 1 17, 20 6 2 17, 20 10 5 17, 22 10 15 19)",
        )

        # missing m values at end of line
        line = QgsLineString([[20, 6, 2, 13], [20, 10, 5, 17], [10, 6, 1, math.nan]])
        self.assertEqual(
            line.interpolateM().asWkt(),
            "LineString ZM (20 6 2 13, 20 10 5 17, 10 6 1 17)",
        )

        line = QgsLineString(
            [[20, 10, 5, 17], [10, 6, 1, math.nan], [20, 6, 2, math.nan]]
        )
        self.assertEqual(
            line.interpolateM().asWkt(),
            "LineString ZM (20 10 5 17, 10 6 1 17, 20 6 2 17)",
        )

        line = QgsLineString(
            [
                [20, 10, 5, 17],
                [22, 10, 15, 19],
                [10, 6, 1, math.nan],
                [20, 6, 2, math.nan],
            ]
        )
        self.assertEqual(
            line.interpolateM().asWkt(),
            "LineString ZM (20 10 5 17, 22 10 15 19, 10 6 1 19, 20 6 2 19)",
        )

        # missing m values in middle of line
        line = QgsLineString(
            [[20, 10, 5, 17], [30, 10, 12, math.nan], [30, 40, 17, 27]]
        )
        # 2d distance
        self.assertEqual(
            line.interpolateM(False).asWkt(),
            "LineString ZM (20 10 5 17, 30 10 12 19.5, 30 40 17 27)",
        )
        # 3d distance
        self.assertEqual(
            line.interpolateM(True).asWkt(2),
            "LineString ZM (20 10 5 17, 30 10 12 19.86, 30 40 17 27)",
        )

        line = QgsLineString(
            [
                [20, 10, 5, 17],
                [30, 10, 12, math.nan],
                [30, 40, 17, math.nan],
                [20, 40, 19, 27],
            ]
        )
        # 2d distance
        self.assertEqual(
            line.interpolateM(False).asWkt(),
            "LineString ZM (20 10 5 17, 30 10 12 19, 30 40 17 25, 20 40 19 27)",
        )
        # 3d distance
        self.assertEqual(
            line.interpolateM(True).asWkt(2),
            "LineString ZM (20 10 5 17, 30 10 12 19.31, 30 40 17 25.07, 20 40 19 27)",
        )

        line = QgsLineString(
            [
                [20, 10, 5, 17],
                [30, 10, 12, math.nan],
                [30, 40, 17, math.nan],
                [20, 40, 19, math.nan],
                [20, 50, 21, 29],
            ]
        )
        # 2d distance
        self.assertEqual(
            line.interpolateM(False).asWkt(),
            "LineString ZM (20 10 5 17, 30 10 12 19, 30 40 17 25, 20 40 19 27, 20 50 21 29)",
        )
        # 3d distance
        self.assertEqual(
            line.interpolateM(True).asWkt(2),
            "LineString ZM (20 10 5 17, 30 10 12 19.32, 30 40 17 25.12, 20 40 19 27.06, 20 50 21 29)",
        )

        # multiple missing chunks
        line = QgsLineString(
            [
                [20, 10, 5, 17],
                [30, 10, 12, math.nan],
                [30, 40, 17, math.nan],
                [20, 40, 19, 27],
                [20, 50, 21, math.nan],
                [25, 50, 22, 30],
            ]
        )
        # 2d distance
        self.assertEqual(
            line.interpolateM(False).asWkt(),
            "LineString ZM (20 10 5 17, 30 10 12 19, 30 40 17 25, 20 40 19 27, 20 50 21 29, 25 50 22 30)",
        )
        # 3d distance
        self.assertEqual(
            line.interpolateM(True).asWkt(2),
            "LineString ZM (20 10 5 17, 30 10 12 19.31, 30 40 17 25.07, 20 40 19 27, 20 50 21 29, 25 50 22 30)",
        )

        line = QgsLineString(
            [
                [20, 10, 5, 17],
                [30, 10, 12, math.nan],
                [30, 40, 17, math.nan],
                [20, 40, 19, 27],
                [20, 50, 21, math.nan],
                [25, 50, 22, math.nan],
                [25, 55, 22, math.nan],
                [30, 55, 22, 37],
            ]
        )
        # 2d distance
        self.assertEqual(
            line.interpolateM(False).asWkt(),
            "LineString ZM (20 10 5 17, 30 10 12 19, 30 40 17 25, 20 40 19 27, 20 50 21 31, 25 50 22 33, 25 55 22 35, 30 55 22 37)",
        )
        # 3d distance
        self.assertEqual(
            line.interpolateM(True).asWkt(2),
            "LineString ZM (20 10 5 17, 30 10 12 19.31, 30 40 17 25.07, 20 40 19 27, 20 50 21 31.03, 25 50 22 33.05, 25 55 22 35.02, 30 55 22 37)",
        )

        # missing at start and middle
        line = QgsLineString(
            [
                [10, 10, 1, math.nan],
                [10, 12, 2, math.nan],
                [20, 10, 5, 17],
                [30, 10, 12, math.nan],
                [30, 40, 17, math.nan],
                [20, 40, 19, math.nan],
                [20, 50, 21, 29],
            ]
        )
        # 2d distance
        self.assertEqual(
            line.interpolateM(False).asWkt(),
            "LineString ZM (10 10 1 17, 10 12 2 17, 20 10 5 17, 30 10 12 19, 30 40 17 25, 20 40 19 27, 20 50 21 29)",
        )
        # 3d distance
        self.assertEqual(
            line.interpolateM(True).asWkt(2),
            "LineString ZM (10 10 1 17, 10 12 2 17, 20 10 5 17, 30 10 12 19.72, 30 40 17 25.27, 20 40 19 27.14, 20 50 21 29)",
        )

        # missing at middle and end
        line = QgsLineString(
            [
                [20, 10, 5, 17],
                [30, 10, 12, math.nan],
                [30, 40, 17, math.nan],
                [20, 40, 19, 27],
                [20, 50, 21, math.nan],
                [25, 50, 22, math.nan],
                [25, 55, 22, math.nan],
            ]
        )
        # 2d distance
        self.assertEqual(
            line.interpolateM(False).asWkt(),
            "LineString ZM (20 10 5 17, 30 10 12 19, 30 40 17 25, 20 40 19 27, 20 50 21 27, 25 50 22 27, 25 55 22 27)",
        )
        # 3d distance
        self.assertEqual(
            line.interpolateM(True).asWkt(2),
            "LineString ZM (20 10 5 17, 30 10 12 19.31, 30 40 17 25.07, 20 40 19 27, 20 50 21 27, 25 50 22 27, 25 55 22 27)",
        )

        # missing at start, middle, end
        line = QgsLineString(
            [
                [5, 10, 15, math.nan],
                [6, 11, 16, math.nan],
                [20, 10, 5, 17],
                [30, 10, 12, math.nan],
                [30, 40, 17, math.nan],
                [20, 40, 19, 27],
                [20, 50, 21, math.nan],
                [25, 50, 22, math.nan],
                [25, 55, 22, math.nan],
            ]
        )
        # 2d distance
        self.assertEqual(
            line.interpolateM(False).asWkt(),
            "LineString ZM (5 10 15 17, 6 11 16 17, 20 10 5 17, 30 10 12 19, 30 40 17 25, 20 40 19 27, 20 50 21 27, 25 50 22 27, 25 55 22 27)",
        )
        # 3d distance
        self.assertEqual(
            line.interpolateM(True).asWkt(2),
            "LineString ZM (5 10 15 17, 6 11 16 17, 20 10 5 17, 30 10 12 19.05, 30 40 17 25, 20 40 19 27, 20 50 21 27, 25 50 22 27, 25 55 22 27)",
        )

    def testLineLocatePointByM(self):
        line = QgsLineString()
        # empty line
        res, x, y, z, distance = line.lineLocatePointByM(5)
        self.assertFalse(res)

        # not m
        line.fromWkt("LineString (10 6, 20 6)")
        res, x, y, z, distance = line.lineLocatePointByM(5)
        self.assertFalse(res)

        # single point
        line.fromWkt("LineStringM (10 6 0)")
        res, x, y, z, distance = line.lineLocatePointByM(5)
        self.assertFalse(res)

        # valid cases
        line.fromWkt("LineStringM (10 6 0, 20 6 10)")
        res, x, y, z, distance = line.lineLocatePointByM(0)
        self.assertTrue(res)
        self.assertEqual(x, 10)
        self.assertEqual(y, 6)
        self.assertEqual(distance, 0)

        res, x, y, z, distance = line.lineLocatePointByM(10)
        self.assertTrue(res)
        self.assertEqual(x, 20)
        self.assertEqual(y, 6)
        self.assertEqual(distance, 10)

        res, x, y, z, distance = line.lineLocatePointByM(5)
        self.assertTrue(res)
        self.assertEqual(x, 15)
        self.assertEqual(y, 6)
        self.assertEqual(distance, 5)

        res, x, y, z, distance = line.lineLocatePointByM(3)
        self.assertTrue(res)
        self.assertEqual(x, 13)
        self.assertEqual(y, 6)
        self.assertEqual(distance, 3)

        # m out of range
        res, x, y, z, distance = line.lineLocatePointByM(15)
        self.assertFalse(res)
        res, x, y, z, distance = line.lineLocatePointByM(-5)
        self.assertFalse(res)

        line.fromWkt("LineStringM (10 6 1, 20 6 0)")
        res, x, y, z, distance = line.lineLocatePointByM(1.0)
        self.assertTrue(res)
        self.assertEqual(x, 10)
        self.assertEqual(y, 6)
        self.assertEqual(distance, 0)

        res, x, y, z, distance = line.lineLocatePointByM(0.0)
        self.assertTrue(res)
        self.assertEqual(x, 20)
        self.assertEqual(y, 6)
        self.assertEqual(distance, 10)

        res, x, y, z, distance = line.lineLocatePointByM(0.5)
        self.assertTrue(res)
        self.assertEqual(x, 15)
        self.assertEqual(y, 6)
        self.assertEqual(distance, 5)

        res, x, y, z, distance = line.lineLocatePointByM(0.3)
        self.assertTrue(res)
        self.assertEqual(x, 17)
        self.assertEqual(y, 6)
        self.assertEqual(distance, 7)

        line.fromWkt("LineStringM (10 6 10, 20 6 0, 20 16 -10)")
        res, x, y, z, distance = line.lineLocatePointByM(5)
        self.assertTrue(res)
        self.assertEqual(x, 15)
        self.assertEqual(y, 6)
        self.assertEqual(distance, 5)

        res, x, y, z, distance = line.lineLocatePointByM(-5)
        self.assertTrue(res)
        self.assertEqual(x, 20)
        self.assertEqual(y, 11)
        self.assertEqual(distance, 15)

        line.setMAt(1, math.nan)

        res, x, y, z, distance = line.lineLocatePointByM(0)
        self.assertTrue(res)
        self.assertEqual(x, 20)
        self.assertEqual(y, 6)
        self.assertEqual(distance, 10)

        res, x, y, z, distance = line.lineLocatePointByM(5)
        self.assertTrue(res)
        self.assertEqual(x, 15)
        self.assertEqual(y, 6)
        self.assertEqual(distance, 5)

        res, x, y, z, distance = line.lineLocatePointByM(-5)
        self.assertTrue(res)
        self.assertEqual(x, 20)
        self.assertEqual(y, 11)
        self.assertEqual(distance, 15)

        res, x, y, z, distance = line.lineLocatePointByM(10)
        self.assertTrue(res)
        self.assertEqual(x, 10)
        self.assertEqual(y, 6)
        self.assertEqual(distance, 0)

        res, x, y, z, distance = line.lineLocatePointByM(-10)
        self.assertTrue(res)
        self.assertEqual(x, 20)
        self.assertEqual(y, 16)
        self.assertEqual(distance, 20)

        # with nan m value to fill in
        line.fromWkt("LineStringM (10 6 10, 20 6 0, 20 16 -10, 30 16 -10)")
        line.setMAt(1, math.nan)
        line.setMAt(2, math.nan)
        res, x, y, z, distance = line.lineLocatePointByM(5)
        self.assertTrue(res)
        self.assertEqual(x, 17.5)
        self.assertEqual(y, 6)
        self.assertAlmostEqual(distance, 7.5, 3)

        res, x, y, z, distance = line.lineLocatePointByM(0)
        self.assertTrue(res)
        self.assertEqual(x, 20)
        self.assertEqual(y, 11)
        self.assertEqual(distance, 15)

        # m value on constant m segment
        line.fromWkt("LineStringM (10 6 10, 20 6 1, 20 16 1, 30 16 -10)")
        res, x, y, z, distance = line.lineLocatePointByM(1)
        self.assertTrue(res)
        self.assertEqual(x, 20)
        self.assertEqual(y, 11)
        self.assertAlmostEqual(distance, 15, 3)

        line.fromWkt("LineStringM (10 6 10, 20 6 1, 20 16 1, 26 16 1)")
        res, x, y, z, distance = line.lineLocatePointByM(1)
        self.assertTrue(res)
        self.assertEqual(x, 20)
        self.assertEqual(y, 14)
        self.assertEqual(distance, 18)

    def test_simplify_by_distance(self):
        """
        test simplifyByDistance
        """
        p = QgsLineString()
        # should never become < 2 vertices
        p.fromWkt("LineString(1 1, 1.001 1.001)")
        self.assertEqual(
            p.simplifyByDistance(0.5).asWkt(3), "LineString (1 1, 1.001 1.001)"
        )
        p.fromWkt(
            "LineString (4.40700000000000003 0.93600000000000005, 3.76500000000000012 8.90499999999999936, 5.87999999999999989 16.61499999999999844, 10.21700000000000053 22.85500000000000043, 16.70599999999999952 27.52499999999999858, 26.00300000000000011 29.80799999999999983, 34.67600000000000193 28.41000000000000014, 46.06099999999999994 30.38700000000000045, 61.74099999999999966 29.02400000000000091)"
        )
        self.assertEqual(
            p.simplifyByDistance(0.75).asWkt(3),
            "LineString (4.407 0.936, 3.765 8.905, 5.88 16.615, 10.217 22.855, 16.706 27.525, 26.003 29.808, 34.676 28.41, 46.061 30.387, 61.741 29.024)",
        )
        self.assertEqual(
            p.simplifyByDistance(2).asWkt(3),
            "LineString (4.407 0.936, 5.88 16.615, 16.706 27.525, 61.741 29.024)",
        )
        # ported geos tests
        p.fromWkt("LINESTRING (0 5, 1 5, 2 5, 5 5)")
        self.assertEqual(p.simplifyByDistance(10).asWkt(), "LineString (0 5, 5 5)")
        p.fromWkt("LINESTRING (1 0, 2 0, 2 2, 0 2, 0 0, 1 0)")
        self.assertEqual(
            p.simplifyByDistance(0).asWkt(), "LineString (2 0, 2 2, 0 2, 0 0, 2 0)"
        )

    def test_orientation(self):
        """
        test orientation. From https://github.com/qgis/QGIS/issues/58333
        """
        geom = QgsLineString()
        geom.fromWkt("LineString (1 1, 2 1, 2 2, 1 2, 1 1)")
        self.assertEqual(geom.sumUpArea(), 1.0)
        self.assertEqual(geom.orientation(), Qgis.AngularDirection.CounterClockwise)
        geom.fromWkt("LineString (1 1, 1 2, 2 2, 2 1, 1 1)")
        self.assertEqual(geom.sumUpArea(), -1.0)
        self.assertEqual(geom.orientation(), Qgis.AngularDirection.Clockwise)
        geom = geom.reversed()
        self.assertEqual(geom.asWkt(), "LineString (1 1, 2 1, 2 2, 1 2, 1 1)")
        self.assertEqual(geom.sumUpArea(), 1.0)
        self.assertEqual(geom.orientation(), Qgis.AngularDirection.CounterClockwise)


if __name__ == "__main__":
    unittest.main()
