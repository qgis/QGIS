"""QGIS Unit tests for QgsCodeEditor

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "03/10/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

import sys

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtGui import QColor, QFont
from qgis.core import QgsApplication, QgsSettings
from qgis.gui import QgsCodeEditor, QgsCodeEditorColorScheme
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import getTestFont

start_app()


class TestQgsCodeEditor(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsColorScheme.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsColorScheme")
        QgsSettings().clear()
        start_app()

    def testDefaultColors(self):
        # default color theme, default application theme
        QgsApplication.setUITheme("default")
        self.assertEqual(
            QgsCodeEditor.defaultColor(
                QgsCodeEditorColorScheme.ColorRole.Keyword
            ).name(),
            "#8959a8",
        )

        # default colors should respond to application ui theme
        QgsApplication.setUITheme("Night Mapping")
        self.assertEqual(
            QgsCodeEditor.defaultColor(
                QgsCodeEditorColorScheme.ColorRole.Keyword
            ).name(),
            "#6cbcf7",
        )

        # explicit theme, should override ui theme defaults
        self.assertEqual(
            QgsCodeEditor.defaultColor(
                QgsCodeEditorColorScheme.ColorRole.Keyword, "solarized"
            ).name(),
            "#859900",
        )
        self.assertEqual(
            QgsCodeEditor.defaultColor(
                QgsCodeEditorColorScheme.ColorRole.Background, "solarized"
            ).name(),
            "#fdf6e3",
        )
        self.assertEqual(
            QgsCodeEditor.defaultColor(
                QgsCodeEditorColorScheme.ColorRole.Keyword, "solarized_dark"
            ).name(),
            "#859900",
        )
        self.assertEqual(
            QgsCodeEditor.defaultColor(
                QgsCodeEditorColorScheme.ColorRole.Background, "solarized_dark"
            ).name(),
            "#002b36",
        )

    def testColors(self):
        QgsApplication.setUITheme("default")
        self.assertEqual(
            QgsCodeEditor.color(QgsCodeEditorColorScheme.ColorRole.Keyword).name(),
            "#8959a8",
        )
        QgsApplication.setUITheme("Night Mapping")
        self.assertEqual(
            QgsCodeEditor.color(QgsCodeEditorColorScheme.ColorRole.Keyword).name(),
            "#6cbcf7",
        )

        QgsSettings().setValue(
            "codeEditor/colorScheme", "solarized", QgsSettings.Section.Gui
        )
        self.assertEqual(
            QgsCodeEditor.color(QgsCodeEditorColorScheme.ColorRole.Keyword).name(),
            "#859900",
        )
        self.assertEqual(
            QgsCodeEditor.color(QgsCodeEditorColorScheme.ColorRole.Background).name(),
            "#fdf6e3",
        )
        QgsSettings().setValue(
            "codeEditor/colorScheme", "solarized_dark", QgsSettings.Section.Gui
        )
        self.assertEqual(
            QgsCodeEditor.color(QgsCodeEditorColorScheme.ColorRole.Keyword).name(),
            "#859900",
        )
        self.assertEqual(
            QgsCodeEditor.color(QgsCodeEditorColorScheme.ColorRole.Background).name(),
            "#002b36",
        )

        QgsSettings().setValue(
            "codeEditor/overrideColors", True, QgsSettings.Section.Gui
        )
        QgsCodeEditor.setColor(
            QgsCodeEditorColorScheme.ColorRole.Keyword, QColor("#cc11bb")
        )
        self.assertEqual(
            QgsCodeEditor.color(QgsCodeEditorColorScheme.ColorRole.Keyword).name(),
            "#cc11bb",
        )

    def testFontFamily(self):
        f = QgsCodeEditor().getMonospaceFont()
        self.assertTrue(f.styleHint() & QFont.StyleHint.Monospace)

        QgsSettings().setValue(
            "codeEditor/fontfamily", getTestFont().family(), QgsSettings.Section.Gui
        )
        f = QgsCodeEditor().getMonospaceFont()
        self.assertEqual(f.family(), "QGIS Vera Sans")

    @unittest.skipIf(sys.platform == "darwin", "MacOS has different font logic")
    def testFontSize(self):
        f = QgsCodeEditor().getMonospaceFont()
        self.assertEqual(f.pointSize(), 10)

        QgsSettings().setValue("codeEditor/fontsize", 14, QgsSettings.Section.Gui)
        f = QgsCodeEditor().getMonospaceFont()
        self.assertEqual(f.pointSize(), 14)


if __name__ == "__main__":
    unittest.main()
