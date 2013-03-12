# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'SextanteToolbox.ui'
#
# Created: Tue Dec 04 00:33:09 2012
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_SextanteToolbox(object):
    def setupUi(self, SextanteToolbox):
        SextanteToolbox.setObjectName(_fromUtf8("SextanteToolbox"))
        SextanteToolbox.resize(225, 444)
        self.dockWidgetContents = QtGui.QWidget()
        self.dockWidgetContents.setObjectName(_fromUtf8("dockWidgetContents"))
        self.verticalLayout = QtGui.QVBoxLayout(self.dockWidgetContents)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.externalAppsButton = QtGui.QPushButton(self.dockWidgetContents)
        self.externalAppsButton.setObjectName(_fromUtf8("externalAppsButton"))
        self.verticalLayout.addWidget(self.externalAppsButton)
        try:
            from qgis.gui import QgsFilterLineEdit
            self.searchBox = QgsFilterLineEdit(self.dockWidgetContents)
        except ImportError:
            self.searchBox = QtGui.QLineEdit(self.dockWidgetContents)
        self.searchBox.setObjectName(_fromUtf8("searchBox"))
        self.verticalLayout.addWidget(self.searchBox)
        self.algorithmTree = QtGui.QTreeWidget(self.dockWidgetContents)
        self.algorithmTree.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.algorithmTree.setHeaderHidden(True)
        self.algorithmTree.setObjectName(_fromUtf8("algorithmTree"))
        self.algorithmTree.headerItem().setText(0, _fromUtf8("1"))
        self.verticalLayout.addWidget(self.algorithmTree)
        SextanteToolbox.setWidget(self.dockWidgetContents)

        self.retranslateUi(SextanteToolbox)
        QtCore.QMetaObject.connectSlotsByName(SextanteToolbox)

    def retranslateUi(self, SextanteToolbox):
        SextanteToolbox.setWindowTitle(QtGui.QApplication.translate("SextanteToolbox", "SEXTANTE Toolbox", None, QtGui.QApplication.UnicodeUTF8))
        self.externalAppsButton.setText(QtGui.QApplication.translate("SextanteToolbox", "Click here to learn\n"
"more about SEXTANTE!", None, QtGui.QApplication.UnicodeUTF8))
        self.searchBox.setToolTip(QtGui.QApplication.translate("SextanteToolbox", "Enter algorithm name to filter list", None, QtGui.QApplication.UnicodeUTF8))

