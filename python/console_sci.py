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
                        QsciLexerPython, 
                        QsciAPIs)

import sys
import os
import traceback
import code

_init_commands = ["from qgis.core import *", "import qgis.utils"]

class PythonEdit(QsciScintilla, code.InteractiveInterpreter):
    def __init__(self, parent=None):
        #QsciScintilla.__init__(self, parent)
        super(PythonEdit,self).__init__(parent)
        code.InteractiveInterpreter.__init__(self, locals=None)
        
        self.current_prompt_pos = None
        self.new_input_line = True 
        
        self.setMarginWidth(0, 0)
        self.setMarginWidth(1, 0)
        self.setMarginWidth(2, 0)
        
        self.buffer = []
        
        self.insertInitText()
        
        self.setCursorPosition(4,4)
        
        self.displayPrompt(False)
        
        for line in _init_commands:
            self.runsource(line)
            
        self.history = QStringList()
        self.historyIndex = 0
          
        # Brace matching: enable for a brace immediately before or after
        # the current position
        self.setBraceMatching(QsciScintilla.SloppyBraceMatch)
        #self.moveToMatchingBrace()
        #self.selectToMatchingBrace()
        
        # Current line visible with special background color
        self.setCaretLineVisible(True)
        self.setCaretLineBackgroundColor(QColor("#ffe4e4"))
        self.setCaretWidth(2)
    
        # Set Python lexer
        # Set style for Python comments (style number 1) to a fixed-width
        # courier.
        self.setLexers(True)
               
        # Indentation
        #self.setAutoIndent(True)
        #self.setIndentationsUseTabs(False)
        #self.setIndentationWidth(4)
        #self.setTabIndents(True)
        #self.setBackspaceUnindents(True)
        #self.setTabWidth(4)

        self.setAutoCompletionThreshold(2)
        self.setAutoCompletionSource(self.AcsAPIs)

        # Don't want to see the horizontal scrollbar at all
        # Use raw message to Scintilla here (all messages are documented
        # here: http://www.scintilla.org/ScintillaDoc.html)
        self.SendScintilla(QsciScintilla.SCI_SETHSCROLLBAR, 0)
    
        # not too small
        #self.setMinimumSize(500, 300)
        
        self.SendScintilla(QsciScintilla.SCI_SETWRAPMODE, 1)
        self.SendScintilla(QsciScintilla.SCI_EMPTYUNDOBUFFER)
        
        ## Disable command key
        ctrl, shift = self.SCMOD_CTRL<<16, self.SCMOD_SHIFT<<16
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('L')+ ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('T')+ ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('D')+ ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('Z')+ ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('Y')+ ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('L')+ ctrl+shift)
        
        ## New QShortcut = ctrl+space/ctrl+alt+space for Autocomplete
        self.newShortcutCS = QShortcut(QKeySequence(Qt.CTRL + Qt.Key_Space), self)
        self.newShortcutCAS = QShortcut(QKeySequence(Qt.CTRL + Qt.ALT + Qt.Key_Space), self)
        self.newShortcutCS.activated.connect(self.autoComplete)
        self.newShortcutCAS.activated.connect(self.autoComplete)
    
    def autoComplete(self):
        self.autoCompleteFromAll()
        
    def clearConsole(self):
        """Clear the contents of the console."""
        self.setText('')
        self.insertInitText()
        self.displayPrompt(False)
        self.setFocus()
        
    def commandConsole(self, command):
        line, pos = self.getCurLine()
        selCmd= self.text(line).length()
        self.setSelection(line, 4, line, selCmd)
        self.removeSelectedText()
        if command == "iface":
            """Import QgisInterface class"""
            self.append('from qgis.utils import iface')
            self.move_cursor_to_end()
        elif command == "sextante":
            """Import Sextante class"""
            self.append('from sextante.core.Sextante import Sextante')
            self.move_cursor_to_end()
        elif command == "cLayer":
            """Retrive current Layer from map camvas"""
            self.append('cLayer = iface.mapCanvas().currentLayer()')
            self.move_cursor_to_end()
        self.setFocus()
        
    def setLexers(self, lexer):
        if lexer:
            font = QFont()
            font.setFamily('Mono') ## Courier New
            font.setFixedPitch(True)
            ## check platform for font size
            if sys.platform.startswith('darwin'):
                font.setPointSize(13)
            else:
                font.setPointSize(10)
            self.setFont(font)
            self.setMarginsFont(font)
            self.lexer = QsciLexerPython()
            self.lexer.setDefaultFont(font)
            #self.lexer.setDefaultFont(QFont('Mono', 10, 0, False))
            #self.lexer.setDefaultColor(Qt.darkGray)
            self.lexer.setColor(Qt.red, 1)
            self.lexer.setColor(Qt.darkGreen, 5)
            self.lexer.setColor(Qt.darkBlue, 15)
            self.lexer.setFont(font, 1)
            self.lexer.setFont(font, 3)
            self.lexer.setFont(font, 4)
            self.api = QsciAPIs(self.lexer)
            self.api.loadPrepared(QString(os.path.dirname(__file__) + "/api/pyqgis_master.pap"))
