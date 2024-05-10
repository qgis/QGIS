"""QGIS Unit tests for QgsLineString.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Loïc Bartoletti'
__date__ = '12/09/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

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
        self.assertEqual(m_line.asWkt(0), "LineStringM EMPTY")

        line = QgsLineString([[0, 0], [2, 0], [4, 0]])
        m_line = line.measuredLine(10, 20)
        self.assertEqual(m_line, QgsLineString([QgsPoint(0, 0, m=10), QgsPoint(2, 0, m=15), QgsPoint(4, 0, m=20)]))

        line = QgsLineString([[0, 0], [9, 0], [10, 0]])
        m_line = line.measuredLine(10, 20)
        self.assertEqual(m_line, QgsLineString([QgsPoint(0, 0, m=10), QgsPoint(9, 0, m=19), QgsPoint(10, 0, m=20)]))

        line = QgsLineString([[0, 0], [0, 0], [0, 0]])
        m_line = line.measuredLine(10, 20)
        self.assertEqual(m_line, QgsLineString([QgsPoint(0, 0, m=10), QgsPoint(0, 0, m=15), QgsPoint(0, 0, m=20)]))

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
        geom1 = QgsLineString(QgsPoint(0.0, 0.0, m=0.0), QgsPoint(0.001, 0.001, m=0.001))
        geom2 = QgsLineString(QgsPoint(0.0, 0.0, m=0.0), QgsPoint(0.001, 0.001, m=0.002))

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
        geom1 = QgsLineString(QgsPoint(0.0, 0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001, 0.001))
        geom2 = QgsLineString(QgsPoint(0.0, 0.0, 0.0, 0.0), QgsPoint(0.002, 0.002, 0.002, 0.002))

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
        line.fromWkt('LineString (10 6, 20 6)')
        self.assertIsNone(line.interpolateM())

        # single point
        line.fromWkt('LineStringM (10 6 0)')
        self.assertEqual(line.interpolateM().asWkt(), 'LineStringM (10 6 0)')

        # valid cases
        line.fromWkt('LineStringM (10 6 0, 20 6 10)')
        self.assertEqual(line.interpolateM().asWkt(), 'LineStringM (10 6 0, 20 6 10)')

        line.fromWkt('LineStringM (10 6 1, 20 6 0, 10 10 1)')
        self.assertEqual(line.interpolateM().asWkt(), 'LineStringM (10 6 1, 20 6 0, 10 10 1)')

        line.fromWkt('LineStringZM (10 6 1 5, 20 6 0 6, 10 10 1 7)')
        self.assertEqual(line.interpolateM().asWkt(), 'LineStringZM (10 6 1 5, 20 6 0 6, 10 10 1 7)')

        # no valid m values
        line = QgsLineString([[10, 6, 1, math.nan], [20, 6, 2, math.nan]])
        self.assertEqual(line.wkbType(), Qgis.WkbType.LineStringZM)
        self.assertIsNone(line.interpolateM())

        # missing m values at start of line
        line = QgsLineString([[10, 6, 1, math.nan], [20, 6, 2, 13], [20, 10, 5, 17]])
        self.assertEqual(line.interpolateM().asWkt(), 'LineStringZM (10 6 1 13, 20 6 2 13, 20 10 5 17)')

        line = QgsLineString([[10, 6, 1, math.nan], [20, 6, 2, math.nan], [20, 10, 5, 17]])
        self.assertEqual(line.interpolateM().asWkt(), 'LineStringZM (10 6 1 17, 20 6 2 17, 20 10 5 17)')

        line = QgsLineString([[10, 6, 1, math.nan], [20, 6, 2, math.nan], [20, 10, 5, 17], [22, 10, 15, 19]])
        self.assertEqual(line.interpolateM().asWkt(), 'LineStringZM (10 6 1 17, 20 6 2 17, 20 10 5 17, 22 10 15 19)')

        # missing m values at end of line
        line = QgsLineString([[20, 6, 2, 13], [20, 10, 5, 17], [10, 6, 1, math.nan]])
        self.assertEqual(line.interpolateM().asWkt(), 'LineStringZM (20 6 2 13, 20 10 5 17, 10 6 1 17)')

        line = QgsLineString([[20, 10, 5, 17], [10, 6, 1, math.nan], [20, 6, 2, math.nan]])
        self.assertEqual(line.interpolateM().asWkt(), 'LineStringZM (20 10 5 17, 10 6 1 17, 20 6 2 17)')

        line = QgsLineString([[20, 10, 5, 17], [22, 10, 15, 19], [10, 6, 1, math.nan], [20, 6, 2, math.nan]])
        self.assertEqual(line.interpolateM().asWkt(), 'LineStringZM (20 10 5 17, 22 10 15 19, 10 6 1 19, 20 6 2 19)')

        # missing m values in middle of line
        line = QgsLineString([[20, 10, 5, 17], [30, 10, 12, math.nan], [30, 40, 17, 27]])
        # 2d distance
        self.assertEqual(line.interpolateM(False).asWkt(), 'LineStringZM (20 10 5 17, 30 10 12 19.5, 30 40 17 27)')
        # 3d distance
        self.assertEqual(line.interpolateM(True).asWkt(2), 'LineStringZM (20 10 5 17, 30 10 12 19.86, 30 40 17 27)')

        line = QgsLineString([[20, 10, 5, 17], [30, 10, 12, math.nan], [30, 40, 17, math.nan], [20, 40, 19, 27]])
        # 2d distance
        self.assertEqual(line.interpolateM(False).asWkt(), 'LineStringZM (20 10 5 17, 30 10 12 19, 30 40 17 25, 20 40 19 27)')
        # 3d distance
        self.assertEqual(line.interpolateM(True).asWkt(2), 'LineStringZM (20 10 5 17, 30 10 12 19.31, 30 40 17 25.07, 20 40 19 27)')

        line = QgsLineString([[20, 10, 5, 17], [30, 10, 12, math.nan], [30, 40, 17, math.nan], [20, 40, 19, math.nan], [20, 50, 21, 29]])
        # 2d distance
        self.assertEqual(line.interpolateM(False).asWkt(), 'LineStringZM (20 10 5 17, 30 10 12 19, 30 40 17 25, 20 40 19 27, 20 50 21 29)')
        # 3d distance
        self.assertEqual(line.interpolateM(True).asWkt(2), 'LineStringZM (20 10 5 17, 30 10 12 19.32, 30 40 17 25.12, 20 40 19 27.06, 20 50 21 29)')

        # multiple missing chunks
        line = QgsLineString([[20, 10, 5, 17], [30, 10, 12, math.nan], [30, 40, 17, math.nan], [20, 40, 19, 27], [20, 50, 21, math.nan], [25, 50, 22, 30]])
        # 2d distance
        self.assertEqual(line.interpolateM(False).asWkt(), 'LineStringZM (20 10 5 17, 30 10 12 19, 30 40 17 25, 20 40 19 27, 20 50 21 29, 25 50 22 30)')
        # 3d distance
        self.assertEqual(line.interpolateM(True).asWkt(2), 'LineStringZM (20 10 5 17, 30 10 12 19.31, 30 40 17 25.07, 20 40 19 27, 20 50 21 29, 25 50 22 30)')

        line = QgsLineString([[20, 10, 5, 17], [30, 10, 12, math.nan], [30, 40, 17, math.nan], [20, 40, 19, 27], [20, 50, 21, math.nan], [25, 50, 22, math.nan], [25, 55, 22, math.nan], [30, 55, 22, 37]])
        # 2d distance
        self.assertEqual(line.interpolateM(False).asWkt(), 'LineStringZM (20 10 5 17, 30 10 12 19, 30 40 17 25, 20 40 19 27, 20 50 21 31, 25 50 22 33, 25 55 22 35, 30 55 22 37)')
        # 3d distance
        self.assertEqual(line.interpolateM(True).asWkt(2), 'LineStringZM (20 10 5 17, 30 10 12 19.31, 30 40 17 25.07, 20 40 19 27, 20 50 21 31.03, 25 50 22 33.05, 25 55 22 35.02, 30 55 22 37)')

        # missing at start and middle
        line = QgsLineString([[10, 10, 1, math.nan], [10, 12, 2, math.nan], [20, 10, 5, 17], [30, 10, 12, math.nan], [30, 40, 17, math.nan], [20, 40, 19, math.nan], [20, 50, 21, 29]])
        # 2d distance
        self.assertEqual(line.interpolateM(False).asWkt(), 'LineStringZM (10 10 1 17, 10 12 2 17, 20 10 5 17, 30 10 12 19, 30 40 17 25, 20 40 19 27, 20 50 21 29)')
        # 3d distance
        self.assertEqual(line.interpolateM(True).asWkt(2), 'LineStringZM (10 10 1 17, 10 12 2 17, 20 10 5 17, 30 10 12 19.72, 30 40 17 25.27, 20 40 19 27.14, 20 50 21 29)')

        # missing at middle and end
        line = QgsLineString([[20, 10, 5, 17], [30, 10, 12, math.nan], [30, 40, 17, math.nan], [20, 40, 19, 27], [20, 50, 21, math.nan], [25, 50, 22, math.nan], [25, 55, 22, math.nan]])
        # 2d distance
        self.assertEqual(line.interpolateM(False).asWkt(), 'LineStringZM (20 10 5 17, 30 10 12 19, 30 40 17 25, 20 40 19 27, 20 50 21 27, 25 50 22 27, 25 55 22 27)')
        # 3d distance
        self.assertEqual(line.interpolateM(True).asWkt(2), 'LineStringZM (20 10 5 17, 30 10 12 19.31, 30 40 17 25.07, 20 40 19 27, 20 50 21 27, 25 50 22 27, 25 55 22 27)')

        # missing at start, middle, end
        line = QgsLineString([[5, 10, 15, math.nan], [6, 11, 16, math.nan], [20, 10, 5, 17], [30, 10, 12, math.nan], [30, 40, 17, math.nan], [20, 40, 19, 27], [20, 50, 21, math.nan], [25, 50, 22, math.nan], [25, 55, 22, math.nan]])
        # 2d distance
        self.assertEqual(line.interpolateM(False).asWkt(), 'LineStringZM (5 10 15 17, 6 11 16 17, 20 10 5 17, 30 10 12 19, 30 40 17 25, 20 40 19 27, 20 50 21 27, 25 50 22 27, 25 55 22 27)')
        # 3d distance
        self.assertEqual(line.interpolateM(True).asWkt(2), 'LineStringZM (5 10 15 17, 6 11 16 17, 20 10 5 17, 30 10 12 19.05, 30 40 17 25, 20 40 19 27, 20 50 21 27, 25 50 22 27, 25 55 22 27)')


if __name__ == '__main__':
    unittest.main()
