# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRasterBandComboBox.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/05/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import os

from qgis.core import QgsRasterLayer
from qgis.gui import QgsRasterBandComboBox
from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QFileInfo
from qgis.PyQt.QtTest import QSignalSpy
from utilities import unitTestDataPath

start_app()


class TestQgsRasterBandComboBox(unittest.TestCase):

    def testNoLayer(self):
        """
        Test widget with no layer
        """

        combo = QgsRasterBandComboBox()
        self.assertFalse(combo.layer())
        self.assertEqual(combo.currentBand(), -1)

        combo.setShowNotSetOption(True)
        self.assertEqual(combo.currentBand(), -1)

        combo.setBand(11111)
        self.assertEqual(combo.currentBand(), -1)
        combo.setBand(-11111)
        self.assertEqual(combo.currentBand(), -1)

    def testOneBandRaster(self):
        path = os.path.join(unitTestDataPath('raster'),
                            'band1_float32_noct_epsg4326.tif')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer = QgsRasterLayer(path, base_name)
        self.assertTrue(layer)

        combo = QgsRasterBandComboBox()
        combo.setLayer(layer)
        self.assertEqual(combo.layer(), layer)
        self.assertEqual(combo.currentBand(), 1)
        self.assertEqual(combo.count(), 1)

        combo.setShowNotSetOption(True)
        self.assertEqual(combo.currentBand(), 1)
        self.assertEqual(combo.count(), 2)
        combo.setBand(-1)
        self.assertEqual(combo.currentBand(), -1)
        combo.setBand(1)
        self.assertEqual(combo.currentBand(), 1)

        combo.setShowNotSetOption(False)
        self.assertEqual(combo.currentBand(), 1)
        self.assertEqual(combo.count(), 1)

    def testMultiBandRaster(self):
        path = os.path.join(unitTestDataPath('raster'),
                            'band3_float32_noct_epsg4326.tif')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer = QgsRasterLayer(path, base_name)
        self.assertTrue(layer)

        combo = QgsRasterBandComboBox()
        combo.setLayer(layer)
        self.assertEqual(combo.layer(), layer)
        self.assertEqual(combo.currentBand(), 1)
        self.assertEqual(combo.count(), 3)
        combo.setBand(2)
        self.assertEqual(combo.currentBand(), 2)

        combo.setShowNotSetOption(True)
        self.assertEqual(combo.currentBand(), 2)
        self.assertEqual(combo.count(), 4)

        combo.setShowNotSetOption(False)
        self.assertEqual(combo.currentBand(), 2)
        self.assertEqual(combo.count(), 3)

    def testSignals(self):
        path = os.path.join(unitTestDataPath('raster'),
                            'band3_float32_noct_epsg4326.tif')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer = QgsRasterLayer(path, base_name)
        self.assertTrue(layer)

        combo = QgsRasterBandComboBox()
        combo.setLayer(layer)

        signal_spy = QSignalSpy(combo.bandChanged)
        combo.setBand(2)
        self.assertEqual(len(signal_spy), 1)
        self.assertEqual(signal_spy[0][0], 2)
        combo.setBand(3)
        self.assertEqual(len(signal_spy), 2)
        self.assertEqual(signal_spy[1][0], 3)


if __name__ == '__main__':
    unittest.main()
