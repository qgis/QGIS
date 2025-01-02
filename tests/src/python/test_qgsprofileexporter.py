"""QGIS Unit tests for QgsRasterLayer profile generation

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "18/03/2022"
__copyright__ = "Copyright 2022, The QGIS Project"

import os
import tempfile

from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsLineString,
    QgsProfileIdentifyContext,
    QgsProfilePoint,
    QgsProfileRequest,
    QgsProfileSnapContext,
    QgsRasterLayer,
    QgsProfileExporter,
    QgsProfileExporterTask,
    QgsProject,
    QgsApplication,
    QgsVectorLayer,
    QgsMemoryProviderUtils,
    QgsFields,
    QgsFeature,
    QgsGeometry,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsProfileExporter(QgisTestCase):

    def testExport(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), "3d", "dtm.tif"), "DTM")
        self.assertTrue(rl.isValid())
        rl.elevationProperties().setEnabled(True)

        curve = QgsLineString()
        curve.fromWkt(
            "LineString (-348095.18706532847136259 6633687.0235139261931181, -347271.57799367723055184 6633093.13086318597197533, -346140.60267287614988163 6632697.89590711053460836, -345777.013075890194159 6631575.50219972990453243)"
        )
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))

        exporter = QgsProfileExporter([rl], req, Qgis.ProfileExportType.Features3D)

        exporter.run()

        layers = exporter.toLayers()
        self.assertEqual(len(layers), 1)

        output_layer = layers[0]
        self.assertEqual(output_layer.wkbType(), Qgis.WkbType.LineStringZ)

        features = [f for f in output_layer.getFeatures()]

        self.assertEqual(len(features), 1)
        self.assertEqual(features[0][0], rl.id())
        self.assertEqual(features[0].geometry().constGet().numPoints(), 1394)
        self.assertEqual(
            features[0].geometry().constGet().pointN(0).asWkt(-2),
            "Point Z (-348100 6633700 200)",
        )
        self.assertEqual(
            features[0].geometry().constGet().pointN(1393).asWkt(-2),
            "Point Z (-345800 6631600 100)",
        )

    def testExportTaskDxf(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), "3d", "dtm.tif"), "DTM")
        self.assertTrue(rl.isValid())
        rl.elevationProperties().setEnabled(True)

        curve = QgsLineString()
        curve.fromWkt(
            "LineString (-348095.18706532847136259 6633687.0235139261931181, -347271.57799367723055184 6633093.13086318597197533, -346140.60267287614988163 6632697.89590711053460836, -345777.013075890194159 6631575.50219972990453243)"
        )
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))

        with tempfile.TemporaryDirectory() as temp_dir:
            exporter = QgsProfileExporterTask(
                [rl],
                req,
                Qgis.ProfileExportType.Features3D,
                os.path.join(temp_dir, "test.dxf"),
                QgsProject.instance().transformContext(),
            )
            QgsApplication.taskManager().addTask(exporter)
            exporter.waitForFinished()

            self.assertEqual(
                exporter.result(), QgsProfileExporterTask.ExportResult.Success
            )
            self.assertEqual(
                exporter.createdFiles(), [os.path.join(temp_dir, "test.dxf")]
            )

            self.assertTrue(os.path.exists(os.path.join(temp_dir, "test.dxf")))

            layers = exporter.takeLayers()
            self.assertEqual(len(layers), 1)

            output_layer = QgsVectorLayer(exporter.createdFiles()[0], "test")
            self.assertTrue(output_layer.isValid())
            self.assertEqual(output_layer.wkbType(), Qgis.WkbType.LineStringZ)

            features = [f for f in output_layer.getFeatures()]

            self.assertEqual(len(features), 1)
            self.assertEqual(features[0].geometry().constGet().numPoints(), 1394)
            self.assertEqual(
                features[0].geometry().constGet().pointN(0).asWkt(-2),
                "Point Z (-348100 6633700 200)",
            )
            self.assertEqual(
                features[0].geometry().constGet().pointN(1393).asWkt(-2),
                "Point Z (-345800 6631600 100)",
            )

    def testExportTaskDxfMultiLayer(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), "3d", "dtm.tif"), "DTM")
        self.assertTrue(rl.isValid())
        rl.elevationProperties().setEnabled(True)

        vl = QgsMemoryProviderUtils.createMemoryLayer(
            "test",
            QgsFields(),
            Qgis.WkbType.LineStringZ,
            QgsCoordinateReferenceSystem("EPSG:3857"),
        )
        self.assertTrue(vl.isValid())
        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)

        feature = QgsFeature(vl.fields())
        feature.setGeometry(
            QgsGeometry.fromWkt(
                "LineStringZ (-347860.62472087447531521 6632536.37540269736200571 30, -347016.72474283445626497 6633588.82537531014531851 40)"
            )
        )
        self.assertTrue(vl.dataProvider().addFeature(feature))

        curve = QgsLineString()
        curve.fromWkt(
            "LineString (-348095.18706532847136259 6633687.0235139261931181, -347271.57799367723055184 6633093.13086318597197533, -346140.60267287614988163 6632697.89590711053460836, -345777.013075890194159 6631575.50219972990453243)"
        )
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))

        with tempfile.TemporaryDirectory() as temp_dir:
            exporter = QgsProfileExporterTask(
                [rl, vl],
                req,
                Qgis.ProfileExportType.Features3D,
                os.path.join(temp_dir, "test.dxf"),
                QgsProject.instance().transformContext(),
            )
            QgsApplication.taskManager().addTask(exporter)
            exporter.waitForFinished()

            self.assertEqual(
                exporter.result(), QgsProfileExporterTask.ExportResult.Success
            )
            self.assertEqual(
                exporter.createdFiles(), [os.path.join(temp_dir, "test.dxf")]
            )

            self.assertTrue(os.path.exists(os.path.join(temp_dir, "test.dxf")))

            layers = exporter.takeLayers()
            self.assertEqual(len(layers), 2)

            point_output = QgsVectorLayer(
                exporter.createdFiles()[0] + "|geometrytype=Point25D", "test"
            )
            line_output = QgsVectorLayer(
                exporter.createdFiles()[0] + "|geometrytype=LineString25D", "test"
            )
            self.assertTrue(point_output.isValid())
            self.assertTrue(line_output.isValid())

            self.assertEqual(line_output.wkbType(), Qgis.WkbType.LineStringZ)
            self.assertEqual(point_output.wkbType(), Qgis.WkbType.PointZ)

            features = [f for f in line_output.getFeatures()]

            self.assertEqual(len(features), 1)
            self.assertEqual(features[0].geometry().constGet().numPoints(), 1394)
            self.assertEqual(
                features[0].geometry().constGet().pointN(0).asWkt(-2),
                "Point Z (-348100 6633700 200)",
            )
            self.assertEqual(
                features[0].geometry().constGet().pointN(1393).asWkt(-2),
                "Point Z (-345800 6631600 100)",
            )

            features = [f for f in point_output.getFeatures()]

            self.assertEqual(len(features), 1)
            self.assertEqual(
                features[0].geometry().asWkt(-1), "Point Z (-347360 6633160 40)"
            )

    def testExportTaskShp(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), "3d", "dtm.tif"), "DTM")
        self.assertTrue(rl.isValid())
        rl.elevationProperties().setEnabled(True)

        curve = QgsLineString()
        curve.fromWkt(
            "LineString (-348095.18706532847136259 6633687.0235139261931181, -347271.57799367723055184 6633093.13086318597197533, -346140.60267287614988163 6632697.89590711053460836, -345777.013075890194159 6631575.50219972990453243)"
        )
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))

        with tempfile.TemporaryDirectory() as temp_dir:
            exporter = QgsProfileExporterTask(
                [rl],
                req,
                Qgis.ProfileExportType.Features3D,
                os.path.join(temp_dir, "test.shp"),
                QgsProject.instance().transformContext(),
            )
            QgsApplication.taskManager().addTask(exporter)
            exporter.waitForFinished()

            self.assertEqual(
                exporter.result(), QgsProfileExporterTask.ExportResult.Success
            )
            self.assertEqual(
                exporter.createdFiles(), [os.path.join(temp_dir, "test.shp")]
            )

            self.assertTrue(os.path.exists(os.path.join(temp_dir, "test.shp")))

            layers = exporter.takeLayers()
            self.assertEqual(len(layers), 1)

            output_layer = QgsVectorLayer(exporter.createdFiles()[0], "test")
            self.assertTrue(output_layer.isValid())
            self.assertEqual(output_layer.wkbType(), Qgis.WkbType.MultiLineStringZ)

            features = [f for f in output_layer.getFeatures()]

            self.assertEqual(len(features), 1)
            self.assertEqual(features[0].geometry().constGet()[0].numPoints(), 1394)
            self.assertEqual(
                features[0].geometry().constGet()[0].pointN(0).asWkt(-2),
                "Point Z (-348100 6633700 200)",
            )
            self.assertEqual(
                features[0].geometry().constGet()[0].pointN(1393).asWkt(-2),
                "Point Z (-345800 6631600 100)",
            )

    def testExportTaskShpMultiLayer(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), "3d", "dtm.tif"), "DTM")
        self.assertTrue(rl.isValid())
        rl.elevationProperties().setEnabled(True)

        vl = QgsMemoryProviderUtils.createMemoryLayer(
            "test",
            QgsFields(),
            Qgis.WkbType.LineStringZ,
            QgsCoordinateReferenceSystem("EPSG:3857"),
        )
        self.assertTrue(vl.isValid())
        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)

        feature = QgsFeature(vl.fields())
        feature.setGeometry(
            QgsGeometry.fromWkt(
                "LineString Z (-347860.62472087447531521 6632536.37540269736200571 30, -347016.72474283445626497 6633588.82537531014531851 40)"
            )
        )
        self.assertTrue(vl.dataProvider().addFeature(feature))

        curve = QgsLineString()
        curve.fromWkt(
            "LineString (-348095.18706532847136259 6633687.0235139261931181, -347271.57799367723055184 6633093.13086318597197533, -346140.60267287614988163 6632697.89590711053460836, -345777.013075890194159 6631575.50219972990453243)"
        )
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))

        with tempfile.TemporaryDirectory() as temp_dir:
            exporter = QgsProfileExporterTask(
                [rl, vl],
                req,
                Qgis.ProfileExportType.Features3D,
                os.path.join(temp_dir, "test.shp"),
                QgsProject.instance().transformContext(),
            )
            QgsApplication.taskManager().addTask(exporter)
            exporter.waitForFinished()

            self.assertEqual(
                exporter.result(), QgsProfileExporterTask.ExportResult.Success
            )
            self.assertCountEqual(
                exporter.createdFiles(),
                [
                    os.path.join(temp_dir, "test_1.shp"),
                    os.path.join(temp_dir, "test_2.shp"),
                ],
            )

            self.assertTrue(os.path.exists(os.path.join(temp_dir, "test_1.shp")))
            self.assertTrue(os.path.exists(os.path.join(temp_dir, "test_2.shp")))

            layers = exporter.takeLayers()
            self.assertEqual(len(layers), 2)

            output_1 = QgsVectorLayer(exporter.createdFiles()[0], "test")
            output_2 = QgsVectorLayer(exporter.createdFiles()[1], "test")
            self.assertTrue(output_1.isValid())
            self.assertTrue(output_2.isValid())

            line_output = (
                output_1
                if output_1.geometryType() == Qgis.GeometryType.Line
                else output_2
            )
            point_output = (
                output_1
                if output_1.geometryType() == Qgis.GeometryType.Point
                else output_2
            )

            self.assertEqual(line_output.wkbType(), Qgis.WkbType.MultiLineStringZ)
            self.assertEqual(point_output.wkbType(), Qgis.WkbType.PointZ)

            features = [f for f in line_output.getFeatures()]

            self.assertEqual(len(features), 1)
            self.assertEqual(features[0].geometry().constGet()[0].numPoints(), 1394)
            self.assertEqual(
                features[0].geometry().constGet()[0].pointN(0).asWkt(-2),
                "Point Z (-348100 6633700 200)",
            )
            self.assertEqual(
                features[0].geometry().constGet()[0].pointN(1393).asWkt(-2),
                "Point Z (-345800 6631600 100)",
            )

            features = [f for f in point_output.getFeatures()]

            self.assertEqual(len(features), 1)
            self.assertEqual(
                features[0].geometry().asWkt(-1), "Point Z (-347360 6633160 40)"
            )

    def testExportTaskGpkg(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), "3d", "dtm.tif"), "DTM")
        self.assertTrue(rl.isValid())
        rl.elevationProperties().setEnabled(True)

        curve = QgsLineString()
        curve.fromWkt(
            "LineString (-348095.18706532847136259 6633687.0235139261931181, -347271.57799367723055184 6633093.13086318597197533, -346140.60267287614988163 6632697.89590711053460836, -345777.013075890194159 6631575.50219972990453243)"
        )
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))

        with tempfile.TemporaryDirectory() as temp_dir:
            exporter = QgsProfileExporterTask(
                [rl],
                req,
                Qgis.ProfileExportType.Features3D,
                os.path.join(temp_dir, "test.gpkg"),
                QgsProject.instance().transformContext(),
            )
            QgsApplication.taskManager().addTask(exporter)
            exporter.waitForFinished()

            self.assertEqual(
                exporter.result(), QgsProfileExporterTask.ExportResult.Success
            )
            self.assertEqual(
                exporter.createdFiles(), [os.path.join(temp_dir, "test.gpkg")]
            )

            self.assertTrue(os.path.exists(os.path.join(temp_dir, "test.gpkg")))

            layers = exporter.takeLayers()
            self.assertEqual(len(layers), 1)

            output_layer = QgsVectorLayer(exporter.createdFiles()[0], "test")
            self.assertTrue(output_layer.isValid())
            self.assertEqual(output_layer.wkbType(), Qgis.WkbType.LineStringZ)

            features = [f for f in output_layer.getFeatures()]

            self.assertEqual(len(features), 1)
            self.assertEqual(features[0].geometry().constGet().numPoints(), 1394)
            self.assertEqual(
                features[0].geometry().constGet().pointN(0).asWkt(-2),
                "Point Z (-348100 6633700 200)",
            )
            self.assertEqual(
                features[0].geometry().constGet().pointN(1393).asWkt(-2),
                "Point Z (-345800 6631600 100)",
            )

    def testExportTaskGpkgMultiLayer(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), "3d", "dtm.tif"), "DTM")
        self.assertTrue(rl.isValid())
        rl.elevationProperties().setEnabled(True)

        vl = QgsMemoryProviderUtils.createMemoryLayer(
            "test",
            QgsFields(),
            Qgis.WkbType.LineStringZ,
            QgsCoordinateReferenceSystem("EPSG:3857"),
        )
        self.assertTrue(vl.isValid())
        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)

        feature = QgsFeature(vl.fields())
        feature.setGeometry(
            QgsGeometry.fromWkt(
                "LineString Z (-347860.62472087447531521 6632536.37540269736200571 30, -347016.72474283445626497 6633588.82537531014531851 40)"
            )
        )
        self.assertTrue(vl.dataProvider().addFeature(feature))

        curve = QgsLineString()
        curve.fromWkt(
            "LineString (-348095.18706532847136259 6633687.0235139261931181, -347271.57799367723055184 6633093.13086318597197533, -346140.60267287614988163 6632697.89590711053460836, -345777.013075890194159 6631575.50219972990453243)"
        )
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))

        with tempfile.TemporaryDirectory() as temp_dir:
            exporter = QgsProfileExporterTask(
                [rl, vl],
                req,
                Qgis.ProfileExportType.Features3D,
                os.path.join(temp_dir, "test.gpkg"),
                QgsProject.instance().transformContext(),
            )
            QgsApplication.taskManager().addTask(exporter)
            exporter.waitForFinished()

            self.assertEqual(
                exporter.result(), QgsProfileExporterTask.ExportResult.Success
            )
            self.assertEqual(
                exporter.createdFiles(), [os.path.join(temp_dir, "test.gpkg")]
            )

            self.assertTrue(os.path.exists(os.path.join(temp_dir, "test.gpkg")))

            layers = exporter.takeLayers()
            self.assertEqual(len(layers), 2)

            output_1 = QgsVectorLayer(exporter.createdFiles()[0] + "|layerId=0", "test")
            output_2 = QgsVectorLayer(exporter.createdFiles()[0] + "|layerId=1", "test")
            self.assertTrue(output_1.isValid())
            self.assertTrue(output_2.isValid())

            line_output = (
                output_1
                if output_1.geometryType() == Qgis.GeometryType.Line
                else output_2
            )
            point_output = (
                output_1
                if output_1.geometryType() == Qgis.GeometryType.Point
                else output_2
            )

            self.assertEqual(line_output.wkbType(), Qgis.WkbType.LineStringZ)
            self.assertEqual(point_output.wkbType(), Qgis.WkbType.PointZ)

            features = [f for f in line_output.getFeatures()]

            self.assertEqual(len(features), 1)
            self.assertEqual(features[0].geometry().constGet().numPoints(), 1394)
            self.assertEqual(
                features[0].geometry().constGet().pointN(0).asWkt(-2),
                "Point Z (-348100 6633700 200)",
            )
            self.assertEqual(
                features[0].geometry().constGet().pointN(1393).asWkt(-2),
                "Point Z (-345800 6631600 100)",
            )

            features = [f for f in point_output.getFeatures()]

            self.assertEqual(len(features), 1)
            self.assertEqual(
                features[0].geometry().asWkt(-1), "Point Z (-347360 6633160 40)"
            )


if __name__ == "__main__":
    unittest.main()
