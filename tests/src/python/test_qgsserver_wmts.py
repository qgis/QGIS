# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer WMTS.

From build dir, run: ctest -R PyQgsServerWMTS -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'René-Luc Dhont'
__date__ = '19/09/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ['QT_HASH_SEED'] = '1'

import re
import urllib.request
import urllib.parse
import urllib.error

from qgis.server import QgsServerRequest

from qgis.testing import unittest
from qgis.PyQt.QtCore import QSize

import osgeo.gdal  # NOQA

from test_qgsserver import QgsServerTestBase

# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = b'MAP=[^"]+|Content-Length: \d+|timeStamp="[^"]+"'
RE_ATTRIBUTES = b'[^>\s]+=[^>\s]+'


class TestQgsServerWMTS(QgsServerTestBase):

    """QGIS Server WMTS Tests"""

    def wmts_request_compare(self, request, version='', extra_query_string='', reference_base_name=None):
        #project = self.testdata_path + "test_project_wfs.qgs"
        project = self.projectGroupsPath
        assert os.path.exists(project), "Project file not found: " + project

        query_string = '?MAP=%s&SERVICE=WMTS&REQUEST=%s' % (urllib.parse.quote(project), request)
        if version:
            query_string += '&VERSION=%s' % version

        if extra_query_string:
            query_string += '&%s' % extra_query_string

        header, body = self._execute_request(query_string)
        self.assert_headers(header, body)
        response = header + body

        if reference_base_name is not None:
            reference_name = reference_base_name
        else:
            reference_name = 'wmts_' + request.lower()

        reference_name += '.txt'

        reference_path = self.testdata_path + reference_name

        self.store_reference(reference_path, response)
        f = open(reference_path, 'rb')
        expected = f.read()
        f.close()
        response = re.sub(RE_STRIP_UNCHECKABLE, b'', response)
        expected = re.sub(RE_STRIP_UNCHECKABLE, b'', expected)

        self.assertXMLEqual(response, expected, msg="request %s failed.\n Query: %s" % (query_string, request))

    def test_project_wmts(self):
        """Test some WMTS request"""
        for request in ('GetCapabilities',):
            self.wmts_request_compare(request)
            #self.wmts_request_compare(request, '1.0.0')

    def test_wmts_gettile(self):
        # Testing project WMTS layer
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "QGIS Server Hello World",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:3857",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "0",
            "FORMAT": "image/png"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMTS_GetTile_Project_3857_0", 20000)

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "QGIS Server Hello World",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:4326",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "0",
            "FORMAT": "image/png"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMTS_GetTile_Project_4326_0", 20000)

        # Testing group WMTS layer
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "CountryGroup",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:3857",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "0",
            "FORMAT": "image/png"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMTS_GetTile_CountryGroup_3857_0", 20000)

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "CountryGroup",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:4326",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "0",
            "FORMAT": "image/png"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMTS_GetTile_CountryGroup_4326_0", 20000)

        # Testing QgsMapLayer WMTS layer
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "Hello",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:3857",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "0",
            "FORMAT": "image/png"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMTS_GetTile_Hello_3857_0", 20000)

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "Hello",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:4326",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "0",
            "FORMAT": "image/png"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMTS_GetTile_Hello_4326_0", 20000)

    def test_wmts_gettile_invalid_parameters(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "Hello",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:3857",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "FOO",
            "FORMAT": "image/png"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"TILECOL (\'FOO\') cannot be converted into int" in r
        self.assertTrue(err)

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "Hello",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:3857",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "1",
            "FORMAT": "image/png"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"TileCol is unknown" in r
        self.assertTrue(err)

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "Hello",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:3857",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "-1",
            "FORMAT": "image/png"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"TileCol is unknown" in r
        self.assertTrue(err)

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "dem",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:3857",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "0",
            "FORMAT": "image/png"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"Layer \'dem\' not found" in r
        self.assertTrue(err)

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMTS",
            "VERSION": "1.0.0",
            "REQUEST": "GetTile",
            "LAYER": "Hello",
            "STYLE": "",
            "TILEMATRIXSET": "EPSG:2154",
            "TILEMATRIX": "0",
            "TILEROW": "0",
            "TILECOL": "0",
            "FORMAT": "image/png"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"TileMatrixSet is unknown" in r
        self.assertTrue(err)


if __name__ == '__main__':
    unittest.main()
