"""QGIS Unit tests for QgsServer WFS.

From build dir, run: ctest -R PyQgsServerWFS -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'René-Luc Dhont'
__date__ = '19/09/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ['QT_HASH_SEED'] = '1'

import json
import re
import urllib.error
import urllib.parse
import urllib.request

import osgeo.gdal  # NOQA

from lxml import etree as et
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransform,
    QgsCoordinateTransformContext,
    QgsExpression,
    QgsFeature,
    QgsFeatureRequest,
    QgsGeometry,
    QgsProject,
    QgsVectorLayer,
)
from qgis.server import QgsServerRequest
from qgis.testing import unittest
from test_qgsserver import QgsServerTestBase

# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = br'MAP=[^"]+|Content-Length: \d+|timeStamp="[^"]+"'
RE_ATTRIBUTES = br'[^>\s]+=[^>\s]+'


class TestQgsServerWFS(QgsServerTestBase):
    """QGIS Server WFS Tests"""

    # Set to True in child classes to re-generate reference files for this class
    regenerate_reference = False

    def wfs_request_compare(self,
                            request, version='',
                            extra_query_string='',
                            reference_base_name=None,
                            project_file="test_project_wfs.qgs",
                            requestMethod=QgsServerRequest.GetMethod,
                            data=None):
        project = self.testdata_path + project_file
        assert os.path.exists(project), "Project file not found: " + project

        query_string = '?MAP={}&SERVICE=WFS&REQUEST={}'.format(
            urllib.parse.quote(project), request)
        if version:
            query_string += f'&VERSION={version}'

        if extra_query_string:
            query_string += f'&{extra_query_string}'

        header, body = self._execute_request(
            query_string, requestMethod=requestMethod, data=data)
        self.assert_headers(header, body)
        response = header + body

        if reference_base_name is not None:
            reference_name = reference_base_name
        else:
            reference_name = 'wfs_' + request.lower()

        if version == '1.0.0':
            reference_name += '_1_0_0'
        reference_name += '.txt'

        reference_path = self.testdata_path + reference_name

        self.store_reference(reference_path, response)
        f = open(reference_path, 'rb')
        expected = f.read()
        f.close()
        response = re.sub(RE_STRIP_UNCHECKABLE, b'', response)
        expected = re.sub(RE_STRIP_UNCHECKABLE, b'', expected)

        # depending on flex/bison versions, these strings will be interchangeable
        response = response.replace(b'end of file', b'$end')

        self.assertXMLEqual(response, expected, msg="request {} failed.\n Query: {}".format(
            query_string, request))
        return header, body

    def test_operation_not_supported(self):
        qs = f'?MAP={urllib.parse.quote(self.projectPath)}&SERVICE=WFS&VERSION=1.1.0&REQUEST=NotAValidRequest'
        self._assert_status_code(501, qs)

    def test_project_wfs(self):
        """Test some WFS request"""

        for request in ('GetCapabilities', 'DescribeFeatureType'):
            self.wfs_request_compare(request)
            self.wfs_request_compare(request, '1.0.0')

    def wfs_getfeature_compare(self, requestid, request):

        project = self.testdata_path + "test_project_wfs.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = '?MAP={}&SERVICE=WFS&VERSION=1.0.0&REQUEST={}'.format(
            urllib.parse.quote(project), request)
        header, body = self._execute_request(query_string)

        if requestid == 'hits':
            body = re.sub(br'timeStamp="\d+-\d+-\d+T\d+:\d+:\d+"',
                          b'timeStamp="****-**-**T**:**:**"', body)

        self.result_compare(
            'wfs_getfeature_' + requestid + '.txt',
            f"request {query_string} failed.\n Query: {request}",
            header, body
        )

    def test_getfeature_invalid_typename(self):
        project = self.testdata_path + "test_project_wfs.qgs"

        # a single invalid typename
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WFS",
            "REQUEST": "GetFeature",
            "TYPENAME": "invalid"
        }.items())])
        header, body = self._execute_request(qs)

        self.assertIn(b"TypeName 'invalid' could not be found", body)

        # an invalid typename preceded by a valid one
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WFS",
            "REQUEST": "GetFeature",
            "TYPENAME": "testlayer,invalid"
        }.items())])
        header, body = self._execute_request(qs)

        self.assertIn(b"TypeName 'invalid' could not be found", body)

    def test_getfeature(self):

        tests = []
        tests.append(('nobbox', 'GetFeature&TYPENAME=testlayer'))
        tests.append(
            ('startindex2', 'GetFeature&TYPENAME=testlayer&STARTINDEX=2'))
        tests.append(('limit2', 'GetFeature&TYPENAME=testlayer&MAXFEATURES=2'))
        tests.append(
            ('start1_limit1', 'GetFeature&TYPENAME=testlayer&MAXFEATURES=1&STARTINDEX=1'))
        tests.append(
            ('srsname', 'GetFeature&TYPENAME=testlayer&SRSNAME=EPSG:3857'))
        tests.append(('sortby', 'GetFeature&TYPENAME=testlayer&SORTBY=id D'))
        tests.append(('hits', 'GetFeature&TYPENAME=testlayer&RESULTTYPE=hits'))

        for id, req in tests:
            self.wfs_getfeature_compare(id, req)

    def test_getfeature_exp_filter(self):
        # multiple filters
        exp_filter = "EXP_FILTER=\"name\"='one';\"name\"='two'"
        req = f"SRSNAME=EPSG:4326&TYPENAME=testlayer,testlayer&{exp_filter}"
        self.wfs_request_compare(
            "GetFeature", '1.0.0', req, 'wfs_getFeature_exp_filter_2')

    def test_wfs_getcapabilities_100_url(self):
        """Check that URL in GetCapabilities response is complete"""

        # empty url in project
        project = os.path.join(
            self.testdata_path, "test_project_without_urls.qgs")
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WFS",
            "VERSION": "1.0.0",
            "REQUEST": "GetCapabilities"
        }.items())])

        r, h = self._result(self._execute_request(qs))

        for item in str(r).split("\\n"):
            if "onlineResource" in item:
                self.assertEqual("onlineResource=\"?" in item, True)

        # url well defined in query string
        project = os.path.join(
            self.testdata_path, "test_project_without_urls.qgs")
        qs = "https://www.qgis-server.org?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WFS",
            "VERSION": "1.0.0",
            "REQUEST": "GetCapabilities"
        }.items())])

        r, h = self._result(self._execute_request(qs))

        for item in str(r).split("\\n"):
            if "onlineResource" in item:
                self.assertTrue(
                    "onlineResource=\"https://www.qgis-server.org?" in item, True)

        # url well defined in project
        project = os.path.join(
            self.testdata_path, "test_project_with_urls.qgs")
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WFS",
            "VERSION": "1.0.0",
            "REQUEST": "GetCapabilities"
        }.items())])

        r, h = self._result(self._execute_request(qs))

        for item in str(r).split("\\n"):
            if "onlineResource" in item:
                self.assertEqual(
                    "onlineResource=\"my_wfs_advertised_url\"" in item, True)

    def test_wfs_getcapabilities_110_no_data(self):
        """Check that GetCapabilities response is correct if a layer
        does not contain data"""

        project = self.testdata_path + "test_wfs_no_data.qgs"
        self.assertTrue(os.path.exists(project), "Project file not found: " + project)

        query_string = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WFS",
            "VERSION": "1.1.0",
            "REQUEST": "GetCapabilities"
        }.items())])

        header, body = self._execute_request(query_string)

        self.result_compare(
            "wfs_getCapabilities_1_1_0_no_data.txt",
            f"request {query_string} failed.\n Query: GetCapabilities",
            header, body
        )

    def result_compare(self, file_name, error_msg_header, header, body):

        self.assert_headers(header, body)
        response = header + body
        reference_path = self.testdata_path + file_name
        self.store_reference(reference_path, response)
        f = open(reference_path, 'rb')
        expected = f.read()
        f.close()
        response = re.sub(RE_STRIP_UNCHECKABLE, b'', response)
        expected = re.sub(RE_STRIP_UNCHECKABLE, b'', expected)
        self.assertXMLEqual(response, expected, msg=f"{error_msg_header}\n")

    def wfs_getfeature_post_compare(self, requestid, request):

        project = self.testdata_path + "test_project_wfs.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = f'?MAP={urllib.parse.quote(project)}'
        header, body = self._execute_request(
            query_string, requestMethod=QgsServerRequest.PostMethod, data=request.encode('utf-8'))

        self.result_compare(
            f'wfs_getfeature_{requestid}.txt',
            f"GetFeature in POST for '{requestid}' failed.",
            header, body,
        )

    def test_getfeature_post(self):
        tests = []

        template = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:GetFeature service="WFS" version="1.0.0" {} xmlns:wfs="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.1.0/wfs.xsd">
  <wfs:Query typeName="testlayer" xmlns:feature="http://www.qgis.org/gml">
    <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
      <ogc:BBOX>
        <ogc:PropertyName>geometry</ogc:PropertyName>
        <gml:Envelope xmlns:gml="http://www.opengis.net/gml">
          <gml:lowerCorner>8 44</gml:lowerCorner>
          <gml:upperCorner>9 45</gml:upperCorner>
        </gml:Envelope>
      </ogc:BBOX>
    </ogc:Filter>
  </wfs:Query>
</wfs:GetFeature>
"""
        tests.append(('nobbox_post', template.format("")))
        tests.append(('startindex2_post', template.format('startIndex="2"')))
        tests.append(('limit2_post', template.format('maxFeatures="2"')))
        tests.append(('start1_limit1_post', template.format(
            'startIndex="1" maxFeatures="1"')))

        srsTemplate = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:GetFeature service="WFS" version="1.0.0" {} xmlns:wfs="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.1.0/wfs.xsd">
  <wfs:Query typeName="testlayer" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
    <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
      <ogc:BBOX>
        <ogc:PropertyName>geometry</ogc:PropertyName>
        <gml:Envelope xmlns:gml="http://www.opengis.net/gml">
          <gml:lowerCorner>890555.92634619 5465442.18332275</gml:lowerCorner>
          <gml:upperCorner>1001875.41713946 5621521.48619207</gml:upperCorner>
        </gml:Envelope>
      </ogc:BBOX>
    </ogc:Filter>
  </wfs:Query>
