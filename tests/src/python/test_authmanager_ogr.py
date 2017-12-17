# -*- coding: utf-8 -*-
"""
Tests for auth manager Basic Auth OGR connection credentials injection

From build dir, run: ctest -R PyQgsAuthManagerOgr -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""


from qgis.core import (
    QgsApplication,
    QgsAuthManager,
    QgsAuthMethodConfig,
    QgsDataSourceUri,
    QgsProviderRegistry,
)

from qgis.testing import (
    start_app,
    unittest,
)


__author__ = 'Alessandro Pasotti'
__date__ = '14/11/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


qgis_app = start_app()

# Note: value is checked with "in" because some drivers may need additional arguments,
# like temporary paths with rootcerts for PG
TEST_URIS = {
    "http://mysite.com/geojson authcfg='%s'": "http://username:password@mysite.com/geojson",
    "PG:\"dbname='databasename' host='addr' port='5432' authcfg='%s'\"": "PG:\"dbname='databasename' host='addr' port='5432' user='username' password='password'",
    'SDE:127.0.0.1,12345,dbname, authcfg=\'%s\'': 'SDE:127.0.0.1,12345,dbname,username,password',
    'IDB:"server=demo_on user=informix dbname=frames authcfg=\'%s\'"': 'IDB:"server=demo_on user=informix dbname=frames user=username pass=password"',
    '@driver=ingres,dbname=test,tables=usa/canada authcfg=\'%s\'': '@driver=ingres,dbname=test,tables=usa/canada,userid=username,password=password',
    'MySQL:westholland,port=3306,tables=bedrijven authcfg=\'%s\'': 'MySQL:westholland,port=3306,tables=bedrijven,user=username,password=password',
    'MSSQL:server=.\MSSQLSERVER2008;database=dbname;trusted_connection=yes authcfg=\'%s\'': 'MSSQL:server=.\MSSQLSERVER2008;database=dbname;uid=username;pwd=password',
    'OCI:/@database_instance:table,table authcfg=\'%s\'': 'OCI:username/password@database_instance:table,table',
    'ODBC:database_instance authcfg=\'%s\'': 'ODBC:username/password@database_instance',
    'couchdb://myconnection authcfg=\'%s\'': 'couchdb://username:password@myconnection',
    'http://www.myconnection.com/geojson authcfg=\'%s\'': 'http://username:password@www.myconnection.com/geojson',
    'https://www.myconnection.com/geojson authcfg=\'%s\'': 'https://username:password@www.myconnection.com/geojson',
    'ftp://www.myconnection.com/geojson authcfg=\'%s\'': 'ftp://username:password@www.myconnection.com/geojson',
    'DODS://www.myconnection.com/geojson authcfg=\'%s\'': 'DODS://username:password@www.myconnection.com/geojson',
}


class TestAuthManager(unittest.TestCase):

    @classmethod
    def setUpAuth(cls):
        """Run before all tests and set up authentication"""
        authm = QgsApplication.authManager()
        assert (authm.setMasterPassword('masterpassword', True))
        # Client side
        cls.auth_config = QgsAuthMethodConfig("Basic")
        cls.auth_config.setConfig('username', cls.username)
        cls.auth_config.setConfig('password', cls.password)
        cls.auth_config.setName('test_basic_auth_config')
        assert (authm.storeAuthenticationConfig(cls.auth_config)[0])
        assert cls.auth_config.isValid()
        cls.authcfg = cls.auth_config.id()

    @classmethod
    def setUpClass(cls):
        """Run before all tests:
        Creates an auth configuration"""
        cls.username = 'username'
        cls.password = 'password'
        cls.dbname = 'test_basic'
        cls.hostname = 'localhost'
        cls.setUpAuth()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        pass

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def testConnections(self):
        """
        Test credentials injection
        """
        pr = QgsProviderRegistry.instance().createProvider('ogr', '')
        for uri, expanded in TEST_URIS.items():
            pr.setDataSourceUri(uri % self.authcfg)
            self.assertTrue(expanded in pr.dataSourceUri(True), "%s != %s" % (expanded, pr.dataSourceUri(True)))

        # Test sublayers
        for uri, expanded in TEST_URIS.items():
            pr.setDataSourceUri((uri + '|sublayer1') % self.authcfg)
            self.assertEqual(pr.dataSourceUri(True).split('|')[1], "sublayer1", pr.dataSourceUri(True))


if __name__ == '__main__':
    unittest.main()
