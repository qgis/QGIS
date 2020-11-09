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

import qgis  # NOQA

import tempfile
import os
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

    def testWildcardsFromFilter(self):
        self.assertEqual(QgsFileUtils.wildcardsFromFilter(''), '')
        self.assertEqual(QgsFileUtils.wildcardsFromFilter('bad'), '')
        self.assertEqual(QgsFileUtils.wildcardsFromFilter('*'), '')
        self.assertEqual(QgsFileUtils.wildcardsFromFilter('*.'), '')
        self.assertEqual(QgsFileUtils.wildcardsFromFilter('Tiff files'), '')
        self.assertEqual(QgsFileUtils.wildcardsFromFilter('(*.*)'), '*.*')
        self.assertEqual(QgsFileUtils.wildcardsFromFilter('PNG Files (*.png)'), '*.png')
        self.assertEqual(QgsFileUtils.wildcardsFromFilter('Tif  Files (*.tif)'), '*.tif')
        self.assertEqual(QgsFileUtils.wildcardsFromFilter('PNG Files (*.PNG)'), '*.PNG')
        self.assertEqual(QgsFileUtils.wildcardsFromFilter('Geotiff Files (*.tiff *.tif)'), '*.tiff *.tif')
        self.assertEqual(QgsFileUtils.wildcardsFromFilter('Geotiff Files (*.tiff *.tif *.TIFF)'), '*.tiff *.tif *.TIFF')
        self.assertEqual(QgsFileUtils.wildcardsFromFilter('EPT files (ept.json)'), 'ept.json')
        self.assertEqual(QgsFileUtils.wildcardsFromFilter('EPT files (ept.json EPT.JSON)'), 'ept.json EPT.JSON')

    def testFileMatchesFilter(self):
        self.assertFalse(QgsFileUtils.fileMatchesFilter('/home/me/test.tif', ''))
        self.assertFalse(QgsFileUtils.fileMatchesFilter('/home/me/test.tif', 'bad'))
        self.assertFalse(QgsFileUtils.fileMatchesFilter('/home/me/test.tif', '*'))
        self.assertFalse(QgsFileUtils.fileMatchesFilter('/home/me/test.tif', '*.'))
        self.assertFalse(QgsFileUtils.fileMatchesFilter('/home/me/test.tif', 'Tiff files'))
        self.assertTrue(QgsFileUtils.fileMatchesFilter('/home/me/test.tif', '(*.*)'))
        self.assertFalse(QgsFileUtils.fileMatchesFilter('/home/me/test.tif', 'PNG Files (*.png)'))
        self.assertTrue(QgsFileUtils.fileMatchesFilter('/home/me/test.tif', 'Tif  Files (*.tif)'))
        self.assertFalse(QgsFileUtils.fileMatchesFilter('/home/me/test.tif', 'PNG Files (*.PNG)'))
        self.assertFalse(QgsFileUtils.fileMatchesFilter('/home/me/test.tif', 'Tif  Files (*.TIF)'))
        self.assertTrue(QgsFileUtils.fileMatchesFilter('/home/me/test.tif', 'Geotiff Files (*.tiff *.tif)'))
        self.assertTrue(QgsFileUtils.fileMatchesFilter('/home/me/test.tiff', 'Geotiff Files (*.tiff *.tif)'))
        self.assertTrue(QgsFileUtils.fileMatchesFilter('/home/me/test.TIFF', 'Geotiff Files (*.tiff *.tif *.TIFF)'))
        self.assertFalse(QgsFileUtils.fileMatchesFilter('/home/me/test.tif', 'PNG Files (*.png);;BMP Files (*.bmp)'))
        self.assertTrue(QgsFileUtils.fileMatchesFilter('/home/me/test.bmp', 'PNG Files (*.png);;BMP Files (*.bmp)'))
        self.assertTrue(QgsFileUtils.fileMatchesFilter('/home/me/test.png', 'PNG Files (*.png);;BMP Files (*.bmp)'))
        self.assertFalse(QgsFileUtils.fileMatchesFilter('/home/me/test.png', 'EPT files (ept.json)'))
        self.assertTrue(QgsFileUtils.fileMatchesFilter('/home/me/ept.json', 'EPT files (ept.json)'))
        self.assertTrue(QgsFileUtils.fileMatchesFilter('/home/me/ept.json', 'EPT files (ept.json EPT.JSON)'))
        self.assertTrue(QgsFileUtils.fileMatchesFilter('/home/me/EPT.JSON', 'EPT files (ept.json EPT.JSON)'))
        self.assertTrue(QgsFileUtils.fileMatchesFilter('/home/me/ept.json', 'EPT files (ept.json);;Entwine files (entwine.json)'))
        self.assertTrue(
            QgsFileUtils.fileMatchesFilter('/home/me/entwine.json', 'EPT files (ept.json);;Entwine files (entwine.json)'))
        self.assertFalse(
            QgsFileUtils.fileMatchesFilter('/home/me/ep.json', 'EPT files (ept.json);;Entwine files (entwine.json)'))

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

    def testFindClosestExistingPath(self):
        self.assertEqual(QgsFileUtils.findClosestExistingPath(''), '')
        self.assertEqual(QgsFileUtils.findClosestExistingPath('.'), '')
        self.assertEqual(QgsFileUtils.findClosestExistingPath('just_a_filename'), '')
        self.assertEqual(QgsFileUtils.findClosestExistingPath('just_a_filename.txt'), '')
        self.assertEqual(QgsFileUtils.findClosestExistingPath(
            'a_very_unlikely_path_to_really_exist/because/no_one_would_have_a_folder_called/MapInfo is the bestest/'),
            '')
        # sorry anyone not on linux!
        self.assertEqual(QgsFileUtils.findClosestExistingPath('/usr/youve_been_hacked/by_the_l77t_krew'), '/usr')

        base_path = tempfile.mkdtemp()
        file = os.path.join(base_path, 'test.csv')
        with open(file, 'wt') as f:
            f.write('\n')

        self.assertEqual(QgsFileUtils.findClosestExistingPath(os.path.join(base_path, 'a file name.bmp')),
                         base_path)  # non-existent file
        self.assertEqual(QgsFileUtils.findClosestExistingPath(file), base_path)  # real file!
        self.assertEqual(QgsFileUtils.findClosestExistingPath(os.path.join(base_path, 'non/existent/subfolder')),
                         base_path)

        sub_folder1 = os.path.join(base_path, 'subfolder1')
        os.mkdir(sub_folder1)
        sub_folder2 = os.path.join(sub_folder1, 'subfolder2')
        os.mkdir(sub_folder2)
        bad_sub_folder = os.path.join(sub_folder2, 'nooo')
        self.assertEqual(QgsFileUtils.findClosestExistingPath(bad_sub_folder), sub_folder2)
        self.assertEqual(QgsFileUtils.findClosestExistingPath(sub_folder2), sub_folder2)
        self.assertEqual(QgsFileUtils.findClosestExistingPath(sub_folder2 + '/.'), sub_folder2)
        self.assertEqual(QgsFileUtils.findClosestExistingPath(sub_folder2 + '/..'), sub_folder1)
        self.assertEqual(QgsFileUtils.findClosestExistingPath(sub_folder2 + '/../ddddddd'), sub_folder1)
        self.assertEqual(QgsFileUtils.findClosestExistingPath(sub_folder2 + '/../subfolder2'), sub_folder2)
        self.assertEqual(QgsFileUtils.findClosestExistingPath(sub_folder2 + '/../subfolder2/zxcv/asfdasd'), sub_folder2)

    def testAutoFinder(self):
        temp_folder = tempfile.mkdtemp()
        base_folder = os.path.join(temp_folder, 'base_level')
        os.mkdir(base_folder)
        side_fold = os.path.join(temp_folder, 'sidefold')
        os.mkdir(side_fold)
        nest = os.path.join(base_folder, 'direct_nest')
        os.mkdir(nest)
        side_nest = os.path.join(side_fold, 'side_nest')
        os.mkdir(side_nest)
        filename = "findme.txt"

        # unexisting
        files = QgsFileUtils.findFile(filename, nest, 1, 4)
        self.assertEqual(len(files), 0)
        # out of depth
        files = QgsFileUtils.findFile(filename, nest, 0, 4)
        self.assertEqual(len(files), 0)
        # too close
        files = QgsFileUtils.findFile(filename, nest, 1, 13)
        self.assertEqual(len(files), 0)
        # side nest
        with open(os.path.join(side_nest, filename), 'w+'):
            files = QgsFileUtils.findFile(os.path.join(base_folder, filename))
            self.assertEqual(files[0], os.path.join(side_nest, filename).replace(os.sep, '/'))
        # side + side nest  =  2
        with open(os.path.join(side_fold, filename), 'w+'):
            files = QgsFileUtils.findFile(filename, base_folder, 3, 4)
            self.assertEqual(len(files), 2)
        # up
        with open(os.path.join(temp_folder, filename), 'w+'):
            files = QgsFileUtils.findFile(filename, base_folder, 3, 4)
            self.assertEqual(files[0], os.path.join(temp_folder, filename).replace(os.sep, '/'))
        # nest
        with open(os.path.join(nest, filename), 'w+'):
            files = QgsFileUtils.findFile(os.path.join(base_folder, filename))
            self.assertEqual(files[0], os.path.join(nest, filename).replace(os.sep, '/'))
        # base level
        with open(os.path.join(base_folder, filename), 'w+'):
            files = QgsFileUtils.findFile(filename, base_folder, 2, 4)
            self.assertEqual(files[0], os.path.join(base_folder, filename).replace(os.sep, '/'))
        # invalid path, too deep
        files = QgsFileUtils.findFile(filename, os.path.join(nest, 'nest2'), 2, 4)
        self.assertEqual(files[0], os.path.join(nest, filename).replace(os.sep, '/'))


if __name__ == '__main__':
    unittest.main()
