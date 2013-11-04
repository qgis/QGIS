# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'python/plugins/processing/ui/widgetNumberInput.ui'
#
# Created: Wed Oct  9 19:20:54 2013
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_widgetNumberInput(object):
    def setupUi(self, widgetNumberInput):
        widgetNumberInput.setObjectName(_fromUtf8("widgetNumberInput"))
        widgetNumberInput.resize(189, 28)
        self.horizontalLayout_2 = QtGui.QHBoxLayout(widgetNumberInput)
        self.horizontalLayout_2.setContentsMargins(0, 0, 0, 2)
        self.horizontalLayout_2.setObjectName(_fromUtf8("horizontalLayout_2"))
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(2)
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
        self.horizontalLayout_2.addLayout(self.horizontalLayout)

        self.retranslateUi(widgetNumberInput)
        QtCore.QMetaObject.connectSlotsByName(widgetNumberInput)

    def retranslateUi(self, widgetNumberInput):
        widgetNumberInput.setWindowTitle(QtGui.QApplication.translate("widgetNumberInput", "Form", None, QtGui.QApplication.UnicodeUTF8))
        self.btnCalc.setToolTip(QtGui.QApplication.translate("widgetNumberInput", "Open number input dialog", None, QtGui.QApplication.UnicodeUTF8))
        self.btnCalc.setText(QtGui.QApplication.translate("widgetNumberInput", "...", None, QtGui.QApplication.UnicodeUTF8))

