# -*- coding: utf-8 -*-

"""
***************************************************************************
    connector_test.py
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
import qgis
from qgis.testing import start_app, unittest
from qgis.core import QgsDataSourceUri
from qgis.utils import iface

start_app()

from db_manager.db_plugins.postgis.connector import PostGisDBConnector


class TestDBManagerPostgisConnector(unittest.TestCase):

    #def setUpClass():

    def _getUser(self, connector):
        r = connector._execute(None, "SELECT USER")
        val = connector._fetchone(r)[0]
        connector._close_cursor(r)
        return val

    def _getDatabase(self, connector):
        r = connector._execute(None, "SELECT current_database()")
        val = connector._fetchone(r)[0]
        connector._close_cursor(r)
        return val

    # See https://github.com/qgis/QGIS/issues/24525
    # and https://github.com/qgis/QGIS/issues/19005
    def test_dbnameLessURI(self):
        c = PostGisDBConnector(QgsDataSourceUri())
        self.assertIsInstance(c, PostGisDBConnector)
        uri = c.uri()

        # No username was passed, so we expect it to be taken
        # from PGUSER or USER environment variables
        expected_user = os.environ.get('PGUSER') or os.environ.get('USER')
        actual_user = self._getUser(c)
        self.assertEqual(actual_user, expected_user)

        # No database was passed, so we expect it to be taken
        # from PGDATABASE or expected user
        expected_db = os.environ.get('PGDATABASE') or expected_user
        actual_db = self._getDatabase(c)
        self.assertEqual(actual_db, expected_db)

    # TODO: add service-only test (requires a ~/.pg_service.conf file)


if __name__ == '__main__':
    unittest.main()
