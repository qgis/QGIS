# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPathResolver.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '22/07/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA

import tempfile
import os
import gc
from qgis.core import (
    QgsPathResolver,
    QgsVectorLayer,
    QgsProject,
    QgsApplication
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestQgsPathResolver(unittest.TestCase):

    def testCustomPreprocessor(self):
        self.assertEqual(QgsPathResolver().readPath('aaaaa'), 'aaaaa')
        with self.assertRaises(KeyError):
            QgsPathResolver().removePathPreprocessor('bad')

        def run_test():
            def my_processor(path):
                return path.upper()

            id = QgsPathResolver.setPathPreprocessor(my_processor)
            self.assertTrue(id)
            self.assertEqual(QgsPathResolver().readPath('aaaaa'), 'AAAAA')
            return id

        id = run_test()
        gc.collect()
        # my_processor should be out of scope and cleaned up, unless things are working
        # correctly and ownership was transferred
        self.assertEqual(QgsPathResolver().readPath('aaaaa'), 'AAAAA')

        QgsPathResolver().removePathPreprocessor(id)
        self.assertEqual(QgsPathResolver().readPath('aaaaa'), 'aaaaa')

        # expect key error
        with self.assertRaises(KeyError):
            QgsPathResolver().removePathPreprocessor(id)

    def testChainedPreprocessors(self):
        """
        Test that chaining preprocessors works correctly
        """
        self.assertEqual(QgsPathResolver().readPath('aaaaa'), 'aaaaa')

        def run_test():
            def my_processor(path):
                return 'x' + path + 'x'

            def my_processor2(path):
                return 'y' + path + 'y'

            id = QgsPathResolver.setPathPreprocessor(my_processor)
            self.assertTrue(id)

            self.assertEqual(QgsPathResolver().readPath('aaaaa'), 'xaaaaax')

            id2 = QgsPathResolver.setPathPreprocessor(my_processor2)
            self.assertTrue(id2)

            self.assertEqual(QgsPathResolver().readPath('aaaaa'), 'yxaaaaaxy')

            return id, id2

        id, id2 = run_test()
        gc.collect()
        # my_processor should be out of scope and cleaned up, unless things are working
        # correctly and ownership was transferred
        self.assertEqual(QgsPathResolver().readPath('aaaaa'), 'yxaaaaaxy')

        QgsPathResolver().removePathPreprocessor(id)
        self.assertEqual(QgsPathResolver().readPath('aaaaa'), 'yaaaaay')

        # expect key error
        with self.assertRaises(KeyError):
            QgsPathResolver().removePathPreprocessor(id)

        QgsPathResolver().removePathPreprocessor(id2)
        self.assertEqual(QgsPathResolver().readPath('aaaaa'), 'aaaaa')

        with self.assertRaises(KeyError):
            QgsPathResolver().removePathPreprocessor(id2)

    def testLoadLayerWithPreprocessor(self):
        """
        Test that custom path preprocessor is used when loading layers
        """
        lines_shp_path = os.path.join(TEST_DATA_DIR, 'moooooo.shp')

        lines_layer = QgsVectorLayer(lines_shp_path, 'Lines', 'ogr')
        self.assertFalse(lines_layer.isValid())
        p = QgsProject()
        p.addMapLayer(lines_layer)
        # save project to a temporary file
        temp_path = tempfile.mkdtemp()
        temp_project_path = os.path.join(temp_path, 'temp.qgs')
        self.assertTrue(p.write(temp_project_path))

        p2 = QgsProject()
        self.assertTrue(p2.read(temp_project_path))
        l = p2.mapLayersByName('Lines')[0]
        self.assertEqual(l.name(), 'Lines')
        self.assertFalse(l.isValid())

        # custom processor to fix path
        def my_processor(path):
            return path.replace('moooooo', 'lines')

        QgsPathResolver.setPathPreprocessor(my_processor)
        p3 = QgsProject()
        self.assertTrue(p3.read(temp_project_path))
        l = p3.mapLayersByName('Lines')[0]
        self.assertEqual(l.name(), 'Lines')
        # layer should have correct path now
        self.assertTrue(l.isValid())

    def testInbuiltPath(self):
        """
        Test resolving and saving inbuilt data paths
        """
        path = "inbuilt:/data/world_map.shp"
        self.assertEqual(QgsPathResolver().readPath(path), QgsApplication.pkgDataPath() + '/resources/data/world_map.shp')

        self.assertEqual(QgsPathResolver().writePath(QgsApplication.pkgDataPath() + '/resources/data/world_map.shp'), 'inbuilt:/data/world_map.shp')


if __name__ == '__main__':
    unittest.main()
