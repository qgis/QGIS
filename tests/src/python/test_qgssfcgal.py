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
    QgsSfcgalGeometry,
    QgsVector3D,
)
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsSFCGAL(QgisTestCase):

    def test_constructor(self):
        default_geom = QgsSfcgalGeometry(QgsPolyhedralSurface())
        is_empty = default_geom.isEmpty()
        self.assertFalse(default_geom.lastError())
        self.assertTrue(is_empty)

        surface = QgsPolyhedralSurface()
        surface_wkt = "POLYHEDRALSURFACE Z (((0 0 0,0 1 0,1 1 0,0 0 0)))"
        surface.fromWkt(surface_wkt)
        self.assertFalse(surface.isEmpty())
        self.assertEqual(surface.numPatches(), 1)
        self.assertTrue(surface.is3D())
        geom_surface = QgsSfcgalGeometry(surface)
        is_empty = geom_surface.isEmpty()
        self.assertFalse(geom_surface.lastError())
        self.assertFalse(is_empty)
        wkt_out = geom_surface.asWkt(0)
        self.assertFalse(geom_surface.lastError())
        self.assertEqual(wkt_out, surface_wkt)

        point = QgsSfcgalGeometry(QgsPoint(2, 3))
        is_empty = point.isEmpty()
        self.assertFalse(point.lastError())
        self.assertFalse(is_empty)
        is_valid = point.isValid()
        self.assertFalse(point.lastError())
        self.assertTrue(is_valid)
        wkt_out = point.asWkt(0)
        self.assertFalse(point.lastError())
        self.assertEqual(wkt_out, "POINT (2 3)")

    def test_from_wkt(self):
        tin_wkt = "TIN M (((0 0 3,0 1 3,1 1 3,0 0 3)))"
        geom = QgsSfcgalGeometry.fromWkt(tin_wkt)
        self.assertFalse(geom.lastError())
        self.assertTrue(isinstance(geom, QgsSfcgalGeometry))
        is_empty = geom.isEmpty()
        self.assertFalse(geom.lastError())
        self.assertFalse(is_empty)

        wkt_out = geom.asWkt(0)
        self.assertFalse(geom.lastError())
        self.assertEqual(wkt_out, tin_wkt)

    def test_from_wkb(self):
        wkb_in = b"\x01\xf8\x03\x00\x00\x02\x00\x00\x00\x01\xf9\x03\x00\x00\x01\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0?\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0?\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\xf9\x03\x00\x00\x01\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0?\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0?\x00\x00\x00\x00\x00\x00\xf0?\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0?\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0?\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        geom = QgsSfcgalGeometry.fromWkb(QgsConstWkbPtr(wkb_in))
        self.assertFalse(geom.lastError())
        self.assertTrue(isinstance(geom, QgsSfcgalGeometry))
        is_empty = geom.isEmpty()
        self.assertFalse(geom.lastError())
        self.assertFalse(is_empty)

        expected_wkt = "TIN Z (((0 0 0,1 0 0,0 1 0,0 0 0)),((1 0 0,1 1 0,0 1 0,1 0 0)))"
        wkt_out = geom.asWkt(0)
        self.assertFalse(geom.lastError())
        self.assertEqual(wkt_out, expected_wkt)

        wkb_out = geom.asWkb()
        self.assertFalse(geom.lastError())
        self.assertEqual(wkb_out, wkb_in)

    def test_is_valid(self):
        surface_wkt = "POLYHEDRALSURFACE Z (((0 0 0,0 100 0,100 100 0,100 0 0,0 0 0)),((0 0 0,50 50 50,0 100 0,0 0 0)),((100 0 0,100 100 0,50 50 50,100 0 0)))"
        sfcgal_geom = QgsSfcgalGeometry.fromWkt(surface_wkt)
        self.assertFalse(sfcgal_geom.lastError())

        is_empty = sfcgal_geom.isEmpty()
        self.assertFalse(sfcgal_geom.lastError())
        self.assertFalse(is_empty)

        is_valid = sfcgal_geom.isValid()
        self.assertFalse(sfcgal_geom.lastError())
        self.assertTrue(is_valid)

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
            self.assertFalse(sfcgal_geom.lastError())
            wkt_out = sfcgal_geom.asWkt(0)
            self.assertFalse(sfcgal_geom.lastError())
            self.assertEqual(wkt_out, geom[0])

            part_count = sfcgal_geom.partCount()
            self.assertFalse(sfcgal_geom.lastError())
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

            self.assertFalse(sfcgal_geom.lastError())
            wkt_out = sfcgal_geom.asWkt(0)
            self.assertFalse(sfcgal_geom.lastError())
            self.assertEqual(wkt_out, geom[0])

            area = sfcgal_geom.area()
            self.assertFalse(sfcgal_geom.lastError())
            self.assertAlmostEqual(area, geom[1], 5)

    def test_scale(self):
        wkt = "LINESTRING Z (0 0 0,1 0 0,0 1 0,0 0 1,0 0 0)"
        sfcgal_geom = QgsSfcgalGeometry.fromWkt(wkt)
        self.assertFalse(sfcgal_geom.lastError())

        scaled_geom = sfcgal_geom.scale(QgsVector3D(1.5, 2, 0.5))
        self.assertFalse(scaled_geom.lastError())
        expected_wkt = (
            "LINESTRING Z (0.0 0.0 0.0,1.5 0.0 0.0,0.0 2.0 0.0,0.0 0.0 0.5,0.0 0.0 0.0)"
        )
        wkt_out = scaled_geom.asWkt(1)
        self.assertFalse(scaled_geom.lastError())
        self.assertEqual(wkt_out, expected_wkt)

    def test_rotate(self):
        # 2D rotation
        wkt = "TRIANGLE ((1 1,2 2,2 1,1 1))"
        sfcgal_geom = QgsSfcgalGeometry.fromWkt(wkt)
        self.assertFalse(sfcgal_geom.lastError())

        rotated_2d_geom = sfcgal_geom.rotate2D(pi / 2.0, QgsPoint(1, 1))
        self.assertFalse(rotated_2d_geom.lastError())
        expected_wkt = "TRIANGLE ((1 1,0 2,1 2,1 1))"
        wkt_out = rotated_2d_geom.asWkt(0)
        self.assertFalse(rotated_2d_geom.lastError())
        self.assertEqual(wkt_out, expected_wkt)

        # 3D rotation
        rotated_3d_geom = sfcgal_geom.rotate3D(
            pi / 2.0, QgsVector3D(2, 1, 0), QgsPoint(1, 1, 0)
        )
        self.assertFalse(rotated_3d_geom.lastError())
        expected_wkt = "TRIANGLE Z ((1 1 0,2 2 0,2 1 0,1 1 0))"
        wkt_out = rotated_3d_geom.asWkt(0)
        self.assertFalse(rotated_3d_geom.lastError())
        self.assertEqual(wkt_out, expected_wkt)

    def test_intersection(self):
        point = QgsSfcgalGeometry(QgsPoint(2, 3))

        polygon1 = QgsSfcgalGeometry.fromWkt("POLYGON ((0 0,10 0,10 10,0 10,0 0))")
        self.assertFalse(polygon1.lastError())

        intersects = polygon1.intersects(point)
        self.assertFalse(polygon1.lastError())
        self.assertTrue(intersects)

        polygon2 = QgsSfcgalGeometry.fromWkt("POLYGON ((-1 -1,1 -1,1 1,-1 1,-1 -1))")
        self.assertFalse(polygon2.lastError())
        intersects = polygon2.intersects(point)
        self.assertFalse(polygon2.lastError())
        self.assertFalse(intersects)

        intersects = polygon2.intersects(polygon1)
        self.assertFalse(polygon2.lastError())
        self.assertTrue(intersects)

        intersection = polygon2.intersection(polygon1)
        self.assertFalse(intersection.lastError())
        wkt_out = intersection.asWkt(0)
        self.assertFalse(intersection.lastError())
        expected_intersection = "POLYGON ((0 1,0 0,1 0,1 1,0 1))"
        self.assertEqual(wkt_out, expected_intersection)

    def test_combine(self):
        point = QgsSfcgalGeometry(QgsPoint(2, 3))

        combined_geom = point.combine([QgsPoint(4, 5)])
        self.assertFalse(combined_geom.lastError())
        wkt_out = combined_geom.asWkt(0)
        expected_wkt = "MULTIPOINT ((2 3),(4 5))"
        self.assertFalse(combined_geom.lastError())
        self.assertEqual(wkt_out, expected_wkt)

    def test_difference(self):
        polygon1 = QgsSfcgalGeometry.fromWkt("POLYGON ((0 0,10 0,10 10,0 10,0 0))")
        self.assertFalse(polygon1.lastError())
        polygon2 = QgsSfcgalGeometry.fromWkt("POLYGON ((-1 -1,1 -1,1 1,-1 1,-1 -1))")
        self.assertFalse(polygon2.lastError())

        diff_geom = polygon1.difference(polygon2)
        self.assertFalse(diff_geom.lastError())
        wkt_out = diff_geom.asWkt(0)
        expected_wkt = "POLYGON ((0 10,0 1,1 1,1 0,10 0,10 10,0 10))"
        self.assertFalse(diff_geom.lastError())
        self.assertEqual(wkt_out, expected_wkt)

    def test_triangulate(self):
        poly_wkt = "POLYGON ((0.0 0.0,1.0 0.0,1.0 1.0,0.0 1.0,0.0 0.0),(0.2 0.2,0.2 0.8,0.8 0.8,0.8 0.2,0.2 0.2))"
        polygon = QgsSfcgalGeometry.fromWkt(poly_wkt)
        self.assertFalse(polygon.lastError())

        triangulation = polygon.triangulate()
        self.assertFalse(triangulation.lastError())
        self.assertEqual(triangulation.wkbType(), Qgis.WkbType.TIN)
        self.assertEqual(triangulation.partCount(), 10)

    def test_convexhull(self):
        poly_wkt = "POLYGON ((0.0 0.0,1.0 0.0,1.0 1.0,0.0 1.0,0.0 0.0),(0.2 0.2,0.2 0.8,0.8 0.8,0.8 0.2,0.2 0.2))"
        polygon = QgsSfcgalGeometry.fromWkt(poly_wkt)
        self.assertFalse(polygon.lastError())

        convex_hull = polygon.convexHull()
        self.assertFalse(convex_hull.lastError())

        expected_wkt = "POLYGON ((0 0,1 0,1 1,0 1,0 0))"
        wkt_out = convex_hull.asWkt(0)
        self.assertFalse(convex_hull.lastError())
        self.assertEqual(wkt_out, expected_wkt)

    def test_approximate_media_axis(self):
        poly_wkt = "TRIANGLE ((1 1,2 1,2 2,1 1))"
        polygon = QgsSfcgalGeometry.fromWkt(poly_wkt)
        self.assertFalse(polygon.lastError())

        medial_axis = polygon.approximateMedialAxis()
        self.assertFalse(medial_axis.lastError())

        expected_wkt = "MULTILINESTRING ((1.0 1.0,1.7 1.3),(2.0 2.0,1.7 1.3))"
        wkt_out = medial_axis.asWkt(1)
        self.assertFalse(medial_axis.lastError())
        self.assertEqual(wkt_out, expected_wkt)


if __name__ == "__main__":
    unittest.main()
