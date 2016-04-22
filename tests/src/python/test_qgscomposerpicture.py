# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsComposerPicture.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2015 by Nyall Dawson'
__date__ = '02/07/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os
import socketserver
import threading
import SimpleHTTPServer
from qgis.PyQt.QtCore import QRectF

from qgis.core import (QgsComposerPicture,
                       QgsComposition,
                       QgsMapSettings
                       )
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgscompositionchecker import QgsCompositionChecker

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposerPicture(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # Bring up a simple HTTP server, for remote picture tests
        os.chdir(unitTestDataPath() + '')
        handler = SimpleHTTPServer.SimpleHTTPRequestHandler

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
        self.mapSettings = QgsMapSettings()
        self.composition = QgsComposition(self.mapSettings)
        self.composition.setPaperSize(297, 210)

        self.composerPicture = QgsComposerPicture(self.composition)
        self.composerPicture.setPicturePath(self.pngImage)
        self.composerPicture.setSceneRect(QRectF(70, 70, 100, 100))
        self.composerPicture.setFrameEnabled(True)
        self.composition.addComposerPicture(self.composerPicture)

    def testResizeZoom(self):
        """Test picture resize zoom mode."""
        self.composerPicture.setResizeMode(QgsComposerPicture.Zoom)

        checker = QgsCompositionChecker('composerpicture_resize_zoom', self.composition)
        checker.setControlPathPrefix("composer_picture")
        testResult, message = checker.testComposition()

        assert testResult, message

    def testRemoteImage(self):
        """Test fetching remote picture."""
        self.composerPicture.setPicturePath('http://localhost:' + str(TestQgsComposerPicture.port) + '/qgis_local_server/logo.png')

        checker = QgsCompositionChecker('composerpicture_remote', self.composition)
        checker.setControlPathPrefix("composer_picture")
        testResult, message = checker.testComposition()

        self.composerPicture.setPicturePath(self.pngImage)
        assert testResult, message

if __name__ == '__main__':
    unittest.main()
