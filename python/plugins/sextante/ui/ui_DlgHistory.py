# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DlgHistory.ui'
#
# Created: Tue Dec 04 00:32:20 2012
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_DlgHistory(object):
    def setupUi(self, DlgHistory):
        DlgHistory.setObjectName(_fromUtf8("DlgHistory"))
        DlgHistory.resize(800, 600)
        self.verticalLayout = QtGui.QVBoxLayout(DlgHistory)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.splitter = QtGui.QSplitter(DlgHistory)
        self.splitter.setOrientation(QtCore.Qt.Vertical)
        self.splitter.setObjectName(_fromUtf8("splitter"))
        self.tree = QtGui.QTreeWidget(self.splitter)
        self.tree.setObjectName(_fromUtf8("tree"))
        self.tree.headerItem().setText(0, _fromUtf8("1"))
        self.tree.header().setVisible(False)
        self.tree.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.text = QtGui.QTextEdit(self.splitter)
        self.text.setReadOnly(True)
        self.text.setObjectName(_fromUtf8("text"))
        self.verticalLayout.addWidget(self.splitter)
        self.buttonBox = QtGui.QDialogButtonBox(DlgHistory)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Close)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout.addWidget(self.buttonBox)

        self.retranslateUi(DlgHistory)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), DlgHistory.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), DlgHistory.reject)
        QtCore.QMetaObject.connectSlotsByName(DlgHistory)

    def retranslateUi(self, DlgHistory):
        DlgHistory.setWindowTitle(QtGui.QApplication.translate("DlgHistory", "History and log", None, QtGui.QApplication.UnicodeUTF8))

