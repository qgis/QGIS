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
        self.setMarginWidth(1, "000") 
        self.setMarginLineNumbers(1, True)
        self.setMarginsBackgroundColor(QColor("#ffe4e4"))
        
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
        
    def refreshLexerProperties(self):
        self.setLexers()
        
    def setLexers(self):
        self.lexer = QsciLexerPython()
        
        settings = QSettings()
        loadFont = settings.value("pythonConsole/fontfamilytext").toString()
        fontSize = settings.value("pythonConsole/fontsize").toInt()[0]
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
