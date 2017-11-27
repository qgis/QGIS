# -*- coding: utf-8 -*-

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

__author__ = 'Matthias Kuhn'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Matthias Kuhn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = ':%H$'

import AlgorithmsTestBase

import nose2
import shutil

from qgis.core import (QgsApplication,
                       QgsProcessingAlgorithm,
                       QgsProcessingFeedback,
                       QgsProcessingException)
from qgis.analysis import (QgsNativeAlgorithms)
from qgis.testing import start_app, unittest
from processing.tools.dataobjects import createContext


class TestAlg(QgsProcessingAlgorithm):

    def __init__(self):
        super().__init__()

    def name(self):
        return 'testalg'

    def displayName(self):
        return 'testalg'

    def initAlgorithm(self, config=None):
        pass

    def createInstance(self):
        return TestAlg()

    def processAlgorithm(self, parameters, context, feedback):
        raise QgsProcessingException('Exception while processing')
        return {}


class TestQgisAlgorithms(unittest.TestCase, AlgorithmsTestBase.AlgorithmsTest):

    @classmethod
    def setUpClass(cls):
        start_app()
        QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())
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

    def test_definition_file(self):
        return 'qgis_algorithm_tests.yaml'

    def testProcessingException(self):
        """
        Test that Python exception is caught when running an alg
        """

        alg = TestAlg()
        context = createContext()
        feedback = QgsProcessingFeedback()
        results, ok = alg.run({}, context, feedback)
        self.assertFalse(ok)


if __name__ == '__main__':
    nose2.main()
