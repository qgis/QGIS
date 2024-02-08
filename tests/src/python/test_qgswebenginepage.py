"""QGIS Unit tests for QgsWebEnginePage

From build dir, run: ctest -R QgsWebEnginePage -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import unittest

from PyQt5.QtCore import Qt
from qgis.PyQt.QtCore import (
    QRectF,
    QUrl
)
from qgis.PyQt.QtGui import (
    QImage,
    QPainter
)
from qgis.core import (
    QgsWebEnginePage
)
from qgis.testing import start_app, QgisTestCase
from qgis.PyQt.QtTest import QSignalSpy

from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestWebEnginePage(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "html"

    def test_render_web_page(self):
        """
        Test rendering web pages to a QPainter
        """
        html_path = os.path.join(TEST_DATA_DIR, 'test_html.html')
        page = QgsWebEnginePage()
        spy = QSignalSpy(page.loadFinished)
        page.setUrl(QUrl.fromLocalFile(html_path))
        spy.wait()
        self.assertTrue(spy[0][0])

        image = QImage(600, 423, QImage.Format.Format_ARGB32_Premultiplied)
        image.fill(Qt.transparent)
        painter = QPainter(image)
        self.assertTrue(
            page.render(
                painter,
                QRectF(0, 0, 600, 423))
        )
        painter.end()

        self.assertTrue(
            self.image_check(
                'Render QgsWebEnginePage to QPainter',
                'render',
                image,
                'expected_render',
            )
        )


if __name__ == "__main__":
    unittest.main()
