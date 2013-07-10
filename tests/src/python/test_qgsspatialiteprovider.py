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

from qgis.core import *

from utilities import (getQgisTestApp,
                       TestCase,
                       unittest
                       )

from pyspatialite import dbapi2 as sqlite3

# Convenience instances in case you may need them
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


def die(error_message):
    raise Exception(error_message)

class TestQgsSpatialiteProvider(TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        if os.path.exists("test.sqlite") :
            os.remove("test.sqlite")
        con = sqlite3.connect("test.sqlite")
        cur = con.cursor()
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)
        sql = "CREATE TABLE test_pg (id INTEGER NOT NULL, name TEXT NOT NULL, PRIMARY KEY (id, name))"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test_pg', 'geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test_pg (id, name, geometry) "
        sql +=    "VALUES (11, 'toto', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        rs = cur.execute(sql)
        con.commit()
        con.close()
        
    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        # for the time beeing, keep the file to check with qgis
        #if os.path.exists("test.sqlite") :
        #    os.remove("test.sqlite")
        pass

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def test_SplitFeature(self):
        """Create spatialite database"""
        layer = QgsVectorLayer("dbname=test.sqlite table=test_pg (geometry)", "test_pg", "spatialite")
        assert(layer.isValid())
        assert(layer.hasGeometryType())
        layer.startEditing()
        layer.splitFeatures([QgsPoint(0.5, -0.5), QgsPoint(0.5, 1.5)], 0)==0 or die("error in split")
        layer.commitChanges() or die("error in commit")
        


if __name__ == '__main__':
    unittest.main()


