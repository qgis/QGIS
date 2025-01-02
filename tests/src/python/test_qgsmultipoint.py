"""QGIS Unit tests for QgsMultiPoint.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Loïc Bartoletti"
__date__ = "19/12/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import qgis  # NOQA

from qgis.core import QgsMultiPoint, QgsPoint, QgsRectangle, QgsLineString
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsMultiPoint(QgisTestCase):

    def testFuzzyComparisons(self):
        ######
        # 2D #
        ######
        epsilon = 0.001
        geom1 = QgsMultiPoint()
        self.assertTrue(geom1.addGeometry(QgsPoint(0.0, 0.0)))
        self.assertTrue(geom1.addGeometry(QgsPoint(0.001, 0.001)))
        geom2 = QgsMultiPoint()
        self.assertTrue(geom2.addGeometry(QgsPoint(0.0, 0.0)))
        self.assertTrue(geom2.addGeometry(QgsPoint(0.002, 0.002)))

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
        geom1 = QgsMultiPoint()
        self.assertTrue(geom1.addGeometry(QgsPoint(0.0, 0.0, 0.0)))
        self.assertTrue(geom1.addGeometry(QgsPoint(0.001, 0.001, 0.001)))
        geom2 = QgsMultiPoint()
        self.assertTrue(geom2.addGeometry(QgsPoint(0.0, 0.0, 0.0)))
        self.assertTrue(geom2.addGeometry(QgsPoint(0.001, 0.001, 0.002)))

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
        geom1 = QgsMultiPoint()
        self.assertTrue(geom1.addGeometry(QgsPoint(0.0, 0.0, m=0.0)))
        self.assertTrue(geom1.addGeometry(QgsPoint(0.001, 0.001, m=0.001)))
        geom2 = QgsMultiPoint()
        self.assertTrue(geom2.addGeometry(QgsPoint(0.0, 0.0, m=0.0)))
        self.assertTrue(geom2.addGeometry(QgsPoint(0.001, 0.001, m=0.002)))

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
        geom1 = QgsMultiPoint()
        self.assertTrue(geom1.addGeometry(QgsPoint(0.0, 0.0, 0.0, 0.0)))
        self.assertTrue(geom1.addGeometry(QgsPoint(0.001, 0.001, 0.001, 0.001)))
        geom2 = QgsMultiPoint()
        self.assertTrue(geom2.addGeometry(QgsPoint(0.0, 0.0, 0.0, 0.0)))
        self.assertTrue(geom2.addGeometry(QgsPoint(0.001, 0.001, 0.002, 0.002)))

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

    def test_add_geometries(self):
        """
        Test adding multiple geometries
        """
        # empty collection
        collection = QgsMultiPoint()
        self.assertTrue(collection.addGeometries([]))
        self.assertEqual(collection.asWkt(), "MultiPoint EMPTY")
        self.assertEqual(collection.boundingBox(), QgsRectangle())

        self.assertTrue(
            collection.addGeometries([QgsPoint(1, 2, 3), QgsPoint(11, 22, 33)])
        )
        self.assertEqual(collection.asWkt(), "MultiPoint Z ((1 2 3),(11 22 33))")
        self.assertEqual(collection.boundingBox(), QgsRectangle(1, 2, 11, 22))

        # can't add non-points
        self.assertFalse(
            collection.addGeometries([QgsLineString([[100, 200], [200, 200]])])
        )
        self.assertEqual(collection.asWkt(), "MultiPoint Z ((1 2 3),(11 22 33))")
        self.assertEqual(collection.boundingBox(), QgsRectangle(1, 2, 11, 22))

        self.assertTrue(collection.addGeometries([QgsPoint(100, 2, 3)]))
        self.assertEqual(
            collection.asWkt(), "MultiPoint Z ((1 2 3),(11 22 33),(100 2 3))"
        )
        self.assertEqual(collection.boundingBox(), QgsRectangle(1, 2, 100, 22))

    def test_simplify_by_distance(self):
        """
        test simplifyByDistance
        """
        p = QgsMultiPoint()
        p.fromWkt("MultiPoint( 0 0, 50 0, 70 0, 80 0, 100 0)")
        # this is just a clone
        self.assertEqual(
            p.simplifyByDistance(10).asWkt(),
            "MultiPoint ((0 0),(50 0),(70 0),(80 0),(100 0))",
        )


if __name__ == "__main__":
    unittest.main()
