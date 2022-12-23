# -*- coding: utf-8 -*-

"""
***************************************************************************
    plugin_test.py
    ---------------------
    Date                 : May 2017
    Copyright            : (C) 2017, Sandro Santilli
    Email                : strk at kbt dot io
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Sandro Santilli'
__date__ = 'May 2017'
__copyright__ = '(C) 2017, Sandro Santilli'

import os
import re
import qgis
from qgis.testing import start_app, unittest
from qgis.core import QgsDataSourceUri
from qgis.utils import iface
from qgis.PyQt.QtCore import QObject

start_app()

from db_manager.db_plugins.postgis.plugin import PostGisDBPlugin, PGRasterTable
from db_manager.db_plugins.postgis.plugin import PGDatabase
from db_manager.db_plugins.postgis.data_model import PGSqlResultModel
from db_manager.db_plugins.plugin import Table
from db_manager.db_plugins.postgis.connector import PostGisDBConnector


class TestDBManagerPostgisPlugin(unittest.TestCase):

    @classmethod
    def setUpClass(self):
        self.old_pgdatabase_env = os.environ.get('PGDATABASE')
        # QGIS_PGTEST_DB contains the full connection string and not only the DB name!
        QGIS_PGTEST_DB = os.environ.get('QGIS_PGTEST_DB')
        if QGIS_PGTEST_DB is not None:
            test_uri = QgsDataSourceUri(QGIS_PGTEST_DB)
            self.testdb = test_uri.database()
        else:
            self.testdb = 'qgis_test'
        os.environ['PGDATABASE'] = self.testdb

        # Create temporary service file
        self.old_pgservicefile_env = os.environ.get('PGSERVICEFILE')
        self.tmpservicefile = '/tmp/qgis-test-{}-pg_service.conf'.format(os.getpid())
        os.environ['PGSERVICEFILE'] = self.tmpservicefile

        f = open(self.tmpservicefile, "w")
        f.write("[dbmanager]\ndbname={}\n".format(self.testdb))
        # TODO: add more things if PGSERVICEFILE was already set ?
        f.close()

    @classmethod
    def tearDownClass(self):
        # Restore previous env variables if needed
        if self.old_pgdatabase_env:
            os.environ['PGDATABASE'] = self.old_pgdatabase_env
        if self.old_pgservicefile_env:
            os.environ['PGSERVICEFILE'] = self.old_pgservicefile_env
        # Remove temporary service file
        os.unlink(self.tmpservicefile)

    # See https://github.com/qgis/QGIS/issues/24525

    def test_rasterTableURI(self):

        def check_rasterTableURI(expected_dbname):
            tables = database.tables()
            raster_tables_count = 0
            for tab in tables:
                if tab.type == Table.RasterType:
                    raster_tables_count += 1
                    uri = tab.uri()
                    m = re.search(' dbname=\'([^ ]*)\' ', uri)
                    self.assertTrue(m)
                    actual_dbname = m.group(1)
                    self.assertEqual(actual_dbname, expected_dbname)
                # print(tab.type)
                # print(tab.quotedName())
                # print(tab)

            # We need to make sure a database is created with at
            # least one raster table !
            self.assertGreaterEqual(raster_tables_count, 1)

        obj = QObject()  # needs to be kept alive
        obj.connectionName = lambda: 'fake'
        obj.providerName = lambda: 'postgres'

        # Test for empty URI
        # See https://github.com/qgis/QGIS/issues/24525
        # and https://github.com/qgis/QGIS/issues/19005

        expected_dbname = self.testdb
        os.environ['PGDATABASE'] = expected_dbname

        database = PGDatabase(obj, QgsDataSourceUri())
        self.assertIsInstance(database, PGDatabase)

        uri = database.uri()
        self.assertEqual(uri.host(), '')
        self.assertEqual(uri.username(), '')
        self.assertEqual(uri.database(), expected_dbname)
        self.assertEqual(uri.service(), '')

        check_rasterTableURI(expected_dbname)

        # Test for service-only URI
        # See https://github.com/qgis/QGIS/issues/24526

        os.environ['PGDATABASE'] = 'fake'
        database = PGDatabase(obj, QgsDataSourceUri('service=dbmanager'))
        self.assertIsInstance(database, PGDatabase)

        uri = database.uri()
        self.assertEqual(uri.host(), '')
        self.assertEqual(uri.username(), '')
        self.assertEqual(uri.database(), '')
        self.assertEqual(uri.service(), 'dbmanager')

        check_rasterTableURI(expected_dbname)

    # See https://github.com/qgis/QGIS/issues/24732
    def test_unicodeInQuery(self):
        os.environ['PGDATABASE'] = self.testdb
        obj = QObject()  # needs to be kept alive
        obj.connectionName = lambda: 'fake'
        obj.providerName = lambda: 'postgres'
        database = PGDatabase(obj, QgsDataSourceUri())
        self.assertIsInstance(database, PGDatabase)
        # SQL as string literal
        res = database.sqlResultModel("SELECT 'é'::text", obj)
        self.assertIsInstance(res, PGSqlResultModel)
        dat = res.getData(0, 0)
        self.assertEqual(dat, "é")
        # SQL as unicode literal
        res = database.sqlResultModel("SELECT 'é'::text", obj)
        self.assertIsInstance(res, PGSqlResultModel)
        dat = res.getData(0, 0)
        self.assertEqual(dat, "é")


if __name__ == '__main__':
    unittest.main()
