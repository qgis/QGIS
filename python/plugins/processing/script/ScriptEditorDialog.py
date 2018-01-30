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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import codecs

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QCursor
from qgis.PyQt.QtWidgets import (QMessageBox,
                                 QFileDialog)

from qgis.gui import QgsGui
from qgis.core import QgsApplication, QgsSettings
from qgis.utils import iface, OverrideCursor

from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.script import ScriptUtils

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, "ui", "DlgScriptEditor.ui"))


class ScriptEditorDialog(BASE, WIDGET):
    hasChanged = False

    def __init__(self, filePath=None, parent=None):
        super(ScriptEditorDialog, self).__init__(parent)
        self.setupUi(self)

        QgsGui.instance().enableAutoGeometryRestore(self)

        #~ self.setWindowFlags(Qt.WindowMinimizeButtonHint |
                            #~ Qt.WindowMaximizeButtonHint |
                            #~ Qt.WindowCloseButtonHint)

        self.searchWidget.setVisible(False)

        self.toolBar.setIconSize(iface.iconSize())

        self.actionOpenScript.setIcon(
            QgsApplication.getThemeIcon('/mActionFileOpen.svg'))
        self.actionSaveScript.setIcon(
            QgsApplication.getThemeIcon('/mActionFileSave.svg'))
        self.actionSaveScriptAs.setIcon(
            QgsApplication.getThemeIcon('/mActionFileSaveAs.svg'))
        self.actionEditScriptHelp.setIcon(
            QgsApplication.getThemeIcon('/mActionEditHelpContent.svg'))
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

        # Connect signals and slots
        self.actionOpenScript.triggered.connect(self.openScript)
        self.actionSaveScript.triggered.connect(self.save)
        self.actionSaveScriptAs.triggered.connect(self.saveAs)
        #self.actionEditScriptHelp.triggered.connect(self.editHelp)
        self.actionRunScript.triggered.connect(self.runAlgorithm)
        self.actionCut.triggered.connect(self.editor.cut)
        self.actionCopy.triggered.connect(self.editor.copy)
        self.actionPaste.triggered.connect(self.editor.paste)
        self.actionUndo.triggered.connect(self.editor.undo)
        self.actionRedo.triggered.connect(self.editor.redo)
        self.actionFindReplace.toggled.connect(self.toggleSearchBox)
        self.actionIncreaseFontSize.triggered.connect(self.editor.zoomIn)
        self.actionDecreaseFontSize.triggered.connect(self.editor.zoomOut)
        self.editor.textChanged.connect(lambda: self.setHasChanged(True))

        self.btnFind.clicked.connect(self.find)
        self.btnReplace.clicked.connect(self.replace)
        self.lastSearch = None

        self.filePath = filePath
        if self.filePath is not None:
            self._loadFile(self.filePath)

        self.needUpdate = False
        self.setHasChanged(False)

        #self.snippets = {}
        #path = os.path.join(os.path.dirname(os.path.dirname(__file__)), "script", "snippets.py")
        #with codecs.open(path, "r", encoding="utf-8") as f:
        #    lines = f.readlines()
        #snippetlines = []
        #name = None
        #for line in lines:
        #    if line.startswith("##"):
        #        if snippetlines:
        #            self.snippets[name] = "".join(snippetlines)
        #        name = line[2:]
        #        snippetlines = []
        #    else:
        #        snippetlines.append(line)
        #if snippetlines:
        #    self.snippets[name] = "".join(snippetlines)
        #if self.snippets:
        #    self.btnSnippets.setVisible(False)

        #if self.alg is not None:
        #    pass
        #    self.filename = self.alg.descriptionFile
        #    self.editor.setText(self.alg.script)
        #else:
        #    self.filename = None

    #def showSnippets(self, evt):
    #    popupmenu = QMenu()
    #    for name, snippet in list(self.snippets.items()):
    #        action = QAction(self.tr(name), self.btnSnippets)
    #        action.triggered[()].connect(lambda snippet=snippet: self.editor.insert(snippet))
    #    popupmenu.addAction(action)
    #    popupmenu.exec_(QCursor.pos())

    def closeEvent(self, event):
        if self.hasChanged:
            ret = QMessageBox.question(self,
                                       self.tr("Unsaved changes"),
                                       self.tr("There are unsaved changes in the script. Continue?"),
                                       QMessageBox.Yes | QMessageBox.No,
                                       QMessageBox.No
                                       )
            if ret == QMessageBox.Yes:
                self.updateProvider()
                event.accept()
            else:
                event.ignore()
        else:
            self.updateProvider()
            event.accept()

    def updateProvider(self):
        if self.needUpdate:
            QgsApplication.processingRegistry().providerById("script").refreshAlgorithms()

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
                                                  self.tr("Script files (*.py *.PY)"))

        if fileName == "":
            return

        with OverrideCursor(Qt.WaitCursor):
            self._loadFile(fileName)
            self.filePath = fileName

    def save(self):
        self.saveScript(False)

    def saveAs(self):
        self.saveScript(True)

    def saveScript(self, saveAs):
        if self.filePath is None or saveAs:
            scriptDir = ScriptUtils.scriptsFolders()[0]
            newPath, _ = QFileDialog.getSaveFileName(self,
                                                     self.tr("Save script"),
                                                     scriptDir,
                                                     self.tr("Script files (*.py *.PY)"))

        if newPath:
            if not newPath.lower().endswith(".py"):
                newPath += ".py"

            self.filePath = newPath

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
            self.needUpdate = True
            self.setHasChanged(False)
        #else:
        #    self.filePath = None

    def setHasChanged(self, hasChanged):
        self.hasChanged = hasChanged
        self.actionSaveScript.setEnabled(hasChanged)

    def runAlgorithm(self):
        #~ if self.filePath is None or self.hasChanged:
            #~ QMessageBox.warning(self,
                                #~ self.tr("Unsaved changes"),
                                #~ self.tr("There are unsaved changes in script. "
                                        #~ "Please save it and try again.")
                                #~ )
            #~ return

        #~ algName = os.path.splitext(os.path.basename(self.filePath))[0]
        #~ alg = ScriptUtils.loadAlgorithm(algName, self.filePath)
        #~ alg.setProvider(QgsApplication.processingRegistry().providerById("script"))
        #~ print("ALG", alg)

        d = {}
        #print(globals())
        #print(locals())
        exec(self.editor.text(), d)
        #print(d)
        #print(d.keys())
        #print(d["SpatialIndex"])
        alg = d["SpatialIndex"]()
        alg.setProvider(QgsApplication.processingRegistry().providerById("script"))

        dlg = alg.createCustomParametersWidget(self)
        if not dlg:
            dlg = AlgorithmDialog(alg)

        canvas = iface.mapCanvas()
        prevMapTool = canvas.mapTool()

        dlg.show()

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

    def _loadFile(self, filePath):
        with codecs.open(filePath, "r", encoding="utf-8") as f:
            txt = f.read()

        self.editor.setText(txt)
        self.hasChanged = False
        self.editor.setModified(False)
        self.editor.recolor()
