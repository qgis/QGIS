# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsNetworkContentFetcherTask

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from builtins import chr
from builtins import str

__author__ = 'Nyall Dawson'
__date__ = '29/03/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA

import os
from qgis.testing import unittest, start_app
from qgis.core import QgsNetworkContentFetcher, QgsNetworkContentFetcherTask, QgsApplication
from utilities import unitTestDataPath
from qgis.PyQt.QtCore import QUrl
from qgis.PyQt.QtNetwork import QNetworkReply, QNetworkRequest
import socketserver
import threading
import http.server

app = start_app()


class TestQgsNetworkContentFetcherTask(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # Bring up a simple HTTP server
        os.chdir(unitTestDataPath() + '')
        handler = http.server.SimpleHTTPRequestHandler

        cls.httpd = socketserver.TCPServer(('localhost', 0), handler)
        cls.port = cls.httpd.server_address[1]

        cls.httpd_thread = threading.Thread(target=cls.httpd.serve_forever)
        cls.httpd_thread.daemon = True
        cls.httpd_thread.start()

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

        self.loaded = False

    def contentLoaded(self):
        self.loaded = True

    def testFetchBadUrl(self):
        fetcher = QgsNetworkContentFetcherTask(QUrl('http://x'))
        self.loaded = False

        def check_reply():
            r = fetcher.reply()
            assert r.error() != QNetworkReply.NoError
            self.loaded = True

        fetcher.fetched.connect(check_reply)
        QgsApplication.taskManager().addTask(fetcher)
        while not self.loaded:
            app.processEvents()

    def testFetchUrlContent(self):
        fetcher = QgsNetworkContentFetcherTask(
            QUrl('http://localhost:' + str(self.port) + '/qgis_local_server/index.html'))
        self.loaded = False

        def check_reply():
            r = fetcher.reply()
            assert r.error() == QNetworkReply.NoError, r.error()

            assert b'QGIS' in r.readAll()
            self.loaded = True

        fetcher.fetched.connect(check_reply)
        QgsApplication.taskManager().addTask(fetcher)
        while not self.loaded:
            app.processEvents()


if __name__ == "__main__":
    unittest.main()
