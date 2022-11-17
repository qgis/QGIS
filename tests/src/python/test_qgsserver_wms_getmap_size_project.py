# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer MaxHeight and MaxWidth Override Options.

From build dir, run: ctest -R PyQgsServerWMSGetMapSizeProject -V

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


import urllib.parse

from qgis.testing import unittest

from test_qgsserver import QgsServerTestBase


class TestQgsServerWMSGetMapSizeProject(QgsServerTestBase):
    """QGIS Server WMS Tests for GetFeatureInfo request"""

    # Set to True to re-generate reference files for this class
    regenerate_reference = False

    def setUp(self):
        os.environ['QGIS_SERVER_WMS_MAX_WIDTH'] = '6000'
        os.environ['QGIS_SERVER_WMS_MAX_HEIGHT'] = '6000'
        super(TestQgsServerWMSGetMapSizeProject, self).setUp()
        self.project = os.path.join(self.testdata_path, "test_project_with_size.qgs")
        self.expected_too_big = self.strip_version_xmlns(b'<?xml version="1.0" encoding="UTF-8"?>\n<ServiceExceptionReport version="1.3.0" xmlns="http://www.opengis.net/ogc">\n <ServiceException code="InvalidParameterValue">The requested map size is too large</ServiceException>\n</ServiceExceptionReport>\n')

    def test_wms_getmap_invalid_size_project(self):
        # test the 6000 limit from server is overridden by the more conservative 5000 in the project
        r = make_request(self, 5001, 5000)
        self.assertEqual(self.strip_version_xmlns(r), self.expected_too_big)


def make_request(instance, height, width):
    qs = "?" + "&".join(["%s=%s" % i for i in list({
        "MAP": urllib.parse.quote(instance.project),
        "SERVICE": "WMS",
        "VERSION": "1.3.0",
        "REQUEST": "GetMap",
        "LAYERS": "",
        "STYLES": "",
        "FORMAT": "image/png",
        "HEIGHT": str(height),
        "WIDTH": str(width)
    }.items())])
    r, h = instance._result(instance._execute_request(qs))
    return instance.strip_version_xmlns(r)


if __name__ == '__main__':
    unittest.main()
