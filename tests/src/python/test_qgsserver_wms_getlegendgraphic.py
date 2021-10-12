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

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ['QT_HASH_SEED'] = '1'

import re
import json
import urllib.request
import urllib.parse
import urllib.error

from qgis.testing import unittest
from qgis.PyQt.QtCore import QSize

import osgeo.gdal  # NOQA

from test_qgsserver_wms import TestQgsServerWMSTestBase
from qgis.core import (
    QgsProject,
    QgsMarkerSymbol,
    QgsRuleBasedRenderer,
    QgsVectorLayer,
)

from qgis.server import (
    QgsBufferServerRequest,
    QgsBufferServerResponse,
    QgsServer,
    QgsServerRequest,
)

# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = br'MAP=[^"]+|Content-Length: \d+'
RE_ATTRIBUTES = br'[^>\s]+=[^>\s]+'


class TestQgsServerWMSGetLegendGraphic(TestQgsServerWMSTestBase):
    """QGIS Server WMS Tests for GetLegendGraphic request"""

    # Set to True to re-generate reference files for this class
    # regenerate_reference = True

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

    def test_wms_getLegendGraphics_invalid_parameters(self):
        """Test that does return an exception"""
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello,db_point",
            "LAYERTITLE": "FALSE",
            "RULELABEL": "FALSE",
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

    def test_wms_getLegendGraphics_layertitle(self):
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
            'RULELABEL': 'TRUE'
        }
        qs = '?' + '&'.join([u"%s=%s" % (k, v) for k, v in parms.items()])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_test", 250, QSize(15, 15))

        # no set of LAYERTITLE and RULELABEL means they are true
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
            'ITEMFONTSIZE': '20'
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
            'RULELABEL': 'FALSE'
        }
        qs = '?' + '&'.join([u"%s=%s" % (k, v) for k, v in parms.items()])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_test_layertitle_false", 250, QSize(15, 15))

    def test_wms_getLegendGraphics_rulelabel(self):
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
            'ITEMFONTBOLD': 'TRUE',
            'ITEMFONTSIZE': '20',
            'LAYERFONTFAMILY': self.fontFamily,
            'ITEMFONTFAMILY': self.fontFamily,
            'RULELABEL': 'FALSE'
        }
        qs = '?' + '&'.join([u"%s=%s" % (k, v) for k, v in parms.items()])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_rulelabel_false", 250, QSize(15, 15))

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
            'LAYERTITLE': 'FALSE',
            'RULELABEL': 'TRUE'
        }
        qs = '?' + '&'.join([u"%s=%s" % (k, v) for k, v in parms.items()])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_rulelabel_true", 250, QSize(15, 15))

        # no set of RULELABEL means it is true
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
            'LAYERTITLE': 'FALSE'
        }
        qs = '?' + '&'.join([u"%s=%s" % (k, v) for k, v in parms.items()])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_rulelabel_notset", 250, QSize(15, 15))

        # RULELABEL AUTO for single symbol means it is removed
        parms = {
            'MAP': self.testdata_path + "test_project.qgs",
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetLegendGraphic',
            'FORMAT': 'image/png',
            'LAYER': u'testlayer%20èé',
            'LAYERTITLE': 'FALSE',
            'RULELABEL': 'AUTO'
        }
        qs = '?' + '&'.join([u"%s=%s" % (k, v) for k, v in parms.items()])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_rulelabel_auto", 250, QSize(15, 15))

    def test_wms_getLegendGraphics_rule(self):
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
            "RULELABEL": "FALSE",
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
            "RULELABEL": "FALSE",
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
            "RULELABEL": "FALSE",
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
            "RULELABEL": "FALSE",
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
            "RULELABEL": "FALSE",
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
            "RULELABEL": "FALSE",
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
            "RULELABEL": "FALSE",
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
            "RULELABEL": "FALSE",
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
            "RULELABEL": "FALSE",
            "FORMAT": "image/png",
            "SRCHEIGHT": "500",
            "SRCWIDTH": "500",
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
            "RULELABEL": "FALSE",
            "FORMAT": "image/png",
            "SRCHEIGHT": "500",
            "SRCWIDTH": "500",
            "BBOX": "-76.08,-6.4,-19.38,38.04",
            "SRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_BBox2")

    def test_wms_GetLegendGraphic_BBox_Fallback(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello,db_point",
            "LAYERTITLE": "FALSE",
            "RULELABEL": "FALSE",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "BBOX": "-151.7,-38.9,51.0,78.0",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_BBox")

    def test_wms_GetLegendGraphic_BBox2_Fallback(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello,db_point",
            "LAYERTITLE": "FALSE",
            "RULELABEL": "FALSE",
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
            "SRCHEIGHT": "840",
            "SRCWIDTH": "1226",
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
            "SRCHEIGHT": "840",
            "SRCWIDTH": "1226",
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
            "SRCHEIGHT": "840",
            "SRCWIDTH": "1226",
            "BBOX": "609152,5808188,625492,5814318",
            "SRS": "EPSG:25832",
            "SCALE": "38976"
        }.items())])

        h, r = self._execute_request(qs)
        self.assertEqual(-1, h.find(b'Content-Type: text/xml; charset=utf-8'), "Header: %s\nResponse:\n%s" % (h, r))
        self.assertNotEqual(-1, h.find(b'Content-Type: image/png'), "Header: %s\nResponse:\n%s" % (h, r))

    def test_wms_GetLegendGraphic_ScaleSymbol_Min(self):
        # 1:500000000 min
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'test_project_scaledsymbols.qgs',
            "SERVICE": "WMS",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "testlayer",
            "FORMAT": "image/png",
            "SRCHEIGHT": "550",
            "SRCWIDTH": "850",
            "BBOX": "-608.4,-1002.6,698.2,1019.0",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_ScaleSymbol_Min", max_size_diff=QSize(1, 1))

        # 1:1000000000 min
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'test_project_scaledsymbols.qgs',
            "SERVICE": "WMS",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "testlayer",
            "FORMAT": "image/png",
            "SRCHEIGHT": "550",
            "SRCWIDTH": "850",
            "BBOX": "-1261.7,-2013.5,1351.5,2029.9",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_ScaleSymbol_Min", max_size_diff=QSize(15, 15))

    def test_wms_GetLegendGraphic_ScaleSymbol_Scaled_01(self):
        # 1:10000000 scaled
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'test_project_scaledsymbols.qgs',
            "SERVICE": "WMS",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "testlayer",
            "FORMAT": "image/png",
            "SRCHEIGHT": "550",
            "SRCWIDTH": "850",
            "BBOX": "31.8,-12.0,58.0,28.4",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_ScaleSymbol_Scaled_01", max_size_diff=QSize(15, 15))

    def test_wms_GetLegendGraphic_ScaleSymbol_Scaled_02(self):
        # 1:15000000 scaled
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'test_project_scaledsymbols.qgs',
            "SERVICE": "WMS",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "testlayer",
            "FORMAT": "image/png",
            "SRCHEIGHT": "550",
            "SRCWIDTH": "850",
            "BBOX": "25.3,-22.1,64.5,38.5",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_ScaleSymbol_Scaled_02", max_size_diff=QSize(15, 15))

    def test_wms_GetLegendGraphic_ScaleSymbol_Max(self):
        # 1:100000 max
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'test_project_scaledsymbols.qgs',
            "SERVICE": "WMS",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "testlayer",
            "FORMAT": "image/png",
            "SRCHEIGHT": "550",
            "SRCWIDTH": "850",
            "BBOX": "44.8,8.0,45.0,8.4",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_ScaleSymbol_Max", max_size_diff=QSize(15, 15))

        # 1:1000000 max
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'test_project_scaledsymbols.qgs',
            "SERVICE": "WMS",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "testlayer",
            "FORMAT": "image/png",
            "SRCHEIGHT": "550",
            "SRCWIDTH": "850",
            "BBOX": "43.6,6.2,46.2,10.2",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_ScaleSymbol_Max", max_size_diff=QSize(15, 15))

    def test_wms_GetLegendGraphic_ScaleSymbol_DefaultMapUnitsPerMillimeter(self):
        # map units per mm on 1:20000000 with SRCHEIGHT=598&SRCWIDTH=1640&BBOX=16.5,-69.7,73.3,86.1 would be around what is set as default: 0.359 map units per mm
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'test_project_scaledsymbols.qgs',
            "SERVICE": "WMS",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "testlayer",
            "FORMAT": "image/png",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_ScaleSymbol_DefaultMapUnitsPerMillimeter",
                             max_size_diff=QSize(15, 15))

    def test_wms_GetLegendGraphic_ScaleSymbol_Scaled_2056(self):
        # 1:1000 scale on an EPSG:2056 calculating DPI that is around 96
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'test_project_scaledsymbols_2056.qgs',
            "SERVICE": "WMS",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "testlayer_2056",
            "FORMAT": "image/png",
            "SRCHEIGHT": "600",
            "SRCWIDTH": "1500",
            "BBOX": "2662610.7,1268841.8,2663010.5,1269000.05",
            "CRS": "EPSG:2056"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_ScaleSymbol_Scaled_2056", max_size_diff=QSize(15, 15))

    def test_wms_GetLegendGraphic_ScaleSymbol_DefaultScale_2056(self):
        # 1:1000 as default value - it's not exactly the same result than passing the bbox and size because of exact DPI 96 (default)
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'test_project_scaledsymbols_2056.qgs',
            "SERVICE": "WMS",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "testlayer_2056",
            "FORMAT": "image/png",
            "CRS": "EPSG:2056"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_ScaleSymbol_DefaultScale_2056", max_size_diff=QSize(15, 15))

    def test_wms_GetLegendGraphic_LAYERFONTCOLOR(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "LAYERFONTCOLOR": "red"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_LAYERFONTCOLOR", max_size_diff=QSize(10, 2))

    def test_wms_GetLegendGraphic_ITEMFONTCOLOR(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "ITEMFONTCOLOR": "red",
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_ITEMFONTCOLOR", max_size_diff=QSize(10, 2))

    def test_wms_GetLegendGraphic_ITEMFONTCOLOR_and_LAYERFONTCOLOR(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "ITEMFONTCOLOR": "red",
            "LAYERFONTCOLOR": "blue"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_ITEMFONTCOLOR_and_LAYERFONTCOLOR", max_size_diff=QSize(10, 2))

    def test_wms_GetLegendGraphic_ITEMFONTCOLOR_and_LAYERFONTCOLOR_hex(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "Country,Hello",
            "FORMAT": "image/png",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "ITEMFONTCOLOR": r"%23FF0000",
            "LAYERFONTCOLOR": r"%230000FF"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_ITEMFONTCOLOR_and_LAYERFONTCOLOR", max_size_diff=QSize(10, 2))

    def test_BBoxNoWidthNoHeight(self):
        """Test with BBOX and no width/height (like QGIS client does)"""

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'test_project_wms_grouped_nested_layers.qgs',
            "SERVICE": "WMS",
            "VERSION": "1.3",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "areas%20and%20symbols",
            "FORMAT": "image/png",
            "CRS": "EPSG:4326",
            "BBOX": "52.44462990911360123,10.6723591605239374,52.44631832182876963,10.6795952150175264",
            "SLD_VERSION": "1.1",
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self.assertFalse(b'Exception' in r)
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_NoWidthNoHeight", max_size_diff=QSize(10, 2))

    def testGetLegendGraphicRegression32020(self):
        """When two classes have the same symbol they both are shown in the contextual
        legend even if just one is actually visible in the map extent

        This test also checks for corner cases (literally) and reprojection.
        """

        # Visible is "Type 1"
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'bug_gh32020.qgs',
            "SERVICE": "WMS",
            "VERSION": "1.3",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "test_layer",
            "FORMAT": "image/png",
            "CRS": "EPSG:4326",
            "BBOX": "0.05148830809982496426,-2.237691019614711507,0.8090701330998248952,-0.2050896957968479928",
            "SLD_VERSION": "1.1",
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self.assertFalse(b'Exception' in r)
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_Regression32020_type1", max_size_diff=QSize(10, 2))

        # Visible is "Type 2"
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'bug_gh32020.qgs',
            "SERVICE": "WMS",
            "VERSION": "1.3",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "test_layer",
            "FORMAT": "image/png",
            "CRS": "EPSG:4326",
            "BBOX": "0.02893333257443075901,-0.2568334631786342026,1.544096982574430621,3.808369184457092604",
            "SLD_VERSION": "1.1",
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self.assertFalse(b'Exception' in r)
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_Regression32020_type2", max_size_diff=QSize(10, 2))

        # Visible is "Type 2" and 3
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'bug_gh32020.qgs',
            "SERVICE": "WMS",
            "VERSION": "1.3",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "test_layer",
            "FORMAT": "image/png",
            "CRS": "EPSG:4326",
            "BBOX": "-0.6636370923817864753,-0.2886757815674259042,0.8515265576182133866,3.776526866068300681",
            "SLD_VERSION": "1.1",
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self.assertFalse(b'Exception' in r)
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_Regression32020_type2_and_3", max_size_diff=QSize(10, 2))

        # Visible is "Type 1" and 3
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'bug_gh32020.qgs',
            "SERVICE": "WMS",
            "VERSION": "1.3",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "test_layer",
            "FORMAT": "image/png",
            "CRS": "EPSG:4326",
            "BBOX": "-0.5787242433450088264,-4.316729057749563836,0.9364394066549910356,-0.2515264101138368069",
            "SLD_VERSION": "1.1",
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self.assertFalse(b'Exception' in r)
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_Regression32020_type1_and_3", max_size_diff=QSize(10, 2))

        # Change CRS: 3857
        # Visible is "Type 2"
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'bug_gh32020.qgs',
            "SERVICE": "WMS",
            "VERSION": "1.3",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "test_layer",
            "FORMAT": "image/png",
            "CRS": "EPSG:3857",
            "BBOX": "-28147.15420315234223,3960.286488616475253,424402.4530122592696,172632.4964886165108",
            "SLD_VERSION": "1.1",
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self.assertFalse(b'Exception' in r)
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_Regression32020_type2_3857", max_size_diff=QSize(10, 2))

    def test_wms_GetLegendGraphic_JSON(self):
        self.wms_request_compare("GetLegendGraphic",
                                 "&LAYERS=testlayer%20%C3%A8%C3%A9"
                                 "&FORMAT=application/json",
                                 "wms_getlegendgraphic_json")

    def test_wms_GetLegendGraphic_JSON_multiple_layers(self):
        self.wms_request_compare("GetLegendGraphic",
                                 "&LAYERS=testlayer%20%C3%A8%C3%A9,testlayer3"
                                 "&FORMAT=application/json",
                                 "wms_getlegendgraphic_json_multiple_layers")

    def test_wms_GetLegendGraphic_JSON_multiple_symbol(self):
        self.wms_request_compare("GetLegendGraphic",
                                 "&LAYERS=cdb_lines"
                                 "&FORMAT=application/json",
                                 "wms_getlegendgraphic_json_multiple_symbol",
                                 'test_project_wms_grouped_layers.qgs')

    def testJsonSymbolMaxMinScale(self):
        """Test min/max scale in symbol json export"""

        project = QgsProject()
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")

        symbol = QgsMarkerSymbol.createSimple(
            {'name': 'square', 'color': 'red'})

        scale_min = 10000
        scale_max = 1000
        rule = QgsRuleBasedRenderer.Rule(symbol, scale_min, scale_max, '')
        rootrule = QgsRuleBasedRenderer.Rule(None)
        rootrule.appendChild(rule)
        layer.setRenderer(QgsRuleBasedRenderer(rootrule))

        project.addMapLayers([layer])

        server = QgsServer()
        request = QgsBufferServerRequest("/?SERVICE=WMS&VERSION=1.3.0&REQUEST=GetLegendGraphic" +
                                         "&LAYERS=layer1" +
                                         "&FORMAT=application/json")
        response = QgsBufferServerResponse()
        server.handleRequest(request, response, project)
        j = json.loads(bytes(response.body()))
        node = j['nodes'][0]
        self.assertEqual(node['scaleMaxDenom'], 1000)
        self.assertEqual(node['scaleMinDenom'], 10000)

    def testLegendPlaceholderIcon(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": self.testdata_path + 'test_project_legend_placeholder_image.qgs',
            "SERVICE": "WMS",
            "VERSION": "1.3",
            "REQUEST": "GetLegendGraphic",
            "LAYER": "landsat",
            "FORMAT": "image/png",
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_Legend_Placeholder_Icon")


if __name__ == '__main__':
    unittest.main()
