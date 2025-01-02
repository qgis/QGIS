"""QGIS Unit tests for QgsMultiLineString.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Loïc Bartoletti"
__date__ = "12/12/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import qgis  # NOQA

from qgis.core import QgsMultiLineString, QgsLineString, QgsPoint, QgsRectangle
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsMultiLineString(QgisTestCase):

    def testConstruct(self):
        # With list
        line = QgsLineString([QgsPoint(1, 2), QgsPoint(3, 4)])
        multiline = QgsMultiLineString()
        multiline.addGeometry(line)
        self.assertEqual(multiline.numGeometries(), 1)
        line = multiline.geometryN(0)
        self.assertEqual(line.startPoint(), QgsPoint(1, 2))
        self.assertEqual(line.endPoint(), QgsPoint(3, 4))

        p = QgsMultiLineString([])
        self.assertTrue(p.isEmpty())

        value = QgsLineString([[1, 2], [10, 2], [10, 10]])
        p = QgsMultiLineString([value])
        self.assertEqual(p.asWkt(), "MultiLineString ((1 2, 10 2, 10 10))")
        # constructor should have made internal copy
        del value
        self.assertEqual(p.asWkt(), "MultiLineString ((1 2, 10 2, 10 10))")

        p = QgsMultiLineString(
            [
                QgsLineString([[1, 2], [10, 2], [10, 10], [1, 2]]),
                QgsLineString([[100, 2], [110, 2], [110, 10], [100, 2]]),
            ]
        )
        self.assertEqual(
            p.asWkt(),
            "MultiLineString ((1 2, 10 2, 10 10, 1 2),(100 2, 110 2, 110 10, 100 2))",
        )

        # with z
        p = QgsMultiLineString(
            [
                QgsLineString([[1, 2, 3], [10, 2, 3], [10, 10, 3], [1, 2, 3]]),
                QgsLineString([[100, 2, 4], [110, 2, 4], [110, 10, 4], [100, 2, 4]]),
            ]
        )
        self.assertEqual(
            p.asWkt(),
            "MultiLineString Z ((1 2 3, 10 2 3, 10 10 3, 1 2 3),(100 2 4, 110 2 4, 110 10 4, 100 2 4))",
        )

        # with zm
        p = QgsMultiLineString(
            [
                QgsLineString(
                    [[1, 2, 3, 5], [10, 2, 3, 5], [10, 10, 3, 5], [1, 2, 3, 5]]
                ),
                QgsLineString(
                    [[100, 2, 4, 6], [110, 2, 4, 6], [110, 10, 4, 6], [100, 2, 4, 6]]
                ),
            ]
        )
        self.assertEqual(
            p.asWkt(),
            "MultiLineString ZM ((1 2 3 5, 10 2 3 5, 10 10 3 5, 1 2 3 5),(100 2 4 6, 110 2 4 6, 110 10 4 6, 100 2 4 6))",
        )

    def testMeasureLine(self):
        multiline = QgsMultiLineString()
        m_line = multiline.measuredLine(10, 20)
        self.assertEqual(m_line.asWkt(0), "MultiLineString M EMPTY")

        multiline.addGeometry(QgsLineString([[0, 0], [2, 0], [4, 0]]))
        m_line = multiline.measuredLine(10, 20)
        self.assertEqual(
            m_line.geometryN(0),
            QgsLineString(
                [QgsPoint(0, 0, m=10), QgsPoint(2, 0, m=15), QgsPoint(4, 0, m=20)]
            ),
        )

        multiline = QgsMultiLineString()
        multiline.addGeometry(QgsLineString([[0, 0], [9, 0], [10, 0]]))
        m_line = multiline.measuredLine(10, 20)
        self.assertEqual(
            m_line.geometryN(0),
            QgsLineString(
                [QgsPoint(0, 0, m=10), QgsPoint(9, 0, m=19), QgsPoint(10, 0, m=20)]
            ),
        )

        multiline = QgsMultiLineString()
        multiline.addGeometry(QgsLineString([[1, 0], [3, 0], [4, 0]]))
        multiline.addGeometry(QgsLineString([[0, 0], [9, 0], [10, 0]]))
        m_line = multiline.measuredLine(10, 20)
        self.assertEqual(m_line.numGeometries(), 2)
        self.assertEqual(
            m_line.asWkt(0),
            "MultiLineString M ((1 0 10, 3 0 12, 4 0 12),(0 0 12, 9 0 19, 10 0 20))",
        )

        multiline = QgsMultiLineString()
        multiline.addGeometry(QgsLineString([[1, 0], [1, 0], [1, 0]]))
        multiline.addGeometry(QgsLineString([[2, 2], [2, 2], [2, 2]]))
        m_line = multiline.measuredLine(10, 20)
        self.assertEqual(m_line.numGeometries(), 2)
        self.assertEqual(
            m_line.asWkt(0),
            "MultiLineString M ((1 0 nan, 1 0 nan, 1 0 nan),(2 2 nan, 2 2 nan, 2 2 nan))",
        )

    def testFuzzyComparisons(self):
        ######
        # 2D #
        ######
        epsilon = 0.001
        geom1 = QgsMultiLineString()
        line1 = QgsLineString(QgsPoint(5.0, 6.0), QgsPoint(8.0, 7.0))
        line2 = QgsLineString(QgsPoint(0.0, 0.0), QgsPoint(0.001, 0.001))
        geom1.addGeometry(line1)
        geom1.addGeometry(line2)

        geom2 = QgsMultiLineString()
        line1 = QgsLineString(QgsPoint(5.0, 6.0), QgsPoint(8.0, 7.0))
        line2 = QgsLineString(QgsPoint(0.0, 0.0), QgsPoint(0.002, 0.002))
        geom2.addGeometry(line1)
        geom2.addGeometry(line2)

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
        geom1 = QgsMultiLineString()
        line1 = QgsLineString(QgsPoint(4.0, 5.0, 6.0), QgsPoint(9.0, 8.0, 7.0))
        line2 = QgsLineString(QgsPoint(0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001))
        geom1.addGeometry(line1)
        geom1.addGeometry(line2)

        geom2 = QgsMultiLineString()
        line1 = QgsLineString(QgsPoint(4.0, 5.0, 6.0), QgsPoint(9.0, 8.0, 7.0))
        line2 = QgsLineString(QgsPoint(0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.002))
        geom2.addGeometry(line1)
        geom2.addGeometry(line2)

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
        geom1 = QgsMultiLineString()
        line1 = QgsLineString(QgsPoint(4.0, 5.0, m=6.0), QgsPoint(9.0, 8.0, m=7.0))
        line2 = QgsLineString(
            QgsPoint(0.0, 0.0, m=0.0), QgsPoint(0.001, 0.001, m=0.001)
        )
        geom1.addGeometry(line1)
        geom1.addGeometry(line2)

        geom2 = QgsMultiLineString()
        line1 = QgsLineString(QgsPoint(4.0, 5.0, m=6.0), QgsPoint(9.0, 8.0, m=7.0))
        line2 = QgsLineString(
            QgsPoint(0.0, 0.0, m=0.0), QgsPoint(0.001, 0.001, m=0.002)
        )
        geom2.addGeometry(line1)
        geom2.addGeometry(line2)

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
        geom1 = QgsMultiLineString()
        line1 = QgsLineString(
            QgsPoint(3.0, 4.0, 5.0, 6.0), QgsPoint(10.0, 9.0, 8.0, 7.0)
        )
        line2 = QgsLineString(
            QgsPoint(0.0, 0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001, 0.001)
        )
        geom1.addGeometry(line1)
        geom1.addGeometry(line2)

        geom2 = QgsMultiLineString()
        line1 = QgsLineString(
            QgsPoint(3.0, 4.0, 5.0, 6.0), QgsPoint(10.0, 9.0, 8.0, 7.0)
        )
        line2 = QgsLineString(
            QgsPoint(0.0, 0.0, 0.0, 0.0), QgsPoint(0.001, 0.001, 0.001, 0.002)
        )
        geom2.addGeometry(line1)
        geom2.addGeometry(line2)

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

    def test_add_geometries(self):
        """
        Test adding multiple geometries
        """
        # empty collection
        collection = QgsMultiLineString()
        self.assertTrue(collection.addGeometries([]))
        self.assertEqual(collection.asWkt(), "MultiLineString EMPTY")
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
            "MultiLineString Z ((1 2 3, 3 4 3, 1 4 3, 1 2 3),(11 22 33, 13 14 33, 11 14 33, 11 22 33))",
        )
        self.assertEqual(collection.boundingBox(), QgsRectangle(1, 2, 13, 22))

        # can't add non-linestrings
        self.assertFalse(collection.addGeometries([QgsPoint(100, 200)]))
        self.assertEqual(
            collection.asWkt(),
            "MultiLineString Z ((1 2 3, 3 4 3, 1 4 3, 1 2 3),(11 22 33, 13 14 33, 11 14 33, 11 22 33))",
        )
        self.assertEqual(collection.boundingBox(), QgsRectangle(1, 2, 13, 22))

        self.assertTrue(
            collection.addGeometries([QgsLineString([[100, 2, 3], [300, 4, 3]])])
        )
        self.assertEqual(
            collection.asWkt(),
            "MultiLineString Z ((1 2 3, 3 4 3, 1 4 3, 1 2 3),(11 22 33, 13 14 33, 11 14 33, 11 22 33),(100 2 3, 300 4 3))",
        )
        self.assertEqual(collection.boundingBox(), QgsRectangle(1, 2, 300, 22))

    def test_simplify_by_distance(self):
        """
        test simplifyByDistance
        """
        p = QgsMultiLineString()
        p.fromWkt(
            "MultiLineString( (0 0, 50 0, 70 0, 80 0, 100 0), (0 0, 50 1, 60 1, 100 0) )"
        )
        self.assertEqual(
            p.simplifyByDistance(10).asWkt(),
            "MultiLineString ((0 0, 100 0),(0 0, 100 0))",
        )


if __name__ == "__main__":
    unittest.main()
