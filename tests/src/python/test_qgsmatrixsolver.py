"""QGIS Unit tests for QgsMatrixSolver.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import Qgis, QgsInvalidArgumentException, QgsMatrixSolver
from qgis.testing import QgisTestCase, start_app

app = start_app()


@unittest.skipIf(not QgsMatrixSolver.isAvailable(), "GSL is not available")
class TestQgsMatrixSolver(QgisTestCase):
    def test_solve_2x2_lu(self):
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

        success, result = solver.solve(2, Qgis.LinearMatrixMethod.Lu)

        self.assertTrue(success)
        self.assertEqual(len(result), 2)
        self.assertAlmostEqual(result[0], 1.0, places=6)
        self.assertAlmostEqual(result[1], 2.0, places=6)

    def test_solve_2x2_svd(self):
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

        success, result = solver.solve(2, Qgis.LinearMatrixMethod.Svd)

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

    def test_singular_matrix_lu(self):
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

        success, result = solver.solve(2, Qgis.LinearMatrixMethod.Lu)
        self.assertFalse(success)

    def test_singular_matrix_svd(self):
        """
        Test that a singular matrix using SVD works
        """
        solver = QgsMatrixSolver(2)

        solver.setValue(0, 0, 1)
        solver.setValue(0, 1, 1)
        solver.setValue(1, 0, 1)
        solver.setValue(1, 1, 1)

        solver.setRightHandSide(0, 2)
        solver.setRightHandSide(1, 3)

        success, result = solver.solve(2, Qgis.LinearMatrixMethod.Svd)
        self.assertTrue(success)
        self.assertEqual(len(result), 2)
        self.assertAlmostEqual(result[0], 1.25, places=2)
        self.assertAlmostEqual(result[1], 1.25, places=2)

        # using the fallback mode should also work for this matrix
        solver = QgsMatrixSolver(2)

        solver.setValue(0, 0, 1)
        solver.setValue(0, 1, 1)
        solver.setValue(1, 0, 1)
        solver.setValue(1, 1, 1)

        solver.setRightHandSide(0, 2)
        solver.setRightHandSide(1, 3)

        success, result = solver.solve(2, Qgis.LinearMatrixMethod.LuWithSvdFallback)
        self.assertTrue(success)
        self.assertEqual(len(result), 2)
        self.assertAlmostEqual(result[0], 1.25, places=2)
        self.assertAlmostEqual(result[1], 1.25, places=2)

    def test_collinear_tps_matrix(self):
        """
        Test solving a 6x6 singular matrix
        LU decomposition will fail due to duplicate rows, while SVD and LuWithSvdFallback must succeed.
        """
        matrix_a = [
            [0.000158025, 8.31777, 0.693147, 1.0, 1.0, 1.0],
            [8.31777, 0.000158025, 0.693147, 1.0, 3.0, 3.0],
            [0.693147, 0.693147, 0.000158025, 1.0, 2.0, 2.0],
            [1.0, 1.0, 1.0, 0.0, 0.0, 0.0],
            [1.0, 3.0, 2.0, 0.0, 0.0, 0.0],
            [1.0, 3.0, 2.0, 0.0, 0.0, 0.0],
        ]
        vector_b = [3.0, 0.0, 2.0, 0.0, 0.0, 0.0]

        # LU decomposition should return False on singular matrix
        solver_lu = QgsMatrixSolver(6)
        for r in range(6):
            solver_lu.setRightHandSide(r, vector_b[r])
            for c in range(6):
                solver_lu.setValue(r, c, matrix_a[r][c])

        success_lu, _ = solver_lu.solve(6, Qgis.LinearMatrixMethod.Lu)
        self.assertFalse(success_lu)

        # SVD mode should solve pseudo-inverse successfully
        solver_svd = QgsMatrixSolver(6)
        for r in range(6):
            solver_svd.setRightHandSide(r, vector_b[r])
            for c in range(6):
                solver_svd.setValue(r, c, matrix_a[r][c])

        success_svd, result_svd = solver_svd.solve(6, Qgis.LinearMatrixMethod.Svd)
        self.assertTrue(success_svd)
        self.assertEqual(len(result_svd), 6)

        # LuWithSvdFallback should fail LU, trigger SVD fallback, and match SVD result
        solver_fallback = QgsMatrixSolver(6)
        for r in range(6):
            solver_fallback.setRightHandSide(r, vector_b[r])
            for c in range(6):
                solver_fallback.setValue(r, c, matrix_a[r][c])

        success_fb, result_fb = solver_fallback.solve(
            6, Qgis.LinearMatrixMethod.LuWithSvdFallback
        )
        self.assertTrue(success_fb)
        self.assertEqual(len(result_fb), 6)
        for i in range(6):
            self.assertAlmostEqual(result_fb[i], result_svd[i], places=6)

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
