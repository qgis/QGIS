# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'gui.ui'
#
# Created: Thu Jan 10 13:11:48 2008
#      by: PyQt4 UI code generator 4.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_Dialog(object):
    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.resize(QtCore.QSize(QtCore.QRect(0,0,761,471).size()).expandedTo(Dialog.minimumSizeHint()))

        self.vboxlayout = QtGui.QVBoxLayout(Dialog)
        self.vboxlayout.setObjectName("vboxlayout")

        self.label = QtGui.QLabel(Dialog)
        self.label.setObjectName("label")
        self.vboxlayout.addWidget(self.label)

        self.groupBox = QtGui.QGroupBox(Dialog)
        self.groupBox.setObjectName("groupBox")

        self.vboxlayout1 = QtGui.QVBoxLayout(self.groupBox)
        self.vboxlayout1.setObjectName("vboxlayout1")

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")

        self.label_4 = QtGui.QLabel(self.groupBox)
        self.label_4.setObjectName("label_4")
        self.hboxlayout.addWidget(self.label_4)

        self.comboRepositories = QtGui.QComboBox(self.groupBox)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.comboRepositories.sizePolicy().hasHeightForWidth())
        self.comboRepositories.setSizePolicy(sizePolicy)
        self.comboRepositories.setObjectName("comboRepositories")
        self.hboxlayout.addWidget(self.comboRepositories)
        self.vboxlayout1.addLayout(self.hboxlayout)

        self.hboxlayout1 = QtGui.QHBoxLayout()
        self.hboxlayout1.setObjectName("hboxlayout1")

        self.buttonBrowse = QtGui.QPushButton(self.groupBox)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.buttonBrowse.sizePolicy().hasHeightForWidth())
        self.buttonBrowse.setSizePolicy(sizePolicy)
        self.buttonBrowse.setObjectName("buttonBrowse")
        self.hboxlayout1.addWidget(self.buttonBrowse)

        spacerItem = QtGui.QSpacerItem(40,20,QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Minimum)
        self.hboxlayout1.addItem(spacerItem)

        self.buttonAddRep = QtGui.QPushButton(self.groupBox)
        self.buttonAddRep.setObjectName("buttonAddRep")
        self.hboxlayout1.addWidget(self.buttonAddRep)

        self.buttonEditRep = QtGui.QPushButton(self.groupBox)
        self.buttonEditRep.setObjectName("buttonEditRep")
        self.hboxlayout1.addWidget(self.buttonEditRep)

        self.buttonDeleteRep = QtGui.QPushButton(self.groupBox)
        self.buttonDeleteRep.setObjectName("buttonDeleteRep")
        self.hboxlayout1.addWidget(self.buttonDeleteRep)
        self.vboxlayout1.addLayout(self.hboxlayout1)
        self.vboxlayout.addWidget(self.groupBox)

        self.treePlugins = QtGui.QTreeWidget(Dialog)
        self.treePlugins.setAlternatingRowColors(True)
        self.treePlugins.setRootIsDecorated(False)
        self.treePlugins.setItemsExpandable(False)
        self.treePlugins.setObjectName("treePlugins")
        self.vboxlayout.addWidget(self.treePlugins)

        self.hboxlayout2 = QtGui.QHBoxLayout()
        self.hboxlayout2.setObjectName("hboxlayout2")

        self.label_2 = QtGui.QLabel(Dialog)
        self.label_2.setObjectName("label_2")
        self.hboxlayout2.addWidget(self.label_2)

        self.linePlugin = QtGui.QLineEdit(Dialog)
        self.linePlugin.setObjectName("linePlugin")
        self.hboxlayout2.addWidget(self.linePlugin)

        self.pbnOK = QtGui.QPushButton(Dialog)
        self.pbnOK.setObjectName("pbnOK")
        self.hboxlayout2.addWidget(self.pbnOK)
        self.vboxlayout.addLayout(self.hboxlayout2)

        self.hboxlayout3 = QtGui.QHBoxLayout()
        self.hboxlayout3.setObjectName("hboxlayout3")

        self.label_3 = QtGui.QLabel(Dialog)
        self.label_3.setObjectName("label_3")
        self.hboxlayout3.addWidget(self.label_3)

        self.pbnCancel = QtGui.QPushButton(Dialog)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pbnCancel.sizePolicy().hasHeightForWidth())
        self.pbnCancel.setSizePolicy(sizePolicy)
        self.pbnCancel.setObjectName("pbnCancel")
        self.hboxlayout3.addWidget(self.pbnCancel)
        self.vboxlayout.addLayout(self.hboxlayout3)

        self.retranslateUi(Dialog)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(QtGui.QApplication.translate("Dialog", "QGIS Plugin Installer", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("Dialog", "Select repository, retrieve the list of available plugins, select one and install it", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox.setTitle(QtGui.QApplication.translate("Dialog", "Repository", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("Dialog", "Active repository:", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonBrowse.setText(QtGui.QApplication.translate("Dialog", "Get List", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonAddRep.setText(QtGui.QApplication.translate("Dialog", "Add", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonEditRep.setText(QtGui.QApplication.translate("Dialog", "Edit", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonDeleteRep.setText(QtGui.QApplication.translate("Dialog", "Delete", None, QtGui.QApplication.UnicodeUTF8))
        self.treePlugins.headerItem().setText(0,QtGui.QApplication.translate("Dialog", "Name", None, QtGui.QApplication.UnicodeUTF8))
        self.treePlugins.headerItem().setText(1,QtGui.QApplication.translate("Dialog", "Version", None, QtGui.QApplication.UnicodeUTF8))
        self.treePlugins.headerItem().setText(2,QtGui.QApplication.translate("Dialog", "Description", None, QtGui.QApplication.UnicodeUTF8))
        self.treePlugins.headerItem().setText(3,QtGui.QApplication.translate("Dialog", "Author", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("Dialog", "Name of plugin to install", None, QtGui.QApplication.UnicodeUTF8))
        self.pbnOK.setText(QtGui.QApplication.translate("Dialog", "Install Plugin", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("Dialog", "The plugin will be installed to ~/.qgis/python/plugins", None, QtGui.QApplication.UnicodeUTF8))
        self.pbnCancel.setText(QtGui.QApplication.translate("Dialog", "Done", None, QtGui.QApplication.UnicodeUTF8))

