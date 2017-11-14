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
from qgis.core import QgsProject

# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = b'MAP=[^"]+|Content-Length: \d+'
RE_ATTRIBUTES = b'[^>\s]+=[^>\s]+'


class TestQgsServerWMS(QgsServerTestBase):

    """QGIS Server WMS Tests"""

    # Set to True to re-generate reference files for this class
    regenerate_reference = False

    def wms_request_compare(self, request, extra=None, reference_file=None, project='test_project.qgs'):
        project = self.testdata_path + project
        assert os.path.exists(project), "Project file not found: " + project

        query_string = 'https://www.qgis.org/?MAP=%s&SERVICE=WMS&VERSION=1.3&REQUEST=%s' % (urllib.parse.quote(project), request)
        if extra is not None:
            query_string += extra
        header, body = self._execute_request(query_string)
        response = header + body
        reference_path = self.testdata_path + (request.lower() if not reference_file else reference_file) + '.txt'
        self.store_reference(reference_path, response)
        f = open(reference_path, 'rb')
        expected = f.read()
        f.close()
        response = re.sub(RE_STRIP_UNCHECKABLE, b'*****', response)
        expected = re.sub(RE_STRIP_UNCHECKABLE, b'*****', expected)

        self.assertXMLEqual(response, expected, msg="request %s failed.\nQuery: %s\nExpected file: %s\nResponse:\n%s" % (query_string, request, reference_path, response.decode('utf-8')))

    def test_getcapabilities(self):
        self.wms_request_compare('GetCapabilities')

    def test_getprojectsettings(self):
        self.wms_request_compare('GetProjectSettings')

    def test_getcontext(self):
        self.wms_request_compare('GetContext')

    def test_getfeatureinfo(self):
        # Test getfeatureinfo response xml
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=text%2Fxml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320',
                                 'wms_getfeatureinfo-text-xml')

        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=&styles=&' +
                                 'info_format=text%2Fxml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320',
                                 'wms_getfeatureinfo-text-xml')

        # Test getfeatureinfo response html
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=text%2Fhtml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320',
                                 'wms_getfeatureinfo-text-html')

        #Test getfeatureinfo response html with geometry
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=text%2Fhtml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320&' +
                                 'with_geometry=true',
                                 'wms_getfeatureinfo-text-html-geometry')

        #Test getfeatureinfo response html with maptip
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=text%2Fhtml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320&' +
                                 'with_maptip=true',
                                 'wms_getfeatureinfo-text-html-maptip')

        # Test getfeatureinfo response text
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320&' +
                                 'info_format=text/plain',
                                 'wms_getfeatureinfo-text-plain')

        # Test getfeatureinfo default info_format
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320',
                                 'wms_getfeatureinfo-text-plain')

        # Test getfeatureinfo invalid info_format
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320&' +
                                 'info_format=InvalidFormat',
                                 'wms_getfeatureinfo-invalid-format')

        # Regression for #8656
        # Mind the gap! (the space in the FILTER expression)
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A3857&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER=testlayer%20%C3%A8%C3%A9' + urllib.parse.quote(':"NAME" = \'two\''),
                                 'wms_getfeatureinfo_filter')

        # Test a filter with NO condition results
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A3857&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER=testlayer%20%C3%A8%C3%A9' + urllib.parse.quote(':"NAME" = \'two\' AND "utf8nameè" = \'no-results\''),
                                 'wms_getfeatureinfo_filter_no_results')

        # Test a filter with OR condition results
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A3857&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER=testlayer%20%C3%A8%C3%A9' + urllib.parse.quote(':"NAME" = \'two\' OR "NAME" = \'three\''),
                                 'wms_getfeatureinfo_filter_or')

        # Test a filter with OR condition and UTF results
        # Note that the layer name that contains utf-8 chars cannot be
        # to upper case.
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A3857&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER=testlayer%20%C3%A8%C3%A9' + urllib.parse.quote(':"NAME" = \'two\' OR "utf8nameè" = \'three èé↓\''),
                                 'wms_getfeatureinfo_filter_or_utf8')

        # Test feature info request with filter geometry
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A3857&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER_GEOM=POLYGON((8.2035381 44.901459,8.2035562 44.901459,8.2035562 44.901418,8.2035381 44.901418,8.2035381 44.901459))',
                                 'wms_getfeatureinfo_geometry_filter')

        # Test feature info request with invalid query_layer
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A3857&' +
                                 'query_layers=InvalidLayer&' +
                                 'FEATURE_COUNT=10&FILTER_GEOM=POLYGON((8.2035381 44.901459,8.2035562 44.901459,8.2035562 44.901418,8.2035381 44.901418,8.2035381 44.901459))',
                                 'wms_getfeatureinfo_invalid_query_layers')

        # Test feature info request with '+' instead of ' ' in layers and
        # query_layers parameters
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer+%C3%A8%C3%A9&styles=&' +
                                 'info_format=text%2Fxml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer+%C3%A8%C3%A9&X=190&Y=320',
                                 'wms_getfeatureinfo-text-xml')

        # layer1 is a clone of layer0 but with a scale visibility. Thus,
        # GetFeatureInfo response contains only a feature for layer0 and layer1
        # is ignored for the required bbox. Without the scale visibility option,
        # the feature for layer1 would have been in the response too.
        mypath = self.testdata_path + "test_project_scalevisibility.qgs"
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=layer0,layer1&styles=&' +
                                 'VERSION=1.1.0&' +
                                 'info_format=text%2Fxml&' +
                                 'width=500&height=500&srs=EPSG%3A4326' +
                                 '&bbox=8.1976,44.8998,8.2100,44.9027&' +
                                 'query_layers=layer0,layer1&X=235&Y=243',
                                 'wms_getfeatureinfo_notvisible',
                                 'test_project_scalevisibility.qgs')

    def test_describelayer(self):
        # Test DescribeLayer
        self.wms_request_compare('DescribeLayer',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'SLD_VERSION=1.1.0',
                                 'describelayer')

    def test_getstyles(self):
        # Test GetStyles
        self.wms_request_compare('GetStyles',
                                 '&layers=testlayer%20%C3%A8%C3%A9&',
                                 'getstyles')

    def test_wms_getschemaextension(self):
        self.wms_request_compare('GetSchemaExtension',
                                 '',
                                 'getschemaextension')

    def wms_request_compare_project(self, request, extra=None, reference_file=None):
        projectPath = self.testdata_path + "test_project.qgs"
        assert os.path.exists(projectPath), "Project file not found: " + projectPath

        project = QgsProject()
        project.read(projectPath)

        query_string = 'https://www.qgis.org/?SERVICE=WMS&VERSION=1.3&REQUEST=%s' % (request)
        if extra is not None:
            query_string += extra
        header, body = self._execute_request_project(query_string, project)
        response = header + body
        reference_path = self.testdata_path + (request.lower() if not reference_file else reference_file) + '.txt'
        self.store_reference(reference_path, response)
        f = open(reference_path, 'rb')
        expected = f.read()
        f.close()
        response = re.sub(RE_STRIP_UNCHECKABLE, b'*****', response)
        expected = re.sub(RE_STRIP_UNCHECKABLE, b'*****', expected)

        self.assertXMLEqual(response, expected, msg="request %s failed.\nQuery: %s\nExpected file: %s\nResponse:\n%s" % (query_string, request, reference_path, response.decode('utf-8')))

    def test_wms_getcapabilities_project(self):
        self.wms_request_compare_project('GetCapabilities')

    def wms_inspire_request_compare(self, request):
        """WMS INSPIRE tests"""
        project = self.testdata_path + "test_project_inspire.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = '?MAP=%s&SERVICE=WMS&VERSION=1.3.0&REQUEST=%s' % (urllib.parse.quote(project), request)
        header, body = self._execute_request(query_string)
        response = header + body
        reference_path = self.testdata_path + request.lower() + '_inspire.txt'
        self.store_reference(reference_path, response)
        f = open(reference_path, 'rb')
        expected = f.read()
        f.close()
        response = re.sub(RE_STRIP_UNCHECKABLE, b'', response)
        expected = re.sub(RE_STRIP_UNCHECKABLE, b'', expected)
        self.assertXMLEqual(response, expected, msg="request %s failed.\nQuery: %s\nExpected file: %s\nResponse:\n%s" % (query_string, request, reference_path, response.decode('utf-8')))

    def test_project_wms_inspire(self):
        """Test some WMS request"""
        for request in ('GetCapabilities',):
            self.wms_inspire_request_compare(request)

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

    def test_wms_getmap_invalid_parameters(self):
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

        # opacities should be a list of int
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

        r, h = self._result(self._execute_request(qs))

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

        r, h = self._result(self._execute_request(qs))

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

        r, h = self._result(self._execute_request(qs))

        item_found = False
        for item in str(r).split("\\n"):
            if "OnlineResource" in item and "xlink:href=\"my_wms_advertised_url?" in item:
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
        r, h = self._result(self._execute_request(qs))

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
            "SLD": "<?xml version=\"1.0\" encoding=\"UTF-8\"?><StyledLayerDescriptor xmlns=\"http://www.opengis.net/sld\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:ogc=\"http://www.opengis.net/ogc\" xsi:schemaLocation=\"http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd\" version=\"1.1.0\" xmlns:se=\"http://www.opengis.net/se\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"> <NamedLayer> <se:Name>db_point</se:Name> <UserStyle> <se:Name>db_point_style</se:Name> <se:FeatureTypeStyle> <se:Rule> <se:Name>Single symbol</se:Name> <ogc:Filter xmlns:ogc=\"http://www.opengis.net/ogc\"> <ogc:PropertyIsEqualTo> <ogc:PropertyName>gid</ogc:PropertyName> <ogc:Literal>1</ogc:Literal> </ogc:PropertyIsEqualTo> </ogc:Filter> <se:PointSymbolizer uom=\"http://www.opengeospatial.org/se/units/metre\"> <se:Graphic> <se:Mark> <se:WellKnownName>square</se:WellKnownName> <se:Fill> <se:SvgParameter name=\"fill\">5e86a1</se:SvgParameter> </se:Fill> <se:Stroke> <se:SvgParameter name=\"stroke\">000000</se:SvgParameter> </se:Stroke> </se:Mark> <se:Size>0.007</se:Size> </se:Graphic> </se:PointSymbolizer> </se:Rule> </se:FeatureTypeStyle> </UserStyle> </NamedLayer> </StyledLayerDescriptor>",
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
            "map0:EXTENT": "-33626185.498,-13032965.185,33978427.737,16020257.031",
            "map0:LAYERS": "Country,Hello",
            "CRS": "EPSG:3857",
            "SELECTION": "Country: 4",
            "LAYERS": "Country,Hello",
            "OPACITIES": "125,125"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Opacity")

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
            "SELECTION": "Country: 4",
            "LAYERS": "Country,Hello",
            "OPACITIES": "125%2C125"
        }.items())])

        r, h = self._result(self._execute_request(qs))
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
        self._img_diff_error(r, h, "WMS_GetLegendGraphic_LayerSpace")

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

    def test_wms_GetProjectSettings_wms_print_layers(self):
        projectPath = self.testdata_path + "test_project_wms_printlayers.qgs"
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": projectPath,
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetProjectSettings"
        }.items())])
        header, body = self._execute_request(qs)
        xmlResult = body.decode('utf-8')
        self.assertTrue(xmlResult.find("<WMSBackgroundLayer>1</WMSBackgroundLayer>") != -1)
        self.assertTrue(xmlResult.find("<WMSDataSource>contextualWMSLegend=0&amp;crs=EPSG:21781&amp;dpiMode=7&amp;featureCount=10&amp;format=image/png&amp;layers=public_geo_gemeinden&amp;styles=&amp;url=https://qgiscloud.com/mhugent/qgis_unittest_wms/wms?</WMSDataSource>") != -1)
        self.assertTrue(xmlResult.find("<WMSPrintLayer>contextualWMSLegend=0&amp;amp;crs=EPSG:21781&amp;amp;dpiMode=7&amp;amp;featureCount=10&amp;amp;format=image/png&amp;amp;layers=public_geo_gemeinden&amp;amp;styles=&amp;amp;url=https://qgiscloud.com/mhugent/qgis_unittest_wms_print/wms?</WMSPrintLayer>") != -1)


if __name__ == '__main__':
    unittest.main()
