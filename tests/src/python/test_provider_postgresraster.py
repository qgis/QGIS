# -*- coding: utf-8 -*-
"""QGIS Unit tests for the postgres raster provider.

Note: to prepare the DB, you need to run the sql files specified in
tests/testdata/provider/testdata_pg.sh

Read tests/README.md about writing/launching tests with PostgreSQL.

Run with ctest -V -R PyQgsPostgresProvider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
from builtins import next
__author__ = 'Alessandro Pasotti'
__date__ = '2019-12-20'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA

import os
import time

from qgis.core import (
    QgsSettings,
    QgsReadWriteContext,
    QgsRectangle,
    QgsCoordinateReferenceSystem,
    QgsProject,
    QgsRasterLayer,
    QgsPointXY,
    QgsRaster,
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsPostgresRasterProvider(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.dbconn = 'service=qgis_test'
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        # Create test layers
        cls.rl = QgsRasterLayer(cls.dbconn + ' sslmode=disable key=\'pk\' srid=9001  table="public"."aspect_clipped_gpu_mini" sql=', 'test', 'postgresraster')
        assert cls.rl.isValid()
        cls.source = cls.rl.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def testExtent(self):
        extent = self.rl.extent()
        self.assertEqual(extent, QgsRectangle(4080050, 2430625, 4080200, 2430750))

    def testGetData(self):
        identify = self.source.identify(QgsPointXY(4080137.9, 2430687.9), QgsRaster.IdentifyFormatValue)
        expected = 192.51044
        self.assertEqual(identify.results()[1], expected)


if __name__ == '__main__':
    unittest.main()
