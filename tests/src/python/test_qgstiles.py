"""QGIS Unit tests for QgsTileMatrix

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "04/03/2022"
__copyright__ = "Copyright 2022, The QGIS Project"

from qgis.PyQt.QtCore import QSize
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsPointXY,
    QgsReadWriteContext,
    QgsRectangle,
    QgsTileMatrix,
    QgsTileMatrixSet,
    QgsTileRange,
    QgsTileXYZ,
    QgsVectorTileMatrixSet,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsTiles(QgisTestCase):

    def testQgsTileXYZ(self):
        tile = QgsTileXYZ(1, 2, 3)
        self.assertEqual(tile.column(), 1)
        self.assertEqual(tile.row(), 2)
        self.assertEqual(tile.zoomLevel(), 3)

    def testQgsTileXYZRepr(self):
        tile = QgsTileXYZ(1, 2, 3)
        self.assertEqual(str(tile), "<QgsTileXYZ: 1, 2, 3>")

    def testQgsTileXYZEquality(self):
        tile = QgsTileXYZ(1, 2, 3)
        tile2 = QgsTileXYZ(1, 2, 3)
        self.assertEqual(tile, tile2)
        tile2 = QgsTileXYZ(1, 2, 4)
        self.assertNotEqual(tile, tile2)
        tile2 = QgsTileXYZ(1, 4, 3)
        self.assertNotEqual(tile, tile2)
        tile2 = QgsTileXYZ(4, 2, 3)
        self.assertNotEqual(tile, tile2)

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
        matrix = QgsTileMatrix.fromCustomDef(
            5, QgsCoordinateReferenceSystem("EPSG:4326"), QgsPointXY(1, 2), 1000, 4, 8
        )
        self.assertEqual(matrix.zoomLevel(), 5)
        self.assertEqual(matrix.crs().authid(), "EPSG:4326")

        matrix.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        self.assertEqual(matrix.crs().authid(), "EPSG:3857")

        matrix.setZoomLevel(6)
        self.assertEqual(matrix.zoomLevel(), 6)

        matrix.setScale(10000)
        self.assertEqual(matrix.scale(), 10000)

    def testQgsTileMatrixSet(self):
        matrix_set = QgsTileMatrixSet()

        # should be applied by default in order to match MapBox rendering of tiles
        self.assertEqual(
            matrix_set.scaleToTileZoomMethod(), Qgis.ScaleToTileZoomLevelMethod.MapBox
        )

        self.assertEqual(matrix_set.minimumZoom(), -1)
        self.assertEqual(matrix_set.maximumZoom(), -1)
        self.assertFalse(matrix_set.crs().isValid())

        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(
                1,
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsPointXY(1, 2),
                1000,
                4,
                8,
            )
        )
        self.assertEqual(matrix_set.minimumZoom(), 1)
        self.assertEqual(matrix_set.maximumZoom(), 1)
        self.assertEqual(matrix_set.crs().authid(), "EPSG:4326")

        range = QgsTileRange(1, 3, 4, 7)
        tiles = matrix_set.tilesInRange(range, 1)
        self.assertEqual(len(tiles), 12)
        self.assertEqual(min(t.column() for t in tiles), 1)
        self.assertEqual(max(t.column() for t in tiles), 3)
        self.assertEqual(min(t.row() for t in tiles), 4)
        self.assertEqual(max(t.row() for t in tiles), 7)

        # should not apply any special logic here, and return scales unchanged
        self.assertEqual(
            matrix_set.calculateTileScaleForMap(
                1000,
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsRectangle(0, 2, 20, 12),
                QSize(20, 10),
                96,
            ),
            1000,
        )
        self.assertEqual(
            matrix_set.calculateTileScaleForMap(
                1000,
                QgsCoordinateReferenceSystem("EPSG:3857"),
                QgsRectangle(0, 2, 20, 12),
                QSize(20, 10),
                96,
            ),
            1000,
        )

        self.assertEqual(matrix_set.tileMatrix(1).zoomLevel(), 1)
        # zoom level not present in matrix!
        self.assertEqual(matrix_set.tileMatrix(99).zoomLevel(), -1)

        self.assertAlmostEqual(matrix_set.tileMatrix(1).scale(), 776503144, -1)
        self.assertEqual(matrix_set.scaleToZoom(776503144), 1)
        self.assertEqual(matrix_set.scaleToZoom(1776503144), 1)
        # overzooming past max zoom level
        self.assertAlmostEqual(matrix_set.scaleToZoom(76503144), 3.423637, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(6503144), 6.928011, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(1625786), 8.928011, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(50805.8125), 13.928011, 5)
        self.assertEqual(matrix_set.scaleToZoomLevel(776503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(1776503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(76503144), 1)
        # turn off zoom level clamping to minimum / maximum zoom values
        self.assertEqual(matrix_set.scaleToZoomLevel(76503144, False), 3)
        self.assertEqual(matrix_set.scaleToZoomLevel(1625786, False), 9)

        # add a second level
        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(
                2,
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsPointXY(1, 2),
                1000,
                4,
                8,
            )
        )
        self.assertEqual(matrix_set.minimumZoom(), 1)
        self.assertEqual(matrix_set.maximumZoom(), 2)
        self.assertEqual(matrix_set.crs().authid(), "EPSG:4326")

        self.assertEqual(matrix_set.tileMatrix(1).zoomLevel(), 1)
        self.assertEqual(matrix_set.tileMatrix(2).zoomLevel(), 2)
        self.assertEqual(matrix_set.tileMatrix(99).zoomLevel(), -1)

        tiles = matrix_set.tilesInRange(QgsTileRange(1, 3, 4, 7), 1)
        self.assertEqual(len(tiles), 12)
        self.assertEqual(min(t.column() for t in tiles), 1)
        self.assertEqual(max(t.column() for t in tiles), 3)
        self.assertEqual(min(t.row() for t in tiles), 4)
        self.assertEqual(max(t.row() for t in tiles), 7)
        self.assertEqual(min(t.zoomLevel() for t in tiles), 1)
        self.assertEqual(max(t.zoomLevel() for t in tiles), 1)
        tiles = matrix_set.tilesInRange(QgsTileRange(2, 4, 1, 3), 2)
        self.assertEqual(len(tiles), 9)
        self.assertEqual(min(t.column() for t in tiles), 2)
        self.assertEqual(max(t.column() for t in tiles), 4)
        self.assertEqual(min(t.row() for t in tiles), 1)
        self.assertEqual(max(t.row() for t in tiles), 3)
        self.assertEqual(min(t.zoomLevel() for t in tiles), 2)
        self.assertEqual(max(t.zoomLevel() for t in tiles), 2)

        self.assertAlmostEqual(matrix_set.scaleToZoom(776503144), 1, 5)
        self.assertEqual(matrix_set.scaleToZoom(1776503144), 1)
        # overzooming past max zoom level
        self.assertAlmostEqual(matrix_set.scaleToZoom(76503144), 3.423637, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(6503144), 6.928011, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(1625786), 8.928011, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(50805.8125), 13.928011, 5)
        self.assertEqual(matrix_set.scaleToZoomLevel(776503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(1776503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(76503144), 2)
        self.assertEqual(matrix_set.scaleToZoomLevel(6503144), 2)

        # add a third level
        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(
                3,
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsPointXY(1, 2),
                1000,
                4,
                8,
            )
        )
        self.assertEqual(matrix_set.minimumZoom(), 1)
        self.assertEqual(matrix_set.maximumZoom(), 3)
        self.assertEqual(matrix_set.crs().authid(), "EPSG:4326")

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
        # overzooming past max zoom level
        self.assertAlmostEqual(matrix_set.scaleToZoom(76503144), 3.423637, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(6503144), 6.928011, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(1625786), 8.928011, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(50805.8125), 13.928011, 5)
        self.assertEqual(matrix_set.scaleToZoomLevel(776503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(1776503144), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(76503144), 3)
        self.assertEqual(matrix_set.scaleToZoomLevel(388251572), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(298251572), 1)
        self.assertEqual(matrix_set.scaleToZoomLevel(198251572), 2)
        self.assertEqual(matrix_set.scaleToZoomLevel(6503144), 3)

        # with ESRI scale to zoom handling
        matrix_set.setScaleToTileZoomMethod(Qgis.ScaleToTileZoomLevelMethod.Esri)
        self.assertEqual(
            matrix_set.scaleToTileZoomMethod(), Qgis.ScaleToTileZoomLevelMethod.Esri
        )

        self.assertAlmostEqual(matrix_set.scaleToZoom(776503144), 1, 5)
        self.assertEqual(matrix_set.scaleToZoom(1776503144), 1)
        self.assertAlmostEqual(matrix_set.scaleToZoom(388251572), 2, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(288251572), 2.515, 2)
        self.assertAlmostEqual(matrix_set.scaleToZoom(194125786), 3, 5)
        # overzooming past max zoom level
        self.assertAlmostEqual(matrix_set.scaleToZoom(188251572), 3.0605, 3)
        self.assertAlmostEqual(matrix_set.scaleToZoom(6503144), 7.928011, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(1625786), 9.928011, 5)
        self.assertAlmostEqual(matrix_set.scaleToZoom(50805.8125), 14.928011, 5)
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

        # tile matrix 0 should not be present -- we restricted the range to 1-13
        self.assertAlmostEqual(matrix_set.tileMatrix(0).zoomLevel(), -1)
        # but the root tile matrix should still be available for calculations
        self.assertTrue(matrix_set.rootMatrix().isRootTileMatrix())
        self.assertEqual(matrix_set.rootMatrix().matrixWidth(), 1)
        self.assertEqual(matrix_set.rootMatrix().matrixHeight(), 1)
        self.assertEqual(matrix_set.rootMatrix().crs().authid(), "EPSG:3857")
        self.assertAlmostEqual(
            matrix_set.rootMatrix().extent().xMinimum(), -20037508.3427892, 3
        )
        self.assertAlmostEqual(
            matrix_set.rootMatrix().extent().xMaximum(), 20037508.3427892, 3
        )
        self.assertAlmostEqual(
            matrix_set.rootMatrix().extent().yMinimum(), -20037508.3427892, 3
        )
        self.assertAlmostEqual(
            matrix_set.rootMatrix().extent().yMaximum(), 20037508.3427892, 3
        )

    def testTileMatrixSetRemoveTiles(self):
        matrix_set = QgsTileMatrixSet()
        matrix_set.addGoogleCrs84QuadTiles(1, 13)

        matrix_set.dropMatricesOutsideZoomRange(4, 10)
        self.assertEqual(matrix_set.minimumZoom(), 4)
        self.assertEqual(matrix_set.maximumZoom(), 10)

    def testReadWriteXml(self):
        matrix_set = QgsTileMatrixSet()
        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(
                1,
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsPointXY(1, 2),
                1000,
                4,
                8,
            )
        )
        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(
                2,
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsPointXY(1, 2),
                1000,
                4,
                8,
            )
        )
        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(
                3,
                QgsCoordinateReferenceSystem("EPSG:3857"),
                QgsPointXY(1, 2),
                1000,
                4,
                8,
            )
        )

        matrix_set.setRootMatrix(
            QgsTileMatrix.fromCustomDef(
                0,
                QgsCoordinateReferenceSystem("EPSG:3857"),
                QgsPointXY(1, 2),
                1000,
                1,
                1,
            )
        )

        doc = QDomDocument("testdoc")
        res = matrix_set.writeXml(doc, QgsReadWriteContext())

        set2 = QgsTileMatrixSet()
        self.assertTrue(set2.readXml(res, QgsReadWriteContext()))
        self.assertEqual(set2.minimumZoom(), 1)
        self.assertEqual(set2.maximumZoom(), 3)

        self.assertEqual(set2.tileMatrix(1).crs().authid(), "EPSG:4326")
        self.assertEqual(set2.tileMatrix(2).crs().authid(), "EPSG:4326")
        self.assertEqual(set2.tileMatrix(3).crs().authid(), "EPSG:3857")

        self.assertEqual(set2.rootMatrix().crs().authid(), "EPSG:3857")
        self.assertTrue(set2.rootMatrix().isRootTileMatrix())
        self.assertAlmostEqual(set2.rootMatrix().extent().xMinimum(), 1, 3)
        self.assertAlmostEqual(set2.rootMatrix().extent().xMaximum(), 1001, 3)
        self.assertAlmostEqual(set2.rootMatrix().extent().yMinimum(), -998, 3)
        self.assertAlmostEqual(set2.rootMatrix().extent().yMaximum(), 2, 3)

    def testVectorTileMatrixSet(self):
        matrix_set = QgsVectorTileMatrixSet()

        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(
                1,
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsPointXY(1, 2),
                1000,
                4,
                8,
            )
        )
        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(
                2,
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsPointXY(1, 2),
                1000,
                4,
                8,
            )
        )
        matrix_set.addMatrix(
            QgsTileMatrix.fromCustomDef(
                3,
                QgsCoordinateReferenceSystem("EPSG:3857"),
                QgsPointXY(1, 2),
                1000,
                4,
                8,
            )
        )

        doc = QDomDocument("testdoc")
        res = matrix_set.writeXml(doc, QgsReadWriteContext())

        set2 = QgsVectorTileMatrixSet()
        self.assertTrue(set2.readXml(res, QgsReadWriteContext()))
        self.assertEqual(set2.minimumZoom(), 1)
        self.assertEqual(set2.maximumZoom(), 3)

        self.assertEqual(set2.tileMatrix(1).crs().authid(), "EPSG:4326")
        self.assertEqual(set2.tileMatrix(2).crs().authid(), "EPSG:4326")
        self.assertEqual(set2.tileMatrix(3).crs().authid(), "EPSG:3857")

    def testVectorTileMatrixSetFromESRI(self):
        esri_metadata = {
            "tiles": ["tile/{z}/{y}/{x}.pbf"],
            "initialExtent": {
                "xmin": -2750565.3405000009,
                "ymin": -936638.5,
                "xmax": 3583872.5,
                "ymax": 4659267,
                "spatialReference": {"wkid": 3978, "latestWkid": 3978},
            },
            "fullExtent": {
                "xmin": -2750565.3405000009,
                "ymin": -936638.5,
                "xmax": 3583872.5,
                "ymax": 4659267,
                "spatialReference": {"wkid": 3978, "latestWkid": 3978},
            },
            "minScale": 511647836.79182798,
            "maxScale": 31228.505663563719,
            "tileInfo": {
                "rows": 512,
                "cols": 512,
                "dpi": 96,
                "format": "pbf",
                "origin": {"x": -34655613.478699818, "y": 38474944.644759327},
                "spatialReference": {"wkid": 3978, "latestWkid": 3978},
                "lods": [
                    {
                        "level": 0,
                        "resolution": 135373.49015117117,
                        "scale": 511647836.79182798,
                    },
                    {
                        "level": 1,
                        "resolution": 67686.745075585583,
                        "scale": 255823918.39591399,
                    },
                    {
                        "level": 2,
                        "resolution": 33843.372537792791,
                        "scale": 127911959.19795699,
                    },
                    {
                        "level": 3,
                        "resolution": 16921.686268896396,
                        "scale": 63955979.598978497,
                    },
                    {
                        "level": 4,
                        "resolution": 8460.8431344481978,
                        "scale": 31977989.799489249,
                    },
                    {
                        "level": 5,
                        "resolution": 4230.4215672240989,
                        "scale": 15988994.899744624,
                    },
                    {
                        "level": 6,
                        "resolution": 2115.2107836120495,
                        "scale": 7994497.4498723121,
                    },
                    {
                        "level": 7,
                        "resolution": 1057.6053918060247,
                        "scale": 3997248.7249361561,
                    },
                    {
                        "level": 8,
                        "resolution": 528.80269590301236,
                        "scale": 1998624.362468078,
                    },
                    {
                        "level": 9,
                        "resolution": 264.40134795150618,
                        "scale": 999312.18123403902,
                    },
                    {
                        "level": 10,
                        "resolution": 132.20067397575309,
                        "scale": 499656.09061701951,
                    },
                    {
                        "level": 11,
                        "resolution": 66.100336987876545,
                        "scale": 249828.04530850975,
                    },
                    {
                        "level": 12,
                        "resolution": 33.050168493938273,
                        "scale": 124914.02265425488,
                    },
                    {
                        "level": 13,
                        "resolution": 16.525084246969136,
                        "scale": 62457.011327127439,
                    },
                    {
                        "level": 14,
                        "resolution": 8.2625421234845682,
                        "scale": 31228.505663563719,
                    },
                ],
            },
            "maxzoom": 14,
            "minLOD": 0,
            "maxLOD": 14,
            "resourceInfo": {
                "styleVersion": 8,
                "tileCompression": "gzip",
                "cacheInfo": {
                    "storageInfo": {"packetSize": 128, "storageFormat": "compactV2"}
                },
            },
        }

        vector_tile_set = QgsVectorTileMatrixSet()
        self.assertFalse(vector_tile_set.fromEsriJson({}))
        self.assertTrue(vector_tile_set.fromEsriJson(esri_metadata))

        # should not apply any special logic here for non-geographic CRS, and return scales unchanged
        self.assertEqual(
            vector_tile_set.calculateTileScaleForMap(
                1000,
                QgsCoordinateReferenceSystem("EPSG:3857"),
                QgsRectangle(0, 2, 20, 12),
                QSize(20, 10),
                96,
            ),
            1000,
        )

        # for geographic CRS the scale should be calculated using the scale at the equator.
        # see https://support.esri.com/en/technical-article/000007211,
        # https://gis.stackexchange.com/questions/33270/how-does-arcmap-calculate-scalebar-inside-a-wgs84-layout
        self.assertAlmostEqual(
            vector_tile_set.calculateTileScaleForMap(
                420735075,
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsRectangle(0, 2, 20, 12),
                QSize(2000, 1000),
                96,
            ),
            4207351,
            0,
        )
        self.assertAlmostEqual(
            vector_tile_set.calculateTileScaleForMap(
                420735075,
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsRectangle(0, 62, 20, 72),
                QSize(2000, 1000),
                96,
            ),
            4207351,
            0,
        )

        # we should NOT apply the tile scale doubling hack to ESRI tiles, otherwise our scales
        # are double what ESRI use for the same tile sets
        self.assertEqual(
            vector_tile_set.scaleToTileZoomMethod(),
            Qgis.ScaleToTileZoomLevelMethod.Esri,
        )

        self.assertEqual(vector_tile_set.minimumZoom(), 0)
        self.assertEqual(vector_tile_set.maximumZoom(), 14)

        self.assertTrue(vector_tile_set.rootMatrix().isRootTileMatrix())
        self.assertEqual(vector_tile_set.rootMatrix().matrixWidth(), 1)
        self.assertEqual(vector_tile_set.rootMatrix().matrixHeight(), 1)
        self.assertEqual(vector_tile_set.rootMatrix().crs().authid(), "EPSG:3978")
        self.assertAlmostEqual(
            vector_tile_set.rootMatrix().extent().xMinimum(), -34655613.47869982, 3
        )
        self.assertAlmostEqual(
            vector_tile_set.rootMatrix().extent().xMaximum(), 34655613.47869982, 3
        )
        self.assertAlmostEqual(
            vector_tile_set.rootMatrix().extent().yMinimum(), -30836282.31264031, 3
        )
        self.assertAlmostEqual(
            vector_tile_set.rootMatrix().extent().yMaximum(), 38474944.64475933, 3
        )

        self.assertEqual(vector_tile_set.crs().authid(), "EPSG:3978")
        self.assertAlmostEqual(
            vector_tile_set.tileMatrix(0).extent().xMinimum(), -34655613.47869982, 3
        )
        self.assertAlmostEqual(
            vector_tile_set.tileMatrix(0).extent().yMinimum(), -30836282.31264031, 3
        )
        self.assertAlmostEqual(
            vector_tile_set.tileMatrix(0).extent().xMaximum(), 34655613.47869982, 3
        )
        self.assertAlmostEqual(
            vector_tile_set.tileMatrix(0).extent().yMaximum(), 38474944.64475933, 3
        )

        self.assertAlmostEqual(
            vector_tile_set.tileMatrix(0).scale(), 511647836.791828, 5
        )
        self.assertAlmostEqual(
            vector_tile_set.tileMatrix(1).scale(), 255823918.395914, 5
        )
        self.assertAlmostEqual(
            vector_tile_set.tileMatrix(2).scale(), 127911959.197957, 5
        )

    def test_esri_with_tilemap(self):
        """
        Test handling a tile matrix set with an ESRI tilemap indicating missing/replaced tiles
        """
        esri_metadata = {
            "currentVersion": 11.0,
            "name": "Map",
            "copyrightText": "usa",
            "capabilities": "TilesOnly",
            "type": "indexedVector",
            "tileMap": "tilemap",
            "defaultStyles": "resources/styles",
            "tiles": ["tile/{z}/{y}/{x}.pbf"],
            "exportTilesAllowed": False,
            "initialExtent": {
                "xmin": 4.5783729072156216,
                "ymin": 46.874323689779565,
                "xmax": 16.331358957713661,
                "ymax": 55.452061808267452,
                "spatialReference": {"wkid": 4326, "latestWkid": 4326},
            },
            "fullExtent": {
                "xmin": 4.5783729072156216,
                "ymin": 46.874323689779565,
                "xmax": 16.331358957713661,
                "ymax": 55.452061808267452,
                "spatialReference": {"wkid": 4326, "latestWkid": 4326},
            },
            "minScale": 295828763.79585469,
            "maxScale": 564.24858817263544,
            "tileInfo": {
                "rows": 512,
                "cols": 512,
                "dpi": 96,
                "format": "pbf",
                "origin": {"x": -180, "y": 90},
                "spatialReference": {"wkid": 4326, "latestWkid": 4326},
                "lods": [
                    {"level": 0, "resolution": 0.703125, "scale": 295828763.79585469},
                    {"level": 1, "resolution": 0.3515625, "scale": 147914381.89792734},
                    {"level": 2, "resolution": 0.17578125, "scale": 73957190.948963672},
                    {
                        "level": 3,
                        "resolution": 0.087890625,
                        "scale": 36978595.474481836,
                    },
                    {
                        "level": 4,
                        "resolution": 0.0439453125,
                        "scale": 18489297.737240918,
                    },
                    {
                        "level": 5,
                        "resolution": 0.02197265625,
                        "scale": 9244648.868620459,
                    },
                    {
                        "level": 6,
                        "resolution": 0.010986328125,
                        "scale": 4622324.4343102295,
                    },
                    {
                        "level": 7,
                        "resolution": 0.0054931640625,
                        "scale": 2311162.2171551147,
                    },
                    {
                        "level": 8,
                        "resolution": 0.00274658203125,
                        "scale": 1155581.1085775574,
                    },
                    {
                        "level": 9,
                        "resolution": 0.001373291015625,
                        "scale": 577790.55428877869,
                    },
                    {
                        "level": 10,
                        "resolution": 0.0006866455078125,
                        "scale": 288895.27714438934,
                    },
                    {
                        "level": 11,
                        "resolution": 0.00034332275390625,
                        "scale": 144447.63857219467,
                    },
                    {
                        "level": 12,
                        "resolution": 0.000171661376953125,
                        "scale": 72223.819286097336,
                    },
                    {
                        "level": 13,
                        "resolution": 8.58306884765625e-05,
                        "scale": 36111.909643048668,
                    },
                    {
                        "level": 14,
                        "resolution": 4.291534423828125e-05,
                        "scale": 18055.954821524334,
                    },
                    {
                        "level": 15,
                        "resolution": 2.1457672119140625e-05,
                        "scale": 9027.977410762167,
                    },
                    {
                        "level": 16,
                        "resolution": 1.0728836059570312e-05,
                        "scale": 4513.9887053810835,
                    },
                    {
                        "level": 17,
                        "resolution": 5.3644180297851562e-06,
                        "scale": 2256.9943526905417,
                    },
                    {
                        "level": 18,
                        "resolution": 2.6822090148925781e-06,
                        "scale": 1128.4971763452709,
                    },
                    {
                        "level": 19,
                        "resolution": 1.3411045074462891e-06,
                        "scale": 564.24858817263544,
                    },
                ],
            },
            "maxzoom": 19,
            "minLOD": 0,
            "maxLOD": 14,
            "resourceInfo": {
                "styleVersion": 8,
                "tileCompression": "gzip",
                "cacheInfo": {
                    "storageInfo": {"packetSize": 128, "storageFormat": "compactV2"}
                },
            },
        }

        tilemap = {
            "index": [
                0,
                [
                    [
                        [
                            0,
                            0,
                            [
                                0,
                                0,
                                0,
                                [
                                    [
                                        0,
                                        0,
                                        [
                                            0,
                                            [
                                                1,
                                                1,
                                                [
                                                    [0, 1, 0, [1, 1, 1, 1]],
                                                    [
                                                        [1, 1, 1, 1],
                                                        [1, 1, 1, [1, 1, 1, 1]],
                                                        [1, 1, 1, 1],
                                                        [
                                                            [1, 1, 1, 1],
                                                            [
                                                                1,
                                                                [1, 1, 1, 1],
                                                                [1, 1, 1, 1],
                                                                [1, 1, 1, 1],
                                                            ],
                                                            1,
                                                            [1, 1, 1, 1],
                                                        ],
                                                    ],
                                                    1,
                                                    1,
                                                ],
                                                [
                                                    [
                                                        [1, 1, 1, 1],
                                                        [1, 1, 1, 1],
                                                        [
                                                            [
                                                                [1, 1, 1, 1],
                                                                [1, 1, 1, 1],
                                                                [1, 1, 1, 1],
                                                                [1, 1, 1, 1],
                                                            ],
                                                            [1, 1, 1, 1],
                                                            [1, 1, 1, 1],
                                                            1,
                                                        ],
                                                        [[1, 1, 1, 1], 1, 1, 1],
                                                    ],
                                                    [1, 1, [1, 1, 1, 1], 1],
                                                    [[1, [1, 1, 1, 1], 0, 1], 1, 0, 0],
                                                    1,
                                                ],
                                            ],
                                            0,
                                            0,
                                        ],
                                        0,
                                    ],
                                    0,
                                    0,
                                    0,
                                ],
                            ],
                            0,
                        ],
                        0,
                        0,
                        0,
                    ],
                    0,
                    0,
                    0,
                ],
                0,
                0,
            ]
        }

        vector_tile_set_orig = QgsVectorTileMatrixSet()
        self.assertTrue(vector_tile_set_orig.fromEsriJson(esri_metadata, tilemap))

        self.assertEqual(vector_tile_set_orig.minimumZoom(), 0)
        self.assertEqual(vector_tile_set_orig.maximumZoom(), 19)

        # use a copy, to also test that tilemap copying works correctly
        vector_tile_set = QgsVectorTileMatrixSet(vector_tile_set_orig)

        # check tile availability
        # zoom level not available
        self.assertEqual(
            vector_tile_set.tileAvailability(QgsTileXYZ(0, 0, 101)),
            Qgis.TileAvailability.NotAvailable,
        )

        # zoom level 0
        self.assertEqual(vector_tile_set.tileMatrix(0).matrixWidth(), 1)
        self.assertEqual(vector_tile_set.tileMatrix(0).matrixHeight(), 1)
        self.assertEqual(
            vector_tile_set.tileAvailability(QgsTileXYZ(0, 0, 0)),
            Qgis.TileAvailability.Available,
        )
        # tile outside matrix
        self.assertEqual(
            vector_tile_set.tileAvailability(QgsTileXYZ(1, 0, 0)),
            Qgis.TileAvailability.NotAvailable,
        )
        self.assertEqual(
            vector_tile_set.tileAvailability(QgsTileXYZ(0, 1, 0)),
            Qgis.TileAvailability.NotAvailable,
        )

        tiles = vector_tile_set.tilesInRange(QgsTileRange(0, 0, 0, 0), 0)
        self.assertEqual(tiles, [QgsTileXYZ(0, 0, 0)])

        # zoom level 1
        self.assertEqual(vector_tile_set.tileMatrix(1).matrixWidth(), 2)
        self.assertEqual(vector_tile_set.tileMatrix(1).matrixHeight(), 2)
        self.assertEqual(
            vector_tile_set.tileAvailability(QgsTileXYZ(0, 0, 1)),
            Qgis.TileAvailability.NotAvailable,
        )
        self.assertEqual(
            vector_tile_set.tileAvailability(QgsTileXYZ(1, 0, 1)),
            Qgis.TileAvailability.Available,
        )
        self.assertEqual(
            vector_tile_set.tileAvailability(QgsTileXYZ(0, 1, 1)),
            Qgis.TileAvailability.NotAvailable,
        )
        self.assertEqual(
            vector_tile_set.tileAvailability(QgsTileXYZ(1, 1, 1)),
            Qgis.TileAvailability.NotAvailable,
        )

        tiles = vector_tile_set.tilesInRange(QgsTileRange(0, 1, 0, 1), 1)
        self.assertEqual(tiles, [QgsTileXYZ(1, 0, 1)])

        # zoom level 2
        self.assertEqual(vector_tile_set.tileMatrix(2).matrixWidth(), 4)
        self.assertEqual(vector_tile_set.tileMatrix(2).matrixHeight(), 4)

        for col in range(4):
            for row in range(4):
                if col == 2 and row == 0:
                    expected = Qgis.TileAvailability.Available
                else:
                    expected = Qgis.TileAvailability.NotAvailable

                self.assertEqual(
                    vector_tile_set.tileAvailability(QgsTileXYZ(col, row, 2)), expected
                )

        tiles = vector_tile_set.tilesInRange(QgsTileRange(0, 2, 0, 2), 2)
        self.assertEqual(tiles, [QgsTileXYZ(2, 0, 2)])

        # zoom level 3
        self.assertEqual(vector_tile_set.tileMatrix(3).matrixWidth(), 8)
        self.assertEqual(vector_tile_set.tileMatrix(3).matrixHeight(), 8)
        for col in range(8):
            for row in range(8):
                if col == 4 and row == 0:
                    expected = Qgis.TileAvailability.Available
                else:
                    expected = Qgis.TileAvailability.NotAvailable

                self.assertEqual(
                    vector_tile_set.tileAvailability(QgsTileXYZ(col, row, 3)), expected
                )

        tiles = vector_tile_set.tilesInRange(QgsTileRange(0, 7, 0, 7), 3)
        self.assertEqual(tiles, [QgsTileXYZ(4, 0, 3)])

        # zoom level 4
        self.assertEqual(vector_tile_set.tileMatrix(4).matrixWidth(), 16)
        self.assertEqual(vector_tile_set.tileMatrix(4).matrixHeight(), 16)
        for col in range(16):
            for row in range(16):
                if col == 8 and row == 1:
                    expected = Qgis.TileAvailability.Available
                else:
                    expected = Qgis.TileAvailability.NotAvailable

                tile = QgsTileXYZ(col, row, 4)
                self.assertEqual(
                    vector_tile_set.tileAvailability(tile),
                    expected,
                    msg=f"Failed for {tile}",
                )

        tiles = vector_tile_set.tilesInRange(QgsTileRange(0, 15, 0, 15), 4)
        self.assertEqual(tiles, [QgsTileXYZ(8, 1, 4)])

        # zoom level 5
        self.assertEqual(vector_tile_set.tileMatrix(5).matrixWidth(), 32)
        self.assertEqual(vector_tile_set.tileMatrix(5).matrixHeight(), 32)
        for col in range(32):
            for row in range(32):
                if col == 17 and row == 3:
                    expected = Qgis.TileAvailability.Available
                else:
                    expected = Qgis.TileAvailability.NotAvailable

                tile = QgsTileXYZ(col, row, 5)
                self.assertEqual(
                    vector_tile_set.tileAvailability(tile),
                    expected,
                    msg=f"Failed for {tile}",
                )

        tiles = vector_tile_set.tilesInRange(QgsTileRange(0, 31, 0, 31), 5)
        self.assertEqual(tiles, [QgsTileXYZ(17, 3, 5)])

        # zoom level 6
        self.assertEqual(vector_tile_set.tileMatrix(6).matrixWidth(), 64)
        self.assertEqual(vector_tile_set.tileMatrix(6).matrixHeight(), 64)
        for col in range(64):
            for row in range(64):
                if col == 34 and row == 6:
                    expected = Qgis.TileAvailability.Available
                else:
                    expected = Qgis.TileAvailability.NotAvailable

                tile = QgsTileXYZ(col, row, 6)
                self.assertEqual(
                    vector_tile_set.tileAvailability(tile),
                    expected,
                    msg=f"Failed for {tile}",
                )

        tiles = vector_tile_set.tilesInRange(QgsTileRange(0, 63, 0, 63), 6)
        self.assertEqual(tiles, [QgsTileXYZ(34, 6, 6)])

        # zoom level 7
        self.assertEqual(vector_tile_set.tileMatrix(7).matrixWidth(), 128)
        self.assertEqual(vector_tile_set.tileMatrix(7).matrixHeight(), 128)
        for col in range(128):
            for row in range(128):
                if col == 68 and row == 13:
                    expected = Qgis.TileAvailability.Available
                else:
                    expected = Qgis.TileAvailability.NotAvailable

                tile = QgsTileXYZ(col, row, 7)
                self.assertEqual(
                    vector_tile_set.tileAvailability(tile),
                    expected,
                    msg=f"Failed for {tile}",
                )

        tiles = vector_tile_set.tilesInRange(QgsTileRange(0, 127, 0, 127), 7)
        self.assertEqual(tiles, [QgsTileXYZ(68, 13, 7)])

        # zoom level 8
        self.assertEqual(vector_tile_set.tileMatrix(8).matrixWidth(), 256)
        self.assertEqual(vector_tile_set.tileMatrix(8).matrixHeight(), 256)
        for col in range(256):
            for row in range(256):
                if col == 137 and row == 26:
                    expected = Qgis.TileAvailability.Available
                else:
                    expected = Qgis.TileAvailability.NotAvailable

                tile = QgsTileXYZ(col, row, 8)
                self.assertEqual(
                    vector_tile_set.tileAvailability(tile),
                    expected,
                    msg=f"Failed for {tile}",
                )

        tiles = vector_tile_set.tilesInRange(QgsTileRange(0, 255, 0, 255), 8)
        self.assertEqual(tiles, [QgsTileXYZ(137, 26, 8)])

        # zoom level 9
        self.assertEqual(vector_tile_set.tileMatrix(9).matrixWidth(), 512)
        self.assertEqual(vector_tile_set.tileMatrix(9).matrixHeight(), 512)
        for col in range(512):
            for row in range(512):
                if col in (274, 275) and row == 52:
                    expected = Qgis.TileAvailability.AvailableNoChildren
                elif col in (274, 275) and row == 53:
                    expected = Qgis.TileAvailability.Available
                else:
                    expected = Qgis.TileAvailability.NotAvailable

                tile = QgsTileXYZ(col, row, 9)
                self.assertEqual(
                    vector_tile_set.tileAvailability(tile),
                    expected,
                    msg=f"Failed for {tile}",
                )

        tiles = vector_tile_set.tilesInRange(QgsTileRange(0, 511, 0, 511), 9)
        self.assertCountEqual(
            tiles,
            [
                QgsTileXYZ(274, 52, 9),
                QgsTileXYZ(275, 52, 9),
                QgsTileXYZ(274, 53, 9),
                QgsTileXYZ(275, 53, 9),
            ],
        )

        # zoom level 10
        self.assertEqual(vector_tile_set.tileMatrix(10).matrixWidth(), 1024)
        self.assertEqual(vector_tile_set.tileMatrix(10).matrixHeight(), 1024)
        for col in range(1024):
            for row in range(1024):
                if col in (548, 549, 550, 551) and row in (104, 105):
                    expected = Qgis.TileAvailability.UseLowerZoomLevelTile
                elif (col in (548, 549, 550, 551) and row == 106) or (
                    col == 550 and row == 107
                ):
                    expected = Qgis.TileAvailability.Available
                elif col in (548, 549, 551) and row == 107:
                    expected = Qgis.TileAvailability.AvailableNoChildren
                else:
                    expected = Qgis.TileAvailability.NotAvailable

                tile = QgsTileXYZ(col, row, 10)
                self.assertEqual(
                    vector_tile_set.tileAvailability(tile),
                    expected,
                    msg=f"Failed for {tile}",
                )

        tiles = vector_tile_set.tilesInRange(QgsTileRange(0, 1023, 0, 1023), 10)
        # we want to see the zoom level 9 tiles here, as the tilemap indicates
        # that they should be used instead of zoom level 10 tiles for their
        # extents
        self.assertCountEqual(
            tiles,
            [
                QgsTileXYZ(274, 52, 9),
                QgsTileXYZ(275, 52, 9),
                QgsTileXYZ(548, 106, 10),
                QgsTileXYZ(549, 106, 10),
                QgsTileXYZ(550, 106, 10),
                QgsTileXYZ(551, 106, 10),
                QgsTileXYZ(548, 107, 10),
                QgsTileXYZ(549, 107, 10),
                QgsTileXYZ(550, 107, 10),
                QgsTileXYZ(551, 107, 10),
            ],
        )

        # zoom level 11
        self.assertEqual(vector_tile_set.tileMatrix(11).matrixWidth(), 2048)
        self.assertEqual(vector_tile_set.tileMatrix(11).matrixHeight(), 2048)
        for col in range(2048):
            for row in range(2048):
                if (
                    col in (1096, 1097, 1098, 1099, 1100, 1101, 1102, 1103)
                    and row in (208, 209, 210, 211)
                ) or (
                    col in (1096, 1097, 1098, 1099, 1102, 1103) and row in (214, 215)
                ):
                    expected = Qgis.TileAvailability.UseLowerZoomLevelTile
                elif (
                    (col in (1098, 1099, 1100, 1101) and row == 212)
                    or (col == 1100 and row == 214)
                    or (col in (1097, 1098, 1099, 1100, 1101, 1102) and row == 213)
                ):
                    expected = Qgis.TileAvailability.Available
                elif (
                    (col in (1096, 1097, 1098, 1099, 1100, 1101) and row == 214)
                    or (col in (1097, 1102, 1103) and row == 212)
                    or (col == 1103 and row == 213)
                ):
                    expected = Qgis.TileAvailability.AvailableNoChildren
                else:
                    expected = Qgis.TileAvailability.NotAvailable

                tile = QgsTileXYZ(col, row, 11)
                self.assertEqual(
                    vector_tile_set.tileAvailability(tile),
                    expected,
                    msg=f"Failed for {tile}",
                )


if __name__ == "__main__":
    unittest.main()
