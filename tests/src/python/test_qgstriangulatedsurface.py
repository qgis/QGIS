"""QGIS Unit tests for QgsTriangulatedSurface

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Jean Felder"
__date__ = "12/08/2024"
__copyright__ = "Copyright 2024, The QGIS Project"

import qgis  # NOQA

from qgis.core import QgsPoint, QgsTriangle, QgsTriangulatedSurface, QgsWkbTypes
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsTriangulatedSurface(QgisTestCase):

    def test_constructor(self):
        surface = QgsTriangulatedSurface()
        self.assertTrue(surface.isEmpty())
        self.assertEqual(surface.numPatches(), 0)
        self.assertFalse(surface.is3D())
        self.assertFalse(surface.isMeasure())
        self.assertEqual(surface.wkbType(), QgsWkbTypes.Type.TIN)
        self.assertEqual(surface.wktTypeStr(), "TIN")
        self.assertEqual(surface.geometryType(), "TIN")
        self.assertEqual(surface.dimension(), 2)

    def test_wkt(self):
        # 2D
        surface = QgsTriangulatedSurface()
        surface.fromWkt("TIN (((0 0,0 1,1 0,0 0)),((1 0,0 1,1 1,1 0)))")
        self.assertFalse(surface.isEmpty())
        self.assertEqual(surface.numPatches(), 2)
        self.assertFalse(surface.is3D())
        self.assertFalse(surface.isMeasure())
        self.assertEqual(surface.wkbType(), QgsWkbTypes.Type.TIN)

        surface2 = QgsTriangulatedSurface()
        surface2.fromWkt(surface.asWkt())
        self.assertEqual(surface, surface2)

        # 3D
        surfaceZ = QgsTriangulatedSurface()
        surfaceZ.fromWkt("TIN Z (((0 0 0,0 1 0,1 1 0,0 0 0)))")
        self.assertFalse(surfaceZ.isEmpty())
        self.assertEqual(surfaceZ.numPatches(), 1)
        self.assertTrue(surfaceZ.is3D())
        self.assertFalse(surfaceZ.isMeasure())
        self.assertEqual(surfaceZ.wkbType(), QgsWkbTypes.Type.TINZ)

        surfaceZ2 = QgsTriangulatedSurface()
        surfaceZ2.fromWkt(surfaceZ.asWkt())
        self.assertEqual(surfaceZ, surfaceZ2)

        # Measure
        surfaceM = QgsTriangulatedSurface()
        surfaceM.fromWkt("TIN M (((0 0 3,0 1 3,1 1 3,0 0 3)))")
        self.assertFalse(surfaceM.isEmpty())
        self.assertEqual(surfaceM.numPatches(), 1)
        self.assertFalse(surfaceM.is3D())
        self.assertTrue(surfaceM.isMeasure())
        self.assertEqual(surfaceM.wkbType(), QgsWkbTypes.Type.TINM)

        surfaceM2 = QgsTriangulatedSurface()
        surfaceM2.fromWkt(surfaceM.asWkt())
        self.assertEqual(surfaceM, surfaceM2)

        # ZM
        surfaceZM = QgsTriangulatedSurface()
        surfaceZM.fromWkt(
            "TIN ZM "
            "(((0 0 1 2,0 1 1 2,1 1 1 2,0 0 1 2)),"
            "((10 10 0 0,10 11 0 0,11 11 0 0,10 10 0 0)))"
        )
        self.assertFalse(surfaceZM.isEmpty())
        self.assertEqual(surfaceZM.numPatches(), 2)
        self.assertTrue(surfaceZM.is3D())
        self.assertTrue(surfaceZM.isMeasure())
        self.assertEqual(surfaceZM.wkbType(), QgsWkbTypes.Type.TINZM)

        surfaceZM2 = QgsTriangulatedSurface()
        surfaceZM2.fromWkt(surfaceZM.asWkt())
        self.assertEqual(surfaceZM, surfaceZM2)

    def test_patch(self):
        surface = QgsTriangulatedSurface()
        self.assertTrue(surface.isEmpty())
        self.assertEqual(surface.numPatches(), 0)
        self.assertFalse(surface.is3D())
        self.assertFalse(surface.isMeasure())

        triangle1 = QgsTriangle(QgsPoint(0, 0), QgsPoint(0, 10), QgsPoint(10, 10))
        surface.addTriangle(triangle1)
        self.assertEqual(surface.numPatches(), 1)
        self.assertFalse(surface.is3D())
        self.assertFalse(surface.isMeasure())

        triangle2 = QgsTriangle(QgsPoint(10, 0), QgsPoint(10, 10), QgsPoint(20, 10))
        surface.addTriangle(triangle2)
        self.assertEqual(surface.numPatches(), 2)
        self.assertFalse(surface.is3D())
        self.assertFalse(surface.isMeasure())

        surface.clear()
        self.assertEqual(surface.numPatches(), 0)
        self.assertFalse(surface.is3D())
        self.assertFalse(surface.isMeasure())


if __name__ == "__main__":
    unittest.main()
