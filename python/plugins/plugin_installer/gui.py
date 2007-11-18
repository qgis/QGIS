# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'gui.ui'
#
# Created: Sat Nov 17 14:57:42 2007
#      by: PyQt4 UI code generator 4.1.1
#
# WARNING! All changes made in this file will be lost!

import sys
from PyQt4 import QtCore, QtGui

class Ui_Dialog(object):
    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.resize(QtCore.QSize(QtCore.QRect(0,0,937,420).size()).expandedTo(Dialog.minimumSizeHint()))

        self.label = QtGui.QLabel(Dialog)
        self.label.setGeometry(QtCore.QRect(10,20,501,21))
        self.label.setObjectName("label")

        self.label_2 = QtGui.QLabel(Dialog)
        self.label_2.setGeometry(QtCore.QRect(120,320,191,21))
        self.label_2.setObjectName("label_2")

        self.linePlugin = QtGui.QLineEdit(Dialog)
        self.linePlugin.setGeometry(QtCore.QRect(290,320,281,25))
        self.linePlugin.setObjectName("linePlugin")

        self.buttonBrowse = QtGui.QPushButton(Dialog)
        self.buttonBrowse.setGeometry(QtCore.QRect(830,10,83,28))
        self.buttonBrowse.setObjectName("buttonBrowse")

        self.pbnCancel = QtGui.QPushButton(Dialog)
        self.pbnCancel.setGeometry(QtCore.QRect(830,370,83,28))
        self.pbnCancel.setObjectName("pbnCancel")

        self.pbnOK = QtGui.QPushButton(Dialog)
        self.pbnOK.setGeometry(QtCore.QRect(590,320,83,28))
        self.pbnOK.setObjectName("pbnOK")

        self.label_3 = QtGui.QLabel(Dialog)
        self.label_3.setGeometry(QtCore.QRect(120,360,501,21))
        self.label_3.setObjectName("label_3")

        self.treePlugins = QtGui.QTreeWidget(Dialog)
        self.treePlugins.setGeometry(QtCore.QRect(10,50,911,251))
        self.treePlugins.setAlternatingRowColors(True)
        self.treePlugins.setItemsExpandable(False)
        self.treePlugins.setObjectName("treePlugins")

        self.retranslateUi(Dialog)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(QtGui.QApplication.translate("Dialog", "QGIS Plugin Installer", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("Dialog", "Retrieve the list of available plugins, select one and install it", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("Dialog", "Name of plugin to install", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonBrowse.setText(QtGui.QApplication.translate("Dialog", "Get List", None, QtGui.QApplication.UnicodeUTF8))
        self.pbnCancel.setText(QtGui.QApplication.translate("Dialog", "Done", None, QtGui.QApplication.UnicodeUTF8))
        self.pbnOK.setText(QtGui.QApplication.translate("Dialog", "Install Plugin", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("Dialog", "The plugin will be installed to ~/.qgis/python/plugins", None, QtGui.QApplication.UnicodeUTF8))
        self.treePlugins.headerItem().setText(0,QtGui.QApplication.translate("Dialog", "Name", None, QtGui.QApplication.UnicodeUTF8))
        self.treePlugins.headerItem().setText(1,QtGui.QApplication.translate("Dialog", "Version", None, QtGui.QApplication.UnicodeUTF8))
        self.treePlugins.headerItem().setText(2,QtGui.QApplication.translate("Dialog", "Description", None, QtGui.QApplication.UnicodeUTF8))
        self.treePlugins.headerItem().setText(3,QtGui.QApplication.translate("Dialog", "Author", None, QtGui.QApplication.UnicodeUTF8))

