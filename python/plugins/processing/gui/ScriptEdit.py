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

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.Qsci import *

from qgis.core import *

from sextante.gui.LexerR import LexerR

class ScriptEdit(QsciScintilla):

    LEXER_PYTHON = 0
    LEXER_R = 1

    def __init__(self, parent=None):
        QsciScintilla.__init__(self, parent)

        self.lexer = None
        self.api = None
        self.lexerType = -1

        self.setCommonOptions()
        self.initShortcuts()

    def setCommonOptions(self):
        # enable non-ASCII characters
        self.setUtf8(True)

        # default font
        font = QFont()
        font.setFamily("Courier")
        font.setFixedPitch(True)
        font.setPointSize(10)
        self.setFont(font)
        self.setMarginsFont(font)

        self.initLexer()

        self.setBraceMatching(QsciScintilla.SloppyBraceMatch)

        self.setWrapMode(QsciScintilla.WrapWord)
        self.setWrapVisualFlags(QsciScintilla.WrapFlagByText, QsciScintilla.WrapFlagNone, 4)

        self.setSelectionForegroundColor(QColor("#2e3436"))
        self.setSelectionBackgroundColor(QColor("#babdb6"))

        # show line numbers
        self.setMarginWidth(1, "000")
        self.setMarginLineNumbers(1, True)
        self.setMarginsForegroundColor(QColor("#2e3436"))
        self.setMarginsBackgroundColor(QColor("#babdb6"))

        # highlight current line
        self.setCaretLineVisible(True)
        self.setCaretLineBackgroundColor(QColor("#d3d7cf"))

        # folding
        self.setFolding(QsciScintilla.BoxedTreeFoldStyle)
        self.setFoldMarginColors(QColor("#d3d7cf"), QColor("#d3d7cf"))

        # mark column 80 with vertical line
        self.setEdgeMode(QsciScintilla.EdgeLine)
        self.setEdgeColumn(80)
        self.setEdgeColor(QColor("#eeeeec"))

        # indentation
        self.setAutoIndent(True)
        self.setIndentationsUseTabs(False)
        self.setIndentationWidth(4)
        self.setTabIndents(True)
        self.setBackspaceUnindents(True)
        self.setTabWidth(4)

        # autocomletion
        self.setAutoCompletionThreshold(2)
        self.setAutoCompletionSource(QsciScintilla.AcsAPIs)

        # load font from Python console settings
        settings = QSettings()
        fontName = settings.value("pythonConsole/fontfamilytext", "Monospace").toString()
        fontSize = settings.value("pythonConsole/fontsize", 10).toInt()[0]

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
        ctrl, shift = self.SCMOD_CTRL << 16, self.SCMOD_SHIFT << 16

        # disable some shortcuts
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord("D") + ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord("L") + ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord("L") + ctrl + shift)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord("T") + ctrl)
        #self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord("Z") + ctrl)
        #self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord("Y") + ctrl)

        # use Ctrl+Space for autocompletion
        self.shortcutAutocomplete = QShortcut(QKeySequence(Qt.CTRL + Qt.Key_Space), self)
        self.shortcutAutocomplete.setContext(Qt.WidgetShortcut)
        self.shortcutAutocomplete.activated.connect(self.autoComplete)

    def autoComplete(self):
        self.autoCompleteFromAll()

    def setLexerType(self, lexerType):
      self.lexerType = lexerType
      self.initLexer()

    def initLexer(self):
        if self.lexerType == self.LEXER_PYTHON:
            self.lexer = QsciLexerPython()

            colorDefault = QColor("#2e3436")
            colorComment = QColor("#c00")
            colorCommentBlock = QColor("#3465a4")
            colorNumber = QColor("#4e9a06")
            colorType = QColor("#4e9a06")
            colorKeyword = QColor("#204a87")
            colorString = QColor("#ce5c00")

            self.lexer.setDefaultFont(self.defaultFont)
            self.lexer.setDefaultColor(colorDefault)

            self.lexer.setColor(colorComment, 1)
            self.lexer.setColor(colorNumber, 2)
            self.lexer.setColor(colorString, 3)
            self.lexer.setColor(colorString, 4)
            self.lexer.setColor(colorKeyword, 5)
            self.lexer.setColor(colorString, 6)
            self.lexer.setColor(colorString, 7)
            self.lexer.setColor(colorType, 8)
            self.lexer.setColor(colorCommentBlock, 12)
            self.lexer.setColor(colorString, 15)

            self.lexer.setFont(self.italicFont, 1)
            self.lexer.setFont(self.boldFont, 5)
            self.lexer.setFont(self.boldFont, 8)
            self.lexer.setFont(self.italicFont, 12)

            self.api = QsciAPIs(self.lexer)

            settings = QSettings()
            useDefaultAPI = settings.value("pythonConsole/preloadAPI", True).toBool()
            if useDefaultAPI:
                # load QGIS API shipped with Python console
                self.api.loadPrepared(QgsApplication.pkgDataPath() + "/python/qsci_apis/pyqgis_master.pap")
            else:
                # load user-defined API files
                apiPaths = settings.value("pythonConsole/userAPI").toStringList()
                for path in apiPaths:
                    self.api.load(path)
                self.api.prepare()
                self.lexer.setAPIs(self.api)
        elif self.lexerType == self.LEXER_R:
            # R lexer
            self.lexer = LexerR()

        self.setLexer(self.lexer)
