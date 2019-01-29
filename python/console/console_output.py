# -*- coding:utf-8 -*-
"""
/***************************************************************************
Python Console for QGIS
                             -------------------
begin                : 2012-09-10
copyright            : (C) 2012 by Salvatore Larosa
email                : lrssvtml (at) gmail (dot) com
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

from qgis.PyQt.QtCore import Qt, QCoreApplication
from qgis.PyQt.QtGui import QColor, QFont, QKeySequence, QFontDatabase
from qgis.PyQt.QtWidgets import QGridLayout, QSpacerItem, QSizePolicy, QShortcut, QMenu, QApplication
from qgis.PyQt.Qsci import QsciScintilla, QsciLexerPython
from qgis.core import Qgis, QgsApplication, QgsSettings
from qgis.gui import QgsMessageBar
import sys


class writeOut(object):

    ERROR_COLOR = "#e31a1c"

    def __init__(self, shellOut, out=None, style=None):
        """
        This class allows writing to stdout and stderr
        """
        self.sO = shellOut
        self.out = None
        self.style = style
        self.fire_keyboard_interrupt = False

    def write(self, m):
        if self.style == "_traceback":
            # Show errors in red
            stderrColor = QColor(self.sO.settings.value("pythonConsole/stderrFontColor", QColor(self.ERROR_COLOR)))
            self.sO.SendScintilla(QsciScintilla.SCI_STYLESETFORE, 0o01, stderrColor)
            self.sO.SendScintilla(QsciScintilla.SCI_STYLESETITALIC, 0o01, True)
            self.sO.SendScintilla(QsciScintilla.SCI_STYLESETBOLD, 0o01, True)
            pos = self.sO.SendScintilla(QsciScintilla.SCI_GETCURRENTPOS)
            self.sO.SendScintilla(QsciScintilla.SCI_STARTSTYLING, pos, 31)
            self.sO.append(m)
            self.sO.SendScintilla(QsciScintilla.SCI_SETSTYLING, len(m), 0o01)
        else:
            self.sO.append(m)

        if self.out:
            self.out.write(m)

        self.move_cursor_to_end()

        if self.style != "_traceback":
            self.sO.repaint()

        if self.fire_keyboard_interrupt:
            self.fire_keyboard_interrupt = False
            raise KeyboardInterrupt

    def move_cursor_to_end(self):
        """Move cursor to end of text"""
        line, index = self.get_end_pos()
        self.sO.setCursorPosition(line, index)
        self.sO.ensureCursorVisible()
        self.sO.ensureLineVisible(line)

    def get_end_pos(self):
        """Return (line, index) position of the last character"""
        line = self.sO.lines() - 1
        return (line, len(self.sO.text(line)))

    def flush(self):
        pass

    def isatty(self):
        return False


class ShellOutputScintilla(QsciScintilla):

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

    def __init__(self, parent=None):
        super(ShellOutputScintilla, self).__init__(parent)
        self.parent = parent
        self.shell = self.parent.shell

        self.settings = QgsSettings()

        # Creates layout for message bar
        self.layout = QGridLayout(self)
        self.layout.setContentsMargins(0, 0, 0, 0)
        spacerItem = QSpacerItem(20, 40, QSizePolicy.Minimum, QSizePolicy.Expanding)
        self.layout.addItem(spacerItem, 1, 0, 1, 1)
        # messageBar instance
        self.infoBar = QgsMessageBar()
        sizePolicy = QSizePolicy(QSizePolicy.Minimum, QSizePolicy.Fixed)
        self.infoBar.setSizePolicy(sizePolicy)
        self.layout.addWidget(self.infoBar, 0, 0, 1, 1)

        # Enable non-ascii chars for editor
        self.setUtf8(True)

        sys.stdout = writeOut(self, sys.stdout)
        sys.stderr = writeOut(self, sys.stderr, "_traceback")

        self.insertInitText()
        self.refreshSettingsOutput()
        self.setReadOnly(True)

        # Set the default font
        font = QFontDatabase.systemFont(QFontDatabase.FixedFont)
        self.setFont(font)
        self.setMarginsFont(font)
        # Margin 0 is used for line numbers
        self.setMarginWidth(0, 0)
        self.setMarginWidth(1, 0)
        self.setMarginWidth(2, 0)
        #fm = QFontMetrics(font)
        self.setMarginsFont(font)
        self.setMarginWidth(1, "00000")
        self.setMarginLineNumbers(1, True)
        self.setCaretLineVisible(True)
        self.setCaretWidth(0)

        self.setMinimumHeight(120)

        self.setWrapMode(QsciScintilla.WrapCharacter)
        self.SendScintilla(QsciScintilla.SCI_SETHSCROLLBAR, 0)

        self.runScut = QShortcut(QKeySequence(Qt.CTRL + Qt.Key_E), self)
        self.runScut.setContext(Qt.WidgetShortcut)
        self.runScut.activated.connect(self.enteredSelected)
        # Reimplemented copy action to prevent paste prompt (>>>,...) in command view
        self.copyShortcut = QShortcut(QKeySequence.Copy, self)
        self.copyShortcut.activated.connect(self.copy)
        self.selectAllShortcut = QShortcut(QKeySequence.SelectAll, self)
        self.selectAllShortcut.activated.connect(self.selectAll)

    def insertInitText(self):
        txtInit = QCoreApplication.translate("PythonConsole",
                                             "Python Console\n"
                                             "Use iface to access QGIS API interface or Type help(iface) for more info\n"
                                             "Security warning: typing commands from an untrusted source can lead to data loss and/or leak")

        # some translation string for the console header ends without '\n'
        # and the first command in console will be appended at the header text.
        # The following code add a '\n' at the end of the string if not present.
        if txtInit.endswith('\n'):
            self.setText(txtInit)
        else:
            self.setText(txtInit + '\n')

    def refreshSettingsOutput(self):
        # Set Python lexer
        self.setLexers()
        self.setSelectionForegroundColor(QColor(self.settings.value("pythonConsole/selectionForegroundColor", QColor(self.SELECTION_FOREGROUND_COLOR))))
        self.setSelectionBackgroundColor(QColor(self.settings.value("pythonConsole/selectionBackgroundColor", QColor(self.SELECTION_BACKGROUND_COLOR))))
        self.setMarginsForegroundColor(QColor(self.settings.value("pythonConsole/marginForegroundColor", QColor(self.MARGIN_FOREGROUND_COLOR))))
        self.setMarginsBackgroundColor(QColor(self.settings.value("pythonConsole/marginBackgroundColor", QColor(self.MARGIN_BACKGROUND_COLOR))))
        caretLineColor = self.settings.value("pythonConsole/caretLineColor", QColor(self.CARET_LINE_COLOR))
        cursorColor = self.settings.value("pythonConsole/cursorColor", QColor(self.CURSOR_COLOR))
        self.setCaretLineBackgroundColor(caretLineColor)
        self.setCaretForegroundColor(cursorColor)

    def setLexers(self):
        self.lexer = QsciLexerPython()

        font = QFontDatabase.systemFont(QFontDatabase.FixedFont)

        loadFont = self.settings.value("pythonConsole/fontfamilytext")
        if loadFont:
            font.setFamily(loadFont)
        fontSize = self.settings.value("pythonConsole/fontsize", type=int)
        if fontSize:
            font.setPointSize(fontSize)

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
        self.lexer.setColor(QColor(self.settings.value("pythonConsole/defaultFontColorEditor", QColor(self.DEFAULT_COLOR))), 13)
        self.lexer.setColor(QColor(Qt.red), 14)
        self.lexer.setFont(font, 1)
        self.lexer.setFont(font, 2)
        self.lexer.setFont(font, 3)
        self.lexer.setFont(font, 4)
        self.lexer.setFont(font, QsciLexerPython.UnclosedString)

        for style in range(0, 33):
            paperColor = QColor(self.settings.value("pythonConsole/paperBackgroundColor", QColor(self.BACKGROUND_COLOR)))
            self.lexer.setPaper(paperColor, style)

        self.setLexer(self.lexer)

    def clearConsole(self):
        self.setText('')
        self.insertInitText()
        self.shell.setFocus()

    def contextMenuEvent(self, e):
        menu = QMenu(self)
        iconRun = QgsApplication.getThemeIcon("console/mIconRunConsole.svg")
        iconClear = QgsApplication.getThemeIcon("console/iconClearConsole.svg")
        iconHideTool = QgsApplication.getThemeIcon("console/iconHideToolConsole.svg")
        iconSettings = QgsApplication.getThemeIcon("console/iconSettingsConsole.svg")
        menu.addAction(iconHideTool,
                       QCoreApplication.translate("PythonConsole", "Hide/Show Toolbar"),
                       self.hideToolBar)
        menu.addSeparator()
        showEditorAction = menu.addAction(
            QCoreApplication.translate("PythonConsole", "Show Editor"),
            self.showEditor)
        menu.addSeparator()
        runAction = menu.addAction(iconRun,
                                   QCoreApplication.translate("PythonConsole", "Enter Selected"),
                                   self.enteredSelected,
                                   QKeySequence(Qt.CTRL + Qt.Key_E))
        clearAction = menu.addAction(iconClear,
                                     QCoreApplication.translate("PythonConsole", "Clear Console"),
                                     self.clearConsole)
        menu.addSeparator()
        copyAction = menu.addAction(
            QCoreApplication.translate("PythonConsole", "Copy"),
            self.copy, QKeySequence.Copy)
        selectAllAction = menu.addAction(
            QCoreApplication.translate("PythonConsole", "Select All"),
            self.selectAll, QKeySequence.SelectAll)
        menu.addSeparator()
        menu.addAction(iconSettings,
                       QCoreApplication.translate("PythonConsole", "Options…"),
                       self.parent.openSettings)
        runAction.setEnabled(False)
        clearAction.setEnabled(False)
        copyAction.setEnabled(False)
        selectAllAction.setEnabled(False)
        showEditorAction.setEnabled(True)
        if self.hasSelectedText():
            runAction.setEnabled(True)
            copyAction.setEnabled(True)
        if not self.text(3) == '':
            selectAllAction.setEnabled(True)
            clearAction.setEnabled(True)
        if self.parent.tabEditorWidget.isVisible():
            showEditorAction.setEnabled(False)
        menu.exec_(self.mapToGlobal(e.pos()))

    def hideToolBar(self):
        tB = self.parent.toolBar
        tB.hide() if tB.isVisible() else tB.show()
        self.shell.setFocus()

    def showEditor(self):
        Ed = self.parent.splitterObj
        if not Ed.isVisible():
            Ed.show()
            self.parent.showEditorButton.setChecked(True)
        self.shell.setFocus()

    def copy(self):
        """Copy text to clipboard... or keyboard interrupt"""
        if self.hasSelectedText():
            text = self.selectedText()
            text = text.replace('>>> ', '').replace('... ', '').strip()  # removing prompts
            QApplication.clipboard().setText(text)
        else:
            raise KeyboardInterrupt

    def enteredSelected(self):
        cmd = self.selectedText()
        self.shell.insertFromDropPaste(cmd)
        self.shell.entered()

    def keyPressEvent(self, e):
        # empty text indicates possible shortcut key sequence so stay in output
        txt = e.text()
        if len(txt) and txt >= " ":
            self.shell.append(txt)
            self.shell.move_cursor_to_end()
            self.shell.setFocus()
            e.ignore()
        else:
            # possible shortcut key sequence, accept it
            e.accept()

    def widgetMessageBar(self, iface, text):
        timeout = iface.messageTimeout()
        self.infoBar.pushMessage(text, Qgis.Info, timeout)
