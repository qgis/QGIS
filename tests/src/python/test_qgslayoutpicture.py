"""QGIS Unit tests for QgsLayoutPicture.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2017 by Nyall Dawson"
__date__ = "23/10/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

import http.server
import os
import socketserver
import threading

from qgis.PyQt.QtCore import QDir, QRectF
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsLayout,
    QgsLayoutItemMap,
    QgsLayoutItemPicture,
    QgsProject,
    QgsReadWriteContext,
    QgsRectangle,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from test_qgslayoutitem import LayoutItemTestCase
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutPicture(QgisTestCase, LayoutItemTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "composer_picture"

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.item_class = QgsLayoutItemPicture

        # Bring up a simple HTTP server, for remote picture tests
        os.chdir(unitTestDataPath() + "")
        handler = http.server.SimpleHTTPRequestHandler

        cls.httpd = socketserver.TCPServer(("localhost", 0), handler)
        cls.port = cls.httpd.server_address[1]

        cls.httpd_thread = threading.Thread(target=cls.httpd.serve_forever)
        cls.httpd_thread.daemon = True
        cls.httpd_thread.start()

    def __init__(self, methodName):
        """Run once on class initialization."""
        QgisTestCase.__init__(self, methodName)

        TEST_DATA_DIR = unitTestDataPath()
        self.pngImage = TEST_DATA_DIR + "/sample_image.png"
        self.svgImage = TEST_DATA_DIR + "/sample_svg.svg"

        # create composition
        self.layout = QgsLayout(QgsProject.instance())
        self.layout.initializeDefaults()

        self.picture = QgsLayoutItemPicture(self.layout)
        self.picture.setPicturePath(self.pngImage)
        self.picture.attemptSetSceneRect(QRectF(70, 70, 100, 100))
        self.picture.setFrameEnabled(True)
        self.layout.addLayoutItem(self.picture)

    def testMode(self):
        pic = QgsLayoutItemPicture(self.layout)
        # should default to unknown
        self.assertEqual(pic.mode(), QgsLayoutItemPicture.Format.FormatUnknown)
        spy = QSignalSpy(pic.changed)
        pic.setMode(QgsLayoutItemPicture.Format.FormatRaster)
        self.assertEqual(pic.mode(), QgsLayoutItemPicture.Format.FormatRaster)
        self.assertEqual(len(spy), 1)
        pic.setMode(QgsLayoutItemPicture.Format.FormatRaster)
        self.assertEqual(len(spy), 1)
        pic.setMode(QgsLayoutItemPicture.Format.FormatSVG)
        self.assertEqual(len(spy), 3)  # ideally only 2!
        self.assertEqual(pic.mode(), QgsLayoutItemPicture.Format.FormatSVG)

        # set picture path without explicit format
        pic.setPicturePath(self.pngImage)
        self.assertEqual(pic.mode(), QgsLayoutItemPicture.Format.FormatRaster)
        pic.setPicturePath(self.svgImage)
        self.assertEqual(pic.mode(), QgsLayoutItemPicture.Format.FormatSVG)
        # forced format
        pic.setPicturePath(self.pngImage, QgsLayoutItemPicture.Format.FormatSVG)
        self.assertEqual(pic.mode(), QgsLayoutItemPicture.Format.FormatSVG)
        pic.setPicturePath(self.pngImage, QgsLayoutItemPicture.Format.FormatRaster)
        self.assertEqual(pic.mode(), QgsLayoutItemPicture.Format.FormatRaster)
        pic.setPicturePath(self.svgImage, QgsLayoutItemPicture.Format.FormatSVG)
        self.assertEqual(pic.mode(), QgsLayoutItemPicture.Format.FormatSVG)
        pic.setPicturePath(self.svgImage, QgsLayoutItemPicture.Format.FormatRaster)
        self.assertEqual(pic.mode(), QgsLayoutItemPicture.Format.FormatRaster)

    def testReadWriteXml(self):
        pr = QgsProject()
        l = QgsLayout(pr)

        pic = QgsLayoutItemPicture(l)
        # mode should be saved/restored
        pic.setMode(QgsLayoutItemPicture.Format.FormatRaster)

        # save original item to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        self.assertTrue(pic.writeXml(elem, doc, QgsReadWriteContext()))

        pic2 = QgsLayoutItemPicture(l)
        self.assertTrue(
            pic2.readXml(elem.firstChildElement(), doc, QgsReadWriteContext())
        )
        self.assertEqual(pic2.mode(), QgsLayoutItemPicture.Format.FormatRaster)

        pic.setMode(QgsLayoutItemPicture.Format.FormatSVG)
        elem = doc.createElement("test2")
        self.assertTrue(pic.writeXml(elem, doc, QgsReadWriteContext()))
        pic3 = QgsLayoutItemPicture(l)
        self.assertTrue(
            pic3.readXml(elem.firstChildElement(), doc, QgsReadWriteContext())
        )
        self.assertEqual(pic3.mode(), QgsLayoutItemPicture.Format.FormatSVG)

    def testResizeZoom(self):
        """Test picture resize zoom mode."""
        self.picture.setResizeMode(QgsLayoutItemPicture.ResizeMode.Zoom)

        self.assertTrue(
            self.render_layout_check("composerpicture_resize_zoom", self.layout)
        )

    def testRemoteImage(self):
        """Test fetching remote picture."""
        self.picture.setPicturePath(
            "http://localhost:"
            + str(TestQgsLayoutPicture.port)
            + "/qgis_local_server/logo.png"
        )

        res = self.render_layout_check("composerpicture_remote", self.layout)

        self.picture.setPicturePath(self.pngImage)
        self.assertTrue(res)

    def testNorthArrowWithMapItemRotation(self):
        """Test picture rotation when map item is also rotated"""

        layout = QgsLayout(QgsProject.instance())

        map = QgsLayoutItemMap(layout)
        map.setExtent(QgsRectangle(0, -256, 256, 0))
        layout.addLayoutItem(map)

        picture = QgsLayoutItemPicture(layout)
        layout.addLayoutItem(picture)

        picture.setLinkedMap(map)
        self.assertEqual(picture.linkedMap(), map)

        picture.setNorthMode(QgsLayoutItemPicture.NorthMode.GridNorth)
        map.setItemRotation(45)
        self.assertEqual(picture.pictureRotation(), 45)
        map.setMapRotation(-34)
        self.assertEqual(picture.pictureRotation(), 11)

        # add an offset
        picture.setNorthOffset(-10)
        self.assertEqual(picture.pictureRotation(), 1)

        map.setItemRotation(55)
        self.assertEqual(picture.pictureRotation(), 11)

    def testGridNorth(self):
        """Test syncing picture to grid north"""

        layout = QgsLayout(QgsProject.instance())

        map = QgsLayoutItemMap(layout)
        map.setExtent(QgsRectangle(0, -256, 256, 0))
        layout.addLayoutItem(map)

        picture = QgsLayoutItemPicture(layout)
        layout.addLayoutItem(picture)

        picture.setLinkedMap(map)
        self.assertEqual(picture.linkedMap(), map)

        picture.setNorthMode(QgsLayoutItemPicture.NorthMode.GridNorth)
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
        map.setExtent(
            QgsRectangle(-2126029.962, -2200807.749, -119078.102, -757031.156)
        )
        layout.addLayoutItem(map)

        picture = QgsLayoutItemPicture(layout)
        layout.addLayoutItem(picture)

        picture.setLinkedMap(map)
        self.assertEqual(picture.linkedMap(), map)

        picture.setNorthMode(QgsLayoutItemPicture.NorthMode.TrueNorth)
        self.assertAlmostEqual(picture.pictureRotation(), 37.20, 1)

        # shift map
        map.setExtent(
            QgsRectangle(2120672.293, -3056394.691, 2481640.226, -2796718.780)
        )
        self.assertAlmostEqual(picture.pictureRotation(), -38.18, 1)

        # rotate map
        map.setMapRotation(45)
        self.assertAlmostEqual(picture.pictureRotation(), -38.18 + 45, 1)

        # add an offset
        picture.setNorthOffset(-10)
        self.assertAlmostEqual(picture.pictureRotation(), -38.18 + 35, 1)

    def testMissingImage(self):
        layout = QgsLayout(QgsProject.instance())

        picture = QgsLayoutItemPicture(layout)

        # SVG
        picture.setPicturePath("invalid_path", QgsLayoutItemPicture.Format.FormatSVG)
        self.assertEqual(picture.isMissingImage(), True)
        self.assertEqual(picture.mode(), QgsLayoutItemPicture.Format.FormatSVG)

        # Raster
        picture.setPicturePath("invalid_path", QgsLayoutItemPicture.Format.FormatRaster)
        self.assertEqual(picture.isMissingImage(), True)
        self.assertEqual(picture.mode(), QgsLayoutItemPicture.Format.FormatRaster)


if __name__ == "__main__":
    unittest.main()
