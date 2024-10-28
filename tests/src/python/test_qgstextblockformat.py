"""QGIS Unit tests for QgsTextBlockFormat.

Run with: ctest -V -R QgsTextBlockFormat

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
import math

from qgis.core import (
    Qgis,
    QgsFontUtils,
    QgsRenderContext,
    QgsTextBlockFormat,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsTextBlockFormat(QgisTestCase):

    def setUp(self):
        QgsFontUtils.loadStandardTestFonts(['Bold', 'Oblique'])

    def testGettersSetters(self):
        format = QgsTextBlockFormat()
        self.assertFalse(format.hasHorizontalAlignmentSet())
        self.assertEqual(format.horizontalAlignment(), Qgis.TextHorizontalAlignment.Left)
        self.assertTrue(math.isnan(format.lineHeight()))
        self.assertTrue(math.isnan(format.lineHeightPercentage()))

        format.setHasHorizontalAlignmentSet(True)
        self.assertTrue(format.hasHorizontalAlignmentSet())
        format.setHorizontalAlignment(Qgis.TextHorizontalAlignment.Right)
        self.assertEqual(format.horizontalAlignment(), Qgis.TextHorizontalAlignment.Right)

        format.setLineHeight(5)
        self.assertEqual(format.lineHeight(), 5)
        self.assertTrue(math.isnan(format.lineHeightPercentage()))

        format = QgsTextBlockFormat()
        format.setLineHeightPercentage(0.5)
        self.assertEqual(format.lineHeightPercentage(), 0.5)
        self.assertTrue(math.isnan(format.lineHeight()))

    def testUpdateFont(self):
        context = QgsRenderContext()
        font = QgsFontUtils.getStandardTestFont()

        format = QgsTextBlockFormat()
        format.updateFontForFormat(font, context)

        # no effect for now...


if __name__ == '__main__':
    unittest.main()
