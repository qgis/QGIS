# -*- coding:utf-8 -*-
"""
/***************************************************************************
Python Console for QGIS
                             -------------------
begin                : 2020-06-04
copyright            : (C) 2020 by Richard Duivenvoorde
email                : Richard Duivenvoorde (at) duif (dot) net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
Some portions of code were taken from https://code.google.com/p/pydee/
"""

from qgis.PyQt.QtCore import Qt, QUrl
from qgis.PyQt.QtGui import QColor, QFontDatabase, QDesktopServices
from qgis.PyQt.Qsci import QsciLexerPython, QsciAPIs
from qgis.core import QgsApplication, Qgis
from qgis.gui import QgsCodeEditorPython, QgsCodeEditor
import os


class QgsPythonConsoleBase(QgsCodeEditorPython):

    MARKER_NUM = 6

    HANDY_COMMANDS = ['_pyqgis', '_api', '_cookbook']

    def __init__(self, parent=None):
        super().__init__(parent)

        # Enable non-ascii chars
        self.setUtf8(True)

        # Set the default font
        font = QFontDatabase.systemFont(QFontDatabase.FixedFont)
        self.setFont(font)

        # Margin 0 is used for line numbers (editor and output)
        self.setMarginWidth(0, "00000")
        # Margin 1 is used for the '>>>' prompt (console input)
        self.setMarginWidth(1, "0")
        self.setMarginType(1, 5)  # TextMarginRightJustified=5
        # Margin 2 is used for the 'folding' (editor)
        self.setMarginWidth(2, "0")

        self.setCaretLineVisible(True)
        self.setCaretWidth(2)

        self.iconRun = QgsApplication.getThemeIcon("console/mIconRunConsole.svg")
        self.iconRunScript = QgsApplication.getThemeIcon("mActionStart.svg")
        self.iconUndo = QgsApplication.getThemeIcon("mActionUndo.svg")
        self.iconRedo = QgsApplication.getThemeIcon("mActionRedo.svg")
        self.iconCodePad = QgsApplication.getThemeIcon("console/iconCodepadConsole.svg")
        self.iconCommentEditor = QgsApplication.getThemeIcon("console/iconCommentEditorConsole.svg")
        self.iconUncommentEditor = QgsApplication.getThemeIcon("console/iconUncommentEditorConsole.svg")
        self.iconSettings = QgsApplication.getThemeIcon("console/iconSettingsConsole.svg")
        self.iconFind = QgsApplication.getThemeIcon("console/iconSearchEditorConsole.svg")
        self.iconSyntaxCk = QgsApplication.getThemeIcon("console/iconSyntaxErrorConsole.svg")
        self.iconObjInsp = QgsApplication.getThemeIcon("console/iconClassBrowserConsole.svg")
        self.iconCut = QgsApplication.getThemeIcon("mActionEditCut.svg")
        self.iconCopy = QgsApplication.getThemeIcon("mActionEditCopy.svg")
        self.iconPaste = QgsApplication.getThemeIcon("mActionEditPaste.svg")
        self.iconClear = QgsApplication.getThemeIcon("console/iconClearConsole.svg")
        self.iconHideTool = QgsApplication.getThemeIcon("console/iconHideToolConsole.svg")
        self.iconShowEditor = QgsApplication.getThemeIcon("console/iconShowEditorConsole.svg")
        self.iconPyQGISHelp = QgsApplication.getThemeIcon("console/iconHelpConsole.svg")

    def setLexers(self):
        self.lexer = QsciLexerPython()
        self.lexer.setIndentationWarning(QsciLexerPython.Inconsistent)
        self.lexer.setFoldComments(True)
        self.lexer.setFoldQuotes(True)

        font = QFontDatabase.systemFont(QFontDatabase.FixedFont)

        # output and console
        loadFont = self.settings.value("pythonConsole/fontfamilytext")
        if loadFont:
            font.setFamily(loadFont)
        fontSize = self.settings.value("pythonConsole/fontsize", type=int)
        if fontSize:
            font.setPointSize(fontSize)

        self.lexer.setDefaultFont(font)
        self.lexer.setDefaultColor(self.color(QgsCodeEditor.ColorRole.Default))
        self.lexer.setColor(self.color(QgsCodeEditor.ColorRole.Comment), 1)
        self.lexer.setColor(self.color(QgsCodeEditor.ColorRole.Number), 2)
        self.lexer.setColor(self.color(QgsCodeEditor.ColorRole.Keyword), 5)
        self.lexer.setColor(self.color(QgsCodeEditor.ColorRole.Class), 8)
        self.lexer.setColor(self.color(QgsCodeEditor.ColorRole.Method), 9)
        self.lexer.setColor(self.color(QgsCodeEditor.ColorRole.Decoration), 15)
        self.lexer.setColor(self.color(QgsCodeEditor.ColorRole.CommentBlock), 12)
        self.lexer.setColor(self.color(QgsCodeEditor.ColorRole.SingleQuote), 4)
        self.lexer.setColor(self.color(QgsCodeEditor.ColorRole.DoubleQuote), 3)
        self.lexer.setColor(self.color(QgsCodeEditor.ColorRole.TripleSingleQuote), 6)
        self.lexer.setColor(self.color(QgsCodeEditor.ColorRole.TripleDoubleQuote), 7)
        self.lexer.setColor(self.color(QgsCodeEditor.ColorRole.Default), 13)
        self.lexer.setColor(QColor(Qt.red), 14)
        self.lexer.setFont(font, 1)
        self.lexer.setFont(font, 2)
        self.lexer.setFont(font, 3)
        self.lexer.setFont(font, 4)
        self.lexer.setFont(font, QsciLexerPython.UnclosedString)

        # ? only for editor and console ?
        paperColor = self.color(QgsCodeEditor.ColorRole.Background)
        for style in range(0, 33):
            self.lexer.setPaper(paperColor, style)

        self.api = QsciAPIs(self.lexer)
        checkBoxAPI = self.settings.value("pythonConsole/preloadAPI", True, type=bool)
        checkBoxPreparedAPI = self.settings.value("pythonConsole/usePreparedAPIFile", False, type=bool)
        if checkBoxAPI:
            pap = os.path.join(QgsApplication.pkgDataPath(), "python", "qsci_apis", "pyqgis.pap")
            self.api.loadPrepared(pap)
        elif checkBoxPreparedAPI:
            self.api.loadPrepared(self.settings.value("pythonConsole/preparedAPIFile"))
        else:
            apiPath = self.settings.value("pythonConsole/userAPI", [])
            for i in range(0, len(apiPath)):
                self.api.load(apiPath[i])
            self.api.prepare()
            self.lexer.setAPIs(self.api)

        self.setLexer(self.lexer)

    def searchPyQGIS(self):
        if self.hasSelectedText():
            text = self.selectedText()
            text = text.replace('>>> ', '').replace('... ', '').strip()  # removing prompts
            version = '.'.join(Qgis.QGIS_VERSION.split('.')[0:2])
            QDesktopServices.openUrl(QUrl('https://qgis.org/pyqgis/' + version + '/search.html?q=' + text))

    def handyCommands(self, hcmd):
        version = 'master' if 'master' in Qgis.QGIS_VERSION.lower() else re.findall(r'^\d.[0-9]*', Qgis.QGIS_VERSION)[0]
        if hcmd == '_pyqgis':
            QDesktopServices.openUrl(QUrl("https://qgis.org/pyqgis/{}".format(version)))
        elif hcmd == '_api':
            QDesktopServices.openUrl(QUrl("https://qgis.org/api/{}".format('' if version == 'master' else version)))
        elif hcmd == '_cookbook':
            QDesktopServices.openUrl(QUrl("https://docs.qgis.org/{}/en/docs/pyqgis_developer_cookbook/".format(
                'testing' if version == 'master' else version)))


if __name__ == "__main__":
    pass
