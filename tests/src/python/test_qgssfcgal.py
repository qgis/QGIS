"""QGIS Unit tests for QgsSfcgalGeometry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Jean Felder"
__date__ = "05/08/2025"
__copyright__ = "Copyright 2025, The QGIS Project"

import unittest
from math import pi

from qgis.core import (
    Qgis,
    QgsConstWkbPtr,
    QgsPoint,
    QgsPolyhedralSurface,
    QgsSfcgalException,
    QgsSfcgalGeometry,
    QgsVector3D,
)
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsSFCGAL(QgisTestCase):

    def test_constructor(self):
        default_geom = QgsSfcgalGeometry(QgsPolyhedralSurface())
        self.assertTrue(default_geom.isEmpty())

        surface = QgsPolyhedralSurface()
        surface_wkt = "POLYHEDRALSURFACE Z (((0 0 0,0 1 0,1 1 0,0 0 0)))"
        surface.fromWkt(surface_wkt)
        self.assertFalse(surface.isEmpty())
        self.assertEqual(surface.numPatches(), 1)
        self.assertTrue(surface.is3D())
        geom_surface = QgsSfcgalGeometry(surface)
        self.assertFalse(geom_surface.isEmpty())
        self.assertEqual(geom_surface.asWkt(0), surface_wkt)

        point = QgsSfcgalGeometry(QgsPoint(2, 3))
        self.assertFalse(point.isEmpty())
        self.assertTrue(point.isValid())
        self.assertEqual(point.asWkt(0), "POINT (2 3)")

        # try to create a geometry from an invalid wkt
        with self.assertRaises(QgsSfcgalException):
            QgsSfcgalGeometry.fromWkt("POINT (33 2, 1 2)")

    def test_from_wkt(self):
        tin_wkt = "TIN M (((0 0 3,0 1 3,1 1 3,0 0 3)))"
        geom = QgsSfcgalGeometry.fromWkt(tin_wkt)
        self.assertTrue(isinstance(geom, QgsSfcgalGeometry))
        self.assertFalse(geom.isEmpty())

        self.assertEqual(geom.asWkt(0), tin_wkt)

    def test_from_wkb(self):
        wkb_in = b"\x01\xf8\x03\x00\x00\x02\x00\x00\x00\x01\xf9\x03\x00\x00\x01\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0?\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0?\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\xf9\x03\x00\x00\x01\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0?\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0?\x00\x00\x00\x00\x00\x00\xf0?\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0?\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0?\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        geom = QgsSfcgalGeometry.fromWkb(QgsConstWkbPtr(wkb_in))
        self.assertTrue(isinstance(geom, QgsSfcgalGeometry))
        self.assertFalse(geom.isEmpty())

        expected_wkt = "TIN Z (((0 0 0,1 0 0,0 1 0,0 0 0)),((1 0 0,1 1 0,0 1 0,1 0 0)))"
        self.assertEqual(geom.asWkt(0), expected_wkt)

        self.assertEqual(geom.asWkb(), wkb_in)

    def test_is_valid(self):
        surface_wkt = "POLYHEDRALSURFACE Z (((0 0 0,0 100 0,100 100 0,100 0 0,0 0 0)),((0 0 0,50 50 50,0 100 0,0 0 0)),((100 0 0,100 100 0,50 50 50,100 0 0)))"
        sfcgal_geom = QgsSfcgalGeometry.fromWkt(surface_wkt)

        self.assertFalse(sfcgal_geom.isEmpty())

        self.assertTrue(sfcgal_geom.isValid())

    def test_part_count(self):
        geoms = [
            ["TIN Z (((0 0 0,1 0 0,0 1 0,0 0 0)),((1 0 0,1 1 0,0 1 0,1 0 0)))", 2],
            ["POINT (0 1)", 1],
            [
                "GEOMETRYCOLLECTION (POINT (0 0),POINT (2 2),LINESTRING (1 1,2 2,1 1))",
                3,
            ],
            ["TRIANGLE ((1 1,2 2,2 1,1 1))", 3],
            [
                "MULTIPOINT ((50 40),(140 70),(80 100),(130 140),(30 150),(70 180),(190 110),(120 20))",
                8,
            ],
            [
                "POLYGON ((100 180,160 20,20 20,100 180),(100 180,80 60,120 60,100 180))",
                2,
            ],
            [
                "POLYHEDRALSURFACE Z (((0 0 0,0 100 0,100 100 0,100 0 0,0 0 0)),((0 0 0,50 50 50,0 100 0,0 0 0)),((100 0 0,100 100 0,50 50 50,100 0 0)))",
                3,
            ],
        ]
        for geom in geoms:
            sfcgal_geom = QgsSfcgalGeometry.fromWkt(geom[0])
            self.assertEqual(sfcgal_geom.asWkt(0), geom[0])

            part_count = sfcgal_geom.partCount()
            self.assertEqual(part_count, geom[1])

    def test_area(self):
        geoms = [
            ["TRIANGLE ((1 1,2 2,2 1,1 1))", 0.5],
            [
                "POLYGON ((100 180,160 20,20 20,100 180),(100 180,80 60,120 60,100 180))",
                8800.0,
            ],
            ["LINESTRING (0 0,1 1,2 2,0 0)", 0.0],
            ["LINESTRING Z (0 0 0,1 0 0,0 1 0,0 0 1,0 0 0)", 0.0],
            [
                "POLYHEDRALSURFACE Z (((0 0 0,0 100 0,100 100 0,100 0 0,0 0 0)),((0 0 0,50 50 50,0 100 0,0 0 0)),((100 0 0,100 100 0,50 50 50,100 0 0)))",
                17071.06781,
            ],
        ]
        for geom in geoms:
            sfcgal_geom = QgsSfcgalGeometry.fromWkt(geom[0])

            self.assertEqual(sfcgal_geom.asWkt(0), geom[0])
            self.assertAlmostEqual(sfcgal_geom.area(), geom[1], 5)

    def test_scale(self):
        wkt = "LINESTRING Z (0 0 0,1 0 0,0 1 0,0 0 1,0 0 0)"
        sfcgal_geom = QgsSfcgalGeometry.fromWkt(wkt)

        scaled_geom = sfcgal_geom.scale(QgsVector3D(1.5, 2, 0.5))
        expected_wkt = (
            "LINESTRING Z (0.0 0.0 0.0,1.5 0.0 0.0,0.0 2.0 0.0,0.0 0.0 0.5,0.0 0.0 0.0)"
        )
        self.assertEqual(scaled_geom.asWkt(1), expected_wkt)

    def test_rotate(self):
        # 2D rotation
        wkt = "TRIANGLE ((1 1,2 2,2 1,1 1))"
        sfcgal_geom = QgsSfcgalGeometry.fromWkt(wkt)

        rotated_2d_geom = sfcgal_geom.rotate2D(pi / 2.0, QgsPoint(1, 1))
        expected_wkt = "TRIANGLE ((1 1,0 2,1 2,1 1))"
        self.assertEqual(rotated_2d_geom.asWkt(0), expected_wkt)

        # 3D rotation
        rotated_3d_geom = sfcgal_geom.rotate3D(
            pi / 2.0, QgsVector3D(2, 1, 0), QgsPoint(1, 1, 0)
        )
        expected_wkt = "TRIANGLE Z ((1 1 0,2 2 0,2 1 0,1 1 0))"
        self.assertEqual(rotated_3d_geom.asWkt(0), expected_wkt)

    def test_intersection(self):
        invalid_polygon = QgsSfcgalGeometry(QgsPoint(2, 3))

        polygon1 = QgsSfcgalGeometry.fromWkt("POLYGON ((0 0,10 0,10 10,0 10,0 0))")

        intersects = polygon1.intersects(invalid_polygon)
        self.assertTrue(intersects)

        polygon2 = QgsSfcgalGeometry.fromWkt("POLYGON ((-1 -1,1 -1,1 1,-1 1,-1 -1))")
        intersects = polygon2.intersects(invalid_polygon)
        self.assertFalse(intersects)

        intersects = polygon2.intersects(polygon1)
        self.assertTrue(intersects)

        intersection = polygon2.intersection(polygon1)
        expected_intersection = "POLYGON ((0 1,0 0,1 0,1 1,0 1))"
        self.assertEqual(intersection.asWkt(0), expected_intersection)

    def test_combine(self):
        invalid_polygon = QgsSfcgalGeometry(QgsPoint(2, 3))

        combined_geom = invalid_polygon.combine([QgsPoint(4, 5)])
        expected_wkt = "MULTIPOINT ((2 3),(4 5))"
        self.assertEqual(combined_geom.asWkt(0), expected_wkt)

    def test_difference(self):
        polygon1 = QgsSfcgalGeometry.fromWkt("POLYGON ((0 0,10 0,10 10,0 10,0 0))")
        polygon2 = QgsSfcgalGeometry.fromWkt("POLYGON ((-1 -1,1 -1,1 1,-1 1,-1 -1))")

        diff_geom = polygon1.difference(polygon2)
        expected_wkt = "POLYGON ((0 10,0 1,1 1,1 0,10 0,10 10,0 10))"
        self.assertEqual(diff_geom.asWkt(0), expected_wkt)

    def test_triangulate(self):
        poly_wkt = "POLYGON ((0.0 0.0,1.0 0.0,1.0 1.0,0.0 1.0,0.0 0.0),(0.2 0.2,0.2 0.8,0.8 0.8,0.8 0.2,0.2 0.2))"
        polygon = QgsSfcgalGeometry.fromWkt(poly_wkt)

        triangulation = polygon.triangulate()
        self.assertEqual(triangulation.wkbType(), Qgis.WkbType.TIN)
        self.assertEqual(triangulation.partCount(), 10)

    def test_convexhull(self):
        poly_wkt = "POLYGON ((0.0 0.0,1.0 0.0,1.0 1.0,0.0 1.0,0.0 0.0),(0.2 0.2,0.2 0.8,0.8 0.8,0.8 0.2,0.2 0.2))"
        polygon = QgsSfcgalGeometry.fromWkt(poly_wkt)

        convex_hull = polygon.convexHull()

        expected_wkt = "POLYGON ((0 0,1 0,1 1,0 1,0 0))"
        self.assertEqual(convex_hull.asWkt(0), expected_wkt)

    def test_approximate_media_axis(self):
        poly_wkt = "TRIANGLE ((1 1,2 1,2 2,1 1))"
        polygon = QgsSfcgalGeometry.fromWkt(poly_wkt)

        medial_axis = polygon.approximateMedialAxis()

        expected_wkt = "MULTILINESTRING ((1.0 1.0,1.7 1.3),(2.0 2.0,1.7 1.3))"
        self.assertEqual(medial_axis.asWkt(1), expected_wkt)

        # medial axis does not work on an invalid geometry
        invalid_polygon_wkt = "POLYGON((0 0, 2 2, 0 2, 2 0, 0 0))"
        invalid_polygon = QgsSfcgalGeometry.fromWkt(invalid_polygon_wkt)
        with self.assertRaises(QgsSfcgalException):
            invalid_polygon.approximateMedialAxis()


if __name__ == "__main__":
    unittest.main()
