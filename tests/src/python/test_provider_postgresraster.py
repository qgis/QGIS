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
        cls.rl = QgsRasterLayer(cls.dbconn + ' sslmode=disable key=\'rid\' srid=3035  table="public"."raster_tiled_3035" sql=', 'test', 'postgresraster')
        assert cls.rl.isValid()
        cls.source = cls.rl.dataProvider()

    def gdal_block_compare(self, rlayer, band, extent, width, height, value):
        """Compare a block result with GDAL raster"""

        uri = rlayer.uri()
        gdal_uri = "PG: dbname={dbname} mode=2 host={host} port={port} table={table} schema={schema} sslmode=disable".format(**{
            'dbname': uri.database(),
            'host': uri.host(),
            'port': uri.port(),
            'table': uri.table(),
            'schema': uri.schema()
        })
        gdal_rl = QgsRasterLayer(gdal_uri, "rl", "gdal")
        self.assertTrue(gdal_rl.isValid())
        self.assertEqual(value, gdal_rl.dataProvider().block(band, self.rl.extent(), 6, 5).data().toHex())

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def testExtent(self):
        extent = self.rl.extent()
        self.assertEqual(extent, QgsRectangle(4080050, 2430625, 4080200, 2430750))

    def testSize(self):
        self.assertEqual(self.source.xSize(), 6)
        self.assertEqual(self.source.ySize(), 5)

    def testCrs(self):
        self.assertEqual(self.source.crs().authid(), 'EPSG:3035')

    def testGetData(self):
        identify = self.source.identify(QgsPointXY(4080137.9, 2430687.9), QgsRaster.IdentifyFormatValue)
        expected = 192.51044
        self.assertAlmostEqual(identify.results()[1], expected, 4)

    def testBlockTiled(self):

        expected = b'6a610843880b0e431cc2194306342543b7633c43861858436e0a1143bbad194359612743a12b334317be4343dece59432b621b43f0e42843132b3843ac824043e6cf48436e465a435c4d2d430fa63d43f87a4843b5494a4349454e4374f35b43906e41433ab54c43b056504358575243b1ec574322615f43'
        block = self.source.block(1, self.rl.extent(), 6, 5)
        actual = block.data().toHex()
        self.assertEqual(len(actual), len(expected))
        self.assertEqual(actual, expected)

    def testNoConstraintRaster(self):
        """Read unconstrained raster layer"""

        rl = QgsRasterLayer(self.dbconn + ' sslmode=disable key=\'pk\' srid=3035  table="public"."raster_3035_no_constraints" sql=', 'test', 'postgresraster')
        self.assertTrue(rl.isValid())

    def testPkGuessing(self):
        """Read raster layer with no pkey in uri"""

        rl = QgsRasterLayer(self.dbconn + ' sslmode=disable srid=3035  table="public"."raster_tiled_3035" sql=', 'test', 'postgresraster')
        self.assertTrue(rl.isValid())

    def testWhereCondition(self):
        """Read raster layer with where condition"""

        rl_nowhere = QgsRasterLayer(self.dbconn + ' sslmode=disable srid=3035  table="public"."raster_3035_tiled_no_overviews"' +
                                    'sql=', 'test', 'postgresraster')
        self.assertTrue(rl_nowhere.isValid())

        rl = QgsRasterLayer(self.dbconn + ' sslmode=disable srid=3035  table="public"."raster_3035_tiled_no_overviews"' +
                            'sql="category" = \'cat2\'', 'test', 'postgresraster')
        self.assertTrue(rl.isValid())

        self.assertTrue(not rl.extent().isEmpty())
        self.assertNotEqual(rl_nowhere.extent(), rl.extent())

        self.assertIsNone(rl.dataProvider().identify(QgsPointXY(4080137.9, 2430687.9), QgsRaster.IdentifyFormatValue).results()[1])
        self.assertIsNotNone(rl_nowhere.dataProvider().identify(QgsPointXY(4080137.9, 2430687.9), QgsRaster.IdentifyFormatValue).results()[1])

        self.assertAlmostEqual(rl.dataProvider().identify(rl.extent().center(), QgsRaster.IdentifyFormatValue).results()[1], 223.38, 2)

    def testNoPk(self):
        """Read raster with no PK"""

        rl = QgsRasterLayer(self.dbconn + ' sslmode=disable srid=3035  table="public"."raster_3035_tiled_no_pk"' +
                            'sql=', 'test', 'postgresraster')
        self.assertTrue(rl.isValid())

    def testCompositeKey(self):
        """Read raster with composite pks"""

        rl = QgsRasterLayer(self.dbconn + ' sslmode=disable srid=3035  table="public"."raster_3035_tiled_composite_pk"' +
                            'sql=', 'test', 'postgresraster')
        self.assertTrue(rl.isValid())


if __name__ == '__main__':
    unittest.main()
