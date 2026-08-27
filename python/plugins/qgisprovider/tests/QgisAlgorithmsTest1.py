"""
***************************************************************************
    QgisAlgorithmTests.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Matthias Kuhn
    Email                : matthias@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Matthias Kuhn"
__date__ = "January 2016"
__copyright__ = "(C) 2016, Matthias Kuhn"

import shutil

import AlgorithmsTestBase
import nose2
from qgis.core import QgsApplication
from qgis.testing import QgisTestCase, start_app

from qgisprovider.qgis_provider import QgisAlgorithmProvider


class TestQgisPythonAlgorithms(QgisTestCase, AlgorithmsTestBase.AlgorithmsTest):
    @classmethod
    def setUpClass(cls):
        start_app()
        cls.provider = QgisAlgorithmProvider()
        QgsApplication.processingRegistry().addProvider(cls.provider)

        cls.cleanup_paths = []
        cls.in_place_layers = {}
        cls.vector_layer_params = {}

    @classmethod
    def tearDownClass(cls):
        from processing.core.Processing import Processing

        Processing.deinitialize()
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def definition_file(self):
        return "qgis_algorithm_tests1.yaml"


if __name__ == "__main__":
    nose2.main()
