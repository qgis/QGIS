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
import subprocess

from qgis.PyQt.QtCore import (
    Qt,
    QTimer,
    QCoreApplication,
    QSize,
    QByteArray,
    QFileInfo,
    QUrl,
    QDir,
)
from qgis.PyQt.QtWidgets import (
    QToolBar,
    QToolButton,
    QWidget,
    QSplitter,
    QTreeWidget,
    QAction,
    QFileDialog,
    QCheckBox,
    QSizePolicy,
    QMenu,
    QGridLayout,
    QApplication,
    QShortcut,
)
from qgis.PyQt.QtGui import QDesktopServices, QKeySequence
from qgis.PyQt.QtWidgets import QVBoxLayout, QMessageBox
from qgis.utils import iface
from .console_sci import ShellScintilla
from .console_output import ShellOutputScintilla
from .console_editor import EditorTabWidget
from .console_settings import ConsoleOptionsFactory
from qgis.core import Qgis, QgsApplication, QgsSettings, QgsFileUtils
from qgis.gui import (
    QgsFilterLineEdit,
    QgsHelp,
    QgsDockWidget,
    QgsGui,
    QgsApplicationExitBlockerInterface,
    QgsCodeEditorDockWidget,
)
from functools import partial

import sys
import re

_console = None
_options_factory = ConsoleOptionsFactory()


def show_console():
    """called from QGIS to open the console"""
    global _console
    if _console is None:
        parent = iface.mainWindow() if iface else None
        _console = PythonConsole(parent)
        if iface:
            _console.visibilityChanged.connect(
                iface.actionShowPythonDialog().setChecked
            )

        _console.show()  # force show even if it was restored as hidden
        # set focus to the console so the user can start typing
        # defer the set focus event so it works also whether the console not visible yet
        QTimer.singleShot(0, _console.activate)
    else:
        _console.setUserVisible(not _console.isUserVisible())
        # set focus to the console so the user can start typing
        if _console.isUserVisible():
            _console.activate()

    return _console


_console_output = None

# hook for python console so all output will be redirected
# and then shown in console


def console_displayhook(obj):
    global _console_output
    _console_output = obj


def init_options_widget():
    """called from QGIS to add the console options widget"""
    global _options_factory
    _options_factory.setTitle(QCoreApplication.translate("PythonConsole", "Python"))
    iface.registerOptionsWidgetFactory(_options_factory)


class ConsoleExitBlocker(QgsApplicationExitBlockerInterface):

    def __init__(self, console):
        super().__init__()
        self.console = console

    def allowExit(self):
        return self.console.allowExit()


class PythonConsole(QgsCodeEditorDockWidget):

    def __init__(self, parent=None):
        super().__init__("PythonConsoleWindow", True)
        self.setDockObjectName("PythonConsole")
        self.setTitle(QCoreApplication.translate("PythonConsole", "Python Console"))

        self.console = PythonConsoleWidget(self)
        QgsGui.instance().optionsChanged.connect(self.console.updateSettings)
        vl = QVBoxLayout()
        vl.setContentsMargins(0, 0, 0, 0)
        vl.addWidget(self.console)
        self.setLayout(vl)
        self.setFocusProxy(self.console)

        # closeEvent is not always called for this widget -- so we also trigger a settings
        # save on application exit
        QgsApplication.instance().aboutToQuit.connect(self.on_app_exit)

    def on_app_exit(self):
        self.console.saveSettingsConsole()
        self.deleteLater()

    def activate(self):
        self.activateWindow()
        self.raise_()
        self.setFocus()

    def closeEvent(self, event):
        self.console.saveSettingsConsole()
        self.hide()
        event.ignore()


