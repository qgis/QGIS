# -*- coding: utf-8 -*-
"""
Tests for auth manager WMS/WFS using QGIS Server through PKI
enabled qgis_wrapped_server.py.

This is an integration test for QGIS Desktop Auth Manager WFS and WMS provider
and QGIS Server WFS/WMS that check if QGIS can use a stored auth manager auth
configuration to access an HTTP Basic protected endpoint.


From build dir, run: ctest -R PyQgsAuthManagerPKIOWSTest -V

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

__author__ = 'Alessandro Pasotti'
__date__ = '25/10/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

from shutil import rmtree

from utilities import unitTestDataPath, waitServer
from qgis.core import (
    QgsApplication,
    QgsAuthManager,
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


class TestAuthManager(unittest.TestCase):

    @classmethod
    def setUpAuth(cls):
        """Run before all tests and set up authentication"""
        authm = QgsApplication.authManager()
        assert (authm.setMasterPassword('masterpassword', True))
        cls.sslrootcert_path = os.path.join(cls.certsdata_path, 'chains_subissuer-issuer-root_issuer2-root2.pem')
        cls.sslcert = os.path.join(cls.certsdata_path, 'gerardus_cert.pem')
        cls.sslkey = os.path.join(cls.certsdata_path, 'gerardus_key.pem')
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
        cls.username = 'Gerardus'
        cls.sslrootcert = QSslCertificate.fromPath(cls.sslrootcert_path)
        assert cls.sslrootcert is not None
        authm.storeCertAuthorities(cls.sslrootcert)
        authm.rebuildCaCertsCache()
        authm.rebuildTrustedCaCertsCache()
        assert (authm.storeAuthenticationConfig(cls.auth_config)[0])
        assert cls.auth_config.isValid()

        # cls.server_cert = os.path.join(cls.certsdata_path, 'localhost_ssl_cert.pem')
        cls.server_cert = os.path.join(cls.certsdata_path, '127_0_0_1_ssl_cert.pem')
        # cls.server_key = os.path.join(cls.certsdata_path, 'localhost_ssl_key.pem')
        cls.server_key = os.path.join(cls.certsdata_path, '127_0_0_1_ssl_key.pem')
        cls.server_rootcert = cls.sslrootcert_path
        os.chmod(cls.server_cert, stat.S_IRUSR)
        os.chmod(cls.server_key, stat.S_IRUSR)
        os.chmod(cls.server_rootcert, stat.S_IRUSR)

        os.environ['QGIS_SERVER_HOST'] = cls.hostname
        os.environ['QGIS_SERVER_PORT'] = str(cls.port)
        os.environ['QGIS_SERVER_PKI_KEY'] = cls.server_key
        os.environ['QGIS_SERVER_PKI_CERTIFICATE'] = cls.server_cert
        os.environ['QGIS_SERVER_PKI_USERNAME'] = cls.username
        os.environ['QGIS_SERVER_PKI_AUTHORITY'] = cls.server_rootcert

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

        cls.setUpAuth()

        server_path = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                                   'qgis_wrapped_server.py')
        cls.server = subprocess.Popen([sys.executable, server_path],
                                      env=os.environ, stdout=subprocess.PIPE)
        line = cls.server.stdout.readline()
        cls.port = int(re.findall(b':(\d+)', line)[0])
        assert cls.port != 0
        # Wait for the server process to start
        assert waitServer('%s://%s:%s' % (cls.protocol, cls.hostname, cls.port)), "Server is not responding! %s://%s:%s" % (cls.protocol, cls.hostname, cls.port)

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        cls.server.terminate()
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

    def testValidAuthAccess(self):
        """
        Access the protected layer with valid credentials
        Note: cannot test invalid access in a separate test  because
              it would fail the subsequent (valid) calls due to cached connections
        """
        wfs_layer = self._getWFSLayer('testlayer_èé', authcfg=self.auth_config.id())
        self.assertTrue(wfs_layer.isValid())
        wms_layer = self._getWMSLayer('testlayer_èé', authcfg=self.auth_config.id())
        self.assertTrue(wms_layer.isValid())


if __name__ == '__main__':
    unittest.main()
