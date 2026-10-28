"""QGIS Unit tests for QgsImageOperation

From build dir, run: ctest -R QgsImageOperation -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.core import QgsImageOperation
from qgis.PyQt.QtGui import QColor, QImage
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsImageOperation(unittest.TestCase):
    def test_is_blank_image(self):
        # null image
        null_img = QImage()
        self.assertTrue(QgsImageOperation.isBlankImage(null_img))

        # image with no alpha channel - should return False regardless of content
        rgb_img = QImage(10, 10, QImage.Format.Format_RGB32)
        rgb_img.fill(QColor(0, 0, 0))
        self.assertFalse(QgsImageOperation.isBlankImage(rgb_img))

        # ARGB32 with even dimensions
        argb_blank = QImage(4, 4, QImage.Format.Format_ARGB32)
        argb_blank.fill(QColor(0, 0, 0, 0))
        self.assertTrue(QgsImageOperation.isBlankImage(argb_blank))

        argb_non_blank = QImage(4, 4, QImage.Format.Format_ARGB32)
        argb_non_blank.fill(QColor(0, 0, 0, 0))
        # a pixel with alpha 0, but non-zero rgb
        argb_non_blank.setPixelColor(2, 2, QColor(255, 0, 0, 255))
        self.assertFalse(QgsImageOperation.isBlankImage(argb_non_blank))

        # ARGB32 with odd pixel count
        argb_odd_blank = QImage(3, 3, QImage.Format.Format_ARGB32)
        argb_odd_blank.fill(QColor(0, 0, 0, 0))
        self.assertTrue(QgsImageOperation.isBlankImage(argb_odd_blank))

        argb_odd_non_blank = QImage(3, 3, QImage.Format.Format_ARGB32)
        argb_odd_non_blank.fill(QColor(0, 0, 0, 0))
        argb_odd_non_blank.setPixelColor(2, 2, QColor(0, 255, 0, 128))
        self.assertFalse(QgsImageOperation.isBlankImage(argb_odd_non_blank))

        # non ARGB32 formats
        alpha8_blank = QImage(8, 8, QImage.Format.Format_Alpha8)
        alpha8_blank.fill(0)
        self.assertTrue(QgsImageOperation.isBlankImage(alpha8_blank))

        alpha8_non_blank = QImage(8, 8, QImage.Format.Format_Alpha8)
        alpha8_non_blank.fill(0)
        alpha8_non_blank.setPixelColor(0, 0, QColor(0, 0, 0, 255))
        self.assertFalse(QgsImageOperation.isBlankImage(alpha8_non_blank))

        rgba64_blank = QImage(2, 2, QImage.Format.Format_RGBA64)
        rgba64_blank.fill(QColor(0, 0, 0, 0))
        self.assertTrue(QgsImageOperation.isBlankImage(rgba64_blank))

        rgba64_non_blank = QImage(2, 2, QImage.Format.Format_RGBA64)
        rgba64_non_blank.fill(QColor(0, 0, 0, 0))
        rgba64_non_blank.setPixelColor(0, 1, QColor(255, 255, 255, 255))
        self.assertFalse(QgsImageOperation.isBlankImage(rgba64_non_blank))

    def test_is_single_color_image(self):
        target_color = QColor(255, 128, 64, 200)

        # null image - return value here isn't so important, we just don't
        # want to crash
        QgsImageOperation.isSingleColor(QImage(), target_color)

        # ARGB32 format
        img_argb = QImage(4, 4, QImage.Format.Format_ARGB32)
        img_argb.fill(target_color)
        self.assertTrue(QgsImageOperation.isSingleColor(img_argb, target_color))
        self.assertFalse(
            QgsImageOperation.isSingleColor(img_argb, QColor(255, 128, 64, 255))
        )

        # set a single pixel mismatch
        img_argb_odd = QImage(3, 3, QImage.Format.Format_ARGB32)
        img_argb_odd.fill(target_color)
        img_argb_odd.setPixelColor(2, 2, QColor(0, 0, 0, 0))
        self.assertFalse(QgsImageOperation.isSingleColor(img_argb_odd, target_color))

        # RGB32 format
        rgb_color = QColor(100, 150, 200)
        img_rgb = QImage(5, 5, QImage.Format.Format_RGB32)
        img_rgb.fill(rgb_color)
        self.assertTrue(QgsImageOperation.isSingleColor(img_rgb, rgb_color))

        img_rgb.fill(rgb_color)
        img_rgb.setPixelColor(2, 2, QColor(0, 0, 0, 0))
        self.assertFalse(QgsImageOperation.isSingleColor(img_rgb, rgb_color))

        # ARGB32 Premultiplied format
        img_premul = QImage(4, 4, QImage.Format.Format_ARGB32_Premultiplied)
        img_premul.fill(target_color)
        self.assertTrue(QgsImageOperation.isSingleColor(img_premul, target_color))
        img_premul.setPixelColor(2, 2, QColor(0, 0, 0, 0))
        self.assertFalse(QgsImageOperation.isSingleColor(img_premul, target_color))
        img_premul.fill(QColor(125, 200, 100, 50))
        self.assertTrue(
            QgsImageOperation.isSingleColor(img_premul, QColor(125, 200, 100, 50))
        )
        img_premul.setPixelColor(2, 2, QColor(125, 200, 100, 150))
        self.assertFalse(
            QgsImageOperation.isSingleColor(img_premul, QColor(125, 200, 100, 50))
        )

        # other formats - should fallback to non-optimized path
        img_rgba8888 = QImage(6, 6, QImage.Format.Format_RGBA8888)
        img_rgba8888.fill(target_color)
        self.assertTrue(QgsImageOperation.isSingleColor(img_rgba8888, target_color))

        img_rgba64 = QImage(2, 2, QImage.Format.Format_RGBA64)
        img_rgba64.fill(target_color)
        self.assertTrue(QgsImageOperation.isSingleColor(img_rgba64, target_color))


if __name__ == "__main__":
    unittest.main()
