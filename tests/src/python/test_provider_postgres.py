# -*- coding: utf-8 -*-
"""QGIS Unit tests for the postgres provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '2015-04-23'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
import os
from qgis.core import NULL

from qgis.core import QgsVectorLayer, QgsFeatureRequest, QgsFeature, QgsProviderRegistry
from PyQt.QtCore import QSettings, QDate, QTime, QDateTime, QVariant
from qgis.testing import (start_app,
                          unittest
                          )
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
        assert(cls.vl.isValid())
        cls.provider = cls.vl.dataProvider()
        cls.poly_vl = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=', 'test', 'postgres')
        assert(cls.poly_vl.isValid())
        cls.poly_provider = cls.poly_vl.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def enableCompiler(self):
        QSettings().setValue(u'/qgis/compileExpressions', True)

    def disableCompiler(self):
        QSettings().setValue(u'/qgis/compileExpressions', False)

    # HERE GO THE PROVIDER SPECIFIC TESTS
    def testDefaultValue(self):
        assert self.provider.defaultValue(0) == u'nextval(\'qgis_test."someData_pk_seq"\'::regclass)'
        assert self.provider.defaultValue(1) == NULL
        assert self.provider.defaultValue(2) == '\'qgis\'::text'

    def testDateTimeTypes(self):
        vl = QgsVectorLayer('%s table="qgis_test"."date_times" sql=' % (self.dbconn), "testdatetimes", "postgres")
        assert(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('date_field')).type(), QVariant.Date)
        self.assertEqual(fields.at(fields.indexFromName('time_field')).type(), QVariant.Time)
        self.assertEqual(fields.at(fields.indexFromName('datetime_field')).type(), QVariant.DateTime)

        f = vl.getFeatures(QgsFeatureRequest()).next()

        date_idx = vl.fieldNameIndex('date_field')
        assert isinstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2004, 3, 4))
        time_idx = vl.fieldNameIndex('time_field')
        assert isinstance(f.attributes()[time_idx], QTime)
        self.assertEqual(f.attributes()[time_idx], QTime(13, 41, 52))
        datetime_idx = vl.fieldNameIndex('datetime_field')
        assert isinstance(f.attributes()[datetime_idx], QDateTime)
        self.assertEqual(f.attributes()[datetime_idx], QDateTime(QDate(2004, 3, 4), QTime(13, 41, 52)))

    def testQueryLayers(self):
        def test_query(dbconn, query, key):
            ql = QgsVectorLayer('%s srid=4326 table="%s" (geom) key=\'%s\' sql=' % (dbconn, query.replace('"', '\\"'), key), "testgeom", "postgres")
            print query, key
            assert(ql.isValid())

        test_query(self.dbconn, '(SELECT NULL::integer "Id1", NULL::integer "Id2", NULL::geometry(Point, 4326) geom LIMIT 0)', '"Id1","Id2"')

    def testWkbTypes(self):
        def test_table(dbconn, table_name, wkt):
            vl = QgsVectorLayer('%s srid=4326 table="qgis_test".%s (geom) sql=' % (dbconn, table_name), "testgeom", "postgres")
            assert(vl.isValid())
            for f in vl.getFeatures():
                print f.geometry().exportToWkt(), wkt
                assert f.geometry().exportToWkt() == wkt

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
    def testSignedIdentifiers(self):
        def test_query_attribute(dbconn, query, att, val, fidval):
            ql = QgsVectorLayer('%s table="%s" (g) key=\'%s\' sql=' % (dbconn, query.replace('"', '\\"'), att), "testgeom", "postgres")
            print query, att
            assert(ql.isValid())
            features = ql.getFeatures()
            att_idx = ql.fieldNameIndex(att)
            count = 0
            for f in features:
                count += 1
                self.assertEqual(f.attributes()[att_idx], val)
                #self.assertEqual(f.id(), val)
            self.assertEqual(count, 1)
        test_query_attribute(self.dbconn, '(SELECT -1::int4 i, NULL::geometry(Point) g)', 'i', -1, 1)
        test_query_attribute(self.dbconn, '(SELECT -1::int2 i, NULL::geometry(Point) g)', 'i', -1, 1)
        test_query_attribute(self.dbconn, '(SELECT -1::int8 i, NULL::geometry(Point) g)', 'i', -1, 1)
        test_query_attribute(self.dbconn, '(SELECT -65535::int8 i, NULL::geometry(Point) g)', 'i', -65535, 1)


if __name__ == '__main__':
    unittest.main()
