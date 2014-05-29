# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '/home/tkralidi/work/foss4g/MetaSearch/MetaSearch/plugin/MetaSearch/ui/recorddialog.ui'
#
# Created: Thu Mar 20 21:56:35 2014
#      by: PyQt4 UI code generator 4.9.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_RecordDialog(object):
    def setupUi(self, RecordDialog):
        RecordDialog.setObjectName(_fromUtf8("RecordDialog"))
        RecordDialog.resize(600, 400)
        self.verticalLayout = QtGui.QVBoxLayout(RecordDialog)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.textMetadata = QtGui.QTextBrowser(RecordDialog)
        self.textMetadata.setLineWrapMode(QtGui.QTextEdit.NoWrap)
        self.textMetadata.setOpenExternalLinks(True)
        self.textMetadata.setObjectName(_fromUtf8("textMetadata"))
        self.verticalLayout.addWidget(self.textMetadata)
        self.buttonBox = QtGui.QDialogButtonBox(RecordDialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Close)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout.addWidget(self.buttonBox)

        self.retranslateUi(RecordDialog)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), RecordDialog.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), RecordDialog.reject)
        QtCore.QMetaObject.connectSlotsByName(RecordDialog)

    def retranslateUi(self, RecordDialog):
        RecordDialog.setWindowTitle(QtGui.QApplication.translate("RecordDialog", "Record Metadata", None, QtGui.QApplication.UnicodeUTF8))

