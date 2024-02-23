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

import os

# Deterministic XML
os.environ['QT_HASH_SEED'] = '1'

import base64
import difflib
import email
import re
import tempfile
import urllib.error
import urllib.parse
import urllib.request

from io import StringIO
from shutil import copytree

import osgeo.gdal  # NOQA

from qgis.core import (
    QgsFontUtils,
    QgsMultiRenderChecker,
)
from qgis.PyQt.QtCore import QSize
from qgis.PyQt.QtGui import QColor, QImage
from qgis.server import (
    QgsBufferServerRequest,
    QgsBufferServerResponse,
    QgsServer,
    QgsServerParameterDefinition,
    QgsServerRequest,
)
import unittest
from qgis.testing import start_app, QgisTestCase
from utilities import unitTestDataPath

start_app()

# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = br'MAP=[^"]+|Content-Length: \d+'
RE_ELEMENT = br'</*([^>\[\s]+)[ >]'
RE_ELEMENT_CONTENT = br'<[^>\[]+>(.+)</[^>\[\s]+>'
RE_ATTRIBUTES = rb'((?:(?!\s|=).)*)\s*?=\s*?["\']?((?:(?<=")(?:(?<=\\)"|[^"])*|(?<=\')(?:(?<=\\)\'|[^\'])*)|(?:(?!"|\')(?:(?!\/>|>|\s).)+))'


