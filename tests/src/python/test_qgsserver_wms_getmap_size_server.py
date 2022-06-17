# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer MaxHeight and MaxWidth Override Options.

From build dir, run: ctest -R PyQgsServerWMSGetMapSizeServer -V

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

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all
# executions
os.environ['QT_HASH_SEED'] = '1'

from qgis.testing import unittest

from test_qgsserver import QgsServerTestBase
from test_qgsserver_wms_getmap_size_project import make_request


class TestQgsServerWMSGetMapSizeServer(QgsServerTestBase):
    """QGIS Server WMS Tests for GetFeatureInfo request"""

    # Set to True to re-generate reference files for this class
    regenerate_reference = False

    @classmethod
    def setUpClass(self):
        os.environ['QGIS_SERVER_WMS_MAX_WIDTH'] = '3000'
        os.environ['QGIS_SERVER_WMS_MAX_HEIGHT'] = '3000'
        super().setUpClass()

    def setUp(self):
        self.project = os.path.join(self.testdata_path, "test_project_with_size.qgs")
        self.expected_too_big = self.strip_version_xmlns(b'<ServiceExceptionReport version="1.3.0" xmlns="http://www.opengis.net/ogc">\n <ServiceException code="InvalidParameterValue">The requested map size is too large</ServiceException>\n</ServiceExceptionReport>\n')

    def test_wms_getmap_invalid_size_server(self):
        # test the 3000 limit from server is overriding the less conservative 5000 in the project
        r = make_request(self, 3001, 3000)
        self.assertEqual(self.strip_version_xmlns(r), self.expected_too_big)


if __name__ == '__main__':
    unittest.main()
