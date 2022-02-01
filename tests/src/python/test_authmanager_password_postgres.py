# -*- coding: utf-8 -*-
"""
Tests for auth manager Password access to postgres.

This is an integration test for QGIS Desktop Auth Manager postgres provider that
checks if QGIS can use a stored auth manager auth configuration to access
a Password protected postgres.

From build dir, run: ctest -R PyQgsAuthManagerPasswordPostgresTest -V

It uses a docker container as postgres/postgis server with certificates from tests/testdata/auth_system/certs_keys_2048

Use docker-compose -f .docker/docker-compose-testing-postgres.yml up postgres to start the server

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

from shutil import rmtree
from contextlib import contextmanager

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

from qgis.testing import (
    start_app,
    unittest,
)

__author__ = 'Alessandro Pasotti'
__date__ = '25/10/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

qgis_app = start_app()


@contextmanager
def ScopedCertAuthority(username, password, sslrootcert_path=None):
    """
    Sets up the certificate authority in the authentication manager
    for the lifetime of this class and removes it when the class is deleted.
    """
    authm = QgsApplication.authManager()
    auth_config = QgsAuthMethodConfig("Basic")
    auth_config.setConfig('username', username)
    auth_config.setConfig('password', password)
    auth_config.setName('test_password_auth_config')
    if sslrootcert_path:
        sslrootcert = QSslCertificate.fromPath(sslrootcert_path)
        assert sslrootcert is not None
        authm.storeCertAuthorities(sslrootcert)
        authm.rebuildCaCertsCache()
        authm.rebuildTrustedCaCertsCache()
        authm.rebuildCertTrustCache()
    assert (authm.storeAuthenticationConfig(auth_config)[0])
    assert auth_config.isValid()
    yield auth_config
    if sslrootcert_path:
        for cert in sslrootcert:
            authm.removeCertAuthority(cert)
    authm.rebuildCaCertsCache()
    authm.rebuildTrustedCaCertsCache()
    authm.rebuildCertTrustCache()


class TestAuthManager(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests:
        Creates an auth configuration"""
        cls.username = 'docker'
        cls.password = 'docker'
        cls.dbname = 'qgis_test'
        cls.hostname = 'postgres'
        cls.port = '5432'

        authm = QgsApplication.authManager()
        assert (authm.setMasterPassword('masterpassword', True))
        cls.certsdata_path = os.path.join(unitTestDataPath('auth_system'), 'certs_keys_2048')
        cls.sslrootcert_path = os.path.join(cls.certsdata_path, 'qgis_ca.crt')

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    @classmethod
    def _getPostGISLayer(cls, type_name, layer_name=None, authcfg=None, sslmode=QgsDataSourceUri.SslVerifyFull):
        """
        PG layer factory
        """
        if layer_name is None:
            layer_name = 'pg_' + type_name
        uri = QgsDataSourceUri()
        uri.setWkbType(QgsWkbTypes.Point)
        uri.setConnection(cls.hostname, cls.port, cls.dbname, "", "", sslmode, authcfg)
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
        with ScopedCertAuthority(self.username, self.password, self.sslrootcert_path) as auth_config:
            pg_layer = self._getPostGISLayer('testlayer_èé', authcfg=auth_config.id())
            self.assertTrue(pg_layer.isValid())

    def testInvalidAuthAccess(self):
        """
        Access the protected layer with invalid credentials
        """
        with ScopedCertAuthority(self.username, self.password, self.sslrootcert_path) as auth_config:
            pg_layer = self._getPostGISLayer('testlayer_èé')
            self.assertFalse(pg_layer.isValid())

    def testSslRequireNoCaCheck(self):
        """
        Access the protected layer with valid credentials and ssl require but without the required cert authority.
        This should work.
        """
        with ScopedCertAuthority(self.username, self.password) as auth_config:
            pg_layer = self._getPostGISLayer('testlayer_èé', authcfg=auth_config.id(), sslmode=QgsDataSourceUri.SslRequire)
            self.assertTrue(pg_layer.isValid())

    def testSslVerifyFullCaCheck(self):
        """
        Access the protected layer with valid credentials and ssl verify full but without the required cert authority.
        This should not work.
        """
        with ScopedCertAuthority(self.username, self.password) as auth_config:
            pg_layer = self._getPostGISLayer('testlayer_èé', authcfg=auth_config.id())
            self.assertFalse(pg_layer.isValid())


if __name__ == '__main__':
    unittest.main()
