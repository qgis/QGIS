# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProviderRegistry.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '16/03/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA
import os

from qgis.core import (
    QgsProviderRegistry,
    QgsMapLayerType,
    QgsWkbTypes,
    QgsProviderSublayerDetails,
    Qgis,
    QgsCoordinateTransformContext,
    QgsVectorLayer
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

# Convenience instances in case you may need them
# to find the srs.db
start_app()


class TestQgsProviderSublayerDetails(unittest.TestCase):

    def testGettersSetters(self):
        """
        Test provider list
        """
        d = QgsProviderSublayerDetails()
        d.setProviderKey('key')
        self.assertEqual(d.providerKey(), 'key')

        d.setType(QgsMapLayerType.MeshLayer)
        self.assertEqual(d.type(), QgsMapLayerType.MeshLayer)

        d.setUri('some uri')
        self.assertEqual(d.uri(), 'some uri')

        d.setName('name')
        self.assertEqual(d.name(), 'name')

        d.setDescription('desc')
        self.assertEqual(d.description(), 'desc')

        d.setPath(['a', 'b', 'c'])
        self.assertEqual(d.path(), ['a', 'b', 'c'])

        self.assertEqual(d.featureCount(), Qgis.FeatureCountState.UnknownCount)
        d.setFeatureCount(1000)
        self.assertEqual(d.featureCount(), 1000)

        self.assertEqual(d.wkbType(), QgsWkbTypes.Unknown)
        d.setWkbType(QgsWkbTypes.Point)
        self.assertEqual(d.wkbType(), QgsWkbTypes.Point)

        d.setGeometryColumnName('geom_col')
        self.assertEqual(d.geometryColumnName(), 'geom_col')

        d.setLayerNumber(13)
        self.assertEqual(d.layerNumber(), 13)

        d.setDriverName('drv')
        self.assertEqual(d.driverName(), 'drv')

        d.setSkippedContainerScan(True)
        self.assertTrue(d.skippedContainerScan())
        d.setSkippedContainerScan(False)
        self.assertFalse(d.skippedContainerScan())

        d.setFlags(Qgis.SublayerFlag.SystemTable)
        self.assertEqual(d.flags(), Qgis.SublayerFlags(Qgis.SublayerFlag.SystemTable))
        d.setFlags(Qgis.SublayerFlags())
        self.assertEqual(d.flags(), Qgis.SublayerFlags())

    def test_equality(self):
        """
        Test equality operator
        """
        d = QgsProviderSublayerDetails()
        d2 = QgsProviderSublayerDetails()
        d.setProviderKey('key')
        self.assertNotEqual(d, d2)
        d2.setProviderKey('key')
        self.assertEqual(d, d2)

        d.setType(QgsMapLayerType.MeshLayer)
        self.assertNotEqual(d, d2)
        d2.setType(QgsMapLayerType.MeshLayer)
        self.assertEqual(d, d2)

        d.setUri('some uri')
        self.assertNotEqual(d, d2)
        d2.setUri('some uri')
        self.assertEqual(d, d2)

        d.setName('name')
        self.assertNotEqual(d, d2)
        d2.setName('name')
        self.assertEqual(d, d2)

        d.setDescription('desc')
        self.assertNotEqual(d, d2)
        d2.setDescription('desc')
        self.assertEqual(d, d2)

        d.setPath(['a', 'b', 'c'])
        self.assertNotEqual(d, d2)
        d2.setPath(['a', 'b', 'c'])
        self.assertEqual(d, d2)

        d.setFeatureCount(1000)
        self.assertNotEqual(d, d2)
        d2.setFeatureCount(1000)
        self.assertEqual(d, d2)

        d.setWkbType(QgsWkbTypes.Point)
        self.assertNotEqual(d, d2)
        d2.setWkbType(QgsWkbTypes.Point)
        self.assertEqual(d, d2)

        d.setGeometryColumnName('geom_col')
        self.assertNotEqual(d, d2)
        d2.setGeometryColumnName('geom_col')
        self.assertEqual(d, d2)

        d.setLayerNumber(13)
        self.assertNotEqual(d, d2)
        d2.setLayerNumber(13)
        self.assertEqual(d, d2)

        d.setDriverName('drv')
        self.assertNotEqual(d, d2)
        d2.setDriverName('drv')
        self.assertEqual(d, d2)

        d.setSkippedContainerScan(True)
        self.assertNotEqual(d, d2)
        d2.setSkippedContainerScan(True)
        self.assertEqual(d, d2)

        d.setFlags(Qgis.SublayerFlag.SystemTable)
        self.assertNotEqual(d, d2)
        d2.setFlags(Qgis.SublayerFlag.SystemTable)
        self.assertEqual(d, d2)

    def test_to_layer(self):
        """
        Test converting sub layer details to a layer
        """
        details = QgsProviderSublayerDetails()
        details.setUri(os.path.join(unitTestDataPath(), 'lines.shp'))
        details.setName('my sub layer')
        details.setType(QgsMapLayerType.VectorLayer)
        details.setProviderKey('ogr')

        options = QgsProviderSublayerDetails.LayerOptions(QgsCoordinateTransformContext())
        ml = details.toLayer(options)
        self.assertTrue(ml.isValid())
        self.assertIsInstance(ml, QgsVectorLayer)
        self.assertEqual(ml.name(), 'my sub layer')

    def test_to_mime(self):
        """
        Test converting sub layer details to mime URIs
        """
        details = QgsProviderSublayerDetails()
        details.setUri(os.path.join(unitTestDataPath(), 'lines.shp'))
        details.setName('my sub layer')
        details.setType(QgsMapLayerType.VectorLayer)
        details.setProviderKey('ogr')

        uri = details.toMimeUri()
        self.assertEqual(uri.layerType, 'vector')
        self.assertEqual(uri.providerKey, 'ogr')
        self.assertEqual(uri.name, 'my sub layer')
        self.assertEqual(uri.uri, os.path.join(unitTestDataPath(), 'lines.shp'))

        details.setType(QgsMapLayerType.RasterLayer)
        uri = details.toMimeUri()
        self.assertEqual(uri.layerType, 'raster')

        details.setType(QgsMapLayerType.MeshLayer)
        uri = details.toMimeUri()
        self.assertEqual(uri.layerType, 'mesh')

        details.setType(QgsMapLayerType.VectorTileLayer)
        uri = details.toMimeUri()
        self.assertEqual(uri.layerType, 'vector-tile')

        details.setType(QgsMapLayerType.PointCloudLayer)
        uri = details.toMimeUri()
        self.assertEqual(uri.layerType, 'pointcloud')

        details.setType(QgsMapLayerType.PluginLayer)
        uri = details.toMimeUri()
        self.assertEqual(uri.layerType, 'plugin')

        details.setType(QgsMapLayerType.GroupLayer)
        uri = details.toMimeUri()
        self.assertEqual(uri.layerType, 'group')

        details.setType(QgsMapLayerType.AnnotationLayer)
        uri = details.toMimeUri()
        self.assertEqual(uri.layerType, 'annotation')


if __name__ == '__main__':
    unittest.main()
