# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsTableCell.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '10/01/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA
from qgis.core import (QgsTableCell,
                       QgsBearingNumericFormat,
                       QgsReadWriteContext,
                       QgsTextFormat)

from qgis.PyQt.QtGui import QColor

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsTableCell(unittest.TestCase):

    def testCell(self):
        c = QgsTableCell('test')
        self.assertEqual(c.content(), 'test')
        c.setContent(5)
        self.assertEqual(c.content(), 5)

        self.assertFalse(c.backgroundColor().isValid())
        self.assertFalse(c.foregroundColor().isValid())
        self.assertFalse(c.numericFormat())
        self.assertFalse(c.textFormat().isValid())

        c.setBackgroundColor(QColor(255, 0, 0))
        c.setForegroundColor(QColor(255, 0, 255))
        c.setNumericFormat(QgsBearingNumericFormat())
        self.assertEqual(c.backgroundColor().name(), '#ff0000')
        self.assertEqual(c.foregroundColor().name(), '#ff00ff')
        self.assertIsInstance(c.numericFormat(), QgsBearingNumericFormat)

        format = QgsTextFormat()
        format.setSize(16.8)
        c.setTextFormat(format)
        self.assertEqual(c.textFormat().size(), 16.8)
        self.assertTrue(c.textFormat().isValid())

    def testProperties(self):
        c = QgsTableCell('test')

        props = c.properties(QgsReadWriteContext())

        c2 = QgsTableCell()
        c2.setProperties(props, QgsReadWriteContext())

        self.assertEqual(c2.content(), 'test')
        self.assertFalse(c2.backgroundColor().isValid())
        self.assertFalse(c2.foregroundColor().isValid())
        self.assertFalse(c2.numericFormat())
        self.assertFalse(c2.textFormat().isValid())

        c.setBackgroundColor(QColor(255, 0, 0))
        c.setForegroundColor(QColor(255, 0, 255))
        format = QgsBearingNumericFormat()
        format.setShowPlusSign(True)
        c.setNumericFormat(format)
        text_format = QgsTextFormat()
        text_format.setSize(16.8)
        c.setTextFormat(text_format)
        props = c.properties(QgsReadWriteContext())

        c3 = QgsTableCell()
        c3.setProperties(props, QgsReadWriteContext())

        self.assertEqual(c3.content(), 'test')
        self.assertEqual(c3.backgroundColor().name(), '#ff0000')
        self.assertEqual(c3.foregroundColor().name(), '#ff00ff')
        self.assertIsInstance(c3.numericFormat(), QgsBearingNumericFormat)
        self.assertTrue(c3.numericFormat().showPlusSign())
        self.assertEqual(c3.textFormat().size(), 16.8)
        self.assertTrue(c3.textFormat().isValid())


if __name__ == '__main__':
    unittest.main()
