"""QGIS Unit tests for QgsWebEnginePage

From build dir, run: ctest -R QgsWebEnginePage -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import tempfile
import unittest

from qgis.core import QgsFontUtils, QgsHtmlRenderSettings, QgsWebEnginePage
from qgis.PyQt.QtCore import QRectF, QSize, Qt, QUrl
from qgis.PyQt.QtGui import QImage, QPainter
from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import QgisTestCase, start_app
from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestWebEnginePage(QgisTestCase):
    @classmethod
    def control_path_prefix(cls):
        return "html"

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        QgsFontUtils.loadStandardTestFonts(["Bold"])

    def test_contents_size(self):
        """
        Test contentsSize
        """
        html_path = os.path.join(TEST_DATA_DIR, "test_html.html")
        page = QgsWebEnginePage()
        spy = QSignalSpy(page.loadFinished)
        page.setUrl(QUrl.fromLocalFile(html_path))
        spy.wait()
        self.assertTrue(spy[0][0])
        self.assertEqual(page.documentSize(), QSize(306, 248))

    def test_contents_size_blocking(self):
        """
        Test contentsSize using a blocking load
        """
        html_path = os.path.join(TEST_DATA_DIR, "test_html.html")
        page = QgsWebEnginePage()
        self.assertTrue(page.setUrl(QUrl.fromLocalFile(html_path), blocking=True))
        self.assertEqual(page.documentSize(), QSize(306, 248))

    def test_render_web_page(self):
        """
        Test rendering web pages to a QPainter
        """
        html_path = os.path.join(TEST_DATA_DIR, "test_html.html")
        page = QgsWebEnginePage()
        self.assertTrue(page.setUrl(QUrl.fromLocalFile(html_path), blocking=True))

        image = QImage(600, 423, QImage.Format.Format_ARGB32_Premultiplied)
        image.fill(Qt.GlobalColor.transparent)
        painter = QPainter(image)
        self.assertTrue(page.render(painter, QRectF(0, 0, 600, 423)))
        painter.end()

        self.assertTrue(
            self.image_check(
                "Render QgsWebEnginePage to QPainter",
                "render",
                image,
                "expected_render",
            )
        )

    def test_render_web_page_with_text(self):
        """
        Test rendering web page with text to a QPainter
        """
        html_path = os.path.join(TEST_DATA_DIR, "test_html_feature.html")
        page = QgsWebEnginePage()
        self.assertTrue(page.setUrl(QUrl.fromLocalFile(html_path), blocking=True))
        self.assertAlmostEqual(page.documentSize().width(), 306, delta=15)
        self.assertAlmostEqual(page.documentSize().height(), 64, delta=15)

        image = QImage(600, 125, QImage.Format.Format_ARGB32_Premultiplied)
        image.fill(Qt.GlobalColor.transparent)
        painter = QPainter(image)
        self.assertTrue(page.render(painter, QRectF(0, 0, 600, 125)))
        painter.end()

        self.assertTrue(
            self.image_check(
                "Render QgsWebEnginePage with text to QPainter",
                "render_with_text",
                image,
                "expected_render_with_text",
            )
        )

    def test_print_url_to_pdf_bad_url(self):
        """
        Test printing html to pdf, with an invalid url
        """
        html_path = os.path.join(TEST_DATA_DIR, "nope.html")

        with tempfile.TemporaryDirectory() as tmp_dir:
            pdf_path = tmp_dir + "/test.pdf"

            res, error = QgsWebEnginePage.printToPdfBlocking(
                QUrl.fromLocalFile(html_path), pdf_path, 400
            )
            self.assertFalse(res)
            self.assertEqual(error, "Loading HTML failed")

    def test_print_url_to_pdf(self):
        """
        Test printing html to pdf
        """
        html_path = os.path.join(TEST_DATA_DIR, "test_html.html")
        with tempfile.TemporaryDirectory() as tmp_dir:
            pdf_path = tmp_dir + "/test.pdf"

            res, error = QgsWebEnginePage.printToPdfBlocking(
                QUrl.fromLocalFile(html_path), pdf_path, 400
            )
            self.assertTrue(res)
            self.assertFalse(error)

            png_path = tmp_dir + "/test.png"
            self.pdf_to_png(pdf_path, png_path, dpi=96, page=1)

            image = QImage(png_path)
            self.assertTrue(
                self.image_check(
                    "local_html",
                    "local_html",
                    image,
                )
            )

    def test_print_html_to_pdf(self):
        """
        Test printing html to pdf
        """
        html_path = os.path.join(TEST_DATA_DIR, "test_html.html")
        with open(html_path) as f:
            html_content = f.read()

        with tempfile.TemporaryDirectory() as tmp_dir:
            pdf_path = tmp_dir + "/test.pdf"

            res, error = QgsWebEnginePage.printToPdfBlocking(
                html_content, pdf_path, 400
            )
            self.assertTrue(res)
            self.assertFalse(error)

            png_path = tmp_dir + "/test.png"
            self.pdf_to_png(pdf_path, png_path, dpi=96, page=1)

            image = QImage(png_path)
            self.assertTrue(
                self.image_check(
                    "local_html",
                    "local_html",
                    image,
                )
            )

    def test_render_web_page_blocking_fixed_width(self):
        """
        Test rendering web pages to a QPainter, with HTML containing fixed width element
        """
        html_path = os.path.join(TEST_DATA_DIR, "test_html.html")

        image = QImage(600, 423, QImage.Format.Format_ARGB32_Premultiplied)
        image.fill(Qt.GlobalColor.transparent)
        painter = QPainter(image)
        res, error, size = QgsWebEnginePage.renderBlocking(
            QUrl.fromLocalFile(html_path), painter, QRectF(0, 0, 600, 423)
        )
        painter.end()
        self.assertTrue(res)
        self.assertEqual(size.width(), 600)
        self.assertAlmostEqual(size.height(), 200, places=-2)

        self.assertTrue(
            self.image_check(
                "Render QgsWebEnginePage to QPainter",
                "render",
                image,
                "expected_render_blocking",
            )
        )

    def test_render_web_page_blocking_dynamic_width(self):
        """
        Test rendering web pages to a QPainter, with HTML containing dynamic width element
        """
        image = QImage(600, 423, QImage.Format.Format_ARGB32_Premultiplied)
        image.fill(Qt.GlobalColor.transparent)
        painter = QPainter(image)

        html = """
        <html>
  <head>
    <style>
      td {
        border: 3px solid red;
        height: 40px;
      }
    </style>
  </head>
  <body style="margin: 0px; padding: 0px">
    <table style="width: 100%">
      <tbody>
        <tr><td></td><td></td></tr>
        <tr><td></td><td></td></tr>
        <tr><td></td><td></td></tr>
        <tr><td></td><td></td></tr>
        <tr><td></td><td></td></tr>
      </tbody>
    </table>
  </body>
