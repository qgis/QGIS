# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServerRequest.

From build dir, run: ctest -R PyQgsServerRequest -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
import unittest
import os
from urllib.parse import parse_qs, urlparse

__author__ = 'Alessandro Pasotti'
__date__ = '29/04/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


from qgis.PyQt.QtCore import QUrl
from qgis.server import QgsServerRequest, QgsFcgiServerRequest


class QgsServerRequestTest(unittest.TestCase):

    def test_requestHeaders(self):
        """Test request headers"""
        headers = {'header-key-1': 'header-value-1', 'header-key-2': 'header-value-2'}
        request = QgsServerRequest('http://somesite.com/somepath', QgsServerRequest.GetMethod, headers)
        for k, v in request.headers().items():
            self.assertEqual(headers[k], v)
        request.removeHeader('header-key-1')
        self.assertEqual(request.headers(), {'header-key-2': 'header-value-2'})
        request.setHeader('header-key-1', 'header-value-1')
        for k, v in request.headers().items():
            self.assertEqual(headers[k], v)

    def test_requestParameters(self):
        """Test request parameters"""
        request = QgsServerRequest('http://somesite.com/somepath?parm1=val1&parm2=val2', QgsServerRequest.GetMethod)
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
        request = QgsServerRequest('http://somesite.com/somepath?parm1=val1%20%2B+val2', QgsServerRequest.GetMethod)
        self.assertEqual(request.parameters()['PARM1'], 'val1 + val2')

    def test_requestUrl(self):
        """Test url"""
        request = QgsServerRequest('http://somesite.com/somepath', QgsServerRequest.GetMethod)
        self.assertEqual(request.url().toString(), 'http://somesite.com/somepath')
        request.setUrl(QUrl('http://someother.com/someotherpath'))
        self.assertEqual(request.url().toString(), 'http://someother.com/someotherpath')

    def test_requestMethod(self):
        request = QgsServerRequest('http://somesite.com/somepath', QgsServerRequest.GetMethod)
        self.assertEqual(request.method(), QgsServerRequest.GetMethod)
        request.setMethod(QgsServerRequest.PostMethod)
        self.assertEqual(request.method(), QgsServerRequest.PostMethod)

    def test_fcgiRequest(self):
        """Test various combinations of FCGI env parameters with rewritten urls"""

        def _test_url(url, env={}):
            for k in ('QUERY_STRING', 'REQUEST_URI', 'SERVER_NAME', 'SERVER_PORT', 'SCRIPT_NAME'):
                try:
                    del os.environ[k]
                except KeyError:
                    pass
                try:
                    os.environ[k] = env[k]
                except KeyError:
                    pass
            request = QgsFcgiServerRequest()
            self.assertEqual(request.url().toString(), url)
            # Check MAP
            if 'QUERY_STRING' in env:
                map = {k.upper(): v[0] for k, v in parse_qs(env['QUERY_STRING']).items()}['MAP']
            else:
                map = {k.upper(): v[0] for k, v in parse_qs(urlparse(env['REQUEST_URI']).query).items()}['MAP']
            self.assertEqual(request.parameter('MAP'), map)

        _test_url('http://somesite.com/somepath/index.html?map=/my/path.qgs', {
            'REQUEST_URI': '/somepath/index.html?map=/my/path.qgs',
            'SERVER_NAME': 'somesite.com',
        })
        _test_url('http://somesite.com/somepath?map=/my/path.qgs', {
            'REQUEST_URI': '/somepath?map=/my/path.qgs',
            'SERVER_NAME': 'somesite.com',
        })
        _test_url('http://somesite.com/somepath/path', {
            'REQUEST_URI': '/somepath/path',
            'SERVER_NAME': 'somesite.com',
            'QUERY_STRING': 'map=/my/path.qgs'
        })
        _test_url('http://somesite.com/somepath/path/?token=QGIS2019', {
            'REQUEST_URI': '/somepath/path/?token=QGIS2019',
            'SERVER_NAME': 'somesite.com',
            'QUERY_STRING': 'map=/my/path.qgs',
        })


if __name__ == '__main__':
    unittest.main()
