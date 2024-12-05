"""QGIS Unit tests for QgsPdfRenderer

From build dir, run: ctest -R QgsPdfRenderer -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import unittest

from qgis.PyQt.QtCore import Qt, QRectF
from qgis.PyQt.QtGui import QImage, QPainter
from qgis.core import QgsPdfRenderer
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestQgsPdfRenderer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "pdf"

    def test_non_pdf(self):
        """Test an invalid PDF"""
        pdf_path = os.path.join(TEST_DATA_DIR, "points.shp")
        renderer = QgsPdfRenderer(pdf_path)
        self.assertEqual(renderer.pageCount(), 0)
        self.assertEqual(renderer.pageMediaBox(0), QRectF())
        # no crash!
        image = QImage(600, 423, QImage.Format.Format_ARGB32_Premultiplied)
        image.fill(Qt.GlobalColor.transparent)
        painter = QPainter(image)
        renderer.render(painter, QRectF(0, 0, 600, 423), pageIndex=0)
        painter.end()

    def test_pdf_properties(self):
        """Test PDF properties"""
        pdf_path = os.path.join(TEST_DATA_DIR, "sample_pdf.pdf")
        renderer = QgsPdfRenderer(pdf_path)
        self.assertEqual(renderer.pageCount(), 2)
        self.assertEqual(renderer.pageMediaBox(-1), QRectF())
        self.assertEqual(renderer.pageMediaBox(0), QRectF(0.0, 0.0, 842.0, 595.0))
        self.assertEqual(renderer.pageMediaBox(1), QRectF(0.0, 0.0, 420.0, 595.0))
        self.assertEqual(renderer.pageMediaBox(2), QRectF())

    def test_pdf_render(self):
        """Test rendering PDF"""
        pdf_path = os.path.join(TEST_DATA_DIR, "sample_pdf.pdf")
        renderer = QgsPdfRenderer(pdf_path)

        image = QImage(600, 423, QImage.Format.Format_ARGB32_Premultiplied)
        image.fill(Qt.GlobalColor.transparent)
        painter = QPainter(image)
        renderer.render(painter, QRectF(0, 0, 600, 423), pageIndex=0)
        painter.end()

        self.assertTrue(
            self.image_check(
                "Render PDF page 1",
                "render_page1",
                image,
                "expected_render_page1",
            )
        )

        # reuse same renderer for second page, to test that reusing a renderer
        # object works
        image = QImage(423, 600, QImage.Format.Format_ARGB32_Premultiplied)
        image.fill(Qt.GlobalColor.transparent)
        painter = QPainter(image)
        renderer.render(painter, QRectF(0, 0, 423, 600), pageIndex=1)
        painter.end()

        self.assertTrue(
            self.image_check(
                "Render PDF page 2",
                "render_page2",
                image,
                "expected_render_page2",
            )
        )


if __name__ == "__main__":
    unittest.main()
