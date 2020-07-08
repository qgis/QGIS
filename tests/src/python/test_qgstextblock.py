# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsTextBlock.

Run with: ctest -V -R QgsTextBlock

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
    QgsTextBlock,
    QgsTextFragment
)
from qgis.testing import start_app, unittest

start_app()


class TestQgsTextBlock(unittest.TestCase):

    def testConstructors(self):
        # empty
        block = QgsTextBlock()
        self.assertEqual(len(block), 0)

        # single fragment block
        fragment = QgsTextFragment('ludicrous gibs!')
        block = QgsTextBlock(fragment)
        self.assertEqual(len(block), 1)
        self.assertEqual(block[0].text(), fragment.text())
        self.assertEqual(block.toPlainText(), 'ludicrous gibs!')

    def testAppend(self):
        block = QgsTextBlock()
        self.assertEqual(len(block), 0)

        frag = QgsTextFragment('a')
        block.append(frag)
        self.assertEqual(len(block), 1)
        self.assertEqual(block[0].text(), 'a')
        frag = QgsTextFragment('b')
        block.append(frag)
        self.assertEqual(len(block), 2)
        self.assertEqual(block[0].text(), 'a')
        self.assertEqual(block[1].text(), 'b')

        self.assertEqual(block.toPlainText(), 'ab')

    def testAt(self):
        block = QgsTextBlock()
        block.append(QgsTextFragment('a'))
        block.append(QgsTextFragment('b'))
        self.assertEqual(len(block), 2)

        self.assertEqual(block.at(0).text(), 'a')
        self.assertEqual(block.at(1).text(), 'b')
        with self.assertRaises(KeyError):
            block.at(2)
        with self.assertRaises(KeyError):
            block.at(-1)

        self.assertEqual(block[0].text(), 'a')
        self.assertEqual(block[1].text(), 'b')
        with self.assertRaises(IndexError):
            _ = block[2]

        self.assertEqual(block[-1].text(), 'b')
        self.assertEqual(block[-2].text(), 'a')

    def testClear(self):
        block = QgsTextBlock()
        block.append(QgsTextFragment('a'))
        block.append(QgsTextFragment('b'))
        self.assertEqual(len(block), 2)
        self.assertFalse(block.empty())

        block.clear()
        self.assertEqual(len(block), 0)
        self.assertTrue(block.empty())


if __name__ == '__main__':
    unittest.main()
