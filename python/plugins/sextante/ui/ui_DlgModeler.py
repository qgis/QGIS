# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DlgModeler.ui'
#
# Created: Thu Mar 28 15:45:09 2013
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

class Ui_DlgModeler(object):
    def setupUi(self, DlgModeler):
        DlgModeler.setObjectName(_fromUtf8("DlgModeler"))
        DlgModeler.resize(1000, 600)
        self.verticalLayout = QtGui.QVBoxLayout(DlgModeler)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.splitter = QtGui.QSplitter(DlgModeler)
        self.splitter.setOrientation(QtCore.Qt.Horizontal)
        self.splitter.setObjectName(_fromUtf8("splitter"))
        self.tabWidget = QtGui.QTabWidget(self.splitter)
        self.tabWidget.setMinimumSize(QtCore.QSize(300, 0))
        self.tabWidget.setObjectName(_fromUtf8("tabWidget"))
        self.tab = QtGui.QWidget()
        self.tab.setObjectName(_fromUtf8("tab"))
        self.verticalLayout_2 = QtGui.QVBoxLayout(self.tab)
        self.verticalLayout_2.setSpacing(2)
        self.verticalLayout_2.setMargin(0)
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.inputsTree = QtGui.QTreeWidget(self.tab)
        self.inputsTree.setAlternatingRowColors(True)
        self.inputsTree.setObjectName(_fromUtf8("inputsTree"))
        self.inputsTree.headerItem().setText(0, _fromUtf8("1"))
        self.inputsTree.header().setVisible(False)
        self.verticalLayout_2.addWidget(self.inputsTree)
        self.tabWidget.addTab(self.tab, _fromUtf8(""))
        self.tab_2 = QtGui.QWidget()
        self.tab_2.setObjectName(_fromUtf8("tab_2"))
        self.verticalLayout_3 = QtGui.QVBoxLayout(self.tab_2)
        self.verticalLayout_3.setSpacing(2)
        self.verticalLayout_3.setMargin(0)
        self.verticalLayout_3.setObjectName(_fromUtf8("verticalLayout_3"))
        self.searchBox = QgsFilterLineEdit(self.tab_2)
        self.searchBox.setObjectName(_fromUtf8("searchBox"))
        self.verticalLayout_3.addWidget(self.searchBox)
        self.algorithmTree = QtGui.QTreeWidget(self.tab_2)
        self.algorithmTree.setAlternatingRowColors(True)
        self.algorithmTree.setObjectName(_fromUtf8("algorithmTree"))
        self.algorithmTree.headerItem().setText(0, _fromUtf8("1"))
        self.algorithmTree.header().setVisible(False)
        self.verticalLayout_3.addWidget(self.algorithmTree)
        self.tabWidget.addTab(self.tab_2, _fromUtf8(""))
        self.layoutWidget = QtGui.QWidget(self.splitter)
        self.layoutWidget.setObjectName(_fromUtf8("layoutWidget"))
        self.gridLayout = QtGui.QGridLayout(self.layoutWidget)
        self.gridLayout.setSpacing(2)
        self.gridLayout.setMargin(0)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.textName = QtGui.QLineEdit(self.layoutWidget)
        self.textName.setObjectName(_fromUtf8("textName"))
        self.gridLayout.addWidget(self.textName, 0, 0, 1, 1)
        self.textGroup = QtGui.QLineEdit(self.layoutWidget)
        self.textGroup.setObjectName(_fromUtf8("textGroup"))
        self.gridLayout.addWidget(self.textGroup, 0, 1, 1, 1)
        self.view = QtGui.QGraphicsView(self.layoutWidget)
        self.view.setObjectName(_fromUtf8("view"))
        self.gridLayout.addWidget(self.view, 1, 0, 1, 2)
        self.verticalLayout.addWidget(self.splitter)
        self.buttonBox = QtGui.QDialogButtonBox(DlgModeler)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Close)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout.addWidget(self.buttonBox)

        self.retranslateUi(DlgModeler)
        self.tabWidget.setCurrentIndex(1)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), DlgModeler.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), DlgModeler.reject)
        QtCore.QMetaObject.connectSlotsByName(DlgModeler)

    def retranslateUi(self, DlgModeler):
        DlgModeler.setWindowTitle(_translate("DlgModeler", "SEXTANTE modeler", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab), _translate("DlgModeler", "Inputs", None))
        self.searchBox.setToolTip(_translate("DlgModeler", "Enter algorithm name to filter list", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_2), _translate("DlgModeler", "Algorithms", None))
        self.textName.setToolTip(_translate("DlgModeler", "Enter model name here", None))
        self.textGroup.setToolTip(_translate("DlgModeler", "Enter group name here", None))

from qgis.gui import QgsFilterLineEdit
