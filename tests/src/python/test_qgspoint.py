"""QGIS Unit tests for QgsPoint.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Tim Sutton"
__date__ = "20/08/2012"
__copyright__ = "Copyright 2012, The QGIS Project"

from qgis.PyQt.QtCore import QPointF
from qgis.core import QgsPoint, QgsPointXY, QgsWkbTypes
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsPointXY(QgisTestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        QgisTestCase.__init__(self, methodName)

    def setUp(self):
        self.mPoint = QgsPointXY(10.0, 10.0)

    def test_Point(self):
        myExpectedValue = 10.0
        myActualValue = self.mPoint.x()
        self.assertEqual(myExpectedValue, myActualValue)

    def test_pointToString(self):
        myExpectedValue = "10, 10"
        myActualValue = self.mPoint.toString()
        self.assertEqual(myExpectedValue, myActualValue)

    def test_hash(self):
        a = QgsPointXY(2.0, 1.0)
        b = QgsPointXY(2.0, 2.0)
        c = QgsPointXY(1.0, 2.0)
        d = QgsPointXY(1.0, 1.0)
        e = QgsPointXY(2.0, 1.0)
        self.assertNotEqual(a.__hash__(), b.__hash__())
        self.assertEqual(e.__hash__(), a.__hash__())

        mySet = {a, b, c, d, e}
        self.assertEqual(len(mySet), 4)

    def test_issue_32443(self):
        p = QgsPoint()
        self.assertTrue(
            p.wkbType() == QgsWkbTypes.Type.Point and p.x() != p.x() and p.y() != p.y()
        )

        # ctor from QgsPointXY should be available
        p = QgsPoint(QgsPointXY(1, 2))
        self.assertTrue(
            p.wkbType() == QgsWkbTypes.Type.Point and p.x() == 1 and p.y() == 2
        )

        # ctor from QPointF should be available
        p = QgsPoint(QPointF(1, 2))
        self.assertTrue(
            p.wkbType() == QgsWkbTypes.Type.Point and p.x() == 1 and p.y() == 2
        )

        p = QgsPoint(1, 2)
        self.assertTrue(
            p.wkbType() == QgsWkbTypes.Type.Point and p.x() == 1 and p.y() == 2
        )

        p = QgsPoint(1, 2, 3)
        self.assertTrue(
            p.wkbType() == QgsWkbTypes.Type.PointZ
            and p.x() == 1
            and p.y() == 2
            and p.z() == 3
        )

        p = QgsPoint(1, 2, z=3)
        self.assertTrue(
            p.wkbType() == QgsWkbTypes.Type.PointZ
            and p.x() == 1
            and p.y() == 2
            and p.z() == 3
        )

        p = QgsPoint(1, 2, m=3)
        self.assertTrue(
            p.wkbType() == QgsWkbTypes.Type.PointM
            and p.x() == 1
            and p.y() == 2
            and p.m() == 3
        )

        p = QgsPoint(1, 2, wkbType=QgsWkbTypes.Type.PointM)
        self.assertTrue(
            p.wkbType() == QgsWkbTypes.Type.PointM
            and p.x() == 1
            and p.y() == 2
            and p.m() != p.m()
        )

        p = QgsPoint(1, 2, 3, 4)
        self.assertTrue(
            p.wkbType() == QgsWkbTypes.Type.PointZM
            and p.x() == 1
            and p.y() == 2
            and p.z() == 3
            and p.m() == 4
        )

        p = QgsPoint(1, 2, m=4, z=3)
        self.assertTrue(
            p.wkbType() == QgsWkbTypes.Type.PointZM
            and p.x() == 1
            and p.y() == 2
            and p.z() == 3
            and p.m() == 4
        )

    def test_empty_QgsPointXY(self):
        p = QgsPoint(QgsPointXY())
        self.assertTrue(p.isEmpty())


class TestQgsPoint(QgisTestCase):

    def testInvalidConstructorArguments(self):
        """Test GH #34557"""

        with self.assertRaises(TypeError):
            point_0 = QgsPoint("a string")

        with self.assertRaises(TypeError):
            point_a = QgsPoint(10, 20)
            point_b = QgsPoint(point_a)

    def testFuzzyComparisons(self):
        ######
        # 2D #
        ######
        epsilon = 0.001
        geom1 = QgsPoint(0.001, 0.001)
        geom2 = QgsPoint(0.002, 0.002)

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
        geom1 = QgsPoint(0.001, 0.001, 0.001)
        geom2 = QgsPoint(0.001, 0.001, 0.002)

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
        geom1 = QgsPoint(0.001, 0.001, m=0.001)
        geom2 = QgsPoint(0.001, 0.001, m=0.002)

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
        geom1 = QgsPoint(0.001, 0.001, 0.001, 0.001)
        geom2 = QgsPoint(0.001, 0.001, 0.002, 0.002)

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
        # for points this is just a clone
        p = QgsPoint(1.1, 2.2)
        self.assertEqual(p.simplifyByDistance(0.5), QgsPoint(1.1, 2.2))
        p = QgsPoint(1.1, 2.2, 3.3, 4.4)
        self.assertEqual(p.simplifyByDistance(0.5), QgsPoint(1.1, 2.2, 3.3, 4.4))


if __name__ == "__main__":
    unittest.main()
