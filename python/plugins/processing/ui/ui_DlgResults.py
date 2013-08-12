# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'D:/src/qgis/python/plugins/sextante/ui/DlgResults.ui'
#
# Created: Tue Jul 16 01:17:48 2013
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_DlgResults(object):
    def setupUi(self, DlgResults):
        DlgResults.setObjectName(_fromUtf8("DlgResults"))
        DlgResults.resize(623, 515)
        self.verticalLayout = QtGui.QVBoxLayout(DlgResults)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
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
        DlgResults.setWindowTitle(QtGui.QApplication.translate("DlgResults", "Results", None, QtGui.QApplication.UnicodeUTF8))

from PyQt4 import QtWebKit
