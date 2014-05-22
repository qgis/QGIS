# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'widgetOutputSelect.ui'
#
# Created: Thu May 22 11:43:10 2014
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

class Ui_widgetOutputSelect(object):
    def setupUi(self, widgetOutputSelect):
        widgetOutputSelect.setObjectName(_fromUtf8("widgetOutputSelect"))
        widgetOutputSelect.resize(400, 24)
        self.horizontalLayout = QtGui.QHBoxLayout(widgetOutputSelect)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.text = QtGui.QLineEdit(widgetOutputSelect)
        self.text.setObjectName(_fromUtf8("text"))
        self.horizontalLayout.addWidget(self.text)
        self.btnBrowse = QtGui.QToolButton(widgetOutputSelect)
        self.btnBrowse.setObjectName(_fromUtf8("btnBrowse"))
        self.horizontalLayout.addWidget(self.btnBrowse)

        self.retranslateUi(widgetOutputSelect)
        QtCore.QMetaObject.connectSlotsByName(widgetOutputSelect)

    def retranslateUi(self, widgetOutputSelect):
        widgetOutputSelect.setWindowTitle(_translate("widgetOutputSelect", "Form", None))
        self.btnBrowse.setText(_translate("widgetOutputSelect", "...", None))

