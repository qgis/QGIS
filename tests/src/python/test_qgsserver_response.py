# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServerResponse.

From build dir, run: ctest -R PyQgsServerResponse -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
import unittest

__author__ = 'Alessandro Pasotti'
__date__ = '29/04/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'


from qgis.server import QgsBufferServerResponse


class QgsServerResponseTest(unittest.TestCase):

    def test_responseHeaders(self):
        """Test response headers"""
        headers = {'header-key-1': 'header-value-1', 'header-key-2': 'header-value-2'}
        response = QgsBufferServerResponse()
        for k, v in headers.items():
            response.setHeader(k, v)
        for k, v in response.headers().items():
            self.assertEqual(headers[k], v)
        response.removeHeader('header-key-1')
        self.assertEqual(response.headers(), {'header-key-2': 'header-value-2'})
        response.setHeader('header-key-1', 'header-value-1')
        for k, v in response.headers().items():
            self.assertEqual(headers[k], v)

    def test_statusCode(self):
        """Test return status HTTP code"""
        response = QgsBufferServerResponse()
        response.setStatusCode(222)
        self.assertEqual(response.statusCode(), 222)

    def test_write(self):
        """Test that writing on the buffer sets the body"""
        # Set as str
        response = QgsBufferServerResponse()
        response.write('Greetings from Essen Linux Hotel 2017 Hack Fest!')
        self.assertEqual(bytes(response.body()), b'')
        response.finish()
        self.assertEqual(bytes(response.body()), b'Greetings from Essen Linux Hotel 2017 Hack Fest!')
        self.assertEqual(response.headers(), {'Content-Length': '48'})

        # Set as a byte array
        response = QgsBufferServerResponse()
        response.write(b'Greetings from Essen Linux Hotel 2017 Hack Fest!')
        self.assertEqual(bytes(response.body()), b'')
        response.finish()
        self.assertEqual(bytes(response.body()), b'Greetings from Essen Linux Hotel 2017 Hack Fest!')


if __name__ == '__main__':
    unittest.main()
