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
import sys
import shutil
import tempfile

from qgis.core import QgsVectorLayer, QgsPoint, QgsFeature

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from providertestbase import ProviderTestCase
from qgis.PyQt.QtCore import QSettings

try:
    from pyspatialite import dbapi2 as sqlite3
except ImportError:
    print("You should install pyspatialite to run the tests")
    raise ImportError

# Pass no_exit=True: for some reason this crashes on exit on Travis MacOSX
start_app(sys.platform != 'darwin')
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
        # setup provider for base tests
        cls.vl = QgsVectorLayer('dbname=\'{}/provider/spatialite.db\' table="somedata" (geom) sql='.format(TEST_DATA_DIR), 'test', 'spatialite')
        assert(cls.vl.isValid())
        cls.provider = cls.vl.dataProvider()

        cls.vl_poly = QgsVectorLayer('dbname=\'{}/provider/spatialite.db\' table="somepolydata" (geom) sql='.format(TEST_DATA_DIR), 'test', 'spatialite')
        assert(cls.vl_poly.isValid())
        cls.poly_provider = cls.vl_poly.dataProvider()

        # create test db
        cls.dbname = os.path.join(tempfile.gettempdir(), "test.sqlite")
        if os.path.exists(cls.dbname):
            os.remove(cls.dbname)
        con = sqlite3.connect(cls.dbname, isolation_level=None)
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

        # table with multiple column primary key
        sql = "CREATE TABLE test_pg_mk (id INTEGER NOT NULL, name TEXT NOT NULL, PRIMARY KEY(id,name))"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_pg_mk', 'geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_pg_mk (id, name, geometry) "
        sql += "VALUES (1, 'toto', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)

        # simple table with primary key
        sql = "CREATE TABLE test_q (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_q', 'geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_q (id, name, geometry) "
        sql += "VALUES (11, 'toto', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)
        sql = "INSERT INTO test_q (id, name, geometry) "
        sql += "VALUES (21, 'toto', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)

        # simple table with a geometry column named 'Geometry'
        sql = "CREATE TABLE test_n (Id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_n', 'Geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_n (id, name, geometry) "
        sql += "VALUES (1, 'toto', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)
        sql = "INSERT INTO test_n (id, name, geometry) "
        sql += "VALUES (2, 'toto', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)

        cur.execute("COMMIT")
        con.close()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        # for the time being, keep the file to check with qgis
        # if os.path.exists(cls.dbname) :
        #    os.remove(cls.dbname)
        pass

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def enableCompiler(self):
        QSettings().setValue(u'/qgis/compileExpressions', True)

    def disableCompiler(self):
        QSettings().setValue(u'/qgis/compileExpressions', False)

    def uncompiledFilters(self):
        return set(['cnt = 10 ^ 2',
                    '"name" ~ \'[OP]ra[gne]+\'',
                    'intersects($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))'])

    def partiallyCompiledFilters(self):
        return set(['"name" NOT LIKE \'Ap%\'',
                    'name LIKE \'Apple\'',
                    'name LIKE \'aPple\''
                    ])

    def test_SplitFeature(self):
        """Create spatialite database"""
        layer = QgsVectorLayer("dbname=%s table=test_pg (geometry)" % self.dbname, "test_pg", "spatialite")
        self.assertTrue(layer.isValid())
        self.assertTrue(layer.hasGeometryType())
        layer.startEditing()
        self.assertEqual(layer.splitFeatures([QgsPoint(0.5, -0.5), QgsPoint(0.5, 1.5)], 0), 0)
        self.assertEqual(layer.splitFeatures([QgsPoint(-0.5, 0.5), QgsPoint(1.5, 0.5)], 0), 0)
        self.assertTrue(layer.commitChanges())
        self.assertEqual(layer.featureCount(), 4)

    def xtest_SplitFeatureWithFailedCommit(self):
        """Create spatialite database"""
        layer = QgsVectorLayer("dbname=%s table=test_pg_mk (geometry)" % self.dbname, "test_pg_mk", "spatialite")
        self.assertTrue(layer.isValid())
        self.assertTrue(layer.hasGeometryType())
        layer.startEditing()
        self.asserEqual(layer.splitFeatures([QgsPoint(0.5, -0.5), QgsPoint(0.5, 1.5)], 0), 0)
        self.asserEqual(layer.splitFeatures([QgsPoint(-0.5, 0.5), QgsPoint(1.5, 0.5)], 0), 0)
        self.assertFalse(layer.commitChanges())
        layer.rollBack()
        feat = next(layer.getFeatures())
        ref = [[(0, 0), (1, 0), (1, 1), (0, 1), (0, 0)]]
        res = feat.geometry().asPolygon()
        for ring1, ring2 in zip(ref, res):
            for p1, p2 in zip(ring1, ring2):
                for c1, c2 in zip(p1, p2):
                    self.asserEqual(c1, c2)

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
        self.assertEqual(sum_id1, 3)   # 1+2
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
        open(corrupt_dbname, 'wb').write('')
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


if __name__ == '__main__':
    unittest.main()
