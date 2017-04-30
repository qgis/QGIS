# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer plugins and filters.


From build dir, run: ctest -R PyQgsServerPlugins -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '22/04/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import re
import urllib.request
import urllib.parse
import urllib.error
import email

from io import StringIO
from qgis.server import QgsServer
from qgis.core import QgsMessageLog, QgsApplication
from qgis.testing import unittest
from utilities import unitTestDataPath

import osgeo.gdal  # NOQA


# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = b'MAP=[^"]+|Content-Length: \d+'
RE_ATTRIBUTES = b'[^>\s]+=[^>\s]+'


class TestQgsServerPlugins(unittest.TestCase):

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
                self.assertEqual(expected_line, response_line, msg=msg + "\nTag line mismatch %s: %s != %s" % (line_no, expected_line, response_line))
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

    def test_pluginfilters(self):
        """Test python plugins filters"""
        try:
            from qgis.server import QgsServerFilter
        except ImportError:
            print("QGIS Server plugins are not compiled. Skipping test")
            return

        class SimpleHelloFilter(QgsServerFilter):

            def requestReady(self):
                QgsMessageLog.logMessage("SimpleHelloFilter.requestReady")

            def sendResponse(self):
                QgsMessageLog.logMessage("SimpleHelloFilter.sendResponse")

            def responseComplete(self):
                request = self.serverInterface().requestHandler()
                params = request.parameterMap()
                QgsMessageLog.logMessage("SimpleHelloFilter.responseComplete")
                if params.get('SERVICE', '').upper() == 'SIMPLE':
                    request.clear()
                    request.setResponseHeader('Content-type', 'text/plain')
                    request.appendBody('Hello from SimpleServer!'.encode('utf-8'))

        serverIface = self.server.serverInterface()
        filter = SimpleHelloFilter(serverIface)
        serverIface.registerFilter(filter, 100)
        # Get registered filters
        self.assertEqual(filter, serverIface.filters()[100][0])

        # global to be modified inside plugin filters
        globals()['status_code'] = 0
        # body to be checked inside plugin filters
        globals()['body2'] = None
        # headers to be checked inside plugin filters
        globals()['headers2'] = None

        # Register some more filters
        class Filter1(QgsServerFilter):

            def responseComplete(self):
                request = self.serverInterface().requestHandler()
                params = request.parameterMap()
                if params.get('SERVICE', '').upper() == 'SIMPLE':
                    request.appendBody('Hello from Filter1!'.encode('utf-8'))

        class Filter2(QgsServerFilter):

            def responseComplete(self):
                request = self.serverInterface().requestHandler()
                params = request.parameterMap()
                if params.get('SERVICE', '').upper() == 'SIMPLE':
                    request.appendBody('Hello from Filter2!'.encode('utf-8'))

        class Filter3(QgsServerFilter):
            """Test get and set status code"""

            def responseComplete(self):
                global status_code
                request = self.serverInterface().requestHandler()
                request.setStatusCode(999)
                status_code = request.statusCode()

        class Filter4(QgsServerFilter):
            """Body getter"""

            def responseComplete(self):
                global body2
                request = self.serverInterface().requestHandler()
                body2 = request.body()

        class Filter5(QgsServerFilter):
            """Body setter, clear body, keep headers"""

            def responseComplete(self):
                global headers2
                request = self.serverInterface().requestHandler()
                request.clearBody()
                headers2 = request.responseHeaders()
                request.appendBody('new body, new life!'.encode('utf-8'))

        filter1 = Filter1(serverIface)
        filter2 = Filter2(serverIface)
        filter3 = Filter3(serverIface)
        filter4 = Filter4(serverIface)
        serverIface.registerFilter(filter1, 101)
        serverIface.registerFilter(filter2, 200)
        serverIface.registerFilter(filter2, 100)
        serverIface.registerFilter(filter3, 300)
        serverIface.registerFilter(filter4, 400)
        self.assertTrue(filter2 in serverIface.filters()[100])
        self.assertEqual(filter1, serverIface.filters()[101][0])
        self.assertEqual(filter2, serverIface.filters()[200][0])
        header, body = [_v for _v in self.server.handleRequest('?service=simple')]
        response = header + body
        expected = b'Content-Length: 62\nContent-type: text/plain\n\nHello from SimpleServer!Hello from Filter1!Hello from Filter2!'
        self.assertEqual(response, expected)

        # Check status code
        self.assertEqual(status_code, 999)

        # Check body getter from filter
        self.assertEqual(body2, b'Hello from SimpleServer!Hello from Filter1!Hello from Filter2!')

        # Check that the bindings for complex type QgsServerFiltersMap are working
        filters = {100: [filter, filter2], 101: [filter1], 200: [filter2]}
        serverIface.setFilters(filters)
        self.assertTrue(filter in serverIface.filters()[100])
        self.assertTrue(filter2 in serverIface.filters()[100])
        self.assertEqual(filter1, serverIface.filters()[101][0])
        self.assertEqual(filter2, serverIface.filters()[200][0])
        header, body = [_v for _v in self.server.handleRequest('?service=simple')]
        response = header + body
        expected = b'Content-Length: 62\nContent-type: text/plain\n\nHello from SimpleServer!Hello from Filter1!Hello from Filter2!'
        self.assertEqual(response, expected)

        # Now, re-run with body setter
        filter5 = Filter5(serverIface)
        serverIface.registerFilter(filter5, 500)
        header, body = [_v for _v in self.server.handleRequest('?service=simple')]
        response = header + body
        expected = b'Content-Length: 19\nContent-type: text/plain\n\nnew body, new life!'
        self.assertEqual(response, expected)
        self.assertEqual(headers2, {'Content-type': 'text/plain'})


if __name__ == '__main__':
    unittest.main()
