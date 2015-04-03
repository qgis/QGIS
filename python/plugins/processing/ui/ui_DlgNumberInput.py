# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DlgNumberInput.ui'
#
# Created: Fri Nov 21 13:25:48 2014
#      by: PyQt4 UI code generator 4.11.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_DlgNumberInput(object):
    def setupUi(self, DlgNumberInput):
        DlgNumberInput.setObjectName(_fromUtf8("DlgNumberInput"))
        DlgNumberInput.resize(445, 300)
        self.verticalLayout = QtGui.QVBoxLayout(DlgNumberInput)
        self.verticalLayout.setSpacing(6)
        self.verticalLayout.setMargin(9)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.label = QtGui.QLabel(DlgNumberInput)
        self.label.setWordWrap(True)
        self.label.setObjectName(_fromUtf8("label"))
        self.verticalLayout.addWidget(self.label)
        self.lblWarning = QtGui.QLabel(DlgNumberInput)
        self.lblWarning.setWordWrap(True)
        self.lblWarning.setObjectName(_fromUtf8("lblWarning"))
        self.verticalLayout.addWidget(self.lblWarning)
        self.treeValues = QtGui.QTreeWidget(DlgNumberInput)
        self.treeValues.setObjectName(_fromUtf8("treeValues"))
        self.treeValues.headerItem().setText(0, _fromUtf8("1"))
        self.treeValues.header().setVisible(False)
        self.verticalLayout.addWidget(self.treeValues)
        self.leFormula = QtGui.QLineEdit(DlgNumberInput)
        self.leFormula.setObjectName(_fromUtf8("leFormula"))
        self.verticalLayout.addWidget(self.leFormula)
        self.buttonBox = QtGui.QDialogButtonBox(DlgNumberInput)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout.addWidget(self.buttonBox)

        self.retranslateUi(DlgNumberInput)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), DlgNumberInput.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), DlgNumberInput.reject)
        QtCore.QMetaObject.connectSlotsByName(DlgNumberInput)

    def retranslateUi(self, DlgNumberInput):
        DlgNumberInput.setWindowTitle(_translate("DlgNumberInput", "Enter number or expression", None))
        self.label.setText(_translate("DlgNumberInput", "<html><head/><body><p>Enter expression in the text field. Double click on elements in the tree to add their values to the expression.</p></body></html>", None))
        self.lblWarning.setText(_translate("DlgNumberInput", "<html><head/><body><p><span style=\" font-weight:600;\">Warning</span>: if expression result is float value, but integer required, result will be rounded to integer.</p></body></html>", None))

