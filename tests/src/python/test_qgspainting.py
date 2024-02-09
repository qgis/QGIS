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

__author__ = "Nyall Dawson"
__date__ = "August 2023"
__copyright__ = "(C) 2023, Nyall Dawson"

from qgis.PyQt.QtCore import Qt, QPointF
from qgis.PyQt.QtGui import QImage, QPainter, QPolygonF

from qgis.core import (
    QgsPainting,
)

import unittest
from qgis.testing import QgisTestCase

from utilities import unitTestDataPath, start_app

TEST_DATA_DIR = unitTestDataPath()
start_app()


class TestQgsPainting(QgisTestCase):
    @classmethod
    def control_path_prefix(cls):
        return "painting"

    def test_triangle_to_triangle_transform_invalid(self):
        """
        Test triangleToTriangleTransform with an impossible transform
        """
        transform, ok = QgsPainting.triangleToTriangleTransform(
            1, 1, 2, 1, 2, 1, 10, 10, 20, 10, 20, 10
        )
        self.assertFalse(ok)

    def test_triangle_to_triangle_transform(self):
        """
        Test triangleToTriangleTransform
        """
        transform, ok = QgsPainting.triangleToTriangleTransform(
            1580,
            933,
            1632,
            902,
            1524,
            858,
            919.46,
            578.713,
            916.356,
            580.972,
            923.826,
            581.999,
        )
        self.assertTrue(ok)

        self.assertEqual(
            transform.map(1580.0, 933.0), (919.4600000000021, 578.7130000000005)
        )
        self.assertEqual(
            transform.map(1632.0, 902.0), (916.3560000000019, 580.9720000000004)
        )
        self.assertEqual(
            transform.map(1524.0, 858.0), (923.8260000000021, 581.9990000000006)
        )

    def test_draw_triangle_using_texture(self):
        """
        Test drawing a triangle using a mapped texture
        """
        texture_image = QImage(TEST_DATA_DIR + "/rgb256x256.png")
        self.assertFalse(texture_image.isNull())

        render_image = QImage(400, 400, QImage.Format.Format_ARGB32)
        render_image.fill(Qt.GlobalColor.transparent)

        painter = QPainter(render_image)
        painter.setPen(Qt.PenStyle.NoPen)

        QgsPainting.drawTriangleUsingTexture(
            painter,
            QPolygonF(
                [
                    QPointF(329, 200),
                    QPointF(180, 352),
                    QPointF(10, 88),
                    QPointF(329, 200),
                ]
            ),
            texture_image,
            129 / 255,
            10 / 255,
            180 / 255,
            152 / 255,
            88 / 255,
            210 / 255,
        )

        painter.end()

        self.assertTrue(
            self.image_check("triangle_texture", "triangle_texture", render_image)
        )


if __name__ == "__main__":
    unittest.main()
