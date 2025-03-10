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
    QgsCoordinateReferenceSystem,
    QgsReferencedRectangle,
    QgsRectangle,
)
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()
start_app()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return (maj) * 1000000 + (min) * 10000 + (rev) * 100


class TestQgsVectorLayerExporter(QgisTestCase):

    def test_selected_features_only(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            dest_file_name = Path(temp_dir) / "test_export.gpkg"

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
            f3 = QgsFeature()
            f3.setAttributes(["test3", 4573])
            f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
            f4 = QgsFeature()
            f4.setAttributes(["test4", 4574])
            f4.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
            self.assertTrue(pr.addFeature(f))
            self.assertTrue(pr.addFeature(f2))
            self.assertTrue(pr.addFeature(f3))
            self.assertTrue(pr.addFeature(f4))

            layer.selectByIds([f.id(), f3.id()])

            export_options = QgsVectorLayerExporter.ExportOptions()
            export_options.setSelectedOnly(True)
            export_options.setDestinationCrs(layer.crs())

            res, error = QgsVectorLayerExporter.exportLayer(
                layer,
                dest_file_name.as_posix(),
                "ogr",
                export_options,
                providerOptions={"layerName": "selected_only", "update": True},
            )
            self.assertEqual(res, Qgis.VectorExportResult.Success)

            # true to read result
            layer = QgsVectorLayer(
                dest_file_name.as_posix() + "|layername=selected_only",
                "test",
                "ogr",
            )
            self.assertTrue(layer.isValid())
            self.assertEqual(layer.featureCount(), 2)

            self.assertCountEqual(
                [f["fldtxt"] for f in layer.getFeatures()], ["test", "test3"]
            )

    def test_destination_crs(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            dest_file_name = Path(temp_dir) / "test_crs.gpkg"

            # create a layer to export
            layer = QgsVectorLayer(
                "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
            )
            self.assertTrue(layer.isValid())
            pr = layer.dataProvider()
            f = QgsFeature()
            f.setAttributes(["test", 123])
            f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
            f2 = QgsFeature()
            f2.setAttributes(["test2", 457])
            f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(3, 4)))
            self.assertTrue(pr.addFeature(f))
            self.assertTrue(pr.addFeature(f2))
            layer.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))

            export_options = QgsVectorLayerExporter.ExportOptions()
            export_options.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))

            res, error = QgsVectorLayerExporter.exportLayer(
                layer,
                dest_file_name.as_posix(),
                "ogr",
                export_options,
                providerOptions={"layerName": "selected_only", "update": True},
            )
            self.assertEqual(res, Qgis.VectorExportResult.Success)

            # true to read result
            layer = QgsVectorLayer(
                dest_file_name.as_posix() + "|layername=selected_only",
                "test",
                "ogr",
            )
            self.assertTrue(layer.isValid())
            self.assertEqual(layer.crs(), QgsCoordinateReferenceSystem("EPSG:3857"))
            self.assertEqual(layer.featureCount(), 2)
            self.assertCountEqual(
                [f.geometry().asWkt(-2) for f in layer.getFeatures()],
                ["Point (111300 222700)", "Point (334000 445600)"],
            )

    def test_extent_filter(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            dest_file_name = Path(temp_dir) / "test_extent.gpkg"

            # create a layer to export
            layer = QgsVectorLayer(
                "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
            )
            self.assertTrue(layer.isValid())
            pr = layer.dataProvider()
            f = QgsFeature()
            f.setAttributes(["test", 123])
            f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
            f2 = QgsFeature()
            f2.setAttributes(["test2", 457])
            f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(3, 4)))
            f3 = QgsFeature()
            f3.setAttributes(["test3", 4573])
            f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(5, 6)))
            f4 = QgsFeature()
            f4.setAttributes(["test4", 4574])
            f4.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(7, 8)))
            self.assertTrue(pr.addFeature(f))
            self.assertTrue(pr.addFeature(f2))
            self.assertTrue(pr.addFeature(f3))
            self.assertTrue(pr.addFeature(f4))
            layer.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))

            export_options = QgsVectorLayerExporter.ExportOptions()
            export_options.setExtent(
                QgsReferencedRectangle(
                    QgsRectangle(181463, 348966, 681661, 778824),
                    QgsCoordinateReferenceSystem("EPSG:3857"),
                )
            )

            res, error = QgsVectorLayerExporter.exportLayer(
                layer,
                dest_file_name.as_posix(),
                "ogr",
                export_options,
                providerOptions={"layerName": "extent", "update": True},
            )
            self.assertEqual(res, Qgis.VectorExportResult.Success)

            # true to read result
            layer = QgsVectorLayer(
                dest_file_name.as_posix() + "|layername=extent",
                "test",
                "ogr",
            )
            self.assertTrue(layer.isValid())
            self.assertEqual(layer.featureCount(), 2)

            self.assertCountEqual(
                [f["fldtxt"] for f in layer.getFeatures()], ["test2", "test3"]
            )

    def test_expression_filter(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            dest_file_name = Path(temp_dir) / "test_expression.gpkg"

            # create a layer to export
            layer = QgsVectorLayer(
                "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
            )
            self.assertTrue(layer.isValid())
            pr = layer.dataProvider()
            f = QgsFeature()
            f.setAttributes(["test", 123])
            f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
            f2 = QgsFeature()
            f2.setAttributes(["abc", 457])
            f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(3, 4)))
            f3 = QgsFeature()
            f3.setAttributes(["def", 4573])
            f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(5, 6)))
            f4 = QgsFeature()
            f4.setAttributes(["a feature", 4574])
            f4.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(7, 8)))
            self.assertTrue(pr.addFeature(f))
            self.assertTrue(pr.addFeature(f2))
            self.assertTrue(pr.addFeature(f3))
            self.assertTrue(pr.addFeature(f4))
            layer.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))

            export_options = QgsVectorLayerExporter.ExportOptions()
            export_options.setFilterExpression("left(\"fldtxt\", 1) = 'a'")

            res, error = QgsVectorLayerExporter.exportLayer(
                layer,
                dest_file_name.as_posix(),
                "ogr",
                export_options,
                providerOptions={"layerName": "expression", "update": True},
            )
            self.assertEqual(res, Qgis.VectorExportResult.Success)

            # true to read result
            layer = QgsVectorLayer(
                dest_file_name.as_posix() + "|layername=expression",
                "test",
                "ogr",
            )
            self.assertTrue(layer.isValid())
            self.assertEqual(layer.featureCount(), 2)

            self.assertCountEqual(
                [f["fldtxt"] for f in layer.getFeatures()], ["abc", "a feature"]
            )

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
