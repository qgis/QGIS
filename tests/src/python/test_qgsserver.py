# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer.

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
import re
import unittest
import urllib
from qgis.server import QgsServer
from qgis.core import QgsMessageLog
from utilities import unitTestDataPath

# Strip path and content length because path may vary
RE_STRIP_PATH = r'MAP=[^&]+|Content-Length: \d+'


class TestQgsServer(unittest.TestCase):

    def setUp(self):
        """Create the server instance"""
        self.testdata_path = unitTestDataPath('qgis_server') + '/'
        # Clean env just to be sure
        env_vars = ['QUERY_STRING', 'QGIS_PROJECT_FILE']
        for ev in env_vars:
            try:
                del os.environ[ev]
            except KeyError:
                pass
        self.server = QgsServer()

    def test_destructor_segfaults(self):
        """Segfault on destructor?"""
        server = QgsServer()
        del server

    def test_multiple_servers(self):
        """Segfaults?"""
        for i in range(10):
            locals()["s%s" % i] = QgsServer()
            locals()["s%s" % i].handleRequest()

    def test_api(self):
        """Using an empty query string (returns an XML exception)
        we are going to test if headers and body are returned correctly"""
        # Test as a whole
        header, body = [str(_v) for _v in self.server.handleRequest()]
        response = header + body
        expected = 'Content-Length: 206\nContent-Type: text/xml; charset=utf-8\n\n<ServiceExceptionReport version="1.3.0" xmlns="http://www.opengis.net/ogc">\n <ServiceException code="Service configuration error">Service unknown or unsupported</ServiceException>\n</ServiceExceptionReport>\n'
        self.assertEqual(response, expected)
        expected = 'Content-Length: 206\nContent-Type: text/xml; charset=utf-8\n\n'
        self.assertEqual(header, expected)
        # Test body
        expected = '<ServiceExceptionReport version="1.3.0" xmlns="http://www.opengis.net/ogc">\n <ServiceException code="Service configuration error">Service unknown or unsupported</ServiceException>\n</ServiceExceptionReport>\n'
        self.assertEqual(body, expected)

    def test_pluginfilters(self):
        """Test python plugins filters"""
        try:
            from qgis.server import QgsServerFilter
        except ImportError:
            print "QGIS Server plugins are not compiled. Skipping test"
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
                    request.clearHeaders()
                    request.setHeader('Content-type', 'text/plain')
                    request.clearBody()
                    request.appendBody('Hello from SimpleServer!')

        serverIface = self.server.serverInterface()
        filter = SimpleHelloFilter(serverIface)
        serverIface.registerFilter(filter, 100)
        # Get registered filters
        self.assertEqual(filter, serverIface.filters()[100][0])

        # Register some more filters
        class Filter1(QgsServerFilter):

            def responseComplete(self):
                request = self.serverInterface().requestHandler()
                params = request.parameterMap()
                if params.get('SERVICE', '').upper() == 'SIMPLE':
                    request.appendBody('Hello from Filter1!')

        class Filter2(QgsServerFilter):

            def responseComplete(self):
                request = self.serverInterface().requestHandler()
                params = request.parameterMap()
                if params.get('SERVICE', '').upper() == 'SIMPLE':
                    request.appendBody('Hello from Filter2!')

        filter1 = Filter1(serverIface)
        filter2 = Filter2(serverIface)
        serverIface.registerFilter(filter1, 101)
        serverIface.registerFilter(filter2, 200)
        serverIface.registerFilter(filter2, 100)
        self.assertTrue(filter2 in serverIface.filters()[100])
        self.assertEqual(filter1, serverIface.filters()[101][0])
        self.assertEqual(filter2, serverIface.filters()[200][0])
        header, body = [str(_v) for _v in self.server.handleRequest('service=simple')]
        response = header + body
        expected = 'Content-type: text/plain\n\nHello from SimpleServer!Hello from Filter1!Hello from Filter2!'
        self.assertEqual(response, expected)

        # Test that the bindings for complex type QgsServerFiltersMap are working
        filters = {100: [filter, filter2], 101: [filter1], 200: [filter2]}
        serverIface.setFilters(filters)
        self.assertTrue(filter in serverIface.filters()[100])
        self.assertTrue(filter2 in serverIface.filters()[100])
        self.assertEqual(filter1, serverIface.filters()[101][0])
        self.assertEqual(filter2, serverIface.filters()[200][0])
        header, body = [str(_v) for _v in self.server.handleRequest('service=simple')]
        response = header + body
        expected = 'Content-type: text/plain\n\nHello from SimpleServer!Hello from Filter1!Hello from Filter2!'
        self.assertEqual(response, expected)

    ## WMS tests
    def wms_request_compare(self, request):
        project = self.testdata_path + "test+project.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = 'MAP=%s&SERVICE=WMS&VERSION=1.3&REQUEST=%s' % (urllib.quote(project), request)
        header, body = [str(_v) for _v in self.server.handleRequest(query_string)]
        response = header + body
        f = open(self.testdata_path + request.lower() + '.txt')
        expected = f.read()
        f.close()
        # Store the output for debug or to regenerate the reference documents:
        """
        f = open(os.path.dirname(__file__) + '/expected.txt', 'w+')
        f.write(expected)
        f.close()
        f = open(os.path.dirname(__file__) + '/response.txt', 'w+')
        f.write(response)
        f.close()
        """
        response = re.sub(RE_STRIP_PATH, '', response)
        expected = re.sub(RE_STRIP_PATH, '', expected)
        self.assertEqual(response, expected, msg="request %s failed.\n Query: %s\n Expected:\n%s\n\n Response:\n%s" % (query_string, request, expected, response))

    def test_project_wms(self):
        """Test some WMS request"""
        for request in ('GetCapabilities', 'GetProjectSettings'):
            self.wms_request_compare(request)

    # The following code was used to test type conversion in python bindings
    #def test_qpair(self):
    #    """Test QPair bindings"""
    #    f, s = self.server.testQPair(('First', 'Second'))
    #    self.assertEqual(f, 'First')
    #    self.assertEqual(s, 'Second')


if __name__ == '__main__':
    unittest.main()
