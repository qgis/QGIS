# -*- coding: utf-8 -*-
"""QGIS Unit tests for the postgres provider.

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

import os

from qgis.core import (
    QgsVectorLayer,
    QgsFeatureRequest,
    QgsFeature,
    QgsTransactionGroup,
    NULL
)
from qgis.PyQt.QtCore import QSettings, QDate, QTime, QDateTime, QVariant
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from providertestbase import ProviderTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsPostgresProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.dbconn = u'dbname=\'qgis_test\''
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        # Create test layers
        cls.vl = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=', 'test', 'postgres')
        assert cls.vl.isValid()
        cls.provider = cls.vl.dataProvider()
        cls.poly_vl = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=', 'test', 'postgres')
        assert cls.poly_vl.isValid()
        cls.poly_provider = cls.poly_vl.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def enableCompiler(self):
        QSettings().setValue(u'/qgis/compileExpressions', True)

    def disableCompiler(self):
        QSettings().setValue(u'/qgis/compileExpressions', False)

    def uncompiledFilters(self):
        return set(['intersects($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))'])

    def partiallyCompiledFilters(self):
        return set([])

    # HERE GO THE PROVIDER SPECIFIC TESTS
    def testDefaultValue(self):
        self.assertEqual(self.provider.defaultValue(0), u'nextval(\'qgis_test."someData_pk_seq"\'::regclass)')
        self.assertEqual(self.provider.defaultValue(1), NULL)
        self.assertEqual(self.provider.defaultValue(2), '\'qgis\'::text')

    def testDateTimeTypes(self):
        vl = QgsVectorLayer('%s table="qgis_test"."date_times" sql=' % (self.dbconn), "testdatetimes", "postgres")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('date_field')).type(), QVariant.Date)
        self.assertEqual(fields.at(fields.indexFromName('time_field')).type(), QVariant.Time)
        self.assertEqual(fields.at(fields.indexFromName('datetime_field')).type(), QVariant.DateTime)

        f = next(vl.getFeatures(QgsFeatureRequest()))

        date_idx = vl.fieldNameIndex('date_field')
        self.assertTrue(isinstance(f.attributes()[date_idx], QDate))
        self.assertEqual(f.attributes()[date_idx], QDate(2004, 3, 4))
        time_idx = vl.fieldNameIndex('time_field')
        self.assertTrue(isinstance(f.attributes()[time_idx], QTime))
        self.assertEqual(f.attributes()[time_idx], QTime(13, 41, 52))
        datetime_idx = vl.fieldNameIndex('datetime_field')
        self.assertTrue(isinstance(f.attributes()[datetime_idx], QDateTime))
        self.assertEqual(f.attributes()[datetime_idx], QDateTime(QDate(2004, 3, 4), QTime(13, 41, 52)))

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
                self.assertEqual(f.geometry().exportToWkt(), wkt)

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

    # See http://hub.qgis.org/issues/14262
    # TODO: accept multi-featured layers, and an array of values/fids
    def testSignedIdentifiers(self):

        def test_layer(ql, att, val, fidval):
            self.assertTrue(ql.isValid())
            features = ql.getFeatures()
            att_idx = ql.fieldNameIndex(att)
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

        #### --- INT16 ----
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

        #### --- INT32 ----
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

        #### --- INT64 (FIDs are always 1 because assigned ex-novo) ----
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
        f['obj_id'] = vl.dataProvider().defaultValue(0)
        f['name'] = 'Test'
        r, f = vl.dataProvider().addFeatures([f])
        self.assertTrue(r)
        self.assertNotEqual(f[0]['obj_id'], NULL, f[0].attributes())
        vl.deleteFeatures([f[0].id()])

    def testNestedInsert(self):
        tg = QgsTransactionGroup()
        tg.addLayer(self.vl)
        self.vl.startEditing()
        it = self.vl.getFeatures()
        f = next(it)
        f['pk'] = NULL
        self.vl.addFeature(f) # Should not deadlock during an active iteration
        f = next(it)

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

        for f, e in expected.items():
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

if __name__ == '__main__':
    unittest.main()
