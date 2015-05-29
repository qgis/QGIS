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
from qgis.server import QgsServer
from qgis.core import QgsMessageLog
from utilities import unitTestDataPath

# Strip path and content lenght because path may vary
RE_STRIP_PATH=r'MAP=[^&]+|Content-Length: \d+'


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
        response = str(self.server.handleRequest())
        expected = 'Content-Type: text/xml; charset=utf-8\nContent-Length: 206\n\n<ServiceExceptionReport version="1.3.0" xmlns="http://www.opengis.net/ogc">\n <ServiceException code="Service configuration error">Service unknown or unsupported</ServiceException>\n</ServiceExceptionReport>\n'
        self.assertEqual(response, expected)
        # Test header
        response = str(self.server.handleRequestGetHeaders())
        expected = 'Content-Type: text/xml; charset=utf-8\nContent-Length: 206\n\n'
        self.assertEqual(response, expected)
        # Test body
        response = str(self.server.handleRequestGetBody())
        expected = '<ServiceExceptionReport version="1.3.0" xmlns="http://www.opengis.net/ogc">\n <ServiceException code="Service configuration error">Service unknown or unsupported</ServiceException>\n</ServiceExceptionReport>\n'
        self.assertEqual(response, expected)


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
        serverIface.registerFilter(SimpleHelloFilter(serverIface), 100 )
        response = str(self.server.handleRequest('service=simple'))
        expected = 'Content-type: text/plain\n\n\nHello from SimpleServer!'
        self.assertEqual(response, expected)


    ## WMS tests
    def wms_request_compare(self, request):
        map = self.testdata_path + "testproject.qgs"
        response = str(self.server.handleRequest('MAP=%s&SERVICE=WMS&VERSION=1.3&REQUEST=%s' % (map, request)))
        f = open(self.testdata_path + request.lower() + '.txt')
        expected = f.read()
        f.close()
        self.assertEqual(re.sub(RE_STRIP_PATH, '', response), re.sub(RE_STRIP_PATH, '', expected))


    def test_project_wms(self):
        """Test some WMS request"""
        for request in ('GetCapabilities', 'GetProjectSettings'):
            self.wms_request_compare(request)


if __name__ == '__main__':
    unittest.main()
