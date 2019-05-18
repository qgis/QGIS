# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsSymbolButton.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '23/07/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsFillSymbol, QgsMarkerSymbol, QgsSymbol
from qgis.gui import QgsSymbolButton, QgsMapCanvas
from qgis.testing import start_app, unittest
from qgis.PyQt.QtGui import QColor, QFont
from qgis.PyQt.QtTest import QSignalSpy
from utilities import getTestFont

start_app()


class TestQgsSymbolButton(unittest.TestCase):

    def testGettersSetters(self):
        button = QgsSymbolButton()
        canvas = QgsMapCanvas()

        button.setDialogTitle('test title')
        self.assertEqual(button.dialogTitle(), 'test title')

        button.setMapCanvas(canvas)
        self.assertEqual(button.mapCanvas(), canvas)

        button.setSymbolType(QgsSymbol.Line)
        self.assertEqual(button.symbolType(), QgsSymbol.Line)

    def testSettingSymbolType(self):
        button = QgsSymbolButton()
        button.setSymbolType(QgsSymbol.Marker)
        symbol = QgsMarkerSymbol.createSimple({})
        symbol.setColor(QColor(255, 0, 0))
        button.setSymbol(symbol)

        # if same symbol type, existing symbol should be kept
        button.setSymbolType(QgsSymbol.Marker)
        self.assertEqual(button.symbol(), symbol)

        # if setting different symbol type, symbol should be reset to new type
        button.setSymbolType(QgsSymbol.Fill)
        self.assertTrue(isinstance(button.symbol(), QgsFillSymbol))

    def testPasteSymbol(self):
        button = QgsSymbolButton()
        button.setSymbolType(QgsSymbol.Marker)
        symbol = QgsMarkerSymbol.createSimple({})
        symbol.setColor(QColor(255, 0, 0))
        button.setSymbol(symbol)

        button2 = QgsSymbolButton()
        button2.setSymbolType(QgsSymbol.Marker)
        symbol2 = QgsMarkerSymbol.createSimple({})
        symbol2.setColor(QColor(0, 255, 0))
        button2.setSymbol(symbol2)

        button.copySymbol()
        button2.pasteSymbol()
        self.assertEqual(button2.symbol().color(), QColor(255, 0, 0))

        # try pasting incompatible symbol
        button2.setSymbolType(QgsSymbol.Fill)
        fill_symbol = QgsFillSymbol.createSimple({})
        fill_symbol.setColor(QColor(0, 0, 255))
        button2.setSymbol(fill_symbol)
        button.copySymbol() # copied a marker symbol
        button2.pasteSymbol() # should have no effect
        self.assertEqual(button2.symbol(), fill_symbol)

    def testSetGetSymbol(self):
        button = QgsSymbolButton()
        symbol = QgsMarkerSymbol.createSimple({})
        symbol.setColor(QColor(255, 0, 0))

        signal_spy = QSignalSpy(button.changed)
        button.setSymbol(symbol)
        self.assertEqual(len(signal_spy), 1)

        r = button.symbol()
        self.assertEqual(r.color(), QColor(255, 0, 0))

    def testSetColor(self):
        button = QgsSymbolButton()

        symbol = QgsMarkerSymbol.createSimple({})
        symbol.setColor(QColor(255, 255, 0))

        button.setSymbol(symbol)

        signal_spy = QSignalSpy(button.changed)
        button.setColor(QColor(0, 255, 0))
        self.assertEqual(len(signal_spy), 1)

        r = button.symbol()
        self.assertEqual(r.color().name(), QColor(0, 255, 0).name())

        # set same color, should not emit signal
        button.setColor(QColor(0, 255, 0))
        self.assertEqual(len(signal_spy), 1)

        # color with transparency - should be stripped
        button.setColor(QColor(0, 255, 0, 100))
        r = button.symbol()
        self.assertEqual(r.color(), QColor(0, 255, 0))


if __name__ == '__main__':
    unittest.main()
