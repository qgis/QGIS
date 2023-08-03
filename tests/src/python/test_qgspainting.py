"""
***************************************************************************
    test_qgspainting.py
    ---------------------
    Date                 : August 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'August 2023'
__copyright__ = '(C) 2023, Nyall Dawson'

import os

import qgis  # NOQA
from qgis.PyQt.QtCore import QSize
from qgis.PyQt.QtGui import QColor, QPainter
from qgis.core import (
    Qgis,
    QgsMapSettings,
    QgsMultiBandColorRenderer,
    QgsMultiRenderChecker,
    QgsPainting,
    QgsProject,
    QgsRasterLayer,
    QgsRectangle,
    QgsVectorLayer,
    QgsVectorSimplifyMethod,
)
from qgis.testing import unittest
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


class TestQgsPainting(unittest.TestCase):

    def test_triangle_to_triangle_transform_invalid(self):
        """
        Test triangleToTriangleTransform with an impossible transform
        """
        transform, ok = QgsPainting.triangleToTriangleTransform(
            1, 1, 2, 1, 2, 1,
            10, 10, 20, 10, 20, 10
        )
        self.assertFalse(ok)

    def test_triangle_to_triangle_transform(self):
        """
        Test triangleToTriangleTransform
        """
        transform, ok = QgsPainting.triangleToTriangleTransform(
            1580, 933,
            1632, 902,
            1524, 858,
            919.46, 578.713,
            916.356, 580.972,
            923.826, 581.999
        )
        self.assertTrue(ok)

        self.assertEqual(transform.map(1580.0, 933.0),
                         (919.4600000000021, 578.7130000000005))
        self.assertEqual(transform.map(1632.0, 902.0),
                         (916.3560000000019, 580.9720000000004))
        self.assertEqual(transform.map(1524.0, 858.0),
                         (923.8260000000021, 581.9990000000006))


if __name__ == '__main__':
    unittest.main()
