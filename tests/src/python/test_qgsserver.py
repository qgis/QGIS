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
import urllib
from mimetools import Message
from StringIO import StringIO
from qgis.server import QgsServer
from qgis.core import QgsMessageLog
from qgis.testing import unittest
from utilities import unitTestDataPath
import osgeo.gdal

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

    def assert_headers(self, header, body):
        headers = Message(StringIO(header))
        if 'content-length' in headers:
            content_length = int(headers['content-length'])
            body_length = len(body)
            self.assertEqual(content_length, body_length, msg="Header reported content-length: %d Actual body length was: %d" % (content_length, body_length))

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

    # WMS tests
    def wms_request_compare(self, request, extra=None, reference_file=None):
        project = self.testdata_path + "test+project.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = 'MAP=%s&SERVICE=WMS&VERSION=1.3&REQUEST=%s' % (urllib.quote(project), request)
        if extra is not None:
            query_string += extra
        header, body = [str(_v) for _v in self.server.handleRequest(query_string)]
        response = header + body
        f = open(self.testdata_path + (request.lower() if not reference_file else reference_file) + '.txt')
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
        #"""
        response = re.sub(RE_STRIP_PATH, '', response)
        expected = re.sub(RE_STRIP_PATH, '', expected)

        # for older GDAL versions (<2.0), id field will be integer type
        if int(osgeo.gdal.VersionInfo()[:1]) < 2:
            expected = expected.replace('typeName="Integer64" precision="0" length="10" editType="TextEdit" type="qlonglong"', 'typeName="Integer" precision="0" length="10" editType="TextEdit" type="int"')

        self.assertEqual(response, expected, msg="request %s failed.\n Query: %s\n Expected:\n%s\n\n Response:\n%s" % (query_string, request, expected, response))

    def test_project_wms(self):
        """Test some WMS request"""
        for request in ('GetCapabilities', 'GetProjectSettings'):
            self.wms_request_compare(request)

        # Test getfeatureinfo response
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'info_format=text%2Fhtml&transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320',
                                 'wms_getfeatureinfo-text-html')

        # Test getfeatureinfo default info_format
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&styles=&' +
                                 'transparent=true&' +
                                 'width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C' +
                                 '5606005.488876367%2C913235.426296057%2C5606035.347090538&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320',
                                 'wms_getfeatureinfo-text-plain')

        # Regression for #8656
        # Mind the gap! (the space in the FILTER expression)
        self.wms_request_compare('GetFeatureInfo',
                                 '&layers=testlayer%20%C3%A8%C3%A9&' +
                                 'INFO_FORMAT=text%2Fxml&' +
                                 'width=600&height=400&srs=EPSG%3A3857&' +
                                 'query_layers=testlayer%20%C3%A8%C3%A9&' +
                                 'FEATURE_COUNT=10&FILTER=testlayer%20%C3%A8%C3%A9' + urllib.quote(':"NAME" = \'two\''),
                                 'wms_getfeatureinfo_filter')

    def wms_inspire_request_compare(self, request):
        """WMS INSPIRE tests"""
        project = self.testdata_path + "test+project_inspire.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = 'MAP=%s&SERVICE=WMS&VERSION=1.3.0&REQUEST=%s' % (urllib.quote(project), request)
        header, body = [str(_v) for _v in self.server.handleRequest(query_string)]
        response = header + body
        f = open(self.testdata_path + request.lower() + '_inspire.txt')
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

    def test_project_wms_inspire(self):
        """Test some WMS request"""
        for request in ('GetCapabilities',):
            self.wms_inspire_request_compare(request)

    # WFS tests
    def wfs_request_compare(self, request):
        project = self.testdata_path + "test+project_wfs.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = 'MAP=%s&SERVICE=WFS&VERSION=1.0.0&REQUEST=%s' % (urllib.quote(project), request)
        header, body = [str(_v) for _v in self.server.handleRequest(query_string)]
        self.assert_headers(header, body)
        response = header + body
        f = open(self.testdata_path + 'wfs_' + request.lower() + '.txt')
        expected = f.read()
        f.close()
        # Store the output for debug or to regenerate the reference documents:
        """
        f = open(os.path.dirname(__file__) + '/wfs_' +  request.lower() + '_expected.txt', 'w+')
        f.write(expected)
        f.close()
        f = open(os.path.dirname(__file__) + '/wfs_' +  request.lower() + '_response.txt', 'w+')
        f.write(response)
        f.close()
        """
        response = re.sub(RE_STRIP_PATH, '', response)
        expected = re.sub(RE_STRIP_PATH, '', expected)

        # for older GDAL versions (<2.0), id field will be integer type
        if int(osgeo.gdal.VersionInfo()[:1]) < 2:
            expected = expected.replace('<element type="long" name="id"/>', '<element type="integer" name="id"/>')

        self.assertEqual(response, expected, msg="request %s failed.\n Query: %s\n Expected:\n%s\n\n Response:\n%s" % (query_string, request, expected, response))

    def test_project_wfs(self):
        """Test some WFS request"""
        for request in ('GetCapabilities', 'DescribeFeatureType'):
            self.wfs_request_compare(request)

    def wfs_getfeature_compare(self, requestid, request):
        project = self.testdata_path + "test+project_wfs.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = 'MAP=%s&SERVICE=WFS&VERSION=1.0.0&REQUEST=%s' % (urllib.quote(project), request)
        header, body = [str(_v) for _v in self.server.handleRequest(query_string)]
        self.result_compare(
            'wfs_getfeature_' + requestid + '.txt',
            u"request %s failed.\n Query: %s" % (
                query_string,
                request,
            ),
            header, body
        )

    def result_compare(self, file_name, error_msg_header, header, body):
        self.assert_headers(header, body)
        response = header + body
        f = open(self.testdata_path + file_name)
        expected = f.read()
        f.close()
        # Store the output for debug or to regenerate the reference documents:
        """
        f = open(os.path.dirname(__file__) + '/wfs_getfeature_' +  requestid + '_expected.txt', 'w+')
        f.write(expected)
        f.close()
        f = open(os.path.dirname(__file__) + '/wfs_getfeature_' +  requestid + '_response.txt', 'w+')
        f.write(response)
        f.close()
        """
        response = re.sub(RE_STRIP_PATH, '', response)
        expected = re.sub(RE_STRIP_PATH, '', expected)
        self.assertEqual(response, expected, msg=u"%s\n Expected:\n%s\n\n Response:\n%s"
                                                 % (error_msg_header,
                                                    unicode(expected, errors='replace'),
                                                    unicode(response, errors='replace')))

    def test_getfeature(self):
        tests = []
        tests.append(('nobbox', u'GetFeature&TYPENAME=testlayer'))
        tests.append(('startindex2', u'GetFeature&TYPENAME=testlayer&STARTINDEX=2'))
        tests.append(('limit2', u'GetFeature&TYPENAME=testlayer&MAXFEATURES=2'))
        tests.append(('start1_limit1', u'GetFeature&TYPENAME=testlayer&MAXFEATURES=1&STARTINDEX=1'))

        for id, req in tests:
            self.wfs_getfeature_compare(id, req)

    def wfs_getfeature_post_compare(self, requestid, request):
        project = self.testdata_path + "test+project_wfs.qgs"
        assert os.path.exists(project), "Project file not found: " + project

        query_string = 'MAP={}'.format(urllib.quote(project))
        self.server.putenv("REQUEST_METHOD", "POST")
        self.server.putenv("REQUEST_BODY", request)
        header, body = self.server.handleRequest(query_string)
        self.server.putenv("REQUEST_METHOD", '')
        self.server.putenv("REQUEST_BODY", '')

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
        tests.append(('nobbox', template.format("")))
        tests.append(('startindex2', template.format('startIndex="2"')))
        tests.append(('limit2', template.format('maxFeatures="2"')))
        tests.append(('start1_limit1', template.format('startIndex="1" maxFeatures="1"')))

        for id, req in tests:
            self.wfs_getfeature_post_compare(id, req)

    def test_getLegendGraphics(self):
        """Test that does not return an exception but an image"""
        parms = {
            'MAP': self.testdata_path + "test%2Bproject.qgs",
            'SERVICE': 'WMS',
            'VERSION': '1.0.0',
            'REQUEST': 'GetLegendGraphic',
            'FORMAT': 'image/png',
            #'WIDTH': '20', # optional
            #'HEIGHT': '20', # optional
            'LAYER': u'testlayer+èé',
        }
        qs = '&'.join([u"%s=%s" % (k, v) for k, v in parms.iteritems()])
        h, r = self.server.handleRequest(qs)
        self.assertEqual(-1, h.find('Content-Type: text/xml; charset=utf-8'), "Header: %s\nResponse:\n%s" % (h, r))
        self.assertNotEquals(-1, h.find('Content-Type: image/png'), "Header: %s\nResponse:\n%s" % (h, r))


if __name__ == '__main__':
    unittest.main()
