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
import os

from qgis.PyQt.QtCore import Qt, QTimer, QSettings, QCoreApplication, QSize, QByteArray, QFileInfo, QUrl, QDir
from qgis.PyQt.QtWidgets import QDockWidget, QToolBar, QToolButton, QWidget, QSplitter, QTreeWidget, QAction, QFileDialog, QCheckBox, QSizePolicy, QMenu, QGridLayout, QApplication, QShortcut
from qgis.PyQt.QtGui import QDesktopServices, QKeySequence
from qgis.PyQt.QtWidgets import QVBoxLayout
from qgis.utils import iface
from .console_sci import ShellScintilla
from .console_output import ShellOutputScintilla
from .console_editor import EditorTabWidget
from .console_settings import optionsDialog
from qgis.core import QgsApplication, QgsContextHelp
from qgis.gui import QgsFilterLineEdit
from functools import partial

import sys

_console = None


def show_console():
    """ called from QGIS to open the console """
    global _console
    if _console is None:
        parent = iface.mainWindow() if iface else None
        _console = PythonConsole(parent)
        _console.show()  # force show even if it was restored as hidden
        # set focus to the console so the user can start typing
        # defer the set focus event so it works also whether the console not visible yet
        QTimer.singleShot(0, _console.activate)
    else:
        _console.setVisible(not _console.isVisible())
        # set focus to the console so the user can start typing
        if _console.isVisible():
            _console.activate()
    ## Shows help on first launch of the console
    settings = QSettings()
    if settings.value('pythonConsole/contextHelpOnFirstLaunch', True, type=bool):
        QgsContextHelp.run("PythonConsole")
        settings.setValue('pythonConsole/contextHelpOnFirstLaunch', False)

    return _console

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
        self.setWidget(self.console)
        self.setFocusProxy(self.console)

        # try to restore position from stored main window state
        if iface and not iface.mainWindow().restoreDockWidget(self):
            iface.mainWindow().addDockWidget(Qt.BottomDockWidgetArea, self)

    def activate(self):
        self.activateWindow()
        self.raise_()
        QDockWidget.setFocus(self)

    def closeEvent(self, event):
        self.console.saveSettingsConsole()
        QWidget.closeEvent(self, event)


