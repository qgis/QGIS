# -*- coding: utf-8 -*-
"""
Tests for auth manager WMS/WFS using QGIS Server through OAuth2
enabled qgis_wrapped_server.py.

This is an integration test for QGIS Desktop Auth Manager WFS and WMS provider
and QGIS Server WFS/WMS that check if QGIS can use a stored auth manager auth
configuration to access an OAuth2 Resource Owner Grant Flow protected endpoint.


From build dir, run: ctest -R PyQgsAuthManagerOAuth2OWSTest -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
import os
import sys
import re
import subprocess
import tempfile
import urllib
import stat
import json
import time
import random

__author__ = 'Alessandro Pasotti'
__date__ = '20/04/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from shutil import rmtree

from utilities import unitTestDataPath, waitServer
from qgis.core import (
    QgsApplication,
    QgsAuthMethodConfig,
    QgsVectorLayer,
    QgsRasterLayer,
)

from qgis.PyQt.QtNetwork import QSslCertificate

from qgis.testing import (
    start_app,
    unittest,
)

try:
    QGIS_SERVER_ENDPOINT_PORT = os.environ['QGIS_SERVER_ENDPOINT_PORT']
except:
    QGIS_SERVER_ENDPOINT_PORT = '0'  # Auto


QGIS_AUTH_DB_DIR_PATH = tempfile.mkdtemp()

os.environ['QGIS_AUTH_DB_DIR_PATH'] = QGIS_AUTH_DB_DIR_PATH

qgis_app = start_app()


def setup_oauth(username, password, token_uri, refresh_token_uri='', authcfg_id='oauth-2', authcfg_name='OAuth2 test configuration'):
    """Setup oauth configuration to access OAuth API,
    return authcfg_id on success, None on failure
    """
    cfgjson = {
        "accessMethod": 0,
        "apiKey": "",
        "clientId": "",
        "clientSecret": "",
        "configType": 1,
        "grantFlow": 2,
        "password": password,
        "persistToken": False,
        "redirectPort": '7070',
        "redirectUrl": "",
        "refreshTokenUrl": refresh_token_uri,
        "requestTimeout": '30',
        "requestUrl": "",
        "scope": "",
        "tokenUrl": token_uri,
        "username": username,
        "version": 1
    }

    if authcfg_id not in QgsApplication.authManager().availableAuthMethodConfigs():
        authConfig = QgsAuthMethodConfig('OAuth2')
        authConfig.setId(authcfg_id)
        authConfig.setName(authcfg_name)
        authConfig.setConfig('oauth2config', json.dumps(cfgjson))
        if QgsApplication.authManager().storeAuthenticationConfig(authConfig):
            return authcfg_id
    else:
        authConfig = QgsAuthMethodConfig()
        QgsApplication.authManager().loadAuthenticationConfig(authcfg_id, authConfig, True)
        authConfig.setName(authcfg_name)
        authConfig.setConfig('oauth2config', json.dumps(cfgjson))
        if QgsApplication.authManager().updateAuthenticationConfig(authConfig):
            return authcfg_id
    return None


class TestAuthManager(unittest.TestCase):

    @classmethod
    def setUpAuth(cls):
        """Run before all tests and set up authentication"""
        authm = QgsApplication.authManager()
        assert (authm.setMasterPassword('masterpassword', True))
        cls.sslrootcert_path = os.path.join(cls.certsdata_path, 'chains_subissuer-issuer-root_issuer2-root2.pem')
        assert os.path.isfile(cls.sslrootcert_path)
        os.chmod(cls.sslrootcert_path, stat.S_IRUSR)

        cls.sslrootcert = QSslCertificate.fromPath(cls.sslrootcert_path)
        assert cls.sslrootcert is not None
        authm.storeCertAuthorities(cls.sslrootcert)
        authm.rebuildCaCertsCache()
        authm.rebuildTrustedCaCertsCache()

        cls.server_cert = os.path.join(cls.certsdata_path, '127_0_0_1_ssl_cert.pem')
        cls.server_key = os.path.join(cls.certsdata_path, '127_0_0_1_ssl_key.pem')
        cls.server_rootcert = cls.sslrootcert_path
        os.chmod(cls.server_cert, stat.S_IRUSR)
        os.chmod(cls.server_key, stat.S_IRUSR)
        os.chmod(cls.server_rootcert, stat.S_IRUSR)

        os.environ['QGIS_SERVER_HOST'] = cls.hostname
        os.environ['QGIS_SERVER_PORT'] = str(cls.port)
        os.environ['QGIS_SERVER_OAUTH2_KEY'] = cls.server_key
        os.environ['QGIS_SERVER_OAUTH2_CERTIFICATE'] = cls.server_cert
        os.environ['QGIS_SERVER_OAUTH2_USERNAME'] = cls.username
        os.environ['QGIS_SERVER_OAUTH2_PASSWORD'] = cls.password
        os.environ['QGIS_SERVER_OAUTH2_AUTHORITY'] = cls.server_rootcert
        # Set default token expiration to 2 seconds, note that this can be
        # also controlled when issuing token requests by adding ttl=<int>
        # to the query string
        os.environ['QGIS_SERVER_OAUTH2_TOKEN_EXPIRES_IN'] = '2'

    @classmethod
    def setUpClass(cls):
        """Run before all tests:
        Creates an auth configuration"""
        cls.port = QGIS_SERVER_ENDPOINT_PORT
        # Clean env just to be sure
        env_vars = ['QUERY_STRING', 'QGIS_PROJECT_FILE']
        for ev in env_vars:
            try:
                del os.environ[ev]
            except KeyError:
                pass
        cls.testdata_path = unitTestDataPath('qgis_server')
        cls.certsdata_path = os.path.join(unitTestDataPath('auth_system'), 'certs_keys')
        cls.project_path = os.path.join(cls.testdata_path, "test_project.qgs")
        # cls.hostname = 'localhost'
        cls.protocol = 'https'
        cls.hostname = '127.0.0.1'
        cls.username = 'username'
        cls.password = 'password'
        cls.setUpAuth()

        server_path = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                                   'qgis_wrapped_server.py')
        cls.server = subprocess.Popen([sys.executable, server_path],
                                      env=os.environ, stdout=subprocess.PIPE)
        line = cls.server.stdout.readline()
        cls.port = int(re.findall(b':(\d+)', line)[0])
        assert cls.port != 0

        # We need a valid port before we setup the oauth configuration
        cls.token_uri = '%s://%s:%s/token' % (cls.protocol, cls.hostname, cls.port)
        cls.refresh_token_uri = '%s://%s:%s/refresh' % (cls.protocol, cls.hostname, cls.port)
        # Need a random authcfg or the cache will bites us back!
        cls.authcfg_id = setup_oauth(cls.username, cls.password, cls.token_uri, cls.refresh_token_uri, str(random.randint(0, 10000000)))
        # This is to test wrong credentials
        cls.wrong_authcfg_id = setup_oauth('wrong', 'wrong', cls.token_uri, cls.refresh_token_uri, str(random.randint(0, 10000000)))
        # Get the authentication configuration instance:
        cls.auth_config = QgsApplication.authManager().availableAuthMethodConfigs()[cls.authcfg_id]
        assert cls.auth_config.isValid()

        # Wait for the server process to start
        assert waitServer('%s://%s:%s' % (cls.protocol, cls.hostname, cls.port)), "Server is not responding! %s://%s:%s" % (cls.protocol, cls.hostname, cls.port)

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        cls.server.kill()
        rmtree(QGIS_AUTH_DB_DIR_PATH)
        del cls.server

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    @classmethod
    def _getWFSLayer(cls, type_name, layer_name=None, authcfg=None):
        """
        WFS layer factory
        """
        if layer_name is None:
            layer_name = 'wfs_' + type_name
        parms = {
            'srsname': 'EPSG:4326',
            'typename': type_name,
            'url': '%s://%s:%s/?map=%s' % (cls.protocol, cls.hostname, cls.port, cls.project_path),
            'version': 'auto',
            'table': '',
        }
        if authcfg is not None:
            parms.update({'authcfg': authcfg})
        uri = ' '.join([("%s='%s'" % (k, v)) for k, v in list(parms.items())])
        wfs_layer = QgsVectorLayer(uri, layer_name, 'WFS')
        return wfs_layer

    @classmethod
    def _getWMSLayer(cls, layers, layer_name=None, authcfg=None):
        """
        WMS layer factory
        """
        if layer_name is None:
            layer_name = 'wms_' + layers.replace(',', '')
        parms = {
            'crs': 'EPSG:4326',
            'url': '%s://%s:%s/?map=%s' % (cls.protocol, cls.hostname, cls.port, cls.project_path),
            'format': 'image/png',
            # This is needed because of a really weird implementation in QGIS Server, that
            # replaces _ in the the real layer name with spaces
            'layers': urllib.parse.quote(layers.replace('_', ' ')),
            'styles': '',
            'version': 'auto',
            # 'sql': '',
        }
        if authcfg is not None:
            parms.update({'authcfg': authcfg})
        uri = '&'.join([("%s=%s" % (k, v.replace('=', '%3D'))) for k, v in list(parms.items())])
        wms_layer = QgsRasterLayer(uri, layer_name, 'wms')
        return wms_layer

    def testNoAuthAccess(self):
        """
        Access the protected layer with no credentials
        """
        wms_layer = self._getWMSLayer('testlayer_èé')
        self.assertFalse(wms_layer.isValid())

    def testInvalidAuthAccess(self):
        """
        Access the protected layer with wrong credentials
        """
        wms_layer = self._getWMSLayer('testlayer_èé', authcfg=self.wrong_authcfg_id)
        self.assertFalse(wms_layer.isValid())

    def testValidAuthAccess(self):
        """
        Access the protected layer with valid credentials
        Note: cannot test invalid access WFS in a separate test  because
              it would fail the subsequent (valid) calls due to cached connections
        """
        wfs_layer = self._getWFSLayer('testlayer_èé', authcfg=self.auth_config.id())
        self.assertTrue(wfs_layer.isValid())
        wms_layer = self._getWMSLayer('testlayer_èé', authcfg=self.auth_config.id())
        self.assertTrue(wms_layer.isValid())


if __name__ == '__main__':
    unittest.main()
