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
from qgis.PyQt.Qsci import QsciScintilla, QsciLexerPython, QsciAPIs
from qgis.core import QgsApplication, Qgis
import os


class QgsQsciScintillaBase(QsciScintilla):

    MARKER_NUM = 6
    DEFAULT_COLOR = "#4d4d4c"
    KEYWORD_COLOR = "#8959a8"
    CLASS_COLOR = "#4271ae"
    METHOD_COLOR = "#4271ae"
    DECORATION_COLOR = "#3e999f"
    NUMBER_COLOR = "#c82829"
    COMMENT_COLOR = "#8e908c"
    COMMENT_BLOCK_COLOR = "#8e908c"
    BACKGROUND_COLOR = "#ffffff"
    CURSOR_COLOR = "#636363"
    CARET_LINE_COLOR = "#efefef"
    SINGLE_QUOTE_COLOR = "#718c00"
    DOUBLE_QUOTE_COLOR = "#718c00"
    TRIPLE_SINGLE_QUOTE_COLOR = "#eab700"
    TRIPLE_DOUBLE_QUOTE_COLOR = "#eab700"
    MARGIN_BACKGROUND_COLOR = "#efefef"
    MARGIN_FOREGROUND_COLOR = "#636363"
    SELECTION_BACKGROUND_COLOR = "#d7d7d7"
    SELECTION_FOREGROUND_COLOR = "#303030"
    MATCHED_BRACE_BACKGROUND_COLOR = "#b7f907"
    MATCHED_BRACE_FOREGROUND_COLOR = "#303030"
    EDGE_COLOR = "#efefef"
    FOLD_COLOR = "#efefef"
    ERROR_COLOR = "#e31a1c"

    def __init__(self, parent=None):
        super(QgsQsciScintillaBase, self).__init__(parent)

        # Enable non-ascii chars
        self.setUtf8(True)

        # Set the default font
        font = QFontDatabase.systemFont(QFontDatabase.FixedFont)
        self.setFont(font)
        self.setMarginsFont(font)

        # Margin 0 is used for line numbers (editor and output)
        self.setMarginWidth(0, "00000")
        # Margin 1 is used for the '>>>' prompt (console input)
        self.setMarginWidth(1, "0")
        self.setMarginType(1, 5) # TextMarginRightJustified=5
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

        # TODO: editor has it's own font and size...
#        loadFont = self.settings.value("pythonConsole/fontfamilytextEditor")
#        if loadFont:
#            font.setFamily(loadFont)
#        fontSize = self.settings.value("pythonConsole/fontsizeEditor", type=int)
#        if fontSize:
#            font.setPointSize(fontSize)

        self.lexer.setDefaultFont(font)
        self.lexer.setDefaultColor(QColor(self.settings.value("pythonConsole/defaultFontColor", QColor(self.DEFAULT_COLOR))))
        self.lexer.setColor(QColor(self.settings.value("pythonConsole/commentFontColor", QColor(self.COMMENT_COLOR))), 1)
        self.lexer.setColor(QColor(self.settings.value("pythonConsole/numberFontColor", QColor(self.NUMBER_COLOR))), 2)
        self.lexer.setColor(QColor(self.settings.value("pythonConsole/keywordFontColor", QColor(self.KEYWORD_COLOR))), 5)
        self.lexer.setColor(QColor(self.settings.value("pythonConsole/classFontColor", QColor(self.CLASS_COLOR))), 8)
        self.lexer.setColor(QColor(self.settings.value("pythonConsole/methodFontColor", QColor(self.METHOD_COLOR))), 9)
        self.lexer.setColor(QColor(self.settings.value("pythonConsole/decorFontColor", QColor(self.DECORATION_COLOR))), 15)
        self.lexer.setColor(QColor(self.settings.value("pythonConsole/commentBlockFontColor", QColor(self.COMMENT_BLOCK_COLOR))), 12)
        self.lexer.setColor(QColor(self.settings.value("pythonConsole/singleQuoteFontColor", QColor(self.SINGLE_QUOTE_COLOR))), 4)
        self.lexer.setColor(QColor(self.settings.value("pythonConsole/doubleQuoteFontColor", QColor(self.DOUBLE_QUOTE_COLOR))), 3)
        self.lexer.setColor(QColor(self.settings.value("pythonConsole/tripleSingleQuoteFontColor", QColor(self.TRIPLE_SINGLE_QUOTE_COLOR))), 6)
        self.lexer.setColor(QColor(self.settings.value("pythonConsole/tripleDoubleQuoteFontColor", QColor(self.TRIPLE_DOUBLE_QUOTE_COLOR))), 7)
        self.lexer.setColor(QColor(self.settings.value("pythonConsole/defaultFontColor", QColor(self.DEFAULT_COLOR))), 13)
        self.lexer.setColor(QColor(Qt.red), 14)
        self.lexer.setFont(font, 1)
        self.lexer.setFont(font, 2)
        self.lexer.setFont(font, 3)
        self.lexer.setFont(font, 4)
        self.lexer.setFont(font, QsciLexerPython.UnclosedString)

        # ? only for editor and console ?
        for style in range(0, 33):
            paperColor = QColor(self.settings.value("pythonConsole/paperBackgroundColor", QColor(self.BACKGROUND_COLOR)))
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


if __name__ == "__main__":
    pass
