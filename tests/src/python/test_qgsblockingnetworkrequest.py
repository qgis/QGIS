# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsBlockingNetworkRequest

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from builtins import chr
from builtins import str
__author__ = 'Nyall Dawson'
__date__ = '12/11/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os
from qgis.testing import unittest, start_app
from qgis.core import QgsBlockingNetworkRequest
from utilities import unitTestDataPath
from qgis.PyQt.QtCore import QUrl
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtNetwork import QNetworkReply, QNetworkRequest
import socketserver
import threading
import http.server

app = start_app()


class TestQgsBlockingNetworkRequest(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # Bring up a simple HTTP server
        os.chdir(unitTestDataPath() + '')
        handler = http.server.SimpleHTTPRequestHandler

        cls.httpd = socketserver.TCPServer(('localhost', 0), handler)
        cls.port = cls.httpd.server_address[1]

        cls.httpd_thread = threading.Thread(target=cls.httpd.serve_forever)
        cls.httpd_thread.setDaemon(True)
        cls.httpd_thread.start()

    def testFetchEmptyUrl(self):
        request = QgsBlockingNetworkRequest()
        spy = QSignalSpy(request.downloadFinished)
        err = request.get(QNetworkRequest(QUrl()))
        self.assertEqual(len(spy), 1)
        self.assertEqual(err, QgsBlockingNetworkRequest.ServerExceptionError)
        self.assertEqual(request.errorMessage(), 'Protocol "" is unknown')
        reply = request.reply()
        self.assertFalse(reply.content())

    def testFetchBadUrl(self):
        request = QgsBlockingNetworkRequest()
        spy = QSignalSpy(request.downloadFinished)
        err = request.get(QNetworkRequest(QUrl('http://x')))
        self.assertEqual(len(spy), 1)
        self.assertEqual(err, QgsBlockingNetworkRequest.ServerExceptionError)
        self.assertEqual(request.errorMessage(), 'Host x not found')
        reply = request.reply()
        self.assertFalse(reply.content())

    def testFetchBadUrl2(self):
        request = QgsBlockingNetworkRequest()
        spy = QSignalSpy(request.downloadFinished)
        err = request.get(QNetworkRequest(QUrl('http://localhost:' + str(TestQgsBlockingNetworkRequest.port) + '/ffff')))
        self.assertEqual(len(spy), 1)
        self.assertEqual(err, QgsBlockingNetworkRequest.ServerExceptionError)
        self.assertIn('File not found', request.errorMessage())
        reply = request.reply()
        self.assertEqual(reply.error(), QNetworkReply.ContentNotFoundError)
        self.assertFalse(reply.content())

    def testGet(self):
        request = QgsBlockingNetworkRequest()
        spy = QSignalSpy(request.downloadFinished)
        err = request.get(QNetworkRequest(QUrl('http://localhost:' + str(TestQgsBlockingNetworkRequest.port) + '/qgis_local_server/index.html')))
        self.assertEqual(len(spy), 1)
        self.assertEqual(err, QgsBlockingNetworkRequest.NoError)
        self.assertEqual(request.errorMessage(), '')
        reply = request.reply()
        self.assertEqual(reply.error(), QNetworkReply.NoError)
        self.assertEqual(reply.content(), '<!DOCTYPE html>\n<html lang="en">\n<head>\n\t<meta charset="utf-8" />\n\t<title>Local QGIS Server Default Index</title>\n</head>\n<body>\n  <h2 style="font-family:Arial;">Web Server Working<h2/>\n</body>\n</html>\n')
        self.assertEqual(reply.rawHeaderList(), [b'Server',
                                                 b'Date',
                                                 b'Content-type',
                                                 b'Content-Length',
                                                 b'Last-Modified'])
        self.assertEqual(reply.rawHeader(b'Content-type'), 'text/html')
        self.assertEqual(reply.rawHeader(b'xxxxxxxxx'), '')
        self.assertEqual(reply.attribute(QNetworkRequest.HttpStatusCodeAttribute), 200)
        self.assertEqual(reply.attribute(QNetworkRequest.HttpReasonPhraseAttribute), 'OK')
        self.assertEqual(reply.attribute(QNetworkRequest.HttpStatusCodeAttribute), 200)
        self.assertEqual(reply.attribute(QNetworkRequest.RedirectionTargetAttribute), None)


if __name__ == "__main__":
    unittest.main()
