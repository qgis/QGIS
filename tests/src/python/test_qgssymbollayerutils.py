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

from qgis.core import (QgsSymbolLayerUtils,
                       QgsMapSettings,
                       QgsRectangle,
                       QgsRenderContext,
                       QgsUnitTypes)
from qgis.PyQt.QtCore import (Qt, QSize, QSizeF, QPointF)
from qgis.testing import unittest


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

    def testConvertToMapUnits(self):
        # test QgsSymbolLayerUtils::convertToMapUnits() without QgsMapUnitScale

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841
        size = QgsSymbolLayerUtils.convertToMapUnits(r, 2, QgsUnitTypes.RenderMapUnits)
        self.assertEqual(size, 2.0)
        size = QgsSymbolLayerUtils.convertToMapUnits(r, 2, QgsUnitTypes.RenderMillimeters)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = QgsSymbolLayerUtils.convertToMapUnits(r, 5.66929, QgsUnitTypes.RenderPoints)
        self.assertAlmostEqual(size, 47.2440833, places=5)
        size = QgsSymbolLayerUtils.convertToMapUnits(r, 2, QgsUnitTypes.RenderPixels)
        self.assertAlmostEqual(size, 4.0, places=5)

    def testConvertFromMapUnits(self):
        # test QgsSymbolLayerUtils::convertToMapUnits() without QgsMapUnitScale

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841
        size = QgsSymbolLayerUtils.convertFromMapUnits(r, 2, QgsUnitTypes.RenderMapUnits)
        self.assertEqual(size, 2.0)
        size = QgsSymbolLayerUtils.convertFromMapUnits(r, 50, QgsUnitTypes.RenderMillimeters)
        self.assertAlmostEqual(size, 2.1166666666, places=5)
        size = QgsSymbolLayerUtils.convertFromMapUnits(r, 50, QgsUnitTypes.RenderPoints)
        self.assertAlmostEqual(size, 6.0000000015, places=5)
        size = QgsSymbolLayerUtils.convertFromMapUnits(r, 4, QgsUnitTypes.RenderPixels)
        self.assertAlmostEqual(size, 2.0, places=5)

if __name__ == '__main__':
    unittest.main()
