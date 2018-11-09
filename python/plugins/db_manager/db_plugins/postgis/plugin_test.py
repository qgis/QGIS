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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

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
        self.testdb = os.environ.get('QGIS_PGTEST_DB') or 'qgis_test'
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

    # See https://issues.qgis.org/issues/16625

    def test_rasterTableGdalURI(self):

        def check_rasterTableGdalURI(expected_dbname):
            tables = database.tables()
            raster_tables_count = 0
            for tab in tables:
                if tab.type == Table.RasterType:
                    raster_tables_count += 1
                    gdalUri = tab.gdalUri()
                    m = re.search(' dbname=\'([^ ]*)\' ', gdalUri)
                    self.assertTrue(m)
                    actual_dbname = m.group(1)
                    self.assertEqual(actual_dbname, expected_dbname)
                #print(tab.type)
                #print(tab.quotedName())
                #print(tab)

            # We need to make sure a database is created with at
            # least one raster table !
            self.assertEqual(raster_tables_count, 1)

        obj = QObject() # needs to be kept alive

        # Test for empty URI
        # See https://issues.qgis.org/issues/16625
        # and https://issues.qgis.org/issues/10600

        expected_dbname = self.testdb
        os.environ['PGDATABASE'] = expected_dbname

        database = PGDatabase(obj, QgsDataSourceUri())
        self.assertIsInstance(database, PGDatabase)

        uri = database.uri()
        self.assertEqual(uri.host(), '')
        self.assertEqual(uri.username(), '')
        self.assertEqual(uri.database(), expected_dbname)
        self.assertEqual(uri.service(), '')

        check_rasterTableGdalURI(expected_dbname)

        # Test for service-only URI
        # See https://issues.qgis.org/issues/16626

        os.environ['PGDATABASE'] = 'fake'
        database = PGDatabase(obj, QgsDataSourceUri('service=dbmanager'))
        self.assertIsInstance(database, PGDatabase)

        uri = database.uri()
        self.assertEqual(uri.host(), '')
        self.assertEqual(uri.username(), '')
        self.assertEqual(uri.database(), '')
        self.assertEqual(uri.service(), 'dbmanager')

        check_rasterTableGdalURI(expected_dbname)

    # See http://issues.qgis.org/issues/16833
    def test_unicodeInQuery(self):
        os.environ['PGDATABASE'] = self.testdb
        obj = QObject() # needs to be kept alive
        database = PGDatabase(obj, QgsDataSourceUri())
        self.assertIsInstance(database, PGDatabase)
        # SQL as string literal
        res = database.sqlResultModel("SELECT 'é'::text", obj)
        self.assertIsInstance(res, PGSqlResultModel)
        dat = res.getData(0, 0)
        self.assertEqual(dat, u"é")
        # SQL as unicode literal
        res = database.sqlResultModel(u"SELECT 'é'::text", obj)
        self.assertIsInstance(res, PGSqlResultModel)
        dat = res.getData(0, 0)
        self.assertEqual(dat, u"é")


if __name__ == '__main__':
    unittest.main()
