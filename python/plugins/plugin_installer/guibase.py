# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'guibase.ui'
#
# Created: Sun Sep  7 21:28:01 2008
#      by: PyQt4 UI code generator 4.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_QgsPluginInstallerDialog(object):
    def setupUi(self, QgsPluginInstallerDialog):
        QgsPluginInstallerDialog.setObjectName("QgsPluginInstallerDialog")
        QgsPluginInstallerDialog.resize(QtCore.QSize(QtCore.QRect(0,0,769,395).size()).expandedTo(QgsPluginInstallerDialog.minimumSizeHint()))
        QgsPluginInstallerDialog.setWindowIcon(QtGui.QIcon(":/plugins/installer/qgis-icon.png"))

        self.gridlayout = QtGui.QGridLayout(QgsPluginInstallerDialog)
        self.gridlayout.setObjectName("gridlayout")

        self.tabWidget = QtGui.QTabWidget(QgsPluginInstallerDialog)
        self.tabWidget.setObjectName("tabWidget")

        self.tab = QtGui.QWidget()
        self.tab.setObjectName("tab")

        self.vboxlayout = QtGui.QVBoxLayout(self.tab)
        self.vboxlayout.setObjectName("vboxlayout")

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")

        self.label_5 = QtGui.QLabel(self.tab)
        self.label_5.setEnabled(True)
        self.label_5.setObjectName("label_5")
        self.hboxlayout.addWidget(self.label_5)

        self.lineFilter = QtGui.QLineEdit(self.tab)
        self.lineFilter.setEnabled(True)
        self.lineFilter.setObjectName("lineFilter")
        self.hboxlayout.addWidget(self.lineFilter)

        self.comboFilter1 = QtGui.QComboBox(self.tab)
        self.comboFilter1.setEnabled(True)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.comboFilter1.sizePolicy().hasHeightForWidth())
        self.comboFilter1.setSizePolicy(sizePolicy)
        self.comboFilter1.setObjectName("comboFilter1")
        self.hboxlayout.addWidget(self.comboFilter1)

        self.comboFilter2 = QtGui.QComboBox(self.tab)
        self.comboFilter2.setEnabled(True)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.comboFilter2.sizePolicy().hasHeightForWidth())
        self.comboFilter2.setSizePolicy(sizePolicy)
        self.comboFilter2.setObjectName("comboFilter2")
        self.hboxlayout.addWidget(self.comboFilter2)
        self.vboxlayout.addLayout(self.hboxlayout)

        self.treePlugins = QtGui.QTreeWidget(self.tab)
        self.treePlugins.setAlternatingRowColors(True)
        self.treePlugins.setRootIsDecorated(False)
        self.treePlugins.setItemsExpandable(False)
        self.treePlugins.setSortingEnabled(True)
        self.treePlugins.setAllColumnsShowFocus(True)
        self.treePlugins.setObjectName("treePlugins")
        self.vboxlayout.addWidget(self.treePlugins)

        self.hboxlayout1 = QtGui.QHBoxLayout()
        self.hboxlayout1.setObjectName("hboxlayout1")

        spacerItem = QtGui.QSpacerItem(40,20,QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Minimum)
        self.hboxlayout1.addItem(spacerItem)

        self.buttonInstall = QtGui.QPushButton(self.tab)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.buttonInstall.sizePolicy().hasHeightForWidth())
        self.buttonInstall.setSizePolicy(sizePolicy)
        self.buttonInstall.setMinimumSize(QtCore.QSize(160,0))
        self.buttonInstall.setObjectName("buttonInstall")
        self.hboxlayout1.addWidget(self.buttonInstall)

        self.buttonUninstall = QtGui.QPushButton(self.tab)
        self.buttonUninstall.setEnabled(True)
        self.buttonUninstall.setObjectName("buttonUninstall")
        self.hboxlayout1.addWidget(self.buttonUninstall)
        self.vboxlayout.addLayout(self.hboxlayout1)
        self.tabWidget.addTab(self.tab,"")

        self.tab_2 = QtGui.QWidget()
        self.tab_2.setObjectName("tab_2")

        self.gridlayout1 = QtGui.QGridLayout(self.tab_2)
        self.gridlayout1.setObjectName("gridlayout1")

        self.treeRepositories = QtGui.QTreeWidget(self.tab_2)
        self.treeRepositories.setRootIsDecorated(False)
        self.treeRepositories.setItemsExpandable(False)
        self.treeRepositories.setObjectName("treeRepositories")
        self.gridlayout1.addWidget(self.treeRepositories,0,0,1,7)

        self.checkUpdates = QtGui.QCheckBox(self.tab_2)
        self.checkUpdates.setObjectName("checkUpdates")
        self.gridlayout1.addWidget(self.checkUpdates,1,0,1,1)

        spacerItem1 = QtGui.QSpacerItem(30,20,QtGui.QSizePolicy.Preferred,QtGui.QSizePolicy.Minimum)
        self.gridlayout1.addItem(spacerItem1,1,1,1,1)

        self.buttonFetchRepositories = QtGui.QPushButton(self.tab_2)
        self.buttonFetchRepositories.setEnabled(True)
        self.buttonFetchRepositories.setObjectName("buttonFetchRepositories")
        self.gridlayout1.addWidget(self.buttonFetchRepositories,1,2,1,1)

        spacerItem2 = QtGui.QSpacerItem(40,20,QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Minimum)
        self.gridlayout1.addItem(spacerItem2,1,3,1,1)

        self.buttonAddRep = QtGui.QPushButton(self.tab_2)
        self.buttonAddRep.setObjectName("buttonAddRep")
        self.gridlayout1.addWidget(self.buttonAddRep,1,4,1,1)

        self.buttonEditRep = QtGui.QPushButton(self.tab_2)
        self.buttonEditRep.setObjectName("buttonEditRep")
        self.gridlayout1.addWidget(self.buttonEditRep,1,5,1,1)

        self.buttonDeleteRep = QtGui.QPushButton(self.tab_2)
        self.buttonDeleteRep.setObjectName("buttonDeleteRep")
        self.gridlayout1.addWidget(self.buttonDeleteRep,1,6,1,1)
        self.tabWidget.addTab(self.tab_2,"")
        self.gridlayout.addWidget(self.tabWidget,0,0,1,1)

        self.hboxlayout2 = QtGui.QHBoxLayout()
        self.hboxlayout2.setObjectName("hboxlayout2")

        self.label_3 = QtGui.QLabel(QgsPluginInstallerDialog)
        self.label_3.setObjectName("label_3")
        self.hboxlayout2.addWidget(self.label_3)

        self.buttonClose = QtGui.QPushButton(QgsPluginInstallerDialog)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.buttonClose.sizePolicy().hasHeightForWidth())
        self.buttonClose.setSizePolicy(sizePolicy)
        self.buttonClose.setObjectName("buttonClose")
        self.hboxlayout2.addWidget(self.buttonClose)
        self.gridlayout.addLayout(self.hboxlayout2,1,0,1,1)

        self.retranslateUi(QgsPluginInstallerDialog)
        self.tabWidget.setCurrentIndex(0)
        QtCore.QObject.connect(self.buttonClose,QtCore.SIGNAL("released()"),QgsPluginInstallerDialog.close)
        QtCore.QMetaObject.connectSlotsByName(QgsPluginInstallerDialog)
        QgsPluginInstallerDialog.setTabOrder(self.tabWidget,self.lineFilter)
        QgsPluginInstallerDialog.setTabOrder(self.lineFilter,self.comboFilter1)
        QgsPluginInstallerDialog.setTabOrder(self.comboFilter1,self.comboFilter2)
        QgsPluginInstallerDialog.setTabOrder(self.comboFilter2,self.treePlugins)
        QgsPluginInstallerDialog.setTabOrder(self.treePlugins,self.buttonInstall)
        QgsPluginInstallerDialog.setTabOrder(self.buttonInstall,self.buttonUninstall)
        QgsPluginInstallerDialog.setTabOrder(self.buttonUninstall,self.buttonClose)
        QgsPluginInstallerDialog.setTabOrder(self.buttonClose,self.treeRepositories)
        QgsPluginInstallerDialog.setTabOrder(self.treeRepositories,self.buttonFetchRepositories)
        QgsPluginInstallerDialog.setTabOrder(self.buttonFetchRepositories,self.checkUpdates)
        QgsPluginInstallerDialog.setTabOrder(self.checkUpdates,self.buttonAddRep)
        QgsPluginInstallerDialog.setTabOrder(self.buttonAddRep,self.buttonEditRep)
        QgsPluginInstallerDialog.setTabOrder(self.buttonEditRep,self.buttonDeleteRep)

    def retranslateUi(self, QgsPluginInstallerDialog):
        QgsPluginInstallerDialog.setWindowTitle(QtGui.QApplication.translate("QgsPluginInstallerDialog", "QGIS Python Plugin Installer", None, QtGui.QApplication.UnicodeUTF8))
        QgsPluginInstallerDialog.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerDialog", "QGIS Plugin Installer", None, QtGui.QApplication.UnicodeUTF8))
        self.label_5.setText(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Filter:", None, QtGui.QApplication.UnicodeUTF8))
        self.lineFilter.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Display only plugins containing this word in their metadata", None, QtGui.QApplication.UnicodeUTF8))
        self.lineFilter.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Display only plugins containing this word in their metadata", None, QtGui.QApplication.UnicodeUTF8))
        self.comboFilter1.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Display only plugins from given repository", None, QtGui.QApplication.UnicodeUTF8))
        self.comboFilter1.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Display only plugins from given repository", None, QtGui.QApplication.UnicodeUTF8))
        self.comboFilter1.addItem(QtGui.QApplication.translate("QgsPluginInstallerDialog", "all repositories", None, QtGui.QApplication.UnicodeUTF8))
        self.comboFilter2.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Display only plugins with matching status", None, QtGui.QApplication.UnicodeUTF8))
        self.comboFilter2.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Display only plugins with matching status", None, QtGui.QApplication.UnicodeUTF8))
        self.treePlugins.headerItem().setText(0,QtGui.QApplication.translate("QgsPluginInstallerDialog", "Status", None, QtGui.QApplication.UnicodeUTF8))
        self.treePlugins.headerItem().setText(1,QtGui.QApplication.translate("QgsPluginInstallerDialog", "Name", None, QtGui.QApplication.UnicodeUTF8))
        self.treePlugins.headerItem().setText(2,QtGui.QApplication.translate("QgsPluginInstallerDialog", "Version", None, QtGui.QApplication.UnicodeUTF8))
        self.treePlugins.headerItem().setText(3,QtGui.QApplication.translate("QgsPluginInstallerDialog", "Description", None, QtGui.QApplication.UnicodeUTF8))
        self.treePlugins.headerItem().setText(4,QtGui.QApplication.translate("QgsPluginInstallerDialog", "Author", None, QtGui.QApplication.UnicodeUTF8))
        self.treePlugins.headerItem().setText(5,QtGui.QApplication.translate("QgsPluginInstallerDialog", "Repository", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonInstall.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Install, reinstall or upgrade the selected plugin", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonInstall.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Install, reinstall or upgrade the selected plugin", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonInstall.setText(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Install/upgrade plugin", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonUninstall.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Uninstall the selected plugin", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonUninstall.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Uninstall the selected plugin", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonUninstall.setText(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Uninstall plugin", None, QtGui.QApplication.UnicodeUTF8))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab), QtGui.QApplication.translate("QgsPluginInstallerDialog", "Plugins", None, QtGui.QApplication.UnicodeUTF8))
        self.tabWidget.setTabToolTip(self.tabWidget.indexOf(self.tab),QtGui.QApplication.translate("QgsPluginInstallerDialog", "List of available and installed plugins", None, QtGui.QApplication.UnicodeUTF8))
        self.treeRepositories.headerItem().setText(0,QtGui.QApplication.translate("QgsPluginInstallerDialog", "Status", None, QtGui.QApplication.UnicodeUTF8))
        self.treeRepositories.headerItem().setText(1,QtGui.QApplication.translate("QgsPluginInstallerDialog", "Name", None, QtGui.QApplication.UnicodeUTF8))
        self.treeRepositories.headerItem().setText(2,QtGui.QApplication.translate("QgsPluginInstallerDialog", "URL", None, QtGui.QApplication.UnicodeUTF8))
        self.checkUpdates.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Allow the Installer to look for updates and news in enabled repositories on QGIS startup", None, QtGui.QApplication.UnicodeUTF8))
        self.checkUpdates.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Allow the Installer to look for updates and news in enabled repositories on QGIS startup", None, QtGui.QApplication.UnicodeUTF8))
        self.checkUpdates.setText(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Check for updates on startup", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonFetchRepositories.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Add third party plugin repositories to the list", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonFetchRepositories.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Add third party plugin repositories to the list", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonFetchRepositories.setText(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Add 3rd party repositories", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonAddRep.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Add a new plugin repository", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonAddRep.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Add a new plugin repository", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonAddRep.setText(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Add...", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonEditRep.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Edit the selected repository", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonEditRep.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Edit the selected repository", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonEditRep.setText(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Edit...", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonDeleteRep.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Remove the selected repository", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonDeleteRep.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Remove the selected repository", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonDeleteRep.setText(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Delete", None, QtGui.QApplication.UnicodeUTF8))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_2), QtGui.QApplication.translate("QgsPluginInstallerDialog", "Repositories", None, QtGui.QApplication.UnicodeUTF8))
        self.tabWidget.setTabToolTip(self.tabWidget.indexOf(self.tab_2),QtGui.QApplication.translate("QgsPluginInstallerDialog", "List of plugin repositories", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("QgsPluginInstallerDialog", "The plugins will be installed to ~/.qgis/python/plugins", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonClose.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Close the Installer window", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonClose.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Close the Installer window", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonClose.setText(QtGui.QApplication.translate("QgsPluginInstallerDialog", "Close", None, QtGui.QApplication.UnicodeUTF8))

import resources_rc
