"""QGIS Unit tests for QgsMeshLayer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.core import QgsMeshLayer, QgsMeshDatasetIndex
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsMeshLayer(QgisTestCase):

    def test_dataset_group_metadata(self):
        """
        Test datasetGroupMetadata
        """
        layer = QgsMeshLayer(
            self.get_test_data_path("mesh/netcdf_parent_quantity.nc").as_posix(),
            "mesh",
            "mdal",
        )
        self.assertTrue(layer.isValid())

        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(0)).name(),
            "air_temperature_height:10",
        )
        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(0)).parentQuantityName(),
            "air_temperature_height",
        )
        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(1)).name(),
            "air_temperature_height:20",
        )
        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(1)).parentQuantityName(),
            "air_temperature_height",
        )
        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(2)).name(),
            "air_temperature_height:30",
        )
        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(2)).parentQuantityName(),
            "air_temperature_height",
        )
        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(3)).name(),
            "air_temperature_height:5",
        )
        self.assertEqual(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(3)).parentQuantityName(),
            "air_temperature_height",
        )
        self.assertFalse(layer.datasetGroupMetadata(QgsMeshDatasetIndex(4)).name())
        self.assertFalse(
            layer.datasetGroupMetadata(QgsMeshDatasetIndex(4)).parentQuantityName()
        )


if __name__ == "__main__":
    unittest.main()
