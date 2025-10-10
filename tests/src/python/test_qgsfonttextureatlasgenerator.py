"""QGIS Unit tests for QgsFontTextureAtlasGenerator

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtCore import QRect, QSize, QPoint
from qgis.PyQt.QtGui import QImage, QColor
from qgis.core import QgsTextFormat
from qgis._3d import (
    QgsFontTextureAtlas,
    QgsFontTextureAtlasGenerator,
)
import unittest
from utilities import getTestFont

from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsFontTextureAtlasGenerator(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "texture_atlas_generator"

    def test_texture_atlas(self):
        atlas = QgsFontTextureAtlas()
        self.assertFalse(atlas.isValid())
        self.assertEqual(atlas.atlasSize(), QSize())
        self.assertEqual(atlas.count(), 0)
        self.assertEqual(len(atlas), 0)
        with self.assertRaises(KeyError):
            atlas.rect("x")

    def test_atlas(self):
        text_format = QgsTextFormat.fromQFont(getTestFont("Bold"))
        text_format.setColor(QColor(255, 0, 0))
        text_format.setSize(20)

        atlas = QgsFontTextureAtlasGenerator.create(text_format, ["H"])
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 1)
        self.assertEqual(len(atlas), 1)
        self.assertEqual(atlas.atlasSize(), QSize(23, 24))
        self.assertEqual(atlas.graphemeCount("H"), 1)
        with self.assertRaises(KeyError):
            atlas.rect("x")
        self.assertEqual(atlas.rect("H"), QRect(0, 0, 23, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("H", 0), QPoint(0, 0))
        self.assertEqual(atlas.textureRectForGrapheme("H", 0), QRect(0, 0, 23, 24))

        self.assertTrue(
            self.image_check(
                "font_texture_atlas_h",
                "font_texture_atlas_h",
                atlas.renderAtlasTexture(),
            )
        )

        atlas = QgsFontTextureAtlasGenerator.create(text_format, ["HY"])
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 2)
        self.assertEqual(len(atlas), 2)
        self.assertEqual(atlas.atlasSize(), QSize(25, 48))
        self.assertEqual(atlas.graphemeCount("HY"), 2)
        with self.assertRaises(KeyError):
            atlas.rect("x")
        self.assertEqual(atlas.rect("Y"), QRect(0, 0, 25, 24))
        self.assertEqual(atlas.rect("H"), QRect(0, 24, 23, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("HY", 0), QPoint(0, 0))
        self.assertEqual(atlas.textureRectForGrapheme("HY", 0), QRect(0, 24, 23, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("HY", 1), QPoint(23, 0))
        self.assertEqual(atlas.textureRectForGrapheme("HY", 1), QRect(0, 0, 25, 24))

        self.assertTrue(
            self.image_check(
                "font_texture_atlas_hy",
                "font_texture_atlas_hy",
                atlas.renderAtlasTexture(),
            )
        )

        atlas = QgsFontTextureAtlasGenerator.create(text_format, ["HY", "YH"])
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 2)
        self.assertEqual(len(atlas), 2)
        self.assertEqual(atlas.atlasSize(), QSize(25, 48))
        self.assertEqual(atlas.graphemeCount("HY"), 2)
        self.assertEqual(atlas.graphemeCount("YH"), 2)
        with self.assertRaises(KeyError):
            atlas.rect("x")
        self.assertEqual(atlas.rect("Y"), QRect(0, 0, 25, 24))
        self.assertEqual(atlas.rect("H"), QRect(0, 24, 23, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("HY", 0), QPoint(0, 0))
        self.assertEqual(atlas.textureRectForGrapheme("HY", 0), QRect(0, 24, 23, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("HY", 1), QPoint(23, 0))
        self.assertEqual(atlas.textureRectForGrapheme("HY", 1), QRect(0, 0, 25, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("YH", 0), QPoint(0, 0))
        self.assertEqual(atlas.textureRectForGrapheme("YH", 0), QRect(0, 0, 25, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("YH", 1), QPoint(20, 0))
        self.assertEqual(atlas.textureRectForGrapheme("YH", 1), QRect(0, 24, 23, 24))

        self.assertTrue(
            self.image_check(
                "font_texture_atlas_hy",
                "font_texture_atlas_hy",
                atlas.renderAtlasTexture(),
            )
        )

        atlas = QgsFontTextureAtlasGenerator.create(text_format, ["HY", "Hi"])
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 3)
        self.assertEqual(len(atlas), 3)
        self.assertEqual(atlas.atlasSize(), QSize(32, 48))
        self.assertEqual(atlas.graphemeCount("HY"), 2)
        self.assertEqual(atlas.graphemeCount("Hi"), 2)
        with self.assertRaises(KeyError):
            atlas.rect("x")
        self.assertEqual(atlas.rect("Y"), QRect(0, 0, 25, 24))
        self.assertEqual(atlas.rect("H"), QRect(0, 24, 23, 24))
        self.assertEqual(atlas.rect("i"), QRect(23, 24, 9, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("HY", 0), QPoint(0, 0))
        self.assertEqual(atlas.textureRectForGrapheme("HY", 0), QRect(0, 24, 23, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("HY", 1), QPoint(23, 0))
        self.assertEqual(atlas.textureRectForGrapheme("HY", 1), QRect(0, 0, 25, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("Hi", 0), QPoint(0, 0))
        self.assertEqual(atlas.textureRectForGrapheme("Hi", 0), QRect(0, 24, 23, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("Hi", 1), QPoint(23, 0))
        self.assertEqual(atlas.textureRectForGrapheme("Hi", 1), QRect(23, 24, 9, 24))

        self.assertTrue(
            self.image_check(
                "font_texture_atlas_hyi",
                "font_texture_atlas_hyi",
                atlas.renderAtlasTexture(),
            )
        )

        atlas = QgsFontTextureAtlasGenerator.create(text_format, ["HY", "hi"])
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 4)
        self.assertEqual(len(atlas), 4)
        self.assertEqual(atlas.atlasSize(), QSize(43, 48))
        self.assertEqual(atlas.graphemeCount("HY"), 2)
        self.assertEqual(atlas.graphemeCount("hi"), 2)
        with self.assertRaises(KeyError):
            atlas.rect("x")
        self.assertEqual(atlas.rect("Y"), QRect(0, 0, 25, 24))
        self.assertEqual(atlas.rect("H"), QRect(0, 24, 23, 24))
        self.assertEqual(atlas.rect("i"), QRect(25, 0, 9, 24))
        self.assertEqual(atlas.rect("h"), QRect(23, 24, 20, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("HY", 0), QPoint(0, 0))
        self.assertEqual(atlas.textureRectForGrapheme("HY", 0), QRect(0, 24, 23, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("HY", 1), QPoint(23, 0))
        self.assertEqual(atlas.textureRectForGrapheme("HY", 1), QRect(0, 0, 25, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("hi", 0), QPoint(0, 0))
        self.assertEqual(atlas.textureRectForGrapheme("hi", 0), QRect(23, 24, 20, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("hi", 1), QPoint(19, 0))
        self.assertEqual(atlas.textureRectForGrapheme("hi", 1), QRect(25, 0, 9, 24))

        self.assertTrue(
            self.image_check(
                "font_texture_atlas_hyih",
                "font_texture_atlas_hyih",
                atlas.renderAtlasTexture(),
            )
        )

        atlas = QgsFontTextureAtlasGenerator.create(text_format, ["HY", "hig"])
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 5)
        self.assertEqual(len(atlas), 5)
        self.assertEqual(atlas.atlasSize(), QSize(50, 49))
        self.assertEqual(atlas.graphemeCount("HY"), 2)
        self.assertEqual(atlas.graphemeCount("hig"), 3)
        with self.assertRaises(KeyError):
            atlas.rect("x")
        self.assertEqual(atlas.rect("Y"), QRect(0, 0, 25, 24))
        self.assertEqual(atlas.rect("H"), QRect(25, 0, 23, 24))
        self.assertEqual(atlas.rect("i"), QRect(41, 24, 9, 24))
        self.assertEqual(atlas.rect("h"), QRect(21, 24, 20, 24))
        self.assertEqual(atlas.rect("g"), QRect(0, 24, 21, 25))
        self.assertEqual(atlas.pixelOffsetForGrapheme("HY", 0), QPoint(0, 0))
        self.assertEqual(atlas.textureRectForGrapheme("HY", 0), QRect(25, 0, 23, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("HY", 1), QPoint(23, 0))
        self.assertEqual(atlas.textureRectForGrapheme("HY", 1), QRect(0, 0, 25, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("hig", 0), QPoint(0, 0))
        self.assertEqual(atlas.textureRectForGrapheme("hig", 0), QRect(21, 24, 20, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("hig", 1), QPoint(19, 0))
        self.assertEqual(atlas.textureRectForGrapheme("hig", 1), QRect(41, 24, 9, 24))
        self.assertEqual(atlas.pixelOffsetForGrapheme("hig", 2), QPoint(28, -6))
        self.assertEqual(atlas.textureRectForGrapheme("hig", 2), QRect(0, 24, 21, 25))

        self.assertTrue(
            self.image_check(
                "font_texture_atlas_hyihg",
                "font_texture_atlas_hyihg",
                atlas.renderAtlasTexture(),
            )
        )

    def test_graphemes(self):
        text_format = QgsTextFormat.fromQFont(getTestFont("Bold"))
        text_format.setColor(QColor(255, 0, 0))
        text_format.setSize(20)

        atlas = QgsFontTextureAtlasGenerator.create(text_format, ["ស្ត្រីល្"])
        self.assertTrue(atlas.isValid())
        self.assertEqual(atlas.count(), 2)
        self.assertEqual(len(atlas), 2)
        self.assertEqual(atlas.graphemeCount("ស្ត្រីល្"), 2)
        with self.assertRaises(KeyError):
            atlas.rect("x")
        self.assertEqual(atlas.textureRectForGrapheme("ស្ត្រីល្", 0), atlas.rect("ស្ត្រី"))
        self.assertEqual(atlas.textureRectForGrapheme("ស្ត្រីល្", 1), atlas.rect("ល្"))


if __name__ == "__main__":
    unittest.main()
