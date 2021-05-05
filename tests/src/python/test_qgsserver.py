# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer.

Set the env var ENCODED_OUTPUT to enable printing the base64 encoded image diff

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

import os

# Deterministic XML
os.environ['QT_HASH_SEED'] = '1'

import re
import urllib.request
import urllib.parse
import urllib.error
import email
import difflib

from io import StringIO
from qgis.server import QgsServer, QgsServerRequest, QgsBufferServerRequest, QgsBufferServerResponse
from qgis.core import QgsRenderChecker, QgsApplication, QgsFontUtils, QgsMultiRenderChecker
from qgis.testing import unittest, start_app
from qgis.PyQt.QtCore import QSize
from utilities import unitTestDataPath

import osgeo.gdal  # NOQA
import tempfile
import base64


start_app()

# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = br'MAP=[^"]+|Content-Length: \d+'
RE_ELEMENT = br'</*([^>\[\s]+)[ >]'
RE_ELEMENT_CONTENT = br'<[^>\[]+>(.+)</[^>\[\s]+>'
RE_ATTRIBUTES = rb'((?:(?!\s|=).)*)\s*?=\s*?["\']?((?:(?<=")(?:(?<=\\)"|[^"])*|(?<=\')(?:(?<=\\)\'|[^\'])*)|(?:(?!"|\')(?:(?!\/>|>|\s).)+))'


