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

from test_qgsserver import QgsServerTestBase


class TestQgsServerWMSGetPrintAtlas(QgsServerTestBase):
    """QGIS Server WMS Tests for GetPrint atlas request"""

    def test_wms_getprint_atlas(self):
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

    def test_wms_getprint_atlas_getProjectSettings(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetProjectSettings",
        }.items())])
        r, h = self._result(self._execute_request(qs))
        self.assertTrue('atlasEnabled="1"' in str(r))
        self.assertTrue('<PrimaryKeyAttribute>' in str(r))


if __name__ == '__main__':
    unittest.main()
