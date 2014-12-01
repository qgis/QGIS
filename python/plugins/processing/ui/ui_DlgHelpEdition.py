# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DlgHelpEdition.ui'
#
# Created: Fri Nov 21 13:25:47 2014
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

class Ui_DlgHelpEdition(object):
    def setupUi(self, DlgHelpEdition):
        DlgHelpEdition.setObjectName(_fromUtf8("DlgHelpEdition"))
        DlgHelpEdition.resize(600, 460)
        self.verticalLayout_3 = QtGui.QVBoxLayout(DlgHelpEdition)
        self.verticalLayout_3.setSpacing(6)
        self.verticalLayout_3.setMargin(9)
        self.verticalLayout_3.setObjectName(_fromUtf8("verticalLayout_3"))
        self.splitter_2 = QtGui.QSplitter(DlgHelpEdition)
        self.splitter_2.setOrientation(QtCore.Qt.Vertical)
        self.splitter_2.setObjectName(_fromUtf8("splitter_2"))
        self.webView = QtWebKit.QWebView(self.splitter_2)
        self.webView.setUrl(QtCore.QUrl(_fromUtf8("about:blank")))
        self.webView.setObjectName(_fromUtf8("webView"))
        self.splitter = QtGui.QSplitter(self.splitter_2)
        self.splitter.setOrientation(QtCore.Qt.Horizontal)
        self.splitter.setObjectName(_fromUtf8("splitter"))
        self.layoutWidget = QtGui.QWidget(self.splitter)
        self.layoutWidget.setObjectName(_fromUtf8("layoutWidget"))
        self.verticalLayout = QtGui.QVBoxLayout(self.layoutWidget)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.label = QtGui.QLabel(self.layoutWidget)
        self.label.setObjectName(_fromUtf8("label"))
        self.verticalLayout.addWidget(self.label)
        self.tree = QtGui.QTreeWidget(self.layoutWidget)
        self.tree.setMinimumSize(QtCore.QSize(0, 200))
        self.tree.setAlternatingRowColors(True)
        self.tree.setObjectName(_fromUtf8("tree"))
        self.tree.headerItem().setText(0, _fromUtf8("1"))
        self.tree.header().setVisible(False)
        self.verticalLayout.addWidget(self.tree)
        self.layoutWidget1 = QtGui.QWidget(self.splitter)
        self.layoutWidget1.setObjectName(_fromUtf8("layoutWidget1"))
        self.verticalLayout_2 = QtGui.QVBoxLayout(self.layoutWidget1)
        self.verticalLayout_2.setSpacing(2)
        self.verticalLayout_2.setMargin(0)
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.lblDescription = QtGui.QLabel(self.layoutWidget1)
        self.lblDescription.setObjectName(_fromUtf8("lblDescription"))
        self.verticalLayout_2.addWidget(self.lblDescription)
        self.text = QtGui.QTextEdit(self.layoutWidget1)
        self.text.setMinimumSize(QtCore.QSize(0, 200))
        self.text.setObjectName(_fromUtf8("text"))
        self.verticalLayout_2.addWidget(self.text)
        self.verticalLayout_3.addWidget(self.splitter_2)
        self.buttonBox = QtGui.QDialogButtonBox(DlgHelpEdition)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout_3.addWidget(self.buttonBox)

        self.retranslateUi(DlgHelpEdition)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), DlgHelpEdition.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), DlgHelpEdition.reject)
        QtCore.QMetaObject.connectSlotsByName(DlgHelpEdition)

    def retranslateUi(self, DlgHelpEdition):
        DlgHelpEdition.setWindowTitle(_translate("DlgHelpEdition", "Help editor", None))
        self.label.setText(_translate("DlgHelpEdition", "Select element to edit", None))
        self.lblDescription.setText(_translate("DlgHelpEdition", "Element description", None))

from PyQt4 import QtWebKit
