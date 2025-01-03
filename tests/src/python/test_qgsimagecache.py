"""QGIS Unit tests for QgsImageCache.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2018 by Nyall Dawson"
__date__ = "02/10/2018"
__copyright__ = "Copyright 2018, The QGIS Project"

import http.server
import os
import socketserver
import threading
import time

from qgis.PyQt.QtCore import QCoreApplication, QSize
from qgis.core import QgsApplication
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class SlowHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):

    def do_GET(self):
        time.sleep(1)
        return http.server.SimpleHTTPRequestHandler.do_GET(self)


class TestQgsImageCache(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "image_cache"

    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        # Bring up a simple HTTP server, for remote SVG tests
        os.chdir(unitTestDataPath() + "")
        handler = SlowHTTPRequestHandler

        cls.httpd = socketserver.TCPServer(("localhost", 0), handler)
        cls.port = cls.httpd.server_address[1]

        cls.httpd_thread = threading.Thread(target=cls.httpd.serve_forever)
        cls.httpd_thread.daemon = True
        cls.httpd_thread.start()

    def setUp(self):
        self.fetched = False
        QgsApplication.imageCache().remoteImageFetched.connect(self.imageFetched)

    def imageFetched(self):
        self.fetched = True

    def waitForFetch(self):
        self.fetched = False
        while not self.fetched:
            QCoreApplication.processEvents()

    def testRemoteImage(self):
        """Test fetching remote image."""
        url = f"http://localhost:{str(TestQgsImageCache.port)}/qgis_local_server/sample_image.png"
        image, in_cache = QgsApplication.imageCache().pathAsImage(
            url, QSize(100, 100), True, 1.0
        )

        # first should be waiting image
        self.assertTrue(
            self.image_check(
                "Remote Image", "waiting_image", image, use_checkerboard_background=True
            )
        )
        self.assertFalse(QgsApplication.imageCache().originalSize(url).isValid())
        self.waitForFetch()

        # second should be correct image
        image, in_cache = QgsApplication.imageCache().pathAsImage(
            url, QSize(100, 100), True, 1.0
        )

        self.assertTrue(
            self.image_check(
                "Remote Image", "remote_image", image, use_checkerboard_background=True
            )
        )
        self.assertEqual(
            QgsApplication.imageCache().originalSize(url), QSize(511, 800), 1.0
        )

    def testRemoteImageMissing(self):
        """Test fetching remote image with bad url"""
        url = f"http://localhost:{str(TestQgsImageCache.port)}/qgis_local_server/xxx.png"  # oooo naughty
        image, in_cache = QgsApplication.imageCache().pathAsImage(
            url, QSize(100, 100), True, 1.0
        )

        self.assertTrue(
            self.image_check(
                "Remote image missing",
                "waiting_image",
                image,
                use_checkerboard_background=True,
            )
        )

    def testRemoteImageBlocking(self):
        """Test fetching remote image."""
        # remote not yet requested so not in cache
        url = f"http://localhost:{str(TestQgsImageCache.port)}/qgis_local_server/logo_2017.png"
        image, in_cache = QgsApplication.imageCache().pathAsImage(
            url, QSize(100, 100), True, 1.0, blocking=1
        )

        # first should be correct image
        self.assertTrue(
            self.image_check(
                "Remote image sync",
                "remote_image_blocking",
                image,
                use_checkerboard_background=True,
            )
        )

        # remote probably in cache
        url = f"http://localhost:{str(TestQgsImageCache.port)}/qgis_local_server/sample_image.png"
        image, in_cache = QgsApplication.imageCache().pathAsImage(
            url, QSize(100, 100), True, 1.0, blocking=1
        )

        self.assertTrue(
            self.image_check(
                "Remote Image", "remote_image", image, use_checkerboard_background=True
            )
        )

        # remote probably in cache
        url = f"http://localhost:{str(TestQgsImageCache.port)}/qgis_local_server/xxx.png"  # oooo naughty
        image, in_cache = QgsApplication.imageCache().pathAsImage(
            url, QSize(100, 100), True, 1.0, blocking=1
        )

        self.assertTrue(
            self.image_check(
                "Remote image missing",
                "waiting_image",
                image,
                use_checkerboard_background=True,
            )
        )


if __name__ == "__main__":
    unittest.main()
