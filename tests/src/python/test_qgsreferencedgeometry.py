# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsReferencedGeometry.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '31/08/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsRectangle,
                       QgsPointXY,
                       QgsReferencedRectangle,
                       QgsReferencedPointXY,
                       QgsCoordinateReferenceSystem)
from qgis.PyQt.QtCore import QVariant
from qgis.testing import start_app, unittest
from utilities import compareWkt

start_app()


class TestQgsReferencedGeometry(unittest.TestCase):

    def testRectangle(self):
        rect = QgsReferencedRectangle(QgsRectangle(0.0, 1.0, 20.0, 10.0), QgsCoordinateReferenceSystem('epsg:3111'))
        self.assertEqual(rect.xMinimum(), 0.0)
        self.assertEqual(rect.yMinimum(), 1.0)
        self.assertEqual(rect.xMaximum(), 20.0)
        self.assertEqual(rect.yMaximum(), 10.0)
        self.assertEqual(rect.crs().authid(), 'EPSG:3111')

        rect.setCrs(QgsCoordinateReferenceSystem('epsg:28356'))
        self.assertEqual(rect.crs().authid(), 'EPSG:28356')

        # in variant
        v = QVariant(QgsReferencedRectangle(QgsRectangle(1.0, 2.0, 3.0, 4.0), QgsCoordinateReferenceSystem('epsg:3111')))
        self.assertEqual(v.value().xMinimum(), 1.0)
        self.assertEqual(v.value().yMinimum(), 2.0)
        self.assertEqual(v.value().xMaximum(), 3.0)
        self.assertEqual(v.value().yMaximum(), 4.0)
        self.assertEqual(v.value().crs().authid(), 'EPSG:3111')

        # to rectangle
        r = QgsRectangle(rect)
        self.assertEqual(r.xMinimum(), 0.0)
        self.assertEqual(r.yMinimum(), 1.0)
        self.assertEqual(r.xMaximum(), 20.0)
        self.assertEqual(r.yMaximum(), 10.0)

        # test that QgsReferencedRectangle IS a QgsRectangle
        r2 = QgsRectangle(5, 6, 30, 40)
        r2.combineExtentWith(rect)
        self.assertEqual(r2.xMinimum(), 0.0)
        self.assertEqual(r2.yMinimum(), 1.0)
        self.assertEqual(r2.xMaximum(), 30.0)
        self.assertEqual(r2.yMaximum(), 40.0)

        # equality
        rect = QgsReferencedRectangle(QgsRectangle(0.0, 1.0, 20.0, 10.0), QgsCoordinateReferenceSystem('epsg:3111'))
        rect2 = QgsReferencedRectangle(QgsRectangle(0.0, 1.0, 20.0, 10.0), QgsCoordinateReferenceSystem('epsg:3111'))
        self.assertEqual(rect, rect2)
        rect2 = QgsReferencedRectangle(QgsRectangle(0.0, 1.0, 20.0, 10.0), QgsCoordinateReferenceSystem('epsg:4326'))
        self.assertNotEqual(rect, rect2)
        rect2 = QgsReferencedRectangle(QgsRectangle(0.0, 1.5, 20.0, 10.0), QgsCoordinateReferenceSystem('epsg:3111'))
        self.assertNotEqual(rect, rect2)

    def testPoint(self):
        point = QgsReferencedPointXY(QgsPointXY(1.0, 2.0), QgsCoordinateReferenceSystem('epsg:3111'))
        self.assertEqual(point.x(), 1.0)
        self.assertEqual(point.y(), 2.0)
        self.assertEqual(point.crs().authid(), 'EPSG:3111')

        point.setCrs(QgsCoordinateReferenceSystem('epsg:28356'))
        self.assertEqual(point.crs().authid(), 'EPSG:28356')

        # in variant
        v = QVariant(QgsReferencedPointXY(QgsPointXY(3.0, 4.0), QgsCoordinateReferenceSystem('epsg:3111')))
        self.assertEqual(v.value().x(), 3.0)
        self.assertEqual(v.value().y(), 4.0)
        self.assertEqual(v.value().crs().authid(), 'EPSG:3111')

        # to QgsPointXY
        p = QgsPointXY(point)
        self.assertEqual(p.x(), 1.0)
        self.assertEqual(p.y(), 2.0)

        # equality
        point = QgsReferencedPointXY(QgsPointXY(1.0, 2.0), QgsCoordinateReferenceSystem('epsg:3111'))
        point2 = QgsReferencedPointXY(QgsPointXY(1.0, 2.0), QgsCoordinateReferenceSystem('epsg:3111'))
        self.assertEqual(point, point2)
        point2 = QgsReferencedPointXY(QgsPointXY(1.0, 2.0), QgsCoordinateReferenceSystem('epsg:4326'))
        self.assertNotEqual(point, point2)
        point2 = QgsReferencedPointXY(QgsPointXY(1.1, 2.0), QgsCoordinateReferenceSystem('epsg:3111'))
        self.assertNotEqual(point, point2)


if __name__ == '__main__':
    unittest.main()
