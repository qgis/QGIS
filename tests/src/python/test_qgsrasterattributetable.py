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

from qgis.PyQt.QtCore import QTemporaryDir, QVariant


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
        self.temp_dir = QTemporaryDir()
        self.temp_path = self.temp_dir.path()

        # Create a 2x2 int raster

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
        self.uri_2x2_2_BANDS_INT16 = os.path.join(self.temp_path, '2x2_2_BANDS_INT16.tif')
        dst_ds = gdal.GetDriverByName('GTiff').Create(
            self.uri_2x2_2_BANDS_INT16, ny, nx, 2, gdal.GDT_Int16)

        dst_ds.SetGeoTransform(geotransform)    # specify coords
        srs = osr.SpatialReference()            # establish encoding
        srs.ImportFromEPSG(3857)                # WGS84 lat/long
        dst_ds.SetProjection(srs.ExportToWkt())  # export coords to file
        dst_ds.GetRasterBand(1).WriteArray(r_pixels)   # write r-band to the raster
        dst_ds.GetRasterBand(2).WriteArray(g_pixels)  # write g-band to the raster

        dst_ds.FlushCache()                     # write to disk
        dst_ds = None

        # Create a 2x2 single band float raster

        #  Initialize the Image Size
        image_size = (2, 2)

        #  Create Each Channel
        r_pixels = np.zeros((image_size), dtype=np.float)

        r_pixels[0, 0] = -1E23
        r_pixels[0, 1] = 2.345
        r_pixels[1, 0] = 3.456E12
        r_pixels[1, 1] = 4.567E23

        self.uri_2x2_1_BAND_FLOAT = os.path.join(self.temp_path, '2x2_1_BAND_FLOAT.tif')
        dst_ds = gdal.GetDriverByName('GTiff').Create(
            self.uri_2x2_1_BAND_FLOAT, ny, nx, 1, gdal.GDT_Float32)

        dst_ds.SetGeoTransform(geotransform)    # specify coords
        srs = osr.SpatialReference()            # establish encoding
        srs.ImportFromEPSG(3857)                # WGS84 lat/long
        dst_ds.SetProjection(srs.ExportToWkt())  # export coords to file
        dst_ds.GetRasterBand(1).WriteArray(r_pixels)   # write r-band to the raster

        dst_ds.FlushCache()                     # write to disk
        dst_ds = None

    def testRat(self):

        # Create RAT
        rat = QgsRasterAttributeTable()
        rat.appendField(QgsRasterAttributeTable.Field('Value', QgsRasterAttributeTable.FieldUsage.MinMax, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Count', QgsRasterAttributeTable.FieldUsage.PixelCount, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Class', QgsRasterAttributeTable.FieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class2', QgsRasterAttributeTable.FieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class3', QgsRasterAttributeTable.FieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Red', QgsRasterAttributeTable.FieldUsage.Red, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Green', QgsRasterAttributeTable.FieldUsage.Green, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Blue', QgsRasterAttributeTable.FieldUsage.Blue, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Double', QgsRasterAttributeTable.FieldUsage.Generic, QVariant.Double))

        data_rows = [
            [0, 1, 'zero', 'zero2', 'zero3', 0, 10, 100, 1.234],
            [2, 1, 'one', 'one2', 'one3', 100, 20, 0, 0.998],
            [4, 2, 'two', 'two2', 'two3', 200, 30, 50, 123456],
        ]

        for row in data_rows:
            rat.appendRow(row)

        raster = QgsRasterLayer(self.uri_2x2_2_BANDS_INT16)
        self.assertTrue(raster.isValid())
        raster.dataProvider().setAttributeTable(1, rat)
        d = raster.dataProvider()
        self.assertTrue(d.writeNativeAttributeTable())  # spellok

        # Check written data
        raster = QgsRasterLayer(self.uri_2x2_2_BANDS_INT16)
        self.assertTrue(raster.isValid())
        d = raster.dataProvider()
        self.assertTrue(d.readNativeAttributeTable())
        self.assertIsNone(d.attributeTable(2))

        rat = d.attributeTable(1)
        rat.isValid()
        rat.fields()
        self.assertEqual([f.name for f in rat.fields()], ['Value', 'Count', 'Class', 'Class2', 'Class3', 'Red', 'Green', 'Blue', 'Double'])
        self.assertEqual([f.type for f in rat.fields()], [QVariant.Int, QVariant.Int, QVariant.String, QVariant.String, QVariant.String, QVariant.Int, QVariant.Int, QVariant.Int, QVariant.Double])
        self.assertEqual(rat.data(), [
            [0, 1, 'zero', 'zero2', 'zero3', 0, 10, 100, 1.234],
            [2, 1, 'one', 'one2', 'one3', 100, 20, 0, 0.998],
            [4, 2, 'two', 'two2', 'two3', 200, 30, 50, 123456]])

        # Band 2
        data_rows = [
            [1, 1, 'one', 'one2', 'one3', 100, 20, 10],
            [3, 1, 'three', 'three2', 'tree3', 200, 10, 20],
            [3, 1, 'five', 'five2', 'five3', 50, 40, 250],
        ]

        rat = QgsRasterAttributeTable()
        rat.appendField(QgsRasterAttributeTable.Field('Value', QgsRasterAttributeTable.FieldUsage.MinMax, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Count', QgsRasterAttributeTable.FieldUsage.PixelCount, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Class', QgsRasterAttributeTable.FieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class2', QgsRasterAttributeTable.FieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class3', QgsRasterAttributeTable.FieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Red', QgsRasterAttributeTable.FieldUsage.Red, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Green', QgsRasterAttributeTable.FieldUsage.Green, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Blue', QgsRasterAttributeTable.FieldUsage.Blue, QVariant.Int))

        for row in data_rows:
            rat.appendRow(row)

        raster = QgsRasterLayer(self.uri_2x2_2_BANDS_INT16)
        self.assertTrue(raster.isValid())
        raster.dataProvider().setAttributeTable(2, rat)
        d = raster.dataProvider()
        self.assertTrue(d.writeNativeAttributeTable())  # spellok

        # Check written data
        raster = QgsRasterLayer(self.uri_2x2_2_BANDS_INT16)
        self.assertTrue(raster.isValid())
        d = raster.dataProvider()
        self.assertTrue(d.readNativeAttributeTable())
        rat = d.attributeTable(1)
        rat.isValid()
        rat.fields()
        self.assertEqual([f.name for f in rat.fields()], ['Value', 'Count', 'Class', 'Class2', 'Class3', 'Red', 'Green', 'Blue', 'Double'])
        self.assertEqual([f.type for f in rat.fields()], [QVariant.Int, QVariant.Int, QVariant.String, QVariant.String, QVariant.String, QVariant.Int, QVariant.Int, QVariant.Int, QVariant.Double])
        self.assertEqual(rat.data(), [
            [0, 1, 'zero', 'zero2', 'zero3', 0, 10, 100, 1.234],
            [2, 1, 'one', 'one2', 'one3', 100, 20, 0, 0.998],
            [4, 2, 'two', 'two2', 'two3', 200, 30, 50, 123456]])

        rat = d.attributeTable(2)
        self.assertTrue(rat.isValid())
        self.assertTrue(rat.hasColor())
        rat.fields()
        self.assertEqual([f.name for f in rat.fields()], ['Value', 'Count', 'Class', 'Class2', 'Class3', 'Red', 'Green', 'Blue'])
        self.assertEqual(rat.data(), [
            [1, 1, 'one', 'one2', 'one3', 100, 20, 10],
            [3, 1, 'three', 'three2', 'tree3', 200, 10, 20],
            [3, 1, 'five', 'five2', 'five3', 50, 40, 250],
        ])

        # Test DBF RATs
        self.assertTrue(d.writeFileBasedAttributeTable(1, self.uri_2x2_2_BANDS_INT16))
        del raster
        os.unlink(self.uri_2x2_2_BANDS_INT16 + '.aux.xml')
        raster = QgsRasterLayer(self.uri_2x2_2_BANDS_INT16)
        self.assertTrue(raster.isValid())
        d = raster.dataProvider()
        self.assertFalse(d.readNativeAttributeTable())
        self.assertTrue(d.readFileBasedAttributeTable(1, self.uri_2x2_2_BANDS_INT16 + '.vat.dbf'))
        rat = d.attributeTable(1)
        self.assertTrue(rat.isValid())
        self.assertEqual([f.name for f in rat.fields()], ['Value', 'Count', 'Class', 'Class2', 'Class3', 'Red', 'Green', 'Blue', 'Double'])
        self.assertEqual([f.type for f in rat.fields()], [QVariant.Int, QVariant.Int, QVariant.String, QVariant.String, QVariant.String, QVariant.Int, QVariant.Int, QVariant.Int, QVariant.Double])
        self.assertEqual(rat.data(), [
            [0, 1, 'zero', 'zero2', 'zero3', 0, 10, 100, 1.234],
            [2, 1, 'one', 'one2', 'one3', 100, 20, 0, 0.998],
            [4, 2, 'two', 'two2', 'two3', 200, 30, 50, 123456]])

    def testWriteRead(self):

        # Create RAT
        rat = QgsRasterAttributeTable()
        rat.appendField(QgsRasterAttributeTable.Field('Value', QgsRasterAttributeTable.FieldUsage.MinMax, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Count', QgsRasterAttributeTable.FieldUsage.PixelCount, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Class', QgsRasterAttributeTable.FieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class2', QgsRasterAttributeTable.FieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class3', QgsRasterAttributeTable.FieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Red', QgsRasterAttributeTable.FieldUsage.Red, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Green', QgsRasterAttributeTable.FieldUsage.Green, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Blue', QgsRasterAttributeTable.FieldUsage.Blue, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Double', QgsRasterAttributeTable.FieldUsage.Generic, QVariant.Double))

        data_rows = [
            [0, 1, 'zero', 'zero2', 'zero3', 0, 10, 100, 1.234],
            [2, 1, 'one', 'one2', 'one3', 100, 20, 0, 0.998],
            [4, 2, 'two', 'two2', 'two3', 200, 30, 50, 123456],
        ]

        usages = [f.usage for f in rat.fields()]

        for row in data_rows:
            rat.appendRow(row)

        rat_path = os.path.join(self.temp_path, 'test_rat1.vat.dbf')
        self.assertTrue(rat.isDirty())
        self.assertTrue(rat.writeToFile(rat_path)[0])
        self.assertTrue(os.path.exists(rat_path))
        self.assertFalse(rat.isDirty())

        # Read it back
        self.assertTrue(rat.readFromFile(rat_path)[0])
        self.assertTrue(rat.isValid())
        self.assertTrue(rat.hasColor())
        self.assertEqual([f.name for f in rat.fields()], ['Value', 'Count', 'Class', 'Class2', 'Class3', 'Red', 'Green', 'Blue', 'Double'])
        self.assertEqual([f.usage for f in rat.fields()], usages)
        self.assertEqual([f.type for f in rat.fields()], [QVariant.LongLong, QVariant.LongLong, QVariant.String, QVariant.String, QVariant.String, QVariant.Int, QVariant.Int, QVariant.Int, QVariant.Double])
        self.assertEqual(rat.data(), [
            [0, 1, 'zero', 'zero2', 'zero3', 0, 10, 100, 1.234],
            [2, 1, 'one', 'one2', 'one3', 100, 20, 0, 0.998],
            [4, 2, 'two', 'two2', 'two3', 200, 30, 50, 123456]])


if __name__ == '__main__':
    unittest.main()
