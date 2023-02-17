# -*- coding: utf-8 -*-

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

__author__ = 'Alexander Bruy'
__date__ = 'December 2012'
__copyright__ = '(C) 2012, Alexander Bruy'

import os
import codecs
import inspect
import traceback
import warnings

from qgis.PyQt import uic, sip
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QCursor
from qgis.PyQt.QtWidgets import (QMessageBox,
                                 QFileDialog)

from qgis.gui import QgsGui, QgsErrorDialog
from qgis.core import (QgsApplication,
                       QgsSettings,
                       QgsError,
                       QgsProcessingAlgorithm,
                       QgsProcessingFeatureBasedAlgorithm)
from qgis.utils import iface, OverrideCursor
from qgis.processing import alg as algfactory

from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.script import ScriptUtils

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, "ui", "DlgScriptEditor.ui"))


class ScriptEditorDialog(BASE, WIDGET):
    hasChanged = False

    def __init__(self, filePath=None, parent=None):
        super(ScriptEditorDialog, self).__init__(parent)
        self.setupUi(self)

        QgsGui.instance().enableAutoGeometryRestore(self)

        self.searchWidget.setVisible(False)

        if iface is not None:
            self.toolBar.setIconSize(iface.iconSize())
            self.setStyleSheet(iface.mainWindow().styleSheet())

        self.actionOpenScript.setIcon(
            QgsApplication.getThemeIcon('/mActionScriptOpen.svg'))
        self.actionSaveScript.setIcon(
            QgsApplication.getThemeIcon('/mActionFileSave.svg'))
        self.actionSaveScriptAs.setIcon(
            QgsApplication.getThemeIcon('/mActionFileSaveAs.svg'))
        self.actionRunScript.setIcon(
            QgsApplication.getThemeIcon('/mActionStart.svg'))
        self.actionCut.setIcon(
            QgsApplication.getThemeIcon('/mActionEditCut.svg'))
        self.actionCopy.setIcon(
            QgsApplication.getThemeIcon('/mActionEditCopy.svg'))
        self.actionPaste.setIcon(
            QgsApplication.getThemeIcon('/mActionEditPaste.svg'))
        self.actionUndo.setIcon(
            QgsApplication.getThemeIcon('/mActionUndo.svg'))
        self.actionRedo.setIcon(
            QgsApplication.getThemeIcon('/mActionRedo.svg'))
        self.actionFindReplace.setIcon(
            QgsApplication.getThemeIcon('/mActionFindReplace.svg'))
        self.actionIncreaseFontSize.setIcon(
            QgsApplication.getThemeIcon('/mActionIncreaseFont.svg'))
        self.actionDecreaseFontSize.setIcon(
            QgsApplication.getThemeIcon('/mActionDecreaseFont.svg'))
        self.actionToggleComment.setIcon(
            QgsApplication.getThemeIcon('console/iconCommentEditorConsole.svg'))

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
        self.actionFindReplace.toggled.connect(self.toggleSearchBox)
        self.actionIncreaseFontSize.triggered.connect(self.editor.zoomIn)
        self.actionDecreaseFontSize.triggered.connect(self.editor.zoomOut)
        self.actionToggleComment.triggered.connect(self.editor.toggleComment)
        self.editor.textChanged.connect(lambda: self.setHasChanged(True))

        self.leFindText.returnPressed.connect(self.find)
        self.btnFind.clicked.connect(self.find)
        self.btnReplace.clicked.connect(self.replace)
        self.lastSearch = None
        self.run_dialog = None

        self.filePath = None
        if filePath is not None:
            self._loadFile(filePath)

        self.setHasChanged(False)

    def update_dialog_title(self):
        """
        Updates the script editor dialog title
        """
        if self.filePath:
            path, file_name = os.path.split(self.filePath)
        else:
            file_name = self.tr('Untitled Script')

        if self.hasChanged:
            file_name = '*' + file_name

        self.setWindowTitle(self.tr('{} - Processing Script Editor').format(file_name))

    def closeEvent(self, event):
        settings = QgsSettings()
        settings.setValue("/Processing/stateScriptEditor", self.saveState())
        settings.setValue("/Processing/geometryScriptEditor", self.saveGeometry())

        if self.hasChanged:
            ret = QMessageBox.question(
                self, self.tr('Save Script?'),
                self.tr('There are unsaved changes in this script. Do you want to keep those?'),
                QMessageBox.Save | QMessageBox.Cancel | QMessageBox.Discard, QMessageBox.Cancel)

            if ret == QMessageBox.Save:
                self.saveScript(False)
                event.accept()
            elif ret == QMessageBox.Discard:
                event.accept()
            else:
                event.ignore()
        else:
            event.accept()

    def openScript(self):
        if self.hasChanged:
            ret = QMessageBox.warning(self,
                                      self.tr("Unsaved changes"),
                                      self.tr("There are unsaved changes in the script. Continue?"),
                                      QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            if ret == QMessageBox.No:
                return

        scriptDir = ScriptUtils.scriptsFolders()[0]
        fileName, _ = QFileDialog.getOpenFileName(self,
                                                  self.tr("Open script"),
                                                  scriptDir,
                                                  self.tr("Processing scripts (*.py *.PY)"))

        if fileName == "":
            return

        with OverrideCursor(Qt.WaitCursor):
            self._loadFile(fileName)

    def save(self):
        self.saveScript(False)

    def saveAs(self):
        self.saveScript(True)

    def saveScript(self, saveAs):
        newPath = None
        if self.filePath is None or saveAs:
            scriptDir = ScriptUtils.scriptsFolders()[0]
            newPath, _ = QFileDialog.getSaveFileName(self,
                                                     self.tr("Save script"),
                                                     scriptDir,
                                                     self.tr("Processing scripts (*.py *.PY)"))

            if newPath:
                if not newPath.lower().endswith(".py"):
                    newPath += ".py"

                self.filePath = newPath

        if self.filePath:
            text = self.editor.text()
            try:
                with codecs.open(self.filePath, "w", encoding="utf-8") as f:
                    f.write(text)
            except IOError as e:
                QMessageBox.warning(self,
                                    self.tr("I/O error"),
                                    self.tr("Unable to save edits:\n{}").format(str(e))
                                    )
                return

            self.setHasChanged(False)

        QgsApplication.processingRegistry().providerById("script").refreshAlgorithms()

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
            QgsErrorDialog.show(error,
                                self.tr("Execution error")
                                )
            return

        alg = None
        try:
            alg = algfactory.instances.pop().createInstance()
        except IndexError:
            for name, attr in _locals.items():
                if inspect.isclass(attr) and issubclass(attr, (QgsProcessingAlgorithm, QgsProcessingFeatureBasedAlgorithm)) and attr.__name__ not in ("QgsProcessingAlgorithm", "QgsProcessingFeatureBasedAlgorithm"):
                    alg = attr()
                    break

        if alg is None:
            QMessageBox.warning(self,
                                self.tr("No script found"),
                                self.tr("Seems there is no valid script in the file.")
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

    def find(self):
        textToFind = self.leFindText.text()
        caseSensitive = self.chkCaseSensitive.isChecked()
        wholeWord = self.chkWholeWord.isChecked()
        if self.lastSearch is None or textToFind != self.lastSearch:
            self.editor.findFirst(textToFind, False, caseSensitive, wholeWord, True)
        else:
            self.editor.findNext()

    def replace(self):
        textToReplace = self.leReplaceText.text()
        self.editor.replaceSelectedText(textToReplace)

    def toggleSearchBox(self, checked):
        self.searchWidget.setVisible(checked)
        if (checked):
            self.leFindText.setFocus()

    def _loadFile(self, filePath):
        with codecs.open(filePath, "r", encoding="utf-8") as f:
            txt = f.read()

        self.editor.setText(txt)
        self.hasChanged = False
        self.editor.setModified(False)
        self.editor.recolor()

        self.filePath = filePath
        self.update_dialog_title()
