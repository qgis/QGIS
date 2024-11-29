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

from __future__ import annotations

import codecs
import importlib
import os
import pyclbr
import re
import sys
import tempfile
from typing import Optional, TYPE_CHECKING
from functools import partial
from operator import itemgetter
from pathlib import Path

from qgis.core import Qgis, QgsApplication, QgsBlockingNetworkRequest, QgsSettings
from qgis.gui import QgsCodeEditorPython, QgsCodeEditorWidget, QgsMessageBar

from qgis.PyQt.Qsci import QsciScintilla
from qgis.PyQt.QtCore import (
    pyqtSignal,
    QByteArray,
    QCoreApplication,
    QDir,
    QEvent,
    QFileInfo,
    QJsonDocument,
    QSize,
    Qt,
    QUrl,
)
from qgis.PyQt.QtGui import QKeySequence
from qgis.PyQt.QtNetwork import QNetworkRequest
from qgis.PyQt.QtWidgets import (
    QAction,
    QApplication,
    QFileDialog,
    QFrame,
    QGridLayout,
    QLabel,
    QMenu,
    QMessageBox,
    QShortcut,
    QSizePolicy,
    QSpacerItem,
    QTabWidget,
    QToolButton,
    QTreeWidgetItem,
    QWidget,
)
from qgis.utils import OverrideCursor, iface

if TYPE_CHECKING:
    from .console import PythonConsoleWidget


