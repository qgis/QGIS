# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DlgFixedTable.ui'
#
# Created: Fri Nov 21 13:25:47 2014
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

class Ui_DlgFixedTable(object):
    def setupUi(self, DlgFixedTable):
        DlgFixedTable.setObjectName(_fromUtf8("DlgFixedTable"))
        DlgFixedTable.resize(380, 320)
        self.horizontalLayout = QtGui.QHBoxLayout(DlgFixedTable)
        self.horizontalLayout.setSpacing(6)
        self.horizontalLayout.setMargin(9)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.tblView = QtGui.QTableView(DlgFixedTable)
        self.tblView.setObjectName(_fromUtf8("tblView"))
        self.tblView.horizontalHeader().setStretchLastSection(True)
        self.horizontalLayout.addWidget(self.tblView)
        self.buttonBox = QtGui.QDialogButtonBox(DlgFixedTable)
        self.buttonBox.setOrientation(QtCore.Qt.Vertical)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.horizontalLayout.addWidget(self.buttonBox)

        self.retranslateUi(DlgFixedTable)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), DlgFixedTable.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), DlgFixedTable.reject)
        QtCore.QMetaObject.connectSlotsByName(DlgFixedTable)

    def retranslateUi(self, DlgFixedTable):
        DlgFixedTable.setWindowTitle(_translate("DlgFixedTable", "Fixed table", None))

