# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'pluginerrorbase.ui'
#
# Created: Fri Sep 12 19:21:37 2008
#      by: PyQt4 UI code generator 4.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_QgsPluginInstallerPluginErrorDialog(object):
    def setupUi(self, QgsPluginInstallerPluginErrorDialog):
        QgsPluginInstallerPluginErrorDialog.setObjectName("QgsPluginInstallerPluginErrorDialog")
        QgsPluginInstallerPluginErrorDialog.resize(QtCore.QSize(QtCore.QRect(0,0,521,383).size()).expandedTo(QgsPluginInstallerPluginErrorDialog.minimumSizeHint()))
        QgsPluginInstallerPluginErrorDialog.setMinimumSize(QtCore.QSize(480,300))
        QgsPluginInstallerPluginErrorDialog.setWindowIcon(QtGui.QIcon(":/plugins/installer/qgis-icon.png"))

        self.gridlayout = QtGui.QGridLayout(QgsPluginInstallerPluginErrorDialog)
        self.gridlayout.setObjectName("gridlayout")

        self.label = QtGui.QLabel(QgsPluginInstallerPluginErrorDialog)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label.sizePolicy().hasHeightForWidth())
        self.label.setSizePolicy(sizePolicy)
        self.label.setWordWrap(True)
        self.label.setOpenExternalLinks(True)
        self.label.setObjectName("label")
        self.gridlayout.addWidget(self.label,1,0,1,1)

        self.textBrowser = QtGui.QTextBrowser(QgsPluginInstallerPluginErrorDialog)
        self.textBrowser.setMinimumSize(QtCore.QSize(0,0))
        self.textBrowser.setFocusPolicy(QtCore.Qt.NoFocus)
        self.textBrowser.setObjectName("textBrowser")
        self.gridlayout.addWidget(self.textBrowser,2,0,1,1)

        spacerItem = QtGui.QSpacerItem(503,10,QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Fixed)
        self.gridlayout.addItem(spacerItem,3,0,1,1)

        self.label1 = QtGui.QLabel(QgsPluginInstallerPluginErrorDialog)
        self.label1.setFrameShape(QtGui.QFrame.NoFrame)
        self.label1.setFrameShadow(QtGui.QFrame.Plain)
        self.label1.setWordWrap(True)
        self.label1.setObjectName("label1")
        self.gridlayout.addWidget(self.label1,4,0,1,1)

        self.buttonBox = QtGui.QDialogButtonBox(QgsPluginInstallerPluginErrorDialog)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.No|QtGui.QDialogButtonBox.NoButton|QtGui.QDialogButtonBox.Yes)
        self.buttonBox.setObjectName("buttonBox")
        self.gridlayout.addWidget(self.buttonBox,6,0,1,1)

        spacerItem1 = QtGui.QSpacerItem(10,10,QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Fixed)
        self.gridlayout.addItem(spacerItem1,0,0,1,1)

        self.retranslateUi(QgsPluginInstallerPluginErrorDialog)
        QtCore.QObject.connect(self.buttonBox,QtCore.SIGNAL("rejected()"),QgsPluginInstallerPluginErrorDialog.reject)
        QtCore.QObject.connect(self.buttonBox,QtCore.SIGNAL("accepted()"),QgsPluginInstallerPluginErrorDialog.accept)
        QtCore.QMetaObject.connectSlotsByName(QgsPluginInstallerPluginErrorDialog)

    def retranslateUi(self, QgsPluginInstallerPluginErrorDialog):
        QgsPluginInstallerPluginErrorDialog.setWindowTitle(QtGui.QApplication.translate("QgsPluginInstallerPluginErrorDialog", "Error loading plugin", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("QgsPluginInstallerPluginErrorDialog", "The plugin seems to be invalid or have unfulfilled dependencies. It has been installed, but can\'t be loaded. If you really need this plugin, you can contact its author or <a href=\"http://lists.osgeo.org/mailman/listinfo/qgis-user\">QGIS users group</a> and try to solve the problem. If not, you can just uninstall it. Here is the error message below:", None, QtGui.QApplication.UnicodeUTF8))
        self.label1.setText(QtGui.QApplication.translate("QgsPluginInstallerPluginErrorDialog", "Do you want to uninstall this plugin now? If you\'re unsure, probably you would like to do this.", None, QtGui.QApplication.UnicodeUTF8))

import resources_rc