class Editor(QgsCodeEditorPython):

    trigger_find = pyqtSignal()

    def __init__(
        self,
        editor_tab: EditorTab,
        console_widget: PythonConsoleWidget,
        tab_widget: EditorTabWidget,
    ):
        super().__init__(editor_tab)
        self.editor_tab: EditorTab = editor_tab
        self.console_widget: PythonConsoleWidget = console_widget
        self.tab_widget: EditorTabWidget = tab_widget
        self.code_editor_widget: QgsCodeEditorWidget | None = None

        self.setMinimumHeight(120)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)

        # Disable default scintilla shortcuts
        ctrl, shift = self.SCMOD_CTRL << 16, self.SCMOD_SHIFT << 16
        self.SendScintilla(
            QsciScintilla.SCI_CLEARCMDKEY, ord("T") + ctrl
        )  # Switch current line with the next one
        self.SendScintilla(
            QsciScintilla.SCI_CLEARCMDKEY, ord("D") + ctrl
        )  # Duplicate current line / selection
        self.SendScintilla(
            QsciScintilla.SCI_CLEARCMDKEY, ord("L") + ctrl
        )  # Delete current line
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord("L") + ctrl + shift)

        # New QShortcut = ctrl+space/ctrl+alt+space for Autocomplete
        self.newShortcutCS = QShortcut(
            QKeySequence(Qt.Modifier.CTRL | Qt.Key.Key_Space), self
        )
        self.newShortcutCS.setContext(Qt.ShortcutContext.WidgetShortcut)
        self.redoScut = QShortcut(
            QKeySequence(Qt.Modifier.CTRL | Qt.Modifier.SHIFT | Qt.Key.Key_Z), self
        )
        self.redoScut.setContext(Qt.ShortcutContext.WidgetShortcut)
        self.redoScut.activated.connect(self.redo)
        self.newShortcutCS.activated.connect(self.autoComplete)
        self.runScut = QShortcut(QKeySequence(Qt.Modifier.CTRL | Qt.Key.Key_E), self)
        self.runScut.setContext(Qt.ShortcutContext.WidgetShortcut)
        self.runScut.activated.connect(self.runSelectedCode)  # spellok
        self.runScriptScut = QShortcut(
            QKeySequence(Qt.Modifier.SHIFT | Qt.Modifier.CTRL | Qt.Key.Key_E), self
        )
        self.runScriptScut.setContext(Qt.ShortcutContext.WidgetShortcut)
        self.runScriptScut.activated.connect(self.runScriptCode)

        self.syntaxCheckScut = QShortcut(
            QKeySequence(Qt.Modifier.CTRL | Qt.Key.Key_4), self
        )
        self.syntaxCheckScut.setContext(Qt.ShortcutContext.WidgetShortcut)
        self.syntaxCheckScut.activated.connect(self.syntaxCheck)
        self.modificationChanged.connect(self.editor_tab.modified)
        self.modificationAttempted.connect(self.fileReadOnly)

    def showApiDocumentation(self, text):
        self.console_widget.shell.showApiDocumentation(text)

    def set_code_editor_widget(self, widget: QgsCodeEditorWidget):
        self.code_editor_widget = widget
        self.code_editor_widget.loadedExternalChanges.connect(
            self.loaded_external_changes
        )

    def settingsEditor(self):
        # Set Python lexer
        self.initializeLexer()

    def contextMenuEvent(self, e):
        menu = QMenu(self)
        menu.addAction(
            QCoreApplication.translate("PythonConsole", "Hide Editor"), self.hideEditor
        )
        menu.addSeparator()

        syntaxCheckAction = QAction(
            QgsApplication.getThemeIcon("console/iconSyntaxErrorConsole.svg"),
            QCoreApplication.translate("PythonConsole", "Check Syntax"),
            menu,
        )
        syntaxCheckAction.triggered.connect(self.syntaxCheck)
        syntaxCheckAction.setShortcut("Ctrl+4")
        menu.addAction(syntaxCheckAction)

        runSelected = QAction(
            QgsApplication.getThemeIcon("console/mIconRunConsole.svg"),  # spellok
            QCoreApplication.translate("PythonConsole", "Run Selected"),
            menu,
        )
        runSelected.triggered.connect(self.runSelectedCode)  # spellok
        runSelected.setShortcut("Ctrl+E")  # spellok
        menu.addAction(runSelected)  # spellok

        word = self.selectedText() or self.wordAtPoint(e.pos())
        if word:
            context_help_action = QAction(
                QgsApplication.getThemeIcon("mActionHelpContents.svg"),
                QCoreApplication.translate("PythonConsole", "Context Help"),
                menu,
            )
            context_help_action.triggered.connect(
                partial(
                    self.console_widget.shell.showApiDocumentation,
                    word,
                    force_search=True,
                )
            )
            context_help_action.setShortcut("F1")
            menu.addAction(context_help_action)

        start_action = QAction(
            QgsApplication.getThemeIcon("mActionStart.svg"),
            QCoreApplication.translate("PythonConsole", "Run Script"),
            menu,
        )
        start_action.triggered.connect(self.runScriptCode)
        start_action.setShortcut("Ctrl+Shift+E")
        menu.addAction(start_action)

        menu.addSeparator()
        undoAction = QAction(
            QgsApplication.getThemeIcon("mActionUndo.svg"),
            QCoreApplication.translate("PythonConsole", "Undo"),
            menu,
        )
        undoAction.triggered.connect(self.undo)
        undoAction.setShortcut(QKeySequence.StandardKey.Undo)
        menu.addAction(undoAction)

        redoAction = QAction(
            QgsApplication.getThemeIcon("mActionRedo.svg"),
            QCoreApplication.translate("PythonConsole", "Redo"),
            menu,
        )
        redoAction.triggered.connect(self.redo)
        redoAction.setShortcut("Ctrl+Shift+Z")
        menu.addAction(redoAction)

        menu.addSeparator()
        find_action = QAction(
            QgsApplication.getThemeIcon("console/iconSearchEditorConsole.svg"),
            QCoreApplication.translate("PythonConsole", "Find Text"),
            menu,
        )
        find_action.triggered.connect(self.trigger_find)
        menu.addAction(find_action)

        cutAction = QAction(
            QgsApplication.getThemeIcon("mActionEditCut.svg"),
            QCoreApplication.translate("PythonConsole", "Cut"),
            menu,
        )
        cutAction.triggered.connect(self.cut)
        cutAction.setShortcut(QKeySequence.StandardKey.Cut)
        menu.addAction(cutAction)

        copyAction = QAction(
            QgsApplication.getThemeIcon("mActionEditCopy.svg"),
            QCoreApplication.translate("PythonConsole", "Copy"),
            menu,
        )
        copyAction.triggered.connect(self.copy)
        copyAction.setShortcut(QKeySequence.StandardKey.Copy)
        menu.addAction(copyAction)

        pasteAction = QAction(
            QgsApplication.getThemeIcon("mActionEditPaste.svg"),
            QCoreApplication.translate("PythonConsole", "Paste"),
            menu,
        )
        pasteAction.triggered.connect(self.paste)
        pasteAction.setShortcut(QKeySequence.StandardKey.Paste)
        menu.addAction(pasteAction)

        selectAllAction = QAction(
            QCoreApplication.translate("PythonConsole", "Select All"), menu
        )
        selectAllAction.triggered.connect(self.selectAll)
        selectAllAction.setShortcut(QKeySequence.StandardKey.SelectAll)
        menu.addAction(selectAllAction)

        menu.addSeparator()
        toggle_comment_action = QAction(
            QgsApplication.getThemeIcon("console/iconCommentEditorConsole.svg"),
            QCoreApplication.translate("PythonConsole", "Toggle Comment"),
            menu,
        )
        toggle_comment_action.triggered.connect(self.toggleComment)
        toggle_comment_action.setShortcut("Ctrl+:")
        menu.addAction(toggle_comment_action)

        menu.addSeparator()
        gist_menu = QMenu(self)
        gist_menu.setTitle(
            QCoreApplication.translate("PythonConsole", "Share on GitHub")
        )
        gist_menu.setIcon(QgsApplication.getThemeIcon("console/iconCodepadConsole.svg"))
        gist_menu.addAction(
            QCoreApplication.translate("PythonConsole", "Secret Gist"),
            partial(self.shareOnGist, False),
        )
        gist_menu.addAction(
            QCoreApplication.translate("PythonConsole", "Public Gist"),
            partial(self.shareOnGist, True),
        )
        menu.addMenu(gist_menu)
        showCodeInspection = menu.addAction(
            QgsApplication.getThemeIcon("console/iconClassBrowserConsole.svg"),
            QCoreApplication.translate("PythonConsole", "Hide/Show Object Inspector"),
            self.objectListEditor,
        )
        menu.addSeparator()
        menu.addAction(
            QgsApplication.getThemeIcon("console/iconSettingsConsole.svg"),
            QCoreApplication.translate("PythonConsole", "Options…"),
            self.console_widget.openSettings,
        )
        syntaxCheckAction.setEnabled(False)
        pasteAction.setEnabled(False)
        cutAction.setEnabled(False)
        runSelected.setEnabled(False)  # spellok
        copyAction.setEnabled(False)
        selectAllAction.setEnabled(False)
        undoAction.setEnabled(False)
        redoAction.setEnabled(False)
        showCodeInspection.setEnabled(False)
        if self.hasSelectedText():
            runSelected.setEnabled(True)  # spellok
            copyAction.setEnabled(True)
            cutAction.setEnabled(True)
        if not self.text() == "":
            selectAllAction.setEnabled(True)
            syntaxCheckAction.setEnabled(True)
        if self.isUndoAvailable():
            undoAction.setEnabled(True)
        if self.isRedoAvailable():
            redoAction.setEnabled(True)
        if QApplication.clipboard().text():
            pasteAction.setEnabled(True)
        if QgsSettings().value("pythonConsole/enableObjectInsp", False, type=bool):
            showCodeInspection.setEnabled(True)
        menu.exec(self.mapToGlobal(e.pos()))

    def objectListEditor(self):
        listObj = self.console_widget.listClassMethod
        if listObj.isVisible():
            listObj.hide()
            self.console_widget.objectListButton.setChecked(False)
        else:
            listObj.show()
            self.console_widget.objectListButton.setChecked(True)

    def shareOnGist(self, is_public):
        self.code_editor_widget.shareOnGist(is_public)

    def hideEditor(self):
        self.console_widget.splitterObj.hide()
        self.console_widget.showEditorButton.setChecked(False)

    def createTempFile(self):
        name = tempfile.NamedTemporaryFile(delete=False).name
        # Need to use newline='' to avoid adding extra \r characters on Windows
        with open(name, "w", encoding="utf-8", newline="") as f:
            f.write(self.text())
        return name

    def runScriptCode(self):
        autoSave = QgsSettings().value("pythonConsole/autoSaveScript", False, type=bool)
        filename = self.code_editor_widget.filePath()
        filename_override = None
        msgEditorBlank = QCoreApplication.translate(
            "PythonConsole", "Hey, type something to run!"
        )
        if filename is None:
            if not self.isModified():
                self.showMessage(msgEditorBlank)
                return

        deleteTempFile = False
        if self.syntaxCheck():
            if filename and self.isModified() and autoSave:
                self.save(filename)
            elif not filename or self.isModified():
                # Create a new temp file if the file isn't already saved.
                filename = self.createTempFile()
                filename_override = self.tab_widget.tabText(
                    self.tab_widget.currentIndex()
                )
                if filename_override.startswith("*"):
                    filename_override = filename_override[1:]
                deleteTempFile = True

            self.console_widget.shell.runFile(filename, filename_override)

            if deleteTempFile:
                Path(filename).unlink()

    def runSelectedCode(self):  # spellok
        cmd = self.selectedText()
        self.console_widget.shell.insertFromDropPaste(cmd)
        self.console_widget.shell.entered()
        self.setFocus()

    def getTextFromEditor(self):
        text = self.text()
        textList = text.split("\n")
        return textList

    def goToLine(self, objName, linenr):
        self.SendScintilla(QsciScintilla.SCI_GOTOLINE, linenr - 1)
        self.SendScintilla(
            QsciScintilla.SCI_SETTARGETSTART,
            self.SendScintilla(QsciScintilla.SCI_GETCURRENTPOS),
        )
        self.SendScintilla(QsciScintilla.SCI_SETTARGETEND, len(self.text()))
        pos = self.SendScintilla(
            QsciScintilla.SCI_SEARCHINTARGET, len(objName), objName
        )
        index = pos - self.SendScintilla(QsciScintilla.SCI_GETCURRENTPOS)
        # line, _ = self.getCursorPosition()
        self.setSelection(linenr - 1, index, linenr - 1, index + len(objName))
        self.ensureLineVisible(linenr)
        self.setFocus()

    def syntaxCheck(self):
        self.code_editor_widget.clearWarnings()
        source = self.text().encode("utf-8")
        try:
            compile(source, "", "exec")
        except SyntaxError as detail:
            eline = detail.lineno or 1
            eline -= 1
            ecolumn = detail.offset or 1
            edescr = detail.msg
            self.code_editor_widget.addWarning(eline, edescr)
            self.setCursorPosition(eline, ecolumn - 1)
            self.ensureLineVisible(eline)
            return False

        return True

    def loaded_external_changes(self):
        self.tab_widget.listObject(self.tab_widget.currentWidget())

    def fileReadOnly(self):
        msgText = QCoreApplication.translate(
            "PythonConsole",
            'The file <b>"{0}"</b> is read only, please save to different file first.',
        ).format(self.code_editor_widget.filePath())
        self.showMessage(msgText)

    def save(self, filename: str | None = None):
        if self.isReadOnly():
            return

        if QgsSettings().value("pythonConsole/formatOnSave", False, type=bool):
            self.reformatCode()

        index = self.tab_widget.indexOf(self.editor_tab)
        if not filename and not self.code_editor_widget.filePath():
            saveTr = QCoreApplication.translate(
                "PythonConsole", "Python Console: Save file"
            )
            folder = QgsSettings().value("pythonConsole/lastDirPath", QDir.homePath())
            path, filter = QFileDialog().getSaveFileName(
                self,
                saveTr,
                os.path.join(
                    folder, self.tab_widget.tabText(index).replace("*", "") + ".py"
                ),
                "Script file (*.py)",
            )
            # If the user didn't select a file, abort the save operation
            if not path:
                self.code_editor_widget.setFilePath(None)
                return
            filename = path

        self.code_editor_widget.save(filename)

        msgText = QCoreApplication.translate(
            "PythonConsole", "Script was correctly saved."
        )
        self.showMessage(msgText)

        # Save the new contents
        # Need to use newline='' to avoid adding extra \r characters on Windows
        with open(
            self.code_editor_widget.filePath(), "w", encoding="utf-8", newline=""
        ) as f:
            f.write(self.text())
        self.tab_widget.setTabTitle(
            index, Path(self.code_editor_widget.filePath()).name
        )
        self.tab_widget.setTabToolTip(index, self.code_editor_widget.filePath())
        self.setModified(False)
        self.console_widget.saveFileButton.setEnabled(False)
        self.console_widget.updateTabListScript(
            self.code_editor_widget.filePath(), action="append"
        )
        self.tab_widget.listObject(self.editor_tab)
        QgsSettings().setValue(
            "pythonConsole/lastDirPath",
            Path(self.code_editor_widget.filePath()).parent.as_posix(),
        )

    def event(self, e):
        """Used to override the Application shortcuts when the editor has focus"""

        if e.type() == QEvent.Type.ShortcutOverride:
            ctrl = e.modifiers() == Qt.KeyboardModifier.ControlModifier
            ctrl_shift = e.modifiers() == (
                Qt.KeyboardModifier.ControlModifier | Qt.KeyboardModifier.ShiftModifier
            )
            if (
                (ctrl and e.key() == Qt.Key.Key_W)
                or (ctrl_shift and e.key() == Qt.Key.Key_W)
                or (ctrl and e.key() == Qt.Key.Key_S)
                or (ctrl_shift and e.key() == Qt.Key.Key_S)
                or (ctrl and e.key() == Qt.Key.Key_T)
                or (ctrl and e.key() == Qt.Key.Key_Tab)
            ):
                e.accept()
                return True

        return super().event(e)

    def keyPressEvent(self, e):
        ctrl = e.modifiers() == Qt.KeyboardModifier.ControlModifier
        ctrl_shift = e.modifiers() == (
            Qt.KeyboardModifier.ControlModifier | Qt.KeyboardModifier.ShiftModifier
        )

        # Ctrl+W: close current tab
        if ctrl and e.key() == Qt.Key.Key_W:
            self.editor_tab.close()

        # Ctrl+Shift+W: close all tabs
        if ctrl_shift and e.key() == Qt.Key.Key_W:
            self.tab_widget.closeAll()

        # Ctrl+S: save current tab
        if ctrl and e.key() == Qt.Key.Key_S:
            self.save()

        # Ctrl+Shift+S: save current tab as
        if ctrl_shift and e.key() == Qt.Key.Key_S:
            self.tab_widget.saveAs()

        # Ctrl+T: open new tab
        if ctrl and e.key() == Qt.Key.Key_T:
            self.tab_widget.newTabEditor()

        super().keyPressEvent(e)

    def showMessage(
        self, text: str, title: str | None = None, level=Qgis.MessageLevel.Info
    ):
        self.editor_tab.showMessage(text, level, title=title)


