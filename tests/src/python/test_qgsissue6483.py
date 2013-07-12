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
import random
import math

from qgis.core import *
from qgis.gui import *

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
        # create test db
        if os.path.exists("test.sqlite") :
            os.remove("test.sqlite")
        con = sqlite3.connect("test.sqlite")
        cur = con.cursor()
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)

        # growing size tables
        nbOrders = 7
        for order in range(0,nbOrders):
            size = 10000*pow(2,order);
            tab = "test_mpg_" + str(size) + "_rows"
            print "creating table " + tab + " " + str(order) + "/" + str(nbOrders)
            sql = "CREATE TABLE " + tab + " (id SERIAL PRIMARY KEY, height DOUBLE)"
            cur.execute(sql)
            sql = "SELECT AddGeometryColumn('" + tab + "', 'geometry', 4326, 'MULTIPOLYGON', 'XY')"
            cur.execute(sql)
            r = int(math.sqrt(size))
            for i in range(0,r):
                for j in range (0,r):
                    sql = "INSERT INTO " + tab + " (height, geometry) "
                    sql +=    "VALUES ("+str(random.random())+", GeomFromText('MULTIPOLYGON((("
                    sql += str(i)   + " " + str(j)   + ","
                    sql += str(i+1) + " " + str(j)   + ","
                    sql += str(i+1) + " " + str(j+1) + ","
                    sql += str(i)   + " " + str(j+1) + ","
                    sql += str(i)   + " " + str(j) + ")))', 4326))"
                    cur.execute(sql)
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

    def test_Nothing(self):
        """do nothing"""



if __name__ == '__main__':
    unittest.main()


