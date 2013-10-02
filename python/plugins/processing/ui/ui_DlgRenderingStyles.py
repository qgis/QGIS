# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'python/plugins/processing/ui/DlgRenderingStyles.ui'
#
# Created: Wed Oct  2 17:04:59 2013
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_DlgRenderingStyles(object):
    def setupUi(self, DlgRenderingStyles):
        DlgRenderingStyles.setObjectName(_fromUtf8("DlgRenderingStyles"))
        DlgRenderingStyles.resize(550, 400)
        self.verticalLayout = QtGui.QVBoxLayout(DlgRenderingStyles)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
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
        DlgRenderingStyles.setWindowTitle(QtGui.QApplication.translate("DlgRenderingStyles", "Dialog", None, QtGui.QApplication.UnicodeUTF8))
        item = self.tblStyles.horizontalHeaderItem(0)
        item.setText(QtGui.QApplication.translate("DlgRenderingStyles", "Output", None, QtGui.QApplication.UnicodeUTF8))
        item = self.tblStyles.horizontalHeaderItem(1)
        item.setText(QtGui.QApplication.translate("DlgRenderingStyles", "Style", None, QtGui.QApplication.UnicodeUTF8))

