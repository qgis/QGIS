"""QGIS Unit tests for QgsFilterableComboBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.gui import QgsFilterableComboBox
from qgis.PyQt.QtCore import QEvent, QModelIndex, Qt
from qgis.PyQt.QtGui import QFocusEvent
from qgis.PyQt.QtTest import QSignalSpy, QTest
from qgis.PyQt.QtWidgets import QApplication, QLineEdit, QWidget
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsFilterableComboBox(QgisTestCase):
    def testGettersSetters(self):
        """test widget getters/setters"""
        w = QgsFilterableComboBox()
        self.assertTrue(w.isEditable())
        self.assertEqual(w.insertPolicy(), w.InsertPolicy.NoInsert)
        # should start with no search icon
        self.assertFalse(w.lineEdit().showSearchIcon())
        self.assertFalse(w.lineEdit().showClearButton())

        # basic QComboBox functionality tests
        spy = QSignalSpy(w.currentIndexChanged)
        w.addItems(["One", "Two", "Three"])
        self.assertEqual(len(spy), 1)
        self.assertEqual(w.currentIndex(), 0)

        w.clear()
        self.assertEqual(len(spy), 2)
        self.assertEqual(w.currentIndex(), -1)

    def test_filter_text_entry(self):
        """
        Test logic for filter text entry behavior
        """
        w = QgsFilterableComboBox()
        w.addItems(["One", "Two", "Three"])
        w.setCurrentIndex(1)

        w.lineEdit().setText("NonExistent")
        # simulate defocus of widget - we should revert back to the previous
        # selection, because the entered text is not a valid item
        w.lineEdit().editingFinished.emit()
        self.assertEqual(w.count(), 3)
        self.assertEqual(w.currentIndex(), 1)
        self.assertEqual(w.currentText(), "Two")

        w.lineEdit().setText("thRee")
        w.lineEdit().editingFinished.emit()
        self.assertEqual(w.count(), 3)
        self.assertEqual(w.currentIndex(), 2)
        self.assertEqual(w.currentText(), "Three")

    def test_substring_filtering(self):
        """
        Filters should be case-insensitive, allow partial match anywhere
        """
        w = QgsFilterableComboBox()
        w.addItems(["Apple", "pineapple", "BaNAna"])

        completer = w.completer()
        completer.setCompletionPrefix("app")
        self.assertEqual(completer.completionCount(), 2)
        completer.setCompletionPrefix("APPl")
        self.assertEqual(completer.completionCount(), 2)

        completer.setCompletionPrefix("ANA")
        self.assertEqual(completer.completionCount(), 1)
        self.assertEqual(completer.currentCompletion(), "BaNAna")

    def test_search_icon_visibility(self):
        """
        We only want the search icon to show while a filter string is being entered
        by the user
        """
        container = QWidget()
        w = QgsFilterableComboBox(container)
        w.addItems(["One", "Two", "Three"])
        container.show()
        container.activateWindow()
        self.assertTrue(QTest.qWaitForWindowActive(container, 1000))

        w.setCurrentIndex(1)
        self.assertFalse(w.lineEdit().showSearchIcon())

        w.lineEdit().setFocus(Qt.FocusReason.TabFocusReason)
        QApplication.processEvents()
        QTest.keyClicks(w.lineEdit(), "Sec")
        self.assertTrue(w.lineEdit().showSearchIcon())

        # simulate an entry being selected by the user, should remove icon
        w.completer().activated.emit("Second")
        self.assertFalse(w.lineEdit().showSearchIcon())

        QTest.keyClicks(w.lineEdit(), "Fir")
        self.assertTrue(w.lineEdit().showSearchIcon())
        w.lineEdit().clearFocus()
        QApplication.processEvents()
        self.assertFalse(w.lineEdit().showSearchIcon())

    def test_preselected_signal(self):
        w = QgsFilterableComboBox()
        w.addItem("five", 5)
        w.addItem("six", 6)
        w.addItem("seven", 7)

        spy = QSignalSpy(w.itemDataPreselected)

        w.highlighted.emit(1)
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[0][0], 6)

        w.completer().setCompletionPrefix("sev")
        proxy_index = w.completer().completionModel().index(0, 0)
        self.assertTrue(proxy_index.isValid())
        w.completer().highlighted[QModelIndex].emit(proxy_index)

        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[1][0], 7)

        w.completer().highlighted[QModelIndex].emit(QModelIndex())
        self.assertEqual(len(spy), 2)


if __name__ == "__main__":
    unittest.main()
