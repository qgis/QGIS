# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer WMS GetPrint.

From build dir, run: ctest -R PyQgsServerWMSGetPrint -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '25/05/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ['QT_HASH_SEED'] = '1'

import re
import urllib.request
import urllib.parse
import urllib.error

from qgis.testing import unittest
from qgis.PyQt.QtCore import QSize

import osgeo.gdal  # NOQA

from test_qgsserver import QgsServerTestBase
from qgis.core import QgsProject
from qgis.server import QgsServerRequest

# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = b'MAP=[^"]+|Content-Length: \d+'
RE_ATTRIBUTES = b'[^>\s]+=[^>\s]+'


class TestQgsServerWMSGetPrint(QgsServerTestBase):

    """QGIS Server WMS Tests for GetPrint request"""

    def test_wms_getprint_basic(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country,Hello",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Basic")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "LAYERS": "Country,Hello",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Basic")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country,Hello",
            "LAYERS": "Country,Hello",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Basic")

    def test_wms_getprint_style(self):
        # default style
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country_Labels",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        assert h.get("Content-Type").startswith('image'), r
        self._img_diff_error(r, h, "WMS_GetPrint_StyleDefault")

        # custom style
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country_Labels",
            "map0:STYLES": "custom",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_StyleCustom")

        # default style
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "LAYERS": "Country_Labels",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_StyleDefault")

        # custom style
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "LAYERS": "Country_Labels",
            "STYLES": "custom",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_StyleCustom")

        # default style
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country_Labels",
            "LAYERS": "Country_Labels",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_StyleDefault")

        # custom style
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country_Labels",
            "map0:STYLES": "custom",
            "LAYERS": "Country_Labels",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_StyleCustom")

    def test_wms_getprint_legend(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4copy",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country,Hello",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Legend")

    def test_wms_getprint_srs(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-309.015,-133.011,312.179,133.949",
            "map0:LAYERS": "Country,Hello",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_SRS")

    def test_wms_getprint_scale(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country,Hello",
            "map0:SCALE": "36293562",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Scale")

    def test_wms_getprint_grid(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country,Hello",
            "map0:GRID_INTERVAL_X": "1000000",
            "map0:GRID_INTERVAL_Y": "2000000",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Grid")

    def test_wms_getprint_rotation(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country,Hello",
            "map0:ROTATION": "45",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Rotation")

    def test_wms_getprint_selection(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "LAYERS": "Country,Hello",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country,Hello",
            "CRS": "EPSG:3857",
            "SELECTION": "Country: 4"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Selection")

    def test_wms_getprint_opacity(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0%3AEXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country,Hello",
            "CRS": "EPSG:3857",
            "SELECTION": "Country: 4",
            "LAYERS": "Country,Hello",
            "OPACITIES": "125,125"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Opacity")

    def test_wms_getprint_opacity_post(self):
        qs = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0%3AEXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country,Hello",
            "CRS": "EPSG:3857",
            "SELECTION": "Country: 4",
            "LAYERS": "Country,Hello",
            "OPACITIES": "125%2C125"
        }.items())])

        r, h = self._result(self._execute_request('', QgsServerRequest.PostMethod, data=qs.encode('utf-8')))
        self._img_diff_error(r, h, "WMS_GetPrint_Opacity")

    def test_wms_getprint_highlight(self):
        # default style
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country_Labels",
            "map0:HIGHLIGHT_GEOM": "POLYGON((-15000000 10000000, -15000000 6110620, 2500000 6110620, 2500000 10000000, -15000000 10000000))",
            "map0:HIGHLIGHT_SYMBOL": "<StyledLayerDescriptor><UserStyle><Name>Highlight</Name><FeatureTypeStyle><Rule><Name>Symbol</Name><LineSymbolizer><Stroke><SvgParameter name=\"stroke\">%23ea1173</SvgParameter><SvgParameter name=\"stroke-opacity\">1</SvgParameter><SvgParameter name=\"stroke-width\">1.6</SvgParameter></Stroke></LineSymbolizer></Rule></FeatureTypeStyle></UserStyle></StyledLayerDescriptor>",
            "map0:HIGHLIGHT_LABELSTRING": "Highlight Layer!",
            "map0:HIGHLIGHT_LABELSIZE": "16",
            "map0:HIGHLIGHT_LABELCOLOR": "%2300FF0000",
            "map0:HIGHLIGHT_LABELBUFFERCOLOR": "%232300FF00",
            "map0:HIGHLIGHT_LABELBUFFERSIZE": "1.5",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        assert h.get("Content-Type").startswith('image'), r
        self._img_diff_error(r, h, "WMS_GetPrint_Highlight")

    def test_wms_getprint_label(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country,Hello",
            "CRS": "EPSG:3857",
            "IDTEXTBOX": "Updated QGIS composer label"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_LabelUpdated")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country,Hello",
            "CRS": "EPSG:3857",
            "IDTEXTBOX": ""
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_LabelRemoved")

    def test_wms_getprint_two_maps(self):
        """Test map0 and map1 apply to the correct maps"""
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4twoMaps",
            "FORMAT": "png",
            "map0:EXTENT": "11863620.20301065221428871,-5848927.97872077487409115,19375243.89574331790208817,138857.97204941",
            "map0:LAYERS": "Country",
            "map1:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map1:LAYERS": "Country,Hello",
            "CRS": "EPSG:3857",
            "IDTEXTBOX": "",
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_TwoMaps")


if __name__ == '__main__':
    unittest.main()
