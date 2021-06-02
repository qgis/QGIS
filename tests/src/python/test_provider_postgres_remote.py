# -*- coding: utf-8 -*-
"""QGIS Unit tests for the remote PostgreSQL server.

Note: The test makes sense if the network latency
between QGIS and the PG server is 100ms.

Run with ctest -V -R PyQgsPostgresProviderRemote

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
from builtins import next

__author__ = 'Daryna Dyka'
__date__ = '2021-06-02'
__copyright__ = 'Copyright 2021, The QGIS Project'

import qgis  # NOQA
import psycopg2

import os
import time

from qgis.core import QgsVectorLayer, QgsFeatureRequest
from qgis.testing import start_app, unittest

QGISAPP = start_app()


class TestPyQgsPostgresProviderRemote(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.dbconn = 'service=qgis_test'
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
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

    def testSaveChangedGeometryToDB(self):
        """Test Save geometries to remote DB"""

        self.execSQLCommand('''
          DROP TABLE IF EXISTS qgis_test.speed_test_remote_db CASCADE;
          CREATE TABLE qgis_test.speed_test_remote_db (
            id_serial serial NOT NULL,
            geom geometry(Polygon,3857),
            CONSTRAINT speed_test_remote_db_pk PRIMARY KEY (id_serial) );
          INSERT INTO qgis_test.speed_test_remote_db(geom)
            SELECT
              ST_Translate(
                ST_GeomFromText(\'POLYGON((3396900.0 6521800.0,3396900.0 6521870.0,
                    3396830.0 6521870.0,3396830.0 6521800.0,3396900.0 6521800.0))\', 3857 ),
                100.0 * dx,
                100.0 * dy )
            FROM generate_series(1,42) dx, generate_series(1,42) dy;''')

        set_new_layer = ' sslmode=disable key=\'id_serial\' srid=3857 type=POLYGON table="qgis_test"."speed_test_remote_db" (geom) sql='
        error_string = 'Save geoms to remote DB : expected < 10s, got {}s'

        vl = QgsVectorLayer(self.dbconn + set_new_layer, 'test_vl_remote_save', 'postgres')
        # fids = [f.id() for f in vl.getFeatures(QgsFeatureRequest().setLimit(1000))]
        fids = [f.id() for f in vl.getFeatures(QgsFeatureRequest().setLimit(50))]
        self.assertTrue(vl.startEditing())
        for f in vl.getFeatures(fids):
            self.assertTrue(vl.changeGeometry(f.id(), f.geometry()))

        start_time = time.time()
        self.assertTrue(vl.commitChanges())
        duration = round(abs(time.time() - start_time), 1)
        self.assertTrue(duration < 10, error_string.format(duration))


if __name__ == '__main__':
    unittest.main()
