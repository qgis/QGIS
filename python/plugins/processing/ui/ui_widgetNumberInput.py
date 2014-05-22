# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'widgetNumberInput.ui'
#
# Created: Thu May 22 12:21:43 2014
#      by: PyQt4 UI code generator 4.9.6
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

class Ui_widgetNumberInput(object):
    def setupUi(self, widgetNumberInput):
        widgetNumberInput.setObjectName(_fromUtf8("widgetNumberInput"))
        widgetNumberInput.resize(251, 24)
        self.horizontalLayout = QtGui.QHBoxLayout(widgetNumberInput)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.spnValue = QtGui.QDoubleSpinBox(widgetNumberInput)
        self.spnValue.setDecimals(6)
        self.spnValue.setMinimum(-100000000.0)
        self.spnValue.setMaximum(100000000.0)
        self.spnValue.setObjectName(_fromUtf8("spnValue"))
        self.horizontalLayout.addWidget(self.spnValue)
        self.btnCalc = QtGui.QToolButton(widgetNumberInput)
        self.btnCalc.setObjectName(_fromUtf8("btnCalc"))
        self.horizontalLayout.addWidget(self.btnCalc)

        self.retranslateUi(widgetNumberInput)
        QtCore.QMetaObject.connectSlotsByName(widgetNumberInput)

    def retranslateUi(self, widgetNumberInput):
        widgetNumberInput.setWindowTitle(_translate("widgetNumberInput", "Form", None))
        self.btnCalc.setToolTip(_translate("widgetNumberInput", "Open number input dialog", None))
        self.btnCalc.setText(_translate("widgetNumberInput", "...", None))

