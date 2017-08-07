# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import re
import urllib
from mimetools import Message
from StringIO import StringIO
from qgis.server import QgsServer
from qgis.testing import unittest
from utilities import unitTestDataPath

# Strip path and content length because path may vary
RE_STRIP_PATH = r'MAP=[^&]+|Content-Length: \d+|<Attribute typeName="[^>]+'


class TestQgsServerTwoLayer(unittest.TestCase):

    def setUp(self):
        """Create the server instance"""
        self.testdata_path = unitTestDataPath('qgis_server')

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

    # WFS tests
    def result_compare(self, file_name, error_msg_header, header, body):
        self.assert_headers(header, body)
        response = header + body
        f = open(os.path.join(self.testdata_path, file_name))
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

    def wfs_getfeature_post_compare(self, requestid, request):
        project = os.path.join(self.testdata_path, "test_project_two_layers.qgs")
        self.assertTrue(os.path.exists(project), msg=u"Project file not found: %s" % (project))

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

    def test_getfeature_post_multiple_layer(self):
        template = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:GetFeature service="WFS" version="1.1.0">
 <wfs:Query typeName="secondlayer" xmlns:feature="http://www.qgis.org/gml">
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
        self.wfs_getfeature_post_compare('multiple', template)


if __name__ == '__main__':
    unittest.main()
