# -*- coding: utf-8 -*-

"""
***************************************************************************
    EditRScriptDialog.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from processing.gui.ParametersDialog import ParametersDialog
from processing.core.QGisLayers import QGisLayers
from processing.modeler.Providers import Providers

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import sys

from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from processing.gui.HelpEditionDialog import HelpEditionDialog
import pickle
from processing.r.RAlgorithm import RAlgorithm
from processing.r.RUtils import RUtils

class EditRScriptDialog(QtGui.QDialog):
    def __init__(self, alg):
        self.alg = alg
        if self.alg is not None:
            self.filename = self.alg.descriptionFile
        else:
            self.filename = None
        QtGui.QDialog.__init__(self)
        self.setModal(False)
        self.setupUi()
        self.update = False
        self.help = None

    def setupUi(self):
        self.resize(600,400)
        self.setWindowTitle("Edit script")
        layout = QVBoxLayout()
        self.text = QtGui.QTextEdit()
        self.text.setObjectName("text")
        self.text.setEnabled(True)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        if self.alg != None:
            self.text.setText(self.alg.script)
        self.editHelpButton = QtGui.QPushButton()
        self.editHelpButton.setText("Edit script help")
        self.buttonBox.addButton(self.editHelpButton, QtGui.QDialogButtonBox.ActionRole)
        QObject.connect(self.editHelpButton, QtCore.SIGNAL("clicked()"), self.editHelp)
        self.saveButton = QtGui.QPushButton()
        self.saveButton.setText("Save")
        self.buttonBox.addButton(self.saveButton, QtGui.QDialogButtonBox.ActionRole)
        self.runButton = QtGui.QPushButton()
        self.runButton.setText("Run")
        self.buttonBox.addButton(self.runButton, QtGui.QDialogButtonBox.ActionRole)
        self.closeButton = QtGui.QPushButton()
        self.closeButton.setText("Close")
        self.buttonBox.addButton(self.closeButton, QtGui.QDialogButtonBox.ActionRole)
        QObject.connect(self.runButton, QtCore.SIGNAL("clicked()"), self.runAlgorithm)
        QObject.connect(self.saveButton, QtCore.SIGNAL("clicked()"), self.saveAlgorithm)
        QObject.connect(self.closeButton, QtCore.SIGNAL("clicked()"), self.cancelPressed)
        layout.addWidget(self.text)
        layout.addWidget(self.buttonBox)
        self.setLayout(layout)
        QtCore.QMetaObject.connectSlotsByName(self)

    def editHelp(self):
        if self.alg is None:
            alg = RAlgorithm(None, unicode(self.text.toPlainText()))
        else:
            alg = self.alg
        dlg = HelpEditionDialog(alg)
        dlg.exec_()
        #We store the description string in case there were not saved because there was no
        #filename defined yet
        if self.alg is None and dlg.descriptions:
            self.help = dlg.descriptions

    def runAlgorithm(self):
        alg = RAlgorithm(None, unicode(self.text.toPlainText()))
        alg.provider = Providers.providers['r']
        dlg = alg.getCustomParametersDialog()
        if not dlg:
            dlg = ParametersDialog(alg)
        canvas = QGisLayers.iface.mapCanvas()
        prevMapTool = canvas.mapTool()
        dlg.show()
        dlg.exec_()
        if canvas.mapTool()!=prevMapTool:
            try:
                canvas.mapTool().reset()
            except:
                pass
            canvas.setMapTool(prevMapTool)

    def saveAlgorithm(self):
        if self.filename is None:
            self.filename = str(QtGui.QFileDialog.getSaveFileName(self, "Save Script", RUtils.RScriptsFolder(), "QGIS R script (*.rsx)"))

        if self.filename:
            if not self.filename.endswith(".rsx"):
                self.filename += ".rsx"
            text = str(self.text.toPlainText())
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
            #if help strings were defined before saving the model for the first time, we do it here
            if self.help:
                f = open(self.filename + ".help", "wb")
                pickle.dump(self.help, f)
                f.close()
                self.help = None
            QtGui.QMessageBox.information(self, "Script saving", "Script was correctly saved.")
        else:
            self.filename = None

    def cancelPressed(self):
        #self.update = False
        self.close()