class EditorTab(QWidget):

    search_bar_toggled = pyqtSignal(bool)

    def __init__(
        self,
        tab_widget: EditorTabWidget,
        console_widget: PythonConsoleWidget,
        filename: str | None,
        read_only: bool,
    ):
        super().__init__(tab_widget)
        self.tab_widget: EditorTabWidget = tab_widget

        self._editor = Editor(
            editor_tab=self, console_widget=console_widget, tab_widget=tab_widget
        )

        self._editor_code_widget = QgsCodeEditorWidget(self._editor)
        self._editor.set_code_editor_widget(self._editor_code_widget)
        self._editor_code_widget.searchBarToggled.connect(self.search_bar_toggled)

        self._editor.trigger_find.connect(self._editor_code_widget.triggerFind)

        if filename:
            if QFileInfo(filename).exists():
                self._editor_code_widget.loadFile(filename)
                if read_only:
                    self._editor.setReadOnly(True)

        self.tabLayout = QGridLayout(self)
        self.tabLayout.setContentsMargins(0, 0, 0, 0)
        self.tabLayout.addWidget(self._editor_code_widget)

    def set_file_path(self, path: str):
        self._editor_code_widget.setFilePath(path)

    def file_path(self) -> str | None:
        return self._editor_code_widget.filePath()

    def open_in_external_editor(self):
        self._editor_code_widget.openInExternalEditor()

    def modified(self, modified):
        self.tab_widget.tabModified(self, modified)

    def search_bar_visible(self) -> bool:
        """
        Returns True if the tab's search bar is visible
        """
        return self._editor_code_widget.isSearchBarVisible()

    def trigger_find(self):
        """
        Triggers a find operation using the default behavior
        """
        self._editor_code_widget.triggerFind()

    def hide_search_bar(self):
        """
        Hides the search bar
        """
        self._editor_code_widget.hideSearchBar()

    def close(self):
        self.tab_widget._removeTab(self, tab2index=True)

    def __getattr__(self, name):
        """Forward all missing attribute requests to the editor."""
        try:
            return super().__getattr__(name)
        except AttributeError:
            return getattr(self._editor, name)

    def __setattr__(self, name, value):
        """Forward all missing attribute requests to the editor."""
        try:
            return super().__setattr__(name, value)
        except AttributeError:
            return setattr(self._editor, name, value)

    def showMessage(self, text, level=Qgis.MessageLevel.Info, timeout=-1, title=""):
        self._editor_code_widget.messageBar().pushMessage(title, text, level, timeout)


