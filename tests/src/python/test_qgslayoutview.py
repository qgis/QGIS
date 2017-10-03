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
                       QgsLayoutItemMap)
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
        item1 = QgsLayoutItemMap(l)
        l.addItem(item1)
        item2 = QgsLayoutItemMap(l)
        l.addItem(item2)
        item3 = QgsLayoutItemMap(l)
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
        item1 = QgsLayoutItemMap(l)
        l.addItem(item1)
        item2 = QgsLayoutItemMap(l)
        l.addItem(item2)
        item3 = QgsLayoutItemMap(l)
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
        item1 = QgsLayoutItemMap(l)
        l.addItem(item1)
        item2 = QgsLayoutItemMap(l)
        l.addItem(item2)
        item3 = QgsLayoutItemMap(l)
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
        item1 = QgsLayoutItemMap(l)
        l.addItem(item1)
        item2 = QgsLayoutItemMap(l)
        l.addItem(item2)
        item3 = QgsLayoutItemMap(l)
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
        item1 = QgsLayoutItemMap(l)
        l.addItem(item1)
        item2 = QgsLayoutItemMap(l)
        l.addItem(item2)
        item3 = QgsLayoutItemMap(l)
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


if __name__ == '__main__':
    unittest.main()
