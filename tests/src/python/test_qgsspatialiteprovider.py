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

import os
import tempfile
import qgis
import sys

from qgis.core import *

from utilities import (getQgisTestApp,
                       TestCase,
                       unittest
                       )

try:
    from pyspatialite import dbapi2 as sqlite3
except ImportError:
    print "You should install pyspatialite to run the tests"
    sys.exit(0)

# Convenience instances in case you may need them
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


def die(error_message):
    raise Exception(error_message)

class TestQgsSpatialiteProvider(TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # create test db
	cls.dbname = os.path.join( tempfile.gettempdir(), "test.sqlite" )
        if os.path.exists( cls.dbname ):
            os.remove( cls.dbname )
        con = sqlite3.connect(cls.dbname)
        cur = con.cursor()
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)

        # simple table with primary key
        sql = "CREATE TABLE test_pg (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_pg', 'geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_pg (id, name, geometry) "
        sql +=    "VALUES (1, 'toto', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)

        # table with multiple column primary key
        sql = "CREATE TABLE test_pg_mk (id INTEGER NOT NULL, name TEXT NOT NULL, PRIMARY KEY(id,name))"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_pg_mk', 'geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_pg_mk (id, name, geometry) "
        sql +=    "VALUES (1, 'toto', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)

        con.commit()
        con.close()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        # for the time being, keep the file to check with qgis
        #if os.path.exists(cls.dbname) :
        #    os.remove(cls.dbname)
        pass

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def test_SplitFeature(self):
        """Create spatialite database"""
        layer = QgsVectorLayer("dbname=%s table=test_pg (geometry)" % self.dbname, "test_pg", "spatialite")
        assert(layer.isValid())
        assert(layer.hasGeometryType())
        layer.startEditing()
        layer.splitFeatures([QgsPoint(0.5, -0.5), QgsPoint(0.5, 1.5)], 0)==0 or die("error in split")
        layer.splitFeatures([QgsPoint(-0.5, 0.5), QgsPoint(1.5, 0.5)], 0)==0 or die("error in split")
        if not layer.commitChanges():
		die("this commit should work")
        layer.featureCount() == 4 or die("we should have 4 features after 2 split")

    def xtest_SplitFeatureWithFailedCommit(self):
        """Create spatialite database"""
        layer = QgsVectorLayer("dbname=%s table=test_pg_mk (geometry)" % self.dbname, "test_pg_mk", "spatialite")
        assert(layer.isValid())
        assert(layer.hasGeometryType())
        layer.startEditing()
        layer.splitFeatures([QgsPoint(0.5, -0.5), QgsPoint(0.5, 1.5)], 0)==0 or die("error in split")
        layer.splitFeatures([QgsPoint(-0.5, 0.5), QgsPoint(1.5, 0.5)], 0)==0 or die("error in split")
        if layer.commitChanges():
		die("this commit should fail")
        layer.rollBack()
        feat = QgsFeature()
        it=layer.getFeatures()
        it.nextFeature(feat)
        ref = [[(0,0), (1,0), (1,1), (0,1), (0,0)]]
        res = feat.geometry().asPolygon()
        for ring1, ring2 in zip(ref, res):
            for p1, p2 in zip(ring1, ring2):
                for c1, c2 in zip(p1, p2):
                    c1 == c2 or die("polygon has been altered by failed edition")

if __name__ == '__main__':
    unittest.main()
