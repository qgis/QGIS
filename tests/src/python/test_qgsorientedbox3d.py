"""QGIS Unit tests for QgsOrientedBox3D

From build dir, run: ctest -R QgsOrientedBox3D -V

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
    QgsOrientedBox3D
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
        self.assertEqual(box.halfAxes(), [10.0, 0.0, 0.0, 0.0, 20.0, 0.0, 0.0, 0.0, 30.0])

        box = QgsOrientedBox3D([1, 2, 3], [1, 0, 0, 0, 1, 0, 0, 0, 1])
        self.assertEqual(box.centerX(), 1)
        self.assertEqual(box.centerY(), 2)
        self.assertEqual(box.centerZ(), 3)
        self.assertEqual(box.halfAxes(), [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0])

        # 45 degree y axis rotation
        box = QgsOrientedBox3D([1, 2, 3],
                                     [math.cos(math.pi / 4), 0, math.sin(math.pi / 4),
                                      0, 1, 0,
                                      -math.sin(math.pi / 4), 0, math.cos(math.pi / 4)])
        self.assertEqual(box.centerX(), 1)
        self.assertEqual(box.centerY(), 2)
        self.assertEqual(box.centerZ(), 3)
        self.assertEqual(box.halfAxes(), [0.7071067811865476, 0.0, 0.7071067811865475, 0.0, 1.0, 0.0, -0.7071067811865475, 0.0, 0.7071067811865476])

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
        box = QgsOrientedBox3D([1, 2, 3],
                                     [math.cos(math.pi / 4), 0, math.sin(math.pi / 4),
                                      0, 1, 0,
                                      -math.sin(math.pi / 4), 0, math.cos(math.pi / 4)])
        bounds = box.extent()
        self.assertAlmostEqual(bounds.xMinimum(), 1 - math.sqrt(2), 5)
        self.assertAlmostEqual(bounds.xMaximum(), 1 + math.sqrt(2), 5)
        self.assertAlmostEqual(bounds.yMinimum(), 1, 5)
        self.assertAlmostEqual(bounds.yMaximum(), 3, 5)
        self.assertAlmostEqual(bounds.zMinimum(), 3 - math.sqrt(2), 5)
        self.assertAlmostEqual(bounds.zMaximum(), 3 + math.sqrt(2), 5)


if __name__ == '__main__':
    unittest.main()
