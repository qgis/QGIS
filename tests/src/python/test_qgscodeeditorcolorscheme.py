"""QGIS Unit tests for QgsCodeEditorColorScheme

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "03/10/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtGui import QColor
from qgis.core import QgsSettings
from qgis.gui import (
    QgsCodeEditorColorScheme,
    QgsCodeEditorColorSchemeRegistry,
    QgsGui,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsCodeEditorColorScheme(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(
            "QGIS_TestPyQgsCodeEditorColorScheme.com"
        )
        QCoreApplication.setApplicationName("QGIS_TestPyQgsCodeEditorColorScheme")
        QgsSettings().clear()
        start_app()

    def testScheme(self):
        scheme = QgsCodeEditorColorScheme("my id", "my name")
        self.assertEqual(scheme.id(), "my id")
        self.assertEqual(scheme.name(), "my name")

        scheme.setColor(QgsCodeEditorColorScheme.ColorRole.Keyword, QColor(255, 0, 0))
        scheme.setColor(QgsCodeEditorColorScheme.ColorRole.Method, QColor(0, 255, 0))
        self.assertEqual(
            scheme.color(QgsCodeEditorColorScheme.ColorRole.Keyword).name(), "#ff0000"
        )
        self.assertEqual(
            scheme.color(QgsCodeEditorColorScheme.ColorRole.Method).name(), "#00ff00"
        )

    def testSchemeRegistry(self):
        default_reg = QgsGui.codeEditorColorSchemeRegistry()
        self.assertGreaterEqual(len(default_reg.schemes()), 3)

        registry = QgsCodeEditorColorSchemeRegistry()
        self.assertCountEqual(
            registry.schemes(), ["default", "solarized", "solarized_dark"]
        )
        self.assertEqual(registry.scheme("solarized").name(), "Solarized (Light)")
        self.assertEqual(registry.scheme("solarized_dark").name(), "Solarized (Dark)")

        # duplicate name
        scheme = QgsCodeEditorColorScheme("solarized", "my name")
        self.assertFalse(registry.addColorScheme(scheme))

        # unique name
        scheme = QgsCodeEditorColorScheme("xxxx", "my name")
        self.assertTrue(registry.addColorScheme(scheme))
        self.assertCountEqual(
            registry.schemes(), ["default", "solarized", "solarized_dark", "xxxx"]
        )
        self.assertEqual(registry.scheme("xxxx").name(), "my name")

        self.assertFalse(registry.removeColorScheme("yyyy"))
        self.assertCountEqual(
            registry.schemes(), ["default", "solarized", "solarized_dark", "xxxx"]
        )
        self.assertTrue(registry.removeColorScheme("xxxx"))
        self.assertCountEqual(
            registry.schemes(), ["default", "solarized", "solarized_dark"]
        )

        # should return default registry if matching one doesn't exist
        self.assertEqual(registry.scheme("xxxx").name(), "Default")


if __name__ == "__main__":
    unittest.main()