class QgsServerTestBase(QgisTestCase):

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

        self.assertEqual(
            len(expected_lines),
            len(response_lines),
            "Expected and response have different number of lines!\n{}\n{}\nWe got :\n{}".format(msg, '\n'.join(diffs), '\n'.join([i.decode("utf-8") for i in response_lines])))
        for expected_line in expected_lines:
            expected_line = expected_line.strip()
            response_line = response_lines[line_no - 1].strip()
            response_line = response_line.replace(b'e+6', br'e+06')
            # Compare tag
            if re.match(RE_ELEMENT, expected_line) and not raw:
                expected_elements = re.findall(RE_ELEMENT, expected_line)
                response_elements = re.findall(RE_ELEMENT, response_line)
                self.assertEqual(expected_elements[0],
                                 response_elements[0], msg=msg + f"\nTag mismatch on line {line_no}: {expected_line} != {response_line}")
                # Compare content
                if len(expected_elements) == 2 and expected_elements[0] == expected_elements[1]:
                    expected_element_content = re.findall(RE_ELEMENT_CONTENT, expected_line)
                    response_element_content = re.findall(RE_ELEMENT_CONTENT, response_line)
                    self.assertEqual(len(expected_element_content), len(response_element_content),
                                     msg=msg + f"\nContent mismatch on line {line_no}: {expected_line} != {response_line}")
                    if len(expected_element_content):
                        self.assertEqual(expected_element_content[0],
                                         response_element_content[0], msg=msg + f"\nContent mismatch on line {line_no}: {expected_line} != {response_line}")
            else:
                self.assertEqual(expected_line, response_line, msg=msg + f"\nTag line mismatch {line_no}: {expected_line} != {response_line}\n{msg}")
            # print("---->%s\t%s == %s" % (line_no, expected_line, response_line))
            # Compare attributes
            if re.findall(RE_ATTRIBUTES, expected_line):  # has attrs
                expected_attrs, expected_values = zip(*sorted(re.findall(RE_ATTRIBUTES, expected_line)))
                self.assertTrue(re.findall(RE_ATTRIBUTES, response_line), msg=msg + f"\nXML attributes differ at line {line_no}: {expected_line} != {response_line}")
                response_attrs, response_values = zip(*sorted(re.findall(RE_ATTRIBUTES, response_line)))
                self.assertEqual(expected_attrs, response_attrs, msg=msg + f"\nXML attributes differ at line {line_no}: {expected_attrs} != {response_attrs}")
                self.assertEqual(expected_values, response_values, msg=msg + f"\nXML attribute values differ at line {line_no}: {expected_values} != {response_values}")
            line_no += 1

    @classmethod
    def setUpClass(self):
        """Create the server instance"""
        super().setUpClass()
        self.fontFamily = QgsFontUtils.standardTestFontFamily()
        QgsFontUtils.loadStandardTestFonts(['All'])

        self.temporary_dir = tempfile.TemporaryDirectory()
        self.temporary_path = self.temporary_dir.name

        # Copy all testdata to the temporary directory
        copytree(unitTestDataPath('qgis_server'), os.path.join(self.temporary_path, 'qgis_server'))
        copytree(unitTestDataPath('qgis_server_accesscontrol'), os.path.join(self.temporary_path, 'qgis_server_accesscontrol'))

        for f in [
            'empty_spatial_layer.dbf',
            'empty_spatial_layer.prj',
            'empty_spatial_layer.qpj',
            'empty_spatial_layer.shp',
            'empty_spatial_layer.shx',
            'france_parts.dbf',
            'france_parts.prj',
            'france_parts.qpj',
            'france_parts.shp',
            'france_parts.shp.xml',
            'france_parts.shx',
            'landsat.tif',
            'points.dbf',
            'points.prj',
            'points.shp',
            'points.shx',
            'requires_warped_vrt.tif',
        ]:
            os.symlink(
                unitTestDataPath(f),
                os.path.join(self.temporary_path, f)
            )

        self.testdata_path = os.path.join(self.temporary_path, 'qgis_server') + '/'

        d = os.path.join(self.temporary_path, 'qgis_server_accesscontrol')

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

    @classmethod
    def tearDownClass(self):
        """Cleanup env"""
        try:
            del os.environ["QGIS_SERVER_DISABLED_APIS"]
        except KeyError:
            pass

        self.temporary_dir.cleanup()
        super().tearDownClass()

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

    def _img_diff(self, image: str, control_image, max_diff, max_size_diff=QSize(), outputFormat='PNG') -> bool:

        if outputFormat == 'PNG':
            extFile = 'png'
        elif outputFormat == 'JPG':
            extFile = 'jpg'
        elif outputFormat == 'WEBP':
            extFile = 'webp'
        else:
            raise RuntimeError('Yeah, new format implemented')

        temp_image = os.path.join(tempfile.gettempdir(), f"{control_image}_result.{extFile}")

        with open(temp_image, "wb") as f:
            f.write(image)

        if outputFormat != 'PNG':
            # TODO fix this, it's not actually testing anything..!
            return True

        rendered_image = QImage(temp_image)
        if rendered_image.format() not in (QImage.Format.Format_RGB32,
                                           QImage.Format.Format_ARGB32,
                                           QImage.Format.Format_ARGB32_Premultiplied):
            rendered_image = rendered_image.convertToFormat(QImage.Format.Format_ARGB32)

        return self.image_check(
            control_image,
            control_image,
            rendered_image,
            control_image,
            allowed_mismatch=max_diff,
            control_path_prefix="qgis_server",
            size_tolerance=max_size_diff
        )

    def _img_diff_error(self, response, headers, test_name: str, max_diff=100, max_size_diff=QSize(), outputFormat='PNG'):
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

        if self.regenerate_reference:
            reference_path = unitTestDataPath(
                'control_images') + '/qgis_server/' + test_name + '/' + test_name + '.' + extFile
            self.store_reference(reference_path, response)

        self.assertEqual(
            headers.get("Content-Type"), contentType,
            f"Content type is wrong: {headers.get('Content-Type')} instead of {contentType}\n{response}")

        self.assertTrue(
            self._img_diff(response, test_name, max_diff, max_size_diff, outputFormat)
        )

    def _execute_request(self, qs, requestMethod=QgsServerRequest.GetMethod, data=None, request_headers=None):
        request = QgsBufferServerRequest(qs, requestMethod, request_headers or {}, data)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response)
        headers = []
        rh = response.headers()
        rk = sorted(rh.keys())
        for k in rk:
            headers.append((f"{k}: {rh[k]}").encode())
        return b"\n".join(headers) + b"\n\n", bytes(response.body())

    def _execute_request_project(self, qs, project, requestMethod=QgsServerRequest.GetMethod, data=None):
        request = QgsBufferServerRequest(qs, requestMethod, {}, data)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        headers = []
        rh = response.headers()
        rk = sorted(rh.keys())
        for k in rk:
            headers.append((f"{k}: {rh[k]}").encode())
        return b"\n".join(headers) + b"\n\n", bytes(response.body())

    def _assert_status_code(self, status_code, qs, requestMethod=QgsServerRequest.GetMethod, data=None, project=None):
        request = QgsBufferServerRequest(qs, requestMethod, {}, data)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        assert response.statusCode() == status_code, f"{response.statusCode()} != {status_code}"

    def _assertRed(self, color: QColor):
        self.assertEqual(color.red(), 255)
        self.assertEqual(color.green(), 0)
        self.assertEqual(color.blue(), 0)

    def _assertGreen(self, color: QColor):
        self.assertEqual(color.red(), 0)
        self.assertEqual(color.green(), 255)
        self.assertEqual(color.blue(), 0)

    def _assertBlue(self, color: QColor):
        self.assertEqual(color.red(), 0)
        self.assertEqual(color.green(), 0)
        self.assertEqual(color.blue(), 255)

    def _assertBlack(self, color: QColor):
        self.assertEqual(color.red(), 0)
        self.assertEqual(color.green(), 0)
        self.assertEqual(color.blue(), 0)

    def _assertWhite(self, color: QColor):
        self.assertEqual(color.red(), 255)
        self.assertEqual(color.green(), 255)
        self.assertEqual(color.blue(), 255)