</wfs:GetFeature>
"""
        tests.append(('srsname_post', srsTemplate.format("")))

        # Issue https://github.com/qgis/QGIS/issues/36398
        # Check get feature within polygon having srsName=EPSG:4326 (same as the project/layer)
        within4326FilterTemplate = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:GetFeature service="WFS" version="1.0.0" {} xmlns:wfs="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.1.0/wfs.xsd">
  <wfs:Query typeName="testlayer" srsName="EPSG:4326" xmlns:feature="http://www.qgis.org/gml">
    <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
      <Within>
        <PropertyName>geometry</PropertyName>
        <Polygon xmlns="http://www.opengis.net/gml" srsName="EPSG:4326">
          <exterior>
            <LinearRing>
              <posList srsDimension="2">
                8.20344131 44.90137909
                8.20347748 44.90137909
                8.20347748 44.90141005
                8.20344131 44.90141005
                8.20344131 44.90137909
              </posList>
            </LinearRing>
          </exterior>
        </Polygon>
      </Within>
    </ogc:Filter>
  </wfs:Query>
</wfs:GetFeature>
"""
        tests.append(('within4326FilterTemplate_post', within4326FilterTemplate.format("")))

        # Check get feature within polygon having srsName=EPSG:3857 (different from the project/layer)
        # The coordinates are converted from the one in 4326
        within3857FilterTemplate = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:GetFeature service="WFS" version="1.0.0" {} xmlns:wfs="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.1.0/wfs.xsd">
  <wfs:Query typeName="testlayer" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
    <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
      <Within>
        <PropertyName>geometry</PropertyName>
        <Polygon xmlns="http://www.opengis.net/gml" srsName="EPSG:3857">
          <exterior>
            <LinearRing>
              <posList srsDimension="2">
              913202.90938171 5606008.98136456
              913206.93580769 5606008.98136456
              913206.93580769 5606013.84701639
              913202.90938171 5606013.84701639
              913202.90938171 5606008.98136456
              </posList>
            </LinearRing>
          </exterior>
        </Polygon>
      </Within>
    </ogc:Filter>
  </wfs:Query>
