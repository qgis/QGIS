"""QGIS Unit tests for QgsCesiumUtils

From build dir, run: ctest -R QgsCesiumUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2023 by Nyall Dawson"
__date__ = "10/07/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import math
from qgis.core import QgsCesiumUtils
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsCesiumUtils(QgisTestCase):

    def test_parse_region(self):
        self.assertTrue(QgsCesiumUtils.parseRegion([]).isNull())
        # invalid length (needs 6 elements)
        self.assertTrue(QgsCesiumUtils.parseRegion([1, 2, 3, 4]).isNull())
        self.assertTrue(QgsCesiumUtils.parseRegion([1, 2, 3, 4, 5, 6, 7]).isNull())
        # not doubles
        self.assertTrue(QgsCesiumUtils.parseRegion([1, "a", 3, 4, 5, 6]).isNull())

        # valid
        box = QgsCesiumUtils.parseRegion([1.2, 2, 3, 4.6, 5.5, 6])
        self.assertAlmostEqual(box.xMinimum(), 68.75493, 2)
        self.assertAlmostEqual(box.xMaximum(), 171.887338, 2)
        self.assertAlmostEqual(box.yMinimum(), 114.591559, 2)
        self.assertAlmostEqual(box.yMaximum(), 263.56058, 2)
        self.assertEqual(box.zMinimum(), 5.5)
        self.assertEqual(box.zMaximum(), 6.0)

    def test_parse_oriented_bounding_box(self):
        box = QgsCesiumUtils.parseBox([])
        self.assertTrue(box.isNull())

        # invalid length (needs 12 elements)
        self.assertTrue(QgsCesiumUtils.parseBox([1, 2, 3, 4]).isNull())
        self.assertTrue(QgsCesiumUtils.parseBox([1, 2, 3, 4, 5, 6, 7]).isNull())
        # not doubles
        self.assertTrue(QgsCesiumUtils.parseBox([1, 2, "a", 4, 5, 6, 7]).isNull())

        # valid
        box = QgsCesiumUtils.parseBox([1, 2, 3, 10, 0, 0, 0, 20, 0, 0, 0, 30])
        self.assertEqual(box.centerX(), 1)
        self.assertEqual(box.centerY(), 2)
        self.assertEqual(box.centerZ(), 3)
        self.assertEqual(
            box.halfAxes(), [10.0, 0.0, 0.0, 0.0, 20.0, 0.0, 0.0, 0.0, 30.0]
        )

    def test_parse_sphere(self):
        sphere = QgsCesiumUtils.parseSphere([])
        self.assertTrue(sphere.isNull())

        # invalid length (needs 4 elements)
        self.assertTrue(QgsCesiumUtils.parseSphere([1, 2, 3]).isNull())
        self.assertTrue(QgsCesiumUtils.parseSphere([1, 2, 3, 4, 5, 6, 7]).isNull())
        # not doubles
        self.assertTrue(QgsCesiumUtils.parseSphere([1, 2, "a", 4]).isNull())

        # valid
        sphere = QgsCesiumUtils.parseSphere([1, 2, 3, 10])
        self.assertEqual(sphere.centerX(), 1)
        self.assertEqual(sphere.centerY(), 2)
        self.assertEqual(sphere.centerZ(), 3)
        self.assertEqual(sphere.radius(), 10)


if __name__ == "__main__":
    unittest.main()
