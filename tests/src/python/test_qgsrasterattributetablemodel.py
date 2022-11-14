# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRasterAttributeTableModel.

From build dir, run:
ctest -R PyQgsRasterAttributeTableModel -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from builtins import str

__author__ = 'Alessandro Pasotti'
__date__ = '04/09/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA

import os
from distutils.version import StrictVersion
from qgis.PyQt.QtCore import QVariant, QModelIndex
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.Qt import Qt, PYQT_VERSION_STR

from qgis.core import (Qgis,
                       QgsRasterAttributeTable,
                       )
from qgis.gui import (QgsRasterAttributeTableModel,)

from qgis.testing import start_app, unittest
from qgis.testing.mocked import get_iface

from qgis.PyQt.QtTest import QAbstractItemModelTester

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsRasterAttributeTableModel(unittest.TestCase):

    def setUp(self):

        self.iface = get_iface()

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

        rat.setDirty(False)

        self.rat = rat

        self.model = QgsRasterAttributeTableModel(self.rat)
        self.tester = QAbstractItemModelTester(self.model, QAbstractItemModelTester.FailureReportingMode.Fatal)

    def testRatModel(self):

        # Test information methods
        self.assertTrue(self.model.hasColor())
        self.assertFalse(self.model.hasRamp())
        self.assertEqual(self.model.headerNames(), ['Value', 'Count', 'Class', 'Class2', 'Class3', 'Red', 'Green', 'Blue', 'Double', 'Color'])
        self.assertFalse(self.model.editable())
        self.model.setEditable(True)
        self.assertTrue(self.model.editable())
        self.model.setEditable(False)
        self.assertFalse(self.model.editable())

        # Test fields operations
        self.assertFalse(self.model.isDirty())
        self.assertFalse(self.model.removeColorOrRamp()[0])
        self.assertFalse(self.model.removeField(1)[0])
        self.model.setEditable(True)
        self.assertTrue(self.model.removeColorOrRamp()[0])
        self.assertFalse(self.model.hasColor())
        self.assertTrue(self.model.isValid()[0])
        self.assertTrue(self.model.isDirty())
        self.assertEqual(self.model.headerNames(), ['Value', 'Count', 'Class', 'Class2', 'Class3', 'Double'])
        self.assertFalse(self.model.removeField(-1)[0])
        self.assertFalse(self.model.removeField(100)[0])
        self.assertTrue(self.model.removeField(1)[0])
        self.assertEqual(self.model.headerNames(), ['Value', 'Class', 'Class2', 'Class3', 'Double'])

        # Test invalid field operations
        # MinMax is unique
        self.assertFalse(self.model.insertField(0, 'MyField', Qgis.RasterAttributeTableFieldUsage.MinMax, QVariant.String)[0])
        self.assertTrue(self.model.insertField(5, 'MyField', Qgis.RasterAttributeTableFieldUsage.Generic, QVariant.Double)[0])
        self.assertEqual(self.model.headerNames(), ['Value', 'Class', 'Class2', 'Class3', 'Double', 'MyField'])
        self.assertTrue(self.model.isValid()[0])

        # Remove MinMax
        self.assertTrue(self.model.removeField(0))
        self.assertFalse(self.model.isValid()[0])
        # Add it back
        self.assertTrue(self.model.insertField(0, 'Value', Qgis.RasterAttributeTableFieldUsage.MinMax, QVariant.Int)[0])
        self.assertTrue(self.model.isValid()[0])

        # Set Values for now zero value column
        for i in range(self.model.rowCount(QModelIndex())):
            self.assertTrue(self.model.setData(self.model.index(i, 0), i, Qt.EditRole))

        # Insert rows
        self.assertFalse(self.model.insertRow(-1, [4, 'four', 'four2', 'four3', 123456, 1.2345])[0])
        self.assertFalse(self.model.insertRow(1000, [4, 'four', 'four2', 'four3', 123456, 1.2345])[0])
        self.assertFalse(self.model.insertRow(2, [4, 'four', 'four2', 'four3', 'xxx', 1.2345])[0])
        self.assertFalse(self.model.insertRow(2, [4, 'four', 'four2', 'four3', 999, 1.2345, 'xxx'])[0])
        self.assertFalse(self.model.insertRow(2, [4, 'four', 'four2', 'four3', 999])[0])
        self.assertTrue(self.model.insertRow(2, [4, 'four', 'four2', 'four3', 4444, 1.4444])[0])
        self.assertEqual(self.model.rowCount(QModelIndex()), 4)
        self.assertEqual(self.model.data(self.model.index(2, 0), Qt.DisplayRole), 4)
        self.assertTrue(self.model.insertRow(4, [5, 'five', 'five', 'five', 5555, 1.5555])[0])
        self.assertEqual(self.model.rowCount(QModelIndex()), 5)
        self.assertEqual(self.model.data(self.model.index(4, 0), Qt.DisplayRole), 5)

        # Remove rows
        self.assertFalse(self.model.removeRow(-1)[0])
        self.assertFalse(self.model.removeRow(100)[0])
        self.assertTrue(self.model.removeRow(4)[0])
        self.assertEqual(self.model.rowCount(QModelIndex()), 4)
        self.assertTrue(self.model.removeRow(2)[0])
        self.assertEqual(self.model.rowCount(QModelIndex()), 3)

        # Test data for color role
        # Add colors back
        self.model.insertField(self.model.columnCount(QModelIndex()), 'Red', Qgis.RasterAttributeTableFieldUsage.Red, QVariant.Int)
        self.model.insertField(self.model.columnCount(QModelIndex()), 'Green', Qgis.RasterAttributeTableFieldUsage.Green, QVariant.Int)
        self.model.insertField(self.model.columnCount(QModelIndex()), 'Blue', Qgis.RasterAttributeTableFieldUsage.Blue, QVariant.Int)

        for i in range(self.model.rowCount(QModelIndex())):
            self.assertTrue(self.model.setData(self.model.index(i, 6), i, Qt.EditRole))
            self.assertTrue(self.model.setData(self.model.index(i, 7), i, Qt.EditRole))
            self.assertTrue(self.model.setData(self.model.index(i, 8), i, Qt.EditRole))

        # Check data from color
        self.assertEqual(self.model.data(self.model.index(1, 9), Qt.DisplayRole), '#010101')
        self.assertEqual(self.model.data(self.model.index(1, 9), Qt.BackgroundRole).name(), '#010101')

        for i in range(self.model.rowCount(QModelIndex())):
            self.assertTrue(self.model.setData(self.model.index(i, 9), QColor(f'#0{i}0{i}0{i}'), Qt.EditRole))

        self.assertEqual(self.model.data(self.model.index(0, 9), Qt.DisplayRole), '#000000')
        self.assertEqual(self.model.data(self.model.index(2, 9), Qt.DisplayRole), '#020202')


if __name__ == '__main__':
    unittest.main()
