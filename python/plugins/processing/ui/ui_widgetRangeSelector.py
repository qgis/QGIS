# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'widgetRangeSelector.ui'
#
# Created: Fri Nov 21 13:25:50 2014
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
        Form.resize(345, 23)
        self.horizontalLayout = QtGui.QHBoxLayout(Form)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.label = QtGui.QLabel(Form)
        self.label.setObjectName(_fromUtf8("label"))
        self.horizontalLayout.addWidget(self.label)
        self.spnMin = QtGui.QDoubleSpinBox(Form)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spnMin.sizePolicy().hasHeightForWidth())
        self.spnMin.setSizePolicy(sizePolicy)
        self.spnMin.setDecimals(6)
        self.spnMin.setMinimum(-100000000.0)
        self.spnMin.setMaximum(100000000.0)
        self.spnMin.setObjectName(_fromUtf8("spnMin"))
        self.horizontalLayout.addWidget(self.spnMin)
        spacerItem = QtGui.QSpacerItem(64, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.label_2 = QtGui.QLabel(Form)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.horizontalLayout.addWidget(self.label_2)
        self.spnMax = QtGui.QDoubleSpinBox(Form)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spnMax.sizePolicy().hasHeightForWidth())
        self.spnMax.setSizePolicy(sizePolicy)
        self.spnMax.setDecimals(6)
        self.spnMax.setMinimum(-100000000.0)
        self.spnMax.setMaximum(100000000.0)
        self.spnMax.setObjectName(_fromUtf8("spnMax"))
        self.horizontalLayout.addWidget(self.spnMax)

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        Form.setWindowTitle(_translate("Form", "Form", None))
        self.label.setText(_translate("Form", "Min", None))
        self.label_2.setText(_translate("Form", "Max", None))

