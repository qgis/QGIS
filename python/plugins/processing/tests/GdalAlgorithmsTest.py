# -*- coding: utf-8 -*-

"""
***************************************************************************
    GdalAlgorithmTests.py
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
from processing.algs.gdal.ogr2ogrtopostgis import Ogr2OgrToPostGis

import nose2
import shutil

from qgis.testing import (
    start_app,
    unittest
)


class TestGdalAlgorithms(unittest.TestCase, AlgorithmsTestBase.AlgorithmsTest):

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
        return 'gdal_algorithm_tests.yaml'


class TestGdalOgr2OgrToPostgis(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # start_app()
        from processing.core.Processing import Processing
        Processing.initialize()

    @classmethod
    def tearDownClass(cls):
        pass

    # See https://issues.qgis.org/issues/15706
    def test_getConnectionString(self):

        obj = Ogr2OgrToPostGis()

        parameters = {}

        # NOTE: defaults are debatable, see
        # https://github.com/qgis/QGIS/pull/3607#issuecomment-253971020
        self.assertEqual(obj.getConnectionString(parameters),
                         "host=localhost port=5432 active_schema=public")

        parameters['HOST'] = 'remote'
        self.assertEqual(obj.getConnectionString(parameters),
                         "host=remote port=5432 active_schema=public")

        parameters['HOST'] = ''
        self.assertEqual(obj.getConnectionString(parameters),
                         "port=5432 active_schema=public")

        parameters['PORT'] = '5555'
        self.assertEqual(obj.getConnectionString(parameters),
                         "port=5555 active_schema=public")

        parameters['PORT'] = ''
        self.assertEqual(obj.getConnectionString(parameters),
                         "active_schema=public")

        parameters['USER'] = 'usr'
        self.assertEqual(obj.getConnectionString(parameters),
                         "active_schema=public user=usr")

        parameters['PASSWORD'] = 'pwd'
        self.assertEqual(obj.getConnectionString(parameters),
                         "password=pwd active_schema=public user=usr")


if __name__ == '__main__':
    nose2.main()
