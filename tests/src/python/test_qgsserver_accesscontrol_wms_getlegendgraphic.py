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

print('CTEST_FULL_OUTPUT')

import qgis  # NOQA

from qgis.testing import unittest
from qgis.PyQt.QtCore import QSize
import urllib.parse
from test_qgsserver_accesscontrol import TestQgsServerAccessControl


class TestQgsServerAccessControlWMSGetlegendgraphic(TestQgsServerAccessControl):

    # Set to True to re-generate reference files for this class
    #regenerate_reference = True

    def test_wms_getlegendgraphic_hello(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYERS": "Hello",
            "FORMAT": "image/png",
            "LAYERFONTBOLD": "TRUE",
            "LAYERFONTSIZE": "30",
            "ITEMFONTBOLD": "TRUE",
            "ITEMFONTSIZE": "20",
            "LAYERFONTFAMILY": self.fontFamily,
            "ITEMFONTFAMILY": self.fontFamily,
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetLegendGraphic_Hello", 250, QSize(10, 10))

        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "WMS_GetLegendGraphic_Hello", 250, QSize(10, 10))

    def test_wms_getlegendgraphic_country(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYERS": "Country",
            "FORMAT": "image/png",
            "LAYERFONTBOLD": "TRUE",
            "LAYERFONTSIZE": "30",
            "ITEMFONTBOLD": "TRUE",
            "ITEMFONTSIZE": "20",
            "LAYERFONTFAMILY": self.fontFamily,
            "ITEMFONTFAMILY": self.fontFamily,
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetLegendGraphic_Country", 250, QSize(10, 10))

        response, headers = self._get_restricted(query_string)
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find('<ServiceException code="Security">') != -1,
            "Not allowed GetLegendGraphic"
        )

    def test_wms_getlegendgraphic_country_grp(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYERS": "Country_grp",
            "FORMAT": "image/png",
            "LAYERFONTBOLD": "TRUE",
            "LAYERFONTSIZE": "30",
            "ITEMFONTBOLD": "TRUE",
            "ITEMFONTSIZE": "20",
            "LAYERFONTFAMILY": self.fontFamily,
            "ITEMFONTFAMILY": self.fontFamily,
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetLegendGraphic_Country", 250, QSize(10, 10))

        response, headers = self._get_restricted(query_string)
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find('<ServiceException code="Security">') != -1,
            "Not allowed GetLegendGraphic"
        )


if __name__ == "__main__":
    unittest.main()
