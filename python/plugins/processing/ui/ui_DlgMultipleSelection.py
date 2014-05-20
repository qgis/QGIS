# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DlgMultipleSelection.ui'
#
# Created: Tue May 20 13:40:43 2014
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

class Ui_DlgMultipleSelection(object):
    def setupUi(self, DlgMultipleSelection):
        DlgMultipleSelection.setObjectName(_fromUtf8("DlgMultipleSelection"))
        DlgMultipleSelection.resize(380, 320)
        self.horizontalLayout = QtGui.QHBoxLayout(DlgMultipleSelection)
        self.horizontalLayout.setSpacing(6)
        self.horizontalLayout.setMargin(9)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.lstLayers = QtGui.QListView(DlgMultipleSelection)
        self.lstLayers.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.lstLayers.setAlternatingRowColors(True)
        self.lstLayers.setSelectionMode(QtGui.QAbstractItemView.NoSelection)
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
        DlgMultipleSelection.setWindowTitle(_translate("DlgMultipleSelection", "Multiple selection", None))

