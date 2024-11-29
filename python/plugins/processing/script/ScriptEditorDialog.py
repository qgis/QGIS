"""
***************************************************************************
    EditScriptDialog.py
    ---------------------
    Date                 : December 2012
    Copyright            : (C) 2012 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Alexander Bruy"
__date__ = "December 2012"
__copyright__ = "(C) 2012, Alexander Bruy"

import os
import codecs
import inspect
import traceback
import warnings

from qgis.PyQt import uic, sip
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import QMessageBox, QFileDialog, QVBoxLayout

from qgis.gui import QgsGui, QgsErrorDialog, QgsCodeEditorWidget
from qgis.core import (
    QgsApplication,
    QgsFileUtils,
    QgsSettings,
    QgsError,
    QgsProcessingAlgorithm,
    QgsProcessingFeatureBasedAlgorithm,
)
from qgis.utils import iface, OverrideCursor
from qgis.processing import alg as algfactory

from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.script import ScriptUtils

from .ScriptEdit import ScriptEdit

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(os.path.join(pluginPath, "ui", "DlgScriptEditor.ui"))


class ScriptEditorDialog(BASE, WIDGET):
    hasChanged = False

    DIALOG_STORE = []

    def __init__(self, filePath=None, parent=None):
        super().__init__(parent)
        # SIP is totally messed up here -- the dialog wrapper or something
        # is always prematurely cleaned which results in broken QObject
        # connections throughout.
        # Hack around this by storing dialog instances in a global list to
        # prevent too early wrapper garbage collection
        ScriptEditorDialog.DIALOG_STORE.append(self)

        def clean_up_store():
            ScriptEditorDialog.DIALOG_STORE = [
                d for d in ScriptEditorDialog.DIALOG_STORE if d != self
            ]

        self.destroyed.connect(clean_up_store)

        self.setupUi(self)
        self.setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose)

        QgsGui.instance().enableAutoGeometryRestore(self)

        vl = QVBoxLayout()
        vl.setContentsMargins(0, 0, 0, 0)
        self.editor_container.setLayout(vl)

        self.editor = ScriptEdit()
        self.code_editor_widget = QgsCodeEditorWidget(self.editor)
        vl.addWidget(self.code_editor_widget)

        if iface is not None:
            self.toolBar.setIconSize(iface.iconSize())
            self.setStyleSheet(iface.mainWindow().styleSheet())

        self.actionOpenScript.setIcon(
            QgsApplication.getThemeIcon("/mActionScriptOpen.svg")
        )
        self.actionSaveScript.setIcon(
            QgsApplication.getThemeIcon("/mActionFileSave.svg")
        )
        self.actionSaveScriptAs.setIcon(
            QgsApplication.getThemeIcon("/mActionFileSaveAs.svg")
        )
        self.actionRunScript.setIcon(QgsApplication.getThemeIcon("/mActionStart.svg"))
        self.actionCut.setIcon(QgsApplication.getThemeIcon("/mActionEditCut.svg"))
        self.actionCopy.setIcon(QgsApplication.getThemeIcon("/mActionEditCopy.svg"))
        self.actionPaste.setIcon(QgsApplication.getThemeIcon("/mActionEditPaste.svg"))
        self.actionUndo.setIcon(QgsApplication.getThemeIcon("/mActionUndo.svg"))
        self.actionRedo.setIcon(QgsApplication.getThemeIcon("/mActionRedo.svg"))
        self.actionFindReplace.setIcon(
            QgsApplication.getThemeIcon("/mActionFindReplace.svg")
        )
        self.actionIncreaseFontSize.setIcon(
            QgsApplication.getThemeIcon("/mActionIncreaseFont.svg")
        )
        self.actionDecreaseFontSize.setIcon(
            QgsApplication.getThemeIcon("/mActionDecreaseFont.svg")
        )
        self.actionToggleComment.setIcon(
            QgsApplication.getThemeIcon("console/iconCommentEditorConsole.svg")
        )

        # Connect signals and slots
        self.actionOpenScript.triggered.connect(self.openScript)
        self.actionSaveScript.triggered.connect(self.save)
        self.actionSaveScriptAs.triggered.connect(self.saveAs)
        self.actionRunScript.triggered.connect(self.runAlgorithm)
        self.actionCut.triggered.connect(self.editor.cut)
        self.actionCopy.triggered.connect(self.editor.copy)
        self.actionPaste.triggered.connect(self.editor.paste)
        self.actionUndo.triggered.connect(self.editor.undo)
        self.actionRedo.triggered.connect(self.editor.redo)
        self.actionFindReplace.toggled.connect(
            self.code_editor_widget.setSearchBarVisible
        )
        self.code_editor_widget.searchBarToggled.connect(
            self.actionFindReplace.setChecked
        )

        self.actionIncreaseFontSize.triggered.connect(self.editor.zoomIn)
        self.actionDecreaseFontSize.triggered.connect(self.editor.zoomOut)
        self.actionToggleComment.triggered.connect(self.editor.toggleComment)
        self.editor.modificationChanged.connect(self._on_text_modified)

        self.run_dialog = None

        if filePath is not None:
            self._loadFile(filePath)

        self.setHasChanged(False)

    def update_dialog_title(self):
        """
        Updates the script editor dialog title
        """
        if self.code_editor_widget.filePath():
            path, file_name = os.path.split(self.code_editor_widget.filePath())
        else:
            file_name = self.tr("Untitled Script")

        if self.hasChanged:
            file_name = "*" + file_name

        self.setWindowTitle(self.tr("{} - Processing Script Editor").format(file_name))

    def closeEvent(self, event):
        settings = QgsSettings()
        settings.setValue("/Processing/stateScriptEditor", self.saveState())
        settings.setValue("/Processing/geometryScriptEditor", self.saveGeometry())

        if self.hasChanged:
            ret = QMessageBox.question(
                self,
                self.tr("Save Script?"),
                self.tr(
                    "There are unsaved changes in this script. Do you want to keep those?"
                ),
                QMessageBox.StandardButton.Save
                | QMessageBox.StandardButton.Cancel
                | QMessageBox.StandardButton.Discard,
                QMessageBox.StandardButton.Cancel,
            )

            if ret == QMessageBox.StandardButton.Save:
                self.saveScript(False)
                event.accept()
            elif ret == QMessageBox.StandardButton.Discard:
                event.accept()
            else:
                event.ignore()
        else:
            event.accept()

    def openScript(self):
        if self.hasChanged:
            ret = QMessageBox.warning(
                self,
                self.tr("Unsaved changes"),
                self.tr("There are unsaved changes in the script. Continue?"),
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                QMessageBox.StandardButton.No,
            )
            if ret == QMessageBox.StandardButton.No:
                return

        scriptDir = ScriptUtils.scriptsFolders()[0]
        fileName, _ = QFileDialog.getOpenFileName(
            self,
            self.tr("Open script"),
            scriptDir,
            self.tr("Processing scripts (*.py *.PY)"),
        )

        if fileName == "":
            return

        with OverrideCursor(Qt.CursorShape.WaitCursor):
            self._loadFile(fileName)

    def save(self):
        self.saveScript(False)

    def saveAs(self):
        self.saveScript(True)

    def saveScript(self, saveAs):
        newPath = None
        if not self.code_editor_widget.filePath() or saveAs:
            scriptDir = ScriptUtils.scriptsFolders()[0]
            newPath, _ = QFileDialog.getSaveFileName(
                self,
                self.tr("Save script"),
                scriptDir,
                self.tr("Processing scripts (*.py *.PY)"),
            )

            if newPath:
                newPath = QgsFileUtils.ensureFileNameHasExtension(newPath, ["py"])
                self.code_editor_widget.save(newPath)
        elif self.code_editor_widget.filePath():
            self.code_editor_widget.save()

        self.setHasChanged(False)
        QgsApplication.processingRegistry().providerById("script").refreshAlgorithms()

    def _on_text_modified(self, modified):
        self.setHasChanged(modified)

    def setHasChanged(self, hasChanged):
        self.hasChanged = hasChanged
        self.actionSaveScript.setEnabled(hasChanged)
        self.update_dialog_title()

    def runAlgorithm(self):
        if self.run_dialog and not sip.isdeleted(self.run_dialog):
            self.run_dialog.close()
            self.run_dialog = None

        _locals = {}
        try:
            exec(self.editor.text(), _locals)
        except Exception as e:
            error = QgsError(traceback.format_exc(), "Processing")
            QgsErrorDialog.show(error, self.tr("Execution error"))
            return

        alg = None
        try:
            alg = algfactory.instances.pop().createInstance()
        except IndexError:
            for name, attr in _locals.items():
                if (
                    inspect.isclass(attr)
                    and issubclass(
                        attr,
                        (QgsProcessingAlgorithm, QgsProcessingFeatureBasedAlgorithm),
                    )
                    and attr.__name__
                    not in (
                        "QgsProcessingAlgorithm",
                        "QgsProcessingFeatureBasedAlgorithm",
                    )
                ):
                    alg = attr()
                    break

        if alg is None:
            QMessageBox.warning(
                self,
                self.tr("No script found"),
                self.tr("Seems there is no valid script in the file."),
            )
            return

        alg.setProvider(QgsApplication.processingRegistry().providerById("script"))
        alg.initAlgorithm()

        self.run_dialog = alg.createCustomParametersWidget(self)
        if not self.run_dialog:
            self.run_dialog = AlgorithmDialog(alg, parent=self)

        canvas = iface.mapCanvas()
        prevMapTool = canvas.mapTool()

        self.run_dialog.show()

        if canvas.mapTool() != prevMapTool:
            try:
                canvas.mapTool().reset()
            except:
                pass
            canvas.setMapTool(prevMapTool)

    def _loadFile(self, filePath):

        self.code_editor_widget.loadFile(filePath)
        self.hasChanged = False

        self.update_dialog_title()
