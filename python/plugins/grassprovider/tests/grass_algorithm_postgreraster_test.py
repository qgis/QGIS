"""QGIS Unit tests for Grass Algorithm with postgreraster provider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Jan Caha"
__date__ = "01/08/2025"
__copyright__ = "Copyright 2025, The QGIS Project"


import os
import tempfile
from shutil import rmtree

import unittest
from qgis.testing import start_app, QgisTestCase
from qgis.core import (
    QgsApplication,
    QgsRasterLayer,
    QgsDataSourceUri,
    QgsAuthMethodConfig,
    QgsProcessingContext,
    QgsProcessingFeedback,
)
from qgis import processing
from grassprovider.grass_provider import GrassProvider
from grassprovider.grass_utils import GrassUtils

QGIS_AUTH_DB_DIR_PATH = tempfile.mkdtemp()
os.environ["QGIS_AUTH_DB_DIR_PATH"] = QGIS_AUTH_DB_DIR_PATH

start_app()


class TestProcessingGrassAlgsPostgreRasterProvider(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.provider = GrassProvider()
        QgsApplication.processingRegistry().addProvider(cls.provider)
        cls.cleanup_paths = []

        assert GrassUtils.installedVersion()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        rmtree(QGIS_AUTH_DB_DIR_PATH)
        del os.environ["QGIS_AUTH_DB_DIR_PATH"]
        super().tearDownClass()

    def test_algorithm_r_buffer(self):
        """
        Test grass algorithm r.buffer with postgreraster provider
        """

        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()

        rl = QgsRasterLayer(
            "dbname='mydb' host=localhost port=5432 user='asdf' password='42'"
            " sslmode=disable table=some_table schema=some_schema column=rast sql=pk = 2",
            "pg_layer",
            "postgresraster",
        )

        alg = QgsApplication.processingRegistry().algorithmById("grass:r.buffer")

        self.assertTrue(alg)

        params = {
            "input": rl,
            "distances": "100,200,500",
            "units": 0,
            "-z": False,
            "output": "TEMPORARY_OUTPUT",
            "GRASS_REGION_PARAMETER": None,
            "GRASS_REGION_CELLSIZE_PARAMETER": 0,
            "GRASS_RASTER_FORMAT_OPT": "",
            "GRASS_RASTER_FORMAT_META": "",
        }

        can_run, _ = alg.checkParameterValues(parameters=params, context=context)

        self.assertTrue(can_run)

        res = alg.run(params, context, feedback)

        # should be tuple
        self.assertTrue(res)
        # should be true if algorithm run successfully
        self.assertTrue(res[1])
        # should be dict with output keys
        self.assertTrue(isinstance(res[0], dict))
        # should be path to result file
        self.assertTrue(isinstance(res[0]["output"], str))

    def test_algorithm_r_info(self):
        """
        Test grass algorithm r.info with postgreraster provider
        """

        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()

        rl = QgsRasterLayer(
            "dbname='mydb' host=localhost port=5432 user='asdf' password='42'"
            " sslmode=disable table=some_table schema=some_schema column=rast sql=pk = 2",
            "pg_layer",
            "postgresraster",
        )

        alg = QgsApplication.processingRegistry().algorithmById("grass:r.info")

        params = {
            "map": rl,
            "-r": False,
            "-g": False,
            "-h": False,
            "-e": False,
            "html": "./report.html",
            "GRASS_REGION_PARAMETER": None,
            "GRASS_REGION_CELLSIZE_PARAMETER": 0,
        }

        can_run, _ = alg.checkParameterValues(parameters=params, context=context)

        self.assertTrue(can_run)

        res = alg.run(params, context, feedback)

        # should be tuple
        self.assertTrue(res)
        # should be true if algorithm run successfully
        self.assertTrue(res[1])
        # should be dict with output keys
        self.assertTrue(isinstance(res[0], dict))
        # should be path to result file
        self.assertTrue(isinstance(res[0]["html"], str))


if __name__ == "__main__":
    unittest.main()
