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

from qgis.core import QgsPoint, QgsVectorLayer

from qgis.testing import start_app, unittest

from pyspatialite import dbapi2 as sqlite3

# Convenience instances in case you may need them
start_app()


def die(error_message):
    raise Exception(error_message)


class TestQgsSpatialiteProvider(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # create test db
        if os.path.exists("test.sqlite"):
            os.remove("test.sqlite")
        con = sqlite3.connect("test.sqlite", isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)

        # simple table with primary key
        sql = "CREATE TABLE test_mpg (id SERIAL PRIMARY KEY, name STRING NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_mpg', 'geometry', 4326, 'MULTIPOLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_mpg (name, geometry) "
        sql += "VALUES ('multipolygon with 8 squares', GeomFromText('MULTIPOLYGON("
        for i in range(0, 4, 2):
            for j in range(0, 4, 2):
                sql += "(("
                sql += str(i) + " " + str(j) + ","
                sql += str(i + 1) + " " + str(j) + ","
                sql += str(i + 1) + " " + str(j + 1) + ","
                sql += str(i) + " " + str(j + 1) + ","
                sql += str(i) + " " + str(j)
                sql += ")),"
        sql = sql[:-1]  # remove last comma
        sql += ")', 4326))"
        cur.execute(sql)

        sql = "CREATE TABLE test_pg (id SERIAL PRIMARY KEY, name STRING NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_pg', 'geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_pg (name, geometry) "
        sql += "VALUES ('polygon with interior ring', GeomFromText('POLYGON((0 0,3 0,3 3,0 3,0 0),(1 1,1 2,2 2,2 1,1 1))', 4326))"
        cur.execute(sql)
        cur.execute("COMMIT")
        con.close()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        # for the time beeing, keep the file to check with qgis
        # if os.path.exists("test.sqlite") :
        #    os.remove("test.sqlite")
        pass

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def test_SplitMultipolygon(self):
        """Split multipolygon"""
        layer = QgsVectorLayer("dbname=test.sqlite table=test_mpg (geometry)", "test_mpg", "spatialite")
        assert(layer.isValid())
        assert(layer.hasGeometryType())
        layer.featureCount() == 1 or die("wrong number of features")
        layer.startEditing()
        layer.splitFeatures([QgsPoint(0.5, -0.5), QgsPoint(0.5, 1.5)], 0) == 0 or die("error in split of one polygon of multipolygon")
        layer.splitFeatures([QgsPoint(2.5, -0.5), QgsPoint(2.5, 4)], 0) == 0 or die("error in split of two polygons of multipolygon at a time")
        layer.commitChanges() or die("this commit should work")
        layer.featureCount() == 7 or die("wrong number of features after 2 split")

    def test_SplitTruToCreateCutEdge(self):
        """Try to creat a cut edge"""
        layer = QgsVectorLayer("dbname=test.sqlite table=test_pg (geometry)", "test_pg", "spatialite")
        assert(layer.isValid())
        assert(layer.hasGeometryType())
        layer.featureCount() == 1 or die("wrong number of features")
        layer.startEditing()
        layer.splitFeatures([QgsPoint(1.5, -0.5), QgsPoint(1.5, 1.5)], 0) == 0 or die("error when trying to create an invalid polygon in split")
        layer.commitChanges() or die("this commit should work")
        layer.featureCount() == 1 or die("wrong number of features, polygon should be unafected by cut")


if __name__ == '__main__':
    unittest.main()
