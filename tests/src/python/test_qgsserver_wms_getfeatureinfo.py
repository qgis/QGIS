# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer GetFeatureInfo WMS.

From build dir, run: ctest -R PyQgsServerWMSGetFeatureInfo -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '11/03/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all
# executions
os.environ['QT_HASH_SEED'] = '1'

import re
import urllib.request
import urllib.parse
import urllib.error

import json

from qgis.testing import unittest
from qgis.PyQt.QtCore import QSize

import osgeo.gdal  # NOQA

from test_qgsserver_wms import TestQgsServerWMSTestBase
from qgis.core import QgsProject
from qgis.server import QgsBufferServerRequest, QgsBufferServerResponse


class TestQgsServerWMSGetFeatureInfo(TestQgsServerWMSTestBase):
    """QGIS Server WMS Tests for GetFeatureInfo request"""

    # regenerate_reference = True

    def tearDown(self):
        super().tearDown()
        os.putenv('QGIS_SERVER_ALLOWED_EXTRA_SQL_TOKENS', '')

    def testGetFeatureInfo(self):
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

        # Test getfeatureinfo on non queryable layer
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer3&styles=&' +
                                 'info_format=text%2Fxml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer3&X=190&Y=320',
                                 'wms_getfeatureinfo-testlayer3-notqueryable')

        # Test getfeatureinfo on group without shortname (no queryable...)
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=groupwithoutshortname&styles=&' +
                                 'info_format=text%2Fxml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=groupwithoutshortname&X=190&Y=320',
                                 'wms_getfeatureinfo-groupwithoutshortname-notqueryable')

        # Test getfeatureinfo on group with shortname (no queryable...)
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=group_name&styles=&' +
                                 'info_format=text%2Fxml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=group_name&X=190&Y=320',
                                 'wms_getfeatureinfo-group_name-notqueryable')

        # Test getfeatureinfo response html
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=text%2Fhtml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320',
                                 'wms_getfeatureinfo-text-html')

        # Test getfeatureinfo response html with geometry
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=text%2Fhtml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320&' +
                                 'with_geometry=true',
                                 'wms_getfeatureinfo-text-html-geometry')

        # Test getfeatureinfo response html with maptip
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=text%2Fhtml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320&' +
                                 'with_maptip=true',
                                 'wms_getfeatureinfo-text-html-maptip')

        # Test getfeatureinfo response html with maptip in text mode
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=text%2Fplain&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320&' +
                                 'with_maptip=true',
                                 'wms_getfeatureinfo-text-html-maptip-plain')

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

        # Test feature info request with filter geometry
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A4326&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER_GEOM=POLYGON((8.2035381 44.901459,8.2035562 44.901459,8.2035562 44.901418,8.2035381 44.901418,8.2035381 44.901459))',
                                 'wms_getfeatureinfo_geometry_filter')

        # Test feature info request with filter geometry in non-layer CRS
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A3857&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER_GEOM=POLYGON ((913213.6839952 5606021.5399693, 913215.6988780 5606021.5399693, 913215.6988780 5606015.09643322, 913213.6839952 5606015.0964332, 913213.6839952 5606021.5399693))',
                                 'wms_getfeatureinfo_geometry_filter_3857')

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

        # Test GetFeatureInfo resolves "value map" widget values but also
        # Server usage of qgs and gpkg file
        mypath = self.testdata_path + "test_project_values.qgz"
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=layer0&styles=&' +
                                 'VERSION=1.3.0&' +
                                 'info_format=text%2Fxml&' +
                                 'width=926&height=787&srs=EPSG%3A4326' +
                                 '&bbox=912217,5605059,914099,5606652' +
                                 '&CRS=EPSG:3857' +
                                 '&FEATURE_COUNT=10' +
                                 '&QUERY_LAYERS=layer0&I=487&J=308',
                                 'wms_getfeatureinfo-values0-text-xml',
                                 'test_project_values.qgz')

        # Test GetFeatureInfo on raster layer
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=landsat&styles=&' +
                                 'info_format=text%2Fxml&transparent=true&' +
                                 'width=500&height=500&srs=EPSG%3A3857&' +
                                 'bbox=1989139.6,3522745.0,2015014.9,3537004.5&' +
                                 'query_layers=landsat&X=250&Y=250',
                                 'wms_getfeatureinfo-raster-text-xml')

    def testGetFeatureInfoValueRelation(self):
        """Test GetFeatureInfo resolves "value relation" widget values. regression 18518"""
        mypath = self.testdata_path + "test_project_values.qgz"
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=layer1&styles=&' +
                                 'VERSION=1.3.0&' +
                                 'info_format=text%2Fxml&' +
                                 'width=926&height=787&srs=EPSG%3A4326' +
                                 '&bbox=912217,5605059,914099,5606652' +
                                 '&CRS=EPSG:3857' +
                                 '&FEATURE_COUNT=10' +
                                 '&WITH_GEOMETRY=True' +
                                 '&QUERY_LAYERS=layer1&I=487&J=308',
                                 'wms_getfeatureinfo-values1-text-xml',
                                 'test_project_values.qgz')

    # TODO make GetFeatureInfo show what's in the display expression and
    # enable test
    @unittest.expectedFailure
    def testGetFeatureInfoRelationReference(self):
        """Test GetFeatureInfo solves "relation reference" widget "display expression" values"""
        mypath = self.testdata_path + "test_project_values.qgz"
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=layer2&styles=&' +
                                 'VERSION=1.3.0&' +
                                 'info_format=text%2Fxml&' +
                                 'width=926&height=787&srs=EPSG%3A4326' +
                                 '&bbox=912217,5605059,914099,5606652' +
                                 '&CRS=EPSG:3857' +
                                 '&FEATURE_COUNT=10' +
                                 '&WITH_GEOMETRY=True' +
                                 '&QUERY_LAYERS=layer2&I=487&J=308',
                                 'wms_getfeatureinfo-values2-text-xml',
                                 'test_project_values.qgz')

    def testGetFeatureInfoSortedByDesigner(self):
        """Test GetFeatureInfo resolves DRAG&DROP Designer order when use attribute form settings for GetFeatureInfo
        is checked, see https://github.com/qgis/QGIS/pull/41031
        """
        mypath = self.testdata_path + "test_project_values.qgz"
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=layer2&styles=&' +
                                 'VERSION=1.3.0&' +
                                 'info_format=text%2Fxml&' +
                                 'width=926&height=787&srs=EPSG%3A4326' +
                                 '&bbox=912217,5605059,914099,5606652' +
                                 '&CRS=EPSG:3857' +
                                 '&FEATURE_COUNT=10' +
                                 '&WITH_GEOMETRY=True' +
                                 '&QUERY_LAYERS=layer3&I=487&J=308',
                                 'wms_getfeatureinfo-values5-text-xml',
                                 'test_project_values.qgz')

    def testGetFeatureInfoFilterGPKG(self):
        # 'test_project.qgz' ='test_project.qgs' but with a gpkg source + different fid
        # Regression for #8656 Test getfeatureinfo response xml with gpkg datasource
        # Mind the gap! (the space in the FILTER expression)
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A3857&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER=testlayer%20%C3%A8%C3%A9' +
                                 urllib.parse.quote(':"NAME" = \'two\''),
                                 'wms_getfeatureinfo_filter_gpkg',
                                 'test_project.qgz')

    def testGetFeatureInfoFilter(self):
        # Test getfeatureinfo response xml

        # Regression for #8656
        # Mind the gap! (the space in the FILTER expression)
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A3857&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER=testlayer%20%C3%A8%C3%A9' +
                                 urllib.parse.quote(':"NAME" = \'two\''),
                                 'wms_getfeatureinfo_filter')

        # Test a filter with NO condition results
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A3857&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER=testlayer%20%C3%A8%C3%A9' +
                                 urllib.parse.quote(
                                     ':"NAME" = \'two\' AND "utf8nameè" = \'no-results\''),
                                 'wms_getfeatureinfo_filter_no_results')

        # Test a filter with OR condition results
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A3857&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER=testlayer%20%C3%A8%C3%A9' +
                                 urllib.parse.quote(
                                     ':"NAME" = \'two\' OR "NAME" = \'three\''),
                                 'wms_getfeatureinfo_filter_or')

        # Test a filter with OR condition and UTF results
        # Note that the layer name that contains utf-8 chars cannot be
        # to upper case.
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A3857&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER=testlayer%20%C3%A8%C3%A9' +
                                 urllib.parse.quote(
                                     ':"NAME" = \'two\' OR "utf8nameè" = \'three èé↓\''),
                                 'wms_getfeatureinfo_filter_or_utf8')

        # Regression #18292 Server GetFeatureInfo FILTER search fails when
        # WIDTH, HEIGHT are not specified
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'srs=EPSG%3A3857&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER=testlayer%20%C3%A8%C3%A9' +
                                 urllib.parse.quote(':"NAME" = \'two\''),
                                 'wms_getfeatureinfo_filter_no_width')

        # Test a filter without CRS parameter
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER=testlayer%20%C3%A8%C3%A9' +
                                 urllib.parse.quote(':"NAME" = \'two\''),
                                 'wms_getfeatureinfo_filter_no_crs')

    def testGetFeatureInfoOGCfilterJSON(self):
        # OGC Filter test with info_format=application/json

        # Test OGC filter with I/J and BBOX
        self.wms_request_compare('GetFeatureInfo',
                                 '&LAYERS=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=application%2Fjson&' +
                                 'WIDTH=1266&HEIGHT=531&' +
                                 'QUERY_LAYERS=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&' +
                                 'CRS=EPSG:4326&' +
                                 'BBOX=44.90139177500000045,8.20339159915254967,44.90148522499999473,8.20361440084745297&' +
                                 'I=882&J=282&'
                                 'FILTER=<Filter><PropertyIsEqualTo><PropertyName>id</PropertyName><Literal>2</Literal></PropertyIsEqualTo></Filter>', 'wms_getfeatureinfo_filter_ogc')

        # Test OGC filter with I/J and BBOX, filter id 3: empty result
        self.wms_request_compare('GetFeatureInfo',
                                 '&LAYERS=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=application%2Fjson&' +
                                 'WIDTH=1266&HEIGHT=531&' +
                                 'QUERY_LAYERS=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&' +
                                 'CRS=EPSG:4326&' +
                                 'BBOX=44.90139177500000045,8.20339159915254967,44.90148522499999473,8.20361440084745297&' +
                                 'I=882&J=282&'
                                 'FILTER=<Filter><PropertyIsEqualTo><PropertyName>id</PropertyName><Literal>3</Literal></PropertyIsEqualTo></Filter>', 'wms_getfeatureinfo_filter_ogc_empty')

        # Test OGC filter with no I/J and BBOX
        self.wms_request_compare('GetFeatureInfo',
                                 '&LAYERS=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=application%2Fjson&' +
                                 'WIDTH=1266&HEIGHT=531&' +
                                 'QUERY_LAYERS=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&' +
                                 'CRS=EPSG:4326&' +
                                 'FILTER=<Filter><PropertyIsEqualTo><PropertyName>id</PropertyName><Literal>2</Literal></PropertyIsEqualTo></Filter>', 'wms_getfeatureinfo_filter_ogc')

        # Test OGC filter with no I/J and wrong BBOX
        self.wms_request_compare('GetFeatureInfo',
                                 '&LAYERS=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=application%2Fjson&' +
                                 'WIDTH=1266&HEIGHT=531&' +
                                 'QUERY_LAYERS=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&' +
                                 'CRS=EPSG:4326&' +
                                 'BBOX=46,9,47,10&' +
                                 'FILTER=<Filter><PropertyIsEqualTo><PropertyName>id</PropertyName><Literal>2</Literal></PropertyIsEqualTo></Filter>', 'wms_getfeatureinfo_filter_ogc_empty')

        # Test OGC filter with no I/J and BBOX plus complex OR filter
        self.wms_request_compare('GetFeatureInfo',
                                 '&LAYERS=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=application%2Fjson&' +
                                 'WIDTH=1266&HEIGHT=531&' +
                                 'QUERY_LAYERS=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&' +
                                 'CRS=EPSG:4326&' +
                                 'FILTER=<Filter><Or><PropertyIsEqualTo><PropertyName>id</PropertyName><Literal>2</Literal></PropertyIsEqualTo>' +
                                 '<PropertyIsEqualTo><PropertyName>id</PropertyName><Literal>3</Literal></PropertyIsEqualTo></Or></Filter>', 'wms_getfeatureinfo_filter_ogc_complex')

        # Test OGC filter with no I/J and BBOX plus complex AND filter
        self.wms_request_compare('GetFeatureInfo',
                                 '&LAYERS=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=application%2Fjson&' +
                                 'WIDTH=1266&HEIGHT=531&' +
                                 'QUERY_LAYERS=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&' +
                                 'CRS=EPSG:4326&' +
                                 'FILTER=<Filter><And><PropertyIsEqualTo><PropertyName>id</PropertyName><Literal>2</Literal></PropertyIsEqualTo>' +
                                 '<PropertyIsEqualTo><PropertyName>id</PropertyName><Literal>3</Literal></PropertyIsEqualTo></And></Filter>', 'wms_getfeatureinfo_filter_ogc_empty')

        # Test OGC filter with no I/J and BBOX plus complex AND filter
        self.wms_request_compare('GetFeatureInfo',
                                 '&LAYERS=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=application%2Fjson&' +
                                 'WIDTH=1266&HEIGHT=531&' +
                                 'QUERY_LAYERS=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&' +
                                 'CRS=EPSG:4326&' +
                                 'FILTER=<Filter><And><PropertyIsEqualTo><PropertyName>id</PropertyName><Literal>2</Literal></PropertyIsEqualTo>' +
                                 '<PropertyIsEqualTo><PropertyName>name</PropertyName><Literal>two</Literal></PropertyIsEqualTo></And></Filter>', 'wms_getfeatureinfo_filter_ogc')

    def testGetFeatureInfoGML(self):
        # Test getfeatureinfo response gml
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=application%2Fvnd.ogc.gml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320',
                                 'wms_getfeatureinfo-text-gml')

        # Test getfeatureinfo response gml with gml
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=application%2Fvnd.ogc.gml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320&' +
                                 'with_geometry=true',
                                 'wms_getfeatureinfo-text-gml-geometry')

    def testGetFeatureInfoJSON(self):
        # simple test without geometry and info_format=application/json
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=application%2Fjson&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320',
                                 'wms_getfeatureinfo_json',
                                 normalizeJson=True)

        # simple test without geometry and info_format=application/geo+json
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=application%2Fgeo%2Bjson&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320',
                                 'wms_getfeatureinfo_geojson',
                                 normalizeJson=True)

        # test with several features and several layers
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9,fields_alias,exclude_attribute&styles=&' +
                                 'info_format=application%2Fjson&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9,fields_alias,exclude_attribute&' +
                                 'X=190&Y=320&FEATURE_COUNT=2&FI_POINT_TOLERANCE=200',
                                 'wms_getfeatureinfo_multiple_json',
                                 normalizeJson=True)

        # simple test with geometry with underlying layer in 3857
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=application%2Fjson&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320&' +
                                 'with_geometry=true',
                                 'wms_getfeatureinfo_geometry_json',
                                 'test_project_epsg3857.qgs',
                                 normalizeJson=True)

        # simple test with geometry with underlying layer in 4326
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=application%2Fjson&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320&' +
                                 'with_geometry=true',
                                 'wms_getfeatureinfo_geometry_json',
                                 'test_project.qgs',
                                 normalizeJson=True)

        # test with alias
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=fields_alias&styles=&' +
                                 'info_format=application%2Fjson&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=fields_alias&X=190&Y=320',
                                 'wms_getfeatureinfo_alias_json',
                                 normalizeJson=True)

        # test with excluded attributes
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=exclude_attribute&styles=&' +
                                 'info_format=application%2Fjson&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=exclude_attribute&X=190&Y=320',
                                 'wms_getfeatureinfo_exclude_attribute_json',
                                 normalizeJson=True)

        # test with raster layer
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=landsat&styles=&' +
                                 'info_format=application%2Fjson&transparent=true&' +
                                 'width=500&height=500&srs=EPSG%3A3857&' +
                                 'bbox=1989139.6,3522745.0,2015014.9,3537004.5&' +
                                 'query_layers=landsat&X=250&Y=250',
                                 'wms_getfeatureinfo_raster_json',
                                 normalizeJson=True)

    def testGetFeatureInfoGroupedLayers(self):
        """Test that we can get feature info from the top and group layers"""

        # areas+and+symbols (not nested)
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=52.44095517977704901,10.71171069440170776,52.440955186258563,10.71171070552261817' +
                                 '&CRS=EPSG:4326' +
                                 '&WIDTH=2&HEIGHT=2' +
                                 '&QUERY_LAYERS=areas+and+symbols' +
                                 '&INFO_FORMAT=application/json' +
                                 '&I=0&J=1' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_group_name_areas',
                                 'test_project_wms_grouped_layers.qgs',
                                 normalizeJson=True)

        # areas+and+symbols (nested)
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=52.44095517977704901,10.71171069440170776,52.440955186258563,10.71171070552261817' +
                                 '&CRS=EPSG:4326' +
                                 '&WIDTH=2&HEIGHT=2' +
                                 '&QUERY_LAYERS=areas+and+symbols' +
                                 '&INFO_FORMAT=application/json' +
                                 '&I=0&J=1' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_group_name_areas_nested',
                                 'test_project_wms_grouped_nested_layers.qgs',
                                 normalizeJson=True)

        # as-areas-short-name
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=52.44095517977704901,10.71171069440170776,52.440955186258563,10.71171070552261817' +
                                 '&CRS=EPSG:4326' +
                                 '&WIDTH=2&HEIGHT=2' +
                                 '&QUERY_LAYERS=as-areas-short-name' +
                                 '&INFO_FORMAT=application/json' +
                                 '&I=0&J=1' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_group_name_areas_nested',
                                 'test_project_wms_grouped_nested_layers.qgs',
                                 normalizeJson=True)

        # Top level:  QGIS Server - Grouped Layer
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=52.44095517977704901,10.71171069440170776,52.440955186258563,10.71171070552261817' +
                                 '&CRS=EPSG:4326' +
                                 '&WIDTH=2&HEIGHT=2' +
                                 '&QUERY_LAYERS=QGIS+Server+-+Grouped Nested Layer' +
                                 '&INFO_FORMAT=application/json' +
                                 '&I=0&J=1' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_group_name_top',
                                 'test_project_wms_grouped_nested_layers.qgs',
                                 normalizeJson=True)

        # Multiple matches from 2 layer groups
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=52.44095517977704901,10.71171069440170776,52.440955186258563,10.71171070552261817' +
                                 '&CRS=EPSG:4326' +
                                 '&WIDTH=2&HEIGHT=2' +
                                 '&QUERY_LAYERS=areas+and+symbols,city+and+district+boundaries' +
                                 '&INFO_FORMAT=application/json' +
                                 '&I=0&J=1' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_group_name_areas_cities',
                                 'test_project_wms_grouped_nested_layers.qgs',
                                 normalizeJson=True)

        # no_query group (nested)
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=52.44095517977704901,10.71171069440170776,52.440955186258563,10.71171070552261817' +
                                 '&CRS=EPSG:4326' +
                                 '&WIDTH=2&HEIGHT=2' +
                                 '&QUERY_LAYERS=no_query' +
                                 '&INFO_FORMAT=application/json' +
                                 '&I=0&J=1' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_group_no_query',
                                 'test_project_wms_grouped_nested_layers.qgs',
                                 normalizeJson=True)

        # query_child group (nested)
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=52.44095517977704901,10.71171069440170776,52.440955186258563,10.71171070552261817' +
                                 '&CRS=EPSG:4326' +
                                 '&WIDTH=2&HEIGHT=2' +
                                 '&QUERY_LAYERS=query_child' +
                                 '&INFO_FORMAT=application/json' +
                                 '&I=0&J=1' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_group_query_child',
                                 'test_project_wms_grouped_nested_layers.qgs',
                                 normalizeJson=True)

        # child_ok group (nested)
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=52.44095517977704901,10.71171069440170776,52.440955186258563,10.71171070552261817' +
                                 '&CRS=EPSG:4326' +
                                 '&WIDTH=2&HEIGHT=2' +
                                 '&QUERY_LAYERS=child_ok' +
                                 '&INFO_FORMAT=application/json' +
                                 '&I=0&J=1' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_group_query_child',
                                 'test_project_wms_grouped_nested_layers.qgs',
                                 normalizeJson=True)

        # as_areas_query_copy == as-areas-short-name-query-copy (nested)
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=52.44095517977704901,10.71171069440170776,52.440955186258563,10.71171070552261817' +
                                 '&CRS=EPSG:4326' +
                                 '&WIDTH=2&HEIGHT=2' +
                                 '&QUERY_LAYERS=as-areas-short-name-query-copy' +
                                 '&INFO_FORMAT=application/json' +
                                 '&I=0&J=1' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_group_query_child',
                                 'test_project_wms_grouped_nested_layers.qgs',
                                 normalizeJson=True)

    def testGetFeatureInfoJsonUseIdAsLayerName(self):
        """Test GH #36262 where json response + use layer id"""

        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=44.90139177500000045,8.20335906129666981,44.90148522499999473,8.20364693870333284' +
                                 '&CRS=EPSG:4326' +
                                 '&WIDTH=1568&HEIGHT=509' +
                                 '&LAYERS=testlayer_%C3%A8%C3%A9_cf86cf11_222f_4b62_929c_12cfc82b9774' +
                                 '&STYLES=' +
                                 '&FORMAT=image/jpeg' +
                                 '&QUERY_LAYERS=testlayer_%C3%A8%C3%A9_cf86cf11_222f_4b62_929c_12cfc82b9774' +
                                 '&INFO_FORMAT=application/json' +
                                 '&I=1022&J=269' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_json_layer_ids',
                                 'test_project_use_layer_ids.qgs',
                                 normalizeJson=True)

        # Raster
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=30.1492201749999964,17.81444988978388722,30.2599248249999988,18.15548111021611533' +
                                 '&CRS=EPSG:4326' +
                                 '&WIDTH=1568&HEIGHT=509' +
                                 '&LAYERS=landsat_a7d15b35_ca83_4b23_a9fb_af3fbdd60d15' +
                                 '&STYLES=' +
                                 '&FORMAT=image/jpeg' +
                                 '&QUERY_LAYERS=landsat_a7d15b35_ca83_4b23_a9fb_af3fbdd60d15' +
                                 '&INFO_FORMAT=application/json' +
                                 '&I=769&J=275&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_json_layer_ids_raster',
                                 'test_project_use_layer_ids.qgs',
                                 normalizeJson=True)

        @unittest.skipIf(os.environ.get('QGIS_CONTINUOUS_INTEGRATION_RUN', 'true'),
                         "This test cannot run in TRAVIS because it relies on cascading external services")
        def testGetFeatureInfoCascadingLayers(self):
            """Test that we can get feature info on cascading WMS layers"""

            project_name = 'bug_gh31177_gfi_cascading_wms.qgs'
            self.wms_request_compare('GetFeatureInfo',
                                     '&BBOX=852729.31,5631138.51,853012.18,5631346.17' +
                                     '&CRS=EPSG:3857' +
                                     '&WIDTH=850&HEIGHT=624' +
                                     '&QUERY_LAYERS=Alberate' +
                                     '&INFO_FORMAT=application/vnd.ogc.gml' +
                                     '&I=509&J=289' +
                                     '&FEATURE_COUNT=10',
                                     'wms_getfeatureinfo_cascading_issue31177',
                                     project_name)

    def testGetFeatureInfoRasterNoData(self):
        # outside the image in text
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=-39.43236293126383885,135.95002698514588246,-30.54405018572365194,156.29582900705395332' +
                                 '&CRS=EPSG:4326' +
                                 '&VERSION=1.3.0' +
                                 '&WIDTH=800&HEIGHT=400' +
                                 '&LAYERS=requires_warped_vrt' +
                                 '&QUERY_LAYERS=requires_warped_vrt' +
                                 '&I=1&J=1' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_raster_nodata_out_txt',
                                 'test_raster_nodata.qgz',
                                 raw=True)

        # 0 text
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=-39.43236293126383885,135.95002698514588246,-30.54405018572365194,156.29582900705395332' +
                                 '&CRS=EPSG:4326' +
                                 '&VERSION=1.3.0' +
                                 '&WIDTH=800&HEIGHT=400' +
                                 '&LAYERS=requires_warped_vrt' +
                                 '&QUERY_LAYERS=requires_warped_vrt' +
                                 '&I=576&J=163' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_raster_nodata_zero_txt',
                                 'test_raster_nodata.qgz',
                                 raw=True)

        # nodata text
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=-39.43236293126383885,135.95002698514588246,-30.54405018572365194,156.29582900705395332' +
                                 '&CRS=EPSG:4326' +
                                 '&VERSION=1.3.0' +
                                 '&WIDTH=800&HEIGHT=400' +
                                 '&LAYERS=requires_warped_vrt' +
                                 '&QUERY_LAYERS=requires_warped_vrt' +
                                 '&I=560&J=78' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_raster_nodata_txt',
                                 'test_raster_nodata.qgz',
                                 raw=True)

        # outside the image in html
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=-39.43236293126383885,135.95002698514588246,-30.54405018572365194,156.29582900705395332' +
                                 '&CRS=EPSG:4326' +
                                 '&VERSION=1.3.0' +
                                 '&INFO_FORMAT=text/html' +
                                 '&WIDTH=800&HEIGHT=400' +
                                 '&LAYERS=requires_warped_vrt' +
                                 '&QUERY_LAYERS=requires_warped_vrt' +
                                 '&I=1&J=1' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_raster_nodata_out_html',
                                 'test_raster_nodata.qgz',
                                 raw=True)

        # 0 html
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=-39.43236293126383885,135.95002698514588246,-30.54405018572365194,156.29582900705395332' +
                                 '&CRS=EPSG:4326' +
                                 '&VERSION=1.3.0' +
                                 '&INFO_FORMAT=text/html' +
                                 '&WIDTH=800&HEIGHT=400' +
                                 '&LAYERS=requires_warped_vrt' +
                                 '&QUERY_LAYERS=requires_warped_vrt' +
                                 '&I=576&J=163' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_raster_nodata_zero_html',
                                 'test_raster_nodata.qgz',
                                 raw=True)

        # nodata html
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=-39.43236293126383885,135.95002698514588246,-30.54405018572365194,156.29582900705395332' +
                                 '&CRS=EPSG:4326' +
                                 '&VERSION=1.3.0' +
                                 '&INFO_FORMAT=text/html' +
                                 '&WIDTH=800&HEIGHT=400' +
                                 '&LAYERS=requires_warped_vrt' +
                                 '&QUERY_LAYERS=requires_warped_vrt' +
                                 '&I=560&J=78' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_raster_nodata_html',
                                 'test_raster_nodata.qgz',
                                 raw=True)

        # outside the image in json
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=-39.43236293126383885,135.95002698514588246,-30.54405018572365194,156.29582900705395332' +
                                 '&CRS=EPSG:4326' +
                                 '&VERSION=1.3.0' +
                                 '&INFO_FORMAT=application/json' +
                                 '&WIDTH=800&HEIGHT=400' +
                                 '&LAYERS=requires_warped_vrt' +
                                 '&QUERY_LAYERS=requires_warped_vrt' +
                                 '&I=1&J=1' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_raster_nodata_out_json',
                                 'test_raster_nodata.qgz',
                                 raw=True)

        # 0 json
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=-39.43236293126383885,135.95002698514588246,-30.54405018572365194,156.29582900705395332' +
                                 '&CRS=EPSG:4326' +
                                 '&VERSION=1.3.0' +
                                 '&INFO_FORMAT=application/json' +
                                 '&WIDTH=800&HEIGHT=400' +
                                 '&LAYERS=requires_warped_vrt' +
                                 '&QUERY_LAYERS=requires_warped_vrt' +
                                 '&I=576&J=163' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_raster_nodata_zero_json',
                                 'test_raster_nodata.qgz',
                                 raw=True)

        # nodata json
        self.wms_request_compare('GetFeatureInfo',
                                 '&BBOX=-39.43236293126383885,135.95002698514588246,-30.54405018572365194,156.29582900705395332' +
                                 '&CRS=EPSG:4326' +
                                 '&VERSION=1.3.0' +
                                 '&INFO_FORMAT=application/json' +
                                 '&WIDTH=800&HEIGHT=400' +
                                 '&LAYERS=requires_warped_vrt' +
                                 '&QUERY_LAYERS=requires_warped_vrt' +
                                 '&I=560&J=78' +
                                 '&FEATURE_COUNT=10',
                                 'wms_getfeatureinfo_raster_nodata_json',
                                 'test_raster_nodata.qgz',
                                 raw=True)

    def test_wrong_filter_throws(self):
        """Test that a wrong FILTER expression throws an InvalidParameterValue exception"""

        _, response_body, _ = self.wms_request(
            'GetFeatureInfo',
            '&layers=testlayer%20%C3%A8%C3%A9&' +
            'INFO_FORMAT=text%2Fxml&' +
            'width=600&height=400&srs=EPSG%3A3857&' +
            'query_layers=testlayer%20%C3%A8%C3%A9&' +
            'FEATURE_COUNT=10&FILTER=testlayer%20%C3%A8%C3%A9' +
            urllib.parse.quote(':"XXXXXXXXXNAMEXXXXXXX" = \'two\''))

        self.assertEqual(response_body.decode('utf8'), '<?xml version="1.0" encoding="UTF-8"?>\n<ServiceExceptionReport xmlns="http://www.opengis.net/ogc" version="1.3.0">\n <ServiceException code="InvalidParameterValue">Filter not valid for layer testlayer èé: check the filter syntax and the field names.</ServiceException>\n</ServiceExceptionReport>\n')

    def testGetFeatureInfoFilterAllowedExtraTokens(self):
        """Test GetFeatureInfo with forbidden and extra tokens
        set by QGIS_SERVER_ALLOWED_EXTRA_SQL_TOKENS
        """
        project_path = self.testdata_path + "test_project_values.qgz"
        project = QgsProject()
        self.assertTrue(project.read(project_path))

        req_params = {
            'SERVICE': 'WMS',
            'REQUEST': 'GetFeatureInfo',
            'VERSION': '1.3.0',
            'LAYERS': 'layer4',
            'STYLES': '',
            'INFO_FORMAT': r'application%2Fjson',
            'WIDTH': '926',
            'HEIGHT': '787',
            'SRS': r'EPSG%3A4326',
            'BBOX': '912217,5605059,914099,5606652',
            'CRS': 'EPSG:3857',
            'FEATURE_COUNT': '10',
            'QUERY_LAYERS': 'layer4',
            'FILTER': 'layer4:"utf8nameè" != \'\'',
        }

        req = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % (k, v) for k, v in req_params.items()]))
        res = QgsBufferServerResponse()
        self.server.handleRequest(req, res, project)
        j_body = json.loads(bytes(res.body()).decode())
        self.assertEqual(len(j_body['features']), 3)

        req_params['FILTER'] = 'layer4:"utf8nameè" = \'three èé↓\''
        req = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % (k, v) for k, v in req_params.items()]))
        res = QgsBufferServerResponse()
        self.server.handleRequest(req, res, project)
        j_body = json.loads(bytes(res.body()).decode())
        self.assertEqual(len(j_body['features']), 1)

        req_params['FILTER'] = 'layer4:"utf8nameè" != \'three èé↓\''
        req = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % (k, v) for k, v in req_params.items()]))
        res = QgsBufferServerResponse()
        self.server.handleRequest(req, res, project)
        j_body = json.loads(bytes(res.body()).decode())
        self.assertEqual(len(j_body['features']), 2)

        # REPLACE filter
        req_params['FILTER'] = 'layer4:REPLACE ( "utf8nameè" , \'three\' , \'____\' ) != \'____ èé↓\''
        req = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % (k, v) for k, v in req_params.items()]))
        res = QgsBufferServerResponse()
        self.server.handleRequest(req, res, project)

        self.assertEqual(res.statusCode(), 403)

        os.putenv('QGIS_SERVER_ALLOWED_EXTRA_SQL_TOKENS', 'RePlAcE')
        self.server.serverInterface().reloadSettings()

        req = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % (k, v) for k, v in req_params.items()]))
        res = QgsBufferServerResponse()
        self.server.handleRequest(req, res, project)
        j_body = json.loads(bytes(res.body()).decode())
        self.assertEqual(len(j_body['features']), 2)

        os.putenv('QGIS_SERVER_ALLOWED_EXTRA_SQL_TOKENS', '')
        self.server.serverInterface().reloadSettings()

        req_params['FILTER'] = 'layer4:REPLACE ( "utf8nameè" , \'three\' , \'____\' ) != \'____ èé↓\''
        req = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % (k, v) for k, v in req_params.items()]))
        res = QgsBufferServerResponse()
        self.server.handleRequest(req, res, project)

        self.assertEqual(res.statusCode(), 403)

        # Multiple filters
        os.putenv('QGIS_SERVER_ALLOWED_EXTRA_SQL_TOKENS', 'RePlAcE,LowEr')
        self.server.serverInterface().reloadSettings()
        req_params['FILTER'] = 'layer4:LOWER ( REPLACE ( "utf8nameè" , \'three\' , \'THREE\' ) ) = \'three èé↓\''

        req = QgsBufferServerRequest('?' + '&'.join(["%s=%s" % (k, v) for k, v in req_params.items()]))
        res = QgsBufferServerResponse()
        self.server.handleRequest(req, res, project)
        j_body = json.loads(bytes(res.body()).decode())
        self.assertEqual(len(j_body['features']), 1)

    def testGetFeatureInfoSortedByDesignerWithJoinLayer(self):
        """Test GetFeatureInfo resolves DRAG&DROP Designer order when use attribute form settings for GetFeatureInfo
        with a column from a Joined Layer when the option is checked, see https://github.com/qgis/QGIS/pull/41031
        """
        mypath = self.testdata_path + "test_project_values.qgz"
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=layer2&styles=&' +
                                 'VERSION=1.3.0&' +
                                 'info_format=text%2Fxml&' +
                                 'width=926&height=787&srs=EPSG%3A4326' +
                                 '&bbox=912217,5605059,914099,5606652' +
                                 '&CRS=EPSG:3857' +
                                 '&FEATURE_COUNT=10' +
                                 '&WITH_GEOMETRY=True' +
                                 '&QUERY_LAYERS=layer4&I=487&J=308',
                                 'wms_getfeatureinfo-values4-text-xml',
                                 'test_project_values.qgz')


if __name__ == '__main__':
    unittest.main()
