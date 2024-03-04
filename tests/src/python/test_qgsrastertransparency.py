"""QGIS Unit tests for QgsRasterTransparency

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '29/02/2024'
__copyright__ = 'Copyright 2024, The QGIS Project'

from qgis.core import QgsRasterTransparency
from qgis.testing import TestCase, unittest


class TestQgsRasterTransparency(TestCase):

    def test_transparency_single_repr(self):
        self.assertEqual(repr(QgsRasterTransparency.TransparentSingleValuePixel(1, 10, 0.3)),
                         '<QgsRasterTransparency.TransparentSingleValuePixel: 1, 10, 0.3>')

    def test_transparency_single_equality(self):
        self.assertEqual(QgsRasterTransparency.TransparentSingleValuePixel(1, 10, 0.3),
                         QgsRasterTransparency.TransparentSingleValuePixel(1, 10, 0.3))
        self.assertNotEqual(QgsRasterTransparency.TransparentSingleValuePixel(1, 10, 0.3),
                            QgsRasterTransparency.TransparentSingleValuePixel(2, 10, 0.3))
        self.assertNotEqual(QgsRasterTransparency.TransparentSingleValuePixel(1, 10, 0.3),
                            QgsRasterTransparency.TransparentSingleValuePixel(1, 11, 0.3))
        self.assertNotEqual(QgsRasterTransparency.TransparentSingleValuePixel(1, 10, 0.3),
                            QgsRasterTransparency.TransparentSingleValuePixel(1, 10, 0.4))

    def test_transparency_single_value(self):
        transparency = QgsRasterTransparency()
        self.assertFalse(transparency.transparentSingleValuePixelList())
        transparency.setTransparentSingleValuePixelList([
            QgsRasterTransparency.TransparentSingleValuePixel(10, 20, 0.3),
            QgsRasterTransparency.TransparentSingleValuePixel(30, 40, 0.6, includeMaximum=False),
            QgsRasterTransparency.TransparentSingleValuePixel(50, 60, 0.9, includeMinimum=False)
        ])
        self.assertEqual(
            transparency.transparentSingleValuePixelList(),
            [
                QgsRasterTransparency.TransparentSingleValuePixel(10, 20, 0.3),
                QgsRasterTransparency.TransparentSingleValuePixel(30, 40, 0.6, includeMaximum=False),
                QgsRasterTransparency.TransparentSingleValuePixel(50, 60, 0.9, includeMinimum=False)
            ]
        )
        self.assertEqual(transparency.alphaValue(0), 255)
        self.assertEqual(transparency.alphaValue(10), 76)
        self.assertEqual(transparency.alphaValue(15), 76)
        self.assertEqual(transparency.alphaValue(20), 76)
        self.assertEqual(transparency.alphaValue(25), 255)
        self.assertEqual(transparency.alphaValue(30), 153)
        self.assertEqual(transparency.alphaValue(35), 153)
        self.assertEqual(transparency.alphaValue(40), 255)
        self.assertEqual(transparency.alphaValue(45), 255)
        self.assertEqual(transparency.alphaValue(50), 255)
        self.assertEqual(transparency.alphaValue(55), 229)
        self.assertEqual(transparency.alphaValue(60), 229)
        self.assertEqual(transparency.alphaValue(61), 255)

        self.assertEqual(transparency.opacityForValue(0), 1.0)
        self.assertEqual(transparency.opacityForValue(10), 0.3)
        self.assertEqual(transparency.opacityForValue(15), 0.3)
        self.assertEqual(transparency.opacityForValue(20), 0.3)
        self.assertEqual(transparency.opacityForValue(25), 1.0)
        self.assertEqual(transparency.opacityForValue(30), 0.6)
        self.assertEqual(transparency.opacityForValue(35), 0.6)
        self.assertEqual(transparency.opacityForValue(40), 1.0)
        self.assertEqual(transparency.opacityForValue(45), 1.0)
        self.assertEqual(transparency.opacityForValue(50), 1.0)
        self.assertEqual(transparency.opacityForValue(55), 0.9)
        self.assertEqual(transparency.opacityForValue(60), 0.9)
        self.assertEqual(transparency.opacityForValue(61), 1.0)

    def test_transparency_three_repr(self):
        self.assertEqual(repr(QgsRasterTransparency.TransparentThreeValuePixel(1, 10, 20, 0.3)),
                         '<QgsRasterTransparency.TransparentThreeValuePixel: 1, 10, 20, 0.3>')

    def test_transparency_three_equality(self):
        self.assertEqual(QgsRasterTransparency.TransparentThreeValuePixel(1, 10, 20, 0.3),
                         QgsRasterTransparency.TransparentThreeValuePixel(1, 10, 20, 0.3))
        self.assertNotEqual(QgsRasterTransparency.TransparentThreeValuePixel(1, 10, 20, 0.3),
                            QgsRasterTransparency.TransparentThreeValuePixel(2, 10, 20, 0.3))
        self.assertNotEqual(QgsRasterTransparency.TransparentThreeValuePixel(1, 10, 20, 0.3),
                            QgsRasterTransparency.TransparentThreeValuePixel(1, 11, 20, 0.3))
        self.assertNotEqual(QgsRasterTransparency.TransparentThreeValuePixel(1, 10, 20, 0.3),
                            QgsRasterTransparency.TransparentThreeValuePixel(1, 10, 25, 0.3))
        self.assertNotEqual(QgsRasterTransparency.TransparentThreeValuePixel(1, 10, 20, 0.3),
                            QgsRasterTransparency.TransparentThreeValuePixel(1, 10, 20, 0.4))

    def test_transparency_three_value(self):
        transparency = QgsRasterTransparency()
        self.assertFalse(transparency.transparentThreeValuePixelList())
        transparency.setTransparentThreeValuePixelList([
            QgsRasterTransparency.TransparentThreeValuePixel(10, 20, 30, 0.3),
            QgsRasterTransparency.TransparentThreeValuePixel(30, 40, 50, 0.6)
        ])
        self.assertEqual(
            transparency.transparentThreeValuePixelList(),
            [
                QgsRasterTransparency.TransparentThreeValuePixel(10, 20, 30, 0.3),
                QgsRasterTransparency.TransparentThreeValuePixel(30, 40, 50, 0.6)
            ]
        )
        self.assertEqual(transparency.alphaValue(0, 0, 0), 255)
        self.assertEqual(transparency.alphaValue(10, 0, 0), 255)
        self.assertEqual(transparency.alphaValue(10, 20, 0), 255)
        self.assertEqual(transparency.alphaValue(10, 20, 30), 76)
        self.assertEqual(transparency.alphaValue(30, 40, 50), 153)
        self.assertEqual(transparency.opacityForRgbValues(0, 0, 0), 1)
        self.assertEqual(transparency.opacityForRgbValues(10, 0, 0), 1)
        self.assertEqual(transparency.opacityForRgbValues(10, 20, 0), 1)
        self.assertEqual(transparency.opacityForRgbValues(10, 20, 30), 0.3)
        self.assertEqual(transparency.opacityForRgbValues(30, 40, 50), 0.6)


if __name__ == '__main__':
    unittest.main()
