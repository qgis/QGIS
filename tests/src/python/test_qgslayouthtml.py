"""QGIS Unit tests for QgsLayoutItemHtml.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "20/11/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

import os

from qgis.PyQt.QtCore import QRectF, QUrl
from qgis.core import (
    QgsLayout,
    QgsLayoutFrame,
    QgsLayoutItemHtml,
    QgsLayoutMultiFrame,
    QgsProject,
)
import unittest
from qgis.testing import start_app, QgisTestCase
from qgis.testing.mocked import get_iface

from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutHtml(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "composer_html"

    def setUp(self):
        """Run before each test."""
        self.iface = get_iface()
        self.layout = QgsLayout(QgsProject.instance())
        self.layout.initializeDefaults()

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

        result = self.render_layout_check("composerhtml_table", self.layout)

        self.layout.removeMultiFrame(layout_html)
        self.assertTrue(result)

    def testTableMultiFrame(self):
        """Test we can render to multiframes."""
        layout_html = QgsLayoutItemHtml(self.layout)
        html_frame = QgsLayoutFrame(self.layout, layout_html)
        html_frame.attemptSetSceneRect(QRectF(10, 10, 100, 50))
        layout_html.addFrame(html_frame)
        layout_html.setResizeMode(QgsLayoutMultiFrame.ResizeMode.RepeatUntilFinished)
        layout_html.setUseSmartBreaks(False)
        layout_html.setUrl(self.htmlUrl())
        layout_html.frame(0).setFrameEnabled(True)

        self.assertTrue(
            self.render_layout_check("composerhtml_multiframe1", self.layout)
        )

        self.assertTrue(
            self.render_layout_check("composerhtml_multiframe2", self.layout, page=1)
        )

        self.layout.removeMultiFrame(layout_html)
        layout_html = None

    def testHtmlSmartBreaks(self):
        """Test rendering to multiframes with smart breaks."""
        layout_html = QgsLayoutItemHtml(self.layout)
        html_frame = QgsLayoutFrame(self.layout, layout_html)
        html_frame.attemptSetSceneRect(QRectF(10, 10, 100, 52))
        layout_html.addFrame(html_frame)
        layout_html.setResizeMode(QgsLayoutMultiFrame.ResizeMode.RepeatUntilFinished)
        layout_html.setUseSmartBreaks(True)
        layout_html.setUrl(self.htmlUrl())
        layout_html.frame(0).setFrameEnabled(True)

        self.assertTrue(
            self.render_layout_check(
                "composerhtml_smartbreaks1", self.layout, allowed_mismatch=200
            )
        )
        self.assertTrue(
            self.render_layout_check(
                "composerhtml_smartbreaks2", self.layout, page=1, allowed_mismatch=200
            )
        )

        self.layout.removeMultiFrame(layout_html)
        layout_html = None


if __name__ == "__main__":
    unittest.main()
