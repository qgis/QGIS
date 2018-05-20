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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QFont, QColor, QKeySequence, QFontDatabase
from qgis.PyQt.QtWidgets import QShortcut
from qgis.core import QgsApplication, QgsSettings

from qgis.PyQt.Qsci import QsciScintilla, QsciLexerPython, QsciAPIs


class ScriptEdit(QsciScintilla):

    def __init__(self, parent=None):
        QsciScintilla.__init__(self, parent)

        self.lexer = None
        self.api = None

        self.setCommonOptions()
        self.initShortcuts()

    def setCommonOptions(self):
        # Enable non-ASCII characters
        self.setUtf8(True)

        # Default font
        font = QFont()
        font.setFamily('Courier')
        font.setFixedPitch(True)
        font.setPointSize(20)
        self.setFont(font)
        self.setMarginsFont(font)

        self.setBraceMatching(QsciScintilla.SloppyBraceMatch)
        self.setMatchedBraceBackgroundColor(QColor("#b7f907"))

        #self.setWrapMode(QsciScintilla.WrapWord)
        #self.setWrapVisualFlags(QsciScintilla.WrapFlagByText,
        #                        QsciScintilla.WrapFlagNone, 4)

        self.setSelectionForegroundColor(QColor('#2e3436'))
        self.setSelectionBackgroundColor(QColor('#babdb6'))

        # Show line numbers
        self.setMarginWidth(1, '000')
        self.setMarginLineNumbers(1, True)
        self.setMarginsForegroundColor(QColor("#3E3EE3"))
        self.setMarginsBackgroundColor(QColor("#f9f9f9"))

        # Highlight current line
        settings = QgsSettings()
        caretLineColorEditor = settings.value("pythonConsole/caretLineColorEditor", QColor("#fcf3ed"))
        cursorColorEditor = settings.value("pythonConsole/cursorColorEditor", QColor(Qt.black))
        self.setCaretLineVisible(True)
        self.setCaretWidth(2)
        self.setCaretLineBackgroundColor(caretLineColorEditor)
        self.setCaretForegroundColor(cursorColorEditor)

        # Folding
        self.setFolding(QsciScintilla.PlainFoldStyle)
        self.setFoldMarginColors(QColor("#f4f4f4"), QColor("#f4f4f4"))

        # Mark column 80 with vertical line
        self.setEdgeMode(QsciScintilla.EdgeLine)
        self.setEdgeColumn(80)
        self.setEdgeColor(QColor("#FF0000"))

        # Indentation
        self.setAutoIndent(True)
        self.setIndentationsUseTabs(False)
        self.setIndentationWidth(4)
        self.setTabIndents(True)
        self.setBackspaceUnindents(True)
        self.setTabWidth(4)
        self.setIndentationGuides(True)

        # Autocomletion
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
        self.defaultFont.setStretch(QFont.SemiCondensed)
        self.defaultFont.setLetterSpacing(QFont.PercentageSpacing, 87.0)
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
        self.lexer.setDefaultColor(QColor(settings.value("pythonConsole/defaultFontColorEditor", QColor(Qt.black))))
        self.lexer.setColor(QColor(settings.value("pythonConsole/commentFontColorEditor", QColor(Qt.gray))), 1)
        self.lexer.setColor(QColor(settings.value("pythonConsole/keywordFontColorEditor", QColor(Qt.darkGreen))), 5)
        self.lexer.setColor(QColor(settings.value("pythonConsole/classFontColorEditor", QColor(Qt.blue))), 8)
        self.lexer.setColor(QColor(settings.value("pythonConsole/methodFontColorEditor", QColor(Qt.darkGray))), 9)
        self.lexer.setColor(QColor(settings.value("pythonConsole/decorFontColorEditor", QColor(Qt.darkBlue))), 15)
        self.lexer.setColor(QColor(settings.value("pythonConsole/commentBlockFontColorEditor", QColor(Qt.gray))), 12)
        self.lexer.setColor(QColor(settings.value("pythonConsole/singleQuoteFontColorEditor", QColor(Qt.blue))), 4)
        self.lexer.setColor(QColor(settings.value("pythonConsole/doubleQuoteFontColorEditor", QColor(Qt.blue))), 3)
        self.lexer.setColor(QColor(settings.value("pythonConsole/tripleSingleQuoteFontColorEditor", QColor(Qt.blue))), 6)
        self.lexer.setColor(QColor(settings.value("pythonConsole/tripleDoubleQuoteFontColorEditor", QColor(Qt.blue))), 7)
        self.lexer.setFont(font, 1)
        self.lexer.setFont(font, 3)
        self.lexer.setFont(font, 4)

        for style in range(0, 33):
            paperColor = QColor(settings.value("pythonConsole/paperBackgroundColorEditor", QColor(Qt.white)))
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
