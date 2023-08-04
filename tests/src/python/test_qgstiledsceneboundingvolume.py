"""QGIS Unit tests for QgsAbstractTiledSceneBoundingVolume

From build dir, run: ctest -R QgsTiledSceneBoundingVolume -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = "(C) 2023 by Nyall Dawson"
__date__ = "10/07/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import unittest

import qgis  # NOQA
from qgis.core import (
    Qgis,
    QgsSphere,
    QgsOrientedBox3D,
    QgsTiledSceneBoundingVolumeSphere,
    QgsTiledSceneBoundingVolumeRegion,
    QgsTiledSceneBoundingVolumeBox,
    QgsBox3d,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransform,
    QgsCoordinateTransformContext,
    QgsMatrix4x4,
)
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsTiledSceneBoundingVolume(QgisTestCase):
    def test_region(self):
        volume = QgsTiledSceneBoundingVolumeRegion(QgsBox3d(1, 2, 3, 10, 11, 12))
        self.assertEqual(volume.type(), Qgis.TiledSceneBoundingVolumeType.Region)
        volume.transform(QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        # should be no change when transforming regions!
        self.assertEqual(volume.region(), QgsBox3d(1, 2, 3, 10, 11, 12))

        cloned = volume.clone()
        self.assertIsInstance(cloned, QgsTiledSceneBoundingVolumeRegion)
        self.assertEqual(cloned.region(), QgsBox3d(1, 2, 3, 10, 11, 12))

        # bounds
        bounds = volume.bounds()
        self.assertEqual(bounds.xMinimum(), 1)
        self.assertEqual(bounds.xMaximum(), 10)
        self.assertEqual(bounds.yMinimum(), 2)
        self.assertEqual(bounds.yMaximum(), 11)
        self.assertEqual(bounds.zMinimum(), 3)
        self.assertEqual(bounds.zMaximum(), 12)

        geometry_2d = volume.as2DGeometry()
        self.assertEqual(geometry_2d.asWkt(), "Polygon ((1 2, 10 2, 10 11, 1 11, 1 2))")

        # with coordinate transform
        volume = QgsTiledSceneBoundingVolumeRegion(
            QgsBox3d(
                -4595750,
                2698725,
                -3493318,
                -4595750 + 1000,
                2698725 + 1500,
                -3493318 + 2000,
            )
        )
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:4978"),
            QgsCoordinateReferenceSystem("EPSG:4979"),
            QgsCoordinateTransformContext(),
        )
        bounds = volume.bounds(transform)
        self.assertAlmostEqual(bounds.xMinimum(), 149.5582617, 3)
        self.assertAlmostEqual(bounds.xMaximum(), 149.577611, 3)
        self.assertAlmostEqual(bounds.yMinimum(), -33.424296, 3)
        self.assertAlmostEqual(bounds.yMaximum(), -33.4011944, 3)
        self.assertAlmostEqual(bounds.zMinimum(), -1122.81806, 3)
        self.assertAlmostEqual(bounds.zMaximum(), 1332.44347, 3)

        geometry_2d = volume.as2DGeometry(transform)
        self.assertEqual(
            geometry_2d.asWkt(3),
            "Polygon ((149.572 -33.424, 149.558 -33.421, 149.558 -33.405, 149.564 -33.401, 149.578 -33.405, 149.578 -33.42, 149.572 -33.424))",
        )

    def test_region_intersects(self):
        volume = QgsTiledSceneBoundingVolumeRegion(QgsBox3d(1, 2, 3, 10, 11, 12))
        self.assertFalse(
            volume.intersects(
                QgsOrientedBox3D.fromBox3D(QgsBox3d(11, 2, 3, 10, 11, 12))
            )
        )
        self.assertTrue(
            volume.intersects(QgsOrientedBox3D.fromBox3D(QgsBox3d(9, 2, 3, 10, 11, 12)))
        )

    def test_box(self):
        volume = QgsTiledSceneBoundingVolumeBox(
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30])
        )
        self.assertEqual(volume.type(), Qgis.TiledSceneBoundingVolumeType.OrientedBox)
        volume.transform(QgsMatrix4x4(1.5, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1))
        self.assertEqual(
            volume.box(), QgsOrientedBox3D([1.5, 4, 9], [15, 0, 0, 0, 40, 0, 0, 0, 90])
        )

        cloned = volume.clone()
        self.assertIsInstance(cloned, QgsTiledSceneBoundingVolumeBox)
        self.assertEqual(
            cloned.box(), QgsOrientedBox3D([1.5, 4, 9], [15, 0, 0, 0, 40, 0, 0, 0, 90])
        )

        # bounds
        bounds = volume.bounds()
        self.assertEqual(bounds.xMinimum(), -13.5)
        self.assertEqual(bounds.xMaximum(), 16.5)
        self.assertEqual(bounds.yMinimum(), -36)
        self.assertEqual(bounds.yMaximum(), 44)
        self.assertEqual(bounds.zMinimum(), -81)
        self.assertEqual(bounds.zMaximum(), 99)

        geometry_2d = volume.as2DGeometry()
        self.assertEqual(
            geometry_2d.asWkt(),
            "Polygon ((-13.5 -36, -13.5 44, 16.5 44, 16.5 -36, -13.5 -36))",
        )

        # with coordinate transform
        volume = QgsTiledSceneBoundingVolumeBox(
            QgsOrientedBox3D(
                [-4595750, 2698725, -3493318], [1000, 0, 0, 0, 1500, 0, 0, 0, 2000]
            )
        )
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:4978"),
            QgsCoordinateReferenceSystem("EPSG:4979"),
            QgsCoordinateTransformContext(),
        )
        bounds = volume.bounds(transform)
        self.assertAlmostEqual(bounds.xMinimum(), 149.5582617, 3)
        self.assertAlmostEqual(bounds.xMaximum(), 149.5969605, 3)
        self.assertAlmostEqual(bounds.yMinimum(), -33.44311, 3)
        self.assertAlmostEqual(bounds.yMaximum(), -33.396915, 3)
        self.assertAlmostEqual(bounds.zMinimum(), -1756.82217, 3)
        self.assertAlmostEqual(bounds.zMaximum(), 3153.6759909, 3)

        geometry_2d = volume.as2DGeometry(transform)
        self.assertEqual(
            geometry_2d.asWkt(5),
            "Polygon ((149.58608 -33.44312, 149.55826 -33.43557, 149.55826 -33.40547, 149.56915 -33.39692, 149.59696 -33.40445, 149.59696 -33.43455, 149.58608 -33.44312))",
        )

    def test_box_intersects(self):
        volume = QgsTiledSceneBoundingVolumeBox(
            QgsOrientedBox3D(
                [1, 1, 1],
                [
                    0.7071,
                    0.7071,
                    0,
                    -0.7071,
                    0.7071,
                    0,
                    0,
                    0,
                    1,
                ],
            )
        )
        self.assertFalse(
            volume.intersects(
                QgsOrientedBox3D(
                    [4, 4, 4],
                    [1, 0, 0, 0, 1, 0, 0, 0, 1],
                )
            )
        )

        volume = QgsTiledSceneBoundingVolumeBox(
            QgsOrientedBox3D(
                [1, 1, 1], [0.7071, 0, 0.7071, 0, 3, 0, -0.7071 * 2, 0, 0.7071 * 2]
            )
        )
        self.assertTrue(
            volume.intersects(
                QgsOrientedBox3D(
                    [4, 4, 4], [0.7071 * 2, 0.7071 * 2, 0, -0.7071, 0.7071, 0, 0, 0, 3]
                )
            )
        )

    def test_sphere(self):
        volume = QgsTiledSceneBoundingVolumeSphere(QgsSphere(1, 2, 3, 10))
        self.assertEqual(volume.type(), Qgis.TiledSceneBoundingVolumeType.Sphere)
        volume.transform(QgsMatrix4x4(1.5, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1))
        self.assertEqual(volume.sphere(), QgsSphere(1.5, 4, 9, 30))

        cloned = volume.clone()
        self.assertIsInstance(cloned, QgsTiledSceneBoundingVolumeSphere)
        self.assertEqual(cloned.sphere(), QgsSphere(1.5, 4, 9, 30))

        # bounds
        bounds = volume.bounds()
        self.assertEqual(bounds.xMinimum(), -28.5)
        self.assertEqual(bounds.xMaximum(), 31.5)
        self.assertEqual(bounds.yMinimum(), -26.0)
        self.assertEqual(bounds.yMaximum(), 34.0)
        self.assertEqual(bounds.zMinimum(), -21.0)
        self.assertEqual(bounds.zMaximum(), 39)

        geometry_2d = volume.as2DGeometry()
        self.assertEqual(
            geometry_2d.asWkt(),
            "CurvePolygon (CircularString (1.5 34, 31.5 4, 1.5 -26, -28.5 4, 1.5 34))",
        )

        # with coordinate transform
        volume = QgsTiledSceneBoundingVolumeSphere(
            QgsSphere(-4595750, 2698725, -3493318, 1983)
        )
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem("EPSG:4978"),
            QgsCoordinateReferenceSystem("EPSG:4979"),
            QgsCoordinateTransformContext(),
        )
        bounds = volume.bounds(transform)
        self.assertAlmostEqual(bounds.xMinimum(), 149.5484294320, 3)
        self.assertAlmostEqual(bounds.xMaximum(), 149.606785943, 3)
        self.assertAlmostEqual(bounds.yMinimum(), -33.448419, 3)
        self.assertAlmostEqual(bounds.yMaximum(), -33.39162, 3)
        self.assertAlmostEqual(bounds.zMinimum(), -2659.1526749, 3)
        self.assertAlmostEqual(bounds.zMaximum(), 4055.8967716, 3)

        geometry_2d = volume.as2DGeometry(transform)
        self.assertEqual(
            geometry_2d.asWkt(3),
            "Polygon ((149.592 -33.433, 149.594 -33.431, 149.596 -33.429, 149.597 -33.427, 149.598 -33.425, 149.599 -33.423, 149.599 -33.421, 149.599 -33.418, 149.598 -33.416, 149.598 -33.414, 149.596 -33.412, 149.595 -33.41, 149.593 -33.408, 149.591 -33.406, 149.589 -33.405, 149.586 -33.404, 149.584 -33.403, 149.581 -33.402, 149.578 -33.402, 149.576 -33.402, 149.573 -33.403, 149.57 -33.403, 149.568 -33.404, 149.565 -33.405, 149.563 -33.407, 149.561 -33.409, 149.56 -33.411, 149.558 -33.413, 149.557 -33.415, 149.557 -33.417, 149.556 -33.419, 149.556 -33.422, 149.557 -33.424, 149.558 -33.426, 149.559 -33.428, 149.56 -33.43, 149.562 -33.432, 149.564 -33.434, 149.566 -33.435, 149.569 -33.436, 149.571 -33.437, 149.574 -33.438, 149.577 -33.438, 149.58 -33.438, 149.582 -33.437, 149.585 -33.437, 149.588 -33.436, 149.59 -33.435, 149.592 -33.433))",
        )

    def test_sphere_intersects(self):
        volume = QgsTiledSceneBoundingVolumeSphere(QgsSphere(1, 2, 3, 6))
        self.assertFalse(
            volume.intersects(
                QgsOrientedBox3D.fromBox3D(QgsBox3d(11, 2, 3, 10, 11, 12))
            )
        )
        self.assertTrue(
            volume.intersects(
                QgsOrientedBox3D.fromBox3D(QgsBox3d(6.5, 2, 3, 10, 11, 12))
            )
        )


if __name__ == "__main__":
    unittest.main()
