# -*- coding: utf-8 -*-

"""
***************************************************************************
    Grass7AlgorithmsImageryTest.py
    ------------------------------
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

__author__ = 'Médéric Ribreux'
__date__ = 'May 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

import AlgorithmsTestBase

import nose2
import shutil

from qgis.core import QgsApplication
from qgis.testing import (
    start_app,
    unittest
)
from grassprovider.Grass7AlgorithmProvider import Grass7AlgorithmProvider
from grassprovider.Grass7Utils import Grass7Utils


class TestGrass7AlgorithmsImageryTest(unittest.TestCase, AlgorithmsTestBase.AlgorithmsTest):

    @classmethod
    def setUpClass(cls):
        start_app()
        cls.provider = Grass7AlgorithmProvider()
        QgsApplication.processingRegistry().addProvider(cls.provider)
        cls.cleanup_paths = []

        assert Grass7Utils.installedVersion()

    @classmethod
    def tearDownClass(cls):
        QgsApplication.processingRegistry().removeProvider(cls.provider)
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def test_definition_file(self):
        return 'grass7_algorithms_imagery_tests.yaml'


if __name__ == '__main__':
    nose2.main()
