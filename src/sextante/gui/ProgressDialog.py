from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.QGisLayers import QGisLayers
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.gui.AlgorithmExecutor import AlgorithmExecutor

class ProgressDialog(QtGui.QDialog):
    def __init__(self, alg, load=True):
        self.alg = alg
        self.load = load
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.ui = Ui_ProgressDialog()
        self.ui.setupUi(self)
        self.isCanceled = False

    def runalg(self):
        try:
            AlgorithmExecutor.runalg(self.alg, self)
            if self.load and not self.isCanceled:
                QGisLayers.loadFromAlg(self.alg)
            self.ui.cancelButton.setText("OK")
            #self.close()
        except GeoAlgorithmExecutionException, e :
            QMessageBox.critical(self, "Error",e.msg)
            self.ui.cancelButton.setText("OK")

    def setPercentage(self, percent):
        self.ui.progress.setValue(percent)

    def addText(self, text):
        oldtext = self.ui.text.toPlainText()
        text = oldtext + text
        self.ui.text.setText(text)
        return not self.isCanceled

class Ui_ProgressDialog(object):
    def setupUi(self, dialog):
        self.dialog = dialog
        dialog.setObjectName("Dialog")
        dialog.resize(600, 390)
        dialog.setWindowTitle("Running...")
        self.progress = QtGui.QProgressBar(dialog)
        self.progress.setMinimum(0)
        self.progress.setMaximum(100)
        self.progress.setGeometry(QtCore.QRect(10, 10, 580, 30))
        self.text = QtGui.QTextEdit(dialog)
        self.text.setGeometry(QtCore.QRect(10, 50, 580, 280))
        self.text.setObjectName("text")
        self.text.setReadOnly(True)
        self.cancelButton = QtGui.QPushButton(dialog)
        self.cancelButton.setGeometry(QtCore.QRect(500, 350, 90, 23))
        self.cancelButton.setObjectName("cancelButton")
        self.cancelButton.setText("Cancel")
        QObject.connect(self.cancelButton, QtCore.SIGNAL("clicked()"), self.cancelAlgorithm)
        QtCore.QMetaObject.connectSlotsByName(dialog)

    def cancelAlgorithm(self):
        self.dialog.isCanceled = True
        self.dialog.close()