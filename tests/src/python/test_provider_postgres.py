# -*- coding: utf-8 -*-
"""QGIS Unit tests for the postgres provider.

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

__author__ = 'Matthias Kuhn'
__date__ = '2015-04-23'
__copyright__ = 'Copyright 2015, The QGIS Project'

import qgis  # NOQA
import psycopg2

import os
import time
from datetime import datetime

from qgis.core import (
    QgsVectorLayer,
    QgsVectorLayerExporter,
    QgsFeatureRequest,
    QgsFeature,
    QgsFieldConstraints,
    QgsDataProvider,
    NULL,
    QgsVectorLayerUtils,
    QgsSettings,
    QgsTransactionGroup,
    QgsReadWriteContext,
    QgsRectangle,
    QgsDefaultValue,
    QgsCoordinateReferenceSystem,
    QgsProject,
    QgsWkbTypes,
    QgsGeometry,
    QgsProviderRegistry,
    QgsVectorDataProvider,
    QgsDataSourceUri,
)
from qgis.gui import QgsGui, QgsAttributeForm
from qgis.PyQt.QtCore import QDate, QTime, QDateTime, QVariant, QDir, QObject, QByteArray
from qgis.PyQt.QtWidgets import QLabel
from qgis.testing import start_app, unittest
from qgis.PyQt.QtXml import QDomDocument
from utilities import unitTestDataPath, compareWkt
from providertestbase import ProviderTestCase

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsPostgresProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.dbconn = 'service=qgis_test'
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        # Create test layers
        cls.vl = QgsVectorLayer(
            cls.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=',
            'test', 'postgres')
        assert cls.vl.isValid()
        cls.source = cls.vl.dataProvider()
        cls.poly_vl = QgsVectorLayer(
            cls.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=',
            'test', 'postgres')
        assert cls.poly_vl.isValid()
        cls.poly_provider = cls.poly_vl.dataProvider()
        QgsGui.editorWidgetRegistry().initEditors()
        cls.con = psycopg2.connect(cls.dbconn)

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def execSQLCommand(self, sql):
        self.assertTrue(self.con)
        cur = self.con.cursor()
        self.assertTrue(cur)
        cur.execute(sql)
        cur.close()
        self.con.commit()

    def getSource(self):
        # create temporary table for edit tests
        self.execSQLCommand(
            'DROP TABLE IF EXISTS qgis_test."editData" CASCADE')
        self.execSQLCommand(
            'CREATE TABLE qgis_test."editData" ( pk SERIAL NOT NULL PRIMARY KEY, cnt integer, name text, name2 text, num_char text, dt timestamp without time zone, "date" date,  "time" time without time zone, geom public.geometry(Point, 4326))')
        self.execSQLCommand("INSERT INTO qgis_test.\"editData\" (pk, cnt, name, name2, num_char, dt, \"date\", \"time\", geom) VALUES "
                            "(5, -200, NULL, 'NuLl', '5', TIMESTAMP '2020-05-04 12:13:14', '2020-05-02', '12:13:01', '0101000020E61000001D5A643BDFC751C01F85EB51B88E5340'),"
                            "(3, 300, 'Pear', 'PEaR', '3', NULL, NULL, NULL, NULL),"
                            "(1, 100, 'Orange', 'oranGe', '1', TIMESTAMP '2020-05-03 12:13:14', '2020-05-03', '12:13:14', '0101000020E61000006891ED7C3F9551C085EB51B81E955040'),"
                            "(2, 200, 'Apple', 'Apple', '2', TIMESTAMP '2020-05-04 12:14:14', '2020-05-04', '12:14:14', '0101000020E6100000CDCCCCCCCC0C51C03333333333B35140'),"
                            "(4, 400, 'Honey', 'Honey', '4', TIMESTAMP '2021-05-04 13:13:14', '2021-05-04', '13:13:14', '0101000020E610000014AE47E17A5450C03333333333935340')")
        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."editData" (geom) sql=',
            'test', 'postgres')
        return vl

    def getEditableLayer(self):
        return self.getSource()

    def getEditableLayerWithCheckConstraint(self):
        """Returns the layer for attribute change CHECK constraint violation"""

        return QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'id\' srid=4326 type=POINT table="public"."test_check_constraint" (geom) sql=', 'test_check_constraint', 'postgres')

    def enableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', True)
        return True

    def disableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', False)

    def uncompiledFilters(self):
        return set(['"dt" = to_datetime(\'000www14ww13ww12www4ww5ww2020\',\'zzzwwwsswwmmwwhhwwwdwwMwwyyyy\')',
                    '"date" = to_date(\'www4ww5ww2020\',\'wwwdwwMwwyyyy\')',
                    '"time" = to_time(\'000www14ww13ww12www\',\'zzzwwwsswwmmwwhhwww\')'])

    def partiallyCompiledFilters(self):
        return set([])

    # HERE GO THE PROVIDER SPECIFIC TESTS
    def testDefaultValue(self):
        self.source.setProviderProperty(
            QgsDataProvider.EvaluateDefaultValues, True)
        self.assertIsInstance(self.source.defaultValue(0), int)
        self.assertEqual(self.source.defaultValue(1), NULL)
        self.assertEqual(self.source.defaultValue(2), 'qgis')
        self.source.setProviderProperty(
            QgsDataProvider.EvaluateDefaultValues, False)

    def testDefaultValueClause(self):
        self.source.setProviderProperty(
            QgsDataProvider.EvaluateDefaultValues, False)
        self.assertEqual(self.source.defaultValueClause(
            0), 'nextval(\'qgis_test."someData_pk_seq"\'::regclass)')
        self.assertFalse(self.source.defaultValueClause(1))
        self.assertEqual(self.source.defaultValueClause(2), '\'qgis\'::text')

    def testDateTimeTypes(self):
        vl = QgsVectorLayer('%s table="qgis_test"."date_times" sql=' % (
            self.dbconn), "testdatetimes", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName(
            'date_field')).type(), QVariant.Date)
        self.assertEqual(fields.at(fields.indexFromName(
            'time_field')).type(), QVariant.Time)
        self.assertEqual(fields.at(fields.indexFromName(
            'datetime_field')).type(), QVariant.DateTime)

        f = next(vl.getFeatures(QgsFeatureRequest()))

        date_idx = vl.fields().lookupField('date_field')
        self.assertIsInstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2004, 3, 4))
        time_idx = vl.fields().lookupField('time_field')
        self.assertIsInstance(f.attributes()[time_idx], QTime)
        self.assertEqual(f.attributes()[time_idx], QTime(13, 41, 52))
        datetime_idx = vl.fields().lookupField('datetime_field')
        self.assertIsInstance(f.attributes()[datetime_idx], QDateTime)
        self.assertEqual(f.attributes()[datetime_idx], QDateTime(
            QDate(2004, 3, 4), QTime(13, 41, 52)))

    def testBooleanType(self):
        vl = QgsVectorLayer('{} table="qgis_test"."boolean_table" sql='.format(
            self.dbconn), "testbool", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(
            fields.at(fields.indexFromName('fld1')).type(), QVariant.Bool)

        values = {feat['id']: feat['fld1'] for feat in vl.getFeatures()}
        expected = {
            1: True,
            2: False,
            3: NULL
        }
        self.assertEqual(values, expected)

    def testByteaType(self):
        vl = QgsVectorLayer('{} table="qgis_test"."byte_a_table" sql='.format(
            self.dbconn), "testbytea", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName(
            'fld1')).type(), QVariant.ByteArray)

        values = {feat['id']: feat['fld1'] for feat in vl.getFeatures()}
        expected = {
            1: QByteArray(b'YmludmFsdWU='),
            2: QByteArray()
        }
        self.assertEqual(values, expected)

        # editing binary values
        self.execSQLCommand(
            'DROP TABLE IF EXISTS qgis_test."byte_a_table_edit" CASCADE')
        self.execSQLCommand(
            'CREATE TABLE qgis_test."byte_a_table_edit" ( pk SERIAL NOT NULL PRIMARY KEY, blobby bytea)')
        self.execSQLCommand("INSERT INTO qgis_test.\"byte_a_table_edit\" (pk, blobby) VALUES "
                            "(1, encode('bbb', 'base64')::bytea)")
        vl = QgsVectorLayer(
            self.dbconn + ' sslmode=disable table="qgis_test"."byte_a_table_edit" sql=',
            'test', 'postgres')
        self.assertTrue(vl.isValid())
        values = {feat['pk']: feat['blobby'] for feat in vl.getFeatures()}
        expected = {
            1: QByteArray(b'YmJi')
        }
        self.assertEqual(values, expected)

        # change attribute value
        self.assertTrue(vl.dataProvider().changeAttributeValues(
            {1: {1: QByteArray(b'bbbvx')}}))
        values = {feat['pk']: feat['blobby'] for feat in vl.getFeatures()}
        expected = {
            1: QByteArray(b'bbbvx')
        }
        self.assertEqual(values, expected)

        # add feature
        f = QgsFeature()
        f.setAttributes([2, QByteArray(b'cccc')])
        self.assertTrue(vl.dataProvider().addFeature(f))
        values = {feat['pk']: feat['blobby'] for feat in vl.getFeatures()}
        expected = {
            1: QByteArray(b'bbbvx'),
            2: QByteArray(b'cccc')
        }
        self.assertEqual(values, expected)

        # change feature
        self.assertTrue(vl.dataProvider().changeFeatures(
            {2: {1: QByteArray(b'dddd')}}, {}))
        values = {feat['pk']: feat['blobby'] for feat in vl.getFeatures()}
        expected = {
            1: QByteArray(b'bbbvx'),
            2: QByteArray(b'dddd')
        }
        self.assertEqual(values, expected)

    def testCitextType(self):
        vl = QgsVectorLayer('{} table="qgis_test"."citext_table" sql='.format(
            self.dbconn), "testbytea", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(
            fields.at(fields.indexFromName('fld1')).type(), QVariant.String)

        values = {feat['id']: feat['fld1'] for feat in vl.getFeatures()}
        expected = {
            1: 'test val',
            2: NULL
        }
        self.assertEqual(values, expected)

        # editing citext values
        self.execSQLCommand(
            'DROP TABLE IF EXISTS qgis_test."citext_table_edit" CASCADE')
        self.execSQLCommand(
            'CREATE TABLE qgis_test."citext_table_edit" ( pk SERIAL NOT NULL PRIMARY KEY, txt citext)')
        self.execSQLCommand("INSERT INTO qgis_test.\"citext_table_edit\" (pk, txt) VALUES "
                            "(1, 'text')")
        vl = QgsVectorLayer(
            self.dbconn + ' sslmode=disable table="qgis_test"."citext_table_edit" sql=',
            'test', 'postgres')
        self.assertTrue(vl.isValid())
        values = {feat['pk']: feat['txt'] for feat in vl.getFeatures()}
        expected = {
            1: 'text'
        }
        self.assertEqual(values, expected)

        # change attribute value
        self.assertTrue(
            vl.dataProvider().changeAttributeValues({1: {1: 'teeeext'}}))
        values = {feat['pk']: feat['txt'] for feat in vl.getFeatures()}
        expected = {
            1: 'teeeext'
        }
        self.assertEqual(values, expected)

        # add feature
        f = QgsFeature()
        f.setAttributes([2, 'teeeeeeeeeext'])
        self.assertTrue(vl.dataProvider().addFeature(f))
        values = {feat['pk']: feat['txt'] for feat in vl.getFeatures()}
        expected = {
            1: 'teeeext',
            2: 'teeeeeeeeeext'
        }
        self.assertEqual(values, expected)

        # change feature
        self.assertTrue(vl.dataProvider().changeFeatures(
            {2: {1: 'teeeeeeeeeeeeeeeeeeeeeeext'}}, {}))
        values = {feat['pk']: feat['txt'] for feat in vl.getFeatures()}
        expected = {
            1: 'teeeext',
            2: 'teeeeeeeeeeeeeeeeeeeeeeext'
        }
        self.assertEqual(values, expected)

    def testQueryLayers(self):
        def test_query(dbconn, query, key):
            ql = QgsVectorLayer(
                '%s srid=4326 table="%s" (geom) key=\'%s\' sql=' % (
                    dbconn, query.replace('"', '\\"'), key), "testgeom",
                "postgres")
            self.assertTrue(ql.isValid(), '{} ({})'.format(query, key))

        test_query(self.dbconn,
                   '(SELECT NULL::integer "Id1", NULL::integer "Id2", NULL::geometry(Point, 4326) geom LIMIT 0)',
                   '"Id1","Id2"')

    def testWkbTypes(self):
        def test_table(dbconn, table_name, wkt):
            vl = QgsVectorLayer('%s srid=4326 table="qgis_test".%s (geom) sql=' % (dbconn, table_name), "testgeom",
                                "postgres")
            self.assertTrue(vl.isValid())
            for f in vl.getFeatures():
                self.assertEqual(f.geometry().asWkt(), wkt)

        test_table(self.dbconn, 'p2d', 'Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')
        test_table(self.dbconn, 'p3d',
                   'PolygonZ ((0 0 0, 1 0 0, 1 1 0, 0 1 0, 0 0 0))')
        test_table(self.dbconn, 'triangle2d', 'Polygon ((0 0, 1 0, 1 1, 0 0))')
        test_table(self.dbconn, 'triangle3d',
                   'PolygonZ ((0 0 0, 1 0 0, 1 1 0, 0 0 0))')
        test_table(self.dbconn, 'tin2d',
                   'MultiPolygon (((0 0, 1 0, 1 1, 0 0)),((0 0, 0 1, 1 1, 0 0)))')
        test_table(self.dbconn, 'tin3d',
                   'MultiPolygonZ (((0 0 0, 1 0 0, 1 1 0, 0 0 0)),((0 0 0, 0 1 0, 1 1 0, 0 0 0)))')
        test_table(self.dbconn, 'ps2d',
                   'MultiPolygon (((0 0, 1 0, 1 1, 0 1, 0 0)))')
        test_table(self.dbconn, 'ps3d',
                   'MultiPolygonZ (((0 0 0, 0 1 0, 1 1 0, 1 0 0, 0 0 0)),((0 0 1, 1 0 1, 1 1 1, 0 1 1, 0 0 1)),((0 0 0, 0 0 1, 0 1 1, 0 1 0, 0 0 0)),((0 1 0, 0 1 1, 1 1 1, 1 1 0, 0 1 0)),((1 1 0, 1 1 1, 1 0 1, 1 0 0, 1 1 0)),((1 0 0, 1 0 1, 0 0 1, 0 0 0, 1 0 0)))')
        test_table(self.dbconn, 'mp3d',
                   'MultiPolygonZ (((0 0 0, 0 1 0, 1 1 0, 1 0 0, 0 0 0)),((0 0 1, 1 0 1, 1 1 1, 0 1 1, 0 0 1)),((0 0 0, 0 0 1, 0 1 1, 0 1 0, 0 0 0)),((0 1 0, 0 1 1, 1 1 1, 1 1 0, 0 1 0)),((1 1 0, 1 1 1, 1 0 1, 1 0 0, 1 1 0)),((1 0 0, 1 0 1, 0 0 1, 0 0 0, 1 0 0)))')
        test_table(self.dbconn, 'pt2d', 'Point (0 0)')
        test_table(self.dbconn, 'pt3d', 'PointZ (0 0 0)')
        test_table(self.dbconn, 'ls2d', 'LineString (0 0, 1 1)')
        test_table(self.dbconn, 'ls3d', 'LineStringZ (0 0 0, 1 1 1)')
        test_table(self.dbconn, 'mpt2d', 'MultiPoint ((0 0),(1 1))')
        test_table(self.dbconn, 'mpt3d', 'MultiPointZ ((0 0 0),(1 1 1))')
        test_table(self.dbconn, 'mls2d',
                   'MultiLineString ((0 0, 1 1),(2 2, 3 3))')
        test_table(self.dbconn, 'mls3d',
                   'MultiLineStringZ ((0 0 0, 1 1 1),(2 2 2, 3 3 3))')

        test_table(self.dbconn, 'pt4d', 'PointZM (1 2 3 4)')

    def testMetadata(self):
        """ Test that metadata is correctly acquired from provider """
        metadata = self.vl.metadata()
        self.assertEqual(
            metadata.crs(), QgsCoordinateReferenceSystem.fromEpsgId(4326))
        self.assertEqual(metadata.type(), 'dataset')
        self.assertEqual(metadata.abstract(), 'QGIS Test Table')

    def testGetFeaturesUniqueId(self):
        """
        Test tables with inheritance for unique ids
        """

        def test_unique(features, num_features):
            featureids = []
            for f in features:
                self.assertFalse(f.id() in featureids)
                featureids.append(f.id())
            self.assertEqual(len(features), num_features)

        vl = QgsVectorLayer('%s srid=4326 table="qgis_test".%s (geom) sql=' % (self.dbconn, 'someData'), "testgeom",
                            "postgres")
        self.assertTrue(vl.isValid())
        # Test someData
        test_unique([f for f in vl.getFeatures()], 5)

        # Test base_table_bad: layer is invalid
        vl = QgsVectorLayer('%s srid=4326 table="qgis_test".%s (geom) sql=' % (self.dbconn, 'base_table_bad'),
                            "testgeom", "postgres")
        self.assertFalse(vl.isValid())
        # Test base_table_bad with use estimated metadata: layer is valid because the unique test is skipped
        vl = QgsVectorLayer(
            '%s srid=4326 estimatedmetadata="true" table="qgis_test".%s (geom) sql=' % (
                self.dbconn, 'base_table_bad'),
            "testgeom", "postgres")
        self.assertTrue(vl.isValid())

        # Test base_table_good: layer is valid
        vl = QgsVectorLayer('%s srid=4326 table="qgis_test".%s (geom) sql=' % (self.dbconn, 'base_table_good'),
                            "testgeom", "postgres")
        self.assertTrue(vl.isValid())
        test_unique([f for f in vl.getFeatures()], 4)
        # Test base_table_good with use estimated metadata: layer is valid
        vl = QgsVectorLayer(
            '%s srid=4326 estimatedmetadata="true" table="qgis_test".%s (geom) sql=' % (
                self.dbconn, 'base_table_good'),
            "testgeom", "postgres")
        self.assertTrue(vl.isValid())
        test_unique([f for f in vl.getFeatures()], 4)

    # See https://github.com/qgis/QGIS/issues/22258
    # TODO: accept multi-featured layers, and an array of values/fids
    def testSignedIdentifiers(self):

        def test_layer(ql, att, val, fidval):
            self.assertTrue(ql.isValid())
            features = ql.getFeatures()
            att_idx = ql.fields().lookupField(att)
            count = 0
            for f in features:
                count += 1
                self.assertEqual(f.attributes()[att_idx], val)
                self.assertEqual(f.id(), fidval)
            self.assertEqual(count, 1)

        def test(dbconn, query, att, val, fidval):
            table = query.replace('"', '\\"')
            uri = '%s table="%s" (g) key=\'%s\'' % (dbconn, table, att)
            ql = QgsVectorLayer(uri, "t", "postgres")
            test_layer(ql, att, val, fidval)
            # now with estimated metadata
            uri += ' estimatedmetadata="true"'
            test_layer(ql, att, val, fidval)

        # --- INT16 ----
        # zero
        test(self.dbconn, '(SELECT 0::int2 i, NULL::geometry(Point) g)', 'i', 0, 0)
        # low positive
        test(self.dbconn, '(SELECT 1::int2 i, NULL::geometry(Point) g)', 'i', 1, 1)
        # low negative
        test(self.dbconn, '(SELECT -1::int2 i, NULL::geometry(Point) g)',
             'i', -1, 4294967295)
        # max positive signed 16bit integer
        test(self.dbconn, '(SELECT 32767::int2 i, NULL::geometry(Point) g)',
             'i', 32767, 32767)
        # max negative signed 16bit integer
        test(self.dbconn, '(SELECT (-32768)::int2 i, NULL::geometry(Point) g)',
             'i', -32768, 4294934528)

        # --- INT32 ----
        # zero
        test(self.dbconn, '(SELECT 0::int4 i, NULL::geometry(Point) g)', 'i', 0, 0)
        # low positive
        test(self.dbconn, '(SELECT 2::int4 i, NULL::geometry(Point) g)', 'i', 2, 2)
        # low negative
        test(self.dbconn, '(SELECT -2::int4 i, NULL::geometry(Point) g)',
             'i', -2, 4294967294)
        # max positive signed 32bit integer
        test(self.dbconn, '(SELECT 2147483647::int4 i, NULL::geometry(Point) g)',
             'i', 2147483647, 2147483647)
        # max negative signed 32bit integer
        test(self.dbconn, '(SELECT (-2147483648)::int4 i, NULL::geometry(Point) g)',
             'i', -2147483648, 2147483648)

        # --- INT64 (FIDs are always 1 because assigned ex-novo) ----
        # zero
        test(self.dbconn, '(SELECT 0::int8 i, NULL::geometry(Point) g)', 'i', 0, 1)
        # low positive
        test(self.dbconn, '(SELECT 3::int8 i, NULL::geometry(Point) g)', 'i', 3, 1)
        # low negative
        test(self.dbconn, '(SELECT -3::int8 i, NULL::geometry(Point) g)', 'i', -3, 1)
        # max positive signed 64bit integer
        test(self.dbconn, '(SELECT 9223372036854775807::int8 i, NULL::geometry(Point) g)',
             'i', 9223372036854775807, 1)
        # max negative signed 32bit integer
        test(self.dbconn, '(SELECT (-9223372036854775808)::int8 i, NULL::geometry(Point) g)', 'i', -9223372036854775808,
             1)

    def testPktIntInsert(self):
        vl = QgsVectorLayer('{} table="qgis_test"."{}" key="pk" sql='.format(self.dbconn, 'bikes_view'), "bikes_view",
                            "postgres")
        self.assertTrue(vl.isValid())
        f = QgsFeature(vl.fields())
        f['pk'] = NULL
        f['name'] = 'Cilo'
        r, f = vl.dataProvider().addFeatures([f])
        self.assertTrue(r)
        self.assertNotEqual(f[0]['pk'], NULL, f[0].attributes())
        vl.deleteFeatures([f[0].id()])

    def testGeneratedFields(self):
        """Test if GENERATED geometry/geography columns are correctly handled by the provider."""
        cur = self.con.cursor()
        cur.execute("SHOW server_version_num")
        pgversion = int(cur.fetchone()[0])

        # GENERATED columns are unsupported by PostgreSQL versions earlier than 12.
        if pgversion < 120000:
            return

        # Geometry columns
        vl = QgsVectorLayer('{} table="qgis_test"."{}" (geom) srid=4326 type=POLYGON key="id" sql='.format(self.dbconn, "test_gen_col"), "test_gen_col", "postgres")
        self.assertTrue(vl.isValid())

        # writing geometry...
        f = QgsFeature(vl.fields())

        ix_name = f.fieldNameIndex('name')

        f.setGeometry(QgsGeometry.fromWkt('Polygon ((-67 -2, -67 0, -68 0, -70 -1, -67 -2))'))
        f.setAttribute(ix_name, 'QGIS-3')

        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.addFeatures([f]))
        self.assertTrue(vl.commitChanges())

        # reading back to see if we saved the centroid correctly.
        vl2 = QgsVectorLayer('{} table="qgis_test"."{}" (cent) srid=4326 type=POINT key="id" sql='.format(self.dbconn, "test_gen_col"), "test_gen_col", "postgres")
        f2 = next(vl2.getFeatures(QgsFeatureRequest()))
        generated_geometry = f2.geometry().asWkt()
        expected_geometry = 'Point (-68.047619047619051 -0.90476190476190477)'
        expected_area = 43069568296.34387

        assert compareWkt(generated_geometry, expected_geometry), "Geometry mismatch! Expected:\n{}\nGot:\n{}\n".format(expected_geometry, generated_geometry)
        self.assertEqual(f2['poly_area'], expected_area)
        self.assertEqual(f2['name'], 'QGIS-3')

        # Checking if we can correctly change values of an existing feature.
        self.assertTrue(vl2.startEditing())
        ix2_name = f2.fieldNameIndex('name')
        fid2 = f2.id()
        vl2.changeAttributeValue(fid2, ix2_name, 'New')
        self.assertTrue(vl2.commitChanges())

        # getting a brand new QgsVectorLayer
        vl = QgsVectorLayer('{} table="qgis_test"."{}" (geom) srid=4326 type=POLYGON key="id" sql='.format(self.dbconn, "test_gen_col"), "test_gen_col", "postgres")
        self.assertTrue(vl.isValid())

        # checking if the name field was correctly updated
        f = next(vl.getFeatures(QgsFeatureRequest()))
        self.assertEqual(f['name'], 'New')

        # Now, check if we can change the value of a GENERATED field (we shouldn't)
        self.assertTrue(vl.startEditing())
        ix_area = f.fieldNameIndex('poly_area')
        fid = f.id()
        vl.changeAttributeValue(fid, ix_area, 42)
        self.assertTrue(vl.commitChanges())

        # reading back
        vl2 = QgsVectorLayer('{} table="qgis_test"."{}" (geom) srid=4326 type=POLYGON key="id" sql='.format(self.dbconn, "test_gen_col"), "test_gen_col", "postgres")
        f2 = next(vl2.getFeatures(QgsFeatureRequest()))
        self.assertEqual(f2['poly_area'], expected_area)

        # now, getting a brand new QgsVectorLayer to check if changes (UPDATE) in the geometry are reflected in the generated fields
        vl = QgsVectorLayer('{} table="qgis_test"."{}" (geom) srid=4326 type=POLYGON key="id" sql='.format(self.dbconn, "test_gen_col"), "test_gen_col", "postgres")
        self.assertTrue(vl.isValid())
        f = next(vl.getFeatures(QgsFeatureRequest()))
        vl.startEditing()
        fid = f.id()
        vl.changeGeometry(fid, QgsGeometry.fromWkt('Polygon ((-67 -2, -65 0, -68 0, -70 -1, -67 -2))'))
        vl.commitChanges()

        # reading back...
        vl2 = QgsVectorLayer('{} table="qgis_test"."{}" (cent) srid=4326 type=POINT key="id" sql='.format(self.dbconn, "test_gen_col"), "test_gen_col", "postgres")
        f2 = next(vl2.getFeatures(QgsFeatureRequest()))
        generated_geometry = f2.geometry().asWkt()

        generated_geometry = f2.geometry().asWkt()
        expected_geometry = 'Point (-67.42424242424242209 -0.81818181818181823)'
        expected_area = 67718478405.28429

        assert compareWkt(generated_geometry, expected_geometry), "Geometry mismatch! Expected:\n{}\nGot:\n{}\n".format(expected_geometry, generated_geometry)
        self.assertEqual(f2['poly_area'], expected_area)
        self.assertEqual(f2['name'], 'New')

        # Geography columns
        vl3 = QgsVectorLayer('{} table="qgis_test"."{}" (geog) srid=4326 type=POLYGON key="id" sql='.format(self.dbconn, "test_gen_geog_col"), "test_gen_geog_col", "postgres")
        self.assertTrue(vl3.isValid())

        # writing geography...
        f3 = QgsFeature(vl3.fields())
        f3.setGeometry(QgsGeometry.fromWkt('Polygon ((-67 -2, -67 0, -68 0, -70 -1, -67 -2))'))
        self.assertTrue(vl3.startEditing())
        self.assertTrue(vl3.addFeatures([f3]))
        self.assertTrue(vl3.commitChanges())

        # reading back geography and checking values
        vl4 = QgsVectorLayer('{} table="qgis_test"."{}" (cent) srid=4326 type=POINT key="id" sql='.format(self.dbconn, "test_gen_geog_col"), "test_gen_geog_col", "postgres")
        f4 = next(vl4.getFeatures(QgsFeatureRequest()))
        generated_geometry = f4.geometry().asWkt()
        expected_geometry = 'Point (-68.0477406158202 -0.904960604589168)'
        expected_area = 43088884296.69713

        assert compareWkt(generated_geometry, expected_geometry), "Geometry mismatch! Expected:\n{}\nGot:\n{}\n".format(expected_geometry, generated_geometry)
        self.assertEqual(f4['poly_area'], expected_area)

    def testNonPkBigintField(self):
        """Test if we can correctly insert, read and change attributes(fields) of type bigint and which are not PKs."""
        vl = QgsVectorLayer(
            '{} sslmode=disable srid=4326 key="pk" table="qgis_test".{} (geom)'.format(
                self.dbconn, 'bigint_pk'),
            "bigint_pk", "postgres")
        self.assertTrue(vl.isValid())
        flds = vl.fields()

        # check if default values are correctly read back
        f = next(vl.getFeatures(QgsFeatureRequest()))
        bigint_with_default_idx = vl.fields().lookupField('bigint_attribute_def')
        self.assertEqual(f.attributes()[bigint_with_default_idx], 42)

        # check if NULL values are correctly read
        bigint_def_null_idx = vl.fields().lookupField('bigint_attribute')
        self.assertEqual(f.attributes()[bigint_def_null_idx], NULL)

        # check if we can overwrite a default value
        vl.startEditing()
        vl.changeAttributeValue(f.id(), bigint_with_default_idx, 43)

        pkidx = vl.fields().lookupField('pk')
        editedid = f.attributes()[pkidx]

        self.assertTrue(vl.commitChanges())
        vl2 = QgsVectorLayer(
            '{} sslmode=disable srid=4326 key="pk" table="qgis_test".{} (geom)'.format(
                self.dbconn, 'bigint_pk'),
            "bigint_pk", "postgres")
        flds = vl2.fields()
        self.assertTrue(vl2.isValid())
        f = next(vl2.getFeatures(
            QgsFeatureRequest().setFilterExpression('pk = ' + str(editedid))))
        bigint_with_default_idx = vl2.fields().lookupField('bigint_attribute_def')
        self.assertEqual(f.attributes()[bigint_with_default_idx], 43)

        # check if we can insert a new value
        dp = vl2.dataProvider()
        dp.setProviderProperty(QgsDataProvider.EvaluateDefaultValues, 1)
        pkidx = vl2.fields().lookupField('pk')
        vl2.startEditing()
        f = QgsFeature(vl2.fields())
        f['pk'] = NULL
        f['value'] = 'The answer.'
        f['bigint_attribute'] = 84
        f.setAttribute(pkidx, vl2.dataProvider().defaultValue(pkidx))
        f.setAttribute(bigint_with_default_idx,
                       vl2.dataProvider().defaultValue(bigint_with_default_idx))
        r, f = vl2.dataProvider().addFeatures([f])
        self.assertTrue(r)
        vl2.commitChanges()
        inserted_id = f[0]['pk']

        f = next(vl2.getFeatures(
            QgsFeatureRequest().setFilterExpression('pk = ' + str(inserted_id))))

        self.assertEqual(f['bigint_attribute'], 84)
        self.assertEqual(f['bigint_attribute_def'], 42)

    def testPktUpdateBigintPk(self):
        """Test if we can update objects with positive, zero and negative bigint PKs."""
        vl = QgsVectorLayer(
            '{} sslmode=disable srid=4326 key="pk" table="qgis_test".{} (geom)'.format(
                self.dbconn, 'bigint_pk'),
            "bigint_pk", "postgres")
        flds = vl.fields()

        self.assertTrue(vl.isValid())

        vl.startEditing()

        statuses = [-1, -1, -1, -1]
        # changing values...
        for ft in vl.getFeatures():
            if ft['value'] == 'first value':
                vl.changeAttributeValue(
                    ft.id(), flds.indexOf('value'), '1st value')
                statuses[0] = 0
            elif ft['value'] == 'second value':
                vl.changeAttributeValue(
                    ft.id(), flds.indexOf('value'), '2nd value')
                statuses[1] = 0
            elif ft['value'] == 'zero value':
                vl.changeAttributeValue(
                    ft.id(), flds.indexOf('value'), '0th value')
                statuses[2] = 0
            elif ft['value'] == 'negative value':
                vl.changeAttributeValue(
                    ft.id(), flds.indexOf('value'), '-1th value')
                statuses[3] = 0
        self.assertTrue(vl.commitChanges())
        self.assertTrue(all(x == 0 for x in statuses))

        # now, let's see if the values were changed
        vl2 = QgsVectorLayer(
            '{} sslmode=disable srid=4326 key="pk" table="qgis_test".{} (geom)'.format(
                self.dbconn, 'bigint_pk'),
            "bigint_pk", "postgres")
        self.assertTrue(vl2.isValid())
        for ft in vl2.getFeatures():
            if ft['value'] == '1st value':
                statuses[0] = 1
            elif ft['value'] == '2nd value':
                statuses[1] = 1
            elif ft['value'] == '0th value':
                statuses[2] = 1
            elif ft['value'] == '-1th value':
                statuses[3] = 1
        self.assertTrue(all(x == 1 for x in statuses))

    def testPktUpdateBigintPkNonFirst(self):
        """Test if we can update objects with positive, zero and negative bigint PKs in tables whose PK is not the first field"""
        vl = QgsVectorLayer('{} sslmode=disable srid=4326 key="pk" table="qgis_test".{} (geom)'.format(self.dbconn,
                                                                                                       'bigint_non_first_pk'),
                            "bigint_non_first_pk", "postgres")
        flds = vl.fields()

        self.assertTrue(vl.isValid())

        vl.startEditing()

        statuses = [-1, -1, -1, -1]
        # changing values...
        for ft in vl.getFeatures():
            if ft['value'] == 'first value':
                vl.changeAttributeValue(
                    ft.id(), flds.indexOf('value'), '1st value')
                statuses[0] = 0
            elif ft['value'] == 'second value':
                vl.changeAttributeValue(
                    ft.id(), flds.indexOf('value'), '2nd value')
                statuses[1] = 0
            elif ft['value'] == 'zero value':
                vl.changeAttributeValue(
                    ft.id(), flds.indexOf('value'), '0th value')
                statuses[2] = 0
            elif ft['value'] == 'negative value':
                vl.changeAttributeValue(
                    ft.id(), flds.indexOf('value'), '-1th value')
                statuses[3] = 0
        self.assertTrue(vl.commitChanges())
        self.assertTrue(all(x == 0 for x in statuses))

        # now, let's see if the values were changed
        vl2 = QgsVectorLayer(
            '{} sslmode=disable srid=4326 key="pk" table="qgis_test".{} (geom)'.format(
                self.dbconn, 'bigint_pk'),
            "bigint_pk_nonfirst", "postgres")
        self.assertTrue(vl2.isValid())
        for ft in vl2.getFeatures():
            if ft['value'] == '1st value':
                statuses[0] = 1
            elif ft['value'] == '2nd value':
                statuses[1] = 1
            elif ft['value'] == '0th value':
                statuses[2] = 1
            elif ft['value'] == '-1th value':
                statuses[3] = 1
        self.assertTrue(all(x == 1 for x in statuses))

    def testPktComposite(self):
        """
        Check that tables with PKs composed of many fields of different types are correctly read and written to
        """
        vl = QgsVectorLayer('{} sslmode=disable srid=4326 key=\'"pk1","pk2"\' table="qgis_test"."tb_test_compound_pk" (geom)'.format(self.dbconn), "test_compound", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.fields()

        f = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk1 = 1 AND pk2 = 2')))
        # first of all: we must be able to fetch a valid feature
        self.assertTrue(f.isValid())
        self.assertEqual(f['pk1'], 1)
        self.assertEqual(f['pk2'], 2)
        self.assertEqual(f['value'], 'test 2')

        # can we edit a field?
        vl.startEditing()
        vl.changeAttributeValue(f.id(), fields.indexOf('value'), 'Edited Test 2')
        self.assertTrue(vl.commitChanges())

        # Did we get it right? Let's create a new QgsVectorLayer and try to read back our changes:
        vl2 = QgsVectorLayer('{} sslmode=disable srid=4326 table="qgis_test"."tb_test_compound_pk" (geom) key=\'"pk1","pk2"\' '.format(self.dbconn), "test_compound2", "postgres")
        self.assertTrue(vl2.isValid())
        f2 = next(vl2.getFeatures(QgsFeatureRequest().setFilterExpression('pk1 = 1 AND pk2 = 2')))
        self.assertTrue(f2.isValid())

        # Then, making sure we really did change our value.
        self.assertEqual(f2['value'], 'Edited Test 2')

        # How about inserting a new field?
        f3 = QgsFeature(vl2.fields())
        f3['pk1'] = 4
        f3['pk2'] = -9223372036854775800
        f3['value'] = 'other test'
        vl.startEditing()
        res, f3 = vl.dataProvider().addFeatures([f3])
        self.assertTrue(res)
        self.assertTrue(vl.commitChanges())

        # can we catch it on another layer?
        f4 = next(vl2.getFeatures(QgsFeatureRequest().setFilterExpression('pk2 = -9223372036854775800')))

        self.assertTrue(f4.isValid())
        expected_attrs = [4, -9223372036854775800, 'other test']
        self.assertEqual(f4.attributes(), expected_attrs)

        # Finally, let's delete one of the features.
        f5 = next(vl2.getFeatures(QgsFeatureRequest().setFilterExpression('pk1 = 2 AND pk2 = 1')))
        vl2.startEditing()
        vl2.deleteFeatures([f5.id()])
        self.assertTrue(vl2.commitChanges())

        # did we really delete? Let's try to get the deleted feature from the first layer.
        f_iterator = vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk1 = 2 AND pk2 = 1'))
        got_feature = True

        try:
            f6 = next(f_iterator)
            got_feature = f6.isValid()
        except StopIteration:
            got_feature = False

        self.assertFalse(got_feature)

    def testPktMapInsert(self):
        vl = QgsVectorLayer('{} table="qgis_test"."{}" key="obj_id" sql='.format(self.dbconn, 'oid_serial_table'),
                            "oid_serial", "postgres")
        self.assertTrue(vl.isValid())
        f = QgsFeature(vl.fields())
        f['obj_id'] = vl.dataProvider().defaultValueClause(0)
        f['name'] = 'Test'
        r, f = vl.dataProvider().addFeatures([f])
        self.assertTrue(r)
        self.assertNotEqual(f[0]['obj_id'], NULL, f[0].attributes())
        vl.deleteFeatures([f[0].id()])

    def testNull(self):
        """
        Asserts that 0, '' and NULL are treated as different values on insert
        """
        vl = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'gid\' table="qgis_test"."constraints" sql=', 'test1',
                            'postgres')
        self.assertTrue(vl.isValid())
        QgsProject.instance().addMapLayer(vl)
        tg = QgsTransactionGroup()
        tg.addLayer(vl)
        vl.startEditing()

        def onError(message):
            """We should not get here. If we do, fail and say why"""
            self.assertFalse(True, message)

        vl.raiseError.connect(onError)

        f = QgsFeature(vl.fields())
        f['gid'] = 100
        f['val'] = 0
        f['name'] = ''
        self.assertTrue(vl.addFeature(f))
        feature = next(vl.getFeatures('"gid" = 100'))
        self.assertEqual(f['val'], feature['val'])
        self.assertEqual(f['name'], feature['name'])

    def testNestedInsert(self):
        tg = QgsTransactionGroup()
        tg.addLayer(self.vl)
        self.vl.startEditing()
        it = self.vl.getFeatures()
        f = next(it)
        f['pk'] = NULL
        self.vl.addFeature(f)  # Should not deadlock during an active iteration
        f = next(it)

    def testTimeout(self):
        """
        Asserts that we will not deadlock if more iterators are opened in parallel than
        available in the connection pool
        """
        request = QgsFeatureRequest()
        request.setTimeout(1)

        iterators = list()
        for i in range(100):
            iterators.append(self.vl.getFeatures(request))

    def testTransactionDirtyName(self):
        # create a vector ayer based on postgres
        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=',
            'test', 'postgres')
        self.assertTrue(vl.isValid())

        # prepare a project with transactions enabled
        p = QgsProject()
        p.setAutoTransaction(True)
        p.addMapLayers([vl])
        vl.startEditing()

        # update the data within the transaction
        tr = vl.dataProvider().transaction()
        sql = "update qgis_test.some_poly_data set pk=1 where pk=1"
        name = "My Awesome Transaction!"
        self.assertTrue(tr.executeSql(sql, True, name)[0])

        # test name
        self.assertEqual(vl.undoStack().command(0).text(), name)

        # rollback
        vl.rollBack()

    def testTransactionDirty(self):
        # create a vector layer based on postgres
        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=',
            'test', 'postgres')
        self.assertTrue(vl.isValid())

        # prepare a project with transactions enabled
        p = QgsProject()
        p.setAutoTransaction(True)
        p.addMapLayers([vl])
        vl.startEditing()

        # check that the feature used for testing is ok
        ft0 = vl.getFeatures('pk=1')
        f = QgsFeature()
        self.assertTrue(ft0.nextFeature(f))

        # update the data within the transaction
        tr = vl.dataProvider().transaction()
        sql = "update qgis_test.some_poly_data set pk=33 where pk=1"
        self.assertTrue(tr.executeSql(sql, True)[0])

        # check that the pk of the feature has been changed
        ft = vl.getFeatures('pk=1')
        self.assertFalse(ft.nextFeature(f))

        ft = vl.getFeatures('pk=33')
        self.assertTrue(ft.nextFeature(f))

        # underlying data has been modified but the layer is not tagged as
        # modified
        self.assertTrue(vl.isModified())

        # undo sql query
        vl.undoStack().undo()

        # check that the original feature with pk is back
        ft0 = vl.getFeatures('pk=1')
        self.assertTrue(ft0.nextFeature(f))

        # redo
        vl.undoStack().redo()

        # check that the pk of the feature has been changed
        ft1 = vl.getFeatures('pk=1')
        self.assertFalse(ft1.nextFeature(f))

    def testTransactionConstraints(self):
        # create a vector layer based on postgres
        vl = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'id\' table="qgis_test"."check_constraints" sql=',
                            'test', 'postgres')
        self.assertTrue(vl.isValid())

        # prepare a project with transactions enabled
        p = QgsProject()
        p.setAutoTransaction(True)
        p.addMapLayers([vl])

        # get feature
        f = QgsFeature()
        self.assertTrue(vl.getFeatures('id=1').nextFeature(f))
        self.assertEqual(f.attributes(), [1, 4, 3])

        # start edition
        vl.startEditing()

        # update attribute form with a failing constraints
        # coming from the database if attributes are updated
        # one at a time.
        # Current feature: a = 4 / b = 3
        # Update feature: a = 1 / b = 0
        # If updated one at a time, '(a = 1) < (b = 3)' => FAIL!
        form = QgsAttributeForm(vl, f)
        for w in form.findChildren(QLabel):
            if w.buddy():
                spinBox = w.buddy()
                if w.text() == 'a':
                    spinBox.setValue(1)
                elif w.text() == 'b':
                    spinBox.setValue(0)

        # save
        form.save()

        # check new values
        self.assertTrue(vl.getFeatures('id=1').nextFeature(f))
        self.assertEqual(f.attributes(), [1, 1, 0])

    def testTransactionTuple(self):
        # create a vector layer based on postgres
        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=',
            'test', 'postgres')
        self.assertTrue(vl.isValid())

        # prepare a project with transactions enabled
        p = QgsProject()
        p.setAutoTransaction(True)
        p.addMapLayers([vl])
        vl.startEditing()

        # execute a query which returns a tuple
        tr = vl.dataProvider().transaction()
        sql = "select * from qgis_test.some_poly_data"
        self.assertTrue(tr.executeSql(sql, False)[0])

        # underlying data has not been modified
        self.assertFalse(vl.isModified())

    def testDomainTypes(self):
        """Test that domain types are correctly mapped"""

        vl = QgsVectorLayer('%s table="qgis_test"."domains" sql=' %
                            (self.dbconn), "domains", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()

        expected = {}
        expected['fld_var_char_domain'] = {'type': QVariant.String, 'typeName': 'qgis_test.var_char_domain',
                                           'length': -1}
        expected['fld_var_char_domain_6'] = {'type': QVariant.String, 'typeName': 'qgis_test.var_char_domain_6',
                                             'length': 6}
        expected['fld_character_domain'] = {'type': QVariant.String, 'typeName': 'qgis_test.character_domain',
                                            'length': 1}
        expected['fld_character_domain_6'] = {'type': QVariant.String, 'typeName': 'qgis_test.character_domain_6',
                                              'length': 6}
        expected['fld_char_domain'] = {
            'type': QVariant.String, 'typeName': 'qgis_test.char_domain', 'length': 1}
        expected['fld_char_domain_6'] = {
            'type': QVariant.String, 'typeName': 'qgis_test.char_domain_6', 'length': 6}
        expected['fld_text_domain'] = {
            'type': QVariant.String, 'typeName': 'qgis_test.text_domain', 'length': -1}
        expected['fld_numeric_domain'] = {'type': QVariant.Double, 'typeName': 'qgis_test.numeric_domain', 'length': 10,
                                          'precision': 4}

        for f, e in list(expected.items()):
            self.assertEqual(
                fields.at(fields.indexFromName(f)).type(), e['type'])
            self.assertEqual(fields.at(fields.indexFromName(f)
                                       ).typeName(), e['typeName'])
            self.assertEqual(
                fields.at(fields.indexFromName(f)).length(), e['length'])
            if 'precision' in e:
                self.assertEqual(
                    fields.at(fields.indexFromName(f)).precision(), e['precision'])

    def testRenameAttributes(self):
        ''' Test renameAttributes() '''
        vl = QgsVectorLayer('%s table="qgis_test"."rename_table" sql=' % (
            self.dbconn), "renames", "postgres")
        provider = vl.dataProvider()
        provider.renameAttributes({1: 'field1', 2: 'field2'})

        # bad rename
        self.assertFalse(provider.renameAttributes({-1: 'not_a_field'}))
        self.assertFalse(provider.renameAttributes({100: 'not_a_field'}))
        # already exists
        self.assertFalse(provider.renameAttributes({1: 'field2'}))

        # rename one field
        self.assertTrue(provider.renameAttributes({1: 'newname'}))
        self.assertEqual(provider.fields().at(1).name(), 'newname')
        vl.updateFields()
        fet = next(vl.getFeatures())
        self.assertEqual(fet.fields()[1].name(), 'newname')

        # rename two fields
        self.assertTrue(provider.renameAttributes(
            {1: 'newname2', 2: 'another'}))
        self.assertEqual(provider.fields().at(1).name(), 'newname2')
        self.assertEqual(provider.fields().at(2).name(), 'another')
        vl.updateFields()
        fet = next(vl.getFeatures())
        self.assertEqual(fet.fields()[1].name(), 'newname2')
        self.assertEqual(fet.fields()[2].name(), 'another')

        # close layer and reopen, then recheck to confirm that changes were saved to db
        del vl
        vl = None
        vl = QgsVectorLayer('%s table="qgis_test"."rename_table" sql=' % (
            self.dbconn), "renames", "postgres")
        provider = vl.dataProvider()
        self.assertEqual(provider.fields().at(1).name(), 'newname2')
        self.assertEqual(provider.fields().at(2).name(), 'another')
        fet = next(vl.getFeatures())
        self.assertEqual(fet.fields()[1].name(), 'newname2')
        self.assertEqual(fet.fields()[2].name(), 'another')

    def testEditorWidgetTypes(self):
        """Test that editor widget types can be fetched from the qgis_editor_widget_styles table"""

        vl = QgsVectorLayer('%s table="qgis_test"."widget_styles" sql=' % (
            self.dbconn), "widget_styles", "postgres")
        self.assertTrue(vl.isValid())
        fields = vl.dataProvider().fields()

        setup1 = fields.field("fld1").editorWidgetSetup()
        self.assertFalse(setup1.isNull())
        self.assertEqual(setup1.type(), "FooEdit")
        self.assertEqual(setup1.config(), {"param1": "value1", "param2": "2"})

        best1 = QgsGui.editorWidgetRegistry().findBest(vl, "fld1")
        self.assertEqual(best1.type(), "FooEdit")
        self.assertEqual(best1.config(), setup1.config())

        self.assertTrue(fields.field("fld2").editorWidgetSetup().isNull())

        best2 = QgsGui.editorWidgetRegistry().findBest(vl, "fld2")
        self.assertEqual(best2.type(), "TextEdit")

    def testHstore(self):
        vl = QgsVectorLayer('%s table="qgis_test"."dict" sql=' %
                            (self.dbconn), "testhstore", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(
            fields.at(fields.indexFromName('value')).type(), QVariant.Map)

        f = next(vl.getFeatures(QgsFeatureRequest()))

        value_idx = vl.fields().lookupField('value')
        self.assertIsInstance(f.attributes()[value_idx], dict)
        self.assertEqual(f.attributes()[value_idx], {'a': 'b', '1': '2'})

        new_f = QgsFeature(vl.fields())
        new_f['pk'] = NULL
        new_f['value'] = {'simple': '1', 'doubleQuote': '"y"',
                          'quote': "'q'", 'backslash': '\\'}
        r, fs = vl.dataProvider().addFeatures([new_f])
        self.assertTrue(r)
        new_pk = fs[0]['pk']
        self.assertNotEqual(new_pk, NULL, fs[0].attributes())

        try:
            read_back = vl.getFeature(new_pk)
            self.assertEqual(read_back['pk'], new_pk)
            self.assertEqual(read_back['value'], new_f['value'])
        finally:
            self.assertTrue(vl.startEditing())
            self.assertTrue(vl.deleteFeatures([new_pk]))
            self.assertTrue(vl.commitChanges())

    def testJson(self):
        vl = QgsVectorLayer('%s table="qgis_test"."json" sql=' %
                            (self.dbconn), "testjson", "postgres")
        self.assertTrue(vl.isValid())

        attrs = (
            123,
            1233.45,
            None,
            True,
            False,
            r"String literal with \"quotes\" 'and' other funny chars []{};#/èé*",
            [1, 2, 3.4, None],
            [True, False],
            {'a': 123, 'b': 123.34, 'c': 'a string', 'd': [
                1, 2, 3], 'e': {'a': 123, 'b': 123.45}}
        )
        attrs2 = (
            246,
            2466.91,
            None,
            True,
            False,
            r"Yet another string literal with \"quotes\" 'and' other funny chars: π []{};#/èé*",
            [2, 4, 3.14159, None],
            [True, False],
            {'a': 246, 'b': 246.68, 'c': 'a rounded area: π × r²', 'd': [
                1, 2, 3], 'e': {'a': 246, 'b': 246.91}}
        )
        json_idx = vl.fields().lookupField('jvalue')
        jsonb_idx = vl.fields().lookupField('jbvalue')

        for attr in attrs:
            # Add a new feature
            vl2 = QgsVectorLayer('%s table="qgis_test"."json" sql=' % (
                self.dbconn), "testjson", "postgres")
            self.assertTrue(vl2.startEditing())
            f = QgsFeature(vl2.fields())
            f.setAttributes([None, attr, attr])
            self.assertTrue(vl2.addFeatures([f]))
            self.assertTrue(vl2.commitChanges(), attr)
            # Read back
            vl2 = QgsVectorLayer('%s table="qgis_test"."json" sql=' % (
                self.dbconn), "testjson", "postgres")
            fid = [f.id() for f in vl2.getFeatures()][-1]
            f = vl2.getFeature(fid)
            self.assertEqual(f.attributes(), [fid, attr, attr])
            # Change attribute values
            vl2 = QgsVectorLayer('%s table="qgis_test"."json" sql=' % (
                self.dbconn), "testjson", "postgres")
            fid = [f.id() for f in vl2.getFeatures()][-1]
            self.assertTrue(vl2.startEditing())
            self.assertTrue(vl2.changeAttributeValues(
                fid, {json_idx: attr, jsonb_idx: attr}))
            self.assertTrue(vl2.commitChanges())
            # Read back
            vl2 = QgsVectorLayer('%s table="qgis_test"."json" sql=' % (
                self.dbconn), "testjson", "postgres")
            f = vl2.getFeature(fid)
            self.assertEqual(f.attributes(), [fid, attr, attr])

        # Let's check changeFeatures:
        for attr in attrs2:
            vl2 = QgsVectorLayer('%s table="qgis_test"."json" sql=' % (
                self.dbconn), "testjson", "postgres")
            fid = [f.id() for f in vl2.getFeatures()][-1]
            self.assertTrue(vl2.startEditing())
            self.assertTrue(vl2.dataProvider().changeFeatures({fid: {json_idx: attr, jsonb_idx: attr}}, {}))
            self.assertTrue(vl2.commitChanges())

            # Read back again
            vl2 = QgsVectorLayer('%s table="qgis_test"."json" sql=' % (
                self.dbconn), "testjson", "postgres")
            f = vl2.getFeature(fid)
            self.assertEqual(f.attributes(), [fid, attr, attr])

    def testStringArray(self):
        vl = QgsVectorLayer('%s table="qgis_test"."string_array" sql=' % (
            self.dbconn), "teststringarray", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName(
            'value')).type(), QVariant.StringList)
        self.assertEqual(fields.at(fields.indexFromName(
            'value')).subType(), QVariant.String)

        f = next(vl.getFeatures(QgsFeatureRequest()))

        value_idx = vl.fields().lookupField('value')
        self.assertIsInstance(f.attributes()[value_idx], list)
        self.assertEqual(f.attributes()[value_idx], ['a', 'b', 'c'])

        new_f = QgsFeature(vl.fields())
        new_f['pk'] = NULL
        new_f['value'] = ['simple', '"doubleQuote"', "'quote'", 'back\\slash']
        r, fs = vl.dataProvider().addFeatures([new_f])
        self.assertTrue(r)
        new_pk = fs[0]['pk']
        self.assertNotEqual(new_pk, NULL, fs[0].attributes())

        try:
            read_back = vl.getFeature(new_pk)
            self.assertEqual(read_back['pk'], new_pk)
            self.assertEqual(read_back['value'], new_f['value'])
        finally:
            self.assertTrue(vl.startEditing())
            self.assertTrue(vl.deleteFeatures([new_pk]))
            self.assertTrue(vl.commitChanges())

    def testIntArray(self):
        vl = QgsVectorLayer('%s table="qgis_test"."int_array" sql=' % (
            self.dbconn), "testintarray", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(
            fields.at(fields.indexFromName('value')).type(), QVariant.List)
        self.assertEqual(fields.at(fields.indexFromName(
            'value')).subType(), QVariant.Int)

        f = next(vl.getFeatures(QgsFeatureRequest()))

        value_idx = vl.fields().lookupField('value')
        self.assertIsInstance(f.attributes()[value_idx], list)
        self.assertEqual(f.attributes()[value_idx], [1, 2, -5])

    def testDoubleArray(self):
        vl = QgsVectorLayer('%s table="qgis_test"."double_array" sql=' % (
            self.dbconn), "testdoublearray", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(
            fields.at(fields.indexFromName('value')).type(), QVariant.List)
        self.assertEqual(fields.at(fields.indexFromName(
            'value')).subType(), QVariant.Double)

        f = next(vl.getFeatures(QgsFeatureRequest()))

        value_idx = vl.fields().lookupField('value')
        self.assertIsInstance(f.attributes()[value_idx], list)
        self.assertEqual(f.attributes()[value_idx], [1.1, 2, -5.12345])

    def testNotNullConstraint(self):
        vl = QgsVectorLayer('%s table="qgis_test"."constraints" sql=' % (
            self.dbconn), "constraints", "postgres")
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 4)

        # test some bad field indexes
        self.assertEqual(vl.dataProvider().fieldConstraints(-1),
                         QgsFieldConstraints.Constraints())
        self.assertEqual(vl.dataProvider().fieldConstraints(
            1001), QgsFieldConstraints.Constraints())

        self.assertTrue(vl.dataProvider().fieldConstraints(0) &
                        QgsFieldConstraints.ConstraintNotNull)
        self.assertFalse(vl.dataProvider().fieldConstraints(1)
                         & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(vl.dataProvider().fieldConstraints(2) &
                        QgsFieldConstraints.ConstraintNotNull)
        self.assertFalse(vl.dataProvider().fieldConstraints(3)
                         & QgsFieldConstraints.ConstraintNotNull)

        # test that constraints have been saved to fields correctly
        fields = vl.fields()
        self.assertTrue(fields.at(0).constraints().constraints()
                        & QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(fields.at(0).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintOriginProvider)
        self.assertFalse(fields.at(1).constraints().constraints()
                         & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(fields.at(2).constraints().constraints()
                        & QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(fields.at(2).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintOriginProvider)
        self.assertFalse(fields.at(3).constraints().constraints()
                         & QgsFieldConstraints.ConstraintNotNull)

    def testUniqueConstraint(self):
        vl = QgsVectorLayer('%s table="qgis_test"."constraints" sql=' % (
            self.dbconn), "constraints", "postgres")
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 4)

        # test some bad field indexes
        self.assertEqual(vl.dataProvider().fieldConstraints(-1),
                         QgsFieldConstraints.Constraints())
        self.assertEqual(vl.dataProvider().fieldConstraints(
            1001), QgsFieldConstraints.Constraints())

        self.assertTrue(vl.dataProvider().fieldConstraints(0)
                        & QgsFieldConstraints.ConstraintUnique)
        self.assertTrue(vl.dataProvider().fieldConstraints(1)
                        & QgsFieldConstraints.ConstraintUnique)
        self.assertTrue(vl.dataProvider().fieldConstraints(2)
                        & QgsFieldConstraints.ConstraintUnique)
        self.assertFalse(vl.dataProvider().fieldConstraints(3)
                         & QgsFieldConstraints.ConstraintUnique)

        # test that constraints have been saved to fields correctly
        fields = vl.fields()
        self.assertTrue(fields.at(0).constraints().constraints()
                        & QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(fields.at(0).constraints().constraintOrigin(QgsFieldConstraints.ConstraintUnique),
                         QgsFieldConstraints.ConstraintOriginProvider)
        self.assertTrue(fields.at(1).constraints().constraints()
                        & QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(fields.at(1).constraints().constraintOrigin(QgsFieldConstraints.ConstraintUnique),
                         QgsFieldConstraints.ConstraintOriginProvider)
        self.assertTrue(fields.at(2).constraints().constraints()
                        & QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(fields.at(2).constraints().constraintOrigin(QgsFieldConstraints.ConstraintUnique),
                         QgsFieldConstraints.ConstraintOriginProvider)
        self.assertFalse(fields.at(3).constraints().constraints()
                         & QgsFieldConstraints.ConstraintUnique)

    def testConstraintOverwrite(self):
        """ test that Postgres provider constraints can't be overwritten by vector layer method """
        vl = QgsVectorLayer('%s table="qgis_test"."constraints" sql=' % (
            self.dbconn), "constraints", "postgres")
        self.assertTrue(vl.isValid())

        self.assertTrue(vl.dataProvider().fieldConstraints(0) &
                        QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(vl.fields().at(0).constraints().constraints()
                        & QgsFieldConstraints.ConstraintNotNull)

        # add a constraint at the layer level
        vl.setFieldConstraint(0, QgsFieldConstraints.ConstraintUnique)

        # should be no change at provider level
        self.assertTrue(vl.dataProvider().fieldConstraints(0) &
                        QgsFieldConstraints.ConstraintNotNull)

        # but layer should still keep provider constraints...
        self.assertTrue(vl.fields().at(0).constraints().constraints()
                        & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(vl.fieldConstraints(
            0) & QgsFieldConstraints.ConstraintNotNull)
        # ...in addition to layer level constraint
        self.assertTrue(vl.fields().at(0).constraints(
        ).constraints() & QgsFieldConstraints.ConstraintUnique)
        self.assertTrue(vl.fieldConstraints(
            0) & QgsFieldConstraints.ConstraintUnique)

    def testVectorLayerUtilsUniqueWithProviderDefault(self):
        vl = QgsVectorLayer('%s table="qgis_test"."someData" sql=' %
                            (self.dbconn), "someData", "postgres")
        default_clause = 'nextval(\'qgis_test."someData_pk_seq"\'::regclass)'
        vl.dataProvider().setProviderProperty(
            QgsDataProvider.EvaluateDefaultValues, False)
        self.assertEqual(
            vl.dataProvider().defaultValueClause(0), default_clause)
        self.assertTrue(QgsVectorLayerUtils.valueExists(vl, 0, 4))

        vl.startEditing()
        f = QgsFeature(vl.fields())
        f.setAttribute(0, default_clause)
        self.assertFalse(
            QgsVectorLayerUtils.valueExists(vl, 0, default_clause))
        self.assertTrue(vl.addFeatures([f]))

        # the default value clause should exist...
        self.assertTrue(QgsVectorLayerUtils.valueExists(vl, 0, default_clause))
        # but it should not prevent the attribute being validated
        self.assertTrue(QgsVectorLayerUtils.validateAttribute(vl, f, 0))
        vl.rollBack()

    def testSkipConstraintCheck(self):
        vl = QgsVectorLayer('%s table="qgis_test"."someData" sql=' %
                            (self.dbconn), "someData", "postgres")
        default_clause = 'nextval(\'qgis_test."someData_pk_seq"\'::regclass)'
        vl.dataProvider().setProviderProperty(
            QgsDataProvider.EvaluateDefaultValues, False)
        self.assertTrue(vl.dataProvider().skipConstraintCheck(
            0, QgsFieldConstraints.ConstraintUnique, default_clause))
        self.assertFalse(vl.dataProvider().skipConstraintCheck(
            0, QgsFieldConstraints.ConstraintUnique, 59))

    def testVectorLayerUtilsCreateFeatureWithProviderDefault(self):
        vl = QgsVectorLayer('%s table="qgis_test"."someData" sql=' %
                            (self.dbconn), "someData", "postgres")
        default_clause = 'nextval(\'qgis_test."someData_pk_seq"\'::regclass)'
        self.assertEqual(
            vl.dataProvider().defaultValueClause(0), default_clause)

        # If an attribute map is provided, QgsVectorLayerUtils.createFeature must
        # respect it, otherwise default values from provider are checked.
        # User's choice will not be respected if the value violates unique constraints.
        # See https://github.com/qgis/QGIS/issues/27758
        f = QgsVectorLayerUtils.createFeature(vl, attributes={1: 5, 3: 'map'})
        # changed so that createFeature respects user choice
        self.assertEqual(f.attributes(), [
                         default_clause, 5, "'qgis'::text", 'map', None, None, None, None, None])

        vl.setDefaultValueDefinition(3, QgsDefaultValue("'mappy'"))
        # test ignore vector layer default value expression overrides postgres provider default clause,
        # due to user's choice
        f = QgsVectorLayerUtils.createFeature(vl, attributes={1: 5, 3: 'map'})
        self.assertEqual(f.attributes(), [
                         default_clause, 5, "'qgis'::text", 'map', None, None, None, None, None])
        # Since user did not enter a default for field 3, test must return the default value chosen
        f = QgsVectorLayerUtils.createFeature(vl, attributes={1: 5})
        self.assertEqual(f.attributes(), [
                         default_clause, 5, "'qgis'::text", 'mappy', None, None, None, None, None])

    # See https://github.com/qgis/QGIS/issues/23127
    def testNumericPrecision(self):
        uri = 'point?field=f1:int'
        uri += '&field=f2:double(6,4)'
        uri += '&field=f3:string(20)'
        lyr = QgsVectorLayer(uri, "x", "memory")
        self.assertTrue(lyr.isValid())
        f = QgsFeature(lyr.fields())
        f['f1'] = 1
        f['f2'] = 123.456
        f['f3'] = '12345678.90123456789'
        lyr.dataProvider().addFeatures([f])
        uri = '%s table="qgis_test"."b18155" (g) key=\'f1\'' % (self.dbconn)
        self.execSQLCommand('DROP TABLE IF EXISTS qgis_test.b18155')
        err = QgsVectorLayerExporter.exportLayer(
            lyr, uri, "postgres", lyr.crs())
        self.assertEqual(err[0], QgsVectorLayerExporter.NoError,
                         'unexpected import error {0}'.format(err))
        lyr = QgsVectorLayer(uri, "y", "postgres")
        self.assertTrue(lyr.isValid())
        f = next(lyr.getFeatures())
        self.assertEqual(f['f1'], 1)
        self.assertEqual(f['f2'], 123.456)
        self.assertEqual(f['f3'], '12345678.90123456789')

    # See https://github.com/qgis/QGIS/issues/23163
    def testImportKey(self):
        uri = 'point?field=f1:int'
        uri += '&field=F2:double(6,4)'
        uri += '&field=f3:string(20)'
        lyr = QgsVectorLayer(uri, "x", "memory")
        self.assertTrue(lyr.isValid())

        def testKey(lyr, key, kfnames):
            self.execSQLCommand('DROP TABLE IF EXISTS qgis_test.import_test')
            uri = '%s table="qgis_test"."import_test" (g)' % self.dbconn
            if key is not None:
                uri += ' key=\'%s\'' % key
            err = QgsVectorLayerExporter.exportLayer(
                lyr, uri, "postgres", lyr.crs())
            self.assertEqual(err[0], QgsVectorLayerExporter.NoError,
                             'unexpected import error {0}'.format(err))
            olyr = QgsVectorLayer(uri, "y", "postgres")
            self.assertTrue(olyr.isValid())
            flds = lyr.fields()
            oflds = olyr.fields()
            if key is None:
                # if the pkey was not given, it will create a pkey
                self.assertEqual(oflds.size(), flds.size() + 1)
                self.assertEqual(oflds[0].name(), kfnames[0])
                for i in range(flds.size()):
                    self.assertEqual(oflds[i + 1].name(), flds[i].name())
            else:
                # pkey was given, no extra field generated
                self.assertEqual(oflds.size(), flds.size())
                for i in range(oflds.size()):
                    self.assertEqual(oflds[i].name(), flds[i].name())
            pks = olyr.primaryKeyAttributes()
            self.assertEqual(len(pks), len(kfnames))
            for i in range(0, len(kfnames)):
                self.assertEqual(oflds[pks[i]].name(), kfnames[i])

        testKey(lyr, 'f1', ['f1'])
        testKey(lyr, '"f1"', ['f1'])
        testKey(lyr, '"f1","F2"', ['f1', 'F2'])
        testKey(lyr, '"f1","F2","f3"', ['f1', 'F2', 'f3'])
        testKey(lyr, None, ['id'])

    # See https://github.com/qgis/QGIS/issues/25415
    def testImportWithoutSchema(self):

        def _test(table, schema=None):
            self.execSQLCommand('DROP TABLE IF EXISTS %s CASCADE' % table)
            uri = 'point?field=f1:int'
            uri += '&field=F2:double(6,4)'
            uri += '&field=f3:string(20)'
            lyr = QgsVectorLayer(uri, "x", "memory")
            self.assertTrue(lyr.isValid())

            table = ("%s" % table) if schema is None else (
                "\"%s\".\"%s\"" % (schema, table))
            dest_uri = "%s sslmode=disable table=%s  (geom) sql" % (
                self.dbconn, table)
            QgsVectorLayerExporter.exportLayer(
                lyr, dest_uri, "postgres", lyr.crs())
            olyr = QgsVectorLayer(dest_uri, "y", "postgres")
            self.assertTrue(olyr.isValid(), "Failed URI: %s" % dest_uri)

        # Test bug 17518
        _test('b17518')

        # Test fully qualified table (with schema)
        _test("b17518", "qgis_test")

        # Test empty schema
        _test("b17518", "")

        # Test public schema
        _test("b17518", "public")

        # Test fully qualified table (with wrong schema)
        with self.assertRaises(AssertionError):
            _test("b17518", "qgis_test_wrong")

    def testStyle(self):
        self.execSQLCommand('DROP TABLE IF EXISTS layer_styles CASCADE')

        vl = self.getEditableLayer()
        self.assertTrue(vl.isValid())
        self.assertTrue(
            vl.dataProvider().isSaveAndLoadStyleToDatabaseSupported())
        self.assertTrue(vl.dataProvider().isDeleteStyleFromDatabaseSupported())

        # table layer_styles does not exit
        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, -1)
        self.assertEqual(idlist, [])
        self.assertEqual(namelist, [])
        self.assertEqual(desclist, [])
        self.assertNotEqual(errmsg, "")

        qml, errmsg = vl.getStyleFromDatabase("1")
        self.assertEqual(qml, "")
        self.assertNotEqual(errmsg, "")

        mFilePath = QDir.toNativeSeparators(
            '%s/symbol_layer/%s.qml' % (unitTestDataPath(), "singleSymbol"))
        status = vl.loadNamedStyle(mFilePath)
        self.assertTrue(status)

        # The style is saved as non-default
        errorMsg = vl.saveStyleToDatabase(
            "by day", "faded greens and elegant patterns", False, "")
        self.assertEqual(errorMsg, "")

        # the style id should be "1", not "by day"
        qml, errmsg = vl.getStyleFromDatabase("by day")
        self.assertEqual(qml, "")
        self.assertNotEqual(errmsg, "")

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 1)
        self.assertEqual(errmsg, "")
        self.assertEqual(idlist, ["1"])
        self.assertEqual(namelist, ["by day"])
        self.assertEqual(desclist, ["faded greens and elegant patterns"])

        qml, errmsg = vl.getStyleFromDatabase("100")
        self.assertEqual(qml, "")
        self.assertNotEqual(errmsg, "")

        qml, errmsg = vl.getStyleFromDatabase("1")
        self.assertTrue(qml.startswith('<!DOCTYPE qgis'), qml)
        self.assertEqual(errmsg, "")

        res, errmsg = vl.deleteStyleFromDatabase("100")
        self.assertTrue(res)
        self.assertEqual(errmsg, "")

        res, errmsg = vl.deleteStyleFromDatabase("1")
        self.assertTrue(res)
        self.assertEqual(errmsg, "")

        # We save now the style again twice but with one as default
        errorMsg = vl.saveStyleToDatabase(
            "related style", "faded greens and elegant patterns", False, "")
        self.assertEqual(errorMsg, "")
        errorMsg = vl.saveStyleToDatabase(
            "default style", "faded greens and elegant patterns", True, "")
        self.assertEqual(errorMsg, "")

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 2)
        self.assertEqual(errmsg, "")
        self.assertEqual(idlist, ["3", "2"])  # Ids must be reversed.
        self.assertEqual(namelist, ["default style", "related style"])
        self.assertEqual(desclist, ["faded greens and elegant patterns"] * 2)

        # We remove these 2 styles
        res, errmsg = vl.deleteStyleFromDatabase("2")
        self.assertTrue(res)
        self.assertEqual(errmsg, "")
        res, errmsg = vl.deleteStyleFromDatabase("3")
        self.assertTrue(res)
        self.assertEqual(errmsg, "")

        # table layer_styles does exit, but is now empty
        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 0)
        self.assertEqual(idlist, [])
        self.assertEqual(namelist, [])
        self.assertEqual(desclist, [])
        self.assertEqual(errmsg, "")

    def testStyleWithGeometryType(self):
        """Test saving styles with the additional geometry type
        Layers are created from geometries_table
        """

        myconn = 'service=\'qgis_test\''
        if 'QGIS_PGTEST_DB' in os.environ:
            myconn = os.environ['QGIS_PGTEST_DB']

        # point layer
        myPoint = QgsVectorLayer(
            myconn +
            ' sslmode=disable srid=4326 type=POINT table="qgis_test"."geometries_table" (geom) sql=', 'Point',
            'postgres')
        self.assertTrue(myPoint.isValid())
        myPoint.saveStyleToDatabase('myPointStyle', '', False, '')

        # polygon layer
        myPolygon = QgsVectorLayer(
            myconn +
            ' sslmode=disable srid=4326 type=POLYGON table="qgis_test"."geometries_table" (geom) sql=', 'Poly',
            'postgres')
        self.assertTrue(myPoint.isValid())
        myPolygon.saveStyleToDatabase('myPolygonStyle', '', False, '')

        # how many
        related_count, idlist, namelist, desclist, errmsg = myPolygon.listStylesInDatabase()
        self.assertEqual(len(idlist), 2)
        self.assertEqual(namelist, ['myPolygonStyle', 'myPointStyle'])

        # raw psycopg2 query
        self.assertTrue(self.con)
        cur = self.con.cursor()
        self.assertTrue(cur)
        cur.execute("select stylename, type from layer_styles order by type")
        self.assertEqual(cur.fetchall(), [
                         ('myPointStyle', 'Point'), ('myPolygonStyle', 'Polygon')])
        cur.close()

        # delete them
        myPolygon.deleteStyleFromDatabase(idlist[1])
        myPolygon.deleteStyleFromDatabase(idlist[0])
        styles = myPolygon.listStylesInDatabase()
        ids = styles[1]
        self.assertEqual(len(ids), 0)

    def testHasMetadata(self):
        # views don't have metadata
        vl = QgsVectorLayer('{} table="qgis_test"."{}" key="pk" sql='.format(self.dbconn, 'bikes_view'), "bikes_view",
                            "postgres")
        self.assertTrue(vl.isValid())
        self.assertFalse(vl.dataProvider().hasMetadata())

        # ordinary tables have metadata
        vl = QgsVectorLayer('%s table="qgis_test"."someData" sql=' %
                            (self.dbconn), "someData", "postgres")
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.dataProvider().hasMetadata())

    def testReadExtentOnView(self):
        # vector layer based on view
        vl0 = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data_view" (geom) sql=',
            'test', 'postgres')
        self.assertTrue(vl0.isValid())
        self.assertFalse(vl0.dataProvider().hasMetadata())

        # set a custom extent
        originalExtent = vl0.extent()

        customExtent = QgsRectangle(-80, 80, -70, 90)
        vl0.setExtent(customExtent)

        # write xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("maplayer")
        self.assertTrue(vl0.writeLayerXml(elem, doc, QgsReadWriteContext()))

        # read xml with the custom extent. It should not be used by default
        vl1 = QgsVectorLayer()
        vl1.readLayerXml(elem, QgsReadWriteContext())
        self.assertTrue(vl1.isValid())

        self.assertEqual(vl1.extent(), originalExtent)

        # read xml with custom extent with readExtent option. Extent read from
        # xml document should be used because we have a view
        vl2 = QgsVectorLayer()
        vl2.setReadExtentFromXml(True)
        vl2.readLayerXml(elem, QgsReadWriteContext())
        self.assertTrue(vl2.isValid())

        self.assertEqual(vl2.extent(), customExtent)

        # but a force update on extent should allow retrieveing the data
        # provider extent
        vl2.updateExtents()
        vl2.readLayerXml(elem, QgsReadWriteContext())
        self.assertEqual(vl2.extent(), customExtent)

        vl2.updateExtents(force=True)
        vl2.readLayerXml(elem, QgsReadWriteContext())
        self.assertEqual(vl2.extent(), originalExtent)

    def testReadExtentOnTable(self):
        # vector layer based on a standard table
        vl0 = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=',
            'test', 'postgres')
        self.assertTrue(vl0.isValid())
        self.assertTrue(vl0.dataProvider().hasMetadata())

        # set a custom extent
        originalExtent = vl0.extent()

        customExtent = QgsRectangle(-80, 80, -70, 90)
        vl0.setExtent(customExtent)

        # write xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("maplayer")
        self.assertTrue(vl0.writeLayerXml(elem, doc, QgsReadWriteContext()))

        # read xml with the custom extent. It should not be used by default
        vl1 = QgsVectorLayer()
        vl1.readLayerXml(elem, QgsReadWriteContext())
        self.assertTrue(vl1.isValid())

        self.assertEqual(vl1.extent(), originalExtent)

        # read xml with custom extent with readExtent option. Extent read from
        # xml document should NOT be used because we don't have a view or a
        # materialized view
        vl2 = QgsVectorLayer()
        vl2.setReadExtentFromXml(True)
        vl2.readLayerXml(elem, QgsReadWriteContext())
        self.assertTrue(vl2.isValid())

        self.assertEqual(vl2.extent(), originalExtent)

    def testDeterminePkey(self):
        """Test primary key auto-determination"""

        vl = QgsVectorLayer(self.dbconn + ' sslmode=disable srid=4326 type=POLYGON table="qgis_test"."authors" sql=',
                            'test', 'postgres')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.dataProvider().hasMetadata())
        self.assertTrue("key='pk'" in vl.source())

    def testCheckPkUnicityOnView(self):
        # vector layer based on view

        # This is valid
        vl0 = QgsVectorLayer(
            self.dbconn +
            ' checkPrimaryKeyUnicity=\'0\'  sslmode=disable key=\'pk\' srid=0 type=POINT table="qgis_test"."b21839_pk_unicity_view" (geom) sql=',
            'test', 'postgres')
        self.assertTrue(vl0.isValid())

        geom = vl0.getFeature(1).geometry().asWkt()

        # This is NOT valid
        vl0 = QgsVectorLayer(
            self.dbconn +
            ' checkPrimaryKeyUnicity=\'1\' sslmode=disable key=\'an_int\' srid=0 type=POINT table="qgis_test"."b21839_pk_unicity_view" (geom) sql=',
            'test', 'postgres')
        self.assertFalse(vl0.isValid())

        # This is NOT valid because the default is to check unicity
        vl0 = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'an_int\' srid=0 type=POINT table="qgis_test"."b21839_pk_unicity_view" (geom) sql=',
            'test', 'postgres')
        self.assertFalse(vl0.isValid())

        # This is valid because the readExtentFromXml option is set
        # loadDefaultStyle, readExtentFromXml
        options = QgsVectorLayer.LayerOptions(True, True)
        vl0 = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'an_int\' srid=0 type=POINT table="qgis_test"."b21839_pk_unicity_view" (geom) sql=',
            'test', 'postgres', options)
        self.assertTrue(vl0.isValid())

        # Valid because a_unique_int is unique and default is to check unicity
        vl0 = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'a_unique_int\' srid=0 type=POINT table="qgis_test"."b21839_pk_unicity_view" (geom) sql=',
            'test', 'postgres')
        self.assertEqual(vl0.getFeature(1).geometry().asWkt(), geom)

        # Valid because a_unique_int is unique
        vl0 = QgsVectorLayer(
            self.dbconn +
            ' checkPrimaryKeyUnicity=\'1\' sslmode=disable key=\'a_unique_int\' srid=0 type=POINT table="qgis_test"."b21839_pk_unicity_view" (geom) sql=',
            'test', 'postgres')
        self.assertTrue(vl0.isValid())
        self.assertEqual(vl0.getFeature(1).geometry().asWkt(), geom)

    def testNotify(self):
        vl0 = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=',
            'test', 'postgres')
        vl0.dataProvider().setListening(True)

        class Notified(QObject):

            def __init__(self):
                super(Notified, self).__init__()
                self.received = ""

            def receive(self, msg):
                self.received = msg

        notified = Notified()
        vl0.dataProvider().notify.connect(notified.receive)

        vl0.dataProvider().setListening(True)

        cur = self.con.cursor()
        ok = False
        start = time.time()
        while True:
            cur.execute("NOTIFY qgis, 'my message'")
            self.con.commit()
            QGISAPP.processEvents()
            if notified.received == "my message":
                ok = True
                break
            if (time.time() - start) > 5:  # timeout
                break

        vl0.dataProvider().notify.disconnect(notified.receive)
        vl0.dataProvider().setListening(False)

        self.assertTrue(ok)

    def testStyleDatabaseWithService(self):
        """Test saving style in DB using a service file.

        To run this test, you first need to setup the test
        database with tests/testdata/provider/testdata_pg.sh
        """
        myconn = 'service=\'qgis_test\''
        if 'QGIS_PGTEST_DB' in os.environ:
            myconn = os.environ['QGIS_PGTEST_DB']
        myvl = QgsVectorLayer(
            myconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=',
            'test', 'postgres')

        styles = myvl.listStylesInDatabase()
        ids = styles[1]
        self.assertEqual(len(ids), 0)

        myvl.saveStyleToDatabase('mystyle', '', False, '')
        styles = myvl.listStylesInDatabase()
        ids = styles[1]
        self.assertEqual(len(ids), 1)

        myvl.deleteStyleFromDatabase(ids[0])
        styles = myvl.listStylesInDatabase()
        ids = styles[1]
        self.assertEqual(len(ids), 0)

    def testCurveToMultipolygon(self):
        self.execSQLCommand(
            'CREATE TABLE IF NOT EXISTS multicurve(pk SERIAL NOT NULL PRIMARY KEY, geom public.geometry(MultiPolygon, 4326))')
        self.execSQLCommand('TRUNCATE multicurve')

        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=MULTIPOLYGON table="multicurve" (geom) sql=',
            'test', 'postgres')

        f = QgsFeature(vl.fields())
        f.setGeometry(QgsGeometry.fromWkt(
            'CurvePolygon(CircularString (20 30, 50 30, 50 90, 10 50, 20 30))'))
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.addFeatures([f]))
        self.assertTrue(vl.commitChanges())

        f = next(vl.getFeatures(QgsFeatureRequest()))

        g = f.geometry().constGet()
        self.assertTrue(g)
        self.assertEqual(g.wkbType(), QgsWkbTypes.MultiPolygon)
        self.assertEqual(g.childCount(), 1)
        self.assertTrue(g.childGeometry(0).vertexCount() > 3)

    def testMassivePaste(self):
        """Speed test to compare createFeature and createFeatures, for regression #21303"""

        import time

        self.execSQLCommand(
            'CREATE TABLE IF NOT EXISTS massive_paste(pk SERIAL NOT NULL PRIMARY KEY, geom public.geometry(Polygon, 4326))')
        self.execSQLCommand('TRUNCATE massive_paste')

        start_time = time.time()
        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="massive_paste" (geom) sql=',
            'test_massive_paste', 'postgres')
        self.assertTrue(vl.startEditing())
        features = []
        context = vl.createExpressionContext()
        for i in range(4000):
            features.append(
                QgsVectorLayerUtils.createFeature(vl, QgsGeometry.fromWkt('Polygon ((7 44, 8 45, 8 46, 7 46, 7 44))'),
                                                  {0: i}, context))
        self.assertTrue(vl.addFeatures(features))
        self.assertTrue(vl.commitChanges())
        self.assertEqual(vl.featureCount(), 4000)
        print("--- %s seconds ---" % (time.time() - start_time))

        self.execSQLCommand('TRUNCATE massive_paste')
        start_time = time.time()
        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="massive_paste" (geom) sql=',
            'test_massive_paste', 'postgres')
        self.assertTrue(vl.startEditing())
        features_data = []
        context = vl.createExpressionContext()
        for i in range(4000):
            features_data.append(
                QgsVectorLayerUtils.QgsFeatureData(QgsGeometry.fromWkt('Polygon ((7 44, 8 45, 8 46, 7 46, 7 44))'),
                                                   {0: i}))
        features = QgsVectorLayerUtils.createFeatures(
            vl, features_data, context)
        self.assertTrue(vl.addFeatures(features))
        self.assertTrue(vl.commitChanges())
        self.assertEqual(vl.featureCount(), 4000)
        print("--- %s seconds ---" % (time.time() - start_time))

    def testFilterOnCustomBbox(self):
        extent = QgsRectangle(-68, 70, -67, 80)
        request = QgsFeatureRequest().setFilterRect(extent)
        dbconn = 'service=qgis_test'
        uri = '%s srid=4326 key="pk" sslmode=disable table="qgis_test"."some_poly_data_shift_bbox" (geom)' % (
            dbconn)

        def _test(vl, ids):
            values = {feat['pk']: 'x' for feat in vl.getFeatures(request)}
            expected = {x: 'x' for x in ids}
            self.assertEqual(values, expected)

        vl = QgsVectorLayer(uri, "testgeom", "postgres")
        self.assertTrue(vl.isValid())
        _test(vl, [2, 3])

        vl = QgsVectorLayer(uri + ' bbox=shiftbox', "testgeom", "postgres")
        self.assertTrue(vl.isValid())
        _test(vl, [1, 3])

    def testValidLayerDiscoverRelationsNone(self):
        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=',
            'test', 'postgres')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.dataProvider().discoverRelations(vl, []), [])

    def testInvalidLayerDiscoverRelations(self):
        vl = QgsVectorLayer('{} table="qgis_test"."invalid_layer" sql='.format(self.dbconn), "invalid_layer",
                            "postgres")
        self.assertFalse(vl.isValid())
        self.assertEqual(vl.dataProvider().discoverRelations(vl, []), [])

    def testCheckTidPkOnViews(self):
        """Test vector layer based on a view with `ctid` as a key"""

        # This is valid
        vl0 = QgsVectorLayer(
            self.dbconn +
            ' checkPrimaryKeyUnicity=\'0\'  sslmode=disable key=\'ctid\' srid=4326 type=POINT table="qgis_test"."b31799_test_view_ctid" (geom) sql=',
            'test', 'postgres')
        self.assertTrue(vl0.isValid())
        self.assertEqual(vl0.featureCount(), 10)
        for f in vl0.getFeatures():
            self.assertNotEqual(f.attribute(0), NULL)

    def testFeatureCountEstimatedOnView(self):
        """
        Test feature count on view when estimated data is enabled
        """
        self.execSQLCommand('DROP VIEW IF EXISTS qgis_test.somedataview')
        self.execSQLCommand(
            'CREATE VIEW qgis_test.somedataview AS SELECT * FROM qgis_test."someData"')
        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' estimatedmetadata=true srid=4326 type=POINT table="qgis_test"."somedataview" (geom) sql=',
            'test', 'postgres')
        self.assertTrue(vl.isValid())
        self.assertTrue(self.source.featureCount() > 0)

    def testIdentityPk(self):
        """Test a table with identity pk, see GH #29560"""

        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'gid\' srid=4326 type=POLYGON table="qgis_test"."b29560"(geom) sql=',
            'testb29560', 'postgres')
        self.assertTrue(vl.isValid())

        feature = QgsFeature(vl.fields())
        geom = QgsGeometry.fromWkt('POLYGON EMPTY')
        feature.setGeometry(geom)
        self.assertTrue(vl.dataProvider().addFeature(feature))

        del (vl)

        # Verify
        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'gid\' srid=4326 type=POLYGON table="qgis_test"."b29560"(geom) sql=',
            'testb29560', 'postgres')
        self.assertTrue(vl.isValid())
        feature = next(vl.getFeatures())
        self.assertIsNotNone(feature.id())

    @unittest.skipIf(os.environ.get('TRAVIS', '') == 'true', 'Test flaky')
    def testDefaultValuesAndClauses(self):
        """Test whether default values like CURRENT_TIMESTAMP or
        now() they are respected. See GH #33383"""

        # Create the test table

        vl = QgsVectorLayer(self.dbconn + ' sslmode=disable  table="public"."test_table_default_values" sql=', 'test',
                            'postgres')
        self.assertTrue(vl.isValid())

        dp = vl.dataProvider()

        # Clean the table
        dp.deleteFeatures(dp.allFeatureIds())

        # Save it for the test
        now = datetime.now()

        # Test default values
        dp.setProviderProperty(QgsDataProvider.EvaluateDefaultValues, 1)
        # FIXME: spatialite provider (and OGR) return a NULL here and the following passes
        # self.assertTrue(dp.defaultValue(0).isNull())
        self.assertIsNotNone(dp.defaultValue(0))
        self.assertIsNone(dp.defaultValue(1))
        self.assertTrue(dp.defaultValue(
            2).startswith(now.strftime('%Y-%m-%d')))
        self.assertTrue(dp.defaultValue(
            3).startswith(now.strftime('%Y-%m-%d')))
        self.assertEqual(dp.defaultValue(4), 123)
        self.assertEqual(dp.defaultValue(5), 'My default')

        # FIXME: the provider should return the clause definition
        #       regardless of the EvaluateDefaultValues setting
        dp.setProviderProperty(QgsDataProvider.EvaluateDefaultValues, 0)
        self.assertEqual(dp.defaultValueClause(
            0), "nextval('test_table_default_values_id_seq'::regclass)")
        self.assertEqual(dp.defaultValueClause(1), '')
        self.assertEqual(dp.defaultValueClause(2), "now()")
        self.assertEqual(dp.defaultValueClause(3), "CURRENT_TIMESTAMP")
        self.assertEqual(dp.defaultValueClause(4), '123')
        self.assertEqual(dp.defaultValueClause(5), "'My default'::text")
        # FIXME: the test fails if the value is not reset to 1
        dp.setProviderProperty(QgsDataProvider.EvaluateDefaultValues, 1)

        feature = QgsFeature(vl.fields())
        for idx in range(vl.fields().count()):
            default = vl.dataProvider().defaultValue(idx)
            if default is not None:
                feature.setAttribute(idx, default)
            else:
                feature.setAttribute(idx, 'A comment')

        self.assertTrue(vl.dataProvider().addFeature(feature))
        del (vl)

        # Verify
        vl2 = QgsVectorLayer(self.dbconn + ' sslmode=disable  table="public"."test_table_default_values" sql=', 'test',
                             'postgres')
        self.assertTrue(vl2.isValid())
        feature = next(vl2.getFeatures())
        self.assertEqual(feature.attribute(1), 'A comment')
        self.assertTrue(feature.attribute(
            2).startswith(now.strftime('%Y-%m-%d')))
        self.assertTrue(feature.attribute(
            3).startswith(now.strftime('%Y-%m-%d')))
        self.assertEqual(feature.attribute(4), 123)
        self.assertEqual(feature.attribute(5), 'My default')

    def testEncodeDecodeUri(self):
        """Test PG encode/decode URI"""

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        self.assertEqual(md.decodeUri(
            'dbname=\'qgis_tests\' host=localhost port=5432 user=\'myuser\' sslmode=disable estimatedmetadata=true srid=3067 table="public"."basic_map_tiled" (rast)'),
            {'dbname': 'qgis_tests',
             'estimatedmetadata': True,
             'geometrycolumn': 'rast',
             'host': 'localhost',
             'port': '5432',
             'schema': 'public',
             'srid': '3067',
             'sslmode': 1,
             'table': 'basic_map_tiled',
             'username': 'myuser'})

        self.assertEqual(md.decodeUri(
            'dbname=\'qgis_tests\' host=localhost port=5432 user=\'myuser\' sslmode=disable key=\'id\' estimatedmetadata=true srid=3763 type=MultiPolygon checkPrimaryKeyUnicity=\'1\' table="public"."copas1" (geom)'),
            {'dbname': 'qgis_tests',
             'estimatedmetadata': True,
             'geometrycolumn': 'geom',
             'host': 'localhost',
             'key': 'id',
             'port': '5432',
             'schema': 'public',
             'srid': '3763',
             'sslmode': 1,
             'table': 'copas1',
             'type': 6,
             'username': 'myuser'})

        self.assertEqual(md.encodeUri({'dbname': 'qgis_tests',
                                       'estimatedmetadata': True,
                                       'geometrycolumn': 'geom',
                                       'host': 'localhost',
                                       'key': 'id',
                                       'port': '5432',
                                       'schema': 'public',
                                       'srid': '3763',
                                       'sslmode': 1,
                                       'table': 'copas1',
                                       'type': 6,
                                       'username': 'myuser'}),
                         "dbname='qgis_tests' user='myuser' srid=3763 estimatedmetadata='true' host='localhost' key='id' port='5432' sslmode='disable' type='MultiPolygon' table=\"public\".\"copas1\" (geom)")

        self.assertEqual(md.encodeUri({'dbname': 'qgis_tests',
                                       'estimatedmetadata': True,
                                       'geometrycolumn': 'rast',
                                       'host': 'localhost',
                                       'port': '5432',
                                       'schema': 'public',
                                       'srid': '3067',
                                       'sslmode': 1,
                                       'table': 'basic_map_tiled',
                                       'username': 'myuser'}),
                         "dbname='qgis_tests' user='myuser' srid=3067 estimatedmetadata='true' host='localhost' port='5432' sslmode='disable' table=\"public\".\"basic_map_tiled\" (rast)")

        def _round_trip(uri):
            decoded = md.decodeUri(uri)
            self.assertEqual(decoded, md.decodeUri(md.encodeUri(decoded)))

        uri = self.dbconn + \
            ' sslmode=disable key=\'gid\' srid=3035  table="public"."my_pg_vector" sql='
        decoded = md.decodeUri(uri)
        self.assertEqual(decoded, {
            'key': 'gid',
            'schema': 'public',
            'service': 'qgis_test',
            'srid': '3035',
            'sslmode': QgsDataSourceUri.SslDisable,
            'table': 'my_pg_vector',
        })

        _round_trip(uri)

        uri = self.dbconn + \
            ' sslmode=prefer key=\'gid\' srid=3035 temporalFieldIndex=2 ' + \
            'authcfg=afebeff username=\'my username\' password=\'my secret password=\' ' + \
            'table="public"."my_pg_vector" (the_geom) sql="a_field" != 1223223'

        _round_trip(uri)

        decoded = md.decodeUri(uri)
        self.assertEqual(decoded, {
            'authcfg': 'afebeff',
            'geometrycolumn': 'the_geom',
            'key': 'gid',
            'password': 'my secret password=',
            'schema': 'public',
            'service': 'qgis_test',
            'sql': '"a_field" != 1223223',
            'srid': '3035',
            'sslmode': QgsDataSourceUri.SslPrefer,
            'table': 'my_pg_vector',
            'username': 'my username',
        })


