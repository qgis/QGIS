"""QGIS Unit tests for QgsMultiPolygon.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Loïc Bartoletti"
__date__ = "19/12/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import qgis  # NOQA

from qgis.core import QgsMultiPolygon, QgsPolygon, QgsLineString, QgsPoint, QgsRectangle
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsMultiPolygon(QgisTestCase):

    def test_constructor(self):
        p = QgsMultiPolygon([])
        self.assertTrue(p.isEmpty())

        value = QgsPolygon(QgsLineString([[1, 2], [10, 2], [10, 10], [1, 2]]))
        p = QgsMultiPolygon([value])
        self.assertEqual(p.asWkt(), "MultiPolygon (((1 2, 10 2, 10 10, 1 2)))")
        # constructor should have made internal copy
        del value
        self.assertEqual(p.asWkt(), "MultiPolygon (((1 2, 10 2, 10 10, 1 2)))")

        p = QgsMultiPolygon(
            [
                QgsPolygon(QgsLineString([[1, 2], [10, 2], [10, 10], [1, 2]])),
                QgsPolygon(QgsLineString([[100, 2], [110, 2], [110, 10], [100, 2]])),
            ]
        )
        self.assertEqual(
            p.asWkt(),
            "MultiPolygon (((1 2, 10 2, 10 10, 1 2)),((100 2, 110 2, 110 10, 100 2)))",
        )

        # with z
        p = QgsMultiPolygon(
            [
                QgsPolygon(
                    QgsLineString([[1, 2, 3], [10, 2, 3], [10, 10, 3], [1, 2, 3]])
                ),
                QgsPolygon(
                    QgsLineString([[100, 2, 4], [110, 2, 4], [110, 10, 4], [100, 2, 4]])
                ),
            ]
        )
        self.assertEqual(
            p.asWkt(),
            "MultiPolygon Z (((1 2 3, 10 2 3, 10 10 3, 1 2 3)),((100 2 4, 110 2 4, 110 10 4, 100 2 4)))",
        )

        # with zm
        p = QgsMultiPolygon(
            [
                QgsPolygon(
                    QgsLineString(
                        [[1, 2, 3, 5], [10, 2, 3, 5], [10, 10, 3, 5], [1, 2, 3, 5]]
                    )
                ),
                QgsPolygon(
                    QgsLineString(
                        [
                            [100, 2, 4, 6],
                            [110, 2, 4, 6],
                            [110, 10, 4, 6],
                            [100, 2, 4, 6],
                        ]
                    )
                ),
            ]
        )
        self.assertEqual(
            p.asWkt(),
            "MultiPolygon ZM (((1 2 3 5, 10 2 3 5, 10 10 3 5, 1 2 3 5)),((100 2 4 6, 110 2 4 6, 110 10 4 6, 100 2 4 6)))",
        )

    def testFuzzyComparisons(self):
        ######
        # 2D #
        ######
        epsilon = 0.001
        geom1 = QgsMultiPolygon()
        geom2 = QgsMultiPolygon()

        p1 = QgsPolygon()
        p1.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(5.0, 5.0),
                    QgsPoint(6.0, 5.0),
                    QgsPoint(6.0, 6.0),
                    QgsPoint(5.0, 5.0),
                ]
            )
        )
        p2 = QgsPolygon()
        p2.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0),
                    QgsPoint(0.001, 0.001),
                    QgsPoint(0.003, 0.003),
                    QgsPoint(0.0, 0.0),
                ]
            )
        )
        self.assertTrue(geom1.addGeometry(p1))
        self.assertTrue(geom1.addGeometry(p2))

        p1 = QgsPolygon()
        p1.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(5.0, 5.0),
                    QgsPoint(6.0, 5.0),
                    QgsPoint(6.0, 6.0),
                    QgsPoint(5.0, 5.0),
                ]
            )
        )
        p2 = QgsPolygon()
        p2.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0),
                    QgsPoint(0.002, 0.002),
                    QgsPoint(0.003, 0.003),
                    QgsPoint(0.0, 0.0),
                ]
            )
        )
        self.assertTrue(geom2.addGeometry(p1))
        self.assertTrue(geom2.addGeometry(p2))

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
        geom1 = QgsMultiPolygon()
        geom2 = QgsMultiPolygon()

        p1 = QgsPolygon()
        p1.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(5.0, 5.0, 5.0),
                    QgsPoint(6.0, 5.0, 5.0),
                    QgsPoint(6.0, 6.0, 5.0),
                    QgsPoint(5.0, 5.0, 5.0),
                ]
            )
        )
        p2 = QgsPolygon()
        p2.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0, 0.0),
                    QgsPoint(0.001, 0.001, 0.001),
                    QgsPoint(0.003, 0.003, 0.003),
                    QgsPoint(0.0, 0.0, 0.0),
                ]
            )
        )
        self.assertTrue(geom1.addGeometry(p1))
        self.assertTrue(geom1.addGeometry(p2))

        p1 = QgsPolygon()
        p1.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(5.0, 5.0, 5.0),
                    QgsPoint(6.0, 5.0, 5.0),
                    QgsPoint(6.0, 6.0, 5.0),
                    QgsPoint(5.0, 5.0, 5.0),
                ]
            )
        )
        p2 = QgsPolygon()
        p2.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0, 0.0),
                    QgsPoint(0.001, 0.001, 0.002),
                    QgsPoint(0.003, 0.003, 0.003),
                    QgsPoint(0.0, 0.0, 0.0),
                ]
            )
        )
        self.assertTrue(geom2.addGeometry(p1))
        self.assertTrue(geom2.addGeometry(p2))

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
        geom1 = QgsMultiPolygon()
        geom2 = QgsMultiPolygon()

        p1 = QgsPolygon()
        p1.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(5.0, 5.0, m=5.0),
                    QgsPoint(6.0, 5.0, m=5.0),
                    QgsPoint(6.0, 6.0, m=5.0),
                    QgsPoint(5.0, 5.0, m=5.0),
                ]
            )
        )
        p2 = QgsPolygon()
        p2.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0, m=0.0),
                    QgsPoint(0.001, 0.001, m=0.001),
                    QgsPoint(0.003, 0.003, m=0.003),
                    QgsPoint(0.0, 0.0, m=0.0),
                ]
            )
        )
        self.assertTrue(geom1.addGeometry(p1))
        self.assertTrue(geom1.addGeometry(p2))

        p1 = QgsPolygon()
        p1.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(5.0, 5.0, m=5.0),
                    QgsPoint(6.0, 5.0, m=5.0),
                    QgsPoint(6.0, 6.0, m=5.0),
                    QgsPoint(5.0, 5.0, m=5.0),
                ]
            )
        )
        p2 = QgsPolygon()
        p2.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0, m=0.0),
                    QgsPoint(0.001, 0.001, m=0.002),
                    QgsPoint(0.003, 0.003, m=0.003),
                    QgsPoint(0.0, 0.0, m=0.0),
                ]
            )
        )
        self.assertTrue(geom2.addGeometry(p1))
        self.assertTrue(geom2.addGeometry(p2))

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
        geom1 = QgsMultiPolygon()
        geom2 = QgsMultiPolygon()

        p1 = QgsPolygon()
        p1.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(5.0, 5.0, 5.0, 5.0),
                    QgsPoint(6.0, 5.0, 5.0, 5.0),
                    QgsPoint(6.0, 6.0, 5.0, 5.0),
                    QgsPoint(5.0, 5.0, 5.0, 5.0),
                ]
            )
        )
        p2 = QgsPolygon()
        p2.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0, 0.0, 0.0),
                    QgsPoint(0.001, 0.001, 0.001, 0.001),
                    QgsPoint(0.003, 0.003, 0.003, 0.003),
                    QgsPoint(0.0, 0.0, 0.0, 0.0),
                ]
            )
        )
        self.assertTrue(geom1.addGeometry(p1))
        self.assertTrue(geom1.addGeometry(p2))

        p1 = QgsPolygon()
        p1.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(5.0, 5.0, 5.0, 5.0),
                    QgsPoint(6.0, 5.0, 5.0, 5.0),
                    QgsPoint(6.0, 6.0, 5.0, 5.0),
                    QgsPoint(5.0, 5.0, 5.0, 5.0),
                ]
            )
        )
        p2 = QgsPolygon()
        p2.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0, 0.0, 0.0),
                    QgsPoint(0.001, 0.001, 0.002, 0.002),
                    QgsPoint(0.003, 0.003, 0.003, 0.003),
                    QgsPoint(0.0, 0.0, 0.0, 0.0),
                ]
            )
        )
        self.assertTrue(geom2.addGeometry(p1))
        self.assertTrue(geom2.addGeometry(p2))

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
        collection = QgsMultiPolygon()
        self.assertTrue(collection.addGeometries([]))
        self.assertEqual(collection.asWkt(), "MultiPolygon EMPTY")
        self.assertEqual(collection.boundingBox(), QgsRectangle())

        self.assertTrue(
            collection.addGeometries(
                [
                    QgsPolygon(
                        QgsLineString([[1, 2, 3], [3, 4, 3], [1, 4, 3], [1, 2, 3]])
                    ),
                    QgsPolygon(
                        QgsLineString(
                            [[11, 22, 33], [13, 14, 33], [11, 14, 33], [11, 22, 33]]
                        )
                    ),
                ]
            )
        )
        self.assertEqual(
            collection.asWkt(),
            "MultiPolygon Z (((1 2 3, 3 4 3, 1 4 3, 1 2 3)),((11 22 33, 13 14 33, 11 14 33, 11 22 33)))",
        )
        self.assertEqual(collection.boundingBox(), QgsRectangle(1, 2, 13, 22))

        # can't add non-polygons
        self.assertFalse(collection.addGeometries([QgsPoint(100, 200)]))
        self.assertEqual(
            collection.asWkt(),
            "MultiPolygon Z (((1 2 3, 3 4 3, 1 4 3, 1 2 3)),((11 22 33, 13 14 33, 11 14 33, 11 22 33)))",
        )
        self.assertEqual(collection.boundingBox(), QgsRectangle(1, 2, 13, 22))

        self.assertTrue(
            collection.addGeometries(
                [
                    QgsPolygon(
                        QgsLineString(
                            [[100, 2, 3], [300, 4, 3], [300, 100, 3], [100, 2, 3]]
                        )
                    )
                ]
            )
        )
        self.assertEqual(
            collection.asWkt(),
            "MultiPolygon Z (((1 2 3, 3 4 3, 1 4 3, 1 2 3)),((11 22 33, 13 14 33, 11 14 33, 11 22 33)),((100 2 3, 300 4 3, 300 100 3, 100 2 3)))",
        )
        self.assertEqual(collection.boundingBox(), QgsRectangle(1, 2, 300, 100))

    def test_simplify_by_distance(self):
        """
        test simplifyByDistance
        """
        p = QgsMultiPolygon()
        p.fromWkt(
            "MultiPolygon (((4.33843532846714908 1.48149845255473167, 4.47478429197079919 8.6398190364963412, 5.8382739270072932 16.47988443795619418, 10.61048764963503288 22.88828572262772809, 17.63245927007298519 27.72867392700729283, 27.04053775182481445 30.11478078832115912, 34.81242867153284237 28.34224426277371478, 42.51614510948904524 23.97907743065692188, 44.83407748905109713 17.84337407299268818, 44.15233267153284658 9.52608729927005982, 42.44797062773722018 1.75419637956203189, 37.26671001459853727 -4.65420490510949492, 29.5629935766423344 -6.63126487591242153, 18.51872753284671091 -7.31300969343067209, 7.1335890802919657 -5.13142627737227031, 5.15652910948904619 -1.9272256350365069, 4.33843532846714908 1.48149845255473167),(20.31173353218648003 19.78274965689762155, 17.28447821560356346 9.99697084282726678, 21.22695025580456729 4.57607178755088739, 26.01423773319150001 3.23844734533983569, 28.33748018545281155 3.87205892322928236, 32.20955093922164991 5.6320910840332985, 34.60319467791511983 8.37774125488756738, 35.23680625580456649 12.24981200865641284, 34.6735959643472782 15.84027761669661771, 32.13914965278949154 19.43074322473681548, 26.92945445680959438 22.03559082272676761, 22.98698241660859054 21.04997281267651488, 20.31173353218648003 19.78274965689762155)),((55.16037031610606789 15.48827118453581164, 57.8356192005281855 18.16352006895792215, 64.10133369299049377 20.27555866192274436, 70.71905461761360812 18.86753293327952719, 74.09831636635732366 16.4034879081538989, 75.71754595429702306 11.33459528503832558, 74.30952022565381299 6.47690652121922739, 69.38143017540255642 2.6752370538825474, 61.63728866786486549 1.90082290312877689, 56.4979947583171338 2.60483576745038192, 53.11873300957341826 6.82891295338003346, 52.83712786384477056 12.32021329508857832, 55.16037031610606789 15.48827118453581164)))"
        )
        self.assertEqual(
            p.simplifyByDistance(1).asWkt(3),
            "MultiPolygon (((4.338 1.481, 5.838 16.48, 10.61 22.888, 17.632 27.729, 27.041 30.115, 34.812 28.342, 42.516 23.979, 44.834 17.843, 44.152 9.526, 42.448 1.754, 37.267 -4.654, 18.519 -7.313, 7.134 -5.131, 4.338 1.481),(20.312 19.783, 17.284 9.997, 21.227 4.576, 26.014 3.238, 32.21 5.632, 34.603 8.378, 35.237 12.25, 32.139 19.431, 26.929 22.036, 20.312 19.783)),((57.836 18.164, 64.101 20.276, 70.719 18.868, 74.098 16.403, 75.718 11.335, 74.31 6.477, 69.381 2.675, 56.498 2.605, 53.119 6.829, 52.837 12.32, 57.836 18.164)))",
        )
        self.assertEqual(
            p.simplifyByDistance(2).asWkt(3),
            "MultiPolygon (((4.338 1.481, 5.838 16.48, 17.632 27.729, 27.041 30.115, 42.516 23.979, 44.152 9.526, 37.267 -4.654, 18.519 -7.313, 7.134 -5.131, 4.338 1.481),(20.312 19.783, 17.284 9.997, 21.227 4.576, 32.21 5.632, 35.237 12.25, 32.139 19.431, 26.929 22.036, 20.312 19.783)),((55.16 15.488, 64.101 20.276, 70.719 18.868, 75.718 11.335, 74.31 6.477, 69.381 2.675, 56.498 2.605, 53.119 6.829, 55.16 15.488)))",
        )


if __name__ == "__main__":
    unittest.main()
