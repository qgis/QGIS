# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServerRequest.

From build dir, run: ctest -R PyQgsServerRequest -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = 'Alessandro Pasotti'
__date__ = '29/04/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'


import os
import re
import unittest
from urllib.parse import parse_qs, urlencode, urlparse

from qgis.PyQt.QtCore import QUrl
from qgis.server import (QgsBufferServerResponse, QgsFcgiServerRequest,
                         QgsServerRequest)
from test_qgsserver import QgsServerTestBase


class QgsServerRequestTest(QgsServerTestBase):

    @staticmethod
    def _set_env(env={}):
        for k in ('QUERY_STRING', 'REQUEST_URI', 'SERVER_NAME', 'CONTENT_LENGTH', 'SERVER_PORT', 'SCRIPT_NAME', 'REQUEST_BODY', 'REQUEST_METHOD'):
            try:
                del os.environ[k]
            except KeyError:
                pass
            try:
                os.environ[k] = env[k]
            except KeyError:
                pass

    def test_requestHeaders(self):
        """Test request headers"""
        headers = {'header-key-1': 'header-value-1',
                   'header-key-2': 'header-value-2'}
        request = QgsServerRequest(
            'http://somesite.com/somepath', QgsServerRequest.GetMethod, headers)
        for k, v in request.headers().items():
            self.assertEqual(headers[k], v)
        request.removeHeader('header-key-1')
        self.assertEqual(request.headers(), {'header-key-2': 'header-value-2'})
        request.setHeader('header-key-1', 'header-value-1')
        for k, v in request.headers().items():
            self.assertEqual(headers[k], v)

    def test_requestParameters(self):
        """Test request parameters"""
        request = QgsServerRequest(
            'http://somesite.com/somepath?parm1=val1&parm2=val2', QgsServerRequest.GetMethod)
        parameters = {'PARM1': 'val1', 'PARM2': 'val2'}
        for k, v in request.parameters().items():
            self.assertEqual(parameters[k], v)
        request.removeParameter('PARM1')
        self.assertEqual(request.parameters(), {'PARM2': 'val2'})
        request.setHeader('PARM1', 'val1')
        for k, v in request.headers().items():
            self.assertEqual(parameters[k], v)

    def test_requestParametersDecoding(self):
        """Test request parameters decoding"""
        request = QgsServerRequest(
            'http://somesite.com/somepath?parm1=val1%20%2B+val2', QgsServerRequest.GetMethod)
        self.assertEqual(request.parameters()['PARM1'], 'val1 + val2')

    def test_requestUrl(self):
        """Test url"""
        request = QgsServerRequest(
            'http://somesite.com/somepath', QgsServerRequest.GetMethod)
        self.assertEqual(request.url().toString(),
                         'http://somesite.com/somepath')
        request.setUrl(QUrl('http://someother.com/someotherpath'))
        self.assertEqual(request.url().toString(),
                         'http://someother.com/someotherpath')

    def test_requestMethod(self):
        request = QgsServerRequest(
            'http://somesite.com/somepath', QgsServerRequest.GetMethod)
        self.assertEqual(request.method(), QgsServerRequest.GetMethod)
        request.setMethod(QgsServerRequest.PostMethod)
        self.assertEqual(request.method(), QgsServerRequest.PostMethod)

    def test_fcgiRequest(self):
        """Test various combinations of FCGI env parameters with rewritten urls"""

        def _test_url(original_url, rewritten_url, env={}):
            self._set_env(env)
            request = QgsFcgiServerRequest()
            self.assertEqual(request.originalUrl().toString(), original_url)
            self.assertEqual(request.url().toString(), rewritten_url)
            # Check MAP
            if 'QUERY_STRING' in env:
                map = {k.upper(): v[0] for k, v in parse_qs(
                    env['QUERY_STRING']).items()}['MAP']
            else:
                map = {k.upper(): v[0] for k, v in parse_qs(
                    urlparse(env['REQUEST_URI']).query).items()}['MAP']
            self.assertEqual(request.parameter('MAP'), map)

        _test_url('http://somesite.com/somepath/project1/',
                  'http://somesite.com/somepath/project1/?map=/my/project1.qgs', {
                      'REQUEST_URI': '/somepath/project1/',
                      'SERVER_NAME': 'somesite.com',
                      'QUERY_STRING': 'map=/my/project1.qgs'
                  })

        _test_url('http://somesite.com/somepath/path/?token=QGIS2019',
                  'http://somesite.com/somepath/path/?map=/my/path.qgs', {
                      'REQUEST_URI': '/somepath/path/?token=QGIS2019',
                      'SERVER_NAME': 'somesite.com',
                      'QUERY_STRING': 'map=/my/path.qgs',
                  })

        _test_url('http://somesite.com/somepath/index.html?map=/my/path.qgs',
                  'http://somesite.com/somepath/index.html?map=/my/path.qgs',
                  {
                      'REQUEST_URI': '/somepath/index.html?map=/my/path.qgs',
                      'SERVER_NAME': 'somesite.com',
                  })

        _test_url('http://somesite.com/somepath?map=/my/path.qgs',
                  'http://somesite.com/somepath?map=/my/path.qgs',
                  {
                      'REQUEST_URI': '/somepath?map=/my/path.qgs',
                      'SERVER_NAME': 'somesite.com',
                  })

    def test_fcgiRequestPOST(self):
        """Test various combinations of FCGI POST parameters with rewritten urls"""

        def _check_links(params, method='GET'):
            data = urlencode(params)
            if method == 'GET':
                env = {
                    'SERVER_NAME': 'www.myserver.com',
                    'REQUEST_URI': '/aproject/',
                    'QUERY_STRING': data,
                    'REQUEST_METHOD': 'GET',
                }
            else:
                env = {
                    'SERVER_NAME': 'www.myserver.com',
                    'REQUEST_URI': '/aproject/',
                    'REQUEST_BODY': data,
                    'CONTENT_LENGTH': str(len(data)),
                    'REQUEST_METHOD': 'POST',
                }

            self._set_env(env)
            request = QgsFcgiServerRequest()
            response = QgsBufferServerResponse()
            self.server.handleRequest(request, response)
            self.assertFalse(b'ServiceExceptionReport' in response.body())

            if method == 'POST':
                self.assertEqual(request.data(), data.encode('utf8'))
            else:
                original_url = request.originalUrl().toString()
                self.assertTrue(original_url.startswith('http://www.myserver.com/aproject/'))
                self.assertEqual(original_url.find(urlencode({'MAP': params['map']})), -1)

            exp = re.compile(r'href="([^"]+)"', re.DOTALL | re.MULTILINE)
            elems = exp.findall(bytes(response.body()).decode('utf8'))
            self.assertTrue(len(elems) > 0)
            for href in elems:
                self.assertTrue(href.startswith('http://www.myserver.com/aproject/'))
                self.assertEqual(href.find(urlencode({'MAP': params['map']})), -1)

        # Test post request handler
        params = {
            'map': os.path.join(self.testdata_path, 'test_project_wfs.qgs'),
            'REQUEST': 'GetCapabilities',
            'SERVICE': 'WFS',
        }
        _check_links(params)
        _check_links(params, 'POST')
        params['SERVICE'] = 'WMS'
        _check_links(params)
        _check_links(params, 'POST')
        params['SERVICE'] = 'WCS'
        _check_links(params)
        _check_links(params, 'POST')
        params['SERVICE'] = 'WMTS'
        _check_links(params)
        _check_links(params, 'POST')

    def test_fcgiRequestBody(self):
        """Test request body"""
        data = '<Literal>+1</Literal>'
        self._set_env({
            'SERVER_NAME': 'www.myserver.com',
            'SERVICE': 'WFS',
            'REQUEST_BODY': data,
            'CONTENT_LENGTH': str(len(data)),
            'REQUEST_METHOD': 'POST',
        })
        request = QgsFcgiServerRequest()
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response)
        self.assertEqual(request.parameter('REQUEST_BODY'), '<Literal>+1</Literal>')

    def test_add_parameters(self):
        request = QgsServerRequest()
        request.setParameter('FOOBAR', 'foobar')
        self.assertEqual(request.parameter('FOOBAR'), 'foobar')
        self.assertEqual(request.parameter('UNKNOWN'), '')

    def test_headers(self):
        """Tests that the headers are working in Fcgi mode"""
        for header, env, enum, value in (
            ("Host", "HTTP_HOST", QgsServerRequest.HOST, "example.com"),
            ("Forwarded", "HTTP_FORWARDED", QgsServerRequest.FORWARDED, "aaa"),
            ("X-Forwarded-For", "HTTP_X_FORWARDED_FOR", QgsServerRequest.X_FORWARDED_FOR, "bbb"),
            ("X-Forwarded-Host", "HTTP_X_FORWARDED_HOST", QgsServerRequest.X_FORWARDED_HOST, "ccc"),
            ("X-Forwarded-Proto", "HTTP_X_FORWARDED_PROTO", QgsServerRequest.X_FORWARDED_PROTO, "ddd"),
            ("X-Qgis-Service-Url", "HTTP_X_QGIS_SERVICE_URL", QgsServerRequest.X_QGIS_SERVICE_URL, "eee"),
            ("X-Qgis-Wms-Service-Url", "HTTP_X_QGIS_WMS_SERVICE_URL", QgsServerRequest.X_QGIS_WMS_SERVICE_URL, "fff"),
            ("X-Qgis-Wfs-Service-Url", "HTTP_X_QGIS_WFS_SERVICE_URL", QgsServerRequest.X_QGIS_WFS_SERVICE_URL, "ggg"),
            ("X-Qgis-Wcs-Service-Url", "HTTP_X_QGIS_WCS_SERVICE_URL", QgsServerRequest.X_QGIS_WCS_SERVICE_URL, "hhh"),
            ("X-Qgis-Wmts-Service-Url", "HTTP_X_QGIS_WMTS_SERVICE_URL", QgsServerRequest.X_QGIS_WMTS_SERVICE_URL, "iii"),
            ("Accept", "HTTP_ACCEPT", QgsServerRequest.ACCEPT, "jjj"),
            ("User-Agent", "HTTP_USER_AGENT", QgsServerRequest.USER_AGENT, "kkk"),
            ("Authorization", "HTTP_AUTHORIZATION", QgsServerRequest.AUTHORIZATION, "lll"),
        ):
            try:
                os.environ[env] = value
                request = QgsFcgiServerRequest()
                self.assertEquals(request.headers(), {header: value})
                request = QgsServerRequest(request)
                self.assertEquals(request.headers(), {header: value})
                self.assertEquals(request.header(enum), value)
            finally:
                del os.environ[env]


if __name__ == '__main__':
    unittest.main()
