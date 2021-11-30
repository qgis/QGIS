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
    QgsGroupLayer,
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

    def test_updateLayerPath(self):
        """
        Test QgsProjectUtils.updateLayerPath
        """
        self.assertFalse(QgsProjectUtils.updateLayerPath(None, '', ''))
        self.assertFalse(QgsProjectUtils.updateLayerPath(None, 'aaaaa', 'bbbb'))
        p = QgsProject()
        self.assertFalse(QgsProjectUtils.updateLayerPath(p, 'aaaaa', 'bbbb'))

        # add some layers to a project
        # shapefile
        layer1 = QgsVectorLayer(unitTestDataPath() + '/points.shp', 'l1')
        self.assertTrue(layer1.isValid())
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

        memory_layer = QgsVectorLayer("Point?field=x:string", 'my layer', "memory")
        old_memory_source = memory_layer.source()
        p.addMapLayer(memory_layer)

        self.assertFalse(QgsProjectUtils.updateLayerPath(p, '', ''))
        self.assertFalse(QgsProjectUtils.updateLayerPath(p, 'aaa', 'bbb'))

        # replace shapefile path
        self.assertTrue(QgsProjectUtils.updateLayerPath(p, unitTestDataPath() + '/points.shp', unitTestDataPath() + '/points22.shp'))
        self.assertEqual(layer1.source(), unitTestDataPath() + '/points22.shp')
        self.assertEqual(gpkg1.source(), unitTestDataPath() + '/mixed_layers.gpkg|layername=lines')
        self.assertEqual(gpkg2.source(), unitTestDataPath() + '/mixed_layers.gpkg|layername=points')
        self.assertEqual(rl.source(), f'GPKG:{unitTestDataPath()}/mixed_layers.gpkg:band1')
        self.assertEqual(memory_layer.source(), old_memory_source)
        # should return false if we call again, no more matching paths
        self.assertFalse(QgsProjectUtils.updateLayerPath(p, unitTestDataPath() + '/points.shp',
                                                         unitTestDataPath() + '/points22.shp'))

        # replace geopackage path
        self.assertTrue(QgsProjectUtils.updateLayerPath(p, unitTestDataPath() + '/mixed_layers.gpkg', unitTestDataPath() + '/mixed_layers22.gpkg'))
        self.assertEqual(layer1.source(), unitTestDataPath() + '/points22.shp')
        self.assertEqual(gpkg1.source(), unitTestDataPath() + '/mixed_layers22.gpkg|layername=lines')
        self.assertEqual(gpkg2.source(), unitTestDataPath() + '/mixed_layers22.gpkg|layername=points')
        self.assertEqual(rl.source(), f'GPKG:{unitTestDataPath()}/mixed_layers22.gpkg:band1')
        self.assertEqual(memory_layer.source(), old_memory_source)
        # should return false if we call again, no more matching paths
        self.assertFalse(QgsProjectUtils.updateLayerPath(p, unitTestDataPath() + '/mixed_layers.gpkg',
                                                         unitTestDataPath() + '/mixed_layers22.gpkg'))

    def test_layer_is_contained_in_group_layer(self):
        p = QgsProject()
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        p.addMapLayer(layer)
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        p.addMapLayer(layer2)
        layer3 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer3", "memory")
        p.addMapLayer(layer3)
        layer4 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer4", "memory")
        p.addMapLayer(layer4)

        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = QgsGroupLayer('group', options)
        group_layer.setChildLayers([layer, layer4])
        p.addMapLayer(group_layer)

        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer2 = QgsGroupLayer('group2', options)
        group_layer2.setChildLayers([group_layer, layer3])
        p.addMapLayer(group_layer2)

        self.assertTrue(QgsProjectUtils.layerIsContainedInGroupLayer(p, layer))
        self.assertFalse(QgsProjectUtils.layerIsContainedInGroupLayer(p, layer2))
        self.assertTrue(QgsProjectUtils.layerIsContainedInGroupLayer(p, layer3))
        self.assertTrue(QgsProjectUtils.layerIsContainedInGroupLayer(p, layer4))


if __name__ == '__main__':
    unittest.main()
