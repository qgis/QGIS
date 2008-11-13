# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'qgsplugininstallerrepositorybase.ui'
#
# Created: Wed Nov 12 23:21:49 2008
#      by: PyQt4 UI code generator 4.3.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_QgsPluginInstallerRepositoryDetailsDialogBase(object):
    def setupUi(self, QgsPluginInstallerRepositoryDetailsDialogBase):
        QgsPluginInstallerRepositoryDetailsDialogBase.setObjectName("QgsPluginInstallerRepositoryDetailsDialogBase")
        QgsPluginInstallerRepositoryDetailsDialogBase.resize(QtCore.QSize(QtCore.QRect(0,0,522,191).size()).expandedTo(QgsPluginInstallerRepositoryDetailsDialogBase.minimumSizeHint()))

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred,QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(QgsPluginInstallerRepositoryDetailsDialogBase.sizePolicy().hasHeightForWidth())
        QgsPluginInstallerRepositoryDetailsDialogBase.setSizePolicy(sizePolicy)
        QgsPluginInstallerRepositoryDetailsDialogBase.setWindowIcon(QtGui.QIcon(":/plugins/installer/qgis-icon.png"))

        self.gridlayout = QtGui.QGridLayout(QgsPluginInstallerRepositoryDetailsDialogBase)
        self.gridlayout.setObjectName("gridlayout")

        self.label = QtGui.QLabel(QgsPluginInstallerRepositoryDetailsDialogBase)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label.sizePolicy().hasHeightForWidth())
        self.label.setSizePolicy(sizePolicy)
        self.label.setObjectName("label")
        self.gridlayout.addWidget(self.label,0,0,1,1)

        spacerItem = QtGui.QSpacerItem(16,27,QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Minimum)
        self.gridlayout.addItem(spacerItem,0,1,1,1)

        self.editName = QtGui.QLineEdit(QgsPluginInstallerRepositoryDetailsDialogBase)
        self.editName.setObjectName("editName")
        self.gridlayout.addWidget(self.editName,0,2,1,2)

        self.label_2 = QtGui.QLabel(QgsPluginInstallerRepositoryDetailsDialogBase)
        self.label_2.setObjectName("label_2")
        self.gridlayout.addWidget(self.label_2,1,0,1,1)

        self.editURL = QtGui.QLineEdit(QgsPluginInstallerRepositoryDetailsDialogBase)
        self.editURL.setObjectName("editURL")
        self.gridlayout.addWidget(self.editURL,1,2,1,2)

        self.checkBoxEnabled = QtGui.QCheckBox(QgsPluginInstallerRepositoryDetailsDialogBase)
        self.checkBoxEnabled.setEnabled(True)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.checkBoxEnabled.sizePolicy().hasHeightForWidth())
        self.checkBoxEnabled.setSizePolicy(sizePolicy)
        self.checkBoxEnabled.setChecked(False)
        self.checkBoxEnabled.setObjectName("checkBoxEnabled")
        self.gridlayout.addWidget(self.checkBoxEnabled,2,2,1,1)

        spacerItem1 = QtGui.QSpacerItem(351,23,QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Minimum)
        self.gridlayout.addItem(spacerItem1,2,3,1,1)

        self.labelInfo = QtGui.QLabel(QgsPluginInstallerRepositoryDetailsDialogBase)
        self.labelInfo.setEnabled(True)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.labelInfo.sizePolicy().hasHeightForWidth())
        self.labelInfo.setSizePolicy(sizePolicy)

        palette = QtGui.QPalette()

        brush = QtGui.QBrush(QtGui.QColor(175,0,0))
        brush.setStyle(QtCore.Qt.SolidPattern)
        palette.setBrush(QtGui.QPalette.Active,QtGui.QPalette.WindowText,brush)

        brush = QtGui.QBrush(QtGui.QColor(175,0,0))
        brush.setStyle(QtCore.Qt.SolidPattern)
        palette.setBrush(QtGui.QPalette.Inactive,QtGui.QPalette.WindowText,brush)

        brush = QtGui.QBrush(QtGui.QColor(128,128,128))
        brush.setStyle(QtCore.Qt.SolidPattern)
        palette.setBrush(QtGui.QPalette.Disabled,QtGui.QPalette.WindowText,brush)
        self.labelInfo.setPalette(palette)

        font = QtGui.QFont()
        font.setWeight(75)
        font.setBold(True)
        self.labelInfo.setFont(font)
        self.labelInfo.setFrameShape(QtGui.QFrame.NoFrame)
        self.labelInfo.setObjectName("labelInfo")
        self.gridlayout.addWidget(self.labelInfo,3,2,1,2)

        self.buttonBox = QtGui.QDialogButtonBox(QgsPluginInstallerRepositoryDetailsDialogBase)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.NoButton|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.gridlayout.addWidget(self.buttonBox,5,0,1,4)

        self.retranslateUi(QgsPluginInstallerRepositoryDetailsDialogBase)
        QtCore.QObject.connect(self.buttonBox,QtCore.SIGNAL("accepted()"),QgsPluginInstallerRepositoryDetailsDialogBase.accept)
        QtCore.QObject.connect(self.buttonBox,QtCore.SIGNAL("rejected()"),QgsPluginInstallerRepositoryDetailsDialogBase.reject)
        QtCore.QMetaObject.connectSlotsByName(QgsPluginInstallerRepositoryDetailsDialogBase)

    def retranslateUi(self, QgsPluginInstallerRepositoryDetailsDialogBase):
        QgsPluginInstallerRepositoryDetailsDialogBase.setWindowTitle(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialogBase", "Repository details", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialogBase", "Name:", None, QtGui.QApplication.UnicodeUTF8))
        self.editName.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialogBase", "Enter a name for the repository", None, QtGui.QApplication.UnicodeUTF8))
        self.editName.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialogBase", "Enter a name for the repository", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialogBase", "URL:", None, QtGui.QApplication.UnicodeUTF8))
        self.editURL.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialogBase", "Enter the repository URL, beginning with \"http://\"", None, QtGui.QApplication.UnicodeUTF8))
        self.editURL.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialogBase", "Enter the repository URL, beginning with \"http://\"", None, QtGui.QApplication.UnicodeUTF8))
        self.checkBoxEnabled.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialogBase", "Enable or disable the repository (disabled repositories will be omitted)", None, QtGui.QApplication.UnicodeUTF8))
        self.checkBoxEnabled.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialogBase", "Enable or disable the repository (disabled repositories will be omitted)", None, QtGui.QApplication.UnicodeUTF8))
        self.checkBoxEnabled.setText(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialogBase", "Enabled", None, QtGui.QApplication.UnicodeUTF8))

import resources_rc
