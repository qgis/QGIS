"""QGIS Unit tests for QgsTextFragment.

Run with: ctest -V -R QgsTextFragment

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "12/05/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtGui import QColor
from qgis.core import QgsStringUtils, QgsTextCharacterFormat, QgsTextFragment
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsTextFragment(QgisTestCase):

    def testConstructors(self):
        # empty
        frag = QgsTextFragment()
        self.assertFalse(frag.text())

        fragment = QgsTextFragment("ludicrous gibs!")
        self.assertEqual(fragment.text(), "ludicrous gibs!")

    def testSetText(self):
        fragment = QgsTextFragment()
        fragment.setText("ludicrous gibs!")
        self.assertEqual(fragment.text(), "ludicrous gibs!")

    def test_is_tab(self):
        fragment = QgsTextFragment()
        self.assertFalse(fragment.isTab())
        fragment.setText("abc")
        self.assertFalse(fragment.isTab())
        fragment.setText("abc\tdef")
        self.assertFalse(fragment.isTab())
        fragment.setText("\t")
        self.assertTrue(fragment.isTab())

    def test_is_whitespace(self):
        fragment = QgsTextFragment()
        self.assertTrue(fragment.isWhitespace())
        fragment.setText("abc")
        self.assertFalse(fragment.isWhitespace())
        fragment.setText(" a bc  ")
        self.assertFalse(fragment.isWhitespace())
        fragment.setText("abc\tdef")
        self.assertFalse(fragment.isWhitespace())
        fragment.setText("\t")
        self.assertTrue(fragment.isWhitespace())
        fragment.setText("    ")
        self.assertTrue(fragment.isWhitespace())
        fragment.setText("\t  ")
        self.assertTrue(fragment.isWhitespace())

    def testSetCharacterFormat(self):
        fragment = QgsTextFragment("a")

        self.assertFalse(fragment.characterFormat().textColor().isValid())
        format = QgsTextCharacterFormat()
        format.setTextColor(QColor(255, 0, 0))
        fragment.setCharacterFormat(format)
        self.assertTrue(fragment.characterFormat().textColor().isValid())
        self.assertEqual(fragment.characterFormat().textColor().name(), "#ff0000")

    def testCapitalize(self):
        fragment = QgsTextFragment("ludicrous gibs!")
        fragment.applyCapitalization(QgsStringUtils.Capitalization.TitleCase)
        self.assertEqual(fragment.text(), "Ludicrous Gibs!")


if __name__ == "__main__":
    unittest.main()
