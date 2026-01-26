"""QGIS Unit tests for QgsFilterLineEdit

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "20/08/2016"
__copyright__ = "Copyright 2016, The QGIS Project"


from qgis.gui import QgsFilterLineEdit

from qgis.PyQt.QtTest import QSignalSpy

import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsFilterLineEdit(QgisTestCase):

    def testGettersSetters(self):
        """test widget getters/setters"""
        w = QgsFilterLineEdit()

        w.setNullValue("null")
        self.assertEqual(w.nullValue(), "null")
        w.setValue("value")
        self.assertEqual(w.value(), "value")
        self.assertEqual(w.text(), "value")
        w.setDefaultValue("default")
        self.assertEqual(w.defaultValue(), "default")
        w.setClearMode(QgsFilterLineEdit.ClearMode.ClearToDefault)
        self.assertEqual(w.clearMode(), QgsFilterLineEdit.ClearMode.ClearToDefault)
        w.setShowClearButton(False)
        self.assertFalse(w.showClearButton())
        w.setShowClearButton(True)
        self.assertTrue(w.showClearButton())

    def testNullValueHandling(self):
        """test widget handling of null values"""
        w = QgsFilterLineEdit()

        # start with no null value
        w.setValue(None)
        self.assertTrue(w.isNull())
        w.setValue("a")
        self.assertEqual(w.text(), "a")
        self.assertFalse(w.isNull())

        # set a null value
        w.setNullValue("null")
        self.assertEqual(w.value(), "a")
        self.assertEqual(w.text(), "a")
        self.assertFalse(w.isNull())

        w.setValue(None)
        self.assertTrue(w.isNull())
        self.assertFalse(w.value())
        self.assertEqual(w.text(), "null")

        w.setValue("null")
        self.assertEqual(w.text(), "null")
        # ND: I don't think this following logic is correct - should be a distinction between
        # the widget's representation of null and the actual value. Ie isNull()
        # should be false and value() should return 'null'
        # in other words - if you break this test to match my desired behavior, feel free to remove it!
        self.assertTrue(w.isNull())
        self.assertFalse(w.value())

    def testClearToNull(self):
        """test clearing widget"""
        w = QgsFilterLineEdit()

        w.setValue("abc")
        w.clearValue()
        self.assertTrue(w.isNull())
        self.assertFalse(w.value())
        w.clearValue()
        self.assertTrue(w.isNull())
        self.assertFalse(w.value())

        w.setNullValue("def")
        w.setValue("abc")
        self.assertFalse(w.isNull())
        w.clearValue()
        self.assertEqual(w.text(), "def")
        self.assertTrue(w.isNull())
        self.assertFalse(w.value())

    def testClearToDefault(self):
        # test clearing to default value
        w = QgsFilterLineEdit()
        w.setClearMode(QgsFilterLineEdit.ClearMode.ClearToDefault)

        w.setValue("abc")
        w.clearValue()
        self.assertTrue(w.isNull())
        self.assertFalse(w.value())
        w.clearValue()
        self.assertTrue(w.isNull())
        self.assertFalse(w.value())

        w.setDefaultValue("def")
        w.setValue("abc")
        self.assertFalse(w.isNull())
        w.clearValue()
        self.assertEqual(w.value(), "def")
        self.assertEqual(w.text(), "def")
        self.assertFalse(w.isNull())

    def test_selectedText(self):
        """test that NULL value is selected on focus and not-null value is not"""
        w = QgsFilterLineEdit(nullValue="my_null_value")
        w.clearValue()
        self.assertEqual(w.selectedText(), "my_null_value")

        w.setValue("my new value")
        self.assertEqual(w.selectedText(), "")

        w.clearValue()
        self.assertEqual(w.selectedText(), "my_null_value")

    def test_ChangedSignals(self):
        """test that signals are correctly emitted when clearing"""

        w = QgsFilterLineEdit()

        cleared_spy = QSignalSpy(w.cleared)
        w.setValue("1")
        w.clearValue()

        self.assertEqual(len(cleared_spy), 1)


if __name__ == "__main__":
    unittest.main()