class TestPyQgsPostgresProviderCompoundKey(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.dbconn = 'service=qgis_test'
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        # Create test layers
        cls.vl = QgsVectorLayer(
            cls.dbconn +
            ' sslmode=disable key=\'"key1","key2"\' srid=4326 type=POINT table="qgis_test"."someDataCompound" (geom) sql=',
            'test', 'postgres')
        assert cls.vl.isValid()
        cls.source = cls.vl.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def enableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', True)
        return True

    def disableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', False)

    def uncompiledFilters(self):
        return set(['"dt" = to_datetime(\'000www14ww13ww12www4ww5ww2020\',\'zzzwwwsswwmmwwhhwwwdwwMwwyyyy\')',
                    '"date" = to_date(\'www4ww5ww2020\',\'wwwdwwMwwyyyy\')',
                    '"time" = to_time(\'000www14ww13ww12www\',\'zzzwwwsswwmmwwhhwww\')'])

    def partiallyCompiledFilters(self):
        return set([])

    def testConstraints(self):
        for key in ["key1", "key2"]:
            idx = self.vl.dataProvider().fieldNameIndex(key)
            self.assertTrue(idx >= 0)
            self.assertFalse(self.vl.dataProvider().fieldConstraints(
                idx) & QgsFieldConstraints.ConstraintUnique)

    def testCompoundPkChanges(self):
        """ Check if fields with compound primary keys can be changed """
        vl = self.vl

        self.assertTrue(vl.isValid())
        idx_key1 = vl.fields().lookupField('key1')
        idx_key2 = vl.fields().lookupField('key2')
        # the name "pk" for this datasource is misleading;
        # the primary key is actually composed by the fields key1 and key2
        idx_pk = vl.fields().lookupField('pk')
        idx_name = vl.fields().lookupField('name')
        idx_name2 = vl.fields().lookupField('name2')

        geomwkt = 'Point(-47.945 -15.812)'

        # start editing ordinary attribute.
        ft1 = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression("key1 = 2 AND key2 = 2")))
        self.assertTrue(ft1.isValid())
        original_geometry = ft1.geometry().asWkt()

        vl.startEditing()
        self.assertTrue(vl.changeAttributeValues(ft1.id(), {idx_name: 'Rose'}))
        self.assertTrue(vl.commitChanges())

        # check change
        ft2 = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression("key1 = 2 AND key2 = 2")))
        self.assertEqual(ft2['name'], 'Rose')
        self.assertEqual(ft2['name2'], 'Apple')
        self.assertEqual(ft2['pk'], 2)

        # now, start editing one of the PK field components
        vl.startEditing()

        self.assertTrue(vl.dataProvider().changeFeatures({ft2.id(): {idx_key2: 42, idx_name: 'Orchid', idx_name2: 'Daisy'}}, {ft2.id(): QgsGeometry.fromWkt(geomwkt)}))
        self.assertTrue(vl.commitChanges())

        # let's check if we still have the same fid...
        ft2 = next(vl.getFeatures(QgsFeatureRequest().setFilterFid(ft2.id())))
        self.assertEqual(ft2['key2'], 42)
        self.assertEqual(ft2['name'], 'Orchid')
        self.assertEqual(ft2['name2'], 'Daisy')
        self.assertTrue(vl.startEditing())
        vl.changeAttributeValues(ft2.id(), {idx_key1: 21, idx_name2: 'Hibiscus'})
        self.assertTrue(vl.commitChanges())
        ft2 = next(vl.getFeatures(QgsFeatureRequest().setFilterFid(ft2.id())))
        self.assertEqual(ft2['key1'], 21)
        self.assertEqual(ft2['name2'], 'Hibiscus')

        # lets get a brand new feature and check how it went...
        ft3 = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk = 2')))
        self.assertEqual(ft3['name'], 'Orchid')
        self.assertEqual(ft3['key1'], 21)
        self.assertEqual(ft3['key2'], 42)

        assert compareWkt(ft3.geometry().asWkt(), geomwkt), "Geometry mismatch. Expected: {} Got: {}\n".format(ft3.geometry().asWkt(), geomwkt)

        # Now, we leave the record as we found it, so further tests can proceed
        vl.startEditing()
        self.assertTrue(vl.dataProvider().changeFeatures({ft3.id(): {idx_key1: 2, idx_key2: 2, idx_pk: 2, idx_name: 'Apple', idx_name2: 'Apple'}}, {ft3.id(): QgsGeometry.fromWkt(original_geometry)}))
        self.assertTrue(vl.commitChanges())


