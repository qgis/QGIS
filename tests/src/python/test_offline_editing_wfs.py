# -*- coding: utf-8 -*-
"""
Offline editing Tests.

WFS-T tests need using QGIS Server through
qgis_wrapped_server.py.

This is an integration test for QGIS Desktop WFS-T provider and QGIS Server
WFS-T that check if QGIS offline editing works with a WFS-T endpoint.

The test uses testdata/wfs_transactional/wfs_transactional.qgs and three
initially empty shapefiles layers with points, lines and polygons.

The point layer is used in the test

From build dir, run: ctest -R PyQgsOfflineEditingWFS -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from builtins import str

__author__ = 'Alessandro Pasotti'
__date__ = '05/15/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'


import os
import sys
import re
import subprocess
from shutil import copytree, rmtree
import tempfile
from utilities import unitTestDataPath, waitServer
from qgis.core import (
    QgsVectorLayer,
    QgsAuthManager,
    QgsApplication
)

from qgis.testing import (
    start_app,
    unittest,
)

from offlineditingtestbase import OfflineTestBase


try:
    QGIS_SERVER_OFFLINE_PORT = os.environ['QGIS_SERVER_OFFLINE_PORT']
except:
    QGIS_SERVER_OFFLINE_PORT = '0'  # Auto

qgis_app = start_app()


class TestWFST(unittest.TestCase, OfflineTestBase):

    # To fake the WFS cache!
    counter = 0

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.port = QGIS_SERVER_OFFLINE_PORT
        # Create tmp folder
        cls.temp_path = tempfile.mkdtemp()
        cls.testdata_path = cls.temp_path + '/' + 'wfs_transactional' + '/'
        copytree(unitTestDataPath('wfs_transactional') + '/',
                 cls.temp_path + '/' + 'wfs_transactional')
        cls.project_path = cls.temp_path + '/' + 'wfs_transactional' + '/' + \
            'wfs_transactional.qgs'
        assert os.path.exists(cls.project_path), "Project not found: %s" % \
            cls.project_path
        # Clean env just to be sure
        env_vars = ['QUERY_STRING', 'QGIS_PROJECT_FILE']
        for ev in env_vars:
            try:
                del os.environ[ev]
            except KeyError:
                pass
        # Clear all test layers
        cls._clearLayer(cls._getLayer('test_point'))
        os.environ['QGIS_SERVER_PORT'] = str(cls.port)
        cls.server_path = os.path.dirname(os.path.realpath(__file__)) + \
            '/qgis_wrapped_server.py'

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        rmtree(cls.temp_path)

    def setUp(self):
        """Run before each test."""
        self.server = subprocess.Popen([sys.executable, self.server_path],
                                       env=os.environ, stdout=subprocess.PIPE)
        line = self.server.stdout.readline()
        self.port = int(re.findall(b':(\d+)', line)[0])
        assert self.port != 0
        # Wait for the server process to start
        assert waitServer('http://127.0.0.1:%s' % self.port), "Server is not responding!"
        self._setUp()

    def tearDown(self):
        """Run after each test."""
        # Clear test layer
        self._clearLayer(self._getOnlineLayer('test_point'))
        # Kill the server
        self.server.terminate()
        self.server.wait()
        del self.server
        # Delete the sqlite db
        os.unlink(os.path.join(self.temp_path, 'offlineDbFile.sqlite'))
        self._tearDown()

    def _getOnlineLayer(self, type_name, layer_name=None):
        """
        Return a new WFS layer, overriding the WFS cache
        """
        if layer_name is None:
            layer_name = 'wfs_' + type_name
        parms = {
            'srsname': 'EPSG:4326',
            'typename': type_name,
            'url': 'http://127.0.0.1:%s/%s/?map=%s' % (self.port,
                                                       self.counter,
                                                       self.project_path),
            'version': 'auto',
            'table': '',
            #'sql': '',
        }
        self.counter += 1
        uri = ' '.join([("%s='%s'" % (k, v)) for k, v in list(parms.items())])
        wfs_layer = QgsVectorLayer(uri, layer_name, 'WFS')
        wfs_layer.setParent(QgsApplication.authManager())
        assert wfs_layer.isValid()
        return wfs_layer

    @classmethod
    def _getLayer(cls, layer_name):
        """
        Layer factory (return the backend layer), provider specific
        """
        path = cls.testdata_path + layer_name + '.shp'
        layer = QgsVectorLayer(path, layer_name, "ogr")
        layer.setParent(QgsApplication.authManager())
        assert layer.isValid()
        return layer


if __name__ == '__main__':
    unittest.main()
