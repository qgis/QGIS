# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapUnitScale.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2015-09'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsMapUnitScale, QgsRenderContext, QgsSymbolLayerV2Utils, QgsSymbolV2, QgsMapSettings, QgsRectangle)
from qgis.PyQt.QtCore import QSize
from qgis.testing import unittest


class PyQgsMapUnitScale(unittest.TestCase):

    def testConstructor(self):
        # test creating QgsMapUnitScale
        c = QgsMapUnitScale()
        self.assertEqual(c.minScale, 0)
        self.assertEqual(c.maxScale, 0)

        c = QgsMapUnitScale(0.0001, 0.005)
        self.assertEqual(c.minScale, 0.0001)
        self.assertEqual(c.maxScale, 0.005)

    def testEquality(self):
        # test equality operator

        c1 = QgsMapUnitScale(0.0001, 0.005)
        c1.minSizeMMEnabled = True
        c1.minSizeMM = 3
        c1.maxSizeMMEnabled = True
        c1.maxSizeMM = 8
        c2 = QgsMapUnitScale(0.0001, 0.005)
        c2.minSizeMMEnabled = True
        c2.minSizeMM = 3
        c2.maxSizeMMEnabled = True
        c2.maxSizeMM = 8
        self.assertEqual(c1, c2)

        c2.minScale = 0.0004
        self.assertNotEqual(c1, c2)
        c2.minScale = 0.0001

        c2.maxScale = 0.007
        self.assertNotEqual(c1, c2)
        c2.maxScale = 0.005

        c2.minSizeMMEnabled = False
        self.assertNotEqual(c1, c2)
        c2.minSizeMMEnabled = True

        c2.maxSizeMMEnabled = False
        self.assertNotEqual(c1, c2)
        c2.maxSizeMMEnabled = True

        c2.minSizeMM = 1
        self.assertNotEqual(c1, c2)
        c2.minSizeMM = 3

        c2.maxSizeMM = 100
        self.assertNotEqual(c1, c2)
        c2.maxSizeMM = 8

        self.assertEqual(c1, c2)

    def testMapUnitsPerPixel(self):
        # test computeMapUnitsPerPixel

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()

        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.0, places=5)

        # add a minimum scale less than the renderer scale, so should be no change
        c.minScale = 1 / 350000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.0, places=5)

        # minimum scale greater than the renderer scale, so should be limited to minScale
        c.minScale = 1 / 150000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 1.0276160, places=5)
        c.minScale = 1 / 50000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 0.3425386, places=5)
        c.minScale = 1 / 350000000.0

        # add a maximum scale greater than the renderer scale, so should be no change
        c.maxScale = 1 / 150000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.0, places=5)

        # maximum scale less than the renderer scale, so should be limited to maxScale
        c.maxScale = 1 / 350000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.3977706, places=5)
        c.maxScale = 1 / 500000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 3.4253867, places=5)

        # test resetting to min/max
        c.minScale = 0
        c.maxScale = 0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.0, places=5)

    def testLineWidthScaleFactor(self):
        # test QgsSymbolLayerV2Utils::lineWidthScaleFactor() using QgsMapUnitScale

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()

        sf = QgsSymbolLayerV2Utils.lineWidthScaleFactor(r, QgsSymbolV2.MapUnit, c)
        self.assertAlmostEqual(sf, 0.5, places=5)
        sf = QgsSymbolLayerV2Utils.lineWidthScaleFactor(r, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = QgsSymbolLayerV2Utils.lineWidthScaleFactor(r, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(sf, 1.0, places=5)

        # minimum scale greater than the renderer scale, so should be limited to minScale
        c.minScale = 1 / 150000000.0
        sf = QgsSymbolLayerV2Utils.lineWidthScaleFactor(r, QgsSymbolV2.MapUnit, c)
        self.assertAlmostEqual(sf, 3.89250455, places=5)
        # only conversion from mapunits should be affected
        sf = QgsSymbolLayerV2Utils.lineWidthScaleFactor(r, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = QgsSymbolLayerV2Utils.lineWidthScaleFactor(r, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(sf, 1.0, places=5)
        c.minScale = 0

        # maximum scale less than the renderer scale, so should be limited to maxScale
        c.maxScale = 1 / 350000000.0
        sf = QgsSymbolLayerV2Utils.lineWidthScaleFactor(r, QgsSymbolV2.MapUnit, c)
        self.assertAlmostEqual(sf, 0.5, places=5)
        # only conversion from mapunits should be affected
        sf = QgsSymbolLayerV2Utils.lineWidthScaleFactor(r, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = QgsSymbolLayerV2Utils.lineWidthScaleFactor(r, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(sf, 1.0, places=5)

    def testConvertToPainterUnits(self):
        # test QgsSymbolLayerV2Utils::convertToPainterUnits() using QgsMapUnitScale

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()

        size = QgsSymbolLayerV2Utils.convertToPainterUnits(r, 2, QgsSymbolV2.MapUnit, c)
        self.assertAlmostEqual(size, 1.0, places=5)
        size = QgsSymbolLayerV2Utils.convertToPainterUnits(r, 2, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(size, 23.622047, places=5)
        size = QgsSymbolLayerV2Utils.convertToPainterUnits(r, 2, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(size, 2.0, places=5)

        # minimum size greater than the calculated size, so size should be limited to minSizeMM
        c.minSizeMM = 5
        c.minSizeMMEnabled = True
        size = QgsSymbolLayerV2Utils.convertToPainterUnits(r, 2, QgsSymbolV2.MapUnit, c)
        self.assertAlmostEqual(size, 59.0551181, places=5)
        # only conversion from mapunits should be affected
        size = QgsSymbolLayerV2Utils.convertToPainterUnits(r, 2, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(size, 23.622047, places=5)
        size = QgsSymbolLayerV2Utils.convertToPainterUnits(r, 2, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(size, 2.0, places=5)
        c.minSizeMMEnabled = False

        # maximum size less than the calculated size, so size should be limited to maxSizeMM
        c.maxSizeMM = 0.1
        c.maxSizeMMEnabled = True
        size = QgsSymbolLayerV2Utils.convertToPainterUnits(r, 2, QgsSymbolV2.MapUnit, c)
        self.assertAlmostEqual(size, 1.0, places=5)
        # only conversion from mapunits should be affected
        size = QgsSymbolLayerV2Utils.convertToPainterUnits(r, 2, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(size, 23.622047, places=5)
        size = QgsSymbolLayerV2Utils.convertToPainterUnits(r, 2, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(size, 2.0, places=5)

    def testConvertToMapUnits(self):
        # test QgsSymbolLayerV2Utils::convertToMapUnits() using QgsMapUnitScale

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()

        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.MapUnit, c)
        self.assertEqual(size, 2.0)
        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(size, 4.0, places=5)

        # minimum size greater than the calculated size, so size should be limited to minSizeMM
        c.minSizeMM = 5
        c.minSizeMMEnabled = True
        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.MapUnit, c)
        self.assertAlmostEqual(size, 118.1102362, places=5)
        # only conversion from mapunits should be affected
        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(size, 4.0, places=5)
        c.minSizeMMEnabled = False

        # maximum size less than the calculated size, so size should be limited to maxSizeMM
        c.maxSizeMM = 0.05
        c.maxSizeMMEnabled = True
        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.MapUnit, c)
        self.assertAlmostEqual(size, 1.1811023622047245, places=5)
        # only conversion from mapunits should be affected
        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(size, 4.0, places=5)
        c.maxSizeMMEnabled = False

        # test with minimum scale set
        c.minScale = 1 / 150000000.0
        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.MapUnit, c)
        self.assertAlmostEqual(size, 15.57001821, places=5)
        # only conversion from mapunits should be affected
        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(size, 4.0, places=5)
        c.minScale = 0

        # test with maximum scale set
        c.maxScale = 1 / 1550000000.0
        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.MapUnit, c)
        self.assertAlmostEqual(size, 1.50677595625, places=5)
        # only conversion from mapunits should be affected
        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = QgsSymbolLayerV2Utils.convertToMapUnits(r, 2, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(size, 4.0, places=5)
        c.maxScale = 0

    def testPixelSizeScaleFactor(self):
        # test QgsSymbolLayerV2Utils::pixelSizeScaleFactor() using QgsMapUnitScale

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()

        sf = QgsSymbolLayerV2Utils.pixelSizeScaleFactor(r, QgsSymbolV2.MapUnit, c)
        self.assertAlmostEqual(sf, 0.5, places=5)
        sf = QgsSymbolLayerV2Utils.pixelSizeScaleFactor(r, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = QgsSymbolLayerV2Utils.pixelSizeScaleFactor(r, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(sf, 1.0, places=5)

        # minimum scale greater than the renderer scale, so should be limited to minScale
        c.minScale = 1 / 150000000.0
        sf = QgsSymbolLayerV2Utils.pixelSizeScaleFactor(r, QgsSymbolV2.MapUnit, c)
        self.assertAlmostEqual(sf, 3.8925045, places=5)
        # only conversion from mapunits should be affected
        sf = QgsSymbolLayerV2Utils.pixelSizeScaleFactor(r, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(sf, 11.811023, places=5)
        sf = QgsSymbolLayerV2Utils.pixelSizeScaleFactor(r, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(sf, 1.0, places=5)
        c.minScale = 0

        # maximum scale less than the renderer scale, so should be limited to maxScale
        c.maxScale = 1 / 350000000.0
        sf = QgsSymbolLayerV2Utils.pixelSizeScaleFactor(r, QgsSymbolV2.MapUnit, c)
        self.assertAlmostEqual(sf, 0.5, places=5)
        # only conversion from mapunits should be affected
        sf = QgsSymbolLayerV2Utils.pixelSizeScaleFactor(r, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = QgsSymbolLayerV2Utils.pixelSizeScaleFactor(r, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(sf, 1.0, places=5)

    def testMapUnitScaleFactor(self):
        # test QgsSymbolLayerV2Utils::mapUnitScaleFactor() using QgsMapUnitScale

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        c = QgsMapUnitScale()
        sf = QgsSymbolLayerV2Utils.mapUnitScaleFactor(r, QgsSymbolV2.MapUnit, c)
        self.assertAlmostEqual(sf, 1.0, places=5)
        sf = QgsSymbolLayerV2Utils.mapUnitScaleFactor(r, QgsSymbolV2.MM, c)
        self.assertAlmostEqual(sf, 23.622047, places=5)
        sf = QgsSymbolLayerV2Utils.mapUnitScaleFactor(r, QgsSymbolV2.Pixel, c)
        self.assertAlmostEqual(sf, 2.0, places=5)

    def testEncodeDecode(self):
        # test encoding and decoding QgsMapUnitScale

        s = QgsMapUnitScale()
        s.minScale = 50
        s.maxScale = 100
        s.minSizeMMEnabled = True
        s.minSizeMM = 3
        s.maxSizeMMEnabled = False
        s.maxSizeMM = 99

        encode = QgsSymbolLayerV2Utils.encodeMapUnitScale(s)
        r = QgsSymbolLayerV2Utils.decodeMapUnitScale(encode)
        self.assertEqual(s, r)

        # check old style encoding
        encode = '9,78.3'
        r = QgsSymbolLayerV2Utils.decodeMapUnitScale(encode)
        self.assertEqual(r.minScale, 9)
        self.assertEqual(r.maxScale, 78.3)
        self.assertFalse(r.minSizeMMEnabled)
        self.assertEqual(r.minSizeMM, 0)
        self.assertFalse(r.maxSizeMMEnabled)
        self.assertEqual(r.maxSizeMM, 0)


if __name__ == '__main__':
    unittest.main()
