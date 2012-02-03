from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.script.ScriptUtils import ScriptUtils

class EditScriptDialog(QtGui.QDialog):
    def __init__(self, alg):
        self.alg = alg
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.ui = Ui_EditScriptDialog()
        self.ui.setupUi(self)
        self.update = False

class Ui_EditScriptDialog(object):
    def setupUi(self, dialog):
        self.dialog = dialog
        dialog.setObjectName("Dialog")
        dialog.resize(600, 350)
        dialog.setWindowTitle("Edit script")
        self.text = QtGui.QTextEdit(dialog)
        self.text.setGeometry(QtCore.QRect(10, 10, 470, 300))
        self.text.setObjectName("text")
        self.text.setEnabled(True)
        if dialog.alg != None:
            self.text.setText(dialog.alg.script)
        self.saveButton = QtGui.QPushButton(dialog)
        self.saveButton.setGeometry(QtCore.QRect(490, 250, 81, 23))
        self.saveButton.setObjectName("saveButton")
        self.saveButton.setText("Save")
        self.cancelButton = QtGui.QPushButton(dialog)
        self.cancelButton.setGeometry(QtCore.QRect(490, 290, 81, 23))
        self.cancelButton.setObjectName("cancelButton")
        self.cancelButton.setText("Cancel")
        QObject.connect(self.saveButton, QtCore.SIGNAL("clicked()"), self.saveAlgorithm)
        QObject.connect(self.cancelButton, QtCore.SIGNAL("clicked()"), self.cancel)
        QtCore.QMetaObject.connectSlotsByName(dialog)

    def saveAlgorithm(self):
        filename = QtGui.QFileDialog.getSaveFileName(self.dialog, "Save Script", ScriptUtils.scriptsFolder(), "Python scripts (*.py)")
        if filename:
            text = self.text.toPlainText()
            fout = open(filename, "w")
            fout.write(text)
            fout.close()
            self.dialog.update = True
            self.dialog.close()

    def cancel(self):
        self.dialog.update = False
        self.dialog.close()