# -*- coding: utf-8 -*-

"""
***************************************************************************
    OTBAlgorithmTests.py
    ---------------------
    Date                 : August 2016
    Copyright            : (C) 2016 by Manuel Grizonnet
    Email                : manuel.grizonnet@cnes.fr
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Manuel Grizonnet'
__date__ = 'August 2016'
__copyright__ = '(C) 2016, Manuel Grizonnet'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = ':%H$'

import AlgorithmsTestBase

import nose2
import shutil

from qgis.testing import (
    start_app,
    unittest
)


class TestOTBAlgorithms(unittest.TestCase, AlgorithmsTestBase.AlgorithmsTest):

    @classmethod
    def setUpClass(cls):
        start_app()
        from processing.core.Processing import Processing
        Processing.initialize()
        cls.cleanup_paths = []

    @classmethod
    def tearDownClass(cls):
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def test_definition_file(self):
        return 'otb_algorithm_tests.yaml'


if __name__ == '__main__':
    nose2.main()
