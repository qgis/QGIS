"""
Tests for auth manager PSQL storage API

From build dir, run from test directory:
LC_ALL=en_US.UTF-8 ctest -R PyQgsAuthManagerStoragePsql -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import unittest

from test_authmanager_storage_base import AuthManagerStorageBaseTestCase, TestAuthManagerStorageBase
from qgis.PyQt.QtCore import QTemporaryDir
from qgis.core import QgsAuthConfigurationStorageDb
from qgis.testing import QgisTestCase


__author__ = 'Alessandro Pasotti'
__date__ = '2024-06-24'
__copyright__ = 'Copyright 2024, The QGIS Project'


class TestAuthStoragePsql(AuthManagerStorageBaseTestCase, TestAuthManagerStorageBase):

    @classmethod
    def setUpClass(cls):
        """Run before each tests"""

        super().setUpClass()

        if 'QGIS_PGTEST_DB' not in os.environ:
            raise unittest.SkipTest('QGIS_PGTEST_DB not defined')

        cls.db_path = os.environ['QGIS_PGTEST_DB']

        config = dict()

        if cls.db_path.startswith('service='):
            try:
                # This needs to be installed with pip install pgserviceparser
                import pgserviceparser
                service_name = cls.db_path[8:]
                # Remove single quotes if present
                if service_name.startswith("'") and service_name.endswith("'"):
                    service_name = service_name[1:-1]
                config = pgserviceparser.service_config(service_name)
            except ImportError:
                raise unittest.SkipTest('QGIS_PGTEST_DB is a service connection string (which is not supported by QtSql) and pgserviceparser is not available')
        else:
            # Parse the connection string
            for item in cls.db_path.split():
                key, value = item.split('=')
                config[key] = value

        config['driver'] = 'QPSQL'
        config['database'] = config['dbname']

        # Remove single quotes if present in user and password and database
        for key in ['user', 'password', 'database']:
            if key in config and config[key].startswith("'") and config[key].endswith("'"):
                config[key] = config[key][1:-1]

        config['schema'] = 'qgis_test'

        cls.storage = QgsAuthConfigurationStorageDb(config)

        assert cls.storage.type() == 'DB-QPSQL'


if __name__ == '__main__':
    unittest.main()
