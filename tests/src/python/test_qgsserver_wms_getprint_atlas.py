# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer WMS GetPrint.

From build dir, run: ctest -R PyQgsServerWMSGetPrintAtlas -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'René-Luc DHONT'
__date__ = '24/06/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ['QT_HASH_SEED'] = '1'

import urllib.parse

from qgis.testing import unittest
from utilities import unitTestDataPath
from test_qgsserver import QgsServerTestBase

from qgis.core import QgsProject


class TestQgsServerWMSGetPrintAtlas(QgsServerTestBase):
    """QGIS Server WMS Tests for GetPrint atlas request"""

    def __test_wms_getprint_atlas(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "CRS": "EPSG:3857",
            "ATLAS_PK": "3",
            "map0:LAYERS": "Country,Hello",
        }.items())])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Atlas")

    def __test_wms_getprint_atlas_getProjectSettings(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetProjectSettings",
        }.items())])
        r, h = self._result(self._execute_request(qs))
        self.assertTrue('atlasEnabled="1"' in str(r))
        self.assertTrue('<PrimaryKeyAttribute>' in str(r))

    def test_wms_getprint_atlas_no_pk(self):
        """Test issue GH #30817"""

        project = QgsProject()
        self.assertTrue(project.read(os.path.join(unitTestDataPath(), 'qgis_server', 'bug_gh30817_atlas_pk.qgs')))
        params = {
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layout_csv",
            "TRANSPARENT": "true",
            "FORMAT": "png",
            "DPI": "50",
            "CRS": "EPSG:2056",
            "ATLAS_PK": "2",
        }
        qs = "?" + "&".join(["%s=%s" % i for i in list(params.items())])
        r, h = self._result(self._execute_request_project(qs, project))
        self._img_diff_error(r, h, "WMS_GetPrint_Atlas_No_Pk")

        # Test issue GH #49900: when using map scales scale
        params['TEMPLATE'] = 'layout_fixed_scale'
        params['ATLAS_PK'] = '4'
        qs = "?" + "&".join(["%s=%s" % i for i in list(params.items())])
        r, h = self._result(self._execute_request_project(qs, project))
        self._img_diff_error(r, h, "WMS_GetPrint_Atlas_Fixed_Scale")


if __name__ == '__main__':
    unittest.main()
