# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsSymbolLayerUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2016-09'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import QgsSymbolLayerUtils, QgsMarkerSymbol
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtCore import QSizeF, QPointF
from qgis.testing import unittest, start_app

start_app()


class PyQgsSymbolLayerUtils(unittest.TestCase):

    def testEncodeDecodeSize(self):
        s = QSizeF()
        string = QgsSymbolLayerUtils.encodeSize(s)
        s2 = QgsSymbolLayerUtils.decodeSize(string)
        self.assertEqual(s2, s)
        s = QSizeF(1.5, 2.5)
        string = QgsSymbolLayerUtils.encodeSize(s)
        s2 = QgsSymbolLayerUtils.decodeSize(string)
        self.assertEqual(s2, s)

        # bad string
        s2 = QgsSymbolLayerUtils.decodeSize('')
        self.assertEqual(s2, QSizeF(0, 0))

    def testEncodeDecodePoint(self):
        s = QPointF()
        string = QgsSymbolLayerUtils.encodePoint(s)
        s2 = QgsSymbolLayerUtils.decodePoint(string)
        self.assertEqual(s2, s)
        s = QPointF(1.5, 2.5)
        string = QgsSymbolLayerUtils.encodePoint(s)
        s2 = QgsSymbolLayerUtils.decodePoint(string)
        self.assertEqual(s2, s)

        # bad string
        s2 = QgsSymbolLayerUtils.decodePoint('')
        self.assertEqual(s2, QPointF())

    def testSymbolToFromMimeData(self):
        """
        Test converting symbols to and from mime data
        """
        symbol = QgsMarkerSymbol.createSimple({})
        symbol.setColor(QColor(255, 0, 255))
        self.assertFalse(QgsSymbolLayerUtils.symbolFromMimeData(None))
        self.assertFalse(QgsSymbolLayerUtils.symbolToMimeData(None))
        mime = QgsSymbolLayerUtils.symbolToMimeData(symbol)
        self.assertTrue(mime is not None)
        symbol2 = QgsSymbolLayerUtils.symbolFromMimeData(mime)
        self.assertTrue(symbol2 is not None)
        self.assertEqual(symbol2.color().name(), symbol.color().name())


if __name__ == '__main__':
    unittest.main()
