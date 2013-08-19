# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'D:/src/qgis/python/plugins/sextante/ui/DlgHelpEdition.ui'
#
# Created: Tue Jul 16 01:17:49 2013
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_DlgHelpEdition(object):
    def setupUi(self, DlgHelpEdition):
        DlgHelpEdition.setObjectName(_fromUtf8("DlgHelpEdition"))
        DlgHelpEdition.resize(600, 460)
        self.verticalLayout_3 = QtGui.QVBoxLayout(DlgHelpEdition)
        self.verticalLayout_3.setSpacing(2)
        self.verticalLayout_3.setMargin(0)
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
        self.widget = QtGui.QWidget(self.splitter)
        self.widget.setObjectName(_fromUtf8("widget"))
        self.verticalLayout = QtGui.QVBoxLayout(self.widget)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.label = QtGui.QLabel(self.widget)
        self.label.setObjectName(_fromUtf8("label"))
        self.verticalLayout.addWidget(self.label)
        self.tree = QtGui.QTreeWidget(self.widget)
        self.tree.setMinimumSize(QtCore.QSize(0, 200))
        self.tree.setAlternatingRowColors(True)
        self.tree.setObjectName(_fromUtf8("tree"))
        self.tree.headerItem().setText(0, _fromUtf8("1"))
        self.tree.header().setVisible(False)
        self.verticalLayout.addWidget(self.tree)
        self.widget1 = QtGui.QWidget(self.splitter)
        self.widget1.setObjectName(_fromUtf8("widget1"))
        self.verticalLayout_2 = QtGui.QVBoxLayout(self.widget1)
        self.verticalLayout_2.setSpacing(2)
        self.verticalLayout_2.setMargin(0)
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.lblDescription = QtGui.QLabel(self.widget1)
        self.lblDescription.setObjectName(_fromUtf8("lblDescription"))
        self.verticalLayout_2.addWidget(self.lblDescription)
        self.text = QtGui.QTextEdit(self.widget1)
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
        DlgHelpEdition.setWindowTitle(QtGui.QApplication.translate("DlgHelpEdition", "Help editor", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("DlgHelpEdition", "Select element to edit", None, QtGui.QApplication.UnicodeUTF8))
        self.lblDescription.setText(QtGui.QApplication.translate("DlgHelpEdition", "Element description", None, QtGui.QApplication.UnicodeUTF8))

from PyQt4 import QtWebKit
