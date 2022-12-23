# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsCombinedStyleModel

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/03/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import os

import qgis  # NOQA

from qgis.PyQt.QtCore import QCoreApplication, QEvent, Qt

from qgis.core import (
    QgsStyle,
    QgsTextFormat,
    QgsStyleModel
)

from qgis.testing import start_app, unittest

start_app()

try:
    from qgis.core import QgsCombinedStyleModel
except ImportError:
    QgsCombinedStyleModel = None


class TestQgsCombinedStyleModel(unittest.TestCase):

    @unittest.skipIf(QgsCombinedStyleModel is None, "QgsCombinedStyleModel not available")
    def test_model(self):
        model = QgsCombinedStyleModel()
        self.assertFalse(model.styles())
        self.assertEqual(model.rowCount(), 0)

        style1 = QgsStyle()
        style1.createMemoryDatabase()
        style1.setName('first style')
        style1.setFileName('/home/my style1.db')

        model.addStyle(style1)
        self.assertEqual(model.styles(), [style1])
        self.assertEqual(model.headerData(0, Qt.Horizontal), 'Name')
        self.assertEqual(model.headerData(1, Qt.Horizontal), 'Tags')

        self.assertEqual(model.columnCount(), 2)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), 'first style')
        self.assertTrue(model.data(model.index(0, 0), QgsStyleModel.IsTitleRole))
        self.assertEqual(model.data(model.index(0, 0), QgsStyleModel.StyleName), 'first style')
        self.assertEqual(model.data(model.index(0, 0), QgsStyleModel.StyleFileName), '/home/my style1.db')

        style1.addTextFormat('format 1', QgsTextFormat(), True)
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), 'first style')
        self.assertTrue(model.data(model.index(0, 0), QgsStyleModel.IsTitleRole))
        self.assertEqual(model.data(model.index(1, 0)), 'format 1')
        self.assertFalse(model.data(model.index(1, 0), QgsStyleModel.IsTitleRole))
        self.assertEqual(model.data(model.index(1, 0), QgsStyleModel.StyleName), 'first style')
        self.assertEqual(model.data(model.index(1, 0), QgsStyleModel.StyleFileName), '/home/my style1.db')

        style2 = QgsStyle()
        style2.createMemoryDatabase()
        style2.setName('second style')
        style2.setFileName('/home/my style2.db')
        style2.addTextFormat('format 2', QgsTextFormat(), True)
        style2.addTextFormat('format 3', QgsTextFormat(), True)

        model.addStyle(style2)
        self.assertEqual(model.styles(), [style1, style2])

        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(model.data(model.index(0, 0)), 'first style')
        self.assertTrue(model.data(model.index(0, 0), QgsStyleModel.IsTitleRole))
        self.assertEqual(model.data(model.index(0, 0), QgsStyleModel.StyleName), 'first style')
        self.assertEqual(model.data(model.index(0, 0), QgsStyleModel.StyleFileName), '/home/my style1.db')
        self.assertEqual(model.data(model.index(1, 0)), 'format 1')
        self.assertFalse(model.data(model.index(1, 0), QgsStyleModel.IsTitleRole))
        self.assertEqual(model.data(model.index(1, 0), QgsStyleModel.StyleName), 'first style')
        self.assertEqual(model.data(model.index(1, 0), QgsStyleModel.StyleFileName), '/home/my style1.db')
        self.assertEqual(model.data(model.index(2, 0)), 'second style')
        self.assertTrue(model.data(model.index(2, 0), QgsStyleModel.IsTitleRole))
        self.assertEqual(model.data(model.index(2, 0), QgsStyleModel.StyleName), 'second style')
        self.assertEqual(model.data(model.index(2, 0), QgsStyleModel.StyleFileName), '/home/my style2.db')
        self.assertEqual(model.data(model.index(3, 0)), 'format 2')
        self.assertFalse(model.data(model.index(3, 0), QgsStyleModel.IsTitleRole))
        self.assertEqual(model.data(model.index(3, 0), QgsStyleModel.StyleName), 'second style')
        self.assertEqual(model.data(model.index(3, 0), QgsStyleModel.StyleFileName), '/home/my style2.db')
        self.assertEqual(model.data(model.index(4, 0)), 'format 3')
        self.assertFalse(model.data(model.index(4, 0), QgsStyleModel.IsTitleRole))
        self.assertEqual(model.data(model.index(4, 0), QgsStyleModel.StyleName), 'second style')
        self.assertEqual(model.data(model.index(4, 0), QgsStyleModel.StyleFileName), '/home/my style2.db')

        style1.deleteLater()
        style1 = None
        QCoreApplication.sendPostedEvents(None, QEvent.DeferredDelete)

        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0)), 'second style')
        self.assertTrue(model.data(model.index(0, 0), QgsStyleModel.IsTitleRole))
        self.assertEqual(model.data(model.index(1, 0)), 'format 2')
        self.assertFalse(model.data(model.index(1, 0), QgsStyleModel.IsTitleRole))
        self.assertEqual(model.data(model.index(2, 0)), 'format 3')
        self.assertFalse(model.data(model.index(2, 0), QgsStyleModel.IsTitleRole))

        model.removeStyle(style2)
        self.assertEqual(model.rowCount(), 0)

        model.removeStyle(style2)


if __name__ == '__main__':
    unittest.main()
