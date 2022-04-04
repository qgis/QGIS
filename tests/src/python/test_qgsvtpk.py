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

    def testOpenInvalid(self):
        """
        Test opening an invalid path
        """
        tiles = QgsVtpkTiles(unitTestDataPath() + '/xxx.vtpk')
        self.assertEqual(tiles.metadata(), {})
        self.assertEqual(tiles.styleDefinition(), {})
        self.assertEqual(tiles.spriteDefinition(), {})
        self.assertTrue(tiles.spriteImage().isNull())
        self.assertFalse(tiles.open())
        self.assertEqual(tiles.metadata(), {})
        self.assertEqual(tiles.styleDefinition(), {})
        self.assertEqual(tiles.spriteDefinition(), {})
        self.assertTrue(tiles.spriteImage().isNull())

    def testOpen(self):
        tiles = QgsVtpkTiles(unitTestDataPath() + '/testvtpk.vtpk')
        self.assertEqual(tiles.metadata(), {})
        self.assertEqual(tiles.styleDefinition(), {})
        self.assertEqual(tiles.spriteDefinition(), {})
        self.assertTrue(tiles.spriteImage().isNull())

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

        self.assertEqual(tiles.styleDefinition(), {'glyphs': '../fonts/{fontstack}/{range}.pbf', 'layers': [
            {'id': 'polys', 'layout': {}, 'paint': {'fill-color': '#B6FCFB', 'fill-outline-color': '#6E6E6E'},
             'source': 'esri', 'source-layer': 'polys', 'type': 'fill'},
            {'id': 'lines', 'layout': {'line-cap': 'round', 'line-join': 'round'},
             'paint': {'line-color': '#4C993A', 'line-width': 1.33333}, 'source': 'esri', 'source-layer': 'lines',
             'type': 'line'}, {'id': 'points', 'layout': {}, 'paint': {'circle-color': '#A16F33', 'circle-radius': 2.2,
                                                                       'circle-stroke-color': '#000000',
                                                                       'circle-stroke-width': 0.933333},
                               'source': 'esri', 'source-layer': 'points', 'type': 'circle'}], 'sources': {
            'esri': {'attribution': 'Map credits', 'bounds': [-180, -85.0511, 180, 85.0511], 'maxzoom': 4, 'minzoom': 0,
                     'scheme': 'xyz', 'type': 'vector', 'url': '../../'}}, 'sprite': '../sprites/sprite', 'version': 8})
        # this file doesn't actually have any sprites...
        self.assertEqual(tiles.spriteDefinition(), {})
        self.assertEqual(tiles.spriteImage().width(), 120)
        self.assertEqual(tiles.spriteImage().height(), 120)

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

    def testLayerMetadata(self):
        tiles = QgsVtpkTiles(unitTestDataPath() + '/testvtpk.vtpk')
        # no crash
        _ = tiles.layerMetadata()

        self.assertTrue(tiles.open())
        layer_metadata = tiles.layerMetadata()
        self.assertEqual(layer_metadata.language(), 'en-AU')
        self.assertEqual(layer_metadata.identifier(), 'FD610B57-9B73-48E5-A7E5-DA07C8D2C245')
        self.assertEqual(layer_metadata.title(), 'testvtpk')
        self.assertEqual(layer_metadata.abstract(), 'Map description')
        self.assertEqual(layer_metadata.keywords(), {'keywords': ['tile tags']})
        self.assertEqual(layer_metadata.rights(), ['Map credits'])
        self.assertEqual(layer_metadata.licenses(), ['Map use limitations'])
        self.assertEqual(layer_metadata.crs().authid(), 'EPSG:3857')
        self.assertEqual(layer_metadata.extent().spatialExtents()[0].bounds.xMinimum(), -179.999988600592)
        self.assertEqual(layer_metadata.extent().spatialExtents()[0].bounds.xMaximum(), 179.999988540844)
        self.assertEqual(layer_metadata.extent().spatialExtents()[0].bounds.yMinimum(), -85.0511277912625)
        self.assertEqual(layer_metadata.extent().spatialExtents()[0].bounds.yMaximum(), 85.0511277912625)
        self.assertEqual(layer_metadata.extent().spatialExtents()[0].extentCrs.authid(), 'EPSG:4326')

    def testVectorTileLayer(self):
        layer = QgsVectorTileLayer('type=vtpk&url={}'.format(unitTestDataPath() + '/testvtpk.vtpk'), 'tiles')
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.sourceType(), 'vtpk')
        self.assertEqual(layer.sourcePath(), unitTestDataPath() + '/testvtpk.vtpk')
        self.assertEqual(layer.sourceMinZoom(), 0)
        self.assertEqual(layer.sourceMaxZoom(), 4)

        self.assertTrue(layer.loadDefaultStyle())

        # make sure style was loaded
        self.assertEqual(len(layer.renderer().styles()), 3)
        self.assertEqual(layer.renderer().style(0).styleName(), 'polys')
        style = layer.renderer().style(0)
        self.assertEqual(style.symbol().color().name(), '#b6fcfb')
        self.assertEqual(layer.renderer().style(1).styleName(), 'lines')
        style = layer.renderer().style(1)
        self.assertEqual(style.symbol().color().name(), '#4c993a')
        self.assertEqual(layer.renderer().style(2).styleName(), 'points')
        style = layer.renderer().style(2)
        self.assertEqual(style.symbol().color().name(), '#a16f33')

        self.assertTrue(layer.loadDefaultMetadata())
        # make sure metadata was loaded
        self.assertEqual(layer.metadata().identifier(), 'FD610B57-9B73-48E5-A7E5-DA07C8D2C245')


if __name__ == '__main__':
    unittest.main()
