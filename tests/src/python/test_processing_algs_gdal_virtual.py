"""QGIS Unit tests for the GDAL Processing provider with input virtual layers.

From build dir, run: ctest -R PyQgsAMSProvider -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Alexander Bruy"
__date__ = "2025-05-15"
__copyright__ = "Copyright 2025, Alexander Bruy"

import os
import tempfile

from processing.core.Processing import Processing
from qgis.PyQt.QtCore import QCoreApplication, QUrl
from qgis.core import (
    Qgis,
    QgsProject,
    QgsSettings,
    QgsFeature,
    QgsApplication,
    QgsVectorLayer,
    QgsProcessing,
    QgsProcessingContext,
    QgsProcessingFeedback,
    QgsProcessingUtils,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


def to_percent_encoding(s: str) -> bytes:
    return bytes(QUrl.toPercentEncoding(s)).decode()


class TestGdalProviderVirtualLayers(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestGdalProviderVirtualLayers.com")
        QCoreApplication.setApplicationName("TestGdalProviderVirtualLayers")
        QgsSettings().clear()
        Processing.initialize()
        cls.registry = QgsApplication.instance().processingRegistry()

    def test_buffer(self):
        """Test GDAL buffer vectors algorithm with virtual layer"""

        layer = QgsVectorLayer(
            os.path.join(TEST_DATA_DIR, "points.shp"), "points", "ogr"
        )
        self.assertTrue(layer.isValid())
        QgsProject.instance().addMapLayer(layer)
        query = to_percent_encoding("SELECT * from points WHERE Class='B52'")
        virtual_layer = QgsVectorLayer(f"?query={query}", "virtual_points", "virtual")
        self.assertTrue(virtual_layer.isValid())
        QgsProject.instance().addMapLayer(virtual_layer)

        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        feedback = QgsProcessingFeedback()

        alg = self.registry.createAlgorithmById("gdal:buffervectors")
        self.assertIsNotNone(alg)

        parameters = {
            "INPUT": virtual_layer,
            "DISTANCE": 0.1,
            "OUTPUT": QgsProcessing.TEMPORARY_OUTPUT,
        }

        results, ok = alg.run(parameters, context, feedback)
        self.assertTrue(ok)
        result_layer = QgsProcessingUtils.mapLayerFromString(results["OUTPUT"], context)
        self.assertTrue(result_layer.isValid())
        self.assertEqual(result_layer.featureCount(), 4)
        self.assertEqual(result_layer.wkbType(), Qgis.WkbType.Polygon)


if __name__ == "__main__":
    unittest.main()
