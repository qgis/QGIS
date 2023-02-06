# -*- coding: utf-8 -*-
"""QGIS Unit tests for the remote PostgreSQL server.

Note: The test makes sense if the network latency
between QGIS and the PG server is 100ms.

Run with ctest -V -R PyQgsPostgresProviderLatency

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = 'Daryna Dyka'
__date__ = '2021-06-13'
__copyright__ = 'Copyright 2021, The QGIS Project'

import os
import time

import psycopg2
import qgis  # NOQA
from qgis.core import QgsVectorLayer, QgsFeatureRequest
from qgis.testing import start_app, unittest

QGISAPP = start_app()


class TestPyQgsPostgresProviderLatency(unittest.TestCase):

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
        os.system('tc qdisc del dev eth0 root')

    def setDelay(self, delay_in_ms):
        if delay_in_ms == 0:
            os.system('tc qdisc del dev eth0 root')
        else:
            os.system('tc qdisc add dev eth0 root netem delay {}ms'.format(delay_in_ms))

    def getDelay(self):
        self.assertTrue(self.con)
        cur = self.con.cursor()
        self.assertTrue(cur)
        start_time = time.time()
        cur.execute('SELECT 1;')
        duration = round(1000 * abs(time.time() - start_time), 1)
        cur.close()
        self.con.commit()
        return duration

    def execSQLCommand(self, sql):
        self.assertTrue(self.con)
        cur = self.con.cursor()
        self.assertTrue(cur)
        cur.execute(sql)
        cur.close()
        self.con.commit()

    @unittest.skip("Skipping")
    def testStatsSetDelayToDB(self):
        """Test Set delay to remote DB"""

        delay_set = [0] * 100
        delay_get = [0] * 100
        delay_delta = [0] * 100
        for delay_ms in range(1, 100, 1):
            delay_set[delay_ms] = delay_ms
            self.setDelay(delay_ms)
            delay_get[delay_ms] = self.getDelay()
            self.setDelay(0)
            delay_delta[delay_ms] = round(delay_get[delay_ms] / 2.0 - delay_set[delay_ms], 1)

        self.assertTrue(False, '\nset: {}\nget: {}\nget/2 - set: {}'.format(delay_set, delay_get, delay_delta))

    def testSetDelayToDB(self):
        """Test Set delay to remote DB"""

        self.setDelay(100)
        delay_get = self.getDelay() / 2
        self.setDelay(0)
        self.assertTrue(delay_get > 90 and delay_get < 110, 'set delay to 100ms - unsuccessful (got: {}ms)'.format(delay_get))

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

        self.setDelay(100)
        start_time = time.time()
        self.assertTrue(vl.commitChanges())
        duration = round(abs(time.time() - start_time), 1)
        self.setDelay(0)
        self.assertTrue(duration < 10, error_string.format(duration))


if __name__ == '__main__':
    unittest.main()
