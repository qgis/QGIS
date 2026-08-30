"""QGIS Unit tests for QgsMatrixSolver.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import QgsInvalidArgumentException, QgsMatrixSolver
from qgis.testing import QgisTestCase, start_app

app = start_app()


@unittest.skipIf(not QgsMatrixSolver.isAvailable(), "GSL is not available")
class TestQgsMatrixSolver(QgisTestCase):
    def test_solve_2x2(self):
        """
        Test solving a basic 2x2 linear system.
        """
        solver = QgsMatrixSolver(2)
        solver.setValue(0, 0, 2)
        solver.setValue(0, 1, 3)
        solver.setValue(1, 0, 3)
        solver.setValue(1, 1, 1)

        solver.setRightHandSide(0, 8)
        solver.setRightHandSide(1, 5)

        success, result = solver.solve(2)

        self.assertTrue(success)
        self.assertEqual(len(result), 2)
        self.assertAlmostEqual(result[0], 1.0, places=6)
        self.assertAlmostEqual(result[1], 2.0, places=6)

    def test_dynamic_active_size(self):
        """
        Test allocating a larger matrix but solving a smaller 3x3 subset
        """
        solver = QgsMatrixSolver(5)

        solver.setValue(0, 0, 1)
        solver.setValue(0, 1, 1)
        solver.setValue(0, 2, 1)

        solver.setValue(1, 0, 0)
        solver.setValue(1, 1, 2)
        solver.setValue(1, 2, 5)

        solver.setValue(2, 0, 2)
        solver.setValue(2, 1, 5)
        solver.setValue(2, 2, -1)

        solver.setRightHandSide(0, 6)
        solver.setRightHandSide(1, -4)
        solver.setRightHandSide(2, 27)

        success, result = solver.solve(3)

        self.assertTrue(success)
        self.assertEqual(len(result), 3)
        self.assertAlmostEqual(result[0], 5.0, places=6)
        self.assertAlmostEqual(result[1], 3.0, places=6)
        self.assertAlmostEqual(result[2], -2.0, places=6)

    def test_singular_matrix(self):
        """
        Test that a singular (unsolvable) matrix gracefully returns False
        """
        solver = QgsMatrixSolver(2)

        solver.setValue(0, 0, 1)
        solver.setValue(0, 1, 1)
        solver.setValue(1, 0, 1)
        solver.setValue(1, 1, 1)

        solver.setRightHandSide(0, 2)
        solver.setRightHandSide(1, 3)

        success, result = solver.solve(2)
        self.assertFalse(success)

    def test_bounds_checking(self):
        """
        Test that out-of-bounds arguments raise QgsInvalidArgumentException
        """
        solver = QgsMatrixSolver(3)

        with self.assertRaises(IndexError):
            solver.setValue(3, 0, 1.0)

        with self.assertRaises(IndexError):
            solver.setValue(0, 3, 1.0)

        with self.assertRaises(IndexError):
            solver.setRightHandSide(3, 1.0)

        with self.assertRaises(QgsInvalidArgumentException):
            solver.solve(4)

        with self.assertRaises(QgsInvalidArgumentException):
            solver.solve(0)


if __name__ == "__main__":
    unittest.main()
