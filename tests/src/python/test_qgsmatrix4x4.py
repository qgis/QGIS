"""QGIS Unit tests for QgsMatrix4x4

From build dir, run: ctest -R QgsMatrix4x4 -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2023 by Martin Dobias"
__date__ = "18/07/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

from qgis.core import (
    Qgis,
    QgsMatrix4x4,
    QgsVector3D,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsMatrix4x4(QgisTestCase):

    def test_basic(self):

        m0 = QgsMatrix4x4()
        self.assertEqual(
            m0.data(),
            [
                1.0,
                0.0,
                0.0,
                0.0,
                0.0,
                1.0,
                0.0,
                0.0,
                0.0,
                0.0,
                1.0,
                0.0,
                0.0,
                0.0,
                0.0,
                1.0,
            ],
        )

        m1 = QgsMatrix4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0, 0, 0, 1)
        self.assertEqual(m1.data(), [1, 5, 9, 0, 2, 6, 10, 0, 3, 7, 11, 0, 4, 8, 12, 1])

        self.assertEqual(m1.map(QgsVector3D(10, 20, 30)), QgsVector3D(144, 388, 632))
        self.assertEqual(m1 * QgsVector3D(10, 20, 30), QgsVector3D(144, 388, 632))

        m2 = QgsMatrix4x4(2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1)
        m3 = m1 * m2
        self.assertEqual(
            m3.data(), [2, 10, 18, 0, 4, 12, 20, 0, 6, 14, 22, 0, 4, 8, 12, 1]
        )

    def test_repr(self):
        self.assertEqual(
            str(QgsMatrix4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)),
            "<QgsMatrix4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)>",
        )

    def test_equality(self):
        self.assertEqual(
            QgsMatrix4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16),
            QgsMatrix4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16),
        )
        for i in range(16):
            self.assertNotEqual(
                QgsMatrix4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16),
                QgsMatrix4x4(
                    *[
                        j if k != i else 0
                        for k, j in enumerate(
                            [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
                        )
                    ]
                ),
            )

    def test_translate(self):
        """
        Test translating a matrix
        """
        m = QgsMatrix4x4()
        m.translate(QgsVector3D(1, 2, 3))
        self.assertEqual(
            m.data(),
            [
                1.0,
                0.0,
                0.0,
                0.0,
                0.0,
                1.0,
                0.0,
                0.0,
                0.0,
                0.0,
                1.0,
                0.0,
                1.0,
                2.0,
                3.0,
                1.0,
            ],
        )

        m = QgsMatrix4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0, 0, 0, 1)

        m.translate(QgsVector3D(1, 2, 3))
        self.assertEqual(
            m.data(),
            [
                1.0,
                5.0,
                9.0,
                0.0,
                2.0,
                6.0,
                10.0,
                0.0,
                3.0,
                7.0,
                11.0,
                0.0,
                18.0,
                46.0,
                74.0,
                1.0,
            ],
        )


if __name__ == "__main__":
    unittest.main()
