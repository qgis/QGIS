"""QGIS Unit tests for QgsGeometryCollection.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Loïc Bartoletti'
__date__ = '19/12/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsGeometryCollection, QgsPoint
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsGeometryCollection(QgisTestCase):

    def testFuzzyComparisons(self):
        ######
        # 2D #
        ######
        epsilon = 0.001
        geom1 = QgsGeometryCollection()
        assert geom1.addGeometry(QgsPoint(0.0, 0.0))
        assert geom1.addGeometry(QgsPoint(0.001, 0.001))
        geom2 = QgsGeometryCollection()
        assert geom2.addGeometry(QgsPoint(0.0, 0.0))
        assert geom2.addGeometry(QgsPoint(0.002, 0.002))

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

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
        geom1 = QgsGeometryCollection()
        assert geom1.addGeometry(QgsPoint(0.0, 0.0, 0.0))
        assert geom1.addGeometry(QgsPoint(0.001, 0.001, 0.001))
        geom2 = QgsGeometryCollection()
        assert geom2.addGeometry(QgsPoint(0.0, 0.0, 0.0))
        assert geom2.addGeometry(QgsPoint(0.001, 0.001, 0.002))

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

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
        geom1 = QgsGeometryCollection()
        assert geom1.addGeometry(QgsPoint(0.0, 0.0, m=0.0))
        assert geom1.addGeometry(QgsPoint(0.001, 0.001, m=0.001))
        geom2 = QgsGeometryCollection()
        assert geom2.addGeometry(QgsPoint(0.0, 0.0, m=0.0))
        assert geom2.addGeometry(QgsPoint(0.001, 0.001, m=0.002))

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

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
        geom1 = QgsGeometryCollection()
        assert geom1.addGeometry(QgsPoint(0.0, 0.0, 0.0, 0.0))
        assert geom1.addGeometry(QgsPoint(0.001, 0.001, 0.001, 0.001))
        geom2 = QgsGeometryCollection()
        assert geom2.addGeometry(QgsPoint(0.0, 0.0, 0.0, 0.0))
        assert geom2.addGeometry(QgsPoint(0.001, 0.001, 0.002, 0.002))

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        assert geom1.fuzzyEqual(geom2, epsilon)
        assert not geom1.fuzzyDistanceEqual(geom2, epsilon)

        # OK for both
        epsilon *= 10
        assert geom1.fuzzyEqual(geom2, epsilon)
        assert geom1.fuzzyDistanceEqual(geom2, epsilon)


if __name__ == '__main__':
    unittest.main()
