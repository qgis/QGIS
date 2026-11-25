"""QGIS Unit tests for QgsPictureRasterizer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import glob
import os
import re
import shutil
import subprocess
import sys
import tempfile

from qgis.core import QgsPictureRasterizer
from qgis.PyQt.QtCore import QRectF, QSize, Qt
from qgis.PyQt.QtGui import QBrush, QColor, QPainter, QPicture
from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import QgisTestCase, unittest
from utilities import start_app, unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()

start_app()


class TestQgsPictureRasterizer(QgisTestCase):
    TMP_DIR = ""

    @classmethod
    def control_path_prefix(cls):
        return "rasterizer"

    def test_rasterizer(self):
        rasterizer = QgsPictureRasterizer()
        image = rasterizer.image(QSize(300, 300))
        self.assertTrue(image.isNull())

        picture = QPicture()
        painter = QPainter(picture)
        for x in range(1, 10):
            for y in range(1, 10):
                color = QColor(x * 20, y * 20, 0)
                painter.setBrush(QBrush(color))
                painter.setPen(Qt.PenStyle.NoPen)
                painter.drawRect(QRectF(x * 100, y * 100, 30, 30))
        painter.end()

        rasterizer.setPicture(picture)
        image = rasterizer.image(QSize(300, 300))
        spy = QSignalSpy(rasterizer.rendered)
        # image should be null -- but this will trigger background generation
        self.assertTrue(image.isNull())
        spy.wait()
        self.assertEqual(len(spy), 1)

        # now get image, should be valid
        image = rasterizer.image(QSize(300, 300))
        self.assertFalse(image.isNull())

        self.assertTrue(
            self.image_check(
                "render_300",
                "render_300",
                image,
            )
        )

        # re-request at same size
        image = rasterizer.image(QSize(300, 300))
        # should be no background regeneration
        spy.wait(100)
        self.assertEqual(len(spy), 1)
        self.assertFalse(image.isNull())
        self.assertTrue(
            self.image_check(
                "render_300",
                "render_300",
                image,
            )
        )

        # re-request at a new size
        image = rasterizer.image(QSize(600, 600))
        # previous image should be returned
        self.assertEqual(image.size(), QSize(300, 300))
        # but we regenerate in the background...
        spy.wait()
        self.assertEqual(len(spy), 2)

        image = rasterizer.image(QSize(600, 600))
        self.assertTrue(
            self.image_check(
                "render_600",
                "render_600",
                image,
            )
        )


if __name__ == "__main__":
    unittest.main()
