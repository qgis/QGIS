# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsNetworkContentFetcher

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from builtins import chr
from builtins import str
__author__ = 'Matthias Kuhn'
__date__ = '4/28/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os
from qgis.testing import unittest
from qgis.core import QgsNetworkContentFetcher
from utilities import unitTestDataPath
from qgis.PyQt.QtCore import QUrl, QCoreApplication
from qgis.PyQt.QtNetwork import QNetworkReply
import socketserver
import threading
import http.server


class TestQgsNetworkContentFetcher(unittest.TestCase):

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

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

        self.loaded = False

        self.app = QCoreApplication([])

    def contentLoaded(self):
        self.loaded = True

    def testFetchEmptyUrl(self):
        fetcher = QgsNetworkContentFetcher()
        self.loaded = False
        fetcher.fetchContent(QUrl())
        fetcher.finished.connect(self.contentLoaded)
        while not self.loaded:
            self.app.processEvents()

        r = fetcher.reply()
        assert r.error() != QNetworkReply.NoError

    def testFetchBadUrl(self):
        fetcher = QgsNetworkContentFetcher()
        self.loaded = False
        fetcher.fetchContent(QUrl('http://x'))
        fetcher.finished.connect(self.contentLoaded)
        while not self.loaded:
            self.app.processEvents()

        r = fetcher.reply()
        assert r.error() != QNetworkReply.NoError

    def testFetchUrlContent(self):
        fetcher = QgsNetworkContentFetcher()
        self.loaded = False
        fetcher.fetchContent(QUrl('http://localhost:' + str(TestQgsNetworkContentFetcher.port) + '/qgis_local_server/index.html'))
        fetcher.finished.connect(self.contentLoaded)
        while not self.loaded:
            self.app.processEvents()

        r = fetcher.reply()
        assert r.error() == QNetworkReply.NoError, r.error()

        html = fetcher.contentAsString()
        assert 'QGIS' in html

    def testDoubleFetch(self):
        fetcher = QgsNetworkContentFetcher()
        self.loaded = False
        fetcher.fetchContent(QUrl('http://www.qgis.org/'))
        # double fetch - this should happen before previous request finishes
        fetcher.fetchContent(QUrl('http://localhost:' + str(TestQgsNetworkContentFetcher.port) + '/qgis_local_server/index.html'))
        fetcher.finished.connect(self.contentLoaded)
        while not self.loaded:
            self.app.processEvents()

        r = fetcher.reply()
        assert r.error() == QNetworkReply.NoError, r.error()

        html = fetcher.contentAsString()
        assert 'QGIS' in html

    def testFetchEncodedContent(self):
        fetcher = QgsNetworkContentFetcher()
        self.loaded = False
        fetcher.fetchContent(QUrl('http://localhost:' + str(TestQgsNetworkContentFetcher.port) + '/encoded_html.html'))
        fetcher.finished.connect(self.contentLoaded)
        while not self.loaded:
            self.app.processEvents()

        r = fetcher.reply()
        assert r.error() == QNetworkReply.NoError, r.error()

        html = fetcher.contentAsString()
        assert chr(6040) in html

if __name__ == "__main__":
    unittest.main()
