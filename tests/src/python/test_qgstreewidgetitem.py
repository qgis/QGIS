"""QGIS Unit tests for QgsTreeWidgetItem.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "12/07/2016"
__copyright__ = "Copyright 2016, The QGIS Project"


from qgis.core import NULL
from qgis.gui import QgsTreeWidgetItem, QgsTreeWidgetItemObject
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import QTreeWidget
import unittest
from qgis.testing import start_app, QgisTestCase

try:
    from qgis.PyQt.QtTest import QSignalSpy

    use_signal_spy = True
except:
    use_signal_spy = False

start_app()


class TestQgsTreeWidgetItem(QgisTestCase):

    def testGettersSetters(self):
        """test getters and setters"""
        i = QgsTreeWidgetItem()

        # sort data should be empty by default
        self.assertEqual(i.sortData(0), NULL)
        i.setSortData(0, "5")
        self.assertEqual(i.sortData(0), "5")
        self.assertEqual(i.sortData(1), NULL)
        i.setSortData(1, "a")
        self.assertEqual(i.sortData(0), "5")
        self.assertEqual(i.sortData(1), "a")

        # should not be always on top by default
        self.assertEqual(i.alwaysOnTopPriority(), -1)
        i.setAlwaysOnTopPriority(1)
        self.assertEqual(i.alwaysOnTopPriority(), 1)

    def testSort(self):
        """test sort logic"""
        w = QTreeWidget()

        i1 = QgsTreeWidgetItem(w)
        i2 = QgsTreeWidgetItem(w)

        # should default to search by display text
        i1.setText(0, "2")
        i1.setText(1, "b")
        i1.setText(2, "c")
        i2.setText(0, "1")
        i2.setText(1, "a")
        i2.setText(2, "d")

        w.sortItems(0, Qt.SortOrder.AscendingOrder)
        self.assertEqual(i1 < i2, False)
        self.assertEqual(i2 < i1, True)
        w.sortItems(1, Qt.SortOrder.AscendingOrder)
        self.assertEqual(i1 < i2, False)
        self.assertEqual(i2 < i1, True)
        w.sortItems(2, Qt.SortOrder.AscendingOrder)
        self.assertEqual(i1 < i2, True)
        self.assertEqual(i2 < i1, False)

        # sortData should take precedence over display text
        i1.setText(1, "2")
        i1.setSortData(1, "200")
        i2.setText(1, "3")
        w.sortItems(1, Qt.SortOrder.AscendingOrder)
        self.assertEqual(i1 < i2, False)
        self.assertEqual(i2 < i1, True)
        i2.setSortData(1, "300")
        self.assertEqual(i1 < i2, True)
        self.assertEqual(i2 < i1, False)

        # test that nulls are sorted before other values
        i1.setSortData(0, "2")
        i2.setSortData(0, NULL)
        w.sortItems(0, Qt.SortOrder.AscendingOrder)
        self.assertEqual(i1 < i2, False)
        self.assertEqual(i2 < i1, True)

        # test numeric sorting
        i1.setSortData(0, "02")
        i2.setSortData(0, "005")
        w.sortItems(0, Qt.SortOrder.AscendingOrder)
        self.assertEqual(i1 < i2, True)
        self.assertEqual(i2 < i1, False)
        # numbers should come first
        i2.setSortData(0, "a")
        self.assertEqual(i1 < i2, True)
        self.assertEqual(i2 < i1, False)
        i1.setSortData(0, "a")
        i2.setSortData(0, "5")
        self.assertEqual(i1 < i2, False)
        self.assertEqual(i2 < i1, True)

        # always on top items should be first
        i1.setSortData(0, "a")
        i2.setSortData(0, "b")
        i2.setAlwaysOnTopPriority(5)
        self.assertEqual(i1 < i2, False)
        self.assertEqual(i2 < i1, True)
        i1.setAlwaysOnTopPriority(3)
        self.assertEqual(i1 < i2, True)
        self.assertEqual(i2 < i1, False)
        # otherwise fall back to sort order
        i2.setAlwaysOnTopPriority(3)
        i1.setSortData(0, "c")
        self.assertEqual(i1 < i2, False)
        self.assertEqual(i2 < i1, True)


class TestQgsTreeWidgetItemObject(QgisTestCase):

    @unittest.skipIf(not use_signal_spy, "No QSignalSpy available")
    def testItemEdited(self):
        """test that itemEdited signal is correctly emitted"""

        i = QgsTreeWidgetItemObject()
        item_edited_spy = QSignalSpy(i.itemEdited)
        i.setData(1, Qt.ItemDataRole.EditRole, "a")
        self.assertEqual(len(item_edited_spy), 1)
        i.setData(1, Qt.ItemDataRole.EditRole, "b")
        self.assertEqual(len(item_edited_spy), 2)


if __name__ == "__main__":
    unittest.main()
