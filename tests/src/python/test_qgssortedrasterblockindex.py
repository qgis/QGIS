"""QGIS Unit tests for QgsSortedRasterBlockIndex.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import Qgis, QgsRasterBlock, QgsSortedRasterBlockIndex
from qgis.PyQt.QtCore import Qt
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsSortedRasterBlockIndex(QgisTestCase):
    def create_raster_block(self, rows, cols, data, nodata_val=None):
        """
        Helper method to construct a Float32 QgsRasterBlock populated with test values.
        """
        block = QgsRasterBlock(Qgis.DataType.Float32, cols, rows)
        if nodata_val is not None:
            block.setNoDataValue(nodata_val)

        for r in range(rows):
            for c in range(cols):
                idx = r * cols + c
                val = data[idx]
                if nodata_val is not None and val == nodata_val:
                    block.setIsNoData(r, c)
                else:
                    block.setValue(r, c, val)
        return block

    def test_basic_sorting(self):
        """
        Test ascending and descending topological index sorting on a 3x3 raster grid.
        """
        data = [9.0, 2.0, 5.0, 1.0, 8.0, 3.0, 7.0, 4.0, 6.0]
        block = self.create_raster_block(3, 3, data)
        index = QgsSortedRasterBlockIndex(block)

        self.assertEqual(index.sortedCount(), 9)
        self.assertEqual(index.block(), block)

        # ascending order
        expected_asc_indices = [3, 1, 5, 7, 2, 8, 6, 4, 0]
        for rank in range(9):
            self.assertEqual(
                index.sortedIndex(rank, Qt.SortOrder.AscendingOrder),
                expected_asc_indices[rank],
            )
            self.assertAlmostEqual(
                index.sortedValue(rank, Qt.SortOrder.AscendingOrder),
                data[expected_asc_indices[rank]],
            )

        col, row = index.sortedColumnRow(0, Qt.SortOrder.AscendingOrder)
        self.assertEqual((col, row), (0, 1))
        col, row = index.sortedColumnRow(8, Qt.SortOrder.AscendingOrder)
        self.assertEqual((col, row), (0, 0))

        # descending orer
        expected_desc_indices = list(reversed(expected_asc_indices))

        for rank in range(9):
            self.assertEqual(
                index.sortedIndex(rank, Qt.SortOrder.DescendingOrder),
                expected_desc_indices[rank],
            )
            self.assertAlmostEqual(
                index.sortedValue(rank, Qt.SortOrder.DescendingOrder),
                data[expected_desc_indices[rank]],
            )

        col, row = index.sortedColumnRow(0, Qt.SortOrder.DescendingOrder)
        self.assertEqual((col, row), (0, 0))

    def test_nodata_handling(self):
        """
        Test that NoData cells are filtered out and excluded from sorted ranks.
        """
        nodata = -9999.0
        data = [nodata, 15.0, 3.0, 42.0, nodata, 8.0]
        block = self.create_raster_block(2, 3, data, nodata_val=nodata)
        index = QgsSortedRasterBlockIndex(block)

        self.assertEqual(index.sortedCount(), 4)

        self.assertAlmostEqual(index.sortedValue(0), 3.0)
        self.assertEqual(index.sortedIndex(0), 2)

        self.assertAlmostEqual(index.sortedValue(1), 8.0)
        self.assertEqual(index.sortedIndex(1), 5)

        self.assertAlmostEqual(index.sortedValue(2), 15.0)
        self.assertEqual(index.sortedIndex(2), 1)

        self.assertAlmostEqual(index.sortedValue(3), 42.0)
        self.assertEqual(index.sortedIndex(3), 3)

    def test_negative_and_floating_point_precision(self):
        """
        Test sorting with negative floating point numbers and zero.
        """
        data = [-10.5, 0.0, -0.001, 100.25, -1000.0]
        block = self.create_raster_block(1, 5, data)
        index = QgsSortedRasterBlockIndex(block)

        self.assertEqual(index.sortedCount(), 5)
        self.assertAlmostEqual(index.sortedValue(0), -1000.0)
        self.assertAlmostEqual(index.sortedValue(1), -10.5)
        self.assertAlmostEqual(index.sortedValue(2), -0.001)
        self.assertAlmostEqual(index.sortedValue(3), 0.0)
        self.assertAlmostEqual(index.sortedValue(4), 100.25)

    def test_all_nodata_block(self):
        """
        Test behavior when an entire block consists of NoData cells.
        """
        nodata = -9999.0
        data = [nodata] * 6
        block = self.create_raster_block(2, 3, data, nodata_val=nodata)
        index = QgsSortedRasterBlockIndex(block)

        self.assertEqual(index.sortedCount(), 0)

    def test_null_or_empty_block(self):
        """
        Test behavior when initialized with None or an empty raster block.
        """
        index_null = QgsSortedRasterBlockIndex(None)
        self.assertEqual(index_null.sortedCount(), 0)
        self.assertIsNone(index_null.block())

        empty_block = QgsRasterBlock()
        index_empty = QgsSortedRasterBlockIndex(empty_block)
        self.assertEqual(index_empty.sortedCount(), 0)


if __name__ == "__main__":
    unittest.main()
