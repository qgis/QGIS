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

    def testFuzzyComparisons(self):
        ######
        # 2D #
        ######
        epsilon = 0.001
        geom1 = QgsMultiPolygon()
        geom2 = QgsMultiPolygon()

        p1 = QgsPolygon()
        p1.setExteriorRing(QgsLineString([QgsPoint(5.0, 5.0), QgsPoint(6.0, 5.0), QgsPoint(6.0, 6.0), QgsPoint(5.0, 5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(QgsLineString([QgsPoint(0.0, 0.0), QgsPoint(0.001, 0.001), QgsPoint(0.003, 0.003), QgsPoint(0.0, 0.0)]))
        assert geom1.addGeometry(p1)
        assert geom1.addGeometry(p2)

        p1 = QgsPolygon()
        p1.setExteriorRing(QgsLineString([QgsPoint(5.0, 5.0), QgsPoint(6.0, 5.0), QgsPoint(6.0, 6.0), QgsPoint(5.0, 5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(QgsLineString([QgsPoint(0.0, 0.0), QgsPoint(0.002, 0.002), QgsPoint(0.003, 0.003), QgsPoint(0.0, 0.0)]))
        assert geom2.addGeometry(p1)
        assert geom2.addGeometry(p2)

        assert not geom1 == geom2  # epsilon = 1e-8 here

        assert geom1.fuzzyEqual(geom2, epsilon)
        assert not geom1.fuzzyDistanceEqual(geom2, epsilon)

        # OK for both
        epsilon *= 10
        assert geom1.fuzzyEqual(geom2, epsilon)
        assert geom1.fuzzyDistanceEqual(geom2, epsilon)

        #######
        # 3DZ #
        #######
        epsilon = 0.001
        geom1 = QgsMultiPolygon()
        geom2 = QgsMultiPolygon()

        p1 = QgsPolygon()
        p1.setExteriorRing(QgsLineString([QgsPoint(5.0, 5.0, 5.0), QgsPoint(6.0, 5.0, 5.0), QgsPoint(6.0, 6.0, 5.0), QgsPoint(5.0, 5.0, 5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(QgsLineString([QgsPoint(0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001), QgsPoint(0.003, 0.003, 0.003), QgsPoint(0.0, 0.0, 0.0)]))
        assert geom1.addGeometry(p1)
        assert geom1.addGeometry(p2)

        p1 = QgsPolygon()
        p1.setExteriorRing(QgsLineString([QgsPoint(5.0, 5.0, 5.0), QgsPoint(6.0, 5.0, 5.0), QgsPoint(6.0, 6.0, 5.0), QgsPoint(5.0, 5.0, 5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(QgsLineString([QgsPoint(0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.002), QgsPoint(0.003, 0.003, 0.003), QgsPoint(0.0, 0.0, 0.0)]))
        assert geom2.addGeometry(p1)
        assert geom2.addGeometry(p2)

        assert not geom1 == geom2  # epsilon = 1e-8 here

        assert geom1.fuzzyEqual(geom2, epsilon)
        assert not geom1.fuzzyDistanceEqual(geom2, epsilon)

        # OK for both
        epsilon *= 10
        assert geom1.fuzzyEqual(geom2, epsilon)
        assert geom1.fuzzyDistanceEqual(geom2, epsilon)

        #######
        # 3DM #
        #######
        epsilon = 0.001
        geom1 = QgsMultiPolygon()
        geom2 = QgsMultiPolygon()

        p1 = QgsPolygon()
        p1.setExteriorRing(QgsLineString([QgsPoint(5.0, 5.0, m=5.0), QgsPoint(6.0, 5.0, m=5.0), QgsPoint(6.0, 6.0, m=5.0), QgsPoint(5.0, 5.0, m=5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(QgsLineString([QgsPoint(0.0, 0.0, m=0.0), QgsPoint(0.001, 0.001, m=0.001), QgsPoint(0.003, 0.003, m=0.003), QgsPoint(0.0, 0.0, m=0.0)]))
        assert geom1.addGeometry(p1)
        assert geom1.addGeometry(p2)

        p1 = QgsPolygon()
        p1.setExteriorRing(QgsLineString([QgsPoint(5.0, 5.0, m=5.0), QgsPoint(6.0, 5.0, m=5.0), QgsPoint(6.0, 6.0, m=5.0), QgsPoint(5.0, 5.0, m=5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(QgsLineString([QgsPoint(0.0, 0.0, m=0.0), QgsPoint(0.001, 0.001, m=0.002), QgsPoint(0.003, 0.003, m=0.003), QgsPoint(0.0, 0.0, m=0.0)]))
        assert geom2.addGeometry(p1)
        assert geom2.addGeometry(p2)

        assert not geom1 == geom2  # epsilon = 1e-8 here

        assert geom1.fuzzyEqual(geom2, epsilon)
        assert not geom1.fuzzyDistanceEqual(geom2, epsilon)

        # OK for both
        epsilon *= 10
        assert geom1.fuzzyEqual(geom2, epsilon)
        assert geom1.fuzzyDistanceEqual(geom2, epsilon)

        ######
        # 4D #
        ######
        epsilon = 0.001
        geom1 = QgsMultiPolygon()
        geom2 = QgsMultiPolygon()

        p1 = QgsPolygon()
        p1.setExteriorRing(QgsLineString([QgsPoint(5.0, 5.0, 5.0, 5.0), QgsPoint(6.0, 5.0, 5.0, 5.0), QgsPoint(6.0, 6.0, 5.0, 5.0), QgsPoint(5.0, 5.0, 5.0, 5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(QgsLineString([QgsPoint(0.0, 0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001, 0.001), QgsPoint(0.003, 0.003, 0.003, 0.003), QgsPoint(0.0, 0.0, 0.0, 0.0)]))
        assert geom1.addGeometry(p1)
        assert geom1.addGeometry(p2)

        p1 = QgsPolygon()
        p1.setExteriorRing(QgsLineString([QgsPoint(5.0, 5.0, 5.0, 5.0), QgsPoint(6.0, 5.0, 5.0, 5.0), QgsPoint(6.0, 6.0, 5.0, 5.0), QgsPoint(5.0, 5.0, 5.0, 5.0)]))
        p2 = QgsPolygon()
        p2.setExteriorRing(QgsLineString([QgsPoint(0.0, 0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.002, 0.002), QgsPoint(0.003, 0.003, 0.003, 0.003), QgsPoint(0.0, 0.0, 0.0, 0.0)]))
        assert geom2.addGeometry(p1)
        assert geom2.addGeometry(p2)

        assert not geom1 == geom2  # epsilon = 1e-8 here

        assert geom1.fuzzyEqual(geom2, epsilon)
        assert not geom1.fuzzyDistanceEqual(geom2, epsilon)

        # OK for both
        epsilon *= 10
        assert geom1.fuzzyEqual(geom2, epsilon)
        assert geom1.fuzzyDistanceEqual(geom2, epsilon)


if __name__ == '__main__':
    unittest.main()
