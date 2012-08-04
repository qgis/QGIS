from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import *
from PyQt4.QtGui import *


class AutofillDialog(QtGui.QDialog):

    DO_NOT_AUTOFILL = 0
    FILL_WITH_NUMBERS = 1
    FILL_WITH_PARAMETER = 2

    def __init__(self,alg):
        QtGui.QDialog.__init__(self)

        self.verticalLayout = QtGui.QVBoxLayout(self)
        self.verticalLayout.setSpacing(40)
        self.verticalLayout.setMargin(20)
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.label = QtGui.QLabel("Autofill mode")
        self.horizontalLayout.addWidget(self.label)
        self.typeCombo = QtGui.QComboBox()
        self.typeCombo.addItem("Do not autofill")
        self.typeCombo.addItem("Fill with numbers")
        self.typeCombo.addItem("Fill with parameter values")
        self.horizontalLayout.addWidget(self.typeCombo)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.horizontalLayout2 = QtGui.QHBoxLayout(self)
        self.horizontalLayout2.setSpacing(2)
        self.horizontalLayout2.setMargin(0)
        self.label2 = QtGui.QLabel("Parameter to use")
        self.horizontalLayout2.addWidget(self.label2)
        self.fieldCombo = QtGui.QComboBox()
        for param in alg.parameters:
            self.fieldCombo.addItem(param.description)
        self.horizontalLayout2.addWidget(self.fieldCombo)
        self.verticalLayout.addLayout(self.horizontalLayout2)

        self.buttonBox = QtGui.QDialogButtonBox(self)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        QObject.connect(self.buttonBox, QtCore.SIGNAL("accepted()"), self.okPressed)
        QObject.connect(self.buttonBox, QtCore.SIGNAL("rejected()"), self.cancelPressed)
        self.verticalLayout.addWidget(self.buttonBox)

        self.setLayout(self.verticalLayout)


    def okPressed(self):
        self.mode = self.typeCombo.currentIndex()
        self.param = self.fieldCombo.currentIndex()
        self.close()

    def cancelPressed(self):
        self.mode == None
        self.param = None
        self.close()