</html>
"""

        res, error, size = QgsWebEnginePage.renderBlocking(
            html, painter, QRectF(0, 0, 600, 423)
        )
        painter.end()
        self.assertTrue(res)
        self.assertEqual(size.width(), 600)
        self.assertAlmostEqual(size.height(), 200, places=-2)

        self.assertTrue(
            self.image_check(
                "Render QgsWebEnginePage to QPainter",
                "render",
                image,
                "expected_render_blocking_dynamic_width_600",
            )
        )

    def test_render_web_page_blocking_dynamic_width_200(self):
        """
        Test rendering web pages to a QPainter, with HTML containing dynamic width element
        """
        image = QImage(200, 423, QImage.Format.Format_ARGB32_Premultiplied)
        image.fill(Qt.GlobalColor.transparent)
        painter = QPainter(image)

        html = """
        <html>
  <head>
    <style>
      td {
        border: 3px solid red;
        height: 40px;
      }
    </style>
  </head>
  <body style="margin: 0px; padding: 0px">
    <table style="width: 100%">
      <tbody>
        <tr><td></td><td></td></tr>
        <tr><td></td><td></td></tr>
        <tr><td></td><td></td></tr>
        <tr><td></td><td></td></tr>
        <tr><td></td><td></td></tr>
      </tbody>
    </table>
  </body>