</wfs:GetFeature>
"""
        tests.append(('within3857FilterTemplate_post', within3857FilterTemplate.format("")))

        srsTwoLayersTemplate = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:GetFeature service="WFS" version="1.0.0" {} xmlns:wfs="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.1.0/wfs.xsd">
  <wfs:Query typeName="testlayer" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
    <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
      <ogc:BBOX>
        <ogc:PropertyName>geometry</ogc:PropertyName>
        <gml:Envelope xmlns:gml="http://www.opengis.net/gml">
          <gml:lowerCorner>890555.92634619 5465442.18332275</gml:lowerCorner>
          <gml:upperCorner>1001875.41713946 5621521.48619207</gml:upperCorner>
        </gml:Envelope>
      </ogc:BBOX>
    </ogc:Filter>
  </wfs:Query>
  <wfs:Query typeName="testlayer" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
    <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
      <ogc:BBOX>
        <ogc:PropertyName>geometry</ogc:PropertyName>
        <gml:Envelope xmlns:gml="http://www.opengis.net/gml">
          <gml:lowerCorner>890555.92634619 5465442.18332275</gml:lowerCorner>
          <gml:upperCorner>1001875.41713946 5621521.48619207</gml:upperCorner>
        </gml:Envelope>
      </ogc:BBOX>
    </ogc:Filter>
  </wfs:Query>
</wfs:GetFeature>
"""
        tests.append(('srs_two_layers_post', srsTwoLayersTemplate.format("")))

        sortTemplate = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:GetFeature service="WFS" version="1.0.0" {} xmlns:wfs="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.1.0/wfs.xsd">
  <wfs:Query typeName="testlayer" xmlns:feature="http://www.qgis.org/gml">
    <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
      <ogc:BBOX>
        <ogc:PropertyName>geometry</ogc:PropertyName>
        <gml:Envelope xmlns:gml="http://www.opengis.net/gml">
          <gml:lowerCorner>8 44</gml:lowerCorner>
          <gml:upperCorner>9 45</gml:upperCorner>
        </gml:Envelope>
      </ogc:BBOX>
    </ogc:Filter>
    <ogc:SortBy>
      <ogc:SortProperty>
        <ogc:PropertyName>id</ogc:PropertyName>
        <ogc:SortOrder>DESC</ogc:SortOrder>
      </ogc:SortProperty>
    </ogc:SortBy>
  </wfs:Query>
</wfs:GetFeature>
"""
        tests.append(('sortby_post', sortTemplate.format("")))

        andTemplate = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:GetFeature service="WFS" version="1.0.0" {} xmlns:wfs="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.1.0/wfs.xsd">
  <wfs:Query typeName="testlayer" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
    <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
      <ogc:And>
        <ogc:PropertyIsGreaterThan>
          <ogc:PropertyName>id</ogc:PropertyName>
          <ogc:Literal>1</ogc:Literal>
        </ogc:PropertyIsGreaterThan>
        <ogc:PropertyIsLessThan>
          <ogc:PropertyName>id</ogc:PropertyName>
          <ogc:Literal>3</ogc:Literal>
        </ogc:PropertyIsLessThan>
      </ogc:And>
    </ogc:Filter>
  </wfs:Query>
</wfs:GetFeature>
"""
        tests.append(('and_post', andTemplate.format("")))

        andBboxTemplate = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:GetFeature service="WFS" version="1.0.0" {} xmlns:wfs="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.1.0/wfs.xsd">
  <wfs:Query typeName="testlayer" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
    <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
      <ogc:And>
        <ogc:BBOX>
          <ogc:PropertyName>geometry</ogc:PropertyName>
          <gml:Envelope xmlns:gml="http://www.opengis.net/gml">
            <gml:lowerCorner>890555.92634619 5465442.18332275</gml:lowerCorner>
            <gml:upperCorner>1001875.41713946 5621521.48619207</gml:upperCorner>
          </gml:Envelope>
        </ogc:BBOX>
        <ogc:PropertyIsGreaterThan>
          <ogc:PropertyName>id</ogc:PropertyName>
          <ogc:Literal>1</ogc:Literal>
        </ogc:PropertyIsGreaterThan>
        <ogc:PropertyIsLessThan>
          <ogc:PropertyName>id</ogc:PropertyName>
          <ogc:Literal>3</ogc:Literal>
        </ogc:PropertyIsLessThan>
      </ogc:And>
    </ogc:Filter>
  </wfs:Query>
</wfs:GetFeature>
"""
        tests.append(('bbox_inside_and_post', andBboxTemplate.format("")))

        # With namespace
        template = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:GetFeature service="WFS" version="1.0.0" {} xmlns:wfs="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.1.0/wfs.xsd">
  <wfs:Query typeName="feature:testlayer" xmlns:feature="http://www.qgis.org/gml">
    <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
      <ogc:BBOX>
        <ogc:PropertyName>geometry</ogc:PropertyName>
        <gml:Envelope xmlns:gml="http://www.opengis.net/gml">
          <gml:lowerCorner>8 44</gml:lowerCorner>
          <gml:upperCorner>9 45</gml:upperCorner>
        </gml:Envelope>
      </ogc:BBOX>
    </ogc:Filter>
  </wfs:Query>
</wfs:GetFeature>
"""
        tests.append(('nobbox_post', template.format("")))

        template = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:GetFeature service="WFS" version="1.0.0" {} xmlns:wfs="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.1.0/wfs.xsd">
  <wfs:Query typeName="testlayer" xmlns="http://www.qgis.org/gml">
    <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
      <ogc:BBOX>
        <ogc:PropertyName>geometry</ogc:PropertyName>
        <gml:Envelope xmlns:gml="http://www.opengis.net/gml">
          <gml:lowerCorner>8 44</gml:lowerCorner>
          <gml:upperCorner>9 45</gml:upperCorner>
        </gml:Envelope>
      </ogc:BBOX>
    </ogc:Filter>
  </wfs:Query>
