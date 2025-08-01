"""QGIS Unit tests for QgsMathUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.core import QgsMathUtils
import unittest
from qgis.testing import QgisTestCase


class TestQgsMathUtils(QgisTestCase):

    def test_to_rationale(self):
        self.assertEqual(QgsMathUtils.doubleToRational(0.0), (0, 1))
        self.assertEqual(QgsMathUtils.doubleToRational(1), (1, 1))
        self.assertEqual(QgsMathUtils.doubleToRational(10), (10, 1))
        self.assertEqual(QgsMathUtils.doubleToRational(500), (500, 1))
        self.assertEqual(QgsMathUtils.doubleToRational(1000), (1000, 1))
        self.assertEqual(QgsMathUtils.doubleToRational(10000000), (10000000, 1))
        self.assertEqual(QgsMathUtils.doubleToRational(0.5), (1, 2))
        self.assertEqual(QgsMathUtils.doubleToRational(0.25), (1, 4))
        self.assertEqual(QgsMathUtils.doubleToRational(0.75), (3, 4))
        self.assertEqual(QgsMathUtils.doubleToRational(0.125), (1, 8))
        self.assertEqual(QgsMathUtils.doubleToRational(1 / 3), (1, 3))
        self.assertEqual(QgsMathUtils.doubleToRational(2 / 3), (2, 3))
        self.assertEqual(QgsMathUtils.doubleToRational(4 / 3), (4, 3))
        self.assertEqual(QgsMathUtils.doubleToRational(11 / 3), (11, 3))
        self.assertEqual(QgsMathUtils.doubleToRational(1 / 7), (1, 7))
        self.assertEqual(QgsMathUtils.doubleToRational(2 / 7), (2, 7))
        self.assertEqual(QgsMathUtils.doubleToRational(3 / 7), (3, 7))
        self.assertEqual(QgsMathUtils.doubleToRational(4 / 7), (4, 7))
        self.assertEqual(QgsMathUtils.doubleToRational(5 / 7), (5, 7))
        self.assertEqual(QgsMathUtils.doubleToRational(6 / 7), (6, 7))
        self.assertEqual(QgsMathUtils.doubleToRational(8 / 7), (8, 7))
        self.assertEqual(QgsMathUtils.doubleToRational(25 / 7), (25, 7))
        self.assertEqual(QgsMathUtils.doubleToRational(2 / 9), (2, 9))
        self.assertEqual(QgsMathUtils.doubleToRational(44 / 45), (44, 45))
        self.assertEqual(
            QgsMathUtils.doubleToRational(999999 / 100000), (999999, 100000)
        )
        self.assertEqual(QgsMathUtils.doubleToRational(10001 / 10000), (10001, 10000))

        self.assertEqual(QgsMathUtils.doubleToRational(1 / 10), (1, 10))
        self.assertEqual(QgsMathUtils.doubleToRational(1 / 100), (1, 100))
        self.assertEqual(QgsMathUtils.doubleToRational(1 / 500), (1, 500))
        self.assertEqual(QgsMathUtils.doubleToRational(1 / 1000), (1, 1000))
        self.assertEqual(QgsMathUtils.doubleToRational(1 / 10000000), (1, 10000000))

        self.assertEqual(QgsMathUtils.doubleToRational(-1), (-1, 1))
        self.assertEqual(QgsMathUtils.doubleToRational(-0.5), (-1, 2))
        self.assertEqual(QgsMathUtils.doubleToRational(-0.25), (-1, 4))
        self.assertEqual(QgsMathUtils.doubleToRational(-0.75), (-3, 4))
        self.assertEqual(QgsMathUtils.doubleToRational(-0.125), (-1, 8))

        self.assertEqual(QgsMathUtils.doubleToRational(3.1415926535), (103993, 33102))
        self.assertEqual(QgsMathUtils.doubleToRational(1.6180339887), (28657, 17711))


if __name__ == "__main__":
    unittest.main()
