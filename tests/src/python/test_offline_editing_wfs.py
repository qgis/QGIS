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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


import os
import sys
import subprocess
from shutil import copytree, rmtree
import tempfile
from time import sleep
from utilities import unitTestDataPath
from qgis.core import (
    QgsVectorLayer,
    QgsProject,
)

from qgis.testing import (
    start_app,
    unittest,
)

from offlineditingtestbase import OfflineTestBase

from qgis.PyQt.QtCore import QFileInfo

try:
    QGIS_SERVER_WFST_DEFAULT_PORT = os.environ['QGIS_SERVER_WFST_DEFAULT_PORT']
except:
    QGIS_SERVER_WFST_DEFAULT_PORT = 8081


qgis_app = start_app()


class TestWFST(unittest.TestCase, OfflineTestBase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.port = QGIS_SERVER_WFST_DEFAULT_PORT
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
        cls._clearLayer('test_point')
        os.environ['QGIS_SERVER_DEFAULT_PORT'] = str(cls.port)
        server_path = os.path.dirname(os.path.realpath(__file__)) + \
            '/qgis_wrapped_server.py'
        cls.server = subprocess.Popen([sys.executable, server_path],
                                      env=os.environ)
        sleep(2)

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        cls.server.terminate()
        del cls.server
        # Clear test layer
        cls._clearLayer('test_point')
        rmtree(cls.temp_path)

    def setUp(self):
        """Run before each test."""
        self._setUp()

    def tearDown(self):
        """Run after each test."""
        self._tearDown()

    @classmethod
    def _getOnlineLayer(cls, type_name, layer_name=None):
        """
        Layer factory (return the online layer), provider specific
        """
        if layer_name is None:
            layer_name = 'wfs_' + type_name
        parms = {
            'srsname': 'EPSG:4326',
            'typename': type_name,
            'url': 'http://127.0.0.1:%s/?map=%s' % (cls.port,
                                                    cls.project_path),
            'version': 'auto',
            'table': '',
            #'sql': '',
        }
        uri = ' '.join([("%s='%s'" % (k, v)) for k, v in parms.items()])
        wfs_layer = QgsVectorLayer(uri, layer_name, 'WFS')
        assert wfs_layer.isValid()
        return wfs_layer

    @classmethod
    def _getLayer(cls, layer_name):
        """
        Layer factory (return the backend layer), provider specific
        """
        path = cls.testdata_path + layer_name + '.shp'
        layer = QgsVectorLayer(path, layer_name, "ogr")
        assert layer.isValid()
        return layer


if __name__ == '__main__':
    unittest.main()
