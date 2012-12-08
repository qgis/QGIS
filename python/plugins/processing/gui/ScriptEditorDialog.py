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

import pickle

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.Qsci import *

from qgis.core import *

from processing.core.QGisLayers import QGisLayers

from processing.gui.ParametersDialog import ParametersDialog
from processing.gui.HelpEditionDialog import HelpEditionDialog
from processing.gui.ScriptEdit import ScriptEdit

from sextante.modeler.Providers import Providers

from processing.r.RAlgorithm import RAlgorithm
from processing.r.RUtils import RUtils
from processing.script.ScriptAlgorithm import ScriptAlgorithm
from processing.script.ScriptUtils import ScriptUtils

from processing.ui.ui_DlgScriptEditor import Ui_DlgScriptEditor

import processing.resources_rc

class ScriptEditorDialog(QDialog, Ui_DlgScriptEditor):

    SCRIPT_PYTHON = 0
    SCRIPT_R = 1

    def __init__(self, algType, alg):
        QDialog.__init__(self)
        self.setupUi(self)

        # set icons
        self.btnSave.setIcon(QgsApplication.getThemeIcon("/mActionFileSave.png"))
        self.btnSaveAs.setIcon(QgsApplication.getThemeIcon("/mActionFileSaveAs.png"))
        self.btnEditHelp.setIcon(QIcon(":/processing/images/edithelp.png"))
        self.btnRun.setIcon(QIcon(":/processing/images/runalgorithm.png"))
        self.btnCut.setIcon(QgsApplication.getThemeIcon("/mActionEditCut.png"))
        self.btnCopy.setIcon(QgsApplication.getThemeIcon("/mActionEditCopy.png"))
        self.btnPaste.setIcon(QgsApplication.getThemeIcon("/mActionEditPaste.png"))
        self.btnUndo.setIcon(QgsApplication.getThemeIcon("/mActionUndo.png"))
        self.btnRedo.setIcon(QgsApplication.getThemeIcon("/mActionRedo.png"))

        # connect signals and slots
        self.btnSave.clicked.connect(self.save)
        self.btnSaveAs.clicked.connect(self.saveAs)
        self.btnEditHelp.clicked.connect(self.editHelp)
        self.btnRun.clicked.connect(self.runAlgorithm)
        self.btnCut.clicked.connect(self.editor.cut)
        self.btnCopy.clicked.connect(self.editor.copy)
        self.btnPaste.clicked.connect(self.editor.paste)
        self.btnUndo.clicked.connect(self.editor.undo)
        self.btnRedo.clicked.connect(self.editor.redo)

        self.alg = alg
        self.algType = algType

        if self.alg is not None:
            self.filename = self.alg.descriptionFile
            self.editor.setText(self.alg.script)
        else:
            self.filename = None

        self.update = False
        self.help = None

        self.editor.setLexerType(self.algType)

    def editHelp(self):
        if self.alg is None:
            if self.algType == self.SCRIPT_PYTHON:
                alg = ScriptAlgorithm(None, unicode(self.editor.toPlainText()))
            elif self.algType == self.SCRIPT_R:
                alg = RAlgorithm(None, unicode(self.editor.toPlainText()))
        else:
            alg = self.alg

        dlg = HelpEditionDialog(alg)
        dlg.exec_()

        # we store the description string in case there were not saved because
        # there was no filename defined yet
        if self.alg is None and dlg.descriptions:
            self.help = dlg.descriptions

    def save(self):
        self.saveScript(False)

    def saveAs(self):
        self.saveScript(True)

    def saveScript(self, saveAs):
        if self.filename is None or saveAs:
            if self.algType == self.SCRIPT_PYTHON:
                scriptDir = ScriptUtils.scriptsFolder()
                filterName = self.tr("Python scripts (*.py)")
            elif self.algType == self.SCRIPT_R:
                scriptDir = RUtils.RScriptsFolder()
                filterName = self.tr("SEXTANTE R script (*.rsx)")

            self.filename = unicode(QFileDialog.getSaveFileName(self,
                                                                self.tr("Save script"),
                                                                scriptDir,
                                                                filterName
                                                               ))

        if self.filename:
            if self.algType == self.SCRIPT_PYTHON and not self.filename.lower().endswith(".py"):
                self.filename += ".py"
            if self.algType == self.SCRIPT_R and not self.filename.lower().endswith(".rsx"):
                self.filename += ".rsx"

            text = unicode(self.editor.text())
            if self.alg is not None:
                self.alg.script = text
            try:
                fout = open(self.filename, "w")
                fout.write(text)
                fout.close()
            except IOError:
                QMessageBox.warning(self,
                                    self.tr("I/O error"),
                                    self.tr("Unable to save edits. Reason:\n %1").arg(unicode(sys.exc_info()[1]))
                                   )
                return
            self.update = True

            # if help strings were defined before saving the script for the first
            # time, we do it here
            if self.help:
                f = open(self.filename + ".help", "wb")
                pickle.dump(self.help, f)
                f.close()
                self.help = None
            QMessageBox.information(self,
                                    self.tr("Script saving"),
                                    self.tr("Script was correctly saved.")
                                   )
        else:
            self.filename = None

    def runAlgorithm(self):
        if self.algType == self.SCRIPT_PYTHON:
            alg = ScriptAlgorithm(None, unicode(self.editor.text()))
            alg.provider = Providers.providers['script']
        if self.algType == self.SCRIPT_R:
            alg = RAlgorithm(None, unicode(self.editor.text()))
            alg.provider = Providers.providers['r']

        dlg = alg.getCustomParametersDialog()
        if not dlg:
            dlg = ParametersDialog(alg)

        canvas = QGisLayers.iface.mapCanvas()
        prevMapTool = canvas.mapTool()

        dlg.show()
        dlg.exec_()

        if canvas.mapTool() != prevMapTool:
            try:
                canvas.mapTool().reset()
            except:
                pass
            canvas.setMapTool(prevMapTool)
