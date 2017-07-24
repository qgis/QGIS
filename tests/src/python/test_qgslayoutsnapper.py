# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutSnapper.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '05/07/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsProject,
                       QgsLayout,
                       QgsLayoutSnapper,
                       QgsLayoutMeasurement,
                       QgsUnitTypes,
                       QgsLayoutPoint,
                       QgsLayoutItemPage)
from qgis.PyQt.QtCore import QRectF, QPointF
from qgis.PyQt.QtGui import (QTransform,
                             QPen,
                             QColor)

from qgis.testing import start_app, unittest

start_app()


class TestQgsLayoutSnapper(unittest.TestCase):

    def testGettersSetters(self):
        p = QgsProject()
        l = QgsLayout(p)
        s = QgsLayoutSnapper(l)
        s.setGridResolution(QgsLayoutMeasurement(5, QgsUnitTypes.LayoutPoints))
        self.assertEqual(s.gridResolution().length(), 5.0)
        self.assertEqual(s.gridResolution().units(), QgsUnitTypes.LayoutPoints)

        s.setGridOffset(QgsLayoutPoint(6, 7, QgsUnitTypes.LayoutPixels))
        self.assertEqual(s.gridOffset().x(), 6.0)
        self.assertEqual(s.gridOffset().y(), 7.0)
        self.assertEqual(s.gridOffset().units(), QgsUnitTypes.LayoutPixels)

        s.setGridPen(QPen(QColor(255, 0, 255)))
        self.assertEqual(s.gridPen().color().name(), QColor(255, 0, 255).name())

        s.setGridStyle(QgsLayoutSnapper.GridDots)
        self.assertEqual(s.gridStyle(), QgsLayoutSnapper.GridDots)

        s.setSnapToGrid(False)
        self.assertFalse(s.snapToGrid())
        s.setSnapToGrid(True)
        self.assertTrue(s.snapToGrid())

        s.setSnapTolerance(15)
        self.assertEqual(s.snapTolerance(), 15)

    def testSnapPointToGrid(self):
        p = QgsProject()
        l = QgsLayout(p)
        # need a page to snap to grid
        page = QgsLayoutItemPage(l)
        page.setPageSize('A4')
        l.pageCollection().addPage(page)
        s = QgsLayoutSnapper(l)

        s.setGridResolution(QgsLayoutMeasurement(5, QgsUnitTypes.LayoutMillimeters))
        s.setSnapToGrid(True)
        s.setSnapTolerance(1)

        point, snapped = s.snapPointToGrid(QPointF(1, 1), 1)
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(0, 0))

        point, snapped = s.snapPointToGrid(QPointF(9, 1), 1)
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(10, 0))

        point, snapped = s.snapPointToGrid(QPointF(1, 11), 1)
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(0, 10))

        point, snapped = s.snapPointToGrid(QPointF(13, 11), 1)
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(13, 10))

        point, snapped = s.snapPointToGrid(QPointF(11, 13), 1)
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(10, 13))

        point, snapped = s.snapPointToGrid(QPointF(13, 23), 1)
        self.assertFalse(snapped)
        self.assertEqual(point, QPointF(13, 23))

        # grid disabled
        s.setSnapToGrid(False)
        point, snapped = s.snapPointToGrid(QPointF(1, 1), 1)
        self.assertFalse(snapped)
        self.assertEqual(point, QPointF(1, 1))
        s.setSnapToGrid(True)

        # with different pixel scale
        point, snapped = s.snapPointToGrid(QPointF(0.5, 0.5), 1)
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(0, 0))
        point, snapped = s.snapPointToGrid(QPointF(0.5, 0.5), 3)
        self.assertFalse(snapped)
        self.assertEqual(point, QPointF(0.5, 0.5))

        # with offset grid
        s.setGridOffset(QgsLayoutPoint(2, 0))
        point, snapped = s.snapPointToGrid(QPointF(13, 23), 1)
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(12, 23))


    def testSnapPoint(self):
        p = QgsProject()
        l = QgsLayout(p)
        page = QgsLayoutItemPage(l)
        page.setPageSize('A4')
        l.pageCollection().addPage(page)
        s = QgsLayoutSnapper(l)

        # first test snapping to grid
        s.setGridResolution(QgsLayoutMeasurement(5, QgsUnitTypes.LayoutMillimeters))
        s.setSnapToGrid(True)
        s.setSnapTolerance(1)

        point, snapped = s.snapPoint(QPointF(1, 1), 1)
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(0, 0))

        s.setSnapToGrid(False)
        point, snapped = s.snapPoint(QPointF(1, 1), 1)
        self.assertFalse(snapped)
        self.assertEqual(point, QPointF(1, 1))


if __name__ == '__main__':
    unittest.main()
