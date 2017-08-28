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
                       QgsLayoutGridSettings,
                       QgsLayoutMeasurement,
                       QgsUnitTypes,
                       QgsLayoutPoint,
                       QgsLayoutItemPage,
                       QgsLayoutGuide)
from qgis.PyQt.QtCore import QPointF

from qgis.testing import start_app, unittest

start_app()


class TestQgsLayoutSnapper(unittest.TestCase):

    def testGettersSetters(self):
        p = QgsProject()
        l = QgsLayout(p)
        s = QgsLayoutSnapper(l)

        s.setSnapToGrid(False)
        self.assertFalse(s.snapToGrid())
        s.setSnapToGrid(True)
        self.assertTrue(s.snapToGrid())

        s.setSnapToGuides(False)
        self.assertFalse(s.snapToGuides())
        s.setSnapToGuides(True)
        self.assertTrue(s.snapToGuides())

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

        l.gridSettings().setResolution(QgsLayoutMeasurement(5, QgsUnitTypes.LayoutMillimeters))

        s.setSnapToGrid(True)
        s.setSnapTolerance(1)

        point, snappedX, snappedY = s.snapPointToGrid(QPointF(1, 1), 1)
        self.assertTrue(snappedX)
        self.assertTrue(snappedY)
        self.assertEqual(point, QPointF(0, 0))

        point, snappedX, snappedY = s.snapPointToGrid(QPointF(9, 1), 1)
        self.assertTrue(snappedX)
        self.assertTrue(snappedY)
        self.assertEqual(point, QPointF(10, 0))

        point, snappedX, snappedY = s.snapPointToGrid(QPointF(1, 11), 1)
        self.assertTrue(snappedX)
        self.assertTrue(snappedY)
        self.assertEqual(point, QPointF(0, 10))

        point, snappedX, snappedY = s.snapPointToGrid(QPointF(13, 11), 1)
        self.assertFalse(snappedX)
        self.assertTrue(snappedY)
        self.assertEqual(point, QPointF(13, 10))

        point, snappedX, snappedY = s.snapPointToGrid(QPointF(11, 13), 1)
        self.assertTrue(snappedX)
        self.assertFalse(snappedY)
        self.assertEqual(point, QPointF(10, 13))

        point, snappedX, snappedY = s.snapPointToGrid(QPointF(13, 23), 1)
        self.assertFalse(snappedX)
        self.assertFalse(snappedY)
        self.assertEqual(point, QPointF(13, 23))

        # grid disabled
        s.setSnapToGrid(False)
        point, nappedX, snappedY = s.snapPointToGrid(QPointF(1, 1), 1)
        self.assertFalse(nappedX)
        self.assertFalse(snappedY)
        self.assertEqual(point, QPointF(1, 1))
        s.setSnapToGrid(True)

        # with different pixel scale
        point, snappedX, snappedY = s.snapPointToGrid(QPointF(0.5, 0.5), 1)
        self.assertTrue(snappedX)
        self.assertTrue(snappedY)
        self.assertEqual(point, QPointF(0, 0))
        point, snappedX, snappedY = s.snapPointToGrid(QPointF(0.5, 0.5), 3)
        self.assertFalse(snappedX)
        self.assertFalse(snappedY)
        self.assertEqual(point, QPointF(0.5, 0.5))

        # with offset grid
        l.gridSettings().setOffset(QgsLayoutPoint(2, 0))
        point, snappedX, snappedY = s.snapPointToGrid(QPointF(13, 23), 1)
        self.assertTrue(snappedX)
        self.assertFalse(snappedY)
        self.assertEqual(point, QPointF(12, 23))

    def testSnapPointToGuides(self):
        p = QgsProject()
        l = QgsLayout(p)
        page = QgsLayoutItemPage(l)
        page.setPageSize('A4')
        l.pageCollection().addPage(page)
        s = QgsLayoutSnapper(l)
        guides = l.guides()

        s.setSnapToGuides(True)
        s.setSnapTolerance(1)

        # no guides
        point, snapped = s.snapPointToGuides(0.5, QgsLayoutGuide.Vertical, 1)
        self.assertFalse(snapped)

        guides.addGuide(QgsLayoutGuide(QgsLayoutGuide.Vertical, QgsLayoutMeasurement(1), page))
        point, snapped = s.snapPointToGuides(0.5, QgsLayoutGuide.Vertical, 1)
        self.assertTrue(snapped)
        self.assertEqual(point, 1)

        # outside tolerance
        point, snapped = s.snapPointToGuides(5.5, QgsLayoutGuide.Vertical, 1)
        self.assertFalse(snapped)

        # snapping off
        s.setSnapToGuides(False)
        point, snapped = s.snapPointToGuides(0.5, QgsLayoutGuide.Vertical, 1)
        self.assertFalse(snapped)

        s.setSnapToGuides(True)
        # snap to hoz
        point, snapped = s.snapPointToGuides(0.5, QgsLayoutGuide.Horizontal, 1)
        self.assertFalse(snapped)
        guides.addGuide(QgsLayoutGuide(QgsLayoutGuide.Horizontal, QgsLayoutMeasurement(1), page))
        point, snapped = s.snapPointToGuides(0.5, QgsLayoutGuide.Horizontal, 1)
        self.assertTrue(snapped)
        self.assertEqual(point, 1)

        # with different pixel scale
        point, snapped = s.snapPointToGuides(0.5, QgsLayoutGuide.Horizontal, 3)
        self.assertFalse(snapped)

    def testSnapPoint(self):
        p = QgsProject()
        l = QgsLayout(p)
        page = QgsLayoutItemPage(l)
        page.setPageSize('A4')
        l.pageCollection().addPage(page)
        s = QgsLayoutSnapper(l)
        guides = l.guides()

        # first test snapping to grid
        l.gridSettings().setResolution(QgsLayoutMeasurement(5, QgsUnitTypes.LayoutMillimeters))
        s.setSnapToGrid(True)
        s.setSnapTolerance(1)

        point, snapped = s.snapPoint(QPointF(1, 1), 1)
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(0, 0))

        s.setSnapToGrid(False)
        point, snapped = s.snapPoint(QPointF(1, 1), 1)
        self.assertFalse(snapped)
        self.assertEqual(point, QPointF(1, 1))

        # test that guide takes precedence
        s.setSnapToGrid(True)
        s.setSnapToGuides(True)
        guides.addGuide(QgsLayoutGuide(QgsLayoutGuide.Horizontal, QgsLayoutMeasurement(0.5), page))
        point, snapped = s.snapPoint(QPointF(1, 1), 1)
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(0, 0.5))


if __name__ == '__main__':
    unittest.main()
