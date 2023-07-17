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
    QgsCoordinateTransformContext
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

        cloned = volume.clone()
        self.assertIsInstance(cloned, QgsTiledMeshNodeBoundingVolumeRegion)
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
        self.assertEqual(geometry_2d.asWkt(), 'Polygon ((1 2, 10 2, 10 11, 1 11, 1 2))')

        # with transform
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
                         'Polygon ((149.578 -33.412, 149.572 -33.417, 149.558 -33.413, 149.564 -33.409, 149.578 -33.412))')

    def test_box(self):
        volume = QgsTiledMeshNodeBoundingVolumeBox(QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]))
        self.assertEqual(volume.type(), Qgis.TiledMeshBoundingVolumeType.OrientedBox)

        cloned = volume.clone()
        self.assertIsInstance(cloned, QgsTiledMeshNodeBoundingVolumeBox)
        self.assertEqual(cloned.box(), QgsOrientedBox3D([1, 2, 3], [10, 0, 0, 0, 20, 0, 0, 0, 30]))

        # bounds
        bounds = volume.bounds()
        self.assertEqual(bounds.xMinimum(), -9)
        self.assertEqual(bounds.xMaximum(), 11)
        self.assertEqual(bounds.yMinimum(), -18)
        self.assertEqual(bounds.yMaximum(), 22)
        self.assertEqual(bounds.zMinimum(), -27)
        self.assertEqual(bounds.zMaximum(), 33)

        geometry_2d = volume.as2DGeometry()
        self.assertEqual(geometry_2d.asWkt(), 'Polygon ((11 22, -9 22, 11 -18, -9 -18, 11 22))')

        # with transform
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
        self.assertEqual(geometry_2d.asWkt(3),
                         'Polygon ((149.558 -33.421, 149.569 -33.412, 149.586 -33.428, 149.597 -33.42, 149.558 -33.421))')

    def test_sphere(self):
        volume = QgsTiledMeshNodeBoundingVolumeSphere(QgsSphere(1, 2, 3, 10))
        self.assertEqual(volume.type(), Qgis.TiledMeshBoundingVolumeType.Sphere)

        cloned = volume.clone()
        self.assertIsInstance(cloned, QgsTiledMeshNodeBoundingVolumeSphere)
        self.assertEqual(cloned.sphere(), QgsSphere(1, 2, 3, 10))

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
        volume = QgsTiledMeshNodeBoundingVolumeSphere(QgsSphere(-4595750, 2698725, -3493318, 1983))
        transform = QgsCoordinateTransform(
            QgsCoordinateReferenceSystem('EPSG:4978'),
            QgsCoordinateReferenceSystem('EPSG:4979'),
            QgsCoordinateTransformContext()
        )
        bounds = volume.bounds(transform)
        self.assertAlmostEqual(bounds.xMinimum(), 149.5562, 3)
        self.assertAlmostEqual(bounds.xMaximum(), 149.5989, 3)
        self.assertAlmostEqual(bounds.yMinimum(), -33.42986, 3)
        self.assertAlmostEqual(bounds.yMaximum(), -33.41017, 3)
        self.assertAlmostEqual(bounds.zMinimum(), -393.63504, 3)
        self.assertAlmostEqual(bounds.zMaximum(), 1790.72811, 3)

        geometry_2d = volume.as2DGeometry(transform)
        self.assertEqual(geometry_2d.asWkt(3),
                         'CurvePolygon ((149.559 -33.415, 149.559 -33.415, 149.559 -33.415, 149.559 -33.415, 149.559 -33.416, 149.558 -33.416, 149.558 -33.416, 149.558 -33.416, 149.558 -33.416, 149.558 -33.416, 149.558 -33.417, 149.558 -33.417, 149.557 -33.417, 149.557 -33.417, 149.557 -33.417, 149.557 -33.417, 149.557 -33.418, 149.557 -33.418, 149.557 -33.418, 149.557 -33.418, 149.557 -33.418, 149.557 -33.418, 149.557 -33.419, 149.556 -33.419, 149.556 -33.419, 149.556 -33.419, 149.556 -33.419, 149.556 -33.419, 149.556 -33.42, 149.556 -33.42, 149.556 -33.42, 149.556 -33.42, 149.556 -33.42, 149.556 -33.42, 149.556 -33.421, 149.556 -33.421, 149.556 -33.421, 149.556 -33.421, 149.556 -33.421, 149.557 -33.421, 149.557 -33.422, 149.557 -33.422, 149.557 -33.422, 149.557 -33.422, 149.557 -33.422, 149.557 -33.422, 149.557 -33.423, 149.557 -33.423, 149.557 -33.423, 149.557 -33.423, 149.558 -33.423, 149.558 -33.423, 149.558 -33.424, 149.558 -33.424, 149.558 -33.424, 149.558 -33.424, 149.558 -33.424, 149.559 -33.424, 149.559 -33.425, 149.559 -33.425, 149.559 -33.425, 149.559 -33.425, 149.559 -33.425, 149.56 -33.425, 149.56 -33.425, 149.56 -33.426, 149.56 -33.426, 149.56 -33.426, 149.561 -33.426, 149.561 -33.426, 149.561 -33.426, 149.561 -33.426, 149.562 -33.427, 149.562 -33.427, 149.562 -33.427, 149.562 -33.427, 149.563 -33.427, 149.563 -33.427, 149.563 -33.427, 149.564 -33.427, 149.564 -33.428, 149.564 -33.428, 149.564 -33.428, 149.565 -33.428, 149.565 -33.428, 149.565 -33.428, 149.566 -33.428, 149.566 -33.428, 149.566 -33.428, 149.566 -33.428, 149.567 -33.429, 149.567 -33.429, 149.567 -33.429, 149.568 -33.429, 149.568 -33.429, 149.568 -33.429, 149.569 -33.429, 149.569 -33.429, 149.569 -33.429, 149.57 -33.429, 149.57 -33.429, 149.571 -33.429, 149.571 -33.429, 149.571 -33.429, 149.572 -33.429, 149.572 -33.43, 149.572 -33.43, 149.573 -33.43, 149.573 -33.43, 149.573 -33.43, 149.574 -33.43, 149.574 -33.43, 149.574 -33.43, 149.575 -33.43, 149.575 -33.43, 149.576 -33.43, 149.576 -33.43, 149.576 -33.43, 149.577 -33.43, 149.577 -33.43, 149.577 -33.43, 149.578 -33.43, 149.578 -33.43, 149.579 -33.43, 149.579 -33.43, 149.579 -33.43, 149.58 -33.43, 149.58 -33.43, 149.58 -33.43, 149.581 -33.43, 149.581 -33.43, 149.582 -33.43, 149.582 -33.43, 149.582 -33.43, 149.583 -33.43, 149.583 -33.43, 149.583 -33.43, 149.584 -33.429, 149.584 -33.429, 149.584 -33.429, 149.585 -33.429, 149.585 -33.429, 149.585 -33.429, 149.586 -33.429, 149.586 -33.429, 149.586 -33.429, 149.587 -33.429, 149.587 -33.429, 149.587 -33.429, 149.588 -33.429, 149.588 -33.429, 149.588 -33.428, 149.589 -33.428, 149.589 -33.428, 149.589 -33.428, 149.59 -33.428, 149.59 -33.428, 149.59 -33.428, 149.591 -33.428, 149.591 -33.428, 149.591 -33.428, 149.591 -33.427, 149.592 -33.427, 149.592 -33.427, 149.592 -33.427, 149.593 -33.427, 149.593 -33.427, 149.593 -33.427, 149.593 -33.427, 149.594 -33.427, 149.594 -33.426, 149.594 -33.426, 149.594 -33.426, 149.595 -33.426, 149.595 -33.426, 149.595 -33.426, 149.595 -33.426, 149.595 -33.425, 149.596 -33.425, 149.596 -33.425, 149.596 -33.425, 149.596 -33.425, 149.596 -33.425, 149.597 -33.425, 149.597 -33.424, 149.597 -33.424, 149.597 -33.424, 149.597 -33.424, 149.597 -33.424, 149.597 -33.424, 149.598 -33.423, 149.598 -33.423, 149.598 -33.423, 149.598 -33.423, 149.598 -33.423, 149.598 -33.423, 149.598 -33.422, 149.598 -33.422, 149.598 -33.422, 149.599 -33.422, 149.599 -33.422, 149.599 -33.422, 149.599 -33.421, 149.599 -33.421, 149.599 -33.421, 149.599 -33.421, 149.599 -33.421, 149.599 -33.421, 149.599 -33.42, 149.599 -33.42, 149.599 -33.42, 149.599 -33.42, 149.599 -33.42, 149.599 -33.42, 149.599 -33.419, 149.599 -33.419, 149.599 -33.419, 149.599 -33.419, 149.599 -33.419, 149.599 -33.419, 149.599 -33.418, 149.599 -33.418, 149.598 -33.418, 149.598 -33.418, 149.598 -33.418, 149.598 -33.418, 149.598 -33.417, 149.598 -33.417, 149.598 -33.417, 149.598 -33.417, 149.598 -33.417, 149.598 -33.417, 149.597 -33.416, 149.597 -33.416, 149.597 -33.416, 149.597 -33.416, 149.597 -33.416, 149.597 -33.416, 149.597 -33.415, 149.596 -33.415, 149.596 -33.415, 149.596 -33.415, 149.596 -33.415, 149.596 -33.415, 149.595 -33.415, 149.595 -33.414, 149.595 -33.414, 149.595 -33.414, 149.595 -33.414, 149.594 -33.414, 149.594 -33.414, 149.594 -33.414, 149.594 -33.413, 149.593 -33.413, 149.593 -33.413, 149.593 -33.413, 149.593 -33.413, 149.592 -33.413, 149.592 -33.413, 149.592 -33.413, 149.591 -33.413, 149.591 -33.412, 149.591 -33.412, 149.591 -33.412, 149.59 -33.412, 149.59 -33.412, 149.59 -33.412, 149.589 -33.412, 149.589 -33.412, 149.589 -33.412, 149.588 -33.412, 149.588 -33.411, 149.588 -33.411, 149.587 -33.411, 149.587 -33.411, 149.587 -33.411, 149.586 -33.411, 149.586 -33.411, 149.586 -33.411, 149.585 -33.411, 149.585 -33.411, 149.585 -33.411, 149.584 -33.411, 149.584 -33.411, 149.584 -33.411, 149.583 -33.411, 149.583 -33.41, 149.583 -33.41, 149.582 -33.41, 149.582 -33.41, 149.581 -33.41, 149.581 -33.41, 149.581 -33.41, 149.58 -33.41, 149.58 -33.41, 149.58 -33.41, 149.579 -33.41, 149.579 -33.41, 149.579 -33.41, 149.578 -33.41, 149.578 -33.41, 149.577 -33.41, 149.577 -33.41, 149.577 -33.41, 149.576 -33.41, 149.576 -33.41, 149.576 -33.41, 149.575 -33.41, 149.575 -33.41, 149.574 -33.41, 149.574 -33.41, 149.574 -33.41, 149.573 -33.41, 149.573 -33.41, 149.573 -33.41, 149.572 -33.41, 149.572 -33.411, 149.572 -33.411, 149.571 -33.411, 149.571 -33.411, 149.57 -33.411, 149.57 -33.411, 149.57 -33.411, 149.569 -33.411, 149.569 -33.411, 149.569 -33.411, 149.568 -33.411, 149.568 -33.411, 149.568 -33.411, 149.567 -33.411, 149.567 -33.411, 149.567 -33.412, 149.566 -33.412, 149.566 -33.412, 149.566 -33.412, 149.566 -33.412, 149.565 -33.412, 149.565 -33.412, 149.565 -33.412, 149.564 -33.412, 149.564 -33.412, 149.564 -33.413, 149.563 -33.413, 149.563 -33.413, 149.563 -33.413, 149.563 -33.413, 149.562 -33.413, 149.562 -33.413, 149.562 -33.413, 149.562 -33.414, 149.561 -33.414, 149.561 -33.414, 149.561 -33.414, 149.561 -33.414, 149.56 -33.414, 149.56 -33.414, 149.56 -33.414, 149.56 -33.415, 149.56 -33.415, 149.559 -33.415, 149.559 -33.415))')


if __name__ == '__main__':
    unittest.main()
