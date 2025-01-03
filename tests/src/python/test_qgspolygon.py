"""QGIS Unit tests for QgsPolygon.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Loïc Bartoletti"
__date__ = "19/12/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import qgis  # NOQA

from qgis.core import QgsGeometry, QgsPolygon, QgsLineString, QgsPoint
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsPolygon(QgisTestCase):

    def testFuzzyComparisons(self):
        ######
        # 2D #
        ######
        epsilon = 0.001
        geom1 = QgsPolygon()
        geom1.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0),
                    QgsPoint(0.001, 0.001),
                    QgsPoint(0.003, 0.003),
                    QgsPoint(0.0, 0.0),
                ]
            )
        )
        geom2 = QgsPolygon()
        geom2.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0),
                    QgsPoint(0.002, 0.002),
                    QgsPoint(0.003, 0.003),
                    QgsPoint(0.0, 0.0),
                ]
            )
        )

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
        geom1 = QgsPolygon()
        geom1.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0, 0.0),
                    QgsPoint(0.001, 0.001, 0.001),
                    QgsPoint(0.003, 0.003, 0.003),
                    QgsPoint(0.0, 0.0, 0.0),
                ]
            )
        )
        geom2 = QgsPolygon()
        geom2.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0, 0.0),
                    QgsPoint(0.001, 0.001, 0.002),
                    QgsPoint(0.003, 0.003, 0.003),
                    QgsPoint(0.0, 0.0, 0.0),
                ]
            )
        )

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
        geom1 = QgsPolygon()
        geom1.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0, m=0.0),
                    QgsPoint(0.001, 0.001, m=0.001),
                    QgsPoint(0.003, 0.003, m=0.003),
                    QgsPoint(0.0, 0.0, m=0.0),
                ]
            )
        )
        geom2 = QgsPolygon()
        geom2.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0, m=0.0),
                    QgsPoint(0.001, 0.001, m=0.002),
                    QgsPoint(0.003, 0.003, m=0.003),
                    QgsPoint(0.0, 0.0, m=0.0),
                ]
            )
        )

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
        geom1 = QgsPolygon()
        geom1.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0, 0.0, 0.0),
                    QgsPoint(0.001, 0.001, 0.001, 0.001),
                    QgsPoint(0.003, 0.003, 0.003, 0.003),
                    QgsPoint(0.0, 0.0, 0.0, 0.0),
                ]
            )
        )
        geom2 = QgsPolygon()
        geom2.setExteriorRing(
            QgsLineString(
                [
                    QgsPoint(0.0, 0.0, 0.0, 0.0),
                    QgsPoint(0.001, 0.001, 0.002, 0.002),
                    QgsPoint(0.003, 0.003, 0.003, 0.003),
                    QgsPoint(0.0, 0.0, 0.0, 0.0),
                ]
            )
        )

        self.assertNotEqual(geom1, geom2)  # epsilon = 1e-8 here

        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertFalse(geom1.fuzzyDistanceEqual(geom2, epsilon))

        # OK for both
        epsilon *= 10
        self.assertTrue(geom1.fuzzyEqual(geom2, epsilon))
        self.assertTrue(geom1.fuzzyDistanceEqual(geom2, epsilon))

    def test_simplify_by_distance(self):
        """
        test simplifyByDistance
        """
        p = QgsPolygon()
        p.fromWkt(
            "Polygon ((4.33843532846714908 1.48149845255473167, 4.47478429197079919 8.6398190364963412, 5.8382739270072932 16.47988443795619418, 10.61048764963503288 22.88828572262772809, 17.63245927007298519 27.72867392700729283, 27.04053775182481445 30.11478078832115912, 34.81242867153284237 28.34224426277371478, 42.51614510948904524 23.97907743065692188, 44.83407748905109713 17.84337407299268818, 44.15233267153284658 9.52608729927005982, 42.44797062773722018 1.75419637956203189, 37.26671001459853727 -4.65420490510949492, 29.5629935766423344 -6.63126487591242153, 18.51872753284671091 -7.31300969343067209, 7.1335890802919657 -5.13142627737227031, 5.15652910948904619 -1.9272256350365069, 4.33843532846714908 1.48149845255473167),(20.31173353218648003 19.78274965689762155, 17.28447821560356346 9.99697084282726678, 21.22695025580456729 4.57607178755088739, 26.01423773319150001 3.23844734533983569, 28.33748018545281155 3.87205892322928236, 32.20955093922164991 5.6320910840332985, 34.60319467791511983 8.37774125488756738, 35.23680625580456649 12.24981200865641284, 34.6735959643472782 15.84027761669661771, 32.13914965278949154 19.43074322473681548, 26.92945445680959438 22.03559082272676761, 22.98698241660859054 21.04997281267651488, 20.31173353218648003 19.78274965689762155))"
        )
        self.assertEqual(
            p.simplifyByDistance(1).asWkt(3),
            "Polygon ((4.338 1.481, 5.838 16.48, 10.61 22.888, 17.632 27.729, 27.041 30.115, 34.812 28.342, 42.516 23.979, 44.834 17.843, 44.152 9.526, 42.448 1.754, 37.267 -4.654, 18.519 -7.313, 7.134 -5.131, 4.338 1.481),(20.312 19.783, 17.284 9.997, 21.227 4.576, 26.014 3.238, 32.21 5.632, 34.603 8.378, 35.237 12.25, 32.139 19.431, 26.929 22.036, 20.312 19.783))",
        )
        self.assertEqual(
            p.simplifyByDistance(2).asWkt(3),
            "Polygon ((4.338 1.481, 5.838 16.48, 17.632 27.729, 27.041 30.115, 42.516 23.979, 44.152 9.526, 37.267 -4.654, 18.519 -7.313, 7.134 -5.131, 4.338 1.481),(20.312 19.783, 17.284 9.997, 21.227 4.576, 32.21 5.632, 35.237 12.25, 32.139 19.431, 26.929 22.036, 20.312 19.783))",
        )

        # ported GEOS tests
        p.fromWkt(
            "POLYGON ((20 220, 40 220, 60 220, 80 220, 100 220, 120 220, 140 220, 140 180, 100 180, 60 180, 20 180, 20 220))"
        )
        self.assertEqual(
            p.simplifyByDistance(10).asWkt(),
            "Polygon ((20 220, 140 220, 140 180, 20 180, 20 220))",
        )
        p.fromWkt(
            "POLYGON ((120 120, 121 121, 122 122, 220 120, 180 199, 160 200, 140 199, 120 120))"
        )
        self.assertEqual(
            p.simplifyByDistance(10).asWkt(),
            "Polygon ((120 120, 220 120, 180 199, 160 200, 140 199, 120 120))",
        )
        p.fromWkt(
            "POLYGON ((80 200, 240 200, 240 60, 80 60, 80 200), (120 120, 220 120, 180 199, 160 200, 140 199, 120 120))"
        )
        self.assertEqual(
            p.simplifyByDistance(10).asWkt(),
            "Polygon ((80 200, 240 200, 240 60, 80 60, 80 200),(120 120, 220 120, 180 199, 160 200, 140 199, 120 120))",
        )
        p.fromWkt("POLYGON ((1 0, 2 0, 2 2, 0 2, 0 0, 1 0))")
        self.assertEqual(
            p.simplifyByDistance(0).asWkt(), "Polygon ((2 0, 2 2, 0 2, 0 0, 2 0))"
        )
        p.fromWkt("POLYGON ((42 42, 0 42, 0 100, 42 100, 100 42, 42 42))")
        self.assertEqual(
            p.simplifyByDistance(1).asWkt(),
            "Polygon ((0 42, 0 100, 42 100, 100 42, 0 42))",
        )


if __name__ == "__main__":
    unittest.main()
