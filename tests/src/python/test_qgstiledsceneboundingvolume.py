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

from qgis.core import (
    Qgis,
    QgsSphere,
    QgsOrientedBox3D,
    QgsTiledSceneBoundingVolume,
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

    def test_box(self):
        volume = QgsTiledSceneBoundingVolume(
            QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30])
        )
        volume.transform(QgsMatrix4x4(1.5, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1))
        self.assertEqual(
            volume.box(), QgsOrientedBox3D([1.5, 4, 9], [15, 0, 0, 0, 40, 0, 0, 0, 90])
        )

        cloned = QgsTiledSceneBoundingVolume(volume)
        self.assertIsInstance(cloned, QgsTiledSceneBoundingVolume)
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
        volume = QgsTiledSceneBoundingVolume(
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
        volume = QgsTiledSceneBoundingVolume(
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

        volume = QgsTiledSceneBoundingVolume(
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


if __name__ == "__main__":
    unittest.main()
