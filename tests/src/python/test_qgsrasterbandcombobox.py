"""QGIS Unit tests for QgsRasterBandComboBox.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "09/05/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

import os
import unittest

import numpy as np
from osgeo import gdal
from qgis.core import QgsRasterLayer
from qgis.gui import QgsRasterBandComboBox
from qgis.PyQt.QtCore import QFileInfo
from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import QgisTestCase, start_app
from utilities import unitTestDataPath

start_app()


class TestQgsRasterBandComboBox(QgisTestCase):
    def testNoLayer(self):
        """
        Test widget with no layer
        """

        combo = QgsRasterBandComboBox()
        self.assertFalse(combo.layer())
        self.assertEqual(combo.currentBand(), -1)
        self.assertTrue(combo.isEditable())

        combo.setShowNotSetOption(True)
        self.assertEqual(combo.currentBand(), -1)
        self.assertTrue(combo.isEditable())

        combo.setBand(11111)
        self.assertEqual(combo.currentBand(), 11111)
        combo.setBand(-11111)
        self.assertEqual(combo.currentBand(), -1)

    def testInvalidLayer(self):
        layer = QgsRasterLayer("blah", "blah")
        self.assertTrue(layer)
        self.assertFalse(layer.isValid())
        combo = QgsRasterBandComboBox()
        combo.setLayer(layer)
        self.assertEqual(combo.count(), 0)
        self.assertTrue(combo.isEditable())

        signal_spy = QSignalSpy(combo.bandChanged)
        combo.setBand(11111)
        self.assertEqual(len(signal_spy), 1)
        self.assertEqual(signal_spy[-1][0], 11111)
        self.assertEqual(combo.currentBand(), 11111)
        combo.setBand(-11111)
        self.assertEqual(combo.currentBand(), -1)
        self.assertEqual(len(signal_spy), 2)
        self.assertEqual(signal_spy[-1][0], -1)

        # replace with valid layer
        path = os.path.join(
            unitTestDataPath("raster"), "band3_float32_noct_epsg4326.tif"
        )
        info = QFileInfo(path)
        base_name = info.baseName()
        layer2 = QgsRasterLayer(path, base_name)
        self.assertTrue(layer2.isValid())

        combo.setBand(2)
        self.assertEqual(combo.currentBand(), 2)
        self.assertEqual(len(signal_spy), 3)
        self.assertEqual(signal_spy[-1][0], 2)
        combo.setLayer(layer2)
        self.assertEqual(combo.count(), 3)
        self.assertFalse(combo.isEditable())
        self.assertEqual(combo.currentBand(), 2)
        self.assertEqual(len(signal_spy), 3)

        # back to invalid
        combo.setLayer(layer)
        self.assertEqual(combo.count(), 0)
        self.assertTrue(combo.isEditable())
        self.assertEqual(combo.currentBand(), 2)
        self.assertEqual(len(signal_spy), 3)

        # with not set option
        combo.setShowNotSetOption(True)
        combo.setBand(-1)
        self.assertEqual(combo.currentBand(), -1)
        self.assertEqual(len(signal_spy), 4)
        self.assertEqual(signal_spy[-1][0], -1)
        combo.setBand(3)
        self.assertEqual(combo.currentBand(), 3)
        self.assertEqual(len(signal_spy), 5)
        self.assertEqual(signal_spy[-1][0], 3)
        combo.setBand(-1)
        self.assertEqual(combo.currentBand(), -1)
        self.assertEqual(len(signal_spy), 6)
        self.assertEqual(signal_spy[-1][0], -1)

        combo.setLayer(layer2)
        self.assertEqual(combo.currentBand(), -1)
        self.assertEqual(len(signal_spy), 6)

        combo.setLayer(layer)
        combo.setCurrentText("bad")
        self.assertEqual(combo.currentBand(), -1)
        self.assertEqual(len(signal_spy), 6)
        self.assertEqual(signal_spy[-1][0], -1)
        combo.setCurrentText("5")
        self.assertEqual(combo.currentBand(), 5)
        self.assertEqual(len(signal_spy), 7)
        self.assertEqual(signal_spy[-1][0], 5)
        combo.setCurrentText("6.5")
        self.assertEqual(combo.currentBand(), -1)
        self.assertEqual(len(signal_spy), 8)
        self.assertEqual(signal_spy[-1][0], -1)
        combo.setCurrentText("5")
        self.assertEqual(combo.currentBand(), 5)
        self.assertEqual(len(signal_spy), 9)
        self.assertEqual(signal_spy[-1][0], 5)
        combo.setCurrentText("Not set")
        self.assertEqual(combo.currentBand(), -1)
        self.assertEqual(len(signal_spy), 10)
        self.assertEqual(signal_spy[-1][0], -1)

    def testOneBandRaster(self):
        path = os.path.join(
            unitTestDataPath("raster"), "band1_float32_noct_epsg4326.tif"
        )
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
        path = os.path.join(
            unitTestDataPath("raster"), "band3_float32_noct_epsg4326.tif"
        )
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
        path = os.path.join(
            unitTestDataPath("raster"), "band3_float32_noct_epsg4326.tif"
        )
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

    def test_GMF_PER_DATASET_mask_band(self):
        """Test issue GH #64642 - rasters with a GMF_PER_DATASET mask band"""

        # Create a temporary raster with a GMF_PER_DATASET mask band
        temp_path = "/vsimem/test_mask_band.tif"
        driver = gdal.GetDriverByName("GTiff")
        dataset = driver.Create(temp_path, 2, 2, 1, gdal.GDT_Byte)
        band = dataset.GetRasterBand(1)
        np_data = np.array([[1, 2], [3, 4]], dtype=np.uint8)
        band.WriteArray(np_data)
        band.CreateMaskBand(gdal.GMF_PER_DATASET)
        mask_band = band.GetMaskBand()
        np_mask = np.array([[0, 255], [255, 0]], dtype=np.uint8)
        mask_band.WriteArray(np_mask)
        gdal_num_bands = dataset.RasterCount
        dataset = None

        # Load the raster via QGIS GDAL provider
        layer = QgsRasterLayer(temp_path, "test_mask_band")
        self.assertTrue(layer.isValid(), "Raster with mask band failed to load")
        provider = layer.dataProvider()

        combo = QgsRasterBandComboBox()
        # Check that GDAL does not raise any error
        gdal.ErrorReset()
        combo.setLayer(layer)
        err_mesg = gdal.GetLastErrorMsg()
        self.assertEqual(err_mesg, "", f"GDAL error occurred: {err_mesg}")


if __name__ == "__main__":
    unittest.main()
