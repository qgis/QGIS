# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsStyleModel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '10/09/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsSymbol,
                       QgsFillSymbol,
                       QgsMarkerSymbol,
                       QgsLineSymbol,
                       QgsLimitedRandomColorRamp,
                       QgsStyleModel,
                       QgsStyle,
                       QgsStyleProxyModel)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QColor

start_app()


def createMarkerSymbol():
    symbol = QgsMarkerSymbol.createSimple({
        "color": "100,150,50",
        "name": "square",
        "size": "3.0"
    })
    return symbol


def createLineSymbol():
    symbol = QgsLineSymbol.createSimple({
        "color": "100,150,50"
    })
    return symbol


def createFillSymbol():
    symbol = QgsFillSymbol.createSimple({
        "color": "100,150,50"
    })
    return symbol


class TestQgsStyleModel(unittest.TestCase):

    def test_style_with_symbols(self):

        style = QgsStyle()
        style.createMemoryDatabase()

        # style with only symbols

        symbol_a = createMarkerSymbol()
        symbol_a.setColor(QColor(255, 10, 10))
        self.assertTrue(style.addSymbol('a', symbol_a, True))
        style.tagSymbol(QgsStyle.SymbolEntity, 'a', ['tag 1', 'tag 2'])
        symbol_B = createMarkerSymbol()
        symbol_B.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol('B ', symbol_B, True))
        symbol_b = createFillSymbol()
        symbol_b.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol('b', symbol_b, True))
        symbol_C = createLineSymbol()
        symbol_C.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol('C', symbol_C, True))
        style.tagSymbol(QgsStyle.SymbolEntity, 'C', ['tag 3'])
        symbol_C = createMarkerSymbol()
        symbol_C.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol(' ----c/- ', symbol_C, True))

        model = QgsStyleModel(style)
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(model.columnCount(), 2)

        self.assertEqual(model.headerData(0, Qt.Horizontal), 'Name')
        self.assertEqual(model.headerData(1, Qt.Horizontal), 'Tags')

        self.assertTrue(model.index(0, 0).isValid())
        self.assertFalse(model.index(10, 0).isValid())
        self.assertFalse(model.index(0, 10).isValid())

        self.assertFalse(model.parent(model.index(0, 0)).isValid())

        self.assertFalse(model.flags(model.index(-1, 0)) & Qt.ItemIsEditable)
        self.assertFalse(model.flags(model.index(5, 0)) & Qt.ItemIsEditable)

        self.assertFalse(model.flags(model.index(0, 1)) & Qt.ItemIsEditable)
        self.assertTrue(model.flags(model.index(0, 0)) & Qt.ItemIsEditable)

        for role in (Qt.DisplayRole, Qt.EditRole):
            self.assertIsNone(model.data(model.index(-1, 0), role))
            self.assertIsNone(model.data(model.index(-1, 1), role))
            self.assertEqual(model.data(model.index(0, 0), role), ' ----c/- ')
            self.assertFalse(model.data(model.index(0, 1), role))
            self.assertIsNone(model.data(model.index(0, 2), role))
            self.assertIsNone(model.data(model.index(0, -1), role))
            self.assertEqual(model.data(model.index(1, 0), role), 'B ')
            self.assertFalse(model.data(model.index(1, 1), role))
            self.assertEqual(model.data(model.index(2, 0), role), 'C')
            self.assertEqual(model.data(model.index(2, 1), role), 'tag 3')
            self.assertEqual(model.data(model.index(3, 0), role), 'a')
            self.assertEqual(model.data(model.index(3, 1), role), 'tag 1, tag 2')
            self.assertEqual(model.data(model.index(4, 0), role), 'b')
            self.assertFalse(model.data(model.index(4, 1), role))
            self.assertIsNone(model.data(model.index(5, 0), role))
            self.assertIsNone(model.data(model.index(5, 1), role))

        # decorations
        self.assertIsNone(model.data(model.index(-1, 0), Qt.DecorationRole))
        self.assertIsNone(model.data(model.index(0, 1), Qt.DecorationRole))
        self.assertIsNone(model.data(model.index(5, 0), Qt.DecorationRole))
        self.assertFalse(model.data(model.index(0, 0), Qt.DecorationRole).isNull())

        self.assertEqual(model.data(model.index(0, 0), QgsStyleModel.TypeRole), QgsStyle.SymbolEntity)
        self.assertEqual(model.data(model.index(1, 0), QgsStyleModel.TypeRole), QgsStyle.SymbolEntity)
        self.assertEqual(model.data(model.index(4, 0), QgsStyleModel.TypeRole), QgsStyle.SymbolEntity)

    def test_style_with_ramps(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        # style with only ramps

        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp('a', ramp_a, True))
        style.tagSymbol(QgsStyle.ColorrampEntity, 'a', ['tag 1', 'tag 2'])
        symbol_B = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp('B ', symbol_B, True))
        symbol_b = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp('b', symbol_b, True))
        symbol_C = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp('C', symbol_C, True))
        style.tagSymbol(QgsStyle.ColorrampEntity, 'C', ['tag 3'])
        symbol_C = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp(' ----c/- ', symbol_C, True))

        model = QgsStyleModel(style)
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(model.columnCount(), 2)

        self.assertTrue(model.index(0, 0).isValid())
        self.assertFalse(model.index(10, 0).isValid())
        self.assertFalse(model.index(0, 10).isValid())

        self.assertFalse(model.parent(model.index(0, 0)).isValid())

        for role in (Qt.DisplayRole, Qt.EditRole):
            self.assertIsNone(model.data(model.index(-1, 0), role))
            self.assertIsNone(model.data(model.index(-1, 1), role))
            self.assertEqual(model.data(model.index(0, 0), role), ' ----c/- ')
            self.assertFalse(model.data(model.index(0, 1), role))
            self.assertIsNone(model.data(model.index(0, 2), role))
            self.assertIsNone(model.data(model.index(0, -1), role))
            self.assertEqual(model.data(model.index(1, 0), role), 'B ')
            self.assertFalse(model.data(model.index(1, 1), role))
            self.assertEqual(model.data(model.index(2, 0), role), 'C')
            self.assertEqual(model.data(model.index(2, 1), role), 'tag 3')
            self.assertEqual(model.data(model.index(3, 0), role), 'a')
            self.assertEqual(model.data(model.index(3, 1), role), 'tag 1, tag 2')
            self.assertEqual(model.data(model.index(4, 0), role), 'b')
            self.assertFalse(model.data(model.index(4, 1), role))
            self.assertIsNone(model.data(model.index(5, 0), role))
            self.assertIsNone(model.data(model.index(5, 1), role))

        # decorations
        self.assertIsNone(model.data(model.index(-1, 0), Qt.DecorationRole))
        self.assertIsNone(model.data(model.index(0, 1), Qt.DecorationRole))
        self.assertIsNone(model.data(model.index(5, 0), Qt.DecorationRole))
        self.assertFalse(model.data(model.index(0, 0), Qt.DecorationRole).isNull())

        self.assertEqual(model.data(model.index(0, 0), QgsStyleModel.TypeRole), QgsStyle.ColorrampEntity)
        self.assertEqual(model.data(model.index(1, 0), QgsStyleModel.TypeRole), QgsStyle.ColorrampEntity)
        self.assertEqual(model.data(model.index(4, 0), QgsStyleModel.TypeRole), QgsStyle.ColorrampEntity)

    def test_mixed_style(self):
        """
        Test style with both symbols and ramps
        """
        style = QgsStyle()
        style.createMemoryDatabase()

        # style with only symbols

        symbol_a = createMarkerSymbol()
        symbol_a.setColor(QColor(255, 10, 10))
        self.assertTrue(style.addSymbol('a', symbol_a, True))
        style.tagSymbol(QgsStyle.SymbolEntity, 'a', ['tag 1', 'tag 2'])
        symbol_B = createMarkerSymbol()
        symbol_B.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol('B ', symbol_B, True))
        symbol_b = createFillSymbol()
        symbol_b.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol('b', symbol_b, True))
        symbol_C = createLineSymbol()
        symbol_C.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol('C', symbol_C, True))
        style.tagSymbol(QgsStyle.SymbolEntity, 'C', ['tag 3'])
        symbol_C = createMarkerSymbol()
        symbol_C.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol(' ----c/- ', symbol_C, True))
        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp('ramp a', ramp_a, True))
        style.tagSymbol(QgsStyle.ColorrampEntity, 'ramp a', ['tag 1', 'tag 2'])
        symbol_B = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp('ramp B ', symbol_B, True))
        symbol_b = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp('ramp c', symbol_b, True))

        model = QgsStyleModel(style)
        self.assertEqual(model.rowCount(), 8)
        self.assertEqual(model.columnCount(), 2)

        self.assertTrue(model.index(0, 0).isValid())
        self.assertFalse(model.index(10, 0).isValid())
        self.assertFalse(model.index(0, 10).isValid())

        self.assertFalse(model.parent(model.index(0, 0)).isValid())

        for role in (Qt.DisplayRole, Qt.EditRole):
            self.assertIsNone(model.data(model.index(-1, 0), role))
            self.assertIsNone(model.data(model.index(-1, 1), role))
            self.assertEqual(model.data(model.index(0, 0), role), ' ----c/- ')
            self.assertFalse(model.data(model.index(0, 1), role))
            self.assertIsNone(model.data(model.index(0, 2), role))
            self.assertIsNone(model.data(model.index(0, -1), role))
            self.assertEqual(model.data(model.index(1, 0), role), 'B ')
            self.assertFalse(model.data(model.index(1, 1), role))
            self.assertEqual(model.data(model.index(2, 0), role), 'C')
            self.assertEqual(model.data(model.index(2, 1), role), 'tag 3')
            self.assertEqual(model.data(model.index(3, 0), role), 'a')
            self.assertEqual(model.data(model.index(3, 1), role), 'tag 1, tag 2')
            self.assertEqual(model.data(model.index(4, 0), role), 'b')
            self.assertFalse(model.data(model.index(4, 1), role))
            self.assertEqual(model.data(model.index(5, 0), role), 'ramp B ')
            self.assertFalse(model.data(model.index(5, 1), role))
            self.assertEqual(model.data(model.index(6, 0), role), 'ramp a')
            self.assertEqual(model.data(model.index(6, 1), role), 'tag 1, tag 2')
            self.assertEqual(model.data(model.index(7, 0), role), 'ramp c')
            self.assertFalse(model.data(model.index(7, 1), role))
            self.assertIsNone(model.data(model.index(8, 0), role))
            self.assertIsNone(model.data(model.index(8, 1), role))

        # decorations
        self.assertIsNone(model.data(model.index(-1, 0), Qt.DecorationRole))
        self.assertIsNone(model.data(model.index(0, 1), Qt.DecorationRole))
        self.assertIsNone(model.data(model.index(8, 0), Qt.DecorationRole))
        self.assertFalse(model.data(model.index(0, 0), Qt.DecorationRole).isNull())
        self.assertFalse(model.data(model.index(5, 0), Qt.DecorationRole).isNull())

        self.assertEqual(model.data(model.index(0, 0), QgsStyleModel.TypeRole), QgsStyle.SymbolEntity)
        self.assertEqual(model.data(model.index(1, 0), QgsStyleModel.TypeRole), QgsStyle.SymbolEntity)
        self.assertEqual(model.data(model.index(4, 0), QgsStyleModel.TypeRole), QgsStyle.SymbolEntity)
        self.assertEqual(model.data(model.index(5, 0), QgsStyleModel.TypeRole), QgsStyle.ColorrampEntity)
        self.assertEqual(model.data(model.index(6, 0), QgsStyleModel.TypeRole), QgsStyle.ColorrampEntity)
        self.assertEqual(model.data(model.index(7, 0), QgsStyleModel.TypeRole), QgsStyle.ColorrampEntity)

    def test_add_delete_symbols(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        model = QgsStyleModel(style)
        self.assertEqual(model.rowCount(), 0)

        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol('a', symbol, True))
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'a')

        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol('c', symbol, True))
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'a')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'c')

        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol('b', symbol, True))
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'a')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'b')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'c')

        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol('1', symbol, True))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), '1')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'a')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'b')
        self.assertEqual(model.data(model.index(3, 0), Qt.DisplayRole), 'c')

        self.assertFalse(style.removeSymbol('xxxx'))
        self.assertEqual(model.rowCount(), 4)

        self.assertTrue(style.removeSymbol('b'))
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), '1')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'a')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'c')

        self.assertTrue(style.removeSymbol('1'))
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'a')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'c')

        self.assertTrue(style.removeSymbol('c'))
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'a')

        self.assertTrue(style.removeSymbol('a'))
        self.assertEqual(model.rowCount(), 0)

    def test_add_remove_ramps(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        model = QgsStyleModel(style)
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol('a', symbol, True))
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol('c', symbol, True))
        self.assertEqual(model.rowCount(), 2)

        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp('ramp a', ramp_a, True))
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'a')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'c')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'ramp a')

        symbol_B = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp('ramp c', symbol_B, True))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'a')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'c')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'ramp a')
        self.assertEqual(model.data(model.index(3, 0), Qt.DisplayRole), 'ramp c')

        symbol_b = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp('ramp b', symbol_b, True))
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'a')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'c')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'ramp a')
        self.assertEqual(model.data(model.index(3, 0), Qt.DisplayRole), 'ramp b')
        self.assertEqual(model.data(model.index(4, 0), Qt.DisplayRole), 'ramp c')

        self.assertTrue(style.removeColorRamp('ramp a'))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'a')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'c')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'ramp b')
        self.assertEqual(model.data(model.index(3, 0), Qt.DisplayRole), 'ramp c')

    def test_renamed(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        model = QgsStyleModel(style)
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol('a', symbol, True))
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol('c', symbol, True))
        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp('ramp a', ramp_a, True))
        symbol_B = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp('ramp c', symbol_B, True))

        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'a')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'c')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'ramp a')
        self.assertEqual(model.data(model.index(3, 0), Qt.DisplayRole), 'ramp c')

        self.assertTrue(style.renameSymbol('a', 'b'))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'b')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'c')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'ramp a')
        self.assertEqual(model.data(model.index(3, 0), Qt.DisplayRole), 'ramp c')

        self.assertTrue(style.renameSymbol('b', 'd'))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'c')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'd')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'ramp a')
        self.assertEqual(model.data(model.index(3, 0), Qt.DisplayRole), 'ramp c')

        self.assertTrue(style.renameSymbol('d', 'e'))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'c')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'e')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'ramp a')
        self.assertEqual(model.data(model.index(3, 0), Qt.DisplayRole), 'ramp c')

        self.assertTrue(style.renameSymbol('c', 'f'))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'e')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'f')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'ramp a')
        self.assertEqual(model.data(model.index(3, 0), Qt.DisplayRole), 'ramp c')

        self.assertTrue(style.renameColorRamp('ramp a', 'ramp b'))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'e')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'f')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'ramp b')
        self.assertEqual(model.data(model.index(3, 0), Qt.DisplayRole), 'ramp c')

        self.assertTrue(style.renameColorRamp('ramp b', 'ramp d'))
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'e')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'f')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'ramp c')
        self.assertEqual(model.data(model.index(3, 0), Qt.DisplayRole), 'ramp d')

        self.assertTrue(style.renameColorRamp('ramp d', 'ramp e'))
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'e')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'f')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'ramp c')
        self.assertEqual(model.data(model.index(3, 0), Qt.DisplayRole), 'ramp e')

        self.assertTrue(style.renameColorRamp('ramp c', 'ramp f'))
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'e')
        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'f')
        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'ramp e')
        self.assertEqual(model.data(model.index(3, 0), Qt.DisplayRole), 'ramp f')

    def test_tags_changed(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        model = QgsStyleModel(style)
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol('a', symbol, True))
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol('c', symbol, True))
        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp('ramp a', ramp_a, True))
        symbol_B = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp('ramp c', symbol_B, True))

        self.assertFalse(model.data(model.index(0, 1), Qt.DisplayRole))
        self.assertFalse(model.data(model.index(1, 1), Qt.DisplayRole))
        self.assertFalse(model.data(model.index(2, 1), Qt.DisplayRole))
        self.assertFalse(model.data(model.index(3, 1), Qt.DisplayRole))

        style.tagSymbol(QgsStyle.SymbolEntity, 'a', ['t1', 't2'])
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 't1, t2')
        self.assertFalse(model.data(model.index(1, 1), Qt.DisplayRole))
        self.assertFalse(model.data(model.index(2, 1), Qt.DisplayRole))
        self.assertFalse(model.data(model.index(3, 1), Qt.DisplayRole))

        style.tagSymbol(QgsStyle.SymbolEntity, 'a', ['t3'])
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 't1, t2, t3')
        self.assertFalse(model.data(model.index(1, 1), Qt.DisplayRole))
        self.assertFalse(model.data(model.index(2, 1), Qt.DisplayRole))
        self.assertFalse(model.data(model.index(3, 1), Qt.DisplayRole))

        style.tagSymbol(QgsStyle.ColorrampEntity, 'ramp a', ['t1', 't2'])
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 't1, t2, t3')
        self.assertFalse(model.data(model.index(1, 1), Qt.DisplayRole))
        self.assertEqual(model.data(model.index(2, 1), Qt.DisplayRole), 't1, t2')
        self.assertFalse(model.data(model.index(3, 1), Qt.DisplayRole))

        style.tagSymbol(QgsStyle.ColorrampEntity, 'ramp c', ['t3'])
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 't1, t2, t3')
        self.assertFalse(model.data(model.index(1, 1), Qt.DisplayRole))
        self.assertEqual(model.data(model.index(2, 1), Qt.DisplayRole), 't1, t2')
        self.assertEqual(model.data(model.index(3, 1), Qt.DisplayRole), 't3')

        style.tagSymbol(QgsStyle.SymbolEntity, 'c', ['t4'])
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 't1, t2, t3')
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), 't4')
        self.assertEqual(model.data(model.index(2, 1), Qt.DisplayRole), 't1, t2')
        self.assertEqual(model.data(model.index(3, 1), Qt.DisplayRole), 't3')

        style.detagSymbol(QgsStyle.SymbolEntity, 'c', ['t4'])
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 't1, t2, t3')
        self.assertFalse(model.data(model.index(1, 1), Qt.DisplayRole))
        self.assertEqual(model.data(model.index(2, 1), Qt.DisplayRole), 't1, t2')
        self.assertEqual(model.data(model.index(3, 1), Qt.DisplayRole), 't3')

        style.detagSymbol(QgsStyle.ColorrampEntity, 'ramp a', ['t1'])
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 't1, t2, t3')
        self.assertFalse(model.data(model.index(1, 1), Qt.DisplayRole))
        self.assertEqual(model.data(model.index(2, 1), Qt.DisplayRole), 't2')
        self.assertEqual(model.data(model.index(3, 1), Qt.DisplayRole), 't3')

    def test_filter_proxy(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        symbol_a = createMarkerSymbol()
        symbol_a.setColor(QColor(255, 10, 10))
        self.assertTrue(style.addSymbol('a', symbol_a, True))
        style.tagSymbol(QgsStyle.SymbolEntity, 'a', ['tag 1', 'tag 2'])
        symbol_B = createMarkerSymbol()
        symbol_B.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol('BB', symbol_B, True))
        symbol_b = createFillSymbol()
        symbol_b.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol('b', symbol_b, True))
        symbol_C = createLineSymbol()
        symbol_C.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol('C', symbol_C, True))
        style.tagSymbol(QgsStyle.SymbolEntity, 'C', ['tag 3'])
        symbol_C = createMarkerSymbol()
        symbol_C.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol('another', symbol_C, True))
        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp('ramp a', ramp_a, True))
        style.tagSymbol(QgsStyle.ColorrampEntity, 'ramp a', ['tag 1', 'tag 2'])
        symbol_B = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp('ramp BB', symbol_B, True))
        symbol_b = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp('ramp c', symbol_b, True))

        model = QgsStyleProxyModel(style)
        self.assertEqual(model.rowCount(), 8)

        # filter string
        model.setFilterString('xx')
        self.assertEqual(model.filterString(), 'xx')
        self.assertEqual(model.rowCount(), 0)
        model.setFilterString('b')
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0)), 'b')
        self.assertEqual(model.data(model.index(1, 0)), 'BB')
        self.assertEqual(model.data(model.index(2, 0)), 'ramp BB')
        model.setFilterString('bb')
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), 'BB')
        self.assertEqual(model.data(model.index(1, 0)), 'ramp BB')
        model.setFilterString('tag 1')
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), 'a')
        self.assertEqual(model.data(model.index(1, 0)), 'ramp a')
        model.setFilterString('TAG 1')
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), 'a')
        self.assertEqual(model.data(model.index(1, 0)), 'ramp a')
        model.setFilterString('')
        self.assertEqual(model.rowCount(), 8)

        # entity type
        model.setEntityFilter(QgsStyle.SymbolEntity)
        self.assertEqual(model.rowCount(), 8)
        model.setEntityFilter(QgsStyle.ColorrampEntity)
        self.assertEqual(model.rowCount(), 8)
        model.setEntityFilterEnabled(True)
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0)), 'ramp a')
        self.assertEqual(model.data(model.index(1, 0)), 'ramp BB')
        self.assertEqual(model.data(model.index(2, 0)), 'ramp c')
        model.setFilterString('BB')
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), 'ramp BB')
        model.setFilterString('')

        model.setEntityFilter(QgsStyle.SymbolEntity)
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(model.data(model.index(0, 0)), 'a')
        self.assertEqual(model.data(model.index(1, 0)), 'another')
        self.assertEqual(model.data(model.index(2, 0)), 'b')
        self.assertEqual(model.data(model.index(3, 0)), 'BB')
        self.assertEqual(model.data(model.index(4, 0)), 'C')
        model.setFilterString('ot')
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), 'another')
        model.setFilterString('')

        model.setEntityFilterEnabled(False)
        self.assertEqual(model.rowCount(), 8)

        # symbol type filter
        model.setSymbolType(QgsSymbol.Line)
        self.assertEqual(model.rowCount(), 8)
        model.setSymbolType(QgsSymbol.Marker)
        self.assertEqual(model.rowCount(), 8)
        model.setSymbolTypeFilterEnabled(True)
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), 'a')
        self.assertEqual(model.data(model.index(1, 0)), 'another')
        self.assertEqual(model.data(model.index(2, 0)), 'BB')
        self.assertEqual(model.data(model.index(3, 0)), 'ramp a')
        self.assertEqual(model.data(model.index(4, 0)), 'ramp BB')
        model.setEntityFilterEnabled(True)
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0)), 'a')
        self.assertEqual(model.data(model.index(1, 0)), 'another')
        self.assertEqual(model.data(model.index(2, 0)), 'BB')
        model.setFilterString('oth')
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), 'another')
        model.setEntityFilterEnabled(False)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), 'another')
        model.setEntityFilterEnabled(True)
        model.setFilterString('')
        model.setSymbolType(QgsSymbol.Line)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), 'C')
        model.setSymbolType(QgsSymbol.Fill)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), 'b')
        model.setSymbolTypeFilterEnabled(False)
        model.setEntityFilterEnabled(False)
        self.assertEqual(model.rowCount(), 8)

        # tag filter
        self.assertEqual(model.tagId(), -1)
        tag_1_id = style.tagId('tag 1')
        tag_3_id = style.tagId('tag 3')
        model.setTagId(tag_1_id)
        self.assertEqual(model.tagId(), tag_1_id)
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), 'a')
        self.assertEqual(model.data(model.index(1, 0)), 'ramp a')
        model.setEntityFilterEnabled(True)
        model.setEntityFilter(QgsStyle.ColorrampEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), 'ramp a')
        model.setEntityFilterEnabled(False)
        model.setFilterString('ra')
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), 'ramp a')
        model.setEntityFilterEnabled(False)
        model.setFilterString('')
        model.setTagId(-1)
        self.assertEqual(model.rowCount(), 8)
        model.setTagId(tag_3_id)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), 'C')
        style.tagSymbol(QgsStyle.ColorrampEntity, 'ramp c', ['tag 3'])
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), 'C')
        self.assertEqual(model.data(model.index(1, 0)), 'ramp c')
        style.detagSymbol(QgsStyle.ColorrampEntity, 'ramp c')
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), 'C')
        model.setTagId(-1)
        self.assertEqual(model.rowCount(), 8)

        # favorite filter
        style.addFavorite(QgsStyle.ColorrampEntity, 'ramp c')
        style.addFavorite(QgsStyle.SymbolEntity, 'another')
        self.assertEqual(model.favoritesOnly(), False)
        model.setFavoritesOnly(True)
        self.assertEqual(model.favoritesOnly(), True)
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), 'another')
        self.assertEqual(model.data(model.index(1, 0)), 'ramp c')
        model.setEntityFilterEnabled(True)
        model.setEntityFilter(QgsStyle.ColorrampEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), 'ramp c')
        model.setEntityFilterEnabled(False)
        model.setFilterString('er')
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), 'another')
        model.setEntityFilterEnabled(False)
        model.setFilterString('')
        self.assertEqual(model.rowCount(), 2)
        style.addFavorite(QgsStyle.ColorrampEntity, 'ramp a')
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0)), 'another')
        self.assertEqual(model.data(model.index(1, 0)), 'ramp a')
        self.assertEqual(model.data(model.index(2, 0)), 'ramp c')
        style.removeFavorite(QgsStyle.ColorrampEntity, 'ramp a')
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), 'another')
        self.assertEqual(model.data(model.index(1, 0)), 'ramp c')
        style.addFavorite(QgsStyle.SymbolEntity, 'BB')
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0)), 'another')
        self.assertEqual(model.data(model.index(1, 0)), 'BB')
        self.assertEqual(model.data(model.index(2, 0)), 'ramp c')
        style.removeFavorite(QgsStyle.SymbolEntity, 'another')
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), 'BB')
        self.assertEqual(model.data(model.index(1, 0)), 'ramp c')
        model.setFavoritesOnly(False)
        self.assertEqual(model.rowCount(), 8)

        # smart group filter
        # can't test in Python -- smart groups cannot be set here!


if __name__ == '__main__':
    unittest.main()
