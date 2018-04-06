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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = ':%H$'

import AlgorithmsTestBase

import nose2
import shutil
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

        cmdList = ["grass74", "grass72", "grass71", "grass70", "grass",
                   "grass74.sh", "grass72.sh", "grass71.sh", "grass70.sh", "grass.sh"]
        commands = {cmd: shutil.which(cmd) for cmd in cmdList}

        assert Grass7Utils.installedVersion(), commands

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
