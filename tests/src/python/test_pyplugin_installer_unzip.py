"""QGIS Unit tests for the Plugin installer.

From build dir, run: ctest -R PyQgsPluginInstaller -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Till Frankenbach"
__date__ = "2026-02-26"
__copyright__ = "Copyright 2025, Till Frankenbach"

import os.path
import tempfile
import unittest
import zipfile

from pyplugin_installer.unzip import unzip
from qgis.testing import QgisTestCase


class TestPyQgsPluginInstaller(QgisTestCase):
    def test_unzip_creates_files_and_directories(self):
        with tempfile.TemporaryDirectory() as td:
            zip_path = os.path.join(td, "test.zip")

            with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_DEFLATED) as zf:
                zf.writestr("dir1/inner.txt", "hello world")
                zf.writestr("root.txt", "root content")

            with tempfile.TemporaryDirectory() as outdir:
                unzip(zip_path, outdir)
                # assert files exist and contents match
                f1 = os.path.join(outdir, "dir1", "inner.txt")
                f2 = os.path.join(outdir, "root.txt")
                self.assertTrue(os.path.isfile(f1))
                self.assertTrue(os.path.isfile(f2))
                with open(f1, encoding="utf8") as fh:
                    self.assertEqual(fh.read(), "hello world")
                with open(f2, encoding="utf8") as fh:
                    self.assertEqual(fh.read(), "root content")

    def test_unzip_fails_with_nested_path(self):
        with tempfile.TemporaryDirectory() as td:
            nested_path = "../../dir1/inner.txt"
            zip_path = os.path.join(td, "test.zip")

            with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_DEFLATED) as zf:
                zf.writestr(nested_path, "i'm not supposed to be here")
                zf.writestr("root.txt", "root content")

            with tempfile.TemporaryDirectory() as outdir:
                unzip(zip_path, outdir)
                # the nested file should not be extracted outside of the target directory
                self.assertFalse(os.path.isfile(os.path.join(outdir, nested_path)))
                # valid file should still be extracted
                self.assertTrue(os.path.isfile(os.path.join(outdir, "root.txt")))


if __name__ == "__main__":
    unittest.main()
