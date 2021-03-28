# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapLayerFactory.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '10/03/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsMapLayerFactory, QgsMapLayerType
from qgis.testing import start_app, unittest

start_app()


class TestQgsMapLayerFactory(unittest.TestCase):

    def testTypeFromString(self):
        """
        Test QgsMapLayerFactory.typeFromString
        """
        self.assertEqual(QgsMapLayerFactory.typeFromString('xxx')[1], False)
        self.assertEqual(QgsMapLayerFactory.typeFromString('')[1], False)
        self.assertEqual(QgsMapLayerFactory.typeFromString('vector'), (QgsMapLayerType.VectorLayer, True))
        self.assertEqual(QgsMapLayerFactory.typeFromString('VECTOR'), (QgsMapLayerType.VectorLayer, True))
        self.assertEqual(QgsMapLayerFactory.typeFromString('raster'), (QgsMapLayerType.RasterLayer, True))
        self.assertEqual(QgsMapLayerFactory.typeFromString('mesh'), (QgsMapLayerType.MeshLayer, True))
        self.assertEqual(QgsMapLayerFactory.typeFromString('vector-tile'), (QgsMapLayerType.VectorTileLayer, True))
        self.assertEqual(QgsMapLayerFactory.typeFromString('point-cloud'), (QgsMapLayerType.PointCloudLayer, True))
        self.assertEqual(QgsMapLayerFactory.typeFromString('plugin'), (QgsMapLayerType.PluginLayer, True))
        self.assertEqual(QgsMapLayerFactory.typeFromString('annotation'), (QgsMapLayerType.AnnotationLayer, True))

    def testTypeToString(self):
        """
        Test QgsMapLayerFactory.typeToString
        """
        # test via round trips...
        self.assertEqual(
            QgsMapLayerFactory.typeFromString(QgsMapLayerFactory.typeToString(QgsMapLayerType.VectorLayer))[0],
            QgsMapLayerType.VectorLayer)
        self.assertEqual(
            QgsMapLayerFactory.typeFromString(QgsMapLayerFactory.typeToString(QgsMapLayerType.RasterLayer))[0],
            QgsMapLayerType.RasterLayer)
        self.assertEqual(
            QgsMapLayerFactory.typeFromString(QgsMapLayerFactory.typeToString(QgsMapLayerType.MeshLayer))[0],
            QgsMapLayerType.MeshLayer)
        self.assertEqual(
            QgsMapLayerFactory.typeFromString(QgsMapLayerFactory.typeToString(QgsMapLayerType.VectorTileLayer))[0],
            QgsMapLayerType.VectorTileLayer)
        self.assertEqual(
            QgsMapLayerFactory.typeFromString(QgsMapLayerFactory.typeToString(QgsMapLayerType.PointCloudLayer))[0],
            QgsMapLayerType.PointCloudLayer)
        self.assertEqual(
            QgsMapLayerFactory.typeFromString(QgsMapLayerFactory.typeToString(QgsMapLayerType.PluginLayer))[0],
            QgsMapLayerType.PluginLayer)
        self.assertEqual(
            QgsMapLayerFactory.typeFromString(QgsMapLayerFactory.typeToString(QgsMapLayerType.AnnotationLayer))[0],
            QgsMapLayerType.AnnotationLayer)


if __name__ == '__main__':
    unittest.main()
