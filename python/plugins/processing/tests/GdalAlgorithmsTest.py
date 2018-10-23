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
from processing.algs.gdal.ogr2ogr import Ogr2Ogr

import nose2
import shutil
import os
from qgis.testing import (
    start_app,
    unittest
)

testDataPath = os.path.join(os.path.dirname(__file__), 'testdata')


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

    def testOgr2Ogr(self):
        source = os.path.join(testDataPath, 'polys.gml')

        alg = Ogr2Ogr()
        alg.setParameterValue('INPUT_LAYER', source)
        alg.setParameterValue('FORMAT', 0)
        alg.setOutputValue('OUTPUT_LAYER', 'd:/temp/check.shp')
        self.assertEqual(
            alg.getConsoleCommands(),
            ['ogr2ogr',
             '-f "ESRI Shapefile" "d:/temp/check.shp" ' +
             source + ' polys2'])

        alg = Ogr2Ogr()
        alg.setParameterValue('INPUT_LAYER', source)
        alg.setParameterValue('FORMAT', 22)
        alg.setOutputValue('OUTPUT_LAYER', 'd:/temp/check.gpkg')
        self.assertEqual(
            alg.getConsoleCommands(),
            ['ogr2ogr',
             '-f GPKG "d:/temp/check.gpkg" ' +
             source + ' polys2'])


class TestGdalOgr2OgrToPostgis(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        #start_app()
        pass

    @classmethod
    def tearDownClass(cls):
        pass

    # See http://hub.qgis.org/issues/15706
    def test_getConnectionString(self):

        obj = Ogr2OgrToPostGis()

        cs = obj.getConnectionString()
        # NOTE: defaults are debatable, see
        # https://github.com/qgis/QGIS/pull/3607#issuecomment-253971020
        self.assertEquals(obj.getConnectionString(),
                          "host=localhost port=5432 active_schema=public")

        obj.setParameterValue('HOST', 'remote')
        self.assertEquals(obj.getConnectionString(),
                          "host=remote port=5432 active_schema=public")

        obj.setParameterValue('HOST', '')
        self.assertEquals(obj.getConnectionString(),
                          "port=5432 active_schema=public")

        obj.setParameterValue('PORT', '5555')
        self.assertEquals(obj.getConnectionString(),
                          "port=5555 active_schema=public")

        obj.setParameterValue('PORT', '')
        self.assertEquals(obj.getConnectionString(),
                          "active_schema=public")

        obj.setParameterValue('USER', 'usr')
        self.assertEquals(obj.getConnectionString(),
                          "active_schema=public user=usr")

        obj.setParameterValue('PASSWORD', 'pwd')
        self.assertEquals(obj.getConnectionString(),
                          "password=pwd active_schema=public user=usr")


if __name__ == '__main__':
    nose2.main()