class PythonConsoleWidget(QWidget):

    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.setWindowTitle(
            QCoreApplication.translate("PythonConsole", "Python Console")
        )

        self.shell = ShellScintilla(console_widget=self)
        self.setFocusProxy(self.shell)
        self.shell_output = ShellOutputScintilla(
            console_widget=self, shell_editor=self.shell
        )
        self.tabEditorWidget = EditorTabWidget(console_widget=self)

        # ------------ UI -------------------------------

        self.splitterEditor = QSplitter(self)
        self.splitterEditor.setOrientation(Qt.Orientation.Horizontal)
        self.splitterEditor.setHandleWidth(6)
        self.splitterEditor.setChildrenCollapsible(True)

        self.shellOutWidget = QWidget(self)
        self.shellOutWidget.setLayout(QVBoxLayout())
        self.shellOutWidget.layout().setContentsMargins(0, 0, 0, 0)
        self.shellOutWidget.layout().addWidget(self.shell_output)

        self.splitter = QSplitter(self.splitterEditor)
        self.splitter.setOrientation(Qt.Orientation.Vertical)
        self.splitter.setHandleWidth(3)
        self.splitter.setChildrenCollapsible(False)
        self.splitter.addWidget(self.shellOutWidget)
        self.splitter.addWidget(self.shell)

        self.splitterObj = QSplitter(self.splitterEditor)
        self.splitterObj.setHandleWidth(3)
        self.splitterObj.setOrientation(Qt.Orientation.Horizontal)

        self.widgetEditor = QWidget(self.splitterObj)

        self.listClassMethod = QTreeWidget(self.splitterObj)
        self.listClassMethod.setColumnCount(2)
        objInspLabel = QCoreApplication.translate("PythonConsole", "Object Inspector")
        self.listClassMethod.setHeaderLabels([objInspLabel, ""])
        self.listClassMethod.setColumnHidden(1, True)
        self.listClassMethod.setAlternatingRowColors(True)

        # Hide side editor on start up
        self.splitterObj.hide()
        self.listClassMethod.hide()

        icon_size = iface.iconSize(dockedToolbar=True) if iface else QSize(16, 16)

        sizes = self.splitter.sizes()
        self.splitter.setSizes(sizes)

        # ----------------Restore Settings------------------------------------

        self.restoreSettingsConsole()

        # ------------------Toolbar Editor-------------------------------------

        # Action for Open File
        openFileBt = QCoreApplication.translate("PythonConsole", "Open Script…")
        self.openFileButton = QAction(self)
        self.openFileButton.setCheckable(False)
        self.openFileButton.setEnabled(True)
        self.openFileButton.setIcon(
            QgsApplication.getThemeIcon("mActionScriptOpen.svg")
        )
        self.openFileButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.openFileButton.setIconVisibleInMenu(True)
        self.openFileButton.setToolTip(openFileBt)
        self.openFileButton.setText(openFileBt)

        openExtEditorBt = QCoreApplication.translate(
            "PythonConsole", "Open in External Editor"
        )
        self.openInEditorButton = QAction(self)
        self.openInEditorButton.setCheckable(False)
        self.openInEditorButton.setEnabled(True)
        self.openInEditorButton.setIcon(
            QgsApplication.getThemeIcon("console/iconShowEditorConsole.svg")
        )
        self.openInEditorButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.openInEditorButton.setIconVisibleInMenu(True)
        self.openInEditorButton.setToolTip(openExtEditorBt)
        self.openInEditorButton.setText(openExtEditorBt)
        # Action for Save File
        saveFileBt = QCoreApplication.translate("PythonConsole", "Save")
        self.saveFileButton = QAction(self)
        self.saveFileButton.setCheckable(False)
        self.saveFileButton.setEnabled(False)
        self.saveFileButton.setIcon(QgsApplication.getThemeIcon("mActionFileSave.svg"))
        self.saveFileButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.saveFileButton.setIconVisibleInMenu(True)
        self.saveFileButton.setToolTip(saveFileBt)
        self.saveFileButton.setText(saveFileBt)
        # Action for Save File As
        saveAsFileBt = QCoreApplication.translate("PythonConsole", "Save As…")
        self.saveAsFileButton = QAction(self)
        self.saveAsFileButton.setCheckable(False)
        self.saveAsFileButton.setEnabled(True)
        self.saveAsFileButton.setIcon(
            QgsApplication.getThemeIcon("mActionFileSaveAs.svg")
        )
        self.saveAsFileButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.saveAsFileButton.setIconVisibleInMenu(True)
        self.saveAsFileButton.setToolTip(saveAsFileBt)
        self.saveAsFileButton.setText(saveAsFileBt)
        # Action Cut
        cutEditorBt = QCoreApplication.translate("PythonConsole", "Cut")
        self.cutEditorButton = QAction(self)
        self.cutEditorButton.setCheckable(False)
        self.cutEditorButton.setEnabled(True)
        self.cutEditorButton.setIcon(QgsApplication.getThemeIcon("mActionEditCut.svg"))
        self.cutEditorButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.cutEditorButton.setIconVisibleInMenu(True)
        self.cutEditorButton.setToolTip(cutEditorBt)
        self.cutEditorButton.setText(cutEditorBt)
        # Action Copy
        copyEditorBt = QCoreApplication.translate("PythonConsole", "Copy")
        self.copyEditorButton = QAction(self)
        self.copyEditorButton.setCheckable(False)
        self.copyEditorButton.setEnabled(True)
        self.copyEditorButton.setIcon(
            QgsApplication.getThemeIcon("mActionEditCopy.svg")
        )
        self.copyEditorButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.copyEditorButton.setIconVisibleInMenu(True)
        self.copyEditorButton.setToolTip(copyEditorBt)
        self.copyEditorButton.setText(copyEditorBt)
        # Action Paste
        pasteEditorBt = QCoreApplication.translate("PythonConsole", "Paste")
        self.pasteEditorButton = QAction(self)
        self.pasteEditorButton.setCheckable(False)
        self.pasteEditorButton.setEnabled(True)
        self.pasteEditorButton.setIcon(
            QgsApplication.getThemeIcon("mActionEditPaste.svg")
        )
        self.pasteEditorButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.pasteEditorButton.setIconVisibleInMenu(True)
        self.pasteEditorButton.setToolTip(pasteEditorBt)
        self.pasteEditorButton.setText(pasteEditorBt)
        # Action Run Script (subprocess)
        runScriptEditorBt = QCoreApplication.translate("PythonConsole", "Run Script")
        self.runScriptEditorButton = QAction(self)
        self.runScriptEditorButton.setCheckable(False)
        self.runScriptEditorButton.setEnabled(True)
        self.runScriptEditorButton.setIcon(
            QgsApplication.getThemeIcon("mActionStart.svg")
        )
        self.runScriptEditorButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.runScriptEditorButton.setIconVisibleInMenu(True)
        self.runScriptEditorButton.setToolTip(runScriptEditorBt)
        self.runScriptEditorButton.setText(runScriptEditorBt)

        # Action Toggle comment
        toggleText = QCoreApplication.translate("PythonConsole", "Toggle Comment")
        self.toggleCommentEditorButton = QAction(self)
        self.toggleCommentEditorButton.setCheckable(False)
        self.toggleCommentEditorButton.setEnabled(True)
        self.toggleCommentEditorButton.setIcon(
            QgsApplication.getThemeIcon("console/iconCommentEditorConsole.svg")
        )
        self.toggleCommentEditorButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.toggleCommentEditorButton.setIconVisibleInMenu(True)
        self.toggleCommentEditorButton.setToolTip(toggleText + " <b>Ctrl+:</b>")
        self.toggleCommentEditorButton.setText(toggleText)

        # Action Format code
        reformatCodeText = QCoreApplication.translate("PythonConsole", "Reformat Code")
        self.reformatCodeEditorButton = QAction(self)
        self.reformatCodeEditorButton.setCheckable(False)
        self.reformatCodeEditorButton.setEnabled(True)
        self.reformatCodeEditorButton.setIcon(
            QgsApplication.getThemeIcon("console/iconFormatCode.svg")
        )
        self.reformatCodeEditorButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.reformatCodeEditorButton.setIconVisibleInMenu(True)
        self.reformatCodeEditorButton.setToolTip(
            reformatCodeText + " <b>Ctrl+Alt+F</b>"
        )
        self.reformatCodeEditorButton.setShortcut("Ctrl+Alt+F")
        self.reformatCodeEditorButton.setText(reformatCodeText)

        # Action for Object browser
        objList = QCoreApplication.translate("PythonConsole", "Object Inspector…")
        self.objectListButton = QAction(self)
        self.objectListButton.setCheckable(True)
        self.objectListButton.setEnabled(
            QgsSettings().value("pythonConsole/enableObjectInsp", False, type=bool)
        )
        self.objectListButton.setIcon(
            QgsApplication.getThemeIcon("console/iconClassBrowserConsole.svg")
        )
        self.objectListButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.objectListButton.setIconVisibleInMenu(True)
        self.objectListButton.setToolTip(objList)
        self.objectListButton.setText(objList)

        # Action for Find text
        findText = QCoreApplication.translate("PythonConsole", "Find Text")
        self.find_text_action = QAction(self)
        self.find_text_action.setCheckable(True)
        self.find_text_action.setEnabled(True)
        self.find_text_action.setIcon(
            QgsApplication.getThemeIcon("console/iconSearchEditorConsole.svg")
        )
        self.find_text_action.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.find_text_action.setIconVisibleInMenu(True)
        self.find_text_action.setToolTip(findText)
        self.find_text_action.setText(findText)

        self.tabEditorWidget.search_bar_toggled.connect(
            self.find_text_action.setChecked
        )
        self.find_text_action.toggled.connect(self.tabEditorWidget.toggle_search_bar)

        # ----------------Toolbar Console-------------------------------------

        # Action Show Editor
        showEditor = QCoreApplication.translate("PythonConsole", "Show Editor")
        self.showEditorButton = QAction(self)
        self.showEditorButton.setEnabled(True)
        self.showEditorButton.setCheckable(True)
        self.showEditorButton.setIcon(
            QgsApplication.getThemeIcon("console/iconShowEditorConsole.svg")
        )
        self.showEditorButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.showEditorButton.setIconVisibleInMenu(True)
        self.showEditorButton.setToolTip(showEditor)
        self.showEditorButton.setText(showEditor)
        # Action for Clear button
        clearBt = QCoreApplication.translate("PythonConsole", "Clear Console")
        self.clearButton = QAction(self)
        self.clearButton.setCheckable(False)
        self.clearButton.setEnabled(True)
        self.clearButton.setIcon(
            QgsApplication.getThemeIcon("console/iconClearConsole.svg")
        )
        self.clearButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.clearButton.setIconVisibleInMenu(True)
        self.clearButton.setToolTip(clearBt)
        self.clearButton.setText(clearBt)
        # Action for settings
        optionsBt = QCoreApplication.translate("PythonConsole", "Options…")
        self.optionsButton = QAction(self)
        self.optionsButton.setCheckable(False)
        self.optionsButton.setEnabled(True)
        self.optionsButton.setIcon(
            QgsApplication.getThemeIcon("console/iconSettingsConsole.svg")
        )
        self.optionsButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.optionsButton.setIconVisibleInMenu(True)
        self.optionsButton.setToolTip(optionsBt)
        self.optionsButton.setText(optionsBt)
        # Action for Run script
        runBt = QCoreApplication.translate("PythonConsole", "Run Command")
        self.runButton = QAction(self)
        self.runButton.setCheckable(False)
        self.runButton.setEnabled(True)
        self.runButton.setIcon(QgsApplication.getThemeIcon("mActionStart.svg"))
        self.runButton.setMenuRole(QAction.MenuRole.PreferencesRole)
        self.runButton.setIconVisibleInMenu(True)
        self.runButton.setToolTip(runBt)
        self.runButton.setText(runBt)

        # Help button
        self.helpConsoleAction = QAction(self)
        self.helpConsoleAction.setEnabled(True)
        self.helpConsoleAction.setText(
            QCoreApplication.translate("PythonConsole", "Python Console Help")
        )
        self.helpAPIAction = QAction(self)
        self.helpAPIAction.setEnabled(True)
        self.helpAPIAction.setText(
            QCoreApplication.translate("PythonConsole", "PyQGIS API Documentation")
        )
        self.helpCookbookAction = QAction(self)
        self.helpCookbookAction.setEnabled(True)
        self.helpCookbookAction.setText(
            QCoreApplication.translate("PythonConsole", "PyQGIS Cookbook")
        )

        self.helpMenu = QMenu(self)
        self.helpMenu.addAction(self.helpConsoleAction)
        self.helpMenu.addAction(self.helpAPIAction)
        self.helpMenu.addAction(self.helpCookbookAction)

        helpBt = QCoreApplication.translate("PythonConsole", "Help…")
        self.helpButton = QToolButton(self)
        self.helpButton.setPopupMode(QToolButton.ToolButtonPopupMode.InstantPopup)
        self.helpButton.setEnabled(True)
        self.helpButton.setIcon(
            QgsApplication.getThemeIcon("console/iconHelpConsole.svg")
        )
        self.helpButton.setToolTip(helpBt)
        self.helpButton.setMenu(self.helpMenu)

        self.toolBar = QToolBar()
        self.toolBar.setEnabled(True)
        self.toolBar.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.toolBar.setContextMenuPolicy(Qt.ContextMenuPolicy.DefaultContextMenu)
        self.toolBar.setLayoutDirection(Qt.LayoutDirection.LeftToRight)
        self.toolBar.setIconSize(icon_size)
        self.toolBar.setMovable(False)
        self.toolBar.setFloatable(False)
        self.toolBar.addAction(self.clearButton)
        self.toolBar.addAction(self.runButton)
        self.toolBar.addSeparator()
        self.toolBar.addAction(self.showEditorButton)
        self.toolBar.addSeparator()
        self.toolBar.addAction(self.optionsButton)
        self.toolBar.addWidget(self.helpButton)
        self.toolBar.addSeparator()
        self.toolBar.addWidget(parent.dockToggleButton())

        self.toolBarEditor = QToolBar()
        self.toolBarEditor.setEnabled(False)
        self.toolBarEditor.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.toolBarEditor.setContextMenuPolicy(Qt.ContextMenuPolicy.DefaultContextMenu)
        self.toolBarEditor.setLayoutDirection(Qt.LayoutDirection.LeftToRight)
        self.toolBarEditor.setIconSize(icon_size)
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
        self.toolBarEditor.addAction(self.cutEditorButton)
        self.toolBarEditor.addAction(self.copyEditorButton)
        self.toolBarEditor.addAction(self.pasteEditorButton)
        self.toolBarEditor.addSeparator()
        self.toolBarEditor.addAction(self.find_text_action)
        self.toolBarEditor.addSeparator()
        self.toolBarEditor.addAction(self.toggleCommentEditorButton)
        self.toolBarEditor.addAction(self.reformatCodeEditorButton)
        self.toolBarEditor.addSeparator()
        self.toolBarEditor.addAction(self.objectListButton)

        self.widgetButton = QWidget()
        sizePolicy = QSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.widgetButton.sizePolicy().hasHeightForWidth())
        self.widgetButton.setSizePolicy(sizePolicy)

        self.widgetButtonEditor = QWidget(self.widgetEditor)
        sizePolicy = QSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(
            self.widgetButtonEditor.sizePolicy().hasHeightForWidth()
        )
        self.widgetButtonEditor.setSizePolicy(sizePolicy)

        sizePolicy = QSizePolicy(
            QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding
        )
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.shell_output.sizePolicy().hasHeightForWidth())
        self.shell_output.setSizePolicy(sizePolicy)

        self.shell_output.setVerticalScrollBarPolicy(
            Qt.ScrollBarPolicy.ScrollBarAsNeeded
        )
        self.shell.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)

        # ------------ Layout -------------------------------

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

        # ------------ Add first Tab in Editor -------------------------------

        # self.tabEditorWidget.newTabEditor(tabName='first', filename=None)

        # ------------ Signal -------------------------------

        self.objectListButton.toggled.connect(self.toggleObjectListWidget)
        self.toggleCommentEditorButton.triggered.connect(self.toggleComment)
        self.reformatCodeEditorButton.triggered.connect(self.reformatCode)
        self.runScriptEditorButton.triggered.connect(self.runScriptEditor)
        self.cutEditorButton.triggered.connect(self.cutEditor)
        self.copyEditorButton.triggered.connect(self.copyEditor)
        self.pasteEditorButton.triggered.connect(self.pasteEditor)
        self.showEditorButton.toggled.connect(self.toggleEditor)
        self.clearButton.triggered.connect(self.shell_output.clearConsole)
        self.optionsButton.triggered.connect(self.openSettings)
        self.runButton.triggered.connect(self.shell.entered)
        self.openFileButton.triggered.connect(self.openScriptFile)
        self.openInEditorButton.triggered.connect(self.openScriptFileExtEditor)
        self.saveFileButton.triggered.connect(self.saveScriptFile)
        self.saveAsFileButton.triggered.connect(self.saveAsScriptFile)
        self.helpConsoleAction.triggered.connect(self.openHelpConsole)
        self.helpAPIAction.triggered.connect(self.openHelpAPI)
        self.helpCookbookAction.triggered.connect(self.openHelpCookbook)
        self.listClassMethod.itemClicked.connect(self.onClickGoToLine)

        if iface is not None:
            self.exit_blocker = ConsoleExitBlocker(self)
            iface.registerApplicationExitBlocker(self.exit_blocker)

    def allowExit(self):
        tab_count = self.tabEditorWidget.count()
        for i in range(tab_count):
            # iterate backwards through tabs, as we may be closing some as we go
            tab_index = tab_count - i - 1
            tab_widget = self.tabEditorWidget.widget(tab_index)
            if tab_widget.isModified():
                ret = QMessageBox.question(
                    self,
                    self.tr("Save {}").format(self.tabEditorWidget.tabText(tab_index)),
                    self.tr(
                        "There are unsaved changes in this script. Do you want to keep those?"
                    ),
                    QMessageBox.StandardButton.Save
                    | QMessageBox.StandardButton.Cancel
                    | QMessageBox.StandardButton.Discard,
                    QMessageBox.StandardButton.Cancel,
                )
                if ret == QMessageBox.StandardButton.Save:
                    tab_widget.save()
                    if tab_widget.isModified():
                        # save failed, treat as cancel
                        return False
                elif ret == QMessageBox.StandardButton.Discard:
                    pass
                else:
                    return False

            self.tabEditorWidget.removeTab(tab_index)

        return True

    def _toggleFind(self):
        self.tabEditorWidget.currentWidget().toggleFindWidget()

    def onClickGoToLine(self, item, column):
        tabEditor = self.tabEditorWidget.currentWidget()
        if item.text(1) == "syntaxError":
            check = tabEditor.syntaxCheck()
            if check and not tabEditor.isReadOnly():
                self.tabEditorWidget.currentWidget().save()
            return
        linenr = int(item.text(1))
        itemName = str(item.text(0))
        charPos = itemName.find(" ")
        if charPos != -1:
            objName = itemName[0:charPos]
        else:
            objName = itemName
        tabEditor.goToLine(str.encode(objName), linenr)

    def toggleEditor(self, checked):
        self.splitterObj.show() if checked else self.splitterObj.hide()
        if not self.tabEditorWidget:
            self.tabEditorWidget.enableToolBarEditor(checked)
            self.tabEditorWidget.restoreTabsOrAddNew()

    def toggleObjectListWidget(self, checked):
        self.listClassMethod.show() if checked else self.listClassMethod.hide()

    def pasteEditor(self):
        self.tabEditorWidget.currentWidget().paste()

    def cutEditor(self):
        self.tabEditorWidget.currentWidget().cut()

    def copyEditor(self):
        self.tabEditorWidget.currentWidget().copy()

    def runScriptEditor(self):
        self.tabEditorWidget.currentWidget().runScriptCode()

    def toggleComment(self):
        self.tabEditorWidget.currentWidget().toggleComment()

    def reformatCode(self):
        self.tabEditorWidget.currentWidget().reformatCode()

    def openScriptFileExtEditor(self):
        tabWidget = self.tabEditorWidget.currentWidget()
        tabWidget.open_in_external_editor()

    def openScriptFile(self):
        settings = QgsSettings()
        lastDirPath = settings.value("pythonConsole/lastDirPath", QDir.homePath())
        openFileTr = QCoreApplication.translate("PythonConsole", "Open File")
        fileList, selected_filter = QFileDialog.getOpenFileNames(
            self, openFileTr, lastDirPath, "Script file (*.py)"
        )
        if fileList:
            for pyFile in fileList:
                for i in range(self.tabEditorWidget.count()):
                    tabWidget = self.tabEditorWidget.widget(i)
                    if tabWidget.file_path() == pyFile:
                        self.tabEditorWidget.setCurrentWidget(tabWidget)
                        break
                else:
                    tabName = QFileInfo(pyFile).fileName()
                    self.tabEditorWidget.newTabEditor(tabName, pyFile)

                    lastDirPath = QFileInfo(pyFile).path()
                    settings.setValue("pythonConsole/lastDirPath", pyFile)
                    self.updateTabListScript(pyFile, action="append")

    def saveScriptFile(self):
        tabWidget = self.tabEditorWidget.currentWidget()
        try:
            tabWidget.save()
        except OSError as error:
            msgText = QCoreApplication.translate(
                "PythonConsole", "The file <b>{0}</b> could not be saved. Error: {1}"
            ).format(tabWidget.file_path(), error.strerror)
            self.callWidgetMessageBarEditor(msgText, Qgis.MessageLevel.Critical)

    def saveAsScriptFile(self, index=None):
        tabWidget = self.tabEditorWidget.currentWidget()
        if not index:
            index = self.tabEditorWidget.currentIndex()
        if not tabWidget.file_path():
            fileName = self.tabEditorWidget.tabText(index).replace("*", "")
            fileName = QgsFileUtils.ensureFileNameHasExtension(fileName, ["py"])
            folder = QgsSettings().value("pythonConsole/lastDirPath", QDir.homePath())
            pathFileName = os.path.join(folder, fileName)
            fileNone = True
        else:
            pathFileName = tabWidget.file_path()
            fileNone = False
        saveAsFileTr = QCoreApplication.translate("PythonConsole", "Save File As")
        filename, filter = QFileDialog.getSaveFileName(
            self, saveAsFileTr, pathFileName, "Script file (*.py)"
        )
        if filename:
            filename = QgsFileUtils.ensureFileNameHasExtension(filename, ["py"])

            try:
                tabWidget.save(filename)
            except OSError as error:
                msgText = QCoreApplication.translate(
                    "PythonConsole",
                    "The file <b>{0}</b> could not be saved. Error: {1}",
                ).format(tabWidget.file_path(), error.strerror)
                self.callWidgetMessageBarEditor(msgText, Qgis.MessageLevel.Critical)
                if fileNone:
                    tabWidget.set_file_path(None)
                else:
                    tabWidget.set_file_path(pathFileName)
                return

            if not fileNone:
                self.updateTabListScript(pathFileName, action="remove")

    def openHelpConsole(self):
        QgsHelp.openHelp("plugins/python_console.html")

    def openHelpAPI(self):
        m = re.search(r"^([0-9]+)\.([0-9]+)\.", Qgis.QGIS_VERSION)
        if m:
            QDesktopServices.openUrl(
                QUrl(f"https://qgis.org/pyqgis/{m.group(1)}.{m.group(2)}/")
            )

    def openHelpCookbook(self):
        m = re.search(r"^([0-9]+)\.([0-9]+)\.", Qgis.QGIS_VERSION)
        if m:
            QDesktopServices.openUrl(
                QUrl(
                    f"https://docs.qgis.org/{m.group(1)}.{m.group(2)}/en/docs/pyqgis_developer_cookbook/index.html"
                )
            )

    def openSettings(self):
        iface.showOptionsDialog(iface.mainWindow(), currentPage="consoleOptions")

    def updateSettings(self):
        self.shell.refreshSettingsShell()
        self.shell_output.refreshSettingsOutput()
        self.tabEditorWidget.refreshSettingsEditor()

    def callWidgetMessageBar(self, text):
        self.shell_output.widgetMessageBar(text)

    def callWidgetMessageBarEditor(self, text, level):
        self.tabEditorWidget.showMessage(text, level)

    def updateTabListScript(self, script, action=None):
        if action == "remove":
            self.tabListScript.remove(script)
        elif action == "append":
            if not self.tabListScript:
                self.tabListScript = []
            if script not in self.tabListScript:
                self.tabListScript.append(script)
        else:
            self.tabListScript = []
        QgsSettings().setValue("pythonConsole/tabScripts", self.tabListScript)

    def saveSettingsConsole(self):
        settings = QgsSettings()
        settings.setValue("pythonConsole/splitterConsole", self.splitter.saveState())
        settings.setValue("pythonConsole/splitterObj", self.splitterObj.saveState())
        settings.setValue(
            "pythonConsole/splitterEditor", self.splitterEditor.saveState()
        )

        self.shell.writeHistoryFile()

    def restoreSettingsConsole(self):
        settings = QgsSettings()
        storedTabScripts = settings.value("pythonConsole/tabScripts", [])
        self.tabListScript = storedTabScripts
        self.splitter.restoreState(
            settings.value("pythonConsole/splitterConsole", QByteArray())
        )
        self.splitterEditor.restoreState(
            settings.value("pythonConsole/splitterEditor", QByteArray())
        )
        self.splitterObj.restoreState(
            settings.value("pythonConsole/splitterObj", QByteArray())
        )


if __name__ == "__main__":
    a = QApplication(sys.argv)
    console = PythonConsoleWidget()
    console.show()
    a.exec()
