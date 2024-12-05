"""QGIS Unit tests for QgsOrientedBox3D

From build dir, run: ctest -R QgsOrientedBox3D -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2023 by Nyall Dawson"
__date__ = "10/07/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import math
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransform,
    QgsCoordinateTransformContext,
    QgsMatrix4x4,
    QgsOrientedBox3D,
    QgsVector3D,
    QgsBox3D,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsOrientedBox3D(QgisTestCase):

    def test_oriented_bounding_box(self):
        box = QgsOrientedBox3D()
        self.assertTrue(box.isNull())

        # valid
        box = QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30])
        self.assertEqual(box.centerX(), 1)
        self.assertEqual(box.centerY(), 2)
        self.assertEqual(box.centerZ(), 3)
        self.assertEqual(box.center(), QgsVector3D(1, 2, 3))
        self.assertEqual(
            box.halfAxes(), [10.0, 0.0, 0.0, 0.0, 20.0, 0.0, 0.0, 0.0, 30.0]
        )

        box = QgsOrientedBox3D(
            QgsVector3D(1, 2, 3),
            [QgsVector3D(10, 0, 0), QgsVector3D(0, 20, 0), QgsVector3D(0, 0, 30)],
        )
        self.assertEqual(box.centerX(), 1)
        self.assertEqual(box.centerY(), 2)
        self.assertEqual(box.centerZ(), 3)
        self.assertEqual(box.center(), QgsVector3D(1, 2, 3))
        self.assertEqual(
            box.halfAxes(), [10.0, 0.0, 0.0, 0.0, 20.0, 0.0, 0.0, 0.0, 30.0]
        )

        box = QgsOrientedBox3D([1, 2, 3], [1, 0, 0, 0, 1, 0, 0, 0, 1])
        self.assertEqual(box.centerX(), 1)
        self.assertEqual(box.centerY(), 2)
        self.assertEqual(box.centerZ(), 3)
        self.assertEqual(box.halfAxes(), [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0])

        # 45 degree y axis rotation
        box = QgsOrientedBox3D(
            [1, 2, 3],
            [
                math.cos(math.pi / 4),
                0,
                math.sin(math.pi / 4),
                0,
                1,
                0,
                -math.sin(math.pi / 4),
                0,
                math.cos(math.pi / 4),
            ],
        )
        self.assertEqual(box.centerX(), 1)
        self.assertEqual(box.centerY(), 2)
        self.assertEqual(box.centerZ(), 3)
        self.assertEqual(
            box.halfAxes(),
            [
                0.7071067811865476,
                0.0,
                0.7071067811865475,
                0.0,
                1.0,
                0.0,
                -0.7071067811865475,
                0.0,
                0.7071067811865476,
            ],
        )

    def test_repr(self):
        box = QgsOrientedBox3D([1, 2, 3], [10, 11, 12, 21, 20, 22, 31, 32, 30])
        self.assertEqual(
            str(box),
            "<QgsOrientedBox3D([1, 2, 3], [10, 11, 12, 21, 20, 22, 31, 32, 30])>",
        )

    def test_equality(self):
        self.assertEqual(
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
        )
        self.assertNotEqual(
            QgsOrientedBox3D([11, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
        )
        self.assertNotEqual(
            QgsOrientedBox3D([1, 12, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
        )
        self.assertNotEqual(
            QgsOrientedBox3D([1, 2, 13], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
        )
        self.assertNotEqual(
            QgsOrientedBox3D([1, 2, 3], [110, 0, 0, 0, 20, 0, 0, 0, 30]),
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
        )
        self.assertNotEqual(
            QgsOrientedBox3D([1, 2, 3], [10, 10, 0, 0, 20, 0, 0, 0, 30]),
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
        )
        self.assertNotEqual(
            QgsOrientedBox3D([1, 2, 3], [10, 0, 10, 0, 20, 0, 0, 0, 30]),
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
        )
        self.assertNotEqual(
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 10, 20, 0, 0, 0, 30]),
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
        )
        self.assertNotEqual(
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 120, 0, 0, 0, 30]),
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
        )
        self.assertNotEqual(
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 10, 0, 0, 30]),
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
        )
        self.assertNotEqual(
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 10, 0, 30]),
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
        )
        self.assertNotEqual(
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 10, 30]),
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
        )
        self.assertNotEqual(
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 310]),
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]),
        )

    def test_from_box3d(self):
        box = QgsOrientedBox3D.fromBox3D(QgsBox3D(5.0, 6.0, 7.0, 11.0, 13.0, 15.0))
        self.assertEqual(
            box, QgsOrientedBox3D([8, 9.5, 11], [3, 0, 0, 0, 3.5, 0, 0, 0, 4])
        )

    def test_box_extent(self):
        box = QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30])
        bounds = box.extent()
        self.assertEqual(bounds.xMinimum(), -9)
        self.assertEqual(bounds.xMaximum(), 11)
        self.assertEqual(bounds.yMinimum(), -18)
        self.assertEqual(bounds.yMaximum(), 22)
        self.assertEqual(bounds.zMinimum(), -27)
        self.assertEqual(bounds.zMaximum(), 33)

        box = QgsOrientedBox3D([1, 2, 3], [1, 0, 0, 0, 1, 0, 0, 0, 1])
        bounds = box.extent()
        self.assertEqual(bounds.xMinimum(), 0)
        self.assertEqual(bounds.xMaximum(), 2)
        self.assertEqual(bounds.yMinimum(), 1)
        self.assertEqual(bounds.yMaximum(), 3)
        self.assertEqual(bounds.zMinimum(), 2)
        self.assertEqual(bounds.zMaximum(), 4)

        # 45 degree y axis rotation
        box = QgsOrientedBox3D(
            [1, 2, 3],
            [
                math.cos(math.pi / 4),
                0,
                math.sin(math.pi / 4),
                0,
                1,
                0,
                -math.sin(math.pi / 4),
                0,
                math.cos(math.pi / 4),
            ],
        )
        bounds = box.extent()
        self.assertAlmostEqual(bounds.xMinimum(), 1 - math.sqrt(2), 5)
        self.assertAlmostEqual(bounds.xMaximum(), 1 + math.sqrt(2), 5)
        self.assertAlmostEqual(bounds.yMinimum(), 1, 5)
        self.assertAlmostEqual(bounds.yMaximum(), 3, 5)
        self.assertAlmostEqual(bounds.zMinimum(), 3 - math.sqrt(2), 5)
        self.assertAlmostEqual(bounds.zMaximum(), 3 + math.sqrt(2), 5)

    def test_corners(self):
        box = QgsOrientedBox3D([1, 2, 3], [1, 0, 0, 0, 1, 0, 0, 0, 1])
        self.assertEqual(
            box.corners(),
            [
                QgsVector3D(2, 3, 4),
                QgsVector3D(0, 3, 4),
                QgsVector3D(2, 1, 4),
                QgsVector3D(0, 1, 4),
                QgsVector3D(2, 3, 2),
                QgsVector3D(0, 3, 2),
                QgsVector3D(2, 1, 2),
                QgsVector3D(0, 1, 2),
            ],
        )

        box = QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30])
        self.assertEqual(
            box.corners(),
            [
                QgsVector3D(11, 22, 33),
                QgsVector3D(-9, 22, 33),
                QgsVector3D(11, -18, 33),
                QgsVector3D(-9, -18, 33),
                QgsVector3D(11, 22, -27),
                QgsVector3D(-9, 22, -27),
                QgsVector3D(11, -18, -27),
                QgsVector3D(-9, -18, -27),
            ],
        )

        # 45 degree y axis rotation
        box = QgsOrientedBox3D(
            [1, 2, 3],
            [
                math.cos(math.pi / 4),
                0,
                math.sin(math.pi / 4),
                0,
                1,
                0,
                -math.sin(math.pi / 4),
                0,
                math.cos(math.pi / 4),
            ],
        )
        self.assertEqual(
            box.corners(),
            [
                QgsVector3D(1, 3, 4.41421356237309492),
                QgsVector3D(-0.41421356237309492, 3, 3),
                QgsVector3D(1, 1, 4.41421356237309492),
                QgsVector3D(-0.41421356237309492, 1, 3),
                QgsVector3D(2.41421356237309492, 3, 3),
                QgsVector3D(0.99999999999999989, 3, 1.58578643762690508),
                QgsVector3D(2.41421356237309492, 1, 3),
                QgsVector3D(0.99999999999999989, 1, 1.58578643762690508),
            ],
        )

    def test_size(self):
        box = QgsOrientedBox3D([1, 2, 3], [1, 0, 0, 0, 1, 0, 0, 0, 1])
        self.assertEqual(box.size(), QgsVector3D(2, 2, 2))

        box = QgsOrientedBox3D([10, 10, 10], [1, 0, 0, 0, 2, 0, 0, 0, 3])
        self.assertEqual(box.size(), QgsVector3D(2, 4, 6))

        box = QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30])
        self.assertEqual(box.size(), QgsVector3D(20, 40, 60))

    def test_reprojectedExtent(self):
        box = QgsOrientedBox3D(
            [-2694341.0, -4296866.0, 3854579.0],
            [-8.867, -14.142, 12.771, 104.740, -65.681, 0.0, 50.287, 80.192, 123.717],
        )

        # from ECEF (XYZ) to lon,lat,alt (deg,deg,m)
        ct = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:4978"),
            QgsCoordinateReferenceSystem("EPSG:4979"),
            QgsCoordinateTransformContext(),
        )
        aabb = box.reprojectedExtent(ct)

        # a box roughly around 37.42 N / 122.09 E
        self.assertEqual(aabb.xMinimum(), -122.09108059552956)
        self.assertEqual(aabb.yMinimum(), 37.41940987216044)
        self.assertEqual(aabb.zMinimum(), -35.7030292628333)
        self.assertEqual(aabb.xMaximum(), -122.088287207839)
        self.assertEqual(aabb.yMaximum(), 37.42221702328144)
        self.assertEqual(aabb.zMaximum(), 6.344767062924802)

    def test_transformed(self):
        box = QgsOrientedBox3D([1, 2, 3], [1, 0, 0, 0, 1, 0, 0, 0, 1])

        # translate by (10, 20, 30)
        m_move = QgsMatrix4x4(1, 0, 0, 10, 0, 1, 0, 20, 0, 0, 1, 30, 0, 0, 0, 1)
        # scale (4*X, 5*Y, 6*Z)
        m_scale = QgsMatrix4x4(4, 0, 0, 0, 0, 5, 0, 0, 0, 0, 6, 0, 0, 0, 0, 1)

        box_moved = box.transformed(m_move)
        self.assertEqual(
            box_moved, QgsOrientedBox3D([11, 22, 33], [1, 0, 0, 0, 1, 0, 0, 0, 1])
        )

        box_scaled = box.transformed(m_scale)
        self.assertEqual(
            box_scaled, QgsOrientedBox3D([4, 10, 18], [4, 0, 0, 0, 5, 0, 0, 0, 6])
        )

    def test_intersects(self):
        a1 = QgsOrientedBox3D([0, 0, 0], [1, 0, 0, 0, 1, 0, 0, 0, 1])
        self.assertTrue(a1.intersects(a1))

        b1 = QgsOrientedBox3D([1, 1, 1], [1, 0, 0, 0, 1, 0, 0, 0, 1])
        self.assertTrue(a1.intersects(b1))
        self.assertTrue(b1.intersects(a1))

        a2 = QgsOrientedBox3D([0, 0, 0], [1, 0, 0, 0, 1, 0, 0, 0, 1])
        b2 = QgsOrientedBox3D([3, 3, 3], [1, 0, 0, 0, 1, 0, 0, 0, 1])
        self.assertFalse(a2.intersects(b2))
        self.assertFalse(b2.intersects(a2))

        # box inside another box
        a3 = QgsOrientedBox3D([0, 0, 0], [1, 0, 0, 0, 1, 0, 0, 0, 1])
        b3 = QgsOrientedBox3D([0.5, 0.5, 0.5], [0.5, 0, 0, 0, 0.5, 0, 0, 0, 0.5])
        self.assertTrue(a3.intersects(b3))
        self.assertTrue(b3.intersects(a3))

        # Intersecting boxes (x-axis rotation)
        a1 = QgsOrientedBox3D(
            QgsVector3D(0, 0, 0),
            [QgsVector3D(1, 0, 0), QgsVector3D(0, 0, -1), QgsVector3D(0, 1, 0)],
        )
        b1 = QgsOrientedBox3D(
            QgsVector3D(1, 1, 1),
            [QgsVector3D(1, 0, 0), QgsVector3D(0, 1, 0), QgsVector3D(0, 0, 1)],
        )
        self.assertTrue(a1.intersects(b1))
        self.assertTrue(b1.intersects(a1))

        # Intersecting boxes (45 degree x-axis rotation)
        a1 = QgsOrientedBox3D(
            QgsVector3D(0, 0, 0),
            [
                QgsVector3D(1, 0, 0),
                QgsVector3D(0, 0.7071, -0.7071),
                QgsVector3D(0, 0.7071, 0.7071),
            ],
        )
        b1 = QgsOrientedBox3D(
            QgsVector3D(1, 1, 1),
            [QgsVector3D(1, 0, 0), QgsVector3D(0, 1, 0), QgsVector3D(0, 0, 1)],
        )
        self.assertTrue(a1.intersects(b1))
        self.assertTrue(b1.intersects(a1))

        # Non-intersecting boxes (45 degree x-axis rotation)
        a1 = QgsOrientedBox3D(
            QgsVector3D(0, -1, -0.6),
            [
                QgsVector3D(1, 0, 0),
                QgsVector3D(0, 0.7071, -0.7071),
                QgsVector3D(0, 0.7071, 0.7071),
            ],
        )
        b1 = QgsOrientedBox3D(
            QgsVector3D(1, 1, 1),
            [QgsVector3D(1, 0, 0), QgsVector3D(0, 1, 0), QgsVector3D(0, 0, 1)],
        )
        self.assertFalse(a1.intersects(b1))
        self.assertFalse(b1.intersects(a1))

        # Intersecting boxes (45 degree y-axis rotation)
        a1 = QgsOrientedBox3D(
            QgsVector3D(0, 0, 0),
            [
                QgsVector3D(0.7071, 0, 0.7071),
                QgsVector3D(0, 1, 0),
                QgsVector3D(-0.7071, 0, 0.7071),
            ],
        )
        b1 = QgsOrientedBox3D(
            QgsVector3D(1, 1, 1),
            [QgsVector3D(1, 0, 0), QgsVector3D(0, 1, 0), QgsVector3D(0, 0, 1)],
        )
        self.assertTrue(a1.intersects(b1))
        self.assertTrue(b1.intersects(a1))

        # Non-intersecting boxes with non-zero centers (45 degrees rotation around z-axis)
        a1 = QgsOrientedBox3D(
            QgsVector3D(1, 1, 1),
            [
                QgsVector3D(0.7071, 0.7071, 0),
                QgsVector3D(-0.7071, 0.7071, 0),
                QgsVector3D(0, 0, 1),
            ],
        )
        b1 = QgsOrientedBox3D(
            QgsVector3D(4, 4, 4),
            [QgsVector3D(1, 0, 0), QgsVector3D(0, 1, 0), QgsVector3D(0, 0, 1)],
        )
        self.assertFalse(a1.intersects(b1))
        self.assertFalse(b1.intersects(a1))

        # Non-intersecting boxes with non-zero centers and rotations
        a1 = QgsOrientedBox3D(
            QgsVector3D(1, 1, 1),
            [
                QgsVector3D(0.7071, 0, 0.7071),
                QgsVector3D(0, 3, 0),
                QgsVector3D(-0.7071 * 2, 0, 0.7071 * 2),
            ],
        )
        b1 = QgsOrientedBox3D(
            QgsVector3D(4, 4, 4),
            [
                QgsVector3D(0.7071 * 2, 0.7071 * 2, 0),
                QgsVector3D(-0.7071, 0.7071, 0),
                QgsVector3D(0, 0, 2.1),
            ],
        )
        self.assertFalse(a1.intersects(b1))
        self.assertFalse(b1.intersects(a1))

        # Intersecting boxes with non-zero centers and rotations
        a1 = QgsOrientedBox3D(
            QgsVector3D(1, 1, 1),
            [
                QgsVector3D(0.7071, 0, 0.7071),
                QgsVector3D(0, 3, 0),
                QgsVector3D(-0.7071 * 2, 0, 0.7071 * 2),
            ],
        )
        b1 = QgsOrientedBox3D(
            QgsVector3D(4, 4, 4),
            [
                QgsVector3D(0.7071 * 2, 0.7071 * 2, 0),
                QgsVector3D(-0.7071, 0.7071, 0),
                QgsVector3D(0, 0, 3),
            ],
        )
        self.assertTrue(a1.intersects(b1))
        self.assertTrue(b1.intersects(a1))


if __name__ == "__main__":
    unittest.main()
