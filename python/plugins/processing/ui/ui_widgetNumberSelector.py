# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'widgetNumberSelector.ui'
#
# Created: Fri Nov 21 13:25:49 2014
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

class Ui_Form(object):
    def setupUi(self, Form):
        Form.setObjectName(_fromUtf8("Form"))
        Form.resize(251, 23)
        self.horizontalLayout = QtGui.QHBoxLayout(Form)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.spnValue = QtGui.QDoubleSpinBox(Form)
        self.spnValue.setDecimals(6)
        self.spnValue.setMinimum(-100000000.0)
        self.spnValue.setMaximum(100000000.0)
        self.spnValue.setObjectName(_fromUtf8("spnValue"))
        self.horizontalLayout.addWidget(self.spnValue)
        self.btnCalc = QtGui.QToolButton(Form)
        self.btnCalc.setObjectName(_fromUtf8("btnCalc"))
        self.horizontalLayout.addWidget(self.btnCalc)

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        Form.setWindowTitle(_translate("Form", "Form", None))
        self.btnCalc.setToolTip(_translate("Form", "Open number input dialog", None))
        self.btnCalc.setText(_translate("Form", "...", None))

