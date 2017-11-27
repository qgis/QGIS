# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutPicture.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2017 by Nyall Dawson'
__date__ = '23/10/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os
import socketserver
import threading
import http.server
from qgis.PyQt.QtCore import QRectF

from qgis.core import (QgsLayoutItemPicture,
                       QgsLayout,
                       QgsLayoutItemMap,
                       QgsRectangle,
                       QgsCoordinateReferenceSystem,
                       QgsProject
                       )
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgslayoutchecker import QgsLayoutChecker

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutPicture(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # Bring up a simple HTTP server, for remote picture tests
        os.chdir(unitTestDataPath() + '')
        handler = http.server.SimpleHTTPRequestHandler

        cls.httpd = socketserver.TCPServer(('localhost', 0), handler)
        cls.port = cls.httpd.server_address[1]

        cls.httpd_thread = threading.Thread(target=cls.httpd.serve_forever)
        cls.httpd_thread.setDaemon(True)
        cls.httpd_thread.start()

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

        TEST_DATA_DIR = unitTestDataPath()
        self.pngImage = TEST_DATA_DIR + "/sample_image.png"

        # create composition
        self.layout = QgsLayout(QgsProject.instance())
        self.layout.initializeDefaults()

        self.picture = QgsLayoutItemPicture(self.layout)
        self.picture.setPicturePath(self.pngImage)
        self.picture.attemptSetSceneRect(QRectF(70, 70, 100, 100))
        self.picture.setFrameEnabled(True)
        self.layout.addLayoutItem(self.picture)

    def testResizeZoom(self):
        """Test picture resize zoom mode."""
        self.picture.setResizeMode(QgsLayoutItemPicture.Zoom)

        checker = QgsLayoutChecker('composerpicture_resize_zoom', self.layout)
        checker.setControlPathPrefix("composer_picture")
        testResult, message = checker.testLayout()

        assert testResult, message

    @unittest.skip('test is broken for qt5/python3 - feature works')
    def testRemoteImage(self):
        """Test fetching remote picture."""
        self.picture.setPicturePath('http://localhost:' + str(TestQgsLayoutPicture.port) + '/qgis_local_server/logo.png')

        checker = QgsLayoutChecker('picture_remote', self.layout)
        checker.setControlPathPrefix("composer_picture")
        testResult, message = checker.testLayout()

        self.picture.setPicturePath(self.pngImage)
        assert testResult, message

    def testGridNorth(self):
        """Test syncing picture to grid north"""

        layout = QgsLayout(QgsProject.instance())

        map = QgsLayoutItemMap(layout)
        map.setExtent(QgsRectangle(0, -256, 256, 0))
        layout.addLayoutItem(map)

        picture = QgsLayoutItemPicture(layout)
        layout.addLayoutItem(picture)

        picture.setRotationMap(map.uuid())
        self.assertEqual(picture.rotationMap(), map.uuid())

        picture.setNorthMode(QgsLayoutItemPicture.GridNorth)
        map.setMapRotation(45)
        self.assertEqual(picture.pictureRotation(), 45)

        # add an offset
        picture.setNorthOffset(-10)
        self.assertEqual(picture.pictureRotation(), 35)

    def testTrueNorth(self):
        """Test syncing picture to true north"""

        layout = QgsLayout(QgsProject.instance())

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(0, 0, 10, 10))
        map.setCrs(QgsCoordinateReferenceSystem.fromEpsgId(3575))
        map.setExtent(QgsRectangle(-2126029.962, -2200807.749, -119078.102, -757031.156))
        layout.addLayoutItem(map)

        picture = QgsLayoutItemPicture(layout)
        layout.addLayoutItem(picture)

        picture.setRotationMap(map.uuid())
        self.assertEqual(picture.rotationMap(), map.uuid())

        picture.setNorthMode(QgsLayoutItemPicture.TrueNorth)
        self.assertAlmostEqual(picture.pictureRotation(), 37.20, 1)

        # shift map
        map.setExtent(QgsRectangle(2120672.293, -3056394.691, 2481640.226, -2796718.780))
        self.assertAlmostEqual(picture.pictureRotation(), -38.18, 1)

        # rotate map
        map.setMapRotation(45)
        self.assertAlmostEqual(picture.pictureRotation(), -38.18 + 45, 1)

        # add an offset
        picture.setNorthOffset(-10)
        self.assertAlmostEqual(picture.pictureRotation(), -38.18 + 35, 1)


if __name__ == '__main__':
    unittest.main()
