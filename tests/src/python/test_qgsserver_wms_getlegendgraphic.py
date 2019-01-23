# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer WMS GetLegendGraphic.

From build dir, run: ctest -R PyQgsServerWMSGetLegendGraphic -V


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

# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = b'MAP=[^"]+|Content-Length: \d+'
RE_ATTRIBUTES = b'[^>\s]+=[^>\s]+'


class TestQgsServerWMSGetLegendGraphic(QgsServerTestBase):
    """QGIS Server WMS Tests for GetLegendGraphic request"""

    # Set to True to re-generate reference files for this class
    #regenerate_reference = True

    def test_getLegendGraphics(self):
        """Test that does not return an exception but an image"""
        parms = {
            'MAP': self.testdata_path + "test_project.qgs",
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetLegendGraphic',
            'FORMAT': 'image/png',
            # 'WIDTH': '20', # optional
            # 'HEIGHT': '20', # optional
            'LAYER': 'testlayer%20èé',
        }
        qs = '?' + '&'.join(["%s=%s" % (k, v) for k, v in parms.items()])
        h, r = self._execute_request(qs)
        self.assertEqual(-1, h.find(b'Content-Type: text/xml; charset=utf-8'), "Header: %s\nResponse:\n%s" % (h, r))
        self.assertNotEqual(-1, h.find(b'Content-Type: image/png'), "Header: %s\nResponse:\n%s" % (h, r))

    def test_getLegendGraphics_invalid_parameters(self):
        """Test that does return an exception"""
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello,db_point",
            "LAYERTITLE": "FALSE",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "RULE": "1",
            "BBOX": "-151.7,-38.9,51.0,78.0",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"BBOX parameter cannot be combined with RULE" in r
        self.assertTrue(err)

    def test_wms_GetLegendGraphic_LayerSpace(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "FORMAT": "image/png",
            # "HEIGHT": "500",
            # "WIDTH": "500",
            "LAYERSPACE": "50.0",
            "LAYERFONTBOLD": "TRUE",
            "LAYERFONTSIZE": "30",
            "ITEMFONTBOLD": "TRUE",
            "ITEMFONTSIZE": "20",
            "LAYERFONTFAMILY": self.fontFamily,
            "ITEMFONTFAMILY": self.fontFamily,
            "LAYERTITLE": "TRUE",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_LayerSpace", max_size_diff=QSize(1, 1))

    def test_wms_GetLegendGraphic_LayerTitleSpace(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "FORMAT": "image/png",
            # "HEIGHT": "500",
            # "WIDTH": "500",
            "LAYERTITLESPACE": "20.0",
            "LAYERFONTBOLD": "TRUE",
            "LAYERFONTSIZE": "30",
            "ITEMFONTBOLD": "TRUE",
            "ITEMFONTSIZE": "20",
            "LAYERFONTFAMILY": self.fontFamily,
            "ITEMFONTFAMILY": self.fontFamily,
            "LAYERTITLE": "TRUE",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_LayerTitleSpace")

    def test_wms_GetLegendGraphic_ShowFeatureCount(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "FORMAT": "image/png",
            # "HEIGHT": "500",
            # "WIDTH": "500",
            "LAYERTITLE": "TRUE",
            "LAYERFONTBOLD": "TRUE",
            "LAYERFONTSIZE": "30",
            "LAYERFONTFAMILY": self.fontFamily,
            "ITEMFONTFAMILY": self.fontFamily,
            "ITEMFONTBOLD": "TRUE",
            "ITEMFONTSIZE": "20",
            "SHOWFEATURECOUNT": "TRUE",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_ShowFeatureCount", max_size_diff=QSize(1, 1))

    def test_getLegendGraphics_layertitle(self):
        """Test that does not return an exception but an image"""

        print("TEST FONT FAMILY: ", self.fontFamily)

        parms = {
            'MAP': self.testdata_path + "test_project.qgs",
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetLegendGraphic',
            'FORMAT': 'image/png',
            # 'WIDTH': '20', # optional
            # 'HEIGHT': '20', # optional
            'LAYER': u'testlayer%20èé',
            'LAYERFONTBOLD': 'TRUE',
            'LAYERFONTSIZE': '30',
            'ITEMFONTBOLD': 'TRUE',
            'LAYERFONTFAMILY': self.fontFamily,
            'ITEMFONTFAMILY': self.fontFamily,
            'ITEMFONTSIZE': '20',
            'LAYERTITLE': 'TRUE',
        }
        qs = '?' + '&'.join([u"%s=%s" % (k, v) for k, v in parms.items()])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_test", 250, QSize(15, 15))

        parms = {
            'MAP': self.testdata_path + "test_project.qgs",
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetLegendGraphic',
            'FORMAT': 'image/png',
            # 'WIDTH': '20', # optional
            # 'HEIGHT': '20', # optional
            'LAYER': u'testlayer%20èé',
            'LAYERTITLE': 'FALSE',
        }
        qs = '?' + '&'.join([u"%s=%s" % (k, v) for k, v in parms.items()])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_test_layertitle_false", 250, QSize(15, 15))

    def test_getLegendGraphics_rulelabel(self):
        """Test that does not return an exception but an image"""
        parms = {
            'MAP': self.testdata_path + "test_project.qgs",
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetLegendGraphic',
            'FORMAT': 'image/png',
            'LAYER': u'testlayer%20èé',
            'LAYERFONTBOLD': 'TRUE',
            'LAYERFONTSIZE': '30',
            'LAYERFONTFAMILY': self.fontFamily,
            'ITEMFONTFAMILY': self.fontFamily,
            'ITEMFONTBOLD': 'TRUE',
            'ITEMFONTSIZE': '20',
            'RULELABEL': 'TRUE',
        }
        qs = '?' + '&'.join([u"%s=%s" % (k, v) for k, v in parms.items()])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_test", 250, QSize(15, 15))

        parms = {
            'MAP': self.testdata_path + "test_project.qgs",
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetLegendGraphic',
            'FORMAT': 'image/png',
            'LAYER': u'testlayer%20èé',
            'LAYERFONTBOLD': 'TRUE',
            'LAYERFONTSIZE': '30',
            'ITEMFONTBOLD': 'TRUE',
            'ITEMFONTSIZE': '20',
            'LAYERFONTFAMILY': self.fontFamily,
            'ITEMFONTFAMILY': self.fontFamily,
            'RULELABEL': 'FALSE',
        }
        qs = '?' + '&'.join([u"%s=%s" % (k, v) for k, v in parms.items()])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_rulelabel_false", 250, QSize(15, 15))

    def test_getLegendGraphics_rule(self):
        """Test that does not return an exception but an image"""
        parms = {
            'MAP': self.testdata_path + "test_project_legend_rule.qgs",
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetLegendGraphic',
            'FORMAT': 'image/png',
            'LAYER': u'testlayer%20èé',
            'WIDTH': '20',
            'HEIGHT': '20',
            'RULE': 'rule0',
        }
        qs = '?' + '&'.join([u"%s=%s" % (k, v) for k, v in parms.items()])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_rule0", 250, QSize(15, 15))

        parms = {
            'MAP': self.testdata_path + "test_project_legend_rule.qgs",
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetLegendGraphic',
            'FORMAT': 'image/png',
            'LAYER': u'testlayer%20èé',
            'WIDTH': '20',
            'HEIGHT': '20',
            'RULE': 'rule1',
        }
        qs = '?' + '&'.join([u"%s=%s" % (k, v) for k, v in parms.items()])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_rule1", 250, QSize(15, 15))

    def test_wms_GetLegendGraphic_Basic(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "LAYERTITLE": "FALSE",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_Basic")

    def test_wms_GetLegendGraphic_Transparent(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "LAYERTITLE": "FALSE",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "TRANSPARENT": "TRUE"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_Transparent")

    def test_wms_GetLegendGraphic_Background(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "LAYERTITLE": "FALSE",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "BGCOLOR": "green"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_Background")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "LAYERTITLE": "FALSE",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "BGCOLOR": "0x008000"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_Background_Hex")

    def test_wms_GetLegendGraphic_BoxSpace(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "LAYERTITLE": "FALSE",
            "BOXSPACE": "100",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_BoxSpace")

    def test_wms_GetLegendGraphic_SymbolSpace(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "LAYERTITLE": "FALSE",
            "SYMBOLSPACE": "100",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_SymbolSpace")

    def test_wms_GetLegendGraphic_IconLabelSpace(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "LAYERTITLE": "FALSE",
            "ICONLABELSPACE": "100",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_IconLabelSpace")

    def test_wms_GetLegendGraphic_SymbolSize(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "LAYERTITLE": "FALSE",
            "SYMBOLWIDTH": "50",
            "SYMBOLHEIGHT": "30",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_SymbolSize")

    def test_wms_GetLegendGraphic_LayerFont(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "LAYERTITLE": "TRUE",
            "LAYERFONTBOLD": "TRUE",
            "LAYERFONTITALIC": "TRUE",
            "LAYERFONTSIZE": "30",
            "ITEMFONTBOLD": "TRUE",
            "ITEMFONTSIZE": "20",
            "LAYERFONTFAMILY": self.fontFamily,
            "ITEMFONTFAMILY": self.fontFamily,
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_LayerFont", max_size_diff=QSize(1, 1))

    def test_wms_GetLegendGraphic_ItemFont(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "LAYERTITLE": "TRUE",
            "LAYERFONTBOLD": "TRUE",
            "LAYERFONTSIZE": "30",
            "ITEMFONTBOLD": "TRUE",
            "ITEMFONTITALIC": "TRUE",
            "ITEMFONTSIZE": "20",
            "LAYERFONTFAMILY": self.fontFamily,
            "ITEMFONTFAMILY": self.fontFamily,
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_ItemFont", max_size_diff=QSize(1, 1))

    def test_wms_GetLegendGraphic_BBox(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello,db_point",
            "LAYERTITLE": "FALSE",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "BBOX": "-151.7,-38.9,51.0,78.0",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_BBox")

    def test_wms_GetLegendGraphic_BBox2(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello,db_point",
            "LAYERTITLE": "FALSE",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "BBOX": "-76.08,-6.4,-19.38,38.04",
            "SRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_BBox2")

    def test_wms_GetLegendGraphic_EmptyLegend(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'test_project_contextual_legend.qgs',
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "QGIS%20Server%20Hello%20World",
            "FORMAT": "image/png",
            "HEIGHT": "840",
            "WIDTH": "1226",
            "BBOX": "10.38450,-49.6370,73.8183,42.9461",
            "SRS": "EPSG:4326",
            "SCALE": "15466642"
        }.items())])

        h, r = self._execute_request(qs)
        self.assertEqual(-1, h.find(b'Content-Type: text/xml; charset=utf-8'), "Header: %s\nResponse:\n%s" % (h, r))
        self.assertNotEqual(-1, h.find(b'Content-Type: image/png'), "Header: %s\nResponse:\n%s" % (h, r))

    def test_wms_GetLegendGraphic_wmsRootName(self):
        """Test an unreported issue when a wmsRootName short name is set in the service capabilities"""

        # First test with the project title itself:
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'test_project_wms_grouped_layers.qgs',
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "QGIS%20Server%20-%20Grouped%20Layer",
            "FORMAT": "image/png",
            "HEIGHT": "840",
            "WIDTH": "1226",
            "BBOX": "609152,5808188,625492,5814318",
            "SRS": "EPSG:25832",
            "SCALE": "38976"
        }.items())])

        h, r = self._execute_request(qs)
        self.assertEqual(-1, h.find(b'Content-Type: text/xml; charset=utf-8'), "Header: %s\nResponse:\n%s" % (h, r))
        self.assertNotEqual(-1, h.find(b'Content-Type: image/png'), "Header: %s\nResponse:\n%s" % (h, r))

        # Then test with the wmsRootName short name:
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'test_project_wms_grouped_layers_wmsroot.qgs',
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "All_grouped_layers",
            "FORMAT": "image/png",
            "HEIGHT": "840",
            "WIDTH": "1226",
            "BBOX": "609152,5808188,625492,5814318",
            "SRS": "EPSG:25832",
            "SCALE": "38976"
        }.items())])

        h, r = self._execute_request(qs)
        self.assertEqual(-1, h.find(b'Content-Type: text/xml; charset=utf-8'), "Header: %s\nResponse:\n%s" % (h, r))
        self.assertNotEqual(-1, h.find(b'Content-Type: image/png'), "Header: %s\nResponse:\n%s" % (h, r))


if __name__ == '__main__':
    unittest.main()
