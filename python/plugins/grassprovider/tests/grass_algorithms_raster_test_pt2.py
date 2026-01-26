"""
***************************************************************************
    grass_algorithms_raster_test_pt2.py
    -----------------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Médéric Ribreux
    Email                : mederic dot ribreux at medspx dot fr
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Médéric Ribreux"
__date__ = "May 2016"
__copyright__ = "(C) 2016, Médéric Ribreux"

import AlgorithmsTestBase

import nose2
import shutil
import os
import tempfile

from qgis.core import QgsApplication, QgsProcessingContext, QgsProcessingFeedback
from qgis.testing import QgisTestCase, start_app
from grassprovider.grass_provider import GrassProvider
from grassprovider.grass_utils import GrassUtils


testDataPath = os.path.join(os.path.dirname(__file__), "testdata")


class TestGrassAlgorithmsRasterTest(QgisTestCase, AlgorithmsTestBase.AlgorithmsTest):

    @classmethod
    def setUpClass(cls):
        start_app()
        cls.provider = GrassProvider()
        QgsApplication.processingRegistry().addProvider(cls.provider)
        cls.cleanup_paths = []

        cls.temp_dir = tempfile.mkdtemp()
        cls.cleanup_paths.append(cls.temp_dir)

        assert GrassUtils.installedVersion()

    @classmethod
    def tearDownClass(cls):
        QgsApplication.processingRegistry().removeProvider(cls.provider)
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def definition_file(self):
        return "grass_algorithms_raster_tests2.yaml"

    def testNeighbors(self):
        context = QgsProcessingContext()
        input_raster = os.path.join(
            testDataPath, "custom", "grass7", "float_raster.tif"
        )

        alg = QgsApplication.processingRegistry().createAlgorithmById(
            "grass:r.neighbors"
        )
        self.assertIsNotNone(alg)

        temp_file = os.path.join(self.temp_dir, "grass_output.tif")

        # Test an even integer for neighborhood size
        parameters = {
            "input": input_raster,
            "selection": None,
            "method": 0,
            "size": 4,
            "gauss": None,
            "quantile": "",
            "-c": False,
            "-a": False,
            "weight": "",
            "output": temp_file,
            "GRASS_REGION_PARAMETER": None,
            "GRASS_REGION_CELLSIZE_PARAMETER": 0,
            "GRASS_RASTER_FORMAT_OPT": "",
            "GRASS_RASTER_FORMAT_META": "",
        }

        ok, msg = alg.checkParameterValues(parameters, context)
        self.assertFalse(ok)


if __name__ == "__main__":
    nose2.main()
