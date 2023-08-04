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

import unittest

import qgis  # NOQA
from qgis.core import (
    Qgis,
    QgsTiledMeshRequest,
    QgsFeedback,
    QgsOrientedBox3D
)
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsTiledMeshRequest(QgisTestCase):

    def test_basic(self):
        request = QgsTiledMeshRequest()

        self.assertEqual(request.flags(),
                         Qgis.TiledMeshRequestFlags())
        request.setFlags(
            Qgis.TiledMeshRequestFlags(Qgis.TiledMeshRequestFlag.NoHierarchyFetch)
        )
        self.assertEqual(request.flags(),
                         Qgis.TiledMeshRequestFlags(
                             Qgis.TiledMeshRequestFlag.NoHierarchyFetch
        ))

        request.setRequiredGeometricError(1.2)
        self.assertEqual(request.requiredGeometricError(), 1.2)

        self.assertIsNone(request.feedback())
        feedback = QgsFeedback()
        request.setFeedback(feedback)
        self.assertEqual(request.feedback(), feedback)

        request.setFilterBox(
            QgsOrientedBox3D([1, 2, 3], [1, 0, 0, 0, 2, 0, 0, 0, 3])
        )
        self.assertEqual(
            request.filterBox(),
            QgsOrientedBox3D([1, 2, 3], [1, 0, 0, 0, 2, 0, 0, 0, 3])
        )

        self.assertFalse(request.parentTileId())
        request.setParentTileId('parent')
        self.assertEqual(request.parentTileId(), 'parent')


if __name__ == '__main__':
    unittest.main()