</html>
"""

        res, error, size = QgsWebEnginePage.renderBlocking(
            html, painter, QRectF(0, 0, 200, 423)
        )

        self.assertTrue(res)
        self.assertEqual(size.width(), 200)
        self.assertAlmostEqual(size.height(), 200, places=-2)

        self.assertTrue(
            self.image_check(
                "Render QgsWebEnginePage to QPainter",
                "render",
                image,
                "expected_render_blocking_dynamic_width_200",
            )
        )

    def test_render_web_page_blocking_user_stylesheet(self):
        """
        Test rendering web pages to a QPainter, with user stylesheet
        """
        image = QImage(600, 423, QImage.Format.Format_ARGB32_Premultiplied)
        image.fill(Qt.GlobalColor.transparent)
        painter = QPainter(image)

        html = """
        <html>
  <head>
    <style>
      td {
        border: 3px solid red;
        height: 40px;
      }
    </style>
  </head>
  <body style="margin: 0px; padding: 0px">
    <table style="width: 100%">
      <tbody>
        <tr><td></td><td></td></tr>
        <tr><td></td><td></td></tr>
        <tr><td></td><td></td></tr>
        <tr><td></td><td></td></tr>
        <tr><td></td><td></td></tr>
      </tbody>
    </table>
  </body>
</html>
"""
        settings = QgsHtmlRenderSettings()
        settings.setUserStylesheet("""
      td {
        border: 6px solid blue !important;
        background: yellow !important;;
      }
        """)

        res, error, size = QgsWebEnginePage.renderBlocking(
            html, painter, QRectF(0, 0, 600, 423), settings
        )
        painter.end()
        self.assertTrue(res, error)
        self.assertEqual(size.width(), 600)
        self.assertAlmostEqual(size.height(), 200, places=-2)

        self.assertTrue(
            self.image_check(
                "Render QgsWebEnginePage to QPainter with user stylesheet",
                "render",
                image,
                "expected_render_with_user_stylesheet",
            )
        )

    def test_render_web_page_blocking_feature_json(self):
        """
        Test rendering web pages to a QPainter, with feature json
        """
        image = QImage(600, 423, QImage.Format.Format_ARGB32_Premultiplied)
        image.fill(Qt.GlobalColor.transparent)
        painter = QPainter(image)

        html = """
                <html>
                <head>
                <script>
                function setFeature(feature) {
                    const box = document.getElementById('box');
                    box.style.backgroundColor = feature.properties.bg;
                    box.style.borderColor = feature.properties.border;
                    box.style.height = feature.properties.height + 'px';
                }
                </script>
                </head>
                <body style="margin: 0px; padding: 0px">
                <div id="box" style="background-color: red; width: 100%; border: 5px solid black; height: 20px; box-sizing: border-box;">
                 </div>
                """
        settings = QgsHtmlRenderSettings()
        settings.setFeatureJson(
            {
                "type": "Feature",
                "id": 1,
                "geometry": None,
                "properties": {
                    "bg": "green",
                    "border": "yellow",
                    "height": 50,
                },
            }
        )

        res, error, size = QgsWebEnginePage.renderBlocking(
            html, painter, QRectF(0, 0, 600, 423), settings
        )
        painter.end()
        self.assertTrue(res, error)
        self.assertEqual(size.width(), 600)
        self.assertAlmostEqual(size.height(), 52, places=0)

        self.assertTrue(
            self.image_check(
                "Render QgsWebEnginePage to QPainter with feature json",
                "render",
                image,
                "expected_render_with_feature_json",
            )
        )


if __name__ == "__main__":
    unittest.main()
