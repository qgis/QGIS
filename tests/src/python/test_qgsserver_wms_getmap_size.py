# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer MaxHeight and MaxWidth Override Options.

From build dir, run: ctest -R PyQgsServerGetMapSize -V

.. note:: This test needs env vars to be set before the server is
          configured for the first time, for this
          reason it cannot run as a test case of another server
          test.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Marco Bernasocchi'
__date__ = '01/04/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all
# executions
os.environ['QT_HASH_SEED'] = '1'


import urllib.request
import urllib.parse
import urllib.error

from utilities import (
    unitTestDataPath,
)
from qgis.testing import unittest

from test_qgsserver import QgsServerTestBase
from qgis.core import QgsProject, QgsFontUtils, QgsApplication
from qgis.server import QgsServer


class TestQgsServerWMSGetMapSize(QgsServerTestBase):
    """QGIS Server WMS Tests for GetFeatureInfo request"""

    # Set to True to re-generate reference files for this class
    regenerate_reference = False

    @classmethod
    def setUpClass(cls):
        # override the QgsServerTestBase method
        pass

    @classmethod
    def tearDownClass(cls):
        # override the QgsServerTestBase method
        pass

    def setUp(self):
        # override the QgsServerTestBase method
        pass

    def tearDown(self):
        self.app.exitQgis()
        del self.app

    def init_server(self):
        """Create our own server instance so it can be called after setting up different os.environ settings"""
        self.app = QgsApplication([], False)
        self.fontFamily = QgsFontUtils.standardTestFontFamily()
        QgsFontUtils.loadStandardTestFonts(['All'])

        self.testdata_path = unitTestDataPath('qgis_server') + '/'

        d = unitTestDataPath('qgis_server') + '/'
        self.projectPath = os.path.join(d, "project.qgs")

        # Clean env just to be sure
        env_vars = ['QUERY_STRING', 'QGIS_PROJECT_FILE']
        for ev in env_vars:
            try:
                del os.environ[ev]
            except KeyError:
                pass

        self.server = QgsServer()

        self.project = os.path.join(self.testdata_path, "test_project_with_size.qgs")
        self.expected_too_big = self.strip_version_xmlns(b'<ServiceExceptionReport version="1.3.0" xmlns="http://www.opengis.net/ogc">\n <ServiceException code="InvalidParameterValue">The requested map size is too large</ServiceException>\n</ServiceExceptionReport>\n')

    def test_wms_getmap_invalid_size_project(self):

        os.environ['QGIS_SERVER_WMS_MAX_WIDTH'] = '6000'
        os.environ['QGIS_SERVER_WMS_MAX_HEIGHT'] = '6000'
        self.init_server()

        # test the 6000 limit from server is overridden by the more conservative 5000 in the project
        r = self._make_request(5001, 5000)
        self.assertEqual(self.strip_version_xmlns(r), self.expected_too_big)

    def test_wms_getmap_invalid_size_server(self):

        # test the QGIS_SERVER_WMS_MAX_XXX env vars
        os.environ['QGIS_SERVER_WMS_MAX_WIDTH'] = '3000'
        os.environ['QGIS_SERVER_WMS_MAX_HEIGHT'] = '3000'
        self.init_server()

        # test the 3000 limit from server is overriding the less conservative 5000 in the project
        r = self._make_request(3001, 3000)
        self.assertEqual(self.strip_version_xmlns(r), self.expected_too_big)

    def _make_request(self, height, width):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.project),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "",
            "STYLES": "",
            "FORMAT": "image/png",
            "HEIGHT": str(height),
            "WIDTH": str(width)
        }.items())])
        r, h = self._result(self._execute_request(qs))
        return self.strip_version_xmlns(r)


if __name__ == '__main__':
    unittest.main()
