# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer.

FIXME: keep here only generic server tests and move specific services
       tests to test_qgsserver_<service>.py

       Already moved services and functionality:
       - WMS
       - plugins
       - settings
       - WFS-T integration test

       TODO:
       - WFS
       - WCS

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

# Deterministic XML
os.environ['QT_HASH_SEED'] = '1'

import re
import urllib.request
import urllib.parse
import urllib.error
import email

from io import StringIO
from qgis.server import QgsServer, QgsServerRequest, QgsBufferServerRequest, QgsBufferServerResponse
from qgis.core import QgsRenderChecker, QgsApplication
from qgis.testing import unittest
from qgis.PyQt.QtCore import QSize
from utilities import unitTestDataPath

import osgeo.gdal  # NOQA
import tempfile
import base64


# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = b'MAP=[^"]+|Content-Length: \d+'
RE_ATTRIBUTES = b'[^>\s]+=[^>\s]+'


class QgsServerTestBase(unittest.TestCase):
    """Base class for QGIS server tests"""

    # Set to True in child classes to re-generate reference files for this class
    regenerate_reference = False

    def assertXMLEqual(self, response, expected, msg=''):
        """Compare XML line by line and sorted attributes"""
        response_lines = response.splitlines()
        expected_lines = expected.splitlines()
        line_no = 1
        for expected_line in expected_lines:
            expected_line = expected_line.strip()
            response_line = response_lines[line_no - 1].strip()
            # Compare tag
            try:
                self.assertEqual(re.findall(b'<([^>\s]+)[ >]', expected_line)[0],
                                 re.findall(b'<([^>\s]+)[ >]', response_line)[0], msg=msg + "\nTag mismatch on line %s: %s != %s" % (line_no, expected_line, response_line))
            except IndexError:
                self.assertEqual(expected_line, response_line, msg=msg + "\nTag line mismatch %s: %s != %s\n%s" % (line_no, expected_line, response_line, msg))
            # print("---->%s\t%s == %s" % (line_no, expected_line, response_line))
            # Compare attributes
            if re.match(RE_ATTRIBUTES, expected_line):  # has attrs
                expected_attrs = sorted(re.findall(RE_ATTRIBUTES, expected_line))
                response_attrs = sorted(re.findall(RE_ATTRIBUTES, response_line))
                self.assertEqual(expected_attrs, response_attrs, msg=msg + "\nXML attributes differ at line {0}: {1} != {2}".format(line_no, expected_attrs, response_attrs))
            line_no += 1

    @classmethod
    def setUpClass(cls):
        cls.app = QgsApplication([], False)

    @classmethod
    def tearDownClass(cls):
        cls.app.exitQgis()

    def setUp(self):
        """Create the server instance"""
        self.testdata_path = unitTestDataPath('qgis_server') + '/'

        d = unitTestDataPath('qgis_server_accesscontrol') + '/'
        self.projectPath = os.path.join(d, "project.qgs")

        # Clean env just to be sure
        env_vars = ['QUERY_STRING', 'QGIS_PROJECT_FILE']
        for ev in env_vars:
            try:
                del os.environ[ev]
            except KeyError:
                pass
        self.server = QgsServer()

    def strip_version_xmlns(self, text):
        """Order of attributes is random, strip version and xmlns"""
        return text.replace(b'version="1.3.0"', b'').replace(b'xmlns="http://www.opengis.net/ogc"', b'')

    def assert_headers(self, header, body):
        stream = StringIO()
        header_string = header.decode('utf-8')
        stream.write(header_string)
        headers = email.message_from_string(header_string)
        if 'content-length' in headers:
            content_length = int(headers['content-length'])
            body_length = len(body)
            self.assertEqual(content_length, body_length, msg="Header reported content-length: %d Actual body length was: %d" % (content_length, body_length))

    @classmethod
    def store_reference(self, reference_path, response):
        """Utility to store reference files"""

        # Normally this is false
        if not self.regenerate_reference:
            return

        # Store the output for debug or to regenerate the reference documents:
        f = open(reference_path, 'wb+')
        f.write(response)
        f.close()

    def _result(self, data):
        headers = {}
        for line in data[0].decode('UTF-8').split("\n"):
            if line != "":
                header = line.split(":")
                self.assertEqual(len(header), 2, line)
                headers[str(header[0])] = str(header[1]).strip()

        return data[1], headers

    def _img_diff(self, image, control_image, max_diff, max_size_diff=QSize()):
        temp_image = os.path.join(tempfile.gettempdir(), "%s_result.png" % control_image)

        with open(temp_image, "wb") as f:
            f.write(image)

        control = QgsRenderChecker()
        control.setControlPathPrefix("qgis_server")
        control.setControlName(control_image)
        control.setRenderedImage(temp_image)
        if max_size_diff.isValid():
            control.setSizeTolerance(max_size_diff.width(), max_size_diff.height())
        return control.compareImages(control_image), control.report()

    def _img_diff_error(self, response, headers, image, max_diff=10, max_size_diff=QSize()):
        self.assertEqual(
            headers.get("Content-Type"), "image/png",
            "Content type is wrong: %s" % headers.get("Content-Type"))

        test, report = self._img_diff(response, image, max_diff, max_size_diff)

        with open(os.path.join(tempfile.gettempdir(), image + "_result.png"), "rb") as rendered_file:
            encoded_rendered_file = base64.b64encode(rendered_file.read())
            message = "Image is wrong\n%s\nImage:\necho '%s' | base64 -d >%s/%s_result.png" % (
                report, encoded_rendered_file.strip().decode('utf8'), tempfile.gettempdir(), image
            )

        # If the failure is in image sizes the diff file will not exists.
        if os.path.exists(os.path.join(tempfile.gettempdir(), image + "_result_diff.png")):
            with open(os.path.join(tempfile.gettempdir(), image + "_result_diff.png"), "rb") as diff_file:
                encoded_diff_file = base64.b64encode(diff_file.read())
                message += "\nDiff:\necho '%s' | base64 -d > %s/%s_result_diff.png" % (
                    encoded_diff_file.strip().decode('utf8'), tempfile.gettempdir(), image
                )

        self.assertTrue(test, message)


