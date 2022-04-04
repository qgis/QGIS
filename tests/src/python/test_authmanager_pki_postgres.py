# -*- coding: utf-8 -*-
"""
Tests for auth manager PKI access to postgres.

This is an integration test for QGIS Desktop Auth Manager postgres provider that
checks if QGIS can use a stored auth manager auth configuration to access
a PKI protected postgres.

From build dir, run: ctest -R PyQgsAuthManagerPKIPostgresTest -V

It uses a docker container as postgres/postgis server with certificates from tests/testdata/auth_system/certs_keys_2048

Use docker-compose -f .docker/docker-compose-testing-postgres.yml up postgres to start the server.

TODO:
    - Document how to restore the server data
    - Document how to use docker inspect to find the IP of the docker postgres server and set a host alias (or some other smart idea to do the same)

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
import os
import time
import signal
import stat
import subprocess
import tempfile
import glob

from shutil import rmtree

from utilities import unitTestDataPath
from qgis.core import (
    QgsApplication,
    QgsAuthManager,
    QgsAuthMethodConfig,
    QgsVectorLayer,
    QgsDataSourceUri,
    QgsWkbTypes,
)

from qgis.PyQt.QtNetwork import QSslCertificate
from qgis.PyQt.QtCore import QFile

from qgis.testing import (
    start_app,
    unittest,
)

__author__ = 'Alessandro Pasotti'
__date__ = '25/10/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

qgis_app = start_app()


class TestAuthManager(unittest.TestCase):

    @classmethod
    def setUpAuth(cls):
        """Run before all tests and set up authentication"""
        authm = QgsApplication.authManager()
        assert (authm.setMasterPassword('masterpassword', True))
        # Client side
        cls.sslrootcert_path = os.path.join(cls.certsdata_path, 'qgis_ca.crt')
        cls.sslcert = os.path.join(cls.certsdata_path, 'docker.crt')
        cls.sslkey = os.path.join(cls.certsdata_path, 'docker.key')
        assert os.path.isfile(cls.sslcert)
        assert os.path.isfile(cls.sslkey)
        assert os.path.isfile(cls.sslrootcert_path)
        os.chmod(cls.sslcert, stat.S_IRUSR)
        os.chmod(cls.sslkey, stat.S_IRUSR)
        os.chmod(cls.sslrootcert_path, stat.S_IRUSR)
        cls.auth_config = QgsAuthMethodConfig("PKI-Paths")
        cls.auth_config.setConfig('certpath', cls.sslcert)
        cls.auth_config.setConfig('keypath', cls.sslkey)
        cls.auth_config.setName('test_pki_auth_config')
        cls.pg_user = 'docker'
        cls.pg_pass = 'docker'
        cls.pg_host = 'postgres'
        cls.pg_port = '5432'
        cls.pg_dbname = 'qgis_test'
        cls.sslrootcert = QSslCertificate.fromPath(cls.sslrootcert_path)
        assert cls.sslrootcert is not None
        authm.storeCertAuthorities(cls.sslrootcert)
        authm.rebuildCaCertsCache()
        authm.rebuildTrustedCaCertsCache()
        authm.rebuildCertTrustCache()
        assert (authm.storeAuthenticationConfig(cls.auth_config)[0])
        assert cls.auth_config.isValid()

    @classmethod
    def setUpClass(cls):
        """Run before all tests:
        Creates an auth configuration"""

        cls.certsdata_path = os.path.join(unitTestDataPath('auth_system'), 'certs_keys_2048')
        cls.setUpAuth()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        super().tearDownClass()

    def setUp(self):
        """Run before each test."""
        super().setUp()

    def tearDown(self):
        """Run after each test."""
        super().tearDown()

    @classmethod
    def _getPostGISLayer(cls, type_name, layer_name=None, authcfg=None):
        """
        PG layer factory
        """
        if layer_name is None:
            layer_name = 'pg_' + type_name
        uri = QgsDataSourceUri()
        uri.setWkbType(QgsWkbTypes.Point)
        uri.setConnection(cls.pg_host, cls.pg_port, cls.pg_dbname, cls.pg_user, cls.pg_pass, QgsDataSourceUri.SslVerifyFull, authcfg)
        uri.setKeyColumn('pk')
        uri.setSrid('EPSG:4326')
        uri.setDataSource('qgis_test', 'someData', "geom", "", "pk")
        # Note: do not expand here!
        layer = QgsVectorLayer(uri.uri(False), layer_name, 'postgres')
        return layer

    def testValidAuthAccess(self):
        """
        Access the protected layer with valid credentials
        """
        pg_layer = self._getPostGISLayer('testlayer_èé', authcfg=self.auth_config.id())
        self.assertTrue(pg_layer.isValid())

    def testInvalidAuthAccess(self):
        """
        Access the protected layer with not valid credentials
        """
        pg_layer = self._getPostGISLayer('testlayer_èé')
        self.assertFalse(pg_layer.isValid())

    def testRemoveTemporaryCerts(self):
        """
        Check that no temporary cert remain after connection with
        postgres provider
        """

        def cleanTempPki():
            pkies = glob.glob(os.path.join(tempfile.gettempdir(), 'tmp*_{*}.pem'))
            for fn in pkies:
                f = QFile(fn)
                f.setPermissions(QFile.WriteOwner)
                f.remove()

        # remove any temppki in temporary path to check that no
        # other pki remain after connection
        cleanTempPki()
        # connect using postgres provider
        pg_layer = self._getPostGISLayer('testlayer_èé', authcfg=self.auth_config.id())
        self.assertTrue(pg_layer.isValid())
        # do test no certs remained
        pkies = glob.glob(os.path.join(tempfile.gettempdir(), 'tmp*_{*}.pem'))
        self.assertEqual(len(pkies), 0)


if __name__ == '__main__':
    unittest.main()
