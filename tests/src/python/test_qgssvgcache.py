"""QGIS Unit tests for QgsSvgCache.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2018 by Nyall Dawson"
__date__ = "29/03/2018"
__copyright__ = "Copyright 2018, The QGIS Project"

import http.server
import os
import socketserver
import threading
import time

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtGui import QColor
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


class TestQgsSvgCache(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "svg_cache"

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
        self.fetched = True
        QgsApplication.svgCache().remoteSvgFetched.connect(self.svgFetched)

    def svgFetched(self):
        self.fetched = True

    def waitForFetch(self):
        self.fetched = False
        while not self.fetched:
            QCoreApplication.processEvents()

    def testRemoteSVG(self):
        """Test fetching remote svg."""
        url = f"http://localhost:{str(TestQgsSvgCache.port)}/qgis_local_server/sample_svg.svg"
        image, in_cache = QgsApplication.svgCache().svgAsImage(
            url,
            100,
            fill=QColor(0, 0, 0),
            stroke=QColor(0, 0, 0),
            strokeWidth=0.1,
            widthScaleFactor=1,
        )
        # first should be waiting image
        self.assertTrue(
            self.image_check(
                "Remote SVG",
                "waiting_svg",
                image,
                color_tolerance=2,
                allowed_mismatch=20,
                use_checkerboard_background=True,
            )
        )
        self.waitForFetch()

        # second should be correct image
        image, in_cache = QgsApplication.svgCache().svgAsImage(
            url,
            100,
            fill=QColor(0, 0, 0),
            stroke=QColor(0, 0, 0),
            strokeWidth=0.1,
            widthScaleFactor=1,
        )

        self.assertTrue(
            self.image_check(
                "Remote SVG",
                "remote_svg",
                image,
                color_tolerance=2,
                allowed_mismatch=20,
                use_checkerboard_background=True,
            )
        )

        for i in range(1000):
            QCoreApplication.processEvents()

    def testRemoteSvgAsText(self):
        """Test fetching remote svg with text mime format - e.g. github raw svgs"""
        url = f"http://localhost:{str(TestQgsSvgCache.port)}/qgis_local_server/svg_as_text.txt"
        image, in_cache = QgsApplication.svgCache().svgAsImage(
            url,
            100,
            fill=QColor(0, 0, 0),
            stroke=QColor(0, 0, 0),
            strokeWidth=0.1,
            widthScaleFactor=1,
        )
        # first should be waiting image
        self.assertTrue(
            self.image_check(
                "Remote SVG as Text",
                "waiting_svg",
                image,
                color_tolerance=2,
                allowed_mismatch=20,
                use_checkerboard_background=True,
            )
        )

        self.waitForFetch()
        # second should be correct image
        image, in_cache = QgsApplication.svgCache().svgAsImage(
            url,
            100,
            fill=QColor(0, 0, 0),
            stroke=QColor(0, 0, 0),
            strokeWidth=0.1,
            widthScaleFactor=1,
        )

        # first should be waiting image
        self.assertTrue(
            self.image_check(
                "Remote SVG as Text",
                "remote_svg",
                image,
                color_tolerance=2,
                allowed_mismatch=20,
                use_checkerboard_background=True,
            )
        )

        for i in range(1000):
            QCoreApplication.processEvents()

    def testRemoteSvgBadMime(self):
        """Test fetching remote svg with bad mime type"""
        url = f"http://localhost:{str(TestQgsSvgCache.port)}/qgis_local_server/logo.png"
        image, in_cache = QgsApplication.svgCache().svgAsImage(
            url,
            100,
            fill=QColor(0, 0, 0),
            stroke=QColor(0, 0, 0),
            strokeWidth=0.1,
            widthScaleFactor=1,
        )
        # first should be waiting image
        self.assertTrue(
            self.image_check(
                "Remote SVG bad MIME type",
                "waiting_svg",
                image,
                color_tolerance=2,
                allowed_mismatch=20,
                use_checkerboard_background=True,
            )
        )

        # second should be correct image
        self.waitForFetch()
        image, in_cache = QgsApplication.svgCache().svgAsImage(
            url,
            100,
            fill=QColor(0, 0, 0),
            stroke=QColor(0, 0, 0),
            strokeWidth=0.1,
            widthScaleFactor=1,
        )
        self.assertTrue(
            self.image_check(
                "Remote SVG bad MIME type",
                "bad_svg",
                image,
                color_tolerance=2,
                allowed_mismatch=20,
                use_checkerboard_background=True,
            )
        )

        for i in range(1000):
            QCoreApplication.processEvents()

    def testRemoteSvgMissing(self):
        """Test fetching remote svg with bad url"""
        url = f"http://localhost:{str(TestQgsSvgCache.port)}/qgis_local_server/xxx.svg"  # oooo naughty
        image, in_cache = QgsApplication.svgCache().svgAsImage(
            url,
            100,
            fill=QColor(0, 0, 0),
            stroke=QColor(0, 0, 0),
            strokeWidth=0.1,
            widthScaleFactor=1,
        )

        self.assertTrue(
            self.image_check(
                "Remote SVG missing",
                "waiting_svg",
                image,
                color_tolerance=2,
                allowed_mismatch=20,
                use_checkerboard_background=True,
            )
        )

        for i in range(1000):
            QCoreApplication.processEvents()

    def testRemoteSVGBlocking(self):
        """Test fetching remote svg."""
        # remote not yet requested so not in cache
        url = f"http://localhost:{str(TestQgsSvgCache.port)}/qgis_local_server/QGIS_logo_2017.svg"
        image, in_cache = QgsApplication.svgCache().svgAsImage(
            url,
            100,
            fill=QColor(0, 0, 0),
            stroke=QColor(0, 0, 0),
            strokeWidth=0.1,
            widthScaleFactor=1,
            blocking=1,
        )
        # first should be correct image
        self.assertTrue(
            self.image_check(
                "Remote SVG sync",
                "remote_svg_blocking",
                image,
                color_tolerance=2,
                allowed_mismatch=20,
                use_checkerboard_background=True,
            )
        )

        # remote probably in cache
        url = f"http://localhost:{str(TestQgsSvgCache.port)}/qgis_local_server/sample_svg.svg"
        image, in_cache = QgsApplication.svgCache().svgAsImage(
            url,
            100,
            fill=QColor(0, 0, 0),
            stroke=QColor(0, 0, 0),
            strokeWidth=0.1,
            widthScaleFactor=1,
            blocking=1,
        )

        self.assertTrue(
            self.image_check(
                "Remote SVG",
                "remote_svg",
                image,
                color_tolerance=2,
                allowed_mismatch=20,
                use_checkerboard_background=True,
            )
        )

        # missing
        url = f"http://localhost:{str(TestQgsSvgCache.port)}/qgis_local_server/xxx.svg"  # oooo naughty
        image, in_cache = QgsApplication.svgCache().svgAsImage(
            url,
            100,
            fill=QColor(0, 0, 0),
            stroke=QColor(0, 0, 0),
            strokeWidth=0.1,
            widthScaleFactor=1,
            blocking=1,
        )

        self.assertTrue(
            self.image_check(
                "Remote SVG missing",
                "waiting_svg",
                image,
                color_tolerance=2,
                allowed_mismatch=20,
                use_checkerboard_background=True,
            )
        )

        for i in range(1000):
            QCoreApplication.processEvents()

    def test_inline_svg(self):
        inline_svg = """data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" version="1.1" id="Layer_1" x="0px" y="0px" viewBox="0 0 100 100" enable-background="new 0 0 100 100" xml:space="preserve" height="100px" width="100px"><path fill="param(fill) #000000" d="M50,2.5c-19.2,0-34.8,15-34.8,33.4C15.2,61.3,50,97.5,50,97.5s34.8-36.2,34.8-61.6  C84.8,17.5,69.2,2.5,50,2.5z M50,48.2c-7.1,0-12.9-5.8-12.9-12.9c0-7.1,5.8-12.9,12.9-12.9c7.1,0,12.9,5.8,12.9,12.9  C62.9,42.4,57.1,48.2,50,48.2z"/></svg>"""
        image, in_cache = QgsApplication.svgCache().svgAsImage(
            inline_svg,
            100,
            fill=QColor(0, 0, 0),
            stroke=QColor(0, 0, 0),
            strokeWidth=0.1,
            widthScaleFactor=1,
            blocking=True,
        )
        self.assertTrue(
            self.image_check(
                "Inline svg",
                "inline_svg",
                image,
                color_tolerance=2,
                allowed_mismatch=20,
                use_checkerboard_background=True,
            )
        )


if __name__ == "__main__":
    unittest.main()
