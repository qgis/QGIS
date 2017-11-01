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
from processing.algs.gdal.OgrToPostGis import OgrToPostGis
from processing.algs.gdal.GdalUtils import GdalUtils
from qgis.core import QgsProcessingContext
import nose2
import os
import shutil
import tempfile

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

    def testOgrLayerNameExtraction(self):
        outdir = tempfile.mkdtemp()
        self.cleanup_paths.append(outdir)

        def _copyFile(dst):
            shutil.copyfile(os.path.join(testDataPath, 'custom', 'grass7', 'weighted.csv'), dst)

        # OGR provider - single layer
        _copyFile(os.path.join(outdir, 'a.csv'))
        name = GdalUtils.ogrLayerName(outdir)
        self.assertEqual(name, 'a')

        # OGR provider - multiple layers
        _copyFile(os.path.join(outdir, 'b.csv'))
        name1 = GdalUtils.ogrLayerName(outdir + '|layerid=0')
        name2 = GdalUtils.ogrLayerName(outdir + '|layerid=1')
        self.assertEqual(sorted([name1, name2]), ['a', 'b'])

        name = GdalUtils.ogrLayerName(outdir + '|layerid=2')
        self.assertIsNone(name)

        # OGR provider - layername takes precedence
        name = GdalUtils.ogrLayerName(outdir + '|layername=f')
        self.assertEqual(name, 'f')

        name = GdalUtils.ogrLayerName(outdir + '|layerid=0|layername=f')
        self.assertEqual(name, 'f')

        name = GdalUtils.ogrLayerName(outdir + '|layername=f|layerid=0')
        self.assertEqual(name, 'f')

        # SQLiite provider
        name = GdalUtils.ogrLayerName('dbname=\'/tmp/x.sqlite\' table="t" (geometry) sql=')
        self.assertEqual(name, 't')

        # PostgreSQL provider
        name = GdalUtils.ogrLayerName('port=5493 sslmode=disable key=\'edge_id\' srid=0 type=LineString table="city_data"."edge" (geom) sql=')
        self.assertEqual(name, 'city_data.edge')


class TestGdalOgrToPostGis(unittest.TestCase):

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

        obj = OgrToPostGis()
        obj.initAlgorithm({})

        parameters = {}
        context = QgsProcessingContext()

        # NOTE: defaults are debatable, see
        # https://github.com/qgis/QGIS/pull/3607#issuecomment-253971020
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "host=localhost port=5432 active_schema=public")

        parameters['HOST'] = 'remote'
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "host=remote port=5432 active_schema=public")

        parameters['HOST'] = ''
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "port=5432 active_schema=public")

        parameters['PORT'] = '5555'
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "port=5555 active_schema=public")

        parameters['PORT'] = ''
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "active_schema=public")

        parameters['USER'] = 'usr'
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "active_schema=public user=usr")

        parameters['PASSWORD'] = 'pwd'
        self.assertEqual(obj.getConnectionString(parameters, context),
                         "password=pwd active_schema=public user=usr")


if __name__ == '__main__':
    nose2.main()
