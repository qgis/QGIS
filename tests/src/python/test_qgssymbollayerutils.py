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

import qgis  # NOQA

from qgis.core import (QgsSymbolLayerUtils,
                       QgsMarkerSymbol,
                       QgsArrowSymbolLayer)
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

    def testToSize(self):
        s2, ok = QgsSymbolLayerUtils.toSize(None)
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toSize(4)
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toSize('4')
        self.assertFalse(ok)

        # arrays
        s2, ok = QgsSymbolLayerUtils.toSize([4])
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toSize([])
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toSize([4, 5, 6])
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toSize([4, 5])
        self.assertTrue(ok)
        self.assertEqual(s2, QSizeF(4, 5))

        s2, ok = QgsSymbolLayerUtils.toSize(['4', '5'])
        self.assertTrue(ok)
        self.assertEqual(s2, QSizeF(4, 5))

        # string values
        s = QSizeF()
        string = QgsSymbolLayerUtils.encodeSize(s)
        s2, ok = QgsSymbolLayerUtils.toSize(string)
        self.assertTrue(ok)
        self.assertEqual(s2, s)
        s = QSizeF(1.5, 2.5)
        string = QgsSymbolLayerUtils.encodeSize(s)
        s2, ok = QgsSymbolLayerUtils.toSize(string)
        self.assertTrue(ok)
        self.assertEqual(s2, s)

        # bad string
        s2, ok = QgsSymbolLayerUtils.toSize('')
        self.assertFalse(ok)
        self.assertEqual(s2, QSizeF())

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

    def testToPoint(self):
        s2, ok = QgsSymbolLayerUtils.toPoint(None)
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toPoint(4)
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toPoint('4')
        self.assertFalse(ok)

        # arrays
        s2, ok = QgsSymbolLayerUtils.toPoint([4])
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toPoint([])
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toPoint([4, 5, 6])
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toPoint([4, 5])
        self.assertTrue(ok)
        self.assertEqual(s2, QPointF(4, 5))

        s2, ok = QgsSymbolLayerUtils.toPoint(['4', '5'])
        self.assertTrue(ok)
        self.assertEqual(s2, QPointF(4, 5))

        # string values
        s = QPointF()
        string = QgsSymbolLayerUtils.encodePoint(s)
        s2, ok = QgsSymbolLayerUtils.toPoint(string)
        self.assertTrue(ok)
        self.assertEqual(s2, s)
        s = QPointF(1.5, 2.5)
        string = QgsSymbolLayerUtils.encodePoint(s)
        s2, ok = QgsSymbolLayerUtils.toPoint(string)
        self.assertTrue(ok)
        self.assertEqual(s2, s)

        # bad string
        s2, ok = QgsSymbolLayerUtils.toPoint('')
        self.assertFalse(ok)
        self.assertEqual(s2, QPointF())

    def testDecodeArrowHeadType(self):
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType(0)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadSingle)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType('single')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadSingle)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType('   SINGLE   ')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadSingle)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType(1)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadReversed)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType('reversed')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadReversed)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType(2)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadDouble)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType('double')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadDouble)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType('xxxxx')
        self.assertFalse(ok)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType(34)
        self.assertFalse(ok)

    def testDecodeArrowType(self):
        type, ok = QgsSymbolLayerUtils.decodeArrowType(0)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowPlain)
        type, ok = QgsSymbolLayerUtils.decodeArrowType('plain')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowPlain)
        type, ok = QgsSymbolLayerUtils.decodeArrowType('   PLAIN   ')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowPlain)
        type, ok = QgsSymbolLayerUtils.decodeArrowType(1)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowLeftHalf)
        type, ok = QgsSymbolLayerUtils.decodeArrowType('lefthalf')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowLeftHalf)
        type, ok = QgsSymbolLayerUtils.decodeArrowType(2)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowRightHalf)
        type, ok = QgsSymbolLayerUtils.decodeArrowType('righthalf')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowRightHalf)
        type, ok = QgsSymbolLayerUtils.decodeArrowType('xxxxx')
        self.assertFalse(ok)
        type, ok = QgsSymbolLayerUtils.decodeArrowType(34)
        self.assertFalse(ok)

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