class TestPyQgsPostgresProviderBigintSinglePk(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.dbconn = 'service=qgis_test'
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        # Create test layers
        cls.vl = QgsVectorLayer(
            cls.dbconn +
            ' sslmode=disable key=\'"pk"\' srid=4326 type=POINT table="qgis_test"."provider_bigint_single_pk" (geom) sql=',
            'bigint_pk', 'postgres')
        assert cls.vl.isValid()
        cls.source = cls.vl.dataProvider()
        cls.con = psycopg2.connect(cls.dbconn)

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def getSource(self):
        """ drops/recreates the test data anew, like TestPyQgsPostgresProvider::getSource above. """
        self.execSqlCommand(
            "DROP TABLE IF EXISTS qgis_test.provider_edit_bigint_single_pk")
        self.execSqlCommand(
            "CREATE TABLE qgis_test.provider_edit_bigint_single_pk ( pk bigserial PRIMARY KEY, cnt integer, name text DEFAULT 'qgis', name2 text DEFAULT 'qgis', num_char text, dt timestamp without time zone, \"date\" date, \"time\" time without time zone, geom public.geometry(Point,4326), key1 integer, key2 integer)")
        self.execSqlCommand(
            "INSERT INTO qgis_test.provider_edit_bigint_single_pk  ( key1, key2, pk, cnt, name, name2, num_char, dt, \"date\", \"time\", geom) VALUES"
            "(1, 1, 5, -200, NULL, 'NuLl', '5', TIMESTAMP '2020-05-04 12:13:14', '2020-05-02', '12:13:01', '0101000020E61000001D5A643BDFC751C01F85EB51B88E5340'),"
            "(1, 2, 3,  300, 'Pear', 'PEaR', '3', NULL, NULL, NULL, NULL),"
            "(2, 1, 1,  100, 'Orange', 'oranGe', '1', TIMESTAMP '2020-05-03 12:13:14', '2020-05-03', '12:13:14', '0101000020E61000006891ED7C3F9551C085EB51B81E955040'),"
            "(2, 2, 2,  200, 'Apple', 'Apple', '2', TIMESTAMP '2020-05-04 12:14:14', '2020-05-04', '12:14:14', '0101000020E6100000CDCCCCCCCC0C51C03333333333B35140'),"
            "(2, 3, 4,  400, 'Honey', 'Honey', '4', TIMESTAMP '2021-05-04 13:13:14', '2021-05-04', '13:13:14', '0101000020E610000014AE47E17A5450C03333333333935340')")
        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'"pk"\' srid=4326 type=POINT table="qgis_test"."provider_edit_bigint_single_pk" (geom) sql=',
            'edit_bigint_pk', 'postgres')
        return vl

    def getEditableLayer(self):
        return self.getSource()

    def execSqlCommand(self, sql):
        self.assertTrue(self.con)
        cur = self.con.cursor()
        self.assertTrue(cur)
        cur.execute(sql)
        cur.close()
        self.con.commit()

    def enableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', True)
        return True

    def disableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', False)

    def uncompiledFilters(self):
        return set(['"dt" = to_datetime(\'000www14ww13ww12www4ww5ww2020\',\'zzzwwwsswwmmwwhhwwwdwwMwwyyyy\')',
                    '"date" = to_date(\'www4ww5ww2020\',\'wwwdwwMwwyyyy\')',
                    '"time" = to_time(\'000www14ww13ww12www\',\'zzzwwwsswwmmwwhhwww\')'])

    def partiallyCompiledFilters(self):
        return set([])

    def testConstraints(self):
        idx = self.vl.dataProvider().fieldNameIndex("pk")
        self.assertTrue(idx >= 0)

    def testGetFeaturesFidTests(self):
        fids = [f.id() for f in self.source.getFeatures()]
        assert len(fids) == 5, 'Expected 5 features, got {} instead'.format(
            len(fids))
        for id in fids:
            features = [f for f in self.source.getFeatures(
                QgsFeatureRequest().setFilterFid(id))]
            self.assertEqual(len(features), 1)
            feature = features[0]
            self.assertTrue(feature.isValid())

            result = [feature.id()]
            expected = [id]
            assert result == expected, 'Expected {} and got {} when testing for feature ID filter'.format(expected,
                                                                                                          result)

            # test that results match QgsFeatureRequest.acceptFeature
            request = QgsFeatureRequest().setFilterFid(id)
            for f in self.source.getFeatures():
                self.assertEqual(request.acceptFeature(f), f.id() == id)

        # TODO: bad features are not tested because the PostgreSQL provider
        # doesn't mark explicitly set invalid features as such.

    def testGetFeatures(self, source=None, extra_features=[], skip_features=[], changed_attributes={},
                        changed_geometries={}):
        """ Test that expected results are returned when fetching all features """

        # IMPORTANT - we do not use `for f in source.getFeatures()` as we are also
        # testing that existing attributes & geometry in f are overwritten correctly
        # (for f in ... uses a new QgsFeature for every iteration)

        if not source:
            source = self.source

        it = source.getFeatures()
        f = QgsFeature()
        attributes = {}
        geometries = {}
        while it.nextFeature(f):
            # expect feature to be valid
            self.assertTrue(f.isValid())
            # some source test datasets will include additional attributes which we ignore,
            # so cherry pick desired attributes
            attrs = [f['pk'], f['cnt'], f['name'], f['name2'], f['num_char']]
            # DON'T force the num_char attribute to be text - some sources (e.g., delimited text) will
            # automatically detect that this attribute contains numbers and set it as a numeric
            # field
            # TODO: PostgreSQL 12 won't accept conversion from integer to text.
            # attrs[4] = str(attrs[4])
            attributes[f['pk']] = attrs
            geometries[f['pk']] = f.hasGeometry() and f.geometry().asWkt()

        expected_attributes = {5: [5, -200, NULL, 'NuLl', '5'],
                               3: [3, 300, 'Pear', 'PEaR', '3'],
                               1: [1, 100, 'Orange', 'oranGe', '1'],
                               2: [2, 200, 'Apple', 'Apple', '2'],
                               4: [4, 400, 'Honey', 'Honey', '4']}

        expected_geometries = {1: 'Point (-70.332 66.33)',
                               2: 'Point (-68.2 70.8)',
                               3: None,
                               4: 'Point(-65.32 78.3)',
                               5: 'Point(-71.123 78.23)'}
        for f in extra_features:
            expected_attributes[f[0]] = f.attributes()
            if f.hasGeometry():
                expected_geometries[f[0]] = f.geometry().asWkt()
            else:
                expected_geometries[f[0]] = None

        for i in skip_features:
            del expected_attributes[i]
            del expected_geometries[i]
        for i, a in changed_attributes.items():
            for attr_idx, v in a.items():
                expected_attributes[i][attr_idx] = v
        for i, g, in changed_geometries.items():
            if g:
                expected_geometries[i] = g.asWkt()
            else:
                expected_geometries[i] = None

        self.assertEqual(attributes, expected_attributes, 'Expected {}, got {}'.format(
            expected_attributes, attributes))

        self.assertEqual(len(expected_geometries), len(geometries))

        for pk, geom in list(expected_geometries.items()):
            if geom:
                assert compareWkt(geom, geometries[pk]), "Geometry {} mismatch Expected:\n{}\nGot:\n{}\n".format(pk,
                                                                                                                 geom,
                                                                                                                 geometries[
                                                                                                                     pk])
            else:
                self.assertFalse(
                    geometries[pk], 'Expected null geometry for {}'.format(pk))

    def testAddFeatureExtraAttributes(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        if not l.dataProvider().capabilities() & QgsVectorDataProvider.AddFeatures:
            return

        # test that adding features with too many attributes drops these attributes
        # we be more tricky and also add a valid feature to stress test the provider
        f1 = QgsFeature()
        f1.setAttributes([6, -220, 'qgis', 'String', '15'])
        f2 = QgsFeature()
        f2.setAttributes([7, -230, 'qgis', 'String', '15', 15, 16, 17])

        result, added = l.dataProvider().addFeatures([f1, f2])
        self.assertTrue(result,
                        'Provider returned False to addFeatures with extra attributes. Providers should accept these features but truncate the extra attributes.')

        # make sure feature was added correctly
        added = [f for f in l.dataProvider().getFeatures() if f['pk'] == 7][0]
        # TODO: The PostgreSQL provider doesn't truncate extra attributes!
        self.assertNotEqual(added.attributes(), [7, -230, 'qgis', 'String', '15'],
                            'The PostgreSQL provider doesn\'t truncate extra attributes.')

    def testAddFeatureMissingAttributes(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        if not l.dataProvider().capabilities() & QgsVectorDataProvider.AddFeatures:
            return

        # test that adding features with missing attributes pads out these
        # attributes with NULL values to the correct length.
        # changed from ProviderTestBase.testAddFeatureMissingAttributes: we use
        # 'qgis' instead of NULL below.
        # TODO: Only unmentioned attributes get filled with the DEFAULT table
        # value; if the attribute is present, the saved value will be NULL if
        # that is indicated, or the value mentioned by the user; there is no
        # implicit conversion of PyQGIS::NULL to PostgreSQL DEFAULT.
        f1 = QgsFeature()
        f1.setAttributes([6, -220, 'qgis', 'String'])
        f2 = QgsFeature()
        f2.setAttributes([7, 330])

        result, added = l.dataProvider().addFeatures([f1, f2])
        self.assertTrue(result,
                        'Provider returned False to addFeatures with missing attributes. Providers should accept these features but add NULL attributes to the end of the existing attributes to the required field length.')
        f1.setId(added[0].id())
        f2.setId(added[1].id())

        # check result - feature attributes MUST be padded out to required number of fields
        f1.setAttributes([6, -220, 'qgis', 'String', NULL])
        f2.setAttributes([7, 330, 'qgis', 'qgis', NULL])
        self.testGetFeatures(l.dataProvider(), [f1, f2])

    def testAddFeature(self):
        if not getattr(self, 'getEditableLayer', None):
            return

        l = self.getEditableLayer()
        self.assertTrue(l.isValid())

        f1 = QgsFeature()
        # changed from ProviderTestBase.testAddFeature: we use 'qgis' instead
        # of NULL below.
        # TODO: Only unmentioned attributes get filled with the DEFAULT table
        # value; if the attribute is present, the saved value will be NULL if
        # that is indicated, or the value mentioned by the user; there is no
        # implicit conversion of PyQGIS::NULL to PostgreSQL DEFAULT.
        f1.setAttributes([6, -220, 'qgis', 'String', '15'])
        f1.setGeometry(QgsGeometry.fromWkt('Point (-72.345 71.987)'))

        f2 = QgsFeature()
        f2.setAttributes([7, 330, 'Coconut', 'CoCoNut', '13'])

        if l.dataProvider().capabilities() & QgsVectorDataProvider.AddFeatures:
            # expect success
            result, added = l.dataProvider().addFeatures([f1, f2])
            self.assertTrue(
                result, 'Provider reported AddFeatures capability, but returned False to addFeatures')
            f1.setId(added[0].id())
            f2.setId(added[1].id())

            # check result
            self.testGetFeatures(l.dataProvider(), [f1, f2])

            # add empty list, should return true for consistency
            self.assertTrue(l.dataProvider().addFeatures([]))

            # ensure that returned features have been given the correct id
            f = next(l.getFeatures(
                QgsFeatureRequest().setFilterFid(added[0].id())))
            self.assertTrue(f.isValid())
            self.assertEqual(f['cnt'], -220)

            f = next(l.getFeatures(
                QgsFeatureRequest().setFilterFid(added[1].id())))
            self.assertTrue(f.isValid())
            self.assertEqual(f['cnt'], 330)
        else:
            # expect fail
            self.assertFalse(l.dataProvider().addFeatures([f1, f2]),
                             'Provider reported no AddFeatures capability, but returned true to addFeatures')

    def testModifyPk(self):
        """ Check if we can modify a primary key value. Since this PK is bigint, we also exercise the mapping between fid and values """

        vl = self.getEditableLayer()
        self.assertTrue(vl.isValid())
        geomwkt = 'Point(-47.945 -15.812)'

        feature = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk = 4')))
        self.assertTrue(feature.isValid())

        self.assertTrue(vl.startEditing())
        idxpk = vl.fields().lookupField('pk')

        self.assertTrue(vl.dataProvider().changeFeatures({feature.id(): {idxpk: 42}}, {feature.id(): QgsGeometry.fromWkt(geomwkt)}))
        self.assertTrue(vl.commitChanges())

        # read back
        ft = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk = 42')))
        self.assertTrue(ft.isValid())
        self.assertEqual(ft['name'], 'Honey')
        assert compareWkt(ft.geometry().asWkt(), geomwkt), "Geometry mismatch. Expected: {} Got: {}\n".format(ft.geometry().asWkt(), geomwkt)

    def testDuplicatedFieldNamesInQueryLayers(self):
        """Test regresssion GH #36205"""

        vl = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'__rid__\' table="(SELECT row_number() OVER () AS __rid__, * FROM (SELECT *  from qgis_test.some_poly_data a, qgis_test.some_poly_data b  where  ST_Intersects(a.geom,b.geom)) as foo)" sql=', 'test_36205', 'postgres')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.featureCount(), 3)

        # This fails because the "geom" field and "pk" fields are ambiguous
        # There is no easy fix: all duplicated fields should be explicitly aliased
        # and the query internally rewritten
        # feature = next(vl.getFeatures())
        # self.assertTrue(vl.isValid())


if __name__ == '__main__':
    unittest.main()
