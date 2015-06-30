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

import codecs
import sys
import json
import os

from PyQt4 import uic
from PyQt4.QtCore import Qt
from PyQt4.QtGui import QIcon, QMenu, QAction, QCursor, QMessageBox, QFileDialog, QApplication

from qgis.core import QgsApplication
from qgis.utils import iface

from processing.modeler.ModelerUtils import ModelerUtils
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.gui.HelpEditionDialog import HelpEditionDialog
from processing.algs.r.RAlgorithm import RAlgorithm
from processing.algs.r.RUtils import RUtils
from processing.script.ScriptAlgorithm import ScriptAlgorithm
from processing.script.ScriptUtils import ScriptUtils

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgScriptEditor.ui'))


class ScriptEditorDialog(BASE, WIDGET):

    SCRIPT_PYTHON = 0
    SCRIPT_R = 1

    hasChanged = False

    def __init__(self, algType, alg):
        super(ScriptEditorDialog, self).__init__(None)
        self.setupUi(self)

        self.setWindowFlags(Qt.WindowMinimizeButtonHint |
                            Qt.WindowMaximizeButtonHint |
                            Qt.WindowCloseButtonHint)
        # Set icons
        self.btnOpen.setIcon(
            QgsApplication.getThemeIcon('/mActionFileOpen.svg'))
        self.btnSave.setIcon(
            QgsApplication.getThemeIcon('/mActionFileSave.svg'))
        self.btnSaveAs.setIcon(
            QgsApplication.getThemeIcon('/mActionFileSaveAs.svg'))
        self.btnEditHelp.setIcon(
            QIcon(os.path.join(pluginPath, 'images', 'edithelp.png')))
        self.btnRun.setIcon(
            QIcon(os.path.join(pluginPath, 'images', 'runalgorithm.png')))
        self.btnCut.setIcon(QgsApplication.getThemeIcon('/mActionEditCut.png'))
        self.btnCopy.setIcon(
            QgsApplication.getThemeIcon('/mActionEditCopy.png'))
        self.btnPaste.setIcon(
            QgsApplication.getThemeIcon('/mActionEditPaste.png'))
        self.btnUndo.setIcon(QgsApplication.getThemeIcon('/mActionUndo.png'))
        self.btnRedo.setIcon(QgsApplication.getThemeIcon('/mActionRedo.png'))
        self.btnSnippets.setIcon(QgsApplication.getThemeIcon('/mActionHelpAPI.png'))

        # Connect signals and slots
        self.btnOpen.clicked.connect(self.openScript)
        self.btnSave.clicked.connect(self.save)
        self.btnSaveAs.clicked.connect(self.saveAs)
        self.btnEditHelp.clicked.connect(self.editHelp)
        self.btnRun.clicked.connect(self.runAlgorithm)
        self.btnSnippets.clicked.connect(self.showSnippets)
        self.btnCut.clicked.connect(self.editor.cut)
        self.btnCopy.clicked.connect(self.editor.copy)
        self.btnPaste.clicked.connect(self.editor.paste)
        self.btnUndo.clicked.connect(self.editor.undo)
        self.btnRedo.clicked.connect(self.editor.redo)
        self.btnIncreaseFont.clicked.connect(self.increaseFontSize)
        self.btnDecreaseFont.clicked.connect(self.decreaseFontSize)
        self.editor.textChanged.connect(lambda: self.setHasChanged(True))

        self.alg = alg
        self.algType = algType

        self.snippets = {}
        if self.algType == self.SCRIPT_PYTHON:
            path = os.path.join(os.path.dirname(os.path.dirname(__file__)), "script", "snippets.py")
            with open(path) as f:
                lines = f.readlines()
            snippetlines = []
            name = None
            for line in lines:
                if line.startswith("##"):
                    if snippetlines:
                        self.snippets[name] = "".join(snippetlines)
                    name = line[2:]
                    snippetlines = []
                else:
                    snippetlines.append(line)
            if snippetlines:
                self.snippets[name] = "".join(snippetlines)

        if not self.snippets:
            self.btnSnippets.setVisible(False)

        if self.alg is not None:
            self.filename = self.alg.descriptionFile
            self.editor.setText(self.alg.script)
        else:
            self.filename = None

        self.update = False
        self.help = None

        self.setHasChanged(False)

        self.editor.setLexerType(self.algType)

    def increaseFontSize(self):
        font = self.editor.defaultFont
        self.editor.setFonts(font.pointSize() + 1)
        self.editor.initLexer()

    def decreaseFontSize(self):
        font = self.editor.defaultFont
        self.editor.setFonts(font.pointSize() - 1)
        self.editor.initLexer()

    def showSnippets(self, evt):
        popupmenu = QMenu()
        for name, snippet in self.snippets.iteritems():
            action = QAction(self.tr(name), self.btnSnippets)
            action.triggered[()].connect(lambda snippet=snippet: self.editor.insert(snippet))
            popupmenu.addAction(action)
        popupmenu.exec_(QCursor.pos())

    def closeEvent(self, evt):
        if self.hasChanged:
            ret = QMessageBox.question(self, self.tr('Unsaved changes'),
                self.tr('There are unsaved changes in script. Continue?'),
                QMessageBox.Yes | QMessageBox.No, QMessageBox.No
            )
            if ret == QMessageBox.Yes:
                evt.accept()
            else:
                evt.ignore()
        else:
            evt.accept()

    def editHelp(self):
        if self.alg is None:
            if self.algType == self.SCRIPT_PYTHON:
                alg = ScriptAlgorithm(None, unicode(self.editor.text()))
            elif self.algType == self.SCRIPT_R:
                alg = RAlgorithm(None, unicode(self.editor.text()))
        else:
            alg = self.alg

        dlg = HelpEditionDialog(alg)
        dlg.exec_()
        if dlg.descriptions:
            self.help = dlg.descriptions
            self.setHasChanged(True)

    def openScript(self):
        if self.hasChanged:
            ret = QMessageBox.warning(self, self.tr('Unsaved changes'),
                self.tr('There are unsaved changes in script. Continue?'),
                QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            if ret == QMessageBox.No:
                return

        if self.algType == self.SCRIPT_PYTHON:
            scriptDir = ScriptUtils.scriptsFolder()
            filterName = self.tr('Python scripts (*.py)')
        elif self.algType == self.SCRIPT_R:
            scriptDir = RUtils.RScriptsFolder()
            filterName = self.tr('Processing R script (*.rsx)')

        self.filename = QFileDialog.getOpenFileName(
            self, self.tr('Save script'), scriptDir, filterName)

        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
        with codecs.open(self.filename, 'r', encoding='utf-8') as f:
            txt = f.read()

        self.editor.setText(txt)
        self.hasChanged = False
        self.editor.setModified(False)
        self.editor.recolor()
        QApplication.restoreOverrideCursor()

    def save(self):
        self.saveScript(False)

    def saveAs(self):
        self.saveScript(True)

    def saveScript(self, saveAs):
        if self.filename is None or saveAs:
            if self.algType == self.SCRIPT_PYTHON:
                scriptDir = ScriptUtils.scriptsFolder()
                filterName = self.tr('Python scripts (*.py)')
            elif self.algType == self.SCRIPT_R:
                scriptDir = RUtils.RScriptsFolder()
                filterName = self.tr('Processing R script (*.rsx)')

            self.filename = unicode(QFileDialog.getSaveFileName(self,
                                    self.tr('Save script'), scriptDir,
                                    filterName))

        if self.filename:
            if self.algType == self.SCRIPT_PYTHON and \
                    not self.filename.lower().endswith('.py'):
                self.filename += '.py'
            if self.algType == self.SCRIPT_R and \
                    not self.filename.lower().endswith('.rsx'):
                self.filename += '.rsx'

            text = unicode(self.editor.text())
            if self.alg is not None:
                self.alg.script = text
            try:
                with codecs.open(self.filename, 'w', encoding='utf-8') as fout:
                    fout.write(text)
            except IOError:
                QMessageBox.warning(self, self.tr('I/O error'),
                    self.tr('Unable to save edits. Reason:\n %s')
                    % unicode(sys.exc_info()[1])
                )
                return
            self.update = True

            # If help strings were defined before saving the script for
            # the first time, we do it here
            if self.help:
                with open(self.filename + '.help', 'w') as f:
                    json.dump(self.help, f)
                self.help = None
            self.setHasChanged(False)
        else:
            self.filename = None

    def setHasChanged(self, hasChanged):
        self.hasChanged = hasChanged
        self.btnSave.setEnabled(hasChanged)

    def runAlgorithm(self):
        if self.algType == self.SCRIPT_PYTHON:
            alg = ScriptAlgorithm(None, unicode(self.editor.text()))
            alg.provider = ModelerUtils.providers['script']
        if self.algType == self.SCRIPT_R:
            alg = RAlgorithm(None, unicode(self.editor.text()))
            alg.provider = ModelerUtils.providers['r']

        dlg = alg.getCustomParametersDialog()
        if not dlg:
            dlg = AlgorithmDialog(alg)

        canvas = iface.mapCanvas()
        prevMapTool = canvas.mapTool()

        dlg.show()
        dlg.exec_()

        if canvas.mapTool() != prevMapTool:
            try:
                canvas.mapTool().reset()
            except:
                pass
            canvas.setMapTool(prevMapTool)
