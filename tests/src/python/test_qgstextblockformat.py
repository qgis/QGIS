"""QGIS Unit tests for QgsTextBlockFormat.

Run with: ctest -V -R QgsTextBlockFormat

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import math

from qgis.PyQt.QtGui import QBrush, QColor
from qgis.core import (
    Qgis,
    QgsFontUtils,
    QgsRenderContext,
    QgsTextBlockFormat,
    QgsMargins,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsTextBlockFormat(QgisTestCase):

    def setUp(self):
        QgsFontUtils.loadStandardTestFonts(["Bold", "Oblique"])

    def testGettersSetters(self):
        format = QgsTextBlockFormat()
        self.assertFalse(format.hasHorizontalAlignmentSet())
        self.assertEqual(
            format.horizontalAlignment(), Qgis.TextHorizontalAlignment.Left
        )
        self.assertTrue(math.isnan(format.lineHeight()))
        self.assertTrue(math.isnan(format.lineHeightPercentage()))
        self.assertTrue(math.isnan(format.margins().top()))
        self.assertTrue(math.isnan(format.margins().right()))
        self.assertTrue(math.isnan(format.margins().left()))
        self.assertTrue(math.isnan(format.margins().bottom()))
        self.assertFalse(format.hasBackground())

        format.setHasHorizontalAlignmentSet(True)
        self.assertTrue(format.hasHorizontalAlignmentSet())
        format.setHorizontalAlignment(Qgis.TextHorizontalAlignment.Right)
        self.assertEqual(
            format.horizontalAlignment(), Qgis.TextHorizontalAlignment.Right
        )

        format.setLineHeight(5)
        self.assertEqual(format.lineHeight(), 5)
        self.assertTrue(math.isnan(format.lineHeightPercentage()))

        format = QgsTextBlockFormat()
        format.setLineHeightPercentage(0.5)
        self.assertEqual(format.lineHeightPercentage(), 0.5)
        self.assertTrue(math.isnan(format.lineHeight()))

        format.setMargins(QgsMargins(1, 2, 3, 4))
        self.assertEqual(format.margins(), QgsMargins(1, 2, 3, 4))

        format.setBackgroundBrush(QBrush(QColor(255, 255, 0)))
        self.assertTrue(format.hasBackground())
        self.assertEqual(format.backgroundBrush().color().name(), "#ffff00")

        format = QgsTextBlockFormat()
        format.setBackgroundImagePath("test")
        self.assertTrue(format.hasBackground())
        self.assertEqual(format.backgroundImagePath(), "test")

    def testUpdateFont(self):
        context = QgsRenderContext()
        font = QgsFontUtils.getStandardTestFont()

        format = QgsTextBlockFormat()
        format.updateFontForFormat(font, context)

        # no effect for now...


if __name__ == "__main__":
    unittest.main()