#            self.api.load(os.path.dirname(__file__) + "/api/PyQGIS_1.8.api")
#            self.api.load(os.path.dirname(__file__) + "/api/osgeo_gdal-ogr_1.9.1-1.api")
#            self.api.load("qgis.networkanalysis.api")
#            self.api.load("qgis.gui.api")
#            self.api.load("qgis.core.api")
#            self.api.load("qgis.analysis.api")

#            self.api.prepare()
#            self.lexer.setAPIs(self.api)
            self.setLexer(self.lexer)
            
     ## TODO: show completion list for file and directory       
#    def show_completion_list(self, completions, text):
#        """Private method to display the possible completions"""
#        if len(completions) == 0:
#            return
#        if len(completions) > 1:
#            self.showUserList(1, QStringList(sorted(completions)))
#            self.completion_chars = 1
#        else:
#            txt = completions[0]
#            if text != "":
#                txt = txt.replace(text, "")
#            self.insert(txt)
#            self.completion_chars = 0
#
#    def show_file_completion(self):
#        """Display a completion list for files and directories"""
#        cwd = os.getcwdu()
#        self.show_completion_list(self.listdir_fullpath('/'), cwd)
#        
#    def listdir_fullpath(self, d):
#        return [os.path.join(d, f) for f in os.listdir(d)]

        
    def insertInitText(self):
        #self.setLexers(False)
        txtInit = ("## To access Quantum GIS environment from this console\n"
                    "## use qgis.utils.iface object (instance of QgisInterface class).\n\n")
        initText = self.setText(txtInit)

    def getCurrentPos(self):
        """ Get the position (as an int) of the cursor. 
        getCursorPosition() returns a (linenr, index) tuple.
        """        
        return self.SendScintilla(self.SCI_GETCURRENTPOS)

    def getText(self):
        """ Get the text as a unicode string. """
        value = self.getBytes().decode('utf-8')
        # print (value) printing can give an error because the console font
        # may not have all unicode characters
        return value

    def getBytes(self):
        """ Get the text as bytes (utf-8 encoded). This is how
        the data is stored internally. """
        len = self.SendScintilla(self.SCI_GETLENGTH)+1
        bb = QByteArray(len,'0')
        N = self.SendScintilla(self.SCI_GETTEXT, len, bb)
        return bytes(bb)[:-1]

    def getTextLength(self):
        return self.SendScintilla(QsciScintilla.SCI_GETLENGTH)

    def getLine(self, linenr):
        """ Get the bytes on the given line number. """
        len = self.SendScintilla(QsciScintilla.SCI_LINELENGTH)+1
        bb = QByteArray(len,'0')
        N = self.SendScintilla(QsciScintilla.SCI_GETLINE, len, bb)
        return bytes(bb)[:-1]
    
    def getCurLine(self):
        """ Get the current line (as a string) and the 
        position of the cursor in it. """
        linenr, index = self.getCursorPosition()
        #line = self.getLine(linenr) #.decode('utf-8')
        return linenr, index
    
    def get_end_pos(self):
        """Return (line, index) position of the last character"""
        line = self.lines() - 1
        return (line, self.text(line).length())
    
    def is_cursor_at_end(self):
        """Return True if cursor is at the end of text"""
        cline, cindex = self.getCursorPosition()
        return (cline, cindex) == self.get_end_pos()
    
    def move_cursor_to_end(self):
        """Move cursor to end of text"""
        line, index = self.get_end_pos()
        self.setCursorPosition(line, index)
        self.ensureCursorVisible()
        
    def on_new_line(self):
        """On new input line"""
        self.move_cursor_to_end()
        self.current_prompt_pos = self.getCursorPosition()
        self.new_input_line = False
        
    def is_cursor_on_last_line(self):
        """Return True if cursor is on the last line"""
        cline, _ = self.getCursorPosition()
        return cline == self.lines() - 1
    
    def new_prompt(self, prompt):
        """
        Print a new prompt and save its (line, index) position
        """
        self.write(prompt, prompt=True)
        # now we update our cursor giving end of prompt
        self.current_prompt_pos = self.getCursorPosition()
        self.ensureCursorVisible()
        
    def check_selection(self):
        """
        Check if selected text is r/w,
        otherwise remove read-only parts of selection
        """
        #if self.current_prompt_pos is None:
            #self.move_cursor_to_end()
            #return
        line_from, index_from, line_to, index_to = self.getSelection()
        pline, pindex = self.getCursorPosition()
        if line_from < pline or \
           (line_from == pline and index_from < pindex):
            self.setSelection(pline, pindex, line_to, index_to)

    def displayPrompt(self, more=False):
        self.append("... ") if more else self.append(">>> ")
        self.move_cursor_to_end()
        
    def updateHistory(self, command):
        if isinstance(command, QStringList):
            for line in command:
                self.history.append(line)
        elif not command == "":
            if len(self.history) <= 0 or \
            not command == self.history[-1]:
                self.history.append(command)
        self.historyIndex = len(self.history)
        
    def showPrevious(self):
        if self.historyIndex < len(self.history) and not self.history.isEmpty():
            line, pos = self.getCurLine()
            selCmd= self.text(line).length()
            self.setSelection(line, 4, line, selCmd)
            self.removeSelectedText()
            self.historyIndex += 1
            if self.historyIndex == len(self.history):
                self.insert("")
                pass
            else:
                self.insert(self.history[self.historyIndex])
            self.move_cursor_to_end()
            #self.SendScintilla(QsciScintilla.SCI_DELETEBACK)
            

    def showNext(self):
        if  self.historyIndex > 0 and not self.history.isEmpty():
            line, pos = self.getCurLine()
            selCmd = self.text(line).length()
            self.setSelection(line, 4, line, selCmd)
            self.removeSelectedText()
            self.historyIndex -= 1
            if self.historyIndex == len(self.history):
                self.insert("")
            else:
                self.insert(self.history[self.historyIndex])
            self.move_cursor_to_end()
            #self.SendScintilla(QsciScintilla.SCI_DELETEBACK)

    def keyPressEvent(self, e):  
        linenr, index = self.getCurLine()
        if not self.is_cursor_on_last_line() or index < 4:
            if e.modifiers() & Qt.ControlModifier or e.modifiers() & Qt.MetaModifier:
                if e.key() == Qt.Key_C or e.key() == Qt.Key_A:
                    QsciScintilla.keyPressEvent(self, e)
            else:
                # all other keystrokes get sent to the input line
                self.move_cursor_to_end()
                #pass
        else:
            if (e.key() == Qt.Key_Return or e.key() == Qt.Key_Enter) and not self.isListActive():
                    self.entered()
            elif e.modifiers() & Qt.ControlModifier:
                if e.key() == Qt.Key_V:
                    self.paste()             
            elif e.key() == Qt.Key_Backspace:
                curPos, pos = self.getCursorPosition()
                line = self.lines() -1
                if curPos < line -1 or pos < 5:
                    return
                #else:
                    #self.move_cursor_to_end()
                QsciScintilla.keyPressEvent(self, e)
            elif e.key() == Qt.Key_Delete:
                if self.hasSelectedText():
                    self.check_selection()
                    self.removeSelectedText()
                elif self.is_cursor_on_last_line():
                    self.SendScintilla(QsciScintilla.SCI_CLEAR)
                e.accept()
            elif e.key() == Qt.Key_Home:
                self.setCursorPosition(linenr,4)
                self.ensureCursorVisible()
            elif e.key() == Qt.Key_Down and not self.isListActive():
                self.showPrevious()
            elif e.key() == Qt.Key_Up and not self.isListActive():
                self.showNext()
            elif e.key() == Qt.Key_Left:
                e.accept()
                if e.modifiers() & Qt.ShiftModifier:
                    if index > 4:
                        if e.modifiers() & Qt.ControlModifier:
                            self.SendScintilla(QsciScintilla.SCI_WORDLEFTEXTEND)
                        else:
                            self.SendScintilla(QsciScintilla.SCI_CHARLEFTEXTEND)
                else:
                    if index > 4:
                        if e.modifiers() & Qt.ControlModifier:
                            self.SendScintilla(QsciScintilla.SCI_WORDLEFT)
                        else:
                            self.SendScintilla(QsciScintilla.SCI_CHARLEFT)
            elif e.key() == Qt.Key_Right:
                e.accept()
                if e.modifiers() & Qt.ShiftModifier:
                    if index >= 4:
                        if e.modifiers() & Qt.ControlModifier:
                            self.SendScintilla(QsciScintilla.SCI_WORDRIGHTEXTEND)
                        else:
                            self.SendScintilla(QsciScintilla.SCI_CHARRIGHTEXTEND)
                else:
                    if index >= 4:
                        if e.modifiers() & Qt.ControlModifier:
                            self.SendScintilla(QsciScintilla.SCI_WORDRIGHT)
                        else:
                            self.SendScintilla(QsciScintilla.SCI_CHARRIGHT)
            elif e.key() == Qt.Key_Delete:
                if self.hasSelectedText():
                    self.check_selection()
                    self.removeSelectedText()
                elif self.is_cursor_on_last_line():
                    self.SendScintilla(QsciScintilla.SCI_CLEAR)
                event.accept()
            ## TODO: press event for auto-completion file directory
            #elif e.key() == Qt.Key_Tab:
                #self.show_file_completion()
                #else:
                    #self.on_new_line()
            else:
                QsciScintilla.keyPressEvent(self, e)
                
    def paste(self):
        """Reimplement QScintilla method"""
        stringPaste = unicode(QApplication.clipboard().text())
        self.insertFromDropPaste(stringPaste)
        
    ## Drag and drop
    def dragEnterEvent(self, e):
        if e.mimeData().hasFormat('text/plain'):
            e.accept()
        else:
            e.ignore() 

    def dropEvent(self, e):
        stringDrag = e.mimeData().text()
        self.insertFromDropPaste(stringDrag)
        
    def insertFromDropPaste(self, textDP):
        pasteList = QStringList()
        pasteList = textDP.split("\n")
        for line in pasteList[:-1]:
            self.append(line)
            self.move_cursor_to_end()
            #self.SendScintilla(QsciScintilla.SCI_DELETEBACK)
            self.runCommand(unicode(self.currentCommand()))
        self.append(unicode(pasteList[-1]))
        self.move_cursor_to_end()

    def getTextFromEditor(self):
        text = self.text()
        textList = QStringList()
        textList = text.split("\n")
        return textList
    
    def insertTextFromFile(self, listOpenFile):
        for line in listOpenFile[:-1]:
            self.append(line)
            self.move_cursor_to_end()
            self.SendScintilla(QsciScintilla.SCI_DELETEBACK)
            self.runCommand(unicode(self.currentCommand()))
        self.append(unicode(listOpenFile[-1]))
        self.move_cursor_to_end()
        self.SendScintilla(QsciScintilla.SCI_DELETEBACK)
            
    def entered(self):
        self.move_cursor_to_end()
        self.runCommand( unicode(self.currentCommand()) )
        self.setFocus()
        #self.SendScintilla(QsciScintilla.SCI_EMPTYUNDOBUFFER)
        
    def currentCommand(self):
        linenr, index = self.getCurLine()
        #for i in range(0, linenr):
        txtLength = self.text(linenr).length()
        string = self.text()
        cmdLine = string.right(txtLength - 4)
        cmd = str(cmdLine)
        return cmd

    def runCommand(self, cmd):
        self.updateHistory(cmd)
        self.SendScintilla(QsciScintilla.SCI_NEWLINE)
        self.buffer.append(cmd)
        src = "\n".join(self.buffer)
        more = self.runsource(src, "<input>")
        if not more:
            self.buffer = []
                       
        output = sys.stdout.get_and_clean_data()
        if output:
            self.append(output)
            
        self.move_cursor_to_end()
        self.displayPrompt(more)

    def write(self, txt):
        self.SendScintilla(QsciScintilla.SCI_SETSTYLING, len(txt), 1)
        self.append(txt)
        self.SendScintilla(QsciScintilla.SCI_SETSTYLING, len(txt), 1)