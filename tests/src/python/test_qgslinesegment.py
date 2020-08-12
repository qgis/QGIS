# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLineSegment2D.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '13/04/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsPointXY, QgsLineSegment2D)

from qgis.testing import start_app, unittest

start_app()


class TestQgsLineSegment2D(unittest.TestCase):

    def testConstruct(self):
        segment = QgsLineSegment2D(QgsPointXY(1, 2), QgsPointXY(3, 4))
        self.assertEqual(segment.start(), QgsPointXY(1, 2))
        self.assertEqual(segment.end(), QgsPointXY(3, 4))
        segment = QgsLineSegment2D(1, 2, 3, 4)
        self.assertEqual(segment.start(), QgsPointXY(1, 2))
        self.assertEqual(segment.end(), QgsPointXY(3, 4))

    def testGettersSetters(self):
        segment = QgsLineSegment2D(QgsPointXY(1, 2), QgsPointXY(3, 4))
        self.assertEqual(segment.start(), QgsPointXY(1, 2))
        self.assertEqual(segment.end(), QgsPointXY(3, 4))
        self.assertEqual(segment.startX(), 1)
        self.assertEqual(segment.startY(), 2)
        self.assertEqual(segment.endX(), 3)
        self.assertEqual(segment.endY(), 4)
        segment.setStartX(5)
        self.assertEqual(segment.start(), QgsPointXY(5, 2))
        self.assertEqual(segment.end(), QgsPointXY(3, 4))
        self.assertEqual(segment.startX(), 5)
        self.assertEqual(segment.startY(), 2)
        self.assertEqual(segment.endX(), 3)
        self.assertEqual(segment.endY(), 4)
        segment.setStartY(6)
        self.assertEqual(segment.start(), QgsPointXY(5, 6))
        self.assertEqual(segment.end(), QgsPointXY(3, 4))
        self.assertEqual(segment.startX(), 5)
        self.assertEqual(segment.startY(), 6)
        self.assertEqual(segment.endX(), 3)
        self.assertEqual(segment.endY(), 4)
        segment.setEndX(7)
        self.assertEqual(segment.start(), QgsPointXY(5, 6))
        self.assertEqual(segment.end(), QgsPointXY(7, 4))
        self.assertEqual(segment.startX(), 5)
        self.assertEqual(segment.startY(), 6)
        self.assertEqual(segment.endX(), 7)
        self.assertEqual(segment.endY(), 4)
        segment.setEndY(8)
        self.assertEqual(segment.start(), QgsPointXY(5, 6))
        self.assertEqual(segment.end(), QgsPointXY(7, 8))
        self.assertEqual(segment.startX(), 5)
        self.assertEqual(segment.startY(), 6)
        self.assertEqual(segment.endX(), 7)
        self.assertEqual(segment.endY(), 8)
        segment.setStart(QgsPointXY(1, 2))
        self.assertEqual(segment.start(), QgsPointXY(1, 2))
        self.assertEqual(segment.end(), QgsPointXY(7, 8))
        self.assertEqual(segment.startX(), 1)
        self.assertEqual(segment.startY(), 2)
        self.assertEqual(segment.endX(), 7)
        self.assertEqual(segment.endY(), 8)
        segment.setEnd(QgsPointXY(3, 4))
        self.assertEqual(segment.start(), QgsPointXY(1, 2))
        self.assertEqual(segment.end(), QgsPointXY(3, 4))
        self.assertEqual(segment.startX(), 1)
        self.assertEqual(segment.startY(), 2)
        self.assertEqual(segment.endX(), 3)
        self.assertEqual(segment.endY(), 4)

    def testEquality(self):
        segment1 = QgsLineSegment2D(QgsPointXY(1, 2), QgsPointXY(3, 4))
        segment2 = QgsLineSegment2D(QgsPointXY(1, 2), QgsPointXY(3, 4))
        self.assertEqual(segment1, segment2)
        self.assertFalse(segment1 != segment2)
        segment2 = QgsLineSegment2D(QgsPointXY(1, 2), QgsPointXY(3, 5))
        self.assertNotEqual(segment1, segment2)
        self.assertTrue(segment1 != segment2)
        segment2 = QgsLineSegment2D(QgsPointXY(1, 2), QgsPointXY(5, 4))
        self.assertNotEqual(segment1, segment2)
        self.assertTrue(segment1 != segment2)
        segment2 = QgsLineSegment2D(QgsPointXY(1, 5), QgsPointXY(3, 4))
        self.assertNotEqual(segment1, segment2)
        self.assertTrue(segment1 != segment2)
        segment2 = QgsLineSegment2D(QgsPointXY(5, 2), QgsPointXY(3, 4))
        self.assertNotEqual(segment1, segment2)
        self.assertTrue(segment1 != segment2)

    def testLength(self):
        segment = QgsLineSegment2D(QgsPointXY(1, 2), QgsPointXY(3, 5))
        self.assertAlmostEqual(segment.length(), 3.60555127546, 5)
        self.assertEqual(segment.lengthSquared(), 13)

    def testPointLeftOfLine(self):
        segment = QgsLineSegment2D(QgsPointXY(1, 2), QgsPointXY(3, 5))
        self.assertEqual(segment.pointLeftOfLine(QgsPointXY(1.5, 6)), -1)
        self.assertEqual(segment.pointLeftOfLine(QgsPointXY(1.5, -6)), 1)
        self.assertEqual(segment.pointLeftOfLine(QgsPointXY(5, 8)), 0)
        segment = QgsLineSegment2D(QgsPointXY(3, 5), QgsPointXY(1, 2))
        self.assertEqual(segment.pointLeftOfLine(QgsPointXY(1.5, 6)), 1)
        self.assertEqual(segment.pointLeftOfLine(QgsPointXY(1.5, -6)), -1)
        self.assertEqual(segment.pointLeftOfLine(QgsPointXY(5, 8)), 0)

    def testReverse(self):
        segment = QgsLineSegment2D(QgsPointXY(1, 2), QgsPointXY(3, 4))
        segment.reverse()
        self.assertEqual(segment.start(), QgsPointXY(3, 4))
        self.assertEqual(segment.end(), QgsPointXY(1, 2))


if __name__ == '__main__':
    unittest.main()
