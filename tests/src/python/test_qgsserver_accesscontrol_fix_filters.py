# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer.

From build dir, run: ctest -R PyQgsServerAccessControlWFS -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'David Marteau'
__date__ = '10/09/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

from qgis.testing import unittest
import urllib.request
import urllib.parse
import urllib.error
from test_qgsserver_accesscontrol import TestQgsServerAccessControl, XML_NS


class TestQgsServerAccessControlFixFilters(TestQgsServerAccessControl):

    def test_wfs_getfeature_fix_feature_filters(self):
        wfs_query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WFS",
            "VERSION": "1.0.0",
            "REQUEST": "GetFeature",
            "TYPENAME": "Hello_Filter",
            "EXP_FILTER": "pkuid = 1"
        }.items())])

        wms_query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "FORMAT": "image/png",
            "LAYERS": "Hello_Filter",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items())])

        # Execute an unrestricted wfs request
        response, headers = self._get_fullaccess(wfs_query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeature\n%s" % response)

        # Execute a restricted WMS request
        # That will store the filter expression in cache
        response, headers = self._get_restricted(wms_query_string)
        self.assertTrue(headers.get("Content-Type") == "image/png")

        # Execute an unrestricted wfs request again
        # We must have same result as the first time
        #
        # This test will fail if we do not clear the filter's cache
        # before each requests.
        response, headers = self._get_fullaccess(wfs_query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeature\n%s" % response)


if __name__ == "__main__":
    unittest.main()
