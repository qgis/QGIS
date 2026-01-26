"""QGIS Unit tests for QgsLayoutItemComboBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2019 by Nyall Dawson"
__date__ = "11/03/2019"
__copyright__ = "Copyright 2019, The QGIS Project"

from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsLayoutItem,
    QgsLayoutItemLabel,
    QgsLayoutItemMap,
    QgsLayoutItemRegistry,
    QgsLayoutItemShape,
    QgsPrintLayout,
    QgsProject,
)
from qgis.gui import QgsLayoutItemComboBox
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutItemComboBox(QgisTestCase):

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def testCombo(self):
        project = QgsProject()
        layout = QgsPrintLayout(project)

        combo = QgsLayoutItemComboBox(None)
        spy = QSignalSpy(combo.itemChanged)
        self.assertEqual(combo.count(), 0)
        self.assertIsNone(combo.currentLayout())
        self.assertIsNone(combo.currentItem())
        self.assertIsNone(combo.item(0))
        self.assertIsNone(combo.item(-1))
        self.assertIsNone(combo.item(1))
        combo.setItem(None)
        self.assertIsNone(combo.currentItem())
        self.assertEqual(combo.currentIndex(), -1)
        self.assertEqual(len(spy), 0)

        combo.setCurrentLayout(layout)
        self.assertEqual(combo.currentIndex(), -1)
        self.assertEqual(len(spy), 0)
        self.assertEqual(combo.currentLayout(), layout)
        self.assertEqual(combo.count(), 0)
        self.assertIsNone(combo.currentItem())
        self.assertIsNone(combo.item(0))
        self.assertIsNone(combo.item(-1))
        self.assertIsNone(combo.item(1))
        combo.setItem(None)
        self.assertIsNone(combo.currentItem())
        self.assertEqual(len(spy), 0)

        combo.setAllowEmptyItem(True)
        self.assertEqual(combo.currentIndex(), 0)
        self.assertEqual(combo.count(), 1)
        self.assertEqual(len(spy), 2)
        self.assertEqual(combo.currentLayout(), layout)
        self.assertIsNone(combo.currentItem())
        self.assertIsNone(combo.item(0))
        self.assertIsNone(combo.item(-1))
        self.assertIsNone(combo.item(1))
        combo.setItem(None)
        self.assertIsNone(combo.currentItem())
        self.assertEqual(len(spy), 2)
        self.assertEqual(combo.currentIndex(), 0)
        self.assertEqual(combo.itemText(0), "")

        combo.setAllowEmptyItem(False)
        self.assertEqual(combo.currentIndex(), -1)
        self.assertEqual(combo.count(), 0)
        self.assertEqual(len(spy), 4)
        self.assertEqual(combo.currentLayout(), layout)
        self.assertIsNone(combo.currentItem())
        self.assertIsNone(combo.item(0))
        self.assertIsNone(combo.item(-1))
        self.assertIsNone(combo.item(1))
        combo.setItem(None)
        self.assertIsNone(combo.currentItem())
        self.assertEqual(len(spy), 4)
        self.assertEqual(combo.currentIndex(), -1)

        label1 = QgsLayoutItemLabel(layout)
        label1.setId("llll")
        # don't add to layout yet!
        combo.setItem(label1)
        self.assertIsNone(combo.currentItem())
        self.assertEqual(len(spy), 4)
        self.assertEqual(combo.currentIndex(), -1)
        combo.setAllowEmptyItem(True)
        combo.setItem(label1)
        self.assertIsNone(combo.currentItem())
        self.assertEqual(len(spy), 6)
        self.assertEqual(combo.currentIndex(), 0)
        combo.setAllowEmptyItem(False)

        layout.addLayoutItem(label1)
        self.assertEqual(combo.currentIndex(), 0)
        self.assertEqual(combo.count(), 1)
        self.assertEqual(combo.itemText(0), "llll")
        self.assertEqual(len(spy), 10)
        self.assertEqual(combo.currentLayout(), layout)
        self.assertEqual(combo.currentItem(), label1)
        self.assertEqual(combo.item(0), label1)
        self.assertIsNone(combo.item(-1))
        self.assertIsNone(combo.item(1))
        combo.setItem(None)
        self.assertIsNone(combo.currentItem())
        self.assertEqual(len(spy), 11)
        self.assertEqual(combo.currentIndex(), -1)
        combo.setItem(label1)
        self.assertEqual(combo.currentItem(), label1)
        self.assertEqual(len(spy), 12)
        self.assertEqual(combo.currentIndex(), 0)

        combo.setAllowEmptyItem(True)
        self.assertEqual(combo.currentIndex(), 1)
        self.assertEqual(combo.count(), 2)
        self.assertEqual(combo.itemText(0), "")
        self.assertEqual(combo.itemText(1), "llll")
        self.assertEqual(len(spy), 13)
        self.assertEqual(combo.currentLayout(), layout)
        self.assertEqual(combo.currentItem(), label1)
        self.assertIsNone(combo.item(0))
        self.assertEqual(combo.item(1), label1)
        self.assertIsNone(combo.item(-1))
        self.assertIsNone(combo.item(2))
        combo.setItem(None)
        self.assertIsNone(combo.currentItem())
        self.assertEqual(len(spy), 14)
        self.assertEqual(combo.currentIndex(), 0)
        combo.setItem(label1)
        self.assertEqual(combo.currentItem(), label1)
        self.assertEqual(len(spy), 15)
        self.assertEqual(combo.currentIndex(), 1)

        label2 = QgsLayoutItemLabel(layout)
        label2.setId("mmmm")
        layout.addLayoutItem(label2)
        self.assertEqual(combo.currentIndex(), 1)
        self.assertEqual(combo.count(), 3)
        self.assertEqual(combo.itemText(0), "")
        self.assertEqual(combo.itemText(1), "llll")
        self.assertEqual(combo.itemText(2), "mmmm")
        self.assertEqual(len(spy), 15)
        self.assertEqual(combo.currentLayout(), layout)
        self.assertEqual(combo.currentItem(), label1)
        self.assertIsNone(combo.item(0))
        self.assertEqual(combo.item(1), label1)
        self.assertEqual(combo.item(2), label2)
        self.assertIsNone(combo.item(-1))
        self.assertIsNone(combo.item(3))
        combo.setItem(None)
        self.assertIsNone(combo.currentItem())
        self.assertEqual(len(spy), 16)
        self.assertEqual(combo.currentIndex(), 0)
        combo.setItem(label1)
        self.assertEqual(combo.currentItem(), label1)
        self.assertEqual(len(spy), 17)
        self.assertEqual(combo.currentIndex(), 1)

        label1.setId("nnnn")
        self.assertEqual(combo.itemText(0), "")
        self.assertEqual(combo.itemText(1), "mmmm")
        self.assertEqual(combo.itemText(2), "nnnn")
        self.assertIsNone(combo.item(0))
        self.assertEqual(combo.item(1), label2)
        self.assertEqual(combo.item(2), label1)
        self.assertEqual(combo.currentItem(), label1)

        combo.setAllowEmptyItem(False)
        self.assertEqual(combo.itemText(0), "mmmm")
        self.assertEqual(combo.itemText(1), "nnnn")
        self.assertEqual(combo.item(0), label2)
        self.assertEqual(combo.item(1), label1)
        self.assertEqual(combo.currentItem(), label1)

        combo.setItem(label2)
        label2.setId("oooo")
        self.assertEqual(combo.itemText(0), "nnnn")
        self.assertEqual(combo.itemText(1), "oooo")
        self.assertEqual(combo.item(0), label1)
        self.assertEqual(combo.item(1), label2)
        self.assertEqual(combo.currentItem(), label2)

        combo.setAllowEmptyItem(True)
        layout.removeLayoutItem(label1)
        self.assertEqual(combo.itemText(0), "")
        self.assertEqual(combo.itemText(1), "oooo")
        self.assertIsNone(combo.item(0))
        self.assertEqual(combo.item(1), label2)
        self.assertEqual(combo.currentItem(), label2)

        map = QgsLayoutItemMap(layout)
        layout.addLayoutItem(map)
        map.setId("pppp")
        self.assertEqual(combo.itemText(0), "")
        self.assertEqual(combo.itemText(1), "oooo")
        self.assertEqual(combo.itemText(2), "pppp")
        self.assertIsNone(combo.item(0))
        self.assertEqual(combo.item(1), label2)
        self.assertEqual(combo.item(2), map)
        self.assertEqual(combo.currentItem(), label2)

        combo.setItemType(QgsLayoutItemRegistry.ItemType.LayoutMap)
        self.assertEqual(combo.itemText(0), "")
        self.assertEqual(combo.itemText(1), "pppp")
        self.assertIsNone(combo.item(0))
        self.assertEqual(combo.item(1), map)
        self.assertIsNone(combo.currentItem())

        combo.setItemType(QgsLayoutItemRegistry.ItemType.LayoutLabel)
        self.assertEqual(combo.itemText(0), "")
        self.assertEqual(combo.itemText(1), "oooo")
        self.assertIsNone(combo.item(0))
        self.assertEqual(combo.item(1), label2)
        self.assertIsNone(combo.currentItem())

        combo.setItemType(QgsLayoutItemRegistry.ItemType.LayoutAttributeTable)
        self.assertEqual(combo.itemText(0), "")
        self.assertIsNone(combo.item(0))
        self.assertIsNone(combo.currentItem())

        combo.setAllowEmptyItem(False)
        self.assertEqual(combo.count(), 0)
        self.assertIsNone(combo.currentItem())
        self.assertEqual(combo.currentIndex(), -1)

        combo.setItemType(QgsLayoutItemRegistry.ItemType.LayoutItem)
        self.assertEqual(combo.count(), 2)
        combo.setItemFlags(QgsLayoutItem.Flag.FlagProvidesClipPath)
        self.assertEqual(combo.count(), 0)
        shape = QgsLayoutItemShape(layout)
        shape.setId("shape 1")
        layout.addLayoutItem(shape)
        self.assertEqual(combo.count(), 1)
        shape2 = QgsLayoutItemShape(layout)
        shape2.setId("shape 2")
        layout.addLayoutItem(shape2)
        self.assertEqual(combo.count(), 2)
        self.assertEqual(combo.itemText(0), "shape 1")
        self.assertEqual(combo.itemText(1), "shape 2")
        combo.setItemFlags(QgsLayoutItem.Flags())
        self.assertEqual(combo.count(), 4)


if __name__ == "__main__":
    unittest.main()
