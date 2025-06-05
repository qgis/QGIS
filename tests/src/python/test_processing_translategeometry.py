"""QGIS Unit tests for the Processing Translate geometry algorithm.

From build dir, run: ctest -R PyQgsAFSProvider -V

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
from processing.gui.AlgorithmExecutor import execute
from qgis.PyQt.QtCore import QCoreApplication, QTemporaryDir
from qgis.analysis import QgsNativeAlgorithms
from qgis.core import (
    QgsSettings,
    QgsApplication,
    QgsProcessingContext,
    QgsProcessingFeedback,
    QgsProperty,
    QgsVectorLayer,
    Qgis,
)
import unittest
from qgis.testing import QgisTestCase

from utilities import unitTestDataPath, start_app

TEST_DATA_DIR = unitTestDataPath()
start_app()


class TestTranslateGeometry(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsTranslateGeometry.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsTranslateGeometry")
        QgsSettings().clear()
        Processing.initialize()

    def tearDown(self):
        super().tearDown()

    def test_translate_dynamic_z(self):
        """Test translate with data-defined Z value"""

        alg = QgsApplication.processingRegistry().createAlgorithmById(
            "native:translategeometry"
        )
        self.assertIsNotNone(alg)

        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        parameters = {
            "INPUT": os.path.join(TEST_DATA_DIR, "points.shp"),
            "DELTA_Z": QgsProperty.fromExpression("@id * 0.1"),
            "OUTPUT": os.path.join(tempfile.gettempdir(), "translated_lines_z.shp"),
        }

        results, ok = alg.run(parameters, context, feedback)
        self.assertTrue(ok)

        layer = QgsVectorLayer(results["OUTPUT"], "output", "ogr")
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.wkbType(), Qgis.WkbType.PointZ)

    def test_translate_dynamic_z(self):
        """Test translate with data-defined M value"""

        alg = QgsApplication.processingRegistry().createAlgorithmById(
            "native:translategeometry"
        )
        self.assertIsNotNone(alg)

        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        parameters = {
            "INPUT": os.path.join(TEST_DATA_DIR, "points.shp"),
            "DELTA_M": QgsProperty.fromExpression("@id * 0.2"),
            "OUTPUT": os.path.join(tempfile.gettempdir(), "translated_lines_m.shp"),
        }

        results, ok = alg.run(parameters, context, feedback)
        self.assertTrue(ok)

        layer = QgsVectorLayer(results["OUTPUT"], "output", "ogr")
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.wkbType(), Qgis.WkbType.PointM)

    def test_translate_dynamic_zm(self):
        """Test translate with data-defined M value"""

        alg = QgsApplication.processingRegistry().createAlgorithmById(
            "native:translategeometry"
        )
        self.assertIsNotNone(alg)

        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        parameters = {
            "INPUT": os.path.join(TEST_DATA_DIR, "points.shp"),
            "DELTA_Z": QgsProperty.fromExpression("@id * 0.1"),
            "DELTA_M": QgsProperty.fromExpression("@id * 0.2"),
            "OUTPUT": os.path.join(tempfile.gettempdir(), "translated_lines_zm.shp"),
        }

        results, ok = alg.run(parameters, context, feedback)
        self.assertTrue(ok)

        layer = QgsVectorLayer(results["OUTPUT"], "output", "ogr")
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.wkbType(), Qgis.WkbType.PointZM)


if __name__ == "__main__":
    unittest.main()
