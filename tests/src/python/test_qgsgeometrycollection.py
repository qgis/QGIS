"""QGIS Unit tests for QgsGeometryCollection.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Loïc Bartoletti"
__date__ = "19/12/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import unittest

from qgis.core import (
    QgsGeometryCollection,
    QgsPoint,
    Qgis,
    QgsLineString,
    QgsPolygon,
    QgsMultiPoint,
    QgsMultiLineString,
    QgsMultiPolygon,
    QgsRectangle,
)
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsGeometryCollection(QgisTestCase):

    def testFuzzyComparisons(self):
        ######
        # 2D #
        ######
        epsilon = 0.001
        geom1 = QgsGeometryCollection()
        self.assertTrue(geom1.addGeometry(QgsPoint(0.0, 0.0)))
        self.assertTrue(geom1.addGeometry(QgsPoint(0.001, 0.001)))
        geom2 = QgsGeometryCollection()
        self.assertTrue(geom2.addGeometry(QgsPoint(0.0, 0.0)))
        self.assertTrue(geom2.addGeometry(QgsPoint(0.002, 0.002)))

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

        #######
        # 3DZ #
        #######
        epsilon = 0.001
        geom1 = QgsGeometryCollection()
        self.assertTrue(geom1.addGeometry(QgsPoint(0.0, 0.0, 0.0)))
        self.assertTrue(geom1.addGeometry(QgsPoint(0.001, 0.001, 0.001)))
        geom2 = QgsGeometryCollection()
        self.assertTrue(geom2.addGeometry(QgsPoint(0.0, 0.0, 0.0)))
        self.assertTrue(geom2.addGeometry(QgsPoint(0.001, 0.001, 0.002)))

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

        #######
        # 3DM #
        #######
        epsilon = 0.001
        geom1 = QgsGeometryCollection()
        self.assertTrue(geom1.addGeometry(QgsPoint(0.0, 0.0, m=0.0)))
        self.assertTrue(geom1.addGeometry(QgsPoint(0.001, 0.001, m=0.001)))
        geom2 = QgsGeometryCollection()
        self.assertTrue(geom2.addGeometry(QgsPoint(0.0, 0.0, m=0.0)))
        self.assertTrue(geom2.addGeometry(QgsPoint(0.001, 0.001, m=0.002)))

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

        ######
        # 4D #
        ######
        epsilon = 0.001
        geom1 = QgsGeometryCollection()
        self.assertTrue(geom1.addGeometry(QgsPoint(0.0, 0.0, 0.0, 0.0)))
        self.assertTrue(geom1.addGeometry(QgsPoint(0.001, 0.001, 0.001, 0.001)))
        geom2 = QgsGeometryCollection()
        self.assertTrue(geom2.addGeometry(QgsPoint(0.0, 0.0, 0.0, 0.0)))
        self.assertTrue(geom2.addGeometry(QgsPoint(0.001, 0.001, 0.002, 0.002)))

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

    def test_extract_parts_by_type(self):
        """
        Test QgsGeometryCollection.extract_parts_by_type
        """

        empty_collection = QgsGeometryCollection()
        self.assertEqual(
            empty_collection.extractPartsByType(Qgis.WkbType.Point).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            empty_collection.extractPartsByType(Qgis.WkbType.MultiPoint).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            empty_collection.extractPartsByType(Qgis.WkbType.MultiPointZ).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            empty_collection.extractPartsByType(Qgis.WkbType.MultiPointM).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            empty_collection.extractPartsByType(Qgis.WkbType.MultiPointZM).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            empty_collection.extractPartsByType(Qgis.WkbType.LineString).asWkt(),
            "MultiLineString EMPTY",
        )
        self.assertEqual(
            empty_collection.extractPartsByType(Qgis.WkbType.Polygon).asWkt(),
            "MultiPolygon EMPTY",
        )
        # ambiguous types
        self.assertIsNone(empty_collection.extractPartsByType(Qgis.WkbType.NoGeometry))
        self.assertIsNone(empty_collection.extractPartsByType(Qgis.WkbType.Unknown))

        point_collection = QgsGeometryCollection()
        point_collection.addGeometry(QgsPoint(1, 2))
        point_collection.addGeometry(QgsPoint(11, 22))
        point_collection.addGeometry(QgsPoint(3, 4))
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.Point).asWkt(),
            "MultiPoint ((1 2),(11 22),(3 4))",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.MultiPoint).asWkt(),
            "MultiPoint ((1 2),(11 22),(3 4))",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.MultiPointZ).asWkt(),
            "MultiPoint ((1 2),(11 22),(3 4))",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.MultiPointM).asWkt(),
            "MultiPoint ((1 2),(11 22),(3 4))",
        )
        # strict dimension check
        self.assertEqual(
            point_collection.extractPartsByType(
                Qgis.WkbType.MultiPointZ, useFlatType=False
            ).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            point_collection.extractPartsByType(
                Qgis.WkbType.MultiPointM, useFlatType=False
            ).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            point_collection.extractPartsByType(
                Qgis.WkbType.MultiPointZM, useFlatType=False
            ).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.LineString).asWkt(),
            "MultiLineString EMPTY",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.Polygon).asWkt(),
            "MultiPolygon EMPTY",
        )
        # with z
        point_collection = QgsGeometryCollection()
        point_collection.addGeometry(QgsPoint(1, 2, 3))
        point_collection.addGeometry(QgsPoint(11, 22, 33))
        point_collection.addGeometry(QgsPoint(3, 4, 5))
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.Point).asWkt(),
            "MultiPoint Z ((1 2 3),(11 22 33),(3 4 5))",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.MultiPoint).asWkt(),
            "MultiPoint Z ((1 2 3),(11 22 33),(3 4 5))",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.MultiPointZ).asWkt(),
            "MultiPoint Z ((1 2 3),(11 22 33),(3 4 5))",
        )
        # strict dimension check
        self.assertEqual(
            point_collection.extractPartsByType(
                Qgis.WkbType.MultiPointZ, useFlatType=False
            ).asWkt(),
            "MultiPoint Z ((1 2 3),(11 22 33),(3 4 5))",
        )
        self.assertEqual(
            point_collection.extractPartsByType(
                Qgis.WkbType.MultiPointZM, useFlatType=False
            ).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.LineString).asWkt(),
            "MultiLineString EMPTY",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.Polygon).asWkt(),
            "MultiPolygon EMPTY",
        )

        # with m
        point_collection = QgsGeometryCollection()
        point_collection.addGeometry(QgsPoint(1, 2, m=3))
        point_collection.addGeometry(QgsPoint(11, 22, m=33))
        point_collection.addGeometry(QgsPoint(3, 4, m=5))
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.Point).asWkt(),
            "MultiPoint M ((1 2 3),(11 22 33),(3 4 5))",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.MultiPoint).asWkt(),
            "MultiPoint M ((1 2 3),(11 22 33),(3 4 5))",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.MultiPointM).asWkt(),
            "MultiPoint M ((1 2 3),(11 22 33),(3 4 5))",
        )
        # strict dimension check
        self.assertEqual(
            point_collection.extractPartsByType(
                Qgis.WkbType.MultiPointM, useFlatType=False
            ).asWkt(),
            "MultiPoint M ((1 2 3),(11 22 33),(3 4 5))",
        )
        self.assertEqual(
            point_collection.extractPartsByType(
                Qgis.WkbType.MultiPointZM, useFlatType=False
            ).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.LineString).asWkt(),
            "MultiLineString EMPTY",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.Polygon).asWkt(),
            "MultiPolygon EMPTY",
        )

        # with z and m
        point_collection = QgsGeometryCollection()
        point_collection.addGeometry(QgsPoint(1, 2, 3, 4))
        point_collection.addGeometry(QgsPoint(11, 22, 33, 44))
        point_collection.addGeometry(QgsPoint(3, 4, 5, 55))
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.Point).asWkt(),
            "MultiPoint ZM ((1 2 3 4),(11 22 33 44),(3 4 5 55))",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.MultiPoint).asWkt(),
            "MultiPoint ZM ((1 2 3 4),(11 22 33 44),(3 4 5 55))",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.MultiPointM).asWkt(),
            "MultiPoint ZM ((1 2 3 4),(11 22 33 44),(3 4 5 55))",
        )
        # strict dimension check
        self.assertEqual(
            point_collection.extractPartsByType(
                Qgis.WkbType.MultiPointZM, useFlatType=False
            ).asWkt(),
            "MultiPoint ZM ((1 2 3 4),(11 22 33 44),(3 4 5 55))",
        )
        self.assertEqual(
            point_collection.extractPartsByType(
                Qgis.WkbType.MultiPointZ, useFlatType=False
            ).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            point_collection.extractPartsByType(
                Qgis.WkbType.MultiPointM, useFlatType=False
            ).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.LineString).asWkt(),
            "MultiLineString EMPTY",
        )
        self.assertEqual(
            point_collection.extractPartsByType(Qgis.WkbType.Polygon).asWkt(),
            "MultiPolygon EMPTY",
        )

        line_collection = QgsGeometryCollection()
        line_collection.addGeometry(QgsLineString([[1, 2], [3, 4]]))
        line_collection.addGeometry(QgsLineString([[11, 22], [33, 44]]))
        self.assertEqual(
            line_collection.extractPartsByType(Qgis.WkbType.LineString).asWkt(),
            "MultiLineString ((1 2, 3 4),(11 22, 33 44))",
        )
        self.assertEqual(
            line_collection.extractPartsByType(Qgis.WkbType.MultiLineString).asWkt(),
            "MultiLineString ((1 2, 3 4),(11 22, 33 44))",
        )
        self.assertEqual(
            line_collection.extractPartsByType(Qgis.WkbType.MultiLineStringZ).asWkt(),
            "MultiLineString ((1 2, 3 4),(11 22, 33 44))",
        )
        # strict dimension check
        self.assertEqual(
            line_collection.extractPartsByType(
                Qgis.WkbType.MultiLineStringZ, useFlatType=False
            ).asWkt(),
            "MultiLineString EMPTY",
        )
        self.assertEqual(
            line_collection.extractPartsByType(Qgis.WkbType.Point).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            line_collection.extractPartsByType(Qgis.WkbType.Polygon).asWkt(),
            "MultiPolygon EMPTY",
        )

        polygon_collection = QgsGeometryCollection()
        polygon_collection.addGeometry(
            QgsPolygon(QgsLineString([[1, 2], [3, 4], [1, 4], [1, 2]]))
        )
        polygon_collection.addGeometry(
            QgsPolygon(QgsLineString([[11, 22], [13, 14], [11, 14], [11, 22]]))
        )
        self.assertEqual(
            polygon_collection.extractPartsByType(Qgis.WkbType.Polygon).asWkt(),
            "MultiPolygon (((1 2, 3 4, 1 4, 1 2)),((11 22, 13 14, 11 14, 11 22)))",
        )
        self.assertEqual(
            polygon_collection.extractPartsByType(Qgis.WkbType.MultiPolygon).asWkt(),
            "MultiPolygon (((1 2, 3 4, 1 4, 1 2)),((11 22, 13 14, 11 14, 11 22)))",
        )
        self.assertEqual(
            polygon_collection.extractPartsByType(Qgis.WkbType.MultiPolygonZ).asWkt(),
            "MultiPolygon (((1 2, 3 4, 1 4, 1 2)),((11 22, 13 14, 11 14, 11 22)))",
        )
        # strict dimension check
        self.assertEqual(
            polygon_collection.extractPartsByType(
                Qgis.WkbType.MultiPolygonZ, useFlatType=False
            ).asWkt(),
            "MultiPolygon EMPTY",
        )
        self.assertEqual(
            polygon_collection.extractPartsByType(Qgis.WkbType.Point).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            polygon_collection.extractPartsByType(Qgis.WkbType.LineString).asWkt(),
            "MultiLineString EMPTY",
        )

        mixed_collection = QgsGeometryCollection()
        mixed_collection.addGeometry(
            QgsPolygon(QgsLineString([[1, 2], [3, 4], [1, 4], [1, 2]]))
        )
        mixed_collection.addGeometry(
            QgsPolygon(QgsLineString([[11, 22], [13, 14], [11, 14], [11, 22]]))
        )
        mixed_collection.addGeometry(QgsLineString([[1, 2], [3, 4]]))
        mixed_collection.addGeometry(QgsLineString([(11, 22), (33, 44)]))
        mixed_collection.addGeometry(QgsPoint(1, 2, 3))
        mixed_collection.addGeometry(QgsPoint(11, 22, 33))
        mixed_collection.addGeometry(QgsPoint(3, 4, 5))

        self.assertEqual(
            mixed_collection.extractPartsByType(Qgis.WkbType.Polygon).asWkt(),
            "MultiPolygon (((1 2, 3 4, 1 4, 1 2)),((11 22, 13 14, 11 14, 11 22)))",
        )
        self.assertEqual(
            mixed_collection.extractPartsByType(Qgis.WkbType.MultiPolygon).asWkt(),
            "MultiPolygon (((1 2, 3 4, 1 4, 1 2)),((11 22, 13 14, 11 14, 11 22)))",
        )
        self.assertEqual(
            mixed_collection.extractPartsByType(Qgis.WkbType.MultiPolygonZ).asWkt(),
            "MultiPolygon (((1 2, 3 4, 1 4, 1 2)),((11 22, 13 14, 11 14, 11 22)))",
        )
        # strict dimension check
        self.assertEqual(
            mixed_collection.extractPartsByType(
                Qgis.WkbType.MultiPolygonZ, useFlatType=False
            ).asWkt(),
            "MultiPolygon EMPTY",
        )
        self.assertEqual(
            mixed_collection.extractPartsByType(Qgis.WkbType.Point).asWkt(),
            "MultiPoint Z ((1 2 3),(11 22 33),(3 4 5))",
        )
        self.assertEqual(
            mixed_collection.extractPartsByType(Qgis.WkbType.LineString).asWkt(),
            "MultiLineString ((1 2, 3 4),(11 22, 33 44))",
        )

        # shortcuts
        mp = QgsMultiPoint()
        mp.addGeometry(QgsPoint(1, 2, 3))
        mp.addGeometry(QgsPoint(11, 22, 33))
        self.assertEqual(
            mp.extractPartsByType(Qgis.WkbType.Point).asWkt(),
            "MultiPoint Z ((1 2 3),(11 22 33))",
        )
        self.assertEqual(
            mp.extractPartsByType(Qgis.WkbType.Point, useFlatType=False).asWkt(),
            "MultiPoint EMPTY",
        )
        self.assertEqual(
            mp.extractPartsByType(Qgis.WkbType.LineString).asWkt(),
            "MultiLineString EMPTY",
        )

        ml = QgsMultiLineString()
        ml.addGeometry(QgsLineString([[1, 2, 3], [3, 4, 5]]))
        ml.addGeometry(QgsLineString([[11, 22, 6], [33, 44, 7]]))
        self.assertEqual(
            ml.extractPartsByType(Qgis.WkbType.LineString).asWkt(),
            "MultiLineString Z ((1 2 3, 3 4 5),(11 22 6, 33 44 7))",
        )
        self.assertEqual(
            ml.extractPartsByType(Qgis.WkbType.LineString, useFlatType=False).asWkt(),
            "MultiLineString EMPTY",
        )
        self.assertEqual(
            ml.extractPartsByType(Qgis.WkbType.Point).asWkt(), "MultiPoint EMPTY"
        )

        mp = QgsMultiPolygon()
        mp.addGeometry(
            QgsPolygon(QgsLineString([[1, 2, 3], [3, 4, 3], [1, 4, 3], [1, 2, 3]]))
        )
        mp.addGeometry(
            QgsPolygon(
                QgsLineString([[11, 22, 33], [13, 14, 33], [11, 14, 33], [11, 22, 33]])
            )
        )
        self.assertEqual(
            mp.extractPartsByType(Qgis.WkbType.Polygon).asWkt(),
            "MultiPolygon Z (((1 2 3, 3 4 3, 1 4 3, 1 2 3)),((11 22 33, 13 14 33, 11 14 33, 11 22 33)))",
        )
        self.assertEqual(
            mp.extractPartsByType(Qgis.WkbType.Polygon, useFlatType=False).asWkt(),
            "MultiPolygon EMPTY",
        )
        self.assertEqual(
            mp.extractPartsByType(Qgis.WkbType.Point).asWkt(), "MultiPoint EMPTY"
        )

    def test_take_geometries(self):
        """
        Test taking geometries
        """
        # empty collection
        collection = QgsGeometryCollection()
        self.assertFalse(collection.takeGeometries())
        self.assertEqual(collection.asWkt(), "GeometryCollection EMPTY")

        collection.addGeometry(
            QgsPolygon(QgsLineString([[1, 2, 3], [3, 4, 3], [1, 4, 3], [1, 2, 3]]))
        )
        collection.addGeometry(
            QgsPolygon(
                QgsLineString([[11, 22, 33], [13, 14, 33], [11, 14, 33], [11, 22, 33]])
            )
        )
        self.assertEqual(collection.boundingBox(), QgsRectangle(1, 2, 13, 22))

        geometries = collection.takeGeometries()
        # should be nothing left
        self.assertEqual(collection.asWkt(), "GeometryCollection EMPTY")
        self.assertEqual(collection.boundingBox(), QgsRectangle())

        del collection

        self.assertEqual(
            [p.asWkt() for p in geometries],
            [
                "Polygon Z ((1 2 3, 3 4 3, 1 4 3, 1 2 3))",
                "Polygon Z ((11 22 33, 13 14 33, 11 14 33, 11 22 33))",
            ],
        )

    def test_add_geometries(self):
        """
        Test adding multiple geometries
        """
        # empty collection
        collection = QgsGeometryCollection()
        self.assertTrue(collection.addGeometries([]))
        self.assertEqual(collection.asWkt(), "GeometryCollection EMPTY")
        self.assertEqual(collection.boundingBox(), QgsRectangle())

        self.assertTrue(
            collection.addGeometries(
                [
                    QgsLineString([[1, 2, 3], [3, 4, 3], [1, 4, 3], [1, 2, 3]]),
                    QgsLineString(
                        [[11, 22, 33], [13, 14, 33], [11, 14, 33], [11, 22, 33]]
                    ),
                ]
            )
        )
        self.assertEqual(
            collection.asWkt(),
            "GeometryCollection (LineString Z (1 2 3, 3 4 3, 1 4 3, 1 2 3),LineString Z (11 22 33, 13 14 33, 11 14 33, 11 22 33))",
        )
        self.assertEqual(collection.boundingBox(), QgsRectangle(1, 2, 13, 22))
        self.assertTrue(collection.addGeometries([QgsPoint(100, 200)]))
        self.assertEqual(
            collection.asWkt(),
            "GeometryCollection (LineString Z (1 2 3, 3 4 3, 1 4 3, 1 2 3),LineString Z (11 22 33, 13 14 33, 11 14 33, 11 22 33),Point (100 200))",
        )
        self.assertEqual(collection.boundingBox(), QgsRectangle(1, 2, 100, 200))

    def test_simplify_by_distance(self):
        """
        test simplifyByDistance
        """
        p = QgsGeometryCollection()
        p.fromWkt(
            "GeometryCollection( LineString(0 0, 50 0, 70 0, 80 0, 100 0), LineString(0 0, 50 1, 60 1, 100 0) )"
        )
        self.assertEqual(
            p.simplifyByDistance(10).asWkt(),
            "GeometryCollection (LineString (0 0, 100 0),LineString (0 0, 100 0))",
        )


if __name__ == "__main__":
    unittest.main()
