# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsBox3d.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '11/04/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsBox3d,
                       QgsPointXY,
                       QgsPoint,
                       QgsWkbTypes,
                       QgsRectangle)

from qgis.testing import unittest


class TestQgsBox3d(unittest.TestCase):

    def testCtor(self):
        box = QgsBox3d(5.0, 6.0, 7.0, 10.0, 11.0, 12.0)

        self.assertEqual(box.xMinimum(), 5.0)
        self.assertEqual(box.yMinimum(), 6.0)
        self.assertEqual(box.zMinimum(), 7.0)
        self.assertEqual(box.xMaximum(), 10.0)
        self.assertEqual(box.yMaximum(), 11.0)
        self.assertEqual(box.zMaximum(), 12.0)

        box = QgsBox3d(QgsPoint(5, 6, 7), QgsPoint(10, 11, 12))
        self.assertEqual(box.xMinimum(), 5.0)
        self.assertEqual(box.yMinimum(), 6.0)
        self.assertEqual(box.zMinimum(), 7.0)
        self.assertEqual(box.xMaximum(), 10.0)
        self.assertEqual(box.yMaximum(), 11.0)
        self.assertEqual(box.zMaximum(), 12.0)

        # point constructor should normalize
        box = QgsBox3d(QgsPoint(10, 11, 12), QgsPoint(5, 6, 7))
        self.assertEqual(box.xMinimum(), 5.0)
        self.assertEqual(box.yMinimum(), 6.0)
        self.assertEqual(box.zMinimum(), 7.0)
        self.assertEqual(box.xMaximum(), 10.0)
        self.assertEqual(box.yMaximum(), 11.0)
        self.assertEqual(box.zMaximum(), 12.0)

        box = QgsBox3d(QgsRectangle(5, 6, 11, 13))
        self.assertEqual(box.xMinimum(), 5.0)
        self.assertEqual(box.yMinimum(), 6.0)
        self.assertEqual(box.zMinimum(), 0.0)
        self.assertEqual(box.xMaximum(), 11.0)
        self.assertEqual(box.yMaximum(), 13.0)
        self.assertEqual(box.zMaximum(), 0.0)

    def testSetters(self):
        box = QgsBox3d(5.0, 6.0, 7.0, 10.0, 11.0, 12.0)

        box.setXMinimum(35.0)
        box.setYMinimum(36.0)
        box.setZMinimum(37.0)
        box.setXMaximum(40.0)
        box.setYMaximum(41.0)
        box.setZMaximum(42.0)
        self.assertEqual(box.xMinimum(), 35.0)
        self.assertEqual(box.yMinimum(), 36.0)
        self.assertEqual(box.zMinimum(), 37.0)
        self.assertEqual(box.xMaximum(), 40.0)
        self.assertEqual(box.yMaximum(), 41.0)
        self.assertEqual(box.zMaximum(), 42.0)

    def testNormalize(self):
        box = QgsBox3d()
        box.setXMinimum(10.0)
        box.setYMinimum(11.0)
        box.setZMinimum(12.0)
        box.setXMaximum(5.0)
        box.setYMaximum(6.0)
        box.setZMaximum(7.0)

        box.normalize()
        self.assertEqual(box.xMinimum(), 5.0)
        self.assertEqual(box.yMinimum(), 6.0)
        self.assertEqual(box.zMinimum(), 7.0)
        self.assertEqual(box.xMaximum(), 10.0)
        self.assertEqual(box.yMaximum(), 11.0)
        self.assertEqual(box.zMaximum(), 12.0)

    def testDimensions(self):
        box = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 15.0)
        self.assertEqual(box.width(), 6.0)
        self.assertEqual(box.height(), 7.0)
        self.assertEqual(box.depth(), 8.0)

    def testIntersect(self):
        box = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 15.0)
        box2 = box.intersect(QgsBox3d(7.0, 8.0, 9.0, 10.0, 11.0, 12.0))
        self.assertEqual(box2.xMinimum(), 7.0)
        self.assertEqual(box2.yMinimum(), 8.0)
        self.assertEqual(box2.zMinimum(), 9.0)
        self.assertEqual(box2.xMaximum(), 10.0)
        self.assertEqual(box2.yMaximum(), 11.0)
        self.assertEqual(box2.zMaximum(), 12.0)
        box2 = box.intersect(QgsBox3d(0.0, 1.0, 2.0, 100.0, 111.0, 112.0))
        self.assertEqual(box2.xMinimum(), 5.0)
        self.assertEqual(box2.yMinimum(), 6.0)
        self.assertEqual(box2.zMinimum(), 7.0)
        self.assertEqual(box2.xMaximum(), 11.0)
        self.assertEqual(box2.yMaximum(), 13.0)
        self.assertEqual(box2.zMaximum(), 15.0)
        box2 = box.intersect(QgsBox3d(1.0, 2.0, 3.0, 6.0, 7.0, 8.0))
        self.assertEqual(box2.xMinimum(), 5.0)
        self.assertEqual(box2.yMinimum(), 6.0)
        self.assertEqual(box2.zMinimum(), 7.0)
        self.assertEqual(box2.xMaximum(), 6.0)
        self.assertEqual(box2.yMaximum(), 7.0)
        self.assertEqual(box2.zMaximum(), 8.0)

    def testIntersects(self):
        box = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 15.0)
        self.assertTrue(box.intersects(QgsBox3d(7.0, 8.0, 9.0, 10.0, 11.0, 12.0)))
        self.assertTrue(box.intersects(QgsBox3d(0.0, 1.0, 2.0, 100.0, 111.0, 112.0)))
        self.assertTrue(box.intersects(QgsBox3d(1.0, 2.0, 3.0, 6.0, 7.0, 8.0)))
        self.assertFalse(box.intersects(QgsBox3d(15.0, 16.0, 17.0, 110.0, 112.0, 113.0)))
        self.assertFalse(box.intersects(QgsBox3d(5.0, 6.0, 17.0, 11.0, 13.0, 113.0)))
        self.assertFalse(box.intersects(QgsBox3d(5.0, 16.0, 7.0, 11.0, 23.0, 15.0)))
        self.assertFalse(box.intersects(QgsBox3d(15.0, 6.0, 7.0, 21.0, 13.0, 15.0)))

    def testContains(self):
        box = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 15.0)
        self.assertTrue(box.contains(QgsBox3d(7.0, 8.0, 9.0, 10.0, 11.0, 12.0)))
        self.assertFalse(box.contains(QgsBox3d(0.0, 1.0, 2.0, 100.0, 111.0, 112.0)))
        self.assertFalse(box.contains(QgsBox3d(1.0, 2.0, 3.0, 6.0, 7.0, 8.0)))
        self.assertFalse(box.contains(QgsBox3d(15.0, 16.0, 17.0, 110.0, 112.0, 113.0)))
        self.assertFalse(box.contains(QgsBox3d(5.0, 6.0, 17.0, 11.0, 13.0, 113.0)))
        self.assertFalse(box.contains(QgsBox3d(5.0, 16.0, 7.0, 11.0, 23.0, 15.0)))
        self.assertFalse(box.contains(QgsBox3d(15.0, 6.0, 7.0, 21.0, 13.0, 15.0)))

    def testContainsPoint(self):
        box = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 15.0)
        self.assertTrue(box.contains(QgsPoint(6, 7, 8)))
        self.assertFalse(box.contains(QgsPoint(16, 7, 8)))
        self.assertFalse(box.contains(QgsPoint(6, 17, 8)))
        self.assertFalse(box.contains(QgsPoint(6, 7, 18)))
        # 2d containment
        self.assertTrue(box.contains(QgsPoint(6, 7)))
        self.assertFalse(box.contains(QgsPoint(16, 7)))
        self.assertFalse(box.contains(QgsPoint(6, 17)))

    def testVolume(self):
        box = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 15.0)
        self.assertEqual(box.volume(), 336.0)

    def testToRectangle(self):
        box = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 15.0)
        rect = box.toRectangle()
        self.assertEqual(rect, QgsRectangle(5, 6, 11, 13))

    def testIs2d(self):
        box = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 15.0)
        self.assertFalse(box.is2d())
        box = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 7.0)
        self.assertTrue(box.is2d())
        box = QgsBox3d(5.0, 6.0, 0.0, 11.0, 13.0, 0.0)
        self.assertTrue(box.is2d())
        box = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, -7.0)
        self.assertTrue(box.is2d())

    def testEquality(self):
        box1 = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 15.0)
        box2 = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 15.0)
        self.assertEqual(box1, box2)
        self.assertNotEqual(box1, QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 14.0))
        self.assertNotEqual(box1, QgsBox3d(5.0, 6.0, 7.0, 11.0, 41.0, 15.0))
        self.assertNotEqual(box1, QgsBox3d(5.0, 6.0, 7.0, 12.0, 13.0, 15.0))
        self.assertNotEqual(box1, QgsBox3d(5.0, 6.0, 17.0, 11.0, 13.0, 15.0))
        self.assertNotEqual(box1, QgsBox3d(5.0, 16.0, 7.0, 11.0, 13.0, 15.0))
        self.assertNotEqual(box1, QgsBox3d(52.0, 6.0, 7.0, 11.0, 13.0, 15.0))

    def testScaling(self):
        box1 = QgsBox3d(-1, -1, -1, 1, 1, 1)
        box1.scale(3.0)
        self.assertEqual(box1.width(), 6.0)
        self.assertEqual(box1.height(), 6.0)
        self.assertEqual(box1.depth(), 6.0)
        self.assertEqual(box1.xMinimum(), -3.0)
        self.assertEqual(box1.yMinimum(), -3.0)
        self.assertEqual(box1.zMinimum(), -3.0)

        box2 = QgsBox3d(-1, -1, -1, 1, 1, 1)
        box2.scale(3.0, QgsPoint(-1.0, -1.0, -1.0))
        self.assertEqual(box2.width(), 6.0)
        self.assertEqual(box2.height(), 6.0)
        self.assertEqual(box2.depth(), 6.0)
        self.assertEqual(box2.xMinimum(), -1.0)
        self.assertEqual(box2.yMinimum(), -1.0)
        self.assertEqual(box2.zMinimum(), -1.0)

        box3 = QgsBox3d(-1, -1, -1, 1, 1, 1)
        box3.scale(3.0, QgsPoint(-2.0, 2.0, 0.0))
        self.assertEqual(box3.width(), 6.0)
        self.assertEqual(box3.height(), 6.0)
        self.assertEqual(box3.depth(), 6.0)
        self.assertEqual(box3.xMinimum(), 1.0)
        self.assertEqual(box3.yMinimum(), -7.0)
        self.assertEqual(box3.zMinimum(), -3.0)


if __name__ == '__main__':
    unittest.main()
