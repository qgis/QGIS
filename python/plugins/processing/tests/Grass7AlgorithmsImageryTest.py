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

from qgis.testing import (
    start_app,
    unittest
)
from processing.algs.grass7.Grass7Utils import Grass7Utils


class TestGrass7AlgorithmsImageryTest(unittest.TestCase, AlgorithmsTestBase.AlgorithmsTest):

    @classmethod
    def setUpClass(cls):
        start_app()
        from processing.core.Processing import Processing
        Processing.initialize()
        cls.cleanup_paths = []

        assert Grass7Utils.installedVersion()

    @classmethod
    def tearDownClass(cls):
        from processing.core.Processing import Processing
        Processing.deinitialize()
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def test_definition_file(self):
        return 'grass7_algorithms_imagery_tests.yaml'


if __name__ == '__main__':
    nose2.main()
