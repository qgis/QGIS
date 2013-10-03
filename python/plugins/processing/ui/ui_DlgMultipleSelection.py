# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'python/plugins/processing/ui/DlgMultipleSelection.ui'
#
# Created: Thu Oct  3 20:32:49 2013
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_DlgMultipleSelection(object):
    def setupUi(self, DlgMultipleSelection):
        DlgMultipleSelection.setObjectName(_fromUtf8("DlgMultipleSelection"))
        DlgMultipleSelection.resize(380, 320)
        self.horizontalLayout = QtGui.QHBoxLayout(DlgMultipleSelection)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setContentsMargins(0, 5, 0, 0)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.lstLayers = QtGui.QListWidget(DlgMultipleSelection)
        self.lstLayers.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.lstLayers.setAlternatingRowColors(True)
        self.lstLayers.setSelectionMode(QtGui.QAbstractItemView.ExtendedSelection)
        self.lstLayers.setObjectName(_fromUtf8("lstLayers"))
        self.horizontalLayout.addWidget(self.lstLayers)
        self.buttonBox = QtGui.QDialogButtonBox(DlgMultipleSelection)
        self.buttonBox.setOrientation(QtCore.Qt.Vertical)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.horizontalLayout.addWidget(self.buttonBox)

        self.retranslateUi(DlgMultipleSelection)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), DlgMultipleSelection.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), DlgMultipleSelection.reject)
        QtCore.QMetaObject.connectSlotsByName(DlgMultipleSelection)

    def retranslateUi(self, DlgMultipleSelection):
        DlgMultipleSelection.setWindowTitle(QtGui.QApplication.translate("DlgMultipleSelection", "Multiple selection", None, QtGui.QApplication.UnicodeUTF8))

