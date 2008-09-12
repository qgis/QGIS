# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'installingbase.ui'
#
# Created: Fri Sep 12 19:21:37 2008
#      by: PyQt4 UI code generator 4.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_QgsPluginInstallerInstallingDialog(object):
    def setupUi(self, QgsPluginInstallerInstallingDialog):
        QgsPluginInstallerInstallingDialog.setObjectName("QgsPluginInstallerInstallingDialog")
        QgsPluginInstallerInstallingDialog.resize(QtCore.QSize(QtCore.QRect(0,0,520,175).size()).expandedTo(QgsPluginInstallerInstallingDialog.minimumSizeHint()))
        QgsPluginInstallerInstallingDialog.setWindowIcon(QtGui.QIcon(":/plugins/installer/qgis-icon.png"))

        self.gridlayout = QtGui.QGridLayout(QgsPluginInstallerInstallingDialog)
        self.gridlayout.setObjectName("gridlayout")

        spacerItem = QtGui.QSpacerItem(502,16,QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.MinimumExpanding)
        self.gridlayout.addItem(spacerItem,0,0,1,1)

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")

        self.label = QtGui.QLabel(QgsPluginInstallerInstallingDialog)
        self.label.setObjectName("label")
        self.hboxlayout.addWidget(self.label)

        self.labelName = QtGui.QLabel(QgsPluginInstallerInstallingDialog)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.labelName.sizePolicy().hasHeightForWidth())
        self.labelName.setSizePolicy(sizePolicy)
        self.labelName.setObjectName("labelName")
        self.hboxlayout.addWidget(self.labelName)
        self.gridlayout.addLayout(self.hboxlayout,1,0,1,1)

        self.labelState = QtGui.QLabel(QgsPluginInstallerInstallingDialog)
        self.labelState.setObjectName("labelState")
        self.gridlayout.addWidget(self.labelState,2,0,1,1)

        self.progressBar = QtGui.QProgressBar(QgsPluginInstallerInstallingDialog)
        self.progressBar.setMaximum(100)
        self.progressBar.setProperty("value",QtCore.QVariant(0))
        self.progressBar.setAlignment(QtCore.Qt.AlignHCenter)
        self.progressBar.setTextDirection(QtGui.QProgressBar.TopToBottom)
        self.progressBar.setObjectName("progressBar")
        self.gridlayout.addWidget(self.progressBar,3,0,1,1)

        spacerItem1 = QtGui.QSpacerItem(502,16,QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Fixed)
        self.gridlayout.addItem(spacerItem1,4,0,1,1)

        self.buttonBox = QtGui.QDialogButtonBox(QgsPluginInstallerInstallingDialog)
        self.buttonBox.setFocusPolicy(QtCore.Qt.NoFocus)
        self.buttonBox.setContextMenuPolicy(QtCore.Qt.NoContextMenu)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Abort)
        self.buttonBox.setCenterButtons(True)
        self.buttonBox.setObjectName("buttonBox")
        self.gridlayout.addWidget(self.buttonBox,5,0,1,1)

        self.retranslateUi(QgsPluginInstallerInstallingDialog)
        QtCore.QMetaObject.connectSlotsByName(QgsPluginInstallerInstallingDialog)

    def retranslateUi(self, QgsPluginInstallerInstallingDialog):
        QgsPluginInstallerInstallingDialog.setWindowTitle(QtGui.QApplication.translate("QgsPluginInstallerInstallingDialog", "QGIS Python Plugin Installer", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("QgsPluginInstallerInstallingDialog", "Installing plugin:", None, QtGui.QApplication.UnicodeUTF8))
        self.labelState.setText(QtGui.QApplication.translate("QgsPluginInstallerInstallingDialog", "Connecting...", None, QtGui.QApplication.UnicodeUTF8))

import resources_rc
