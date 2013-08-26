# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DlgConfig.ui'
#
# Created: Tue Aug 20 11:01:25 2013
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_DlgConfig(object):
    def setupUi(self, DlgConfig):
        DlgConfig.setObjectName(_fromUtf8("DlgConfig"))
        DlgConfig.resize(640, 450)
        self.verticalLayout = QtGui.QVBoxLayout(DlgConfig)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.searchBox = QgsFilterLineEdit(DlgConfig)
        self.searchBox.setObjectName(_fromUtf8("searchBox"))
        self.verticalLayout.addWidget(self.searchBox)
        self.tree = QtGui.QTreeWidget(DlgConfig)
        self.tree.setAlternatingRowColors(True)
        self.tree.setObjectName(_fromUtf8("tree"))
        self.verticalLayout.addWidget(self.tree)
        self.buttonBox = QtGui.QDialogButtonBox(DlgConfig)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout.addWidget(self.buttonBox)

        self.retranslateUi(DlgConfig)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), DlgConfig.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), DlgConfig.reject)
        QtCore.QMetaObject.connectSlotsByName(DlgConfig)

    def retranslateUi(self, DlgConfig):
        DlgConfig.setWindowTitle(QtGui.QApplication.translate("DlgConfig", "Processing options", None, QtGui.QApplication.UnicodeUTF8))
        self.searchBox.setToolTip(QtGui.QApplication.translate("DlgConfig", "Enter setting name to filter list", None, QtGui.QApplication.UnicodeUTF8))
        self.tree.headerItem().setText(0, QtGui.QApplication.translate("DlgConfig", "Setting", None, QtGui.QApplication.UnicodeUTF8))
        self.tree.headerItem().setText(1, QtGui.QApplication.translate("DlgConfig", "Value", None, QtGui.QApplication.UnicodeUTF8))

from qgis.gui import QgsFilterLineEdit
