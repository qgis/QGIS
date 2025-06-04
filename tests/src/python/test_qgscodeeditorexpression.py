"""QGIS Unit tests for QgsCodeEditorExpression

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Juho Ervasti"
__date__ = "06/04/2025"
__copyright__ = "Copyright 2025, The QGIS Project"


from qgis.PyQt.QtCore import QCoreApplication, Qt
from qgis.PyQt.QtTest import QTest
from qgis.core import QgsSettings
from qgis.gui import QgsCodeEditorExpression
import unittest
from qgis.testing import start_app, QgisTestCase


class TestQgsCodeEditorExpression(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsCodeEditorExpression.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsCodeEditorExpression")
        start_app()

    def setUp(self):
        # Ensure that the settings are cleared before each test
        QgsSettings().clear()

    def testToggleComment(self):

        editor = QgsCodeEditorExpression()

        # Check single line comment
        editor.setText("--Hello World")
        editor.toggleComment()
        self.assertEqual(editor.text(), "Hello World")
        editor.toggleComment()
        self.assertEqual(editor.text(), "-- Hello World")

        # Check multiline comment
        editor.setText("Hello\nQGIS\nWorld")
        editor.setSelection(0, 0, 1, 4)
        editor.toggleComment()
        self.assertEqual(editor.text(), "-- Hello\n-- QGIS\nWorld")
        editor.toggleComment()
        self.assertEqual(editor.text(), "Hello\nQGIS\nWorld")

        # Check multiline comment with already commented lines
        editor.setText("Hello\n--QGIS\nWorld")
        editor.setSelection(0, 0, 2, 4)
        editor.toggleComment()
        self.assertEqual(editor.text(), "-- Hello\n-- --QGIS\n-- World")
        editor.toggleComment()
        self.assertEqual(editor.text(), "Hello\n--QGIS\nWorld")

        # Check multiline comment with empty lines
        editor.setText("Hello\n\n\nWorld")
        editor.setSelection(0, 0, 2, 4)
        editor.toggleComment()
        self.assertEqual(editor.text(), "-- Hello\n\n\n-- World")
        editor.toggleComment()
        self.assertEqual(editor.text(), "Hello\n\n\nWorld")


if __name__ == "__main__":
    unittest.main()