class TestQgsServerTestBase(QgisTestCase):

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
            locals()[f"s{i}"] = QgsServer()
            locals()[f"rq{i}"] = QgsBufferServerRequest("")
            locals()[f"re{i}"] = QgsBufferServerResponse()
            locals()[f"s{i}"].handleRequest(locals()[f"rq{i}"], locals()[f"re{i}"])

    def test_requestHandler(self):
        """Test request handler"""
        headers = {'header-key-1': 'header-value-1', 'header-key-2': 'header-value-2'}
        request = QgsBufferServerRequest('http://somesite.com/somepath', QgsServerRequest.GetMethod, headers)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response)
        self.assertEqual(bytes(response.body()), b'<?xml version="1.0" encoding="UTF-8"?>\n<ServerException>Project file error. For OWS services: please provide a SERVICE and a MAP parameter pointing to a valid QGIS project file</ServerException>\n')
        self.assertEqual(response.headers(), {'Content-Length': '195', 'Content-Type': 'text/xml; charset=utf-8'})
        self.assertEqual(response.statusCode(), 500)

    def test_requestHandlerProject(self):
        """Test request handler with none project"""
        headers = {'header-key-1': 'header-value-1', 'header-key-2': 'header-value-2'}
        request = QgsBufferServerRequest('http://somesite.com/somepath', QgsServerRequest.GetMethod, headers)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, None)
        self.assertEqual(bytes(response.body()), b'<?xml version="1.0" encoding="UTF-8"?>\n<ServerException>Project file error. For OWS services: please provide a SERVICE and a MAP parameter pointing to a valid QGIS project file</ServerException>\n')
        self.assertEqual(response.headers(), {'Content-Length': '195', 'Content-Type': 'text/xml; charset=utf-8'})
        self.assertEqual(response.statusCode(), 500)

    def test_api(self):
        """Using an empty query string (returns an XML exception)
        we are going to test if headers and body are returned correctly"""
        # Test as a whole
        header, body = self._execute_request("")
        response = self.strip_version_xmlns(header + body)
        expected = self.strip_version_xmlns(b'Content-Length: 195\nContent-Type: text/xml; charset=utf-8\n\n<?xml version="1.0" encoding="UTF-8"?>\n<ServerException>Project file error. For OWS services: please provide a SERVICE and a MAP parameter pointing to a valid QGIS project file</ServerException>\n')
        self.assertEqual(response, expected)
        expected = b'Content-Length: 195\nContent-Type: text/xml; charset=utf-8\n\n'
        self.assertEqual(header, expected)

        # Test response when project is specified but without service
        project = self.testdata_path + "test_project_wfs.qgs"
        qs = f'?MAP={urllib.parse.quote(project)}'
        header, body = self._execute_request(qs)
        response = self.strip_version_xmlns(header + body)
        expected = self.strip_version_xmlns(b'Content-Length: 365\nContent-Type: text/xml; charset=utf-8\n\n<?xml version="1.0" encoding="UTF-8"?>\n<ServiceExceptionReport  >\n <ServiceException code="Service configuration error">Service unknown or unsupported. Current supported services (case-sensitive): WMS WFS WCS WMTS SampleService, or use a WFS3 (OGC API Features) endpoint</ServiceException>\n</ServiceExceptionReport>\n')
        self.assertEqual(response, expected)
        expected = b'Content-Length: 365\nContent-Type: text/xml; charset=utf-8\n\n'
        self.assertEqual(header, expected)

        # Test body
        expected = self.strip_version_xmlns(b'<?xml version="1.0" encoding="UTF-8"?>\n<ServiceExceptionReport  >\n <ServiceException code="Service configuration error">Service unknown or unsupported. Current supported services (case-sensitive): WMS WFS WCS WMTS SampleService, or use a WFS3 (OGC API Features) endpoint</ServiceException>\n</ServiceExceptionReport>\n')
        self.assertEqual(self.strip_version_xmlns(body), expected)

    # WCS tests
    def wcs_request_compare(self, request):
        project = self.projectPath
        assert os.path.exists(project), "Project file not found: " + project

        query_string = f'?MAP={urllib.parse.quote(project)}&SERVICE=WCS&VERSION=1.0.0&REQUEST={request}'
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

        self.assertXMLEqual(response, expected, msg=f"request {query_string} failed.\n Query: {request}\n Expected:\n{expected.decode('utf-8')}\n\n Response:\n{response.decode('utf-8')}")

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


