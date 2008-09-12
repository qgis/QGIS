# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'repositorybase.ui'
#
# Created: Fri Sep 12 19:21:37 2008
#      by: PyQt4 UI code generator 4.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_QgsPluginInstallerRepositoryDetailsDialog(object):
    def setupUi(self, QgsPluginInstallerRepositoryDetailsDialog):
        QgsPluginInstallerRepositoryDetailsDialog.setObjectName("QgsPluginInstallerRepositoryDetailsDialog")
        QgsPluginInstallerRepositoryDetailsDialog.resize(QtCore.QSize(QtCore.QRect(0,0,522,191).size()).expandedTo(QgsPluginInstallerRepositoryDetailsDialog.minimumSizeHint()))

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred,QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(QgsPluginInstallerRepositoryDetailsDialog.sizePolicy().hasHeightForWidth())
        QgsPluginInstallerRepositoryDetailsDialog.setSizePolicy(sizePolicy)
        QgsPluginInstallerRepositoryDetailsDialog.setWindowIcon(QtGui.QIcon(":/plugins/installer/qgis-icon.png"))

        self.gridlayout = QtGui.QGridLayout(QgsPluginInstallerRepositoryDetailsDialog)
        self.gridlayout.setObjectName("gridlayout")

        self.label = QtGui.QLabel(QgsPluginInstallerRepositoryDetailsDialog)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label.sizePolicy().hasHeightForWidth())
        self.label.setSizePolicy(sizePolicy)
        self.label.setObjectName("label")
        self.gridlayout.addWidget(self.label,0,0,1,1)

        spacerItem = QtGui.QSpacerItem(16,27,QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Minimum)
        self.gridlayout.addItem(spacerItem,0,1,1,1)

        self.editName = QtGui.QLineEdit(QgsPluginInstallerRepositoryDetailsDialog)
        self.editName.setObjectName("editName")
        self.gridlayout.addWidget(self.editName,0,2,1,2)

        self.label_2 = QtGui.QLabel(QgsPluginInstallerRepositoryDetailsDialog)
        self.label_2.setObjectName("label_2")
        self.gridlayout.addWidget(self.label_2,1,0,1,1)

        self.editURL = QtGui.QLineEdit(QgsPluginInstallerRepositoryDetailsDialog)
        self.editURL.setObjectName("editURL")
        self.gridlayout.addWidget(self.editURL,1,2,1,2)

        self.checkBoxEnabled = QtGui.QCheckBox(QgsPluginInstallerRepositoryDetailsDialog)
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

        self.labelInfo = QtGui.QLabel(QgsPluginInstallerRepositoryDetailsDialog)
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

        self.buttonBox = QtGui.QDialogButtonBox(QgsPluginInstallerRepositoryDetailsDialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.NoButton|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.gridlayout.addWidget(self.buttonBox,5,0,1,4)

        self.retranslateUi(QgsPluginInstallerRepositoryDetailsDialog)
        QtCore.QObject.connect(self.buttonBox,QtCore.SIGNAL("accepted()"),QgsPluginInstallerRepositoryDetailsDialog.accept)
        QtCore.QObject.connect(self.buttonBox,QtCore.SIGNAL("rejected()"),QgsPluginInstallerRepositoryDetailsDialog.reject)
        QtCore.QMetaObject.connectSlotsByName(QgsPluginInstallerRepositoryDetailsDialog)

    def retranslateUi(self, QgsPluginInstallerRepositoryDetailsDialog):
        QgsPluginInstallerRepositoryDetailsDialog.setWindowTitle(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialog", "Repository details", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialog", "Name:", None, QtGui.QApplication.UnicodeUTF8))
        self.editName.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialog", "Enter a name for the repository", None, QtGui.QApplication.UnicodeUTF8))
        self.editName.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialog", "Enter a name for the repository", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialog", "URL:", None, QtGui.QApplication.UnicodeUTF8))
        self.editURL.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialog", "Enter the repository URL, beginning with \"http://\"", None, QtGui.QApplication.UnicodeUTF8))
        self.editURL.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialog", "Enter the repository URL, beginning with \"http://\"", None, QtGui.QApplication.UnicodeUTF8))
        self.checkBoxEnabled.setToolTip(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialog", "Enable or disable the repository (disabled repositories will be omitted)", None, QtGui.QApplication.UnicodeUTF8))
        self.checkBoxEnabled.setWhatsThis(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialog", "Enable or disable the repository (disabled repositories will be omitted)", None, QtGui.QApplication.UnicodeUTF8))
        self.checkBoxEnabled.setText(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialog", "Enabled", None, QtGui.QApplication.UnicodeUTF8))
        self.labelInfo.setText(QtGui.QApplication.translate("QgsPluginInstallerRepositoryDetailsDialog", "[place for a warning message]", None, QtGui.QApplication.UnicodeUTF8))

import resources_rc
