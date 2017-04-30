# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer WMS.

From build dir, run: ctest -R PyQgsServerWMS -V


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

# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = b'MAP=[^"]+|Content-Length: \d+'
RE_ATTRIBUTES = b'[^>\s]+=[^>\s]+'


class TestQgsServerWMS(QgsServerTestBase):
    """QGIS Server WMS Tests"""

    # Set to True to re-generate reference files for this class
    regenerate_reference = False

    def wms_request_compare(self, request, extra=None, reference_file=None):
        project = self.testdata_path + "test_project.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = 'https://www.qgis.org/?MAP=%s&SERVICE=WMS&VERSION=1.3&REQUEST=%s' % (urllib.parse.quote(project), request)
        if extra is not None:
            query_string += extra
        header, body = self.server.handleRequest(query_string)
        response = header + body
        reference_path = self.testdata_path + (request.lower() if not reference_file else reference_file) + '.txt'
        self.store_reference(reference_path, response)
        f = open(reference_path, 'rb')
        expected = f.read()
        f.close()
        response = re.sub(RE_STRIP_UNCHECKABLE, b'*****', response)
        expected = re.sub(RE_STRIP_UNCHECKABLE, b'*****', expected)

        self.assertXMLEqual(response, expected, msg="request %s failed.\n Query: %s\n Expected:\n%s\n\n Response:\n%s" % (query_string, request, expected.decode('utf-8'), response.decode('utf-8')))

    def test_project_wms(self):
        """Test some WMS request"""
        for request in ('GetCapabilities', 'GetProjectSettings'):
            self.wms_request_compare(request)

        # Test getfeatureinfo response
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=text%2Fhtml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320',
                                 'wms_getfeatureinfo-text-html')

        # Test getfeatureinfo default info_format
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320',
                                 'wms_getfeatureinfo-text-plain')

        # Regression for #8656
        # Mind the gap! (the space in the FILTER expression)
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A3857&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER=testlayer%20%C3%A8%C3%A9' + urllib.parse.quote(':"NAME" = \'two\''),
                                 'wms_getfeatureinfo_filter')

    def wms_inspire_request_compare(self, request):
        """WMS INSPIRE tests"""
        project = self.testdata_path + "test_project_inspire.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = '?MAP=%s&SERVICE=WMS&VERSION=1.3.0&REQUEST=%s' % (urllib.parse.quote(project), request)
        header, body = self.server.handleRequest(query_string)
        response = header + body
        reference_path = self.testdata_path + request.lower() + '_inspire.txt'
        self.store_reference(reference_path, response)
        f = open(reference_path, 'rb')
        expected = f.read()
        f.close()
        response = re.sub(RE_STRIP_UNCHECKABLE, b'', response)
        expected = re.sub(RE_STRIP_UNCHECKABLE, b'', expected)
        self.assertXMLEqual(response, expected, msg="request %s failed.\n Query: %s\n Expected:\n%s\n\n Response:\n%s" % (query_string, request, expected.decode('utf-8'), response.decode('utf-8')))

    def test_project_wms_inspire(self):
        """Test some WMS request"""
        for request in ('GetCapabilities',):
            self.wms_inspire_request_compare(request)

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

        r, h = self._result(self.server.handleRequest(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Basic")

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

        r, h = self._result(self.server.handleRequest(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Transparent")

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

        r, h = self._result(self.server.handleRequest(qs))
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

        r, h = self._result(self.server.handleRequest(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Background_Hex")

    def test_wms_getcapabilities_url(self):
        # empty url in project
        project = os.path.join(self.testdata_path, "test_project_without_urls.qgs")
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetCapabilities",
            "STYLES": ""
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))

        item_found = False
        for item in str(r).split("\\n"):
            if "OnlineResource" in item:
                self.assertEqual("xlink:href=\"?" in item, True)
                item_found = True
        self.assertTrue(item_found)

        # url passed in quesry string
        project = os.path.join(self.testdata_path, "test_project_without_urls.qgs")
        qs = "https://www.qgis-server.org?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetCapabilities",
            "STYLES": ""
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))

        item_found = False
        for item in str(r).split("\\n"):
            if "OnlineResource" in item:
                self.assertEqual("xlink:href=\"https://www.qgis-server.org?" in item, True)
                item_found = True
        self.assertTrue(item_found)

        # url well defined in project
        project = os.path.join(self.testdata_path, "test_project_with_urls.qgs")
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetCapabilities",
            "STYLES": ""
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))

        item_found = False
        for item in str(r).split("\\n"):
            if "OnlineResource" in item:
                self.assertEqual("xlink:href=\"my_wms_advertised_url?" in item, True)
                item_found = True
        self.assertTrue(item_found)

    def test_wms_getmap_invalid_size(self):
        project = os.path.join(self.testdata_path, "test_project_with_size.qgs")
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "HEIGHT": "5001",
            "WIDTH": "5000"
        }.items())])

        expected = self.strip_version_xmlns(b'<ServiceExceptionReport version="1.3.0" xmlns="http://www.opengis.net/ogc">\n <ServiceException code="Size error">The requested map size is too large</ServiceException>\n</ServiceExceptionReport>\n')
        r, h = self._result(self.server.handleRequest(qs))

        self.assertEqual(self.strip_version_xmlns(r), expected)

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

        r, h = self._result(self.server.handleRequest(qs))
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
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))
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

        r, h = self._result(self.server.handleRequest(qs))
        self._img_diff_error(r, h, "WMS_GetMap_StyleDefault")

        # custom style
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

        r, h = self._result(self.server.handleRequest(qs))
        self._img_diff_error(r, h, "WMS_GetMap_StyleCustom")

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

        r, h = self._result(self.server.handleRequest(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Filter")

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
            "SELECTION": "Country: 4"
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Selection")

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

        r, h = self._result(self.server.handleRequest(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Opacities")

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
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Basic")

    @unittest.skip('Randomly failing to draw the map layer')
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
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:4326"
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))
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
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))
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
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))
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
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Rotation")

    def test_wms_getprint_selection(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country,Hello",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "SELECTION": "Country: 4"
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Selection")

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
        h, r = self.server.handleRequest(qs)
        self.assertEqual(-1, h.find(b'Content-Type: text/xml; charset=utf-8'), "Header: %s\nResponse:\n%s" % (h, r))
        self.assertNotEqual(-1, h.find(b'Content-Type: image/png'), "Header: %s\nResponse:\n%s" % (h, r))

    def test_getLegendGraphics_layertitle(self):
        """Test that does not return an exception but an image"""
        parms = {
            'MAP': self.testdata_path + "test_project.qgs",
            'SERVICE': 'WMS',
            'VERSION': '1.3.0',
            'REQUEST': 'GetLegendGraphic',
            'FORMAT': 'image/png',
            # 'WIDTH': '20', # optional
            # 'HEIGHT': '20', # optional
            'LAYER': u'testlayer%20èé',
            'LAYERTITLE': 'TRUE',
        }
        qs = '?' + '&'.join([u"%s=%s" % (k, v) for k, v in parms.items()])
        r, h = self._result(self.server.handleRequest(qs))
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
        r, h = self._result(self.server.handleRequest(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_test_layertitle_false", 250, QSize(15, 15))

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

        r, h = self._result(self.server.handleRequest(qs))
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

        r, h = self._result(self.server.handleRequest(qs))
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

        r, h = self._result(self.server.handleRequest(qs))
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

        r, h = self._result(self.server.handleRequest(qs))
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

        r, h = self._result(self.server.handleRequest(qs))
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

        r, h = self._result(self.server.handleRequest(qs))
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

        r, h = self._result(self.server.handleRequest(qs))
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

        r, h = self._result(self.server.handleRequest(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_SymbolSize")

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

        r, h = self._result(self.server.handleRequest(qs))
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

        r, h = self._result(self.server.handleRequest(qs))
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_BBox2")

    # WCS tests
    def wcs_request_compare(self, request):
        project = self.projectPath
        assert os.path.exists(project), "Project file not found: " + project

        query_string = '?MAP=%s&SERVICE=WCS&VERSION=1.0.0&REQUEST=%s' % (urllib.parse.quote(project), request)
        header, body = self.server.handleRequest(query_string)
        self.assert_headers(header, body)
        response = header + body
        reference_path = self.testdata_path + 'wcs_' + request.lower() + '.txt'
        self.store_reference(reference_path, response)
        f = open(reference_path, 'rb')
        expected = f.read()
        f.close()
        response = re.sub(RE_STRIP_UNCHECKABLE, b'', response)
        expected = re.sub(RE_STRIP_UNCHECKABLE, b'', expected)

        self.assertXMLEqual(response, expected, msg="request %s failed.\n Query: %s\n Expected:\n%s\n\n Response:\n%s" % (query_string, request, expected.decode('utf-8'), response.decode('utf-8')))


if __name__ == '__main__':
    unittest.main()