class TestQgsServerParameter(QgisTestCase):

    def test_filter(self):
        # empty filter
        param = QgsServerParameterDefinition()
        param.mValue = ""

        self.assertEqual(len(param.toOgcFilterList()), 0)
        self.assertEqual(len(param.toExpressionList()), 0)

        # single qgis expression
        filter = "\"name\"=concat('t', 'wo')"

        param = QgsServerParameterDefinition()
        param.mValue = filter

        self.assertEqual(len(param.toOgcFilterList()), 0)
        self.assertEqual(len(param.toExpressionList()), 1)

        self.assertEqual(param.toExpressionList()[0], filter)

        # multiple qgis expressions
        filter0 = "to_datetime('2017-09-29 12:00:00')"
        filter1 = "Contours:\"elev\" <= 1200"
        filter2 = "\"name\"='three'"

        param = QgsServerParameterDefinition()
        param.mValue = f"{filter0};{filter1};{filter2}"

        self.assertEqual(len(param.toOgcFilterList()), 0)
        self.assertEqual(len(param.toExpressionList()), 3)

        self.assertEqual(param.toExpressionList()[0], filter0)
        self.assertEqual(param.toExpressionList()[1], filter1)
        self.assertEqual(param.toExpressionList()[2], filter2)

        # multiple qgis expressions with some empty one
        param = QgsServerParameterDefinition()
        param.mValue = f";;{filter0};;;{filter2};;"

        self.assertEqual(len(param.toOgcFilterList()), 0)
        self.assertEqual(len(param.toExpressionList()), 8)

        self.assertEqual(param.toExpressionList()[0], "")
        self.assertEqual(param.toExpressionList()[1], "")
        self.assertEqual(param.toExpressionList()[2], filter0)
        self.assertEqual(param.toExpressionList()[3], "")
        self.assertEqual(param.toExpressionList()[4], "")
        self.assertEqual(param.toExpressionList()[5], filter2)
        self.assertEqual(param.toExpressionList()[6], "")
        self.assertEqual(param.toExpressionList()[7], "")

        # two empty expressions
        param = QgsServerParameterDefinition()
        param.mValue = ";"

        self.assertEqual(len(param.toOgcFilterList()), 0)
        self.assertEqual(len(param.toExpressionList()), 2)

        # single ogc empty filter
        param = QgsServerParameterDefinition()
        param.mValue = "()"

        self.assertEqual(len(param.toExpressionList()), 0)
        self.assertEqual(len(param.toOgcFilterList()), 1)

        self.assertEqual(param.toOgcFilterList()[0], "")

        # single ogc filter
        filter = "<Filter><Within><PropertyName>name<PropertyName><gml:Envelope><gml:lowerCorner>43.5707 -79.5797</gml:lowerCorner><gml:upperCorner>43.8219 -79.2693</gml:upperCorner></gml:Envelope></Within></Filter>"

        param = QgsServerParameterDefinition()
        param.mValue = filter

        self.assertEqual(len(param.toExpressionList()), 0)
        self.assertEqual(len(param.toOgcFilterList()), 1)

        self.assertEqual(param.toOgcFilterList()[0], filter)

        # multiple ogc filter
        filter0 = "<Filter><Within><PropertyName>InWaterA_1M/wkbGeom<PropertyName><gml:Envelope><gml:lowerCorner>43.5707 -79.5797</gml:lowerCorner><gml:upperCorner>43.8219 -79.2693</gml:upperCorner></gml:Envelope></Within></Filter>"
        filter1 = "<Filter><Within><PropertyName>BuiltUpA_1M/wkbGeom<PropertyName><gml:Envelope><gml:lowerCorner>43.5705 -79.5797</gml:lowerCorner><gml:upperCorner>43.8219 -79.2693</gml:upperCorner></gml:Envelope></Within></Filter>"

        param = QgsServerParameterDefinition()
        param.mValue = f"({filter0})({filter1})"

        self.assertEqual(len(param.toExpressionList()), 0)
        self.assertEqual(len(param.toOgcFilterList()), 2)

        self.assertEqual(param.toOgcFilterList()[0], filter0)
        self.assertEqual(param.toOgcFilterList()[1], filter1)

        # multiple ogc filter with some empty one
        filter0 = "<Filter><Within><PropertyName>InWaterA_1M/wkbGeom<PropertyName><gml:Envelope><gml:lowerCorner>43.5707 -79.5797</gml:lowerCorner><gml:upperCorner>43.8219 -79.2693</gml:upperCorner></gml:Envelope></Within></Filter>"
        filter1 = "<Filter><Within><PropertyName>BuiltUpA_1M/wkbGeom<PropertyName><gml:Envelope><gml:lowerCorner>43.5705 -79.5797</gml:lowerCorner><gml:upperCorner>43.8219 -79.2693</gml:upperCorner></gml:Envelope></Within></Filter>"

        param = QgsServerParameterDefinition()
        param.mValue = f"()()({filter0})()()({filter1})()()"

        self.assertEqual(len(param.toExpressionList()), 0)
        self.assertEqual(len(param.toOgcFilterList()), 8)

        self.assertEqual(param.toOgcFilterList()[0], "")
        self.assertEqual(param.toOgcFilterList()[1], "")
        self.assertEqual(param.toOgcFilterList()[2], filter0)
        self.assertEqual(param.toOgcFilterList()[3], "")
        self.assertEqual(param.toOgcFilterList()[4], "")
        self.assertEqual(param.toOgcFilterList()[5], filter1)
        self.assertEqual(param.toOgcFilterList()[6], "")
        self.assertEqual(param.toOgcFilterList()[7], "")


if __name__ == '__main__':
    unittest.main()
