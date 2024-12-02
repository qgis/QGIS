"""QGIS Unit tests for QgsCodeEditorPython

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Yoann Quenach de Quivillic"
__date__ = "31/03/2023"
__copyright__ = "Copyright 2023, The QGIS Project"


from qgis.PyQt.QtCore import QCoreApplication, Qt
from qgis.PyQt.QtTest import QTest
from qgis.core import QgsSettings
from qgis.gui import QgsCodeEditorPython
import unittest
from qgis.testing import start_app, QgisTestCase


COMPLETIONS_PAIRS = {"(": ")", "[": "]", "{": "}", "'": "'", '"': '"'}
COMPLETIONS_SINGLE_CHARACTERS = ["`", "*"]


class TestQgsCodeEditorPython(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsCodeEditorPython.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsCodeEditorPython")
        start_app()

    def setUp(self):
        # Ensure that the settings are cleared before each test
        QgsSettings().clear()

    def testAutoSurround(self):
        self.assertEqual(QgsSettings().value("pythonConsole/autoSurround"), None)

        editor = QgsCodeEditorPython()

        test_string = "Hello World"

        for opening, closing in COMPLETIONS_PAIRS.items():

            editor.setText(test_string)

            # Select the whole text
            editor.selectAll()

            # Type the opening character
            QTest.keyClicks(editor, opening)
            self.assertEqual(editor.text(), opening + test_string + closing)

        for character in COMPLETIONS_SINGLE_CHARACTERS:

            editor.setText(test_string)

            # Type the character
            editor.selectAll()
            QTest.keyClicks(editor, character)
            self.assertEqual(editor.text(), character + test_string + character)

        # Check nested autosurround
        editor.setText(test_string)
        editor.selectAll()
        for opening, closing in COMPLETIONS_PAIRS.items():

            text = editor.text()
            # Select the whole text
            QTest.keyClicks(editor, opening)
            self.assertEqual(
                editor.text(),
                text.replace(test_string, opening + test_string + closing),
            )

        # Check multiline autosurround with quotes (should insert triple quotes)
        test_string = "Hello\nWorld"
        editor.setText(test_string)
        editor.selectAll()
        QTest.keyClicks(editor, "'")
        self.assertEqual(editor.text(), f"'''{test_string}'''")

        editor.setText(test_string)
        editor.selectAll()
        QTest.keyClicks(editor, '"')
        self.assertEqual(editor.text(), f'"""{test_string}"""')

        # Check disabled autosurround
        test_string = "will not be surrounded"
        QgsSettings().setValue("pythonConsole/autoSurround", False)
        editor.setText(test_string)
        editor.selectAll()
        QTest.keyClicks(editor, "(")
        self.assertEqual(editor.text(), "(")

    def testAutoCloseBrackets(self):

        self.assertEqual(QgsSettings().value("pythonConsole/autoCloseBracket"), None)

        editor = QgsCodeEditorPython()

        for opening, closing in COMPLETIONS_PAIRS.items():

            # Type the opening character, should insert both characters
            QTest.keyClicks(editor, opening)
            self.assertEqual(editor.text(), opening + closing)
            self.assertEqual(editor.getCursorPosition(), (0, 1))

            # Typing the closing character should just move the cursor
            QTest.keyClicks(editor, closing)
            self.assertEqual(editor.text(), opening + closing)
            self.assertEqual(editor.getCursorPosition(), (0, 2))

            # Pressing backspace while inside an opening/closing pair should remove both characters
            QTest.keyClick(editor, Qt.Key.Key_Left)
            self.assertEqual(editor.getCursorPosition(), (0, 1))
            QTest.keyClick(editor, Qt.Key.Key_Backspace)
            self.assertEqual(editor.text(), "")

            editor.clear()

        # Check nested brackets
        for i, (opening, closing) in enumerate(COMPLETIONS_PAIRS.items()):
            # Type the opening character, should insert both characters
            self.assertEqual(editor.getCursorPosition(), (0, i))
            text = editor.text()
            QTest.keyClicks(editor, opening)
            self.assertEqual(editor.text(), text[:i] + opening + closing + text[i:])

        # Check do not auto close brackets when the following character is not a space, a colon or a closing bracket
        editor.setText("(")
        editor.setCursorPosition(0, 0)
        QTest.keyClicks(editor, "[")
        self.assertEqual(editor.text(), "[(")
        editor.clear()

        editor.setText(")")
        editor.setCursorPosition(0, 0)
        QTest.keyClicks(editor, "[")
        self.assertEqual(editor.text(), "[])")
        editor.clear()

        editor.setText("a")
        editor.setCursorPosition(0, 0)
        QTest.keyClicks(editor, "[")
        self.assertEqual(editor.text(), "[a")
        editor.clear()

        editor.setText(" a")
        editor.setCursorPosition(0, 0)
        QTest.keyClicks(editor, "[")
        self.assertEqual(editor.text(), "[] a")
        editor.clear()

        editor.setText("a:")
        editor.setCursorPosition(0, 0)
        QTest.keyClicks(editor, "{")
        self.assertEqual(editor.text(), "{a:")
        editor.clear()

        editor.setText("a:")
        editor.setCursorPosition(0, 1)
        QTest.keyClicks(editor, "{")
        self.assertEqual(editor.text(), "a{}:")
        editor.clear()

        # Check disabled auto close brackets
        QgsSettings().setValue("pythonConsole/autoCloseBracket", False)

        QTest.keyClicks(editor, "{")
        self.assertEqual(editor.text(), "{")
        editor.clear()

    def testToggleComment(self):

        editor = QgsCodeEditorPython()

        # Check single line comment
        editor.setText("#Hello World")
        QTest.keyClick(editor, ":", Qt.KeyboardModifier.ControlModifier)
        self.assertEqual(editor.text(), "Hello World")
        QTest.keyClick(editor, ":", Qt.KeyboardModifier.ControlModifier)
        self.assertEqual(editor.text(), "# Hello World")

        # Check multiline comment
        editor.setText("Hello\nQGIS\nWorld")
        editor.setSelection(0, 0, 1, 4)
        QTest.keyClick(editor, ":", Qt.KeyboardModifier.ControlModifier)
        self.assertEqual(editor.text(), "# Hello\n# QGIS\nWorld")
        QTest.keyClick(editor, ":", Qt.KeyboardModifier.ControlModifier)
        self.assertEqual(editor.text(), "Hello\nQGIS\nWorld")

        # Check multiline comment with already commented lines
        editor.setText("Hello\n# QGIS\nWorld")
        editor.setSelection(0, 0, 2, 4)
        QTest.keyClick(editor, ":", Qt.KeyboardModifier.ControlModifier)
        self.assertEqual(editor.text(), "# Hello\n# # QGIS\n# World")
        QTest.keyClick(editor, ":", Qt.KeyboardModifier.ControlModifier)
        self.assertEqual(editor.text(), "Hello\n# QGIS\nWorld")


if __name__ == "__main__":
    unittest.main()
