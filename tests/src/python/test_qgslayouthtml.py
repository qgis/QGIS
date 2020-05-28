# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutItemHtml.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '20/11/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QUrl, qDebug, QRectF
from qgis.core import (QgsLayout,
                       QgsLayoutItemHtml,
                       QgsLayoutFrame,
                       QgsLayoutMultiFrame,
                       QgsMapSettings,
                       QgsProject)

from qgslayoutchecker import QgsLayoutChecker

from qgis.testing import start_app, unittest
from qgis.testing.mocked import get_iface
from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutHtml(unittest.TestCase):

    def setUp(self):
        """Run before each test."""
        self.iface = get_iface()
        self.layout = QgsLayout(QgsProject.instance())
        self.layout.initializeDefaults()

    def tearDown(self):
        """Run after each test."""
        print("Tear down")

    def htmlUrl(self):
        """Helper to get the url of the html doc."""
        myPath = os.path.join(TEST_DATA_DIR, "test_html.html")
        myUrl = QUrl("file:///" + myPath)
        return myUrl

    def testTable(self):
        """Test we can render a html table in a single frame."""
        layout_html = QgsLayoutItemHtml(self.layout)
        html_frame = QgsLayoutFrame(self.layout, layout_html)
        html_frame.attemptSetSceneRect(QRectF(0, 0, 100, 200))
        html_frame.setFrameEnabled(True)
        layout_html.addFrame(html_frame)
        layout_html.setUrl(self.htmlUrl())

        checker = QgsLayoutChecker('composerhtml_table', self.layout)
        checker.setControlPathPrefix("composer_html")
        myTestResult, myMessage = checker.testLayout()

        qDebug(myMessage)
        self.layout.removeMultiFrame(layout_html)
        assert myTestResult, myMessage

    def testTableMultiFrame(self):
        """Test we can render to multiframes."""
        layout_html = QgsLayoutItemHtml(self.layout)
        html_frame = QgsLayoutFrame(self.layout, layout_html)
        html_frame.attemptSetSceneRect(QRectF(10, 10, 100, 50))
        layout_html.addFrame(html_frame)
        layout_html.setResizeMode(
            QgsLayoutMultiFrame.RepeatUntilFinished)
        layout_html.setUseSmartBreaks(False)
        layout_html.setUrl(self.htmlUrl())
        layout_html.frame(0).setFrameEnabled(True)

        print("Checking page 1")
        myPage = 0
        checker1 = QgsLayoutChecker('composerhtml_multiframe1', self.layout)
        checker1.setControlPathPrefix("composer_html")
        myTestResult, myMessage = checker1.testLayout(myPage)
        assert myTestResult, myMessage

        print("Checking page 2")
        myPage = 1
        checker2 = QgsLayoutChecker('composerhtml_multiframe2', self.layout)
        checker2.setControlPathPrefix("composer_html")
        myTestResult, myMessage = checker2.testLayout(myPage)
        assert myTestResult, myMessage

        self.layout.removeMultiFrame(layout_html)
        layout_html = None

        assert myTestResult, myMessage

    def testHtmlSmartBreaks(self):
        """Test rendering to multiframes with smart breaks."""
        layout_html = QgsLayoutItemHtml(self.layout)
        html_frame = QgsLayoutFrame(self.layout, layout_html)
        html_frame.attemptSetSceneRect(QRectF(10, 10, 100, 52))
        layout_html.addFrame(html_frame)
        layout_html.setResizeMode(
            QgsLayoutMultiFrame.RepeatUntilFinished)
        layout_html.setUseSmartBreaks(True)
        layout_html.setUrl(self.htmlUrl())
        layout_html.frame(0).setFrameEnabled(True)

        print("Checking page 1")
        myPage = 0
        checker1 = QgsLayoutChecker('composerhtml_smartbreaks1', self.layout)
        checker1.setControlPathPrefix("composer_html")
        myTestResult, myMessage = checker1.testLayout(myPage, 200)
        assert myTestResult, myMessage

        print("Checking page 2")
        myPage = 1
        checker2 = QgsLayoutChecker('composerhtml_smartbreaks2', self.layout)
        checker2.setControlPathPrefix("composer_html")
        myTestResult, myMessage = checker2.testLayout(myPage, 200)
        assert myTestResult, myMessage

        self.layout.removeMultiFrame(layout_html)
        layout_html = None

        assert myTestResult, myMessage


if __name__ == '__main__':
    unittest.main()
