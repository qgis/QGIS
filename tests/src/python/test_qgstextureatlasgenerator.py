"""QGIS Unit tests for QgsTextureAtlasGenerator

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtCore import QRect, QSize
from qgis.PyQt.QtGui import QImage, QColor
from qgis._3d import (
    QgsTextureAtlasGenerator,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsTextureAtlasGenerator(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "texture_atlas_generator"

    def test_generation_with_rects(self):
        generator = QgsTextureAtlasGenerator()
        self.assertEqual(generator.atlasSize(), QSize())

        generator.appendRect(QRect(0, 0, 100, 50))

        # max size too small to fit -- should fail
        self.assertFalse(generator.generateAtlas(90))

        self.assertTrue(generator.generateAtlas())
        self.assertEqual(generator.atlasSize(), QSize(100, 50))
        self.assertEqual(generator.rect(-1), QRect())
        self.assertEqual(generator.rect(0), QRect(0, 0, 100, 50))
        self.assertEqual(generator.rect(1), QRect())

        generator.appendRect(QRect(0, 0, 85, 40))

        # max size too small to fit -- should fail
        self.assertFalse(generator.generateAtlas(99))

        self.assertTrue(generator.generateAtlas())
        self.assertEqual(generator.atlasSize(), QSize(100, 90))
        self.assertEqual(generator.rect(-1), QRect())
        self.assertEqual(generator.rect(0), QRect(0, 0, 100, 50))
        self.assertEqual(generator.rect(1), QRect(0, 50, 85, 40))

        generator.appendRect(QRect(0, 0, 150, 120))

        # max size too small to fit -- should fail
        self.assertFalse(generator.generateAtlas(149))

        self.assertTrue(generator.generateAtlas())
        self.assertEqual(generator.atlasSize(), QSize(185, 170))
        self.assertEqual(generator.rect(-1), QRect())
        self.assertEqual(generator.rect(0), QRect(0, 120, 100, 50))
        self.assertEqual(generator.rect(1), QRect(100, 120, 85, 40))
        self.assertEqual(generator.rect(2), QRect(0, 0, 150, 120))

        generator.appendRect(QRect(0, 0, 20, 90))

        # max size too small to fit -- should fail
        self.assertFalse(generator.generateAtlas(184))

        self.assertTrue(generator.generateAtlas(185))
        self.assertEqual(generator.atlasSize(), QSize(185, 170))
        self.assertEqual(generator.rect(-1), QRect())
        self.assertEqual(generator.rect(0), QRect(0, 120, 100, 50))
        self.assertEqual(generator.rect(1), QRect(100, 120, 85, 40))
        self.assertEqual(generator.rect(2), QRect(0, 0, 150, 120))
        self.assertEqual(generator.rect(3), QRect(150, 0, 20, 90))

        generator.appendRect(QRect(0, 0, 30, 30))

        # max size too small to fit -- should fail
        self.assertFalse(generator.generateAtlas(184))

        self.assertTrue(generator.generateAtlas())
        self.assertEqual(generator.atlasSize(), QSize(185, 170))
        self.assertEqual(generator.rect(-1), QRect())
        self.assertEqual(generator.rect(0), QRect(0, 120, 100, 50))
        self.assertEqual(generator.rect(1), QRect(100, 120, 85, 40))
        self.assertEqual(generator.rect(2), QRect(0, 0, 150, 120))
        self.assertEqual(generator.rect(3), QRect(150, 30, 20, 90))
        self.assertEqual(generator.rect(4), QRect(150, 0, 30, 30))

    def test_images(self):
        generator = QgsTextureAtlasGenerator()
        self.assertEqual(generator.atlasSize(), QSize())

        im = QImage(100, 50, QImage.Format.Format_ARGB32)
        im.fill(QColor(255, 100, 100))
        generator.appendImage(im)

        self.assertTrue(generator.generateAtlas())
        self.assertEqual(generator.atlasSize(), QSize(100, 50))
        self.assertEqual(generator.rect(-1), QRect())
        self.assertEqual(generator.rect(0), QRect(0, 0, 100, 50))
        self.assertEqual(generator.rect(1), QRect())

        atlas = generator.atlasTexture()
        self.assertTrue(self.image_check("texture_atlas_1", "texture_atlas_1", atlas))

        im = QImage(85, 40, QImage.Format.Format_ARGB32)
        im.fill(QColor(100, 200, 100))
        generator.appendImage(im)

        self.assertTrue(generator.generateAtlas())
        self.assertEqual(generator.atlasSize(), QSize(100, 90))
        self.assertEqual(generator.rect(-1), QRect())
        self.assertEqual(generator.rect(0), QRect(0, 0, 100, 50))
        self.assertEqual(generator.rect(1), QRect(0, 50, 85, 40))

        atlas = generator.atlasTexture()
        self.assertTrue(self.image_check("texture_atlas_2", "texture_atlas_2", atlas))

        im = QImage(150, 120, QImage.Format.Format_ARGB32)
        im.fill(QColor(100, 100, 200))
        generator.appendImage(im)

        self.assertTrue(generator.generateAtlas())
        self.assertEqual(generator.atlasSize(), QSize(185, 170))
        self.assertEqual(generator.rect(-1), QRect())
        self.assertEqual(generator.rect(0), QRect(0, 120, 100, 50))
        self.assertEqual(generator.rect(1), QRect(100, 120, 85, 40))
        self.assertEqual(generator.rect(2), QRect(0, 0, 150, 120))

        atlas = generator.atlasTexture()
        self.assertTrue(self.image_check("texture_atlas_3", "texture_atlas_3", atlas))

        im = QImage(20, 90, QImage.Format.Format_ARGB32)
        im.fill(QColor(100, 200, 200))
        generator.appendImage(im)

        self.assertTrue(generator.generateAtlas())
        self.assertEqual(generator.atlasSize(), QSize(185, 170))
        self.assertEqual(generator.rect(-1), QRect())
        self.assertEqual(generator.rect(0), QRect(0, 120, 100, 50))
        self.assertEqual(generator.rect(1), QRect(100, 120, 85, 40))
        self.assertEqual(generator.rect(2), QRect(0, 0, 150, 120))
        self.assertEqual(generator.rect(3), QRect(150, 0, 20, 90))

        atlas = generator.atlasTexture()
        self.assertTrue(self.image_check("texture_atlas_4", "texture_atlas_4", atlas))

        im = QImage(30, 30, QImage.Format.Format_ARGB32)
        im.fill(QColor(200, 200, 100))
        generator.appendImage(im)

        self.assertTrue(generator.generateAtlas())
        self.assertEqual(generator.atlasSize(), QSize(185, 170))
        self.assertEqual(generator.rect(-1), QRect())
        self.assertEqual(generator.rect(0), QRect(0, 120, 100, 50))
        self.assertEqual(generator.rect(1), QRect(100, 120, 85, 40))
        self.assertEqual(generator.rect(2), QRect(0, 0, 150, 120))
        self.assertEqual(generator.rect(3), QRect(150, 30, 20, 90))
        self.assertEqual(generator.rect(4), QRect(150, 0, 30, 30))

        atlas = generator.atlasTexture()
        self.assertTrue(self.image_check("texture_atlas_5", "texture_atlas_5", atlas))


if __name__ == "__main__":
    unittest.main()
