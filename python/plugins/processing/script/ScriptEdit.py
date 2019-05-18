# -*- coding: utf-8 -*-

"""
***************************************************************************
    ScriptEdit.py
    ---------------------
    Date                 : April 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'April 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

import os

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QFont, QColor, QKeySequence, QFontDatabase, QFontMetrics
from qgis.PyQt.QtWidgets import QShortcut
from qgis.core import QgsApplication, QgsSettings

from qgis.PyQt.Qsci import QsciScintilla, QsciLexerPython, QsciAPIs


class ScriptEdit(QsciScintilla):

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

    def __init__(self, parent=None):
        super().__init__(parent)

        self.lexer = None
        self.api = None

        self.setCommonOptions()
        self.initShortcuts()

    def setCommonOptions(self):
        # Enable non-ASCII characters
        self.setUtf8(True)

        settings = QgsSettings()

        # Default font
        font = QFontDatabase.systemFont(QFontDatabase.FixedFont)
        self.setFont(font)
        self.setMarginsFont(font)

        self.setBraceMatching(QsciScintilla.SloppyBraceMatch)
        self.setMatchedBraceBackgroundColor(QColor(settings.value("pythonConsole/matchedBraceBackgroundColorEditor", QColor(self.MATCHED_BRACE_BACKGROUND_COLOR))))
        self.setMatchedBraceForegroundColor(QColor(settings.value("pythonConsole/matchedBraceForegroundColorEditor", QColor(self.MATCHED_BRACE_FOREGROUND_COLOR))))

        #self.setWrapMode(QsciScintilla.WrapWord)
        #self.setWrapVisualFlags(QsciScintilla.WrapFlagByText,
        #                        QsciScintilla.WrapFlagNone, 4)

        self.setSelectionForegroundColor(QColor(settings.value("pythonConsole/selectionForegroundColorEditor", QColor(self.SELECTION_FOREGROUND_COLOR))))
        self.setSelectionBackgroundColor(QColor(settings.value("pythonConsole/selectionBackgroundColorEditor", QColor(self.SELECTION_BACKGROUND_COLOR))))

        # Show line numbers
        fontmetrics = QFontMetrics(font)
        self.setMarginWidth(1, fontmetrics.width("0000") + 5)
        self.setMarginLineNumbers(1, True)
        self.setMarginsForegroundColor(QColor(settings.value("pythonConsole/marginForegroundColorEditor", QColor(self.MARGIN_FOREGROUND_COLOR))))
        self.setMarginsBackgroundColor(QColor(settings.value("pythonConsole/marginBackgroundColorEditor", QColor(self.MARGIN_BACKGROUND_COLOR))))
        self.setIndentationGuidesForegroundColor(QColor(settings.value("pythonConsole/marginForegroundColorEditor", QColor(self.MARGIN_FOREGROUND_COLOR))))
        self.setIndentationGuidesBackgroundColor(QColor(settings.value("pythonConsole/marginBackgroundColorEditor", QColor(self.MARGIN_BACKGROUND_COLOR))))

        # Highlight current line
        caretLineColorEditor = settings.value("pythonConsole/caretLineColorEditor", QColor(self.CARET_LINE_COLOR))
        cursorColorEditor = settings.value("pythonConsole/cursorColorEditor", QColor(self.CURSOR_COLOR))
        self.setCaretLineVisible(True)
        self.setCaretWidth(2)
        self.setCaretLineBackgroundColor(caretLineColorEditor)
        self.setCaretForegroundColor(cursorColorEditor)

        # Folding
        self.setFolding(QsciScintilla.PlainFoldStyle)
        foldColor = QColor(settings.value("pythonConsole/foldColorEditor", QColor(self.FOLD_COLOR)))
        self.setFoldMarginColors(foldColor, foldColor)

        # Mark column 80 with vertical line
        self.setEdgeMode(QsciScintilla.EdgeLine)
        self.setEdgeColumn(80)
        self.setEdgeColor(QColor(settings.value("pythonConsole/edgeColorEditor", QColor(self.EDGE_COLOR))))

        # Indentation
        self.setAutoIndent(True)
        self.setIndentationsUseTabs(False)
        self.setIndentationWidth(4)
        self.setTabIndents(True)
        self.setBackspaceUnindents(True)
        self.setTabWidth(4)
        self.setIndentationGuides(True)

        # Autocompletion
        self.setAutoCompletionThreshold(2)
        self.setAutoCompletionSource(QsciScintilla.AcsAPIs)

        self.setFonts(10)
        self.initLexer()

    def setFonts(self, size):
        # Load font from Python console settings
        settings = QgsSettings()
        fontName = settings.value('pythonConsole/fontfamilytext', 'Monospace')
        fontSize = int(settings.value('pythonConsole/fontsize', size))

        self.defaultFont = QFont(fontName)
        self.defaultFont.setFixedPitch(True)
        self.defaultFont.setPointSize(fontSize)
        self.defaultFont.setStyleHint(QFont.TypeWriter)
        self.defaultFont.setBold(False)

        self.boldFont = QFont(self.defaultFont)
        self.boldFont.setBold(True)

        self.italicFont = QFont(self.defaultFont)
        self.italicFont.setItalic(True)

        self.setFont(self.defaultFont)
        self.setMarginsFont(self.defaultFont)

    def initShortcuts(self):
        (ctrl, shift) = (self.SCMOD_CTRL << 16, self.SCMOD_SHIFT << 16)

        # Disable some shortcuts
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('D') + ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('L') + ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('L') + ctrl +
                           shift)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('T') + ctrl)

        # self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord("Z") + ctrl)
        # self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord("Y") + ctrl)

        # Use Ctrl+Space for autocompletion
        self.shortcutAutocomplete = QShortcut(QKeySequence(Qt.CTRL +
                                                           Qt.Key_Space), self)
        self.shortcutAutocomplete.setContext(Qt.WidgetShortcut)
        self.shortcutAutocomplete.activated.connect(self.autoComplete)

    def autoComplete(self):
        self.autoCompleteFromAll()

    def initLexer(self):
        settings = QgsSettings()
        self.lexer = QsciLexerPython()

        font = QFontDatabase.systemFont(QFontDatabase.FixedFont)

        loadFont = settings.value("pythonConsole/fontfamilytextEditor")
        if loadFont:
            font.setFamily(loadFont)
        fontSize = settings.value("pythonConsole/fontsizeEditor", type=int)
        if fontSize:
            font.setPointSize(fontSize)

        self.lexer.setDefaultFont(font)
        self.lexer.setDefaultColor(QColor(settings.value("pythonConsole/defaultFontColorEditor", QColor(self.DEFAULT_COLOR))))
        self.lexer.setColor(QColor(settings.value("pythonConsole/commentFontColorEditor", QColor(self.COMMENT_COLOR))), 1)
        self.lexer.setColor(QColor(settings.value("pythonConsole/numberFontColorEditor", QColor(self.NUMBER_COLOR))), 2)
        self.lexer.setColor(QColor(settings.value("pythonConsole/keywordFontColorEditor", QColor(self.KEYWORD_COLOR))), 5)
        self.lexer.setColor(QColor(settings.value("pythonConsole/classFontColorEditor", QColor(self.CLASS_COLOR))), 8)
        self.lexer.setColor(QColor(settings.value("pythonConsole/methodFontColorEditor", QColor(self.METHOD_COLOR))), 9)
        self.lexer.setColor(QColor(settings.value("pythonConsole/decorFontColorEditor", QColor(self.DECORATION_COLOR))), 15)
        self.lexer.setColor(QColor(settings.value("pythonConsole/commentBlockFontColorEditor", QColor(self.COMMENT_BLOCK_COLOR))), 12)
        self.lexer.setColor(QColor(settings.value("pythonConsole/singleQuoteFontColorEditor", QColor(self.SINGLE_QUOTE_COLOR))), 4)
        self.lexer.setColor(QColor(settings.value("pythonConsole/doubleQuoteFontColorEditor", QColor(self.DOUBLE_QUOTE_COLOR))), 3)
        self.lexer.setColor(QColor(settings.value("pythonConsole/tripleSingleQuoteFontColorEditor", QColor(self.TRIPLE_SINGLE_QUOTE_COLOR))), 6)
        self.lexer.setColor(QColor(settings.value("pythonConsole/tripleDoubleQuoteFontColorEditor", QColor(self.TRIPLE_DOUBLE_QUOTE_COLOR))), 7)
        self.lexer.setColor(QColor(settings.value("pythonConsole/defaultFontColorEditor", QColor(self.DEFAULT_COLOR))), 13)
        self.lexer.setFont(font, 1)
        self.lexer.setFont(font, 3)
        self.lexer.setFont(font, 4)
        self.lexer.setFont(font, QsciLexerPython.UnclosedString)

        for style in range(0, 33):
            paperColor = QColor(settings.value("pythonConsole/paperBackgroundColorEditor", QColor(self.BACKGROUND_COLOR)))
            self.lexer.setPaper(paperColor, style)

        self.api = QsciAPIs(self.lexer)

        useDefaultAPI = bool(settings.value('pythonConsole/preloadAPI',
                                            True))
        if useDefaultAPI:
            # Load QGIS API shipped with Python console
            self.api.loadPrepared(
                os.path.join(QgsApplication.pkgDataPath(),
                             'python', 'qsci_apis', 'pyqgis.pap'))
        else:
            # Load user-defined API files
            apiPaths = settings.value('pythonConsole/userAPI', [])
            for path in apiPaths:
                self.api.load(path)
            self.api.prepare()
            self.lexer.setAPIs(self.api)

        self.setLexer(self.lexer)
