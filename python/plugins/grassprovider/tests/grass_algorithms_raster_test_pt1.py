"""
***************************************************************************
    grass_algorithms_raster_test_pt1.py
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

from qgis.core import QgsApplication
from qgis.testing import QgisTestCase, start_app
from grassprovider.grass_provider import GrassProvider
from grassprovider.grass_utils import GrassUtils


class TestGrassAlgorithmsRasterTest(QgisTestCase, AlgorithmsTestBase.AlgorithmsTest):

    @classmethod
    def setUpClass(cls):
        start_app()
        cls.provider = GrassProvider()
        QgsApplication.processingRegistry().addProvider(cls.provider)
        cls.cleanup_paths = []

        assert GrassUtils.installedVersion()

    @classmethod
    def tearDownClass(cls):
        QgsApplication.processingRegistry().removeProvider(cls.provider)
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def definition_file(self):
        return "grass_algorithms_raster_tests1.yaml"


if __name__ == "__main__":
    nose2.main()
