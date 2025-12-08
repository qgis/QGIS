"""
***************************************************************************
    test_qgsvector3d.py
    ---------------------
    Date                 : December 2025
    Copyright            : (C) 2025 by Jean Felder
    Email                : jean dot felder at oslandia dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Jean Felder"
__date__ = "December 2025"
__copyright__ = "(C) 2025, Jean Felder"

from qgis.core import QgsVector3D
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsVector3D(QgisTestCase):

    def testCtor(self):
        vector = QgsVector3D()
        self.assertEqual(vector.x(), 0.0)
        self.assertEqual(vector.y(), 0.0)
        self.assertEqual(vector.z(), 0.0)

        vector = QgsVector3D(3, 4, 5)
        self.assertEqual(vector.x(), 3.0)
        self.assertEqual(vector.y(), 4.0)
        self.assertEqual(vector.z(), 5.0)

    def testSetter(self):
        vector = QgsVector3D()
        self.assertEqual(vector.x(), 0.0)
        self.assertEqual(vector.y(), 0.0)
        self.assertEqual(vector.z(), 0.0)

        vector.setX(-7.0)
        self.assertEqual(vector.x(), -7.0)

        vector.setY(2.0)
        self.assertEqual(vector.y(), 2.0)

        vector.setZ(-1.0)
        self.assertEqual(vector.z(), -1.0)

        vector.set(1.0, 2.0, 3.0)
        self.assertEqual(vector.x(), 1.0)
        self.assertEqual(vector.y(), 2.0)
        self.assertEqual(vector.z(), 3.0)

    def testEquality(self):
        vector1 = QgsVector3D(4, 6, 7)
        vector2 = QgsVector3D()
        self.assertFalse(vector1 == vector2)
        self.assertTrue(vector1 != vector2)

        vector2.setX(4)
        self.assertFalse(vector1 == vector2)
        self.assertTrue(vector1 != vector2)

        vector2.setY(6)
        self.assertFalse(vector1 == vector2)
        self.assertTrue(vector1 != vector2)

        vector2.setZ(7)
        self.assertTrue(vector1 == vector2)
        self.assertFalse(vector1 != vector2)

    def testOperators(self):
        vector1 = QgsVector3D(10, 20, 40)
        vector2 = QgsVector3D(3, 5.1, -4.2)
        vector3 = vector1 + vector2
        self.assertEqual(vector3, QgsVector3D(13, 25.1, 35.8))

        vector3 = vector1 - vector2
        self.assertEqual(vector3, QgsVector3D(7, 14.9, 44.2))

        self.assertEqual(-vector1, QgsVector3D(-10, -20, -40))
        self.assertEqual(-vector2, QgsVector3D(-3, -5.1, 4.2))

        vector3 = vector2 * 3
        self.assertAlmostEqual(vector3.x(), 9.0)
        self.assertAlmostEqual(vector3.y(), 15.3)
        self.assertAlmostEqual(vector3.z(), -12.6)

        vector3 = vector2 / 3
        self.assertAlmostEqual(vector3.x(), 1.0)
        self.assertAlmostEqual(vector3.y(), 1.7)
        self.assertAlmostEqual(vector3.z(), -1.4)

    def testIsNull(self):
        vector = QgsVector3D()
        self.assertTrue(vector.isNull())

        vector = QgsVector3D(1, 0, 0)
        self.assertFalse(vector.isNull())

        vector = QgsVector3D(1, 0, 0)
        self.assertFalse(vector.isNull())

        vector = QgsVector3D(0, -2, 0)
        self.assertFalse(vector.isNull())

        vector = QgsVector3D(0, 0, 7.1)
        self.assertFalse(vector.isNull())

        vector = QgsVector3D(6, -2, 0)
        self.assertFalse(vector.isNull())

        vector = QgsVector3D(6, 7, 3)
        self.assertFalse(vector.isNull())

        vector = QgsVector3D(-3, 2, 7)
        self.assertFalse(vector.isNull())

    def testLength(self):
        vector = QgsVector3D()
        self.assertEqual(vector.length(), 0.0)
        self.assertEqual(vector.lengthSquared(), 0.0)

        vector = QgsVector3D(3, 0, 0)
        self.assertEqual(vector.length(), 3.0)
        self.assertEqual(vector.lengthSquared(), 9.0)

        vector = QgsVector3D(0, -2, 0)
        self.assertEqual(vector.length(), 2.0)
        self.assertEqual(vector.lengthSquared(), 4.0)

        vector = QgsVector3D(0, 0, -3.1)
        self.assertEqual(vector.length(), 3.1)
        self.assertAlmostEqual(vector.lengthSquared(), 9.61)

        vector = QgsVector3D(2, -3.4, -3.1)
        self.assertAlmostEqual(vector.length(), 5.0169711978)
        self.assertAlmostEqual(vector.lengthSquared(), 25.17)


if __name__ == "__main__":
    unittest.main()
