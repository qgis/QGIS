# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsNetworkAccessManager.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2022 by Nyall Dawson'
__date__ = '27/04/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA

import os
import socketserver
import threading
import http.server
from functools import partial

from qgis.PyQt.QtNetwork import (
    QNetworkRequest,
    QNetworkReply
)
from qgis.PyQt.QtCore import (
    QUrl,
    QCoreApplication
)
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (QgsNetworkAccessManager,
                       QgsLayout,
                       QgsLayoutItemMap,
                       QgsRectangle,
                       QgsCoordinateReferenceSystem,
                       QgsProject,
                       QgsReadWriteContext
                       )
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgslayoutchecker import QgsLayoutChecker

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsNetworkAccessManager(unittest.TestCase):

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

    def test_request_preprocessor(self):
        """Test request preprocessor."""
        url = 'http://localhost:' + str(TestQgsNetworkAccessManager.port) + '/qgis_local_server/index.html'

        TestQgsNetworkAccessManager.preprocessed = False

        def _preprocessor(request):
            self.assertIsInstance(request, QNetworkRequest)
            self.assertEqual(request.url(), QUrl(url))
            TestQgsNetworkAccessManager.preprocessed = True

        _id = QgsNetworkAccessManager.setRequestPreprocessor(_preprocessor)

        reply = QgsNetworkAccessManager.instance().get(QNetworkRequest(QUrl(url)))
        spy = QSignalSpy(reply.finished)
        spy.wait(1000)

        self.assertTrue(TestQgsNetworkAccessManager.preprocessed)

        # test a second time
        TestQgsNetworkAccessManager.preprocessed = False
        reply = QgsNetworkAccessManager.instance().get(QNetworkRequest(QUrl(url)))
        self.assertTrue(TestQgsNetworkAccessManager.preprocessed)
        spy = QSignalSpy(reply.finished)
        spy.wait(1000)

        # remove preprocessor and ensure that it's no longer called
        QgsNetworkAccessManager.removeRequestPreprocessor(_id)
        TestQgsNetworkAccessManager.preprocessed = False
        reply = QgsNetworkAccessManager.instance().get(QNetworkRequest(QUrl(url)))
        spy = QSignalSpy(reply.finished)
        spy.wait(1000)

        self.assertFalse(TestQgsNetworkAccessManager.preprocessed)

        # no longer exists, so a key error should be raised
        with self.assertRaises(KeyError):
            QgsNetworkAccessManager.removeRequestPreprocessor(_id)

    def _on_reply_ready_read(self, reply):
        _bytes = reply.peek(reply.bytesAvailable())
        self.assertEqual(_bytes.data().decode()[:14], '<!DOCTYPE html')
        TestQgsNetworkAccessManager.peeked = True

    def test_response_preprocessor(self):
        """Test response preprocessor."""
        url = 'http://localhost:' + str(TestQgsNetworkAccessManager.port) + '/qgis_local_server/index.html'

        TestQgsNetworkAccessManager.preprocessed = False
        TestQgsNetworkAccessManager.peeked = False

        def _preprocessor(request, reply):
            self.assertIsInstance(request, QNetworkRequest)
            self.assertIsInstance(reply, QNetworkReply)
            self.assertEqual(request.url(), QUrl(url))

            self.assertTrue(reply.readyRead.connect(partial(self._on_reply_ready_read, reply)))

            TestQgsNetworkAccessManager.preprocessed = True

        _id = QgsNetworkAccessManager.setReplyPreprocessor(_preprocessor)

        reply = QgsNetworkAccessManager.instance().get(QNetworkRequest(QUrl(url)))
        spy = QSignalSpy(reply.readyRead)
        spy.wait(1000)

        self.assertTrue(TestQgsNetworkAccessManager.preprocessed)
        self.assertTrue(TestQgsNetworkAccessManager.peeked)

        # test a second time
        TestQgsNetworkAccessManager.preprocessed = False
        reply = QgsNetworkAccessManager.instance().get(QNetworkRequest(QUrl(url)))
        self.assertTrue(TestQgsNetworkAccessManager.preprocessed)
        spy = QSignalSpy(reply.readyRead)
        spy.wait(1000)

        # remove preprocessor and ensure that it's no longer called
        QgsNetworkAccessManager.removeReplyPreprocessor(_id)
        TestQgsNetworkAccessManager.preprocessed = False
        TestQgsNetworkAccessManager.peeked = False
        reply = QgsNetworkAccessManager.instance().get(QNetworkRequest(QUrl(url)))
        spy = QSignalSpy(reply.readyRead)
        spy.wait(1000)

        self.assertFalse(TestQgsNetworkAccessManager.preprocessed)
        self.assertFalse(TestQgsNetworkAccessManager.peeked)

        # no longer exists, so a key error should be raised
        with self.assertRaises(KeyError):
            QgsNetworkAccessManager.removeReplyPreprocessor(_id)


if __name__ == '__main__':
    unittest.main()
