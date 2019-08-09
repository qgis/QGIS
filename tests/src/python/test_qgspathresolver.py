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
from qgis.core import QgsPathResolver, QgsVectorLayer, QgsProject
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestQgsPathResolver(unittest.TestCase):

    def testCustomPreprocessor(self):
        self.assertEqual(QgsPathResolver().readPath('aaaaa'), 'aaaaa')

        def run_test():
            def my_processor(path):
                return path.upper()

            QgsPathResolver.setPathPreprocessor(my_processor)
            self.assertEqual(QgsPathResolver().readPath('aaaaa'), 'AAAAA')

        run_test()
        gc.collect()
        # my_processor should be out of scope and cleaned up, unless things are working
        # correctly and ownership was transferred
        self.assertEqual(QgsPathResolver().readPath('aaaaa'), 'AAAAA')

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


if __name__ == '__main__':
    unittest.main()
