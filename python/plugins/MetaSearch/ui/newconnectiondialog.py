# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '/home/tkralidi/work/foss4g/MetaSearch/MetaSearch/plugin/MetaSearch/ui/newconnectiondialog.ui'
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

class Ui_NewConnectionDialog(object):
    def setupUi(self, NewConnectionDialog):
        NewConnectionDialog.setObjectName(_fromUtf8("NewConnectionDialog"))
        NewConnectionDialog.resize(368, 120)
        self.gridLayout = QtGui.QGridLayout(NewConnectionDialog)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.label = QtGui.QLabel(NewConnectionDialog)
        self.label.setObjectName(_fromUtf8("label"))
        self.gridLayout.addWidget(self.label, 0, 0, 1, 1)
        self.leName = QtGui.QLineEdit(NewConnectionDialog)
        self.leName.setObjectName(_fromUtf8("leName"))
        self.gridLayout.addWidget(self.leName, 0, 1, 1, 1)
        self.label_2 = QtGui.QLabel(NewConnectionDialog)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.gridLayout.addWidget(self.label_2, 1, 0, 1, 1)
        self.leURL = QtGui.QLineEdit(NewConnectionDialog)
        self.leURL.setObjectName(_fromUtf8("leURL"))
        self.gridLayout.addWidget(self.leURL, 1, 1, 1, 1)
        self.buttonBox = QtGui.QDialogButtonBox(NewConnectionDialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.gridLayout.addWidget(self.buttonBox, 2, 0, 1, 2)

        self.retranslateUi(NewConnectionDialog)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), NewConnectionDialog.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), NewConnectionDialog.reject)
        QtCore.QMetaObject.connectSlotsByName(NewConnectionDialog)

    def retranslateUi(self, NewConnectionDialog):
        NewConnectionDialog.setWindowTitle(QtGui.QApplication.translate("NewConnectionDialog", "Create a new Catalogue connection", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("NewConnectionDialog", "Name", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("NewConnectionDialog", "URL", None, QtGui.QApplication.UnicodeUTF8))

