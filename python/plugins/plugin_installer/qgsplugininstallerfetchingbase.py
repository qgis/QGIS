# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'qgsplugininstallerfetchingbase.ui'
#
# Created: Wed Nov 12 23:21:49 2008
#      by: PyQt4 UI code generator 4.3.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_QgsPluginInstallerFetchingDialogBase(object):
    def setupUi(self, QgsPluginInstallerFetchingDialogBase):
        QgsPluginInstallerFetchingDialogBase.setObjectName("QgsPluginInstallerFetchingDialogBase")
        QgsPluginInstallerFetchingDialogBase.resize(QtCore.QSize(QtCore.QRect(0,0,521,332).size()).expandedTo(QgsPluginInstallerFetchingDialogBase.minimumSizeHint()))
        QgsPluginInstallerFetchingDialogBase.setWindowIcon(QtGui.QIcon(":/plugins/installer/qgis-icon.png"))

        self.gridlayout = QtGui.QGridLayout(QgsPluginInstallerFetchingDialogBase)
        self.gridlayout.setObjectName("gridlayout")

        spacerItem = QtGui.QSpacerItem(249,10,QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Fixed)
        self.gridlayout.addItem(spacerItem,1,0,1,1)

        self.label1 = QtGui.QLabel(QgsPluginInstallerFetchingDialogBase)
        self.label1.setObjectName("label1")
        self.gridlayout.addWidget(self.label1,2,0,1,1)

        self.progressBar = QtGui.QProgressBar(QgsPluginInstallerFetchingDialogBase)
        self.progressBar.setProperty("value",QtCore.QVariant(24))
        self.progressBar.setAlignment(QtCore.Qt.AlignHCenter)
        self.progressBar.setTextDirection(QtGui.QProgressBar.TopToBottom)
        self.progressBar.setObjectName("progressBar")
        self.gridlayout.addWidget(self.progressBar,3,0,1,1)

        spacerItem1 = QtGui.QSpacerItem(248,10,QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Fixed)
        self.gridlayout.addItem(spacerItem1,4,0,1,1)

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")

        spacerItem2 = QtGui.QSpacerItem(140,27,QtGui.QSizePolicy.MinimumExpanding,QtGui.QSizePolicy.Minimum)
        self.hboxlayout.addItem(spacerItem2)

        self.buttonSkip = QtGui.QPushButton(QgsPluginInstallerFetchingDialogBase)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.buttonSkip.sizePolicy().hasHeightForWidth())
        self.buttonSkip.setSizePolicy(sizePolicy)
        self.buttonSkip.setMinimumSize(QtCore.QSize(180,0))
        self.buttonSkip.setFocusPolicy(QtCore.Qt.NoFocus)
        self.buttonSkip.setAutoDefault(False)
        self.buttonSkip.setDefault(False)
        self.buttonSkip.setFlat(False)
        self.buttonSkip.setObjectName("buttonSkip")
        self.hboxlayout.addWidget(self.buttonSkip)

        spacerItem3 = QtGui.QSpacerItem(140,27,QtGui.QSizePolicy.MinimumExpanding,QtGui.QSizePolicy.Minimum)
        self.hboxlayout.addItem(spacerItem3)
        self.gridlayout.addLayout(self.hboxlayout,5,0,1,1)

        self.treeWidget = QtGui.QTreeWidget(QgsPluginInstallerFetchingDialogBase)
        self.treeWidget.setEnabled(True)
        self.treeWidget.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.treeWidget.setProperty("showDropIndicator",QtCore.QVariant(False))
        self.treeWidget.setSelectionMode(QtGui.QAbstractItemView.NoSelection)
        self.treeWidget.setHorizontalScrollMode(QtGui.QAbstractItemView.ScrollPerItem)
        self.treeWidget.setRootIsDecorated(False)
        self.treeWidget.setItemsExpandable(False)
        self.treeWidget.setObjectName("treeWidget")
        self.gridlayout.addWidget(self.treeWidget,0,0,1,1)

        self.retranslateUi(QgsPluginInstallerFetchingDialogBase)
        QtCore.QObject.connect(self.buttonSkip,QtCore.SIGNAL("clicked()"),QgsPluginInstallerFetchingDialogBase.reject)
        QtCore.QMetaObject.connectSlotsByName(QgsPluginInstallerFetchingDialogBase)

    def retranslateUi(self, QgsPluginInstallerFetchingDialogBase):
        QgsPluginInstallerFetchingDialogBase.setWindowTitle(QtGui.QApplication.translate("QgsPluginInstallerFetchingDialogBase", "Fetching repositories", None, QtGui.QApplication.UnicodeUTF8))
        self.label1.setText(QtGui.QApplication.translate("QgsPluginInstallerFetchingDialogBase", "Overall progress:", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonSkip.setText(QtGui.QApplication.translate("QgsPluginInstallerFetchingDialogBase", "Abort fetching", None, QtGui.QApplication.UnicodeUTF8))
        self.treeWidget.headerItem().setText(0,QtGui.QApplication.translate("QgsPluginInstallerFetchingDialogBase", "Repository", None, QtGui.QApplication.UnicodeUTF8))
        self.treeWidget.headerItem().setText(1,QtGui.QApplication.translate("QgsPluginInstallerFetchingDialogBase", "State", None, QtGui.QApplication.UnicodeUTF8))

import resources_rc
