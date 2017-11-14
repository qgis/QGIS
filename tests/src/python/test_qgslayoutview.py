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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsProject,
                       QgsLayout,
                       QgsUnitTypes,
                       QgsLayoutItemPicture,
                       QgsLayoutPoint,
                       QgsLayoutSize,
                       QgsLayoutAligner)
from qgis.gui import QgsLayoutView
from qgis.PyQt.QtCore import QRectF
from qgis.PyQt.QtGui import QTransform
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


if __name__ == '__main__':
    unittest.main()
