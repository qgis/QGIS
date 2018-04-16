# -*- coding: utf-8 -*-
"""
Test the QgsSlopeFilter class

Run with: ctest -V -R PyQgsOpenCL

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import tempfile
import shutil
from filecmp import cmp
from qgis.analysis import (QgsSlopeFilter, QgsAspectFilter)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

__author__ = 'Alessandro Pasotti'
__date__ = '09/04/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

TEST_DATA_DIR = unitTestDataPath()

start_app()


class TestQgsSettings(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.input = os.path.join(TEST_DATA_DIR, 'opencl', 'dem.tif')
        cls.expected_slope = os.path.join(TEST_DATA_DIR, 'opencl', 'slope.tif')
        cls.expected_slope = os.path.join(TEST_DATA_DIR, 'opencl', 'aspect.tif')

        cls.tempdir = tempfile.mkdtemp()
        cls.output_slope = os.path.join(tempfile.mkdtemp(), 'slope.tif')
        cls.output_aspect = os.path.join(tempfile.mkdtemp(), 'aspect.tif')

    @classmethod
    def tearDownClass(cls):
        print(cls.output_slope)
        print(cls.output_aspect)
        #shutil.rmtree(self.tempdir)

    def test_slope(self):
        filter = QgsSlopeFilter(self.input, self.output_slope, 'GTiff')
        self.assertEqual(filter.processRaster(), 0)
        self.assertTrue(cmp(self.output_slope, self.expected_slope, False))

    def test_aspect(self):
        filter = QgsAspectFilter(self.input, self.output_aspect, 'GTiff')
        self.assertEqual(filter.processRaster(), 0)
        self.assertTrue(cmp(self.output_aspect, self.expected_aspect, False))


if __name__ == '__main__':
    unittest.main()
