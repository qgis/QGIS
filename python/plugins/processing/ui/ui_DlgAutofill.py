# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DlgAutofill.ui'
#
# Created: Fri Nov 21 13:25:46 2014
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

class Ui_DlgAutofill(object):
    def setupUi(self, DlgAutofill):
        DlgAutofill.setObjectName(_fromUtf8("DlgAutofill"))
        DlgAutofill.resize(400, 104)
        self.gridLayout = QtGui.QGridLayout(DlgAutofill)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.label = QtGui.QLabel(DlgAutofill)
        self.label.setObjectName(_fromUtf8("label"))
        self.gridLayout.addWidget(self.label, 0, 0, 1, 1)
        self.cmbFillType = QtGui.QComboBox(DlgAutofill)
        self.cmbFillType.setObjectName(_fromUtf8("cmbFillType"))
        self.cmbFillType.addItem(_fromUtf8(""))
        self.cmbFillType.addItem(_fromUtf8(""))
        self.cmbFillType.addItem(_fromUtf8(""))
        self.gridLayout.addWidget(self.cmbFillType, 0, 1, 1, 1)
        self.lblParameters = QtGui.QLabel(DlgAutofill)
        self.lblParameters.setEnabled(False)
        self.lblParameters.setObjectName(_fromUtf8("lblParameters"))
        self.gridLayout.addWidget(self.lblParameters, 1, 0, 1, 1)
        self.cmbParameters = QtGui.QComboBox(DlgAutofill)
        self.cmbParameters.setEnabled(False)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.cmbParameters.sizePolicy().hasHeightForWidth())
        self.cmbParameters.setSizePolicy(sizePolicy)
        self.cmbParameters.setObjectName(_fromUtf8("cmbParameters"))
        self.gridLayout.addWidget(self.cmbParameters, 1, 1, 1, 1)
        self.buttonBox = QtGui.QDialogButtonBox(DlgAutofill)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.gridLayout.addWidget(self.buttonBox, 2, 0, 1, 2)

        self.retranslateUi(DlgAutofill)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), DlgAutofill.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), DlgAutofill.reject)
        QtCore.QMetaObject.connectSlotsByName(DlgAutofill)

    def retranslateUi(self, DlgAutofill):
        DlgAutofill.setWindowTitle(_translate("DlgAutofill", "Autofill settings", None))
        self.label.setText(_translate("DlgAutofill", "Autofill mode", None))
        self.cmbFillType.setItemText(0, _translate("DlgAutofill", "Do not autofill", None))
        self.cmbFillType.setItemText(1, _translate("DlgAutofill", "Fill with numbers", None))
        self.cmbFillType.setItemText(2, _translate("DlgAutofill", "Fill with parameter values", None))
        self.lblParameters.setText(_translate("DlgAutofill", "Parameter to use", None))

