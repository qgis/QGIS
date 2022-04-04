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
    Qgis,
    QgsApplication,
    QgsVectorLayer,
    QgsVectorLayerExporter,
    QgsFeatureRequest,
    QgsFeatureSource,
    QgsFeature,
    QgsFieldConstraints,
    QgsDataProvider,
    QgsVectorLayerUtils,
    QgsSettings,
    QgsTransactionGroup,
    QgsReadWriteContext,
    QgsRectangle,
    QgsReferencedGeometry,
    QgsDefaultValue,
    QgsCoordinateReferenceSystem,
    QgsProcessingUtils,
    QgsProcessingContext,
    QgsProcessingFeedback,
    QgsProject,
    QgsWkbTypes,
    QgsGeometry,
    QgsProviderRegistry,
    QgsVectorDataProvider,
    QgsDataSourceUri,
    QgsProviderConnectionException,
    NULL,
)
from qgis.analysis import QgsNativeAlgorithms
from qgis.gui import QgsGui, QgsAttributeForm
from qgis.PyQt.QtCore import QDate, QTime, QDateTime, QVariant, QDir, QObject, QByteArray, QTemporaryDir
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
        assert cls.vl.isValid(), "Could not create a layer from the 'qgis_test.someData' table using dbconn '" + cls.dbconn + "'"

        cls.source = cls.vl.dataProvider()
        cls.poly_vl = QgsVectorLayer(
            cls.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=',
            'test', 'postgres')
        assert cls.poly_vl.isValid(), "Could not create a layer from the 'qgis_test.some_poly_data' table using dbconn '" + cls.dbconn + "'"

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

    # Create instances of this class for scoped backups,
    # example:
    #
    #  backup1 = self.scopedTableBackup('qgis_test', 'datatable1')
    #  backup2 = self.scopedTableBackup('qgis_test', 'datatable2')
    #
    def scopedTableBackup(self, schema, table):

        class ScopedBackup():
            def __init__(self, tester, schema, table):
                self.schema = schema
                self.table = table
                self.tester = tester
                tester.execSQLCommand('DROP TABLE IF EXISTS {s}.{t}_edit CASCADE'.format(s=schema, t=table))
                tester.execSQLCommand('CREATE TABLE {s}.{t}_edit AS SELECT * FROM {s}.{t}'.format(s=schema, t=table))

            def __del__(self):
                self.tester.execSQLCommand('TRUNCATE TABLE {s}.{t}'.format(s=self.schema, t=self.table))
                self.tester.execSQLCommand('INSERT INTO {s}.{t} SELECT * FROM {s}.{t}_edit'.format(s=self.schema, t=self.table))
                self.tester.execSQLCommand('DROP TABLE {s}.{t}_edit'.format(s=self.schema, t=self.table))

        return ScopedBackup(self, schema, table)

    def temporarySchema(self, name):

        class TemporarySchema():
            def __init__(self, tester, name):
                self.tester = tester
                self.name = name + '_tmp'

            def __enter__(self):
                self.tester.execSQLCommand('DROP SCHEMA IF EXISTS {} CASCADE'.format(self.name))
                self.tester.execSQLCommand('CREATE SCHEMA {}'.format(self.name))
                return self.name

            def __exit__(self, type, value, traceback):
                self.tester.execSQLCommand('DROP SCHEMA {} CASCADE'.format(self.name))

        return TemporarySchema(self, 'qgis_test_' + name)

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
        self.execSQLCommand("SELECT setval('qgis_test.\"editData_pk_seq\"', 5, true)")
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

    def getGeneratedColumnsData(self):
        """
        return a tuple with the generated column test layer and the expected generated value
        """
        cur = self.con.cursor()
        cur.execute("SHOW server_version_num")
        pgversion = int(cur.fetchone()[0])

        # don't trigger this test when PostgreSQL versions earlier than 12.
        if pgversion < 120000:
            return (None, None)
        else:
            return (QgsVectorLayer(self.dbconn + ' sslmode=disable table="qgis_test"."generated_columns"', 'test', 'postgres'),
                    """('test:'::text || ((pk)::character varying)::text)""")

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

        # Backup test table (will be edited)
        self.execSQLCommand('DROP TABLE IF EXISTS qgis_test.test_gen_col_edit CASCADE')
        self.execSQLCommand('CREATE TABLE qgis_test.test_gen_col_edit AS SELECT id,name,geom FROM qgis_test.test_gen_col')

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
        self.assertAlmostEqual(f2['poly_area'], expected_area, places=4)
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
        self.assertAlmostEqual(f2['poly_area'], expected_area, places=4)

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
        self.assertAlmostEqual(f2['poly_area'], expected_area, places=4)
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

        # Restore test table (after editing it)
        self.execSQLCommand('TRUNCATE TABLE qgis_test.test_gen_col')
        self.execSQLCommand('INSERT INTO qgis_test.test_gen_col(id,name,geom) SELECT id,name,geom FROM qgis_test.test_gen_col_edit')
        self.execSQLCommand('DROP TABLE qgis_test.test_gen_col_edit')

    def testNonPkBigintField(self):
        """Test if we can correctly insert, read and change attributes(fields) of type bigint and which are not PKs."""
        vl = QgsVectorLayer(
            '{} sslmode=disable srid=4326 key="pk" table="qgis_test".{} (geom)'.format(
                self.dbconn, 'bigint_pk'),
            "bigint_pk", "postgres")
        self.assertTrue(vl.isValid())
        flds = vl.fields()

        # Backup test table (will be edited)
        scopedBackup = self.scopedTableBackup('qgis_test', 'bigint_pk')

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

        # Backup test table (will be edited)
        scopedBackup = self.scopedTableBackup('qgis_test', 'bigint_pk')

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

        # Backup test table (will be edited)
        scopedBackup = self.scopedTableBackup('qgis_test', 'bigint_non_first_pk')

        statuses = [-1, -1, -1, -1]
        values = ['first value', 'second value', 'zero value', 'negative value']
        newvalues = ['1st value', '2nd value', '0th value', '-1th value']
        # changing values...
        for ft in vl.getFeatures():
            if ft['value'] == values[0]:
                vl.changeAttributeValue(
                    ft.id(), flds.indexOf('value'), newvalues[0])
                statuses[0] = 0
            elif ft['value'] == values[1]:
                vl.changeAttributeValue(
                    ft.id(), flds.indexOf('value'), newvalues[1])
                statuses[1] = 0
            elif ft['value'] == values[2]:
                vl.changeAttributeValue(
                    ft.id(), flds.indexOf('value'), newvalues[2])
                statuses[2] = 0
            elif ft['value'] == values[3]:
                vl.changeAttributeValue(
                    ft.id(), flds.indexOf('value'), newvalues[3])
                statuses[3] = 0
        self.assertTrue(vl.commitChanges())
        for i in range(len(statuses)):
            self.assertEqual(statuses[i], 0, 'start value "{}" not found'.format(values[i]))

        # now, let's see if the values were changed
        vl2 = QgsVectorLayer(
            '{} sslmode=disable srid=4326 key="pk" table="qgis_test".{} (geom)'.format(
                self.dbconn, 'bigint_non_first_pk'),
            "bigint_pk_nonfirst", "postgres")
        self.assertTrue(vl2.isValid())
        for ft in vl2.getFeatures():
            if ft['value'] == newvalues[0]:
                statuses[0] = 1
            elif ft['value'] == newvalues[1]:
                statuses[1] = 1
            elif ft['value'] == newvalues[2]:
                statuses[2] = 1
            elif ft['value'] == newvalues[3]:
                statuses[3] = 1
        for i in range(len(statuses)):
            self.assertEqual(statuses[i], 1, 'changed value "{}" not found'.format(newvalues[i]))

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

        # Backup test table (will be edited)
        scopedBackup = self.scopedTableBackup('qgis_test', 'tb_test_compound_pk')

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

    def testPktCompositeFloat(self):
        """
        Check that tables with PKs composed of many fields of different types are correctly read and written to
        """
        vl = QgsVectorLayer('{} sslmode=disable srid=4326 key=\'"pk1","pk2","pk3"\' table="qgis_test"."tb_test_composite_float_pk" (geom)'.format(self.dbconn), "test_composite_float", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.fields()

        f = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression("pk3 = '3.14159274'")))
        # first of all: we must be able to fetch a valid feature
        self.assertTrue(f.isValid())
        self.assertEqual(f['pk1'], 1)
        self.assertEqual(f['pk2'], 2)

        self.assertAlmostEqual(f['pk3'], 3.14159274)
        self.assertEqual(f['value'], 'test 2')

        # Backup test table (will be edited)
        scopedBackup = self.scopedTableBackup('qgis_test', 'tb_test_composite_float_pk')

        # can we edit a field?
        vl.startEditing()
        vl.changeAttributeValue(f.id(), fields.indexOf('value'), 'Edited Test 2')
        self.assertTrue(vl.commitChanges())

        # Did we get it right? Let's create a new QgsVectorLayer and try to read back our changes:
        vl2 = QgsVectorLayer('{} sslmode=disable srid=4326 key=\'"pk1","pk2","pk3"\' table="qgis_test"."tb_test_composite_float_pk" (geom)'.format(self.dbconn), "test_composite_float2", "postgres")
        self.assertTrue(vl2.isValid())
        f2 = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression("pk3 = '3.14159274'")))
        self.assertTrue(f2.isValid())

        # just making sure we have the correct feature
        self.assertAlmostEqual(f2['pk3'], 3.14159274)

        # Then, making sure we really did change our value.
        self.assertEqual(f2['value'], 'Edited Test 2')

        # How about inserting a new field?
        f3 = QgsFeature(vl2.fields())
        f3['pk1'] = 4
        f3['pk2'] = -9223372036854775800
        f3['pk3'] = 7.29154
        f3['value'] = 'other test'
        vl.startEditing()
        res, f3 = vl.dataProvider().addFeatures([f3])
        self.assertTrue(res)
        self.assertTrue(vl.commitChanges())

        # can we catch it on another layer?
        f4 = next(vl2.getFeatures(QgsFeatureRequest().setFilterExpression("pk2 = '-9223372036854775800'")))

        self.assertTrue(f4.isValid())
        expected_attrs = [4, -9223372036854775800, 7.29154, 'other test']
        gotten_attrs = [f4['pk1'], f4['pk2'], f4['pk3'], f4['value']]
        self.assertEqual(gotten_attrs[0], expected_attrs[0])
        self.assertEqual(gotten_attrs[1], expected_attrs[1])
        self.assertAlmostEqual(gotten_attrs[2], expected_attrs[2], places=4)
        self.assertEqual(gotten_attrs[3], expected_attrs[3])

        # Finally, let's delete one of the features.
        f5 = next(vl2.getFeatures(QgsFeatureRequest().setFilterExpression("pk3 = '7.29154'")))
        vl2.startEditing()
        vl2.deleteFeatures([f5.id()])
        self.assertTrue(vl2.commitChanges())

        # did we really delete?
        f_iterator = vl.getFeatures(QgsFeatureRequest().setFilterExpression("pk3 = '7.29154'"))
        got_feature = True

        try:
            f6 = next(f_iterator)
            got_feature = f6.isValid()
        except StopIteration:
            got_feature = False

        self.assertFalse(got_feature)

    def testPktFloatingPoint(self):
        """
        Check if we can handle floating point/numeric primary keys correctly
        """

        # 0. Backup test table (will be edited)
        scopedBackup1 = self.scopedTableBackup('qgis_test', 'tb_test_float_pk')
        scopedBackup2 = self.scopedTableBackup('qgis_test', 'tb_test_double_pk')

        # 1. 32 bit float (PostgreSQL "REAL" type)
        vl = QgsVectorLayer(self.dbconn + ' sslmode=disable srid=4326 key="pk" table="qgis_test"."tb_test_float_pk" (geom)', "test_float_pk", "postgres")
        self.assertTrue(vl.isValid())

        # 1.1. Retrieving
        f = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression("pk = '3.141592653589793238462643383279502884197169'")))
        self.assertTrue(f.isValid())
        self.assertEqual(f['value'], 'first teste')
        # 1.2. Editing
        self.assertTrue(vl.startEditing())
        vl.changeAttributeValue(f.id(), vl.fields().indexOf('value'), 'Changed first')
        self.assertTrue(vl.commitChanges())
        # 1.2.1. Checking edit from another vector layer
        vl2 = QgsVectorLayer(self.dbconn + ' sslmode=disable srid=4326 key="pk1" table="qgis_test"."tb_test_float_pk" (geom)', "test_float_pk2", "postgres")
        self.assertTrue(vl2.isValid())
        f2 = next(vl2.getFeatures(QgsFeatureRequest().setFilterExpression("pk = '3.141592653589793238462643383279502884197169'")))
        self.assertTrue(f2.isValid())
        self.assertEqual(f2['value'], 'Changed first')
        # 1.3. Deleting
        f = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression("pk = '2.718281828459045235360287471352662497757247'")))
        vl.startEditing()
        vl.deleteFeatures([f.id()])
        self.assertTrue(vl.commitChanges())
        # 1.3.1. Checking deletion
        f_iterator = vl2.getFeatures(QgsFeatureRequest().setFilterExpression("pk = '2.718281828459045235360287471352662497757247'"))
        got_feature = True

        try:
            f2 = next(f_iterator)
            got_feature = f2.isValid()
        except StopIteration:
            got_feature = False
        self.assertFalse(got_feature)
        # 1.4. Inserting new feature
        newpointwkt = 'Point(-47.751 -15.644)'
        f = QgsFeature(vl.fields())
        f['pk'] = 0.22222222222222222222222
        f['value'] = 'newly inserted'
        f.setGeometry(QgsGeometry.fromWkt(newpointwkt))
        vl.startEditing()
        res, f = vl.dataProvider().addFeatures([f])
        self.assertTrue(res)
        self.assertTrue(vl.commitChanges())
        # 1.4.1. Checking insertion
        f2 = next(vl2.getFeatures(QgsFeatureRequest().setFilterExpression("pk = '0.22222222222222222222222'")))
        self.assertTrue(f2.isValid())
        self.assertAlmostEqual(f2['pk'], 0.2222222222222222)
        self.assertEqual(f2['value'], 'newly inserted')
        assert compareWkt(f2.geometry().asWkt(), newpointwkt), "Geometry mismatch. Expected: {} Got: {} \n".format(f2.geometry().asWkt(), newpointwkt)
        # One more check: can we retrieve the same row with the value that we got from this layer?
        floatpk = f2['pk']
        f3 = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression("pk = '{}'".format(floatpk))))
        self.assertTrue(f3.isValid())
        self.assertEqual(f3['value'], 'newly inserted')
        self.assertEqual(f3['pk'], floatpk)

        # 2. 64 bit float (PostgreSQL "DOUBLE PRECISION" type)
        vl = QgsVectorLayer(self.dbconn + ' sslmode=disable srid=4326 key="pk" table="qgis_test"."tb_test_double_pk" (geom)', "test_double_pk", "postgres")
        self.assertTrue(vl.isValid())

        # 2.1. Retrieving
        f = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression("pk = '3.141592653589793238462643383279502884197169'")))
        self.assertTrue(f.isValid())
        self.assertEqual(f['value'], 'first teste')
        # 2.2. Editing
        self.assertTrue(vl.startEditing())
        vl.changeAttributeValue(f.id(), vl.fields().indexOf('value'), 'Changed first')
        self.assertTrue(vl.commitChanges())
        # 2.2.1. Checking edit from another vector layer
        vl2 = QgsVectorLayer(self.dbconn + ' sslmode=disable srid=4326 key="pk" table="qgis_test"."tb_test_double_pk" (geom)', "test_double_pk2", "postgres")
        self.assertTrue(vl2.isValid())
        f2 = next(vl2.getFeatures(QgsFeatureRequest().setFilterExpression("pk = '3.141592653589793238462643383279502884197169'")))
        self.assertTrue(f2.isValid())
        self.assertEqual(f2['value'], 'Changed first')
        # 2.3. Deleting
        f = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression("pk = '2.718281828459045235360287471352662497757247'")))
        vl.startEditing()
        vl.deleteFeatures([f.id()])
        self.assertTrue(vl.commitChanges())
        # 2.3.1. Checking deletion
        f_iterator = vl2.getFeatures(QgsFeatureRequest().setFilterExpression("pk = '2.718281828459045235360287471352662497757247'"))
        got_feature = True

        try:
            f2 = next(f_iterator)
            got_feature = f2.isValid()
        except StopIteration:
            got_feature = False
        self.assertFalse(got_feature)
        # 2.4. Inserting new feature
        newpointwkt = 'Point(-47.751 -15.644)'
        f = QgsFeature(vl.fields())
        f['pk'] = 0.22222222222222222222222
        f['value'] = 'newly inserted'
        f.setGeometry(QgsGeometry.fromWkt(newpointwkt))
        vl.startEditing()
        res, f = vl.dataProvider().addFeatures([f])
        self.assertTrue(res)
        self.assertTrue(vl.commitChanges())
        # 2.4.1. Checking insertion
        f2 = next(vl2.getFeatures(QgsFeatureRequest().setFilterExpression("pk = '0.22222222222222222222222'")))
        self.assertTrue(f2.isValid())
        self.assertAlmostEqual(f2['pk'], 0.2222222222222222, places=15)
        self.assertEqual(f2['value'], 'newly inserted')
        assert compareWkt(f2.geometry().asWkt(), newpointwkt), "Geometry mismatch. Expected: {} Got: {} \n".format(f2.geometry().asWkt(), newpointwkt)
        # One more check: can we retrieve the same row with the value that we got from this layer?
        doublepk = f2['pk']
        f3 = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression("pk = '{}'".format(doublepk))))
        self.assertTrue(f3.isValid())
        self.assertEqual(f3['value'], 'newly inserted')
        self.assertEqual(f3['pk'], doublepk)

        # no NUMERIC/DECIMAL checks here. NUMERIC primary keys are unsupported.
        # TODO: implement NUMERIC primary keys/arbitrary precision arithmethics/fixed point math in QGIS.

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

    def testClonePreservesFidMap(self):
        vl = QgsVectorLayer(
            '{} sslmode=disable srid=4326 key="pk" table="qgis_test".{} (geom)'.format(
                self.dbconn, 'bigint_pk'),
            "bigint_pk", "postgres")

        # Generate primary keys
        f = next(vl.getFeatures('pk = 2'))  # 1
        f = next(vl.getFeatures('pk = -1'))  # 2
        fid_orig = f.id()

        clone = vl.clone()
        f = next(clone.getFeatures('pk = -1'))  # should still be 2
        fid_copy = f.id()
        self.assertEqual(fid_orig, fid_copy)

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
        l = self.getEditableLayer()
        tg.addLayer(l)
        l.startEditing()
        it = l.getFeatures()
        f = next(it)
        f['pk'] = NULL
        self.assertTrue(l.addFeature(f))  # Should not deadlock during an active iteration
        f = next(it)
        l.commitChanges()

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
        p.setTransactionMode(Qgis.TransactionMode.AutomaticGroups)
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
        p.setTransactionMode(Qgis.TransactionMode.AutomaticGroups)
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
        p.setTransactionMode(Qgis.TransactionMode.AutomaticGroups)
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
        p.setTransactionMode(Qgis.TransactionMode.AutomaticGroups)
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

        # Backup test table (will be edited)
        tableBackup = self.scopedTableBackup('qgis_test', 'json')

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

        # table layer_styles does not exist

        res, err = QgsProviderRegistry.instance().styleExists('postgres', vl.source(), '')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('postgres', vl.source(), 'a style')
        self.assertFalse(res)
        self.assertFalse(err)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, -1)
        self.assertEqual(idlist, [])
        self.assertEqual(namelist, [])
        self.assertEqual(desclist, [])
        self.assertFalse(errmsg)

        qml, errmsg = vl.getStyleFromDatabase("1")
        self.assertFalse(qml)
        self.assertTrue(errmsg)

        mFilePath = QDir.toNativeSeparators(
            '%s/symbol_layer/%s.qml' % (unitTestDataPath(), "singleSymbol"))
        status = vl.loadNamedStyle(mFilePath)
        self.assertTrue(status)

        # The style is saved as non-default
        errorMsg = vl.saveStyleToDatabase(
            "by day", "faded greens and elegant patterns", False, "")
        self.assertFalse(errorMsg)

        # the style id should be "1", not "by day"
        qml, errmsg = vl.getStyleFromDatabase("by day")
        self.assertEqual(qml, "")
        self.assertNotEqual(errmsg, "")

        res, err = QgsProviderRegistry.instance().styleExists('postgres', vl.source(), '')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('postgres', vl.source(), 'a style')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('postgres', vl.source(), 'by day')
        self.assertTrue(res)
        self.assertFalse(err)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 1)
        self.assertFalse(errmsg)
        self.assertEqual(idlist, ["1"])
        self.assertEqual(namelist, ["by day"])
        self.assertEqual(desclist, ["faded greens and elegant patterns"])

        qml, errmsg = vl.getStyleFromDatabase("100")
        self.assertFalse(qml)
        self.assertTrue(errmsg)

        qml, errmsg = vl.getStyleFromDatabase("1")
        self.assertTrue(qml.startswith('<!DOCTYPE qgis'), qml)
        self.assertFalse(errmsg)

        res, errmsg = vl.deleteStyleFromDatabase("100")
        self.assertTrue(res)
        self.assertEqual(errmsg, "")

        res, errmsg = vl.deleteStyleFromDatabase("1")
        self.assertTrue(res)
        self.assertFalse(errmsg)

        # We save now the style again twice but with one as default
        errmsg = vl.saveStyleToDatabase(
            "related style", "faded greens and elegant patterns", False, "")
        self.assertEqual(errmsg, "")
        errmsg = vl.saveStyleToDatabase(
            "default style", "faded greens and elegant patterns", True, "")
        self.assertFalse(errmsg)

        res, err = QgsProviderRegistry.instance().styleExists('postgres', vl.source(), '')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('postgres', vl.source(), 'a style')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('postgres', vl.source(), 'default style')
        self.assertTrue(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('postgres', vl.source(), 'related style')
        self.assertTrue(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('postgres', vl.source(), 'by day')
        self.assertFalse(res)
        self.assertFalse(err)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 2)
        self.assertEqual(errmsg, "")
        self.assertEqual(idlist, ["3", "2"])  # Ids must be reversed.
        self.assertEqual(namelist, ["default style", "related style"])
        self.assertEqual(desclist, ["faded greens and elegant patterns"] * 2)

        # We remove these 2 styles
        res, errmsg = vl.deleteStyleFromDatabase("2")
        self.assertTrue(res)
        self.assertFalse(errmsg)
        res, errmsg = vl.deleteStyleFromDatabase("3")
        self.assertTrue(res)
        self.assertFalse(errmsg)

        # table layer_styles does exist, but is now empty
        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 0)
        self.assertEqual(idlist, [])
        self.assertEqual(namelist, [])
        self.assertEqual(desclist, [])
        self.assertFalse(errmsg)

        res, err = QgsProviderRegistry.instance().styleExists('postgres', vl.source(), '')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('postgres', vl.source(), 'a style')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('postgres', vl.source(), 'default style')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('postgres', vl.source(), 'related style')
        self.assertFalse(res)
        self.assertFalse(err)

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

    def testSaveStyleInvalidXML(self):

        self.execSQLCommand('DROP TABLE IF EXISTS layer_styles CASCADE')

        vl = self.getEditableLayer()
        self.assertTrue(vl.isValid())
        self.assertTrue(
            vl.dataProvider().isSaveAndLoadStyleToDatabaseSupported())
        self.assertTrue(vl.dataProvider().isDeleteStyleFromDatabaseSupported())

        mFilePath = QDir.toNativeSeparators(
            '%s/symbol_layer/%s.qml' % (unitTestDataPath(), "fontSymbol"))
        status = vl.loadNamedStyle(mFilePath)
        self.assertTrue(status)

        errorMsg = vl.saveStyleToDatabase(
            "fontSymbol", "font with invalid utf8 char", False, "")
        self.assertEqual(errorMsg, "")

        qml, errmsg = vl.getStyleFromDatabase("1")
        self.assertTrue('v="\u001E"' in qml)
        self.assertEqual(errmsg, "")

        # Test loadStyle from metadata
        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        qml = md.loadStyle(self.dbconn + " type=POINT table=\"qgis_test\".\"editData\" (geom)", 'fontSymbol')
        self.assertTrue(qml.startswith('<!DOCTYPE qgi'), qml)
        self.assertTrue('v="\u001E"' in qml)

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
        # xml document should be used even if we don't have a view or a
        # materialized view
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

    def testPreparedFailure(self):
        """Test error from issue GH #45100"""

        layer = self.getEditableLayerWithCheckConstraint()
        self.assertTrue(layer.startEditing())
        old_value = layer.getFeature(1).attribute('i_will_fail_on_no_name')
        layer.changeAttributeValue(1, 1, 'no name')
        layer.changeGeometry(1, QgsGeometry.fromWkt('point(7 45)'))
        self.assertFalse(layer.commitChanges())
        layer.changeAttributeValue(1, 1, old_value)
        self.assertTrue(layer.commitChanges())

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

        # try with a layer which doesn't have geom
        myvl = QgsVectorLayer(
            myconn +
            ' sslmode=disable key=\'pk\' table="qgis_test"."bikes" sql=', 'test', 'postgres')
        self.assertTrue(myvl.isValid())

        myvl.saveStyleToDatabase('mystyle_wo_geom', '', False, '')
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
        """
        Test checks that discover relation feature can be used on a layer that has no relation.
        """
        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=',
            'test', 'postgres')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.dataProvider().discoverRelations(vl, []), [])

    def testInvalidLayerDiscoverRelations(self):
        """
        Test that discover relations feature can be used on invalid layer.
        """
        vl = QgsVectorLayer('{} table="qgis_test"."invalid_layer" sql='.format(self.dbconn), "invalid_layer",
                            "postgres")
        self.assertFalse(vl.isValid())
        self.assertEqual(vl.dataProvider().discoverRelations(vl, []), [])

    def testValidLayerDiscoverRelations(self):
        """
        Test implicit relations that can be discovers between tables, based on declared foreign keys.
        The test also checks that two distinct relations can be discovered when two foreign keys are declared (see #41138).
        """
        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' checkPrimaryKeyUnicity=\'1\' table="qgis_test"."referencing_layer"',
            'referencing_layer', 'postgres')
        vls = [
            QgsVectorLayer(
                self.dbconn +
                ' sslmode=disable key=\'pk_ref_1\' checkPrimaryKeyUnicity=\'1\' table="qgis_test"."referenced_layer_1"',
                'referenced_layer_1', 'postgres'),
            QgsVectorLayer(
                self.dbconn +
                ' sslmode=disable key=\'pk_ref_2\' checkPrimaryKeyUnicity=\'1\' table="qgis_test"."referenced_layer_2"',
                'referenced_layer_2', 'postgres'),
            vl
        ]

        for lyr in vls:
            self.assertTrue(lyr.isValid())
            QgsProject.instance().addMapLayer(lyr)
        relations = vl.dataProvider().discoverRelations(vl, vls)
        self.assertEqual(len(relations), 2)
        for i, r in enumerate(relations):
            self.assertEqual(r.referencedLayer(), vls[i])

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

    def testFeatureCountEstimatedOnTable(self):
        """
        Test feature count on table when estimated data is enabled
        """
        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' estimatedmetadata=true srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=',
            'test', 'postgres')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.featureCount() > 0)

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
        self.assertTrue(vl.featureCount() > 0)

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

        self.assertEqual(vl.dataProvider().defaultValueClause(0), "nextval('b29560_gid_seq'::regclass)")

        del (vl)

        # Verify
        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'gid\' srid=4326 type=POLYGON table="qgis_test"."b29560"(geom) sql=',
            'testb29560', 'postgres')
        self.assertTrue(vl.isValid())
        feature = next(vl.getFeatures())
        self.assertIsNotNone(feature.id())

    @unittest.skipIf(os.environ.get('QGIS_CONTINUOUS_INTEGRATION_RUN', 'true'), 'Test flaky')
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

    def testHasSpatialIndex(self):
        for layer_name in ('hspi_table', 'hspi_materialized_view'):
            columns = {'geom_without_index': QgsFeatureSource.SpatialIndexNotPresent, 'geom_with_index': QgsFeatureSource.SpatialIndexPresent}
            for (geometry_column, spatial_index) in columns.items():
                conn = 'service=\'qgis_test\''
                if 'QGIS_PGTEST_DB' in os.environ:
                    conn = os.environ['QGIS_PGTEST_DB']
                vl = QgsVectorLayer(
                    conn +
                    ' sslmode=disable key=\'id\' srid=4326 type=\'Polygon\' table="qgis_test"."{n}" ({c}) sql='.format(n=layer_name, c=geometry_column),
                    'test', 'postgres')
                self.assertTrue(vl.isValid())
                self.assertEqual(vl.hasSpatialIndex(), spatial_index)

    def testBBoxFilterOnGeographyType(self):
        """Test bounding box filter on geography type"""

        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."testgeog" (geog) sql=',
            'test', 'postgres')

        self.assertTrue(vl.isValid())

        def _test(vl, extent, ids):
            request = QgsFeatureRequest().setFilterRect(extent)
            values = {feat['pk']: 'x' for feat in vl.getFeatures(request)}
            expected = {x: 'x' for x in ids}
            self.assertEqual(values, expected)

        _test(vl, QgsRectangle(40 - 0.01, -0.01, 40 + 0.01, 0.01), [1])
        _test(vl, QgsRectangle(40 - 5, -5, 40 + 5, 5), [1])
        _test(vl, QgsRectangle(40 - 5, 0, 40 + 5, 5), [1])
        _test(vl, QgsRectangle(40 - 10, -10, 40 + 10, 10), [1])  # no use of spatial index currently
        _test(vl, QgsRectangle(40 - 5, 0.01, 40 + 5, 5), [])  # no match

        _test(vl, QgsRectangle(40 - 0.01, 60 - 0.01, 40 + 0.01, 60 + 0.01), [2])
        _test(vl, QgsRectangle(40 - 5, 60 - 5, 40 + 5, 60 + 5), [2])
        _test(vl, QgsRectangle(40 - 5, 60 - 0.01, 40 + 5, 60 + 9.99), [2])

        _test(vl, QgsRectangle(40 - 0.01, -60 - 0.01, 40 + 0.01, -60 + 0.01), [3])
        _test(vl, QgsRectangle(40 - 5, -60 - 5, 40 + 5, -60 + 5), [3])
        _test(vl, QgsRectangle(40 - 5, -60 - 9.99, 40 + 5, -60 + 0.01), [3])

        _test(vl, QgsRectangle(-181, -90, 181, 90), [1, 2, 3])  # no use of spatial index currently

    def testReadCustomSRID(self):
        """Test that we can correctly read the SRS from a custom SRID"""

        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn = md.createConnection(self.dbconn, {})

        # Cleanup if needed
        try:
            conn.dropVectorTable('qgis_test', 'test_custom_srid')
        except QgsProviderConnectionException:
            pass

        conn.executeSql("DELETE FROM spatial_ref_sys WHERE srid = 543210 AND auth_name='FOO' AND auth_srid=32600;")
        conn.executeSql("""INSERT INTO spatial_ref_sys (srid, auth_name, auth_srid, srtext, proj4text) VALUES (543210, 'FOO', 32600, 'PROJCS["my_projection",GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",0],PARAMETER["scale_factor",1],PARAMETER["false_easting",0],PARAMETER["false_northing",0],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["Easting",EAST],AXIS["Northing",NORTH]]','+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +no_defs');""")

        conn.executeSql('''
        CREATE TABLE "qgis_test"."test_custom_srid" (
            gid serial primary key,
            geom geometry(Point, 543210)
        );''')

        layer = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'gid\'table="qgis_test"."test_custom_srid" (geom) sql=', 'test', 'postgres')

        conn.executeSql("DELETE FROM spatial_ref_sys WHERE srid = 543210 AND auth_name='FOO' AND auth_srid=32600;")

        self.assertTrue(layer.isValid())
        self.assertEqual(layer.crs().description(), 'my_projection')

    def testSingleMultiColumnPkSmallData(self):
        """Test Single and Multi Column PK, `Small` Data"""
        from itertools import combinations

        def test_for_pk_combinations(test_type_list, pk_column_name_list, fids_get_count):
            pk_column_name = ','.join(pk_column_name_list)
            set_new_pk = '''
                ALTER TABLE qgis_test.multi_column_pk_small_data_table DROP CONSTRAINT multi_column_pk_small_data_pk;
                ALTER TABLE qgis_test.multi_column_pk_small_data_table
                    ADD CONSTRAINT multi_column_pk_small_data_pk PRIMARY KEY ({});'''
            set_new_layer = ' sslmode=disable key=\'{}\' srid=3857 type=POLYGON table="qgis_test"."multi_column_pk_small_data_{}" (geom) sql='
            error_string = 'from {} with PK - {} : expected {}, got {}'

            if 'table' in test_type_list:
                self.execSQLCommand(set_new_pk.format(pk_column_name))
            for test_type in test_type_list:
                vl = QgsVectorLayer(self.dbconn + set_new_layer.format(pk_column_name, test_type), 'test_multi_column_pk_small_data', 'postgres')
                fids = [f.id() for f in vl.getFeatures(QgsFeatureRequest().setLimit(fids_get_count))]
                fids2 = [f.id() for f in vl.getFeatures(fids)]
                self.assertEqual(fids_get_count, len(fids), "Get with limit " +
                                 error_string.format(test_type, pk_column_name, fids_get_count, len(fids)))
                self.assertEqual(fids_get_count, len(fids2), "Get by fids " +
                                 error_string.format(test_type, pk_column_name, fids_get_count, len(fids2)))

        self.execSQLCommand('DROP TABLE IF EXISTS qgis_test.multi_column_pk_small_data_table CASCADE;')
        self.execSQLCommand('''
        CREATE TABLE qgis_test.multi_column_pk_small_data_table (
          id_serial serial NOT NULL,
          id_uuid uuid NOT NULL,
          id_int int NOT NULL,
          id_bigint bigint NOT NULL,
          id_str character varying(20) NOT NULL,
          id_inet4 inet NOT NULL,
          id_inet6 inet NOT NULL,
          id_cidr4 cidr NOT NULL,
          id_cidr6 cidr NOT NULL,
          id_macaddr macaddr NOT NULL,
          id_macaddr8 macaddr8 NOT NULL,
          id_timestamp timestamp with time zone NOT NULL,
          id_half_null_uuid uuid,
          id_all_null_uuid uuid,
          geom geometry(Polygon,3857),
          CONSTRAINT multi_column_pk_small_data_pk
            PRIMARY KEY (id_serial, id_uuid, id_int, id_bigint, id_str) );''')
        self.execSQLCommand('''
        CREATE OR REPLACE VIEW qgis_test.multi_column_pk_small_data_view AS
          SELECT * FROM qgis_test.multi_column_pk_small_data_table;
        DROP MATERIALIZED VIEW IF EXISTS qgis_test.multi_column_pk_small_data_mat_view;
        CREATE MATERIALIZED VIEW qgis_test.multi_column_pk_small_data_mat_view AS
          SELECT * FROM qgis_test.multi_column_pk_small_data_table;''')
        self.execSQLCommand('''
        TRUNCATE qgis_test.multi_column_pk_small_data_table;
        INSERT INTO qgis_test.multi_column_pk_small_data_table(
          id_uuid, id_int, id_bigint, id_str, id_inet4, id_inet6, id_cidr4, id_cidr6,
            id_macaddr, id_macaddr8, id_timestamp, id_half_null_uuid, id_all_null_uuid, geom)
          SELECT
            ( (10000000)::text || (100000000000 + dy)::text || (100000000000 + dx)::text )::uuid,
            dx + 1000000 * dy,                                                    --id_int
            dx + 1000000 * dy,                                                    --id_bigint
            dx || E\' ot\\'her \' || dy,                                          --id_str
            (\'192.168.0.1\'::inet + dx + 100 * dy )::inet,                       --id_inet4
            (\'2001:4f8:3:ba:2e0:81ff:fe22:d1f1\'::inet + dx + 100 * dy )::inet,  --id_inet6
            (\'192.168.0.1\'::cidr + dx + 100 * dy )::cidr,                       --id_cidr4
            (\'2001:4f8:3:ba:2e0:81ff:fe22:d1f1\'::cidr + dx + 100 * dy )::cidr,  --id_cidr6
            ((112233445566 + dx + 100 * dy)::text)::macaddr,                      --id_macaddr
            ((1122334455667788 + dx + 100 * dy)::text)::macaddr8,                 --id_macaddr8
            now() - ((dx||\' hour\')::text)::interval - ((dy||\' day\')::text)::interval,
            NULLIF( ( (10000000)::text || (100000000000 + dy)::text || (100000000000 + dx)::text )::uuid,
              ( (10000000)::text || (100000000000 + dy + dy%2)::text || (100000000000 + dx)::text )::uuid ),
            NULL,
            ST_Translate(
              ST_GeomFromText(\'POLYGON((3396900.0 6521800.0,3396900.0 6521870.0,
                  3396830.0 6521870.0,3396830.0 6521800.0,3396900.0 6521800.0))\', 3857 ),
              100.0 * dx,
              100.0 * dy )
          FROM generate_series(1,3) dx, generate_series(1,3) dy;
        REFRESH MATERIALIZED VIEW qgis_test.multi_column_pk_small_data_mat_view;''')

        pk_col_list = ("id_serial", "id_uuid", "id_int", "id_bigint", "id_str", "id_inet4", "id_inet6", "id_cidr4", "id_cidr6", "id_macaddr", "id_macaddr8")
        test_type_list = ["table", "view", "mat_view"]
        for n in [1, 2, len(pk_col_list)]:
            pk_col_set_list = list(combinations(pk_col_list, n))
            for pk_col_set in pk_col_set_list:
                test_for_pk_combinations(test_type_list, pk_col_set, 7)

        for col_name in ["id_serial", "id_uuid", "id_int", "id_bigint", "id_str", "id_inet4"]:
            test_for_pk_combinations(["view", "mat_view"], ["id_half_null_uuid", col_name], 7)
            test_for_pk_combinations(["view", "mat_view"], ["id_all_null_uuid", col_name], 7)

    def testChangeAttributeWithDefaultValue(self):
        """Test that we can change an attribute value with its default value"""

        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn = md.createConnection(self.dbconn, {})

        # Cleanup
        try:
            conn.dropVectorTable('qgis_test', 'test_change_att_w_default_value')
        except QgsProviderConnectionException:
            pass

        conn.executeSql('''
        CREATE TABLE "qgis_test"."test_change_att_w_default_value" (
            id serial primary key,
            thetext1 character varying(8) DEFAULT NULL::character varying,
            thetext2 character varying(8) DEFAULT NULL,
            thetext3 character varying(8) DEFAULT 'blabla',
            thenumber integer DEFAULT 2+2
        );''')

        conn.executeSql('''
        INSERT INTO "qgis_test"."test_change_att_w_default_value" (thetext1,thetext2,thetext3,thenumber) VALUES ('test1','test2','test3',6);''')

        layer = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'id\'table="qgis_test"."test_change_att_w_default_value" sql=', 'test', 'postgres')
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.featureCount(), 1)
        feat = next(layer.getFeatures())
        self.assertTrue(feat["thetext1"], "test1")
        self.assertTrue(feat["thetext2"], "test2")
        self.assertTrue(feat["thetext3"], "test3")
        self.assertTrue(feat["thenumber"], 6)

        self.assertEqual(layer.dataProvider().defaultValueClause(1), "NULL::character varying")
        self.assertEqual(layer.dataProvider().defaultValueClause(2), "NULL::character varying")
        self.assertEqual(layer.dataProvider().defaultValueClause(3), "'blabla'::character varying")
        self.assertEqual(layer.dataProvider().defaultValueClause(4), "(2 + 2)")

        layer.startEditing()
        self.assertTrue(layer.changeAttributeValues(1, {1: "NULL::character varying", 2: "NULL::character varying",
                                                        3: "'blabla'::character varying", 4: "(2 + 2)"}))
        self.assertTrue(layer.commitChanges())

        feat = next(layer.getFeatures())

        # print( "hein |{}| expected=|{}| bool={}".format( feat["thetext"], expected, (feat["thetext"] expected) ) )
        self.assertEqual(feat["thetext1"], NULL)
        self.assertEqual(feat["thetext2"], NULL)
        self.assertEqual(feat["thetext3"], "blabla")
        self.assertEqual(feat["thenumber"], 4)

    def testExtractWithinDistanceAlgorithm(self):

        with self.temporarySchema('extract_within_distance') as schema:

            # Create and populate target table in PseudoWebMercator CRS
            self.execSQLCommand(
                'CREATE TABLE {}.target_3857 (id serial primary key, g geometry(linestring, 3857))'
                .format(schema))
            # -- first line (id=1)
            self.execSQLCommand(
                "INSERT INTO {}.target_3857 (g) values('SRID=3857;LINESTRING(0 0, 1000 1000)')"
                .format(schema))
            # -- secodn line is a great circle line on the right (id=2)
            self.execSQLCommand(
                "INSERT INTO {}.target_3857 (g) values( ST_Transform('SRID=4326;LINESTRING(80 0,160 80)'::geometry, 3857) )"
                .format(schema))

            # Create and populate reference table in PseudoWebMercator CRS
            self.execSQLCommand(
                'CREATE TABLE {}.reference_3857 (id serial primary key, g geometry(point, 3857))'
                .format(schema))
            self.execSQLCommand(
                "INSERT INTO {}.reference_3857 (g) values('SRID=3857;POINT(500 999)')"
                .format(schema))
            self.execSQLCommand(
                "INSERT INTO {}.reference_3857 (g) values('SRID=3857;POINT(501 999)')"
                .format(schema))
            self.execSQLCommand(
                # -- this reference (id=3) is ON the first line (id=1) in webmercator
                # -- and probably very close in latlong WGS84
                "INSERT INTO {}.reference_3857 (g) values('SRID=3857;POINT(500 500)')"
                .format(schema))
            self.execSQLCommand(
                # -- this reference (id=4) is at ~ 5 meters from second line (id=2) in webmercator
                "INSERT INTO {}.reference_3857 (g) values('SRID=3857;POINT(12072440.688888172 5525668.358321408)')"
                .format(schema))
            self.execSQLCommand(
                "INSERT INTO {}.reference_3857 (g) values('SRID=3857;POINT(503 999)')"
                .format(schema))
            self.execSQLCommand(
                "INSERT INTO {}.reference_3857 (g) values('SRID=3857;POINT(504 999)')"
                .format(schema))

            # Create and populate target and reference table in WGS84 latlong
            self.execSQLCommand(
                'CREATE TABLE {0}.target_4326 AS SELECT id, ST_Transform(g, 4326)::geometry(linestring,4326) as g FROM {0}.target_3857'
                .format(schema))
            self.execSQLCommand(
                'CREATE TABLE {0}.reference_4326 AS SELECT id, ST_Transform(g, 4326)::geometry(point,4326) as g FROM {0}.reference_3857'
                .format(schema))

            # Create target and reference layers
            vl_target_3857 = QgsVectorLayer(
                '{} sslmode=disable key=id srid=3857 type=LINESTRING table="{}"."target_3857" (g) sql='
                .format(self.dbconn, schema),
                'target_3857', 'postgres')
            self.assertTrue(vl_target_3857.isValid(), "Could not create a layer from the '{}.target_3857' table using dbconn '{}'" .format(schema, self.dbconn))
            vl_reference_3857 = QgsVectorLayer(
                '{} sslmode=disable key=id srid=3857 type=POINT table="{}"."reference_3857" (g) sql='
                .format(self.dbconn, schema),
                'reference_3857', 'postgres')
            self.assertTrue(vl_reference_3857.isValid(), "Could not create a layer from the '{}.reference_3857' table using dbconn '{}'" .format(schema, self.dbconn))
            vl_target_4326 = QgsVectorLayer(
                '{} sslmode=disable key=id srid=4326 type=LINESTRING table="{}"."target_4326" (g) sql='
                .format(self.dbconn, schema),
                'target_4326', 'postgres')
            self.assertTrue(vl_target_4326.isValid(), "Could not create a layer from the '{}.target_4326' table using dbconn '{}'" .format(schema, self.dbconn))
            vl_reference_4326 = QgsVectorLayer(
                '{} sslmode=disable key=id srid=4326 type=POINT table="{}"."reference_4326" (g) sql='
                .format(self.dbconn, schema),
                'reference_4326', 'postgres')
            self.assertTrue(vl_reference_4326.isValid(), "Could not create a layer from the '{}.reference_4326' table using dbconn '{}'" .format(schema, self.dbconn))

            # Create the ExtractWithinDistance algorithm
            # TODO: move registry initialization in class initialization ?
            QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())
            registry = QgsApplication.instance().processingRegistry()
            alg = registry.createAlgorithmById('native:extractwithindistance')
            self.assertIsNotNone(alg)

            # Utility feedback and context objects
            class ConsoleFeedBack(QgsProcessingFeedback):
                _error = ''

                def reportError(self, error, fatalError=False):
                    self._error = error
                    print(error)

            feedback = ConsoleFeedBack()
            context = QgsProcessingContext()

            # ----------------------------------------------------------------
            # Run the ExtractWithinDistance algorithm with matching
            # target_3857 and reference_3857 CRSs

            parameters = {
                # extract features from here:
                'INPUT': vl_target_3857,
                # extracted features must be within given
                # distance from this layer:
                'REFERENCE': vl_reference_3857,
                # distance (in INPUT units)
                'DISTANCE': 10,  # meters
                'OUTPUT': 'memory:result'
            }

            # Note: the following returns true also in case of errors ...
            result = alg.run(parameters, context, feedback)
            self.assertEqual(result[1], True)

            result_layer_name = result[0]['OUTPUT']
            vl_result = QgsProcessingUtils.mapLayerFromString(result_layer_name, context)
            self.assertTrue(vl_result.isValid())

            extracted_fids = [f['id'] for f in vl_result.getFeatures()]
            self.assertEqual(set(extracted_fids), {1, 2})

            # ----------------------------------------------------------------
            # Run the ExtractWithinDistance algorithm with matching
            # target_4326 and reference_4326 CRSs

            parameters = {
                # extract features from here:
                'INPUT': vl_target_4326,
                # extracted features must be within given
                # distance from this layer:
                'REFERENCE': vl_reference_4326,
                # distance (in INPUT units)
                'DISTANCE': 9e-5,  # degrees
                'OUTPUT': 'memory:result'
            }

            # Note: the following returns true also in case of errors ...
            result = alg.run(parameters, context, feedback)
            self.assertEqual(result[1], True)

            result_layer_name = result[0]['OUTPUT']
            vl_result = QgsProcessingUtils.mapLayerFromString(result_layer_name, context)
            self.assertTrue(vl_result.isValid())

            extracted_fids = [f['id'] for f in vl_result.getFeatures()]
            self.assertEqual(set(extracted_fids), {1})

            # ----------------------------------------------------------------
            # Run the ExtractWithinDistance algorithm with
            # target in 4326 CRS and reference in 3857 CRS

            parameters = {
                # extract features from here:
                'INPUT': vl_target_4326,
                # extracted features must be within given
                # distance from this layer:
                'REFERENCE': vl_reference_3857,
                # distance (in INPUT units)
                'DISTANCE': 9e-5,  # degrees
                'OUTPUT': 'memory:result'
            }

            # Note: the following returns true also in case of errors ...
            result = alg.run(parameters, context, feedback)
            self.assertEqual(result[1], True)

            result_layer_name = result[0]['OUTPUT']
            vl_result = QgsProcessingUtils.mapLayerFromString(result_layer_name, context)
            self.assertTrue(vl_result.isValid())

            extracted_fids = [f['id'] for f in vl_result.getFeatures()]
            self.assertEqual(set(extracted_fids), {1})

            # ----------------------------------------------------------------
            # Run the ExtractWithinDistance algorithm with
            # target in 3857 CRS and reference in 4326 CRS

            parameters = {
                # extract features from here:
                'INPUT': vl_target_3857,
                # extracted features must be within given
                # distance from this layer:
                'REFERENCE': vl_reference_4326,
                # distance (in INPUT units)
                'DISTANCE': 10,  # meters
                'OUTPUT': 'memory:result'
            }

            # Note: the following returns true also in case of errors ...
            result = alg.run(parameters, context, feedback)
            self.assertEqual(result[1], True)

            result_layer_name = result[0]['OUTPUT']
            vl_result = QgsProcessingUtils.mapLayerFromString(result_layer_name, context)
            self.assertTrue(vl_result.isValid())

            extracted_fids = [f['id'] for f in vl_result.getFeatures()]
            self.assertEqual(set(extracted_fids), {1, 2})  # Bug ?


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

    def testUnrestrictedGeometryType(self):
        """Test geometry column with no explicit geometry type, regression GH #38565"""

        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn = md.createConnection(self.dbconn, {})

        # Cleanup if needed
        try:
            conn.dropVectorTable('qgis_test', 'test_unrestricted_geometry')
        except QgsProviderConnectionException:
            pass

        conn.executeSql('''
        CREATE TABLE "qgis_test"."test_unrestricted_geometry" (
            gid serial primary key,
            geom geometry(Geometry, 4326)
        );''')

        points = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'gid\' srid=4326 type=POINT table="qgis_test"."test_unrestricted_geometry" (geom) sql=', 'test_points', 'postgres')
        lines = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'gid\' srid=4326 type=LINESTRING table="qgis_test"."test_unrestricted_geometry" (geom) sql=', 'test_lines', 'postgres')
        polygons = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'gid\' srid=4326 type=POLYGON table="qgis_test"."test_unrestricted_geometry" (geom) sql=', 'test_polygons', 'postgres')

        self.assertTrue(points.isValid())
        self.assertTrue(lines.isValid())
        self.assertTrue(polygons.isValid())

        f = QgsFeature(points.fields())
        f.setGeometry(QgsGeometry.fromWkt('point(9 45)'))
        self.assertTrue(points.dataProvider().addFeatures([f]))
        self.assertEqual(points.featureCount(), 1)
        self.assertEqual(lines.featureCount(), 0)
        self.assertEqual(polygons.featureCount(), 0)

        # Fetch from iterator
        self.assertTrue(compareWkt(next(points.getFeatures()).geometry().asWkt(), 'point(9 45)'))
        with self.assertRaises(StopIteration):
            next(lines.getFeatures())
        with self.assertRaises(StopIteration):
            next(polygons.getFeatures())

        f.setGeometry(QgsGeometry.fromWkt('linestring(9 45, 10 46)'))
        self.assertTrue(lines.dataProvider().addFeatures([f]))
        self.assertEqual(points.featureCount(), 1)
        self.assertEqual(lines.featureCount(), 1)
        self.assertEqual(polygons.featureCount(), 0)

        # Fetch from iterator
        self.assertTrue(compareWkt(next(points.getFeatures()).geometry().asWkt(), 'point(9 45)'))
        self.assertTrue(compareWkt(next(lines.getFeatures()).geometry().asWkt(), 'linestring(9 45, 10 46)'))
        with self.assertRaises(StopIteration):
            next(polygons.getFeatures())

        # Test regression GH #38567 (no SRID requested in the data source URI)
        # Cleanup if needed
        conn.executeSql('DELETE FROM "qgis_test"."test_unrestricted_geometry" WHERE \'t\'')

        points = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'gid\' type=POINT table="qgis_test"."test_unrestricted_geometry" (geom) sql=', 'test_points', 'postgres')
        lines = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'gid\' type=LINESTRING table="qgis_test"."test_unrestricted_geometry" (geom) sql=', 'test_lines', 'postgres')
        polygons = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'gid\' type=POLYGON table="qgis_test"."test_unrestricted_geometry" (geom) sql=', 'test_polygons', 'postgres')

        self.assertTrue(points.isValid())
        self.assertTrue(lines.isValid())
        self.assertTrue(polygons.isValid())

    def test_read_wkb(self):
        """ Test to read WKB from Postgis. """
        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn = md.createConnection(self.dbconn, {})
        results = conn.executeSql("SELECT ST_AsBinary(ST_MakePoint(5, 10));")
        wkb = results[0][0]
        geom = QgsGeometry()
        import binascii
        geom.fromWkb(binascii.unhexlify(wkb[2:]))
        self.assertEqual(geom.asWkt(), "Point (5 10)")

    def testTrustFlag(self):
        """Test regression https://github.com/qgis/QGIS/issues/38809"""

        vl = QgsVectorLayer(
            self.dbconn +
            ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."editData" (geom) sql=',
            'testTrustFlag', 'postgres')

        self.assertTrue(vl.isValid())

        p = QgsProject.instance()
        d = QTemporaryDir()
        dir_path = d.path()

        self.assertTrue(p.addMapLayers([vl]))
        project_path = os.path.join(dir_path, 'testTrustFlag.qgs')
        self.assertTrue(p.write(project_path))

        del vl

        p.clear()
        self.assertTrue(p.read(project_path))
        vl = p.mapLayersByName('testTrustFlag')[0]
        self.assertTrue(vl.isValid())
        self.assertFalse(p.trustLayerMetadata())

        # Set the trust flag
        p.setTrustLayerMetadata(True)
        self.assertTrue(p.write(project_path))

        # Re-read
        p.clear()
        self.assertTrue(p.read(project_path))
        self.assertTrue(p.trustLayerMetadata())
        vl = p.mapLayersByName('testTrustFlag')[0]
        self.assertTrue(vl.isValid())

    def testQueryLayerDuplicatedFields(self):
        """Test that duplicated fields from a query layer are returned"""

        def _get_layer(sql):
            return QgsVectorLayer(
                self.dbconn +
                ' sslmode=disable key=\'__rid__\' table=\'(SELECT row_number() OVER () AS __rid__, * FROM (' + sql + ') as foo)\'  sql=',
                'test', 'postgres')

        l = _get_layer('SELECT 1, 2')
        self.assertEqual(l.fields().count(), 3)
        self.assertEqual([f.name() for f in l.fields()], ['__rid__', '?column?', '?column? (2)'])

        l = _get_layer('SELECT 1 as id, 2 as id')
        self.assertEqual(l.fields().count(), 3)
        self.assertEqual([f.name() for f in l.fields()], ['__rid__', 'id', 'id (2)'])

    def testInsertOnlyFieldIsEditable(self):
        """Test issue #40922 when an INSERT only use cannot insert a new feature"""

        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn = md.createConnection(self.dbconn, {})
        conn.executeSql('DROP TABLE IF EXISTS public.insert_only_points')
        conn.executeSql('DROP USER IF EXISTS insert_only_user')
        conn.executeSql('CREATE USER insert_only_user WITH PASSWORD \'insert_only_user\'')
        conn.executeSql('CREATE TABLE insert_only_points (id SERIAL PRIMARY KEY, name VARCHAR(64))')
        conn.executeSql("SELECT AddGeometryColumn('public', 'insert_only_points', 'geom', 4326, 'POINT', 2 )")
        conn.executeSql('GRANT SELECT ON "public"."insert_only_points" TO insert_only_user')

        uri = QgsDataSourceUri(self.dbconn +
                               ' sslmode=disable  key=\'id\'srid=4326 type=POINT table="public"."insert_only_points" (geom) sql=')
        uri.setUsername('insert_only_user')
        uri.setPassword('insert_only_user')
        vl = QgsVectorLayer(uri.uri(), 'test', 'postgres')
        self.assertTrue(vl.isValid())
        self.assertFalse(vl.startEditing())

        feature = QgsFeature(vl.fields())
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 0, feature))
        self.assertFalse(QgsVectorLayerUtils.fieldIsEditable(vl, 1, feature))

        conn.executeSql('GRANT INSERT ON "public"."insert_only_points" TO insert_only_user')
        vl = QgsVectorLayer(uri.uri(), 'test', 'postgres')

        feature = QgsFeature(vl.fields())
        self.assertTrue(vl.startEditing())
        self.assertTrue(QgsVectorLayerUtils.fieldIsEditable(vl, 0, feature))
        self.assertTrue(QgsVectorLayerUtils.fieldIsEditable(vl, 1, feature))

    def testPkeyIntArray(self):
        """
        Test issue #42778 when pkey is an int array
        """
        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn = md.createConnection(self.dbconn, {})
        conn.executeSql('DROP TABLE IF EXISTS public.test_pkey_intarray')
        conn.executeSql('CREATE TABLE public.test_pkey_intarray (id _int8 PRIMARY KEY, name VARCHAR(64))')
        conn.executeSql("""INSERT INTO public.test_pkey_intarray (id, name) VALUES('{0,0,19111815}', 'test')""")

        uri = QgsDataSourceUri(self.dbconn +
                               ' sslmode=disable  key=\'id\' table="public"."test_pkey_intarray" sql=')
        vl = QgsVectorLayer(uri.uri(), 'test', 'postgres')
        self.assertTrue(vl.isValid())

        feat = next(vl.getFeatures())
        self.assertTrue(feat.isValid())
        self.assertEqual(feat["name"], "test")

        fid = feat.id()
        self.assertTrue(fid > 0)

        feat = vl.getFeature(fid)
        self.assertTrue(feat.isValid())
        self.assertEqual(feat["name"], "test")

    def testExportPkGuessLogic(self):
        """Test that when creating an empty layer a NOT NULL UNIQUE numeric field is identified as a PK"""

        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn = md.createConnection(self.dbconn, {})
        conn.executeSql(
            'DROP TABLE IF EXISTS qgis_test."testExportPkGuessLogic_source" CASCADE')
        conn.executeSql(
            'DROP TABLE IF EXISTS qgis_test."testExportPkGuessLogic_exported" CASCADE')
        conn.executeSql(
            """CREATE TABLE qgis_test."testExportPkGuessLogic_source" ( id bigint generated always as identity primary key,
                    geom geometry(Point, 4326) check (st_isvalid(geom)),
                    name text unique, author text not null)""")

        source_layer = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'id\' srid=4326 type=POINT table="qgis_test"."testExportPkGuessLogic_source" (geom) sql=', 'testExportPkGuessLogic_source', 'postgres')

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        conn = md.createConnection(self.dbconn, {})
        table = conn.table("qgis_test", "testExportPkGuessLogic_source")
        self.assertEqual(table.primaryKeyColumns(), ['id'])

        self.assertTrue(source_layer.isValid())

        # Create the URI as the browser does (no PK information)
        uri = self.dbconn + ' sslmode=disable srid=4326 type=POINT table="qgis_test"."testExportPkGuessLogic_exported" (geom) sql='

        exporter = QgsVectorLayerExporter(uri, 'postgres', source_layer.fields(), source_layer.wkbType(), source_layer.crs(), True, {})
        self.assertTrue(exporter.lastError() == '')

        exported_layer = QgsVectorLayer(self.dbconn + ' sslmode=disable srid=4326 type=POINT table="qgis_test"."testExportPkGuessLogic_exported" (geom) sql=', 'testExportPkGuessLogic_exported', 'postgres')
        self.assertTrue(exported_layer.isValid())

        table = conn.table("qgis_test", "testExportPkGuessLogic_exported")
        self.assertEqual(table.primaryKeyColumns(), ['id'])

        self.assertEqual(exported_layer.fields().names(), ['id', 'name', 'author'])

    def testEwkt(self):
        vl = QgsVectorLayer(f'{self.dbconn} table="qgis_test"."someData" sql=', "someData", "postgres")
        tg = QgsTransactionGroup()
        tg.addLayer(vl)

        feature = next(vl.getFeatures())
        # make sure we get a QgsReferenceGeometry and not "just" a string
        self.assertEqual(feature['geom'].crs().authid(), 'EPSG:4326')

        vl.startEditing()

        # Layer accepts a referenced geometry
        feature['geom'] = QgsReferencedGeometry(QgsGeometry.fromWkt('POINT(70 70)'), QgsCoordinateReferenceSystem.fromEpsgId(4326))
        self.assertTrue(vl.updateFeature(feature))

        # Layer will accept null geometry
        feature['geom'] = QgsReferencedGeometry()
        self.assertTrue(vl.updateFeature(feature))

        # Layer will not accept invalid crs
        feature['geom'] = QgsReferencedGeometry(QgsGeometry.fromWkt('POINT(1 1)'), QgsCoordinateReferenceSystem())
        self.assertFalse(vl.updateFeature(feature))

        # EWKT strings are accepted too
        feature['geom'] = 'SRID=4326;Point (71 78)'
        self.assertTrue(vl.updateFeature(feature))

        # addFeature
        feature['pk'] = 8
        self.assertTrue(vl.addFeature(feature))

        # changeAttributeValue
        geom = QgsReferencedGeometry(QgsGeometry.fromWkt('POINT(3 3)'), QgsCoordinateReferenceSystem.fromEpsgId(4326))

        feature['pk'] = 8
        self.assertTrue(vl.changeAttributeValue(8, 8, geom))
        self.assertEqual(vl.getFeature(8)['geom'].asWkt(), geom.asWkt())
        self.assertEqual(vl.getFeature(8)['geom'].crs(), geom.crs())


if __name__ == '__main__':
    unittest.main()
