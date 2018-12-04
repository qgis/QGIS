# -*- coding: utf-8 -*-

"""
***************************************************************************
    SagaAlgorithmsTests.py
    ---------------------
    Date                 : September 2017
    Copyright            : (C) 2017 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'September 2017'
__copyright__ = '(C) 2017, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = ':%H$'

import nose2
import shutil

from qgis.core import (QgsProcessingParameterNumber,
                       QgsProcessingParameterDefinition)
from qgis.testing import start_app, unittest

from processing.algs.saga.SagaParameters import Parameters, SagaImageOutputParam
import AlgorithmsTestBase


class TestSagaAlgorithms(unittest.TestCase, AlgorithmsTestBase.AlgorithmsTest):

    @classmethod
    def setUpClass(cls):
        start_app()
        from processing.core.Processing import Processing
        Processing.initialize()
        cls.cleanup_paths = []

    @classmethod
    def tearDownClass(cls):
        from processing.core.Processing import Processing
        Processing.deinitialize()
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def test_definition_file(self):
        return 'saga_algorithm_tests.yaml'

    def test_is_parameter_line(self):
        # Test determining whether a line is a parameter line
        self.assertFalse(Parameters.is_parameter_line(''))
        self.assertFalse(Parameters.is_parameter_line('xxxxxxxxx'))
        self.assertTrue(Parameters.is_parameter_line('QgsProcessingParameterNumber|R_PERCTL_MIN|Percentiles Range for RED max|QgsProcessingParameterNumber.Integer|1|False|1|99'))
        self.assertTrue(Parameters.is_parameter_line('*QgsProcessingParameterNumber|R_PERCTL_MIN|Percentiles Range for RED max|QgsProcessingParameterNumber.Integer|1|False|1|99'))
        self.assertTrue(Parameters.is_parameter_line('SagaImageOutput|RGB|Output RGB'))

    def test_param_line(self):
        # Test creating a parameter from a description line
        param = Parameters.create_parameter_from_line('QgsProcessingParameterNumber|R_PERCTL_MIN|Percentiles Range for RED max|QgsProcessingParameterNumber.Integer|1|False|1|99')
        self.assertIsInstance(param, QgsProcessingParameterNumber)
        self.assertEqual(param.name(), 'R_PERCTL_MIN')
        self.assertEqual(param.description(), 'Percentiles Range for RED max')
        self.assertEqual(param.dataType(), QgsProcessingParameterNumber.Integer)
        self.assertFalse(param.flags() & QgsProcessingParameterDefinition.FlagOptional)
        self.assertEqual(param.minimum(), 1)
        self.assertEqual(param.maximum(), 99)

        # Test SagaImageOutputParam line
        param = Parameters.create_parameter_from_line('SagaImageOutput|RGB|Output RGB')
        self.assertIsInstance(param, SagaImageOutputParam)
        self.assertEqual(param.name(), 'RGB')
        self.assertEqual(param.description(), 'Output RGB')
        self.assertEqual(param.defaultFileExtension(), 'tif')
        self.assertEqual(param.supportedOutputRasterLayerExtensions(), ['tif'])


if __name__ == '__main__':
    nose2.main()
