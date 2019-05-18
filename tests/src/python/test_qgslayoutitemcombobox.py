# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutItemComboBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2019 by Nyall Dawson'
__date__ = '11/03/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsProject,
                       QgsPrintLayout,
                       QgsLayoutItemMap,
                       QgsLayoutItemLabel,
                       QgsLayoutItemRegistry)
from qgis.gui import QgsLayoutItemComboBox
from qgis.PyQt.QtCore import Qt, QModelIndex
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgis.PyQt.QtTest import QSignalSpy

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutItemComboBox(unittest.TestCase):

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
        self.assertEqual(combo.itemText(0), '')

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
        label1.setId('llll')
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
        self.assertEqual(combo.itemText(0), 'llll')
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
        self.assertEqual(combo.itemText(0), '')
        self.assertEqual(combo.itemText(1), 'llll')
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
        label2.setId('mmmm')
        layout.addLayoutItem(label2)
        self.assertEqual(combo.currentIndex(), 1)
        self.assertEqual(combo.count(), 3)
        self.assertEqual(combo.itemText(0), '')
        self.assertEqual(combo.itemText(1), 'llll')
        self.assertEqual(combo.itemText(2), 'mmmm')
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

        label1.setId('nnnn')
        self.assertEqual(combo.itemText(0), '')
        self.assertEqual(combo.itemText(1), 'mmmm')
        self.assertEqual(combo.itemText(2), 'nnnn')
        self.assertIsNone(combo.item(0))
        self.assertEqual(combo.item(1), label2)
        self.assertEqual(combo.item(2), label1)
        self.assertEqual(combo.currentItem(), label1)

        combo.setAllowEmptyItem(False)
        self.assertEqual(combo.itemText(0), 'mmmm')
        self.assertEqual(combo.itemText(1), 'nnnn')
        self.assertEqual(combo.item(0), label2)
        self.assertEqual(combo.item(1), label1)
        self.assertEqual(combo.currentItem(), label1)

        combo.setItem(label2)
        label2.setId('oooo')
        self.assertEqual(combo.itemText(0), 'nnnn')
        self.assertEqual(combo.itemText(1), 'oooo')
        self.assertEqual(combo.item(0), label1)
        self.assertEqual(combo.item(1), label2)
        self.assertEqual(combo.currentItem(), label2)

        combo.setAllowEmptyItem(True)
        layout.removeLayoutItem(label1)
        self.assertEqual(combo.itemText(0), '')
        self.assertEqual(combo.itemText(1), 'oooo')
        self.assertIsNone(combo.item(0))
        self.assertEqual(combo.item(1), label2)
        self.assertEqual(combo.currentItem(), label2)

        map = QgsLayoutItemMap(layout)
        layout.addLayoutItem(map)
        map.setId('pppp')
        self.assertEqual(combo.itemText(0), '')
        self.assertEqual(combo.itemText(1), 'oooo')
        self.assertEqual(combo.itemText(2), 'pppp')
        self.assertIsNone(combo.item(0))
        self.assertEqual(combo.item(1), label2)
        self.assertEqual(combo.item(2), map)
        self.assertEqual(combo.currentItem(), label2)

        combo.setItemType(QgsLayoutItemRegistry.LayoutMap)
        self.assertEqual(combo.itemText(0), '')
        self.assertEqual(combo.itemText(1), 'pppp')
        self.assertIsNone(combo.item(0))
        self.assertEqual(combo.item(1), map)
        self.assertIsNone(combo.currentItem())

        combo.setItemType(QgsLayoutItemRegistry.LayoutLabel)
        self.assertEqual(combo.itemText(0), '')
        self.assertEqual(combo.itemText(1), 'oooo')
        self.assertIsNone(combo.item(0))
        self.assertEqual(combo.item(1), label2)
        self.assertIsNone(combo.currentItem())

        combo.setItemType(QgsLayoutItemRegistry.LayoutAttributeTable)
        self.assertEqual(combo.itemText(0), '')
        self.assertIsNone(combo.item(0))
        self.assertIsNone(combo.currentItem())

        combo.setAllowEmptyItem(False)
        self.assertEqual(combo.count(), 0)
        self.assertIsNone(combo.currentItem())
        self.assertEqual(combo.currentIndex(), -1)


if __name__ == '__main__':
    unittest.main()
