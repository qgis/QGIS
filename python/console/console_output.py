# -*- coding:utf-8 -*-
"""
/***************************************************************************
Python Conosle for QGIS
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

class writeOut:
    def __init__(self, edit, out=None, style=None):
        """
        This class allow to write stdout and stderr
        """
        self.outputArea = edit
        self.out = None
        self.style = style

    def write(self, m):
        if self.style == "traceback":
            # Show errors in red
            pos = self.outputArea.SendScintilla(QsciScintilla.SCI_GETCURRENTPOS)
            self.outputArea.SendScintilla(QsciScintilla.SCI_STARTSTYLING, pos, 31)
            self.outputArea.append(m)
            self.outputArea.SendScintilla(QsciScintilla.SCI_SETSTYLING, len(m), 1)
        else:
            self.outputArea.append(m)
        self.move_cursor_to_end()

        if self.out:
            self.out.write(m)

    def move_cursor_to_end(self):
        """Move cursor to end of text"""
        line, index = self.get_end_pos()
        self.outputArea.setCursorPosition(line, index)
        self.outputArea.ensureCursorVisible()
        self.outputArea.ensureLineVisible(line)

    def get_end_pos(self):
        """Return (line, index) position of the last character"""
        line = self.outputArea.lines() - 1
        return (line, self.outputArea.text(line).length())

    def flush(self):
        pass

class EditorOutput(QsciScintilla):
    def __init__(self, parent=None):
        #QsciScintilla.__init__(self, parent)
        super(EditorOutput,self).__init__(parent)
        self.parent = parent
        self.edit = self.parent.edit

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
        sys.stderr = writeOut(self, sys.stderr, "traceback")

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
        #fm = QFontMetrics(font)
        self.setMarginsFont(font)
        self.setMarginWidth(1, "00000")
        self.setMarginLineNumbers(1, True)
        self.setMarginsForegroundColor(QColor("#3E3EE3"))
        self.setMarginsBackgroundColor(QColor("#f9f9f9"))
        self.setCaretLineVisible(True)
        self.setCaretLineBackgroundColor(QColor("#fcf3ed"))

        self.setMinimumHeight(120)

        # Folding
        #self.setFolding(QsciScintilla.BoxedTreeFoldStyle)
        #self.setFoldMarginColors(QColor("#99CC66"),QColor("#333300"))
        #self.setWrapMode(QsciScintilla.WrapCharacter)

        ## Edge Mode
        #self.setEdgeMode(QsciScintilla.EdgeLine)
        #self.setEdgeColumn(80)
        #self.setEdgeColor(QColor("#FF0000"))

        self.setWrapMode(QsciScintilla.WrapCharacter)
        self.SendScintilla(QsciScintilla.SCI_SETHSCROLLBAR, 0)

        self.runShortcut = QShortcut(QKeySequence(Qt.CTRL + Qt.Key_E), self)
        self.runShortcut.activated.connect(self.enteredSelected)
        # Reimplemeted copy action to prevent paste prompt (>>>,...) in command view
        self.copyShortcut = QShortcut(QKeySequence.Copy, self)
        self.copyShortcut.activated.connect(self.copy)
        self.selectAllShortcut = QShortcut(QKeySequence.SelectAll, self)
        self.selectAllShortcut.activated.connect(self.selectAll)

    def insertInitText(self):
        txtInit = QCoreApplication.translate("PythonConsole",
                                             "## To access Quantum GIS environment from this console\n"
                                             "## use iface object (instance of QgisInterface class).\n"
                                             "## Type help(iface) for more info and list of methods.\n\n")
        initText = self.setText(txtInit)

    def refreshLexerProperties(self):
        self.setLexers()

    def setLexers(self):
        self.lexer = QsciLexerPython()

        settings = QSettings()
        loadFont = settings.value("pythonConsole/fontfamilytext", "Monospace").toString()
        fontSize = settings.value("pythonConsole/fontsize", 10).toInt()[0]
        font = QFont(loadFont)
        font.setFixedPitch(True)
        font.setPointSize(fontSize)

        self.lexer.setDefaultFont(font)
        self.lexer.setColor(Qt.red, 1)
        self.lexer.setColor(Qt.darkGreen, 5)
        self.lexer.setColor(Qt.darkBlue, 15)
        self.lexer.setFont(font, 1)
        self.lexer.setFont(font, 2)
        self.lexer.setFont(font, 3)
        self.lexer.setFont(font, 4)

        self.setLexer(self.lexer)

    def getTextFromEditor(self):
        text = self.text()
        textList = text.split("\n")
        return textList

    def clearConsole(self):
        #self.SendScintilla(QsciScintilla.SCI_CLEARALL)
        self.setText('')
        self.insertInitText()
        self.edit.setFocus()

    def contextMenuEvent(self, e):
        menu = QMenu(self)
        iconRun = QgsApplication.getThemeIcon("console/iconRunConsole.png")
        iconPastebin = QgsApplication.getThemeIcon("console/iconCodepadConsole.png")
        iconClear = QgsApplication.getThemeIcon("console/iconClearConsole.png")
        iconHideTool = QgsApplication.getThemeIcon("console/iconHideToolConsole.png")
        hideToolBar = menu.addAction(iconHideTool,
                                     "Hide/Show Toolbar",
                                     self.hideToolBar)
        menu.addSeparator()
        runAction = menu.addAction(iconRun,
                                   "Enter Selected",
                                   self.enteredSelected,
                                   QKeySequence(Qt.CTRL + Qt.Key_E))
        clearAction = menu.addAction(iconClear,
                                     "Clear console",
                                     self.clearConsole)
        menu.addSeparator()
        copyAction = menu.addAction("Copy",
                                    self.copy,
                                    QKeySequence.Copy)
        pastebinAction = menu.addAction(iconPastebin,
                                        "Share on codepad",
                                        self.pastebin)
        menu.addSeparator()
        selectAllAction = menu.addAction("Select All",
                                         self.selectAll,
                                         QKeySequence.SelectAll)
        runAction.setEnabled(False)
        clearAction.setEnabled(False)
        copyAction.setEnabled(False)
        pastebinAction.setEnabled(False)
        selectAllAction.setEnabled(False)
        if self.hasSelectedText():
            runAction.setEnabled(True)
            copyAction.setEnabled(True)
            pastebinAction.setEnabled(True)
        if not self.text(3) == '':
            selectAllAction.setEnabled(True)
            clearAction.setEnabled(True)
        action = menu.exec_(self.mapToGlobal(e.pos()))

    def hideToolBar(self):
        tB = self.parent.toolBar
        tB.hide() if tB.isVisible() else tB.show()
        self.edit.setFocus()

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
        self.edit.insertFromDropPaste(cmd)
        self.edit.entered()

    def keyPressEvent(self, e):
        # empty text indicates possible shortcut key sequence so stay in output
        txt = e.text()
        if txt.length() and txt >= " ":
            self.edit.append(txt)
            self.edit.move_cursor_to_end()
            self.edit.setFocus()
            e.ignore()
        else:
            # possible shortcut key sequence, accept it
            e.accept()

    def pastebin(self):
        import urllib2, urllib
        listText = self.selectedText().split('\n')
        getCmd = []
        for strLine in listText:
            if strLine != "":
            #if s[0:3] in (">>>", "..."):
                # filter for special command (_save,_clear) and comment
                if strLine[4] != "_" and strLine[:2] != "##":
                    strLine.replace(">>> ", "").replace("... ", "")
                    getCmd.append(unicode(strLine))
        pasteText= u"\n".join(getCmd)
        url = 'http://codepad.org'
        values = {'lang' : 'Python',
                  'code' : pasteText,
                  'submit':'Submit'}
        try:
            response = urllib2.urlopen(url, urllib.urlencode(values))
            url = response.read()
            for href in url.split("</a>"):
                if "Link:" in href:
                    ind=href.index('Link:')
                    found = href[ind+5:]
                    for i in found.split('">'):
                        if '<a href=' in i:
                             link = i.replace('<a href="',"").strip()
            if link:
                QApplication.clipboard().setText(link)
                msgText = QCoreApplication.translate('PythonConsole', 'URL copied to clipboard.')
                self.parent.callWidgetMessageBar(msgText)
        except urllib2.URLError, e:
            msgText = QCoreApplication.translate('PythonConsole', 'Connection error: ')
            self.parent.callWidgetMessageBar(msgText + str(e.args))

    def widgetMessageBar(self, iface, text):
        timeout = iface.messageTimeout()
        self.infoBar.pushMessage(text, QgsMessageBar.INFO, timeout)
