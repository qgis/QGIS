# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'python/plugins/processing/ui/DlgAutofill.ui'
#
# Created: Wed Oct  2 20:49:45 2013
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

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
        DlgAutofill.setWindowTitle(QtGui.QApplication.translate("DlgAutofill", "Autofill settings", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("DlgAutofill", "Autofill mode", None, QtGui.QApplication.UnicodeUTF8))
        self.cmbFillType.setItemText(0, QtGui.QApplication.translate("DlgAutofill", "Do not autofill", None, QtGui.QApplication.UnicodeUTF8))
        self.cmbFillType.setItemText(1, QtGui.QApplication.translate("DlgAutofill", "Fill with numbers", None, QtGui.QApplication.UnicodeUTF8))
        self.cmbFillType.setItemText(2, QtGui.QApplication.translate("DlgAutofill", "Fill with parameter values", None, QtGui.QApplication.UnicodeUTF8))
        self.lblParameters.setText(QtGui.QApplication.translate("DlgAutofill", "Parameter to use", None, QtGui.QApplication.UnicodeUTF8))

