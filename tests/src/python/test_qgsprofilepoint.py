# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProfilePoint

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/03/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import os

import qgis  # NOQA

from qgis.PyQt.QtCore import QTemporaryDir

from qgis.core import (
    QgsProfilePoint
)

from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsProfilePoint(unittest.TestCase):

    def testBasic(self):
        point = QgsProfilePoint()
        self.assertTrue(point.isEmpty())
        self.assertEqual(str(point), '<QgsProfilePoint: EMPTY>')

        point.setDistance(1)
        self.assertFalse(point.isEmpty())
        self.assertEqual(point.distance(), 1)

        point = QgsProfilePoint()
        point.setElevation(1)
        self.assertFalse(point.isEmpty())
        self.assertEqual(point.elevation(), 1)

        point = QgsProfilePoint(1, 2)
        self.assertEqual(point.distance(), 1)
        self.assertEqual(point.elevation(), 2)
        self.assertEqual(str(point), '<QgsProfilePoint: 1, 2>')
        self.assertEqual(point[0], 1)
        self.assertEqual(point[1], 2)
        self.assertEqual(len(point), 2)

    def test_equality(self):
        p1 = QgsProfilePoint()
        p2 = QgsProfilePoint()
        self.assertEqual(p1, p2)
        self.assertFalse(p1 != p2)

        p1 = QgsProfilePoint(1, 2)
        p2 = QgsProfilePoint()
        self.assertNotEqual(p1, p2)
        self.assertFalse(p1 == p2)

        p1 = QgsProfilePoint()
        p2 = QgsProfilePoint(1, 2)
        self.assertNotEqual(p1, p2)
        self.assertFalse(p1 == p2)

        p1 = QgsProfilePoint(11, 12)
        p2 = QgsProfilePoint(1, 2)
        self.assertNotEqual(p1, p2)
        self.assertFalse(p1 == p2)

        p1 = QgsProfilePoint(1, 2)
        p2 = QgsProfilePoint(1, 2)
        self.assertEqual(p1, p2)
        self.assertFalse(p1 != p2)


if __name__ == '__main__':
    unittest.main()
