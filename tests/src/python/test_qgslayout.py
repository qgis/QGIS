# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayout

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/07/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import sip

from qgis.core import (QgsUnitTypes,
                       QgsLayout,
                       QgsLayoutItemPage,
                       QgsLayoutGuide,
                       QgsLayoutObject,
                       QgsProject,
                       QgsProperty,
                       QgsLayoutPageCollection,
                       QgsLayoutMeasurement,
                       QgsFillSymbol,
                       QgsReadWriteContext,
                       QgsLayoutItemMap,
                       QgsLayoutSize,
                       QgsLayoutPoint)
from qgis.PyQt.QtCore import Qt, QCoreApplication, QEvent, QPointF, QRectF
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest

start_app()


class TestQgsLayout(unittest.TestCase):

    def testReadWriteXml(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.setName('my layout')
        l.setUnits(QgsUnitTypes.LayoutInches)
        collection = l.pageCollection()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize('A6')
        collection.addPage(page)

        grid = l.gridSettings()
        grid.setResolution(QgsLayoutMeasurement(5, QgsUnitTypes.LayoutPoints))

        g1 = QgsLayoutGuide(Qt.Horizontal, QgsLayoutMeasurement(5, QgsUnitTypes.LayoutCentimeters),
                            l.pageCollection().page(0))
        l.guides().addGuide(g1)

        snapper = l.snapper()
        snapper.setSnapTolerance(7)

        doc = QDomDocument("testdoc")
        elem = l.writeXml(doc, QgsReadWriteContext())

        l2 = QgsLayout(p)
        self.assertTrue(l2.readXml(elem, doc, QgsReadWriteContext()))
        self.assertEqual(l2.name(), 'my layout')
        self.assertEqual(l2.units(), QgsUnitTypes.LayoutInches)

        collection2 = l2.pageCollection()
        self.assertEqual(collection2.pageCount(), 1)
        self.assertAlmostEqual(collection2.page(0).pageSize().width(), 105, 4)
        self.assertEqual(collection2.page(0).pageSize().height(), 148)
        self.assertEqual(l2.gridSettings().resolution().length(), 5.0)
        self.assertEqual(l2.gridSettings().resolution().units(), QgsUnitTypes.LayoutPoints)
        self.assertEqual(l2.guides().guidesOnPage(0)[0].orientation(), Qt.Horizontal)
        self.assertEqual(l2.guides().guidesOnPage(0)[0].position().length(), 5.0)
        self.assertEqual(l2.guides().guidesOnPage(0)[0].position().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(l2.snapper().snapTolerance(), 7)

    def testSelectedItems(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemMap(l)
        l.addItem(item1)
        item2 = QgsLayoutItemMap(l)
        l.addItem(item2)
        item3 = QgsLayoutItemMap(l)
        l.addItem(item3)

        self.assertFalse(l.selectedLayoutItems())
        item1.setSelected(True)
        self.assertEqual(set(l.selectedLayoutItems()), set([item1]))
        item2.setSelected(True)
        self.assertEqual(set(l.selectedLayoutItems()), set([item1, item2]))
        item3.setSelected(True)
        self.assertEqual(set(l.selectedLayoutItems()), set([item1, item2, item3]))
        item3.setLocked(True)
        self.assertEqual(set(l.selectedLayoutItems(False)), set([item1, item2]))
        self.assertEqual(set(l.selectedLayoutItems(True)), set([item1, item2, item3]))

    def testSelections(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemMap(l)
        l.addItem(item1)
        item2 = QgsLayoutItemMap(l)
        l.addItem(item2)
        item3 = QgsLayoutItemMap(l)
        l.addItem(item3)

        select_changed_spy = QSignalSpy(l.selectedItemChanged)
        l.setSelectedItem(None)
        self.assertFalse(l.selectedLayoutItems())
        self.assertEqual(len(select_changed_spy), 1)
        self.assertEqual(select_changed_spy[-1][0], None)

        l.setSelectedItem(item1)
        self.assertEqual(l.selectedLayoutItems(), [item1])
        self.assertEqual(len(select_changed_spy), 2)
        self.assertEqual(select_changed_spy[-1][0], item1)

        l.setSelectedItem(None)
        self.assertFalse(l.selectedLayoutItems())
        self.assertEqual(len(select_changed_spy), 3)
        self.assertEqual(select_changed_spy[-1][0], None)

        l.setSelectedItem(item2)
        self.assertEqual(l.selectedLayoutItems(), [item2])
        self.assertEqual(len(select_changed_spy), 4)
        self.assertEqual(select_changed_spy[-1][0], item2)

        l.deselectAll()
        self.assertFalse(l.selectedLayoutItems())
        self.assertEqual(len(select_changed_spy), 5)
        self.assertEqual(select_changed_spy[-1][0], None)

    def testLayoutItemAt(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemMap(l)
        item1.attemptMove(QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        item1.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item1)

        item2 = QgsLayoutItemMap(l)
        item2.attemptMove(QgsLayoutPoint(6, 10, QgsUnitTypes.LayoutMillimeters))
        item2.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item2)

        item3 = QgsLayoutItemMap(l)
        item3.attemptMove(QgsLayoutPoint(8, 12, QgsUnitTypes.LayoutMillimeters))
        item3.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        item3.setLocked(True)
        l.addItem(item3)

        self.assertIsNone(l.layoutItemAt(QPointF(0, 0)))
        self.assertIsNone(l.layoutItemAt(QPointF(100, 100)))

        self.assertEqual(l.layoutItemAt(QPointF(5, 9)), item1)
        self.assertEqual(l.layoutItemAt(QPointF(25, 23)), item3)
        self.assertIsNone(l.layoutItemAt(QPointF(25, 23), True))
        self.assertEqual(l.layoutItemAt(QPointF(7, 11)), item2)
        self.assertEqual(l.layoutItemAt(QPointF(9, 13)), item3)
        self.assertEqual(l.layoutItemAt(QPointF(9, 13), True), item2)

        self.assertEqual(l.layoutItemAt(QPointF(9, 13), item3), item2)
        self.assertEqual(l.layoutItemAt(QPointF(9, 13), item2), item1)
        self.assertIsNone(l.layoutItemAt(QPointF(9, 13), item1))
        item2.setLocked(True)
        self.assertEqual(l.layoutItemAt(QPointF(9, 13), item3, True), item1)

    def testStacking(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemMap(l)
        l.addLayoutItem(item1)
        item2 = QgsLayoutItemMap(l)
        l.addLayoutItem(item2)
        item3 = QgsLayoutItemMap(l)
        l.addLayoutItem(item3)

        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 2)
        self.assertEqual(item3.zValue(), 3)

        # no effect interactions
        self.assertFalse(l.raiseItem(None))
        self.assertFalse(l.lowerItem(None))
        self.assertFalse(l.moveItemToTop(None))
        self.assertFalse(l.moveItemToBottom(None))

        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 2)
        self.assertEqual(item3.zValue(), 3)

        # raising
        self.assertFalse(l.raiseItem(item3))
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 2)
        self.assertEqual(item3.zValue(), 3)

        self.assertTrue(l.raiseItem(item2))
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 2)

        self.assertFalse(l.raiseItem(item2))
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 2)

        self.assertTrue(l.raiseItem(item1))
        self.assertEqual(item1.zValue(), 2)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 1)

        # lower
        self.assertFalse(l.lowerItem(item3))
        self.assertEqual(item1.zValue(), 2)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 1)

        self.assertTrue(l.lowerItem(item2))
        self.assertEqual(item1.zValue(), 3)
        self.assertEqual(item2.zValue(), 2)
        self.assertEqual(item3.zValue(), 1)

        self.assertTrue(l.lowerItem(item2))
        self.assertEqual(item1.zValue(), 3)
        self.assertEqual(item2.zValue(), 1)
        self.assertEqual(item3.zValue(), 2)

        # raise to top
        self.assertFalse(l.moveItemToTop(item1))
        self.assertEqual(item1.zValue(), 3)
        self.assertEqual(item2.zValue(), 1)
        self.assertEqual(item3.zValue(), 2)

        self.assertTrue(l.moveItemToTop(item3))
        self.assertEqual(item1.zValue(), 2)
        self.assertEqual(item2.zValue(), 1)
        self.assertEqual(item3.zValue(), 3)

        self.assertTrue(l.moveItemToTop(item2))
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 2)

        # move to bottom
        self.assertFalse(l.moveItemToBottom(item1))
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 2)

        self.assertTrue(l.moveItemToBottom(item3))
        self.assertEqual(item1.zValue(), 2)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 1)

        self.assertTrue(l.moveItemToBottom(item2))
        self.assertEqual(item1.zValue(), 3)
        self.assertEqual(item2.zValue(), 1)
        self.assertEqual(item3.zValue(), 2)


if __name__ == '__main__':
    unittest.main()