</wfs:GetFeature>
"""
        tests.append(('nobbox_post', template.format("")))

        for id, req in tests:
            self.wfs_getfeature_post_compare(id, req)

    def test_getFeatureBBOX(self):
        """Test with (1.1.0) and without (1.0.0) CRS"""

        # Tests without CRS
        self.wfs_request_compare(
            "GetFeature", '1.0.0', "TYPENAME=testlayer&RESULTTYPE=hits&BBOX=8.20347,44.901471,8.2035354,44.901493", 'wfs_getFeature_1_0_0_bbox_1_feature')
        self.wfs_request_compare(
            "GetFeature", '1.0.0', "TYPENAME=testlayer&RESULTTYPE=hits&BBOX=8.203127,44.9012765,8.204138,44.901632", 'wfs_getFeature_1_0_0_bbox_3_feature')

        # Tests with CRS
        self.wfs_request_compare(
            "GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&RESULTTYPE=hits&BBOX=8.20347,44.901471,8.2035354,44.901493,EPSG:4326", 'wfs_getFeature_1_0_0_epsgbbox_1_feature')
        self.wfs_request_compare(
            "GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&RESULTTYPE=hits&BBOX=8.203127,44.9012765,8.204138,44.901632,EPSG:4326", 'wfs_getFeature_1_0_0_epsgbbox_3_feature')
        self.wfs_request_compare(
            "GetFeature", '1.1.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&RESULTTYPE=hits&BBOX=8.20347,44.901471,8.2035354,44.901493,EPSG:4326", 'wfs_getFeature_1_1_0_epsgbbox_1_feature')
        self.wfs_request_compare(
            "GetFeature", '1.1.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&RESULTTYPE=hits&BBOX=8.203127,44.9012765,8.204138,44.901632,EPSG:4326", 'wfs_getFeature_1_1_0_epsgbbox_3_feature')

        self.wfs_request_compare("GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&RESULTTYPE=hits&BBOX=913144,5605992,913303,5606048,EPSG:3857",
                                 'wfs_getFeature_1_0_0_epsgbbox_3_feature_3857')
        self.wfs_request_compare("GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&RESULTTYPE=hits&BBOX=913206,5606024,913213,5606026,EPSG:3857",
                                 'wfs_getFeature_1_0_0_epsgbbox_1_feature_3857')
        self.wfs_request_compare("GetFeature", '1.1.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&RESULTTYPE=hits&BBOX=913144,5605992,913303,5606048,EPSG:3857",
                                 'wfs_getFeature_1_1_0_epsgbbox_3_feature_3857')
        self.wfs_request_compare("GetFeature", '1.1.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&RESULTTYPE=hits&BBOX=913206,5606024,913213,5606026,EPSG:3857",
                                 'wfs_getFeature_1_1_0_epsgbbox_1_feature_3857')

        self.wfs_request_compare("GetFeature", '1.0.0', "SRSNAME=EPSG:3857&TYPENAME=testlayer&RESULTTYPE=hits&BBOX=913144,5605992,913303,5606048,EPSG:3857",
                                 'wfs_getFeature_1_0_0_epsgbbox_3_feature_3857')
        self.wfs_request_compare("GetFeature", '1.0.0', "SRSNAME=EPSG:3857&TYPENAME=testlayer&RESULTTYPE=hits&BBOX=913206,5606024,913213,5606026,EPSG:3857",
                                 'wfs_getFeature_1_0_0_epsgbbox_1_feature_3857')
        self.wfs_request_compare("GetFeature", '1.1.0', "SRSNAME=EPSG:3857&TYPENAME=testlayer&RESULTTYPE=hits&BBOX=913144,5605992,913303,5606048,EPSG:3857",
                                 'wfs_getFeature_1_1_0_epsgbbox_3_feature_3857')
        self.wfs_request_compare("GetFeature", '1.1.0', "SRSNAME=EPSG:3857&TYPENAME=testlayer&RESULTTYPE=hits&BBOX=913206,5606024,913213,5606026,EPSG:3857",
                                 'wfs_getFeature_1_1_0_epsgbbox_1_feature_3857')

    def test_getFeatureFeatureId(self):
        """Test GetFeature with featureid"""

        self.wfs_request_compare(
            "GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&FEATUREID=testlayer.0", 'wfs_getFeature_1_0_0_featureid_0')

        # with multiple feature ids
        self.wfs_request_compare(
            "GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&FEATUREID=testlayer.0,testlayer.2", 'wfs_getFeature_1_0_0_featureid_02')

        # with layer name Testing Layer (copy)
        self.wfs_request_compare(
            "GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=Testing_Layer_(copy)&FEATUREID=Testing_Layer_(copy).0", 'wfs_getFeature_1_0_0_featureid_0_testing')

        # with propertynanme *
        self.wfs_request_compare(
            "GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&FEATUREID=testlayer.0&PROPERTYNAME=*", 'wfs_getFeature_1_0_0_featureid_0')

    def test_getFeatureFeature11urn(self):
        """Test GetFeature with SRSNAME urn:ogc:def:crs:EPSG::4326"""

        self.wfs_request_compare(
            "GetFeature", '1.1.0', "SRSNAME=urn:ogc:def:crs:EPSG::4326&TYPENAME=testlayer&FEATUREID=testlayer.0", 'wfs_getFeature_1_1_0_featureid_0_1_1_0')

    def test_get_feature_srsname_empty(self):
        """Test GetFeature with an empty SRSNAME."""
        self.wfs_request_compare(
            "GetFeature", '1.1.0', "TYPENAME=testlayer&FEATUREID=testlayer.0", 'wfs_getFeature_1_1_0_featureid_0_1_1_0_srsname')

    def test_get_feature_wrong_version_nomber(self):
        """Test GetFeature with a wrong version number.
           This should fall back to the default version: 1.1.0
        """
        self.wfs_request_compare(
            "GetFeature", '2.0.0', "SRSNAME=urn:ogc:def:crs:EPSG::4326&TYPENAME=testlayer&FEATUREID=testlayer.0", 'wfs_getFeature_1_1_0_featureid_0_1_1_0')

        self.wfs_request_compare(
            "GetFeature", '2.0.0', "TYPENAME=testlayer&FEATUREID=testlayer.0", 'wfs_getFeature_1_1_0_featureid_0_1_1_0_srsname')

    def test_getFeature_EXP_FILTER_regression_20927(self):
        """Test expressions with EXP_FILTER"""

        self.wfs_request_compare(
            "GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&FEATUREID=testlayer.0&EXP_FILTER=\"name\"='one'", 'wfs_getFeature_1_0_0_EXP_FILTER_FID_one')
        # Note that FEATUREID takes precedence over EXP_FILTER and the filter is completely ignored when FEATUREID is set
        self.wfs_request_compare(
            "GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&FEATUREID=testlayer.0&EXP_FILTER=\"name\"='two'", 'wfs_getFeature_1_0_0_EXP_FILTER_FID_one')
        self.wfs_request_compare(
            "GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&EXP_FILTER=\"name\"='two'", 'wfs_getFeature_1_0_0_EXP_FILTER_two')
        self.wfs_request_compare(
            "GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&EXP_FILTER=\"name\"=concat('tw', 'o')", 'wfs_getFeature_1_0_0_EXP_FILTER_two')
        # Syntax ok but function does not exist
        self.wfs_request_compare("GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&EXP_FILTER=\"name\"=invalid_expression('tw', 'o')",
                                 'wfs_getFeature_1_0_0_EXP_FILTER_invalid_expression')
        # Syntax error in exp
        self.wfs_request_compare("GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&EXP_FILTER=\"name\"=concat('tw, 'o')",
                                 'wfs_getFeature_1_0_0_EXP_FILTER_syntax_error')
        # BBOX gml expressions
        self.wfs_request_compare("GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&EXP_FILTER=intersects($geometry, geom_from_gml('<gml:Box> <gml:coordinates cs=\",\" ts=\" \">8.20344750430995617,44.9013881888184514 8.20347909100379269,44.90140004005827024</gml:coordinates></gml:Box>'))", 'wfs_getFeature_1_0_0_EXP_FILTER_gml_bbox_three')
        self.wfs_request_compare("GetFeature", '1.0.0', "SRSNAME=EPSG:4326&TYPENAME=testlayer&EXP_FILTER=intersects($geometry, geom_from_gml('<gml:Box> <gml:coordinates cs=\",\" ts=\" \">8.20348458304175665,44.90147459621791626 8.20351616973559317,44.9014864474577351</gml:coordinates></gml:Box>'))", 'wfs_getFeature_1_0_0_EXP_FILTER_gml_bbox_one')

    def test_describeFeatureType(self):
        """Test DescribeFeatureType with TYPENAME filters"""

        project_file = "test_project_wms_grouped_layers.qgs"
        self.wfs_request_compare("DescribeFeatureType", '1.0.0', "TYPENAME=as_areas&",
                                 'wfs_describeFeatureType_1_0_0_typename_as_areas', project_file=project_file)
        self.wfs_request_compare("DescribeFeatureType", '1.0.0', "TYPENAME=as_areas&OUTPUTFORMAT=XMLSCHEMA&",
                                 'wfs_describeFeatureType_1_0_0_typename_as_areas', project_file=project_file)

        self.wfs_request_compare("DescribeFeatureType", '1.1.0', "TYPENAME=as_areas&",
                                 'wfs_describeFeatureType_1_1_0_typename_as_areas', project_file=project_file)
        self.wfs_request_compare("DescribeFeatureType", '1.1.0', "TYPENAME=as_areas&OUTPUTFORMAT=XMLSCHEMA&",
                                 'wfs_describeFeatureType_1_1_0_typename_as_areas', project_file=project_file)
        self.wfs_request_compare("DescribeFeatureType", '1.1.0', "TYPENAME=as_areas&OUTPUTFORMAT=text/xml; subtype=gml/3.1.1&",
                                 'wfs_describeFeatureType_1_1_0_typename_as_areas', project_file=project_file)

        self.wfs_request_compare("DescribeFeatureType", '1.0.0', "",
                                 'wfs_describeFeatureType_1_0_0_typename_empty', project_file=project_file)
        self.wfs_request_compare("DescribeFeatureType", '1.1.0', "",
                                 'wfs_describeFeatureType_1_1_0_typename_empty', project_file=project_file)

        self.wfs_request_compare("DescribeFeatureType", '1.0.0', "TYPENAME=does_not_exist&",
                                 'wfs_describeFeatureType_1_0_0_typename_wrong', project_file=project_file)
        self.wfs_request_compare("DescribeFeatureType", '1.1.0', "TYPENAME=does_not_exist&",
                                 'wfs_describeFeatureType_1_1_0_typename_wrong', project_file=project_file)

    def test_describeFeatureTypeGeoJson(self):
        """Test DescribeFeatureType with GeoJSON format with TYPENAME filters
        """
        project_file = "test_project_wms_grouped_layers.qgs"
        self.wfs_request_compare("DescribeFeatureType", '1.1.0', "TYPENAME=as_areas&OUTPUTFORMAT=GEOJSON",
                                 'wfs_describeFeatureType_1_1_0_typename_as_areas_geojson', project_file=project_file)

    def test_GetFeature_with_cdata(self):
        """ Test GetFeature with CDATA."""
        self.wfs_request_compare(
            "GetFeature",
            "1.0.0",
            "TYPENAME=test_layer_wfs_cdata_lines&",
            'wfs_getfeature_cdata',
            project_file="test_layer_wfs_cdata.qgs")

    def test_describeFeatureTypeVirtualFields(self):
        """Test DescribeFeatureType with virtual fields: bug GH-29767"""

        project_file = "bug_gh29767_double_vfield.qgs"
        self.wfs_request_compare("DescribeFeatureType", '1.1.0', "",
                                 'wfs_describeFeatureType_1_1_0_virtual_fields', project_file=project_file)

    def test_getFeatureFeature_0_nulls(self):
        """Test that 0 and null in integer columns are reported correctly"""

        # Test transactions with 0 and nulls

        post_data = """<?xml version="1.0" ?>
