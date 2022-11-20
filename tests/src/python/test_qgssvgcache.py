# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsSvgCache.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2018 by Nyall Dawson'
__date__ = '29/03/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA

import os
import socketserver
import threading
import http.server
import time
from qgis.PyQt.QtCore import QDir, QCoreApplication
from qgis.PyQt.QtGui import QColor, QImage, QPainter

from qgis.core import (QgsSvgCache, QgsRenderChecker, QgsApplication, QgsMultiRenderChecker)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class SlowHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):

    def do_GET(self):
        time.sleep(1)
        return http.server.SimpleHTTPRequestHandler.do_GET(self)


class TestQgsSvgCache(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # Bring up a simple HTTP server, for remote SVG tests
        os.chdir(unitTestDataPath() + '')
        handler = SlowHTTPRequestHandler

        cls.httpd = socketserver.TCPServer(('localhost', 0), handler)
        cls.port = cls.httpd.server_address[1]

        cls.httpd_thread = threading.Thread(target=cls.httpd.serve_forever)
        cls.httpd_thread.daemon = True
        cls.httpd_thread.start()

    def setUp(self):
        self.report = "<h1>Python QgsSvgCache Tests</h1>\n"

        self.fetched = True
        QgsApplication.svgCache().remoteSvgFetched.connect(self.svgFetched)

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def svgFetched(self):
        self.fetched = True

    def waitForFetch(self):
        self.fetched = False
        while not self.fetched:
            QCoreApplication.processEvents()

    def testRemoteSVG(self):
        """Test fetching remote svg."""
        url = 'http://localhost:{}/qgis_local_server/sample_svg.svg'.format(str(TestQgsSvgCache.port))
        image, in_cache = QgsApplication.svgCache().svgAsImage(url, 100, fill=QColor(0, 0, 0), stroke=QColor(0, 0, 0),
                                                               strokeWidth=0.1, widthScaleFactor=1)
        # first should be waiting image
        self.assertTrue(self.imageCheck('Remote SVG', 'waiting_svg', image))
        self.waitForFetch()

        # second should be correct image
        image, in_cache = QgsApplication.svgCache().svgAsImage(url, 100, fill=QColor(0, 0, 0), stroke=QColor(0, 0, 0),
                                                               strokeWidth=0.1, widthScaleFactor=1)
        self.assertTrue(self.imageCheck('Remote SVG', 'remote_svg', image))

        for i in range(1000):
            QCoreApplication.processEvents()

    def testRemoteSvgAsText(self):
        """Test fetching remote svg with text mime format - e.g. github raw svgs"""
        url = 'http://localhost:{}/qgis_local_server/svg_as_text.txt'.format(str(TestQgsSvgCache.port))
        image, in_cache = QgsApplication.svgCache().svgAsImage(url, 100, fill=QColor(0, 0, 0), stroke=QColor(0, 0, 0),
                                                               strokeWidth=0.1, widthScaleFactor=1)
        # first should be waiting image
        self.assertTrue(self.imageCheck('Remote SVG as Text', 'waiting_svg', image))

        self.waitForFetch()
        # second should be correct image
        image, in_cache = QgsApplication.svgCache().svgAsImage(url, 100, fill=QColor(0, 0, 0), stroke=QColor(0, 0, 0),
                                                               strokeWidth=0.1, widthScaleFactor=1)

        # first should be waiting image
        self.assertTrue(self.imageCheck('Remote SVG as Text', 'remote_svg', image))

        for i in range(1000):
            QCoreApplication.processEvents()

    def testRemoteSvgBadMime(self):
        """Test fetching remote svg with bad mime type"""
        url = 'http://localhost:{}/qgis_local_server/logo.png'.format(str(TestQgsSvgCache.port))
        image, in_cache = QgsApplication.svgCache().svgAsImage(url, 100, fill=QColor(0, 0, 0), stroke=QColor(0, 0, 0),
                                                               strokeWidth=0.1, widthScaleFactor=1)
        # first should be waiting image
        self.assertTrue(self.imageCheck('Remote SVG bad MIME type', 'waiting_svg', image))

        # second should be correct image
        self.waitForFetch()
        image, in_cache = QgsApplication.svgCache().svgAsImage(url, 100, fill=QColor(0, 0, 0), stroke=QColor(0, 0, 0),
                                                               strokeWidth=0.1, widthScaleFactor=1)
        self.assertTrue(self.imageCheck('Remote SVG bad MIME type', 'bad_svg', image))

        for i in range(1000):
            QCoreApplication.processEvents()

    def testRemoteSvgMissing(self):
        """Test fetching remote svg with bad url"""
        url = 'http://localhost:{}/qgis_local_server/xxx.svg'.format(str(TestQgsSvgCache.port))  # oooo naughty
        image, in_cache = QgsApplication.svgCache().svgAsImage(url, 100, fill=QColor(0, 0, 0), stroke=QColor(0, 0, 0),
                                                               strokeWidth=0.1, widthScaleFactor=1)

        self.assertTrue(self.imageCheck('Remote SVG missing', 'waiting_svg', image))

        for i in range(1000):
            QCoreApplication.processEvents()

    def testRemoteSVGBlocking(self):
        """Test fetching remote svg."""
        # remote not yet requested so not in cache
        url = 'http://localhost:{}/qgis_local_server/QGIS_logo_2017.svg'.format(str(TestQgsSvgCache.port))
        image, in_cache = QgsApplication.svgCache().svgAsImage(url, 100, fill=QColor(0, 0, 0), stroke=QColor(0, 0, 0),
                                                               strokeWidth=0.1, widthScaleFactor=1, blocking=1)
        # first should be correct image
        self.assertTrue(self.imageCheck('Remote SVG sync', 'remote_svg_blocking', image))

        # remote probably in cache
        url = 'http://localhost:{}/qgis_local_server/sample_svg.svg'.format(str(TestQgsSvgCache.port))
        image, in_cache = QgsApplication.svgCache().svgAsImage(url, 100, fill=QColor(0, 0, 0), stroke=QColor(0, 0, 0),
                                                               strokeWidth=0.1, widthScaleFactor=1, blocking=1)

        self.assertTrue(self.imageCheck('Remote SVG', 'remote_svg', image))

        # missing
        url = 'http://localhost:{}/qgis_local_server/xxx.svg'.format(str(TestQgsSvgCache.port))  # oooo naughty
        image, in_cache = QgsApplication.svgCache().svgAsImage(url, 100, fill=QColor(0, 0, 0), stroke=QColor(0, 0, 0),
                                                               strokeWidth=0.1, widthScaleFactor=1, blocking=1)

        self.assertTrue(self.imageCheck('Remote SVG missing', 'waiting_svg', image))

        for i in range(1000):
            QCoreApplication.processEvents()

    def imageCheck(self, name, reference_image, image):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'svg_' + name + ".png"

        output_image = QImage(image.size(), QImage.Format_RGB32)
        QgsMultiRenderChecker.drawBackground(output_image)
        painter = QPainter(output_image)
        painter.drawImage(0, 0, image)
        painter.end()

        output_image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("svg_cache")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
