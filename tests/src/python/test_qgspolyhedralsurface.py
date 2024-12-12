"""QGIS Unit tests for QgsPolyhedralSurface

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Jean Felder"
__date__ = "12/08/2024"
__copyright__ = "Copyright 2024, The QGIS Project"

import qgis  # NOQA

from qgis.core import (
    QgsLineString,
    QgsPoint,
    QgsPolygon,
    QgsPolyhedralSurface,
    QgsWkbTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsPolyhedralSurface(QgisTestCase):

    def test_constructor(self):
        surface = QgsPolyhedralSurface()
        self.assertTrue(surface.isEmpty())
        self.assertEqual(surface.numPatches(), 0)
        self.assertFalse(surface.is3D())
        self.assertFalse(surface.isMeasure())
        self.assertEqual(surface.wkbType(), QgsWkbTypes.Type.PolyhedralSurface)
        self.assertEqual(surface.wktTypeStr(), "PolyhedralSurface")
        self.assertEqual(surface.geometryType(), "PolyhedralSurface")
        self.assertEqual(surface.dimension(), 2)

    def test_wkt(self):
        # 2D
        surface = QgsPolyhedralSurface()
        surface.fromWkt("POLYHEDRALSURFACE (((0 0,0 1,1 1,1 0,0 0)))")
        self.assertFalse(surface.isEmpty())
        self.assertEqual(surface.numPatches(), 1)
        self.assertFalse(surface.is3D())
        self.assertFalse(surface.isMeasure())
        self.assertEqual(surface.wkbType(), QgsWkbTypes.Type.PolyhedralSurface)

        surface2 = QgsPolyhedralSurface()
        surface2.fromWkt(surface.asWkt())
        self.assertEqual(surface, surface2)

        # 3D
        surfaceZ = QgsPolyhedralSurface()
        surfaceZ.fromWkt("POLYHEDRALSURFACE Z (((0 0 0,0 1 0,1 1 0,0 0 0)))")
        self.assertFalse(surfaceZ.isEmpty())
        self.assertEqual(surfaceZ.numPatches(), 1)
        self.assertTrue(surfaceZ.is3D())
        self.assertFalse(surfaceZ.isMeasure())
        self.assertEqual(surfaceZ.wkbType(), QgsWkbTypes.Type.PolyhedralSurfaceZ)

        surfaceZ2 = QgsPolyhedralSurface()
        surfaceZ2.fromWkt(surfaceZ.asWkt())
        self.assertEqual(surfaceZ, surfaceZ2)

        # Measure
        surfaceM = QgsPolyhedralSurface()
        surfaceM.fromWkt("POLYHEDRALSURFACE M (((0 0 3,0 1 3,1 1 3,0 0 3)))")
        self.assertFalse(surfaceM.isEmpty())
        self.assertEqual(surfaceM.numPatches(), 1)
        self.assertFalse(surfaceM.is3D())
        self.assertTrue(surfaceM.isMeasure())
        self.assertEqual(surfaceM.wkbType(), QgsWkbTypes.Type.PolyhedralSurfaceM)

        surfaceM2 = QgsPolyhedralSurface()
        surfaceM2.fromWkt(surfaceM.asWkt())
        self.assertEqual(surfaceM, surfaceM2)

        # ZM
        surfaceZM = QgsPolyhedralSurface()
        surfaceZM.fromWkt(
            "POLYHEDRALSURFACE ZM "
            "(((0 0 1 2,0 1 1 2,1 1 1 2,0 0 1 2)),"
            "((10 10 0 0,10 11 0 0,11 11 0 0,10 10 0 0)))"
        )
        self.assertFalse(surfaceZM.isEmpty())
        self.assertEqual(surfaceZM.numPatches(), 2)
        self.assertTrue(surfaceZM.is3D())
        self.assertTrue(surfaceZM.isMeasure())
        self.assertEqual(surfaceZM.wkbType(), QgsWkbTypes.Type.PolyhedralSurfaceZM)

        surfaceZM2 = QgsPolyhedralSurface()
        surfaceZM2.fromWkt(surfaceZM.asWkt())
        self.assertEqual(surfaceZM, surfaceZM2)

    def test_patch(self):
        surface = QgsPolyhedralSurface()
        self.assertTrue(surface.isEmpty())
        self.assertEqual(surface.numPatches(), 0)
        self.assertFalse(surface.is3D())
        self.assertFalse(surface.isMeasure())

        patch1 = QgsPolygon()
        patchExterior1 = QgsLineString(
            [
                QgsPoint(0, 0),
                QgsPoint(0, 10),
                QgsPoint(10, 10),
                QgsPoint(10, 0),
                QgsPoint(0, 0),
            ]
        )
        patch1.setExteriorRing(patchExterior1)
        patchInteriorRing = QgsLineString(
            [
                QgsPoint(1, 1),
                QgsPoint(1, 9),
                QgsPoint(9, 9),
                QgsPoint(9, 1),
                QgsPoint(1, 1),
            ]
        )
        patch1.addInteriorRing(patchInteriorRing)
        surface.addPatch(patch1)
        self.assertEqual(surface.numPatches(), 1)
        self.assertFalse(surface.is3D())
        self.assertFalse(surface.isMeasure())

        patch2 = QgsPolygon()
        patchExterior2 = QgsLineString(
            [
                QgsPoint(10, 0),
                QgsPoint(10, 10),
                QgsPoint(20, 10),
                QgsPoint(20, 0),
                QgsPoint(10, 0),
            ]
        )
        patch2.setExteriorRing(patchExterior2)
        surface.addPatch(patch2)
        self.assertEqual(surface.numPatches(), 2)
        self.assertFalse(surface.is3D())
        self.assertFalse(surface.isMeasure())

        surface.clear()
        self.assertEqual(surface.numPatches(), 0)
        self.assertFalse(surface.is3D())
        self.assertFalse(surface.isMeasure())

    def test_len(self):
        surface1 = QgsPolyhedralSurface()
        self.assertEqual(surface1.numPatches(), 0)
        self.assertEqual(len(surface1), 0)

        patch1 = QgsPolygon()
        patchExterior1 = QgsLineString(
            [
                QgsPoint(0, 0),
                QgsPoint(0, 10),
                QgsPoint(10, 10),
                QgsPoint(10, 0),
                QgsPoint(0, 0),
            ]
        )
        patch1.setExteriorRing(patchExterior1)
        patchInteriorRing = QgsLineString(
            [
                QgsPoint(1, 1),
                QgsPoint(1, 9),
                QgsPoint(9, 9),
                QgsPoint(9, 1),
                QgsPoint(1, 1),
            ]
        )
        patch1.addInteriorRing(patchInteriorRing)
        surface1.addPatch(patch1)
        self.assertEqual(surface1.numPatches(), 1)
        self.assertEqual(len(surface1), 1)

        surface2 = QgsPolyhedralSurface()
        surface2.fromWkt(
            "POLYHEDRALSURFACE ZM "
            "(((0 0 1 2,0 1 1 2,1 1 1 2,0 0 1 2)),"
            "((10 10 0 0,10 11 0 0,11 11 0 0,10 10 0 0)))"
        )
        self.assertTrue(surface2.numPatches(), 2)
        self.assertTrue(len(surface2), 2)

    def test_getitem(self):
        surface = QgsPolyhedralSurface()
        with self.assertRaises(IndexError):
            surface[0]
        with self.assertRaises(IndexError):
            surface[-1]

        patch1 = QgsPolygon()
        patchExterior1 = QgsLineString(
            [
                QgsPoint(0, 0),
                QgsPoint(0, 10),
                QgsPoint(10, 10),
                QgsPoint(10, 0),
                QgsPoint(0, 0),
            ]
        )
        patch1.setExteriorRing(patchExterior1)
        patchInteriorRing = QgsLineString(
            [
                QgsPoint(1, 1),
                QgsPoint(1, 9),
                QgsPoint(9, 9),
                QgsPoint(9, 1),
                QgsPoint(1, 1),
            ]
        )
        patch1.addInteriorRing(patchInteriorRing)
        surface.addPatch(patch1)
        self.assertEqual(surface.numPatches(), 1)
        self.assertEqual(surface[0], patch1)
        self.assertEqual(surface[-1], patch1)
        with self.assertRaises(IndexError):
            surface[1]
        with self.assertRaises(IndexError):
            surface[-2]

        patch2 = QgsPolygon()
        patchExterior2 = QgsLineString(
            [
                QgsPoint(10, 0),
                QgsPoint(10, 10),
                QgsPoint(20, 10),
                QgsPoint(20, 0),
                QgsPoint(10, 0),
            ]
        )
        patch2.setExteriorRing(patchExterior2)
        surface.addPatch(patch2)
        self.assertEqual(surface.numPatches(), 2)
        self.assertEqual(surface[0], patch1)
        self.assertEqual(surface[1], patch2)
        self.assertEqual(surface[-1], patch2)
        self.assertEqual(surface[-2], patch1)
        with self.assertRaises(IndexError):
            surface[2]
        with self.assertRaises(IndexError):
            surface[-3]


if __name__ == "__main__":
    unittest.main()
