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
    QgsProviderRegistry,
    QgsRasterBandStats,
    QgsDataSourceUri,
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath, compareWkt

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsPostgresRasterProvider(unittest.TestCase):

    @classmethod
    def _load_test_table(cls, schemaname, tablename, basename=None):

        postgres_conn = cls.dbconn + " sslmode=disable "
        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        conn = md.createConnection(postgres_conn, {})

        if basename is None:
            basename = tablename

        if tablename not in [n.tableName() for n in conn.tables(schemaname)]:
            with open(os.path.join(TEST_DATA_DIR, 'provider', 'postgresraster', basename + '.sql'), 'r') as f:
                sql = f.read()
                conn.executeSql(sql)
            assert (tablename in [n.tableName() for n in conn.tables(
                schemaname)]), tablename + ' not found!'

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.dbconn = 'service=qgis_test'
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']

        cls._load_test_table('public', 'raster_tiled_3035')
        cls._load_test_table('public', 'raster_3035_no_constraints')
        cls._load_test_table('public', 'raster_3035_tiled_no_overviews')
        cls._load_test_table('public', 'raster_3035_tiled_no_pk')
        cls._load_test_table('public', 'raster_3035_tiled_composite_pk')
        cls._load_test_table('public', 'raster_3035_untiled_multiple_rows')
        cls._load_test_table('idro', 'cosmo_i5_snow', 'bug_34823_pg_raster')
        cls._load_test_table(
            'public', 'int16_regression_36689', 'bug_36689_pg_raster')
        cls._load_test_table('public', 'bug_37968_dem_linear_cdn_extract')
        cls._load_test_table('public', 'bug_39017_untiled_no_metadata')

        # Fix timing issues in backend
        # time.sleep(1)

        # Create test layer
        cls.rl = QgsRasterLayer(
            cls.dbconn + ' sslmode=disable key=\'rid\' srid=3035  table="public"."raster_tiled_3035" sql=', 'test',
            'postgresraster')
        assert cls.rl.isValid()
        cls.source = cls.rl.dataProvider()

    def gdal_block_compare(self, rlayer, band, extent, width, height, value):
        """Compare a block result with GDAL raster"""

        uri = rlayer.uri()
        gdal_uri = "PG: dbname={dbname} mode=2 host={host} port={port} table={table} schema={schema} sslmode=disable".format(
            **{
                'dbname': uri.database(),
                'host': uri.host(),
                'port': uri.port(),
                'table': uri.table(),
                'schema': uri.schema()
            })
        gdal_rl = QgsRasterLayer(gdal_uri, "rl", "gdal")
        self.assertTrue(gdal_rl.isValid())
        self.assertEqual(value, gdal_rl.dataProvider().block(
            band, self.rl.extent(), 6, 5).data().toHex())

    def testExtent(self):
        extent = self.rl.extent()
        self.assertEqual(extent, QgsRectangle(
            4080050, 2430625, 4080200, 2430750))

    def testSize(self):
        self.assertEqual(self.source.xSize(), 6)
        self.assertEqual(self.source.ySize(), 5)

    def testCrs(self):
        self.assertEqual(self.source.crs().authid(), 'EPSG:3035')

    def testGetData(self):
        identify = self.source.identify(QgsPointXY(
            4080137.9, 2430687.9), QgsRaster.IdentifyFormatValue)
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

        rl = QgsRasterLayer(
            self.dbconn + ' sslmode=disable key=\'pk\' srid=3035  table="public"."raster_3035_no_constraints" sql=',
            'test', 'postgresraster')
        self.assertTrue(rl.isValid())

    def testPkGuessing(self):
        """Read raster layer with no pkey in uri"""

        rl = QgsRasterLayer(self.dbconn + ' sslmode=disable srid=3035  table="public"."raster_tiled_3035" sql=', 'test',
                            'postgresraster')
        self.assertTrue(rl.isValid())

    def testWhereCondition(self):
        """Read raster layer with where condition"""

        rl_nowhere = QgsRasterLayer(
            self.dbconn + ' sslmode=disable srid=3035  table="public"."raster_3035_tiled_no_overviews"' +
            'sql=', 'test', 'postgresraster')
        self.assertTrue(rl_nowhere.isValid())

        rl = QgsRasterLayer(
            self.dbconn + ' sslmode=disable srid=3035  table="public"."raster_3035_tiled_no_overviews"' +
            'sql="category" = \'cat2\'', 'test', 'postgresraster')
        self.assertTrue(rl.isValid())

        self.assertTrue(not rl.extent().isEmpty())
        self.assertNotEqual(rl_nowhere.extent(), rl.extent())

        self.assertIsNone(
            rl.dataProvider().identify(QgsPointXY(4080137.9, 2430687.9), QgsRaster.IdentifyFormatValue).results()[1])
        self.assertIsNotNone(rl_nowhere.dataProvider().identify(QgsPointXY(4080137.9, 2430687.9),
                                                                QgsRaster.IdentifyFormatValue).results()[1])

        self.assertAlmostEqual(
            rl.dataProvider().identify(rl.extent().center(), QgsRaster.IdentifyFormatValue).results()[1], 223.38, 2)

        self.assertTrue(compareWkt(rl_nowhere.extent().asWktPolygon(),
                                   'POLYGON((4080050 2430625, 4080200 2430625, 4080200 2430750, 4080050 2430750, 4080050 2430625))'))

        self.assertTrue(compareWkt(rl.extent().asWktPolygon(),
                                   'POLYGON((4080150 2430625, 4080200 2430625, 4080200 2430650, 4080150 2430650, 4080150 2430625))'))

        self.assertNotEqual(rl.extent(), rl_nowhere.extent())

        # Now check if setSubsetString updates the extent
        self.assertTrue(rl_nowhere.setSubsetString('"category" = \'cat2\''))
        self.assertEqual(rl.extent(), rl_nowhere.extent())

    def testNoPk(self):
        """Read raster with no PK"""

        rl = QgsRasterLayer(self.dbconn + ' sslmode=disable srid=3035  table="public"."raster_3035_tiled_no_pk"' +
                            'sql=', 'test', 'postgresraster')
        self.assertTrue(rl.isValid())

    def testCompositeKey(self):
        """Read raster with composite pks"""

        rl = QgsRasterLayer(
            self.dbconn + ' sslmode=disable srid=3035  table="public"."raster_3035_tiled_composite_pk"' +
            'sql=', 'test', 'postgresraster')
        self.assertTrue(rl.isValid())
        data = rl.dataProvider().block(1, rl.extent(), 3, 3)
        self.assertEqual(int(data.value(0, 0)), 142)

    @unittest.skip('Performance test is disabled in Travis environment')
    def testSpeed(self):
        """Compare speed with GDAL provider, this test was used during development"""

        conn = "user={user} host=localhost port=5432 password={password} dbname={speed_db} ".format(
            user=os.environ.get('USER'),
            password=os.environ.get('USER'),
            speed_db='qgis_test'
        )

        table = 'basic_map_tiled'
        schema = 'public'

        def _speed_check(schema, table, width, height):
            print('-' * 80)
            print("Testing: {schema}.{table}".format(
                table=table, schema=schema))
            print('-' * 80)

            # GDAL
            start = time.time()
            rl = QgsRasterLayer(
                "PG: " + conn +
                "table={table} mode=2 schema={schema}".format(
                    table=table, schema=schema), 'gdal_layer',
                'gdal')
            self.assertTrue(rl.isValid())
            # Make is smaller than full extent
            extent = rl.extent().buffered(-rl.extent().width() * 0.2)
            checkpoint_1 = time.time()
            print("Tiled GDAL start time: {:.6f}".format(checkpoint_1 - start))
            rl.dataProvider().block(1, extent, width, height)
            checkpoint_2 = time.time()
            print("Tiled GDAL first block time: {:.6f}".format(
                checkpoint_2 - checkpoint_1))
            # rl.dataProvider().block(1, extent, width, height)
            checkpoint_3 = time.time()
            print("Tiled GDAL second block time: {:.6f}".format(
                checkpoint_3 - checkpoint_2))
            print("Total GDAL time: {:.6f}".format(checkpoint_3 - start))
            print('-' * 80)

            # PG native
            start = time.time()
            rl = QgsRasterLayer(conn + "table={table} schema={schema}".format(table=table, schema=schema), 'gdal_layer',
                                'postgresraster')
            self.assertTrue(rl.isValid())
            extent = rl.extent().buffered(-rl.extent().width() * 0.2)
            checkpoint_1 = time.time()
            print("Tiled PG start time: {:.6f}".format(checkpoint_1 - start))
            rl.dataProvider().block(1, extent, width, height)
            checkpoint_2 = time.time()
            print("Tiled PG first block time: {:.6f}".format(
                checkpoint_2 - checkpoint_1))
            rl.dataProvider().block(1, extent, width, height)
            checkpoint_3 = time.time()
            print("Tiled PG second block time: {:.6f}".format(
                checkpoint_3 - checkpoint_2))
            print("Total PG time: {:.6f}".format(checkpoint_3 - start))
            print('-' * 80)

        _speed_check(schema, table, 1000, 1000)

    def testOtherSchema(self):
        """Test that a layer in a different schema than public can be loaded
        See: GH #34823"""

        rl = QgsRasterLayer(
            self.dbconn + " sslmode=disable table={table} schema={schema}".format(
                table='cosmo_i5_snow', schema='idro'),
            'pg_layer', 'postgresraster')
        self.assertTrue(rl.isValid())
        self.assertTrue(compareWkt(rl.extent().asWktPolygon(),
                                   'POLYGON((-64.79286766849691048 -77.26689086732433509, -62.18292922825105506 -77.26689086732433509, -62.18292922825105506 -74.83694818157819384, -64.79286766849691048 -74.83694818157819384, -64.79286766849691048 -77.26689086732433509))'))

    def testUntiledMultipleRows(self):
        """Test multiple rasters (one per row)"""

        rl = QgsRasterLayer(self.dbconn + " sslmode=disable table={table} schema={schema} sql=\"pk\" = 1".format(
            table='raster_3035_untiled_multiple_rows', schema='public'), 'pg_layer', 'postgresraster')
        self.assertTrue(rl.isValid())
        block = rl.dataProvider().block(1, rl.extent(), 2, 2)
        data = []
        for i in range(2):
            for j in range(2):
                data.append(int(block.value(i, j)))
        self.assertEqual(data, [136, 142, 145, 153])

        rl = QgsRasterLayer(self.dbconn + " sslmode=disable table={table} schema={schema} sql=\"pk\" = 2".format(
            table='raster_3035_untiled_multiple_rows', schema='public'), 'pg_layer', 'postgresraster')
        self.assertTrue(rl.isValid())
        block = rl.dataProvider().block(1, rl.extent(), 2, 2)
        data = []
        for i in range(2):
            for j in range(2):
                data.append(int(block.value(i, j)))
        self.assertEqual(data, [136, 142, 161, 169])

    def testSetSubsetString(self):
        """Test setSubsetString"""

        rl = QgsRasterLayer(self.dbconn + " sslmode=disable table={table} schema={schema} sql=\"pk\" = 2".format(
            table='raster_3035_untiled_multiple_rows', schema='public'), 'pg_layer', 'postgresraster')
        self.assertTrue(rl.isValid())

        block = rl.dataProvider().block(1, rl.extent(), 2, 2)
        data = []
        for i in range(2):
            for j in range(2):
                data.append(int(block.value(i, j)))
        self.assertEqual(data, [136, 142, 161, 169])

        stats = rl.dataProvider().bandStatistics(
            1, QgsRasterBandStats.Min | QgsRasterBandStats.Max, rl.extent())
        self.assertEqual(int(stats.minimumValue), 136)
        self.assertEqual(int(stats.maximumValue), 169)

        ce = rl.renderer().contrastEnhancement()
        min_max = int(ce.minimumValue()), int(ce.maximumValue())
        self.assertEqual(min_max, (136, 169))

        # Change filter:
        self.assertTrue(rl.setSubsetString('"pk" = 1'))
        block = rl.dataProvider().block(1, rl.extent(), 2, 2)
        data = []
        for i in range(2):
            for j in range(2):
                data.append(int(block.value(i, j)))
        self.assertEqual(data, [136, 142, 145, 153])

        # Check that we have new statistics
        stats = rl.dataProvider().bandStatistics(
            1, QgsRasterBandStats.Min | QgsRasterBandStats.Max, rl.extent())
        self.assertEqual(int(stats.minimumValue), 136)
        self.assertEqual(int(stats.maximumValue), 153)

        # Check that the renderer has been updated
        ce = rl.renderer().contrastEnhancement()
        new_min_max = int(ce.minimumValue()), int(ce.maximumValue())
        self.assertNotEqual(new_min_max, min_max)
        self.assertEqual(new_min_max, (136, 153))

        # Set invalid filter
        self.assertFalse(rl.setSubsetString('"pk_wrong" = 1'))
        block = rl.dataProvider().block(1, rl.extent(), 2, 2)
        data = []
        for i in range(2):
            for j in range(2):
                data.append(int(block.value(i, j)))
        self.assertEqual(data, [136, 142, 145, 153])

    def testTime(self):
        """Test time series and time subset string when default value is set"""

        def _test_block(rl, expected_block, expected_single):

            self.assertTrue(rl.isValid())
            block = rl.dataProvider().block(1, rl.extent(), 2, 2)
            data = []
            for i in range(2):
                for j in range(2):
                    data.append(int(block.value(i, j)))
            self.assertEqual(data, expected_block)

            block = rl.dataProvider().block(1, rl.extent(), 1, 1)
            self.assertEqual(int(block.value(0, 0)), expected_single)

        # First check that setting different temporal default values we get different results
        rl = QgsRasterLayer(
            self.dbconn + " sslmode=disable table={table} schema={schema} temporalDefaultTime='2020-04-01T00:00:00' temporalFieldIndex='1'".format(
                table='raster_3035_untiled_multiple_rows', schema='public'), 'pg_layer', 'postgresraster')
        self.assertEqual(rl.subsetString(), "")

        _test_block(rl, [136, 142, 145, 153], 153)

        rl = QgsRasterLayer(
            self.dbconn + " sslmode=disable table={table} schema={schema} temporalDefaultTime='2020-04-05T00:00:00' temporalFieldIndex='1'".format(
                table='raster_3035_untiled_multiple_rows', schema='public'), 'pg_layer', 'postgresraster')
        self.assertEqual(rl.subsetString(), "")

        _test_block(rl, [136, 142, 161, 169], 169)

        # Check that manually setting a subsetString we get the same results
        rl = QgsRasterLayer(
            self.dbconn + " sslmode=disable table={table} schema={schema}  sql=\"data\" = '2020-04-01'".format(
                table='raster_3035_untiled_multiple_rows', schema='public'), 'pg_layer', 'postgresraster')
        self.assertEqual(rl.subsetString(), '"data" = \'2020-04-01\'')

        _test_block(rl, [136, 142, 145, 153], 153)

        rl = QgsRasterLayer(
            self.dbconn + " sslmode=disable table={table} schema={schema}  sql=\"data\" = '2020-04-05'".format(
                table='raster_3035_untiled_multiple_rows', schema='public'), 'pg_layer', 'postgresraster')
        self.assertEqual(rl.subsetString(), '"data" = \'2020-04-05\'')

        _test_block(rl, [136, 142, 161, 169], 169)

        # Now check if the varchar temporal field works the same
        rl = QgsRasterLayer(
            self.dbconn + " sslmode=disable table={table} schema={schema} temporalDefaultTime='2020-04-01T00:00:00' temporalFieldIndex='2'".format(
                table='raster_3035_untiled_multiple_rows', schema='public'), 'pg_layer', 'postgresraster')
        self.assertEqual(rl.subsetString(), '')

        _test_block(rl, [136, 142, 145, 153], 153)

        rl = QgsRasterLayer(
            self.dbconn + " sslmode=disable table={table} schema={schema} temporalDefaultTime='2020-04-05T00:00:00' temporalFieldIndex='2'".format(
                table='raster_3035_untiled_multiple_rows', schema='public'), 'pg_layer', 'postgresraster')
        self.assertEqual(rl.subsetString(), '')

        _test_block(rl, [136, 142, 161, 169], 169)

    def testMetadataEncodeDecode(self):
        """Round trip tests on URIs"""

        def _round_trip(uri):
            decoded = md.decodeUri(uri)
            self.assertEqual(decoded, md.decodeUri(md.encodeUri(decoded)))

        uri = self.dbconn + \
            ' sslmode=disable key=\'rid\' srid=3035  table="public"."raster_tiled_3035" sql='
        md = QgsProviderRegistry.instance().providerMetadata('postgresraster')
        decoded = md.decodeUri(uri)
        self.assertEqual(decoded, {
            'key': 'rid',
            'schema': 'public',
            'service': 'qgis_test',
            'srid': '3035',
            'sslmode': QgsDataSourceUri.SslDisable,
            'table': 'raster_tiled_3035',
        })

        _round_trip(uri)

        uri = self.dbconn + \
            ' sslmode=prefer key=\'rid\' srid=3035 temporalFieldIndex=2 temporalDefaultTime=2020-03-02 ' + \
            'authcfg=afebeff username=\'my username\' password=\'my secret password=\' ' + \
            'enableTime=true table="public"."raster_tiled_3035" (rast) sql="a_field" != 1223223'

        _round_trip(uri)

        decoded = md.decodeUri(uri)
        self.assertEqual(decoded, {
            'authcfg': 'afebeff',
            'enableTime': 'true',
            'geometrycolumn': 'rast',
            'key': 'rid',
            'password': 'my secret password=',
            'schema': 'public',
            'service': 'qgis_test',
            'sql': '"a_field" != 1223223',
            'srid': '3035',
            'sslmode': QgsDataSourceUri.SslPrefer,
            'table': 'raster_tiled_3035',
            'temporalDefaultTime':
                '2020-03-02',
            'temporalFieldIndex': '2',
            'username': 'my username',
        })

    def testInt16(self):
        """Test regression https://github.com/qgis/QGIS/issues/36689"""

        rl = QgsRasterLayer(
            self.dbconn + " sslmode=disable table={table} schema={schema}".format(
                table='int16_regression_36689', schema='public'), 'pg_layer', 'postgresraster')

        self.assertTrue(rl.isValid())
        block = rl.dataProvider().block(1, rl.extent(), 6, 6)
        data = []
        for i in range(6):
            for j in range(6):
                data.append(int(block.value(i, j)))

        self.assertEqual(data, [55, 52, 46, 39, 33, 30, 58, 54, 49, 45, 41, 37, 58, 54, 50,
                                47, 45, 43, 54, 51, 49, 47, 46, 44, 47, 47, 47, 47, 46, 45, 41, 43, 45, 48, 49, 46])

    def testNegativeScaleY(self):
        """Test regression https://github.com/qgis/QGIS/issues/37968
        Y is growing south
        """

        rl = QgsRasterLayer(
            self.dbconn + " sslmode=disable table={table} schema={schema}".format(
                table='bug_37968_dem_linear_cdn_extract', schema='public'), 'pg_layer', 'postgresraster')

        self.assertTrue(rl.isValid())
        self.assertTrue(compareWkt(rl.extent().asWktPolygon(
        ), 'POLYGON((-40953 170588, -40873 170588, -40873 170668, -40953 170668, -40953 170588))', 1))
        block = rl.dataProvider().block(1, rl.extent(), 6, 6)
        data = []
        for i in range(6):
            for j in range(6):
                data.append(int(block.value(i, j)))

        self.assertEqual(data, [52, 52, 52, 52, 44, 43, 52, 52, 52, 48, 44, 44, 49, 52, 49, 44, 44, 44, 43, 47, 46, 44, 44, 44, 42, 42, 43, 44, 44, 48, 42, 43, 43, 44, 44, 47])

    def testUntiledMosaicNoMetadata(self):
        """Test regression https://github.com/qgis/QGIS/issues/39017

            +-----------+------------------------------+
            |           |                              |
            |  rid = 1  |          rid = 2             |
            |           |                              |
            +-----------+------------------------------+

        """

        rl = QgsRasterLayer(
            self.dbconn + " sslmode=disable table={table} schema={schema}".format(
                table='bug_39017_untiled_no_metadata', schema='public'), 'pg_layer', 'postgresraster')
        self.assertTrue(rl.isValid())
        self.assertTrue(compareWkt(rl.extent().asWktPolygon(
        ), 'POLYGON((47.061 40.976, 47.123 40.976, 47.123 41.000, 47.061 41.000, 47.061 40.976))', 0.01))

        rl1 = QgsRasterLayer(
            self.dbconn + " sslmode=disable table={table} schema={schema} sql=\"rid\"=1".format(
                table='bug_39017_untiled_no_metadata', schema='public'), 'pg_layer', 'postgresraster')
        self.assertTrue(rl1.isValid())
        self.assertTrue(compareWkt(rl1.extent().asWktPolygon(
        ), 'POLYGON((47.061 40.976, 47.070 40.976, 47.070 41.000, 47.061 41.000, 47.061 40.976))', 0.01))

        rl2 = QgsRasterLayer(
            self.dbconn + " sslmode=disable table={table} schema={schema} sql=\"rid\"=2".format(
                table='bug_39017_untiled_no_metadata', schema='public'), 'pg_layer', 'postgresraster')
        self.assertTrue(rl2.isValid())
        self.assertTrue(compareWkt(rl2.extent().asWktPolygon(
        ), 'POLYGON((47.061 40.976, 47.123 40.976, 47.123 41.000, 47.070 41.000, 47.070 40.976))', 0.01))

        extent_1 = rl1.extent()
        extent_2 = rl2.extent()

        def _6x6_block_data(layer, extent):
            block = layer.dataProvider().block(1, extent, 6, 6)
            data = []
            for i in range(6):
                for j in range(6):
                    data.append(int(block.value(i, j)))
            return data

        rl_r1 = _6x6_block_data(rl, extent_1)
        r1_r1 = _6x6_block_data(rl1, extent_1)
        self.assertEqual(rl_r1, r1_r1)

        rl_r2 = _6x6_block_data(rl, extent_2)
        r2_r2 = _6x6_block_data(rl2, extent_2)
        self.assertEqual(rl_r2, r2_r2)

    def testView(self):
        """Test issue GH #50841"""

        rl = QgsRasterLayer(
            self.dbconn + " key=\'rid\' srid=3035 sslmode=disable table={table} schema={schema}".format(
                table='raster_tiled_3035_view', schema='public'), 'pg_layer', 'postgresraster')

        self.assertTrue(rl.isValid())


if __name__ == '__main__':
    unittest.main()
