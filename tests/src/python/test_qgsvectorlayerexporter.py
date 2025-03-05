"""QGIS Unit tests for QgsVectorLayerExporter.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import tempfile
import unittest
from pathlib import Path

import osgeo.gdal  # NOQA
from osgeo import gdal, ogr
from qgis.core import (
    Qgis,
    QgsVectorLayerExporter,
    QgsFeature,
    QgsGeometry,
    QgsPointXY,
    QgsVectorLayer,
)
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()
start_app()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return (maj) * 1000000 + (min) * 10000 + (rev) * 100


class TestQgsVectorLayerExporter(QgisTestCase):

    @unittest.skipIf(
        int(gdal.VersionInfo("VERSION_NUM")) < GDAL_COMPUTE_VERSION(3, 8, 0),
        "GDAL 3.8 required",
    )
    def test_write_laundered_table_names(self):
        """
        Test exporting layer name with spaces to File Geodatabase, the name
        should be laundered but things should still work
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            dest_file_name = Path(temp_dir) / "test_export.gdb"
            ds = ogr.GetDriverByName("OpenFileGDB").CreateDataSource(
                dest_file_name.as_posix()
            )
            self.assertIsNotNone(ds)
            del ds

            # create a layer to export
            layer = QgsVectorLayer(
                "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
            )
            self.assertTrue(layer.isValid())
            pr = layer.dataProvider()
            f = QgsFeature()
            f.setAttributes(["test", 123])
            f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
            f2 = QgsFeature()
            f2.setAttributes(["test2", 457])
            f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
            self.assertTrue(pr.addFeatures([f, f2]))

            res, error = QgsVectorLayerExporter.exportLayer(
                layer,
                dest_file_name.as_posix(),
                "ogr",
                layer.crs(),
                options={"layerName": "Must be Laundered", "update": True},
            )
            self.assertEqual(res, Qgis.VectorExportResult.Success)

            # true to read result
            layer = QgsVectorLayer(
                dest_file_name.as_posix() + "|layername=Must_be_Laundered",
                "test",
                "ogr",
            )
            self.assertTrue(layer.isValid())
            self.assertEqual(layer.featureCount(), 2)


if __name__ == "__main__":
    unittest.main()
