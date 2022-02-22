# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer WMS GetMap.

From build dir, run: ctest -R PyQgsServerWMSGetMap -V


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
import urllib.request
import urllib.parse
import urllib.error

from qgis.testing import unittest
from qgis.PyQt.QtCore import QSize
from qgis.PyQt.QtGui import QImage

import osgeo.gdal  # NOQA

from test_qgsserver import QgsServerTestBase
from utilities import unitTestDataPath
from qgis.core import QgsProject, QgsVectorLayer

# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = br'MAP=[^"]+|Content-Length: \d+'
RE_ATTRIBUTES = br'[^>\s]+=[^>\s]+'


class TestQgsServerWMSGetMap(QgsServerTestBase):
    """QGIS Server WMS Tests for GetMap request"""

    # regenerate_reference = True

    def test_wms_getmap_basic_mode(self):
        # 1 bits
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png; mode=1bit",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Mode_1bit", 20000)

        # 8 bits
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png; mode=8bit",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Mode_8bit", 20000)

        # 16 bits
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png; mode=16bit",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Mode_16bit", 20000)

        # webp
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/webp",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Mode_16bit", 20000, outputFormat='WEBP')

    def test_wms_getmap_basic(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Basic")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,dem",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Basic2")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectUseLayerIdsPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "country20131022151106556",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Basic3")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,db_point",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Basic4")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "sERVICE": "WMS",
            "VeRSION": "1.1.1",
            "REqUEST": "GetMap",
            "LAYeRS": "Country,db_point",
            "STYLeS": "",
            "FORMAt": "image/png",
            "bBOX": "-16817707,-4710778,5696513,14587125",
            "HeIGHT": "500",
            "WIDTH": "500",
            "CRs": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Basic4")

    def test_wms_getmap_complex_labeling(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "pointlabel",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Labeling_Complex")

    def test_wms_getmap_context_rendering(self):
        project = os.path.join(self.testdata_path, "test_project_render_context.qgs")
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "points",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-119.8,20.4,-82.4,49.2",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_ContextRendering")

    def test_wms_getmap_dpi(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "DPI": "112.5"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Basic5")
        img = QImage.fromData(r, "PNG")
        self.assertEqual(round(img.dotsPerMeterX() / 39.37, 1), 112.5)
        self.assertEqual(round(img.dotsPerMeterY() / 39.37, 1), 112.5)

    def test_wms_getmap_dpi_png_8bit(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png; mode=8bit",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "DPI": "112.5"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        img = QImage.fromData(r, "PNG")
        self.assertEqual(round(img.dotsPerMeterX() / 39.37, 1), 112.5)
        self.assertEqual(round(img.dotsPerMeterY() / 39.37, 1), 112.5)

    def test_wms_getmap_invalid_parameters(self):
        # invalid format
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "pdf",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"The format \'pdf\' from FORMAT is not supported." in r
        self.assertTrue(err)

        # height should be an int
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "FOO",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"HEIGHT (\'FOO\') cannot be converted into int" in r
        self.assertTrue(err)

        # width should be an int
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "FOO",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"WIDTH (\'FOO\') cannot be converted into int" in r
        self.assertTrue(err)

        # bbox should be formatted like "double,double,double,double"
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"BBOX (\'-16817707,-4710778,5696513\') cannot be converted into a rectangle" in r
        self.assertTrue(err)

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,FOO",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"BBOX (\'-16817707,-4710778,5696513,FOO\') cannot be converted into a rectangle" in r
        self.assertTrue(err)

        # test invalid bbox : xmin > xmax
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "1,0,0,1",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"cannot be converted into a rectangle" in r
        self.assertTrue(err)

        # test invalid bbox : ymin > ymax
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "0,1,0,0",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"cannot be converted into a rectangle" in r
        self.assertTrue(err)

        # opacities should be a list of int
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "OPACITIES": "253,FOO",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"OPACITIES (\'253,FOO\') cannot be converted into a list of int" in r
        self.assertTrue(err)

        # filters should be formatted like "layer0:filter0;layer1:filter1"
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": "Country \"name\" = 'eurasia'"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"FILTER (\'Country \"name\" = \'eurasia\'\') is not properly formatted" in r
        self.assertTrue(err)

        # selections should be formatted like "layer0:id0,id1;layer1:id0"
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "SELECTION": "Country=4"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"SELECTION (\'Country=4\') is not properly formatted" in r
        self.assertTrue(err)

        # invalid highlight geometries
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_GEOM": "POLYGONN((-15000000 10000000, -15000000 6110620, 2500000 6110620, 2500000 10000000, -15000000 10000000))",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"HIGHLIGHT_GEOM (\'POLYGONN((-15000000 10000000, -15000000 6110620, 2500000 6110620, 2500000 10000000, -15000000 10000000))\') cannot be converted into a list of geometries" in r
        self.assertTrue(err)

        # invalid highlight label colors
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_LABELCOLOR": "%2300230000;%230023000",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"HIGHLIGHT_LABELCOLOR (\'#00230000;#0023000\') cannot be converted into a list of colors" in r
        self.assertTrue(err)

        # invalid list of label sizes
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_LABELSIZE": "16;17;FOO",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"HIGHLIGHT_LABELSIZE (\'16;17;FOO\') cannot be converted into a list of int" in r
        self.assertTrue(err)

        # invalid list of label buffer size
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_LABELBUFFERSIZE": "1.5;2;FF",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"HIGHLIGHT_LABELBUFFERSIZE (\'1.5;2;FF\') cannot be converted into a list of float" in r
        self.assertTrue(err)

        # invalid buffer color
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_LABELBUFFERCOLOR": "%232300FF00;%232300FF0",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        err = b"HIGHLIGHT_LABELBUFFERCOLOR (\'#2300FF00;#2300FF0\') cannot be converted into a list of colors" in r
        self.assertTrue(err)

    def test_wms_getmap_transparent(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "TRANSPARENT": "TRUE"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Transparent")

    def test_wms_getmap_labeling_settings(self):
        # Test the `DrawRectOnly` option
        # May fail if the labeling position engine is tweaked.

        d = unitTestDataPath('qgis_server_accesscontrol') + '/'
        project = os.path.join(d, "project_labeling_settings.qgs")
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "TRANSPARENT": "TRUE"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_LabelingSettings")

    def test_wms_getmap_background(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "BGCOLOR": "green"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Background")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "BGCOLOR": "0x008000"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Background_Hex")

    def test_wms_getmap_order(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello,Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_LayerOrder")

    def test_wms_getmap_srs(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-151.7,-38.9,51.0,78.0",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_SRS")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-151.7,-38.9,51.0,78.0",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_SRS")

    def test_wms_getmap_style(self):
        # default style
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_StyleDefault")

        # custom style with STYLES parameter
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "STYLES": "custom",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_StyleCustom")

        # custom style with STYLE parameter
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "STYLE": "custom",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_StyleCustom")

        # mixed custom and default style with STYLES parameter
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels,Hello",
            "STYLES": "custom,",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_StyleMixed")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello,Country_Labels",
            "STYLES": "default,custom",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_StyleMixed_LayerOrder")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello,Country_Labels",
            "STYLES": ",custom",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_StyleMixed_LayerOrder")

    def test_wms_getmap_filter(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": "Country:\"name\" = 'eurasia'"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter")

        # try to display a feature yet filtered by the project
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectStatePath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": "Country:\"name\" = 'africa'"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter2")

        # display all features to check that initial filter is restored
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectStatePath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter3")

        # display multiple features filtered from multiple layers
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectStatePath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello,Hello_Filter_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": "Country: \"name\" IN ( 'arctic' , 'eurasia' );Hello: \"color\" = 'red';Hello_Filter_SubsetString: \"color\" = 'slate'"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter4")

        # display multiple features filtered from multiple layers with same filter for some
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Country_Diagrams,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "1017529,-4226661,11271098,17063190",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": "Country,Country_Diagrams: \"name\" IN ( 'africa' , 'eurasia' );Hello: \"color\" IN ( 'magenta' , 'cerese' )"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter5")

        # test that filters with colons in values work as expected
        projectPath = os.path.join(self.testdata_path, "test_project_wms_filter.qgs")
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "points",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-80000,25000,-15000,50000.0",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": "points:\"name\" = 'foo:bar'"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter6")

        # Error in filter (missing quote after africa) with multiple layer filter
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Country_Diagrams,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "1017529,-4226661,11271098,17063190",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": "Country,Country_Diagrams: \"name\" IN ( 'africa , 'eurasia' );Hello: \"color\" IN ( 'magenta' , 'cerese' )"
        }.items())])

        expected = self.strip_version_xmlns(
            b'<ServiceExceptionReport  >\n <ServiceException code="Security">The filter string  "name" IN ( \'africa , \'eurasia\' ) has been rejected because of security reasons. Note: Text strings have to be enclosed in single or double quotes. A space between each word / special character is mandatory. Allowed Keywords and special characters are  IS,NOT,NULL,AND,OR,IN,=,&lt;,>=,>,>=,!=,\',\',(,),DMETAPHONE,SOUNDEX. Not allowed are semicolons in the filter expression.</ServiceException>\n</ServiceExceptionReport>\n')
        r, h = self._result(self._execute_request(qs))

        self.assertEqual(self.strip_version_xmlns(r), expected)

        # Test IS NOT NULL, so display all features
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectStatePath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": "Country:\"name\" IS NOT NULL"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter3")

    def test_wms_getmap_filter_ogc(self):
        filter = "<Filter><PropertyIsEqualTo><PropertyName>name</PropertyName>" + \
                 "<Literal>eurasia</Literal></PropertyIsEqualTo></Filter>"
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": filter
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter_OGC")

    def test_wms_getmap_filter_ogc_with_empty(self):
        filter = "(<Filter><PropertyIsEqualTo><PropertyName>name</PropertyName>" + \
                 "<Literal>eurasia</Literal></PropertyIsEqualTo></Filter>)()"
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": filter
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter_OGC")

        # empty filter
        filter = ("(<ogc:Filter xmlns=\"http://www.opengis.net/ogc\">"
                  "</ogc:Filter>)")
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": filter
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter_OGC2")

        # filter on the second layer
        filter_hello = ("(<Filter></Filter>)")
        filter_country = ("(<Filter><PropertyIsEqualTo><PropertyName>name"
                          "</PropertyName><Literal>eurasia</Literal>"
                          "</PropertyIsEqualTo></Filter>)")
        filter = "{}{}".format(filter_hello, filter_country)
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello,Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": filter
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter_OGC3")

    def test_wms_getmap_filter_ogc_v2(self):
        # with namespace
        filter = ('<fes:Filter xmlns:fes=\"http://www.opengis.net/fes/2.0\">'
                  '<fes:PropertyIsEqualTo>'
                  '<fes:ValueReference>name</fes:ValueReference>'
                  '<fes:Literal>eurasia</fes:Literal>'
                  '</fes:PropertyIsEqualTo>'
                  '</fes:Filter>')

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": filter
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter_OGC_V2")

        # without namespace (only with prefix)
        filter = ('<fes:Filter>'
                  '<fes:PropertyIsEqualTo>'
                  '<fes:ValueReference>name</fes:ValueReference>'
                  '<fes:Literal>eurasia</fes:Literal>'
                  '</fes:PropertyIsEqualTo>'
                  '</fes:Filter>')

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "FILTER": filter
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter_OGC_V2")

    def test_wms_getmap_selection(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "SELECTION": "Country: 4,1;Hello: 2,5"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Selection")

    def test_wms_getmap_diagrams(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Diagrams,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Diagrams")

    def test_wms_getmap_opacities(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "OPACITIES": "125, 50"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Opacities")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello,dem",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "OPACITIES": "125,50,150"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Opacities2")

        # Test OPACITIES with specific STYLES
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello,dem",
            "STYLES": "origin,default,default",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "OPACITIES": "125,50,150"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Opacities3")

    def test_wms_getmap_highlight(self):
        # highlight layer with color separated from sld
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_GEOM": "POLYGON((-15000000 10000000, -15000000 6110620, 2500000 6110620, 2500000 10000000, -15000000 10000000))",
            "HIGHLIGHT_SYMBOL": "<StyledLayerDescriptor><UserStyle><Name>Highlight</Name><FeatureTypeStyle><Rule><Name>Symbol</Name><LineSymbolizer><Stroke><SvgParameter name=\"stroke\">%23ea1173</SvgParameter><SvgParameter name=\"stroke-opacity\">1</SvgParameter><SvgParameter name=\"stroke-width\">1.6</SvgParameter></Stroke></LineSymbolizer></Rule></FeatureTypeStyle></UserStyle></StyledLayerDescriptor>",
            "HIGHLIGHT_LABELSTRING": "Highlight Layer!",
            "HIGHLIGHT_LABELSIZE": "16",
            "HIGHLIGHT_LABELCOLOR": "%2300FF0000",
            "HIGHLIGHT_LABELBUFFERCOLOR": "%232300FF00",
            "HIGHLIGHT_LABELBUFFERSIZE": "1.5",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Highlight")

    def test_wms_getmap_highlight_point(self):
        # checks SLD stroke-width works for Points See issue 19795 comments
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_GEOM": "POINT(-6250000 8055310)",
            "HIGHLIGHT_SYMBOL": "<?xml version=\"1.0\" encoding=\"UTF-8\"?><StyledLayerDescriptor xmlns=\"http://www.opengis.net/sld\" xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version=\"1.1.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xsi:schemaLocation=\"http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd\" xmlns:se=\"http://www.opengis.net/se\"><UserStyle><se:FeatureTypeStyle><se:Rule><se:PointSymbolizer><se:Graphic><se:Mark><se:WellKnownName>circle</se:WellKnownName><se:Stroke><se:SvgParameter name=\"stroke\">%23ff0000</se:SvgParameter><se:SvgParameter name=\"stroke-opacity\">1</se:SvgParameter><se:SvgParameter name=\"stroke-width\">7.5</se:SvgParameter></se:Stroke><se:Fill><se:SvgParameter name=\"fill\">%237bdcb5</se:SvgParameter><se:SvgParameter name=\"fill-opacity\">1</se:SvgParameter></se:Fill></se:Mark><se:Size>28.4</se:Size></se:Graphic></se:PointSymbolizer></se:Rule></se:FeatureTypeStyle></UserStyle></StyledLayerDescriptor>",
            "HIGHLIGHT_LABELSTRING": "Highlight Point :)",
            "HIGHLIGHT_LABELSIZE": "16",
            "HIGHLIGHT_LABELCOLOR": "%2300FF0000",
            "HIGHLIGHT_LABELBUFFERCOLOR": "%232300FF00",
            "HIGHLIGHT_LABELBUFFERSIZE": "1.2",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Highlight_Point")

    def test_wms_getmap_highlight_point_label_options(self):
        # checks HIGHLIGHT_LABEL_ROTATION, HIGHLIGHT_LABEL_HORIZONTAL_ALIGNMENT, HIGHLIGHT_LABEL_VERTICAL_ALIGNMENT
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_GEOM": "POINT(-6250000 8055310)",
            "HIGHLIGHT_SYMBOL": "<?xml version=\"1.0\" encoding=\"UTF-8\"?><StyledLayerDescriptor xmlns=\"http://www.opengis.net/sld\" xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version=\"1.1.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xsi:schemaLocation=\"http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd\" xmlns:se=\"http://www.opengis.net/se\"><UserStyle><se:FeatureTypeStyle><se:Rule><se:PointSymbolizer><se:Graphic><se:Mark><se:WellKnownName>circle</se:WellKnownName><se:Stroke><se:SvgParameter name=\"stroke\">%23ff0000</se:SvgParameter><se:SvgParameter name=\"stroke-opacity\">1</se:SvgParameter><se:SvgParameter name=\"stroke-width\">7.5</se:SvgParameter></se:Stroke><se:Fill><se:SvgParameter name=\"fill\">%237bdcb5</se:SvgParameter><se:SvgParameter name=\"fill-opacity\">1</se:SvgParameter></se:Fill></se:Mark><se:Size>28.4</se:Size></se:Graphic></se:PointSymbolizer></se:Rule></se:FeatureTypeStyle></UserStyle></StyledLayerDescriptor>",
            "HIGHLIGHT_LABELSTRING": "Highlight Point :)",
            "HIGHLIGHT_LABELSIZE": "16",
            "HIGHLIGHT_LABELCOLOR": "%2300FF0000",
            "HIGHLIGHT_LABELBUFFERCOLOR": "%232300FF00",
            "HIGHLIGHT_LABELBUFFERSIZE": "1.2",
            "HIGHLIGHT_LABEL_ROTATION": "45",
            "HIGHLIGHT_LABEL_HORIZONTAL_ALIGNMENT": "center",
            "HIGHLIGHT_LABEL_VERTICAL_ALIGNMENT": "half",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Highlight_Point_Label_Options")

    # TODO make Server Getmap work with SLD Highlight feature and enable test

    def test_wms_getmap_highlight_line(self):
        # checks SLD stroke-width works for Lines See issue 19795 comments
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Labels",
            "HIGHLIGHT_GEOM": "LINESTRING(-15000000 8055310, 2500000 8055310)",
            "HIGHLIGHT_SYMBOL": "<?xml version=\"1.0\" encoding=\"UTF-8\"?><StyledLayerDescriptor xmlns=\"http://www.opengis.net/sld\" xmlns:ogc=\"http://www.opengis.net/ogc\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version=\"1.1.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xsi:schemaLocation=\"http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd\" xmlns:se=\"http://www.opengis.net/se\"><UserStyle><se:FeatureTypeStyle><se:Rule><se:LineSymbolizer><se:Stroke><se:SvgParameter name=\"stroke\">%23ff0000</se:SvgParameter><se:SvgParameter name=\"stroke-opacity\">1</se:SvgParameter><se:SvgParameter name=\"stroke-width\">17.3</se:SvgParameter><se:SvgParameter name=\"stroke-linecap\">round</se:SvgParameter></se:Stroke></se:LineSymbolizer></se:Rule></se:FeatureTypeStyle></UserStyle></StyledLayerDescriptor>",
            "HIGHLIGHT_LABELSTRING": "",
            "HIGHLIGHT_LABELSIZE": "10",
            "HIGHLIGHT_LABELCOLOR": "black",
            "HIGHLIGHT_LABELBUFFERCOLOR": "white",
            "HIGHLIGHT_LABELBUFFERSIZE": "1",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])
        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Highlight_Line")

    def test_wms_getmap_annotations(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectAnnotationPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Annotations")

    def test_wms_getmap_sld(self):
        import socketserver
        import threading
        import http.server

        # Bring up a simple HTTP server
        os.chdir(unitTestDataPath() + '')
        handler = http.server.SimpleHTTPRequestHandler

        httpd = socketserver.TCPServer(('localhost', 0), handler)
        port = httpd.server_address[1]

        httpd_thread = threading.Thread(target=httpd.serve_forever)
        httpd_thread.setDaemon(True)
        httpd_thread.start()

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,db_point",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_SLDRestored")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "REQUEST": "GetMap",
            "VERSION": "1.1.1",
            "SERVICE": "WMS",
            "SLD": "http://localhost:" + str(
                port) + "/qgis_local_server/db_point.sld",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "WIDTH": "500",
            "HEIGHT": "500",
            "LAYERS": "db_point",
            "STYLES": "",
            "FORMAT": "image/png",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_SLD")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,db_point",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_SLDRestored")

        # Test SVG
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "REQUEST": "GetMap",
            "VERSION": "1.1.1",
            "SERVICE": "WMS",
            "SLD": "http://localhost:" + str(
                port) + "/qgis_local_server/db_point_svg.sld",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "WIDTH": "500",
            "HEIGHT": "500",
            "LAYERS": "db_point",
            "STYLES": "",
            "FORMAT": "image/png",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_SLD_SVG")

        httpd.server_close()

    def test_wms_getmap_sld_body(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,db_point",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_SLDRestored")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "REQUEST": "GetMap",
            "VERSION": "1.1.1",
            "SERVICE": "WMS",
            "SLD_BODY": "<?xml version=\"1.0\" encoding=\"UTF-8\"?><StyledLayerDescriptor xmlns=\"http://www.opengis.net/sld\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:ogc=\"http://www.opengis.net/ogc\" xsi:schemaLocation=\"http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd\" version=\"1.1.0\" xmlns:se=\"http://www.opengis.net/se\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"> <NamedLayer> <se:Name>db_point</se:Name> <UserStyle> <se:Name>db_point_style</se:Name> <se:FeatureTypeStyle> <se:Rule> <se:Name>Single symbol</se:Name> <ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\"> <ogc:PropertyIsEqualTo> <ogc:PropertyName>gid</ogc:PropertyName> <ogc:Literal>1</ogc:Literal> </ogc:PropertyIsEqualTo> </ogc:Filter> <se:PointSymbolizer uom=\"http://www.opengeospatial.org/se/units/metre\"> <se:Graphic> <se:Mark> <se:WellKnownName>square</se:WellKnownName> <se:Fill> <se:SvgParameter name=\"fill\">5e86a1</se:SvgParameter> </se:Fill> <se:Stroke> <se:SvgParameter name=\"stroke\">000000</se:SvgParameter> </se:Stroke> </se:Mark> <se:Size>0.007</se:Size> </se:Graphic> </se:PointSymbolizer> </se:Rule> </se:FeatureTypeStyle> </UserStyle> </NamedLayer> </StyledLayerDescriptor>",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "WIDTH": "500",
            "HEIGHT": "500",
            "LAYERS": "db_point",
            "STYLES": "",
            "FORMAT": "image/png",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_SLD")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,db_point",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_SLDRestored")

    def test_wms_getmap_sld_restore_labeling(self):
        """QGIS Server has to restore all the style. This test is not done
        to evaluate the SLD application but the style restoration and
        specifically the labeling."""
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "pointlabel",
            "STYLES": "",
            "SLD_BODY": "<?xml version=\"1.0\" encoding=\"UTF-8\"?><StyledLayerDescriptor xmlns=\"http://www.opengis.net/sld\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:ogc=\"http://www.opengis.net/ogc\" xsi:schemaLocation=\"http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd\" version=\"1.1.0\" xmlns:se=\"http://www.opengis.net/se\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"> <NamedLayer> <se:Name>pointlabel</se:Name> <UserStyle> <se:Name>pointlabel_style</se:Name> <se:FeatureTypeStyle> <se:Rule> <se:Name>Single symbol</se:Name> <se:PointSymbolizer uom=\"http://www.opengeospatial.org/se/units/metre\"> <se:Graphic> <se:Mark> <se:WellKnownName>square</se:WellKnownName> <se:Fill> <se:SvgParameter name=\"fill\">5e86a1</se:SvgParameter> </se:Fill> <se:Stroke> <se:SvgParameter name=\"stroke\">000000</se:SvgParameter> </se:Stroke> </se:Mark> <se:Size>0.007</se:Size> </se:Graphic> </se:PointSymbolizer> </se:Rule> </se:FeatureTypeStyle> </UserStyle> </NamedLayer> </StyledLayerDescriptor>",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])
        r, h = self._result(self._execute_request(qs))

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "pointlabel",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Labeling_Complex")

    def test_wms_getmap_group(self):
        """A WMS shall render the requested layers by drawing the leftmost in the list
        bottommost, the next one over that, and so on."""

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_Diagrams,Country_Labels,Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r_individual, _ = self._result(self._execute_request(qs))

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "CountryGroup",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r_group, _ = self._result(self._execute_request(qs))

        """ Debug check:
        f = open('grouped.png', 'wb+')
        f.write(r_group)
        f.close()
        f = open('individual.png', 'wb+')
        f.write(r_individual)
        f.close()
        #"""

        self.assertEqual(r_individual, r_group,
                         'Individual layers query and group layers query results should be identical')

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "group_short_name",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r_group, _ = self._result(self._execute_request(qs))

        self.assertEqual(r_individual, r_group,
                         'Individual layers query and group layers query with short name results should be identical')

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectGroupsPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "SLD_BODY": "<?xml version=\"1.0\" encoding=\"UTF-8\"?><StyledLayerDescriptor xmlns=\"http://www.opengis.net/sld\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:ogc=\"http://www.opengis.net/ogc\" xsi:schemaLocation=\"http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd\" version=\"1.1.0\" xmlns:se=\"http://www.opengis.net/se\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"> <NamedLayer> <se:Name>CountryGroup</se:Name></NamedLayer> </StyledLayerDescriptor>",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r_group_sld, _ = self._result(self._execute_request(qs))
        self.assertEqual(r_individual, r_group_sld,
                         'Individual layers query and SLD group layers query results should be identical')

    def test_wms_getmap_group_regression_20810(self):
        """A WMS shall render the requested layers by drawing the leftmost in the list
        bottommost, the next one over that, and so on. Even if the layers are inside groups."""

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(os.path.join(self.testdata_path,
                                                   'test_project_wms_grouped_layers.qgs')),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "BBOX": "613402.5658687877003,5809005.018114360981,619594.408781287726,5813869.006602735259",
            "CRS": "EPSG:25832",
            "WIDTH": "429",
            "HEIGHT": "337",
            "LAYERS": "osm,areas and symbols",
            "STYLES": ",",
            "FORMAT": "image/png",
            "DPI": "200",
            "MAP_RESOLUTION": "200",
            "FORMAT_OPTIONS": "dpi:200"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_GroupedLayersUp")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(os.path.join(self.testdata_path,
                                                   'test_project_wms_grouped_layers.qgs')),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "BBOX": "613402.5658687877003,5809005.018114360981,619594.408781287726,5813869.006602735259",
            "CRS": "EPSG:25832",
            "WIDTH": "429",
            "HEIGHT": "337",
            "LAYERS": "areas and symbols,osm",
            "STYLES": ",",
            "FORMAT": "image/png",
            "DPI": "200",
            "MAP_RESOLUTION": "200",
            "FORMAT_OPTIONS": "dpi:200"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_GroupedLayersDown")

    def test_wms_getmap_datasource_error(self):
        """Must throw a server exception if datasource if not available"""
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(os.path.join(self.testdata_path,
                                                   'test_project_wms_invalid_layers.qgs')),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "BBOX": "613402.5658687877003,5809005.018114360981,619594.408781287726,5813869.006602735259",
            "CRS": "EPSG:25832",
            "WIDTH": "429",
            "HEIGHT": "337",
            "LAYERS": "areas and symbols,osm",
            "STYLES": ",",
            "FORMAT": "image/png",
            "DPI": "200",
            "MAP_RESOLUTION": "200",
            "FORMAT_OPTIONS": "dpi:200"
        }.items())])

        r, h = self._result(self._execute_request(qs))

        self.assertTrue('ServerException' in str(r))

    @unittest.skipIf(os.environ.get('QGIS_CONTINUOUS_INTEGRATION_RUN', 'true'),
                     'Can\'t rely on external resources for continuous integration')
    def test_wms_getmap_external(self):
        # 1 bits
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "REQUEST": "GetMap",
            "LAYERS": "EXTERNAL_WMS:landsat,Country",
            "OPACITIES": "255,150",
            "landsat:layers": "GEBCO_LATEST",
            "landsat:dpiMode": "7",
            "landsat:url": "https://www.gebco.net/data_and_products/gebco_web_services/web_map_service/mapserv",
            "landsat:crs": "EPSG:4326",
            "landsat:styles": "default",
            "landsat:format": "image/jpeg",
            "landsat:bbox": "-90,-180,90,180",
            "landsat:version": "1.3.0",
            "STYLES": "",
            "BBOX": "-90,-180,90,180",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_External", 20000)

    def test_wms_getmap_root_custom_layer_order_regression_21917(self):
        """When drawing root layer, custom layer order order should be respected."""

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(os.path.join(self.testdata_path,
                                                   'bug_21917_root_layer_order.qgs')),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "BBOX": "44.9014,8.20346,44.9015,8.20355",
            "CRS": "EPSG:4326",
            "WIDTH": "400",
            "HEIGHT": "400",
            "LAYERS": "group",
            "STYLES": ",",
            "FORMAT": "image/png",
            "DPI": "200",
            "MAP_RESOLUTION": "200",
            "FORMAT_OPTIONS": "dpi:200"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Group_Layer_Order")

        # Check with root_layer
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(os.path.join(self.testdata_path,
                                                   'bug_21917_root_layer_order.qgs')),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "BBOX": "44.9014,8.20346,44.9015,8.20355",
            "CRS": "EPSG:4326",
            "WIDTH": "400",
            "HEIGHT": "400",
            "LAYERS": "root_layer",
            "STYLES": ",",
            "FORMAT": "image/png",
            "DPI": "200",
            "MAP_RESOLUTION": "200",
            "FORMAT_OPTIONS": "dpi:200"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Group_Layer_Order")

    def test_wms_getmap_tile_buffer(self):
        """Test the implementation of tile_map_edge_buffer from mapserver."""

        # Check without tiled parameters (default is false)
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(
                os.path.join(self.testdata_path, 'wms_tile_buffer.qgs')),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "BBOX": "310187,6163153,324347,6177313",
            "CRS": "EPSG:3857",
            "WIDTH": "512",
            "HEIGHT": "512",
            "LAYERS": "wms_tile_buffer_data",
            "FORMAT": "image/png"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Tiled_False")

        # Check with tiled=false
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(
                os.path.join(self.testdata_path, 'wms_tile_buffer.qgs')),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "BBOX": "310187,6163153,324347,6177313",
            "CRS": "EPSG:3857",
            "WIDTH": "512",
            "HEIGHT": "512",
            "LAYERS": "wms_tile_buffer_data",
            "FORMAT": "image/png",
            "TILED": "false"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Tiled_False")

        # Check with tiled=true
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(
                os.path.join(self.testdata_path, 'wms_tile_buffer.qgs')),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "BBOX": "310187,6163153,324347,6177313",
            "CRS": "EPSG:3857",
            "WIDTH": "512",
            "HEIGHT": "512",
            "LAYERS": "wms_tile_buffer_data",
            "FORMAT": "image/png",
            "TILED": "true"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Tiled_True")

        # Check with labels and tiled=false
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(
                os.path.join(self.testdata_path, 'wms_tile_buffer.qgs')),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "BBOX": "310187,6163153,324347,6177313",
            "CRS": "EPSG:3857",
            "WIDTH": "512",
            "HEIGHT": "512",
            "LAYERS": "wms_tile_buffer_labels",
            "FORMAT": "image/png",
            "TILED": "false"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Tiled_Labels_False")

        # Check with labels and tiled=true
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(
                os.path.join(self.testdata_path, 'wms_tile_buffer.qgs')),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "BBOX": "310187,6163153,324347,6177313",
            "CRS": "EPSG:3857",
            "WIDTH": "512",
            "HEIGHT": "512",
            "LAYERS": "wms_tile_buffer_labels",
            "FORMAT": "image/png",
            "TILED": "true"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Tiled_Labels_True")

        # Check with rotated labels and tiled=true
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(
                os.path.join(self.testdata_path, 'wms_tile_buffer.qgs')),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "BBOX": "317914,6163276,327923,6173244",
            "CRS": "EPSG:3857",
            "WIDTH": "512",
            "HEIGHT": "512",
            "LAYERS": "wms_tile_buffer_labels_rotated",
            "FORMAT": "image/png",
            "TILED": "true"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Tiled_Rotated_Labels_True")

    @unittest.skipIf(os.getenv('QGIS_CONTINUOUS_INTEGRATION_RUN'), "This tests fails on GH workflow")
    def test_mode8bit_with_transparency(self):
        # 8 bits
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(
                self.testdata_path + 'test_project_wms_8bit_with_transparency.qgs'),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "test_layer",
            "STYLES": "",
            "FORMAT": "image/png; mode=8bit",
            "BBOX": "913204.62,5606011.36,913217.45,5606025.58",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "TRANSPARENT": "TRUE"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Mode_8bit_with_transparency")

    def test_multiple_layers_with_equal_name(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(os.path.join(self.testdata_path,
                                                   'test_project_wms_duplicate_names.qgz')),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "BBOX": "613402.5658687877003,5809005.018114360981,619594.408781287726,5813869.006602735259",
            "CRS": "EPSG:25832",
            "WIDTH": "429",
            "HEIGHT": "337",
            "LAYERS": "layer",
            "STYLES": ",",
            "FORMAT": "image/png",
            "DPI": "200",
            "MAP_RESOLUTION": "200",
            "FORMAT_OPTIONS": "dpi:200"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_DuplicateNames")

    def test_wms_getmap_plus_sign(self):
        """Test issue GH #41116"""

        vl = QgsVectorLayer('Point?crs=epsg:4326&field=int:integer', 'test+plus', 'memory')
        p = QgsProject()
        p.addMapLayers([vl])
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": urllib.parse.quote('test+plus'),
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-170,-80,170,80",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request_project(qs, p))
        # No exceptions
        self.assertEqual(h['Content-Type'], 'image/png')
        self.assertFalse(b"The layer 'test plus' does not exist" in r)

        # + literal: we get an exception
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": 'test+plus',
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-170,-80,170,80",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self._execute_request_project(qs, p))
        self.assertTrue(b"The layer 'test plus' does not exist" in r)

    def test_wms_annotation_item(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(os.path.join(self.testdata_path,
                                                   'test_project_annotation_item.qgz')),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "BBOX": "44.9014,8.20346,44.9015,8.20364",
            "CRS": "EPSG:4326",
            "WIDTH": "800",
            "HEIGHT": "400",
            "LAYERS": "points",
            "STYLES": ",",
            "FORMAT": "image/png",
            "DPI": "96",
            "MAP_RESOLUTION": "96",
            "FORMAT_OPTIONS": "dpi:96"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_AnnotationItem")


if __name__ == '__main__':
    unittest.main()
