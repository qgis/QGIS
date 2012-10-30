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
from console_sci import PythonEdit 
import sys
              
class writeOut:
    def __init__(self, edit, out=None, style=None):
        """
        This class allow to write stdout and stderr
        """
        self.editor = edit
        self.out = None
        self.style = style

    def write(self, m):
        if self.style == "traceback":
            self.editor.SendScintilla(QsciScintilla.SCI_SETSTYLING, len(m), 1)
            self.editor.append(m)
            self.editor.SendScintilla(QsciScintilla.SCI_SETSTYLING, len(m), 1)
        else:
            self.editor.append(m)
        self.move_cursor_to_end()

        if self.out:
            self.out.write(m)
            
    def move_cursor_to_end(self):
        """Move cursor to end of text"""
        line, index = self.get_end_pos()
        self.editor.setCursorPosition(line, index)
        self.editor.ensureCursorVisible()
        self.editor.ensureLineVisible(line)
        
    def get_end_pos(self):
        """Return (line, index) position of the last character"""
        line = self.editor.lines() - 1
        return (line, self.editor.text(line).length())
    
    def flush(self):
        pass

class EditorOutput(QsciScintilla):
    def __init__(self, parent=None):
        #QsciScintilla.__init__(self, parent)
        super(EditorOutput,self).__init__(parent)
        # Enable non-ascii chars for editor
        self.setUtf8(True)
        
        sys.stdout = writeOut(self, sys.stdout)
        sys.stderr = writeOut(self, sys.stderr, "traceback")
        
        self.edit = PythonEdit() 
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


        # Folding
        #self.setFolding(QsciScintilla.BoxedTreeFoldStyle)
        #self.setFoldMarginColors(QColor("#99CC66"),QColor("#333300"))
        #self.setWrapMode(QsciScintilla.WrapCharacter)
        
         ## Edge Mode : does not seems to work
        #self.setEdgeMode(QsciScintilla.EdgeLine)
        #self.setEdgeColumn(80)
        #self.setEdgeColor(QColor("#FF0000")) 
        
        self.SendScintilla(QsciScintilla.SCI_SETWRAPMODE, 2)  
        self.SendScintilla(QsciScintilla.SCI_SETHSCROLLBAR, 0)
        
        self.runShortcut = QShortcut(QKeySequence(Qt.CTRL + Qt.Key_E), self)
        self.runShortcut.activated.connect(self.enteredSelected)
        # Reimplemeted copy action to prevent paste prompt (>>>,...) in command view
        self.copyShortcut = QShortcut(QKeySequence(Qt.CTRL + Qt.Key_C), self)
        self.copyShortcut.activated.connect(self.copy)
    
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
        
    def contextMenuEvent(self, e):   
        menu = QMenu(self)
        iconRun = QIcon(":/images/console/iconRunConsole.png")
        runAction = menu.addAction(iconRun, "Enter Selected", self.enteredSelected, QKeySequence(Qt.CTRL + Qt.Key_E))
        menu.addSeparator()
        copyAction = menu.addAction("Copy", self.copy, QKeySequence.Copy)
        runAction.setEnabled(False)
        copyAction.setEnabled(False)
        if self.hasSelectedText():
            runAction.setEnabled(True)
            copyAction.setEnabled(True)
        action = menu.exec_(self.mapToGlobal(e.pos()))
            
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
        
