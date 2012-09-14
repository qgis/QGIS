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

class PythonConsole(QDockWidget):
    def __init__(self, parent=None):
        QDockWidget.__init__(self, parent)
        self.setObjectName("PythonConsole")
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
        self.e.setSpacing(0)
        self.b.setMargin(0)

        ## Action for Clear button
        self.clearButton = QAction(parent)
        self.clearButton.setCheckable(False)
        self.clearButton.setEnabled(True)
        self.clearButton.setIcon(QIcon(os.path.dirname(__file__) + "/iconConsole/iconClearConsole.png"))
        self.clearButton.setMenuRole(QAction.PreferencesRole)
        self.clearButton.setIconVisibleInMenu(True)
        self.clearButton.setToolTip('Clear console')
        self.clearButton.setText('Clear console')
        ## Action for paste snippets code
#        self.currentLayerButton = QAction(parent)
#        self.currentLayerButton.setCheckable(False)
#        self.currentLayerButton.setEnabled(True)
#        self.currentLayerButton.setIcon(QIcon("icon/iconTempConsole.png"))
#        self.currentLayerButton.setMenuRole(QAction.PreferencesRole)
#        self.currentLayerButton.setIconVisibleInMenu(True)
        ## Import Sextante class
        self.loadSextanteButton = QAction(parent)
        self.loadSextanteButton.setCheckable(False)
        self.loadSextanteButton.setEnabled(True)
        self.loadSextanteButton.setIcon(QIcon(os.path.dirname(__file__) + "/iconConsole/iconSextanteConsole.png"))
        self.loadSextanteButton.setMenuRole(QAction.PreferencesRole)
        self.loadSextanteButton.setIconVisibleInMenu(True)
        self.loadSextanteButton.setToolTip('Import sextante class')
        self.loadSextanteButton.setText('Import sextante class')
        ## Import QgisInterface class
        self.loadIfaceButton = QAction(parent)
        self.loadIfaceButton.setCheckable(False)
        self.loadIfaceButton.setEnabled(True)
        self.loadIfaceButton.setIcon(QIcon(os.path.dirname(__file__) + "/iconConsole/iconTempConsole.png"))
        self.loadIfaceButton.setMenuRole(QAction.PreferencesRole)
        self.loadIfaceButton.setIconVisibleInMenu(True)
        self.loadIfaceButton.setToolTip('Import iface class')
        self.loadIfaceButton.setText('Import iface class')
        ## Action for Open File
        self.openFileButton = QAction(parent)
        self.openFileButton.setCheckable(False)
        self.openFileButton.setEnabled(True)
        self.openFileButton.setIcon(QIcon(os.path.dirname(__file__) + "/iconConsole/iconOpenConsole.png"))
        self.openFileButton.setMenuRole(QAction.PreferencesRole)
        self.openFileButton.setIconVisibleInMenu(True)
        self.openFileButton.setToolTip('Open script file')
        self.openFileButton.setText('Open script file')
        ## Action for Save File
        self.saveFileButton = QAction(parent)
        self.saveFileButton.setCheckable(False)
        self.saveFileButton.setEnabled(True)
        self.saveFileButton.setIcon(QIcon(os.path.dirname(__file__) + "/iconConsole/iconSaveConsole.png"))
        self.saveFileButton.setMenuRole(QAction.PreferencesRole)
        self.saveFileButton.setIconVisibleInMenu(True)
        self.saveFileButton.setToolTip('Save to script file')
        self.saveFileButton.setText('Save to script file')
        ## Action for Run script
        self.runButton = QAction(parent)
        self.runButton.setCheckable(False)
        self.runButton.setEnabled(True)
        self.runButton.setIcon(QIcon(os.path.dirname(__file__) + "/iconConsole/iconRunConsole.png"))
        self.runButton.setMenuRole(QAction.PreferencesRole)
        self.runButton.setIconVisibleInMenu(True)
        self.runButton.setToolTip('Run command')
        self.runButton.setText('Run')
        ## Help action
        self.helpButton = QAction(parent)
        self.helpButton.setCheckable(False)
        self.helpButton.setEnabled(True)
        self.helpButton.setIcon(QIcon(os.path.dirname(__file__) + "/iconConsole/iconHelpConsole.png"))
        self.helpButton.setMenuRole(QAction.PreferencesRole)
        self.helpButton.setIconVisibleInMenu(True)
        self.helpButton.setToolTip('Help')
        self.helpButton.setText('Help')

        self.toolBar.addAction(self.clearButton)
        #self.toolBar.addAction(self.currentLayerButton)
        self.toolBar.addAction(self.loadIfaceButton)
        self.toolBar.addAction(self.loadSextanteButton)
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
        self.clearButton.triggered.connect(self.edit.clearConsole)
        #self.currentLayerButton.triggered.connect(self.cLayer)
        self.loadIfaceButton.triggered.connect(self.iface)
        self.loadSextanteButton.triggered.connect(self.sextante)
        self.runButton.triggered.connect(self.edit.entered)
        self.openFileButton.triggered.connect(self.openScriptFile)
        self.saveFileButton.triggered.connect(self.saveScriptFile)
        self.helpButton.triggered.connect(self.openHelp)
        # try to restore position from stored main window state
        if not iface.mainWindow().restoreDockWidget(self):
            iface.mainWindow().addDockWidget(Qt.BottomDockWidgetArea, self)

    def cLayer(self):
        self.edit.commandConsole('cLayer')

    def sextante(self):
       self.edit.commandConsole('sextante')

    def iface(self):
       self.edit.commandConsole('iface')

    def openScriptFile(self):
        settings = QSettings()
        lastDirPath = settings.value("/pythonConsole/lastDirPath").toString()
        scriptFile = QFileDialog.getOpenFileName(
                        self, "Open File", lastDirPath, "Script file (*.py)")
        if scriptFile.isEmpty() == False:
            oF = open(scriptFile, 'r')
            listScriptFile = []
            for line in oF:
                if line != "\n":
                    listScriptFile.append(line)
            self.edit.insertTextFromFile(listScriptFile)

            lastDirPath = QFileInfo(scriptFile).path()
            settings.setValue("/pythonConsole/lastDirPath", QVariant(scriptFile))


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
                    s.replace(">>> ", "").replace("... ", "")
                    if is_first_line:
                        is_first_line = False
                    else:
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