class PythonConsoleWidget(QWidget):

    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.setWindowTitle(QCoreApplication.translate("PythonConsole", "Python Console"))

        self.settings = QSettings()

        self.shell = ShellScintilla(self)
        self.setFocusProxy(self.shell)
        self.shellOut = ShellOutputScintilla(self)
        self.tabEditorWidget = EditorTabWidget(self)

        ##------------ UI -------------------------------

        self.splitterEditor = QSplitter(self)
        self.splitterEditor.setOrientation(Qt.Horizontal)
        self.splitterEditor.setHandleWidth(6)
        self.splitterEditor.setChildrenCollapsible(True)

        self.shellOutWidget = QWidget(self)
        self.shellOutWidget.setLayout(QVBoxLayout())
        self.shellOutWidget.layout().setContentsMargins(0, 0, 0, 0)
        self.shellOutWidget.layout().addWidget(self.shellOut)

        self.splitter = QSplitter(self.splitterEditor)
        self.splitter.setOrientation(Qt.Vertical)
        self.splitter.setHandleWidth(3)
        self.splitter.setChildrenCollapsible(False)
        self.splitter.addWidget(self.shellOutWidget)
        self.splitter.addWidget(self.shell)

        #self.splitterEditor.addWidget(self.tabEditorWidget)

        self.splitterObj = QSplitter(self.splitterEditor)
        self.splitterObj.setHandleWidth(3)
        self.splitterObj.setOrientation(Qt.Horizontal)
        #self.splitterObj.setSizes([0, 0])
        #self.splitterObj.setStretchFactor(0, 1)

        self.widgetEditor = QWidget(self.splitterObj)
        self.widgetFind = QWidget(self)

        self.listClassMethod = QTreeWidget(self.splitterObj)
        self.listClassMethod.setColumnCount(2)
        objInspLabel = QCoreApplication.translate("PythonConsole", "Object Inspector")
        self.listClassMethod.setHeaderLabels([objInspLabel, ''])
        self.listClassMethod.setColumnHidden(1, True)
        self.listClassMethod.setAlternatingRowColors(True)

        #self.splitterEditor.addWidget(self.widgetEditor)
        #self.splitterObj.addWidget(self.listClassMethod)
        #self.splitterObj.addWidget(self.widgetEditor)

        # Hide side editor on start up
        self.splitterObj.hide()
        self.listClassMethod.hide()
        # Hide search widget on start up
        self.widgetFind.hide()

        sizes = self.splitter.sizes()
        self.splitter.setSizes(sizes)

        ##----------------Restore Settings------------------------------------

        self.restoreSettingsConsole()

        ##------------------Toolbar Editor-------------------------------------

        ## Action for Open File
        openFileBt = QCoreApplication.translate("PythonConsole", "Open Script...")
        self.openFileButton = QAction(self)
        self.openFileButton.setCheckable(False)
        self.openFileButton.setEnabled(True)
        self.openFileButton.setIcon(QgsApplication.getThemeIcon("console/iconOpenConsole.png"))
        self.openFileButton.setMenuRole(QAction.PreferencesRole)
        self.openFileButton.setIconVisibleInMenu(True)
        self.openFileButton.setToolTip(openFileBt)
        self.openFileButton.setText(openFileBt)

        openExtEditorBt = QCoreApplication.translate("PythonConsole", "Open in External Editor")
        self.openInEditorButton = QAction(self)
        self.openInEditorButton.setCheckable(False)
        self.openInEditorButton.setEnabled(True)
        self.openInEditorButton.setIcon(QgsApplication.getThemeIcon("console/iconShowEditorConsole.png"))
        self.openInEditorButton.setMenuRole(QAction.PreferencesRole)
        self.openInEditorButton.setIconVisibleInMenu(True)
        self.openInEditorButton.setToolTip(openExtEditorBt)
        self.openInEditorButton.setText(openExtEditorBt)
        ## Action for Save File
        saveFileBt = QCoreApplication.translate("PythonConsole", "Save")
        self.saveFileButton = QAction(self)
        self.saveFileButton.setCheckable(False)
        self.saveFileButton.setEnabled(False)
        self.saveFileButton.setIcon(QgsApplication.getThemeIcon("console/iconSaveConsole.png"))
        self.saveFileButton.setMenuRole(QAction.PreferencesRole)
        self.saveFileButton.setIconVisibleInMenu(True)
        self.saveFileButton.setToolTip(saveFileBt)
        self.saveFileButton.setText(saveFileBt)
        ## Action for Save File As
        saveAsFileBt = QCoreApplication.translate("PythonConsole", "Save As...")
        self.saveAsFileButton = QAction(self)
        self.saveAsFileButton.setCheckable(False)
        self.saveAsFileButton.setEnabled(True)
        self.saveAsFileButton.setIcon(QgsApplication.getThemeIcon("console/iconSaveAsConsole.png"))
        self.saveAsFileButton.setMenuRole(QAction.PreferencesRole)
        self.saveAsFileButton.setIconVisibleInMenu(True)
        self.saveAsFileButton.setToolTip(saveAsFileBt)
        self.saveAsFileButton.setText(saveAsFileBt)
        ## Action Cut
        cutEditorBt = QCoreApplication.translate("PythonConsole", "Cut")
        self.cutEditorButton = QAction(self)
        self.cutEditorButton.setCheckable(False)
        self.cutEditorButton.setEnabled(True)
        self.cutEditorButton.setIcon(QgsApplication.getThemeIcon("mActionEditCut.png"))
        self.cutEditorButton.setMenuRole(QAction.PreferencesRole)
        self.cutEditorButton.setIconVisibleInMenu(True)
        self.cutEditorButton.setToolTip(cutEditorBt)
        self.cutEditorButton.setText(cutEditorBt)
        ## Action Copy
        copyEditorBt = QCoreApplication.translate("PythonConsole", "Copy")
        self.copyEditorButton = QAction(self)
        self.copyEditorButton.setCheckable(False)
        self.copyEditorButton.setEnabled(True)
        self.copyEditorButton.setIcon(QgsApplication.getThemeIcon("mActionEditCopy.png"))
        self.copyEditorButton.setMenuRole(QAction.PreferencesRole)
        self.copyEditorButton.setIconVisibleInMenu(True)
        self.copyEditorButton.setToolTip(copyEditorBt)
        self.copyEditorButton.setText(copyEditorBt)
        ## Action Paste
        pasteEditorBt = QCoreApplication.translate("PythonConsole", "Paste")
        self.pasteEditorButton = QAction(self)
        self.pasteEditorButton.setCheckable(False)
        self.pasteEditorButton.setEnabled(True)
        self.pasteEditorButton.setIcon(QgsApplication.getThemeIcon("mActionEditPaste.png"))
        self.pasteEditorButton.setMenuRole(QAction.PreferencesRole)
        self.pasteEditorButton.setIconVisibleInMenu(True)
        self.pasteEditorButton.setToolTip(pasteEditorBt)
        self.pasteEditorButton.setText(pasteEditorBt)
        ## Action Run Script (subprocess)
        runScriptEditorBt = QCoreApplication.translate("PythonConsole", "Run script")
        self.runScriptEditorButton = QAction(self)
        self.runScriptEditorButton.setCheckable(False)
        self.runScriptEditorButton.setEnabled(True)
        self.runScriptEditorButton.setIcon(QgsApplication.getThemeIcon("console/iconRunScriptConsole.png"))
        self.runScriptEditorButton.setMenuRole(QAction.PreferencesRole)
        self.runScriptEditorButton.setIconVisibleInMenu(True)
        self.runScriptEditorButton.setToolTip(runScriptEditorBt)
        self.runScriptEditorButton.setText(runScriptEditorBt)
        ## Action Run Script (subprocess)
        commentEditorBt = QCoreApplication.translate("PythonConsole", "Comment")
        self.commentEditorButton = QAction(self)
        self.commentEditorButton.setCheckable(False)
        self.commentEditorButton.setEnabled(True)
        self.commentEditorButton.setIcon(QgsApplication.getThemeIcon("console/iconCommentEditorConsole.png"))
        self.commentEditorButton.setMenuRole(QAction.PreferencesRole)
        self.commentEditorButton.setIconVisibleInMenu(True)
        self.commentEditorButton.setToolTip(commentEditorBt)
        self.commentEditorButton.setText(commentEditorBt)
        ## Action Run Script (subprocess)
        uncommentEditorBt = QCoreApplication.translate("PythonConsole", "Uncomment")
        self.uncommentEditorButton = QAction(self)
        self.uncommentEditorButton.setCheckable(False)
        self.uncommentEditorButton.setEnabled(True)
        self.uncommentEditorButton.setIcon(QgsApplication.getThemeIcon("console/iconUncommentEditorConsole.png"))
        self.uncommentEditorButton.setMenuRole(QAction.PreferencesRole)
        self.uncommentEditorButton.setIconVisibleInMenu(True)
        self.uncommentEditorButton.setToolTip(uncommentEditorBt)
        self.uncommentEditorButton.setText(uncommentEditorBt)
        ## Action for Object browser
        objList = QCoreApplication.translate("PythonConsole", "Object Inspector...")
        self.objectListButton = QAction(self)
        self.objectListButton.setCheckable(True)
        self.objectListButton.setEnabled(self.settings.value("pythonConsole/enableObjectInsp",
                                                             False, type=bool))
        self.objectListButton.setIcon(QgsApplication.getThemeIcon("console/iconClassBrowserConsole.png"))
        self.objectListButton.setMenuRole(QAction.PreferencesRole)
        self.objectListButton.setIconVisibleInMenu(True)
        self.objectListButton.setToolTip(objList)
        self.objectListButton.setText(objList)
        ## Action for Find text
        findText = QCoreApplication.translate("PythonConsole", "Find Text")
        self.findTextButton = QAction(self)
        self.findTextButton.setCheckable(True)
        self.findTextButton.setEnabled(True)
        self.findTextButton.setIcon(QgsApplication.getThemeIcon("console/iconSearchEditorConsole.png"))
        self.findTextButton.setMenuRole(QAction.PreferencesRole)
        self.findTextButton.setIconVisibleInMenu(True)
        self.findTextButton.setToolTip(findText)
        self.findTextButton.setText(findText)

        ##----------------Toolbar Console-------------------------------------

        ## Action Show Editor
        showEditor = QCoreApplication.translate("PythonConsole", "Show Editor")
        self.showEditorButton = QAction(self)
        self.showEditorButton.setEnabled(True)
        self.showEditorButton.setCheckable(True)
        self.showEditorButton.setIcon(QgsApplication.getThemeIcon("console/iconShowEditorConsole.png"))
        self.showEditorButton.setMenuRole(QAction.PreferencesRole)
        self.showEditorButton.setIconVisibleInMenu(True)
        self.showEditorButton.setToolTip(showEditor)
        self.showEditorButton.setText(showEditor)
        ## Action for Clear button
        clearBt = QCoreApplication.translate("PythonConsole", "Clear Console")
        self.clearButton = QAction(self)
        self.clearButton.setCheckable(False)
        self.clearButton.setEnabled(True)
        self.clearButton.setIcon(QgsApplication.getThemeIcon("console/iconClearConsole.png"))
        self.clearButton.setMenuRole(QAction.PreferencesRole)
        self.clearButton.setIconVisibleInMenu(True)
        self.clearButton.setToolTip(clearBt)
        self.clearButton.setText(clearBt)
        ## Action for settings
        optionsBt = QCoreApplication.translate("PythonConsole", "Options...")
        self.optionsButton = QAction(self)
        self.optionsButton.setCheckable(False)
        self.optionsButton.setEnabled(True)
        self.optionsButton.setIcon(QgsApplication.getThemeIcon("console/iconSettingsConsole.png"))
        self.optionsButton.setMenuRole(QAction.PreferencesRole)
        self.optionsButton.setIconVisibleInMenu(True)
        self.optionsButton.setToolTip(optionsBt)
        self.optionsButton.setText(optionsBt)
        ## Action menu for class
        actionClassBt = QCoreApplication.translate("PythonConsole", "Import Class")
        self.actionClass = QAction(self)
        self.actionClass.setCheckable(False)
        self.actionClass.setEnabled(True)
        self.actionClass.setIcon(QgsApplication.getThemeIcon("console/iconClassConsole.png"))
        self.actionClass.setMenuRole(QAction.PreferencesRole)
        self.actionClass.setIconVisibleInMenu(True)
        self.actionClass.setToolTip(actionClassBt)
        self.actionClass.setText(actionClassBt)
        ## Action for Run script
        runBt = QCoreApplication.translate("PythonConsole", "Run Command")
        self.runButton = QAction(self)
        self.runButton.setCheckable(False)
        self.runButton.setEnabled(True)
        self.runButton.setIcon(QgsApplication.getThemeIcon("console/iconRunConsole.png"))
        self.runButton.setMenuRole(QAction.PreferencesRole)
        self.runButton.setIconVisibleInMenu(True)
        self.runButton.setToolTip(runBt)
        self.runButton.setText(runBt)
        ## Help action
        helpBt = QCoreApplication.translate("PythonConsole", "Help...")
        self.helpButton = QAction(self)
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
        self.toolBar.setIconSize(QSize(16, 16))
        self.toolBar.setMovable(False)
        self.toolBar.setFloatable(False)
        self.toolBar.addAction(self.clearButton)
        self.toolBar.addAction(self.actionClass)
        self.toolBar.addAction(self.runButton)
        self.toolBar.addSeparator()
        self.toolBar.addAction(self.showEditorButton)
        self.toolBar.addSeparator()
        self.toolBar.addAction(self.optionsButton)
        self.toolBar.addAction(self.helpButton)

        self.toolBarEditor = QToolBar()
        self.toolBarEditor.setEnabled(False)
        self.toolBarEditor.setFocusPolicy(Qt.NoFocus)
        self.toolBarEditor.setContextMenuPolicy(Qt.DefaultContextMenu)
        self.toolBarEditor.setLayoutDirection(Qt.LeftToRight)
        self.toolBarEditor.setIconSize(QSize(16, 16))
        self.toolBarEditor.setMovable(False)
        self.toolBarEditor.setFloatable(False)
        self.toolBarEditor.addAction(self.openFileButton)
        self.toolBarEditor.addAction(self.openInEditorButton)
        self.toolBarEditor.addSeparator()
        self.toolBarEditor.addAction(self.saveFileButton)
        self.toolBarEditor.addAction(self.saveAsFileButton)
        self.toolBarEditor.addSeparator()
        self.toolBarEditor.addAction(self.runScriptEditorButton)
        self.toolBarEditor.addSeparator()
        self.toolBarEditor.addAction(self.findTextButton)
        self.toolBarEditor.addSeparator()
        self.toolBarEditor.addAction(self.cutEditorButton)
        self.toolBarEditor.addAction(self.copyEditorButton)
        self.toolBarEditor.addAction(self.pasteEditorButton)
        self.toolBarEditor.addSeparator()
        self.toolBarEditor.addAction(self.commentEditorButton)
        self.toolBarEditor.addAction(self.uncommentEditorButton)
        self.toolBarEditor.addSeparator()
        self.toolBarEditor.addAction(self.objectListButton)

        ## Menu Import Class
        default_command = {
            (QCoreApplication.translate("PythonConsole", "Import Processing Class"),
             QgsApplication.getThemeIcon("console/iconProcessingConsole.png")):
            ["import processing"],
            (QCoreApplication.translate("PythonConsole", "Import PyQt.QtCore Class"),
             QgsApplication.getThemeIcon("console/iconQtCoreConsole.png")):
            ["from qgis.PyQt.QtCore import *"],
            (QCoreApplication.translate("PythonConsole", "Import PyQt.QtGui Class"),
             QgsApplication.getThemeIcon("console/iconQtGuiConsole.png")):
            ["from qgis.PyQt.QtGui import *", "from qgis.PyQt.QtWidgets import *"]
        }

        self.classMenu = QMenu()
        for (title, icon), commands in default_command.items():
            action = self.classMenu.addAction(icon, title)
            action.triggered.connect(partial(self.shell.commandConsole, commands))

        cM = self.toolBar.widgetForAction(self.actionClass)
        cM.setMenu(self.classMenu)
        cM.setPopupMode(QToolButton.InstantPopup)

        self.widgetButton = QWidget()
        sizePolicy = QSizePolicy(QSizePolicy.Fixed, QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.widgetButton.sizePolicy().hasHeightForWidth())
        self.widgetButton.setSizePolicy(sizePolicy)

        self.widgetButtonEditor = QWidget(self.widgetEditor)
        sizePolicy = QSizePolicy(QSizePolicy.Fixed, QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.widgetButtonEditor.sizePolicy().hasHeightForWidth())
        self.widgetButtonEditor.setSizePolicy(sizePolicy)

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

        self.shellOutWidget.layout().insertWidget(0, self.toolBar)

        self.layoutEditor = QGridLayout(self.widgetEditor)
        self.layoutEditor.setMargin(0)
        self.layoutEditor.setSpacing(0)
        self.layoutEditor.addWidget(self.toolBarEditor, 0, 1, 1, 1)
        self.layoutEditor.addWidget(self.widgetButtonEditor, 1, 0, 2, 1)
        self.layoutEditor.addWidget(self.tabEditorWidget, 1, 1, 1, 1)
        self.layoutEditor.addWidget(self.widgetFind, 2, 1, 1, 1)

        ## Layout for the find widget
        self.layoutFind = QGridLayout(self.widgetFind)
        self.layoutFind.setContentsMargins(0, 0, 0, 0)
        self.lineEditFind = QgsFilterLineEdit()
        placeHolderTxt = QCoreApplication.translate("PythonConsole", "Enter text to find...")

        self.lineEditFind.setPlaceholderText(placeHolderTxt)
        self.findNextButton = QToolButton()
        self.findNextButton.setEnabled(False)
        toolTipfindNext = QCoreApplication.translate("PythonConsole", "Find Next")
        self.findNextButton.setToolTip(toolTipfindNext)
        self.findNextButton.setIcon(QgsApplication.getThemeIcon("console/iconSearchNextEditorConsole.png"))
        self.findNextButton.setIconSize(QSize(24, 24))
        self.findNextButton.setAutoRaise(True)
        self.findPrevButton = QToolButton()
        self.findPrevButton.setEnabled(False)
        toolTipfindPrev = QCoreApplication.translate("PythonConsole", "Find Previous")
        self.findPrevButton.setToolTip(toolTipfindPrev)
        self.findPrevButton.setIcon(QgsApplication.getThemeIcon("console/iconSearchPrevEditorConsole.png"))
        self.findPrevButton.setIconSize(QSize(24, 24))
        self.findPrevButton.setAutoRaise(True)
        self.caseSensitive = QCheckBox()
        caseSensTr = QCoreApplication.translate("PythonConsole", "Case Sensitive")
        self.caseSensitive.setText(caseSensTr)
        self.wholeWord = QCheckBox()
        wholeWordTr = QCoreApplication.translate("PythonConsole", "Whole Word")
        self.wholeWord.setText(wholeWordTr)
        self.wrapAround = QCheckBox()
        self.wrapAround.setChecked(True)
        wrapAroundTr = QCoreApplication.translate("PythonConsole", "Wrap Around")
        self.wrapAround.setText(wrapAroundTr)
        self.layoutFind.addWidget(self.lineEditFind, 0, 1, 1, 1)
        self.layoutFind.addWidget(self.findPrevButton, 0, 2, 1, 1)
        self.layoutFind.addWidget(self.findNextButton, 0, 3, 1, 1)
        self.layoutFind.addWidget(self.caseSensitive, 0, 4, 1, 1)
        self.layoutFind.addWidget(self.wholeWord, 0, 5, 1, 1)
        self.layoutFind.addWidget(self.wrapAround, 0, 6, 1, 1)

        ##------------ Add first Tab in Editor -------------------------------

        #self.tabEditorWidget.newTabEditor(tabName='first', filename=None)

        ##------------ Signal -------------------------------

        self.findTextButton.triggered.connect(self._toggleFind)
        self.objectListButton.toggled.connect(self.toggleObjectListWidget)
        self.commentEditorButton.triggered.connect(self.commentCode)
        self.uncommentEditorButton.triggered.connect(self.uncommentCode)
        self.runScriptEditorButton.triggered.connect(self.runScriptEditor)
        self.cutEditorButton.triggered.connect(self.cutEditor)
        self.copyEditorButton.triggered.connect(self.copyEditor)
        self.pasteEditorButton.triggered.connect(self.pasteEditor)
        self.showEditorButton.toggled.connect(self.toggleEditor)
        self.clearButton.triggered.connect(self.shellOut.clearConsole)
        self.optionsButton.triggered.connect(self.openSettings)
        self.runButton.triggered.connect(self.shell.entered)
        self.openFileButton.triggered.connect(self.openScriptFile)
        self.openInEditorButton.triggered.connect(self.openScriptFileExtEditor)
        self.saveFileButton.triggered.connect(self.saveScriptFile)
        self.saveAsFileButton.triggered.connect(self.saveAsScriptFile)
        self.helpButton.triggered.connect(self.openHelp)
        self.listClassMethod.itemClicked.connect(self.onClickGoToLine)
        self.lineEditFind.returnPressed.connect(self._findNext)
        self.findNextButton.clicked.connect(self._findNext)
        self.findPrevButton.clicked.connect(self._findPrev)
        self.lineEditFind.textChanged.connect(self._textFindChanged)

        self.findScut = QShortcut(QKeySequence.Find, self.widgetEditor)
        self.findScut.setContext(Qt.WidgetWithChildrenShortcut)
        self.findScut.activated.connect(self._openFind)

        self.findNextScut = QShortcut(QKeySequence.FindNext, self.widgetEditor)
        self.findNextScut.setContext(Qt.WidgetWithChildrenShortcut)
        self.findNextScut.activated.connect(self._findNext)

        self.findPreviousScut = QShortcut(QKeySequence.FindPrevious, self.widgetEditor)
        self.findPreviousScut.setContext(Qt.WidgetWithChildrenShortcut)
        self.findPreviousScut.activated.connect(self._findPrev)

        # Escape on editor hides the find bar
        self.findScut = QShortcut(Qt.Key_Escape, self.widgetEditor)
        self.findScut.setContext(Qt.WidgetWithChildrenShortcut)
        self.findScut.activated.connect(self._closeFind)

    def _toggleFind(self):
        self.tabEditorWidget.currentWidget().newEditor.toggleFindWidget()

    def _openFind(self):
        self.tabEditorWidget.currentWidget().newEditor.openFindWidget()

    def _closeFind(self):
        self.tabEditorWidget.currentWidget().newEditor.closeFindWidget()

    def _findNext(self):
        self.tabEditorWidget.currentWidget().newEditor.findText(True)

    def _findPrev(self):
        self.tabEditorWidget.currentWidget().newEditor.findText(False)

    def _textFindChanged(self):
        if self.lineEditFind.text():
            self.findNextButton.setEnabled(True)
            self.findPrevButton.setEnabled(True)
            self.tabEditorWidget.currentWidget().newEditor.findText(True, showMessage=False, findFirst=True)
        else:
            self.lineEditFind.setStyleSheet('')
            self.findNextButton.setEnabled(False)
            self.findPrevButton.setEnabled(False)

    def onClickGoToLine(self, item, column):
        tabEditor = self.tabEditorWidget.currentWidget().newEditor
        if item.text(1) == 'syntaxError':
            check = tabEditor.syntaxCheck(fromContextMenu=False)
            if check and not tabEditor.isReadOnly():
                self.tabEditorWidget.currentWidget().save()
            return
        linenr = int(item.text(1))
        itemName = str(item.text(0))
        charPos = itemName.find(' ')
        if charPos != -1:
            objName = itemName[0:charPos]
        else:
            objName = itemName
        tabEditor.goToLine(objName, linenr)

    def toggleEditor(self, checked):
        self.splitterObj.show() if checked else self.splitterObj.hide()
        if not self.tabEditorWidget:
            self.tabEditorWidget.enableToolBarEditor(checked)
            self.tabEditorWidget.restoreTabsOrAddNew()

    def toggleObjectListWidget(self, checked):
        self.listClassMethod.show() if checked else self.listClassMethod.hide()

    def pasteEditor(self):
        self.tabEditorWidget.currentWidget().newEditor.paste()

    def cutEditor(self):
        self.tabEditorWidget.currentWidget().newEditor.cut()

    def copyEditor(self):
        self.tabEditorWidget.currentWidget().newEditor.copy()

    def runScriptEditor(self):
        self.tabEditorWidget.currentWidget().newEditor.runScriptCode()

    def commentCode(self):
        self.tabEditorWidget.currentWidget().newEditor.commentEditorCode(True)

    def uncommentCode(self):
        self.tabEditorWidget.currentWidget().newEditor.commentEditorCode(False)

    def openScriptFileExtEditor(self):
        tabWidget = self.tabEditorWidget.currentWidget()
        path = tabWidget.path
        import subprocess
        try:
            subprocess.Popen([os.environ['EDITOR'], path])
        except KeyError:
            QDesktopServices.openUrl(QUrl.fromLocalFile(path))

    def openScriptFile(self):
        lastDirPath = self.settings.value("pythonConsole/lastDirPath", QDir.homePath())
        openFileTr = QCoreApplication.translate("PythonConsole", "Open File")
        fileList = QFileDialog.getOpenFileNames(
            self, openFileTr, lastDirPath, "Script file (*.py)")
        if fileList:
            for pyFile in fileList:
                for i in range(self.tabEditorWidget.count()):
                    tabWidget = self.tabEditorWidget.widget(i)
                    if tabWidget.path == pyFile:
                        self.tabEditorWidget.setCurrentWidget(tabWidget)
                        break
                else:
                    tabName = QFileInfo(pyFile).fileName()
                    self.tabEditorWidget.newTabEditor(tabName, pyFile)

                    lastDirPath = QFileInfo(pyFile).path()
                    self.settings.setValue("pythonConsole/lastDirPath", pyFile)
                    self.updateTabListScript(pyFile, action='append')

    def saveScriptFile(self):
        tabWidget = self.tabEditorWidget.currentWidget()
        try:
            tabWidget.save()
        except (IOError, OSError) as error:
            msgText = QCoreApplication.translate('PythonConsole',
                                                 'The file <b>{0}</b> could not be saved. Error: {1}').format(tabWidget.path,
                                                                                                              error.strerror)
            self.callWidgetMessageBarEditor(msgText, 2, False)

    def saveAsScriptFile(self, index=None):
        tabWidget = self.tabEditorWidget.currentWidget()
        if not index:
            index = self.tabEditorWidget.currentIndex()
        if not tabWidget.path:
            fileName = self.tabEditorWidget.tabText(index) + '.py'
            folder = self.settings.value("pythonConsole/lastDirPath", QDir.home())
            pathFileName = os.path.join(folder, fileName)
            fileNone = True
        else:
            pathFileName = tabWidget.path
            fileNone = False
        saveAsFileTr = QCoreApplication.translate("PythonConsole", "Save File As")
        filename = QFileDialog.getSaveFileName(self,
                                               saveAsFileTr,
                                               pathFileName, "Script file (*.py)")
        if filename:
            try:
                tabWidget.save(filename)
            except (IOError, OSError) as error:
                msgText = QCoreApplication.translate('PythonConsole',
                                                     'The file <b>{0}</b> could not be saved. Error: {1}').format(tabWidget.path,
                                                                                                                  error.strerror)
                self.callWidgetMessageBarEditor(msgText, 2, False)
                if fileNone:
                    tabWidget.path = None
                else:
                    tabWidget.path = pathFileName
                return

            if not fileNone:
                self.updateTabListScript(pathFileName, action='remove')

    def openHelp(self):
        QgsContextHelp.run("PythonConsole")

    def openSettings(self):
        if optionsDialog(self).exec_():
            self.shell.refreshSettingsShell()
            self.shellOut.refreshSettingsOutput()
            self.tabEditorWidget.refreshSettingsEditor()

    def callWidgetMessageBar(self, text):
        self.shellOut.widgetMessageBar(iface, text)

    def callWidgetMessageBarEditor(self, text, level, timed):
        self.tabEditorWidget.widgetMessageBar(iface, text, level, timed)

    def updateTabListScript(self, script, action=None):
        if action == 'remove':
            self.tabListScript.remove(script)
        elif action == 'append':
            if not self.tabListScript:
                self.tabListScript = []
            if script not in self.tabListScript:
                self.tabListScript.append(script)
        else:
            self.tabListScript = []
        self.settings.setValue("pythonConsole/tabScripts",
                               self.tabListScript)

    def saveSettingsConsole(self):
        self.settings.setValue("pythonConsole/splitterConsole", self.splitter.saveState())
        self.settings.setValue("pythonConsole/splitterObj", self.splitterObj.saveState())
        self.settings.setValue("pythonConsole/splitterEditor", self.splitterEditor.saveState())

        self.shell.writeHistoryFile(True)

    def restoreSettingsConsole(self):
        storedTabScripts = self.settings.value("pythonConsole/tabScripts", [])
        self.tabListScript = storedTabScripts
        self.splitter.restoreState(self.settings.value("pythonConsole/splitterConsole", QByteArray()))
        self.splitterEditor.restoreState(self.settings.value("pythonConsole/splitterEditor", QByteArray()))
        self.splitterObj.restoreState(self.settings.value("pythonConsole/splitterObj", QByteArray()))

if __name__ == '__main__':
    a = QApplication(sys.argv)
    console = PythonConsoleWidget()
    console.show()
    a.exec_()
