# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsTileMatrix

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '04/03/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA
from qgis.core import (
    QgsVtpkTiles,
    QgsVectorTileLayer,
    QgsCoordinateTransformContext
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()


class TestQgsVtpk(unittest.TestCase):

    def testOpen(self):
        tiles = QgsVtpkTiles(unitTestDataPath() + '/testvtpk.vtpk')
        self.assertTrue(tiles.open())
        self.assertEqual(tiles.metadata(),
                         {'capabilities': 'TilesOnly', 'copyrightText': 'Map credits', 'currentVersion': 10.91,
                          'defaultStyles': 'resources/styles', 'exportTilesAllowed': False,
                          'fullExtent': {'spatialReference': {'latestWkid': 3857, 'wkid': 102100},
                                         'xmax': 20037507.0671618, 'xmin': -20037507.0738129, 'ymax': 20037507.0671618,
                                         'ymin': -20037507.0671618},
                          'initialExtent': {'spatialReference': {'latestWkid': 3857, 'wkid': 102100},
                                            'xmax': 20037507.0671618, 'xmin': -20037507.0738129,
                                            'ymax': 20037507.0671618, 'ymin': -20037507.0671618}, 'maxLOD': 4,
                          'maxScale': 18489297.737236, 'maxzoom': 4, 'minLOD': 0, 'minScale': 295828763.7957775,
                          'name': 'Map', 'resourceInfo': {
                              'cacheInfo': {'storageInfo': {'packetSize': 128, 'storageFormat': 'compactV2'}},
                              'styleVersion': 8, 'tileCompression': 'gzip'},
                          'tileInfo': {'cols': 512, 'dpi': 96, 'format': 'pbf',
                                       'lods': [{'level': 0, 'resolution': 78271.516964, 'scale': 295828763.7957775},
                                                {'level': 1, 'resolution': 39135.75848199995,
                                                 'scale': 147914381.8978885},
                                                {'level': 2, 'resolution': 19567.87924100005,
                                                 'scale': 73957190.9489445},
                                                {'level': 3, 'resolution': 9783.93962049995, 'scale': 36978595.474472},
                                                {'level': 4, 'resolution': 4891.96981024998, 'scale': 18489297.737236}],
                                       'origin': {'x': -20037508.342787, 'y': 20037508.342787}, 'rows': 512,
                                       'spatialReference': {'latestWkid': 3857, 'wkid': 102100}},
                          'tiles': ['tile/{z}/{y}/{x}.pbf'], 'type': 'indexedVector'}
                         )

        self.assertEqual(tiles.matrixSet().minimumZoom(), 0)
        self.assertEqual(tiles.matrixSet().maximumZoom(), 4)

        self.assertEqual(tiles.matrixSet().tileMatrix(0).scale(), 295828763.7957775)
        self.assertEqual(tiles.matrixSet().tileMatrix(1).scale(), 147914381.8978885)
        self.assertEqual(tiles.matrixSet().tileMatrix(2).scale(), 73957190.9489445)
        self.assertEqual(tiles.matrixSet().tileMatrix(3).scale(), 36978595.474472)
        self.assertEqual(tiles.matrixSet().tileMatrix(4).scale(), 18489297.737236)

        self.assertEqual(tiles.crs().authid(), 'EPSG:3857')

        context = QgsCoordinateTransformContext()
        self.assertEqual(tiles.extent(context).xMinimum(), -20037507.0738129)
        self.assertEqual(tiles.extent(context).yMinimum(), -20037507.0671618)
        self.assertEqual(tiles.extent(context).xMaximum(), 20037507.0671618)
        self.assertEqual(tiles.extent(context).yMaximum(), 20037507.0671618)

        tile_data = tiles.tileData(0, 0, 0)
        self.assertEqual(tile_data.length(), 1202)

    def testVectorTileLayer(self):
        layer = QgsVectorTileLayer('type=vtpk&url={}'.format(unitTestDataPath() + '/testvtpk.vtpk'), 'tiles')
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.sourceType(), 'vtpk')
        self.assertEqual(layer.sourcePath(), unitTestDataPath() + '/testvtpk.vtpk')
        self.assertEqual(layer.sourceMinZoom(), 0)
        self.assertEqual(layer.sourceMaxZoom(), 4)


if __name__ == '__main__':
    unittest.main()
