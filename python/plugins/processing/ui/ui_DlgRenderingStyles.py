# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DlgRenderingStyles.ui'
#
# Created: Fri Nov 21 13:25:48 2014
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

class Ui_DlgRenderingStyles(object):
    def setupUi(self, DlgRenderingStyles):
        DlgRenderingStyles.setObjectName(_fromUtf8("DlgRenderingStyles"))
        DlgRenderingStyles.resize(550, 400)
        self.verticalLayout = QtGui.QVBoxLayout(DlgRenderingStyles)
        self.verticalLayout.setSpacing(6)
        self.verticalLayout.setMargin(9)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.tblStyles = QtGui.QTableWidget(DlgRenderingStyles)
        self.tblStyles.setObjectName(_fromUtf8("tblStyles"))
        self.tblStyles.setColumnCount(2)
        self.tblStyles.setRowCount(0)
        item = QtGui.QTableWidgetItem()
        self.tblStyles.setHorizontalHeaderItem(0, item)
        item = QtGui.QTableWidgetItem()
        self.tblStyles.setHorizontalHeaderItem(1, item)
        self.tblStyles.verticalHeader().setVisible(False)
        self.verticalLayout.addWidget(self.tblStyles)
        self.buttonBox = QtGui.QDialogButtonBox(DlgRenderingStyles)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout.addWidget(self.buttonBox)

        self.retranslateUi(DlgRenderingStyles)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), DlgRenderingStyles.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), DlgRenderingStyles.reject)
        QtCore.QMetaObject.connectSlotsByName(DlgRenderingStyles)

    def retranslateUi(self, DlgRenderingStyles):
        DlgRenderingStyles.setWindowTitle(_translate("DlgRenderingStyles", "Dialog", None))
        item = self.tblStyles.horizontalHeaderItem(0)
        item.setText(_translate("DlgRenderingStyles", "Output", None))
        item = self.tblStyles.horizontalHeaderItem(1)
        item.setText(_translate("DlgRenderingStyles", "Style", None))

