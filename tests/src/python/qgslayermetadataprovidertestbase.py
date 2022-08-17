# coding=utf-8
""""Base test for layer metadata providers for vector layers

.. note:: This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

"""

__author__ = 'elpaso@itopen.it'
__date__ = '2022-08-19'
__copyright__ = 'Copyright 2022, ItOpen'

import os

from qgis.core import (
    QgsVectorLayer,
    QgsRasterLayer,
    QgsMapLayerType,
    QgsProviderRegistry,
    QgsWkbTypes,
    QgsLayerMetadata,
    QgsProviderMetadata,
    QgsBox3d,
    QgsRectangle,
)

from qgis.PyQt.QtCore import QCoreApplication
from utilities import compareWkt, unitTestDataPath
from qgis.testing import start_app

QGIS_APP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class LayerMetadataProviderTestBase():
    """Base test for layer metadata providers

    Provider tests must implement:
    - getLayer() -> return a QgsVectorLayer or a QgsRasterLayer
    - getProviderName() -> str returns the name of the data provider to be tested ('ogr', 'postgres' ...)
    """

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)

    def testMetadataWriteRead(self):

        self.test_layer = self.getLayer()
        self.assertTrue(self.test_layer.isValid())
        extent_as_wkt = self.test_layer.extent().asWktPolygon()
        layer_type = self.test_layer.type()
        layer_authid = self.test_layer.crs().authid()
        data_provider_name = self.test_layer.dataProvider().name()

        m = self.test_layer.metadata()
        m.setAbstract('QGIS Some Data')
        m.setIdentifier('MD012345')
        m.setTitle('QGIS Test Title')
        ext = QgsLayerMetadata.Extent()
        spatial_ext = QgsLayerMetadata.SpatialExtent()
        spatial_ext.bounds = QgsBox3d(self.test_layer.extent())
        spatial_ext.crs = self.test_layer.crs()
        ext.setSpatialExtents([spatial_ext])
        m.setExtent(ext)
        self.test_layer.setMetadata(m)

        md = QgsProviderRegistry.instance().providerMetadata(data_provider_name)
        self.assertIsNotNone(md)
        self.assertTrue(bool(md.providerCapabilities() & QgsProviderMetadata.ProviderCapability.SaveLayerMetadata))

        layer_uri = self.test_layer.publicSource()
        self.assertTrue(md.saveLayerMetadata(layer_uri, m)[0])

        self.test_layer = self.getLayer()
        m = self.test_layer.metadata()
        self.assertEqual(m.title(), 'QGIS Test Title')
        self.assertEqual(m.identifier(), 'MD012345')
        self.assertEqual(m.abstract(), 'QGIS Some Data')
        self.assertEqual(m.crs().authid(), layer_authid)

        del self.test_layer

        reg = QGIS_APP.layerMetadataProviderRegistry()
        md_provider = reg.layerMetadataProviderFromType(self.getProviderName())
        results = md_provider.search('QGIS Some Data')
        self.assertEqual(len(results.metadata()), 1)

        result = results.metadata()[0]

        self.assertEqual(result.abstract(), 'QGIS Some Data')
        self.assertEqual(result.identifier(), 'MD012345')
        self.assertEqual(result.title(), 'QGIS Test Title')
        self.assertEqual(result.layerType, layer_type)
        self.assertEqual(result.crs, layer_authid)
        # For raster is unknown
        if layer_type != QgsMapLayerType.VectorLayer:
            self.assertEqual(result.geometryType, QgsWkbTypes.UnknownGeometry)
        else:
            self.assertEqual(result.geometryType, QgsWkbTypes.PointGeometry)
        self.assertEqual(result.dataProviderName, data_provider_name)
        self.assertEqual(result.standardUri, 'http://mrcc.com/qgis.dtd')
        self.assertTrue(compareWkt(result.geographicExtent.asWkt(), extent_as_wkt))

        # Check layer load
        if layer_type == QgsMapLayerType.VectorLayer:
            test_layer = QgsVectorLayer(result.uri, 'PG MD Layer', result.dataProviderName)
        else:
            test_layer = QgsRasterLayer(result.uri, 'PG MD Layer', result.dataProviderName)

        self.assertTrue(test_layer.isValid())

        # Test search filters
        results = md_provider.search('', QgsRectangle(0, 0, 1, 1))
        self.assertEqual(len(results.metadata()), 0)
        results = md_provider.search('', test_layer.extent())
        self.assertEqual(len(results.metadata()), 1)
        results = md_provider.search('NOT HERE!', test_layer.extent())
        self.assertEqual(len(results.metadata()), 0)
        results = md_provider.search('QGIS', test_layer.extent())
        self.assertEqual(len(results.metadata()), 1)
