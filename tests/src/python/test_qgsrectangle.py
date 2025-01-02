"""QGIS Unit tests for QgsRectangle.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Tim Sutton"
__date__ = "20/08/2012"
__copyright__ = "Copyright 2012, The QGIS Project"

from qgis.core import QgsPointXY, QgsRectangle, QgsVector
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import compareWkt

start_app()


class TestQgsRectangle(QgisTestCase):

    def testCtor(self):
        rect = QgsRectangle(5.0, 5.0, 10.0, 10.0)
        self.assertEqual(rect.xMinimum(), 5.0)
        self.assertEqual(rect.yMinimum(), 5.0)
        self.assertEqual(rect.xMaximum(), 10.0)
        self.assertEqual(rect.yMaximum(), 10.0)

    def testDimensions(self):
        rect = QgsRectangle(0.0, 0.0, 10.0, 20.0)
        self.assertEqual(rect.width(), 10.0)
        self.assertEqual(rect.height(), 20.0)
        self.assertEqual(rect.center(), QgsPointXY(5.0, 10.0))

        rect.scale(2.0)
        self.assertEqual(rect.width(), 20.0)
        self.assertEqual(rect.height(), 40.0)

    def testCalculations(self):
        rect = QgsRectangle(0.0, 1.0, 20.0, 10.0)
        self.assertEqual(rect.area(), 180.0)
        self.assertEqual(rect.perimeter(), 58.0)

    def testIntersection(self):
        rect1 = QgsRectangle()
        rect2 = QgsRectangle()

        # both rectangle are null, they do not intersect
        self.assertTrue(rect1.isNull())
        self.assertTrue(rect2.isNull())
        self.assertFalse(rect2.intersects(rect1))

        rect1 = QgsRectangle(0.0, 0.0, 5.0, 5.0)
        rect2 = QgsRectangle(2.0, 2.0, 7.0, 7.0)

        self.assertTrue(rect1.intersects(rect2))

        rect3 = rect1.intersect(rect2)
        self.assertFalse(rect3.isEmpty(), "Empty rectangle returned")
        self.assertEqual(rect3.width(), 3.0)
        self.assertEqual(rect3.height(), 3.0)

    def testContains(self):
        rect1 = QgsRectangle(0.0, 0.0, 5.0, 5.0)
        rect2 = QgsRectangle(2.0, 2.0, 7.0, 7.0)
        pnt1 = QgsPointXY(4.0, 4.0)
        pnt2 = QgsPointXY(6.0, 2.0)

        rect3 = rect1.intersect(rect2)
        self.assertTrue(rect1.contains(rect3))
        self.assertTrue(rect2.contains(rect3))

        # test for point
        self.assertTrue(rect1.contains(pnt1))
        self.assertTrue(rect1.contains(pnt1.x(), pnt1.y()))
        self.assertTrue(rect2.contains(pnt1))
        self.assertTrue(rect2.contains(pnt1.x(), pnt1.y()))
        self.assertTrue(rect3.contains(pnt1))
        self.assertTrue(rect3.contains(pnt1.x(), pnt1.y()))
        self.assertFalse(rect1.contains(pnt2))
        self.assertFalse(rect1.contains(pnt2.x(), pnt2.y()))
        self.assertTrue(rect2.contains(pnt2))
        self.assertTrue(rect2.contains(pnt2.x(), pnt2.y()))
        self.assertFalse(rect3.contains(pnt2))
        self.assertFalse(rect3.contains(pnt2.x(), pnt2.y()))
        self.assertTrue(rect3.contains(pnt1))
        self.assertTrue(rect3.contains(pnt1.x(), pnt1.y()))

    def testUnion(self):
        rect1 = QgsRectangle(0.0, 0.0, 5.0, 5.0)
        rect2 = QgsRectangle(2.0, 2.0, 7.0, 7.0)
        pnt1 = QgsPointXY(6.0, 2.0)

        rect1.combineExtentWith(rect2)
        self.assertTrue(rect1.contains(rect2))

        self.assertEqual(rect1, QgsRectangle(0.0, 0.0, 7.0, 7.0))

        rect1 = QgsRectangle(0.0, 0.0, 5.0, 5.0)
        rect1.combineExtentWith(6.0, 2.0)
        self.assertTrue(rect1.contains(pnt1))

        self.assertEqual(rect1.toString(), QgsRectangle(0.0, 0.0, 6.0, 5.0).toString())

        rect1 = QgsRectangle(0.0, 0.0, 5.0, 5.0)
        rect1.combineExtentWith(rect2)
        self.assertTrue(rect1.contains(rect2))

        self.assertEqual(rect1, QgsRectangle(0.0, 0.0, 7.0, 7.0))

    def testAsWktCoordinates(self):
        """Test that we can get a proper wkt representation fo the rect"""
        rect1 = QgsRectangle(0.0, 0.0, 5.0, 5.0)
        myExpectedWkt = "0 0, " "5 5"
        myWkt = rect1.asWktCoordinates()
        myMessage = f"Expected: {myExpectedWkt}\nGot: {myWkt}\n"
        self.assertTrue(compareWkt(myWkt, myExpectedWkt), myMessage)

    def testAsWktPolygon(self):
        """Test that we can get a proper rect wkt polygon representation for rect"""
        rect1 = QgsRectangle(0.0, 0.0, 5.0, 5.0)
        myExpectedWkt = "Polygon ((0 0, " "5 0, " "5 5, " "0 5, " "0 0))"
        myWkt = rect1.asWktPolygon()
        myMessage = f"Expected: {myExpectedWkt}\nGot: {myWkt}\n"
        self.assertTrue(compareWkt(myWkt, myExpectedWkt), myMessage)

    def testToString(self):
        """Test the different string representations"""
        self.assertEqual(QgsRectangle().toString(), "Null")
        rect = QgsRectangle(0, 0.1, 0.2, 0.3)
        self.assertEqual(
            rect.toString(),
            "0.0000000000000000,0.1000000000000000 : 0.2000000000000000,0.3000000000000000",
        )

        # can't test the actual result here, because floating point inaccuracies mean the result is unpredictable
        # at this precision
        self.assertEqual(len(rect.toString(20)), 93)

        self.assertEqual(rect.toString(0), "0,0 : 0,0")
        self.assertEqual(rect.toString(2), "0.00,0.10 : 0.20,0.30")
        self.assertEqual(rect.toString(1), "0.0,0.1 : 0.2,0.3")
        self.assertEqual(rect.toString(-1), "0.00,0.10 : 0.20,0.30")

        rect = QgsRectangle(5000000.01111, -0.3, 5000000.44111, 99.8)
        self.assertEqual(rect.toString(-1), "5000000.01,-0.30 : 5000000.44,99.80")

    def testAsPolygon(self):
        """Test string representation as polygon"""
        self.assertEqual(QgsRectangle().asPolygon(), "EMPTY")
        self.assertEqual(
            QgsRectangle(0, 0.1, 0.2, 0.3).asPolygon(),
            "0.00000000 0.10000000, 0.00000000 0.30000000, 0.20000000 0.30000000, 0.20000000 0.10000000, 0.00000000 0.10000000",
        )

    def testToBox3d(self):
        rect = QgsRectangle(0, 0.1, 0.2, 0.3)
        box = rect.toBox3d(0.4, 0.5)
        self.assertEqual(box.xMinimum(), 0.0)
        self.assertEqual(box.yMinimum(), 0.1)
        self.assertEqual(box.zMinimum(), 0.4)
        self.assertEqual(box.xMaximum(), 0.2)
        self.assertEqual(box.yMaximum(), 0.3)
        self.assertEqual(box.zMaximum(), 0.5)

    def testOperators(self):
        rect1 = QgsRectangle(10, 20, 40, 40)
        rect2 = rect1 + QgsVector(3, 5.5)
        self.assertEqual(rect2, QgsRectangle(13, 25.5, 43, 45.5))

        # Subtracting the center point, so it becomes zero.
        rect1 -= rect1.center() - QgsPointXY(0, 0)
        self.assertEqual(rect1.center(), QgsPointXY(0, 0))

    def testInvert(self):
        rect = QgsRectangle(0, 0.1, 0.2, 0.3)
        rect.invert()
        self.assertEqual(rect.xMinimum(), 0.1)
        self.assertEqual(rect.yMinimum(), 0)
        self.assertEqual(rect.xMaximum(), 0.3)
        self.assertEqual(rect.yMaximum(), 0.2)


if __name__ == "__main__":
    unittest.main()
