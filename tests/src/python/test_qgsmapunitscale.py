"""QGIS Unit tests for QgsMapUnitScale.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2015-09"
__copyright__ = "Copyright 2015, The QGIS Project"

from qgis.PyQt.QtCore import QSize
from qgis.core import (
    QgsMapSettings,
    QgsMapUnitScale,
    QgsRectangle,
    QgsRenderContext,
    QgsSymbolLayerUtils,
)
from qgis.testing import unittest


class PyQgsMapUnitScale(unittest.TestCase):

    def testConstructor(self):
        # test creating QgsMapUnitScale
        c = QgsMapUnitScale()
        self.assertEqual(c.minScale, 0)
        self.assertEqual(c.maxScale, 0)

        c = QgsMapUnitScale(10000, 200)
        self.assertEqual(c.minScale, 10000)
        self.assertEqual(c.maxScale, 200)

    def testEquality(self):
        # test equality operator

        c1 = QgsMapUnitScale(10000, 200)
        c1.minSizeMMEnabled = True
        c1.minSizeMM = 3
        c1.maxSizeMMEnabled = True
        c1.maxSizeMM = 8
        c2 = QgsMapUnitScale(10000, 200)
        c2.minSizeMMEnabled = True
        c2.minSizeMM = 3
        c2.maxSizeMMEnabled = True
        c2.maxSizeMM = 8
        self.assertEqual(c1, c2)

        c2.minScale = 2500.0
        self.assertNotEqual(c1, c2)
        c2.minScale = 10000

        c2.maxScale = 142.857
        self.assertNotEqual(c1, c2)
        c2.maxScale = 200

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
        ms.setOutputDpi(75)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()

        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.0, places=5)

        # add a minimum scale less than the renderer scale, so should be no change
        c.minScale = 350000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.0, places=5)

        # minimum scale greater than the renderer scale, so should be limited to minScale
        c.minScale = 150000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 1.0276160, places=5)
        c.minScale = 50000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 0.3425386, places=5)
        c.minScale = 350000000.0

        # add a maximum scale greater than the renderer scale, so should be no change
        c.maxScale = 150000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.0, places=5)

        # maximum scale less than the renderer scale, so should be limited to maxScale
        c.maxScale = 350000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.3977706, places=5)
        c.maxScale = 500000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 3.4253867, places=5)

        # test resetting to min/max
        c.minScale = 0
        c.maxScale = 0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.0, places=5)

    def testEncodeDecode(self):
        # test encoding and decoding QgsMapUnitScale

        s = QgsMapUnitScale()
        s.minScale = 100
        s.maxScale = 50
        s.minSizeMMEnabled = True
        s.minSizeMM = 3
        s.maxSizeMMEnabled = False
        s.maxSizeMM = 99

        encode = QgsSymbolLayerUtils.encodeMapUnitScale(s)
        r = QgsSymbolLayerUtils.decodeMapUnitScale(encode)
        self.assertEqual(s, r)

        # check old style encoding
        encode = "9,78.3"
        r = QgsSymbolLayerUtils.decodeMapUnitScale(encode)
        self.assertAlmostEqual(r.minScale, 1.0 / 9, 3)
        self.assertAlmostEqual(r.maxScale, 1.0 / 78.3, 3)
        self.assertFalse(r.minSizeMMEnabled)
        self.assertEqual(r.minSizeMM, 0)
        self.assertFalse(r.maxSizeMMEnabled)
        self.assertEqual(r.maxSizeMM, 0)


if __name__ == "__main__":
    unittest.main()
