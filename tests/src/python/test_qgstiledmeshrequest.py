"""QGIS Unit tests for QgsTiledMeshRequest

From build dir, run: ctest -R QgsTiledMeshRequest -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2023 by Nyall Dawson'
__date__ = '26/07/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

import math
import qgis  # NOQA
from qgis.core import (
    QgsTiledMeshRequest,
    QgsFeedback
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsTiledMeshRequest(QgisTestCase):

    def test_basic(self):
        request = QgsTiledMeshRequest()

        request.setRequiredGeometricError(1.2)
        self.assertEqual(request.requiredGeometricError(), 1.2)

        self.assertIsNone(request.feedback())
        feedback = QgsFeedback()
        request.setFeedback(feedback)
        self.assertEqual(request.feedback(), feedback)


if __name__ == '__main__':
    unittest.main()
