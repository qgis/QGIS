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
from console_sci import ShellScintilla
from console_output import ShellOutputScintilla
from console_editor import EditorTabWidget
from console_help import HelpDialog
from console_settings import optionsDialog
from qgis.core import QgsApplication

import sys
import os

_console = None

def show_console():
  """ called from QGIS to open the console """
  global _console
  if _console is None:
    parent = iface.mainWindow() if iface else None
    _console = PythonConsole( parent )
    _console.show() # force show even if it was restored as hidden

    # set focus to the console so the user can start typing
    # defer the set focus event so it works also whether the console not visible yet
    QTimer.singleShot(0, _console.activate)
  else:
    _console.setVisible(not _console.isVisible())

    # set focus to the console so the user can start typing
    if _console.isVisible():
      _console.activate()

_old_stdout = sys.stdout
_console_output = None

# hook for python console so all output will be redirected
# and then shown in console
def console_displayhook(obj):
    global _console_output
    _console_output = obj

class PythonConsole(QDockWidget):
    def __init__(self, parent=None):
        QDockWidget.__init__(self, parent)
        self.setObjectName("PythonConsole")
        self.setWindowTitle(QCoreApplication.translate("PythonConsole", "Python Console"))
        #self.setAllowedAreas(Qt.BottomDockWidgetArea)

        self.console = PythonConsoleWidget(self)
        self.setWidget( self.console )
        self.setFocusProxy( self.console )

        # try to restore position from stored main window state
        if iface and not iface.mainWindow().restoreDockWidget(self):
            iface.mainWindow().addDockWidget(Qt.BottomDockWidgetArea, self)

    def activate(self):
        self.activateWindow()
        self.raise_()
        QDockWidget.setFocus(self)

    def closeEvent(self, event):
        self.console.shell.writeHistoryFile()
        QWidget.closeEvent(self, event)

