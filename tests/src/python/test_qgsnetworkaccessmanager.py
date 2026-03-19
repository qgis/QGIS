"""QGIS Unit tests for QgsNetworkAccessManager.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2022 by Nyall Dawson"
__date__ = "27/04/2022"
__copyright__ = "Copyright 2022, The QGIS Project"

import http.server
import os
import random
import socketserver
import string
import threading
import unittest
from functools import partial

from qgis.core import (
    QgsNetworkAccessManager,
)
from qgis.PyQt.QtCore import QCoreApplication, QEvent, QUrl
from qgis.PyQt.QtNetwork import QNetworkAccessManager, QNetworkReply, QNetworkRequest
from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import QgisTestCase, start_app
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class MockServerRequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        content = b"response"
        if self.path.startswith("/no-cache"):
            if self.path == "/no-cache-same-etag":
                etag = 'W/"1111"'
            else:
                etag = (
                    'W/"'
                    + "".join(random.choice(string.ascii_lowercase) for i in range(20))
                    + '"'
                )
            if self.headers.get("If-None-Match") == etag:
                self.send_response(304)  # 304 Not Modified
                self.send_header("Cache-Control", "no-cache")
                self.send_header("ETag", etag)
                self.end_headers()
                return  # Stop here! Do not write the body.

            self.send_response(200)
            self.send_header("Cache-Control", "no-cache")
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("ETag", etag)
            self.send_header("Content-Length", str(len(content)))
            self.end_headers()
            self.wfile.write(content)
        elif self.path == "/no-store":
            self.send_response(200)
            self.send_header("Cache-Control", "no-store")
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(content)))
            self.end_headers()
            self.wfile.write(content)
        elif self.path == "/cache":
            self.send_response(200)
            self.send_header("Cache-Control", "max-age=604800")
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(content)))
            self.end_headers()
            self.wfile.write(content)
        elif self.path.startswith("/vary-"):
            vary_by = self.path[len("/vary-") :].replace("_", " ")
            self.send_response(200)
            self.send_header("Cache-Control", "max-age=604800")
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(content)))
            self.send_header("Vary", vary_by)
            self.end_headers()
            self.wfile.write(content)
        else:
            # Fallback to standard behavior for other files like index.html
            super().do_GET()


class TestQgsNetworkAccessManager(QgisTestCase):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        # Bring up a simple HTTP server
        os.chdir(unitTestDataPath() + "")

        handler = MockServerRequestHandler

        cls.httpd = socketserver.TCPServer(("localhost", 0), handler)
        cls.port = cls.httpd.server_address[1]

        cls.httpd_thread = threading.Thread(target=cls.httpd.serve_forever)
        cls.httpd_thread.daemon = True
        cls.httpd_thread.start()

    def test_request_preprocessor(self):
        """Test request preprocessor."""
        url = (
            "http://localhost:"
            + str(TestQgsNetworkAccessManager.port)
            + "/qgis_local_server/index.html"
        )

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
        self.assertEqual(bytes(_bytes).decode()[:14], "<!DOCTYPE html")
        TestQgsNetworkAccessManager.peeked = True

    def test_response_preprocessor(self):
        """Test response preprocessor."""
        url = (
            "http://localhost:"
            + str(TestQgsNetworkAccessManager.port)
            + "/qgis_local_server/index.html"
        )

        TestQgsNetworkAccessManager.preprocessed = False
        TestQgsNetworkAccessManager.peeked = False

        def _preprocessor(request, reply):
            self.assertIsInstance(request, QNetworkRequest)
            self.assertIsInstance(reply, QNetworkReply)
            self.assertEqual(request.url(), QUrl(url))

            self.assertTrue(
                reply.readyRead.connect(partial(self._on_reply_ready_read, reply))
            )

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

    def test_default_port_removal(self):
        """
        Test removal of redundant default ports from hosts

        See https://github.com/qgis/QGIS/issues/29475
        """

        # HTTP request with redundant default port 80
        request = QNetworkRequest(
            QUrl("http://testhost.com:80/qgis_local_server/index.html")
        )
        self.assertEqual(
            request.url().toString(),
            "http://testhost.com:80/qgis_local_server/index.html",
        )

        reply = QgsNetworkAccessManager.instance().createRequest(
            QNetworkAccessManager.Operation.GetOperation, request, None
        )
        # default port should be removed in actual used request
        self.assertEqual(
            reply.request().url().toString(),
            "http://testhost.com/qgis_local_server/index.html",
        )
        reply.deleteLater()
        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)

        # HTTP request with non-default port 8080
        request = QNetworkRequest(
            QUrl("http://testhost.com:8080/qgis_local_server/index.html")
        )
        self.assertEqual(
            request.url().toString(),
            "http://testhost.com:8080/qgis_local_server/index.html",
        )

        reply = QgsNetworkAccessManager.instance().createRequest(
            QNetworkAccessManager.Operation.GetOperation, request, None
        )
        self.assertEqual(
            reply.request().url().toString(),
            "http://testhost.com:8080/qgis_local_server/index.html",
        )
        reply.deleteLater()
        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)

        # HTTPS request with redundant default port 443
        request = QNetworkRequest(
            QUrl("https://testhost.com:443/qgis_local_server/index.html")
        )
        self.assertEqual(
            request.url().toString(),
            "https://testhost.com:443/qgis_local_server/index.html",
        )

        reply = QgsNetworkAccessManager.instance().createRequest(
            QNetworkAccessManager.Operation.GetOperation, request, None
        )
        # default port should be removed in actual used request
        self.assertEqual(
            reply.request().url().toString(),
            "https://testhost.com/qgis_local_server/index.html",
        )
        reply.deleteLater()
        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)

        # HTTPS request with non-default port 8080
        request = QNetworkRequest(
            QUrl("https://testhost.com:8080/qgis_local_server/index.html")
        )
        self.assertEqual(
            request.url().toString(),
            "https://testhost.com:8080/qgis_local_server/index.html",
        )

        reply = QgsNetworkAccessManager.instance().createRequest(
            QNetworkAccessManager.Operation.GetOperation, request, None
        )
        self.assertEqual(
            reply.request().url().toString(),
            "https://testhost.com:8080/qgis_local_server/index.html",
        )
        reply.deleteLater()
        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)

    def test_cache_control_allow_cache(self):
        """
        Test caching of a reply which allows it
        """
        url = f"http://localhost:{TestQgsNetworkAccessManager.port}/cache"

        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        reply = QgsNetworkAccessManager.instance().get(request)
        spy = QSignalSpy(reply.finished)
        spy.wait(1000)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

        # try again, should definitely be cached
        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        reply = QgsNetworkAccessManager.instance().get(request)
        spy = QSignalSpy(reply.finished)
        spy.wait(1000)
        self.assertTrue(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

    def test_cache_control_no_cache_same_etag(self):
        """
        Test caching of a reply with no-cache attribute, matching etag on second request
        """
        url = f"http://localhost:{TestQgsNetworkAccessManager.port}/no-cache-same-etag"

        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

        # try again
        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        # second request CAN use cached version, the server resource etag is identical
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertTrue(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

    def test_cache_control_no_cache_different_etag(self):
        """
        Test caching of a reply with no-cache attribute, different etag on second request
        """
        url = f"http://localhost:{TestQgsNetworkAccessManager.port}/no-cache-different-etag"

        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

        # try again, should still not be cached
        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        # second request CANNOT use cached version, the server resource etag is different
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

    def test_cache_control_no_store(self):
        """
        Test caching of a reply with no-store attribute
        """
        url = f"http://localhost:{TestQgsNetworkAccessManager.port}/no-store"

        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

        # try again, should still not be cached
        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        # second request CANNOT use cached version, the response had "no-store" cache control
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

    def test_cache_control_allow_cache(self):
        """
        Test caching of a reply which allows it
        """
        QgsNetworkAccessManager.instance().cache().clear()

        url = f"http://localhost:{TestQgsNetworkAccessManager.port}/cache"

        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        reply = QgsNetworkAccessManager.instance().get(request)
        spy = QSignalSpy(reply.finished)
        spy.wait(1000)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

        # try again, should definitely be cached
        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        reply = QgsNetworkAccessManager.instance().get(request)
        spy = QSignalSpy(reply.finished)
        spy.wait(1000)
        self.assertTrue(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

    def test_cache_control_no_cache_same_etag(self):
        """
        Test caching of a reply with no-cache attribute, matching etag on second request
        """
        QgsNetworkAccessManager.instance().cache().clear()

        url = f"http://localhost:{TestQgsNetworkAccessManager.port}/no-cache-same-etag"

        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

        # try again
        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        # second request CAN use cached version, the server resource etag is identical
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertTrue(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

    def test_cache_control_no_cache_different_etag(self):
        """
        Test caching of a reply with no-cache attribute, different etag on second request
        """
        QgsNetworkAccessManager.instance().cache().clear()

        url = f"http://localhost:{TestQgsNetworkAccessManager.port}/no-cache-different-etag"

        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

        # try again, should still not be cached
        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        # second request CANNOT use cached version, the server resource etag is different
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

    def test_cache_control_no_store(self):
        """
        Test caching of a reply with no-store attribute
        """
        QgsNetworkAccessManager.instance().cache().clear()

        url = f"http://localhost:{TestQgsNetworkAccessManager.port}/no-store"

        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

        # try again, should still not be cached
        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        # second request CANNOT use cached version, the response had "no-store" cache control
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

    def test_cache_control_vary_authorization(self):
        """
        Test caching of a reply with Vary: Authorization attribute
        """
        QgsNetworkAccessManager.instance().cache().clear()

        url = f"http://localhost:{TestQgsNetworkAccessManager.port}/vary-Authorization"

        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )
        self.assertEqual(reply.rawHeader(b"Vary"), b"Authorization")

        # try again, should be cached, because Authorization header is same (unset)
        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertTrue(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

        # use a different Authorization header, now we must not use the cached response
        request.setRawHeader(b"Authorization", b"Bearer: mytoken")
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )
        # but should have been stored in the cache, evicting the original vary response
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertTrue(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

        # original response should have been evicted from cache, but in turn
        # evict the Bearer response from the cache
        request.setRawHeader(b"Authorization", b"")
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertTrue(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

        # if a new request with a different Authorization header is set NOT to
        # save the result in cache, then we shouldn't evict the previous
        # non-matching response
        request.setRawHeader(b"Authorization", b"Bearer: mytoken")
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, False)
        # can't use blockingGet here -- that always sets CacheSaveControlAttribute to True!
        reply = QgsNetworkAccessManager.instance().get(request)
        wait_spy = QSignalSpy(reply.finished)
        wait_spy.wait()
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )
        reply = QgsNetworkAccessManager.instance().get(request)
        wait_spy = QSignalSpy(reply.finished)
        wait_spy.wait()
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )
        # we shouldn't have evicted this reply, can still retrieve it from cache
        request.setRawHeader(b"Authorization", b"")
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertTrue(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

    def test_cache_control_vary_accept(self):
        """
        Test caching of a reply with Vary: Accept attribute
        """
        QgsNetworkAccessManager.instance().cache().clear()

        url = f"http://localhost:{TestQgsNetworkAccessManager.port}/vary-Accept"

        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)
        request.setRawHeader(b"Accept", b"application/json")

        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )
        self.assertEqual(reply.rawHeader(b"Vary"), b"Accept")

        # try again, should be cached, because Accept header is same
        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setRawHeader(b"Accept", b"application/json")
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertTrue(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

        # use a different Accept header, now we must not use the cached response
        request.setRawHeader(b"accept", b"application/xml")
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

    def test_cache_control_vary_star(self):
        """
        Test caching of a reply with Vary: * attribute
        """
        QgsNetworkAccessManager.instance().cache().clear()

        url = f"http://localhost:{TestQgsNetworkAccessManager.port}/vary-*"

        request = QNetworkRequest(QUrl(url))
        request.setAttribute(
            QNetworkRequest.Attribute.CacheLoadControlAttribute,
            QNetworkRequest.CacheLoadControl.PreferCache,
        )
        request.setAttribute(QNetworkRequest.Attribute.CacheSaveControlAttribute, True)

        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )
        self.assertEqual(reply.rawHeader(b"Vary"), b"*")

        # try again, should NOT be cached
        request = QNetworkRequest(QUrl(url))
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )

        # use a different Authorization header, still must not use the cached response
        request.setRawHeader(b"Authorization", b"Bearer: mytoken")
        reply = QgsNetworkAccessManager.instance().blockingGet(request)
        self.assertFalse(
            reply.attribute(QNetworkRequest.Attribute.SourceIsFromCacheAttribute)
        )


if __name__ == "__main__":
    unittest.main()
