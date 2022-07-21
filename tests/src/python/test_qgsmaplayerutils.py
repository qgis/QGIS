# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapLayerUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2021-05'
__copyright__ = 'Copyright 2021, The QGIS Project'


import qgis  # NOQA

from qgis.testing import unittest
from qgis.core import (
    QgsMapLayerUtils,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransformContext,
    QgsVectorLayer,
    QgsRasterLayer,
    QgsAnnotationLayer,
    QgsGroupLayer,
    QgsRectangle,
    QgsProject,
    QgsMapLayerType
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsMapLayerUtils(unittest.TestCase):

    def testCombinedExtent(self):
        extent = QgsMapLayerUtils.combinedExtent([], QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext())
        self.assertTrue(extent.isEmpty())

        layer1 = QgsVectorLayer(unitTestDataPath() + '/points.shp', 'l1')
        self.assertTrue(layer1.isValid())

        # one layer
        extent = QgsMapLayerUtils.combinedExtent([layer1], QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext())
        self.assertEqual(extent.toString(3), '-118.889,22.800 : -83.333,46.872')

        extent = QgsMapLayerUtils.combinedExtent([layer1], QgsCoordinateReferenceSystem('EPSG:4326'),
                                                 QgsCoordinateTransformContext())
        self.assertEqual(extent.toString(3), '-118.889,22.800 : -83.333,46.872')
        extent = QgsMapLayerUtils.combinedExtent([layer1], QgsCoordinateReferenceSystem('EPSG:3857'),
                                                 QgsCoordinateTransformContext())
        self.assertEqual(extent.toString(0), '-13234651,2607875 : -9276624,5921203')

        # two layers
        layer2 = QgsRasterLayer(unitTestDataPath() + '/landsat-f32-b1.tif', 'l2')
        self.assertTrue(layer2.isValid())
        extent = QgsMapLayerUtils.combinedExtent([layer1, layer2], QgsCoordinateReferenceSystem('EPSG:4326'),
                                                 QgsCoordinateTransformContext())
        self.assertEqual(extent.toString(3), '-118.889,22.800 : 18.046,46.872')
        extent = QgsMapLayerUtils.combinedExtent([layer2, layer1], QgsCoordinateReferenceSystem('EPSG:4326'),
                                                 QgsCoordinateTransformContext())
        self.assertEqual(extent.toString(3), '-118.889,22.800 : 18.046,46.872')
        extent = QgsMapLayerUtils.combinedExtent([layer1, layer2], QgsCoordinateReferenceSystem('EPSG:3857'),
                                                 QgsCoordinateTransformContext())
        self.assertEqual(extent.toString(0), '-13234651,2607875 : 2008833,5921203')
        extent = QgsMapLayerUtils.combinedExtent([layer2, layer1], QgsCoordinateReferenceSystem('EPSG:3857'),
                                                 QgsCoordinateTransformContext())
        self.assertEqual(extent.toString(0), '-13234651,2607875 : 2008833,5921203')

    def test_layerSourceMatchesPath(self):
        """
        Test QgsMapLayerUtils.layerSourceMatchesPath()
        """
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(None, ''))
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(None, 'aaaaa'))

        # shapefile
        layer1 = QgsVectorLayer(unitTestDataPath() + '/points.shp', 'l1')
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(layer1, ''))
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(layer1, 'aaaaa'))
        self.assertTrue(QgsMapLayerUtils.layerSourceMatchesPath(layer1, unitTestDataPath() + '/points.shp'))

        # geopackage with layers
        layer1 = QgsVectorLayer(unitTestDataPath() + '/mixed_layers.gpkg|layername=lines', 'l1')
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(layer1, ''))
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(layer1, 'aaaaa'))
        self.assertTrue(QgsMapLayerUtils.layerSourceMatchesPath(layer1, unitTestDataPath() + '/mixed_layers.gpkg'))
        layer2 = QgsVectorLayer(unitTestDataPath() + '/mixed_layers.gpkg|layername=points', 'l1')
        self.assertTrue(QgsMapLayerUtils.layerSourceMatchesPath(layer2, unitTestDataPath() + '/mixed_layers.gpkg'))

        # raster layer from gpkg
        rl = QgsRasterLayer(f'GPKG:{unitTestDataPath()}/mixed_layers.gpkg:band1')
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(rl, ''))
        self.assertFalse(QgsMapLayerUtils.layerSourceMatchesPath(rl, 'aaaaa'))
        self.assertTrue(QgsMapLayerUtils.layerSourceMatchesPath(rl, unitTestDataPath() + '/mixed_layers.gpkg'))

    def test_updateLayerSourcePath(self):
        """
        Test QgsMapLayerUtils.updateLayerSourcePath()
        """
        self.assertFalse(QgsMapLayerUtils.updateLayerSourcePath(None, ''))
        self.assertFalse(QgsMapLayerUtils.updateLayerSourcePath(None, 'aaaaa'))

        # shapefile
        layer1 = QgsVectorLayer(unitTestDataPath() + '/points.shp', 'l1')
        self.assertTrue(QgsMapLayerUtils.updateLayerSourcePath(layer1, unitTestDataPath() + '/points22.shp'))
        self.assertEqual(layer1.source(), unitTestDataPath() + '/points22.shp')

        # geopackage with layers
        layer1 = QgsVectorLayer(unitTestDataPath() + '/mixed_layers.gpkg|layername=lines', 'l1')
        self.assertTrue(QgsMapLayerUtils.updateLayerSourcePath(layer1, unitTestDataPath() + '/mixed_layers22.gpkg'))
        self.assertEqual(layer1.source(), unitTestDataPath() + '/mixed_layers22.gpkg|layername=lines')
        layer2 = QgsVectorLayer(unitTestDataPath() + '/mixed_layers.gpkg|layername=points', 'l1')
        self.assertTrue(QgsMapLayerUtils.updateLayerSourcePath(layer2, unitTestDataPath() + '/mixed_layers22.gpkg'))
        self.assertEqual(layer2.source(), unitTestDataPath() + '/mixed_layers22.gpkg|layername=points')

        # raster layer from gpkg
        rl = QgsRasterLayer(f'GPKG:{unitTestDataPath()}/mixed_layers.gpkg:band1')
        self.assertTrue(QgsMapLayerUtils.updateLayerSourcePath(rl, unitTestDataPath() + '/mixed_layers22.gpkg'))
        self.assertEqual(rl.source(), f'GPKG:{unitTestDataPath()}/mixed_layers22.gpkg:band1')

        # a layer from a provider which doesn't use file based paths
        layer = QgsVectorLayer("Point?field=x:string", 'my layer', "memory")
        old_source = layer.source()
        self.assertTrue(layer.isValid())
        self.assertFalse(QgsMapLayerUtils.updateLayerSourcePath(layer, unitTestDataPath() + '/mixed_layers22.gpkg'))
        self.assertEqual(layer.source(), old_source)

    def test_sort_layers_by_type(self):
        vl1 = QgsVectorLayer("Point?field=x:string", 'vector 1', "memory")
        vl2 = QgsVectorLayer("Point?field=x:string", 'vector 2', "memory")
        options = QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext())
        al1 = QgsAnnotationLayer('annotations 1', options)
        al2 = QgsAnnotationLayer('annotations 2', options)
        rl1 = QgsRasterLayer(f'GPKG:{unitTestDataPath()}/mixed_layers.gpkg:band1', 'raster 1')
        options = QgsGroupLayer.LayerOptions(QgsProject.instance().transformContext())
        gp1 = QgsGroupLayer('group 1', options)

        self.assertEqual(QgsMapLayerUtils.sortLayersByType([vl1, rl1, gp1, vl2, al2, al1], []), [vl1, rl1, gp1, vl2, al2, al1])
        self.assertEqual(QgsMapLayerUtils.sortLayersByType([vl1, rl1, gp1, vl2, al2, al1], [QgsMapLayerType.VectorLayer]), [vl1, vl2, rl1, gp1, al2, al1])
        self.assertEqual(QgsMapLayerUtils.sortLayersByType([vl1, rl1, gp1, vl2, al2, al1], [QgsMapLayerType.RasterLayer, QgsMapLayerType.VectorLayer]),
                         [rl1, vl1, vl2, gp1, al2, al1])
        self.assertEqual(QgsMapLayerUtils.sortLayersByType([vl1, rl1, gp1, vl2, al2, al1], [QgsMapLayerType.GroupLayer, QgsMapLayerType.VectorLayer]),
                         [gp1, vl1, vl2, rl1, al2, al1])
        self.assertEqual(QgsMapLayerUtils.sortLayersByType([vl1, rl1, gp1, vl2, al2, al1], [QgsMapLayerType.GroupLayer,
                                                                                            QgsMapLayerType.VectorLayer,
                                                                                            QgsMapLayerType.AnnotationLayer]),
                         [gp1, vl1, vl2, al2, al1, rl1])
        self.assertEqual(QgsMapLayerUtils.sortLayersByType([vl1, rl1, gp1, vl2, al2, al1], [QgsMapLayerType.GroupLayer,
                                                                                            QgsMapLayerType.VectorLayer,
                                                                                            QgsMapLayerType.RasterLayer,
                                                                                            QgsMapLayerType.AnnotationLayer]),
                         [gp1, vl1, vl2, rl1, al2, al1])
        self.assertEqual(QgsMapLayerUtils.sortLayersByType([vl1, rl1, gp1, vl2], [QgsMapLayerType.GroupLayer,
                                                                                  QgsMapLayerType.VectorLayer,
                                                                                  QgsMapLayerType.RasterLayer]),
                         [gp1, vl1, vl2, rl1])
        self.assertEqual(QgsMapLayerUtils.sortLayersByType([vl1, rl1, gp1, vl2], [QgsMapLayerType.AnnotationLayer]),
                         [vl1, rl1, gp1, vl2])

    def test_launder_layer_name(self):
        self.assertEqual(QgsMapLayerUtils.launderLayerName('abc Def4_a.h%'), 'abc_def4_ah')


if __name__ == '__main__':
    unittest.main()
