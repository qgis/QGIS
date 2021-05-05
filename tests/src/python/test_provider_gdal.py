# -*- coding: utf-8 -*-
"""Generic Unit tests for the GDAL provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2018-30-10'
__copyright__ = 'Copyright 2018, Nyall Dawson'

import os

from qgis.core import (
    QgsProviderRegistry,
    QgsDataProvider,
    QgsRasterLayer,
    QgsRectangle,
)
from qgis.testing import start_app, unittest

from qgis.PyQt.QtGui import qRed

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class PyQgsGdalProvider(unittest.TestCase):

    def checkBlockContents(self, block, expected):
        res = []
        for r in range(block.height()):
            res.extend([block.value(r, c) for c in range(block.width())])
        self.assertEqual(res, expected)

    def testCapabilities(self):
        self.assertTrue(QgsProviderRegistry.instance().providerCapabilities("gdal") & QgsDataProvider.File)
        self.assertTrue(QgsProviderRegistry.instance().providerCapabilities("gdal") & QgsDataProvider.Dir)
        self.assertTrue(QgsProviderRegistry.instance().providerCapabilities("gdal") & QgsDataProvider.Net)

    def testRasterBlock(self):
        """Test raster block with extent"""

        path = os.path.join(unitTestDataPath(), 'landsat_4326.tif')
        raster_layer = QgsRasterLayer(path, 'test')
        self.assertTrue(raster_layer.isValid())

        extent = QgsRectangle(17.94284482577178252, 30.23021770271909503, 17.94407867909909626, 30.23154272264058307)
        block = raster_layer.dataProvider().block(1, extent, 2, 3)
        self.checkBlockContents(block, [
            125.0, 125.0,
            125.0, 125.0,
            125.0, 124.0,
        ])

        full_content = [
            125.0, 125.0, 125.0,
            125.0, 125.0, 125.0,
            125.0, 124.0, 125.0,
            126.0, 127.0, 127.0,
        ]

        extent = raster_layer.extent()
        block = raster_layer.dataProvider().block(1, extent, 3, 4)
        self.checkBlockContents(block, full_content)

        extent = raster_layer.extent()
        extent.grow(-0.0001)
        block = raster_layer.dataProvider().block(1, extent, 3, 4)
        self.checkBlockContents(block, full_content)

        row_height = raster_layer.extent().height() / raster_layer.height()

        for row in range(raster_layer.height()):
            extent = raster_layer.extent()
            extent.setYMaximum(extent.yMaximum() - row_height * row)
            extent.setYMinimum(extent.yMaximum() - row_height)
            block = raster_layer.dataProvider().block(1, extent, 3, 1)
            self.checkBlockContents(block, full_content[row * 3:row * 3 + 3])

    def testDecodeEncodeUriGpkg(self):
        """Test decodeUri/encodeUri geopackage support"""

        uri = '/my/raster.gpkg'
        parts = QgsProviderRegistry.instance().decodeUri('gdal', uri)
        self.assertEqual(parts, {'path': '/my/raster.gpkg', 'layerName': None})
        encodedUri = QgsProviderRegistry.instance().encodeUri('gdal', parts)
        self.assertEqual(encodedUri, uri)

        uri = 'GPKG:/my/raster.gpkg'
        parts = QgsProviderRegistry.instance().decodeUri('gdal', uri)
        self.assertEqual(parts, {'path': '/my/raster.gpkg', 'layerName': None})
        encodedUri = QgsProviderRegistry.instance().encodeUri('gdal', parts)
        self.assertEqual(encodedUri, '/my/raster.gpkg')

        uri = 'GPKG:/my/raster.gpkg:mylayer'
        parts = QgsProviderRegistry.instance().decodeUri('gdal', uri)
        self.assertEqual(parts, {'path': '/my/raster.gpkg', 'layerName': 'mylayer'})
        encodedUri = QgsProviderRegistry.instance().encodeUri('gdal', parts)
        self.assertEqual(encodedUri, uri)

    def testDecodeEncodeUriOptions(self):
        """Test decodeUri/encodeUri options support"""

        uri = '/my/raster.pdf|option:DPI=300|option:GIVEME=TWO'
        parts = QgsProviderRegistry.instance().decodeUri('gdal', uri)
        self.assertEqual(parts, {'path': '/my/raster.pdf', 'layerName': None, 'openOptions': ['DPI=300', 'GIVEME=TWO']})
        encodedUri = QgsProviderRegistry.instance().encodeUri('gdal', parts)
        self.assertEqual(encodedUri, uri)

    def testDecodeEncodeUriVsizip(self):
        """Test decodeUri/encodeUri for /vsizip/ prefixed URIs"""

        uri = '/vsizip//my/file.zip/image.tif'
        parts = QgsProviderRegistry.instance().decodeUri('gdal', uri)
        self.assertEqual(parts, {'path': '/my/file.zip', 'layerName': None, 'vsiPrefix': '/vsizip/', 'vsiSuffix': '/image.tif'})
        encodedUri = QgsProviderRegistry.instance().encodeUri('gdal', parts)
        self.assertEqual(encodedUri, uri)


if __name__ == '__main__':
    unittest.main()