<wfs:Transaction service="WFS" version="{version}"
  xmlns:ogc="http://www.opengis.net/ogc"
  xmlns:wfs="http://www.opengis.net/wfs"
  xmlns:gml="http://www.opengis.net/gml">
   <wfs:Update typeName="cdb_lines">
      <wfs:Property>
         <wfs:Name>{field}</wfs:Name>
         <wfs:Value>{value}</wfs:Value>
      </wfs:Property>
      <fes:Filter>
         <fes:FeatureId fid="cdb_lines.22"/>
      </fes:Filter>
   </wfs:Update>
</wfs:Transaction>
            """

        def _round_trip(value, field, version='1.1.0'):
            """Set a value on fid 22 and field and check it back"""

            encoded_data = post_data.format(field=field, value=value, version=version).encode('utf8')
            # Strip the field if NULL
            if value is None:
                encoded_data = encoded_data.replace(b'<wfs:Value>None</wfs:Value>', b'')

            header, body = self._execute_request("?MAP={}&SERVICE=WFS&VERSION={}".format(
                self.testdata_path + 'test_project_wms_grouped_layers.qgs', version), QgsServerRequest.PostMethod, encoded_data)
            if version == '1.0.0':
                self.assertIn(b'<SUCCESS/>', body)
            else:
                self.assertIn(b'<totalUpdated>1</totalUpdated>', body)
            header, body = self._execute_request("?MAP=%s&SERVICE=WFS&REQUEST=GetFeature&TYPENAME=cdb_lines&FEATUREID=cdb_lines.22" % (
                self.testdata_path + 'test_project_wms_grouped_layers.qgs'))
            if value is not None:
                xml_value = '<qgs:{0}>{1}</qgs:{0}>'.format(field, value).encode('utf8')
                self.assertTrue(xml_value in body, f"{xml_value} not found in body")
            else:
                xml_value = f'<qgs:{field}>'.encode()
                self.assertNotIn(xml_value, body)
            # Check the backend
            vl = QgsVectorLayer(
                self.testdata_path + 'test_project_wms_grouped_layers.gpkg|layername=cdb_lines', 'vl', 'ogr')
            self.assertTrue(vl.isValid())
            self.assertEqual(
                str(vl.getFeature(22)[field]), value if value is not None else 'NULL')

        for version in ('1.0.0', '1.1.0'):
            _round_trip('0', 'id_long', version)
            _round_trip('12345', 'id_long', version)
            _round_trip('0', 'id', version)
            _round_trip('12345', 'id', version)
            _round_trip(None, 'id', version)
            _round_trip(None, 'id_long', version)

            # "name" is NOT NULL: try to set it to empty string
            _round_trip('', 'name', version)
            # Then NULL
            data = post_data.format(field='name', value='', version=version).encode('utf8')
            encoded_data = data.replace(b'<wfs:Value></wfs:Value>', b'')
            header, body = self._execute_request("?MAP=%s&SERVICE=WFS" % (
                self.testdata_path + 'test_project_wms_grouped_layers.qgs'), QgsServerRequest.PostMethod, encoded_data)
            if version == '1.0.0':
                self.assertIn(b'<ERROR/>', body)
            else:
                self.assertTrue(b'<totalUpdated>0</totalUpdated>' in body)
            self.assertIn(b'<Message>NOT NULL constraint error on layer \'cdb_lines\', field \'name\'</Message>', body)

    def test_describeFeatureTypeGeometryless(self):
        """Test DescribeFeatureType with geometryless tables - bug GH-30381"""

        project_file = "test_project_geometryless_gh30381.qgs"
        self.wfs_request_compare("DescribeFeatureType", '1.1.0',
                                 reference_base_name='wfs_describeFeatureType_1_1_0_geometryless',
                                 project_file=project_file)

    def test_getFeatureFeatureIdJson(self):
        """Test GetFeature with featureid JSON format and various content types"""

        for ct in ('GeoJSON', 'application/vnd.geo+json', 'application/json', 'application/geo+json'):
            self.wfs_request_compare(
                "GetFeature",
                '1.0.0',
                f"OUTPUTFORMAT={ct}"
                + "&SRSNAME=EPSG:4326&TYPENAME=testlayer&FEATUREID=testlayer.0",
                'wfs_getFeature_1_0_0_featureid_0_json')

    def test_getFeatureFeatureJsonCrs(self):
        """Test issue GH #25171, geojson srsName"""

        project = QgsProject()
        layer = QgsVectorLayer("Point?crs=epsg:3857&field=fldint:integer",
                               "layer", "memory")
        project.addMapLayers([layer])
        project.writeEntry("WFSLayers", "/", [layer.id()])
        f = QgsFeature(layer.fields())

        f.setGeometry(QgsGeometry.fromWkt('point(807305 5592878)'))
        f.setAttributes([123])
        layer.dataProvider().addFeatures([f])
        f = QgsFeature(layer.fields())
        f.setGeometry(QgsGeometry.fromWkt('point(812191 5589555)'))
        f.setAttributes([123])
        layer.dataProvider().addFeatures([f])

        query_string = "?" + "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WFS",
            "REQUEST": "GetFeature",
            "VERSION": "1.1.0",
            "TYPENAME": "layer",
            "SRSNAME": "EPSG:3857",
            "outputFormat": "GeoJSON"
        }.items())])

        header, body = self._execute_request_project(query_string, project)
        json.loads(body)
        jdata = json.loads(body)
        jdata['features'][0]['geometry']
        jdata['features'][0]['geometry']['coordinates']
        self.assertEqual(jdata['features'][0]['geometry']['coordinates'], [807305, 5592878])
        self.assertEqual(jdata['crs']['properties']['name'], "urn:ogc:def:crs:EPSG:0:3857")

        query_string = "?" + "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WFS",
            "REQUEST": "GetFeature",
            "VERSION": "1.1.0",
            "TYPENAME": "layer",
            "SRSNAME": "EPSG:4326",
            "outputFormat": "GeoJSON"
        }.items())])

        header, body = self._execute_request_project(query_string, project)
        json.loads(body)
        jdata = json.loads(body)
        jdata['features'][0]['geometry']
        jdata['features'][0]['geometry']['coordinates']
        self.assertEqual([int(i) for i in jdata['features'][0]['geometry']['coordinates']], [7, 44])
        self.assertFalse('crs' in jdata)

        query_string = "?" + "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WFS",
            "REQUEST": "GetFeature",
            "VERSION": "1.1.0",
            "TYPENAME": "layer",
            "outputFormat": "GeoJSON"
        }.items())])

        header, body = self._execute_request_project(query_string, project)
        json.loads(body)
        jdata = json.loads(body)
        jdata['features'][0]['geometry']
        jdata['features'][0]['geometry']['coordinates']
        self.assertEqual([int(i) for i in jdata['features'][0]['geometry']['coordinates']], [7, 44])

        query_string = "?" + "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WFS",
            "REQUEST": "GetFeature",
            "VERSION": "1.1.0",
            "TYPENAME": "layer",
            "SRSNAME": "EPSG:32632",
            "outputFormat": "GeoJSON"
        }.items())])

        header, body = self._execute_request_project(query_string, project)
        json.loads(body)
        jdata = json.loads(body)
        jdata['features'][0]['geometry']
        jdata['features'][0]['geometry']['coordinates']
        self.assertEqual([int(i) for i in jdata['features'][0]['geometry']['coordinates']], [361806, 4964192])
        self.assertEqual(jdata['crs']['properties']['name'], "urn:ogc:def:crs:EPSG:0:32632")

        query_string = "?" + "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WFS",
            "REQUEST": "GetFeature",
            "VERSION": "1.1.0",
            "TYPENAME": "layer",
            "SRSNAME": "EPSG:3857",
            "outputFormat": "GeoJSON",
            "FEATUREID": "layer.2"
        }.items())])

        header, body = self._execute_request_project(query_string, project)
        json.loads(body)
        jdata = json.loads(body)
        jdata['features'][0]['geometry']
        jdata['features'][0]['geometry']['coordinates']
        self.assertEqual([int(i) for i in jdata['features'][0]['geometry']['coordinates']], [812191, 5589555])
        self.assertEqual(jdata['crs']['properties']['name'], "urn:ogc:def:crs:EPSG:0:3857")

    def test_insert_srsName(self):
        """Test srsName is respected when insering"""

        post_data = """
        <Transaction xmlns="http://www.opengis.net/wfs" xsi:schemaLocation="http://www.qgis.org/gml http://localhost:8000/?SERVICE=WFS&amp;REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=as_symbols" service="WFS" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="{version}" xmlns:gml="http://www.opengis.net/gml">
            <Insert xmlns="http://www.opengis.net/wfs">
                <as_symbols xmlns="http://www.qgis.org/gml">
                <name xmlns="http://www.qgis.org/gml">{name}</name>
                <geometry xmlns="http://www.qgis.org/gml">
                    <gml:Point srsName="{srsName}">
                    <gml:coordinates cs="," ts=" ">{coordinates}</gml:coordinates>
                    </gml:Point>
                </geometry>
                </as_symbols>
            </Insert>
        </Transaction>
        """

        project = self.testdata_path + \
            "test_project_wms_grouped_layers.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = f'?SERVICE=WFS&MAP={urllib.parse.quote(project)}'
        request = post_data.format(
            name='4326-test1',
            version='1.1.0',
            srsName='EPSG:4326',
            coordinates='10.67,52.48'
        )
        header, body = self._execute_request(
            query_string, requestMethod=QgsServerRequest.PostMethod, data=request.encode('utf-8'))

        # Verify
        vl = QgsVectorLayer(self.testdata_path + 'test_project_wms_grouped_layers.gpkg|layername=as_symbols', 'as_symbols')
        self.assertTrue(vl.isValid())
        feature = next(vl.getFeatures(QgsFeatureRequest(QgsExpression('"name" = \'4326-test1\''))))
        geom = feature.geometry()

        tr = QgsCoordinateTransform(QgsCoordinateReferenceSystem.fromEpsgId(4326), vl.crs(), QgsCoordinateTransformContext())

        geom_4326 = QgsGeometry.fromWkt('point( 10.67 52.48)')
        geom_4326.transform(tr)
        self.assertEqual(geom.asWkt(0), geom_4326.asWkt(0))

        # Now: insert a feature in layer's CRS
        request = post_data.format(
            name='25832-test1',
            version='1.1.0',
            srsName='EPSG:25832',
            coordinates='613412,5815738'
        )
        header, body = self._execute_request(
            query_string, requestMethod=QgsServerRequest.PostMethod, data=request.encode('utf-8'))

        feature = next(vl.getFeatures(QgsFeatureRequest(QgsExpression('"name" = \'25832-test1\''))))
        geom = feature.geometry()
        self.assertEqual(geom.asWkt(0), geom_4326.asWkt(0))

        # Tests for inverted axis issue GH #36584
        # Cleanup
        self.assertTrue(vl.startEditing())
        vl.selectByExpression('"name" LIKE \'4326-test%\'')
        vl.deleteSelectedFeatures()
        self.assertTrue(vl.commitChanges())

        self.i = 0

        def _test(version, srsName, lat_lon=False):
            self.i += 1
            name = f'4326-test_{self.i}'
            request = post_data.format(
                name=name,
                version=version,
                srsName=srsName,
                coordinates='52.48,10.67' if lat_lon else '10.67,52.48'
            )
            header, body = self._execute_request(
                query_string, requestMethod=QgsServerRequest.PostMethod, data=request.encode('utf-8'))
            feature = next(vl.getFeatures(QgsFeatureRequest(QgsExpression(f'"name" = \'{name}\''))))
            geom = feature.geometry()
            self.assertEqual(geom.asWkt(0), geom_4326.asWkt(0), f"Transaction Failed: {version} , {srsName}, lat_lon={lat_lon}")

        _test('1.1.0', 'urn:ogc:def:crs:EPSG::4326', lat_lon=True)
        _test('1.1.0', 'http://www.opengis.net/def/crs/EPSG/0/4326', lat_lon=True)
        _test('1.1.0', 'http://www.opengis.net/gml/srs/epsg.xml#4326', lat_lon=False)
        _test('1.1.0', 'EPSG:4326', lat_lon=False)

        _test('1.0.0', 'urn:ogc:def:crs:EPSG::4326', lat_lon=True)
        _test('1.0.0', 'http://www.opengis.net/def/crs/EPSG/0/4326', lat_lon=True)
        _test('1.0.0', 'http://www.opengis.net/gml/srs/epsg.xml#4326', lat_lon=False)
        _test('1.0.0', 'EPSG:4326', lat_lon=False)

        def _test_getFeature(version, srsName, lat_lon=False):
            # Now get the feature through WFS using BBOX filter
            bbox = QgsGeometry.fromWkt('point( 10.7006 52.4317)').boundingBox()
            bbox.grow(0.0001)
            bbox_text = "%s,%s,%s,%s" % ((bbox.yMinimum(), bbox.xMinimum(), bbox.yMaximum(), bbox.xMaximum()) if lat_lon else (bbox.xMinimum(), bbox.yMinimum(), bbox.xMaximum(), bbox.yMaximum()))
            req = query_string + '&REQUEST=GetFeature&VERSION={version}&TYPENAME=as_symbols&SRSNAME={srsName}&BBOX={bbox},{srsName}'.format(version=version, srsName=srsName, bbox=bbox_text)
            header, body = self._execute_request(req)
            self.assertTrue(b'gid>7' in body, f"GetFeature Failed: {version} , {srsName}, lat_lon={lat_lon}")

        _test_getFeature('1.1.0', 'urn:ogc:def:crs:EPSG::4326', lat_lon=True)
        _test_getFeature('1.1.0', 'EPSG:4326', lat_lon=False)

        _test_getFeature('1.0.0', 'urn:ogc:def:crs:EPSG::4326', lat_lon=True)
        _test_getFeature('1.0.0', 'EPSG:4326', lat_lon=False)

        # Cleanup
        self.assertTrue(vl.startEditing())
        vl.selectByExpression('"name" LIKE \'4326-test%\'')
        vl.deleteSelectedFeatures()
        self.assertTrue(vl.commitChanges())

    def test_getFeatureFeatureEnvelopeCrs(self):
        """Test issue GH #48642"""

        project = QgsProject()
        layer = QgsVectorLayer("Point?crs=epsg:3857&field=fldint:integer",
                               "layer", "memory")
        project.addMapLayers([layer])
        project.writeEntry("WFSLayers", "/", [layer.id()])
        f = QgsFeature(layer.fields())

        f.setGeometry(QgsGeometry.fromWkt('point(807305 5592878)'))
        f.setAttributes([123])
        layer.dataProvider().addFeatures([f])
        f = QgsFeature(layer.fields())
        f.setGeometry(QgsGeometry.fromWkt('point(812191 5589555)'))
        f.setAttributes([123])
        layer.dataProvider().addFeatures([f])

        query_string = "?" + "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WFS",
            "REQUEST": "GetFeature",
            "VERSION": "1.1.0",
            "TYPENAME": "layer",
            "SRSNAME": "EPSG:4326"
        }.items())])

        header, body = self._execute_request_project(query_string, project)
        root = et.fromstring(body)
        e = root.findall('.//gml:Envelope', root.nsmap)[0]
        self.assertEqual(e.attrib, {'srsName': 'EPSG:4326'})

        self.assertEqual([c[:4] for c in e.findall('.//')[0].text.split(' ')], ['7.25', '44.7'])
        self.assertEqual([c[:4] for c in e.findall('.//')[1].text.split(' ')], ['7.29', '44.8'])

        query_string = "?" + "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WFS",
            "REQUEST": "GetFeature",
            "VERSION": "1.1.0",
            "TYPENAME": "layer",
            "SRSNAME": "EPSG:4326",
            "BBOX": "7.2,44.5,8.2,45.1,EPSG:4326"
        }.items())])

        header, body = self._execute_request_project(query_string, project)
        root = et.fromstring(body)
        e = root.findall('.//gml:Envelope', root.nsmap)[0]
        self.assertEqual(e.attrib, {'srsName': 'EPSG:4326'})
        self.assertEqual([c[:4] for c in e.findall('.//')[0].text.split(' ')], ['7.2', '44.5'])
        self.assertEqual([c[:4] for c in e.findall('.//')[1].text.split(' ')], ['8.2', '45.1'])

        query_string = "?" + "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WFS",
            "REQUEST": "GetFeature",
            "VERSION": "1.1.0",
            "TYPENAME": "layer",
            "SRSNAME": "EPSG:4326",
            "BBOX": "807305,5589555,812191,5592878,EPSG:3857"
        }.items())])

        header, body = self._execute_request_project(query_string, project)
        root = et.fromstring(body)
        e = root.findall('.//gml:Envelope', root.nsmap)[0]
        self.assertEqual(e.attrib, {'srsName': 'EPSG:4326'})
        self.assertEqual([c[:4] for c in e.findall('.//')[0].text.split(' ')], ['7.25', '44.7'])
        self.assertEqual([c[:4] for c in e.findall('.//')[1].text.split(' ')], ['7.29', '44.8'])

    def test_describeFeatureTypeDateTime(self):
        """Test DescribeFeatureType with dateTime fields"""
        project_file = "../qgis_server_accesscontrol/project_with_dimensions.qgs"
        self.wfs_request_compare("DescribeFeatureType", '1.0.0', "TYPENAME=Datetime_dim&",
                                 'wfs_describeFeatureType_1_0_0_typename_datetime_dim', project_file=project_file)
        self.wfs_request_compare("DescribeFeatureType", '1.1.0', "TYPENAME=Datetime_dim&",
                                 'wfs_describeFeatureType_1_1_0_typename_datetime_dim', project_file=project_file)

        self.wfs_request_compare("DescribeFeatureType", '1.1.0', "TYPENAME=datetime_str&",
                                 'wfs_describeFeatureType_1_1_0_typename_datetime_str', project_file=project_file)

    def test_GetFeature_with_datetime(self):
        """ Test GetFeature with date-time data"""
        project_file = "../qgis_server_accesscontrol/project_with_dimensions.qgs"

        self.wfs_request_compare(
            "GetFeature",
            "1.0.0",
            "TYPENAME=datetime_str&",
            'wfs_getfeature_datetime_str',
            project_file=project_file)

        self.wfs_request_compare(
            "GetFeature",
            "1.0.0",
            "TYPENAME=Datetime_dim&",
            'wfs_getfeature_datetime',
            project_file=project_file)


if __name__ == '__main__':
    unittest.main()
