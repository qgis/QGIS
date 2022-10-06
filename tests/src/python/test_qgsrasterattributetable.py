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
from qgis.PyQt.QtGui import QColor


from qgis.core import (Qgis,
                       QgsRasterLayer,
                       QgsRasterAttributeTable,
                       QgsPalettedRasterRenderer,
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

        """ Raster 2x2_2_BANDS_INT16
        Band 1
        0 2
        4 4

        Band 2
        1 3
        5 5
        """

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

        # Create the single band version of the 16 bit raster
        # 2x2_1_BAND_INT16
        self.uri_2x2_1_BAND_INT16 = os.path.join(self.temp_path, '2x2_1_BAND_INT16.tif')
        dst_ds = gdal.GetDriverByName('GTiff').Create(
            self.uri_2x2_1_BAND_INT16, ny, nx, 1, gdal.GDT_Int16)

        dst_ds.SetGeoTransform(geotransform)    # specify coords
        srs = osr.SpatialReference()            # establish encoding
        srs.ImportFromEPSG(3857)                # WGS84 lat/long
        dst_ds.SetProjection(srs.ExportToWkt())  # export coords to file
        dst_ds.GetRasterBand(1).WriteArray(r_pixels)   # write r-band to the raster

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
        rat.appendField(QgsRasterAttributeTable.Field('Value', Qgis.RasterAttributeTableFieldUsage.MinMax, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Count', Qgis.RasterAttributeTableFieldUsage.PixelCount, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Class', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class2', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class3', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Red', Qgis.RasterAttributeTableFieldUsage.Red, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Green', Qgis.RasterAttributeTableFieldUsage.Green, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Blue', Qgis.RasterAttributeTableFieldUsage.Blue, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Double', Qgis.RasterAttributeTableFieldUsage.Generic, QVariant.Double))

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
        ok, errors = rat.isValid()
        self.assertTrue(ok)
        ok, errors = d.writeNativeAttributeTable()  # spellok
        self.assertTrue(ok)

        # Check written data
        raster = QgsRasterLayer(self.uri_2x2_2_BANDS_INT16)
        self.assertTrue(raster.isValid())
        d = raster.dataProvider()
        ok, errors = d.readNativeAttributeTable()
        self.assertTrue(ok)
        self.assertIsNone(d.attributeTable(2))

        rat = d.attributeTable(1)
        self.assertEqual(rat.type(), Qgis.RasterAttributeTableType.Thematic)
        self.assertTrue(rat.isValid()[0])
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
            [5, 2, 'five', 'five2', 'five3', 50, 40, 250],
        ]

        rat = QgsRasterAttributeTable()
        rat.appendField(QgsRasterAttributeTable.Field('Value', Qgis.RasterAttributeTableFieldUsage.MinMax, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Count', Qgis.RasterAttributeTableFieldUsage.PixelCount, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Class', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class2', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class3', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Red', Qgis.RasterAttributeTableFieldUsage.Red, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Green', Qgis.RasterAttributeTableFieldUsage.Green, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Blue', Qgis.RasterAttributeTableFieldUsage.Blue, QVariant.Int))

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
        self.assertTrue(rat.isValid()[0])
        rat.fields()
        self.assertEqual([f.name for f in rat.fields()], ['Value', 'Count', 'Class', 'Class2', 'Class3', 'Red', 'Green', 'Blue', 'Double'])
        self.assertEqual([f.type for f in rat.fields()], [QVariant.Int, QVariant.Int, QVariant.String, QVariant.String, QVariant.String, QVariant.Int, QVariant.Int, QVariant.Int, QVariant.Double])
        self.assertEqual(rat.data(), [
            [0, 1, 'zero', 'zero2', 'zero3', 0, 10, 100, 1.234],
            [2, 1, 'one', 'one2', 'one3', 100, 20, 0, 0.998],
            [4, 2, 'two', 'two2', 'two3', 200, 30, 50, 123456]])

        rat = d.attributeTable(2)
        self.assertTrue(rat.isValid()[0])
        self.assertTrue(rat.hasColor())
        rat.fields()
        self.assertEqual([f.name for f in rat.fields()], ['Value', 'Count', 'Class', 'Class2', 'Class3', 'Red', 'Green', 'Blue'])
        self.assertEqual(rat.data(), [
            [1, 1, 'one', 'one2', 'one3', 100, 20, 10],
            [3, 1, 'three', 'three2', 'tree3', 200, 10, 20],
            [5, 2, 'five', 'five2', 'five3', 50, 40, 250],
        ])

        # Test DBF RATs
        self.assertTrue(d.writeFileBasedAttributeTable(1, self.uri_2x2_2_BANDS_INT16))
        del raster
        os.unlink(self.uri_2x2_2_BANDS_INT16 + '.aux.xml')
        raster = QgsRasterLayer(self.uri_2x2_2_BANDS_INT16)
        self.assertTrue(raster.isValid())
        d = raster.dataProvider()
        self.assertFalse(d.readNativeAttributeTable()[0])
        self.assertTrue(d.readFileBasedAttributeTable(1, self.uri_2x2_2_BANDS_INT16 + '.vat.dbf')[0])
        rat = d.attributeTable(1)
        self.assertTrue(rat.isValid()[0])
        self.assertEqual([f.name for f in rat.fields()], ['Value', 'Count', 'Class', 'Class2', 'Class3', 'Red', 'Green', 'Blue', 'Double'])
        # Note: when reading from DBF, count and value are always long
        self.assertEqual([f.type for f in rat.fields()], [QVariant.LongLong, QVariant.LongLong, QVariant.String, QVariant.String, QVariant.String, QVariant.Int, QVariant.Int, QVariant.Int, QVariant.Double])
        self.assertEqual(rat.data(), [
            [0, 1, 'zero', 'zero2', 'zero3', 0, 10, 100, 1.234],
            [2, 1, 'one', 'one2', 'one3', 100, 20, 0, 0.998],
            [4, 2, 'two', 'two2', 'two3', 200, 30, 50, 123456]])

        # Test color
        self.assertTrue(rat.hasColor())
        self.assertFalse(rat.hasRamp())
        self.assertEqual(rat.color(0).name(), '#000a64')
        self.assertEqual(rat.color(0).alpha(), 255)
        self.assertEqual(rat.color(1).name(), '#641400')

        self.assertEqual(len(rat.fieldsByUsage(Qgis.RasterAttributeTableFieldUsage.Blue)), 1)
        self.assertEqual(len(rat.fieldsByUsage(Qgis.RasterAttributeTableFieldUsage.Alpha)), 0)

        self.assertTrue(rat.fieldsByUsage(Qgis.RasterAttributeTableFieldUsage.Red)[0].isColor())
        self.assertFalse(rat.fieldsByUsage(Qgis.RasterAttributeTableFieldUsage.Red)[0].isRamp())

        self.assertTrue(rat.fieldByName("Blue")[1])
        self.assertTrue(rat.fieldByName("Red")[1])
        self.assertFalse(rat.fieldByName("xxx")[1])

    def testTableOperationsAndValidation(self):

        rat = QgsRasterAttributeTable()
        valid, error = rat.isValid()
        self.assertFalse(valid)
        self.assertNotEqual(len(error), 0)

        valid, error = rat.isValid()
        rat.appendField(QgsRasterAttributeTable.Field('Class', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        self.assertFalse(valid)
        self.assertNotEqual(len(error), 0)
        rat.appendField(QgsRasterAttributeTable.Field('Value', Qgis.RasterAttributeTableFieldUsage.MinMax, QVariant.Int))
        self.assertFalse(valid)
        self.assertNotEqual(len(error), 0)
        data_rows = [
            ['zero', 0],
            ['two', 2],
            ['four', 4],
        ]
        for row in data_rows:
            rat.appendRow(row)

        for row in rat.data():
            self.assertEqual(len(row), 2)

        valid, error = rat.isValid()
        self.assertTrue(valid)
        self.assertFalse(rat.hasColor())
        self.assertFalse(rat.hasRamp())

        # Add color
        rat.appendField(QgsRasterAttributeTable.Field('Red', Qgis.RasterAttributeTableFieldUsage.Red, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Green', Qgis.RasterAttributeTableFieldUsage.Green, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Blue', Qgis.RasterAttributeTableFieldUsage.Blue, QVariant.Int))
        self.assertTrue(rat.hasColor())
        self.assertFalse(rat.hasRamp())
        self.assertEqual([f.name for f in rat.fields()], ['Class', 'Value', 'Red', 'Green', 'Blue'])

        for row in rat.data():
            self.assertEqual(len(row), 5)

        # Remove fields
        self.assertFalse(rat.removeField('xxx')[0])
        self.assertTrue(rat.removeField('Red')[0])
        self.assertEqual([f.name for f in rat.fields()], ['Class', 'Value', 'Green', 'Blue'])
        for row in rat.data():
            self.assertEqual(len(row), 4)
        self.assertFalse(rat.isValid()[0])

        # Test insert and append
        # unique field already exists
        self.assertFalse(rat.appendField(QgsRasterAttributeTable.Field('Blue', Qgis.RasterAttributeTableFieldUsage.Blue, QVariant.Int))[0])
        self.assertTrue(rat.appendField(QgsRasterAttributeTable.Field('Red', Qgis.RasterAttributeTableFieldUsage.Red, QVariant.Int))[0])

        self.assertEqual([f.name for f in rat.fields()], ['Class', 'Value', 'Green', 'Blue', 'Red'])
        for row in rat.data():
            self.assertEqual(len(row), 5)
        self.assertTrue(rat.isValid()[0])

        self.assertTrue(rat.removeField('Red')[0])
        self.assertTrue(rat.insertField(-1, QgsRasterAttributeTable.Field('Red', Qgis.RasterAttributeTableFieldUsage.Red, QVariant.Int))[0])
        self.assertEqual([f.name for f in rat.fields()], ['Red', 'Class', 'Value', 'Green', 'Blue'])
        self.assertTrue(rat.removeField('Red')[0])
        # 100 is not valid but field is appended
        self.assertTrue(rat.insertField(100, QgsRasterAttributeTable.Field('Red', Qgis.RasterAttributeTableFieldUsage.Red, QVariant.Int))[0])
        self.assertEqual([f.name for f in rat.fields()], ['Class', 'Value', 'Green', 'Blue', 'Red'])

        # Test row operations
        self.assertFalse(rat.appendRow([])[0])

        self.assertFalse(rat.appendRow(['class', '3', '10', 20, 100.1, 'xxx'])[0])
        self.assertTrue(rat.appendRow(['class', '3', '10', 20, 100.1])[0])
        self.assertEqual(rat.data()[3], ['class', 3, 10, 20, 100])

        # Get/set color
        color = rat.color(3)
        self.assertEqual(color.name(), '#640a14')
        self.assertTrue(rat.setColor(3, QColor('#010203')))
        color = rat.color(3)
        self.assertEqual(color.name(), '#010203')

        # Set value
        self.assertFalse(rat.setValue(-1, 0, 'class_new'))
        self.assertFalse(rat.setValue(100, 0, 'class_new'))
        self.assertFalse(rat.setValue(0, 100, 'class_new'))
        self.assertTrue(rat.setValue(3, 0, 'class_new'))
        self.assertTrue(rat.value(3, 0), 'class_new')

        # Remove row
        self.assertEqual(len(rat.data()), 4)
        self.assertFalse(rat.removeRow(-1)[0])
        self.assertTrue(rat.removeRow(0)[0])
        self.assertEqual(len(rat.data()), 3)
        self.assertEqual(rat.data()[0][0], 'two')

        # Insert row
        self.assertTrue(rat.insertRow(-1, ['zero', 0, 0, 0, 0])[0])
        self.assertEqual(rat.data()[0][0], 'zero')
        self.assertEqual(len(rat.data()), 4)
        self.assertTrue(rat.insertRow(1000, ['five', 1, 2, 3, 4])[0])
        self.assertEqual(rat.data()[4], ['five', 1, 2, 3, 4])
        self.assertEqual(len(rat.data()), 5)
        self.assertTrue(rat.insertRow(1, ['after 0', 1, 2, 3, 4])[0])
        self.assertEqual(rat.data()[1], ['after 0', 1, 2, 3, 4])
        self.assertEqual(len(rat.data()), 6)

        # Remove color and add ramp
        self.assertTrue(rat.removeField('Red')[0])
        self.assertTrue(rat.removeField('Green')[0])
        self.assertTrue(rat.removeField('Blue')[0])
        self.assertTrue(rat.removeField('Value')[0])
        rat.appendField(QgsRasterAttributeTable.Field('RedMin', Qgis.RasterAttributeTableFieldUsage.RedMin, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('GreenMin', Qgis.RasterAttributeTableFieldUsage.GreenMin, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('BlueMin', Qgis.RasterAttributeTableFieldUsage.BlueMin, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('RedMax', Qgis.RasterAttributeTableFieldUsage.RedMax, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('GreenMax', Qgis.RasterAttributeTableFieldUsage.GreenMax, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('BlueMax', Qgis.RasterAttributeTableFieldUsage.BlueMax, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('ValueMin', Qgis.RasterAttributeTableFieldUsage.Min, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('ValueMax', Qgis.RasterAttributeTableFieldUsage.Max, QVariant.Int))
        valid, error = rat.isValid()
        self.assertTrue(valid)
        self.assertFalse(rat.hasColor())
        self.assertTrue(rat.hasRamp())

        self.assertTrue(rat.setRamp(1, QColor('#010203'), QColor('#020304')))
        self.assertEqual(rat.ramp(1).min.name(), '#010203')
        self.assertEqual(rat.ramp(1).max.name(), '#020304')

    def testWriteReadDbfRat(self):

        # Create RAT
        rat = QgsRasterAttributeTable()
        rat.appendField(QgsRasterAttributeTable.Field('Value', Qgis.RasterAttributeTableFieldUsage.MinMax, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Count', Qgis.RasterAttributeTableFieldUsage.PixelCount, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Class', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class2', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class3', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Red', Qgis.RasterAttributeTableFieldUsage.Red, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Green', Qgis.RasterAttributeTableFieldUsage.Green, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Blue', Qgis.RasterAttributeTableFieldUsage.Blue, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Double', Qgis.RasterAttributeTableFieldUsage.Generic, QVariant.Double))

        data_rows = [
            [0, 1, 'zero', 'zero2', 'zero3', 0, 10, 100, 1.234],
            [2, 1, 'one', 'one2', 'one3', 100, 20, 0, 0.998],
            [4, 2, 'two', 'two2', 'two3', 200, 30, 50, 123456],
        ]

        usages = [f.usage for f in rat.fields()]
        self.assertEqual(usages, rat.usages())

        for row in data_rows:
            rat.appendRow(row)

        rat_path = os.path.join(self.temp_path, 'test_rat1.vat.dbf')
        self.assertTrue(rat.isDirty())
        self.assertTrue(rat.writeToFile(rat_path)[0])
        self.assertTrue(os.path.exists(rat_path))
        self.assertFalse(rat.isDirty())

        # Read it back
        self.assertTrue(rat.readFromFile(rat_path)[0])
        self.assertTrue(rat.isValid()[0])
        self.assertTrue(rat.hasColor())
        self.assertEqual([f.name for f in rat.fields()], ['Value', 'Count', 'Class', 'Class2', 'Class3', 'Red', 'Green', 'Blue', 'Double'])
        self.assertEqual([f.usage for f in rat.fields()], usages)
        self.assertEqual([f.type for f in rat.fields()], [QVariant.LongLong, QVariant.LongLong, QVariant.String, QVariant.String, QVariant.String, QVariant.Int, QVariant.Int, QVariant.Int, QVariant.Double])
        self.assertEqual(rat.data(), [
            [0, 1, 'zero', 'zero2', 'zero3', 0, 10, 100, 1.234],
            [2, 1, 'one', 'one2', 'one3', 100, 20, 0, 0.998],
            [4, 2, 'two', 'two2', 'two3', 200, 30, 50, 123456]])

    def testClassification(self):

        # Create RAT for 16 bit raster
        rat = QgsRasterAttributeTable()
        rat.appendField(QgsRasterAttributeTable.Field('Value', Qgis.RasterAttributeTableFieldUsage.MinMax, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Class', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class2', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class3', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Red', Qgis.RasterAttributeTableFieldUsage.Red, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Green', Qgis.RasterAttributeTableFieldUsage.Green, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Blue', Qgis.RasterAttributeTableFieldUsage.Blue, QVariant.Int))

        data_rows = [
            [0, 'zero', 'zero', 'even', 0, 0, 0],
            [1, 'one', 'not0', 'odd', 1, 1, 1],
            [2, 'two', 'not0', 'even', 2, 2, 2],
        ]

        for row in data_rows:
            rat.appendRow(row)

        raster = QgsRasterLayer(self.uri_2x2_1_BAND_INT16)
        self.assertTrue(raster.isValid())
        raster.dataProvider().setAttributeTable(1, rat)
        d = raster.dataProvider()
        ok, errors = rat.isValid()
        self.assertTrue(ok)
        ok, errors = d.writeNativeAttributeTable()  # spellok
        self.assertTrue(ok)

        raster = QgsRasterLayer(self.uri_2x2_1_BAND_INT16)
        self.assertIsNotNone(raster.attributeTable(1))

        # Test classes
        rat = raster.attributeTable(1)
        classes = rat.minMaxClasses()
        self.assertEqual({c.name: c.minMaxValues for c in classes}, {'zero': [0.0], 'one': [1.0], 'two': [2.0]})
        self.assertEqual(classes[0].color.name(), '#000000')
        self.assertEqual(classes[1].color.name(), '#010101')
        self.assertEqual(classes[2].color.name(), '#020202')

        classes = rat.minMaxClasses(1)
        self.assertEqual({c.name: c.minMaxValues for c in classes}, {'zero': [0.0], 'one': [1.0], 'two': [2.0]})
        classes = rat.minMaxClasses(2)
        self.assertEqual({c.name: c.minMaxValues for c in classes}, {'zero': [0.0], 'not0': [1.0, 2.0]})
        self.assertEqual(classes[0].color.name(), '#000000')
        self.assertEqual(classes[1].color.name(), '#010101')

        classes = rat.minMaxClasses(3)
        self.assertEqual({c.name: c.minMaxValues for c in classes}, {'even': [0.0, 2.0], 'odd': [1.0]})
        self.assertEqual(classes[0].color.name(), '#000000')
        self.assertEqual(classes[1].color.name(), '#010101')

        # Class out of range errors
        classes = rat.minMaxClasses(100)
        self.assertEqual(len(classes), 0)
        classes = rat.minMaxClasses(-100)
        self.assertEqual(len(classes), 0)
        classes = rat.minMaxClasses(4)
        self.assertEqual(len(classes), 0)

        # Test row function
        self.assertEqual(rat.row(0), [0, 'zero', 'zero', 'even', 0, 0, 0])
        self.assertEqual(rat.row(2.0), [2, 'two', 'not0', 'even', 2, 2, 2])
        self.assertEqual(rat.row(100.0), [])

    def testPalettedRenderer(self):

        # Create RAT for 16 bit raster
        rat = QgsRasterAttributeTable()
        rat.appendField(QgsRasterAttributeTable.Field('Value', Qgis.RasterAttributeTableFieldUsage.MinMax, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Class', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class2', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Class3', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Red', Qgis.RasterAttributeTableFieldUsage.Red, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Green', Qgis.RasterAttributeTableFieldUsage.Green, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Blue', Qgis.RasterAttributeTableFieldUsage.Blue, QVariant.Int))

        data_rows = [
            [0, 'zero', 'zero', 'even', 0, 0, 0],
            [1, 'one', 'not0', 'odd', 1, 1, 1],
            [2, 'two', 'not0', 'even', 2, 2, 2],
        ]

        for row in data_rows:
            rat.appendRow(row)

        raster = QgsRasterLayer(self.uri_2x2_1_BAND_INT16)
        self.assertTrue(raster.isValid())
        raster.dataProvider().setAttributeTable(1, rat)
        d = raster.dataProvider()
        ok, errors = rat.isValid()
        self.assertTrue(ok)
        ok, errors = d.writeNativeAttributeTable()  # spellok
        self.assertTrue(ok)

        raster = QgsRasterLayer(self.uri_2x2_1_BAND_INT16)
        rat = raster.attributeTable(1)
        renderer = QgsPalettedRasterRenderer(raster.dataProvider(), 1, [])

        classes = []

        for c in rat.minMaxClasses():
            mc = QgsPalettedRasterRenderer.MultiValueClass(c.minMaxValues, c.color, c.name)
            classes.append(mc)

        renderer.setMultiValueClasses(classes)
        self.assertEqual(["%s:%s:%s" % (c.values, c.label, c.color.name()) for c in renderer.multiValueClasses()], ['[0.0]:zero:#000000', '[1.0]:one:#010101', '[2.0]:two:#020202'])
        self.assertEqual(["%s:%s:%s" % (c.values, c.label, c.color.name()) for c in QgsPalettedRasterRenderer.rasterAttributeTableToClassData(rat)], ['[0.0]:zero:#000000', '[1.0]:one:#010101', '[2.0]:two:#020202'])
        self.assertEqual(["%s:%s:%s" % (c.values, c.label, c.color.name()) for c in raster.renderer().multiValueClasses()], ['[0.0]:zero:#000000', '[1.0]:one:#010101', '[2.0]:two:#020202'])

    def testCreateFromPalettedRaster(self):

        raster = QgsRasterLayer(self.uri_2x2_1_BAND_INT16)
        classes = QgsPalettedRasterRenderer.classDataFromRaster(raster.dataProvider(), 1, None)

        for i in range(len(classes)):
            classes[i].color = QColor(f'#0{i}0{i}0{i}')

        renderer = QgsPalettedRasterRenderer(raster.dataProvider(), 1, classes)
        raster.setRenderer(renderer)

        rat = QgsRasterAttributeTable.createFromRaster(raster)
        self.assertEqual(rat.data(), [
            [0.0, '0', 0, 0, 0, 255],
            [2.0, '2', 1, 1, 1, 255],
            [4.0, '4', 2, 2, 2, 255]
        ])

        usages = rat.usages()
        self.assertEqual(usages, [Qgis.RasterAttributeTableFieldUsage.MinMax, Qgis.RasterAttributeTableFieldUsage.Name, Qgis.RasterAttributeTableFieldUsage.Red, Qgis.RasterAttributeTableFieldUsage.Green, Qgis.RasterAttributeTableFieldUsage.Blue, Qgis.RasterAttributeTableFieldUsage.Alpha])


if __name__ == '__main__':
    unittest.main()
