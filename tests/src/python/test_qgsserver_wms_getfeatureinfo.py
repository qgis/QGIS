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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all
# executions
os.environ['QT_HASH_SEED'] = '1'

import re
import urllib.request
import urllib.parse
import urllib.error

import xml.etree.ElementTree as ET
import json

from qgis.testing import unittest
from qgis.PyQt.QtCore import QSize

import osgeo.gdal  # NOQA

from test_qgsserver_wms import TestQgsServerWMSTestBase
from qgis.core import QgsProject


class TestQgsServerWMSGetFeatureInfo(TestQgsServerWMSTestBase):

    """QGIS Server WMS Tests for GetFeatureInfo request"""

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

    def testGetFeatureInfoValueRelationArray(self):
        """Test GetFeatureInfo on "value relation" widget with array field (multiple selections)"""
        mypath = self.testdata_path + "test_project_values.qgz"
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=layer3&styles=&' +
                                 'VERSION=1.3.0&' +
                                 'info_format=text%2Fxml&' +
                                 'width=926&height=787&srs=EPSG%3A4326' +
                                 '&bbox=912217,5605059,914099,5606652' +
                                 '&CRS=EPSG:3857' +
                                 '&FEATURE_COUNT=10' +
                                 '&WITH_GEOMETRY=True' +
                                 '&QUERY_LAYERS=layer3&I=487&J=308',
                                 'wms_getfeatureinfo-values3-text-xml',
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

    def testGetFeatureInfoTolerance(self):
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=layer3&styles=&' +
                                 'VERSION=1.3.0&' +
                                 'info_format=text%2Fxml&' +
                                 'width=400&height=200' +
                                 '&bbox=913119.2,5605988.9,913316.0,5606047.4' +
                                 '&CRS=EPSG:3857' +
                                 '&FEATURE_COUNT=10' +
                                 '&WITH_GEOMETRY=False' +
                                 '&QUERY_LAYERS=layer3&I=193&J=100' +
                                 '&FI_POINT_TOLERANCE=0',
                                 'wms_getfeatureinfo_point_tolerance_0_text_xml',
                                 'test_project_values.qgz')

        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=layer3&styles=&' +
                                 'VERSION=1.3.0&' +
                                 'info_format=text%2Fxml&' +
                                 'width=400&height=200' +
                                 '&bbox=913119.2,5605988.9,913316.0,5606047.4' +
                                 '&CRS=EPSG:3857' +
                                 '&FEATURE_COUNT=10' +
                                 '&WITH_GEOMETRY=False' +
                                 '&QUERY_LAYERS=layer3&I=193&J=100' +
                                 '&FI_POINT_TOLERANCE=20',
                                 'wms_getfeatureinfo_point_tolerance_20_text_xml',
                                 'test_project_values.qgz')

        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=ls2d&styles=&' +
                                 'VERSION=1.3.0&' +
                                 'info_format=text%2Fxml&' +
                                 'width=400&height=200' +
                                 '&bbox=-50396.4,-2783.0,161715.8,114108.6' +
                                 '&CRS=EPSG:3857' +
                                 '&FEATURE_COUNT=10' +
                                 '&WITH_GEOMETRY=False' +
                                 '&QUERY_LAYERS=ls2d&I=153&J=147' +
                                 '&FI_LINE_TOLERANCE=0',
                                 'wms_getfeatureinfo_line_tolerance_0_text_xml',
                                 'test_project_values.qgz')

        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=ls2d&styles=&' +
                                 'VERSION=1.3.0&' +
                                 'info_format=text%2Fxml&' +
                                 'width=400&height=200' +
                                 '&bbox=-50396.4,-2783.0,161715.8,114108.6' +
                                 '&CRS=EPSG:3857' +
                                 '&FEATURE_COUNT=10' +
                                 '&WITH_GEOMETRY=False' +
                                 '&QUERY_LAYERS=ls2d&I=153&J=147' +
                                 '&FI_LINE_TOLERANCE=20',
                                 'wms_getfeatureinfo_line_tolerance_20_text_xml',
                                 'test_project_values.qgz')

        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=p2d&styles=&' +
                                 'VERSION=1.3.0&' +
                                 'info_format=text%2Fxml&' +
                                 'width=400&height=200' +
                                 '&bbox=-135832.0,-66482.4,240321.9,167300.4' +
                                 '&CRS=EPSG:3857' +
                                 '&FEATURE_COUNT=10' +
                                 '&WITH_GEOMETRY=False' +
                                 '&QUERY_LAYERS=p2d&I=206&J=144' +
                                 '&FI_POLYGON_TOLERANCE=0',
                                 'wms_getfeatureinfo_polygon_tolerance_0_text_xml',
                                 'test_project_values.qgz')

        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=p2d&styles=&' +
                                 'VERSION=1.3.0&' +
                                 'info_format=text%2Fxml&' +
                                 'width=400&height=200' +
                                 '&bbox=-135832.0,-66482.4,240321.9,167300.4' +
                                 '&CRS=EPSG:3857' +
                                 '&FEATURE_COUNT=10' +
                                 '&WITH_GEOMETRY=False' +
                                 '&QUERY_LAYERS=p2d&I=206&J=144' +
                                 '&FI_POLYGON_TOLERANCE=20',
                                 'wms_getfeatureinfo_polygon_tolerance_20_text_xml',
                                 'test_project_values.qgz')

    def testGetFeatureInfoPostgresTypes(self):
        # compare json list output with file
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=json' +
                                 '&info_format=text%2Fxml' +
                                 '&srs=EPSG%3A3857' +
                                 '&QUERY_LAYERS=json' +
                                 '&FILTER=json' +
                                 urllib.parse.quote(':"pk" = 1'),
                                 'get_postgres_types_json_list',
                                 'test_project_postgres_types.qgs')

        # compare dict output with file
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=json' +
                                 '&info_format=text%2Fxml' +
                                 '&srs=EPSG%3A3857' +
                                 '&QUERY_LAYERS=json' +
                                 '&FILTER=json' +
                                 urllib.parse.quote(':"pk" = 2'),
                                 'get_postgres_types_json_dict',
                                 'test_project_postgres_types.qgs')

        # compare decoded json field list
        response_header, response_body, query_string = self.wms_request('GetFeatureInfo',
                                                                        '&layers=json' +
                                                                        '&info_format=text%2Fxml' +
                                                                        '&srs=EPSG%3A3857' +
                                                                        '&QUERY_LAYERS=json' +
                                                                        '&FILTER=json' +
                                                                        urllib.parse.quote(
                                                                            ':"pk" = 1'),
                                                                        'test_project_postgres_types.qgs')
        root = ET.fromstring(response_body)
        for attribute in root.iter('Attribute'):
            if attribute.get('name') == 'jvalue':
                self.assertIsInstance(json.loads(attribute.get('value')), list)
                self.assertEqual(json.loads(attribute.get('value')), [1, 2, 3])
                self.assertEqual(
                    json.loads(
                        attribute.get('value')), [
                        1.0, 2.0, 3.0])
            if attribute.get('name') == 'jbvalue':
                self.assertIsInstance(json.loads(attribute.get('value')), list)
                self.assertEqual(json.loads(attribute.get('value')), [4, 5, 6])
                self.assertEqual(
                    json.loads(
                        attribute.get('value')), [
                        4.0, 5.0, 6.0])

        # compare decoded json field dict
        response_header, response_body, query_string = self.wms_request('GetFeatureInfo',
                                                                        '&layers=json' +
                                                                        '&info_format=text%2Fxml' +
                                                                        '&srs=EPSG%3A3857' +
                                                                        '&QUERY_LAYERS=json' +
                                                                        '&FILTER=json' +
                                                                        urllib.parse.quote(
                                                                            ':"pk" = 2'),
                                                                        'test_project_postgres_types.qgs')
        root = ET.fromstring(response_body)
        for attribute in root.iter('Attribute'):
            if attribute.get('name') == 'jvalue':
                self.assertIsInstance(json.loads(attribute.get('value')), dict)
                self.assertEqual(
                    json.loads(
                        attribute.get('value')), {
                        'a': 1, 'b': 2})
                self.assertEqual(
                    json.loads(
                        attribute.get('value')), {
                        'a': 1.0, 'b': 2.0})
            if attribute.get('name') == 'jbvalue':
                self.assertIsInstance(json.loads(attribute.get('value')), dict)
                self.assertEqual(
                    json.loads(
                        attribute.get('value')), {
                        'c': 4, 'd': 5})
                self.assertEqual(
                    json.loads(
                        attribute.get('value')), {
                        'c': 4.0, 'd': 5.0})


if __name__ == '__main__':
    unittest.main()
