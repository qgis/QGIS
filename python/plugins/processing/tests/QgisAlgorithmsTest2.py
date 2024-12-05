"""
***************************************************************************
    QgisAlgorithmTests2.py
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

import AlgorithmsTestBase

import nose2
import shutil
import os

from qgis.core import QgsApplication, QgsProcessingException
from qgis.analysis import QgsNativeAlgorithms
import unittest
from qgis.testing import start_app, QgisTestCase
from processing.core.ProcessingConfig import ProcessingConfig
from processing.modeler.ModelerUtils import ModelerUtils


class TestQgisAlgorithms2(QgisTestCase, AlgorithmsTestBase.AlgorithmsTest):

    @classmethod
    def setUpClass(cls):
        start_app()
        from processing.core.Processing import Processing

        Processing.initialize()
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
        return "qgis_algorithm_tests2.yaml"


if __name__ == "__main__":
    nose2.main()
