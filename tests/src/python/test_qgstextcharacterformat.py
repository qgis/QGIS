# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsTextCharacterFormat.

Run with: ctest -V -R QgsTextCharacterFormat

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '12/05/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (
    Qgis,
    QgsFontUtils,
    QgsTextCharacterFormat,
    QgsRenderContext
)
from qgis.PyQt.QtGui import QColor
from qgis.testing import start_app, unittest

start_app()


class TestQgsTextCharacterFormat(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsTextRenderer Tests</h1>\n"
        QgsFontUtils.loadStandardTestFonts(['Bold', 'Oblique'])

    def testGettersSetters(self):
        format = QgsTextCharacterFormat()
        self.assertFalse(format.textColor().isValid())
        self.assertEqual(format.underline(), QgsTextCharacterFormat.BooleanValue.NotSet)
        self.assertEqual(format.strikeOut(), QgsTextCharacterFormat.BooleanValue.NotSet)
        self.assertEqual(format.overline(), QgsTextCharacterFormat.BooleanValue.NotSet)
        self.assertEqual(format.fontPointSize(), -1)
        self.assertFalse(format.family())
        self.assertFalse(format.hasVerticalAlignmentSet())
        self.assertEqual(format.verticalAlignment(), Qgis.TextCharacterVerticalAlignment.Normal)

        format.setTextColor(QColor(255, 0, 0))
        self.assertTrue(format.textColor().isValid())
        self.assertEqual(format.textColor().name(), '#ff0000')

        format.setUnderline(QgsTextCharacterFormat.BooleanValue.SetTrue)
        self.assertEqual(format.underline(), QgsTextCharacterFormat.BooleanValue.SetTrue)

        format.setStrikeOut(QgsTextCharacterFormat.BooleanValue.SetTrue)
        self.assertEqual(format.strikeOut(), QgsTextCharacterFormat.BooleanValue.SetTrue)

        format.setOverline(QgsTextCharacterFormat.BooleanValue.SetTrue)
        self.assertEqual(format.overline(), QgsTextCharacterFormat.BooleanValue.SetTrue)

        format.setFontPointSize(12.5)
        self.assertEqual(format.fontPointSize(), 12.5)

        format.setFamily('comic sans')
        self.assertEqual(format.family(), 'comic sans')

        format.setHasVerticalAlignmentSet(True)
        self.assertTrue(format.hasVerticalAlignmentSet())
        format.setVerticalAlignment(Qgis.TextCharacterVerticalAlignment.SuperScript)
        self.assertEqual(format.verticalAlignment(), Qgis.TextCharacterVerticalAlignment.SuperScript)

    def testUpdateFont(self):
        context = QgsRenderContext()
        font = QgsFontUtils.getStandardTestFont()

        old_size = font.pointSizeF()
        old_family = font.family()

        format = QgsTextCharacterFormat()
        format.updateFontForFormat(font, context)

        self.assertFalse(font.underline())
        self.assertFalse(font.strikeOut())
        self.assertFalse(font.overline())
        self.assertEqual(font.family(), old_family)
        self.assertEqual(font.pointSizeF(), old_size)

        format.setUnderline(QgsTextCharacterFormat.BooleanValue.SetTrue)
        format.updateFontForFormat(font, context)
        self.assertTrue(font.underline())
        self.assertFalse(font.strikeOut())
        self.assertFalse(font.overline())

        format.setUnderline(QgsTextCharacterFormat.BooleanValue.NotSet)
        format.setStrikeOut(QgsTextCharacterFormat.BooleanValue.SetTrue)
        format.updateFontForFormat(font, context)
        self.assertTrue(font.underline())
        self.assertTrue(font.strikeOut())
        self.assertFalse(font.overline())

        format.setUnderline(QgsTextCharacterFormat.BooleanValue.SetFalse)
        format.setStrikeOut(QgsTextCharacterFormat.BooleanValue.NotSet)
        format.setOverline(QgsTextCharacterFormat.BooleanValue.SetTrue)
        format.updateFontForFormat(font, context)
        self.assertFalse(font.underline())
        self.assertTrue(font.strikeOut())
        self.assertTrue(font.overline())

        format.setStrikeOut(QgsTextCharacterFormat.BooleanValue.SetFalse)
        format.setOverline(QgsTextCharacterFormat.BooleanValue.NotSet)
        format.updateFontForFormat(font, context)
        self.assertFalse(font.strikeOut())
        self.assertTrue(font.overline())

        format.setOverline(QgsTextCharacterFormat.BooleanValue.SetFalse)
        format.updateFontForFormat(font, context)
        self.assertFalse(font.overline())
        self.assertEqual(font.pointSizeF(), old_size)

        format.setFontPointSize(49)
        format.updateFontForFormat(font, context)
        self.assertEqual(font.pixelSize(), 17)

        context.setScaleFactor(10)
        format.updateFontForFormat(font, context)
        self.assertEqual(font.pixelSize(), 172)
        format.updateFontForFormat(font, context, 5)
        self.assertEqual(font.pixelSize(), 864)
        format.setFontPointSize(-1)
        font.setPointSizeF(old_size)
        format.updateFontForFormat(font, context)
        self.assertEqual(font.pointSizeF(), old_size)

        self.assertEqual(font.family(), old_family)
        format.setFamily('Serif')
        format.updateFontForFormat(font, context)
        self.assertEqual(font.family(), 'Serif')
        format.setFamily('')
        font.setFamily(old_family)
        format.updateFontForFormat(font, context)
        self.assertEqual(font.family(), old_family)


if __name__ == '__main__':
    unittest.main()
