"""QGIS Unit tests for QgsRasterBlock.

From build dir, run:
ctest -R PyQgsRasterBlock -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Till Frankenbach"
__date__ = "24/07/2024"
__copyright__ = "Copyright 2024, The QGIS Project"

import numpy

from qgis.core import Qgis, QgsRasterLayer, QgsRasterBlock
import unittest
from qgis.testing import start_app, QgisTestCase
from qgis.PyQt.QtCore import QTemporaryDir

from test_qgsrasterattributetable import createTestRasters

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsRasterBlock(QgisTestCase):

    def setUp(self):
        self.temp_dir = QTemporaryDir()
        self.temp_path = self.temp_dir.path()

        createTestRasters(self, self.temp_path)

    def test_as_numpy(self):
        raster = QgsRasterLayer(self.uri_2x2_2_BANDS_INT16)
        self.assertTrue(raster.isValid())
        provider = raster.dataProvider()

        # test without noDataValue set
        block = provider.block(1, raster.extent(), raster.width(), raster.height())
        expected_array = numpy.array([[0, 2], [4, 4]])
        self.assertTrue((block.as_numpy() == expected_array).all())

        # test with noDataValue set
        block.setNoDataValue(-999)
        data = numpy.array([[numpy.nan, 2], [4, 4]])
        expected_masked_array = numpy.ma.masked_array(data, mask=numpy.isnan(data))
        self.assertTrue((block.as_numpy() == expected_masked_array).all())
        self.assertTrue(numpy.ma.isMaskedArray(block.as_numpy()))
        self.assertTrue(block.as_numpy().fill_value == -999)

        # test with noDataValue set and use_masking == False
        self.assertTrue((block.as_numpy(use_masking=False) == expected_array).all())

    def test_types(self):
        for numeric_type in (
            Qgis.DataType.Byte,
            Qgis.DataType.Int8,
            Qgis.DataType.UInt16,
            Qgis.DataType.Int16,
            Qgis.DataType.UInt32,
            Qgis.DataType.Int32,
            Qgis.DataType.Float32,
            Qgis.DataType.Float64,
        ):
            self.assertTrue(QgsRasterBlock.typeIsNumeric(numeric_type))
            self.assertFalse(QgsRasterBlock.typeIsColor(numeric_type))
            self.assertFalse(QgsRasterBlock.typeIsComplex(numeric_type))

        for complex_type in (
            Qgis.DataType.CInt16,
            Qgis.DataType.CInt32,
            Qgis.DataType.CFloat32,
            Qgis.DataType.CFloat64,
        ):
            self.assertTrue(QgsRasterBlock.typeIsNumeric(complex_type))
            self.assertFalse(QgsRasterBlock.typeIsColor(complex_type))
            self.assertTrue(QgsRasterBlock.typeIsComplex(complex_type))

        for color_type in (Qgis.DataType.ARGB32, Qgis.DataType.ARGB32_Premultiplied):
            self.assertFalse(QgsRasterBlock.typeIsNumeric(color_type))
            self.assertTrue(QgsRasterBlock.typeIsColor(color_type))
            self.assertFalse(QgsRasterBlock.typeIsComplex(color_type))

        self.assertFalse(QgsRasterBlock.typeIsNumeric(Qgis.DataType.UnknownDataType))
        self.assertFalse(QgsRasterBlock.typeIsColor(Qgis.DataType.UnknownDataType))
        self.assertFalse(QgsRasterBlock.typeIsComplex(Qgis.DataType.UnknownDataType))

    def test_fill(self):
        """
        Test filling a block
        """
        # empty block
        block = QgsRasterBlock()
        with self.assertRaises(ValueError):
            block.fill(0)
        block = QgsRasterBlock(Qgis.DataType.Byte, 0, 10)
        with self.assertRaises(ValueError):
            block.fill(0)
        block = QgsRasterBlock(Qgis.DataType.Byte, 10, 0)
        with self.assertRaises(ValueError):
            block.fill(0)

        # byte block
        block = QgsRasterBlock(Qgis.DataType.Byte, 20, 10)
        block.fill(0)
        for row in range(10):
            for col in range(20):
                self.assertEqual(block.value(row, col), 0)
        block.fill(1)
        for row in range(10):
            for col in range(20):
                self.assertEqual(block.value(row, col), 1)

        # Int16
        block = QgsRasterBlock(Qgis.DataType.Int16, 20, 10)
        block.fill(0)
        for row in range(10):
            for col in range(20):
                self.assertEqual(block.value(row, col), 0)
        block.fill(12345)
        for row in range(10):
            for col in range(20):
                self.assertEqual(block.value(row, col), 12345)

        # Float32
        block = QgsRasterBlock(Qgis.DataType.Float32, 20, 10)
        block.fill(0)
        for row in range(10):
            for col in range(20):
                self.assertEqual(block.value(row, col), 0)
        block.fill(12345.5)
        for row in range(10):
            for col in range(20):
                self.assertEqual(block.value(row, col), 12345.5)
        block.fill(-12345.5)
        for row in range(10):
            for col in range(20):
                self.assertEqual(block.value(row, col), -12345.5)

        # color types
        block = QgsRasterBlock(Qgis.DataType.ARGB32, 20, 10)
        with self.assertRaises(ValueError):
            block.fill(0)

        block = QgsRasterBlock(Qgis.DataType.ARGB32_Premultiplied, 20, 10)
        with self.assertRaises(ValueError):
            block.fill(0)

        # complex type
        block = QgsRasterBlock(Qgis.DataType.CInt16, 20, 10)
        with self.assertRaises(ValueError):
            block.fill(0)


if __name__ == "__main__":
    unittest.main()
