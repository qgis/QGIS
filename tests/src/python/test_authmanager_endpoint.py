# -*- coding: utf-8 -*-
"""
Tests for auth manager WMS/WFS using QGIS Server through HTTP Basic
enabled qgis_wrapped_server.py.

This is an integration test for QGIS Desktop Auth Manager WFS and WMS provider
and QGIS Server WFS/WMS that check if QGIS can use a stored auth manager auth
configuration to access an HTTP Basic protected endpoint.


From build dir, run: ctest -R PyQgsAuthManagerEndpointTest -V

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
import random
import string
import urllib

__author__ = 'Alessandro Pasotti'
__date__ = '18/09/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from urllib.parse import quote
from shutil import rmtree

from utilities import unitTestDataPath, waitServer
from qgis.core import (
    QgsAuthManager,
    QgsAuthMethodConfig,
    QgsVectorLayer,
    QgsRasterLayer,
)
from qgis.testing import (
    start_app,
    unittest,
)


try:
    QGIS_SERVER_ENDPOINT_PORT = os.environ['QGIS_SERVER_ENDPOINT_PORT']
except:
    QGIS_SERVER_ENDPOINT_PORT = '0' # Auto


QGIS_AUTH_DB_DIR_PATH = tempfile.mkdtemp()

os.environ['QGIS_AUTH_DB_DIR_PATH'] = QGIS_AUTH_DB_DIR_PATH

qgis_app = start_app()


class TestAuthManager(unittest.TestCase):

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
        cls.testdata_path = unitTestDataPath('qgis_server') + '/'
        cls.project_path = quote(cls.testdata_path + "test_project.qgs")
        # Enable auth
        #os.environ['QGIS_AUTH_PASSWORD_FILE'] = QGIS_AUTH_PASSWORD_FILE
        authm = QgsAuthManager.instance()
        assert (authm.setMasterPassword('masterpassword', True))
        cls.auth_config = QgsAuthMethodConfig('Basic')
        cls.auth_config.setName('test_auth_config')
        cls.username = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))
        cls.password = cls.username[::-1] # reversed
        cls.auth_config.setConfig('username', cls.username)
        cls.auth_config.setConfig('password', cls.password)
        assert (authm.storeAuthenticationConfig(cls.auth_config)[0])

        os.environ['QGIS_SERVER_HTTP_BASIC_AUTH'] = '1'
        os.environ['QGIS_SERVER_USERNAME'] = cls.username
        os.environ['QGIS_SERVER_PASSWORD'] = cls.password
        os.environ['QGIS_SERVER_PORT'] = str(cls.port)
        server_path = os.path.dirname(os.path.realpath(__file__)) + \
            '/qgis_wrapped_server.py'
        cls.server = subprocess.Popen([sys.executable, server_path],
                                      env=os.environ, stdout=subprocess.PIPE)
        line = cls.server.stdout.readline()
        cls.port = int(re.findall(b':(\d+)', line)[0])
        assert cls.port != 0
        # Wait for the server process to start
        assert waitServer('http://127.0.0.1:%s' % cls.port), "Server is not responding!"

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        cls.server.terminate()
        cls.server.wait()
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
            'url': 'http://127.0.0.1:%s/?map=%s' % (cls.port, cls.project_path),
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
            'url': 'http://127.0.0.1:%s/?map=%s' % (cls.port, cls.project_path),
            'format': 'image/png',
            # This is needed because of a really weird implementation in QGIS Server, that
            # replaces _ in the the real layer name with spaces
            'layers': urllib.parse.quote(layers).replace('_', ' '),
            'styles': '',
            #'sql': '',
        }
        if authcfg is not None:
            parms.update({'authcfg': authcfg})
        uri = '&'.join([("%s=%s" % (k, v.replace('=', '%3D'))) for k, v in list(parms.items())])
        wms_layer = QgsRasterLayer(uri, layer_name, 'wms')
        return wms_layer

    def testValidAuthAccess(self):
        """
        Access the HTTP Basic protected layer with valid credentials
        """
        wfs_layer = self._getWFSLayer('testlayer_èé', authcfg=self.auth_config.id())
        self.assertTrue(wfs_layer.isValid())
        wms_layer = self._getWMSLayer('testlayer_èé', authcfg=self.auth_config.id())
        self.assertTrue(wms_layer.isValid())

    def testInvalidAuthAccess(self):
        """
        Access the HTTP Basic protected layer with no credentials
        """
        wfs_layer = self._getWFSLayer('testlayer_èé')
        self.assertFalse(wfs_layer.isValid())
        wms_layer = self._getWMSLayer('testlayer_èé')
        self.assertFalse(wms_layer.isValid())


if __name__ == '__main__':
    unittest.main()
