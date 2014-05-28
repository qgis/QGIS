# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '/home/tkralidi/work/foss4g/MetaSearch/MetaSearch/plugin/MetaSearch/ui/manageconnectionsdialog.ui'
#
# Created: Thu Mar 20 21:56:34 2014
#      by: PyQt4 UI code generator 4.9.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_ManageConnectionsDialog(object):
    def setupUi(self, ManageConnectionsDialog):
        ManageConnectionsDialog.setObjectName(_fromUtf8("ManageConnectionsDialog"))
        ManageConnectionsDialog.resize(400, 300)
        self.verticalLayout = QtGui.QVBoxLayout(ManageConnectionsDialog)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.label = QtGui.QLabel(ManageConnectionsDialog)
        self.label.setObjectName(_fromUtf8("label"))
        self.horizontalLayout.addWidget(self.label)
        self.leFileName = QtGui.QLineEdit(ManageConnectionsDialog)
        self.leFileName.setObjectName(_fromUtf8("leFileName"))
        self.horizontalLayout.addWidget(self.leFileName)
        self.btnBrowse = QtGui.QPushButton(ManageConnectionsDialog)
        self.btnBrowse.setObjectName(_fromUtf8("btnBrowse"))
        self.horizontalLayout.addWidget(self.btnBrowse)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.listConnections = QtGui.QListWidget(ManageConnectionsDialog)
        self.listConnections.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.listConnections.setAlternatingRowColors(True)
        self.listConnections.setSelectionMode(QtGui.QAbstractItemView.ExtendedSelection)
        self.listConnections.setObjectName(_fromUtf8("listConnections"))
        self.verticalLayout.addWidget(self.listConnections)
        self.buttonBox = QtGui.QDialogButtonBox(ManageConnectionsDialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout.addWidget(self.buttonBox)

        self.retranslateUi(ManageConnectionsDialog)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), ManageConnectionsDialog.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), ManageConnectionsDialog.reject)
        QtCore.QMetaObject.connectSlotsByName(ManageConnectionsDialog)

    def retranslateUi(self, ManageConnectionsDialog):
        ManageConnectionsDialog.setWindowTitle(QtGui.QApplication.translate("ManageConnectionsDialog", "Manage connections", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("ManageConnectionsDialog", "Save to file", None, QtGui.QApplication.UnicodeUTF8))
        self.btnBrowse.setText(QtGui.QApplication.translate("ManageConnectionsDialog", "Browse", None, QtGui.QApplication.UnicodeUTF8))

