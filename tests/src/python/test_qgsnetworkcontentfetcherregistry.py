# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsNetworkContentFetcherRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from builtins import chr
from builtins import str

__author__ = 'Denis Rouzaud'
__date__ = '27/04/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA

import os
from qgis.testing import unittest, start_app
from qgis.core import QgsNetworkContentFetcherRegistry, QgsFetchedContent, QgsApplication
from utilities import unitTestDataPath
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
        self.file_content = ''

    def testFetchBadUrl(self):
        registry = QgsApplication.networkContentFetcherRegistry()
        content = registry.fetch('http://x')
        self.loaded = False

        def check_reply():
            self.assertEqual(content.status(), QgsFetchedContent.Failed)
            self.assertNotEqual(content.error(), QNetworkReply.NoError)
            self.assertEqual(content.filePath(), '')
            self.loaded = True

        content.fetched.connect(check_reply)
        content.download()
        while not self.loaded:
            app.processEvents()

    def testFetchGoodUrl(self):
        url = 'http://localhost:' + str(self.port) + '/qgis_local_server/index.html'
        registry = QgsApplication.networkContentFetcherRegistry()
        content = registry.fetch(url)
        self.loaded = False

        def check_reply():
            self.loaded = True
            self.assertEqual(content.status(), QgsFetchedContent.Finished)
            self.assertEqual(content.error(), QNetworkReply.NoError)
            self.assertNotEqual(content.filePath(), '')

        content.fetched.connect(check_reply)
        content.download()
        while not self.loaded:
            app.processEvents()

        self.assertEqual(registry.localPath(url), content.filePath())

        # create new content with same URL
        contentV2 = registry.fetch(url)
        self.assertEqual(contentV2.status(), QgsFetchedContent.Finished)

    def testFetchReloadUrl(self):
        def writeSimpleFile(content):
            with open('qgis_local_server/simple_content.txt', 'w') as f:
                f.write(content)
            self.file_content = content

        registry = QgsApplication.networkContentFetcherRegistry()
        content = registry.fetch('http://localhost:' + str(self.port) + '/qgis_local_server/simple_content.txt')
        self.loaded = False
        writeSimpleFile('my initial content')

        def check_reply():
            self.loaded = True
            self.assertEqual(content.status(), QgsFetchedContent.Finished)
            self.assertEqual(content.error(), QNetworkReply.NoError)
            self.assertNotEqual(content.filePath(), '')
            with open(content.filePath(), encoding="utf-8") as file:
                self.assertEqual(file.readline().rstrip(), self.file_content)

        content.fetched.connect(check_reply)
        content.download()
        while not self.loaded:
            app.processEvents()

        writeSimpleFile('my second content')
        content.download()
        with open(content.filePath(), encoding="utf-8") as file:
            self.assertNotEqual(file.readline().rstrip(), self.file_content)

        content.download(True)
        while not self.loaded:
            app.processEvents()

        os.remove('qgis_local_server/simple_content.txt')

    def testLocalPath(self):
        registry = QgsApplication.networkContentFetcherRegistry()
        filePath = 'qgis_local_server/index.html'
        self.assertEqual(registry.localPath(filePath), filePath)

        # a non existent download shall return untouched the path
        self.assertEqual(registry.localPath('xxxx'), 'xxxx')

        # an existent but unfinished download should return an empty path
        content = registry.fetch('xxxx')
        self.assertEqual(registry.localPath('xxxx'), '')


if __name__ == "__main__":
    unittest.main()
