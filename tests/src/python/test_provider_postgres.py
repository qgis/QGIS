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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import psycopg2

import os
import time

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
    QgsGeometry
)
from qgis.gui import QgsGui, QgsAttributeForm
from qgis.PyQt.QtCore import QDate, QTime, QDateTime, QVariant, QDir, QObject
from qgis.PyQt.QtWidgets import QLabel
from qgis.testing import start_app, unittest
from qgis.PyQt.QtXml import QDomDocument
from utilities import unitTestDataPath
from providertestbase import ProviderTestCase

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsPostgresProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.dbconn = 'dbname=\'qgis_test\''
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        # Create test layers
        cls.vl = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=', 'test', 'postgres')
        assert cls.vl.isValid()
        cls.source = cls.vl.dataProvider()
        cls.poly_vl = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=', 'test', 'postgres')
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
        self.execSQLCommand('DROP TABLE IF EXISTS qgis_test."editData" CASCADE')
        self.execSQLCommand('CREATE TABLE qgis_test."editData" ( pk SERIAL NOT NULL PRIMARY KEY, cnt integer, name text, name2 text, num_char text, geom public.geometry(Point, 4326))')
        self.execSQLCommand("INSERT INTO qgis_test.\"editData\" (pk, cnt, name, name2, num_char, geom) VALUES "
                            "(5, -200, NULL, 'NuLl', '5', '0101000020E61000001D5A643BDFC751C01F85EB51B88E5340'),"
                            "(3, 300, 'Pear', 'PEaR', '3', NULL),"
                            "(1, 100, 'Orange', 'oranGe', '1', '0101000020E61000006891ED7C3F9551C085EB51B81E955040'),"
                            "(2, 200, 'Apple', 'Apple', '2', '0101000020E6100000CDCCCCCCCC0C51C03333333333B35140'),"
                            "(4, 400, 'Honey', 'Honey', '4', '0101000020E610000014AE47E17A5450C03333333333935340')")
        vl = QgsVectorLayer(
            self.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."editData" (geom) sql=',
            'test', 'postgres')
        return vl

    def getEditableLayer(self):
        return self.getSource()

    def enableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', True)
        return True

    def disableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', False)

    def uncompiledFilters(self):
        return set([])

    def partiallyCompiledFilters(self):
        return set([])

    # HERE GO THE PROVIDER SPECIFIC TESTS
    def testDefaultValue(self):
        self.source.setProviderProperty(QgsDataProvider.EvaluateDefaultValues, True)
        self.assertIsInstance(self.source.defaultValue(0), int)
        self.assertEqual(self.source.defaultValue(1), NULL)
        self.assertEqual(self.source.defaultValue(2), 'qgis')
        self.source.setProviderProperty(QgsDataProvider.EvaluateDefaultValues, False)

    def testDefaultValueClause(self):
        self.source.setProviderProperty(QgsDataProvider.EvaluateDefaultValues, False)
        self.assertEqual(self.source.defaultValueClause(0), 'nextval(\'qgis_test."someData_pk_seq"\'::regclass)')
        self.assertFalse(self.source.defaultValueClause(1))
        self.assertEqual(self.source.defaultValueClause(2), '\'qgis\'::text')

    def testDateTimeTypes(self):
        vl = QgsVectorLayer('%s table="qgis_test"."date_times" sql=' % (self.dbconn), "testdatetimes", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('date_field')).type(), QVariant.Date)
        self.assertEqual(fields.at(fields.indexFromName('time_field')).type(), QVariant.Time)
        self.assertEqual(fields.at(fields.indexFromName('datetime_field')).type(), QVariant.DateTime)

        f = next(vl.getFeatures(QgsFeatureRequest()))

        date_idx = vl.fields().lookupField('date_field')
        self.assertIsInstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2004, 3, 4))
        time_idx = vl.fields().lookupField('time_field')
        self.assertIsInstance(f.attributes()[time_idx], QTime)
        self.assertEqual(f.attributes()[time_idx], QTime(13, 41, 52))
        datetime_idx = vl.fields().lookupField('datetime_field')
        self.assertIsInstance(f.attributes()[datetime_idx], QDateTime)
        self.assertEqual(f.attributes()[datetime_idx], QDateTime(QDate(2004, 3, 4), QTime(13, 41, 52)))

    def testBooleanType(self):
        vl = QgsVectorLayer('{} table="qgis_test"."boolean_table" sql='.format(self.dbconn), "testbool", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('fld1')).type(), QVariant.Bool)

        values = {feat['id']: feat['fld1'] for feat in vl.getFeatures()}
        expected = {
            1: True,
            2: False,
            3: NULL
        }
        self.assertEqual(values, expected)

    def testQueryLayers(self):
        def test_query(dbconn, query, key):
            ql = QgsVectorLayer('%s srid=4326 table="%s" (geom) key=\'%s\' sql=' % (dbconn, query.replace('"', '\\"'), key), "testgeom", "postgres")
            self.assertTrue(ql.isValid(), '{} ({})'.format(query, key))

        test_query(self.dbconn, '(SELECT NULL::integer "Id1", NULL::integer "Id2", NULL::geometry(Point, 4326) geom LIMIT 0)', '"Id1","Id2"')

    def testWkbTypes(self):
        def test_table(dbconn, table_name, wkt):
            vl = QgsVectorLayer('%s srid=4326 table="qgis_test".%s (geom) sql=' % (dbconn, table_name), "testgeom", "postgres")
            self.assertTrue(vl.isValid())
            for f in vl.getFeatures():
                self.assertEqual(f.geometry().asWkt(), wkt)

        test_table(self.dbconn, 'p2d', 'Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')
        test_table(self.dbconn, 'p3d', 'PolygonZ ((0 0 0, 1 0 0, 1 1 0, 0 1 0, 0 0 0))')
        test_table(self.dbconn, 'triangle2d', 'Polygon ((0 0, 1 0, 1 1, 0 0))')
        test_table(self.dbconn, 'triangle3d', 'PolygonZ ((0 0 0, 1 0 0, 1 1 0, 0 0 0))')
        test_table(self.dbconn, 'tin2d', 'MultiPolygon (((0 0, 1 0, 1 1, 0 0)),((0 0, 0 1, 1 1, 0 0)))')
        test_table(self.dbconn, 'tin3d', 'MultiPolygonZ (((0 0 0, 1 0 0, 1 1 0, 0 0 0)),((0 0 0, 0 1 0, 1 1 0, 0 0 0)))')
        test_table(self.dbconn, 'ps2d', 'MultiPolygon (((0 0, 1 0, 1 1, 0 1, 0 0)))')
        test_table(self.dbconn, 'ps3d', 'MultiPolygonZ (((0 0 0, 0 1 0, 1 1 0, 1 0 0, 0 0 0)),((0 0 1, 1 0 1, 1 1 1, 0 1 1, 0 0 1)),((0 0 0, 0 0 1, 0 1 1, 0 1 0, 0 0 0)),((0 1 0, 0 1 1, 1 1 1, 1 1 0, 0 1 0)),((1 1 0, 1 1 1, 1 0 1, 1 0 0, 1 1 0)),((1 0 0, 1 0 1, 0 0 1, 0 0 0, 1 0 0)))')
        test_table(self.dbconn, 'mp3d', 'MultiPolygonZ (((0 0 0, 0 1 0, 1 1 0, 1 0 0, 0 0 0)),((0 0 1, 1 0 1, 1 1 1, 0 1 1, 0 0 1)),((0 0 0, 0 0 1, 0 1 1, 0 1 0, 0 0 0)),((0 1 0, 0 1 1, 1 1 1, 1 1 0, 0 1 0)),((1 1 0, 1 1 1, 1 0 1, 1 0 0, 1 1 0)),((1 0 0, 1 0 1, 0 0 1, 0 0 0, 1 0 0)))')
        test_table(self.dbconn, 'pt2d', 'Point (0 0)')
        test_table(self.dbconn, 'pt3d', 'PointZ (0 0 0)')
        test_table(self.dbconn, 'ls2d', 'LineString (0 0, 1 1)')
        test_table(self.dbconn, 'ls3d', 'LineStringZ (0 0 0, 1 1 1)')
        test_table(self.dbconn, 'mpt2d', 'MultiPoint ((0 0),(1 1))')
        test_table(self.dbconn, 'mpt3d', 'MultiPointZ ((0 0 0),(1 1 1))')
        test_table(self.dbconn, 'mls2d', 'MultiLineString ((0 0, 1 1),(2 2, 3 3))')
        test_table(self.dbconn, 'mls3d', 'MultiLineStringZ ((0 0 0, 1 1 1),(2 2 2, 3 3 3))')

        test_table(self.dbconn, 'pt4d', 'PointZM (1 2 3 4)')

    def testMetadata(self):
        """ Test that metadata is correctly acquired from provider """
        metadata = self.vl.metadata()
        self.assertEqual(metadata.crs(), QgsCoordinateReferenceSystem.fromEpsgId(4326))
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

        vl = QgsVectorLayer('%s srid=4326 table="qgis_test".%s (geom) sql=' % (self.dbconn, 'someData'), "testgeom", "postgres")
        self.assertTrue(vl.isValid())
        # Test someData
        test_unique([f for f in vl.getFeatures()], 5)

        # Test base_table_bad: layer is invalid
        vl = QgsVectorLayer('%s srid=4326 table="qgis_test".%s (geom) sql=' % (self.dbconn, 'base_table_bad'), "testgeom", "postgres")
        self.assertFalse(vl.isValid())
        # Test base_table_bad with use estimated metadata: layer is valid because the unique test is skipped
        vl = QgsVectorLayer('%s srid=4326 estimatedmetadata="true" table="qgis_test".%s (geom) sql=' % (self.dbconn, 'base_table_bad'), "testgeom", "postgres")
        self.assertTrue(vl.isValid())

        # Test base_table_good: layer is valid
        vl = QgsVectorLayer('%s srid=4326 table="qgis_test".%s (geom) sql=' % (self.dbconn, 'base_table_good'), "testgeom", "postgres")
        self.assertTrue(vl.isValid())
        test_unique([f for f in vl.getFeatures()], 4)
        # Test base_table_good with use estimated metadata: layer is valid
        vl = QgsVectorLayer('%s srid=4326 estimatedmetadata="true" table="qgis_test".%s (geom) sql=' % (self.dbconn, 'base_table_good'), "testgeom", "postgres")
        self.assertTrue(vl.isValid())
        test_unique([f for f in vl.getFeatures()], 4)

    # See https://issues.qgis.org/issues/14262
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
        test(self.dbconn, '(SELECT -1::int2 i, NULL::geometry(Point) g)', 'i', -1, 4294967295)
        # max positive signed 16bit integer
        test(self.dbconn, '(SELECT 32767::int2 i, NULL::geometry(Point) g)', 'i', 32767, 32767)
        # max negative signed 16bit integer
        test(self.dbconn, '(SELECT (-32768)::int2 i, NULL::geometry(Point) g)', 'i', -32768, 4294934528)

        # --- INT32 ----
        # zero
        test(self.dbconn, '(SELECT 0::int4 i, NULL::geometry(Point) g)', 'i', 0, 0)
        # low positive
        test(self.dbconn, '(SELECT 2::int4 i, NULL::geometry(Point) g)', 'i', 2, 2)
        # low negative
        test(self.dbconn, '(SELECT -2::int4 i, NULL::geometry(Point) g)', 'i', -2, 4294967294)
        # max positive signed 32bit integer
        test(self.dbconn, '(SELECT 2147483647::int4 i, NULL::geometry(Point) g)', 'i', 2147483647, 2147483647)
        # max negative signed 32bit integer
        test(self.dbconn, '(SELECT (-2147483648)::int4 i, NULL::geometry(Point) g)', 'i', -2147483648, 2147483648)

        # --- INT64 (FIDs are always 1 because assigned ex-novo) ----
        # zero
        test(self.dbconn, '(SELECT 0::int8 i, NULL::geometry(Point) g)', 'i', 0, 1)
        # low positive
        test(self.dbconn, '(SELECT 3::int8 i, NULL::geometry(Point) g)', 'i', 3, 1)
        # low negative
        test(self.dbconn, '(SELECT -3::int8 i, NULL::geometry(Point) g)', 'i', -3, 1)
        # max positive signed 64bit integer
        test(self.dbconn, '(SELECT 9223372036854775807::int8 i, NULL::geometry(Point) g)', 'i', 9223372036854775807, 1)
        # max negative signed 32bit integer
        test(self.dbconn, '(SELECT (-9223372036854775808)::int8 i, NULL::geometry(Point) g)', 'i', -9223372036854775808, 1)

    def testPktIntInsert(self):
        vl = QgsVectorLayer('{} table="qgis_test"."{}" key="pk" sql='.format(self.dbconn, 'bikes_view'), "bikes_view", "postgres")
        self.assertTrue(vl.isValid())
        f = QgsFeature(vl.fields())
        f['pk'] = NULL
        f['name'] = 'Cilo'
        r, f = vl.dataProvider().addFeatures([f])
        self.assertTrue(r)
        self.assertNotEqual(f[0]['pk'], NULL, f[0].attributes())
        vl.deleteFeatures([f[0].id()])

    def testPktMapInsert(self):
        vl = QgsVectorLayer('{} table="qgis_test"."{}" key="obj_id" sql='.format(self.dbconn, 'oid_serial_table'), "oid_serial", "postgres")
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
        vl = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'gid\' table="qgis_test"."constraints" sql=', 'test1', 'postgres')
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
        vl = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=', 'test', 'postgres')
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
        vl = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=', 'test', 'postgres')
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

    def testTransactionConstrains(self):
        # create a vector layer based on postgres
        vl = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'id\' table="qgis_test"."check_constraints" sql=', 'test', 'postgres')
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
        vl = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=', 'test', 'postgres')
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

        vl = QgsVectorLayer('%s table="qgis_test"."domains" sql=' % (self.dbconn), "domains", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()

        expected = {}
        expected['fld_var_char_domain'] = {'type': QVariant.String, 'typeName': 'qgis_test.var_char_domain', 'length': -1}
        expected['fld_var_char_domain_6'] = {'type': QVariant.String, 'typeName': 'qgis_test.var_char_domain_6', 'length': 6}
        expected['fld_character_domain'] = {'type': QVariant.String, 'typeName': 'qgis_test.character_domain', 'length': 1}
        expected['fld_character_domain_6'] = {'type': QVariant.String, 'typeName': 'qgis_test.character_domain_6', 'length': 6}
        expected['fld_char_domain'] = {'type': QVariant.String, 'typeName': 'qgis_test.char_domain', 'length': 1}
        expected['fld_char_domain_6'] = {'type': QVariant.String, 'typeName': 'qgis_test.char_domain_6', 'length': 6}
        expected['fld_text_domain'] = {'type': QVariant.String, 'typeName': 'qgis_test.text_domain', 'length': -1}
        expected['fld_numeric_domain'] = {'type': QVariant.Double, 'typeName': 'qgis_test.numeric_domain', 'length': 10, 'precision': 4}

        for f, e in list(expected.items()):
            self.assertEqual(fields.at(fields.indexFromName(f)).type(), e['type'])
            self.assertEqual(fields.at(fields.indexFromName(f)).typeName(), e['typeName'])
            self.assertEqual(fields.at(fields.indexFromName(f)).length(), e['length'])
            if 'precision' in e:
                self.assertEqual(fields.at(fields.indexFromName(f)).precision(), e['precision'])

    def testRenameAttributes(self):
        ''' Test renameAttributes() '''
        vl = QgsVectorLayer('%s table="qgis_test"."rename_table" sql=' % (self.dbconn), "renames", "postgres")
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
        self.assertTrue(provider.renameAttributes({1: 'newname2', 2: 'another'}))
        self.assertEqual(provider.fields().at(1).name(), 'newname2')
        self.assertEqual(provider.fields().at(2).name(), 'another')
        vl.updateFields()
        fet = next(vl.getFeatures())
        self.assertEqual(fet.fields()[1].name(), 'newname2')
        self.assertEqual(fet.fields()[2].name(), 'another')

        # close layer and reopen, then recheck to confirm that changes were saved to db
        del vl
        vl = None
        vl = QgsVectorLayer('%s table="qgis_test"."rename_table" sql=' % (self.dbconn), "renames", "postgres")
        provider = vl.dataProvider()
        self.assertEqual(provider.fields().at(1).name(), 'newname2')
        self.assertEqual(provider.fields().at(2).name(), 'another')
        fet = next(vl.getFeatures())
        self.assertEqual(fet.fields()[1].name(), 'newname2')
        self.assertEqual(fet.fields()[2].name(), 'another')

    def testEditorWidgetTypes(self):
        """Test that editor widget types can be fetched from the qgis_editor_widget_styles table"""

        vl = QgsVectorLayer('%s table="qgis_test"."widget_styles" sql=' % (self.dbconn), "widget_styles", "postgres")
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
        vl = QgsVectorLayer('%s table="qgis_test"."dict" sql=' % (self.dbconn), "testhstore", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('value')).type(), QVariant.Map)

        f = next(vl.getFeatures(QgsFeatureRequest()))

        value_idx = vl.fields().lookupField('value')
        self.assertIsInstance(f.attributes()[value_idx], dict)
        self.assertEqual(f.attributes()[value_idx], {'a': 'b', '1': '2'})

        new_f = QgsFeature(vl.fields())
        new_f['pk'] = NULL
        new_f['value'] = {'simple': '1', 'doubleQuote': '"y"', 'quote': "'q'", 'backslash': '\\'}
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

    def testStringArray(self):
        vl = QgsVectorLayer('%s table="qgis_test"."string_array" sql=' % (self.dbconn), "teststringarray", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('value')).type(), QVariant.StringList)
        self.assertEqual(fields.at(fields.indexFromName('value')).subType(), QVariant.String)

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
        vl = QgsVectorLayer('%s table="qgis_test"."int_array" sql=' % (self.dbconn), "testintarray", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('value')).type(), QVariant.List)
        self.assertEqual(fields.at(fields.indexFromName('value')).subType(), QVariant.Int)

        f = next(vl.getFeatures(QgsFeatureRequest()))

        value_idx = vl.fields().lookupField('value')
        self.assertIsInstance(f.attributes()[value_idx], list)
        self.assertEqual(f.attributes()[value_idx], [1, 2, -5])

    def testDoubleArray(self):
        vl = QgsVectorLayer('%s table="qgis_test"."double_array" sql=' % (self.dbconn), "testdoublearray", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('value')).type(), QVariant.List)
        self.assertEqual(fields.at(fields.indexFromName('value')).subType(), QVariant.Double)

        f = next(vl.getFeatures(QgsFeatureRequest()))

        value_idx = vl.fields().lookupField('value')
        self.assertIsInstance(f.attributes()[value_idx], list)
        self.assertEqual(f.attributes()[value_idx], [1.1, 2, -5.12345])

    def testNotNullConstraint(self):
        vl = QgsVectorLayer('%s table="qgis_test"."constraints" sql=' % (self.dbconn), "constraints", "postgres")
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 4)

        # test some bad field indexes
        self.assertEqual(vl.dataProvider().fieldConstraints(-1), QgsFieldConstraints.Constraints())
        self.assertEqual(vl.dataProvider().fieldConstraints(1001), QgsFieldConstraints.Constraints())

        self.assertTrue(vl.dataProvider().fieldConstraints(0) & QgsFieldConstraints.ConstraintNotNull)
        self.assertFalse(vl.dataProvider().fieldConstraints(1) & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(vl.dataProvider().fieldConstraints(2) & QgsFieldConstraints.ConstraintNotNull)
        self.assertFalse(vl.dataProvider().fieldConstraints(3) & QgsFieldConstraints.ConstraintNotNull)

        # test that constraints have been saved to fields correctly
        fields = vl.fields()
        self.assertTrue(fields.at(0).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(fields.at(0).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull), QgsFieldConstraints.ConstraintOriginProvider)
        self.assertFalse(fields.at(1).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(fields.at(2).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(fields.at(2).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull), QgsFieldConstraints.ConstraintOriginProvider)
        self.assertFalse(fields.at(3).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)

    def testUniqueConstraint(self):
        vl = QgsVectorLayer('%s table="qgis_test"."constraints" sql=' % (self.dbconn), "constraints", "postgres")
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 4)

        # test some bad field indexes
        self.assertEqual(vl.dataProvider().fieldConstraints(-1), QgsFieldConstraints.Constraints())
        self.assertEqual(vl.dataProvider().fieldConstraints(1001), QgsFieldConstraints.Constraints())

        self.assertTrue(vl.dataProvider().fieldConstraints(0) & QgsFieldConstraints.ConstraintUnique)
        self.assertTrue(vl.dataProvider().fieldConstraints(1) & QgsFieldConstraints.ConstraintUnique)
        self.assertTrue(vl.dataProvider().fieldConstraints(2) & QgsFieldConstraints.ConstraintUnique)
        self.assertFalse(vl.dataProvider().fieldConstraints(3) & QgsFieldConstraints.ConstraintUnique)

        # test that constraints have been saved to fields correctly
        fields = vl.fields()
        self.assertTrue(fields.at(0).constraints().constraints() & QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(fields.at(0).constraints().constraintOrigin(QgsFieldConstraints.ConstraintUnique), QgsFieldConstraints.ConstraintOriginProvider)
        self.assertTrue(fields.at(1).constraints().constraints() & QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(fields.at(1).constraints().constraintOrigin(QgsFieldConstraints.ConstraintUnique), QgsFieldConstraints.ConstraintOriginProvider)
        self.assertTrue(fields.at(2).constraints().constraints() & QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(fields.at(2).constraints().constraintOrigin(QgsFieldConstraints.ConstraintUnique), QgsFieldConstraints.ConstraintOriginProvider)
        self.assertFalse(fields.at(3).constraints().constraints() & QgsFieldConstraints.ConstraintUnique)

    def testConstraintOverwrite(self):
        """ test that Postgres provider constraints can't be overwritten by vector layer method """
        vl = QgsVectorLayer('%s table="qgis_test"."constraints" sql=' % (self.dbconn), "constraints", "postgres")
        self.assertTrue(vl.isValid())

        self.assertTrue(vl.dataProvider().fieldConstraints(0) & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(vl.fields().at(0).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)

        # add a constraint at the layer level
        vl.setFieldConstraint(0, QgsFieldConstraints.ConstraintUnique)

        # should be no change at provider level
        self.assertTrue(vl.dataProvider().fieldConstraints(0) & QgsFieldConstraints.ConstraintNotNull)

        # but layer should still keep provider constraints...
        self.assertTrue(vl.fields().at(0).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(vl.fieldConstraints(0) & QgsFieldConstraints.ConstraintNotNull)
        # ...in addition to layer level constraint
        self.assertTrue(vl.fields().at(0).constraints().constraints() & QgsFieldConstraints.ConstraintUnique)
        self.assertTrue(vl.fieldConstraints(0) & QgsFieldConstraints.ConstraintUnique)

    def testVectorLayerUtilsUniqueWithProviderDefault(self):
        vl = QgsVectorLayer('%s table="qgis_test"."someData" sql=' % (self.dbconn), "someData", "postgres")
        default_clause = 'nextval(\'qgis_test."someData_pk_seq"\'::regclass)'
        vl.dataProvider().setProviderProperty(QgsDataProvider.EvaluateDefaultValues, False)
        self.assertEqual(vl.dataProvider().defaultValueClause(0), default_clause)
        self.assertTrue(QgsVectorLayerUtils.valueExists(vl, 0, 4))

        vl.startEditing()
        f = QgsFeature(vl.fields())
        f.setAttribute(0, default_clause)
        self.assertFalse(QgsVectorLayerUtils.valueExists(vl, 0, default_clause))
        self.assertTrue(vl.addFeatures([f]))

        # the default value clause should exist...
        self.assertTrue(QgsVectorLayerUtils.valueExists(vl, 0, default_clause))
        # but it should not prevent the attribute being validated
        self.assertTrue(QgsVectorLayerUtils.validateAttribute(vl, f, 0))
        vl.rollBack()

    def testSkipConstraintCheck(self):
        vl = QgsVectorLayer('%s table="qgis_test"."someData" sql=' % (self.dbconn), "someData", "postgres")
        default_clause = 'nextval(\'qgis_test."someData_pk_seq"\'::regclass)'
        vl.dataProvider().setProviderProperty(QgsDataProvider.EvaluateDefaultValues, False)
        self.assertTrue(vl.dataProvider().skipConstraintCheck(0, QgsFieldConstraints.ConstraintUnique, default_clause))
        self.assertFalse(vl.dataProvider().skipConstraintCheck(0, QgsFieldConstraints.ConstraintUnique, 59))

    def testVectorLayerUtilsCreateFeatureWithProviderDefault(self):
        vl = QgsVectorLayer('%s table="qgis_test"."someData" sql=' % (self.dbconn), "someData", "postgres")
        default_clause = 'nextval(\'qgis_test."someData_pk_seq"\'::regclass)'
        self.assertEqual(vl.dataProvider().defaultValueClause(0), default_clause)

        # check that provider default clause takes precedence over passed attribute values
        # this also checks that the inbuilt unique constraint handling is bypassed in the case of a provider default clause
        f = QgsVectorLayerUtils.createFeature(vl, attributes={1: 5, 3: 'map'})
        self.assertEqual(f.attributes(), [default_clause, 5, "'qgis'::text", "'qgis'::text", None, None])

        # test take vector layer default value expression overrides postgres provider default clause
        vl.setDefaultValueDefinition(3, QgsDefaultValue("'mappy'"))
        f = QgsVectorLayerUtils.createFeature(vl, attributes={1: 5, 3: 'map'})
        self.assertEqual(f.attributes(), [default_clause, 5, "'qgis'::text", 'mappy', None, None])

    # See https://issues.qgis.org/issues/15188
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
        err = QgsVectorLayerExporter.exportLayer(lyr, uri, "postgres", lyr.crs())
        self.assertEqual(err[0], QgsVectorLayerExporter.NoError,
                         'unexpected import error {0}'.format(err))
        lyr = QgsVectorLayer(uri, "y", "postgres")
        self.assertTrue(lyr.isValid())
        f = next(lyr.getFeatures())
        self.assertEqual(f['f1'], 1)
        self.assertEqual(f['f2'], 123.456)
        self.assertEqual(f['f3'], '12345678.90123456789')

    # See https://issues.qgis.org/issues/15226
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
            err = QgsVectorLayerExporter.exportLayer(lyr, uri, "postgres", lyr.crs())
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

    # See https://issues.qgis.org/issues/17518
    def testImportWithoutSchema(self):

        def _test(table, schema=None):
            self.execSQLCommand('DROP TABLE IF EXISTS %s CASCADE' % table)
            uri = 'point?field=f1:int'
            uri += '&field=F2:double(6,4)'
            uri += '&field=f3:string(20)'
            lyr = QgsVectorLayer(uri, "x", "memory")
            self.assertTrue(lyr.isValid())

            table = ("%s" % table) if schema is None else ("\"%s\".\"%s\"" % (schema, table))
            dest_uri = "%s sslmode=disable table=%s  (geom) sql" % (self.dbconn, table)
            QgsVectorLayerExporter.exportLayer(lyr, dest_uri, "postgres", lyr.crs())
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
        self.assertTrue(vl.dataProvider().isSaveAndLoadStyleToDatabaseSupported())
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

        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.qml' % (unitTestDataPath(), "singleSymbol"))
        status = vl.loadNamedStyle(mFilePath)
        self.assertTrue(status)

        # The style is saved as non-default
        errorMsg = vl.saveStyleToDatabase("by day", "faded greens and elegant patterns", False, "")
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
        errorMsg = vl.saveStyleToDatabase("related style", "faded greens and elegant patterns", False, "")
        self.assertEqual(errorMsg, "")
        errorMsg = vl.saveStyleToDatabase("default style", "faded greens and elegant patterns", True, "")
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

    def testHasMetadata(self):
        # views don't have metadata
        vl = QgsVectorLayer('{} table="qgis_test"."{}" key="pk" sql='.format(self.dbconn, 'bikes_view'), "bikes_view", "postgres")
        self.assertTrue(vl.isValid())
        self.assertFalse(vl.dataProvider().hasMetadata())

        # ordinary tables have metadata
        vl = QgsVectorLayer('%s table="qgis_test"."someData" sql=' % (self.dbconn), "someData", "postgres")
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.dataProvider().hasMetadata())

    def testReadExtentOnView(self):
        # vector layer based on view
        vl0 = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data_view" (geom) sql=', 'test', 'postgres')
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
        vl0 = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=', 'test', 'postgres')
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

    def testNotify(self):
        vl0 = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=', 'test', 'postgres')
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
            if (time.time() - start) > 5: # timeout
                break

        vl0.dataProvider().notify.disconnect(notified.receive)
        vl0.dataProvider().setListening(False)

        self.assertTrue(ok)

    def testStyleDatabaseWithService(self):
        """Test saving style in DB using a service file.

        To run this test, you first need to create a service with:
        [qgis_test]
        host=localhost
        port=5432
        dbname=qgis_test
        user=USERNAME
        password=PASSWORD
        """
        myconn = 'service=\'qgis_test\''
        if 'QGIS_PGTEST_DB' in os.environ:
            myconn = os.environ['QGIS_PGTEST_DB']
        myvl = QgsVectorLayer(myconn + ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=', 'test', 'postgres')

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
        self.execSQLCommand('CREATE TABLE IF NOT EXISTS multicurve(pk SERIAL NOT NULL PRIMARY KEY, geom public.geometry(MultiPolygon, 4326))')
        self.execSQLCommand('TRUNCATE multicurve')

        vl = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=MULTIPOLYGON table="multicurve" (geom) sql=', 'test', 'postgres')

        f = QgsFeature(vl.fields())
        f.setGeometry(QgsGeometry.fromWkt('CurvePolygon(CircularString (20 30, 50 30, 50 90, 10 50, 20 30))'))
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.addFeatures([f]))
        self.assertTrue(vl.commitChanges())

        f = next(vl.getFeatures(QgsFeatureRequest()))

        g = f.geometry().constGet()
        self.assertTrue(g)
        self.assertEqual(g.wkbType(), QgsWkbTypes.MultiPolygon)
        self.assertEqual(g.childCount(), 1)
        self.assertTrue(g.childGeometry(0).vertexCount() > 3)


class TestPyQgsPostgresProviderCompoundKey(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.dbconn = 'dbname=\'qgis_test\''
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        # Create test layers
        cls.vl = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'"key1","key2"\' srid=4326 type=POINT table="qgis_test"."someDataCompound" (geom) sql=', 'test', 'postgres')
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
        return set([])

    def partiallyCompiledFilters(self):
        return set([])


if __name__ == '__main__':
    unittest.main()
