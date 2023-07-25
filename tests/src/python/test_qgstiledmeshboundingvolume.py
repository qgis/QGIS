"""QGIS Unit tests for QgsAbstractTiledMeshNodeBoundingVolume

From build dir, run: ctest -R QgsTiledMeshNodeBoundingVolume -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2023 by Nyall Dawson'
__date__ = '10/07/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

import math
import qgis  # NOQA
from qgis.core import (
    Qgis,
    QgsSphere,
    QgsOrientedBox3D,
    QgsTiledMeshNodeBoundingVolumeSphere,
    QgsTiledMeshNodeBoundingVolumeRegion,
    QgsTiledMeshNodeBoundingVolumeBox,
    QgsBox3d,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransform,
    QgsCoordinateTransformContext,
    QgsMatrix4x4
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsTiledMeshNodeBoundingVolume(QgisTestCase):

    def test_region(self):
        volume = QgsTiledMeshNodeBoundingVolumeRegion(QgsBox3d(1, 2, 3, 10, 11, 12))
        self.assertEqual(volume.type(), Qgis.TiledMeshBoundingVolumeType.Region)
        volume.setTransform(QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        self.assertEqual(volume.transform(),
                         QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))

        cloned = volume.clone()
        self.assertIsInstance(cloned, QgsTiledMeshNodeBoundingVolumeRegion)
        self.assertEqual(cloned.region(), QgsBox3d(1, 2, 3, 10, 11, 12))
        self.assertEqual(cloned.transform(), QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        volume.setTransform(QgsMatrix4x4())

        # bounds
        bounds = volume.bounds()
        self.assertEqual(bounds.xMinimum(), 1)
        self.assertEqual(bounds.xMaximum(), 10)
        self.assertEqual(bounds.yMinimum(), 2)
        self.assertEqual(bounds.yMaximum(), 11)
        self.assertEqual(bounds.zMinimum(), 3)
        self.assertEqual(bounds.zMaximum(), 12)

        geometry_2d = volume.as2DGeometry()
        self.assertEqual(geometry_2d.asWkt(), 'Polygon ((1 2, 10 2, 10 11, 1 11, 1 2))')

        # with transform
        volume.setTransform(
            QgsMatrix4x4(1, 0, 0, 2, 0, 1, 0, 3, 0, 0, 1, 4, 0, 0, 0, 1))
        bounds = volume.bounds()
        self.assertEqual(bounds.xMinimum(), 3)
        self.assertEqual(bounds.xMaximum(), 12)
        self.assertEqual(bounds.yMinimum(), 5)
        self.assertEqual(bounds.yMaximum(), 14)
        self.assertEqual(bounds.zMinimum(), 7)
        self.assertEqual(bounds.zMaximum(), 16)

        geometry_2d = volume.as2DGeometry()
        self.assertEqual(geometry_2d.asWkt(), 'Polygon ((3 5, 3 14, 12 14, 12 5, 3 5))')

        # with coordinate transform
        volume = QgsTiledMeshNodeBoundingVolumeRegion(QgsBox3d(-4595750, 2698725, -3493318,
                                                               -4595750 + 1000, 2698725 + 1500, -3493318 + 2000))
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem('EPSG:4978'),
            QgsCoordinateReferenceSystem('EPSG:4979'),
            QgsCoordinateTransformContext()
        )
        bounds = volume.bounds(transform)
        self.assertAlmostEqual(bounds.xMinimum(), 149.5582617, 3)
        self.assertAlmostEqual(bounds.xMaximum(), 149.577611, 3)
        self.assertAlmostEqual(bounds.yMinimum(), -33.424296, 3)
        self.assertAlmostEqual(bounds.yMaximum(), -33.4011944, 3)
        self.assertAlmostEqual(bounds.zMinimum(), -1122.81806, 3)
        self.assertAlmostEqual(bounds.zMaximum(), 1332.44347, 3)

        geometry_2d = volume.as2DGeometry(transform)
        self.assertEqual(geometry_2d.asWkt(3),
                         'Polygon ((149.572 -33.424, 149.558 -33.421, 149.558 -33.405, 149.564 -33.401, 149.578 -33.405, 149.578 -33.42, 149.572 -33.424))')

        # coordinate transform + volume transform
        volume.setTransform(
            QgsMatrix4x4(1, 0, 0, 200, 0, 1, 0, 500, 0, 0, 1, 1000, 0, 0, 0, 1))
        bounds = volume.bounds(transform)
        self.assertAlmostEqual(bounds.xMinimum(), 149.55253, 3)
        self.assertAlmostEqual(bounds.xMaximum(), 149.57188, 3)
        self.assertAlmostEqual(bounds.yMinimum(), -33.416372, 3)
        self.assertAlmostEqual(bounds.yMaximum(), -33.39326, 3)
        self.assertAlmostEqual(bounds.zMinimum(), -1605.92465, 3)
        self.assertAlmostEqual(bounds.zMaximum(), 849.29506, 3)

        geometry_2d = volume.as2DGeometry(transform)
        self.assertEqual(geometry_2d.asWkt(3),
                         'Polygon ((149.566 -33.416, 149.553 -33.413, 149.553 -33.398, 149.558 -33.393, 149.572 -33.397, 149.572 -33.412, 149.566 -33.416))')

    def test_box(self):
        volume = QgsTiledMeshNodeBoundingVolumeBox(QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]))
        self.assertEqual(volume.type(), Qgis.TiledMeshBoundingVolumeType.OrientedBox)
        volume.setTransform(QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        self.assertEqual(volume.transform(),
                         QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))

        cloned = volume.clone()
        self.assertIsInstance(cloned, QgsTiledMeshNodeBoundingVolumeBox)
        self.assertEqual(cloned.box(), QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]))
        self.assertEqual(cloned.transform(),
                         QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        volume.setTransform(QgsMatrix4x4())

        # bounds
        bounds = volume.bounds()
        self.assertEqual(bounds.xMinimum(), -9)
        self.assertEqual(bounds.xMaximum(), 11)
        self.assertEqual(bounds.yMinimum(), -18)
        self.assertEqual(bounds.yMaximum(), 22)
        self.assertEqual(bounds.zMinimum(), -27)
        self.assertEqual(bounds.zMaximum(), 33)

        geometry_2d = volume.as2DGeometry()
        self.assertEqual(geometry_2d.asWkt(), 'Polygon ((-9 -18, -9 22, 11 22, 11 -18, -9 -18))')

        # with transform
        volume.setTransform(
            QgsMatrix4x4(1, 0, 0, 2, 0, 1, 0, 3, 0, 0, 1, 4, 0, 0, 0, 1))
        bounds = volume.bounds()
        self.assertEqual(bounds.xMinimum(), -7)
        self.assertEqual(bounds.xMaximum(), 13)
        self.assertEqual(bounds.yMinimum(), -15)
        self.assertEqual(bounds.yMaximum(), 25)
        self.assertEqual(bounds.zMinimum(), -23)
        self.assertEqual(bounds.zMaximum(), 37)

        geometry_2d = volume.as2DGeometry()
        self.assertEqual(geometry_2d.asWkt(), 'Polygon ((-7 -15, -7 25, 13 25, 13 -15, -7 -15))')

        # with coordinate transform
        volume = QgsTiledMeshNodeBoundingVolumeBox(QgsOrientedBox3D([-4595750, 2698725, -3493318],
                                                                    [1000, 0, 0, 0, 1500, 0, 0, 0, 2000]))
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem('EPSG:4978'),
            QgsCoordinateReferenceSystem('EPSG:4979'),
            QgsCoordinateTransformContext()
        )
        bounds = volume.bounds(transform)
        self.assertAlmostEqual(bounds.xMinimum(), 149.5582617, 3)
        self.assertAlmostEqual(bounds.xMaximum(), 149.5969605, 3)
        self.assertAlmostEqual(bounds.yMinimum(), -33.44311, 3)
        self.assertAlmostEqual(bounds.yMaximum(), -33.396915, 3)
        self.assertAlmostEqual(bounds.zMinimum(), -1756.82217, 3)
        self.assertAlmostEqual(bounds.zMaximum(), 3153.6759909, 3)

        geometry_2d = volume.as2DGeometry(transform)
        self.assertEqual(geometry_2d.asWkt(5),
                         'Polygon ((149.58608 -33.44312, 149.55826 -33.43557, 149.55826 -33.40547, 149.56915 -33.39692, 149.59696 -33.40445, 149.59696 -33.43455, 149.58608 -33.44312))')

        # coordinate transform + volume transform
        volume.setTransform(
            QgsMatrix4x4(1, 0, 0, 200, 0, 1, 0, 500, 0, 0, 1, 1000, 0, 0, 0, 1))
        bounds = volume.bounds(transform)
        self.assertAlmostEqual(bounds.xMinimum(), 149.5525380, 3)
        self.assertAlmostEqual(bounds.xMaximum(), 149.5912365, 3)
        self.assertAlmostEqual(bounds.yMinimum(), -33.435195, 3)
        self.assertAlmostEqual(bounds.yMaximum(), -33.38898, 3)
        self.assertAlmostEqual(bounds.zMinimum(), -2240.09454, 3)
        self.assertAlmostEqual(bounds.zMaximum(), 2670.320130, 3)

        geometry_2d = volume.as2DGeometry(transform)
        self.assertEqual(geometry_2d.asWkt(5),
                         'Polygon ((149.58035 -33.4352, 149.55254 -33.42765, 149.55254 -33.39754, 149.56343 -33.38899, 149.59124 -33.39653, 149.59124 -33.42663, 149.58035 -33.4352))')

    def test_sphere(self):
        volume = QgsTiledMeshNodeBoundingVolumeSphere(QgsSphere(1, 2, 3, 10))
        self.assertEqual(volume.type(), Qgis.TiledMeshBoundingVolumeType.Sphere)
        volume.setTransform(QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        self.assertEqual(volume.transform(),
                         QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))

        cloned = volume.clone()
        self.assertIsInstance(cloned, QgsTiledMeshNodeBoundingVolumeSphere)
        self.assertEqual(cloned.sphere(), QgsSphere(1, 2, 3, 10))
        self.assertEqual(cloned.transform(),
                         QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        volume.setTransform(QgsMatrix4x4())

        # bounds
        bounds = volume.bounds()
        self.assertEqual(bounds.xMinimum(), -9)
        self.assertEqual(bounds.xMaximum(), 11)
        self.assertEqual(bounds.yMinimum(), -8)
        self.assertEqual(bounds.yMaximum(), 12)
        self.assertEqual(bounds.zMinimum(), -7)
        self.assertEqual(bounds.zMaximum(), 13)

        geometry_2d = volume.as2DGeometry()
        self.assertEqual(geometry_2d.asWkt(), 'CurvePolygon (CircularString (1 12, 11 2, 1 -8, -9 2, 1 12))')

        # with transform
        volume.setTransform(
            QgsMatrix4x4(1, 0, 0, 2, 0, 1, 0, 3, 0, 0, 1, 4, 0, 0, 0, 1))
        bounds = volume.bounds()
        self.assertEqual(bounds.xMinimum(), -7)
        self.assertEqual(bounds.xMaximum(), 13)
        self.assertEqual(bounds.yMinimum(), -5)
        self.assertEqual(bounds.yMaximum(), 15)
        self.assertEqual(bounds.zMinimum(), -3)
        self.assertEqual(bounds.zMaximum(), 17)

        geometry_2d = volume.as2DGeometry()
        self.assertEqual(geometry_2d.asWkt(3), 'Polygon ((3 13.137, 1.768 13.318, 0.556 13.355, -0.613 13.25, -1.721 13.004, -2.748 12.621, -3.677 12.107, -4.491 11.472, -5.177 10.726, -5.724 9.882, -6.121 8.955, -6.361 7.96, -6.442 6.914, -6.361 5.835, -6.121 4.743, -5.724 3.654, -5.177 2.589, -4.491 1.565, -3.677 0.599, -2.748 -0.291, -1.721 -1.09, -0.613 -1.785, 0.556 -2.365, 1.768 -2.818, 3 -3.137, 4.232 -3.318, 5.444 -3.355, 6.613 -3.25, 7.721 -3.004, 8.748 -2.621, 9.677 -2.107, 10.491 -1.472, 11.177 -0.726, 11.724 0.118, 12.121 1.045, 12.361 2.04, 12.442 3.086, 12.361 4.165, 12.121 5.257, 11.724 6.346, 11.177 7.411, 10.491 8.435, 9.677 9.401, 8.748 10.291, 7.721 11.09, 6.613 11.785, 5.444 12.365, 4.232 12.818, 3 13.137))')

        # with transform
        volume = QgsTiledMeshNodeBoundingVolumeSphere(QgsSphere(-4595750, 2698725, -3493318, 1983))
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem('EPSG:4978'),
            QgsCoordinateReferenceSystem('EPSG:4979'),
            QgsCoordinateTransformContext()
        )
        bounds = volume.bounds(transform)
        self.assertAlmostEqual(bounds.xMinimum(), 149.5484294320, 3)
        self.assertAlmostEqual(bounds.xMaximum(), 149.606785943, 3)
        self.assertAlmostEqual(bounds.yMinimum(), -33.448419, 3)
        self.assertAlmostEqual(bounds.yMaximum(), -33.39162, 3)
        self.assertAlmostEqual(bounds.zMinimum(), -2659.1526749, 3)
        self.assertAlmostEqual(bounds.zMaximum(), 4055.8967716, 3)

        geometry_2d = volume.as2DGeometry(transform)
        self.assertEqual(geometry_2d.asWkt(3),
                         'Polygon ((149.592 -33.433, 149.594 -33.431, 149.596 -33.429, 149.597 -33.427, 149.598 -33.425, 149.599 -33.423, 149.599 -33.421, 149.599 -33.418, 149.598 -33.416, 149.598 -33.414, 149.596 -33.412, 149.595 -33.41, 149.593 -33.408, 149.591 -33.406, 149.589 -33.405, 149.586 -33.404, 149.584 -33.403, 149.581 -33.402, 149.578 -33.402, 149.576 -33.402, 149.573 -33.403, 149.57 -33.403, 149.568 -33.404, 149.565 -33.405, 149.563 -33.407, 149.561 -33.409, 149.56 -33.411, 149.558 -33.413, 149.557 -33.415, 149.557 -33.417, 149.556 -33.419, 149.556 -33.422, 149.557 -33.424, 149.558 -33.426, 149.559 -33.428, 149.56 -33.43, 149.562 -33.432, 149.564 -33.434, 149.566 -33.435, 149.569 -33.436, 149.571 -33.437, 149.574 -33.438, 149.577 -33.438, 149.58 -33.438, 149.582 -33.437, 149.585 -33.437, 149.588 -33.436, 149.59 -33.435, 149.592 -33.433))')

        # coordinate transform + volume transform
        volume.setTransform(
            QgsMatrix4x4(1, 0, 0, 200, 0, 1, 0, 500, 0, 0, 1, 1000, 0, 0, 0, 1))
        bounds = volume.bounds(transform)
        self.assertAlmostEqual(bounds.xMinimum(), 149.542705241, 3)
        self.assertAlmostEqual(bounds.xMaximum(), 149.601062383, 3)
        self.assertAlmostEqual(bounds.yMinimum(), -33.44049463, 3)
        self.assertAlmostEqual(bounds.yMaximum(), -33.383697, 3)
        self.assertAlmostEqual(bounds.zMinimum(), -3142.503473, 3)
        self.assertAlmostEqual(bounds.zMaximum(), 3572.619264, 3)

        geometry_2d = volume.as2DGeometry(transform)
        self.assertEqual(geometry_2d.asWkt(3),
                         'Polygon ((149.586 -33.425, 149.588 -33.423, 149.59 -33.422, 149.591 -33.419, 149.592 -33.417, 149.593 -33.415, 149.593 -33.413, 149.593 -33.41, 149.593 -33.408, 149.592 -33.406, 149.591 -33.404, 149.589 -33.402, 149.587 -33.4, 149.585 -33.398, 149.583 -33.397, 149.581 -33.396, 149.578 -33.395, 149.575 -33.394, 149.573 -33.394, 149.57 -33.394, 149.567 -33.395, 149.564 -33.395, 149.562 -33.396, 149.56 -33.398, 149.557 -33.399, 149.555 -33.401, 149.554 -33.403, 149.552 -33.405, 149.552 -33.407, 149.551 -33.409, 149.551 -33.411, 149.551 -33.414, 149.551 -33.416, 149.552 -33.418, 149.553 -33.42, 149.555 -33.422, 149.556 -33.424, 149.558 -33.426, 149.561 -33.427, 149.563 -33.428, 149.566 -33.429, 149.568 -33.43, 149.571 -33.43, 149.574 -33.43, 149.577 -33.43, 149.579 -33.429, 149.582 -33.428, 149.584 -33.427, 149.586 -33.425))')


if __name__ == '__main__':
    unittest.main()
