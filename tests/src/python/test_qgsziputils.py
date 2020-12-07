# -*- coding: utf-8 -*-
"""QGIS Unit tests for zip functions.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Paul Blottiere'
__date__ = '06/7/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import os
from qgis.core import QgsZipUtils
from qgis.testing import unittest
from utilities import unitTestDataPath
from qgis.PyQt.QtCore import QTemporaryFile, QTemporaryDir


def tmpPath():
    f = QTemporaryFile()
    f.open()
    f.close()
    os.remove(f.fileName())

    return f.fileName()


class TestQgsZip(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.zipDir = os.path.join(unitTestDataPath(), "zip")

    def test_zip_ok(self):
        f0 = os.path.join(unitTestDataPath(), 'multipoint.shp')
        f1 = os.path.join(unitTestDataPath(), 'lines.shp')
        f2 = os.path.join(unitTestDataPath(), 'joins.qgs')

        rc = QgsZipUtils.zip(tmpPath(), [f0, f1, f2])
        self.assertTrue(rc)

    def test_zip_file_yet_exist(self):
        zip = QTemporaryFile()
        zip.open()
        zip.close()
        os.remove(zip.fileName())

        f0 = os.path.join(unitTestDataPath(), 'multipoint.shp')
        f1 = os.path.join(unitTestDataPath(), 'lines.shp')
        f2 = os.path.join(unitTestDataPath(), 'joins.qgs')

        rc = QgsZipUtils.zip(zip.fileName(), [f0, f1, f2])
        self.assertTrue(rc)

        rc = QgsZipUtils.zip(zip.fileName(), [f0, f1, f2])
        self.assertFalse(rc)

    def test_zip_file_empty(self):
        f0 = os.path.join(unitTestDataPath(), 'multipoint.shp')
        f1 = os.path.join(unitTestDataPath(), 'lines.shp')
        f2 = os.path.join(unitTestDataPath(), 'joins.qgs')

        rc = QgsZipUtils.zip("", [f0, f1, f2])
        self.assertFalse(rc)

    def test_zip_input_file_not_exist(self):
        f0 = os.path.join(unitTestDataPath(), 'multipoint.shp')
        f1 = os.path.join(unitTestDataPath(), 'fake.shp')
        f2 = os.path.join(unitTestDataPath(), 'joins.qgs')

        rc = QgsZipUtils.zip(tmpPath(), [f0, f1, f2])
        self.assertFalse(rc)

    def test_unzip_ok(self):
        outDir = QTemporaryDir()

        zip = os.path.join(self.zipDir, 'testzip.zip')
        rc, files = QgsZipUtils.unzip(zip, outDir.path())

        self.assertTrue(rc)
        self.assertEqual(len(files), 11)

    def test_unzip_file_not_exist(self):
        outDir = QTemporaryDir()

        zip = os.path.join(self.zipDir, 'fake.zip')
        rc, files = QgsZipUtils.unzip(zip, outDir.path())

        self.assertFalse(rc)

    def test_unzip_file_empty(self):
        outDir = QTemporaryDir()
        rc, files = QgsZipUtils.unzip("", outDir.path())
        self.assertFalse(rc)

    def test_unzip_dir_not_exist(self):
        zip = os.path.join(self.zipDir, 'testzip.zip')
        rc, files = QgsZipUtils.unzip(zip, '/tmp/fake')
        self.assertFalse(rc)

    def test_unzip_dir_empty(self):
        zip = os.path.join(self.zipDir, 'testzip.zip')
        rc, files = QgsZipUtils.unzip(zip, '')
        self.assertFalse(rc)

    def test_zip_unzip_ok(self):
        zip = tmpPath()

        f0 = os.path.join(unitTestDataPath(), 'multipoint.shp')
        f1 = os.path.join(unitTestDataPath(), 'lines.shp')
        f2 = os.path.join(unitTestDataPath(), 'joins.qgs')

        rc = QgsZipUtils.zip(zip, [f0, f1, f2])
        self.assertTrue(rc)

        outDir = QTemporaryDir()
        rc, files = QgsZipUtils.unzip(zip, outDir.path())

        self.assertTrue(rc)
        self.assertEqual(len(files), 3)


if __name__ == '__main__':
    unittest.main()
