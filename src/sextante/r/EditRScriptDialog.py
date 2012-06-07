from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.gui.HelpEditionDialog import HelpEditionDialog
import pickle
from sextante.r.RAlgorithm import RAlgorithm
from sextante.r.RUtils import RUtils

class EditRScriptDialog(QtGui.QDialog):
    def __init__(self, alg):
        self.alg = alg
        if self.alg is not None:
            self.filename = self.alg.descriptionFile
        else:
            self.filename = None
        QtGui.QDialog.__init__(self)
        self.setModal(True)
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
        self.closeButton = QtGui.QPushButton()
        self.closeButton.setText("Close")
        self.buttonBox.addButton(self.closeButton, QtGui.QDialogButtonBox.ActionRole)
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


    def saveAlgorithm(self):
        if self.filename is None:
            self.filename = QtGui.QFileDialog.getSaveFileName(self, "Save Script", RUtils.RScriptsFolder(), "SEXTANTE R script (*.rsx)")

        if self.filename:
            text = str(self.text.toPlainText())
            if self.alg is not None:
                self.alg.script = text
            fout = open(self.filename, "w")
            fout.write(text)
            fout.close()
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