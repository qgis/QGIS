# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ProcessingToolbox.ui'
#
# Created: Tue Aug 20 11:02:22 2013
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_ProcessingToolbox(object):
    def setupUi(self, ProcessingToolbox):
        ProcessingToolbox.setObjectName(_fromUtf8("ProcessingToolbox"))
        ProcessingToolbox.resize(289, 438)
        self.dockWidgetContents = QtGui.QWidget()
        self.dockWidgetContents.setObjectName(_fromUtf8("dockWidgetContents"))
        self.verticalLayout = QtGui.QVBoxLayout(self.dockWidgetContents)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.searchBox = QgsFilterLineEdit(self.dockWidgetContents)
        self.searchBox.setObjectName(_fromUtf8("searchBox"))
        self.verticalLayout.addWidget(self.searchBox)
        self.algorithmTree = QtGui.QTreeWidget(self.dockWidgetContents)
        self.algorithmTree.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.algorithmTree.setHeaderHidden(True)
        self.algorithmTree.setObjectName(_fromUtf8("algorithmTree"))
        self.algorithmTree.headerItem().setText(0, _fromUtf8("1"))
        self.verticalLayout.addWidget(self.algorithmTree)
        self.modeComboBox = QtGui.QComboBox(self.dockWidgetContents)
        self.modeComboBox.setObjectName(_fromUtf8("modeComboBox"))
        self.verticalLayout.addWidget(self.modeComboBox)
        ProcessingToolbox.setWidget(self.dockWidgetContents)

        self.retranslateUi(ProcessingToolbox)
        QtCore.QMetaObject.connectSlotsByName(ProcessingToolbox)

    def retranslateUi(self, ProcessingToolbox):
        ProcessingToolbox.setWindowTitle(QtGui.QApplication.translate("ProcessingToolbox", "Processing Toolbox", None, QtGui.QApplication.UnicodeUTF8))
        self.searchBox.setToolTip(QtGui.QApplication.translate("ProcessingToolbox", "Enter algorithm name to filter list", None, QtGui.QApplication.UnicodeUTF8))

from qgis.gui import QgsFilterLineEdit
