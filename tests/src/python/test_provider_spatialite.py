"""QGIS Unit tests for QgsSpatialiteProvider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Vincent Mora'
__date__ = '09/07/2013'
__copyright__ = 'Copyright 2013, The QGIS Project'

import os
import re
import shutil
import sys
import tempfile
from datetime import datetime

from osgeo import ogr
from qgis.PyQt.QtCore import QByteArray, QVariant
from qgis.core import (
    Qgis,
    QgsBox3D,
    QgsDataSourceUri,
    QgsDefaultValue,
    QgsFeature,
    QgsFeatureRequest,
    QgsFieldConstraints,
    QgsGeometry,
    QgsPathResolver,
    QgsPointXY,
    QgsProject,
    QgsProviderRegistry,
    QgsReadWriteContext,
    QgsRectangle,
    QgsSettings,
    QgsVectorDataProvider,
    QgsVectorLayer,
    QgsVectorLayerExporter,
    QgsVectorLayerUtils,
    QgsWkbTypes, NULL)
import unittest
from qgis.testing import start_app, QgisTestCase
from qgis.utils import spatialite_connect

from providertestbase import ProviderTestCase
from utilities import unitTestDataPath, compareWkt

# Pass no_exit=True: for some reason this crashes sometimes on exit on Travis
start_app(True)
TEST_DATA_DIR = unitTestDataPath()


def count_opened_filedescriptors(filename_to_test):
    count = -1
    if sys.platform.startswith('linux'):
        count = 0
        open_files_dirname = '/proc/%d/fd' % os.getpid()
        filenames = os.listdir(open_files_dirname)
        for filename in filenames:
            full_filename = open_files_dirname + '/' + filename
            if os.path.exists(full_filename):
                link = os.readlink(full_filename)
                if os.path.basename(link) == os.path.basename(filename_to_test):
                    count += 1
    return count


class TestQgsSpatialiteProvider(QgisTestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super(TestQgsSpatialiteProvider, cls).setUpClass()
        print(' ### Setup Spatialite Provider Test Class')
        # setup provider for base tests
        cls.vl = QgsVectorLayer(
            'dbname=\'{}/provider/spatialite.db\' table="somedata" (geom) sql='.format(
                TEST_DATA_DIR), 'test',
            'spatialite')
        assert (cls.vl.isValid())
        cls.source = cls.vl.dataProvider()

        cls.vl_poly = QgsVectorLayer(
            'dbname=\'{}/provider/spatialite.db\' table="somepolydata" (geom) sql='.format(
                TEST_DATA_DIR), 'test',
            'spatialite')
        assert (cls.vl_poly.isValid())
        cls.poly_provider = cls.vl_poly.dataProvider()

        # create test db
        cls.dbname = os.path.join(tempfile.gettempdir(), "test.sqlite")
        if os.path.exists(cls.dbname):
            os.remove(cls.dbname)
        con = spatialite_connect(cls.dbname, isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)

        # simple table with primary key
        sql = "CREATE TABLE test_pg (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_pg', 'geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_pg (id, name, geometry) "
        sql += "VALUES (1, 'toto 1', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)

        # table with Z dimension geometry, 1 point
        sql = "CREATE TABLE test_z (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_z', 'geometry', 4326, 'POINT', 'XYZ')"
        cur.execute(sql)
        sql = "INSERT INTO test_z (id, name, geometry) "
        sql += "VALUES (1, 'toto 2', GeomFromText('POINT Z (0 0 1)', 4326))"
        cur.execute(sql)

        # table with Z dimension geometry, multiple points
        sql = "CREATE TABLE test_z2 (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_z2', 'geometry', 4326, 'POINT', 'XYZ')"
        cur.execute(sql)
        for i in range(2, 12):
            sql = "INSERT INTO test_z2 (id, name, geometry) "
            sql += f"VALUES ({i-1}, 'toto 2', GeomFromText('POINT Z ({i-4} {i+1} {i})', 4326))"
            cur.execute(sql)

        # table with M value geometry
        sql = "CREATE TABLE test_m (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_m', 'geometry', 4326, 'POINT', 'XYM')"
        cur.execute(sql)
        sql = "INSERT INTO test_m (id, name, geometry) "
        sql += "VALUES (1, 'toto 3', GeomFromText('POINT M (0 0 1)', 4326))"
        cur.execute(sql)

        # table with Z dimension and M value geometry
        sql = "CREATE TABLE test_zm (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_zm', 'geometry', 4326, 'POINT', 'XYZM')"
        cur.execute(sql)
        sql = "INSERT INTO test_zm (id, name, geometry) "
        sql += "VALUES (1, 'toto 1', GeomFromText('POINT ZM (0 0 1 1)', 4326))"
        cur.execute(sql)

        # table with multiple column primary key
        sql = "CREATE TABLE test_pg_mk (id INTEGER NOT NULL, name TEXT NOT NULL, PRIMARY KEY(id,name))"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_pg_mk', 'geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_pg_mk (id, name, geometry) "
        sql += "VALUES (1, 'toto 1', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)

        # simple table with primary key
        sql = "CREATE TABLE test_q (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_q', 'geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_q (id, name, geometry) "
        sql += "VALUES (11, 'toto 11', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)
        sql = "INSERT INTO test_q (id, name, geometry) "
        sql += "VALUES (21, 'toto 12', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)

        # simple table with a geometry column named 'Geometry'
        sql = "CREATE TABLE test_n (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_n', 'Geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_n (id, name, geometry) "
        sql += "VALUES (1, 'toto 1', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)
        sql = "INSERT INTO test_n (id, name, geometry) "
        sql += "VALUES (2, 'toto 1', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)

        # table with different array types, stored as JSON
        sql = "CREATE TABLE test_arrays (id INTEGER NOT NULL PRIMARY KEY, strings JSONSTRINGLIST NOT NULL, ints JSONINTEGERLIST NOT NULL, reals JSONREALLIST NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_arrays', 'Geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_arrays (id, strings, ints, reals, geometry) "
        sql += "VALUES (1, '[\"toto\",\"tutu\"]', '[1,-2,724562]', '[1.0, -232567.22]', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)

        # table with different array types, stored as JSON
        sql = "CREATE TABLE test_arrays_write (id INTEGER NOT NULL PRIMARY KEY, array JSONARRAY NOT NULL, strings JSONSTRINGLIST NOT NULL, ints JSONINTEGERLIST NOT NULL, reals JSONREALLIST NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_arrays_write', 'Geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)

        # 2 tables with relations
        sql = "PRAGMA foreign_keys = ON;"
        cur.execute(sql)
        sql = "CREATE TABLE test_relation_a(artistid INTEGER PRIMARY KEY, artistname  TEXT);"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_relation_a', 'Geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "CREATE TABLE test_relation_b(trackid INTEGER, trackname TEXT, trackartist INTEGER, FOREIGN KEY(trackartist) REFERENCES test_relation_a(artistid));"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_relation_b', 'Geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)

        # table to test auto increment
        sql = "CREATE TABLE test_autoincrement(id INTEGER PRIMARY KEY AUTOINCREMENT, num INTEGER);"
        cur.execute(sql)
        sql = "INSERT INTO test_autoincrement (num) VALUES (123);"
        cur.execute(sql)

        # tables with constraints
        sql = "CREATE TABLE test_constraints(id INTEGER PRIMARY KEY, num INTEGER NOT NULL, desc TEXT UNIQUE, desc2 TEXT, num2 INTEGER NOT NULL UNIQUE)"
        cur.execute(sql)

        # simple table with defaults
        sql = "CREATE TABLE test_defaults (id INTEGER NOT NULL PRIMARY KEY, name TEXT DEFAULT 'qgis ''is good', number INTEGER DEFAULT 5, number2 REAL DEFAULT 5.7, no_default REAL)"
        cur.execute(sql)

        # simple table with catgorized points
        sql = "CREATE TABLE test_filter (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_filter', 'geometry', 4326, 'POINT', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_filter (id, name, geometry) "
        sql += "VALUES (1, 'ext', GeomFromText('POINT(0 0)', 4326))"
        cur.execute(sql)
        sql = "INSERT INTO test_filter (id, name, geometry) "
        sql += "VALUES (2, 'ext', GeomFromText('POINT(0 3)', 4326))"
        cur.execute(sql)
        sql = "INSERT INTO test_filter (id, name, geometry) "
        sql += "VALUES (3, 'ext', GeomFromText('POINT(3 3)', 4326))"
        cur.execute(sql)
        sql = "INSERT INTO test_filter (id, name, geometry) "
        sql += "VALUES (4, 'ext', GeomFromText('POINT(3 0)', 4326))"
        cur.execute(sql)

        sql = "INSERT INTO test_filter (id, name, geometry) "
        sql += "VALUES (5, 'int', GeomFromText('POINT(1 1)', 4326))"
        cur.execute(sql)
        sql = "INSERT INTO test_filter (id, name, geometry) "
        sql += "VALUES (6, 'int', GeomFromText('POINT(1 2)', 4326))"
        cur.execute(sql)
        sql = "INSERT INTO test_filter (id, name, geometry) "
        sql += "VALUES (7, 'int', GeomFromText('POINT(2 2)', 4326))"
        cur.execute(sql)
        sql = "INSERT INTO test_filter (id, name, geometry) "
        sql += "VALUES (8, 'int', GeomFromText('POINT(2 1)', 4326))"
        cur.execute(sql)

        # bigint table
        sql = "CREATE TABLE test_bigint (id BIGINT, value INT)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_bigint', 'position', 4326, 'LINESTRING', 'XYM')"
        cur.execute(sql)
        sql = """
        INSERT INTO test_bigint (id, value, position) VALUES
        (987654321012345, 1, ST_GeomFromtext('LINESTRINGM(10.416255 55.3786316 1577093516, 10.516255 55.4786316 157709)', 4326) ),
        (987654321012346, 2, ST_GeomFromtext('LINESTRINGM(10.316255 55.3786316 1577093516, 11.216255 56.3786316 157709)', 4326) )"""
        cur.execute(sql)

        # no fields table
        sql = "CREATE TABLE \"test_nofields\"(pkuid integer primary key autoincrement)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_nofields', 'geometry', 4326, 'POINT', 'XY')"
        cur.execute(sql)

        # constraints check table
        sql = "CREATE TABLE \"check_constraint\"(pkuid integer primary key autoincrement, i_will_fail_on_no_name TEXT CHECK (i_will_fail_on_no_name != 'no name'))"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('check_constraint', 'geometry', 4326, 'POINT', 'XY')"
        cur.execute(sql)
        sql = """
        INSERT INTO check_constraint (pkuid, geometry, i_will_fail_on_no_name) VALUES(1, ST_GeomFromtext('POINT(10.416255 55.3786316)', 4326), 'I have a name'),
        (2, ST_GeomFromtext('POINT(9.416255 45.3786316)', 4326), 'I have a name too');
        """
        cur.execute(sql)

        # Unique and not null constraints
        sql = "CREATE TABLE \"unique_not_null_constraints\"(pkuid integer primary key autoincrement, \"unique\" TEXT UNIQUE, \"not_null\" TEXT NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('unique_not_null_constraints', 'geometry', 4326, 'POINT', 'XY')"
        cur.execute(sql)

        # blob test table
        sql = "CREATE TABLE blob_table ( id INTEGER NOT NULL PRIMARY KEY, fld1 BLOB )"
        cur.execute(sql)
        sql = """
        INSERT INTO blob_table VALUES
        (1, X'0053514C697465'),
        (2, NULL),
        (3, X'53514C697465')
        """
        cur.execute(sql)

        # Transaction tables
        sql = "CREATE TABLE \"test_transactions1\"(pkuid integer primary key autoincrement)"
        cur.execute(sql)
        sql = "CREATE TABLE \"test_transactions2\"(pkuid integer primary key autoincrement)"
        cur.execute(sql)
        sql = "INSERT INTO \"test_transactions2\" VALUES (NULL)"
        cur.execute(sql)

        # Commit all test data
        cur.execute("COMMIT")
        con.close()

        cls.dirs_to_cleanup = []

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        del cls.vl
        del cls.vl_poly
        # for the time being, keep the file to check with qgis
        # if os.path.exists(cls.dbname) :
        #    os.remove(cls.dbname)
        for dirname in cls.dirs_to_cleanup:
            shutil.rmtree(dirname, True)
        super(TestQgsSpatialiteProvider, cls).tearDownClass()

    def getSource(self):
        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        datasource = os.path.join(tmpdir, 'spatialite.db')
        shutil.copy(os.path.join(srcpath, 'spatialite.db'), datasource)

        vl = QgsVectorLayer(
            f'dbname=\'{datasource}\' table="somedata" (geom) sql=', 'test',
            'spatialite')
        return vl

    def getEditableLayerWithCheckConstraint(self):
        """Returns the layer for attribute change CHECK constraint violation"""

        vl = QgsVectorLayer(
            'dbname=\'{}\' table="check_constraint" (geometry) sql='.format(
                self.dbname), 'check_constraint',
            'spatialite')
        return vl

    def getEditableLayerWithUniqueNotNullConstraints(self):
        """Returns the layer for UNIQUE and NOT NULL constraints detection"""

        vl = QgsVectorLayer(
            'dbname=\'{}\' table="unique_not_null_constraints" (geometry) sql='.format(
                self.dbname), 'unique_not_null_constraints',
            'spatialite')
        return vl

    def treat_time_as_string(self):
        return True

    def getEditableLayer(self):
        return self.getSource()

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def enableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', True)
        return True

    def disableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', False)

    def uncompiledFilters(self):
        return {'cnt = 10 ^ 2',
                '"name" ~ \'[OP]ra[gne]+\'',
                'sqrt(pk) >= 2',
                'radians(cnt) < 2',
                'degrees(pk) <= 200',
                'cos(pk) < 0',
                'sin(pk) < 0',
                'tan(pk) < 0',
                'acos(-1) < pk',
                'asin(1) < pk',
                'atan(3.14) < pk',
                'atan2(3.14, pk) < 1',
                'exp(pk) < 10',
                'ln(pk) <= 1',
                'log(3, pk) <= 1',
                'log10(pk) < 0.5',
                'floor(3.14) <= pk',
                'ceil(3.14) <= pk',
                'pk < pi()',
                'floor(cnt / 66.67) <= 2',
                'ceil(cnt / 66.67) <= 2',
                'pk < pi() / 2',
                'x($geometry) < -70',
                'y($geometry) > 70',
                'xmin($geometry) < -70',
                'ymin($geometry) > 70',
                'xmax($geometry) < -70',
                'ymax($geometry) > 70',
                'disjoint($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))',
                'intersects($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))',
                'contains(geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'),$geometry)',
                'distance($geometry,geom_from_wkt( \'Point (-70 70)\')) > 7',
                'intersects($geometry,geom_from_gml( \'<gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-72.2,66.1 -65.2,66.1 -65.2,72.0 -72.2,72.0 -72.2,66.1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon>\'))',
                'x($geometry) < -70',
                'y($geometry) > 79',
                'xmin($geometry) < -70',
                'ymin($geometry) < 76',
                'xmax($geometry) > -68',
                'ymax($geometry) > 80',
                'area($geometry) > 10',
                'perimeter($geometry) < 12',
                'relate($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\')) = \'FF2FF1212\'',
                'relate($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\'), \'****F****\')',
                'crosses($geometry,geom_from_wkt( \'Linestring (-68.2 82.1, -66.95 82.1, -66.95 79.05)\'))',
                'overlaps($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\'))',
                'within($geometry,geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
                'overlaps(translate($geometry,-1,-1),geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
                'overlaps(buffer($geometry,1),geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
                'intersects(centroid($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))',
                'intersects(point_on_surface($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))',
                '"dt" = to_datetime(\'000www14ww13ww12www4ww5ww2020\',\'zzzwwwsswwmmwwhhwwwdwwMwwyyyy\')',
                '"dt" <= format_date(make_datetime(2020, 5, 4, 12, 13, 14), \'yyyy-MM-dd hh:mm:ss\')',
                '"dt" < format_date(make_date(2020, 5, 4), \'yyyy-MM-dd hh:mm:ss\')',
                '"dt" = format_date(to_datetime(\'000www14ww13ww12www4ww5ww2020\',\'zzzwwwsswwmmwwhhwwwdwwMwwyyyy\'),\'yyyy-MM-dd hh:mm:ss\')',
                'to_time("time") >= make_time(12, 14, 14)',
                'to_time("time") = to_time(\'000www14ww13ww12www\',\'zzzwwwsswwmmwwhhwww\')',
                '"date" = to_date(\'www4ww5ww2020\',\'wwwdwwMwwyyyy\')',
                'dt BETWEEN make_datetime(2020, 5, 3, 12, 13, 14) AND make_datetime(2020, 5, 4, 12, 14, 14)',
                'dt NOT BETWEEN make_datetime(2020, 5, 3, 12, 13, 14) AND make_datetime(2020, 5, 4, 12, 14, 14)',
                '"dt" <= make_datetime(2020, 5, 4, 12, 13, 14)',
                '"date" <= make_datetime(2020, 5, 4, 12, 13, 14)'
                }

    def partiallyCompiledFilters(self):
        return {'"name" NOT LIKE \'Ap%\'',
                'name LIKE \'Apple\'',
                'name LIKE \'aPple\'',
                'name LIKE \'Ap_le\'',
                'name LIKE \'Ap\\_le\''
                }

    def test_SplitFeature(self):
        """Create SpatiaLite database"""
        layer = QgsVectorLayer("dbname=%s table=test_pg (geometry)" %
                               self.dbname, "test_pg", "spatialite")
        self.assertTrue(layer.isValid())
        self.assertTrue(layer.isSpatial())
        layer.startEditing()
        self.assertEqual(layer.splitFeatures(
            [QgsPointXY(0.75, -0.5), QgsPointXY(0.75, 1.5)], 0), 0)
        self.assertEqual(layer.splitFeatures(
            [QgsPointXY(-0.5, 0.25), QgsPointXY(1.5, 0.25)], 0), 0)
        self.assertTrue(layer.commitChanges())
        self.assertEqual(layer.featureCount(), 4)

    def test_SplitFeatureWithMultiKey(self):
        """Create SpatiaLite database"""
        layer = QgsVectorLayer("dbname=%s table=test_pg_mk (geometry)" %
                               self.dbname, "test_pg_mk", "spatialite")
        self.assertTrue(layer.isValid())
        self.assertTrue(layer.isSpatial())
        layer.startEditing()
        self.assertEqual(layer.splitFeatures(
            [QgsPointXY(0.5, -0.5), QgsPointXY(0.5, 1.5)], 0), 0)
        self.assertEqual(layer.splitFeatures(
            [QgsPointXY(-0.5, 0.5), QgsPointXY(1.5, 0.5)], 0), 0)
        self.assertTrue(layer.commitChanges())

    def test_crash_on_constraint_detection(self):
        """
        Test that constraint detection does not crash
        """
        # should be no crash!
        QgsVectorLayer(f"dbname={TEST_DATA_DIR + '/views_test.sqlite'} table=KNN", "KNN",
                       "spatialite")

    def test_queries(self):
        """Test loading of query-based layers"""

        # a query with a geometry, but no unique id
        # the id will be autoincremented
        l = QgsVectorLayer(f"dbname={self.dbname} table='(select * from test_q)' (geometry)", "test_pg_query1",
                           "spatialite")
        self.assertTrue(l.isValid())
        # the id() is autoincremented
        sum_id1 = sum(f.id() for f in l.getFeatures())
        # the attribute 'id' works
        sum_id2 = sum(f.attributes()[0] for f in l.getFeatures())
        self.assertEqual(sum_id1, 32)  # 11 + 21
        self.assertEqual(sum_id2, 32)  # 11 + 21

        # and now with an id declared
        l = QgsVectorLayer(f"dbname={self.dbname} table='(select * from test_q)' (geometry) key='id'",
                           "test_pg_query1", "spatialite")
        self.assertTrue(l.isValid())
        sum_id1 = sum(f.id() for f in l.getFeatures())
        sum_id2 = sum(f.attributes()[0] for f in l.getFeatures())
        self.assertEqual(sum_id1, 32)
        self.assertEqual(sum_id2, 32)

        # a query, but no geometry
        l = QgsVectorLayer(f"dbname={self.dbname} table='(select id,name from test_q)' key='id'", "test_pg_query1",
                           "spatialite")
        self.assertTrue(l.isValid())
        sum_id1 = sum(f.id() for f in l.getFeatures())
        sum_id2 = sum(f.attributes()[0] for f in l.getFeatures())
        self.assertEqual(sum_id1, 32)
        self.assertEqual(sum_id2, 32)

    def test_zm(self):
        """Test Z dimension and M value"""
        l = QgsVectorLayer("dbname=%s table='test_z' (geometry) key='id'" %
                           self.dbname, "test_z", "spatialite")
        self.assertTrue(l.isValid())
        self.assertTrue(QgsWkbTypes.hasZ(l.wkbType()))
        feature = l.getFeature(1)
        geom = feature.geometry().constGet()
        self.assertEqual(geom.z(), 1.0)

        l = QgsVectorLayer("dbname=%s table='test_m' (geometry) key='id'" %
                           self.dbname, "test_m", "spatialite")
        self.assertTrue(l.isValid())
        self.assertTrue(QgsWkbTypes.hasM(l.wkbType()))
        feature = l.getFeature(1)
        geom = feature.geometry().constGet()
        self.assertEqual(geom.m(), 1.0)

        l = QgsVectorLayer("dbname=%s table='test_zm' (geometry) key='id'" %
                           self.dbname, "test_zm", "spatialite")
        self.assertTrue(l.isValid())
        self.assertTrue(QgsWkbTypes.hasZ(l.wkbType()))
        self.assertTrue(QgsWkbTypes.hasM(l.wkbType()))
        feature = l.getFeature(1)
        geom = feature.geometry().constGet()
        self.assertEqual(geom.z(), 1.0)
        self.assertEqual(geom.m(), 1.0)

    def test_case(self):
        """Test case sensitivity issues"""
        l = QgsVectorLayer("dbname=%s table='test_n' (geometry) key='id'" %
                           self.dbname, "test_n1", "spatialite")
        self.assertTrue(l.isValid())
        self.assertEqual(l.dataProvider().fields().count(), 2)
        fields = [f.name() for f in l.dataProvider().fields()]
        self.assertNotIn('Geometry', fields)

    def test_invalid_iterator(self):
        """ Test invalid iterator """
        corrupt_dbname = self.dbname + '.corrupt'
        shutil.copy(self.dbname, corrupt_dbname)
        layer = QgsVectorLayer("dbname=%s table=test_pg (geometry)" %
                               corrupt_dbname, "test_pg", "spatialite")
        # Corrupt the database
        with open(corrupt_dbname, 'wb') as f:
            f.write(b'')
        layer.getFeatures()
        layer = None
        os.unlink(corrupt_dbname)

    def testNoDanglingFileDescriptorAfterCloseVariant1(self):
        ''' Test that when closing the provider all file handles are released '''

        temp_dbname = self.dbname + '.no_dangling_test1'
        shutil.copy(self.dbname, temp_dbname)

        vl = QgsVectorLayer("dbname=%s table=test_n (geometry)" %
                            temp_dbname, "test_n", "spatialite")
        self.assertTrue(vl.isValid())
        # The iterator will take one extra connection
        myiter = vl.getFeatures()
        print(vl.featureCount())
        # Consume one feature but the iterator is still opened
        f = next(myiter)
        self.assertTrue(f.isValid())

        if sys.platform.startswith('linux'):
            self.assertEqual(count_opened_filedescriptors(temp_dbname), 2)

        # does NO release one file descriptor, because shared with the iterator
        del vl

        # Non portable, but Windows testing is done with trying to unlink
        if sys.platform.startswith('linux'):
            self.assertEqual(count_opened_filedescriptors(temp_dbname), 2)

        f = next(myiter)
        self.assertTrue(f.isValid())

        # Should release one file descriptor
        del myiter

        # Non portable, but Windows testing is done with trying to unlink
        if sys.platform.startswith('linux'):
            self.assertEqual(count_opened_filedescriptors(temp_dbname), 0)

        # Check that deletion works well (can only fail on Windows)
        os.unlink(temp_dbname)
        self.assertFalse(os.path.exists(temp_dbname))

    def testNoDanglingFileDescriptorAfterCloseVariant2(self):
        ''' Test that when closing the provider all file handles are released '''

        temp_dbname = self.dbname + '.no_dangling_test2'
        shutil.copy(self.dbname, temp_dbname)

        vl = QgsVectorLayer("dbname=%s table=test_n (geometry)" %
                            temp_dbname, "test_n", "spatialite")
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.isValid())
        # Consume all features.
        myiter = vl.getFeatures()
        for feature in myiter:
            pass
        # The iterator is closed
        if sys.platform.startswith('linux'):
            self.assertEqual(count_opened_filedescriptors(temp_dbname), 2)

        # Should release one file descriptor
        del vl

        # Non portable, but Windows testing is done with trying to unlink
        if sys.platform.startswith('linux'):
            self.assertEqual(count_opened_filedescriptors(temp_dbname), 0)

        # Check that deletion works well (can only fail on Windows)
        os.unlink(temp_dbname)
        self.assertFalse(os.path.exists(temp_dbname))

    def test_arrays(self):
        """Test loading of layers with arrays"""
        l = QgsVectorLayer("dbname=%s table=test_arrays (geometry)" %
                           self.dbname, "test_arrays", "spatialite")
        self.assertTrue(l.isValid())

        features = [f for f in l.getFeatures()]
        self.assertEqual(len(features), 1)

        strings_field = l.fields().field('strings')
        self.assertEqual(strings_field.typeName(), 'jsonstringlist')
        self.assertEqual(strings_field.type(), QVariant.StringList)
        self.assertEqual(strings_field.subType(), QVariant.String)
        strings = features[0].attributes()[1]
        self.assertEqual(strings, ['toto', 'tutu'])

        ints_field = l.fields().field('ints')
        self.assertEqual(ints_field.typeName(), 'jsonintegerlist')
        self.assertEqual(ints_field.type(), QVariant.List)
        self.assertEqual(ints_field.subType(), QVariant.LongLong)
        ints = features[0].attributes()[2]
        self.assertEqual(ints, [1, -2, 724562])

        reals_field = l.fields().field('reals')
        self.assertEqual(reals_field.typeName(), 'jsonreallist')
        self.assertEqual(reals_field.type(), QVariant.List)
        self.assertEqual(reals_field.subType(), QVariant.Double)
        reals = features[0].attributes()[3]
        self.assertEqual(reals, [1.0, -232567.22])

        new_f = QgsFeature(l.fields())
        new_f['id'] = 2
        new_f['strings'] = ['simple', '"doubleQuote"', "'quote'", 'back\\slash']
        new_f['ints'] = [1, 2, 3, 4]
        new_f['reals'] = [1e67, 1e-56]
        r, fs = l.dataProvider().addFeatures([new_f])
        self.assertTrue(r)

        read_back = l.getFeature(new_f['id'])
        self.assertEqual(read_back['id'], new_f['id'])
        self.assertEqual(read_back['strings'], new_f['strings'])
        self.assertEqual(read_back['ints'], new_f['ints'])
        self.assertEqual(read_back['reals'], new_f['reals'])

    def test_arrays_write(self):
        """Test writing of layers with arrays"""
        l = QgsVectorLayer("dbname=%s table=test_arrays_write (geometry)" %
                           self.dbname, "test_arrays", "spatialite")
        self.assertTrue(l.isValid())

        new_f = QgsFeature(l.fields())
        new_f['id'] = 2
        new_f['array'] = ['simple', '"doubleQuote"', "'quote'", 'back\\slash']
        new_f['strings'] = ['simple', '"doubleQuote"', "'quote'", 'back\\slash']
        new_f['ints'] = [1, 2, 3, 4]
        new_f['reals'] = [1e67, 1e-56]
        r, fs = l.dataProvider().addFeatures([new_f])
        self.assertTrue(r)

        read_back = l.getFeature(new_f['id'])
        self.assertEqual(read_back['id'], new_f['id'])
        self.assertEqual(read_back['array'], new_f['array'])
        self.assertEqual(read_back['strings'], new_f['strings'])
        self.assertEqual(read_back['ints'], new_f['ints'])
        self.assertEqual(read_back['reals'], new_f['reals'])

        new_f = QgsFeature(l.fields())
        new_f['id'] = 3
        new_f['array'] = [1, 1.2345, '"doubleQuote"', "'quote'", 'back\\slash']
        new_f['strings'] = ['simple', '"doubleQuote"', "'quote'", 'back\\slash']
        new_f['ints'] = [1, 2, 3, 4]
        new_f['reals'] = [1e67, 1e-56]
        r, fs = l.dataProvider().addFeatures([new_f])
        self.assertTrue(r)

        read_back = l.getFeature(new_f['id'])
        self.assertEqual(read_back['id'], new_f['id'])
        self.assertEqual(read_back['array'], new_f['array'])
        self.assertEqual(read_back['strings'], new_f['strings'])
        self.assertEqual(read_back['ints'], new_f['ints'])
        self.assertEqual(read_back['reals'], new_f['reals'])

        read_back = l.getFeature(new_f['id'])

    def test_discover_relation(self):
        artist = QgsVectorLayer(f"dbname={self.dbname} table=test_relation_a (geometry)", "test_relation_a",
                                "spatialite")
        self.assertTrue(artist.isValid())
        track = QgsVectorLayer(f"dbname={self.dbname} table=test_relation_b (geometry)", "test_relation_b",
                               "spatialite")
        self.assertTrue(track.isValid())
        QgsProject.instance().addMapLayer(artist)
        QgsProject.instance().addMapLayer(track)
        try:
            relMgr = QgsProject.instance().relationManager()
            relations = relMgr.discoverRelations([], [artist, track])
            relations = {r.name(): r for r in relations}
            self.assertEqual({'fk_test_relation_b_0'}, set(relations.keys()))

            a2t = relations['fk_test_relation_b_0']
            self.assertTrue(a2t.isValid())
            self.assertEqual('test_relation_b', a2t.referencingLayer().name())
            self.assertEqual('test_relation_a', a2t.referencedLayer().name())
            self.assertEqual([2], a2t.referencingFields())
            self.assertEqual([0], a2t.referencedFields())
        finally:
            QgsProject.instance().removeMapLayer(track.id())
            QgsProject.instance().removeMapLayer(artist.id())

    def testNotNullConstraint(self):
        vl = QgsVectorLayer(f"dbname={self.dbname} table=test_constraints key='id'", "test_constraints",
                            "spatialite")
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 5)

        # test some bad field indexes
        self.assertEqual(vl.dataProvider().fieldConstraints(-1),
                         QgsFieldConstraints.Constraints())
        self.assertEqual(vl.dataProvider().fieldConstraints(
            1001), QgsFieldConstraints.Constraints())

        self.assertTrue(vl.dataProvider().fieldConstraints(0) &
                        QgsFieldConstraints.Constraint.ConstraintNotNull)
        self.assertTrue(vl.dataProvider().fieldConstraints(1) &
                        QgsFieldConstraints.Constraint.ConstraintNotNull)
        self.assertFalse(vl.dataProvider().fieldConstraints(2)
                         & QgsFieldConstraints.Constraint.ConstraintNotNull)
        self.assertFalse(vl.dataProvider().fieldConstraints(3)
                         & QgsFieldConstraints.Constraint.ConstraintNotNull)
        self.assertTrue(vl.dataProvider().fieldConstraints(4) &
                        QgsFieldConstraints.Constraint.ConstraintNotNull)

        # test that constraints have been saved to fields correctly
        fields = vl.fields()
        self.assertTrue(fields.at(0).constraints().constraints()
                        & QgsFieldConstraints.Constraint.ConstraintNotNull)
        self.assertEqual(fields.at(0).constraints().constraintOrigin(QgsFieldConstraints.Constraint.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintOrigin.ConstraintOriginProvider)
        self.assertTrue(fields.at(1).constraints().constraints()
                        & QgsFieldConstraints.Constraint.ConstraintNotNull)
        self.assertEqual(fields.at(1).constraints().constraintOrigin(QgsFieldConstraints.Constraint.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintOrigin.ConstraintOriginProvider)
        self.assertFalse(fields.at(2).constraints().constraints()
                         & QgsFieldConstraints.Constraint.ConstraintNotNull)
        self.assertFalse(fields.at(3).constraints().constraints()
                         & QgsFieldConstraints.Constraint.ConstraintNotNull)
        self.assertTrue(fields.at(4).constraints().constraints()
                        & QgsFieldConstraints.Constraint.ConstraintNotNull)
        self.assertEqual(fields.at(4).constraints().constraintOrigin(QgsFieldConstraints.Constraint.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintOrigin.ConstraintOriginProvider)

    def testUniqueConstraint(self):
        vl = QgsVectorLayer(f"dbname={self.dbname} table=test_constraints key='id'", "test_constraints",
                            "spatialite")
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 5)

        # test some bad field indexes
        self.assertEqual(vl.dataProvider().fieldConstraints(-1),
                         QgsFieldConstraints.Constraints())
        self.assertEqual(vl.dataProvider().fieldConstraints(
            1001), QgsFieldConstraints.Constraints())

        self.assertTrue(vl.dataProvider().fieldConstraints(0)
                        & QgsFieldConstraints.Constraint.ConstraintUnique)
        self.assertFalse(vl.dataProvider().fieldConstraints(1)
                         & QgsFieldConstraints.Constraint.ConstraintUnique)
        self.assertTrue(vl.dataProvider().fieldConstraints(2)
                        & QgsFieldConstraints.Constraint.ConstraintUnique)
        self.assertFalse(vl.dataProvider().fieldConstraints(3)
                         & QgsFieldConstraints.Constraint.ConstraintUnique)
        self.assertTrue(vl.dataProvider().fieldConstraints(4)
                        & QgsFieldConstraints.Constraint.ConstraintUnique)

        # test that constraints have been saved to fields correctly
        fields = vl.fields()
        self.assertTrue(fields.at(0).constraints().constraints()
                        & QgsFieldConstraints.Constraint.ConstraintUnique)
        self.assertEqual(fields.at(0).constraints().constraintOrigin(QgsFieldConstraints.Constraint.ConstraintUnique),
                         QgsFieldConstraints.ConstraintOrigin.ConstraintOriginProvider)
        self.assertFalse(fields.at(1).constraints().constraints()
                         & QgsFieldConstraints.Constraint.ConstraintUnique)
        self.assertTrue(fields.at(2).constraints().constraints()
                        & QgsFieldConstraints.Constraint.ConstraintUnique)
        self.assertEqual(fields.at(2).constraints().constraintOrigin(QgsFieldConstraints.Constraint.ConstraintUnique),
                         QgsFieldConstraints.ConstraintOrigin.ConstraintOriginProvider)
        self.assertFalse(fields.at(3).constraints().constraints()
                         & QgsFieldConstraints.Constraint.ConstraintUnique)
        self.assertTrue(fields.at(4).constraints().constraints()
                        & QgsFieldConstraints.Constraint.ConstraintUnique)
        self.assertEqual(fields.at(4).constraints().constraintOrigin(QgsFieldConstraints.Constraint.ConstraintUnique),
                         QgsFieldConstraints.ConstraintOrigin.ConstraintOriginProvider)

    def testSkipConstraintCheck(self):
        vl = QgsVectorLayer(f"dbname={self.dbname} table=test_autoincrement", "test_autoincrement",
                            "spatialite")
        self.assertTrue(vl.isValid())

        self.assertTrue(
            vl.dataProvider().skipConstraintCheck(0, QgsFieldConstraints.Constraint.ConstraintUnique, "Autogenerate"))
        self.assertFalse(vl.dataProvider().skipConstraintCheck(
            0, QgsFieldConstraints.Constraint.ConstraintUnique, 123))

    # This test would fail. It would require turning on WAL
    def XXXXXtestLocking(self):

        temp_dbname = self.dbname + '.locking'
        shutil.copy(self.dbname, temp_dbname)

        vl = QgsVectorLayer("dbname=%s table=test_n (geometry)" %
                            temp_dbname, "test_n", "spatialite")
        self.assertTrue(vl.isValid())

        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.changeGeometry(
            1, QgsGeometry.fromWkt('POLYGON((0 0,1 0,1 1,0 1,0 0))')))

        # The iterator will take one extra connection
        myiter = vl.getFeatures()

        # Consume one feature but the iterator is still opened
        f = next(myiter)
        self.assertTrue(f.isValid())

        self.assertTrue(vl.commitChanges())

    def testDefaultValues(self):

        l = QgsVectorLayer("dbname=%s table='test_defaults' key='id'" %
                           self.dbname, "test_defaults", "spatialite")
        self.assertTrue(l.isValid())

        self.assertEqual(l.dataProvider().defaultValue(1), "qgis 'is good")
        self.assertEqual(l.dataProvider().defaultValue(2), 5)
        self.assertEqual(l.dataProvider().defaultValue(3), 5.7)
        self.assertFalse(l.dataProvider().defaultValue(4))

    def testVectorLayerUtilsCreateFeatureWithProviderDefaultLiteral(self):
        vl = QgsVectorLayer("dbname=%s table='test_defaults' key='id'" %
                            self.dbname, "test_defaults", "spatialite")
        self.assertEqual(vl.dataProvider().defaultValue(2), 5)

        f = QgsVectorLayerUtils.createFeature(vl)
        self.assertEqual(f.attributes(), [None, "qgis 'is good", 5, 5.7, None])

        # check that provider default literals do not take precedence over passed attribute values
        f = QgsVectorLayerUtils.createFeature(
            vl, attributes={1: 'qgis is great', 0: 3})
        self.assertEqual(f.attributes(), [3, "qgis is great", 5, 5.7, None])

        # test that vector layer default value expression overrides provider default literal
        vl.setDefaultValueDefinition(3, QgsDefaultValue("4*3"))
        f = QgsVectorLayerUtils.createFeature(
            vl, attributes={1: 'qgis is great', 0: 3})
        self.assertEqual(f.attributes(), [3, "qgis is great", 5, 12, None])

    def testCreateAttributeIndex(self):
        vl = QgsVectorLayer("dbname=%s table='test_defaults' key='id'" %
                            self.dbname, "test_defaults", "spatialite")
        self.assertTrue(vl.dataProvider().capabilities() &
                        QgsVectorDataProvider.Capability.CreateAttributeIndex)
        self.assertFalse(vl.dataProvider().createAttributeIndex(-1))
        self.assertFalse(vl.dataProvider().createAttributeIndex(100))
        self.assertTrue(vl.dataProvider().createAttributeIndex(1))

        con = spatialite_connect(self.dbname, isolation_level=None)
        cur = con.cursor()
        rs = cur.execute(
            "SELECT * FROM sqlite_master WHERE type='index' AND tbl_name='test_defaults'")
        res = [row for row in rs]
        self.assertEqual(len(res), 1)
        index_name = res[0][1]
        rs = cur.execute(f"PRAGMA index_info({index_name})")
        res = [row for row in rs]
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0][2], 'name')

        # second index
        self.assertTrue(vl.dataProvider().createAttributeIndex(2))
        rs = cur.execute(
            "SELECT * FROM sqlite_master WHERE type='index' AND tbl_name='test_defaults'")
        res = [row for row in rs]
        self.assertEqual(len(res), 2)
        indexed_columns = []
        for row in res:
            index_name = row[1]
            rs = cur.execute(f"PRAGMA index_info({index_name})")
            res = [row for row in rs]
            self.assertEqual(len(res), 1)
            indexed_columns.append(res[0][2])

        self.assertEqual(set(indexed_columns), {'name', 'number'})
        con.close()

    def testSubsetStringRegexp(self):
        """Check that the provider supports the REGEXP syntax"""

        testPath = f"dbname={self.dbname} table='test_filter' (geometry) key='id'"
        vl = QgsVectorLayer(testPath, 'test', 'spatialite')
        self.assertTrue(vl.isValid())
        vl.setSubsetString('"name" REGEXP \'[txe]{3}\'')
        self.assertEqual(vl.featureCount(), 4)
        del (vl)

    def testSubsetStringExtent_bug17863(self):
        """Check that the extent is correct when applied in the ctor and when
        modified after a subset string is set """

        def _lessdigits(s):
            return re.sub(r'(\d+\.\d{3})\d+', r'\1', s)

        testPath = f"dbname={self.dbname} table='test_filter' (geometry) key='id'"

        subSetString = '"name" = \'int\''
        subSet = f' sql={subSetString}'

        # unfiltered
        vl = QgsVectorLayer(testPath, 'test', 'spatialite')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.featureCount(), 8)
        unfiltered_extent = _lessdigits(vl.extent().toString())
        self.assertNotEqual('Empty', unfiltered_extent)
        del (vl)

        # filter after construction ...
        subSet_vl2 = QgsVectorLayer(testPath, 'test', 'spatialite')
        self.assertEqual(_lessdigits(
            subSet_vl2.extent().toString()), unfiltered_extent)
        self.assertEqual(subSet_vl2.featureCount(), 8)
        # ... apply filter now!
        subSet_vl2.setSubsetString(subSetString)
        self.assertEqual(subSet_vl2.featureCount(), 4)
        self.assertEqual(subSet_vl2.subsetString(), subSetString)
        self.assertNotEqual(_lessdigits(
            subSet_vl2.extent().toString()), unfiltered_extent)
        filtered_extent = _lessdigits(subSet_vl2.extent().toString())
        del (subSet_vl2)

        # filtered in constructor
        subSet_vl = QgsVectorLayer(
            testPath + subSet, 'subset_test', 'spatialite')
        self.assertEqual(subSet_vl.subsetString(), subSetString)
        self.assertTrue(subSet_vl.isValid())

        # This was failing in bug 17863
        self.assertEqual(subSet_vl.featureCount(), 4)
        self.assertEqual(_lessdigits(
            subSet_vl.extent().toString()), filtered_extent)
        self.assertNotEqual(_lessdigits(
            subSet_vl.extent().toString()), unfiltered_extent)

        self.assertTrue(subSet_vl.setSubsetString(''))
        self.assertEqual(subSet_vl.featureCount(), 8)
        self.assertEqual(_lessdigits(
            subSet_vl.extent().toString()), unfiltered_extent)

    def testDecodeUri(self):
        """Check that the provider URI decoding returns expected values"""

        filename = '/home/to/path/test.db'
        uri = f'dbname=\'{filename}\' table="test" (geometry) key=testkey sql=1=1'
        registry = QgsProviderRegistry.instance()
        components = registry.decodeUri('spatialite', uri)
        self.assertEqual(components['path'], filename)
        self.assertEqual(components['layerName'], 'test')
        self.assertEqual(components['subset'], '1=1')
        self.assertEqual(components['geometryColumn'], 'geometry')
        self.assertEqual(components['keyColumn'], 'testkey')

    def testEncodeUri(self):
        """Check that the provider URI encoding returns expected values"""

        filename = '/home/to/path/test.db'
        registry = QgsProviderRegistry.instance()

        parts = {'path': filename,
                 'layerName': 'test',
                 'subset': '1=1',
                 'geometryColumn': 'geometry',
                 'keyColumn': 'testkey'}
        uri = registry.encodeUri('spatialite', parts)
        self.assertEqual(uri, f'dbname=\'{filename}\' key=\'testkey\' table="test" (geometry) sql=1=1')

    def testPKNotInt(self):
        """ Check when primary key is not an integer """
        # create test db
        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        dbname = os.path.join(tmpdir, "test_pknotint.sqlite")
        con = spatialite_connect(dbname, isolation_level=None)
        cur = con.cursor()

        # try the two different types of index creation
        for index_creation_method in ['CreateSpatialIndex', 'CreateMbrCache']:
            table_name = f"pk_is_string_{index_creation_method}"

            cur.execute("BEGIN")
            sql = "SELECT InitSpatialMetadata()"
            cur.execute(sql)

            # create table with spatial index and pk is string
            sql = "CREATE TABLE {}(id VARCHAR PRIMARY KEY NOT NULL, name TEXT NOT NULL);"
            cur.execute(sql.format(table_name))

            sql = "SELECT AddGeometryColumn('{}', 'geometry',  4326, 'POINT', 'XY')"
            cur.execute(sql.format(table_name))

            sql = "SELECT {}('{}', 'geometry')"
            cur.execute(sql.format(index_creation_method, table_name))

            sql = "insert into {} ('id', 'name', 'geometry') values( 'test_id', 'test_name', st_geomfromtext('POINT(1 2)', 4326))"
            cur.execute(sql.format(table_name))

            cur.execute("COMMIT")

            testPath = f"dbname={dbname} table='{table_name}' (geometry)"
            vl = QgsVectorLayer(testPath, 'test', 'spatialite')
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.featureCount(), 1)

            # make spatial request to force the index use
            request = QgsFeatureRequest(QgsRectangle(0, 0, 2, 3))
            feature = next(vl.getFeatures(request), None)
            self.assertTrue(feature)

            self.assertEqual(feature.id(), 1)
            point = feature.geometry().asPoint()
            self.assertTrue(point)
            self.assertEqual(point.x(), 1)
            self.assertEqual(point.y(), 2)

        con.close()

    def testLoadStyle(self):
        """Check that we can store and load a style"""

        # create test db
        dbname = os.path.join(tempfile.gettempdir(), "test_loadstyle.sqlite")
        if os.path.exists(dbname):
            os.remove(dbname)
        con = spatialite_connect(dbname, isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)

        # simple table with primary key
        sql = "CREATE TABLE test_pg (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)

        sql = "SELECT AddGeometryColumn('test_pg', 'geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)

        sql = "INSERT INTO test_pg (id, name, geometry) "
        sql += "VALUES (1, 'toto', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)

        cur.execute("COMMIT")
        con.close()

        testPath = f"dbname={dbname} table='test_pg' (geometry) key='id'"
        vl = QgsVectorLayer(testPath, 'test', 'spatialite')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.featureCount(), 1)
        err, ok = vl.loadDefaultStyle()
        self.assertFalse(ok)
        vl.saveStyleToDatabase('my_style', 'My description', True, '')
        err, ok = vl.loadDefaultStyle()
        self.assertTrue(ok)

    def testStyleStorage(self):

        # First test with invalid URI
        vl = QgsVectorLayer('/idont/exist.sqlite', 'test', 'spatialite')

        self.assertEqual(int(vl.dataProvider().styleStorageCapabilities()) & Qgis.ProviderStyleStorageCapability.LoadFromDatabase, 0)
        self.assertEqual(int(vl.dataProvider().styleStorageCapabilities()) & Qgis.ProviderStyleStorageCapability.SaveToDatabase, 0)

        res, err = QgsProviderRegistry.instance().styleExists('spatialite', '/idont/exist.sqlite', '')
        self.assertFalse(res)
        self.assertTrue(err)
        res, err = QgsProviderRegistry.instance().styleExists('spatialite', '/idont/exist.sqlite', 'a style')
        self.assertFalse(res)
        self.assertTrue(err)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, -1)
        self.assertEqual(idlist, [])
        self.assertEqual(namelist, [])
        self.assertEqual(desclist, [])
        self.assertTrue(errmsg)

        qml, errmsg = vl.getStyleFromDatabase("1")
        self.assertFalse(qml)
        self.assertTrue(errmsg)

        qml, success = vl.loadNamedStyle('/idont/exist.sqlite')
        self.assertFalse(success)

        errorMsg = vl.saveStyleToDatabase("name", "description", False, "")
        self.assertTrue(errorMsg)

        # create test db
        dbname = os.path.join(tempfile.gettempdir(), "test_stylehandling.sqlite")
        if os.path.exists(dbname):
            os.remove(dbname)
        con = spatialite_connect(dbname, isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)

        # simple table with primary key
        sql = "CREATE TABLE test_pg (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)

        sql = "SELECT AddGeometryColumn('test_pg', 'geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)

        sql = "INSERT INTO test_pg (id, name, geometry) "
        sql += "VALUES (1, 'toto', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)

        cur.execute("COMMIT")
        con.close()

        testPath = f"dbname={dbname} table='test_pg' (geometry) key='id'"
        vl = QgsVectorLayer(testPath, 'test', 'spatialite')
        self.assertTrue(vl.isValid())

        self.assertEqual(int(vl.dataProvider().styleStorageCapabilities()) & Qgis.ProviderStyleStorageCapability.LoadFromDatabase, Qgis.ProviderStyleStorageCapability.LoadFromDatabase)
        self.assertEqual(int(vl.dataProvider().styleStorageCapabilities()) & Qgis.ProviderStyleStorageCapability.SaveToDatabase, Qgis.ProviderStyleStorageCapability.SaveToDatabase)

        # style tables don't exist yet
        res, err = QgsProviderRegistry.instance().styleExists('spatialite', vl.source(), '')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('spatialite', vl.source(), 'a style')
        self.assertFalse(res)
        self.assertFalse(err)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 0)
        self.assertEqual(idlist, [])
        self.assertEqual(namelist, [])
        self.assertEqual(desclist, [])
        self.assertTrue(errmsg)

        qml, errmsg = vl.getStyleFromDatabase("not_existing")
        self.assertFalse(qml)
        self.assertTrue(errmsg)

        qml, success = vl.loadNamedStyle(f'{dbname}|layerid=0')
        self.assertFalse(success)

        errorMsg = vl.saveStyleToDatabase("name", "description", False, "")
        self.assertEqual(errorMsg, "")

        res, err = QgsProviderRegistry.instance().styleExists('spatialite', vl.source(), '')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('spatialite', vl.source(), 'a style')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('spatialite', vl.source(), 'name')
        self.assertTrue(res)
        self.assertFalse(err)

        qml, errmsg = vl.getStyleFromDatabase("not_existing")
        self.assertFalse(qml)
        self.assertTrue(errmsg)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 1)
        self.assertFalse(errmsg)
        self.assertEqual(idlist, ['1'])
        self.assertEqual(namelist, ['name'])
        self.assertEqual(desclist, ['description'])

        qml, errmsg = vl.getStyleFromDatabase("100")
        self.assertEqual(qml, "")
        self.assertNotEqual(errmsg, "")

        qml, errmsg = vl.getStyleFromDatabase("1")
        self.assertTrue(qml.startswith('<!DOCTYPE qgis'), qml)
        self.assertFalse(errmsg)

        # overwrite existing style
        settings = QgsSettings()
        settings.setValue("/qgis/overwriteStyle", True)
        errorMsg = vl.saveStyleToDatabase("name", "description_bis", False, "")
        self.assertFalse(errorMsg)

        res, err = QgsProviderRegistry.instance().styleExists('spatialite', vl.source(), 'name')
        self.assertTrue(res)
        self.assertFalse(err)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 1)
        self.assertFalse(errmsg)
        self.assertEqual(idlist, ['1'])
        self.assertEqual(namelist, ['name'])
        self.assertEqual(desclist, ['description_bis'])

        errorMsg = vl.saveStyleToDatabase("name_test2", "description_test2", True, "")
        self.assertFalse(errmsg)

        res, err = QgsProviderRegistry.instance().styleExists('spatialite', vl.source(), 'name_test2')
        self.assertTrue(res)
        self.assertFalse(err)

        errorMsg = vl.saveStyleToDatabase("name2", "description2", True, "")
        self.assertFalse(errmsg)

        res, err = QgsProviderRegistry.instance().styleExists('spatialite', vl.source(), 'name2')
        self.assertTrue(res)
        self.assertFalse(err)

        errorMsg = vl.saveStyleToDatabase("name3", "description3", True, "")
        self.assertFalse(errmsg)

        res, err = QgsProviderRegistry.instance().styleExists('spatialite', vl.source(), 'name3')
        self.assertTrue(res)
        self.assertFalse(err)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 4)
        self.assertFalse(errmsg)
        self.assertCountEqual(idlist, ['1', '3', '4', '2'])
        self.assertCountEqual(namelist, ['name', 'name2', 'name3', 'name_test2'])
        self.assertCountEqual(desclist, ['description_bis', 'description2', 'description3', 'description_test2'])

    def _aliased_sql_helper(self, dbname):
        queries = (
            '(SELECT * FROM (SELECT * from \\"some view\\"))',
            '(SELECT * FROM \\"some view\\")',
            '(select sd.* from somedata as sd left join somedata as sd2 on ( sd2.name = sd.name ))',
            '(select sd.* from \\"somedata\\" as sd left join \\"somedata\\" as sd2 on ( sd2.name = sd.name ))',
            "(SELECT * FROM somedata as my_alias1\n)",
            "(SELECT * FROM somedata as my_alias2)",
            "(SELECT * FROM somedata AS my_alias3)",
            '(SELECT * FROM \\"somedata\\" as my_alias4\n)',
            '(SELECT * FROM (SELECT * FROM \\"somedata\\"))',
            '(SELECT my_alias5.* FROM (SELECT * FROM \\"somedata\\") AS my_alias5)',
            '(SELECT my_alias6.* FROM (SELECT * FROM \\"somedata\\" as my_alias\n) AS my_alias6)',
            '(SELECT my_alias7.* FROM (SELECT * FROM \\"somedata\\" as my_alias\n) AS my_alias7\n)',
            '(SELECT my_alias8.* FROM (SELECT * FROM \\"some data\\") AS my_alias8)',
            '(SELECT my_alias9.* FROM (SELECT * FROM \\"some data\\" as my_alias\n) AS my_alias9)',
            '(SELECT my_alias10.* FROM (SELECT * FROM \\"some data\\" as my_alias\n) AS my_alias10\n)',
            '(select sd.* from \\"some data\\" as sd left join \\"some data\\" as sd2 on ( sd2.name = sd.name ))',
            '(SELECT * FROM \\"some data\\" as my_alias11\n)',
            '(SELECT * FROM \\"some data\\" as my_alias12)',
            '(SELECT * FROM \\"some data\\" AS my_alias13)',
            '(SELECT * from \\"some data\\" AS my_alias14\n)',
            '(SELECT * FROM (SELECT * from \\"some data\\"))',
        )
        for sql in queries:
            vl = QgsVectorLayer('dbname=\'{}\' table="{}" (geom) sql='.format(
                dbname, sql), 'test', 'spatialite')
            self.assertTrue(
                vl.isValid(), f'dbname: {dbname} - sql: {sql}')
            self.assertGreater(vl.featureCount(), 1)
            self.assertTrue(vl.isSpatial())

    def testPkLessQuery(self):
        """Test if features in queries with/without pk can be retrieved by id"""
        # create test db
        dbname = os.path.join(tempfile.gettempdir(), "test_pkless.sqlite")
        if os.path.exists(dbname):
            os.remove(dbname)
        con = spatialite_connect(dbname, isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)

        # simple table with primary key
        sql = "CREATE TABLE \"test pk\" (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)

        sql = "SELECT AddGeometryColumn('test pk', 'geometry', 4326, 'POINT', 'XY')"
        cur.execute(sql)

        for i in range(11, 21):
            sql = "INSERT INTO \"test pk\" (id, name, geometry) "
            sql += "VALUES ({id}, 'name {id}', GeomFromText('POINT({id} {id})', 4326))".format(id=i)
            cur.execute(sql)

        def _make_table(table_name):
            # simple table without primary key
            sql = f"CREATE TABLE \"{table_name}\" (name TEXT NOT NULL)"
            cur.execute(sql)

            sql = f"SELECT AddGeometryColumn('{table_name}', 'geom', 4326, 'POINT', 'XY')"
            cur.execute(sql)

            for i in range(11, 21):
                sql = f"INSERT INTO \"{table_name}\" (name, geom) "
                sql += "VALUES ('name {id}', GeomFromText('POINT({id} {id})', 4326))".format(id=i)
                cur.execute(sql)

        _make_table("somedata")
        _make_table("some data")

        sql = "CREATE VIEW \"some view\" AS SELECT * FROM \"somedata\""
        cur.execute(sql)

        cur.execute("COMMIT")
        con.close()

        def _check_features(vl, offset):
            self.assertEqual(vl.featureCount(), 10)
            i = 11
            for f in vl.getFeatures():
                self.assertTrue(f.isValid())
                self.assertTrue(vl.getFeature(i - offset).isValid())
                self.assertEqual(vl.getFeature(i - offset)
                                 ['name'], f'name {i}')
                self.assertEqual(f.id(), i - offset)
                self.assertEqual(f['name'], f'name {i}')
                self.assertEqual(f.geometry().asWkt(),
                                 'Point ({id} {id})'.format(id=i))
                i += 1

        vl_pk = QgsVectorLayer(f'dbname=\'{dbname}\' table="(select * from \\"test pk\\")" (geometry) sql=', 'pk',
                               'spatialite')
        self.assertTrue(vl_pk.isValid())
        _check_features(vl_pk, 0)

        vl_no_pk = QgsVectorLayer(f'dbname=\'{dbname}\' table="(select * from somedata)" (geom) sql=', 'pk',
                                  'spatialite')
        self.assertTrue(vl_no_pk.isValid())
        _check_features(vl_no_pk, 10)

        vl_no_pk = QgsVectorLayer(f'dbname=\'{dbname}\' table="(select * from \\"some data\\")" (geom) sql=', 'pk',
                                  'spatialite')
        self.assertTrue(vl_no_pk.isValid())
        _check_features(vl_no_pk, 10)

        # Test regression when sending queries with aliased tables from DB manager
        self._aliased_sql_helper(dbname)

    def testAliasedQueries(self):
        """Test regression when sending queries with aliased tables from DB manager"""

        dbname = TEST_DATA_DIR + '/provider/spatialite.db'
        self._aliased_sql_helper(dbname)

    def testTextPks(self):
        """Test regression when retrieving features from tables with text PKs, see #21176"""

        # create test db
        dbname = os.path.join(tempfile.gettempdir(), "test_text_pks.sqlite")
        if os.path.exists(dbname):
            os.remove(dbname)
        con = spatialite_connect(dbname, isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)

        # simple table with primary key
        sql = "CREATE TABLE test_pg (id TEXT NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)

        sql = "SELECT AddGeometryColumn('test_pg', 'geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)

        sql = "INSERT INTO test_pg (id, name, geometry) "
        sql += "VALUES ('one', 'toto', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)

        sql = "INSERT INTO test_pg (id, name, geometry) "
        sql += "VALUES ('two', 'bogo', GeomFromText('POLYGON((0 0,2 0,2 2,0 2,0 0))', 4326))"
        cur.execute(sql)

        cur.execute("COMMIT")
        con.close()

        def _test_db(testPath):
            vl = QgsVectorLayer(testPath, 'test', 'spatialite')
            self.assertTrue(vl.isValid())

            f = next(vl.getFeatures())
            self.assertTrue(f.isValid())
            fid = f.id()
            self.assertGreater(fid, 0)
            self.assertTrue(vl.getFeature(fid).isValid())
            f2 = next(vl.getFeatures(QgsFeatureRequest().setFilterFid(fid)))
            self.assertTrue(f2.isValid())
            self.assertEqual(f2.id(), f.id())
            self.assertEqual(f2.geometry().asWkt(), f.geometry().asWkt())

            for f in vl.getFeatures():
                self.assertTrue(f.isValid())
                self.assertTrue(vl.getFeature(f.id()).isValid())
                self.assertEqual(vl.getFeature(f.id()).id(), f.id())

        testPath = f"dbname={dbname} table='test_pg' (geometry) key='id'"
        _test_db(testPath)
        testPath = f"dbname={dbname} table='test_pg' (geometry)"
        _test_db(testPath)
        testPath = f"dbname={dbname} table='test_pg' key='id'"
        _test_db(testPath)
        testPath = f"dbname={dbname} table='test_pg'"
        _test_db(testPath)

    def testGeometryTypes(self):
        """Test creating db with various geometry types"""

        # create test db
        dbname = os.path.join(tempfile.gettempdir(),
                              "testGeometryTypes.sqlite")
        if os.path.exists(dbname):
            os.remove(dbname)
        con = spatialite_connect(dbname, isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)

        cur.execute("COMMIT")
        con.close()

        tests = [('Point', 'Point (0 0)', QgsWkbTypes.Type.Point),
                 ('PointZ', 'PointZ (0 0 10)', QgsWkbTypes.Type.PointZ),
                 ('Point25D', 'PointZ (0 0 10)', QgsWkbTypes.Type.PointZ),
                 ('MultiPoint', 'MultiPoint (0 0, 0 1)', QgsWkbTypes.Type.MultiPoint),
                 ('MultiPointZ', 'MultiPointZ ((0 0 10, 0 1 10))',
                  QgsWkbTypes.Type.MultiPointZ),
                 ('MultiPoint25D', 'MultiPointZ ((0 0 10, 0 1 10))',
                  QgsWkbTypes.Type.MultiPointZ),
                 ('LineString', 'LineString (0 0, 0 1)', QgsWkbTypes.Type.LineString),
                 ('LineStringZ', 'LineStringZ (0 0 10, 0 1 10)',
                  QgsWkbTypes.Type.LineStringZ),
                 ('LineString25D', 'LineStringZ (0 0 10, 0 1 10)',
                  QgsWkbTypes.Type.LineStringZ),
                 ('MultiLineString', 'MultiLineString (0 0, 0 1)',
                  QgsWkbTypes.Type.MultiLineString),
                 ('MultiLineStringZ', 'MultiLineStringZ ((0 0 10, 0 1 10))',
                  QgsWkbTypes.Type.MultiLineStringZ),
                 ('MultiLineString25D', 'MultiLineStringZ ((0 0 10, 0 1 10))',
                  QgsWkbTypes.Type.MultiLineStringZ),
                 ('Polygon', 'Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))', QgsWkbTypes.Type.Polygon),
                 ('PolygonZ', 'PolygonZ ((0 0 10, 0 1 10, 1 1 10, 1 0 10, 0 0 10))',
                  QgsWkbTypes.Type.PolygonZ),
                 ('Polygon25D', 'PolygonZ ((0 0 10, 0 1 10, 1 1 10, 1 0 10, 0 0 10))',
                  QgsWkbTypes.Type.PolygonZ),
                 ('MultiPolygon', 'MultiPolygon (((0 0, 0 1, 1 1, 1 0, 0 0)))',
                  QgsWkbTypes.Type.MultiPolygon),
                 ('MultiPolygonZ', 'MultiPolygonZ (((0 0 10, 0 1 10, 1 1 10, 1 0 10, 0 0 10)))',
                  QgsWkbTypes.Type.MultiPolygonZ),
                 ('MultiPolygon25D', 'MultiPolygonZ (((0 0 10, 0 1 10, 1 1 10, 1 0 10, 0 0 10)))',
                  QgsWkbTypes.Type.MultiPolygonZ)
                 ]
        for typeStr, wkt, qgisType in tests:
            ml = QgsVectorLayer(
                (typeStr + '?crs=epsg:4326&field=id:int'),
                typeStr,
                'memory')

            provider = ml.dataProvider()
            ft = QgsFeature()
            ft.setGeometry(QgsGeometry.fromWkt(wkt))
            res, features = provider.addFeatures([ft])

            layer = typeStr
            uri = f"dbname={dbname} table='{layer}' (geometry)"
            write_result, error_message = QgsVectorLayerExporter.exportLayer(ml,
                                                                             uri,
                                                                             'spatialite',
                                                                             ml.crs(),
                                                                             False,
                                                                             {},
                                                                             )
            self.assertEqual(
                write_result, QgsVectorLayerExporter.ExportError.NoError, error_message)

            vl = QgsVectorLayer(uri, typeStr, 'spatialite')
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.wkbType(), qgisType)

    def testBigint(self):
        """Test unique values bigint, see GH #33585"""

        l = QgsVectorLayer(f"dbname={self.dbname} table='test_bigint' (position) key='id'", "test_bigint",
                           "spatialite")
        self.assertTrue(l.isValid())
        self.assertEqual(l.uniqueValues(1), {1, 2})
        self.assertEqual(l.uniqueValues(0), {987654321012345, 987654321012346})

    def testSpatialiteDefaultValues(self):
        """Test whether in spatialite table with default values like CURRENT_TIMESTAMP or
        (datetime('now','localtime')) they are respected. See GH #33383"""

        # Create the test table

        dbname = os.path.join(tempfile.gettempdir(),
                              "test_default_values.sqlite")
        if os.path.exists(dbname):
            os.remove(dbname)
        con = spatialite_connect(dbname, isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)

        # simple table with primary key
        sql = """
        CREATE TABLE test_table_default_values (
            `id` integer primary key autoincrement,
            comment text,
            created_at_01 text DEFAULT (datetime('now','localtime')),
            created_at_02 text DEFAULT CURRENT_TIMESTAMP,
            anumber INTEGER DEFAULT 123,
            atext TEXT default 'My default'
        )
        """
        cur.execute(sql)
        cur.execute("COMMIT")
        con.close()

        vl = QgsVectorLayer(f"dbname='{dbname}' table='test_table_default_values'", 'test_table_default_values',
                            'spatialite')
        self.assertTrue(vl.isValid())

        # Save it for the test
        now = datetime.now()

        # Test default values
        dp = vl.dataProvider()
        self.assertEqual(dp.defaultValue(0), NULL)
        self.assertEqual(dp.defaultValue(1), NULL)
        # FIXME: This fails because there is no backend-side evaluation in this provider
        # self.assertTrue(dp.defaultValue(2).startswith(now.strftime('%Y-%m-%d')))
        self.assertTrue(dp.defaultValue(
            3).startswith(now.strftime('%Y-%m-%d')))
        self.assertEqual(dp.defaultValue(4), 123)
        self.assertEqual(dp.defaultValue(5), 'My default')

        self.assertEqual(dp.defaultValueClause(0), 'Autogenerate')
        self.assertEqual(dp.defaultValueClause(1), '')
        self.assertEqual(dp.defaultValueClause(
            2), "datetime('now','localtime')")
        self.assertEqual(dp.defaultValueClause(3), "CURRENT_TIMESTAMP")
        self.assertEqual(dp.defaultValueClause(4), '')
        self.assertEqual(dp.defaultValueClause(5), '')

        feature = QgsFeature(vl.fields())
        for idx in range(vl.fields().count()):
            default = vl.dataProvider().defaultValue(idx)
            if not default:
                feature.setAttribute(idx, 'A comment')
            else:
                feature.setAttribute(idx, default)

        self.assertTrue(vl.dataProvider().addFeature(feature))
        del (vl)

        # Verify
        vl2 = QgsVectorLayer(f"dbname='{dbname}' table='test_table_default_values'", 'test_table_default_values',
                             'spatialite')
        self.assertTrue(vl2.isValid())
        feature = next(vl2.getFeatures())
        self.assertEqual(feature.attribute(1), 'A comment')
        self.assertTrue(feature.attribute(
            2).startswith(now.strftime('%Y-%m-%d')))
        self.assertTrue(feature.attribute(
            3).startswith(now.strftime('%Y-%m-%d')))
        self.assertEqual(feature.attribute(4), 123)
        self.assertEqual(feature.attribute(5), 'My default')

    def testSpatialiteAspatialMultipleAdd(self):
        """Add multiple features in aspatial table. See GH #34379"""

        # Create the test table

        dbname = os.path.join(tempfile.gettempdir(),
                              "test_aspatial_multiple_edits.sqlite")
        if os.path.exists(dbname):
            os.remove(dbname)
        con = spatialite_connect(dbname, isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)

        # simple table with primary key
        sql = """
        CREATE TABLE "test_aspatial_multiple_edits"(pkuid integer primary key autoincrement,"id" integer,"note" text)
        """
        cur.execute(sql)
        cur.execute("COMMIT")
        con.close()

        vl = QgsVectorLayer(f"dbname='{dbname}' table='test_aspatial_multiple_edits'", 'test_aspatial_multiple_edits',
                            'spatialite')
        self.assertTrue(vl.isValid())

        self.assertTrue(vl.startEditing())
        f1 = QgsFeature(vl.fields())
        f1.setAttribute('note', 'a note')
        f1.setAttribute('id', 123)
        f2 = QgsFeature(vl.fields())
        f2.setAttribute('note', 'another note')
        f2.setAttribute('id', 456)
        self.assertTrue(vl.addFeatures([f1, f2]))
        self.assertTrue(vl.commitChanges())

        # Verify
        self.assertEqual(vl.getFeature(1).attributes(), [1, 123, 'a note'])
        self.assertEqual(vl.getFeature(2).attributes(),
                         [2, 456, 'another note'])

    def testAddFeatureNoFields(self):
        """Test regression #34696"""

        vl = QgsVectorLayer("dbname=%s table='test_nofields' (geometry)" %
                            self.dbname, "test_nofields", "spatialite")
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.startEditing())
        f = QgsFeature(vl.fields())
        g = QgsGeometry.fromWkt('point(9 45)')
        f.setGeometry(g)
        self.assertTrue(vl.addFeatures([f]))
        self.assertTrue(vl.commitChanges())
        vl = QgsVectorLayer("dbname=%s table='test_nofields' (geometry)" %
                            self.dbname, "test_nofields", "spatialite")
        self.assertEqual(vl.featureCount(), 1)
        self.assertEqual(vl.getFeature(
            1).geometry().asWkt().upper(), 'POINT (9 45)')

    def testBLOBType(self):
        """Test binary field"""
        vl = QgsVectorLayer(f'dbname={self.dbname} table="blob_table" sql=', "testBLOBType", "spatialite")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('fld1')).type(), QVariant.ByteArray)

        values = {feat['id']: feat['fld1'] for feat in vl.getFeatures()}
        expected = {
            1: QByteArray(b'\x00SQLite'),
            2: QByteArray(),
            3: QByteArray(b'SQLite')
        }
        self.assertEqual(values, expected)

        # change attribute value
        self.assertTrue(vl.dataProvider().changeAttributeValues(
            {1: {1: QByteArray(b'bbbvx')}}))
        values = {feat['id']: feat['fld1'] for feat in vl.getFeatures()}
        expected = {
            1: QByteArray(b'bbbvx'),
            2: QByteArray(),
            3: QByteArray(b'SQLite')
        }
        self.assertEqual(values, expected)

        # add feature
        f = QgsFeature()
        f.setAttributes([4, QByteArray(b'cccc')])
        self.assertTrue(vl.dataProvider().addFeature(f))
        values = {feat['id']: feat['fld1'] for feat in vl.getFeatures()}
        expected = {
            1: QByteArray(b'bbbvx'),
            2: QByteArray(),
            3: QByteArray(b'SQLite'),
            4: QByteArray(b'cccc')
        }
        self.assertEqual(values, expected)

    def testTransaction(self):
        """Test spatialite transactions"""

        tmpfile = tempfile.mktemp('.db')
        ds = ogr.GetDriverByName('SQLite').CreateDataSource(
            tmpfile, options=['SPATIALITE=YES'])
        lyr = ds.CreateLayer('lyr1', geom_type=ogr.wkbPoint)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 1)'))
        lyr.CreateFeature(f)
        lyr = ds.CreateLayer('lyr2', geom_type=ogr.wkbPoint)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(2 3)'))
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(4 5)'))
        lyr.CreateFeature(f)
        ds = None

        uri1 = QgsDataSourceUri()
        uri1.setDatabase(tmpfile)
        uri1.setTable('lyr1')
        uri2 = QgsDataSourceUri()
        uri2.setDatabase(tmpfile)
        uri2.setTable('lyr2')

        vl1 = QgsVectorLayer(uri1.uri(), 'test', 'spatialite')
        self.assertTrue(vl1.isValid())
        vl2 = QgsVectorLayer(uri2.uri(), 'test', 'spatialite')
        self.assertTrue(vl2.isValid())

        # prepare a project with transactions enabled
        p = QgsProject()
        p.setTransactionMode(Qgis.TransactionMode.AutomaticGroups)
        p.addMapLayers([vl1, vl2])

        self.assertTrue(vl1.startEditing())
        self.assertIsNotNone(vl1.dataProvider().transaction())

        self.assertTrue(vl1.deleteFeature(1))

        # An iterator opened on the layer should see the feature deleted
        self.assertEqual(
            len([f for f in vl1.getFeatures(QgsFeatureRequest())]), 0)

        # But not if opened from another connection
        vl1_external = QgsVectorLayer(uri1.uri(), 'test', 'spatialite')
        self.assertTrue(vl1_external.isValid())
        self.assertEqual(
            len([f for f in vl1_external.getFeatures(QgsFeatureRequest())]), 1)
        del vl1_external

        self.assertTrue(vl1.commitChanges())

        # Should still get zero features on vl1
        self.assertEqual(
            len([f for f in vl1.getFeatures(QgsFeatureRequest())]), 0)
        self.assertEqual(
            len([f for f in vl2.getFeatures(QgsFeatureRequest())]), 2)

        # Test undo/redo
        self.assertTrue(vl2.startEditing())
        self.assertIsNotNone(vl2.dataProvider().transaction())
        self.assertTrue(vl2.editBuffer().deleteFeature(1))
        self.assertEqual(
            len([f for f in vl2.getFeatures(QgsFeatureRequest())]), 1)
        self.assertTrue(vl2.editBuffer().deleteFeature(2))
        self.assertEqual(
            len([f for f in vl2.getFeatures(QgsFeatureRequest())]), 0)
        vl2.undoStack().undo()
        self.assertEqual(
            len([f for f in vl2.getFeatures(QgsFeatureRequest())]), 1)
        vl2.undoStack().undo()
        self.assertEqual(
            len([f for f in vl2.getFeatures(QgsFeatureRequest())]), 2)
        vl2.undoStack().redo()
        self.assertEqual(
            len([f for f in vl2.getFeatures(QgsFeatureRequest())]), 1)
        self.assertTrue(vl2.commitChanges())

        self.assertEqual(
            len([f for f in vl2.getFeatures(QgsFeatureRequest())]), 1)
        del vl1
        del vl2

        vl2_external = QgsVectorLayer(uri2.uri(), 'test', 'spatialite')
        self.assertTrue(vl2_external.isValid())
        self.assertEqual(
            len([f for f in vl2_external.getFeatures(QgsFeatureRequest())]), 1)
        del vl2_external

    def testTransactions(self):
        """Test autogenerate"""

        vl = QgsVectorLayer("dbname=%s table=test_transactions1 ()" %
                            self.dbname, "test_transactions1", "spatialite")
        self.assertTrue(vl.isValid())
        vl2 = QgsVectorLayer("dbname=%s table=test_transactions2 ()" %
                             self.dbname, "test_transactions2", "spatialite")
        self.assertTrue(vl.isValid())
        self.assertTrue(vl2.isValid())
        self.assertEqual(vl.featureCount(), 0)
        self.assertEqual(vl2.featureCount(), 1)

        project = QgsProject()
        project.setTransactionMode(Qgis.TransactionMode.AutomaticGroups)
        project.addMapLayers([vl, vl2])
        project.setEvaluateDefaultValues(True)

        self.assertTrue(vl.startEditing())

        self.assertEqual(vl2.dataProvider().defaultValueClause(0), '')
        self.assertEqual(vl2.dataProvider().defaultValue(0), 2)

        self.assertEqual(vl.dataProvider().defaultValueClause(0), '')
        self.assertEqual(vl.dataProvider().defaultValue(0), 1)

    def testViewsExtentFilter(self):
        """Test extent filtering of a views-based spatialite layer"""

        vl = QgsVectorLayer("dbname='%s' table=\"vs_controle_ok_nok\" (geom)" %
                            os.path.join(TEST_DATA_DIR, "views_test.sqlite"), "vs_controle_ok_nok", "spatialite")
        self.assertTrue(vl.isValid())

        feature = QgsFeature()
        rect = QgsRectangle(822733, 6699265, 829351, 6707266)
        it = vl.getFeatures(rect)
        it.nextFeature(feature)

        self.assertTrue(feature.isValid())

    def testRegression50523(self):
        """Test for issue GH #50523"""

        con = spatialite_connect(self.dbname, isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = """CREATE TABLE table50523 (
            _id INTEGER PRIMARY KEY AUTOINCREMENT,
            atttext TEXT NOT NULL)"""

        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('table50523', 'position', 25832, 'POLYGON', 'XY', 0)"
        cur.execute(sql)
        cur.execute("COMMIT")
        con.close()

        layer = QgsVectorLayer(
            f'dbname=\'{self.dbname}\' table="table50523" (position) sql=', 'test', 'spatialite')

        self.assertTrue(layer.isValid())

        # Check NOT NULL constraint on atttext
        field = layer.fields().at(1)
        self.assertTrue(bool(field.constraints().constraints() & QgsFieldConstraints.Constraint.ConstraintNotNull))

        self.assertTrue(layer.startEditing())

        f = QgsFeature(layer.fields())
        g = QgsGeometry.fromWkt('polygon((0 0, 1 1, 1 0, 0 0))')
        g.isGeosValid()
        self.assertTrue(g.isGeosValid())
        f.setGeometry(g)
        f.fields()
        f.fields().names()
        f.setAttribute(1, NULL)
        f.setAttribute(0, 'Autogenerate')
        self.assertTrue(layer.addFeatures([f]))
        self.assertFalse(layer.commitChanges())
        self.assertTrue(layer.rollBack())

        self.assertTrue(layer.startEditing())
        f.setAttribute(1, 'some text')
        self.assertTrue(layer.addFeatures([f]))
        self.assertTrue(layer.commitChanges())

        layer = QgsVectorLayer(
            f'dbname=\'{self.dbname}\' table="table50523" (position) sql=', 'test', 'spatialite')
        self.assertEqual(len([f for f in layer.getFeatures()]), 1)

    def test_absolute_relative_uri(self):
        context = QgsReadWriteContext()
        context.setPathResolver(QgsPathResolver(os.path.join(TEST_DATA_DIR, "project.qgs")))

        absolute_uri = 'dbname=\'{}\' table="somedata" (geom)'.format(os.path.join(TEST_DATA_DIR, 'provider', 'spatialite.db'))
        relative_uri = 'dbname=\'./provider/spatialite.db\' table="somedata" (geom)'

        meta = QgsProviderRegistry.instance().providerMetadata("spatialite")
        assert meta is not None

        self.assertEqual(meta.absoluteToRelativeUri(absolute_uri, context), relative_uri)
        self.assertEqual(meta.relativeToAbsoluteUri(relative_uri, context), absolute_uri)

    def testRegression54622Multisurface(self):

        con = spatialite_connect(self.dbname, isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = sql = """CREATE TABLE table54622 (
            _id INTEGER PRIMARY KEY AUTOINCREMENT)"""
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('table54622', 'geometry', 25832, 'MULTIPOLYGON', 'XY', 0)"
        cur.execute(sql)
        cur.execute("COMMIT")
        con.close()

        def _check_feature():
            layer = QgsVectorLayer(
                'dbname=\'{}\' table="table54622" (geometry) sql='.format(self.dbname), 'test', 'spatialite')
            feature = next(layer.getFeatures())
            self.assertFalse(feature.geometry().isNull())
            self.assertTrue(compareWkt(feature.geometry().asWkt(), 'MultiPolygon (((-0.886 0.135, -0.886 -0.038, -0.448 -0.070, -0.426 0.143, -0.886 0.135)))', 0.01))

        layer = QgsVectorLayer(
            'dbname=\'{}\' table="table54622" (geometry) sql='.format(self.dbname), 'test', 'spatialite')

        self.assertTrue(layer.isValid())
        feature = QgsFeature(layer.fields())
        geom = QgsGeometry.fromWkt('MULTISURFACE(CURVEPOLYGON(COMPOUNDCURVE((-0.886 0.135,-0.886 -0.038,-0.448 -0.070,-0.427 0.144,-0.886 0.135))))')
        feature.setGeometry(geom)
        self.assertTrue(layer.dataProvider().addFeatures([feature]))

        _check_feature()

        self.assertTrue(layer.dataProvider().changeFeatures({}, {feature.id(): geom}))

        _check_feature()

        self.assertTrue(layer.dataProvider().changeGeometryValues({feature.id(): geom}))

        _check_feature()

    def test_extent(self):
        layer2D = QgsVectorLayer(
            f"dbname={self.dbname} table='(select * from test_q)' (geometry)",
            "test_pg_query1", "spatialite")
        self.assertTrue(layer2D.isValid())
        self.assertEqual(layer2D.extent(), QgsRectangle(0, 0, 1, 1))
        self.assertEqual(layer2D.extent3D(), QgsBox3D(0, 0, float('nan'), 1, 1, float('nan')))

        layer3D = QgsVectorLayer(
            f"dbname={self.dbname} table='test_z2' (geometry) key='id'",
            "test_z2", "spatialite")
        self.assertTrue(layer3D.isValid())
        self.assertEqual(layer3D.extent(), QgsRectangle(-2, 3, 7, 12))
        self.assertEqual(layer3D.extent3D(), QgsBox3D(-2, 3, 2, 7, 12, 11))


if __name__ == '__main__':
    unittest.main()
