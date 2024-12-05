"""QGIS Unit tests for QgsLayoutSnapper.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "05/07/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtCore import QPointF, QRectF, Qt
from qgis.PyQt.QtWidgets import QGraphicsLineItem
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsLayout,
    QgsLayoutGuide,
    QgsLayoutItemMap,
    QgsLayoutItemPage,
    QgsLayoutMeasurement,
    QgsLayoutPoint,
    QgsLayoutSize,
    QgsLayoutSnapper,
    QgsProject,
    QgsReadWriteContext,
    QgsUnitTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLayoutSnapper(QgisTestCase):

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

        s.setSnapToItems(False)
        self.assertFalse(s.snapToItems())
        s.setSnapToItems(True)
        self.assertTrue(s.snapToItems())

        s.setSnapTolerance(15)
        self.assertEqual(s.snapTolerance(), 15)

    def testSnapPointToGrid(self):
        p = QgsProject()
        l = QgsLayout(p)
        # need a page to snap to grid
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        l.pageCollection().addPage(page)
        s = QgsLayoutSnapper(l)

        l.gridSettings().setResolution(
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )

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

    def testSnapPointsToGrid(self):
        p = QgsProject()
        l = QgsLayout(p)
        # need a page to snap to grid
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        l.pageCollection().addPage(page)
        s = QgsLayoutSnapper(l)

        l.gridSettings().setResolution(
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )

        s.setSnapToGrid(True)
        s.setSnapTolerance(1)

        delta, snappedX, snappedY = s.snapPointsToGrid([QPointF(1, 0.5)], 1)
        self.assertTrue(snappedX)
        self.assertTrue(snappedY)
        self.assertEqual(delta, QPointF(-1, -0.5))

        point, snappedX, snappedY = s.snapPointsToGrid(
            [QPointF(9, 2), QPointF(12, 6)], 1
        )
        self.assertTrue(snappedX)
        self.assertTrue(snappedY)
        self.assertEqual(point, QPointF(1, -1))

        point, snappedX, snappedY = s.snapPointsToGrid(
            [QPointF(9, 2), QPointF(12, 7)], 1
        )
        self.assertTrue(snappedX)
        self.assertFalse(snappedY)
        self.assertEqual(point, QPointF(1, 0))

        point, snappedX, snappedY = s.snapPointsToGrid(
            [QPointF(8, 2), QPointF(12, 6)], 1
        )
        self.assertFalse(snappedX)
        self.assertTrue(snappedY)
        self.assertEqual(point, QPointF(0, -1))

        # grid disabled
        s.setSnapToGrid(False)
        point, snappedX, snappedY = s.snapPointsToGrid([QPointF(1, 1)], 1)
        self.assertFalse(snappedX)
        self.assertFalse(snappedY)
        self.assertEqual(point, QPointF(0, 0))
        s.setSnapToGrid(True)

        # with different pixel scale
        point, snappedX, snappedY = s.snapPointsToGrid([QPointF(0.5, 0.5)], 1)
        self.assertTrue(snappedX)
        self.assertTrue(snappedY)
        self.assertEqual(point, QPointF(-0.5, -0.5))
        point, snappedX, snappedY = s.snapPointsToGrid([QPointF(0.5, 0.5)], 3)
        self.assertFalse(snappedX)
        self.assertFalse(snappedY)
        self.assertEqual(point, QPointF(0, 0))

        # with offset grid
        l.gridSettings().setOffset(QgsLayoutPoint(2, 0))
        point, snappedX, snappedY = s.snapPointsToGrid([QPointF(13, 23)], 1)
        self.assertTrue(snappedX)
        self.assertFalse(snappedY)
        self.assertEqual(point, QPointF(-1, 0))

    def testSnapPointToGuides(self):
        p = QgsProject()
        l = QgsLayout(p)
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        l.pageCollection().addPage(page)
        s = QgsLayoutSnapper(l)
        guides = l.guides()

        s.setSnapToGuides(True)
        s.setSnapTolerance(1)

        # no guides
        point, snapped = s.snapPointToGuides(0.5, Qt.Orientation.Vertical, 1)
        self.assertFalse(snapped)

        guides.addGuide(
            QgsLayoutGuide(Qt.Orientation.Vertical, QgsLayoutMeasurement(1), page)
        )
        point, snapped = s.snapPointToGuides(0.5, Qt.Orientation.Vertical, 1)
        self.assertTrue(snapped)
        self.assertEqual(point, 1)

        # outside tolerance
        point, snapped = s.snapPointToGuides(5.5, Qt.Orientation.Vertical, 1)
        self.assertFalse(snapped)

        # snapping off
        s.setSnapToGuides(False)
        point, snapped = s.snapPointToGuides(0.5, Qt.Orientation.Vertical, 1)
        self.assertFalse(snapped)

        s.setSnapToGuides(True)
        # snap to hoz
        point, snapped = s.snapPointToGuides(0.5, Qt.Orientation.Horizontal, 1)
        self.assertFalse(snapped)
        guides.addGuide(
            QgsLayoutGuide(Qt.Orientation.Horizontal, QgsLayoutMeasurement(1), page)
        )
        point, snapped = s.snapPointToGuides(0.5, Qt.Orientation.Horizontal, 1)
        self.assertTrue(snapped)
        self.assertEqual(point, 1)

        # with different pixel scale
        point, snapped = s.snapPointToGuides(0.5, Qt.Orientation.Horizontal, 3)
        self.assertFalse(snapped)

    def testSnapPointsToGuides(self):
        p = QgsProject()
        l = QgsLayout(p)
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        l.pageCollection().addPage(page)
        s = QgsLayoutSnapper(l)
        guides = l.guides()

        s.setSnapToGuides(True)
        s.setSnapTolerance(1)

        # no guides
        delta, snapped = s.snapPointsToGuides([0.5], Qt.Orientation.Vertical, 1)
        self.assertFalse(snapped)

        guides.addGuide(
            QgsLayoutGuide(Qt.Orientation.Vertical, QgsLayoutMeasurement(1), page)
        )
        point, snapped = s.snapPointsToGuides([0.7], Qt.Orientation.Vertical, 1)
        self.assertTrue(snapped)
        self.assertAlmostEqual(point, 0.3, 5)

        point, snapped = s.snapPointsToGuides([0.7, 1.2], Qt.Orientation.Vertical, 1)
        self.assertTrue(snapped)
        self.assertAlmostEqual(point, -0.2, 5)

        # outside tolerance
        point, snapped = s.snapPointsToGuides([5.5], Qt.Orientation.Vertical, 1)
        self.assertFalse(snapped)

        # snapping off
        s.setSnapToGuides(False)
        point, snapped = s.snapPointsToGuides([0.5], Qt.Orientation.Vertical, 1)
        self.assertFalse(snapped)

        s.setSnapToGuides(True)

        # snap to hoz
        point, snapped = s.snapPointsToGuides([0.5], Qt.Orientation.Horizontal, 1)
        self.assertFalse(snapped)
        guides.addGuide(
            QgsLayoutGuide(Qt.Orientation.Horizontal, QgsLayoutMeasurement(1), page)
        )
        point, snapped = s.snapPointsToGuides([0.7], Qt.Orientation.Horizontal, 1)
        self.assertTrue(snapped)
        self.assertAlmostEqual(point, 0.3, 5)
        point, snapped = s.snapPointsToGuides([0.7, 1.2], Qt.Orientation.Horizontal, 1)
        self.assertTrue(snapped)
        self.assertAlmostEqual(point, -0.2, 5)
        point, snapped = s.snapPointsToGuides(
            [0.7, 0.9, 1.2], Qt.Orientation.Horizontal, 1
        )
        self.assertTrue(snapped)
        self.assertAlmostEqual(point, 0.1, 5)

        # with different pixel scale
        point, snapped = s.snapPointsToGuides([0.5, 1.5], Qt.Orientation.Horizontal, 3)
        self.assertFalse(snapped)

    def testSnapPointToItems(self):
        p = QgsProject()
        l = QgsLayout(p)
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        # l.pageCollection().addPage(page)
        s = QgsLayoutSnapper(l)
        guides = l.guides()

        s.setSnapToItems(True)
        s.setSnapTolerance(1)

        # no items
        point, snapped = s.snapPointToItems(0.5, Qt.Orientation.Horizontal, 1, [])
        self.assertFalse(snapped)

        line = QGraphicsLineItem()
        line.setVisible(True)
        point, snapped = s.snapPointToItems(0.5, Qt.Orientation.Horizontal, 1, [], line)
        self.assertFalse(line.isVisible())

        guides.addGuide(
            QgsLayoutGuide(Qt.Orientation.Vertical, QgsLayoutMeasurement(1), page)
        )

        # add an item
        item1 = QgsLayoutItemMap(l)
        item1.attemptMove(
            QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        item1.attemptResize(
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        l.addItem(item1)

        point, snapped = s.snapPointToItems(3.5, Qt.Orientation.Horizontal, 1, [], line)
        self.assertTrue(snapped)
        self.assertEqual(point, 4)
        self.assertTrue(line.isVisible())
        point, snapped = s.snapPointToItems(4.5, Qt.Orientation.Horizontal, 1, [])
        self.assertTrue(snapped)
        self.assertEqual(point, 4)

        # ignoring item
        point, snapped = s.snapPointToItems(4.5, Qt.Orientation.Horizontal, 1, [item1])
        self.assertFalse(snapped)

        # outside tolerance
        point, snapped = s.snapPointToItems(5.5, Qt.Orientation.Horizontal, 1, [], line)
        self.assertFalse(snapped)
        self.assertFalse(line.isVisible())

        # snap to center
        point, snapped = s.snapPointToItems(12.5, Qt.Orientation.Horizontal, 1, [])
        self.assertTrue(snapped)
        self.assertEqual(point, 13)

        # snap to right
        point, snapped = s.snapPointToItems(22.5, Qt.Orientation.Horizontal, 1, [])
        self.assertTrue(snapped)
        self.assertEqual(point, 22)

        # snap to top
        point, snapped = s.snapPointToItems(7.5, Qt.Orientation.Vertical, 1, [], line)
        self.assertTrue(snapped)
        self.assertEqual(point, 8)
        self.assertTrue(line.isVisible())
        point, snapped = s.snapPointToItems(8.5, Qt.Orientation.Vertical, 1, [])
        self.assertTrue(snapped)
        self.assertEqual(point, 8)

        # outside tolerance
        point, snapped = s.snapPointToItems(5.5, Qt.Orientation.Vertical, 1, [], line)
        self.assertFalse(snapped)
        self.assertFalse(line.isVisible())

        # snap to center
        point, snapped = s.snapPointToItems(13.5, Qt.Orientation.Vertical, 1, [])
        self.assertTrue(snapped)
        self.assertEqual(point, 14)

        # snap to bottom
        point, snapped = s.snapPointToItems(20.5, Qt.Orientation.Vertical, 1, [])
        self.assertTrue(snapped)
        self.assertEqual(point, 20)

        # snapping off
        s.setSnapToItems(False)
        line.setVisible(True)
        point, snapped = s.snapPointToItems(20.5, Qt.Orientation.Vertical, 1, [], line)
        self.assertFalse(snapped)
        self.assertFalse(line.isVisible())

        # with different pixel scale
        s.setSnapToItems(True)
        point, snapped = s.snapPointToItems(20.5, Qt.Orientation.Vertical, 3, [])
        self.assertFalse(snapped)

    def testSnapPointsToItems(self):
        p = QgsProject()
        l = QgsLayout(p)
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        # l.pageCollection().addPage(page)
        s = QgsLayoutSnapper(l)
        guides = l.guides()

        s.setSnapToItems(True)
        s.setSnapTolerance(1)

        # no items
        point, snapped = s.snapPointsToItems([0.5], Qt.Orientation.Horizontal, 1, [])
        self.assertFalse(snapped)

        line = QGraphicsLineItem()
        line.setVisible(True)
        point, snapped = s.snapPointsToItems(
            [0.5], Qt.Orientation.Horizontal, 1, [], line
        )
        self.assertFalse(line.isVisible())

        guides.addGuide(
            QgsLayoutGuide(Qt.Orientation.Vertical, QgsLayoutMeasurement(1), page)
        )

        # add an item
        item1 = QgsLayoutItemMap(l)
        item1.attemptMove(
            QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        item1.attemptResize(
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        l.addItem(item1)

        point, snapped = s.snapPointsToItems(
            [3.5], Qt.Orientation.Horizontal, 1, [], line
        )
        self.assertTrue(snapped)
        self.assertEqual(point, 0.5)
        self.assertTrue(line.isVisible())
        point, snapped = s.snapPointsToItems([4.5], Qt.Orientation.Horizontal, 1, [])
        self.assertTrue(snapped)
        self.assertEqual(point, -0.5)
        point, snapped = s.snapPointsToItems(
            [4.6, 4.5], Qt.Orientation.Horizontal, 1, []
        )
        self.assertTrue(snapped)
        self.assertEqual(point, -0.5)
        point, snapped = s.snapPointsToItems(
            [4.6, 4.5, 3.7], Qt.Orientation.Horizontal, 1, []
        )
        self.assertTrue(snapped)
        self.assertAlmostEqual(point, 0.3, 5)

        # ignoring item
        point, snapped = s.snapPointsToItems(
            [4.5], Qt.Orientation.Horizontal, 1, [item1]
        )
        self.assertFalse(snapped)

        # outside tolerance
        point, snapped = s.snapPointsToItems(
            [5.5], Qt.Orientation.Horizontal, 1, [], line
        )
        self.assertFalse(snapped)
        self.assertFalse(line.isVisible())

        # snap to center
        point, snapped = s.snapPointsToItems([12.5], Qt.Orientation.Horizontal, 1, [])
        self.assertTrue(snapped)
        self.assertEqual(point, 0.5)

        # snap to right
        point, snapped = s.snapPointsToItems([22.5], Qt.Orientation.Horizontal, 1, [])
        self.assertTrue(snapped)
        self.assertEqual(point, -0.5)

        # snap to top
        point, snapped = s.snapPointsToItems(
            [7.5], Qt.Orientation.Vertical, 1, [], line
        )
        self.assertTrue(snapped)
        self.assertEqual(point, 0.5)
        self.assertTrue(line.isVisible())
        point, snapped = s.snapPointsToItems([8.5], Qt.Orientation.Vertical, 1, [])
        self.assertTrue(snapped)
        self.assertEqual(point, -0.5)

        # outside tolerance
        point, snapped = s.snapPointsToItems(
            [5.5], Qt.Orientation.Vertical, 1, [], line
        )
        self.assertFalse(snapped)
        self.assertFalse(line.isVisible())

        # snap to center
        point, snapped = s.snapPointsToItems([13.5], Qt.Orientation.Vertical, 1, [])
        self.assertTrue(snapped)
        self.assertEqual(point, 0.5)

        # snap to bottom
        point, snapped = s.snapPointsToItems([20.5], Qt.Orientation.Vertical, 1, [])
        self.assertTrue(snapped)
        self.assertEqual(point, -0.5)

        # snapping off
        s.setSnapToItems(False)
        line.setVisible(True)
        point, snapped = s.snapPointsToItems(
            [20.5], Qt.Orientation.Vertical, 1, [], line
        )
        self.assertFalse(snapped)
        self.assertFalse(line.isVisible())

        # with different pixel scale
        s.setSnapToItems(True)
        point, snapped = s.snapPointsToItems([20.5], Qt.Orientation.Vertical, 3, [])
        self.assertFalse(snapped)

    def testSnapPoint(self):
        p = QgsProject()
        l = QgsLayout(p)
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        l.pageCollection().addPage(page)
        s = QgsLayoutSnapper(l)
        guides = l.guides()

        # first test snapping to grid
        l.gridSettings().setResolution(
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        s.setSnapToGrid(True)
        s.setSnapTolerance(1)

        point, snapped = s.snapPoint(QPointF(1, 1), 1)
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(0, 0))

        s.setSnapToItems(False)
        s.setSnapToGrid(False)
        point, snapped = s.snapPoint(QPointF(1, 1), 1)
        self.assertFalse(snapped)
        self.assertEqual(point, QPointF(1, 1))

        # test that guide takes precedence
        s.setSnapToGrid(True)
        s.setSnapToGuides(True)
        guides.addGuide(
            QgsLayoutGuide(Qt.Orientation.Horizontal, QgsLayoutMeasurement(0.5), page)
        )
        point, snapped = s.snapPoint(QPointF(1, 1), 1)
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(0, 0.5))

        # add an item
        item1 = QgsLayoutItemMap(l)
        item1.attemptMove(
            QgsLayoutPoint(121, 1.1, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        l.addItem(item1)

        # test that guide takes precedence over item
        s.setSnapToGrid(True)
        s.setSnapToGuides(True)
        s.setSnapToItems(True)
        point, snapped = s.snapPoint(QPointF(1, 1), 1)
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(0, 0.5))
        # but items take precedence over grid
        s.setSnapToGuides(False)
        point, snapped = s.snapPoint(QPointF(1, 1), 1)
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(0, 1.1))

        # ... unless item is ignored!
        point, snapped = s.snapPoint(QPointF(1, 1), 1, None, None, [item1])
        self.assertTrue(snapped)
        self.assertEqual(point, QPointF(0, 0))

    def testSnapRect(self):
        p = QgsProject()
        l = QgsLayout(p)
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        l.pageCollection().addPage(page)
        s = QgsLayoutSnapper(l)
        guides = l.guides()

        # first test snapping to grid
        l.gridSettings().setResolution(
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        s.setSnapToItems(False)
        s.setSnapToGrid(True)
        s.setSnapTolerance(1)

        rect, snapped = s.snapRect(QRectF(1, 1, 2, 1), 1)
        self.assertTrue(snapped)
        self.assertEqual(rect, QRectF(0, 0, 2, 1))
        rect, snapped = s.snapRect(QRectF(1, 1, 3.5, 3.5), 1)
        self.assertTrue(snapped)
        self.assertEqual(rect, QRectF(1.5, 1.5, 3.5, 3.5))

        s.setSnapToItems(False)
        s.setSnapToGrid(False)
        rect, snapped = s.snapRect(QRectF(1, 1, 3.5, 3.5), 1)
        self.assertFalse(snapped)
        self.assertEqual(rect, QRectF(1, 1, 3.5, 3.5))

        # test that guide takes precedence
        s.setSnapToGrid(True)
        s.setSnapToGuides(True)
        guides.addGuide(
            QgsLayoutGuide(Qt.Orientation.Horizontal, QgsLayoutMeasurement(0.5), page)
        )
        rect, snapped = s.snapRect(QRectF(1, 1, 2, 3), 1)
        self.assertTrue(snapped)
        self.assertEqual(rect, QRectF(0.0, 0.5, 2.0, 3.0))

        # add an item
        item1 = QgsLayoutItemMap(l)
        item1.attemptMove(
            QgsLayoutPoint(121, 1.1, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        l.addItem(item1)

        # test that guide takes precedence over item
        s.setSnapToGrid(True)
        s.setSnapToGuides(True)
        s.setSnapToItems(True)
        rect, snapped = s.snapRect(QRectF(1, 1, 2, 3), 1)
        self.assertTrue(snapped)
        self.assertEqual(rect, QRectF(0.0, 0.5, 2.0, 3.0))
        # but items take precedence over grid
        s.setSnapToGuides(False)
        rect, snapped = s.snapRect(QRectF(1, 1, 2, 3), 1)
        self.assertTrue(snapped)
        self.assertEqual(rect, QRectF(0.0, 1.1, 2.0, 3.0))

        # ... unless item is ignored!
        rect, snapped = s.snapRect(QRectF(1, 1, 2, 3), 1, None, None, [item1])
        self.assertTrue(snapped)
        self.assertEqual(rect, QRectF(0.0, 0.0, 2.0, 3.0))

    def testReadWriteXml(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults()
        snapper = l.snapper()

        snapper.setSnapToGrid(True)
        snapper.setSnapTolerance(1)
        snapper.setSnapToGuides(True)
        snapper.setSnapToItems(True)

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        self.assertTrue(snapper.writeXml(elem, doc, QgsReadWriteContext()))

        l2 = QgsLayout(p)
        l2.initializeDefaults()
        snapper2 = l2.snapper()

        self.assertTrue(
            snapper2.readXml(elem.firstChildElement(), doc, QgsReadWriteContext())
        )
        self.assertTrue(snapper2.snapToGrid())
        self.assertEqual(snapper2.snapTolerance(), 1)
        self.assertTrue(snapper2.snapToGuides())
        self.assertTrue(snapper2.snapToItems())

        snapper.setSnapToGrid(False)
        snapper.setSnapTolerance(1)
        snapper.setSnapToGuides(False)
        snapper.setSnapToItems(False)

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        self.assertTrue(snapper.writeXml(elem, doc, QgsReadWriteContext()))

        self.assertTrue(
            snapper2.readXml(elem.firstChildElement(), doc, QgsReadWriteContext())
        )
        self.assertFalse(snapper2.snapToGrid())
        self.assertFalse(snapper2.snapToGuides())
        self.assertFalse(snapper2.snapToItems())


if __name__ == "__main__":
    unittest.main()
