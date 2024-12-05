"""QGIS Unit tests for QgsBlockingNetworkRequest

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "12/11/2018"
__copyright__ = "Copyright 2018, The QGIS Project"

from qgis.PyQt.QtCore import QUrl
from qgis.PyQt.QtNetwork import QNetworkReply, QNetworkRequest
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import QgsBlockingNetworkRequest
import unittest
from qgis.testing import start_app, QgisTestCase

import mockedwebserver

app = start_app()


class TestQgsBlockingNetworkRequest(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.server, cls.port = mockedwebserver.launch()

    @classmethod
    def tearDownClass(cls):
        cls.server.stop()
        super().tearDownClass()

    def testFetchEmptyUrl(self):
        request = QgsBlockingNetworkRequest()
        spy = QSignalSpy(request.finished)
        err = request.get(QNetworkRequest(QUrl()))
        self.assertEqual(len(spy), 1)
        self.assertEqual(err, QgsBlockingNetworkRequest.ErrorCode.ServerExceptionError)
        self.assertEqual(request.errorMessage(), 'Protocol "" is unknown')
        reply = request.reply()
        self.assertFalse(reply.content())

    def testFetchBadUrl(self):
        request = QgsBlockingNetworkRequest()
        spy = QSignalSpy(request.finished)
        err = request.get(QNetworkRequest(QUrl("http://x")))
        self.assertEqual(len(spy), 1)
        self.assertEqual(err, QgsBlockingNetworkRequest.ErrorCode.ServerExceptionError)
        self.assertEqual(request.errorMessage(), "Host x not found")
        reply = request.reply()
        self.assertFalse(reply.content())

    def testFetchBadUrl2(self):
        request = QgsBlockingNetworkRequest()
        spy = QSignalSpy(request.finished)

        handler = mockedwebserver.SequentialHandler()
        handler.add(
            "GET",
            "/ffff",
            404,
            {},
            '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"\n        "http://www.w3.org/TR/html4/strict.dtd">\n<html>\n    <head>\n        <meta http-equiv="Content-Type" content="text/html;charset=utf-8">\n        <title>Error response</title>\n    </head>\n    <body>\n        <h1>Error response</h1>\n        <p>Error code: 404</p>\n        <p>Message: File not found.</p>\n        <p>Error code explanation: HTTPStatus.NOT_FOUND - Nothing matches the given URI.</p>\n    </body>\n</html>\n',
        )
        with mockedwebserver.install_http_handler(handler):
            err = request.get(
                QNetworkRequest(
                    QUrl(
                        "http://localhost:"
                        + str(TestQgsBlockingNetworkRequest.port)
                        + "/ffff"
                    )
                )
            )
        self.assertEqual(len(spy), 1)
        self.assertEqual(err, QgsBlockingNetworkRequest.ErrorCode.ServerExceptionError)
        self.assertIn("Not Found", request.errorMessage())
        reply = request.reply()
        self.assertEqual(reply.error(), QNetworkReply.NetworkError.ContentNotFoundError)
        self.assertEqual(
            reply.content(),
            '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"\n        "http://www.w3.org/TR/html4/strict.dtd">\n<html>\n    <head>\n        <meta http-equiv="Content-Type" content="text/html;charset=utf-8">\n        <title>Error response</title>\n    </head>\n    <body>\n        <h1>Error response</h1>\n        <p>Error code: 404</p>\n        <p>Message: File not found.</p>\n        <p>Error code explanation: HTTPStatus.NOT_FOUND - Nothing matches the given URI.</p>\n    </body>\n</html>\n',
        )

    def testGet(self):
        request = QgsBlockingNetworkRequest()
        spy = QSignalSpy(request.finished)
        handler = mockedwebserver.SequentialHandler()
        handler.add(
            "GET", "/test.html", 200, {"Content-type": "text/html"}, "<html></html>\n"
        )
        with mockedwebserver.install_http_handler(handler):
            err = request.get(
                QNetworkRequest(
                    QUrl(
                        "http://localhost:"
                        + str(TestQgsBlockingNetworkRequest.port)
                        + "/test.html"
                    )
                ),
                True,
            )
        self.assertEqual(len(spy), 1)
        self.assertEqual(err, QgsBlockingNetworkRequest.ErrorCode.NoError)
        self.assertEqual(request.errorMessage(), "")
        reply = request.reply()
        self.assertEqual(reply.error(), QNetworkReply.NetworkError.NoError)
        self.assertEqual(reply.content(), "<html></html>\n")
        # newer qt versions FORCE lowercase header keys, older ones didn't
        self.assertEqual(
            [h.data().decode().lower() for h in reply.rawHeaderList()],
            ["server", "date", "content-type", "content-length"],
        )
        self.assertEqual(reply.rawHeader(b"Content-type"), "text/html")
        self.assertEqual(reply.rawHeader(b"xxxxxxxxx"), "")
        self.assertEqual(
            reply.attribute(QNetworkRequest.Attribute.HttpStatusCodeAttribute), 200
        )
        self.assertEqual(
            reply.attribute(QNetworkRequest.Attribute.HttpReasonPhraseAttribute), "OK"
        )
        self.assertEqual(
            reply.attribute(QNetworkRequest.Attribute.RedirectionTargetAttribute), None
        )

    def testHead(self):
        request = QgsBlockingNetworkRequest()
        spy = QSignalSpy(request.finished)
        handler = mockedwebserver.SequentialHandler()
        handler.add("HEAD", "/test.html", 200, {"Content-type": "text/html"})
        with mockedwebserver.install_http_handler(handler):
            err = request.head(
                QNetworkRequest(
                    QUrl(
                        "http://localhost:"
                        + str(TestQgsBlockingNetworkRequest.port)
                        + "/test.html"
                    )
                ),
                True,
            )
        self.assertEqual(len(spy), 1)
        self.assertEqual(err, QgsBlockingNetworkRequest.ErrorCode.NoError)
        self.assertEqual(request.errorMessage(), "")

    def testPost(self):
        request = QgsBlockingNetworkRequest()
        spy = QSignalSpy(request.finished)
        handler = mockedwebserver.SequentialHandler()
        handler.add("POST", "/test.html", 200, expected_body=b"foo")
        with mockedwebserver.install_http_handler(handler):
            req = QNetworkRequest(
                QUrl(
                    "http://localhost:"
                    + str(TestQgsBlockingNetworkRequest.port)
                    + "/test.html"
                )
            )
            req.setHeader(QNetworkRequest.KnownHeaders.ContentTypeHeader, "text/plain")
            err = request.post(req, b"foo")
        self.assertEqual(err, QgsBlockingNetworkRequest.ErrorCode.NoError)
        self.assertEqual(request.errorMessage(), "")

    def testPut(self):
        request = QgsBlockingNetworkRequest()
        spy = QSignalSpy(request.finished)
        handler = mockedwebserver.SequentialHandler()
        handler.add("PUT", "/test.html", 200, expected_body=b"foo")
        with mockedwebserver.install_http_handler(handler):
            req = QNetworkRequest(
                QUrl(
                    "http://localhost:"
                    + str(TestQgsBlockingNetworkRequest.port)
                    + "/test.html"
                )
            )
            req.setHeader(QNetworkRequest.KnownHeaders.ContentTypeHeader, "text/plain")
            err = request.put(req, b"foo")
        self.assertEqual(len(spy), 1)
        self.assertEqual(err, QgsBlockingNetworkRequest.ErrorCode.NoError)
        self.assertEqual(request.errorMessage(), "")

    def testDelete(self):
        request = QgsBlockingNetworkRequest()
        spy = QSignalSpy(request.finished)
        handler = mockedwebserver.SequentialHandler()
        handler.add("DELETE", "/test.html", 200)
        with mockedwebserver.install_http_handler(handler):
            err = request.deleteResource(
                QNetworkRequest(
                    QUrl(
                        "http://localhost:"
                        + str(TestQgsBlockingNetworkRequest.port)
                        + "/test.html"
                    )
                )
            )
        self.assertEqual(len(spy), 1)
        self.assertEqual(err, QgsBlockingNetworkRequest.ErrorCode.NoError)
        self.assertEqual(request.errorMessage(), "")


if __name__ == "__main__":
    unittest.main()