class QgsServerTestBase(unittest.TestCase):

    """Base class for QGIS server tests"""

    # Set to True in child classes to re-generate reference files for this class
    regenerate_reference = False

    def assertXMLEqual(self, response, expected, msg='', raw=False):
        """Compare XML line by line and sorted attributes"""
        response_lines = response.splitlines()
        expected_lines = expected.splitlines()
        line_no = 1

        diffs = []
        for diff in difflib.unified_diff([l.decode('utf8') for l in expected_lines], [l.decode('utf8') for l in response_lines]):
            diffs.append(diff)

        self.assertEqual(len(expected_lines), len(response_lines), "Expected and response have different number of lines!\n{}\n{}".format(msg, '\n'.join(diffs)))
        for expected_line in expected_lines:
            expected_line = expected_line.strip()
            response_line = response_lines[line_no - 1].strip()
            response_line = response_line.replace(b'e+6', br'e+06')
            # Compare tag
            if re.match(RE_ELEMENT, expected_line) and not raw:
                expected_elements = re.findall(RE_ELEMENT, expected_line)
                response_elements = re.findall(RE_ELEMENT, response_line)
                self.assertEqual(expected_elements[0],
                                 response_elements[0], msg=msg + "\nTag mismatch on line %s: %s != %s" % (line_no, expected_line, response_line))
                # Compare content
                if len(expected_elements) == 2 and expected_elements[0] == expected_elements[1]:
                    expected_element_content = re.findall(RE_ELEMENT_CONTENT, expected_line)
                    response_element_content = re.findall(RE_ELEMENT_CONTENT, response_line)
                    self.assertEqual(len(expected_element_content), len(response_element_content),
                                     msg=msg + "\nContent mismatch on line %s: %s != %s" % (line_no, expected_line, response_line))
                    if len(expected_element_content):
                        self.assertEqual(expected_element_content[0],
                                         response_element_content[0], msg=msg + "\nContent mismatch on line %s: %s != %s" % (line_no, expected_line, response_line))
            else:
                self.assertEqual(expected_line, response_line, msg=msg + "\nTag line mismatch %s: %s != %s\n%s" % (line_no, expected_line, response_line, msg))
            # print("---->%s\t%s == %s" % (line_no, expected_line, response_line))
            # Compare attributes
            if re.findall(RE_ATTRIBUTES, expected_line):  # has attrs
                expected_attrs, expected_values = zip(*sorted(re.findall(RE_ATTRIBUTES, expected_line)))
                self.assertTrue(re.findall(RE_ATTRIBUTES, response_line), msg=msg + "\nXML attributes differ at line {0}: {1} != {2}".format(line_no, expected_line, response_line))
                response_attrs, response_values = zip(*sorted(re.findall(RE_ATTRIBUTES, response_line)))
                self.assertEqual(expected_attrs, response_attrs, msg=msg + "\nXML attributes differ at line {0}: {1} != {2}".format(line_no, expected_attrs, response_attrs))
                self.assertEqual(expected_values, response_values, msg=msg + "\nXML attribute values differ at line {0}: {1} != {2}".format(line_no, expected_values, response_values))
            line_no += 1

    def setUp(self):
        """Create the server instance"""
        self.fontFamily = QgsFontUtils.standardTestFontFamily()
        QgsFontUtils.loadStandardTestFonts(['All'])

        self.testdata_path = unitTestDataPath('qgis_server') + '/'

        d = unitTestDataPath('qgis_server_accesscontrol') + '/'
        self.projectPath = os.path.join(d, "project.qgs")
        self.projectAnnotationPath = os.path.join(d, "project_with_annotations.qgs")
        self.projectStatePath = os.path.join(d, "project_state.qgs")
        self.projectUseLayerIdsPath = os.path.join(d, "project_use_layerids.qgs")
        self.projectGroupsPath = os.path.join(d, "project_groups.qgs")

        # Clean env just to be sure
        env_vars = ['QUERY_STRING', 'QGIS_PROJECT_FILE']
        for ev in env_vars:
            try:
                del os.environ[ev]
            except KeyError:
                pass

        self.server = QgsServer()

        # Disable landing page API to test standard legacy XML responses in case of errors
        os.environ["QGIS_SERVER_DISABLED_APIS"] = "Landing Page"

    def tearDown(self):
        """Cleanup env"""

        super().tearDown()
        try:
            del os.environ["QGIS_SERVER_DISABLED_APIS"]
        except KeyError:
            pass

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

    def _img_diff(self, image, control_image, max_diff, max_size_diff=QSize(), outputFormat='PNG'):

        if outputFormat == 'PNG':
            extFile = 'png'
        elif outputFormat == 'JPG':
            extFile = 'jpg'
        elif outputFormat == 'WEBP':
            extFile = 'webp'
        else:
            raise RuntimeError('Yeah, new format implemented')

        temp_image = os.path.join(tempfile.gettempdir(), "%s_result.%s" % (control_image, extFile))

        with open(temp_image, "wb") as f:
            f.write(image)

        if outputFormat != 'PNG':
            return (True, "QgsRenderChecker can only be used for PNG")

        control = QgsMultiRenderChecker()
        control.setControlPathPrefix("qgis_server")
        control.setControlName(control_image)
        control.setRenderedImage(temp_image)
        if max_size_diff.isValid():
            control.setSizeTolerance(max_size_diff.width(), max_size_diff.height())
        return control.runTest(control_image, max_diff), control.report()

    def _img_diff_error(self, response, headers, image, max_diff=100, max_size_diff=QSize(), unittest_data_path='control_images', outputFormat='PNG'):
        """
        :param outputFormat: PNG, JPG or WEBP
        """

        if outputFormat == 'PNG':
            extFile = 'png'
            contentType = 'image/png'
        elif outputFormat == 'JPG':
            extFile = 'jpg'
            contentType = 'image/jpeg'
        elif outputFormat == 'WEBP':
            extFile = 'webp'
            contentType = 'image/webp'
        else:
            raise RuntimeError('Yeah, new format implemented')

        reference_path = unitTestDataPath(unittest_data_path) + '/qgis_server/' + image + '/' + image + '.' + extFile
        self.store_reference(reference_path, response)

        self.assertEqual(
            headers.get("Content-Type"), contentType,
            "Content type is wrong: %s instead of %s\n%s" % (headers.get("Content-Type"), contentType, response))

        test, report = self._img_diff(response, image, max_diff, max_size_diff, outputFormat)

        with open(os.path.join(tempfile.gettempdir(), image + "_result." + extFile), "rb") as rendered_file:
            encoded_rendered_file = base64.b64encode(rendered_file.read())
            if not os.environ.get('ENCODED_OUTPUT'):
                message = "Image is wrong: rendered file %s/%s_result.%s" % (tempfile.gettempdir(), image, extFile)
            else:
                message = "Image is wrong\n%s\nImage:\necho '%s' | base64 -d >%s/%s_result.%s" % (
                    report, encoded_rendered_file.strip().decode('utf8'), tempfile.gettempdir(), image, extFile
                )

        # If the failure is in image sizes the diff file will not exists.
        if os.path.exists(os.path.join(tempfile.gettempdir(), image + "_result_diff." + extFile)):
            with open(os.path.join(tempfile.gettempdir(), image + "_result_diff." + extFile), "rb") as diff_file:
                if not os.environ.get('ENCODED_OUTPUT'):
                    message = "Image is wrong: diff file %s/%s_result_diff.%s" % (tempfile.gettempdir(), image, extFile)
                else:
                    encoded_diff_file = base64.b64encode(diff_file.read())
                    message += "\nDiff:\necho '%s' | base64 -d > %s/%s_result_diff.%s" % (
                        encoded_diff_file.strip().decode('utf8'), tempfile.gettempdir(), image, extFile
                    )

        self.assertTrue(test, message)

    def _execute_request(self, qs, requestMethod=QgsServerRequest.GetMethod, data=None, request_headers=None):
        request = QgsBufferServerRequest(qs, requestMethod, request_headers or {}, data)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response)
        headers = []
        rh = response.headers()
        rk = sorted(rh.keys())
        for k in rk:
            headers.append(("%s: %s" % (k, rh[k])).encode('utf-8'))
        return b"\n".join(headers) + b"\n\n", bytes(response.body())

    def _execute_request_project(self, qs, project, requestMethod=QgsServerRequest.GetMethod, data=None):
        request = QgsBufferServerRequest(qs, requestMethod, {}, data)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        headers = []
        rh = response.headers()
        rk = sorted(rh.keys())
        for k in rk:
            headers.append(("%s: %s" % (k, rh[k])).encode('utf-8'))
        return b"\n".join(headers) + b"\n\n", bytes(response.body())

    def _assert_status_code(self, status_code, qs, requestMethod=QgsServerRequest.GetMethod, data=None, project=None):
        request = QgsBufferServerRequest(qs, requestMethod, {}, data)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        assert response.statusCode() == status_code, "%s != %s" % (response.statusCode(), status_code)


