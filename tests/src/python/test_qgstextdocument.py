# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsTextDocument.

Run with: ctest -V -R QgsTextDocument

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '12/05/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (
    Qgis,
    QgsTextDocument,
    QgsTextBlock,
    QgsTextFragment,
    QgsTextCharacterFormat,
    QgsStringUtils
)
from qgis.testing import start_app, unittest

start_app()


class TestQgsTextDocument(unittest.TestCase):

    def testConstructors(self):
        # empty
        doc = QgsTextDocument()
        self.assertEqual(len(doc), 0)

        # single block document
        block = QgsTextBlock()
        doc = QgsTextDocument(block)
        self.assertEqual(len(doc), 1)
        self.assertEqual(len(doc[0]), 0)

        # single fragment document
        fragment = QgsTextFragment('ludicrous gibs!')
        doc = QgsTextDocument(fragment)
        self.assertEqual(len(doc), 1)
        self.assertEqual(len(doc[0]), 1)
        self.assertEqual(doc[0][0].text(), fragment.text())

    def testFromPlainText(self):
        doc = QgsTextDocument.fromPlainText(['a', 'b c d', 'e'])
        self.assertEqual(len(doc), 3)
        self.assertEqual(len(doc[0]), 1)
        self.assertEqual(doc[0][0].text(), 'a')
        self.assertEqual(len(doc[1]), 1)
        self.assertEqual(doc[1][0].text(), 'b c d')
        self.assertEqual(len(doc[2]), 1)
        self.assertEqual(doc[2][0].text(), 'e')

    def testFromHtml(self):
        doc = QgsTextDocument.fromHtml(['abc<div style="color: red"><b style="text-decoration: underline; font-style: italic; font-size: 15pt; font-family: Serif">def</b> ghi<div>jkl</div></div>', 'b c d', 'e'])
        self.assertEqual(len(doc), 5)
        self.assertEqual(len(doc[0]), 1)
        self.assertEqual(doc[0][0].text(), 'abc')
        self.assertEqual(doc[0][0].characterFormat().underline(), QgsTextCharacterFormat.BooleanValue.NotSet)
        self.assertEqual(doc[0][0].characterFormat().italic(), QgsTextCharacterFormat.BooleanValue.NotSet)
        self.assertEqual(doc[0][0].characterFormat().fontWeight(), -1)
        self.assertFalse(doc[0][0].characterFormat().family())
        self.assertEqual(doc[0][0].characterFormat().fontPointSize(), -1)
        self.assertFalse(doc[0][0].characterFormat().textColor().isValid())
        self.assertFalse(doc[0][0].characterFormat().hasVerticalAlignmentSet())
        self.assertEqual(len(doc[1]), 2)
        self.assertEqual(doc[1][0].text(), 'def')
        self.assertEqual(doc[1][0].characterFormat().underline(), QgsTextCharacterFormat.BooleanValue.SetTrue)
        self.assertEqual(doc[1][0].characterFormat().italic(), QgsTextCharacterFormat.BooleanValue.SetTrue)
        self.assertEqual(doc[1][0].characterFormat().fontWeight(), 75)
        self.assertEqual(doc[1][0].characterFormat().family(), 'Serif')
        self.assertEqual(doc[1][0].characterFormat().textColor().name(), '#ff0000')
        self.assertEqual(doc[1][0].characterFormat().fontPointSize(), 15)
        self.assertFalse(doc[1][0].characterFormat().hasVerticalAlignmentSet())
        self.assertEqual(doc[1][1].text(), ' ghi')
        self.assertEqual(doc[1][1].characterFormat().underline(), QgsTextCharacterFormat.BooleanValue.NotSet)
        self.assertEqual(doc[1][1].characterFormat().italic(), QgsTextCharacterFormat.BooleanValue.NotSet)
        self.assertEqual(doc[1][1].characterFormat().fontWeight(), -1)
        self.assertFalse(doc[1][1].characterFormat().family())
        self.assertEqual(doc[1][1].characterFormat().textColor().name(), '#ff0000')
        self.assertEqual(doc[1][1].characterFormat().fontPointSize(), -1)
        self.assertFalse(doc[1][1].characterFormat().hasVerticalAlignmentSet())
        self.assertEqual(len(doc[2]), 1)
        self.assertEqual(doc[2][0].text(), 'jkl')
        self.assertEqual(len(doc[3]), 1)
        self.assertEqual(doc[3][0].text(), 'b c d')
        self.assertEqual(len(doc[4]), 1)
        self.assertEqual(doc[4][0].text(), 'e')

    def testFromHtmlVerticalAlignment(self):
        doc = QgsTextDocument.fromHtml(['abc<div style="color: red"><sub>def<b>extra</b></sub> ghi</div><sup>sup</sup><span style="vertical-align: sub">css</span>'])
        self.assertEqual(len(doc), 3)
        self.assertEqual(len(doc[0]), 1)
        self.assertEqual(doc[0][0].text(), 'abc')
        self.assertFalse(doc[0][0].characterFormat().hasVerticalAlignmentSet())
        self.assertEqual(len(doc[1]), 3)
        self.assertEqual(doc[1][0].text(), 'def')
        self.assertTrue(doc[1][0].characterFormat().hasVerticalAlignmentSet())
        self.assertEqual(doc[1][0].characterFormat().verticalAlignment(), Qgis.TextCharacterVerticalAlignment.SubScript)
        self.assertEqual(doc[1][1].text(), 'extra')
        self.assertTrue(doc[1][1].characterFormat().hasVerticalAlignmentSet())
        self.assertEqual(doc[1][1].characterFormat().verticalAlignment(), Qgis.TextCharacterVerticalAlignment.SubScript)
        self.assertEqual(doc[1][2].text(), ' ghi')
        self.assertFalse(doc[1][2].characterFormat().hasVerticalAlignmentSet())
        self.assertEqual(len(doc[2]), 2)
        self.assertEqual(doc[2][0].text(), 'sup')
        self.assertTrue(doc[2][0].characterFormat().hasVerticalAlignmentSet())
        self.assertEqual(doc[2][0].characterFormat().verticalAlignment(), Qgis.TextCharacterVerticalAlignment.SuperScript)
        self.assertEqual(doc[2][1].text(), 'css')
        self.assertTrue(doc[2][1].characterFormat().hasVerticalAlignmentSet())
        self.assertEqual(doc[2][1].characterFormat().verticalAlignment(), Qgis.TextCharacterVerticalAlignment.SubScript)

    def testAppend(self):
        doc = QgsTextDocument()
        self.assertEqual(len(doc), 0)

        block = QgsTextBlock()
        doc.append(block)
        self.assertEqual(len(doc), 1)
        self.assertEqual(len(doc[0]), 0)

    def testAt(self):
        doc = QgsTextDocument()
        self.assertEqual(len(doc), 0)

        block = QgsTextBlock()
        block.append(QgsTextFragment('a'))
        doc.append(block)
        block = QgsTextBlock()
        block.append(QgsTextFragment('b'))
        doc.append(block)
        self.assertEqual(len(doc), 2)

        self.assertEqual(doc.at(0)[0].text(), 'a')
        self.assertEqual(doc.at(1)[0].text(), 'b')
        with self.assertRaises(KeyError):
            doc.at(2)
        with self.assertRaises(KeyError):
            doc.at(-1)

        self.assertEqual(doc[0][0].text(), 'a')
        self.assertEqual(doc[1][0].text(), 'b')
        with self.assertRaises(IndexError):
            _ = doc[2]

        self.assertEqual(doc[-1][0].text(), 'b')
        self.assertEqual(doc[-2][0].text(), 'a')

    def testToPlainText(self):
        self.assertEqual(QgsTextDocument.fromHtml(['']).toPlainText(), [])
        self.assertEqual(QgsTextDocument.fromHtml(['abc']).toPlainText(), ['abc'])
        self.assertEqual(QgsTextDocument.fromHtml(['abc\ndef']).toPlainText(), ['abc def'])
        self.assertEqual(QgsTextDocument.fromHtml(['abc<b>def</b>']).toPlainText(), ['abcdef'])
        self.assertEqual(QgsTextDocument.fromHtml(['abc<div><b>def</b><div>ghi</div></div>']).toPlainText(), ['abc', 'def', 'ghi'])

    def testSplitLines(self):
        doc = QgsTextDocument.fromHtml(['abc def'])
        self.assertEqual(len(doc), 1)
        self.assertEqual(len(doc[0]), 1)
        self.assertEqual(doc[0][0].text(), 'abc def')
        doc.splitLines(' ')
        self.assertEqual(len(doc), 2)
        self.assertEqual(len(doc[0]), 1)
        self.assertEqual(doc[0][0].text(), 'abc')
        self.assertEqual(len(doc[1]), 1)
        self.assertEqual(doc[1][0].text(), 'def')

        doc = QgsTextDocument.fromHtml(['<span style="color: red">R_ED</span> not <div>red</div>'])
        self.assertEqual(len(doc), 2)
        self.assertEqual(len(doc[0]), 2)
        self.assertEqual(doc[0][0].text(), 'R_ED')
        self.assertEqual(doc[0][1].text(), ' not ')
        self.assertEqual(len(doc[1]), 1)
        self.assertEqual(doc[1][0].text(), 'red')
        doc.splitLines(' ')
        self.assertEqual(len(doc), 4)
        self.assertEqual(len(doc[0]), 1)
        self.assertEqual(doc[0][0].text(), 'R_ED')
        self.assertEqual(len(doc[1]), 1)
        self.assertEqual(doc[1][0].text(), 'not')
        self.assertEqual(len(doc[2]), 1)
        self.assertEqual(doc[2][0].text(), '')
        self.assertEqual(len(doc[3]), 1)
        self.assertEqual(doc[3][0].text(), 'red')

        doc = QgsTextDocument.fromHtml(['<span style="color: red">R_ED</span> not <div>red</div>'])
        doc.splitLines('_')
        self.assertEqual(len(doc), 3)
        self.assertEqual(len(doc[0]), 1)
        self.assertEqual(doc[0][0].text(), 'R')
        self.assertEqual(len(doc[1]), 2)
        self.assertEqual(doc[1][0].text(), 'ED')
        self.assertEqual(doc[1][1].text(), ' not ')
        self.assertEqual(len(doc[2]), 1)
        self.assertEqual(doc[2][0].text(), 'red')

    def testCapitalize(self):
        doc = QgsTextDocument.fromPlainText(['abc def ghi', 'more text', 'another block'])
        doc.applyCapitalization(QgsStringUtils.TitleCase)
        self.assertEqual(doc.toPlainText(), ['Abc Def Ghi', 'More Text', 'Another Block'])


if __name__ == '__main__':
    unittest.main()
