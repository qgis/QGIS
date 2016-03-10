# -*- coding: utf-8 -*-

"""
***************************************************************************
    ScriptEdit.py
    ---------------------
    Date                 : February 2014
    Copyright            : (C) 2014 by Alexander Bruy
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
__date__ = 'February 2014'
__copyright__ = '(C) 2014, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from PyQt4.QtCore import Qt, QSettings
from PyQt4.QtGui import QColor, QFont, QShortcut, QKeySequence
from PyQt4.Qsci import QsciScintilla, QsciLexerSQL


class SqlEdit(QsciScintilla):
    LEXER_PYTHON = 0
    LEXER_R = 1

    def __init__(self, parent=None):
        QsciScintilla.__init__(self, parent)

        self.mylexer = None
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
        font.setPointSize(10)
        self.setFont(font)
        self.setMarginsFont(font)

        self.setBraceMatching(QsciScintilla.SloppyBraceMatch)

        self.setWrapMode(QsciScintilla.WrapWord)
        self.setWrapVisualFlags(QsciScintilla.WrapFlagByText,
                                QsciScintilla.WrapFlagNone, 4)

        self.setSelectionForegroundColor(QColor('#2e3436'))
        self.setSelectionBackgroundColor(QColor('#babdb6'))

        # Show line numbers
        self.setMarginWidth(1, '000')
        self.setMarginLineNumbers(1, True)
        self.setMarginsForegroundColor(QColor('#2e3436'))
        self.setMarginsBackgroundColor(QColor('#babdb6'))

        # Highlight current line
        self.setCaretLineVisible(True)
        self.setCaretLineBackgroundColor(QColor('#d3d7cf'))

        # Folding
        self.setFolding(QsciScintilla.BoxedTreeFoldStyle)
        self.setFoldMarginColors(QColor('#d3d7cf'), QColor('#d3d7cf'))

        # Mark column 80 with vertical line
        self.setEdgeMode(QsciScintilla.EdgeLine)
        self.setEdgeColumn(80)
        self.setEdgeColor(QColor('#eeeeec'))

        # Indentation
        self.setAutoIndent(True)
        self.setIndentationsUseTabs(False)
        self.setIndentationWidth(4)
        self.setTabIndents(True)
        self.setBackspaceUnindents(True)
        self.setTabWidth(4)

        # Autocomletion
        self.setAutoCompletionThreshold(2)
        self.setAutoCompletionSource(QsciScintilla.AcsAPIs)
        self.setAutoCompletionCaseSensitivity(False)

        # Load font from Python console settings
        settings = QSettings()
        fontName = settings.value('pythonConsole/fontfamilytext', 'Monospace')
        fontSize = int(settings.value('pythonConsole/fontsize', 10))

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

        self.initLexer()

    def initShortcuts(self):
        (ctrl, shift) = (self.SCMOD_CTRL << 16, self.SCMOD_SHIFT << 16)

        # Disable some shortcuts
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('D') + ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('L') + ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('L') + ctrl
                           + shift)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('T') + ctrl)

        # self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord("Z") + ctrl)
        #self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord("Y") + ctrl)

        # Use Ctrl+Space for autocompletion
        self.shortcutAutocomplete = QShortcut(QKeySequence(Qt.CTRL
                                                           + Qt.Key_Space), self)
        self.shortcutAutocomplete.setContext(Qt.WidgetShortcut)
        self.shortcutAutocomplete.activated.connect(self.autoComplete)

    def autoComplete(self):
        self.autoCompleteFromAll()

    def initLexer(self):
        self.mylexer = QsciLexerSQL()

        colorDefault = QColor('#2e3436')
        colorComment = QColor('#c00')
        colorCommentBlock = QColor('#3465a4')
        colorNumber = QColor('#4e9a06')
        colorType = QColor('#4e9a06')
        colorKeyword = QColor('#204a87')
        colorString = QColor('#ce5c00')

        self.mylexer.setDefaultFont(self.defaultFont)
        self.mylexer.setDefaultColor(colorDefault)

        self.mylexer.setColor(colorComment, 1)
        self.mylexer.setColor(colorNumber, 2)
        self.mylexer.setColor(colorString, 3)
        self.mylexer.setColor(colorString, 4)
        self.mylexer.setColor(colorKeyword, 5)
        self.mylexer.setColor(colorString, 6)
        self.mylexer.setColor(colorString, 7)
        self.mylexer.setColor(colorType, 8)
        self.mylexer.setColor(colorCommentBlock, 12)
        self.mylexer.setColor(colorString, 15)

        self.mylexer.setFont(self.italicFont, 1)
        self.mylexer.setFont(self.boldFont, 5)
        self.mylexer.setFont(self.boldFont, 8)
        self.mylexer.setFont(self.italicFont, 12)

        self.setLexer(self.mylexer)

    def lexer(self):
        return self.mylexer
