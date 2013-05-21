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

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.Qsci import (QsciScintilla,
                        QsciScintillaBase,
                        QsciLexerPython)
from qgis.core import QgsApplication
from qgis.gui import QgsMessageBar
import sys
import socket

class writeOut:
    def __init__(self, shellOut, out=None, style=None):
        """
        This class allow to write stdout and stderr
        """
        self.sO = shellOut
        self.out = None
        self.style = style

    def write(self, m):
        if self.style == "_traceback":
            # Show errors in red
            pos = self.sO.SendScintilla(QsciScintilla.SCI_GETCURRENTPOS)
            self.sO.SendScintilla(QsciScintilla.SCI_STARTSTYLING, pos, 31)
            self.sO.append(m)
            self.sO.SendScintilla(QsciScintilla.SCI_SETSTYLING, len(m), 1)
        else:
            self.sO.append(m)

        if self.out:
            self.out.write(m)

        self.move_cursor_to_end()

    def move_cursor_to_end(self):
        """Move cursor to end of text"""
        line, index = self.get_end_pos()
        self.sO.setCursorPosition(line, index)
        self.sO.ensureCursorVisible()
        self.sO.ensureLineVisible(line)

    def get_end_pos(self):
        """Return (line, index) position of the last character"""
        line = self.sO.lines() - 1
        return (line, self.sO.text(line).length())

    def flush(self):
        pass

class ShellOutputScintilla(QsciScintilla):
    def __init__(self, parent=None):
        super(ShellOutputScintilla,self).__init__(parent)
        self.parent = parent
        self.shell = self.parent.shell

        self.settings = QSettings()

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
        self.setLexers()
        self.setReadOnly(True)

        # Set the default font
        font = QFont()
        font.setFamily('Courier')
        font.setFixedPitch(True)
        font.setPointSize(10)
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
        self.setMarginsForegroundColor(QColor("#3E3EE3"))
        self.setMarginsBackgroundColor(QColor("#f9f9f9"))
        self.setCaretLineVisible(True)
        self.setCaretLineBackgroundColor(QColor("#fcf3ed"))

        self.setMinimumHeight(120)

        self.setWrapMode(QsciScintilla.WrapCharacter)
        self.SendScintilla(QsciScintilla.SCI_SETHSCROLLBAR, 0)

        self.runScut = QShortcut(QKeySequence(Qt.CTRL + Qt.Key_E), self)
        self.runScut.setContext(Qt.WidgetShortcut)
        self.runScut.activated.connect(self.enteredSelected)
        # Reimplemeted copy action to prevent paste prompt (>>>,...) in command view
        self.copyShortcut = QShortcut(QKeySequence.Copy, self)
        self.copyShortcut.activated.connect(self.copy)
        self.selectAllShortcut = QShortcut(QKeySequence.SelectAll, self)
        self.selectAllShortcut.activated.connect(self.selectAll)

    def insertInitText(self):
        txtInit = QCoreApplication.translate("PythonConsole",
                                             "Python %1 on %2\n"
                                             "## Type help(iface) for more info and list of methods.\n").arg(sys.version,  socket.gethostname())
        initText = self.setText(txtInit)

    def refreshLexerProperties(self):
        self.setLexers()

    def setLexers(self):
        self.lexer = QsciLexerPython()

        loadFont = self.settings.value("pythonConsole/fontfamilytext", "Monospace").toString()
        fontSize = self.settings.value("pythonConsole/fontsize", 10).toInt()[0]
        font = QFont(loadFont)
        font.setFixedPitch(True)
        font.setPointSize(fontSize)
        font.setStyleHint(QFont.TypeWriter)
        font.setStretch(QFont.SemiCondensed)
        font.setLetterSpacing(QFont.PercentageSpacing, 87.0)
        font.setBold(False)

        self.lexer.setDefaultFont(font)
        self.lexer.setColor(Qt.red, 1)
        self.lexer.setColor(Qt.darkGreen, 5)
        self.lexer.setColor(Qt.darkBlue, 15)
        self.lexer.setFont(font, 1)
        self.lexer.setFont(font, 2)
        self.lexer.setFont(font, 3)
        self.lexer.setFont(font, 4)

        self.setLexer(self.lexer)

    def clearConsole(self):
        self.setText('')
        self.insertInitText()
        self.shell.setFocus()

    def contextMenuEvent(self, e):
        menu = QMenu(self)
        iconRun = QgsApplication.getThemeIcon("console/iconRunConsole.png")
        iconClear = QgsApplication.getThemeIcon("console/iconClearConsole.png")
        iconHideTool = QgsApplication.getThemeIcon("console/iconHideToolConsole.png")
        iconSettings = QgsApplication.getThemeIcon("console/iconSettingsConsole.png")
        hideToolBar = menu.addAction(iconHideTool,
                                     QCoreApplication.translate("PythonConsole",
                                                                "Hide/Show Toolbar"),
                                     self.hideToolBar)
        menu.addSeparator()
        showEditorAction = menu.addAction(QCoreApplication.translate("PythonConsole",
                                                                     "Show Editor"),
                                          self.showEditor)
        menu.addSeparator()
        runAction = menu.addAction(iconRun,
                                   QCoreApplication.translate("PythonConsole",
                                                              "Enter Selected"),
                                   self.enteredSelected,
                                   QKeySequence(Qt.CTRL + Qt.Key_E))
        clearAction = menu.addAction(iconClear,
                                     QCoreApplication.translate("PythonConsole",
                                                                "Clear console"),
                                     self.clearConsole)
        menu.addSeparator()
        copyAction = menu.addAction(QCoreApplication.translate("PythonConsole",
                                                               "Copy"),
                                    self.copy,
                                    QKeySequence.Copy)
        menu.addSeparator()
        selectAllAction = menu.addAction(QCoreApplication.translate("PythonConsole",
                                                                    "Select All"),
                                         self.selectAll,
                                         QKeySequence.SelectAll)
        menu.addSeparator()
        settingsDialog = menu.addAction(iconSettings,
                                        QCoreApplication.translate("PythonConsole",
                                                                   "Settings"),
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
        action = menu.exec_(self.mapToGlobal(e.pos()))

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
            text = unicode(self.selectedText())
            text = text.replace('>>> ', '').replace('... ', '').strip() # removing prompts
            QApplication.clipboard().setText(text)
        else:
            self.emit(SIGNAL("keyboard_interrupt()"))

    def enteredSelected(self):
        cmd = self.selectedText()
        self.shell.insertFromDropPaste(cmd)
        self.shell.entered()

    def keyPressEvent(self, e):
        # empty text indicates possible shortcut key sequence so stay in output
        txt = e.text()
        if txt.length() and txt >= " ":
            self.shell.append(txt)
            self.shell.move_cursor_to_end()
            self.shell.setFocus()
            e.ignore()
        else:
            # possible shortcut key sequence, accept it
            e.accept()

    def widgetMessageBar(self, iface, text):
        timeout = iface.messageTimeout()
        self.infoBar.pushMessage(text, QgsMessageBar.INFO, timeout)
