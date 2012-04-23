from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.r.RUtils import RUtils
import pickle

class EditRScriptDialog(QtGui.QDialog):
    def __init__(self, alg):
        self.alg = alg
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.setupUi()
        self.update = False

    def setupUi(self):
        self.setWindowTitle("Edit script")
        layout = QVBoxLayout()
        self.text = QtGui.QTextEdit()
        self.text.setObjectName("text")
        self.text.setEnabled(True)
        if self.alg != None:
            self.text.setText(self.alg.script)
        self.buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Close)
        self.editHelpButton = QtGui.QPushButton()
        self.editHelpButton.setText("Edit model help")
        self.buttonBox.addButton(self.editHelpButton, QtGui.QDialogButtonBox.ActionRole)
        layout.addWidget(self.text)
        layout.addWidget(self.buttonBox)
        self.setLayout(layout)
        self.connect(self.buttonBox, SIGNAL("accepted()"), self.saveAlgorithm)
        self.connect(self.buttonBox, SIGNAL("rejected()"), self.cancelPressed)
        self.connect(self.editHelpButton, SIGNAL("clicked()"), self.editHelp)
        QtCore.QMetaObject.connectSlotsByName(self)

    def editHelp(self):
        dlg = HelpEditionDialog(self.alg)
        dlg.exec_()
        #We store the description string in case there were not saved because there was no
        #filename defined yet
        if self.alg.descriptionFile is None and dlg.descriptions:
            self.help = dlg.descriptions

    def saveAlgorithm(self):
        if self.alg!=None:
            filename = self.alg.descriptionFile
        else:
            filename = QtGui.QFileDialog.getSaveFileName(self, "Save Script", RUtils.RScriptsFolder(), "R-SEXTANTE scripts (*.rsx)")
        if filename:
            text = self.text.toPlainText()
            fout = open(filename, "w")
            fout.write(text)
            fout.close()
            self.update = True
            self.close()

        #if help strings were defined before saving the model for the first time, we do it here
        if self.help:
            f = open(self.alg.descriptionFile + ".help", "wb")
            pickle.dump(self.help, f)
            f.close()
            self.help = None

    def cancelPressed(self):
        self.update = False
        self.close()