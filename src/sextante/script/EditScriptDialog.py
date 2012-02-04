from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.script.ScriptUtils import ScriptUtils
import os

class EditScriptDialog(QtGui.QDialog):
    def __init__(self, alg):
        self.alg = alg
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.setupUi()
        self.update = False

    def setupUi(self):
        self.setObjectName("Dialog")
        self.resize(655, 360)
        self.setWindowTitle("Edit script")
        self.text = QtGui.QTextEdit(self)
        self.text.setGeometry(QtCore.QRect(5, 5, 550, 350))
        self.text.setObjectName("text")
        self.text.setEnabled(True)
        if self.alg != None:
            self.text.setText(self.alg.script)
        self.saveButton = QtGui.QPushButton(self)
        self.saveButton.setGeometry(QtCore.QRect(570, 300, 80, 23))
        self.saveButton.setObjectName("saveButton")
        self.saveButton.setText("Save")
        self.cancelButton = QtGui.QPushButton(self)
        self.cancelButton.setGeometry(QtCore.QRect(570, 327, 80, 23))
        self.cancelButton.setObjectName("cancelButton")
        self.cancelButton.setText("Cancel")
        QObject.connect(self.saveButton, QtCore.SIGNAL("clicked()"), self.saveAlgorithm)
        QObject.connect(self.cancelButton, QtCore.SIGNAL("clicked()"), self.cancel)
        QtCore.QMetaObject.connectSlotsByName(self)

    def saveAlgorithm(self):
        if self.alg!=None:
            filename = os.path.join(ScriptUtils.scriptsFolder(), self.alg.descriptionFile)
        else:
            filename = QtGui.QFileDialog.getSaveFileName(self, "Save Script", ScriptUtils.scriptsFolder(), "Python scripts (*.py)")
        if filename:
            text = self.text.toPlainText()
            fout = open(filename, "w")
            fout.write(text)
            fout.close()
            self.update = True
            self.close()

    def cancel(self):
        self.update = False
        self.dialog.close()