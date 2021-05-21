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
    QgsRectangle
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


if __name__ == '__main__':
    unittest.main()
