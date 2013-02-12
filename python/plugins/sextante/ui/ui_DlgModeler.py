# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DlgModeler.ui'
#
# Created: Thu Dec 13 18:41:08 2012
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

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
        self.inputsTree.header().setVisible(False)
        self.verticalLayout_2.addWidget(self.inputsTree)
        self.tabWidget.addTab(self.tab, _fromUtf8(""))
        self.tab_2 = QtGui.QWidget()
        self.tab_2.setObjectName(_fromUtf8("tab_2"))
        self.verticalLayout_3 = QtGui.QVBoxLayout(self.tab_2)
        self.verticalLayout_3.setSpacing(2)
        self.verticalLayout_3.setMargin(0)
        self.verticalLayout_3.setObjectName(_fromUtf8("verticalLayout_3"))
        try:
            from qgis.gui import QgsFilterLineEdit
            self.searchBox = QgsFilterLineEdit(self.tab_2)
        except ImportError:
            self.searchBox = QtGui.QLineEdit(self.tab_2)
        self.searchBox.setObjectName(_fromUtf8("searchBox"))
        self.verticalLayout_3.addWidget(self.searchBox)
        self.algorithmTree = QtGui.QTreeWidget(self.tab_2)
        self.algorithmTree.setAlternatingRowColors(True)
        self.algorithmTree.setObjectName(_fromUtf8("algorithmTree"))
        self.algorithmTree.header().setVisible(False)
        self.verticalLayout_3.addWidget(self.algorithmTree)
        self.tabWidget.addTab(self.tab_2, _fromUtf8(""))
        self.widget = QtGui.QWidget(self.splitter)
        self.widget.setObjectName(_fromUtf8("widget"))
        self.gridLayout = QtGui.QGridLayout(self.widget)
        self.gridLayout.setSpacing(2)
        self.gridLayout.setMargin(0)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.textName = QtGui.QLineEdit(self.widget)
        self.textName.setObjectName(_fromUtf8("textName"))
        self.gridLayout.addWidget(self.textName, 0, 0, 1, 1)
        self.textGroup = QtGui.QLineEdit(self.widget)
        self.textGroup.setObjectName(_fromUtf8("textGroup"))
        self.gridLayout.addWidget(self.textGroup, 0, 1, 1, 1)
        self.view = QtGui.QGraphicsView(self.widget)
        self.view.setObjectName(_fromUtf8("view"))
        self.gridLayout.addWidget(self.view, 1, 0, 1, 2)
        self.verticalLayout.addWidget(self.splitter)
        self.buttonBox = QtGui.QDialogButtonBox(DlgModeler)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Close)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout.addWidget(self.buttonBox)

        self.retranslateUi(DlgModeler)
        self.tabWidget.setCurrentIndex(0)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), DlgModeler.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), DlgModeler.reject)
        QtCore.QMetaObject.connectSlotsByName(DlgModeler)

    def retranslateUi(self, DlgModeler):
        DlgModeler.setWindowTitle(QtGui.QApplication.translate("DlgModeler", "SEXTANTE modeler", None, QtGui.QApplication.UnicodeUTF8))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab), QtGui.QApplication.translate("DlgModeler", "Inputs", None, QtGui.QApplication.UnicodeUTF8))
        self.searchBox.setToolTip(QtGui.QApplication.translate("DlgModeler", "Enter algorithm name to filter list", None, QtGui.QApplication.UnicodeUTF8))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_2), QtGui.QApplication.translate("DlgModeler", "Algorithms", None, QtGui.QApplication.UnicodeUTF8))
        self.textName.setToolTip(QtGui.QApplication.translate("DlgModeler", "Enter model name here", None, QtGui.QApplication.UnicodeUTF8))
        self.textGroup.setToolTip(QtGui.QApplication.translate("DlgModeler", "Enter group name here", None, QtGui.QApplication.UnicodeUTF8))


