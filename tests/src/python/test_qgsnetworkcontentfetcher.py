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
import unittest

from qgis.core import QgsApplication, QgsAuthMethodConfig, QgsNetworkContentFetcher
from qgis.PyQt.QtCore import QUrl
from qgis.PyQt.QtNetwork import QNetworkReply, QNetworkRequest
from qgis.testing import QgisTestCase, start_app
from utilities import unitTestDataPath

app = start_app()


class SmartRedirectHandler(http.server.SimpleHTTPRequestHandler):
    REDIRECTS = {
        "/redir_same_host": ("http://localhost/qgis_local_server/index.html", 301),
        "/redir_different_host": ("http://example.com/new_path", 302),
    }

    def do_GET(self):
        if self.path in self.REDIRECTS:
            target, code = self.REDIRECTS[self.path]
            self.send_response(code)
            self.send_header("Location", target)
            self.end_headers()
        else:
            super().do_GET()


class TestQgsNetworkContentFetcher(QgisTestCase):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        # Bring up a simple HTTP server
        os.chdir(unitTestDataPath() + "")
        handler = SmartRedirectHandler

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

    def testAuthConfig(self):
        authm = QgsApplication.authManager()
        auth_config = QgsAuthMethodConfig("APIHeader")
        auth_config.setConfig("key", "value")
        auth_config.setName("test_header_config")
        self.assertTrue(authm.storeAuthenticationConfig(auth_config)[0])
        self.assertTrue(auth_config.isValid())
        authcfg = auth_config.id()

        fetcher = QgsNetworkContentFetcher()
        self.loaded = False
        request = QNetworkRequest(
            QUrl(
                "http://localhost:"
                + str(TestQgsNetworkContentFetcher.port)
                + "/qgis_local_server/index.html"
            )
        )
        fetcher.fetchContent(request, authcfg)
        fetcher.finished.connect(self.contentLoaded)
        while not self.loaded:
            app.processEvents()

        r = fetcher.reply()
        rreq = r.request()
        self.assertEqual(rreq.rawHeader("key"), "value")

    def testAuthConfigRedirectSameHost(self):
        authm = QgsApplication.authManager()
        auth_config = QgsAuthMethodConfig("APIHeader")
        auth_config.setConfig("key", "value")
        auth_config.setName("test_header_config")
        self.assertTrue(authm.storeAuthenticationConfig(auth_config)[0])
        self.assertTrue(auth_config.isValid())
        authcfg = auth_config.id()

        fetcher = QgsNetworkContentFetcher()
        self.loaded = False
        request = QNetworkRequest(
            QUrl(
                "http://localhost:"
                + str(TestQgsNetworkContentFetcher.port)
                + "/redir_same_host"
            )
        )
        fetcher.fetchContent(request, authcfg)
        fetcher.finished.connect(self.contentLoaded)
        while not self.loaded:
            app.processEvents()

        r = fetcher.reply()
        rreq = r.request()
        self.assertEqual(rreq.url().host(), request.url().host())
        self.assertEqual(rreq.url().path(), "/qgis_local_server/index.html")
        self.assertEqual(rreq.rawHeader("key"), "value")

    def testAuthConfigRedirectDifferentHost(self):
        authm = QgsApplication.authManager()
        auth_config = QgsAuthMethodConfig("APIHeader")
        auth_config.setConfig("key", "value")
        auth_config.setName("test_header_config")
        self.assertTrue(authm.storeAuthenticationConfig(auth_config)[0])
        self.assertTrue(auth_config.isValid())
        authcfg = auth_config.id()

        fetcher = QgsNetworkContentFetcher()
        self.loaded = False
        request = QNetworkRequest(
            QUrl(
                "http://localhost:"
                + str(TestQgsNetworkContentFetcher.port)
                + "/redir_different_host"
            )
        )
        fetcher.fetchContent(request, authcfg)
        fetcher.finished.connect(self.contentLoaded)
        while not self.loaded:
            app.processEvents()

        r = fetcher.reply()
        rreq = r.request()
        self.assertNotEqual(rreq.url().host(), request.url().host())
        self.assertFalse(rreq.hasRawHeader("key"))


if __name__ == "__main__":
    unittest.main()
