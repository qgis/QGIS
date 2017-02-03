# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRenderContext.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '16/01/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsRenderContext,
                       QgsMapSettings,
                       QgsRectangle,
                       QgsMapUnitScale,
                       QgsUnitTypes)
from qgis.PyQt.QtCore import QSize
from qgis.PyQt.QtGui import QPainter, QImage
from qgis.testing import unittest


class TestQgsRenderContext(unittest.TestCase):

    def testFromQPainter(self):
        """ test QgsRenderContext.fromQPainter """

        # no painter
        c = QgsRenderContext.fromQPainter(None)
        self.assertFalse(c.painter())
        # assuming 88 dpi as fallback
        self.assertAlmostEqual(c.scaleFactor(), 88 / 25.4, 3)

        # no painter destination
        p = QPainter()
        c = QgsRenderContext.fromQPainter(p)
        self.assertEqual(c.painter(), p)
        self.assertAlmostEqual(c.scaleFactor(), 88 / 25.4, 3)

        im = QImage(1000, 600, QImage.Format_RGB32)
        dots_per_m = 300 / 25.4 * 1000 # 300 dpi to dots per m
        im.setDotsPerMeterX(dots_per_m)
        im.setDotsPerMeterY(dots_per_m)
        p = QPainter(im)
        c = QgsRenderContext.fromQPainter(p)
        self.assertEqual(c.painter(), p)
        self.assertAlmostEqual(c.scaleFactor(), dots_per_m / 1000, 3) # scaleFactor should be pixels/mm

    def testConvertSingleUnit(self):

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()
        #self.assertEqual(r.scaleFactor(),666)

        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 0.5, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0, places=5)

        # minimum scale greater than the renderer scale, so should be limited to minScale
        c.minScale = 1 / 150000000.0
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 3.89250455, places=5)
        # only conversion from mapunits should be affected
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0, places=5)
        c.minScale = 0

        # maximum scale less than the renderer scale, so should be limited to maxScale
        c.maxScale = 1 / 350000000.0
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 0.5, places=5)
        # only conversion from mapunits should be affected
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0, places=5)

    def testConvertToPainterUnits(self):

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()

        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 1.0, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 23.622047, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 8.33333333125, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 600.0, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 2.0, places=5)

        # minimum size greater than the calculated size, so size should be limited to minSizeMM
        c.minSizeMM = 5
        c.minSizeMMEnabled = True
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 59.0551181, places=5)
        # only conversion from mapunits should be affected
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 23.622047, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 8.33333333125, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 600.0, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 2.0, places=5)
        c.minSizeMMEnabled = False

        # maximum size less than the calculated size, so size should be limited to maxSizeMM
        c.maxSizeMM = 0.1
        c.maxSizeMMEnabled = True
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 1.0, places=5)
        # only conversion from mapunits should be affected
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 23.622047, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 8.33333333125, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 600.0, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 2.0, places=5)

    def testConvertToMapUnits(self):
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()

        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertEqual(size, 2.0)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 47.2440833, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 3401.574, places=5)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 4.0, places=5)

        # minimum size greater than the calculated size, so size should be limited to minSizeMM
        c.minSizeMM = 5
        c.minSizeMMEnabled = True
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 118.1102362, places=5)
        # only conversion from mapunits should be affected
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 47.2440833, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 3401.574, places=5)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 4.0, places=5)
        c.minSizeMMEnabled = False

        # maximum size less than the calculated size, so size should be limited to maxSizeMM
        c.maxSizeMM = 0.05
        c.maxSizeMMEnabled = True
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 1.1811023622047245, places=5)
        # only conversion from mapunits should be affected
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 47.2440833, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 3401.574, places=5)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 4.0, places=5)
        c.maxSizeMMEnabled = False

        # test with minimum scale set
        c.minScale = 1 / 150000000.0
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 15.57001821, places=5)
        # only conversion from mapunits should be affected
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 47.2440833, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 3401.574, places=5)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 4.0, places=5)
        c.minScale = 0

        # test with maximum scale set
        c.maxScale = 1 / 1550000000.0
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 1.50677595625, places=5)
        # only conversion from mapunits should be affected
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 47.2440833, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 3401.574, places=5)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 4.0, places=5)
        c.maxScale = 0

    def testPixelSizeScaleFactor(self):

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()

        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 0.5, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0, places=5)

        # minimum scale greater than the renderer scale, so should be limited to minScale
        c.minScale = 1 / 150000000.0
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 3.8925045, places=5)
        # only conversion from mapunits should be affected
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.811023, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0, places=5)
        c.minScale = 0

        # maximum scale less than the renderer scale, so should be limited to maxScale
        c.maxScale = 1 / 350000000.0
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 0.5, places=5)
        # only conversion from mapunits should be affected
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0, places=5)

    def testMapUnitScaleFactor(self):
        # test QgsSymbolLayerUtils::mapUnitScaleFactor() using QgsMapUnitScale

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        c = QgsMapUnitScale()
        sf = r.convertToMapUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 1.0, places=5)
        sf = r.convertToMapUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 23.622047, places=5)
        sf = r.convertToMapUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 8.33333324723, places=5)
        sf = r.convertToMapUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 600.0, places=5)
        sf = r.convertToMapUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 2.0, places=5)


if __name__ == '__main__':
    unittest.main()
