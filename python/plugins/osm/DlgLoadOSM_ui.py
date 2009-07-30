# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_files/DlgLoadOSM.ui'
#
# Created: Wed Jul 22 12:16:56 2009
#      by: PyQt4 UI code generator 4.4.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_DlgLoadOSM(object):
    def setupUi(self, DlgLoadOSM):
        DlgLoadOSM.setObjectName("DlgLoadOSM")
        DlgLoadOSM.setWindowModality(QtCore.Qt.ApplicationModal)
        DlgLoadOSM.resize(508, 309)
        DlgLoadOSM.setModal(True)
        self.gridlayout = QtGui.QGridLayout(DlgLoadOSM)
        self.gridlayout.setObjectName("gridlayout")
        self.gridlayout1 = QtGui.QGridLayout()
        self.gridlayout1.setObjectName("gridlayout1")
        self.label = QtGui.QLabel(DlgLoadOSM)
        self.label.setIndent(-1)
        self.label.setObjectName("label")
        self.gridlayout1.addWidget(self.label, 0, 0, 1, 2)
        self.OSMFileEdit = QtGui.QLineEdit(DlgLoadOSM)
        self.OSMFileEdit.setObjectName("OSMFileEdit")
        self.gridlayout1.addWidget(self.OSMFileEdit, 1, 0, 1, 1)
        self.browseOSMButton = QtGui.QPushButton(DlgLoadOSM)
        self.browseOSMButton.setObjectName("browseOSMButton")
        self.gridlayout1.addWidget(self.browseOSMButton, 1, 1, 1, 1)
        self.gridlayout.addLayout(self.gridlayout1, 0, 0, 1, 2)
        self.label_2 = QtGui.QLabel(DlgLoadOSM)
        self.label_2.setObjectName("label_2")
        self.gridlayout.addWidget(self.label_2, 1, 0, 1, 1)
        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")
        self.lstTags = QtGui.QListWidget(DlgLoadOSM)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.lstTags.sizePolicy().hasHeightForWidth())
        self.lstTags.setSizePolicy(sizePolicy)
        self.lstTags.setObjectName("lstTags")
        self.hboxlayout.addWidget(self.lstTags)
        self.gridlayout.addLayout(self.hboxlayout, 2, 0, 1, 2)
        self.hboxlayout1 = QtGui.QHBoxLayout()
        self.hboxlayout1.setObjectName("hboxlayout1")
        self.chkCustomRenderer = QtGui.QCheckBox(DlgLoadOSM)
        self.chkCustomRenderer.setChecked(True)
        self.chkCustomRenderer.setObjectName("chkCustomRenderer")
        self.hboxlayout1.addWidget(self.chkCustomRenderer)
        self.styleCombo = QtGui.QComboBox(DlgLoadOSM)
        self.styleCombo.setMinimumSize(QtCore.QSize(182, 0))
        self.styleCombo.setMaximumSize(QtCore.QSize(182, 16777215))
        self.styleCombo.setObjectName("styleCombo")
        self.hboxlayout1.addWidget(self.styleCombo)
        self.gridlayout.addLayout(self.hboxlayout1, 4, 0, 1, 1)
        self.buttonBox = QtGui.QDialogButtonBox(DlgLoadOSM)
        self.buttonBox.setMaximumSize(QtCore.QSize(110, 16777215))
        self.buttonBox.setBaseSize(QtCore.QSize(110, 0))
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.gridlayout.addWidget(self.buttonBox, 4, 1, 1, 1)
        self.chkReplaceData = QtGui.QCheckBox(DlgLoadOSM)
        self.chkReplaceData.setChecked(False)
        self.chkReplaceData.setObjectName("chkReplaceData")
        self.gridlayout.addWidget(self.chkReplaceData, 3, 0, 1, 1)

        self.retranslateUi(DlgLoadOSM)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("rejected()"), DlgLoadOSM.reject)
        QtCore.QMetaObject.connectSlotsByName(DlgLoadOSM)
        DlgLoadOSM.setTabOrder(self.OSMFileEdit, self.browseOSMButton)
        DlgLoadOSM.setTabOrder(self.browseOSMButton, self.lstTags)
        DlgLoadOSM.setTabOrder(self.lstTags, self.chkCustomRenderer)
        DlgLoadOSM.setTabOrder(self.chkCustomRenderer, self.buttonBox)

    def retranslateUi(self, DlgLoadOSM):
        DlgLoadOSM.setWindowTitle(QtGui.QApplication.translate("DlgLoadOSM", "Load OSM", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("DlgLoadOSM", "OpenStreetMap file to load:", None, QtGui.QApplication.UnicodeUTF8))
        self.browseOSMButton.setText(QtGui.QApplication.translate("DlgLoadOSM", "...", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("DlgLoadOSM", "Add columns for tags:", None, QtGui.QApplication.UnicodeUTF8))
        self.chkCustomRenderer.setText(QtGui.QApplication.translate("DlgLoadOSM", "Use custom renderer", None, QtGui.QApplication.UnicodeUTF8))
        self.chkReplaceData.setText(QtGui.QApplication.translate("DlgLoadOSM", "Replace current data (current layers will be removed)", None, QtGui.QApplication.UnicodeUTF8))

