# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DlgGetScriptsAndModels.ui'
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

class Ui_DlgGetScriptsAndModels(object):
    def setupUi(self, DlgGetScriptsAndModels):
        DlgGetScriptsAndModels.setObjectName(_fromUtf8("DlgGetScriptsAndModels"))
        DlgGetScriptsAndModels.resize(826, 520)
        self.verticalLayout = QtGui.QVBoxLayout(DlgGetScriptsAndModels)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.splitter = QtGui.QSplitter(DlgGetScriptsAndModels)
        self.splitter.setOrientation(QtCore.Qt.Horizontal)
        self.splitter.setObjectName(_fromUtf8("splitter"))
        self.tree = QtGui.QTreeWidget(self.splitter)
        self.tree.setMinimumSize(QtCore.QSize(350, 0))
        self.tree.setMaximumSize(QtCore.QSize(100000, 100000))
        self.tree.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)
        self.tree.setObjectName(_fromUtf8("tree"))
        self.tree.headerItem().setText(0, _fromUtf8("1"))
        self.tree.header().setVisible(False)
        self.tree.header().setCascadingSectionResizes(False)
        self.tree.header().setDefaultSectionSize(350)
        self.tree.header().setHighlightSections(False)
        self.tree.header().setSortIndicatorShown(True)
        self.tree.header().setStretchLastSection(True)
        self.frame = QtGui.QFrame(self.splitter)
        self.frame.setFrameShape(QtGui.QFrame.StyledPanel)
        self.frame.setFrameShadow(QtGui.QFrame.Sunken)
        self.frame.setObjectName(_fromUtf8("frame"))
        self.horizontalLayout = QtGui.QHBoxLayout(self.frame)
        self.horizontalLayout.setSpacing(0)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.webView = QtWebKit.QWebView(self.frame)
        self.webView.setMaximumSize(QtCore.QSize(10000, 10000))
        self.webView.setUrl(QtCore.QUrl(_fromUtf8("about:blank")))
        self.webView.setObjectName(_fromUtf8("webView"))
        self.horizontalLayout.addWidget(self.webView)
        self.verticalLayout.addWidget(self.splitter)
        self.horizontalLayout_2 = QtGui.QHBoxLayout()
        self.horizontalLayout_2.setObjectName(_fromUtf8("horizontalLayout_2"))
        self.progressBar = QtGui.QProgressBar(DlgGetScriptsAndModels)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.MinimumExpanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.progressBar.sizePolicy().hasHeightForWidth())
        self.progressBar.setSizePolicy(sizePolicy)
        self.progressBar.setMinimumSize(QtCore.QSize(0, 0))
        self.progressBar.setProperty("value", 0)
        self.progressBar.setInvertedAppearance(False)
        self.progressBar.setObjectName(_fromUtf8("progressBar"))
        self.horizontalLayout_2.addWidget(self.progressBar)
        self.buttonBox = QtGui.QDialogButtonBox(DlgGetScriptsAndModels)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.MinimumExpanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.buttonBox.sizePolicy().hasHeightForWidth())
        self.buttonBox.setSizePolicy(sizePolicy)
        self.buttonBox.setMaximumSize(QtCore.QSize(200, 16777215))
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.horizontalLayout_2.addWidget(self.buttonBox)
        self.verticalLayout.addLayout(self.horizontalLayout_2)

        self.retranslateUi(DlgGetScriptsAndModels)
        QtCore.QMetaObject.connectSlotsByName(DlgGetScriptsAndModels)

    def retranslateUi(self, DlgGetScriptsAndModels):
        DlgGetScriptsAndModels.setWindowTitle(_translate("DlgGetScriptsAndModels", "Get scripts and models", None))

from PyQt4 import QtWebKit
