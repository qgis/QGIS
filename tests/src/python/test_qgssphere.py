"""QGIS Unit tests for QgsSphere

From build dir, run: ctest -R QgsSphere -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2023 by Nyall Dawson"
__date__ = "14/07/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import math
from qgis.core import QgsSphere, QgsPoint, QgsCircle, QgsVector3D
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSphere(QgisTestCase):

    def test_null(self):
        sphere = QgsSphere()
        self.assertTrue(sphere.isNull())
        self.assertEqual(str(sphere), "<QgsSphere: null>")
        # a null sphere should also be considered empty
        self.assertTrue(sphere.isEmpty())

    def test_sphere(self):
        sphere = QgsSphere(1, 2, 3, 4)
        self.assertFalse(sphere.isNull())
        self.assertEqual(sphere.centerX(), 1)
        self.assertEqual(sphere.centerY(), 2)
        self.assertEqual(sphere.centerZ(), 3)
        self.assertEqual(sphere.center(), QgsPoint(1, 2, 3))
        self.assertEqual(sphere.centerVector(), QgsVector3D(1, 2, 3))
        self.assertEqual(sphere.radius(), 4)
        self.assertEqual(str(sphere), "<QgsSphere: (1, 2, 3) radius 4>")

    def test_setters(self):
        sphere = QgsSphere(1, 2, 3, 4)
        sphere.setCenter(11, 12, 13)
        self.assertEqual(sphere.centerX(), 11)
        self.assertEqual(sphere.centerY(), 12)
        self.assertEqual(sphere.centerZ(), 13)
        self.assertEqual(sphere.radius(), 4)
        sphere.setCenter(QgsPoint(21, 22, 23))
        self.assertEqual(sphere.centerX(), 21)
        self.assertEqual(sphere.centerY(), 22)
        self.assertEqual(sphere.centerZ(), 23)
        self.assertEqual(sphere.radius(), 4)
        sphere.setRadius(5)
        self.assertEqual(sphere.centerX(), 21)
        self.assertEqual(sphere.centerY(), 22)
        self.assertEqual(sphere.centerZ(), 23)
        self.assertEqual(sphere.radius(), 5)

    def test_empty(self):
        sphere = QgsSphere(1, 2, 3, 4)
        self.assertFalse(sphere.isEmpty())
        sphere = QgsSphere(1, 2, 3, 0)
        self.assertTrue(sphere.isEmpty())

    def test_equality(self):
        self.assertEqual(QgsSphere(), QgsSphere())
        self.assertNotEqual(QgsSphere(1, 2, 3, 4), QgsSphere())
        self.assertNotEqual(QgsSphere(), QgsSphere(1, 2, 3, 4))
        self.assertEqual(QgsSphere(1, 2, 3, 4), QgsSphere(1, 2, 3, 4))
        self.assertNotEqual(QgsSphere(1, 2, 3, 4), QgsSphere(11, 2, 3, 4))
        self.assertNotEqual(QgsSphere(1, 2, 3, 4), QgsSphere(1, 12, 3, 4))
        self.assertNotEqual(QgsSphere(1, 2, 3, 4), QgsSphere(1, 2, 13, 4))
        self.assertNotEqual(QgsSphere(1, 2, 3, 4), QgsSphere(1, 2, 3, 14))

    def test_volume(self):
        self.assertEqual(QgsSphere(1, 1, 1, 3).volume(), 113.09733552923254)
        self.assertEqual(QgsSphere(1, 1, 1, 0).volume(), 0)

    def test_surface_area(self):
        self.assertEqual(QgsSphere(1, 1, 1, 7).surfaceArea(), 615.7521601035994)
        self.assertEqual(QgsSphere(1, 1, 1, 0).surfaceArea(), 0)

    def test_to_circle(self):
        circle = QgsSphere().toCircle()
        self.assertEqual(circle, QgsCircle())
        circle = QgsSphere(1, 2, 3, 4).toCircle()
        self.assertEqual(circle, QgsCircle(QgsPoint(1, 2), 4))

    def test_bounding_box(self):
        box = QgsSphere().boundingBox()
        self.assertTrue(box.isNull())
        box = QgsSphere(1, 2, 3, 4).boundingBox()
        self.assertEqual(box.xMinimum(), -3)
        self.assertEqual(box.yMinimum(), -2)
        self.assertEqual(box.zMinimum(), -1)
        self.assertEqual(box.xMaximum(), 5)
        self.assertEqual(box.yMaximum(), 6)
        self.assertEqual(box.zMaximum(), 7)


if __name__ == "__main__":
    unittest.main()
