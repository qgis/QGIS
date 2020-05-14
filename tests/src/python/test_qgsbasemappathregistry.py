# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsBasemapPathRegistry.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Denis Rouzaud'
__date__ = '13/05/2020'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA

from tempfile import NamedTemporaryFile, gettempdir
from pathlib import Path
import os
import gc
from qgis.core import (
    QgsBasemapPathRegistry,
    QgsApplication,
    QgsPathResolver,
    QgsProject,
    QgsVectorLayer
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()

MAP_PATH = "data/world_map.gpkg"
BASE_PATH = QgsApplication.pkgDataPath() + '/resources'
ABSOLUTE_PATH = '{}/{}'.format(BASE_PATH, MAP_PATH)


class TestQgsBasemapPathRegistry(unittest.TestCase):
    """
    Test resolving and saving basemap data paths
    """

    def setUp(self):
        QgsApplication.basemapPathRegistry().registerPath(BASE_PATH)

    def tearDown(self):
        QgsApplication.basemapPathRegistry().unregisterPath(BASE_PATH)

    def testQgsBasemapPathRegistry(self):
        self.assertEqual(QgsApplication.basemapPathRegistry().relativePath(ABSOLUTE_PATH), MAP_PATH)
        self.assertEqual(QgsApplication.basemapPathRegistry().fullPath(MAP_PATH), ABSOLUTE_PATH)

    def testOrderOfPreference(self):
        temp_dir = gettempdir()
        alt_dir = '{}/{}'.format(temp_dir, MAP_PATH)
        Path(alt_dir).touch()
        QgsApplication.basemapPathRegistry().registerPath(temp_dir, 0)
        self.assertEqual(QgsApplication.basemapPathRegistry().fullPath(MAP_PATH), alt_dir)
        QgsApplication.basemapPathRegistry().unregisterPath(temp_dir)

    def testWithResolver(self):
        self.assertEqual(QgsPathResolver().readPath('basemap:' + MAP_PATH), ABSOLUTE_PATH)
        self.assertEqual(QgsPathResolver().writePath(ABSOLUTE_PATH), 'basemap:' + MAP_PATH)

    def testProject(self):
        layer = QgsVectorLayer('{}|layername=countries'.format(ABSOLUTE_PATH), 'Test', 'ogr')

        # write
        p = QgsProject()
        fh = NamedTemporaryFile(delete=False)
        p.setFileName(fh.name)
        p.addMapLayer(layer)
        self.assertTrue(p.write())
        found = False
        with open(fh.name) as fh:
            for line in fh:
                if '<datasource>basemap:data/world_map.gpkg|layername=countries</datasource>' in line:
                    found = True
                    break
        self.assertTrue(found)

        # read
        p2 = QgsProject()
        p2.setFileName(fh.name)
        p2.read()
        self.assertTrue(len(p2.mapLayers()))
        self.assertEqual(p2.mapLayers()[layer.id()].source(), '{}/{}|layername=countries'.format(BASE_PATH, MAP_PATH))

        os.remove(fh.name)


if __name__ == '__main__':
    unittest.main()
