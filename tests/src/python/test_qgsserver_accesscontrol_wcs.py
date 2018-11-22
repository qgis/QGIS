# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Stephane Brunner'
__date__ = '28/08/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

print('CTEST_FULL_OUTPUT')

from qgis.testing import unittest
import urllib.request
import urllib.parse
import urllib.error
from test_qgsserver_accesscontrol import TestQgsServerAccessControl


class TestQgsServerAccessControlWCS(TestQgsServerAccessControl):

    def test_wcs_getcapabilities(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WCS",
            "VERSION": "1.0.0",
            "REQUEST": "GetCapabilities",
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<name>dem</name>") != -1,
            "No dem layer in WCS/GetCapabilities\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<name>dem</name>") != -1,
            "No dem layer in WCS/GetCapabilities\n%s" % response)

        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WCS",
            "VERSION": "1.0.0",
            "REQUEST": "GetCapabilities",
            "TEST": "dem",
        }.items())])

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<name>dem</name>") != -1,
            "Unexpected dem layer in WCS/GetCapabilities\n%s" % response)

    def test_wcs_describecoverage(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WCS",
            "VERSION": "1.0.0",
            "REQUEST": "DescribeCoverage",
            "COVERAGE": "dem",
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<name>dem</name>") != -1,
            "No dem layer in DescribeCoverage\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<name>dem</name>") != -1,
            "No dem layer in DescribeCoverage\n%s" % response)

        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WCS",
            "VERSION": "1.0.0",
            "REQUEST": "DescribeCoverage",
            "COVERAGE": "dem",
            "TEST": "dem",
        }.items())])

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<name>dem</name>") != -1,
            "Unexpected dem layer in DescribeCoverage\n%s" % response)

    def test_wcs_getcoverage(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WCS",
            "VERSION": "1.0.0",
            "REQUEST": "GetCoverage",
            "COVERAGE": "dem",
            "CRS": "EPSG:3857",
            "BBOX": "-1387454,4252256,431091,5458375",
            "HEIGHT": "100",
            "WIDTH": "100",
            "FORMAT": "GTiff",
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertEqual(
            headers.get("Content-Type"), "image/tiff",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            self._geo_img_diff(response, "WCS_GetCoverage.geotiff") == 0,
            "Image for GetCoverage is wrong")

        response, headers = self._get_restricted(query_string)
        self.assertEqual(
            headers.get("Content-Type"), "image/tiff",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            self._geo_img_diff(response, "WCS_GetCoverage.geotiff") == 0,
            "Image for GetCoverage is wrong")

        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WCS",
            "VERSION": "1.0.0",
            "REQUEST": "GetCoverage",
            "COVERAGE": "dem",
            "CRS": "EPSG:3857",
            "BBOX": "-1387454,4252256,431091,5458375",
            "HEIGHT": "100",
            "WIDTH": "100",
            "FORMAT": "GTiff",
            "TEST": "dem",
        }.items())])

        response, headers = self._get_restricted(query_string)
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find('<ServiceException code="RequestNotWellFormed">') != -1,
            "The layer for the COVERAGE 'dem' is not found")


if __name__ == "__main__":
    unittest.main()
