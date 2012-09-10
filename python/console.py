# -*- coding:utf-8 -*-
"""
/***************************************************************************
Python Conosle for QGIS
                             -------------------
begin                : 2012-09-xx 
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
from qgis.utils import iface
from console_sci import PythonEdit
from help import HelpDialog

import sys
import os

_console = None

def show_console():
  """ called from QGIS to open the console """
  global _console
  if _console is None:
    _console = PythonConsole(iface.mainWindow())
    _console.show() # force show even if it was restored as hidden
  else:
    _console.setVisible(not _console.isVisible())
  # set focus to the edit box so the user can start typing
  if _console.isVisible():
    _console.activateWindow()
    _console.edit.setFocus()

_old_stdout = sys.stdout
_console_output = None

# hook for python console so all output will be redirected
# and then shown in console
def console_displayhook(obj):
    global _console_output
    _console_output = obj

class QgisOutputCatcher:
    def __init__(self):
        self.data = ''
    def write(self, stuff):
        self.data += stuff
    def get_and_clean_data(self):
        tmp = self.data
        self.data = ''
        return tmp
    def flush(self):
        pass

sys.stdout = QgisOutputCatcher()

class ConsoleHighlighter(QSyntaxHighlighter):
    EDIT_LINE, ERROR, OUTPUT, INIT = range(4)
    def __init__(self, doc):
        QSyntaxHighlighter.__init__(self,doc)
        formats = { self.OUTPUT    : Qt.black,
            self.ERROR     : Qt.red,
            self.EDIT_LINE : Qt.darkGreen,
            self.INIT      : Qt.gray }
        self.f = {}
        for tag, color in formats.iteritems():
              self.f[tag] = QTextCharFormat()
              self.f[tag].setForeground(color)

    def highlightBlock(self, txt):
        size = txt.length()
        state = self.currentBlockState()
        if state == self.OUTPUT or state == self.ERROR or state == self.INIT:
              self.setFormat(0,size, self.f[state])
        # highlight prompt only
        if state == self.EDIT_LINE:
              self.setFormat(0,3, self.f[self.EDIT_LINE])

class PythonConsole(QDockWidget):
    def __init__(self, parent=None):
        QDockWidget.__init__(self, parent)
        self.setObjectName("Python Console")
        #self.setAllowedAreas(Qt.BottomDockWidgetArea)
        
        self.widgetButton = QWidget()
        self.widgetEdit = QWidget()
        
        self.toolBar = QToolBar()
        self.toolBar.setEnabled(True)
        #self.toolBar.setFont(font)
        self.toolBar.setFocusPolicy(Qt.NoFocus)
        self.toolBar.setContextMenuPolicy(Qt.DefaultContextMenu)
        self.toolBar.setLayoutDirection(Qt.LeftToRight)
        self.toolBar.setIconSize(QSize(24, 24))
        self.toolBar.setOrientation(Qt.Vertical)
        self.toolBar.setMovable(True)
        self.toolBar.setFloatable(True)
        #self.toolBar.setAllowedAreas(Qt.LeftToolBarArea)
        #self.toolBar.setAllowedAreas(Qt.RightToolBarArea)
        #self.toolBar.setObjectName(_fromUtf8("toolMappa"))
        
        self.b = QVBoxLayout(self.widgetButton)
        self.e = QHBoxLayout(self.widgetEdit)
        
        self.e.setMargin(0)
        self.b.setMargin(0)
        
        ## Action for Clear button
        self.clearButton = QAction(parent)
        self.clearButton.setCheckable(False)
        self.clearButton.setEnabled(True)
        self.clearButton.setIcon(QIcon("icon/iconClearConsole.png"))
        self.clearButton.setMenuRole(QAction.PreferencesRole)
        self.clearButton.setIconVisibleInMenu(True)
        self.clearButton.setToolTip('Clear console')
        ## Action for paste snippets code
        self.clearButton = QAction(parent)
        self.clearButton.setCheckable(False)
        self.clearButton.setEnabled(True)
        self.clearButton.setIcon(QIcon(os.path.dirname(__file__) + "/iconConsole/iconClearConsole.png"))
        self.clearButton.setMenuRole(QAction.PreferencesRole)
        self.clearButton.setIconVisibleInMenu(True)
        self.clearButton.setToolTip('Clear console')
        ## Action for paste snippets code
#        self.currentLayerButton = QAction(parent)
#        self.currentLayerButton.setCheckable(False)
#        self.currentLayerButton.setEnabled(True)
#        self.currentLayerButton.setIcon(QIcon("icon/iconTempConsole.png"))
#        self.currentLayerButton.setMenuRole(QAction.PreferencesRole)
#        self.currentLayerButton.setIconVisibleInMenu(True)
        ##
        self.loadIfaceButton = QAction(parent)
        self.loadIfaceButton.setCheckable(False)
        self.loadIfaceButton.setEnabled(True)
        self.loadIfaceButton.setIcon(QIcon(os.path.dirname(__file__) + "/iconConsole/iconTempConsole.png"))
        self.loadIfaceButton.setMenuRole(QAction.PreferencesRole)
        self.loadIfaceButton.setIconVisibleInMenu(True)
        self.loadIfaceButton.setToolTip('Import iface class')
        ## Action for Open File
        self.openFileButton = QAction(parent)
        self.openFileButton.setCheckable(False)
        self.openFileButton.setEnabled(True)
        self.openFileButton.setIcon(QIcon(os.path.dirname(__file__) + "/iconConsole/iconOpenConsole.png"))
        self.openFileButton.setMenuRole(QAction.PreferencesRole)
        self.openFileButton.setIconVisibleInMenu(True)
        self.openFileButton.setToolTip('Open script file')
        ## Action for Save File
        self.saveFileButton = QAction(parent)
        self.saveFileButton.setCheckable(False)
        self.saveFileButton.setEnabled(True)
        self.saveFileButton.setIcon(QIcon(os.path.dirname(__file__) + "/iconConsole/iconSaveConsole.png"))
        self.saveFileButton.setMenuRole(QAction.PreferencesRole)
        self.saveFileButton.setIconVisibleInMenu(True)
        self.saveFileButton.setToolTip('Save to file')
        ## Action for Run script
        self.runButton = QAction(parent)
        self.runButton.setCheckable(False)
        self.runButton.setEnabled(True)
        self.runButton.setIcon(QIcon(os.path.dirname(__file__) + "/iconConsole/iconRunConsole.png"))
        self.runButton.setMenuRole(QAction.PreferencesRole)
        self.runButton.setIconVisibleInMenu(True)
        self.runButton.setToolTip('Run command')
        ## Help action
        self.helpButton = QAction(parent)
        self.helpButton.setCheckable(False)
        self.helpButton.setEnabled(True)
        self.helpButton.setIcon(QIcon(os.path.dirname(__file__) + "/iconConsole/iconHelpConsole.png"))
        self.helpButton.setMenuRole(QAction.PreferencesRole)
        self.helpButton.setIconVisibleInMenu(True)
        self.helpButton.setToolTip('Help')
        
        self.toolBar.addAction(self.clearButton)
        #self.toolBar.addAction(self.currentLayerButton)
        self.toolBar.addAction(self.loadIfaceButton)
        self.toolBar.addAction(self.openFileButton)
        self.toolBar.addAction(self.saveFileButton)
        self.toolBar.addAction(self.helpButton)
        self.toolBar.addAction(self.runButton)
        
        self.b.addWidget(self.toolBar)
        self.edit = PythonEdit()
        
        self.setWidget(self.widgetEdit)
        
        self.e.addWidget(self.widgetButton)
        self.e.addWidget(self.edit)
        
        self.edit.setFocus()
        
        self.setWindowTitle(QCoreApplication.translate("PythonConsole", "Python Console"))
        self.clearButton.activated.connect(self.edit.clearConsole)
        #self.currentLayerButton.activated.connect(self.cLayer)
        self.loadIfaceButton.activated.connect(self.iface)
        self.runButton.activated.connect(self.edit.entered)
        self.openFileButton.activated.connect(self.openScriptFile)
        self.saveFileButton.activated.connect(self.saveScriptFile)
        self.helpButton.activated.connect(self.openHelp)
        # try to restore position from stored main window state
        if not iface.mainWindow().restoreDockWidget(self):
            iface.mainWindow().addDockWidget(Qt.BottomDockWidgetArea, self)
            
    def cLayer(self):
        self.edit.commandConsole('cLayer')
        
    def iface(self):
       self.edit.commandConsole('iface')
       
    def openScriptFile(self):
        scriptFile = QFileDialog.getOpenFileName(
                        self, "Open File", "", "Script file (*.py)")
        if scriptFile.isEmpty() == False:
            oF = open(scriptFile, 'r')
            listScriptFile = []
            for line in oF:
                if line != "\n":
                    listScriptFile.append(line)
            self.edit.insertTextFromFile(listScriptFile)
            
        
    def saveScriptFile(self):
        scriptFile = QFileDialog()
        scriptFile.setDefaultSuffix(".py")
        fName = scriptFile.getSaveFileName(
                        self, "Save file", QString(), "Script file (*.py)")
            
        if fName.isEmpty() == False:
            filename = str(fName)
            if not filename.endswith(".py"):
                fName += ".py"
            sF = open(fName,'w')
            listText = self.edit.getTextFromEditor()
            is_first_line = True
            for s in listText:
                if s[0:3] in (">>>", "..."):
                    s.replace(">>> ", "")
                    s.replace("... ", "")
                    if is_first_line:
                        # see, no write() in this branch
                        is_first_line = False 
                    else:
                        # we've just written a line; add a newline
                        sF.write('\n')
                    sF.write(s)
            sF.close()

    def openHelp(self):
        dlg = HelpDialog()
        dlg.exec_()

    def closeEvent(self, event):
        QWidget.closeEvent(self, event)

        
if __name__ == '__main__':
    a = QApplication(sys.argv)
    show_console()
    a.exec_()