class TestQgsServer(QgsServerTestBase):
    """Tests container"""

    # Set to True to re-generate reference files for this class
    regenerate_reference = False

    def test_destructor_segfaults(self):
        """Segfault on destructor?"""
        server = QgsServer()
        del server

    def test_multiple_servers(self):
        """Segfaults?"""
        for i in range(10):
            locals()["s%s" % i] = QgsServer()
            locals()["s%s" % i].handleRequest("")

    def test_requestHandler(self):
        """Test request handler"""
        headers = {'header-key-1': 'header-value-1', 'header-key-2': 'header-value-2'}
        request = QgsBufferServerRequest('http://somesite.com/somepath', QgsServerRequest.GetMethod, headers)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response)
        self.assertEqual(bytes(response.body()), b'<ServerException>Project file error</ServerException>\n')
        self.assertEqual(response.headers(), {'Content-Length': '54', 'Content-Type': 'text/xml; charset=utf-8'})
        self.assertEqual(response.statusCode(), 500)

    def test_api(self):
        """Using an empty query string (returns an XML exception)
        we are going to test if headers and body are returned correctly"""
        # Test as a whole
        header, body = [_v for _v in self.server.handleRequest("")]
        response = self.strip_version_xmlns(header + body)
        expected = self.strip_version_xmlns(b'Content-Length: 54\nContent-Type: text/xml; charset=utf-8\n\n<ServerException>Project file error</ServerException>\n')
        self.assertEqual(response, expected)
        expected = b'Content-Length: 54\nContent-Type: text/xml; charset=utf-8\n\n'
        self.assertEqual(header, expected)

        # Test response when project is specified but without service
        project = self.testdata_path + "test_project_wfs.qgs"
        qs = '?MAP=%s' % (urllib.parse.quote(project))
        header, body = [_v for _v in self.server.handleRequest(qs)]
        response = self.strip_version_xmlns(header + body)
        expected = self.strip_version_xmlns(b'Content-Length: 206\nContent-Type: text/xml; charset=utf-8\n\n<ServiceExceptionReport version="1.3.0" xmlns="http://www.opengis.net/ogc">\n <ServiceException code="Service configuration error">Service unknown or unsupported</ServiceException>\n</ServiceExceptionReport>\n')
        self.assertEqual(response, expected)
        expected = b'Content-Length: 206\nContent-Type: text/xml; charset=utf-8\n\n'
        self.assertEqual(header, expected)

        # Test body
        expected = self.strip_version_xmlns(b'<ServiceExceptionReport version="1.3.0" xmlns="http://www.opengis.net/ogc">\n <ServiceException code="Service configuration error">Service unknown or unsupported</ServiceException>\n</ServiceExceptionReport>\n')
        self.assertEqual(self.strip_version_xmlns(body), expected)

    # WFS tests
    def wfs_request_compare(self, request):
        project = self.testdata_path + "test_project_wfs.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = '?MAP=%s&SERVICE=WFS&VERSION=1.0.0&REQUEST=%s' % (urllib.parse.quote(project), request)
        header, body = self.server.handleRequest(query_string)
        self.assert_headers(header, body)
        response = header + body
        reference_path = self.testdata_path + 'wfs_' + request.lower() + '.txt'
        self.store_reference(reference_path, response)
        f = open(reference_path, 'rb')
        expected = f.read()
        f.close()
        response = re.sub(RE_STRIP_UNCHECKABLE, b'', response)
        expected = re.sub(RE_STRIP_UNCHECKABLE, b'', expected)

        self.assertXMLEqual(response, expected, msg="request %s failed.\n Query: %s" % (query_string, request))

    def test_project_wfs(self):
        """Test some WFS request"""
        for request in ('GetCapabilities', 'DescribeFeatureType'):
            self.wfs_request_compare(request)

    def wfs_getfeature_compare(self, requestid, request):
        project = self.testdata_path + "test_project_wfs.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = '?MAP=%s&SERVICE=WFS&VERSION=1.0.0&REQUEST=%s' % (urllib.parse.quote(project), request)
        header, body = self.server.handleRequest(query_string)
        self.result_compare(
            'wfs_getfeature_' + requestid + '.txt',
            "request %s failed.\n Query: %s" % (
                query_string,
                request,
            ),
            header, body
        )

    def test_getfeature(self):
        tests = []
        tests.append(('nobbox', 'GetFeature&TYPENAME=testlayer'))
        tests.append(('startindex2', 'GetFeature&TYPENAME=testlayer&STARTINDEX=2'))
        tests.append(('limit2', 'GetFeature&TYPENAME=testlayer&MAXFEATURES=2'))
        tests.append(('start1_limit1', 'GetFeature&TYPENAME=testlayer&MAXFEATURES=1&STARTINDEX=1'))

        for id, req in tests:
            self.wfs_getfeature_compare(id, req)

    def test_wfs_getcapabilities_url(self):
        """Check that URL in GetCapabilities response is complete"""
        # empty url in project
        project = os.path.join(self.testdata_path, "test_project_without_urls.qgs")
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WFS",
            "VERSION": "1.3.0",
            "REQUEST": "GetCapabilities",
            "STYLES": ""
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))

        for item in str(r).split("\\n"):
            if "onlineResource" in item:
                self.assertEqual("onlineResource=\"?" in item, True)

        # url well defined in query string
        project = os.path.join(self.testdata_path, "test_project_without_urls.qgs")
        qs = "https://www.qgis-server.org?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WFS",
            "VERSION": "1.3.0",
            "REQUEST": "GetCapabilities",
            "STYLES": ""
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))

        for item in str(r).split("\\n"):
            if "onlineResource" in item:
                self.assertTrue("onlineResource=\"https://www.qgis-server.org?" in item, True)

        # url well defined in project
        project = os.path.join(self.testdata_path, "test_project_with_urls.qgs")
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WFS",
            "VERSION": "1.3.0",
            "REQUEST": "GetCapabilities",
            "STYLES": ""
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))

        for item in str(r).split("\\n"):
            if "onlineResource" in item:
                self.assertEqual("onlineResource=\"my_wfs_advertised_url\"" in item, True)

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
        self.assertXMLEqual(response, expected, msg="%s\n" % (error_msg_header))

    def wfs_getfeature_post_compare(self, requestid, request):
        project = self.testdata_path + "test_project_wfs.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = '?MAP={}'.format(urllib.parse.quote(project))
        header, body = self.server.handleRequest(query_string, requestMethod=QgsServerRequest.PostMethod, data=request)

        self.result_compare(
            'wfs_getfeature_{}.txt'.format(requestid),
            "GetFeature in POST for '{}' failed.".format(requestid),
            header, body,
        )

    def test_getfeature_post(self):
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

        tests = []
        tests.append(('nobbox_post', template.format("")))
        tests.append(('startindex2_post', template.format('startIndex="2"')))
        tests.append(('limit2_post', template.format('maxFeatures="2"')))
        tests.append(('start1_limit1_post', template.format('startIndex="1" maxFeatures="1"')))

        for id, req in tests:
            self.wfs_getfeature_post_compare(id, req)

    # WCS tests
    def wcs_request_compare(self, request):
        project = self.projectPath
        assert os.path.exists(project), "Project file not found: " + project

        query_string = '?MAP=%s&SERVICE=WCS&VERSION=1.0.0&REQUEST=%s' % (urllib.parse.quote(project), request)
        header, body = self.server.handleRequest(query_string)
        self.assert_headers(header, body)
        response = header + body
        reference_path = self.testdata_path + 'wcs_' + request.lower() + '.txt'
        f = open(reference_path, 'rb')
        self.store_reference(reference_path, response)
        expected = f.read()
        f.close()
        response = re.sub(RE_STRIP_UNCHECKABLE, b'', response)
        expected = re.sub(RE_STRIP_UNCHECKABLE, b'', expected)

        self.assertXMLEqual(response, expected, msg="request %s failed.\n Query: %s\n Expected:\n%s\n\n Response:\n%s" % (query_string, request, expected.decode('utf-8'), response.decode('utf-8')))

    def test_project_wcs(self):
        """Test some WCS request"""
        for request in ('GetCapabilities', 'DescribeCoverage'):
            self.wcs_request_compare(request)

    def test_wcs_getcapabilities_url(self):
        # empty url in project
        project = os.path.join(self.testdata_path, "test_project_without_urls.qgs")
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WCS",
            "VERSION": "1.0.0",
            "REQUEST": "GetCapabilities",
            "STYLES": ""
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))

        item_found = False
        for item in str(r).split("\\n"):
            if "OnlineResource" in item:
                self.assertEqual("=\"?" in item, True)
                item_found = True
        self.assertTrue(item_found)

        # url well defined in project
        project = os.path.join(self.testdata_path, "test_project_with_urls.qgs")
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(project),
            "SERVICE": "WCS",
            "VERSION": "1.0.0",
            "REQUEST": "GetCapabilities",
            "STYLES": ""
        }.items())])

        r, h = self._result(self.server.handleRequest(qs))

        item_found = False
        for item in str(r).split("\\n"):
            if "OnlineResource" in item:
                print("OnlineResource: ", item)
                self.assertEqual("\"my_wcs_advertised_url" in item, True)
                item_found = True
        self.assertTrue(item_found)


if __name__ == '__main__':
    unittest.main()
