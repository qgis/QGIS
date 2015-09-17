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

import qgis

from qgis.core import (QgsMapUnitScale, QgsRenderContext, QgsMapSettings, QgsRectangle)
from PyQt4.QtCore import QSize
from utilities import (TestCase, unittest)


class PyQgsMapUnitScale(TestCase):

    def testConstructor(self):
        #test creating QgsMapUnitScale
        c = QgsMapUnitScale()
        self.assertEqual(c.minScale, 0)
        self.assertEqual(c.maxScale, 0)

        c = QgsMapUnitScale(0.0001, 0.005)
        self.assertEqual(c.minScale, 0.0001)
        self.assertEqual(c.maxScale, 0.005)

    def testEquality(self):
        #test equality operator

        c1 = QgsMapUnitScale(0.0001, 0.005)
        c2 = QgsMapUnitScale(0.0001, 0.005)
        self.assertEqual(c1, c2)

        c2.minScale = 0.0004
        self.assertNotEqual(c1, c2)

        c2.minScale = 0.0001
        c2.maxScale = 0.007
        self.assertNotEqual(c1, c2)

        c2.maxScale = 0.005
        self.assertEqual(c1, c2)

    def testMapUnitsPerPixel(self):
        #test computeMapUnitsPerPixel

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        r = QgsRenderContext.fromMapSettings(ms)

        #renderer scale should be about 1:291937841

        #start with no min/max scale
        c = QgsMapUnitScale()

        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.0, places=5)

        #add a minimum scale less than the renderer scale, so should be no change
        c.minScale = 1 / 350000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.0, places=5)

        #minimum scale greater than the renderer scale, so should be limited to minScale
        c.minScale = 1 / 150000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 1.0276160, places=5)
        c.minScale = 1 / 50000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 0.3425386, places=5)
        c.minScale = 1 / 350000000.0

        #add a maximum scale greater than the renderer scale, so should be no change
        c.maxScale = 1 / 150000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.0, places=5)

        #maximum scale less than the renderer scale, so should be limited to maxScale
        c.maxScale = 1 / 350000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.3977706, places=5)
        c.maxScale = 1 / 500000000.0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 3.4253867, places=5)

        #test resetting to min/max
        c.minScale = 0
        c.maxScale = 0
        mup = c.computeMapUnitsPerPixel(r)
        self.assertAlmostEqual(mup, 2.0, places=5)

if __name__ == '__main__':
    unittest.main()
