# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutView.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '05/07/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt import sip
from qgis.core import (QgsProject,
                       QgsLayout,
                       QgsUnitTypes,
                       QgsLayoutItemPicture,
                       QgsLayoutItemLabel,
                       QgsLayoutItemHtml,
                       QgsLayoutItemRegistry,
                       QgsLayoutFrame,
                       QgsLayoutPoint,
                       QgsLayoutSize,
                       QgsLayoutAligner)
from qgis.gui import QgsLayoutView
from qgis.PyQt.QtCore import QRectF, QMimeData, QByteArray
from qgis.PyQt.QtGui import QTransform
from qgis.PyQt.QtWidgets import QApplication
from qgis.PyQt.QtTest import QSignalSpy

from qgis.testing import start_app, unittest

start_app()


class TestQgsLayoutView(unittest.TestCase):

    def testScaleSafe(self):
        """ test scaleSafe method """

        view = QgsLayoutView()
        view.fitInView(QRectF(0, 0, 10, 10))
        scale = view.transform().m11()
        view.scaleSafe(2)
        self.assertAlmostEqual(view.transform().m11(), 2)
        view.scaleSafe(4)
        self.assertAlmostEqual(view.transform().m11(), 8)

        # try to zoom in heaps
        view.scaleSafe(99999999)
        # assume we have hit the limit
        scale = view.transform().m11()
        view.scaleSafe(2)
        self.assertAlmostEqual(view.transform().m11(), scale)

        view.setTransform(QTransform.fromScale(1, 1))
        self.assertAlmostEqual(view.transform().m11(), 1)
        # test zooming out
        view.scaleSafe(0.5)
        self.assertAlmostEqual(view.transform().m11(), 0.5)
        view.scaleSafe(0.1)
        self.assertAlmostEqual(view.transform().m11(), 0.05)

        # try zooming out heaps
        view.scaleSafe(0.000000001)
        # assume we have hit the limit
        scale = view.transform().m11()
        view.scaleSafe(0.5)
        self.assertAlmostEqual(view.transform().m11(), scale)

    def testLayoutScalePixels(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.setUnits(QgsUnitTypes.LayoutPixels)
        view = QgsLayoutView()
        view.setCurrentLayout(l)
        view.setZoomLevel(1)
        # should be no transform, since 100% with pixel units should be pixel-pixel
        self.assertEqual(view.transform().m11(), 1)
        view.setZoomLevel(0.5)
        self.assertEqual(view.transform().m11(), 0.5)

    def testSelectAll(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemPicture(l)
        l.addItem(item1)
        item2 = QgsLayoutItemPicture(l)
        l.addItem(item2)
        item3 = QgsLayoutItemPicture(l)
        item3.setLocked(True)
        l.addItem(item3)

        view = QgsLayoutView()
        # no layout, no crash
        view.selectAll()

        view.setCurrentLayout(l)

        focused_item_spy = QSignalSpy(view.itemFocused)

        view.selectAll()
        self.assertTrue(item1.isSelected())
        self.assertTrue(item2.isSelected())
        self.assertFalse(item3.isSelected()) # locked

        self.assertEqual(len(focused_item_spy), 1)

        item3.setSelected(True) # locked item selection should be cleared
        view.selectAll()
        self.assertTrue(item1.isSelected())
        self.assertTrue(item2.isSelected())
        self.assertFalse(item3.isSelected())  # locked

    def testDeselectAll(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemPicture(l)
        l.addItem(item1)
        item2 = QgsLayoutItemPicture(l)
        l.addItem(item2)
        item3 = QgsLayoutItemPicture(l)
        item3.setLocked(True)
        l.addItem(item3)

        view = QgsLayoutView()
        # no layout, no crash
        view.deselectAll()

        view.setCurrentLayout(l)

        focused_item_spy = QSignalSpy(view.itemFocused)

        view.deselectAll()
        self.assertFalse(item1.isSelected())
        self.assertFalse(item2.isSelected())
        self.assertFalse(item3.isSelected())

        self.assertEqual(len(focused_item_spy), 1)

        item1.setSelected(True)
        item2.setSelected(True)
        item3.setSelected(True)
        view.deselectAll()
        self.assertFalse(item1.isSelected())
        self.assertFalse(item2.isSelected())
        self.assertFalse(item3.isSelected())

    def testInvertSelection(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemPicture(l)
        l.addItem(item1)
        item2 = QgsLayoutItemPicture(l)
        l.addItem(item2)
        item3 = QgsLayoutItemPicture(l)
        item3.setLocked(True)
        l.addItem(item3)

        view = QgsLayoutView()
        # no layout, no crash
        view.invertSelection()

        view.setCurrentLayout(l)

        focused_item_spy = QSignalSpy(view.itemFocused)

        view.invertSelection()
        self.assertTrue(item1.isSelected())
        self.assertTrue(item2.isSelected())
        self.assertFalse(item3.isSelected()) # locked

        self.assertEqual(len(focused_item_spy), 1)

        item3.setSelected(True) # locked item selection should be cleared
        view.invertSelection()
        self.assertFalse(item1.isSelected())
        self.assertFalse(item2.isSelected())
        self.assertFalse(item3.isSelected())  # locked

    def testSelectNextByZOrder(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemPicture(l)
        l.addItem(item1)
        item2 = QgsLayoutItemPicture(l)
        l.addItem(item2)
        item3 = QgsLayoutItemPicture(l)
        item3.setLocked(True)
        l.addItem(item3)

        view = QgsLayoutView()
        # no layout, no crash
        view.selectNextItemAbove()
        view.selectNextItemBelow()

        view.setCurrentLayout(l)

        focused_item_spy = QSignalSpy(view.itemFocused)

        # no selection
        view.selectNextItemAbove()
        view.selectNextItemBelow()
        self.assertEqual(len(focused_item_spy), 0)

        l.setSelectedItem(item1)
        self.assertEqual(len(focused_item_spy), 1)
        # already bottom most
        view.selectNextItemBelow()
        self.assertTrue(item1.isSelected())
        self.assertFalse(item2.isSelected())
        self.assertFalse(item3.isSelected())
        self.assertEqual(len(focused_item_spy), 1)

        view.selectNextItemAbove()
        self.assertFalse(item1.isSelected())
        self.assertTrue(item2.isSelected())
        self.assertFalse(item3.isSelected())
        self.assertEqual(len(focused_item_spy), 2)

        view.selectNextItemAbove()
        self.assertFalse(item1.isSelected())
        self.assertFalse(item2.isSelected())
        self.assertTrue(item3.isSelected())
        self.assertEqual(len(focused_item_spy), 3)

        view.selectNextItemAbove() # already top most
        self.assertFalse(item1.isSelected())
        self.assertFalse(item2.isSelected())
        self.assertTrue(item3.isSelected())
        self.assertEqual(len(focused_item_spy), 3)

        view.selectNextItemBelow()
        self.assertFalse(item1.isSelected())
        self.assertTrue(item2.isSelected())
        self.assertFalse(item3.isSelected())
        self.assertEqual(len(focused_item_spy), 4)

        view.selectNextItemBelow()
        self.assertTrue(item1.isSelected())
        self.assertFalse(item2.isSelected())
        self.assertFalse(item3.isSelected())
        self.assertEqual(len(focused_item_spy), 5)

        view.selectNextItemBelow() # back to bottom most
        self.assertTrue(item1.isSelected())
        self.assertFalse(item2.isSelected())
        self.assertFalse(item3.isSelected())
        self.assertEqual(len(focused_item_spy), 5)

    def testLockActions(self):
        p = QgsProject()
        l = QgsLayout(p)

        view = QgsLayoutView()
        view.setCurrentLayout(l)

        # add some items
        item1 = QgsLayoutItemPicture(l)
        l.addItem(item1)
        item2 = QgsLayoutItemPicture(l)
        l.addItem(item2)
        item3 = QgsLayoutItemPicture(l)
        l.addItem(item3)

        item1.setLocked(True)
        item3.setLocked(True)
        self.assertTrue(item1.isLocked())
        self.assertFalse(item2.isLocked())
        self.assertTrue(item3.isLocked())

        view.unlockAllItems()
        self.assertFalse(item1.isLocked())
        self.assertFalse(item2.isLocked())
        self.assertFalse(item3.isLocked())
        self.assertTrue(item1.isSelected())
        self.assertFalse(item2.isSelected())
        self.assertTrue(item3.isSelected())

        view.lockSelectedItems()
        self.assertTrue(item1.isLocked())
        self.assertFalse(item2.isLocked())
        self.assertTrue(item3.isLocked())
        self.assertFalse(item1.isSelected())
        self.assertFalse(item2.isSelected())
        self.assertFalse(item3.isSelected())

    def testStacking(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemPicture(l)
        l.addLayoutItem(item1)
        item2 = QgsLayoutItemPicture(l)
        l.addLayoutItem(item2)
        item3 = QgsLayoutItemPicture(l)
        l.addLayoutItem(item3)

        view = QgsLayoutView()
        view.setCurrentLayout(l)

        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 2)
        self.assertEqual(item3.zValue(), 3)

        # no effect interactions
        view.raiseSelectedItems()
        view.lowerSelectedItems()
        view.moveSelectedItemsToTop()
        view.moveSelectedItemsToBottom()

        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 2)
        self.assertEqual(item3.zValue(), 3)

        # raising
        item3.setSelected(True)
        view.raiseSelectedItems()
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 2)
        self.assertEqual(item3.zValue(), 3)

        item3.setSelected(False)
        item2.setSelected(True)
        view.raiseSelectedItems()
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 2)

        view.raiseSelectedItems()
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 2)

        item2.setSelected(False)
        item1.setSelected(True)
        view.raiseSelectedItems()
        self.assertEqual(item1.zValue(), 2)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 1)

        # lower
        item1.setSelected(False)
        item3.setSelected(True)
        view.lowerSelectedItems()
        self.assertEqual(item1.zValue(), 2)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 1)

        item3.setSelected(False)
        item2.setSelected(True)
        view.lowerSelectedItems()
        self.assertEqual(item1.zValue(), 3)
        self.assertEqual(item2.zValue(), 2)
        self.assertEqual(item3.zValue(), 1)

        view.lowerSelectedItems()
        self.assertEqual(item1.zValue(), 3)
        self.assertEqual(item2.zValue(), 1)
        self.assertEqual(item3.zValue(), 2)

        # raise to top
        item2.setSelected(False)
        item1.setSelected(True)
        view.moveSelectedItemsToTop()
        self.assertEqual(item1.zValue(), 3)
        self.assertEqual(item2.zValue(), 1)
        self.assertEqual(item3.zValue(), 2)

        item1.setSelected(False)
        item3.setSelected(True)
        view.moveSelectedItemsToTop()
        self.assertEqual(item1.zValue(), 2)
        self.assertEqual(item2.zValue(), 1)
        self.assertEqual(item3.zValue(), 3)

        item3.setSelected(False)
        item2.setSelected(True)
        view.moveSelectedItemsToTop()
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 2)

        # move to bottom
        item2.setSelected(False)
        item1.setSelected(True)
        view.moveSelectedItemsToBottom()
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 2)

        item1.setSelected(False)
        item3.setSelected(True)
        view.moveSelectedItemsToBottom()
        self.assertEqual(item1.zValue(), 2)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 1)

        item3.setSelected(False)
        item2.setSelected(True)
        view.moveSelectedItemsToBottom()
        self.assertEqual(item1.zValue(), 3)
        self.assertEqual(item2.zValue(), 1)
        self.assertEqual(item3.zValue(), 2)

    def testAlign(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemPicture(l)
        item1.attemptMove(QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        item1.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item1)
        item2 = QgsLayoutItemPicture(l)
        item2.attemptMove(QgsLayoutPoint(6, 10, QgsUnitTypes.LayoutMillimeters))
        item2.attemptResize(QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item2)
        item3 = QgsLayoutItemPicture(l)
        item3.attemptMove(QgsLayoutPoint(0.8, 1.2, QgsUnitTypes.LayoutCentimeters))
        item3.attemptResize(QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))
        l.addItem(item3)

        view = QgsLayoutView()
        view.setCurrentLayout(l)

        view.alignSelectedItems(QgsLayoutAligner.AlignLeft)

        item1.setSelected(True)
        item2.setSelected(True)
        item3.setSelected(True)

        view.alignSelectedItems(QgsLayoutAligner.AlignLeft)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(4, 10, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 1.2, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        view.alignSelectedItems(QgsLayoutAligner.AlignHCenter)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(8, 10, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 1.2, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        view.alignSelectedItems(QgsLayoutAligner.AlignRight)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(12, 10, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 1.2, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        view.alignSelectedItems(QgsLayoutAligner.AlignTop)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(12, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 0.8, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        view.alignSelectedItems(QgsLayoutAligner.AlignVCenter)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 10, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(12, 11.5, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 0.8, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        view.alignSelectedItems(QgsLayoutAligner.AlignBottom)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(12, 15, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 0.8, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

    def testDistribute(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemPicture(l)
        item1.attemptMove(QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        item1.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item1)
        item2 = QgsLayoutItemPicture(l)
        item2.attemptMove(QgsLayoutPoint(7, 10, QgsUnitTypes.LayoutMillimeters))
        item2.attemptResize(QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item2)
        item3 = QgsLayoutItemPicture(l)
        item3.attemptMove(QgsLayoutPoint(0.8, 1.2, QgsUnitTypes.LayoutCentimeters))
        item3.attemptResize(QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))
        l.addItem(item3)

        view = QgsLayoutView()
        view.setCurrentLayout(l)

        view.distributeSelectedItems(QgsLayoutAligner.DistributeLeft)

        item1.setSelected(True)
        item2.setSelected(True)
        item3.setSelected(True)

        view.distributeSelectedItems(QgsLayoutAligner.DistributeLeft)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item2.positionWithUnits().x(), 6.0, 3)
        self.assertEqual(item2.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item3.positionWithUnits().x(), 0.8, 3)
        self.assertEqual(item3.positionWithUnits().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        view.distributeSelectedItems(QgsLayoutAligner.DistributeHCenter)
        self.assertAlmostEqual(item1.positionWithUnits().x(), 5.0, 3)
        self.assertEqual(item1.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item2.positionWithUnits().x(), 6.0, 3)
        self.assertEqual(item2.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item3.positionWithUnits().x(), 0.8, 3)
        self.assertEqual(item3.positionWithUnits().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        view.distributeSelectedItems(QgsLayoutAligner.DistributeRight)
        self.assertAlmostEqual(item1.positionWithUnits().x(), 3.0, 3)
        self.assertEqual(item1.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item2.positionWithUnits().x(), 6.0, 3)
        self.assertEqual(item2.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item3.positionWithUnits().x(), 0.8, 3)
        self.assertEqual(item3.positionWithUnits().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        view.distributeSelectedItems(QgsLayoutAligner.DistributeTop)
        self.assertAlmostEqual(item1.positionWithUnits().y(), 8.0, 3)
        self.assertEqual(item1.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item2.positionWithUnits().y(), 10.0, 3)
        self.assertEqual(item2.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item3.positionWithUnits().y(), 1.2, 3)
        self.assertEqual(item3.positionWithUnits().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        view.distributeSelectedItems(QgsLayoutAligner.DistributeVCenter)
        self.assertAlmostEqual(item1.positionWithUnits().y(), 8.0, 3)
        self.assertEqual(item1.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item2.positionWithUnits().y(), 12.5, 3)
        self.assertEqual(item2.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item3.positionWithUnits().y(), 1.2, 3)
        self.assertEqual(item3.positionWithUnits().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        view.distributeSelectedItems(QgsLayoutAligner.DistributeBottom)
        self.assertAlmostEqual(item1.positionWithUnits().y(), 8.0, 3)
        self.assertEqual(item1.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item2.positionWithUnits().y(), 15.0, 3)
        self.assertEqual(item2.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item3.positionWithUnits().y(), 1.2, 3)
        self.assertEqual(item3.positionWithUnits().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

    def testResize(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemPicture(l)
        item1.attemptMove(QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        item1.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item1)
        item2 = QgsLayoutItemPicture(l)
        item2.attemptMove(QgsLayoutPoint(7, 10, QgsUnitTypes.LayoutMillimeters))
        item2.attemptResize(QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item2)
        item3 = QgsLayoutItemPicture(l)
        item3.attemptMove(QgsLayoutPoint(0.8, 1.2, QgsUnitTypes.LayoutCentimeters))
        item3.attemptResize(QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))
        l.addItem(item3)

        view = QgsLayoutView()
        view.setCurrentLayout(l)

        view.resizeSelectedItems(QgsLayoutAligner.ResizeNarrowest)

        item1.setSelected(True)
        item2.setSelected(True)
        item3.setSelected(True)

        view.resizeSelectedItems(QgsLayoutAligner.ResizeNarrowest)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(10, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.0, 1.6, QgsUnitTypes.LayoutCentimeters))
        l.undoStack().stack().undo()

        view.resizeSelectedItems(QgsLayoutAligner.ResizeWidest)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(18, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))
        l.undoStack().stack().undo()

        view.resizeSelectedItems(QgsLayoutAligner.ResizeShortest)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 0.9, QgsUnitTypes.LayoutCentimeters))
        l.undoStack().stack().undo()

        view.resizeSelectedItems(QgsLayoutAligner.ResizeTallest)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 16, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 16, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))
        l.undoStack().stack().undo()

        item2.attemptResize(QgsLayoutSize(10, 19, QgsUnitTypes.LayoutMillimeters))
        view.resizeSelectedItems(QgsLayoutAligner.ResizeToSquare)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 18, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(19, 19, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.8, QgsUnitTypes.LayoutCentimeters))

    def testDeleteItems(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemLabel(l)
        item1.setText('label 1')
        l.addLayoutItem(item1)
        item2 = QgsLayoutItemLabel(l)
        item2.setText('label 2')
        l.addLayoutItem(item2)
        item3 = QgsLayoutItemLabel(l)
        item3.setText('label 2')
        l.addLayoutItem(item3)

        view = QgsLayoutView()
        view.setCurrentLayout(l)
        count_before = len(l.items())
        view.deleteSelectedItems()
        self.assertEqual(len(l.items()), count_before)

        item2.setSelected(True)
        view.deleteSelectedItems()
        self.assertEqual(len(l.items()), count_before - 1)
        self.assertIn(item1, l.items())
        self.assertIn(item3, l.items())
        view.deleteItems([item3])
        self.assertEqual(len(l.items()), count_before - 2)
        self.assertIn(item1, l.items())

    def testCopyPaste(self):
        p = QgsProject()
        l = QgsLayout(p)

        # clear clipboard
        mime_data = QMimeData()
        mime_data.setData("text/xml", QByteArray())
        clipboard = QApplication.clipboard()
        clipboard.setMimeData(mime_data)

        # add an item
        item1 = QgsLayoutItemLabel(l)
        item1.setText('label 1')
        l.addLayoutItem(item1)
        item1.setSelected(True)
        item2 = QgsLayoutItemLabel(l)
        item2.setText('label 2')
        l.addLayoutItem(item2)
        item2.setSelected(True)

        # multiframes
        multiframe1 = QgsLayoutItemHtml(l)
        multiframe1.setHtml('mf1')
        l.addMultiFrame(multiframe1)
        frame1 = QgsLayoutFrame(l, multiframe1)
        frame1.setId('frame1a')
        multiframe1.addFrame(frame1)
        frame1b = QgsLayoutFrame(l, multiframe1)
        frame1b.setId('frame1b')
        multiframe1.addFrame(frame1b) # not selected
        frame1c = QgsLayoutFrame(l, multiframe1)
        frame1c.setId('frame1b')
        multiframe1.addFrame(frame1c) # not selected

        multiframe2 = QgsLayoutItemHtml(l)
        multiframe2.setHtml('mf2')
        l.addMultiFrame(multiframe2)
        frame2 = QgsLayoutFrame(l, multiframe2)
        frame2.setId('frame2')
        multiframe2.addFrame(frame2)

        frame1.setSelected(True)
        frame2.setSelected(True)

        view = QgsLayoutView()
        view.setCurrentLayout(l)
        self.assertFalse(view.hasItemsInClipboard())

        view.copySelectedItems(QgsLayoutView.ClipboardCopy)
        self.assertTrue(view.hasItemsInClipboard())

        pasted = view.pasteItems(QgsLayoutView.PasteModeCursor)
        self.assertEqual(len(pasted), 4)

        new_multiframes = [m for m in l.multiFrames() if m not in [multiframe1, multiframe2]]
        self.assertEqual(len(new_multiframes), 2)

        self.assertIn(pasted[0], l.items())
        self.assertIn(pasted[1], l.items())
        labels = [p for p in pasted if p.type() == QgsLayoutItemRegistry.LayoutLabel]
        self.assertIn(sip.cast(labels[0], QgsLayoutItemLabel).text(), ('label 1', 'label 2'))
        self.assertIn(sip.cast(labels[1], QgsLayoutItemLabel).text(), ('label 1', 'label 2'))
        frames = [p for p in pasted if p.type() == QgsLayoutItemRegistry.LayoutFrame]
        pasted_frame1 = sip.cast(frames[0], QgsLayoutFrame)
        pasted_frame2 = sip.cast(frames[1], QgsLayoutFrame)
        self.assertIn(pasted_frame1.multiFrame(), new_multiframes)
        self.assertIn(new_multiframes[0].frames()[0].uuid(), (pasted_frame1.uuid(), pasted_frame2.uuid()))
        self.assertIn(pasted_frame2.multiFrame(), new_multiframes)
        self.assertIn(new_multiframes[1].frames()[0].uuid(), (pasted_frame1.uuid(), pasted_frame2.uuid()))

        self.assertEqual(frame1.multiFrame(), multiframe1)
        self.assertCountEqual(multiframe1.frames(), [frame1, frame1b, frame1c])
        self.assertEqual(frame1b.multiFrame(), multiframe1)
        self.assertEqual(frame1c.multiFrame(), multiframe1)
        self.assertEqual(frame2.multiFrame(), multiframe2)
        self.assertCountEqual(multiframe2.frames(), [frame2])

        # copy specific item
        view.copyItems([item2], QgsLayoutView.ClipboardCopy)
        l2 = QgsLayout(p)
        view2 = QgsLayoutView()
        view2.setCurrentLayout(l2)
        pasted = view2.pasteItems(QgsLayoutView.PasteModeCursor)
        self.assertEqual(len(pasted), 1)
        self.assertIn(pasted[0], l2.items())
        self.assertEqual(sip.cast(pasted[0], QgsLayoutItemLabel).text(), 'label 2')

    def testCutPaste(self):
        p = QgsProject()
        l = QgsLayout(p)

        # clear clipboard
        mime_data = QMimeData()
        mime_data.setData("text/xml", QByteArray())
        clipboard = QApplication.clipboard()
        clipboard.setMimeData(mime_data)

        # add an item
        item1 = QgsLayoutItemLabel(l)
        item1.setText('label 1')
        l.addLayoutItem(item1)
        item1.setSelected(True)
        item2 = QgsLayoutItemLabel(l)
        item2.setText('label 2')
        l.addLayoutItem(item2)
        item2.setSelected(True)

        view = QgsLayoutView()
        view.setCurrentLayout(l)
        self.assertFalse(view.hasItemsInClipboard())

        len_before = len(l.items())
        view.copySelectedItems(QgsLayoutView.ClipboardCut)
        self.assertTrue(view.hasItemsInClipboard())
        self.assertEqual(len(l.items()), len_before - 2)

        pasted = view.pasteItems(QgsLayoutView.PasteModeCursor)
        self.assertEqual(len(pasted), 2)
        self.assertEqual(len(l.items()), len_before)
        self.assertIn(pasted[0], l.items())
        self.assertIn(pasted[1], l.items())
        self.assertIn(sip.cast(pasted[0], QgsLayoutItemLabel).text(), ('label 1', 'label 2'))
        self.assertIn(sip.cast(pasted[1], QgsLayoutItemLabel).text(), ('label 1', 'label 2'))


if __name__ == '__main__':
    unittest.main()
