# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsSpatialiteProvider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Vincent Mora'
__date__ = '09/07/2013'
__copyright__ = 'Copyright 2013, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os
import re
import sys
import shutil
import tempfile

from qgis.core import (QgsProviderRegistry,
                       QgsVectorLayer,
                       QgsVectorDataProvider,
                       QgsPointXY,
                       QgsFeature,
                       QgsGeometry,
                       QgsProject,
                       QgsFieldConstraints,
                       QgsVectorLayerUtils,
                       QgsSettings,
                       QgsDefaultValue,
                       QgsWkbTypes,
                       QgsFeatureRequest,
                       QgsRectangle)

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from providertestbase import ProviderTestCase
from qgis.PyQt.QtCore import QObject, QVariant

from qgis.utils import spatialite_connect

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


class TestQgsSpatialiteProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        print(' ### Setup Spatialite Provider Test Class')
        # setup provider for base tests
        cls.vl = QgsVectorLayer('dbname=\'{}/provider/spatialite.db\' table="somedata" (geom) sql='.format(TEST_DATA_DIR), 'test', 'spatialite')
        assert(cls.vl.isValid())
        cls.source = cls.vl.dataProvider()

        cls.vl_poly = QgsVectorLayer('dbname=\'{}/provider/spatialite.db\' table="somepolydata" (geom) sql='.format(TEST_DATA_DIR), 'test', 'spatialite')
        assert(cls.vl_poly.isValid())
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

        # table with Z dimension geometry
        sql = "CREATE TABLE test_z (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_z', 'geometry', 4326, 'POINT', 'XYZ')"
        cur.execute(sql)
        sql = "INSERT INTO test_z (id, name, geometry) "
        sql += "VALUES (1, 'toto 2', GeomFromText('POINT Z (0 0 1)', 4326))"
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
        sql = "CREATE TABLE test_n (Id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
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
        sql = "CREATE TABLE test_arrays (Id INTEGER NOT NULL PRIMARY KEY, strings JSONSTRINGLIST NOT NULL, ints JSONINTEGERLIST NOT NULL, reals JSONREALLIST NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_arrays', 'Geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_arrays (id, strings, ints, reals, geometry) "
        sql += "VALUES (1, '[\"toto\",\"tutu\"]', '[1,-2,724562]', '[1.0, -232567.22]', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
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

        cur.execute("COMMIT")
        con.close()

        cls.dirs_to_cleanup = []

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        print(' ### Tear Down Spatialite Provider Test Class')

        # for the time being, keep the file to check with qgis
        # if os.path.exists(cls.dbname) :
        #    os.remove(cls.dbname)
        for dirname in cls.dirs_to_cleanup:
            shutil.rmtree(dirname, True)

    def getSource(self):
        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        datasource = os.path.join(tmpdir, 'spatialite.db')
        shutil.copy(os.path.join(srcpath, 'spatialite.db'), datasource)

        vl = QgsVectorLayer(
            'dbname=\'{}\' table="somedata" (geom) sql='.format(datasource), 'test',
            'spatialite')
        return vl

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
        return set(['cnt = 10 ^ 2',
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
                    'intersects(point_on_surface($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))'
                    ])

    def partiallyCompiledFilters(self):
        return set(['"name" NOT LIKE \'Ap%\'',
                    'name LIKE \'Apple\'',
                    'name LIKE \'aPple\''
                    ])

    def test_SplitFeature(self):
        """Create SpatiaLite database"""
        layer = QgsVectorLayer("dbname=%s table=test_pg (geometry)" % self.dbname, "test_pg", "spatialite")
        self.assertTrue(layer.isValid())
        self.assertTrue(layer.isSpatial())
        layer.startEditing()
        self.assertEqual(layer.splitFeatures([QgsPointXY(0.75, -0.5), QgsPointXY(0.75, 1.5)], 0), 0)
        self.assertEqual(layer.splitFeatures([QgsPointXY(-0.5, 0.25), QgsPointXY(1.5, 0.25)], 0), 0)
        self.assertTrue(layer.commitChanges())
        self.assertEqual(layer.featureCount(), 4)

    def test_SplitFeatureWithMultiKey(self):
        """Create SpatiaLite database"""
        layer = QgsVectorLayer("dbname=%s table=test_pg_mk (geometry)" % self.dbname, "test_pg_mk", "spatialite")
        self.assertTrue(layer.isValid())
        self.assertTrue(layer.isSpatial())
        layer.startEditing()
        self.assertEqual(layer.splitFeatures([QgsPointXY(0.5, -0.5), QgsPointXY(0.5, 1.5)], 0), 0)
        self.assertEqual(layer.splitFeatures([QgsPointXY(-0.5, 0.5), QgsPointXY(1.5, 0.5)], 0), 0)
        self.assertTrue(layer.commitChanges())

    def test_queries(self):
        """Test loading of query-based layers"""

        # a query with a geometry, but no unique id
        # the id will be autoincremented
        l = QgsVectorLayer("dbname=%s table='(select * from test_q)' (geometry)" % self.dbname, "test_pg_query1", "spatialite")
        self.assertTrue(l.isValid())
        # the id() is autoincremented
        sum_id1 = sum(f.id() for f in l.getFeatures())
        # the attribute 'id' works
        sum_id2 = sum(f.attributes()[0] for f in l.getFeatures())
        self.assertEqual(sum_id1, 32)  # 11 + 21
        self.assertEqual(sum_id2, 32)  # 11 + 21

        # and now with an id declared
        l = QgsVectorLayer("dbname=%s table='(select * from test_q)' (geometry) key='id'" % self.dbname, "test_pg_query1", "spatialite")
        self.assertTrue(l.isValid())
        sum_id1 = sum(f.id() for f in l.getFeatures())
        sum_id2 = sum(f.attributes()[0] for f in l.getFeatures())
        self.assertEqual(sum_id1, 32)
        self.assertEqual(sum_id2, 32)

        # a query, but no geometry
        l = QgsVectorLayer("dbname=%s table='(select id,name from test_q)' key='id'" % self.dbname, "test_pg_query1", "spatialite")
        self.assertTrue(l.isValid())
        sum_id1 = sum(f.id() for f in l.getFeatures())
        sum_id2 = sum(f.attributes()[0] for f in l.getFeatures())
        self.assertEqual(sum_id1, 32)
        self.assertEqual(sum_id2, 32)

    def test_zm(self):
        """Test Z dimension and M value"""
        l = QgsVectorLayer("dbname=%s table='test_z' (geometry) key='id'" % self.dbname, "test_z", "spatialite")
        self.assertTrue(l.isValid())
        self.assertTrue(QgsWkbTypes.hasZ(l.wkbType()))
        feature = l.getFeature(1)
        geom = feature.geometry().constGet()
        self.assertEqual(geom.z(), 1.0)

        l = QgsVectorLayer("dbname=%s table='test_m' (geometry) key='id'" % self.dbname, "test_m", "spatialite")
        self.assertTrue(l.isValid())
        self.assertTrue(QgsWkbTypes.hasM(l.wkbType()))
        feature = l.getFeature(1)
        geom = feature.geometry().constGet()
        self.assertEqual(geom.m(), 1.0)

        l = QgsVectorLayer("dbname=%s table='test_zm' (geometry) key='id'" % self.dbname, "test_zm", "spatialite")
        self.assertTrue(l.isValid())
        self.assertTrue(QgsWkbTypes.hasZ(l.wkbType()))
        self.assertTrue(QgsWkbTypes.hasM(l.wkbType()))
        feature = l.getFeature(1)
        geom = feature.geometry().constGet()
        self.assertEqual(geom.z(), 1.0)
        self.assertEqual(geom.m(), 1.0)

    def test_case(self):
        """Test case sensitivity issues"""
        l = QgsVectorLayer("dbname=%s table='test_n' (geometry) key='id'" % self.dbname, "test_n1", "spatialite")
        self.assertTrue(l.isValid())
        self.assertEqual(l.dataProvider().fields().count(), 2)
        fields = [f.name() for f in l.dataProvider().fields()]
        self.assertTrue('Geometry' not in fields)

    def test_invalid_iterator(self):
        """ Test invalid iterator """
        corrupt_dbname = self.dbname + '.corrupt'
        shutil.copy(self.dbname, corrupt_dbname)
        layer = QgsVectorLayer("dbname=%s table=test_pg (geometry)" % corrupt_dbname, "test_pg", "spatialite")
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

        vl = QgsVectorLayer("dbname=%s table=test_n (geometry)" % temp_dbname, "test_n", "spatialite")
        self.assertTrue(vl.isValid())
        # The iterator will take one extra connection
        myiter = vl.getFeatures()
        print((vl.featureCount()))
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

        vl = QgsVectorLayer("dbname=%s table=test_n (geometry)" % temp_dbname, "test_n", "spatialite")
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
        l = QgsVectorLayer("dbname=%s table=test_arrays (geometry)" % self.dbname, "test_arrays", "spatialite")
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

    def test_discover_relation(self):
        artist = QgsVectorLayer("dbname=%s table=test_relation_a (geometry)" % self.dbname, "test_relation_a", "spatialite")
        self.assertTrue(artist.isValid())
        track = QgsVectorLayer("dbname=%s table=test_relation_b (geometry)" % self.dbname, "test_relation_b", "spatialite")
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
        vl = QgsVectorLayer("dbname=%s table=test_constraints key='id'" % self.dbname, "test_constraints",
                            "spatialite")
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 5)

        # test some bad field indexes
        self.assertEqual(vl.dataProvider().fieldConstraints(-1), QgsFieldConstraints.Constraints())
        self.assertEqual(vl.dataProvider().fieldConstraints(1001), QgsFieldConstraints.Constraints())

        self.assertTrue(vl.dataProvider().fieldConstraints(0) & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(vl.dataProvider().fieldConstraints(1) & QgsFieldConstraints.ConstraintNotNull)
        self.assertFalse(vl.dataProvider().fieldConstraints(2) & QgsFieldConstraints.ConstraintNotNull)
        self.assertFalse(vl.dataProvider().fieldConstraints(3) & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(vl.dataProvider().fieldConstraints(4) & QgsFieldConstraints.ConstraintNotNull)

        # test that constraints have been saved to fields correctly
        fields = vl.fields()
        self.assertTrue(fields.at(0).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(fields.at(0).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull), QgsFieldConstraints.ConstraintOriginProvider)
        self.assertTrue(fields.at(1).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(fields.at(1).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull), QgsFieldConstraints.ConstraintOriginProvider)
        self.assertFalse(fields.at(2).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)
        self.assertFalse(fields.at(3).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(fields.at(4).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(fields.at(4).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull), QgsFieldConstraints.ConstraintOriginProvider)

    def testUniqueConstraint(self):
        vl = QgsVectorLayer("dbname=%s table=test_constraints key='id'" % self.dbname, "test_constraints",
                            "spatialite")
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 5)

        # test some bad field indexes
        self.assertEqual(vl.dataProvider().fieldConstraints(-1), QgsFieldConstraints.Constraints())
        self.assertEqual(vl.dataProvider().fieldConstraints(1001), QgsFieldConstraints.Constraints())

        self.assertTrue(vl.dataProvider().fieldConstraints(0) & QgsFieldConstraints.ConstraintUnique)
        self.assertFalse(vl.dataProvider().fieldConstraints(1) & QgsFieldConstraints.ConstraintUnique)
        self.assertTrue(vl.dataProvider().fieldConstraints(2) & QgsFieldConstraints.ConstraintUnique)
        self.assertFalse(vl.dataProvider().fieldConstraints(3) & QgsFieldConstraints.ConstraintUnique)
        self.assertTrue(vl.dataProvider().fieldConstraints(4) & QgsFieldConstraints.ConstraintUnique)

        # test that constraints have been saved to fields correctly
        fields = vl.fields()
        self.assertTrue(fields.at(0).constraints().constraints() & QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(fields.at(0).constraints().constraintOrigin(QgsFieldConstraints.ConstraintUnique), QgsFieldConstraints.ConstraintOriginProvider)
        self.assertFalse(fields.at(1).constraints().constraints() & QgsFieldConstraints.ConstraintUnique)
        self.assertTrue(fields.at(2).constraints().constraints() & QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(fields.at(2).constraints().constraintOrigin(QgsFieldConstraints.ConstraintUnique), QgsFieldConstraints.ConstraintOriginProvider)
        self.assertFalse(fields.at(3).constraints().constraints() & QgsFieldConstraints.ConstraintUnique)
        self.assertTrue(fields.at(4).constraints().constraints() & QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(fields.at(4).constraints().constraintOrigin(QgsFieldConstraints.ConstraintUnique), QgsFieldConstraints.ConstraintOriginProvider)

    def testSkipConstraintCheck(self):
        vl = QgsVectorLayer("dbname=%s table=test_autoincrement" % self.dbname, "test_autoincrement",
                            "spatialite")
        self.assertTrue(vl.isValid())

        self.assertTrue(vl.dataProvider().skipConstraintCheck(0, QgsFieldConstraints.ConstraintUnique, str("Autogenerate")))
        self.assertFalse(vl.dataProvider().skipConstraintCheck(0, QgsFieldConstraints.ConstraintUnique, 123))

    # This test would fail. It would require turning on WAL
    def XXXXXtestLocking(self):

        temp_dbname = self.dbname + '.locking'
        shutil.copy(self.dbname, temp_dbname)

        vl = QgsVectorLayer("dbname=%s table=test_n (geometry)" % temp_dbname, "test_n", "spatialite")
        self.assertTrue(vl.isValid())

        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.changeGeometry(1, QgsGeometry.fromWkt('POLYGON((0 0,1 0,1 1,0 1,0 0))')))

        # The iterator will take one extra connection
        myiter = vl.getFeatures()

        # Consume one feature but the iterator is still opened
        f = next(myiter)
        self.assertTrue(f.isValid())

        self.assertTrue(vl.commitChanges())

    def testDefaultValues(self):

        l = QgsVectorLayer("dbname=%s table='test_defaults' key='id'" % self.dbname, "test_defaults", "spatialite")
        self.assertTrue(l.isValid())

        self.assertEqual(l.dataProvider().defaultValue(1), "qgis 'is good")
        self.assertEqual(l.dataProvider().defaultValue(2), 5)
        self.assertEqual(l.dataProvider().defaultValue(3), 5.7)
        self.assertFalse(l.dataProvider().defaultValue(4))

    def testVectorLayerUtilsCreateFeatureWithProviderDefaultLiteral(self):
        vl = QgsVectorLayer("dbname=%s table='test_defaults' key='id'" % self.dbname, "test_defaults", "spatialite")
        self.assertEqual(vl.dataProvider().defaultValue(2), 5)

        f = QgsVectorLayerUtils.createFeature(vl)
        self.assertEqual(f.attributes(), [None, "qgis 'is good", 5, 5.7, None])

        # check that provider default literals take precedence over passed attribute values
        f = QgsVectorLayerUtils.createFeature(vl, attributes={1: 'qgis is great', 0: 3})
        self.assertEqual(f.attributes(), [3, "qgis 'is good", 5, 5.7, None])

        # test that vector layer default value expression overrides provider default literal
        vl.setDefaultValueDefinition(3, QgsDefaultValue("4*3"))
        f = QgsVectorLayerUtils.createFeature(vl, attributes={1: 'qgis is great', 0: 3})
        self.assertEqual(f.attributes(), [3, "qgis 'is good", 5, 12, None])

    def testCreateAttributeIndex(self):
        vl = QgsVectorLayer("dbname=%s table='test_defaults' key='id'" % self.dbname, "test_defaults", "spatialite")
        self.assertTrue(vl.dataProvider().capabilities() & QgsVectorDataProvider.CreateAttributeIndex)
        self.assertFalse(vl.dataProvider().createAttributeIndex(-1))
        self.assertFalse(vl.dataProvider().createAttributeIndex(100))
        self.assertTrue(vl.dataProvider().createAttributeIndex(1))

        con = spatialite_connect(self.dbname, isolation_level=None)
        cur = con.cursor()
        rs = cur.execute("SELECT * FROM sqlite_master WHERE type='index' AND tbl_name='test_defaults'")
        res = [row for row in rs]
        self.assertEqual(len(res), 1)
        index_name = res[0][1]
        rs = cur.execute("PRAGMA index_info({})".format(index_name))
        res = [row for row in rs]
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0][2], 'name')

        # second index
        self.assertTrue(vl.dataProvider().createAttributeIndex(2))
        rs = cur.execute("SELECT * FROM sqlite_master WHERE type='index' AND tbl_name='test_defaults'")
        res = [row for row in rs]
        self.assertEqual(len(res), 2)
        indexed_columns = []
        for row in res:
            index_name = row[1]
            rs = cur.execute("PRAGMA index_info({})".format(index_name))
            res = [row for row in rs]
            self.assertEqual(len(res), 1)
            indexed_columns.append(res[0][2])

        self.assertEqual(set(indexed_columns), set(['name', 'number']))
        con.close()

    def testSubsetStringExtent_bug17863(self):
        """Check that the extent is correct when applied in the ctor and when
        modified after a subset string is set """

        def _lessdigits(s):
            return re.sub(r'(\d+\.\d{3})\d+', r'\1', s)

        testPath = "dbname=%s table='test_filter' (geometry) key='id'" % self.dbname

        subSetString = '"name" = \'int\''
        subSet = ' sql=%s' % subSetString

        # unfiltered
        vl = QgsVectorLayer(testPath, 'test', 'spatialite')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.featureCount(), 8)
        unfiltered_extent = _lessdigits(vl.extent().toString())
        self.assertNotEqual('Empty', unfiltered_extent)
        del(vl)

        # filter after construction ...
        subSet_vl2 = QgsVectorLayer(testPath, 'test', 'spatialite')
        self.assertEqual(_lessdigits(subSet_vl2.extent().toString()), unfiltered_extent)
        self.assertEqual(subSet_vl2.featureCount(), 8)
        # ... apply filter now!
        subSet_vl2.setSubsetString(subSetString)
        self.assertEqual(subSet_vl2.featureCount(), 4)
        self.assertEqual(subSet_vl2.subsetString(), subSetString)
        self.assertNotEqual(_lessdigits(subSet_vl2.extent().toString()), unfiltered_extent)
        filtered_extent = _lessdigits(subSet_vl2.extent().toString())
        del(subSet_vl2)

        # filtered in constructor
        subSet_vl = QgsVectorLayer(testPath + subSet, 'subset_test', 'spatialite')
        self.assertEqual(subSet_vl.subsetString(), subSetString)
        self.assertTrue(subSet_vl.isValid())

        # This was failing in bug 17863
        self.assertEqual(subSet_vl.featureCount(), 4)
        self.assertEqual(_lessdigits(subSet_vl.extent().toString()), filtered_extent)
        self.assertNotEqual(_lessdigits(subSet_vl.extent().toString()), unfiltered_extent)

        self.assertTrue(subSet_vl.setSubsetString(''))
        self.assertEqual(subSet_vl.featureCount(), 8)
        self.assertEqual(_lessdigits(subSet_vl.extent().toString()), unfiltered_extent)

    def testDecodeUri(self):
        """Check that the provider URI decoding returns expected values"""

        filename = '/home/to/path/test.db'
        uri = 'dbname=\'{}\' table="test" (geometry) sql='.format(filename)
        registry = QgsProviderRegistry.instance()
        components = registry.decodeUri('spatialite', uri)
        self.assertEqual(components['path'], filename)

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

        testPath = "dbname=%s table='test_pg' (geometry) key='id'" % dbname
        vl = QgsVectorLayer(testPath, 'test', 'spatialite')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.featureCount(), 1)
        err, ok = vl.loadDefaultStyle()
        self.assertFalse(ok)
        vl.saveStyleToDatabase('my_style', 'My description', True, '')
        err, ok = vl.loadDefaultStyle()
        self.assertTrue(ok)

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
        sql = "CREATE TABLE test_pk (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)

        sql = "SELECT AddGeometryColumn('test_pk', 'geometry', 4326, 'POINT', 'XY')"
        cur.execute(sql)

        for i in range(11, 21):
            sql = "INSERT INTO test_pk (id, name, geometry) "
            sql += "VALUES ({id}, 'name {id}', GeomFromText('POINT({id} {id})', 4326))".format(id=i)
            cur.execute(sql)

        # simple table without primary key
        sql = "CREATE TABLE test_no_pk (name TEXT NOT NULL)"
        cur.execute(sql)

        sql = "SELECT AddGeometryColumn('test_no_pk', 'geometry', 4326, 'POINT', 'XY')"
        cur.execute(sql)

        for i in range(11, 21):
            sql = "INSERT INTO test_no_pk (name, geometry) "
            sql += "VALUES ('name {id}', GeomFromText('POINT({id} {id})', 4326))".format(id=i)
            cur.execute(sql)

        cur.execute("COMMIT")
        con.close()

        def _check_features(vl, offset):
            self.assertEqual(vl.featureCount(), 10)
            i = 11
            for f in vl.getFeatures():
                self.assertTrue(f.isValid())
                self.assertTrue(vl.getFeature(i - offset).isValid())
                self.assertEqual(vl.getFeature(i - offset)['name'], 'name {id}'.format(id=i))
                self.assertEqual(f.id(), i - offset)
                self.assertEqual(f['name'], 'name {id}'.format(id=i))
                self.assertEqual(f.geometry().asWkt(), 'Point ({id} {id})'.format(id=i))
                i += 1

        vl_pk = QgsVectorLayer('dbname=\'%s\' table="(select * from test_pk)" (geometry) sql=' % dbname, 'pk', 'spatialite')
        self.assertTrue(vl_pk.isValid())
        _check_features(vl_pk, 0)

        vl_no_pk = QgsVectorLayer('dbname=\'%s\' table="(select * from test_no_pk)" (geometry) sql=' % dbname, 'pk', 'spatialite')
        self.assertTrue(vl_no_pk.isValid())
        _check_features(vl_no_pk, 10)

        req = QgsFeatureRequest().setFilterRect(QgsRectangle(12.5, 12.5, 13.5, 13.5))
        self.assertEqual([(f.id(), f["name"]) for f in vl_no_pk.getFeatures(req)], [(3, "name 13")])

        req = QgsFeatureRequest().setFilterFid(5)
        self.assertEqual([(f.id(), f["name"]) for f in vl_no_pk.getFeatures(req)], [(5, "name 15")])

        req = QgsFeatureRequest().setFilterFids([5, 7, 9])
        self.assertEqual([(f.id(), f["name"]) for f in vl_no_pk.getFeatures(req)], [(5, "name 15"), (7, "name 17"), (9, "name 19")])

        req = QgsFeatureRequest().setFilterExpression("name = 'name 15'")
        self.assertEqual([(f.id(), f["name"]) for f in vl_no_pk.getFeatures(req)], [(5, "name 15")])

    def testViews(self):
        """Test if features in views have a consistent id"""
        # create test db
        dbname = os.path.join(tempfile.gettempdir(), "test_views.sqlite")
        if os.path.exists(dbname):
            os.remove(dbname)
        con = spatialite_connect(dbname, isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)

        # simple table with primary key
        sql = "CREATE TABLE test_table (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)

        sql = "SELECT AddGeometryColumn('test_table', 'geometry', 4326, 'POINT', 'XY')"
        cur.execute(sql)

        for i in range(11, 21):
            sql = "INSERT INTO test_table (id, name, geometry) "
            sql += "VALUES ({id}, 'name {id}', GeomFromText('POINT({id} {id})', 4326))".format(id=i)
            cur.execute(sql)

        # trickier: a table with an explicit rowid column
        sql = "CREATE TABLE test_table_rowid (rowid INTGER, id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)

        sql = "SELECT AddGeometryColumn('test_table_rowid', 'geometry', 4326, 'POINT', 'XY')"
        cur.execute(sql)

        for i in range(11, 21):
            sql = "INSERT INTO test_table_rowid (rowid, id, name, geometry) "
            sql += "VALUES ({id}, {id}, 'name {id}', GeomFromText('POINT({id} {id})', 4326))".format(id=i)
            cur.execute(sql)

        # now create views without pk
        sql = "CREATE VIEW test_view_no_pk_no_rowid AS SELECT * FROM test_table"
        cur.execute(sql)
        sql = "CREATE VIEW test_view_no_pk_rowid AS SELECT * FROM test_table_rowid"
        cur.execute(sql)

        # declare them as geometry tables
        sql = "INSERT INTO geometry_columns (f_table_name,f_geometry_column,geometry_type,coord_dimension,srid,spatial_index_enabled)"
        sql += " VALUES ('test_view_no_pk_no_rowid', 'geometry', 1, 2, 4326, 0)"
        cur.execute(sql)
        sql = "INSERT INTO geometry_columns (f_table_name,f_geometry_column,geometry_type,coord_dimension,srid,spatial_index_enabled)"
        sql += " VALUES ('test_view_no_pk_rowid', 'geometry', 1, 2, 4326, 0)"
        cur.execute(sql)

        # create a second view
        sql = "CREATE VIEW test_view_pk AS SELECT * FROM test_table"
        cur.execute(sql)

        # declare it as a view, where a rowid is mandatory
        sql = "INSERT INTO views_geometry_columns (view_name, view_geometry, view_rowid, f_table_name, f_geometry_column, read_only)"
        sql += " VALUES ('test_view_pk', 'geometry', 'id', 'test_table', 'geometry', 1)"
        cur.execute(sql)

        cur.execute("COMMIT")
        con.close()

        def _check_features(vl, offset):
            self.assertTrue(vl.isValid())

            req = QgsFeatureRequest().setFilterRect(QgsRectangle(12.5, 12.5, 13.5, 13.5))
            self.assertEqual([(f.id(), f["name"]) for f in vl.getFeatures(req)], [(13 - offset, "name 13")])

            req = QgsFeatureRequest().setFilterFid(15 - offset)
            self.assertEqual([(f.id(), f["name"]) for f in vl.getFeatures(req)], [(15 - offset, "name 15")])

            req = QgsFeatureRequest().setFilterFids([13 - offset, 15 - offset])
            self.assertEqual([(f.id(), f["name"]) for f in vl.getFeatures(req)], [(13 - offset, "name 13"), (15 - offset, "name 15")])

            req = QgsFeatureRequest().setFilterExpression("name = 'name 15'")
            self.assertEqual([(f.id(), f["name"]) for f in vl.getFeatures(req)], [(15 - offset, "name 15")])

        vl_pk = QgsVectorLayer('dbname=\'%s\' table="test_view_pk" (geometry) sql=' % dbname, 'view', 'spatialite')
        _check_features(vl_pk, 0)

        vl_no_pk_no_rowid = QgsVectorLayer('dbname=\'%s\' table="test_view_no_pk_no_rowid" (geometry) sql=' % dbname, 'view', 'spatialite')
        _check_features(vl_no_pk_no_rowid, 10)

        vl_no_pk_rowid = QgsVectorLayer('dbname=\'%s\' table="test_view_no_pk_rowid" (geometry) sql=' % dbname, 'view', 'spatialite')
        _check_features(vl_no_pk_no_rowid, 10)


if __name__ == '__main__':
    unittest.main()
