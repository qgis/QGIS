# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRasterAttributeTableWidget.

From build dir, run:
ctest -R PyQgsRasterAttributeTableWidget -V

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
from qgis.PyQt.QtWidgets import QDialog, QVBoxLayout


from qgis.core import (Qgis,
                       QgsRasterLayer,
                       QgsRasterAttributeTable,
                       QgsPalettedRasterRenderer,
                       QgsSingleBandPseudoColorRenderer,
                       QgsPresetSchemeColorRamp,
                       )

from qgis.gui import (QgsRasterAttributeTableWidget,
                      )

from test_qgsrasterattributetable import createTestRasters

from qgis.testing import start_app, unittest
from qgis.testing.mocked import get_iface

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsRasterAttributeTableWidget(unittest.TestCase):

    def setUp(self):

        self.iface = get_iface()
        self.temp_dir = QTemporaryDir()
        self.temp_path = self.temp_dir.path()

        createTestRasters(self, self.temp_path)

    def testWidget(self):

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

        ok, errors = rat.isValid()
        self.assertTrue(ok)

        raster = QgsRasterLayer(self.uri_2x2_2_BANDS_INT16)
        self.assertTrue(raster.isValid())
        raster.dataProvider().setAttributeTable(1, rat)
        d = raster.dataProvider()
        ok, errors = rat.isValid()
        self.assertTrue(ok)
        ok, errors = d.writeNativeAttributeTable()  # spellok
        self.assertTrue(ok)

        rat = QgsRasterAttributeTable()
        rat.appendField(QgsRasterAttributeTable.Field('ValueMin', Qgis.RasterAttributeTableFieldUsage.Min, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('ValueMax', Qgis.RasterAttributeTableFieldUsage.Max, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Count', Qgis.RasterAttributeTableFieldUsage.PixelCount, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('Class', Qgis.RasterAttributeTableFieldUsage.Name, QVariant.String))
        rat.appendField(QgsRasterAttributeTable.Field('Double', Qgis.RasterAttributeTableFieldUsage.Generic, QVariant.Double))
        rat.appendField(QgsRasterAttributeTable.Field('RedMin', Qgis.RasterAttributeTableFieldUsage.RedMin, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('GreenMin', Qgis.RasterAttributeTableFieldUsage.GreenMin, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('BlueMin', Qgis.RasterAttributeTableFieldUsage.BlueMin, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('RedMax', Qgis.RasterAttributeTableFieldUsage.RedMax, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('GreenMax', Qgis.RasterAttributeTableFieldUsage.GreenMax, QVariant.Int))
        rat.appendField(QgsRasterAttributeTable.Field('BlueMax', Qgis.RasterAttributeTableFieldUsage.BlueMax, QVariant.Int))

        data_rows = [
            [0, 1, 1, 'zero', 1.234, 0, 0, 0, 0, 100, 100],
            [1, 2, 1, 'one', 0.998, 0, 100, 100, 0, 150, 150],
            [2, 4, 2, 'two', 123456, 0, 150, 150, 255, 0, 255],
        ]

        for row in data_rows:
            rat.appendRow(row)

        raster.dataProvider().setAttributeTable(2, rat)
        d = raster.dataProvider()
        ok, errors = rat.isValid()
        self.assertTrue(ok)
        ok, errors = d.writeNativeAttributeTable()  # spellok

        widget = QgsRasterAttributeTableWidget(None, raster)
        dialog = QDialog()
        layout = QVBoxLayout(dialog)
        layout.addWidget(widget)
        dialog.setLayout(layout)

        # For interactive testing while developing:
        # dialog.exec_()


if __name__ == '__main__':
    unittest.main()
