"""QGIS Unit tests for QgsTextureAtlasGenerator

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtCore import QRect, QSize
from qgis.PyQt.QtGui import QImage, QColor
from qgis._3d import (
    QgsTextureAtlas,
    QgsTextureAtlasGenerator,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsTextureAtlasGenerator(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "texture_atlas_generator"

    def test_texture_atlas(self):
        atlas = QgsTextureAtlas()
        self.assertFalse(atlas.isValid())
        self.assertEqual(atlas.atlasSize(), QSize())
        self.assertEqual(atlas.count(), 0)
        self.assertEqual(len(atlas), 0)
        with self.assertRaises(IndexError):
            atlas.rect(0)

    def test_generation_with_rects(self):
        rects = [QRect(0, 0, 100, 50)]
        # max size too small to fit -- should fail
        atlas = QgsTextureAtlasGenerator.createFromRects(rects, 90)
        self.assertFalse(atlas.isValid())

        atlas = QgsTextureAtlasGenerator.createFromRects(rects)
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 1)
        self.assertEqual(len(atlas), 1)
        self.assertEqual(atlas.atlasSize(), QSize(100, 50))
        with self.assertRaises(IndexError):
            atlas.rect(-1)
        self.assertEqual(atlas.rect(0), QRect(0, 0, 100, 50))
        with self.assertRaises(IndexError):
            atlas.rect(1)

        rects.append(QRect(0, 0, 85, 40))

        # max size too small to fit -- should fail
        atlas = QgsTextureAtlasGenerator.createFromRects(rects, 99)
        self.assertFalse(atlas.isValid())

        atlas = QgsTextureAtlasGenerator.createFromRects(rects)
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 2)
        self.assertEqual(len(atlas), 2)
        self.assertEqual(atlas.atlasSize(), QSize(100, 90))
        with self.assertRaises(IndexError):
            atlas.rect(-1)
        self.assertEqual(atlas.rect(0), QRect(0, 0, 100, 50))
        self.assertEqual(atlas.rect(1), QRect(0, 50, 85, 40))
        with self.assertRaises(IndexError):
            atlas.rect(2)

        rects.append(QRect(0, 0, 150, 120))

        # max size too small to fit -- should fail
        atlas = QgsTextureAtlasGenerator.createFromRects(rects, 149)
        self.assertFalse(atlas.isValid())

        atlas = QgsTextureAtlasGenerator.createFromRects(rects)
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 3)
        self.assertEqual(len(atlas), 3)
        self.assertEqual(atlas.atlasSize(), QSize(185, 170))
        with self.assertRaises(IndexError):
            atlas.rect(-1)
        self.assertEqual(atlas.rect(0), QRect(0, 120, 100, 50))
        self.assertEqual(atlas.rect(1), QRect(100, 120, 85, 40))
        self.assertEqual(atlas.rect(2), QRect(0, 0, 150, 120))
        with self.assertRaises(IndexError):
            atlas.rect(3)

        rects.append(QRect(0, 0, 20, 90))

        # max size too small to fit -- should fail
        atlas = QgsTextureAtlasGenerator.createFromRects(rects, 184)
        self.assertFalse(atlas.isValid())

        atlas = QgsTextureAtlasGenerator.createFromRects(rects)
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 4)
        self.assertEqual(len(atlas), 4)
        self.assertEqual(atlas.atlasSize(), QSize(185, 170))
        with self.assertRaises(IndexError):
            atlas.rect(-1)
        self.assertEqual(atlas.rect(0), QRect(0, 120, 100, 50))
        self.assertEqual(atlas.rect(1), QRect(100, 120, 85, 40))
        self.assertEqual(atlas.rect(2), QRect(0, 0, 150, 120))
        self.assertEqual(atlas.rect(3), QRect(150, 0, 20, 90))
        with self.assertRaises(IndexError):
            atlas.rect(4)

        rects.append(QRect(0, 0, 30, 30))

        # max size too small to fit -- should fail
        atlas = QgsTextureAtlasGenerator.createFromRects(rects, 184)
        self.assertFalse(atlas.isValid())

        atlas = QgsTextureAtlasGenerator.createFromRects(rects)
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 5)
        self.assertEqual(len(atlas), 5)
        self.assertEqual(atlas.atlasSize(), QSize(185, 170))
        with self.assertRaises(IndexError):
            atlas.rect(-1)
        self.assertEqual(atlas.rect(0), QRect(0, 120, 100, 50))
        self.assertEqual(atlas.rect(1), QRect(100, 120, 85, 40))
        self.assertEqual(atlas.rect(2), QRect(0, 0, 150, 120))
        self.assertEqual(atlas.rect(3), QRect(150, 30, 20, 90))
        self.assertEqual(atlas.rect(4), QRect(150, 0, 30, 30))
        with self.assertRaises(IndexError):
            atlas.rect(5)

    def test_images(self):
        im = QImage(100, 50, QImage.Format.Format_ARGB32)
        im.fill(QColor(255, 100, 100))
        images = [im]

        atlas = QgsTextureAtlasGenerator.createFromImages(images)
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 1)
        self.assertEqual(len(atlas), 1)
        self.assertEqual(atlas.atlasSize(), QSize(100, 50))
        with self.assertRaises(IndexError):
            atlas.rect(-1)
        self.assertEqual(atlas.rect(0), QRect(0, 0, 100, 50))
        with self.assertRaises(IndexError):
            atlas.rect(1)

        self.assertTrue(
            self.image_check(
                "texture_atlas_1", "texture_atlas_1", atlas.renderAtlasTexture()
            )
        )

        im = QImage(85, 40, QImage.Format.Format_ARGB32)
        im.fill(QColor(100, 200, 100))
        images.append(im)

        atlas = QgsTextureAtlasGenerator.createFromImages(images)
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 2)
        self.assertEqual(len(atlas), 2)
        self.assertEqual(atlas.atlasSize(), QSize(100, 90))
        with self.assertRaises(IndexError):
            atlas.rect(-1)
        self.assertEqual(atlas.rect(0), QRect(0, 0, 100, 50))
        self.assertEqual(atlas.rect(1), QRect(0, 50, 85, 40))
        with self.assertRaises(IndexError):
            atlas.rect(2)

        self.assertTrue(
            self.image_check(
                "texture_atlas_2", "texture_atlas_2", atlas.renderAtlasTexture()
            )
        )

        im = QImage(150, 120, QImage.Format.Format_ARGB32)
        im.fill(QColor(100, 100, 200))
        images.append(im)

        atlas = QgsTextureAtlasGenerator.createFromImages(images)
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 3)
        self.assertEqual(len(atlas), 3)
        self.assertEqual(atlas.atlasSize(), QSize(185, 170))
        with self.assertRaises(IndexError):
            atlas.rect(-1)
        self.assertEqual(atlas.rect(0), QRect(0, 120, 100, 50))
        self.assertEqual(atlas.rect(1), QRect(100, 120, 85, 40))
        self.assertEqual(atlas.rect(2), QRect(0, 0, 150, 120))
        with self.assertRaises(IndexError):
            atlas.rect(3)

        self.assertTrue(
            self.image_check(
                "texture_atlas_3", "texture_atlas_3", atlas.renderAtlasTexture()
            )
        )

        im = QImage(20, 90, QImage.Format.Format_ARGB32)
        im.fill(QColor(100, 200, 200))
        images.append(im)

        atlas = QgsTextureAtlasGenerator.createFromImages(images)
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 4)
        self.assertEqual(len(atlas), 4)
        self.assertEqual(atlas.atlasSize(), QSize(185, 170))
        with self.assertRaises(IndexError):
            atlas.rect(-1)
        self.assertEqual(atlas.rect(0), QRect(0, 120, 100, 50))
        self.assertEqual(atlas.rect(1), QRect(100, 120, 85, 40))
        self.assertEqual(atlas.rect(2), QRect(0, 0, 150, 120))
        self.assertEqual(atlas.rect(3), QRect(150, 0, 20, 90))
        with self.assertRaises(IndexError):
            atlas.rect(4)

        self.assertTrue(
            self.image_check(
                "texture_atlas_4", "texture_atlas_4", atlas.renderAtlasTexture()
            )
        )

        im = QImage(30, 30, QImage.Format.Format_ARGB32)
        im.fill(QColor(200, 200, 100))
        images.append(im)

        atlas = QgsTextureAtlasGenerator.createFromImages(images)
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 5)
        self.assertEqual(len(atlas), 5)
        self.assertEqual(atlas.atlasSize(), QSize(185, 170))
        with self.assertRaises(IndexError):
            atlas.rect(-1)
        self.assertEqual(atlas.rect(0), QRect(0, 120, 100, 50))
        self.assertEqual(atlas.rect(1), QRect(100, 120, 85, 40))
        self.assertEqual(atlas.rect(2), QRect(0, 0, 150, 120))
        self.assertEqual(atlas.rect(3), QRect(150, 30, 20, 90))
        self.assertEqual(atlas.rect(4), QRect(150, 0, 30, 30))
        with self.assertRaises(IndexError):
            atlas.rect(5)

        self.assertTrue(
            self.image_check(
                "texture_atlas_5", "texture_atlas_5", atlas.renderAtlasTexture()
            )
        )


if __name__ == "__main__":
    unittest.main()
