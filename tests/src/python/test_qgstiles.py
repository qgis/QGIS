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
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsTileXYZ,
    QgsTileRange,
    QgsTileMatrix,
    QgsCoordinateReferenceSystem,
    QgsPointXY,
    QgsTileMatrixSet,
    QgsVectorTileMatrixSet,
    QgsReadWriteContext,
    Qgis
)
from qgis.testing import start_app, unittest

start_app()


class TestQgsTiles(unittest.TestCase):

    def testQgsTileXYZ(self):
        tile = QgsTileXYZ(1, 2, 3)
        self.assertEqual(tile.column(), 1)
        self.assertEqual(tile.row(), 2)
        self.assertEqual(tile.zoomLevel(), 3)

    def testQgsTileRange(self):
        range = QgsTileRange(1, 2, 3, 4)
        self.assertEqual(range.startColumn(), 1)
        self.assertEqual(range.endColumn(), 2)
        self.assertEqual(range.startRow(), 3)
        self.assertEqual(range.endRow(), 4)
        self.assertTrue(range.isValid())

        # invalid range
        range = QgsTileRange()
        self.assertFalse(range.isValid())

    def testQgsTileMatrix(self):
        matrix = QgsTileMatrix.fromCustomDef(5, QgsCoordinateReferenceSystem('EPSG:4326'), QgsPointXY(1, 2), 1000, 4, 8)
        self.assertEqual(matrix.zoomLevel(), 5)
        self.assertEqual(matrix.crs().authid(), 'EPSG:4326')

        matrix.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        self.assertEqual(matrix.crs().authid(), 'EPSG:3857')

        matrix.setZoomLevel(6)
        self.assertEqual(matrix.zoomLevel(), 6)

        matrix.setScale(10000)
        self.assertEqual(matrix.scale(), 10000)

    def testQgsTileMatrixSet(self):
        matrix_set = QgsTileMatrixSet()

        # should be applied by default in order to match MapBox rendering of tiles
        self.assertEqual(matrix_set.scaleToTileZoomMethod(), Qgis.ScaleToTileZoomLevelMethod.MapBox)

        self.assertEqual(matrix_set.minimumZoom(), -1)
        self.assertEqual(matrix_set.maximumZoom(), -1)
        self.assertFalse(matrix_set.crs().isValid())

        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(1, QgsCoordinateReferenceSystem('EPSG:4326'), QgsPointXY(1, 2), 1000, 4, 8))
        self.assertEqual(matrix_set.minimumZoom(), 1)
        self.assertEqual(matrix_set.maximumZoom(), 1)
        self.assertEqual(matrix_set.crs().authid(), 'EPSG:4326')

        self.assertEqual(matrix_set.tileMatrix(1).zoomLevel(), 1)
        # zoom level not present in matrix!
        self.assertEqual(matrix_set.tileMatrix(99).zoomLevel(), -1)

        self.assertAlmostEqual(matrix_set.tileMatrix(1).scale(), 776503144, -1)
        self.assertEqual(matrix_set.scaleToZoom(776503144), 1)
        self.assertEqual(matrix_set.scaleToZoom(1776503144), 1)
        self.assertEqual(matrix_set.scaleToZoom(76503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(776503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(1776503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(76503144), 1)

        # add a second level
        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(2, QgsCoordinateReferenceSystem('EPSG:4326'), QgsPointXY(1, 2), 1000, 4, 8))
        self.assertEqual(matrix_set.minimumZoom(), 1)
        self.assertEqual(matrix_set.maximumZoom(), 2)
        self.assertEqual(matrix_set.crs().authid(), 'EPSG:4326')

        self.assertEqual(matrix_set.tileMatrix(1).zoomLevel(), 1)
        self.assertEqual(matrix_set.tileMatrix(2).zoomLevel(), 2)
        self.assertEqual(matrix_set.tileMatrix(99).zoomLevel(), -1)

        self.assertAlmostEqual(matrix_set.scaleToZoom(776503144), 1, 5)
        self.assertEqual(matrix_set.scaleToZoom(1776503144), 1)
        self.assertAlmostEqual(matrix_set.scaleToZoom(76503144), 2, 5)
        self.assertEqual(matrix_set.scaleToZoom(6503144), 2)
        self.assertEqual(matrix_set.scaleToZoomLevel(776503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(1776503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(76503144), 2)
        self.assertEqual(matrix_set.scaleToZoomLevel(6503144), 2)

        # add a third level
        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(3, QgsCoordinateReferenceSystem('EPSG:4326'), QgsPointXY(1, 2), 1000, 4, 8))
        self.assertEqual(matrix_set.minimumZoom(), 1)
        self.assertEqual(matrix_set.maximumZoom(), 3)
        self.assertEqual(matrix_set.crs().authid(), 'EPSG:4326')

        self.assertEqual(matrix_set.tileMatrix(1).zoomLevel(), 1)
        self.assertEqual(matrix_set.tileMatrix(2).zoomLevel(), 2)
        self.assertEqual(matrix_set.tileMatrix(3).zoomLevel(), 3)
        self.assertEqual(matrix_set.tileMatrix(99).zoomLevel(), -1)

        self.assertAlmostEqual(matrix_set.scaleToZoom(776503144), 1, 5)
        self.assertEqual(matrix_set.scaleToZoom(1776503144), 1)
        self.assertAlmostEqual(matrix_set.scaleToZoom(388251572), 1, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(288251572), 1.515, 2)
        self.assertAlmostEqual(matrix_set.scaleToZoom(194125786), 2, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(188251572), 2.060519, 3)
        self.assertEqual(matrix_set.scaleToZoom(6503144), 3)
        self.assertEqual(matrix_set.scaleToZoomLevel(776503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(1776503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(76503144), 3)
        self.assertEqual(matrix_set.scaleToZoomLevel(388251572), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(298251572), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(198251572), 2)
        self.assertEqual(matrix_set.scaleToZoomLevel(6503144), 3)

        # with ESRI scale to zoom handling
        matrix_set.setScaleToTileZoomMethod(Qgis.ScaleToTileZoomLevelMethod.Esri)
        self.assertEqual(matrix_set.scaleToTileZoomMethod(), Qgis.ScaleToTileZoomLevelMethod.Esri)

        self.assertAlmostEqual(matrix_set.scaleToZoom(776503144), 1, 5)
        self.assertEqual(matrix_set.scaleToZoom(1776503144), 1)
        self.assertAlmostEqual(matrix_set.scaleToZoom(388251572), 2, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(288251572), 2.515, 2)
        self.assertAlmostEqual(matrix_set.scaleToZoom(194125786), 3, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(188251572), 3.0, 3)
        self.assertEqual(matrix_set.scaleToZoom(6503144), 3)
        self.assertEqual(matrix_set.scaleToZoomLevel(776503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(1776503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(76503144), 3)
        self.assertEqual(matrix_set.scaleToZoomLevel(388251572), 2)
        self.assertEqual(matrix_set.scaleToZoomLevel(298251572), 2)
        self.assertEqual(matrix_set.scaleToZoomLevel(198251572), 2)
        self.assertEqual(matrix_set.scaleToZoomLevel(6503144), 3)

    def testTileMatrixSetGoogle(self):
        matrix_set = QgsTileMatrixSet()
        matrix_set.addGoogleCrs84QuadTiles(1, 13)

        self.assertEqual(matrix_set.minimumZoom(), 1)
        self.assertEqual(matrix_set.maximumZoom(), 13)

        self.assertAlmostEqual(matrix_set.tileMatrix(1).scale(), 279541132, 0)
        self.assertAlmostEqual(matrix_set.tileMatrix(2).scale(), 139770566, 0)
        self.assertAlmostEqual(matrix_set.tileMatrix(3).scale(), 69885283, 0)
        self.assertAlmostEqual(matrix_set.tileMatrix(4).scale(), 34942642, 0)
        self.assertAlmostEqual(matrix_set.tileMatrix(5).scale(), 17471321, 0)

    def testTileMatrixSetRemoveTiles(self):
        matrix_set = QgsTileMatrixSet()
        matrix_set.addGoogleCrs84QuadTiles(1, 13)

        matrix_set.dropMatricesOutsideZoomRange(4, 10)
        self.assertEqual(matrix_set.minimumZoom(), 4)
        self.assertEqual(matrix_set.maximumZoom(), 10)

    def testReadWriteXml(self):
        matrix_set = QgsTileMatrixSet()
        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(1, QgsCoordinateReferenceSystem('EPSG:4326'), QgsPointXY(1, 2), 1000, 4, 8))
        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(2, QgsCoordinateReferenceSystem('EPSG:4326'), QgsPointXY(1, 2), 1000, 4, 8))
        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(3, QgsCoordinateReferenceSystem('EPSG:3857'), QgsPointXY(1, 2), 1000, 4, 8))

        doc = QDomDocument("testdoc")
        res = matrix_set.writeXml(doc, QgsReadWriteContext())

        set2 = QgsTileMatrixSet()
        self.assertTrue(set2.readXml(res, QgsReadWriteContext()))
        self.assertEqual(set2.minimumZoom(), 1)
        self.assertEqual(set2.maximumZoom(), 3)

        self.assertEqual(set2.tileMatrix(1).crs().authid(), 'EPSG:4326')
        self.assertEqual(set2.tileMatrix(2).crs().authid(), 'EPSG:4326')
        self.assertEqual(set2.tileMatrix(3).crs().authid(), 'EPSG:3857')

    def testVectorTileMatrixSet(self):
        matrix_set = QgsVectorTileMatrixSet()

        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(1, QgsCoordinateReferenceSystem('EPSG:4326'), QgsPointXY(1, 2), 1000, 4, 8))
        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(2, QgsCoordinateReferenceSystem('EPSG:4326'), QgsPointXY(1, 2), 1000, 4, 8))
        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(3, QgsCoordinateReferenceSystem('EPSG:3857'), QgsPointXY(1, 2), 1000, 4, 8))

        doc = QDomDocument("testdoc")
        res = matrix_set.writeXml(doc, QgsReadWriteContext())

        set2 = QgsVectorTileMatrixSet()
        self.assertTrue(set2.readXml(res, QgsReadWriteContext()))
        self.assertEqual(set2.minimumZoom(), 1)
        self.assertEqual(set2.maximumZoom(), 3)

        self.assertEqual(set2.tileMatrix(1).crs().authid(), 'EPSG:4326')
        self.assertEqual(set2.tileMatrix(2).crs().authid(), 'EPSG:4326')
        self.assertEqual(set2.tileMatrix(3).crs().authid(), 'EPSG:3857')

    def testVectorTileMatrixSetFromESRI(self):
        esri_metadata = {
            "tiles": [
                "tile/{z}/{y}/{x}.pbf"
            ],
            "initialExtent": {
                "xmin": -2750565.3405000009,
                "ymin": -936638.5,
                "xmax": 3583872.5,
                "ymax": 4659267,
                "spatialReference": {
                    "wkid": 3978,
                    "latestWkid": 3978
                }
            },
            "fullExtent": {
                "xmin": -2750565.3405000009,
                "ymin": -936638.5,
                "xmax": 3583872.5,
                "ymax": 4659267,
                "spatialReference": {
                    "wkid": 3978,
                    "latestWkid": 3978
                }
            },
            "minScale": 511647836.79182798,
            "maxScale": 31228.505663563719,
            "tileInfo": {
                "rows": 512,
                "cols": 512,
                "dpi": 96,
                "format": "pbf",
                "origin": {
                    "x": -34655613.478699818,
                    "y": 38474944.644759327
                },
                "spatialReference": {
                    "wkid": 3978,
                    "latestWkid": 3978
                },
                "lods": [
                    {
                        "level": 0,
                        "resolution": 135373.49015117117,
                        "scale": 511647836.79182798
                    },
                    {
                        "level": 1,
                        "resolution": 67686.745075585583,
                        "scale": 255823918.39591399
                    },
                    {
                        "level": 2,
                        "resolution": 33843.372537792791,
                        "scale": 127911959.19795699
                    },
                    {
                        "level": 3,
                        "resolution": 16921.686268896396,
                        "scale": 63955979.598978497
                    },
                    {
                        "level": 4,
                        "resolution": 8460.8431344481978,
                        "scale": 31977989.799489249
                    },
                    {
                        "level": 5,
                        "resolution": 4230.4215672240989,
                        "scale": 15988994.899744624
                    },
                    {
                        "level": 6,
                        "resolution": 2115.2107836120495,
                        "scale": 7994497.4498723121
                    },
                    {
                        "level": 7,
                        "resolution": 1057.6053918060247,
                        "scale": 3997248.7249361561
                    },
                    {
                        "level": 8,
                        "resolution": 528.80269590301236,
                        "scale": 1998624.362468078
                    },
                    {
                        "level": 9,
                        "resolution": 264.40134795150618,
                        "scale": 999312.18123403902
                    },
                    {
                        "level": 10,
                        "resolution": 132.20067397575309,
                        "scale": 499656.09061701951
                    },
                    {
                        "level": 11,
                        "resolution": 66.100336987876545,
                        "scale": 249828.04530850975
                    },
                    {
                        "level": 12,
                        "resolution": 33.050168493938273,
                        "scale": 124914.02265425488
                    },
                    {
                        "level": 13,
                        "resolution": 16.525084246969136,
                        "scale": 62457.011327127439
                    },
                    {
                        "level": 14,
                        "resolution": 8.2625421234845682,
                        "scale": 31228.505663563719
                    }
                ]
            },
            "maxzoom": 14,
            "minLOD": 0,
            "maxLOD": 14,
            "resourceInfo": {
                "styleVersion": 8,
                "tileCompression": "gzip",
                "cacheInfo": {
                    "storageInfo": {
                        "packetSize": 128,
                        "storageFormat": "compactV2"
                    }
                }
            }
        }

        vector_tile_set = QgsVectorTileMatrixSet()
        self.assertFalse(vector_tile_set.fromEsriJson({}))
        self.assertTrue(vector_tile_set.fromEsriJson(esri_metadata))

        # we should NOT apply the tile scale doubling hack to ESRI tiles, otherwise our scales
        # are double what ESRI use for the same tile sets
        self.assertEqual(vector_tile_set.scaleToTileZoomMethod(), Qgis.ScaleToTileZoomLevelMethod.Esri)

        self.assertEqual(vector_tile_set.minimumZoom(), 0)
        self.assertEqual(vector_tile_set.maximumZoom(), 14)

        self.assertEqual(vector_tile_set.crs().authid(), 'EPSG:3978')
        self.assertAlmostEqual(vector_tile_set.tileMatrix(0).extent().xMinimum(), -34655613.47869982, 3)
        self.assertAlmostEqual(vector_tile_set.tileMatrix(0).extent().yMinimum(), -30836282.31264031, 3)
        self.assertAlmostEqual(vector_tile_set.tileMatrix(0).extent().xMaximum(), 34655613.47869982, 3)
        self.assertAlmostEqual(vector_tile_set.tileMatrix(0).extent().yMaximum(), 38474944.64475933, 3)

        self.assertAlmostEqual(vector_tile_set.tileMatrix(0).scale(), 511647836.791828, 5)
        self.assertAlmostEqual(vector_tile_set.tileMatrix(1).scale(), 255823918.395914, 5)
        self.assertAlmostEqual(vector_tile_set.tileMatrix(2).scale(), 127911959.197957, 5)


if __name__ == '__main__':
    unittest.main()