class TestQgsServerTestBase(unittest.TestCase):

    def test_assert_xml_equal(self):
        engine = QgsServerTestBase()

        # test bad assertion
        expected = b'</WFSLayers>\n<Layer queryable="1">\n'
        response = b'<Layer>\n'
        self.assertRaises(AssertionError, engine.assertXMLEqual, response, expected)

        expected = b'</WFSLayers>\n<Layer queryable="1">\n'
        response = b'</WFSLayers>\n<Layer>\n'
        self.assertRaises(AssertionError, engine.assertXMLEqual, response, expected)

        expected = b'</WFSLayers>\n<Layer queryable="1">\n'
        response = b'</WFSLayers>\n<Layer fake="1">\n'
        self.assertRaises(AssertionError, engine.assertXMLEqual, response, expected)

        expected = b'</WFSLayers>\n<Layer queryable="1">\n'
        response = b'</WFSLayers>\n<Layer queryable="2">\n'
        self.assertRaises(AssertionError, engine.assertXMLEqual, response, expected)

        expected = b'<TreeName>QGIS Test Project</TreeName>\n<Layer geometryType="Point" queryable="1" displayField="name" visible="1">\n'
        response = b'<TreeName>QGIS Test Project</TreeName>\n<Layer geometryType="Point" queryable="1" displayField="name">\n'
        self.assertRaises(AssertionError, engine.assertXMLEqual, response, expected)

        expected = b'<TreeName>QGIS Test Project</TreeName>\n<Layer geometryType="Point" queryable="1" displayField="name" visible="1">\n'
        response = b'<TreeName>QGIS Test Project</TreeName>\n<Layer geometryType="Point" queryable="1" displayField="name" visible="0">\n'
        self.assertRaises(AssertionError, engine.assertXMLEqual, response, expected)

        # test valid assertion
        expected = b'</WFSLayers>\n<Layer queryable="1">\n'
        response = b'</WFSLayers>\n<Layer queryable="1">\n'
        self.assertFalse(engine.assertXMLEqual(response, expected))

        expected = b'<TreeName>QGIS Test Project</TreeName>\n<Layer geometryType="Point" queryable="1" displayField="name" visible="1">\n'
        response = b'<TreeName>QGIS Test Project</TreeName>\n<Layer geometryType="Point" queryable="1" displayField="name" visible="1">\n'
        self.assertFalse(engine.assertXMLEqual(response, expected))


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
            locals()["rq%s" % i] = QgsBufferServerRequest("")
            locals()["re%s" % i] = QgsBufferServerResponse()
            locals()["s%s" % i].handleRequest(locals()["rq%s" % i], locals()["re%s" % i])

    def test_requestHandler(self):
        """Test request handler"""
        headers = {'header-key-1': 'header-value-1', 'header-key-2': 'header-value-2'}
        request = QgsBufferServerRequest('http://somesite.com/somepath', QgsServerRequest.GetMethod, headers)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response)
        self.assertEqual(bytes(response.body()), b'<ServerException>Project file error. For OWS services: please provide a SERVICE and a MAP parameter pointing to a valid QGIS project file</ServerException>\n')
        self.assertEqual(response.headers(), {'Content-Length': '156', 'Content-Type': 'text/xml; charset=utf-8'})
        self.assertEqual(response.statusCode(), 500)

    def test_requestHandlerProject(self):
        """Test request handler with none project"""
        headers = {'header-key-1': 'header-value-1', 'header-key-2': 'header-value-2'}
        request = QgsBufferServerRequest('http://somesite.com/somepath', QgsServerRequest.GetMethod, headers)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, None)
        self.assertEqual(bytes(response.body()), b'<ServerException>Project file error. For OWS services: please provide a SERVICE and a MAP parameter pointing to a valid QGIS project file</ServerException>\n')
        self.assertEqual(response.headers(), {'Content-Length': '156', 'Content-Type': 'text/xml; charset=utf-8'})
        self.assertEqual(response.statusCode(), 500)

    def test_api(self):
        """Using an empty query string (returns an XML exception)
        we are going to test if headers and body are returned correctly"""
        # Test as a whole
        header, body = self._execute_request("")
        response = self.strip_version_xmlns(header + body)
        expected = self.strip_version_xmlns(b'Content-Length: 156\nContent-Type: text/xml; charset=utf-8\n\n<ServerException>Project file error. For OWS services: please provide a SERVICE and a MAP parameter pointing to a valid QGIS project file</ServerException>\n')
        self.assertEqual(response, expected)
        expected = b'Content-Length: 156\nContent-Type: text/xml; charset=utf-8\n\n'
        self.assertEqual(header, expected)

        # Test response when project is specified but without service
        project = self.testdata_path + "test_project_wfs.qgs"
        qs = '?MAP=%s' % (urllib.parse.quote(project))
        header, body = self._execute_request(qs)
        response = self.strip_version_xmlns(header + body)
        expected = self.strip_version_xmlns(b'Content-Length: 326\nContent-Type: text/xml; charset=utf-8\n\n<ServiceExceptionReport  >\n <ServiceException code="Service configuration error">Service unknown or unsupported. Current supported services (case-sensitive): WMS WFS WCS WMTS SampleService, or use a WFS3 (OGC API Features) endpoint</ServiceException>\n</ServiceExceptionReport>\n')
        self.assertEqual(response, expected)
        expected = b'Content-Length: 326\nContent-Type: text/xml; charset=utf-8\n\n'
        self.assertEqual(header, expected)

        # Test body
        expected = self.strip_version_xmlns(b'<ServiceExceptionReport  >\n <ServiceException code="Service configuration error">Service unknown or unsupported. Current supported services (case-sensitive): WMS WFS WCS WMTS SampleService, or use a WFS3 (OGC API Features) endpoint</ServiceException>\n</ServiceExceptionReport>\n')
        self.assertEqual(self.strip_version_xmlns(body), expected)

    # WCS tests
    def wcs_request_compare(self, request):
        project = self.projectPath
        assert os.path.exists(project), "Project file not found: " + project

        query_string = '?MAP=%s&SERVICE=WCS&VERSION=1.0.0&REQUEST=%s' % (urllib.parse.quote(project), request)
        header, body = self._execute_request(query_string)
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

        r, h = self._result(self._execute_request(qs))

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

        r, h = self._result(self._execute_request(qs))

        item_found = False
        for item in str(r).split("\\n"):
            if "OnlineResource" in item:
                self.assertEqual("\"my_wcs_advertised_url" in item, True)
                item_found = True
        self.assertTrue(item_found)

        # Service URL in header
        for header_name, header_value in (("X-Qgis-Service-Url", "http://test1"), ("X-Qgis-Wcs-Service-Url", "http://test2")):
            # empty url in project
            project = os.path.join(self.testdata_path, "test_project_without_urls.qgs")
            qs = "?" + "&".join(["%s=%s" % i for i in list({
                "MAP": urllib.parse.quote(project),
                "SERVICE": "WCS",
                "VERSION": "1.0.0",
                "REQUEST": "GetCapabilities",
                "STYLES": ""
            }.items())])

            r, h = self._result(self._execute_request(qs, request_headers={header_name: header_value}))

            item_found = False
            for item in str(r).split("\\n"):
                if "OnlineResource" in item:
                    print(item)
                    print(header_name)
                    print(header_value)
                    self.assertEqual(header_value in item, True)
                    item_found = True
            self.assertTrue(item_found)

        # Other headers combinaison
        for headers, online_resource in (
            ({"Forwarded": "host=test3;proto=https"}, "https://test3"),
            ({"Forwarded": "host=test4;proto=https, host=test5;proto=https"}, "https://test4"),
            ({"X-Forwarded-Host": "test6", "X-Forwarded-Proto": "https"}, "https://test6"),
            ({"Host": "test7"}, "test7"),
        ):
            # empty url in project
            project = os.path.join(self.testdata_path, "test_project_without_urls.qgs")
            qs = "?" + "&".join(["%s=%s" % i for i in list({
                "MAP": urllib.parse.quote(project),
                "SERVICE": "WCS",
                "VERSION": "1.0.0",
                "REQUEST": "GetCapabilities",
                "STYLES": ""
            }.items())])

            r, h = self._result(self._execute_request(qs, request_headers=headers))

            item_found = False
            for item in str(r).split("\\n"):
                if "OnlineResource" in item:
                    self.assertEqual(online_resource in item, True)
                    item_found = True
            self.assertTrue(item_found)


if __name__ == '__main__':
    unittest.main()
