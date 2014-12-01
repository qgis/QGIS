# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'widgetBatchPanel.ui'
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
        Form.resize(400, 252)
        self.gridLayout = QtGui.QGridLayout(Form)
        self.gridLayout.setMargin(0)
        self.gridLayout.setSpacing(2)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.btnAdvanced = QtGui.QToolButton(Form)
        self.btnAdvanced.setCheckable(True)
        self.btnAdvanced.setAutoRaise(True)
        self.btnAdvanced.setObjectName(_fromUtf8("btnAdvanced"))
        self.gridLayout.addWidget(self.btnAdvanced, 0, 0, 1, 1)
        self.btnAdd = QtGui.QToolButton(Form)
        self.btnAdd.setAutoRaise(True)
        self.btnAdd.setObjectName(_fromUtf8("btnAdd"))
        self.gridLayout.addWidget(self.btnAdd, 0, 1, 1, 1)
        self.btnRemove = QtGui.QToolButton(Form)
        self.btnRemove.setAutoRaise(True)
        self.btnRemove.setObjectName(_fromUtf8("btnRemove"))
        self.gridLayout.addWidget(self.btnRemove, 0, 2, 1, 1)
        spacerItem = QtGui.QSpacerItem(313, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.gridLayout.addItem(spacerItem, 0, 3, 1, 1)
        self.tblParameters = QtGui.QTableWidget(Form)
        self.tblParameters.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.tblParameters.setObjectName(_fromUtf8("tblParameters"))
        self.tblParameters.setColumnCount(0)
        self.tblParameters.setRowCount(0)
        self.tblParameters.horizontalHeader().setStretchLastSection(True)
        self.tblParameters.verticalHeader().setVisible(False)
        self.gridLayout.addWidget(self.tblParameters, 1, 0, 1, 4)

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        Form.setWindowTitle(_translate("Form", "Form", None))
        self.btnAdvanced.setToolTip(_translate("Form", "Toggle advanced mode", None))
        self.btnAdvanced.setText(_translate("Form", "...", None))
        self.btnAdd.setToolTip(_translate("Form", "Add row", None))
        self.btnAdd.setText(_translate("Form", "...", None))
        self.btnRemove.setToolTip(_translate("Form", "Remove row(s)", None))
        self.btnRemove.setText(_translate("Form", "...", None))

