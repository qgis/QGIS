"""QGIS Unit tests for qgis_html_to_pdf.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import glob
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

from qgis.core import QgsApplication
from qgis.PyQt.QtGui import QImage
from qgis.testing import QgisTestCase, unittest
from utilities import start_app, unitTestDataPath

print("CTEST_FULL_OUTPUT")

TEST_DATA_DIR = unitTestDataPath()

start_app()


class TestQgsHtmlToPdf(QgisTestCase):
    TMP_DIR = ""

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.TMP_DIR = tempfile.mkdtemp()
        # print('TMP_DIR: ' + cls.TMP_DIR)
        # subprocess.call(['open', cls.TMP_DIR])

    @classmethod
    def tearDownClass(cls):
        super().tearDownClass()
        shutil.rmtree(cls.TMP_DIR, ignore_errors=True)

    @classmethod
    def control_path_prefix(cls):
        return "html_to_pdf"

    def run_html_to_pdf(self, arguments, settings_json):
        with tempfile.TemporaryDirectory() as tmp_dir:
            if settings_json:
                with open(tmp_dir + "/settings.json", "w", encoding="utf8") as f:
                    f.write(json.dumps(settings_json))
                arguments.append(f"--settings={tmp_dir}/settings.json")

            call = [QGIS_HTML_TO_PDF_BIN] + arguments
            print(" ".join(call))

            myenv = os.environ.copy()
            myenv["QGIS_DEBUG"] = "0"

            p = subprocess.Popen(
                call, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=myenv
            )
            output, err = p.communicate()
            rc = p.returncode

            return rc, output.decode(), err.decode()

    def testNoArgs(self):
        rc, output, err = self.run_html_to_pdf([], {})
        self.assertIn("QGIS HTML to PDF converter utility", output)
        self.assertFalse(self.strip_std_ignorable_errors(err))
        self.assertEqual(rc, 1)

    def test_local_html(self):
        html_path = TEST_DATA_DIR + "/test_html.html"
        with tempfile.TemporaryDirectory() as tmp_dir:
            pdf_path = tmp_dir + "/test.pdf"
            png_path = tmp_dir + "/test.png"
            rc, output, err = self.run_html_to_pdf(
                [html_path, pdf_path, "--width=400"], {}
            )
            self.assertEqual(rc, 0)

            self.pdf_to_png(pdf_path, png_path, dpi=96, page=1)

            image = QImage(png_path)
            self.assertTrue(
                self.image_check(
                    "local_html",
                    "local_html",
                    image,
                )
            )

    def test_dynamic_width(self):
        with tempfile.TemporaryDirectory() as tmp_dir:
            html_path = tmp_dir + "/test.html"
            with open(html_path, "w", encoding="utf8") as f_out:
                f_out.write("""
                <html>
                <body>
                <div style="background-color: yellow; width: 100%; border: 3px solid blue; height: 30px">
                </div>
                """)

            pdf_path = tmp_dir + "/test.pdf"
            png_path = tmp_dir + "/test.png"
            rc, output, err = self.run_html_to_pdf(
                [html_path, pdf_path, "--width=400"], {}
            )
            self.assertEqual(rc, 0)

            self.pdf_to_png(pdf_path, png_path, dpi=96, page=1)

            image = QImage(png_path)
            self.assertTrue(
                self.image_check(
                    "dynamic_width",
                    "dynamic_width",
                    image,
                )
            )

            rc, output, err = self.run_html_to_pdf(
                [html_path, pdf_path, "--width=600"], {}
            )
            self.assertEqual(rc, 0)

            self.pdf_to_png(pdf_path, png_path, dpi=96, page=1)

            image = QImage(png_path)
            self.assertTrue(
                self.image_check(
                    "dynamic_width_600",
                    "dynamic_width_600",
                    image,
                )
            )

    def test_user_stylesheet(self):
        with tempfile.TemporaryDirectory() as tmp_dir:
            html_path = tmp_dir + "/test.html"
            with open(html_path, "w", encoding="utf8") as f_out:
                f_out.write("""
                <html>
                <body>
                <div style="background-color: yellow; width: 100%; border: 3px solid blue; height: 30px">\
                </div>
                """)

            pdf_path = tmp_dir + "/test.pdf"
            png_path = tmp_dir + "/test.png"
            rc, output, err = self.run_html_to_pdf(
                [html_path, pdf_path, "--width=400"],
                {
                    "userStylesheet": """div { background-color: blue !important; border: 5px solid yellow  !important;}"""
                },
            )
            self.assertEqual(rc, 0)

            self.pdf_to_png(pdf_path, png_path, dpi=96, page=1)

            image = QImage(png_path)
            self.assertTrue(
                self.image_check(
                    "user_stylesheet",
                    "user_stylesheet",
                    image,
                )
            )

    def test_feature_json(self):
        with tempfile.TemporaryDirectory() as tmp_dir:
            html_path = tmp_dir + "/test.html"
            with open(html_path, "w", encoding="utf8") as f_out:
                f_out.write("""
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
                <body>
                <div id="box" style="background-color: red; width: 100%; border: 5px solid black; height: 20px; box-sizing: border-box;">
                 </div>
                """)

            pdf_path = tmp_dir + "/test.pdf"
            png_path = tmp_dir + "/test.png"
            rc, output, err = self.run_html_to_pdf(
                [html_path, pdf_path, "--width=400"],
                {
                    "feature": {
                        "type": "Feature",
                        "id": 1,
                        "geometry": None,
                        "properties": {
                            "bg": "green",
                            "border": "yellow",
                            "height": 50,
                        },
                    }
                },
            )
            self.assertEqual(rc, 0)

            self.pdf_to_png(pdf_path, png_path, dpi=96, page=1)

            image = QImage(png_path)
            self.assertTrue(
                self.image_check(
                    "feature_json",
                    "feature_json",
                    image,
                )
            )


if __name__ == "__main__":
    # look for qgis bin path
    QGIS_HTML_TO_PDF_BIN = ""

    prefixPath = QgsApplication.libexecPath()
    for f in ["", "..", "bin"]:
        d = os.path.join(prefixPath, f)
        b = os.path.abspath(os.path.join(d, "qgis_html_to_pdf"))
        if os.path.exists(b):
            QGIS_HTML_TO_PDF_BIN = b
            break
        b = os.path.abspath(os.path.join(d, "qgis_html_to_pdf.exe"))
        if os.path.exists(b):
            QGIS_HTML_TO_PDF_BIN = b
            break
        if sys.platform[:3] == "dar":  # Mac
            # QGIS.app may be QGIS_x.x-dev.app for nightlies
            # internal binary will match, minus the '.app'
            found = False
            for app_path in glob.glob(d + "/QGIS*.app"):
                m = re.search(r"/(QGIS(_\d\.\d-dev)?)\.app", app_path)
                if m:
                    QGIS_HTML_TO_PDF_BIN = app_path + "/Contents/MacOS/" + m.group(1)
                    found = True
                    break
            if found:
                break

    print(f"\nQGIS_HTML_TO_PDF_BIN: {QGIS_HTML_TO_PDF_BIN}")
    assert QGIS_HTML_TO_PDF_BIN, (
        "qgis_html_to_pdf binary not found, skipping test suite"
    )
    unittest.main()
