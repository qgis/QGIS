"""QGIS Unit tests for QgsMultiPolygon.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Loïc Bartoletti'
__date__ = '19/12/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsMultiPolygon, QgsPolygon, QgsLineString, QgsPoint
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsMultiPolygon(QgisTestCase):

    def test_constructor(self):
        p = QgsMultiPolygon([])
        self.assertTrue(p.isEmpty())

        value = QgsPolygon(QgsLineString([[1, 2], [10, 2], [10, 10], [1, 2]]))
        p = QgsMultiPolygon([value])
        self.assertEqual(p.asWkt(), 'MultiPolygon (((1 2, 10 2, 10 10, 1 2)))')
        # constructor should have made internal copy
        del value
        self.assertEqual(p.asWkt(), 'MultiPolygon (((1 2, 10 2, 10 10, 1 2)))')

        p = QgsMultiPolygon([QgsPolygon(QgsLineString([[1, 2], [10, 2], [10, 10], [1, 2]])),
                             QgsPolygon(QgsLineString([[100, 2], [110, 2], [110, 10], [100, 2]]))])
        self.assertEqual(p.asWkt(), 'MultiPolygon (((1 2, 10 2, 10 10, 1 2)),((100 2, 110 2, 110 10, 100 2)))')

        # with z
        p = QgsMultiPolygon([QgsPolygon(QgsLineString([[1, 2, 3], [10, 2, 3], [10, 10, 3], [1, 2, 3]])),
                             QgsPolygon(QgsLineString([[100, 2, 4], [110, 2, 4], [110, 10, 4], [100, 2, 4]]))])
        self.assertEqual(p.asWkt(),
                         'MultiPolygonZ (((1 2 3, 10 2 3, 10 10 3, 1 2 3)),((100 2 4, 110 2 4, 110 10 4, 100 2 4)))')

        # with zm
        p = QgsMultiPolygon([QgsPolygon(QgsLineString([[1, 2, 3, 5], [10, 2, 3, 5], [10, 10, 3, 5], [1, 2, 3, 5]])),
                             QgsPolygon(
                                 QgsLineString([[100, 2, 4, 6], [110, 2, 4, 6], [110, 10, 4, 6], [100, 2, 4, 6]]))])
        self.assertEqual(p.asWkt(),
                         'MultiPolygonZM (((1 2 3 5, 10 2 3 5, 10 10 3 5, 1 2 3 5)),((100 2 4 6, 110 2 4 6, 110 10 4 6, 100 2 4 6)))')

    def testFuzzyComparisons(self):
        ######
        # 2D #
        ######
        epsilon = 0.001
        geom1 = QgsMultiPolygon()
        geom2 = QgsMultiPolygon()

        p1 = QgsPolygon()
        p1.setExteriorRing(
            QgsLineString([QgsPoint(5.0, 5.0), QgsPoint(6.0, 5.0), QgsPoint(6.0, 6.0), QgsPoint(5.0, 5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(
            QgsLineString([QgsPoint(0.0, 0.0), QgsPoint(0.001, 0.001), QgsPoint(0.003, 0.003), QgsPoint(0.0, 0.0)]))
        self.assertTrue(geom1.addGeometry(p1))
        self.assertTrue(geom1.addGeometry(p2))

        p1 = QgsPolygon()
        p1.setExteriorRing(
            QgsLineString([QgsPoint(5.0, 5.0), QgsPoint(6.0, 5.0), QgsPoint(6.0, 6.0), QgsPoint(5.0, 5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(
            QgsLineString([QgsPoint(0.0, 0.0), QgsPoint(0.002, 0.002), QgsPoint(0.003, 0.003), QgsPoint(0.0, 0.0)]))
        self.assertTrue(geom2.addGeometry(p1))
        self.assertTrue(geom2.addGeometry(p2))

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
        geom1 = QgsMultiPolygon()
        geom2 = QgsMultiPolygon()

        p1 = QgsPolygon()
        p1.setExteriorRing(QgsLineString(
            [QgsPoint(5.0, 5.0, 5.0), QgsPoint(6.0, 5.0, 5.0), QgsPoint(6.0, 6.0, 5.0), QgsPoint(5.0, 5.0, 5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(QgsLineString(
            [QgsPoint(0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001), QgsPoint(0.003, 0.003, 0.003),
             QgsPoint(0.0, 0.0, 0.0)]))
        self.assertTrue(geom1.addGeometry(p1))
        self.assertTrue(geom1.addGeometry(p2))

        p1 = QgsPolygon()
        p1.setExteriorRing(QgsLineString(
            [QgsPoint(5.0, 5.0, 5.0), QgsPoint(6.0, 5.0, 5.0), QgsPoint(6.0, 6.0, 5.0), QgsPoint(5.0, 5.0, 5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(QgsLineString(
            [QgsPoint(0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.002), QgsPoint(0.003, 0.003, 0.003),
             QgsPoint(0.0, 0.0, 0.0)]))
        self.assertTrue(geom2.addGeometry(p1))
        self.assertTrue(geom2.addGeometry(p2))

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
        geom1 = QgsMultiPolygon()
        geom2 = QgsMultiPolygon()

        p1 = QgsPolygon()
        p1.setExteriorRing(QgsLineString(
            [QgsPoint(5.0, 5.0, m=5.0), QgsPoint(6.0, 5.0, m=5.0), QgsPoint(6.0, 6.0, m=5.0),
             QgsPoint(5.0, 5.0, m=5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(QgsLineString(
            [QgsPoint(0.0, 0.0, m=0.0), QgsPoint(0.001, 0.001, m=0.001), QgsPoint(0.003, 0.003, m=0.003),
             QgsPoint(0.0, 0.0, m=0.0)]))
        self.assertTrue(geom1.addGeometry(p1))
        self.assertTrue(geom1.addGeometry(p2))

        p1 = QgsPolygon()
        p1.setExteriorRing(QgsLineString(
            [QgsPoint(5.0, 5.0, m=5.0), QgsPoint(6.0, 5.0, m=5.0), QgsPoint(6.0, 6.0, m=5.0),
             QgsPoint(5.0, 5.0, m=5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(QgsLineString(
            [QgsPoint(0.0, 0.0, m=0.0), QgsPoint(0.001, 0.001, m=0.002), QgsPoint(0.003, 0.003, m=0.003),
             QgsPoint(0.0, 0.0, m=0.0)]))
        self.assertTrue(geom2.addGeometry(p1))
        self.assertTrue(geom2.addGeometry(p2))

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
        geom1 = QgsMultiPolygon()
        geom2 = QgsMultiPolygon()

        p1 = QgsPolygon()
        p1.setExteriorRing(QgsLineString(
            [QgsPoint(5.0, 5.0, 5.0, 5.0), QgsPoint(6.0, 5.0, 5.0, 5.0), QgsPoint(6.0, 6.0, 5.0, 5.0),
             QgsPoint(5.0, 5.0, 5.0, 5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(QgsLineString(
            [QgsPoint(0.0, 0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001, 0.001), QgsPoint(0.003, 0.003, 0.003, 0.003),
             QgsPoint(0.0, 0.0, 0.0, 0.0)]))
        self.assertTrue(geom1.addGeometry(p1))
        self.assertTrue(geom1.addGeometry(p2))

        p1 = QgsPolygon()
        p1.setExteriorRing(QgsLineString(
            [QgsPoint(5.0, 5.0, 5.0, 5.0), QgsPoint(6.0, 5.0, 5.0, 5.0), QgsPoint(6.0, 6.0, 5.0, 5.0),
             QgsPoint(5.0, 5.0, 5.0, 5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(QgsLineString(
            [QgsPoint(0.0, 0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.002, 0.002), QgsPoint(0.003, 0.003, 0.003, 0.003),
             QgsPoint(0.0, 0.0, 0.0, 0.0)]))
        self.assertTrue(geom2.addGeometry(p1))
        self.assertTrue(geom2.addGeometry(p2))

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))


if __name__ == '__main__':
    unittest.main()
