# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DlgResults.ui'
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

class Ui_DlgResults(object):
    def setupUi(self, DlgResults):
        DlgResults.setObjectName(_fromUtf8("DlgResults"))
        DlgResults.resize(623, 515)
        self.verticalLayout = QtGui.QVBoxLayout(DlgResults)
        self.verticalLayout.setSpacing(6)
        self.verticalLayout.setMargin(9)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.splitter = QtGui.QSplitter(DlgResults)
        self.splitter.setOrientation(QtCore.Qt.Horizontal)
        self.splitter.setObjectName(_fromUtf8("splitter"))
        self.tree = QtGui.QTreeWidget(self.splitter)
        self.tree.setMinimumSize(QtCore.QSize(0, 0))
        self.tree.setObjectName(_fromUtf8("tree"))
        self.tree.headerItem().setText(0, _fromUtf8("1"))
        self.tree.header().setVisible(False)
        self.webView = QtWebKit.QWebView(self.splitter)
        self.webView.setMinimumSize(QtCore.QSize(0, 0))
        self.webView.setUrl(QtCore.QUrl(_fromUtf8("about:blank")))
        self.webView.setObjectName(_fromUtf8("webView"))
        self.verticalLayout.addWidget(self.splitter)
        self.buttonBox = QtGui.QDialogButtonBox(DlgResults)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Close)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout.addWidget(self.buttonBox)

        self.retranslateUi(DlgResults)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), DlgResults.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), DlgResults.reject)
        QtCore.QMetaObject.connectSlotsByName(DlgResults)

    def retranslateUi(self, DlgResults):
        DlgResults.setWindowTitle(_translate("DlgResults", "Results", None))

from PyQt4 import QtWebKit
