# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProjectUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2021-07'
__copyright__ = 'Copyright 2021, The QGIS Project'


import qgis  # NOQA

from qgis.testing import unittest
from qgis.core import (
    QgsProjectUtils,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransformContext,
    QgsVectorLayer,
    QgsRasterLayer,
    QgsProject
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsProjectUtils(unittest.TestCase):

    def test_layersMatchingPath(self):
        """
        Test QgsProjectUtils.layersMatchingPath()
        """
        self.assertFalse(QgsProjectUtils.layersMatchingPath(None, ''))
        self.assertFalse(QgsProjectUtils.layersMatchingPath(None, 'aaaaa'))

        # add some layers to a project
        # shapefile
        layer1 = QgsVectorLayer(unitTestDataPath() + '/points.shp', 'l1')
        self.assertTrue(layer1.isValid())
        p = QgsProject()
        p.addMapLayer(layer1)

        gpkg1 = QgsVectorLayer(unitTestDataPath() + '/mixed_layers.gpkg|layername=lines', 'l1')
        self.assertTrue(gpkg1.isValid())
        p.addMapLayer(gpkg1)

        gpkg2 = QgsVectorLayer(unitTestDataPath() + '/mixed_layers.gpkg|layername=points', 'l1')
        self.assertTrue(gpkg2.isValid())
        p.addMapLayer(gpkg2)

        # raster layer from gpkg
        rl = QgsRasterLayer(f'GPKG:{unitTestDataPath()}/mixed_layers.gpkg:band1')
        self.assertTrue(rl.isValid())
        p.addMapLayer(rl)

        self.assertFalse(QgsProjectUtils.layersMatchingPath(p, ''))
        self.assertFalse(QgsProjectUtils.layersMatchingPath(p, 'aaa'))
        self.assertCountEqual(QgsProjectUtils.layersMatchingPath(p, unitTestDataPath() + '/points.shp'), [layer1])
        self.assertCountEqual(QgsProjectUtils.layersMatchingPath(p, unitTestDataPath() + '/mixed_layers.gpkg'), [gpkg1, gpkg2, rl])


if __name__ == '__main__':
    unittest.main()
