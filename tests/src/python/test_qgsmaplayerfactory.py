"""QGIS Unit tests for QgsMapLayerFactory.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "10/03/2021"
__copyright__ = "Copyright 2021, The QGIS Project"

import os

from qgis.core import (
    QgsAnnotationLayer,
    QgsCoordinateTransformContext,
    QgsDataSourceUri,
    QgsMapLayerFactory,
    QgsMapLayerType,
    QgsMeshLayer,
    QgsPointCloudLayer,
    QgsRasterLayer,
    QgsVectorLayer,
    QgsVectorTileLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsMapLayerFactory(QgisTestCase):

    def testTypeFromString(self):
        """
        Test QgsMapLayerFactory.typeFromString
        """
        self.assertEqual(QgsMapLayerFactory.typeFromString("xxx")[1], False)
        self.assertEqual(QgsMapLayerFactory.typeFromString("")[1], False)
        self.assertEqual(
            QgsMapLayerFactory.typeFromString("vector"),
            (QgsMapLayerType.VectorLayer, True),
        )
        self.assertEqual(
            QgsMapLayerFactory.typeFromString("VECTOR"),
            (QgsMapLayerType.VectorLayer, True),
        )
        self.assertEqual(
            QgsMapLayerFactory.typeFromString("raster"),
            (QgsMapLayerType.RasterLayer, True),
        )
        self.assertEqual(
            QgsMapLayerFactory.typeFromString("mesh"), (QgsMapLayerType.MeshLayer, True)
        )
        self.assertEqual(
            QgsMapLayerFactory.typeFromString("vector-tile"),
            (QgsMapLayerType.VectorTileLayer, True),
        )
        self.assertEqual(
            QgsMapLayerFactory.typeFromString("point-cloud"),
            (QgsMapLayerType.PointCloudLayer, True),
        )
        self.assertEqual(
            QgsMapLayerFactory.typeFromString("plugin"),
            (QgsMapLayerType.PluginLayer, True),
        )
        self.assertEqual(
            QgsMapLayerFactory.typeFromString("annotation"),
            (QgsMapLayerType.AnnotationLayer, True),
        )

    def testTypeToString(self):
        """
        Test QgsMapLayerFactory.typeToString
        """
        # test via round trips...
        self.assertEqual(
            QgsMapLayerFactory.typeFromString(
                QgsMapLayerFactory.typeToString(QgsMapLayerType.VectorLayer)
            )[0],
            QgsMapLayerType.VectorLayer,
        )
        self.assertEqual(
            QgsMapLayerFactory.typeFromString(
                QgsMapLayerFactory.typeToString(QgsMapLayerType.RasterLayer)
            )[0],
            QgsMapLayerType.RasterLayer,
        )
        self.assertEqual(
            QgsMapLayerFactory.typeFromString(
                QgsMapLayerFactory.typeToString(QgsMapLayerType.MeshLayer)
            )[0],
            QgsMapLayerType.MeshLayer,
        )
        self.assertEqual(
            QgsMapLayerFactory.typeFromString(
                QgsMapLayerFactory.typeToString(QgsMapLayerType.VectorTileLayer)
            )[0],
            QgsMapLayerType.VectorTileLayer,
        )
        self.assertEqual(
            QgsMapLayerFactory.typeFromString(
                QgsMapLayerFactory.typeToString(QgsMapLayerType.PointCloudLayer)
            )[0],
            QgsMapLayerType.PointCloudLayer,
        )
        self.assertEqual(
            QgsMapLayerFactory.typeFromString(
                QgsMapLayerFactory.typeToString(QgsMapLayerType.PluginLayer)
            )[0],
            QgsMapLayerType.PluginLayer,
        )
        self.assertEqual(
            QgsMapLayerFactory.typeFromString(
                QgsMapLayerFactory.typeToString(QgsMapLayerType.AnnotationLayer)
            )[0],
            QgsMapLayerType.AnnotationLayer,
        )

    def testCreateLayer(self):
        # create vector
        options = QgsMapLayerFactory.LayerOptions(QgsCoordinateTransformContext())
        ml = QgsMapLayerFactory.createLayer(
            os.path.join(unitTestDataPath(), "lines.shp"),
            "lines",
            QgsMapLayerType.VectorLayer,
            options,
            "ogr",
        )
        self.assertTrue(ml.isValid())
        self.assertIsInstance(ml, QgsVectorLayer)
        self.assertEqual(ml.name(), "lines")

        # create raster
        ml = QgsMapLayerFactory.createLayer(
            os.path.join(unitTestDataPath(), "landsat.tif"),
            "rl",
            QgsMapLayerType.RasterLayer,
            options,
            "gdal",
        )
        self.assertTrue(ml.isValid())
        self.assertIsInstance(ml, QgsRasterLayer)
        self.assertEqual(ml.name(), "rl")

        # create mesh
        ml = QgsMapLayerFactory.createLayer(
            os.path.join(unitTestDataPath(), "mesh", "lines.2dm"),
            "ml",
            QgsMapLayerType.MeshLayer,
            options,
            "mdal",
        )
        self.assertTrue(ml.isValid())
        self.assertIsInstance(ml, QgsMeshLayer)
        self.assertEqual(ml.name(), "ml")

        # create point cloud
        ml = QgsMapLayerFactory.createLayer(
            os.path.join(unitTestDataPath(), "point_clouds", "ept", "rgb", "ept.json"),
            "pcl",
            QgsMapLayerType.PointCloudLayer,
            options,
            "ept",
        )
        self.assertTrue(ml.isValid())
        self.assertIsInstance(ml, QgsPointCloudLayer)
        self.assertEqual(ml.name(), "pcl")

        # annotation layer
        ml = QgsMapLayerFactory.createLayer(
            "", "al", QgsMapLayerType.AnnotationLayer, options
        )
        self.assertTrue(ml.isValid())
        self.assertIsInstance(ml, QgsAnnotationLayer)
        self.assertEqual(ml.name(), "al")

        # vector tile layer
        ds = QgsDataSourceUri()
        ds.setParam("type", "xyz")
        ds.setParam(
            "url",
            f"file://{os.path.join(unitTestDataPath(), 'vector_tile')}/{{z}}-{{x}}-{{y}}.pbf",
        )
        ds.setParam("zmax", "1")
        ml = QgsMapLayerFactory.createLayer(
            ds.encodedUri().data().decode(),
            "vtl",
            QgsMapLayerType.VectorTileLayer,
            options,
        )
        self.assertTrue(ml.isValid())
        self.assertIsInstance(ml, QgsVectorTileLayer)
        self.assertEqual(ml.name(), "vtl")


if __name__ == "__main__":
    unittest.main()