class EditorTabWidget(QTabWidget):

    search_bar_toggled = pyqtSignal(bool)

    def __init__(self, console_widget: PythonConsoleWidget):
        super().__init__(parent=None)
        self.console_widget: PythonConsoleWidget = console_widget

        self.idx = -1
        # Layout for top frame (restore tabs)
        self.layoutTopFrame = QGridLayout(self)
        self.layoutTopFrame.setContentsMargins(0, 0, 0, 0)
        spacerItem = QSpacerItem(
            20, 40, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding
        )
        self.layoutTopFrame.addItem(spacerItem, 1, 0, 1, 1)
        self.topFrame = QFrame(self)
        self.topFrame.setStyleSheet("background-color: rgb(255, 255, 230);")
        self.topFrame.setFrameShape(QFrame.Shape.StyledPanel)
        self.topFrame.setMinimumHeight(24)
        self.layoutTopFrame2 = QGridLayout(self.topFrame)
        self.layoutTopFrame2.setContentsMargins(0, 0, 0, 0)
        label = QCoreApplication.translate(
            "PythonConsole", "Click on button to restore all tabs from last session."
        )
        self.label = QLabel(label)

        self.restoreTabsButton = QToolButton()
        toolTipRestore = QCoreApplication.translate("PythonConsole", "Restore tabs")
        self.restoreTabsButton.setToolTip(toolTipRestore)
        self.restoreTabsButton.setIcon(
            QgsApplication.getThemeIcon("console/iconRestoreTabsConsole.svg")
        )
        self.restoreTabsButton.setIconSize(QSize(24, 24))
        self.restoreTabsButton.setAutoRaise(True)
        self.restoreTabsButton.setCursor(Qt.CursorShape.PointingHandCursor)
        self.restoreTabsButton.setStyleSheet(
            "QToolButton:hover{border: none } \
                                              QToolButton:pressed{border: none}"
        )

        self.clButton = QToolButton()
        toolTipClose = QCoreApplication.translate("PythonConsole", "Close")
        self.clButton.setToolTip(toolTipClose)
        self.clButton.setIcon(QgsApplication.getThemeIcon("/mIconClose.svg"))
        self.clButton.setIconSize(QSize(18, 18))
        self.clButton.setCursor(Qt.CursorShape.PointingHandCursor)
        self.clButton.setStyleSheet(
            "QToolButton:hover{border: none } \
                                     QToolButton:pressed{border: none}"
        )
        self.clButton.setAutoRaise(True)

        sizePolicy = QSizePolicy(QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Fixed)
        self.topFrame.setSizePolicy(sizePolicy)
        self.layoutTopFrame.addWidget(self.topFrame, 0, 0, 1, 1)
        self.layoutTopFrame2.addWidget(self.label, 0, 1, 1, 1)
        self.layoutTopFrame2.addWidget(self.restoreTabsButton, 0, 0, 1, 1)
        self.layoutTopFrame2.addWidget(self.clButton, 0, 2, 1, 1)

        self.topFrame.hide()
        self.restoreTabsButton.clicked.connect(self.restoreTabs)
        self.clButton.clicked.connect(self.closeRestore)

        # Fixes #7653
        if sys.platform != "darwin":
            self.setDocumentMode(True)

        self.setMovable(True)
        self.setTabsClosable(True)
        self.setTabPosition(QTabWidget.TabPosition.North)

        # Menu button list tabs
        self.fileTabMenu = QMenu()
        self.fileTabMenu.aboutToShow.connect(self.showFileTabMenu)
        self.fileTabMenu.triggered.connect(self.showFileTabMenuTriggered)
        self.fileTabButton = QToolButton()
        txtToolTipMenuFile = QCoreApplication.translate(
            "PythonConsole", "List all tabs"
        )
        self.fileTabButton.setToolTip(txtToolTipMenuFile)
        self.fileTabButton.setIcon(
            QgsApplication.getThemeIcon("console/iconFileTabsMenuConsole.svg")
        )
        self.fileTabButton.setIconSize(QSize(24, 24))
        self.fileTabButton.setAutoRaise(True)
        self.fileTabButton.setPopupMode(QToolButton.ToolButtonPopupMode.InstantPopup)
        self.fileTabButton.setMenu(self.fileTabMenu)
        self.setCornerWidget(self.fileTabButton, Qt.Corner.TopRightCorner)
        self.tabCloseRequested.connect(self._removeTab)
        self.currentChanged.connect(self._currentWidgetChanged)

        # New Editor button
        self.newTabButton = QToolButton()
        txtToolTipNewTab = QCoreApplication.translate("PythonConsole", "New Editor")
        self.newTabButton.setToolTip(txtToolTipNewTab)
        self.newTabButton.setAutoRaise(True)
        self.newTabButton.setIcon(
            QgsApplication.getThemeIcon("console/iconNewTabEditorConsole.svg")
        )
        self.newTabButton.setIconSize(QSize(24, 24))
        self.setCornerWidget(self.newTabButton, Qt.Corner.TopLeftCorner)
        self.newTabButton.clicked.connect(self.newTabEditor)

    def _currentWidgetChanged(self, tab):
        if QgsSettings().value("pythonConsole/enableObjectInsp", False, type=bool):
            self.listObject(tab)
        self.changeLastDirPath(tab)
        self.enableSaveIfModified(tab)

        if self.currentWidget():
            self.search_bar_toggled.emit(self.currentWidget().search_bar_visible())

    def toggle_search_bar(self, visible: bool):
        """
        Toggles whether the search bar should be visible
        """
        if visible and not self.currentWidget().search_bar_visible():
            self.currentWidget().trigger_find()
        elif not visible and self.currentWidget().search_bar_visible():
            self.currentWidget().hide_search_bar()

    def contextMenuEvent(self, e):
        tabBar = self.tabBar()
        self.idx = tabBar.tabAt(e.pos())
        if self.widget(self.idx):
            cW = self.widget(self.idx)
            menu = QMenu(self)
            menu.addSeparator()
            menu.addAction(
                QCoreApplication.translate("PythonConsole", "New Editor"),
                self.newTabEditor,
            )
            menu.addSeparator()
            closeTabAction = menu.addAction(
                QCoreApplication.translate("PythonConsole", "Close Tab"), cW.close
            )
            closeAllTabAction = menu.addAction(
                QCoreApplication.translate("PythonConsole", "Close All"), self.closeAll
            )
            closeOthersTabAction = menu.addAction(
                QCoreApplication.translate("PythonConsole", "Close Others"),
                self.closeOthers,
            )
            menu.addSeparator()
            saveAction = menu.addAction(
                QCoreApplication.translate("PythonConsole", "Save"), cW.save
            )
            menu.addAction(
                QCoreApplication.translate("PythonConsole", "Save As"), self.saveAs
            )
            closeTabAction.setEnabled(False)
            closeAllTabAction.setEnabled(False)
            closeOthersTabAction.setEnabled(False)
            saveAction.setEnabled(False)
            if self.count() > 1:
                closeTabAction.setEnabled(True)
                closeAllTabAction.setEnabled(True)
                closeOthersTabAction.setEnabled(True)
            if self.widget(self.idx).isModified():
                saveAction.setEnabled(True)
            menu.exec(self.mapToGlobal(e.pos()))

    def closeOthers(self):
        idx = self.idx
        countTab = self.count()
        for i in list(range(countTab - 1, idx, -1)) + list(range(idx - 1, -1, -1)):
            self._removeTab(i)

    def closeAll(self):
        countTab = self.count()
        for i in range(countTab - 1, 0, -1):
            self._removeTab(i)

        self.newTabEditor(
            tabName=QCoreApplication.translate("PythonConsole", "Untitled-0")
        )
        self._removeTab(0)

    def saveAs(self):
        self.console_widget.saveAsScriptFile(self.idx)

    def enableSaveIfModified(self, tab):
        tabWidget = self.widget(tab)
        if tabWidget:
            self.console_widget.saveFileButton.setEnabled(tabWidget.isModified())

    def enableToolBarEditor(self, enable):
        if self.topFrame.isVisible():
            enable = False
        self.console_widget.toolBarEditor.setEnabled(enable)

    def newTabEditor(self, tabName=None, filename: str | None = None):
        read_only = False
        if filename:
            read_only = not QFileInfo(filename).isWritable()
            try:
                fn = codecs.open(filename, "rb", encoding="utf-8")
                fn.read()
                fn.close()
            except OSError as error:
                IOErrorTr = QCoreApplication.translate(
                    "PythonConsole", "The file {0} could not be opened. Error: {1}\n"
                ).format(filename, error.strerror)
                print("## Error: ")
                sys.stderr.write(IOErrorTr)
                return

        nr = self.count()
        if not tabName:
            tabName = QCoreApplication.translate(
                "PythonConsole", "Untitled-{0}"
            ).format(nr)
        tab = EditorTab(
            tab_widget=self,
            console_widget=self.console_widget,
            filename=filename,
            read_only=read_only,
        )
        self.iconTab = QgsApplication.getThemeIcon("console/iconTabEditorConsole.svg")
        self.addTab(tab, self.iconTab, tabName + " (ro)" if read_only else tabName)
        self.setCurrentWidget(tab)
        if filename:
            self.setTabToolTip(self.currentIndex(), filename)
        else:
            self.setTabToolTip(self.currentIndex(), tabName)

        tab.search_bar_toggled.connect(self._tab_search_bar_toggled)

    def _tab_search_bar_toggled(self, visible: bool):
        if self.sender() != self.currentWidget():
            return

        self.search_bar_toggled.emit(visible)

    def tabModified(self, tab, modified):
        index = self.indexOf(tab)
        s = self.tabText(index)
        self.setTabTitle(index, f"*{s}" if modified else re.sub(r"^(\*)", "", s))
        self.console_widget.saveFileButton.setEnabled(modified)

    def setTabTitle(self, tab, title):
        self.setTabText(tab, title)

    def _removeTab(self, tab, tab2index=False):
        if tab2index:
            tab = self.indexOf(tab)
        editorTab = self.widget(tab)
        if editorTab.isModified():
            txtSaveOnRemove = QCoreApplication.translate(
                "PythonConsole", "Python Console: Save File"
            )
            txtMsgSaveOnRemove = QCoreApplication.translate(
                "PythonConsole",
                "The file <b>'{0}'</b> has been modified, save changes?",
            ).format(self.tabText(tab))
            res = QMessageBox.question(
                self,
                txtSaveOnRemove,
                txtMsgSaveOnRemove,
                QMessageBox.StandardButton.Save
                | QMessageBox.StandardButton.Discard
                | QMessageBox.StandardButton.Cancel,
            )
            if res == QMessageBox.StandardButton.Cancel:
                return
            if res == QMessageBox.StandardButton.Save:
                editorTab.save()
            if editorTab.code_editor_widget.filePath():
                self.console_widget.updateTabListScript(
                    editorTab.code_editor_widget.filePath(), action="remove"
                )
            self.removeTab(tab)
            if self.count() < 1:
                self.newTabEditor()
        else:
            if editorTab.code_editor_widget.filePath():
                self.console_widget.updateTabListScript(
                    editorTab.code_editor_widget.filePath(), action="remove"
                )
            if self.count() <= 1:
                self.removeTab(tab)
                self.newTabEditor()
            else:
                self.removeTab(tab)

        editorTab.deleteLater()
        self.currentWidget()._editor.setFocus(Qt.FocusReason.TabFocusReason)

    def restoreTabsOrAddNew(self):
        """
        Restore tabs if they are found in the settings. If none are found it will add a new empty tab.
        """
        # Restore scripts from the previous session
        tabScripts = QgsSettings().value("pythonConsole/tabScripts", [])
        self.restoreTabList = tabScripts

        if self.restoreTabList:
            self.restoreTabs()
        else:
            self.newTabEditor(filename=None)

    def restoreTabs(self):
        for script in self.restoreTabList:
            pathFile = script
            if QFileInfo(pathFile).exists():
                tabName = pathFile.split("/")[-1]
                self.newTabEditor(tabName, pathFile)
            else:
                errOnRestore = QCoreApplication.translate(
                    "PythonConsole", "Unable to restore the file: \n{0}\n"
                ).format(pathFile)
                print("## Error: ")
                s = errOnRestore
                sys.stderr.write(s)
                self.console_widget.updateTabListScript(pathFile, action="remove")
        if self.count() < 1:
            self.newTabEditor(filename=None)
        self.topFrame.close()
        self.enableToolBarEditor(True)
        self.currentWidget()._editor.setFocus(Qt.FocusReason.TabFocusReason)

    def closeRestore(self):
        self.console_widget.updateTabListScript(None)
        self.topFrame.close()
        self.newTabEditor(filename=None)
        self.enableToolBarEditor(True)

    def showFileTabMenu(self):
        self.fileTabMenu.clear()
        for index in range(self.count()):
            action = self.fileTabMenu.addAction(
                self.tabIcon(index), self.tabText(index)
            )
            action.setData(index)

    def showFileTabMenuTriggered(self, action):
        index = action.data()
        if index is not None:
            self.setCurrentIndex(index)

    def listObject(self, tab):
        self.console_widget.listClassMethod.clear()
        if isinstance(tab, EditorTab):
            tabWidget = self.widget(self.indexOf(tab))
        else:
            tabWidget = self.widget(tab)
        if tabWidget:
            if tabWidget.file_path():
                pathFile, file = os.path.split(tabWidget.file_path())
                module, ext = os.path.splitext(file)
                found = False
                if pathFile not in sys.path:
                    sys.path.append(pathFile)
                    found = True
                try:
                    importlib.reload(pyclbr)  # NOQA
                    dictObject = {}
                    readModule = pyclbr.readmodule(module)
                    readModuleFunction = pyclbr.readmodule_ex(module)
                    for name, class_data in sorted(
                        list(readModule.items()), key=lambda x: x[1].lineno
                    ):
                        if os.path.normpath(class_data.file) == os.path.normpath(
                            tabWidget.file_path()
                        ):
                            superClassName = []
                            for superClass in class_data.super:
                                if superClass == "object":
                                    continue
                                if isinstance(superClass, str):
                                    superClassName.append(superClass)
                                else:
                                    superClassName.append(superClass.name)
                            classItem = QTreeWidgetItem()
                            if superClassName:
                                super = ", ".join([i for i in superClassName])
                                classItem.setText(0, name + " [" + super + "]")
                                classItem.setToolTip(0, name + " [" + super + "]")
                            else:
                                classItem.setText(0, name)
                                classItem.setToolTip(0, name)
                            if sys.platform.startswith("win"):
                                classItem.setSizeHint(0, QSize(18, 18))
                            classItem.setText(1, str(class_data.lineno))
                            iconClass = QgsApplication.getThemeIcon(
                                "console/iconClassTreeWidgetConsole.svg"
                            )
                            classItem.setIcon(0, iconClass)
                            dictObject[name] = class_data.lineno
                            for meth, lineno in sorted(
                                list(class_data.methods.items()), key=itemgetter(1)
                            ):
                                methodItem = QTreeWidgetItem()
                                methodItem.setText(0, meth + " ")
                                methodItem.setText(1, str(lineno))
                                methodItem.setToolTip(0, meth)
                                iconMeth = QgsApplication.getThemeIcon(
                                    "console/iconMethodTreeWidgetConsole.svg"
                                )
                                methodItem.setIcon(0, iconMeth)
                                if sys.platform.startswith("win"):
                                    methodItem.setSizeHint(0, QSize(18, 18))
                                classItem.addChild(methodItem)
                                dictObject[meth] = lineno
                            self.console_widget.listClassMethod.addTopLevelItem(
                                classItem
                            )
                    for func_name, data in sorted(
                        list(readModuleFunction.items()), key=lambda x: x[1].lineno
                    ):
                        if isinstance(data, pyclbr.Function) and os.path.normpath(
                            data.file
                        ) == os.path.normpath(tabWidget.file_path()):
                            funcItem = QTreeWidgetItem()
                            funcItem.setText(0, func_name + " ")
                            funcItem.setText(1, str(data.lineno))
                            funcItem.setToolTip(0, func_name)
                            iconFunc = QgsApplication.getThemeIcon(
                                "console/iconFunctionTreeWidgetConsole.svg"
                            )
                            funcItem.setIcon(0, iconFunc)
                            if sys.platform.startswith("win"):
                                funcItem.setSizeHint(0, QSize(18, 18))
                            dictObject[func_name] = data.lineno
                            self.console_widget.listClassMethod.addTopLevelItem(
                                funcItem
                            )
                    if found:
                        sys.path.remove(pathFile)
                except:
                    msgItem = QTreeWidgetItem()
                    msgItem.setText(
                        0, QCoreApplication.translate("PythonConsole", "Check Syntax")
                    )
                    msgItem.setText(1, "syntaxError")
                    iconWarning = QgsApplication.getThemeIcon(
                        "console/iconSyntaxErrorConsole.svg"
                    )
                    msgItem.setIcon(0, iconWarning)
                    self.console_widget.listClassMethod.addTopLevelItem(msgItem)

    def refreshSettingsEditor(self):
        objInspectorEnabled = QgsSettings().value(
            "pythonConsole/enableObjectInsp", False, type=bool
        )
        listObj = self.console_widget.objectListButton
        if self.console_widget.listClassMethod.isVisible():
            listObj.setChecked(objInspectorEnabled)
        listObj.setEnabled(objInspectorEnabled)
        if objInspectorEnabled:
            cW = self.currentWidget()
            if cW and not self.console_widget.listClassMethod.isVisible():
                with OverrideCursor(Qt.CursorShape.WaitCursor):
                    self.listObject(cW)

    def changeLastDirPath(self, tab):
        tabWidget = self.widget(tab)
        if tabWidget and tabWidget.file_path():
            QgsSettings().setValue(
                "pythonConsole/lastDirPath",
                Path(tabWidget.file_path()).parent.as_posix(),
            )

    def showMessage(self, text, level=Qgis.MessageLevel.Info, timeout=-1, title=""):
        currWidget = self.currentWidget()
        currWidget.showMessage(text, level, timeout, title)
