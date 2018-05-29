# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFileUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/12/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import QgsFileUtils
from qgis.testing import unittest


class TestQgsFileUtils(unittest.TestCase):

    def testExtensionsFromFilter(self):
        self.assertEqual(QgsFileUtils.extensionsFromFilter(''), [])
        self.assertEqual(QgsFileUtils.extensionsFromFilter('bad'), [])
        self.assertEqual(QgsFileUtils.extensionsFromFilter('*'), [])
        self.assertEqual(QgsFileUtils.extensionsFromFilter('*.'), [])
        self.assertEqual(QgsFileUtils.extensionsFromFilter('Tiff files'), [])
        self.assertEqual(QgsFileUtils.extensionsFromFilter('(*.)'), [])
        self.assertEqual(QgsFileUtils.extensionsFromFilter('PNG Files (*.png)'), ['png'])
        self.assertEqual(QgsFileUtils.extensionsFromFilter('PNG Files (*.PNG)'), ['PNG'])
        self.assertEqual(QgsFileUtils.extensionsFromFilter('Geotiff Files (*.tiff *.tif)'), ['tiff', 'tif'])

    def testEnsureFileNameHasExtension(self):
        self.assertEqual(QgsFileUtils.ensureFileNameHasExtension('', ['']), '')
        self.assertEqual(QgsFileUtils.ensureFileNameHasExtension('', []), '')
        self.assertEqual(QgsFileUtils.ensureFileNameHasExtension('test', []), 'test')
        self.assertEqual(QgsFileUtils.ensureFileNameHasExtension('', ['.tif']), '')
        self.assertEqual(QgsFileUtils.ensureFileNameHasExtension('test', ['.tif']), 'test.tif')
        self.assertEqual(QgsFileUtils.ensureFileNameHasExtension('test', ['tif']), 'test.tif')
        self.assertEqual(QgsFileUtils.ensureFileNameHasExtension('test.tif', []), 'test.tif')
        self.assertEqual(QgsFileUtils.ensureFileNameHasExtension('test.tif', ['bmp']), 'test.tif.bmp')
        self.assertEqual(QgsFileUtils.ensureFileNameHasExtension('test.tif', ['tiff']), 'test.tif.tiff')
        self.assertEqual(QgsFileUtils.ensureFileNameHasExtension('test.tif', ['tiff', 'tif']), 'test.tif')
        self.assertEqual(QgsFileUtils.ensureFileNameHasExtension('test.tif', ['TIFF', 'TIF']), 'test.tif')

    def testAddExtensionFromFilter(self):
        self.assertEqual(QgsFileUtils.addExtensionFromFilter('test', 'TIFF Files (*.tif)'), 'test.tif')
        self.assertEqual(QgsFileUtils.addExtensionFromFilter('test', 'TIFF Files (*.tif)'), 'test.tif')
        self.assertEqual(QgsFileUtils.addExtensionFromFilter('test.tif', ''), 'test.tif')
        self.assertEqual(QgsFileUtils.addExtensionFromFilter('test.tif', 'BMP Files (*.bmp)'), 'test.tif.bmp')
        self.assertEqual(QgsFileUtils.addExtensionFromFilter('test.tif', 'TIFF Files (*.tiff)'), 'test.tif.tiff')
        self.assertEqual(QgsFileUtils.addExtensionFromFilter('test.tif', 'TIFF Files (*.tif *.tiff)'), 'test.tif')
        self.assertEqual(QgsFileUtils.addExtensionFromFilter('test.tif', 'TIFF Files (*.TIF *.TIFF)'), 'test.tif')
        self.assertEqual(QgsFileUtils.addExtensionFromFilter('test.tif', 'All Files (*.*)'), 'test.tif')
        self.assertEqual(QgsFileUtils.addExtensionFromFilter('test', 'All Files (*.*)'), 'test')

    def testStringToSafeFilename(self):
        self.assertEqual(QgsFileUtils.stringToSafeFilename('my FiLe v2.0_new.tif'), 'my FiLe v2.0_new.tif')
        self.assertEqual(
            QgsFileUtils.stringToSafeFilename('rendered map_final? rev (12-03-1017)_real/\\?%*:|"<>.tif'),
            'rendered map_final_ rev (12-03-1017)_real__________.tif')


if __name__ == '__main__':
    unittest.main()
