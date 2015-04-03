# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'widgetBaseSelector.ui'
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
        Form.resize(249, 23)
        self.horizontalLayout = QtGui.QHBoxLayout(Form)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.leText = QtGui.QLineEdit(Form)
        self.leText.setObjectName(_fromUtf8("leText"))
        self.horizontalLayout.addWidget(self.leText)
        self.btnSelect = QtGui.QToolButton(Form)
        self.btnSelect.setObjectName(_fromUtf8("btnSelect"))
        self.horizontalLayout.addWidget(self.btnSelect)

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        Form.setWindowTitle(_translate("Form", "Form", None))
        self.btnSelect.setText(_translate("Form", "...", None))