class PythonConsoleWidget(QWidget):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.setWindowTitle(QCoreApplication.translate("PythonConsole", "Python Console"))
        
        self.options = optionsDialog(self)
        self.helpDlg = HelpDialog(self)
        
        self.shell = ShellScintilla(self)
        self.setFocusProxy(self.shell)
        self.shellOut = ShellOutputScintilla(self)
        self.tabEditorWidget = EditorTabWidget(self)
        
        ##------------ UI -------------------------------

        self.splitterEditor = QSplitter(self)
        self.splitterEditor.setOrientation(Qt.Horizontal)
        self.splitterEditor.setHandleWidth(6)
        self.splitterEditor.setChildrenCollapsible(True)
        self.splitter = QSplitter(self.splitterEditor)
        self.splitter.setOrientation(Qt.Vertical)
        self.splitter.setHandleWidth(3)
        self.splitter.setChildrenCollapsible(False)
        self.splitter.addWidget(self.shellOut)
        self.splitter.addWidget(self.shell)
        self.splitterEditor.addWidget(self.tabEditorWidget)
        
        # Hide side editor on start up
        self.tabEditorWidget.hide()
        
        # List for tab script
        self.settings = QSettings()
        storedTabScripts = self.settings.value("pythonConsole/tabScripts")
        self.tabListScript = storedTabScripts.toList()
        
        sizes = self.splitter.sizes()
        self.splitter.setSizes(sizes)

        ## Action Show Editor
        showEditor = QCoreApplication.translate("PythonConsole", "Show editor")
        self.showEditorButton = QAction(parent)
        self.showEditorButton.setCheckable(False)
        self.showEditorButton.setEnabled(True)
        self.showEditorButton.setCheckable(True)
        self.showEditorButton.setIcon(QgsApplication.getThemeIcon("console/iconShowEditorConsole.png"))
        self.showEditorButton.setMenuRole(QAction.PreferencesRole)
        self.showEditorButton.setIconVisibleInMenu(True)
        self.showEditorButton.setToolTip(showEditor)
        self.showEditorButton.setText(showEditor)
        ## Action for Clear button
        clearBt = QCoreApplication.translate("PythonConsole", "Clear console")
        self.clearButton = QAction(parent)
        self.clearButton.setCheckable(False)
        self.clearButton.setEnabled(True)
        self.clearButton.setIcon(QgsApplication.getThemeIcon("console/iconClearConsole.png"))
        self.clearButton.setMenuRole(QAction.PreferencesRole)
        self.clearButton.setIconVisibleInMenu(True)
        self.clearButton.setToolTip(clearBt)
        self.clearButton.setText(clearBt)
        ## Action for settings
        optionsBt = QCoreApplication.translate("PythonConsole", "Settings")
        self.optionsButton = QAction(parent)
        self.optionsButton.setCheckable(False)
        self.optionsButton.setEnabled(True)
        self.optionsButton.setIcon(QgsApplication.getThemeIcon("console/iconSettingsConsole.png"))
        self.optionsButton.setMenuRole(QAction.PreferencesRole)
        self.optionsButton.setIconVisibleInMenu(True)
        self.optionsButton.setToolTip(optionsBt)
        self.optionsButton.setText(optionsBt)
        ## Action menu for class
        actionClassBt = QCoreApplication.translate("PythonConsole", "Import Class")
        self.actionClass = QAction(parent)
        self.actionClass.setCheckable(False)
        self.actionClass.setEnabled(True)
        self.actionClass.setIcon(QgsApplication.getThemeIcon("console/iconClassConsole.png"))
        self.actionClass.setMenuRole(QAction.PreferencesRole)
        self.actionClass.setIconVisibleInMenu(True)
        self.actionClass.setToolTip(actionClassBt)
        self.actionClass.setText(actionClassBt)
        ## Action menu Open/Save script
        actionScriptBt = QCoreApplication.translate("PythonConsole", "Manage Script")
        self.actionScript = QAction(parent)
        self.actionScript.setCheckable(False)
        self.actionScript.setEnabled(True)
        self.actionScript.setIcon(QgsApplication.getThemeIcon("console/iconScriptConsole.png"))
        self.actionScript.setMenuRole(QAction.PreferencesRole)
        self.actionScript.setIconVisibleInMenu(True)
        self.actionScript.setToolTip(actionScriptBt)
        self.actionScript.setText(actionScriptBt)
        ## Import Sextante class
        loadSextanteBt = QCoreApplication.translate("PythonConsole", "Import Sextante class")
        self.loadSextanteButton = QAction(parent)
        self.loadSextanteButton.setCheckable(False)
        self.loadSextanteButton.setEnabled(True)
        self.loadSextanteButton.setIcon(QgsApplication.getThemeIcon("console/iconSextanteConsole.png"))
        self.loadSextanteButton.setMenuRole(QAction.PreferencesRole)
        self.loadSextanteButton.setIconVisibleInMenu(True)
        self.loadSextanteButton.setToolTip(loadSextanteBt)
        self.loadSextanteButton.setText(loadSextanteBt)
        ## Import QtCore class
        loadQtCoreBt = QCoreApplication.translate("PythonConsole", "Import PyQt.QtCore class")
        self.loadQtCoreButton = QAction(parent)
        self.loadQtCoreButton.setCheckable(False)
        self.loadQtCoreButton.setEnabled(True)
        self.loadQtCoreButton.setIcon(QgsApplication.getThemeIcon("console/iconQtCoreConsole.png"))
        self.loadQtCoreButton.setMenuRole(QAction.PreferencesRole)
        self.loadQtCoreButton.setIconVisibleInMenu(True)
        self.loadQtCoreButton.setToolTip(loadQtCoreBt)
        self.loadQtCoreButton.setText(loadQtCoreBt)
        ## Import QtGui class
        loadQtGuiBt = QCoreApplication.translate("PythonConsole", "Import PyQt.QtGui class")
        self.loadQtGuiButton = QAction(parent)
        self.loadQtGuiButton.setCheckable(False)
        self.loadQtGuiButton.setEnabled(True)
        self.loadQtGuiButton.setIcon(QgsApplication.getThemeIcon("console/iconQtGuiConsole.png"))
        self.loadQtGuiButton.setMenuRole(QAction.PreferencesRole)
        self.loadQtGuiButton.setIconVisibleInMenu(True)
        self.loadQtGuiButton.setToolTip(loadQtGuiBt)
        self.loadQtGuiButton.setText(loadQtGuiBt)
        ## Action for Open File
        openFileBt = QCoreApplication.translate("PythonConsole", "Open file")
        self.openFileButton = QAction(parent)
        self.openFileButton.setCheckable(False)
        self.openFileButton.setEnabled(False)
        self.openFileButton.setIcon(QgsApplication.getThemeIcon("console/iconOpenConsole.png"))
        self.openFileButton.setMenuRole(QAction.PreferencesRole)
        self.openFileButton.setIconVisibleInMenu(True)
        self.openFileButton.setToolTip(openFileBt)
        self.openFileButton.setText(openFileBt)
        ## Action for Save File
        saveFileBt = QCoreApplication.translate("PythonConsole", "Save")
        self.saveFileButton = QAction(parent)
        self.saveFileButton.setCheckable(False)
        self.saveFileButton.setEnabled(False)
        self.saveFileButton.setIcon(QgsApplication.getThemeIcon("console/iconSaveConsole.png"))
        self.saveFileButton.setMenuRole(QAction.PreferencesRole)
        self.saveFileButton.setIconVisibleInMenu(True)
        self.saveFileButton.setToolTip(saveFileBt)
        self.saveFileButton.setText(saveFileBt)
        ## Action for Save File As
        saveAsFileBt = QCoreApplication.translate("PythonConsole", "Save As..")
        self.saveAsFileButton = QAction(parent)
        self.saveAsFileButton.setCheckable(False)
        self.saveAsFileButton.setEnabled(False)
        #self.saveAsFileButton.setIcon(QgsApplication.getThemeIcon("console/iconSaveConsole.png"))
        self.saveAsFileButton.setMenuRole(QAction.PreferencesRole)
        self.saveAsFileButton.setIconVisibleInMenu(True)
        self.saveAsFileButton.setToolTip(saveAsFileBt)
        self.saveAsFileButton.setText(saveAsFileBt)
        ## Action for Run script
        runBt = QCoreApplication.translate("PythonConsole", "Run command")
        self.runButton = QAction(parent)
        self.runButton.setCheckable(False)
        self.runButton.setEnabled(True)
        self.runButton.setIcon(QgsApplication.getThemeIcon("console/iconRunConsole.png"))
        self.runButton.setMenuRole(QAction.PreferencesRole)
        self.runButton.setIconVisibleInMenu(True)
        self.runButton.setToolTip(runBt)
        self.runButton.setText(runBt)
        ## Help action
        helpBt = QCoreApplication.translate("PythonConsole", "Help")
        self.helpButton = QAction(parent)
        self.helpButton.setCheckable(False)
        self.helpButton.setEnabled(True)
        self.helpButton.setIcon(QgsApplication.getThemeIcon("console/iconHelpConsole.png"))
        self.helpButton.setMenuRole(QAction.PreferencesRole)
        self.helpButton.setIconVisibleInMenu(True)
        self.helpButton.setToolTip(helpBt)
        self.helpButton.setText(helpBt)

        self.toolBar = QToolBar()
        self.toolBar.setEnabled(True)
        self.toolBar.setFocusPolicy(Qt.NoFocus)
        self.toolBar.setContextMenuPolicy(Qt.DefaultContextMenu)
        self.toolBar.setLayoutDirection(Qt.LeftToRight)
        self.toolBar.setIconSize(QSize(24, 24))
        self.toolBar.setOrientation(Qt.Vertical)
        self.toolBar.setMovable(True)
        self.toolBar.setFloatable(True)
        self.toolBar.addAction(self.clearButton)
        self.toolBar.addAction(self.actionClass)
        self.toolBar.addAction(self.runButton)
        self.toolBar.addSeparator()
        self.toolBar.addAction(self.showEditorButton)
        self.toolBar.addAction(self.actionScript)
        self.toolBar.addSeparator()
        self.toolBar.addAction(self.optionsButton)
        self.toolBar.addAction(self.helpButton)
        
        ## Menu Import Class
        self.classMenu = QMenu(self)
        self.classMenu.addAction(self.loadSextanteButton)
        self.classMenu.addAction(self.loadQtCoreButton)
        self.classMenu.addAction(self.loadQtGuiButton)
        cM = self.toolBar.widgetForAction(self.actionClass)
        cM.setMenu(self.classMenu)
        cM.setPopupMode(QToolButton.InstantPopup)
        ## Menu Manage Script
        self.scriptMenu = QMenu(self)
        self.scriptMenu.addAction(self.openFileButton)
        self.scriptMenu.addAction(self.saveFileButton)
        self.scriptMenu.addAction(self.saveAsFileButton)
        sM = self.toolBar.widgetForAction(self.actionScript)
        sM.setMenu(self.scriptMenu)
        sM.setPopupMode(QToolButton.InstantPopup)

        self.widgetButton = QWidget()
        sizePolicy = QSizePolicy(QSizePolicy.Fixed, QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.widgetButton.sizePolicy().hasHeightForWidth())
        self.widgetButton.setSizePolicy(sizePolicy)
        sizePolicy = QSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.shellOut.sizePolicy().hasHeightForWidth())
        self.shellOut.setSizePolicy(sizePolicy)
        self.shellOut.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.shell.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)

        ##------------ Layout -------------------------------
        
        self.mainLayout = QGridLayout(self)
        self.mainLayout.setMargin(0)
        self.mainLayout.setSpacing(0)
        self.mainLayout.addWidget(self.widgetButton, 0, 0, 1, 1)
        self.mainLayout.addWidget(self.splitterEditor, 0, 1, 1, 1)
        self.toolBarLayout = QGridLayout(self.widgetButton)
        self.toolBarLayout.setMargin(0)
        self.toolBarLayout.setSpacing(0)
        self.toolBarLayout.addWidget(self.toolBar)
        
        ##------------ Add first Tab in Editor -------------------------------
        
        #self.tabEditorWidget.newTabEditor(tabName='first', filename=None)
        
        ##------------ Signal -------------------------------

        self.showEditorButton.toggled.connect(self.toggleEditor)
        self.clearButton.triggered.connect(self.shellOut.clearConsole)
        self.optionsButton.triggered.connect(self.openSettings)
        self.loadSextanteButton.triggered.connect(self.sextante)
        self.loadQtCoreButton.triggered.connect(self.qtCore)
        self.loadQtGuiButton.triggered.connect(self.qtGui)
        self.runButton.triggered.connect(self.shell.entered)
        self.openFileButton.triggered.connect(self.openScriptFile)
        self.saveFileButton.triggered.connect(self.saveScriptFile)
        self.saveAsFileButton.triggered.connect(self.saveAsScriptFile)
        self.helpButton.triggered.connect(self.openHelp)
        QObject.connect(self.options.buttonBox, SIGNAL("accepted()"),
                        self.prefChanged)

    def sextante(self):
       self.shell.commandConsole('sextante')

    def qtCore(self):
       self.shell.commandConsole('qtCore')

    def qtGui(self):
       self.shell.commandConsole('qtGui')
       
    def toggleEditor(self, checked):
        self.tabEditorWidget.show() if checked else self.tabEditorWidget.hide()
        self.openFileButton.setEnabled(checked)
        self.saveFileButton.setEnabled(checked)
        self.saveAsFileButton.setEnabled(checked)
            
