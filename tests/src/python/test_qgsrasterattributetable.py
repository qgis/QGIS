# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRasterAttributeTable.

From build dir, run:
ctest -R PyQgsRasterAttributeTable -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from builtins import str

__author__ = 'Alessandro Pasotti'
__date__ = '20/08/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA

from osgeo import gdal
from osgeo import osr
import numpy as np

import os

from qgis.PyQt.QtCore import QTemporaryDir


from qgis.core import (Qgis,
                       QgsMapLayerServerProperties,
                       QgsRaster,
                       QgsRasterLayer,
                       QgsRasterAttributeTable,
                       )

from qgis.testing import start_app, unittest
from qgis.testing.mocked import get_iface

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsRasterAttributeTable(unittest.TestCase):

    def setUp(self):
        self.iface = get_iface()

    def createTestRaster(self, path):

        # Create a 2x2 int raster
        dest = path

        #  Initialize the Image Size
        image_size = (2, 2)

        #  Choose some Geographic Transform (Around Lake Tahoe)
        lat = [39, 38.5]
        lon = [-120, -119.5]

        #  Create Each Channel
        r_pixels = np.zeros((image_size), dtype=np.uint16)
        g_pixels = np.zeros((image_size), dtype=np.uint16)

        r_pixels[0, 0] = 0
        g_pixels[0, 0] = 1

        r_pixels[0, 1] = 2
        g_pixels[0, 1] = 3

        r_pixels[1, 0] = 4
        g_pixels[1, 0] = 5

        r_pixels[1, 1] = 4
        g_pixels[1, 1] = 5

        # set geotransform
        nx = image_size[0]
        ny = image_size[1]
        xmin, ymin, xmax, ymax = [min(lon), min(lat), max(lon), max(lat)]
        xres = (xmax - xmin) / float(nx)
        yres = (ymax - ymin) / float(ny)
        geotransform = (xmin, xres, 0, ymax, 0, -yres)

        # create the 2-band raster file
        dst_ds = gdal.GetDriverByName('GTiff').Create(
            os.path.join(dest, '2x2_2_BANDS_INT16.tif'), ny, nx, 2, gdal.GDT_Int16)

        dst_ds.SetGeoTransform(geotransform)    # specify coords
        srs = osr.SpatialReference()            # establish encoding
        srs.ImportFromEPSG(3857)                # WGS84 lat/long
        dst_ds.SetProjection(srs.ExportToWkt())  # export coords to file
        dst_ds.GetRasterBand(1).WriteArray(r_pixels)   # write r-band to the raster
        dst_ds.GetRasterBand(2).WriteArray(g_pixels)  # write g-band to the raster

        dst_ds.FlushCache()                     # write to disk
        dst_ds = None

        # Create RAT
        rat = QgsRasterAttributeTable()
        rat.appendField(QgsRasterAttributeTable.Field('Value', gdal.GFU_MinMax, gdal.GFT_Integer))
        rat.appendField(QgsRasterAttributeTable.Field('Count', gdal.GFU_PixelCount, gdal.GFT_Integer))
        rat.appendField(QgsRasterAttributeTable.Field('Class', gdal.GFU_Name, gdal.GFT_String))
        rat.appendField(QgsRasterAttributeTable.Field('Class2', gdal.GFU_Name, gdal.GFT_String))
        rat.appendField(QgsRasterAttributeTable.Field('Class3', gdal.GFU_Generic, gdal.GFT_String))
        rat.appendField(QgsRasterAttributeTable.Field('Red', gdal.GFU_Red, gdal.GFT_Integer))
        rat.appendField(QgsRasterAttributeTable.Field('Green', gdal.GFU_Green, gdal.GFT_Integer))
        rat.appendField(QgsRasterAttributeTable.Field('Blue', gdal.GFU_Blue, gdal.GFT_Integer))

        data = {
            'Value': [0, 2, 4],
            'Count': [1, 1, 2],
            'Class': ['zero', 'one', 'two'],
            'Class2': ['zero2', 'one2', 'two2'],
            'Class3': ['zero3', 'one3', 'two3'],
            'Red': [0, 100, 200],
            'Green': [10, 20, 30],
            'Blue': [100, 0, 50],
        }

        for row in data:
            rat.appendRow(row)

        assert rat.saveToFile(os.path.join(
            dest, '2x2_2_BANDS_INT16.tif.aux.xml')), 'Error saving RAT for band 1'

        # Band 2
        data = {
            'Value': [1, 3, 5],
            'Count': [1, 1, 2],
            'Class': ['one', 'three', 'five'],
            'Class2': ['one2', 'three2', 'five2'],
            'Class3': ['one3', 'three3', 'five3'],
            'Red': [100, 200, 50],
            'Green': [20, 10, 40],
            'Blue': [10, 20, 250],
        }

        rat = RAT(data, False, fields, os.path.join(
            dest, '2x2_2_BANDS_INT16.tif.aux.xml'))
        assert rat.save(2), 'Error saving RAT for band 2'

        # Create a 2x2 single band float raster

        #  Initialize the Image Size
        image_size = (2, 2)

        #  Create Each Channel
        r_pixels = np.zeros((image_size), dtype=np.float)

        r_pixels[0, 0] = -1E23
        r_pixels[0, 1] = 2.345
        r_pixels[1, 0] = 3.456E12
        r_pixels[1, 1] = 4.567E23

        # set geotransform
        # create the 1-band raster file
        dst_ds = gdal.GetDriverByName('GTiff').Create(
            os.path.join(dest, '2x2_1_BAND_FLOAT.tif'), ny, nx, 1, gdal.GDT_Float32)

        dst_ds.SetGeoTransform(geotransform)    # specify coords
        srs = osr.SpatialReference()            # establish encoding
        srs.ImportFromEPSG(3857)                # WGS84 lat/long
        dst_ds.SetProjection(srs.ExportToWkt())  # export coords to file
        dst_ds.GetRasterBand(1).WriteArray(r_pixels)   # write r-band to the raster

        dst_ds.FlushCache()                     # write to disk
        dst_ds = None

        # Create RAT
        fields = []
        rat.appendField(QgsRasterAttributeTable.Field('Value Min', gdal.GFU_Min, gdal.GFT_Real))
        rat.appendField(QgsRasterAttributeTable.Field('Value Max', gdal.GFU_Max, gdal.GFT_Real))
        rat.appendField(QgsRasterAttributeTable.Field('Class', gdal.GFU_Name, gdal.GFT_String))
        rat.appendField(QgsRasterAttributeTable.Field('Class2', gdal.GFU_Name, gdal.GFT_String))
        rat.appendField(QgsRasterAttributeTable.Field('Class3', gdal.GFU_Generic, gdal.GFT_String))
        rat.appendField(QgsRasterAttributeTable.Field('Red', gdal.GFU_Red, gdal.GFT_Integer))
        rat.appendField(QgsRasterAttributeTable.Field('Green', gdal.GFU_Green, gdal.GFT_Integer))
        rat.appendField(QgsRasterAttributeTable.Field('Blue', gdal.GFU_Blue, gdal.GFT_Integer))

        fields = {field.name: field for field in fields}

        data = {
            'Value Min': [-1E25, 3E12, 1E20],
            'Value Max': [3E12, 1E20, 5E25],
            'Count': [1, 1, 2],
            'Class': ['zero', 'one', 'two'],
            'Class2': ['zero2', 'one2', 'zero2'],  # for classify test!
            'Class3': ['zero3', 'one3', 'two3'],
            'Red': [0, 100, 200],
            'Green': [10, 20, 30],
            'Blue': [100, 0, 50],
        }

        rat = RAT(data, False, fields, os.path.join(
            dest, '2x2_1_BAND_FLOAT.tif.aux.xml'))
        assert rat.save(1), 'Error saving RAT for band 1'

    def testEmbeddedRAT(self):

        temp_dir = QTemporaryDir()
        temp_path = temp_dir.path()
        self.createTestRaster(temp_path)

        raster_layer = QgsRasterLayer(os.path.join(temp_path, '2x2_1_BAND_FLOAT.tif'))
        self.assertTrue(raster_layer.isValid())
        from IPython import embed
        embed(using=False)


if __name__ == '__main__':
    unittest.main()
