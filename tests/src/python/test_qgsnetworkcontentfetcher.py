"""QGIS Unit tests for QgsNetworkContentFetcher

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Matthias Kuhn"
__date__ = "4/28/2015"
__copyright__ = "Copyright 2015, The QGIS Project"

import http.server
import os
import socketserver
import threading

from qgis.PyQt.QtCore import QUrl
from qgis.PyQt.QtNetwork import QNetworkReply, QNetworkRequest
from qgis.core import QgsNetworkContentFetcher
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

app = start_app()


class TestQgsNetworkContentFetcher(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        # Bring up a simple HTTP server
        os.chdir(unitTestDataPath() + "")
        handler = http.server.SimpleHTTPRequestHandler

        cls.httpd = socketserver.TCPServer(("localhost", 0), handler)
        cls.port = cls.httpd.server_address[1]

        cls.httpd_thread = threading.Thread(target=cls.httpd.serve_forever)
        cls.httpd_thread.daemon = True
        cls.httpd_thread.start()

    def __init__(self, methodName):
        """Run once on class initialization."""
        QgisTestCase.__init__(self, methodName)

        self.loaded = False

    def contentLoaded(self):
        self.loaded = True

    def testFetchEmptyUrl(self):
        fetcher = QgsNetworkContentFetcher()
        self.loaded = False
        fetcher.fetchContent(QUrl())
        fetcher.finished.connect(self.contentLoaded)
        while not self.loaded:
            app.processEvents()

        r = fetcher.reply()
        assert r.error() != QNetworkReply.NetworkError.NoError

    def testFetchBadUrl(self):
        fetcher = QgsNetworkContentFetcher()
        self.loaded = False
        fetcher.fetchContent(QUrl("http://x"))
        fetcher.finished.connect(self.contentLoaded)
        while not self.loaded:
            app.processEvents()

        r = fetcher.reply()
        assert r.error() != QNetworkReply.NetworkError.NoError

    def testFetchUrlContent(self):
        fetcher = QgsNetworkContentFetcher()
        self.loaded = False
        fetcher.fetchContent(
            QUrl(
                "http://localhost:"
                + str(TestQgsNetworkContentFetcher.port)
                + "/qgis_local_server/index.html"
            )
        )
        fetcher.finished.connect(self.contentLoaded)
        while not self.loaded:
            app.processEvents()

        r = fetcher.reply()
        assert r.error() == QNetworkReply.NetworkError.NoError, r.error()

        html = fetcher.contentAsString()
        assert "QGIS" in html

    def testFetchRequestContent(self):
        fetcher = QgsNetworkContentFetcher()
        self.loaded = False
        request = QNetworkRequest(
            QUrl(
                "http://localhost:"
                + str(TestQgsNetworkContentFetcher.port)
                + "/qgis_local_server/index.html"
            )
        )
        fetcher.fetchContent(request)
        fetcher.finished.connect(self.contentLoaded)
        while not self.loaded:
            app.processEvents()

        r = fetcher.reply()
        assert r.error() == QNetworkReply.NetworkError.NoError, r.error()

        html = fetcher.contentAsString()
        assert "QGIS" in html

    def testDoubleFetch(self):
        fetcher = QgsNetworkContentFetcher()
        self.loaded = False
        fetcher.fetchContent(QUrl("http://www.qgis.org/"))
        # double fetch - this should happen before previous request finishes
        fetcher.fetchContent(
            QUrl(
                "http://localhost:"
                + str(TestQgsNetworkContentFetcher.port)
                + "/qgis_local_server/index.html"
            )
        )
        fetcher.finished.connect(self.contentLoaded)
        while not self.loaded:
            app.processEvents()

        r = fetcher.reply()
        assert r.error() == QNetworkReply.NetworkError.NoError, r.error()

        html = fetcher.contentAsString()
        assert "QGIS" in html

    def testFetchEncodedContent(self):
        fetcher = QgsNetworkContentFetcher()
        self.loaded = False
        fetcher.fetchContent(
            QUrl(
                "http://localhost:"
                + str(TestQgsNetworkContentFetcher.port)
                + "/encoded_html.html"
            )
        )
        fetcher.finished.connect(self.contentLoaded)
        while not self.loaded:
            app.processEvents()

        r = fetcher.reply()
        assert r.error() == QNetworkReply.NetworkError.NoError, r.error()

        html = fetcher.contentAsString()
        assert chr(6040) in html


if __name__ == "__main__":
    unittest.main()