#    def openScriptFile(self):
#       settings = QSettings()
#       lastDirPath = settings.value("pythonConsole/lastDirPath").toString()
#       scriptFile = QFileDialog.getOpenFileName(
#                       self, "Open File", lastDirPath, "Script file (*.py)")
#       if scriptFile.isEmpty() == False:
#           oF = open(scriptFile, 'r')
#           listScriptFile = []
#           for line in oF:
#               if line != "\n":
#                   listScriptFile.append(line)
#           self.shell.insertTextFromFile(listScriptFile)
#    
#           lastDirPath = QFileInfo(scriptFile).path()
#           settings.setValue("pythonConsole/lastDirPath", QVariant(scriptFile))
#
#
#    def saveScriptFile(self):
#        scriptFile = QFileDialog()
#        scriptFile.setDefaultSuffix(".py")
#        fName = scriptFile.getSaveFileName(
#                        self, "Save file", QString(), "Script file (*.py)")
#    
#        if fName.isEmpty() == False:
#            filename = str(fName)
#            if not filename.endswith(".py"):
#                fName += ".py"
#            sF = open(fName,'w')
#            listText = self.shellOut.getTextFromEditor()
#            is_first_line = True
#            for s in listText:
#                if s[0:3] in (">>>", "..."):
#                    s.replace(">>> ", "").replace("... ", "")
#                    if is_first_line:
#                        is_first_line = False
#                    else:
#                        sF.write('\n')
#                    sF.write(s)
#            sF.close()
#            self.callWidgetMessageBar('Script was correctly saved.')
        
    def openScriptFile(self):
        settings = QSettings()
        lastDirPath = settings.value("pythonConsole/lastDirPath").toString()
        filename = QFileDialog.getOpenFileName(
                        self, "Open File", lastDirPath, "Script file (*.py)")
        if not filename.isEmpty():
            for i in range(self.tabEditorWidget.count()):
                tabWidget = self.tabEditorWidget.widget(i)
                if tabWidget.path == filename:
                    self.tabEditorWidget.setCurrentWidget(tabWidget)
                    break
            else:
                tabName = filename.split('/')[-1]
                self.tabEditorWidget.newTabEditor(tabName, filename)
        
        lastDirPath = QFileInfo(filename).path()
        settings.setValue("pythonConsole/lastDirPath", QVariant(filename))
        self.tabListScript.append(filename)
        self.updateTabListScript(script=None)
                
    def saveScriptFile(self):
        tabWidget = self.tabEditorWidget.currentWidget()
        try:
            tabWidget.save()
        except (IOError, OSError), e:
            QMessageBox.warning(self, "Save Error",
                    "Failed to save %s: %s" % (tabWidget.path, e))
        
    def saveAsScriptFile(self):
        tabWidget = self.tabEditorWidget.currentWidget()
        if tabWidget is None:
            return
        filename = QFileDialog.getSaveFileName(self,
                        "Save File As",
                        tabWidget.path, "Script file (*.py)")
        if not filename.isEmpty():
            self.tabListScript.remove(tabWidget.path)
            tabWidget.path = filename
            self.saveScriptFile()

    def openHelp(self):
        self.helpDlg.show()
        self.helpDlg.activateWindow()

    def openSettings(self):
        self.options.exec_()

    def prefChanged(self):
        self.shell.refreshLexerProperties()
        self.shellOut.refreshLexerProperties()
        self.tabEditorWidget.currentWidget().changeFont()

    def callWidgetMessageBar(self, text):
        self.shellOut.widgetMessageBar(iface, text)
        
    def callWidgetMessageBarEditor(self, text):
        self.tabEditorWidget.widgetMessageBar(iface, text)
        
    def updateTabListScript(self, script, action=None): 
        if script == 'empty':
            self.tabListScript = []
        if script is not None and not action and script != 'empty':
            self.tabListScript.remove(script)
        if action:
            self.tabListScript.append(script)
        self.settings.setValue("pythonConsole/tabScripts",
                               QVariant(self.tabListScript))
            
   
if __name__ == '__main__':
    a = QApplication(sys.argv)
    console = PythonConsoleWidget()
    console.show()
    a.exec_()
