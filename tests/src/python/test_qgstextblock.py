"""QGIS Unit tests for QgsTextBlock.

Run with: ctest -V -R QgsTextBlock

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "12/05/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtGui import QColor

from qgis.core import (
    QgsStringUtils,
    QgsTextBlock,
    QgsTextFragment,
    QgsTextBlockFormat,
    QgsTextCharacterFormat,
)

import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsTextBlock(QgisTestCase):

    def testConstructors(self):
        # empty
        block = QgsTextBlock()
        self.assertEqual(len(block), 0)

        # single fragment block
        fragment = QgsTextFragment("ludicrous gibs!")
        block = QgsTextBlock(fragment)
        self.assertEqual(len(block), 1)
        self.assertEqual(block[0].text(), fragment.text())
        self.assertEqual(block.toPlainText(), "ludicrous gibs!")

    def test_format(self):
        block = QgsTextBlock()
        self.assertFalse(block.blockFormat().hasHorizontalAlignmentSet())
        format = QgsTextBlockFormat()
        format.setHasHorizontalAlignmentSet(True)
        block.setBlockFormat(format)
        self.assertTrue(block.blockFormat().hasHorizontalAlignmentSet())

    def testFromPlainText(self):
        block = QgsTextBlock.fromPlainText("abc def")
        self.assertEqual(len(block), 1)
        self.assertEqual(block[0].text(), "abc def")

        # with format
        char_format = QgsTextCharacterFormat()
        char_format.setTextColor(QColor(255, 0, 0))
        block = QgsTextBlock.fromPlainText("abc def", char_format)
        self.assertEqual(len(block), 1)
        self.assertEqual(block[0].text(), "abc def")
        self.assertTrue(block[0].characterFormat().textColor().isValid())
        self.assertEqual(block[0].characterFormat().textColor().name(), "#ff0000")

    def testFromPlainTextWithTabs(self):
        block = QgsTextBlock.fromPlainText("b c\td\t gah")
        self.assertEqual(block[0].text(), "b c")
        self.assertTrue(block[1].isTab())
        self.assertEqual(block[2].text(), "d")
        self.assertTrue(block[3].isTab())
        self.assertEqual(block[4].text(), " gah")

        block = QgsTextBlock.fromPlainText("b\t\tc\td")
        self.assertEqual(len(block), 6)
        self.assertEqual(block[0].text(), "b")
        self.assertTrue(block[1].isTab())
        self.assertTrue(block[2].isTab())
        self.assertEqual(block[3].text(), "c")
        self.assertTrue(block[4].isTab())
        self.assertEqual(block[5].text(), "d")

    def testAppend(self):
        block = QgsTextBlock()
        self.assertEqual(len(block), 0)

        frag = QgsTextFragment("a")
        block.append(frag)
        self.assertEqual(len(block), 1)
        self.assertEqual(block[0].text(), "a")
        frag = QgsTextFragment("b")
        block.append(frag)
        self.assertEqual(len(block), 2)
        self.assertEqual(block[0].text(), "a")
        self.assertEqual(block[1].text(), "b")

        self.assertEqual(block.toPlainText(), "ab")

    def testInsert(self):
        block = QgsTextBlock()
        self.assertEqual(len(block), 0)

        frag = QgsTextFragment("a")
        with self.assertRaises(IndexError):
            block.insert(-1, frag)
        with self.assertRaises(IndexError):
            block.insert(1, frag)
        self.assertEqual(len(block), 0)
        block.insert(0, frag)
        self.assertEqual(len(block), 1)
        self.assertEqual(block[0].text(), "a")
        frag = QgsTextFragment("b")

        block.insert(0, frag)
        self.assertEqual(len(block), 2)
        self.assertEqual(block[0].text(), "b")
        self.assertEqual(block[1].text(), "a")

        frag = QgsTextFragment("c")
        block.insert(1, frag)
        self.assertEqual(len(block), 3)
        self.assertEqual(block[0].text(), "b")
        self.assertEqual(block[1].text(), "c")
        self.assertEqual(block[2].text(), "a")

        frag = QgsTextFragment("d")
        with self.assertRaises(IndexError):
            block.insert(4, frag)
        block.insert(3, frag)
        self.assertEqual(len(block), 4)
        self.assertEqual(block[0].text(), "b")
        self.assertEqual(block[1].text(), "c")
        self.assertEqual(block[2].text(), "a")
        self.assertEqual(block[3].text(), "d")

    def testAt(self):
        block = QgsTextBlock()
        block.append(QgsTextFragment("a"))
        block.append(QgsTextFragment("b"))
        self.assertEqual(len(block), 2)

        self.assertEqual(block.at(0).text(), "a")
        self.assertEqual(block.at(1).text(), "b")
        with self.assertRaises(KeyError):
            block.at(2)
        with self.assertRaises(KeyError):
            block.at(-1)

        self.assertEqual(block[0].text(), "a")
        self.assertEqual(block[1].text(), "b")
        with self.assertRaises(IndexError):
            _ = block[2]

        self.assertEqual(block[-1].text(), "b")
        self.assertEqual(block[-2].text(), "a")

    def testClear(self):
        block = QgsTextBlock()
        block.append(QgsTextFragment("a"))
        block.append(QgsTextFragment("b"))
        self.assertEqual(len(block), 2)
        self.assertFalse(block.empty())

        block.clear()
        self.assertEqual(len(block), 0)
        self.assertTrue(block.empty())

    def testCapitalize(self):
        fragment = QgsTextFragment("ludicrous gibs!")
        block = QgsTextBlock(fragment)
        block.append(QgsTextFragment("another part"))
        block.applyCapitalization(QgsStringUtils.Capitalization.TitleCase)
        self.assertEqual(block.toPlainText(), "Ludicrous Gibs!Another Part")


if __name__ == "__main__":
    unittest.main()
